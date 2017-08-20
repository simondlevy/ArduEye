/* ARDUEYE_STONYMAN_EXAMPLE_V1

   This sketch features the ArduEye_SMH library, which provides an
   interface between the ArduEye and the Stonyman or Hawksbill 
   vision chips.  It also uses the ArduEye_GUI library to visualize
   the output on the ArduEye Processing GUI.  All code can be found
   at www.ardueye.com.

   An image is taken from a connected Stonyman/Hawksbill vision chip,
   the maximum value of the image is found, and the image plus the
   maximum value pixel are sent down to the GUI for display.

   Several serial commands can change the parameters of the vision chip,
   change the image being taken, and calculate and apply a Fixed-Pattern 
   Noise mask.  Type "?" into the serial terminal or processing GUI for
   a list of commands. 

   Started July 12, 2012

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
   ===============================================================================
 */

/*
   Note on image arrays: To save space and speed up operations, we store all images,
   whether 1D or 2D, in a 1D array. The pixels are stored row-wise. So suppose
   array A holds a 16x16 image. Then values A[0] through A[15] store the first row
   of pixel values, values A[16] through A[31] the second row of pixel values, and
   so on.
 */

//=============================================================================
// INCLUDE FILES. The top two files are part of the ArduEye library and should
// be included in the Arduino "libraries" folder.

#include <Stonyman.h>  //Stonyman/Hawksbill vision chip library
#include <GUIClient.h>       //ArduEye processing GUI interface

#include <SPI.h>  //SPI library is needed to use an external ADC
//not supported for MEGA 2560

//==============================================================================
// GLOBAL VARIABLES

// pins
#define RESP   3
#define INCP   4
#define RESV   5
#define INCV   8

// recall from note above that image arrays are stored row-size in a 1D array

static uint16_t img[MAX_PIXELS];        // 1D image array

static uint8_t row=MAX_ROWS;            // maximum rows allowed by memory
static uint8_t col=MAX_COLS;            // maximum cols allowed by memory
static uint8_t skiprow=SKIP_PIXELS;     // pixels to be skipped during readout because of downsampling
static uint8_t skipcol=SKIP_PIXELS;     // pixels to be skipped during readout because of downsampling 
static uint8_t sr=START_PIXEL;          // first pixel row to read
static uint8_t sc=START_PIXEL;          // first pixel col to read

static uint8_t input=0;                 // which vision chip to read from

// FPN calibration. To save memory we are placing the FPN calibration
// array in an uint8_t, since the variation between pixels may be 
// expressed with a single uint8_t. Variable mask_base holds an offset value 
// applied to the entire FPN array. Thus the FPN mask for pixel i will be the 
// value mask[i]+mask_base.
static uint8_t mask[MAX_PIXELS]; // 16x16 FPN calibration image
static uint16_t mask_base=0; // FPN calibration image base.

// Command inputs - for receiving commands from user via Serial terminal
static char command; // command character
static int commandArgument; // argument of command

//we find the maximum pixel to display in the GUI
static uint8_t row_max;
static uint8_t col_max;
static uint8_t points[2];  //points array to send down to the GUI

//object representing our sensor
static Stonyman stonyman(RESP, INCP, RESV, INCV);

//for communicating with GUI
static GUIClient gui;

//=======================================================================
// FUNCTIONS DEFINED FOR THIS SKETCH

// the fitsMemory function makes sure the image will fit
// in Arduino's memory
static bool fitsMemory(uint16_t r,uint16_t c)
{
    return(((r*c)<=MAX_PIXELS));
}

// the fitsArray function makes sure the image will not
// be larger than the Stonyman 112x112 array size
static bool fitsArray(uint16_t r,uint16_t c,uint16_t str,uint16_t stc,uint16_t skr,uint16_t skc)
{
    return( (((str+r*skr)<=112)&&((stc+c*skc)<=112)) );
}

// the processCommands function reads and responds to commands sent to
// the Arduino over the serial connection.  
static void processCommands()
{
    char charbuf[20];

    // PROCESS USER COMMANDS, IF ANY
    if (Serial.available()>0) // Check Serial buffer for input from user
    { 

        // get user command and argument
        // this function also checks for the presence of the GUI
        // so you must use this function if you are using the GUI 
        gui.getCommand(&command,&commandArgument); 

        //switch statement to process commands
        switch (command) {

            //CHANGE ADC TYPE
            case 'a':
                /*
                   if(commandArgument==0)
                   {
                   Serial.println("Onboard ADC");
                   }
                   if(commandArgument==1)
                   {
                   Serial.println("External ADC (doesn't work with Mega2560)");
                   }*/
                break;

                // Reset the chip - use this command if you plug in a new Stonyman chip w/o power cycling
            case 'b':
                //initialize ArduEye Stonyman
                row=MAX_ROWS;            //maximum rows allowed by memory
                col=MAX_COLS;            //maximum cols allowed by memory
                skiprow=SKIP_PIXELS;     //pixels to be skipped during readout because of downsampling
                skipcol=SKIP_PIXELS;     //pixels to be skipped during readout because of downsampling 
                sr=START_PIXEL;          //first pixel row to read
                sc=START_PIXEL;          //first pixel col to read
                input=0;            //which vision chip to read from

                stonyman.begin();
                //set the initial binning on the vision chip
                stonyman.setBinning(skipcol,skiprow);
                Serial.println("Chip reset");
                break;

                // Set number of columns
            case 'c':   
                if(fitsMemory(row,commandArgument)&&fitsArray(row,commandArgument,sr,sc,skiprow,skipcol))
                {
                    col=commandArgument;
                    sprintf(charbuf,"cols = %d",commandArgument);
                    Serial.println(charbuf);
                } 
                else Serial.println("exceeds memory or 112x112 array size");
                break;

                //set starting col
            case 'C':   
                if(fitsMemory(row,col)&&fitsArray(row,col,sr,commandArgument,skiprow,skipcol))
                {
                    sc=commandArgument; 
                    sprintf(charbuf,"start col = %d",sc);
                    Serial.println(charbuf);
                }
                else Serial.println("exceeds memory or 112x112 array size");
                break;

                //set amplifier gain
            case 'g':
                stonyman.setAmpGain(commandArgument);
                sprintf(charbuf,"Amplifier gain = %d",commandArgument);
                Serial.println(charbuf);
                break;

                //set horizontal binning
            case 'h':
                if(fitsMemory(row,col)&&fitsArray(row,col,sr,sc,skipcol,commandArgument))
                {
                    skipcol=commandArgument;
                    stonyman.setBinning(skipcol,skiprow);
                    sprintf(charbuf,"Horiz Binning = %d",skipcol);
                    Serial.println(charbuf);      
                }
                else Serial.println("exceeds memory or 112x112 array size");
                break;

                // calculate FPN mask and apply it to current image
            case 'f': 
                stonyman.getImageAnalog(img,sr,row,skiprow,sc,col,skipcol,input);
                stonyman.calcMask(img,row*col,mask,&mask_base);
                Serial.println("FPN Mask done");  
                break;   

                //change VREF
            case 'l':
                stonyman.setVref(commandArgument);
                sprintf(charbuf,"VREF = %d",commandArgument);
                Serial.println(charbuf);  
                break;

                //print the current array over Serial in Matlab format      
            case 'm':
                stonyman.sectionToMatlabAnalog(sr,row,skiprow,sc,col,skipcol,input);
                break;

                //print the entire chip over Serial in Matlab format
            case 'M':   
                stonyman.chipToMatlabAnalog(input);
                break;

                //change NBIAS
            case 'n': // set pinhole column
                stonyman.setNbias(commandArgument);
                sprintf(charbuf,"NBIAS = %d",commandArgument);
                Serial.println(charbuf);  
                break;     

                // Set number of rows
            case 'r':
                if(fitsMemory(commandArgument,col)&&fitsArray(commandArgument,col,sr,sc,skiprow,skipcol))
                {
                    row=commandArgument; 
                    sprintf(charbuf,"rows = %d",row);
                    Serial.println(charbuf);
                }
                else Serial.println("exceeds memory or 112x112 array size");
                break;   

                //set starting row
            case 'R':
                if(fitsMemory(row,col)&&fitsArray(row,col,commandArgument,sc,skiprow,skipcol))
                {
                    sr=commandArgument; 
                    sprintf(charbuf,"start row = %d",sr);
                    Serial.println(charbuf);
                }
                else Serial.println("exceeds memory or 112x112 array size");
                break;

                //change chip select
            case 's':
                input=commandArgument;
                sprintf(charbuf,"chip select = %d",input);
                Serial.println(charbuf);
                break;

                //set vertical binning
            case 'v':
                if(fitsMemory(row,col)&&fitsArray(row,col,sr,sc,commandArgument,skipcol))
                {
                    skiprow=commandArgument;
                    stonyman.setBinning(skipcol,skiprow);
                    sprintf(charbuf,"Vertical Binning = %d",skiprow);
                    Serial.println(charbuf);    
                } 
                else Serial.println("exceeds memory or 112x112 array size");
                break;

                // ? - print up command list
            case '?':
                Serial.println("a: ADC"); 
                Serial.println("b: reset"); 
                Serial.println("c: cols"); 
                Serial.println("C: start col");
                Serial.println("g: amp gain"); 
                Serial.println("h: hor binning"); 
                Serial.println("f: FPN mask"); 
                Serial.println("l: VREF"); 
                Serial.println("m: matlab section"); 
                Serial.println("M: matlab all");
                Serial.println("n: NBIAS");
                Serial.println("r: rows");
                Serial.println("R: start row");
                Serial.println("s: chip select");
                Serial.println("v: vert binning"); 
                break;

            default:
                break;
        }
    }
}

//=======================================================================
// ARDUINO SETUP AND LOOP FUNCTIONS

void setup() 
{
    // initialize serial port
    Serial.begin(115200); //GUI defaults to this baud rate

    //initialize SPI (needed for external ADC
    SPI.begin();

    //initialize ArduEye Stonyman
    stonyman.begin();

    //set the initial binning on the vision chip
    stonyman.setBinning(skipcol,skiprow);
}

void loop() 
{
    //process commands from serial (should be performed once every execution of loop())
    processCommands();

    //get an image from the stonyman chip
    stonyman.getImageAnalog(img,sr,row,skiprow,sc,col,skipcol,input);

    //find the maximum value.  This actually takes an image a second time, so
    //to speed up this loop you should comment this out
    stonyman.findMaxAnalog(sr,row,skiprow,sc,col,skipcol,input,&row_max,&col_max);

    //apply an FPNMask to the image.  This needs to be calculated with the "f" command
    //while the vision chip is covered with a white sheet of paper to expose it to 
    //uniform illumination.  Once calculated, it will produce a better image  
    stonyman.applyMask(img,row*col,mask,mask_base);

    //if GUI is enabled then send image for display
    gui.sendImage(row,col,img,row*col);

    //if GUI is enabled then send max point for display
    points[0]=(uint8_t)row_max;
    points[1]=(uint8_t)col_max;
    gui.sendPoints(row,col,points,1);
}
