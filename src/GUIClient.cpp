/*
   GUIClient.cpp
   Centeye Library to interface with the Processing GUI

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
 */

#include <Arduino.h>
#include <GUIClient.h>

/*********************************************************************/
//	Constructor
//	Defaults to no GUI, no GUI commands will be sent
/*********************************************************************/

GUIClient::GUIClient(void)
{
    // initialize this instance's variables
    detected=false;	//arduGUI not detected
}

/*********************************************************************/
//	start
//	Enables the interface to start sending data to the GUI
/*********************************************************************/

void GUIClient::start(void)
{
    detected=true;	//GUI detected
}

/*********************************************************************/
//	stop
//	Disables the interface so that no data is sent to the GUI
/*********************************************************************/

void GUIClient::stop(void)
{
    detected=false;	//GUI not detected
}

/*********************************************************************/
//	getCommand
//	checks the serial port for incoming commands and parses them
//	into command and argument using the format "X#", where "X" is a //	single character and "#" is an optional number. The number "#" 
//	may be omitted, or may be one or more digits in size. The 
//	extracted command character and number argument are returned
// 	via pointers.  The special command !0 and !1, which enables and
//	disables the GUI, is intercepted here, but still passed through.
/*********************************************************************/

void GUIClient::getCommand(char *command, int *argument) 
{
    char cmdbuf[11];

    // initialize
    for (uint8_t i=0; i<11; ++i)
        cmdbuf[i] = 0;

    // delay to ensure that all stuff is sent through serial port
    delay(100);

    // load cmdbuf
    for (uint8_t i=0; i<10 && Serial.available(); ++i) {
        cmdbuf[i] = Serial.read();
    }

    // clear end of array
    cmdbuf[10]=0;

    // clear rest of buffer
    while (Serial.available())
        ;

    // get command
    *command = cmdbuf[0];

    // get argument
    sscanf(cmdbuf+1,"%d",argument);

    // Turns on or off GUI mode. USE ONLY WHEN CONNECTED TO PROCESSING GUI
    // Otherwise you'll get a lot of gobblygook on the serial monitor...
    if(*command=='!')
    {
        if(*argument==0) {
            stop();
            Serial.println("Arduino Out! GUI off");
        }
        if(*argument==1) {
            start();
            Serial.println("Arduino Here! GUI on");
        }
    }        
}

/*********************************************************************/
//	sendEscChar
//	write the sent-in uint8_t preceded by the escape character.  
//	This is used for special characters
/*********************************************************************/

void GUIClient::sendEscChar(uint8_t char_out)
{ 
    Serial.write(ESC);		//send escape char
    Serial.write(char_out);	//send special char
}

/*********************************************************************/
//	sendDataByte
//	writes the data uint8_t to the serial port.  If the data uint8_t is
//	the escape character, then it is duplicated to indicate that
//	it is a regular data uint8_t.
/*********************************************************************/

void GUIClient::sendDataByte(uint8_t data_out)
{
    if(data_out!=ESC)
        Serial.write(data_out);		//send data uint8_t
    else
    {
        Serial.write(data_out);		//duplicate escape character
        Serial.write(data_out);
    }

}

/*********************************************************************/
//	sendImage (uint16_t version)
//	sends an image to the GUI for display
//
//	ARGUMENTS:
//	rows: number of rows in image
//	cols: number of cols in image
//	pixels: a 1D array of uint16_t pixel values in the image
//	size: number of pixels in image (rows*cols)
/*********************************************************************/

void GUIClient::sendImage(uint8_t rows,uint8_t cols,uint16_t *pixels, uint16_t size)
{

    union	//to get the signed uint8_ts to format properly, use a union
    {	
        uint16_t i_out;
        uint8_t b[2];
    }u;

    if(detected)	//if GUI is detected, send uint8_ts
    {
        sendEscChar(START);	//send start packet

        sendDataByte(IMAGE);		//write image header
        sendDataByte(rows);		//write rows of image
        sendDataByte(cols);		//write cols of image

        //for some reason, Serial.write(uint8_t_array,num)
        //doesn't work over a certain number of uint8_ts
        //so send uint8_ts one at a time
        for (uint16_t i=0;i<size;i++)	
        {
            u.i_out=pixels[i];		//put two uint8_ts into union
            sendDataByte(u.b[0]);	//send first uint8_t 
            sendDataByte(u.b[1]);	//send second uint8_t
        }

        sendEscChar(STOP);		//send stop packet
    }

}

/*********************************************************************/
//	sendImage (char version)
//	sends an image to the GUI for display
//
//	ARGUMENTS:
//	rows: number of rows in image
//	cols: number of cols in image
//	pixels: a 1D array of char pixel values in the image
//	size: number of pixels in image (rows*cols)
/*********************************************************************/

void GUIClient::sendImage(uint8_t rows,uint8_t cols,char *pixels,
        uint16_t size)
{

    if(detected)	//if GUI is detected, send uint8_ts
    {
        sendEscChar(START);	//send start packet

        sendDataByte(IMAGE_CHAR);		//write image header
        sendDataByte(rows);		//write rows of image
        sendDataByte(cols);		//write cols of image

        //for some reason, Serial.write(uint8_t_array,num)
        //doesn't work over a certain number of uint8_ts
        //so send uint8_ts one at a time
        for (uint16_t i=0;i<size;i++)	
        {
            sendDataByte((uint8_t)pixels[i]);	//send first uint8_t 
        }

        sendEscChar(STOP);		//send stop packet
    }
}

/*********************************************************************/
//	sendVectors (uint16_t version)
//	sends an image to the GUI for display
//
//	ARGUMENTS:
//	rows: number of rows in vector display (NOT IMAGE)
//	cols: number of cols in vector display (NOT IMAGE)
//	vector: an array of vectors in [X1,Y1,X2,Y2,...] format
//	num_vectors: number of vectors (size of vector/2)
//
//	EXAMPLES:
//	uint16_t vector[4]={vx1,vy1,vx2,vy2}
//	sendVectors(1,2,vector,2)
//	displays a 1x2 array of two vectors in the display windows
//	which means vx will display on the left and vy on the right
/*********************************************************************/

void GUIClient::sendVectors(uint8_t rows,uint8_t cols,uint16_t *vector,uint16_t num_vectors)
{ 

    union
    {	//to get the signed uint8_ts to format properly, use a union
        uint16_t i_out;
        uint8_t b[2];
    }u;

    if(detected)	//if GUI is detected, send uint8_ts
    {
        sendEscChar(START);		//send start packet

        sendDataByte(VECTORS_SHORT);		//send vector header
        sendDataByte(rows);			//send rows of vectors
        sendDataByte(cols);			//send cols of vectors

        for (uint16_t i=0;i<num_vectors*2;i+=2)
        {
            u.i_out=vector[i];		//put two uint8_ts into union
            sendDataByte(u.b[0]);	//send first uint8_t 
            sendDataByte(u.b[1]);	//send second uint8_t
            u.i_out=vector[i+1];		//put two uint8_ts into union
            sendDataByte(u.b[0]);	//send first uint8_t 
            sendDataByte(u.b[1]);	//send second uint8_t
        }

        sendEscChar(STOP);			//send stop packet
    }
}

/*********************************************************************/
//	sendVectors (char version)
//	sends an image to the GUI for display
//
//	ARGUMENTS:
//	rows: number of rows in vector display (NOT IMAGE)
//	cols: number of cols in vector display (NOT IMAGE)
//	vector: an array of vectors in [X1,Y1,X2,Y2,...] format
//	num_vectors: number of vectors (size of vector/2)
//
//	EXAMPLES:
//	char vector[4]={vx1,vy1,vx2,vy2}
//	sendVectors(1,2,vector,2)
//	displays a 1x2 array of two vectors in the display windows
//	which means vx will display on the left and vy on the right
/*********************************************************************/

void GUIClient::sendVectors(uint8_t rows,uint8_t cols, int8_t *vector, uint16_t num_vectors)
{ 

    if(detected)	//if GUI is detected, send uint8_ts
    {
        sendEscChar(START);		//send start packet

        sendDataByte(VECTORS);		//send vector header
        sendDataByte(rows);			//send rows of vectors
        sendDataByte(cols);			//send cols of vectors

        for (uint16_t i=0;i<num_vectors*2;i+=2)
        {
            sendDataByte((uint8_t)vector[i]);
            sendDataByte((uint8_t)vector[i+1]);
        }

        sendEscChar(STOP);			//send stop packet
    }
}

/*********************************************************************/
//	sendPoints 
//	sends an array of points to highlight in the GUI display
//
//	ARGUMENTS:
//	rows: number of rows in image
//	cols: number of cols in image
//	points: an array of points in [r1,c1,r2,c2,...] format
//	num_vectors: number of points (size of points/2)
//
//	EXAMPLES:
//	char points[4]={2,4,10,11}
//	sendpoints(16,16,points,2)
//	on a 16x16 array, highlights the points (2,4) and (10,11)
/*********************************************************************/

void GUIClient::sendPoints(uint8_t rows, uint8_t cols, uint8_t *points, uint16_t num_points)
{ 
    if(detected)	//if GUI is detected, send uint8_ts
    {
        sendEscChar(START);		//send start packet

        sendDataByte(POINTS);			//send points header
        sendDataByte(rows);			//send rows of image
        sendDataByte(cols);			//send cols of image

        for (uint16_t i=0;i<num_points*2;i+=2)
        {
            sendDataByte(points[i]);		//send point row
            sendDataByte(points[i+1]);		//send point col
        }

        sendEscChar(STOP);			//send stop packet
    }

}
