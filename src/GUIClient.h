/*
   GUIClient.h Library to interface with the Centeye Processing GUI

   Functions to send data to the ArduEye processing GUI for display
   For example, send an image array to the GUI and have it display
   the image in the GUI window. A special serial command "!1" is
   sent from the GUI to enable all functions, so that a bunch of 
   unreadable data isn't sent to the serial monitor.

   Working revision started July 9, 2012

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

#pragma once

#include <Arduino.h>

/**
  * A class for communicating with a GUI like the one in ArduEyeGUI.pde
  */
class GUIClient
{
    public:

        /**
          * Constructs a GUIClient object.
          * GUI functions must be enabled by command sent from GUI on start-up.
          */
        GUIClient(void);		

        /**
        * Enables all functions. Called in response to special command from GUI.
        */
        void start(void);		//allow Arduino to send data

        /**
          * Disables all functions so unreadable data isn't sent to  the serial monitor
          */
        void stop(void);		//don't allow Arduino to send data

        /**
        * Sends the escape character plus another chacater.
        * Can be used for sending header information.
        * @param out extra character to send
        */
        void sendEscChar(char extra);

        /**
        * Writes a data byte to the serial port.  
        * If the byte is the escape character, then it is duplicated to 
        * indicate that it is a regular data character.
        * @param data the data byte to send
        */
        void sendDataByte(uint8_t data);	

        /**
        * Checks the serial port for incoming commands and parses them into
        * command and argument using the format "X#", where "X" is a single
        * character and "#" is an optional number. The number "#" may be omitted,
        * or may be one or more digits in size. The extracted command character
        * and number argument are returned via pointers.  The special command !0
        * and !1, which enables and disables the GUI, is intercepted here, but
        * still passed through.
        * @param command the single-character command
        * @param argument the command argument
        */

        void getCommand(char * command, int * argument);

        /**
        *	Sends an image to the GUI for display.
        *
        *	@param rows number of rows in image
        *	@param cols number of cols in image
        *	@param pixels a 1D array of uint16_t pixel values in the image
        *	@param size number of pixels in image (rows*cols)
        */

        void sendImage(uint8_t rows, uint8_t cols, uint16_t * pixels, uint16_t size);

        /**
         * Eight-bit version of above.
         */
        void sendImage(uint8_t rows, uint8_t cols, uint8_t * pixels, uint16_t size);


        /**
        * Sends an array of vectors to the GUI for display
        *
        * @param rows number of rows in vector display (NOT IMAGE)
        * @param cols number of cols in vector display (NOT IMAGE)
        * @param vectors an array of vectors in [X1,Y1,X2,Y2,...] format
        * @param numvecs number of vectors (size of vector/2)
        *
        * Example:
        *
        * &nbsp;&nbsp;&nbsp;&nbsp;<tt>uint16_t vectors[4] = {vx1,vy1,vx2,vy2};</tt><br>
        * &nbsp;&nbsp;&nbsp;&nbsp;<tt>sendVectors(1,2,vectors, 2);</tt><br><br>
        * displays a 1x2 array of two vectors in the display windows
        * which means vx will display on the left and vy on the right
        */
        void sendVectors(uint8_t rows, uint8_t cols, uint16_t * vectors, uint16_t numvecs);

        /**
         * Eight-bit version of above.
         */
        void sendVectors(uint8_t,uint8_t,int8_t*,uint16_t);

        /**
        * Sends an array of points to highlight in the GUI display.
        *
        * @param rows number of rows in image
        * @param cols number of cols in image
        * @param points an array of points in [r1,c1,r2,c2,...] format
        * @param numpts number of points (size of points/2)
        *
        * Example:
        *
        * &nbsp;&nbsp;&nbsp;&nbsp;<tt>char points[4]={2,4,10,11};</tt><br>
        * &nbsp;&nbsp;&nbsp;&nbsp;<tt>sendpoints(16,16,points,2);</tt><br><br>
        * on a 16x16 array, highlights the points (2,4) and (10,11)
        */

        void sendPoints(uint8_t,uint8_t,uint8_t*,uint16_t);


    private:

        bool detected;	 //whether the GUI is detected


};
