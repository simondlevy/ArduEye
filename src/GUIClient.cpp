/*
   GUIClient.cpp Library to interface with the Processing GUI

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

//Defines GUI comm handler special characters
static const int ESC   = 27;	//escape char
static const int START = 1;		//start packet
static const int STOP  = 2;		//stop packet

//Defines GUI comm handler data sets
static const int IMAGE  	= 	  2;	//uint16_t image packet
static const int POINTS 	= 	  4;	//points packet
static const int VECTORS 	 =    6;	//uint8_t vectors packet
static const int IMAGE_CHAR	   =  8;	//uint8_t image packet
static const int VECTORS_SHORT	= 10;	//uint16_t vectors packet

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

void GUIClient::sendEscChar(char extra)
{ 
    Serial.write(ESC);		//send escape char
    Serial.write(extra);	//send extra char
}

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

void GUIClient::sendImage(uint8_t rows,uint8_t cols,uint8_t *pixels, uint16_t size)
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
