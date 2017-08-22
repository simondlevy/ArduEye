
/* 
 This sketch features the Centeye GUI library, which allows you
 to visualize the output of the ArduEye in the ArduEye 
 Processing GUI.  All code can be found at www.ardueye.com.

 An image is taken using the stonyman library and several data 
 sets are sent down to the processing GUI.
 
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
 */

//=============================================================================
// INCLUDE FILES. The top two files are part of the ArduEye library and should
// be included in the Arduino "libraries" folder.

#include <Stonyman.h>       // Stonyman vision chip library
#include <StonymanUtils.h>  // Stonyman utilities library
#include <ImageUtils.h>     // Image utilities library
#include <GUIClient.h>      // Processing GUI interface

#include <SPI.h>  //SPI library is needed to use an external ADC

//==============================================================================
// GLOBAL VARIABLES

// pins
static const uint8_t RESP = 3;
static const uint8_t INCP = 4;
static const uint8_t RESV = 5;
static const uint8_t INCV = 8;


// recall from note above that image arrays are stored row-size in a 1D array

static uint16_t img[MAX_PIXELS];         //1D image array
static uint16_t row = MAX_ROWS;          //maximum rows allowed by memory
static uint16_t col = MAX_COLS;          //maximum cols allowed by memory
static uint16_t skiprow = SKIP_PIXELS;   //pixels to be skipped during readout because of downsampling
static uint16_t skipcol = SKIP_PIXELS;   //pixels to be skipped during readout because of downsampling 
static uint16_t sr = START_PIXEL;        //first pixel row to read
static uint16_t sc = START_PIXEL;        //first pixel col to read

static uint16_t input=0;            //which vision chip to read from

// FPN calibration. To save memory we are placing the FPN calibration
// array in an uint8_t, since the variation between pixels may be 
// expressed with a single byte. Variable mask_base holds an offset value 
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

//a set of vectors to send down
static int8_t vectors[8];

//object representing our sensor
static Stonyman stonyman(RESP, INCP, RESV, INCV);

// object for communicating with GUI
static GUIClient gui;

//=======================================================================
// FUNCTIONS DEFINED FOR THIS SKETCH

// the processCommands function reads and responds to commands sent to
// the Arduino over the serial connection.  
void processCommands()
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
                Serial.println("Not implemented");
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

                // calculate FPN mask and apply it to current image
            case 'f': 
                {
                    ImageBounds bounds(sr,row,skiprow,sc,col,skipcol);
                    stonymanGetImage(stonyman, img, input, bounds);
                    imgCalcMask(img,row*col,mask,&mask_base);
                    Serial.println("FPN Mask done");  
                }
                break;   

                //change chip select
            case 's':
                input=commandArgument;
                sprintf(charbuf,"chip select = %d",input);
                Serial.println(charbuf);
                break;

                // ? - print up command list
            case '?':
                Serial.println("a: ADC"); 
                Serial.println("f: FPN mask"); 
                Serial.println("s: chip select");
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

    //set up image bounds for this iteration
    ImageBounds bounds(sr,row,skiprow,sc,col,skipcol);

    //get an image from the stonyman chip
    stonymanGetImage(stonyman, img, input, bounds);

    //find the maximum value.  This actually takes an image a second time, so
    //to speed up this loop you should comment this out
    stonymanFindMax(stonyman, input, &row_max, &col_max, bounds);

    //apply an FPNMask to the image.  This needs to be calculated with the "f" command
    //while the vision chip is covered with a white sheet of paper to expose it to 
    //uniform illumination.  Once calculated, it will remove fixed-pattern noise  
    imgApplyMask(img,row*col,mask,mask_base);

    /***********************************************************************************/
    /***********************************************************************************/
    //   GUI EXAMPLE FUNCTIONS
    /***********************************************************************************/
    /***********************************************************************************/

    //if GUI is enabled then send image for display
    gui.sendImage(row,col,img,row*col);

    //if GUI is enabled then send max point for display
    points[0]=(byte)row_max;
    points[1]=(byte)col_max;
    gui.sendPoints(row,col,points,1);

    //if GUI is enabled create an array of 2x2 vectors to send to the GUI
    //these vectors should create an X pattern on the GUI display
    vectors[0]=-50;    //vector1 x
    vectors[1]=50;     //vector1 y

    vectors[2]=50;     //vector2 x
    vectors[3]=50;     //vector2 y

    vectors[4]=-50;    //vector3 x
    vectors[5]=-50;    //vector3 y

    vectors[6]=50;     //vector4 x
    vectors[7]=-50;    //vector4 y

    //send array of vectors, 2x2 display, 4 vectors
    gui.sendVectors(2,2,vectors,4);
}
