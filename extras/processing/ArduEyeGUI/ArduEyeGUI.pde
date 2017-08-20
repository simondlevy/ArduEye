/*
arduGUI_v1.pde

Basic GUI to read text, images, vectors, and points from ArduEye

Working revision started February 16, 2011
February 17, 2012: renamed to ArduEyeGUI_v1, escape character changed to 27
March 19, 2012: increased buffer to handle full 112x112 image, added support for
short vectors and char images, and added recording function for all data types.

*/
/*
===============================================================================
Copyright (c) 2012 Centeye, Inc. 
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
    
Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY CENTEYE, INC. ``AS IS'' AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
EVENT SHALL CENTEYE, INC. OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are 
those of the authors and should not be interpreted as representing official 
policies, either expressed or implied, of Centeye, Inc.
===============================================================================*/

//***********************************************************
//Imported Libraries
import processing.serial.*;  //handles serial comms with Arduino
import controlP5.*;  //provides GUI objects like buttons,textfields, etc.

//***********************************************************
//Global Variables

//Serial Variables
Serial myPort;            //serial port instance
static final int BAUD = 115200;

//CONTROLP5 Objects
ControlP5 controlP5;      //controlP5 object for GUI object class
Textfield serialTextfield;    //textfield for typing serial commands  
DropdownList com_ports;   //dropdown list of available com ports
Button myButton;          //button to connect to Arduino
int state=0;              //state of button
Textlabel fpsTextlabel;
Textlabel sendTextlabel;

//Serial log text display
ArrayList text_log;          //array list of strings to be displayed
String string_buffer="";     //buffer for incoming characters    
int string_length=0;         //length of string buffer
int line_width=40;           //number of chars per line of text
int num_lines=25;            //number of lines of text

//Record to file variables
PrintWriter file;
int record_image=0;
int record_vectors=0;
int record_points=0;

//***********************************************************
//Display Variables
PFont myFont;              //The display font:

PImage img;         //main image
PImage overlay_image;      //overlay image (for highlighting points,etc.)

final int MAX_VECTORS=64;
int[] vector_x=new int[64];            //x display vector
int[] vector_y=new int[64];            //y display vector
int vector_rows=0,vector_cols=0;

//***********************************************************
//COMM Variables

final int escape=27;         //ESCAPE SPECIAL CHAR
final int start_packet=1;    //START PACKET
final int end_packet=2;      //END PACKET
final int image_data=2;      //IMAGE DATA
final int pixel_data=4;      //PIXEL DATA
final int vector_data=6;     //VECTOR DATA
final int image_char_data=8;  //CHAR IMAGE DATA
final int vector_short_data=10; //SHORT VECTOR DATA

final int MAX_SIZE=25100;  //big enough to handle 112x112 image + header
byte[] data=new byte[MAX_SIZE];  //packet buffer
int data_index=0;            //buffer index

int store_byte=0;            //whether to store byte in buffer
int in_packet=0;             //whether we are currently in a packet           
byte escape_detected=0;      //whether last byte was an escape

// timing variables
int framecount;
long startMillis;

//***********************************************************
//***********************************************************


//***********************************************************
//setup function
void setup() {

    size(800, 500);  //set size of window

    myFont = createFont("Georgia",20);
    textFont(myFont, 12);  //set font

    //text_log contains serial log display strings
    text_log=new ArrayList();

    //initialize ControlP5
    controlP5 = new ControlP5(this);

    //setup textfield for serial_out data
    serialTextfield = controlP5.addTextfield("serial_out", 100, 150, 160, 20)
        .setFocus(true)
        .setColorBackground(color(200))
        .setColorActive(color(0))
        .setColorForeground(color(0))
        .setColorLabel(color(0))
        .setColorValue(color(0))
        .setColorValueLabel(color(0))
        .setColorCaptionLabel(color(0))
        .setFont(myFont)
        .setVisible(false);

    //setup button for connect/disconnect Arduino
    myButton=controlP5.addButton("connect")
        .setPosition(130,10)
        .setWidth(80)
        .setHeight(20)
        .setColorBackground(color(200))  //set colors
        .setColorActive(color(255))
        .setColorForeground(color(255))
        .setColorLabel(color(0));

    //setup dropdown menu for current serial ports
    com_ports=controlP5.addDropdownList("com-ports", 10, 10, 100, 80)
        .setItemHeight(20)
        .setBarHeight(20);

    for (int i=0;i<Serial.list().length;i++) {  //add serial list to dropdowns
        com_ports.addItem(Serial.list()[i], i);
    }
    com_ports.setColorBackground(color(200));
    com_ports.setColorActive(color(255));
    com_ports.setColorForeground(color(255));
    com_ports.setColorLabel(color(0));

    //initialize image and overlay_image at maximum size
    // (may be revised when actual image data arrives)
    img = createImage(112, 112, ALPHA);
    overlay_image = createImage(112, 112, ARGB);

    // Create text label for Send
    sendTextlabel = controlP5.addTextlabel("sendLabel")
        .setText("Send:")
        .setPosition(10,147)
        .setFont(myFont)
        .setVisible(false);

    // Create text label for FPS
    fpsTextlabel = controlP5.addTextlabel("fpsLabel")
        .setPosition(10,200)
        .setFont(myFont);

    rectMode(CORNERS);
}


//***********************************************************
//Draw Function
void draw() {

    int vector_row_div,vector_col_div;

    background(0);              //set background color
    stroke(color(255, 0, 0));   //set draw color
    fill(255,255,255);          //set text fill color

    // wait till we have something to draw
    if (img == null) return;

    image(img, 350, 50, 400, 400);  //display image
    image(overlay_image, 350, 50, 400, 400);  //dispay overlay image

    for(int i=0;i<text_log.size();i++)  //display text_log strings
        text((String)text_log.get(i),10,80+i*15);

    //display vectors
    int ctr=0;

    int maxi=0;
    for(int r=0;r<vector_rows;r++)    //for each division
        for(int c=0;c<vector_cols;c++)
            if(abs(vector_y[c])>maxi)
            {
                maxi=abs(vector_y[c]);
            }

    if((vector_rows>0)&&(vector_cols>0))  //if there are vectors to display
    {
        vector_row_div=400/vector_rows;  //evenly space vectors over image
        vector_col_div=400/vector_cols;


        for(int r=0;r<vector_rows;r++)    //for each division
            for(int c=0;c<vector_cols;c++)
            {
                stroke(color(0, 255, 0));
                strokeWeight(4);

                //plot vector
                line(350+c*vector_col_div+vector_col_div/2, 
                        50+r*vector_row_div+vector_row_div/2, 
                        350+c*vector_col_div+vector_col_div/2+vector_x[ctr], 
                        50+r*vector_row_div+vector_row_div/2-vector_y[ctr]); 
                ctr++;  
            }
    }


    if((record_image==1)||(record_points==1)||(record_vectors==1))  //indicate that we are recording
    {
        fill(255,0,0); 
        text("Recording... ", 360, 70);  //display recording indicator
    } 
}

//***********************************************************
//serialEvent: Interrupts whenever a byte is received from
//serial port
void serialEvent(Serial p) {

    int new_byte=0;  //incoming byte


    while (p.available () > 0)  //read all available bytes
    {
        new_byte=myPort.read();  //read next byte
        store_byte=1;  //default is store byte in buffer

        if (escape_detected==1)  //if last byte was escape, check for special chars
        {
            switch(new_byte)
            {
                case start_packet:  //special char start packet
                    in_packet=1;      //we are now in a packet
                    data_index=0;     //first byte of packet
                    store_byte=0;     //don't store this
                    break;
                case end_packet:    //special char end packet
                    store_byte=0;     //don't store this

                    if (in_packet==1)  //if we were in a packet
                        processPacket(data, data_index);  //process packet

                    in_packet=0;      //we are no longer in a packet
                    break;
                case escape:        //this is a double escape
                    store_byte=1;     //this byte is data, store it
                    break;
                default:
                    store_byte=0;    //unknown escape char, don't store
                    break;
            }
            escape_detected=0;  //end of escape sequence
        }
        else //if last character wasn't escape
        {
            if (new_byte==escape)  //is this byte escape
            {
                escape_detected=1;  //mark as escape sequence
                store_byte=0;       //don't store
            }
        }

        if (store_byte==1)  //if this is a byte of data
        {
            if (in_packet==1)  //if we're in a packet
            {
                if(data_index<MAX_SIZE)
                {
                    data[data_index]=(byte)new_byte;  //store in packet buffer
                    data_index++;  //increase packet size
                }
                else in_packet=0;
            }
            else   //if the data isn't in packet, assume its text 
            {
                string_buffer=string_buffer+(char)new_byte;  //add to string buffer
                string_length++;  //increase string_length

                //if string_length is as wide as screen or line feed received
                //then we want to create a new string for the next line
                if((string_length==line_width)||((char)new_byte=='\n'))
                {
                    text_log.add(string_buffer);  //add finished string to text log
                    string_buffer="";  //clear string buffer
                    string_length=0;   //set string length to zero
                    if(text_log.size()>=num_lines)  //if text_log strings reach max
                        text_log.remove(0);           //then remove oldest string
                }
            }
        }
    }
}

//***********************************************************
//processPacket: update display based on received packets
void processPacket(byte[] packet, int packet_length)
{
    int minimum=4096;     //min pixel
    int maximum=-4096;    //max pixel
    byte minimum2=127;     //min pixel
    byte maximum2=-128;    //max pixel
    float mult_factor=0;  //scale factor
    float range=0;
    int ctr=0;            //counter
    int[] pix1;
    byte[] pix2;
    short low=0;          //upper and lower bytes
    short high=0;

    switch(packet[0])  //switch of packet ID
    {
        case image_data:  //if image packet received

            // get image size
            int rows = packet[2];
            int cols = packet[1];

            //create blank image of given size
            if((rows>0)&&(cols>0))
                img = createImage(rows, cols, ALPHA);
            else break;

            // report FPS
            framecount++;
            fpsTextlabel.setText("FPS: " +  (int)((double)framecount / (System.currentTimeMillis()-startMillis) * 1000.));

            img.loadPixels();  //load pixel array

            pix1=new int[(packet[1]*packet[2])];  //create integer array

            //each pixel is two bytes, form integer from then
            for (int i = 3; i < ((img.pixels.length*2)+2); i+=2) 
            {
                low = (short)(packet[i] & 0xff);    //low byte
                high = (short)(packet[i+1] & 0xff);  //high byte
                pix1[ctr]=(short) ((high << 8) | low) ;  //combine to get pixel
                if(pix1[ctr]>maximum) maximum=pix1[ctr];  //find max
                if(pix1[ctr]<minimum) minimum=pix1[ctr];  //find min
                ctr++;
            }

            //mult_factor maps pixel values onto 0->255
            range=abs((float)maximum-(float)minimum);
            if((maximum>0)&&(minimum<=0))
                range++;
            if((maximum<0)&&(minimum>=0))
                range++;
            if(range!=0)
                mult_factor=255/range;
            else range=1;

            //set image pixels based on data
            for (int i = 0; i < img.pixels.length; i++) {
                int row = i / cols;
                int col = i % cols;
                row = rows - row - 1; // invert rows
                int k = cols * row + col;
                img.pixels[i] = color(((int)((pix1[k]-minimum)*mult_factor)));
            }

            //recording image
            if(record_image==1)
            {
                //print pixel values to file
                for(int i=0;i<img.pixels.length;i++)
                {
                    file.print(Integer.toString(pix1[i])+" ");
                } 
                file.println();

            }

            img.updatePixels();  //update pixels
            break;
        case image_char_data:  //if image packet received

            //create blank image of given size
            // [rows] [cols] are bytes 2 and 3
            if((packet[2]>0)&&(packet[1]>0))
                img = createImage(packet[2], packet[1], ALPHA);
            else break;

            img.loadPixels();  //load pixel array

            pix2=new byte[(packet[1]*packet[2])];  //create integer array

            //each pixel is two bytes, form integer from then
            for (int i = 3; i < (img.pixels.length+3); i++) 
            {

                pix2[ctr]=(byte)(packet[i] & 0xff);  //combine to get pixel
                if(pix2[ctr]>maximum2) maximum2=pix2[ctr];  //find max
                if(pix2[ctr]<minimum2) minimum2=pix2[ctr];  //find min
                ctr++;
            }

            //mult_factor maps pixel values onto 0->255
            range=abs((float)maximum2-(float)minimum2);
            if((maximum2>0)&&(minimum2<=0))
                range++;
            if((maximum2<0)&&(minimum2>=0))
                range++;
            if(range!=0)
                mult_factor=255/range;
            else range=1;

            //set image pixels based on data
            for (int i = 0; i < img.pixels.length; i++) 
                //img.pixels[i] = color((255-(int)((pix1[i]-minimum)*mult_factor)));
                img.pixels[i] = color(((int)((pix2[i]-minimum2)*mult_factor)));

            //recording image
            if(record_image==1)
            {
                //print pixel values to file
                for(int i=0;i<img.pixels.length;i++)
                {
                    file.print(Integer.toString(pix2[i])+" ");
                } 
                file.println();

            }

            img.updatePixels();  //update pixels
            break;

        case pixel_data:  //if points data received

            //create image based on [rows] [cols], bytes 2 and 3
            overlay_image = createImage(packet[2], packet[1], ARGB);

            overlay_image.loadPixels();  //load pixels

            //make all pixels transparent
            for (int i = 0; i < overlay_image.pixels.length; i++) 
                overlay_image.pixels[i] = 0;

            //for each pair of pixels, color the proper pixel red
            for (int i=3; i<packet_length-1;i+=2)
                overlay_image.pixels[(packet[i]*packet[1]+packet[i+1])]=color(255, 0, 0);

            overlay_image.updatePixels();  //update pixels

            //recording points
            if(record_points==1)
            {
                //print points values to file
                for(int i=3;i<packet_length-1;i+=2)
                {
                    file.print(Integer.toString(packet[i])+" "+Integer.toString(packet[i+1])+" ");
                } 
                file.println(";");

            }
            break;

        case vector_data:    //if vector received 
            vector_rows=packet[1];
            vector_cols=packet[2];

            ctr=0;
            for(int i=3;i<(packet_length-1);i+=2)
            {

                vector_x[ctr]=packet[i];    //x component of vector
                vector_y[ctr]=packet[i+1];   //y component of vector
                //println(vector_x[ctr]+" "+vector_y[ctr]);
                ctr++;
            }
            //println();

            //recording vectors
            if(record_vectors==1)
            {
                //print vector values to file
                for(int i=0;i<ctr;i++)
                {
                    file.print(Integer.toString(vector_x[i])+" "+Integer.toString(vector_y[i])+" ");
                } 
                file.println(";");

            }
            break;

        case vector_short_data:    //if 2-byte vectors received 
            vector_rows=packet[1];
            vector_cols=packet[2];

            ctr=0;
            for(int i=3;i<(packet_length-1);i+=4)
            {
                low = (short)(packet[i] & 0xff);    //low byte
                high = (short)(packet[i+1] & 0xff);  //high byte
                vector_x[ctr]=-(short) ((high << 8) | low) ;   //x component of vector
                low = (short)(packet[i+2] & 0xff);    //low byte
                high = (short)(packet[i+3] & 0xff);  //high byte
                vector_y[ctr]=-(short) ((high << 8) | low) ;   //x component of vector
                ctr++;
            }

            //recording points
            if(record_vectors==1)
            {
                //print vector values to file
                for(int i=0;i<ctr;i++)
                {
                    file.print(Integer.toString(vector_x[i])+" "+Integer.toString(vector_y[i])+" ");
                } 
                file.println(";");

            }
            break;

        default:break;
    }
}

//***********************************************************
//serial_out- triggers whenever text is entered in textfield
public void serial_out(String theText) {

    String[] words = split(theText, ' ');

    // receiving text from textfield
    if (theText.equals("clr"))  //if clear command
    {
        text_log.clear();  //clear text_log
    }
    else if ((words.length==3)&&(words[0].equals("record"))&&(record_image==0)&&(record_vectors==0)&&(record_points==0))  //if record plus two other words
    {
        if(words[1].equals("image"))  //if recording image
        {
            file=createWriter(words[2]);  //open file name of third word
            record_image=1;  //we are recording
            text_log.add("recording image to file: "+words[2]); 
        }
        if(words[1].equals("vectors"))
        {
            file=createWriter(words[2]);
            record_vectors=1;
            text_log.add("recording vectors to file: "+words[2]); 
        }
        if(words[1].equals("points"))
        {
            file=createWriter(words[2]);
            record_points=1;
            text_log.add("recording points to file: "+words[2]); 
        }
    }
    else if(theText.equals("stop")||(theText.equals("stop record")))  //stop recording
    {
        if((record_image==1)||(record_vectors==1)||(record_points==1))
        {
            file.flush();  //flush file
            file.close();  //close file
            record_image=0;      //stop record
            record_vectors=0;
            record_points=0;
            text_log.add("finished recording to file"); 
        }  
    }
    else  
    {
        myPort.write(theText);  //send over serial port
    }
}

//***********************************************************
//connect- triggers whenever button is pushed
public void connect() {

    if(state==0)  //we want to connect to the Arduino
    {
        state=1;  //change state

        //connect to current serial port
        myPort = new Serial(this, Serial.list()[(int)com_ports.getValue()], BAUD);
        myPort.clear();    //clear port

        delay(500);        //small delay 
        String out="!0";  //send stop to Arduino if its flooding serial port with output
        myPort.write(out);  

        delay(500);        //small delay
        text_log.clear();  //clear text log 
        text_log.add("Are you there, Arduino?\n");  //ask arduino to respond

        out="!1";         //send start to Arduino (sends text in response)
        myPort.write(out);

        myButton.setLabel("disconnect");  //change button text

        // show communication widgets
        serialTextfield.show(); 
        sendTextlabel.show(); 
        fpsTextlabel.show(); 

        // start timing
        framecount = 0;
        startMillis = System.currentTimeMillis();
    }
    else
    {
        state=0;  //change state

        String out="!0";  //send stop to Arduino
        myPort.write(out);

        text_log.add("Goodbye, Arduino...\n");  //to text log

        myButton.setLabel("connect");  //change button text

        // hide communication widgets
        serialTextfield.hide();
        sendTextlabel.hide(); 
        fpsTextlabel.hide(); 

        delay(500);  //small delay so Arduino can respond
        myPort.stop();  //stop port   
    }
}

//***********************************************************
//stop- executes when window is closed
void stop()
{
    String out="!0";  //stop arduino
    myPort.write(out);

    myPort.stop();  //stop serial port

    super.stop();  //call real stop function
}
