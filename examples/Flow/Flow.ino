
/* Optical Flow example

   This sketch features the OpticalFlow library, which helps
   to calculate optical flow between two images using
   several methods which are demonstrated here.

   An image is taken using the stonyman library, optical
   flow is calculated using the ArduEyeOFO library, and
   data is sent to the GUI using the ArduEyeGUI library

   This example supports a Stonyman chip with cell phone optics

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

#include <Stonyman.h>    // Stonymanl vision chip library
#include <GUIClient.h>   // ArduEye processing GUI interface
#include <OpticalFlow.h> // Optical Flow support
#include <ImageUtils.h>  // Some image support functions


#include <SPI.h>  //SPI library is needed to use an external ADC

//==============================================================================
// GLOBAL VARIABLES

// pins
#define RESP   3
#define INCP   4
#define RESV   5
#define INCV   8

// recall from note above that image arrays are stored row-size in a 1D array

static uint16_t last_img[MAX_PIXELS];         //1D image array
static uint16_t current_img[MAX_PIXELS];
static uint16_t row=MAX_ROWS;            //maximum rows allowed by memory
static uint16_t col=MAX_COLS;            //maximum cols allowed by memory
static uint16_t skiprow=SKIP_PIXELS;     //pixels to be skipped during readout because of downsampling
static uint16_t skipcol=SKIP_PIXELS;     //pixels to be skipped during readout because of downsampling 
static uint16_t sr=START_ROW;          //first pixel row to read
static uint16_t sc=START_COL;          //first pixel col to read

static uint16_t inputPin=0;            //which vision chip to read from

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

//a set of vectors to send down
static int8_t vectors[2];

static uint8_t OFType;

//optical flow X and Y
static int16_t filtered_OFX,filtered_OFY;
static int16_t OFX,OFY;

//object representing our sensor
static Stonyman stonyman(RESP, INCP, RESV, INCV);

//object for communicating with GUI
static GUIClient gui;

//=======================================================================
// FUNCTIONS DEFINED FOR THIS SKETCH

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
        switch (command) 
        {

            //CHANGE ADC TYPE
            case 'a':
                Serial.println("Unimplemented");
                /*
                if(commandArgument==0)
                {
                    useDigital = false;
                    Serial.println("Onboard ADC");
                }
                if(commandArgument==1)
                {
                    useDigital = true;
                    Serial.println("External ADC");
                    useDigital = false;
                }*/
                break;

                // calculate FPN mask and apply it to current image
            case 'f': 
                stonyman.getImageAnalog(current_img,sr,row,skiprow,sc,col,skipcol,inputPin);
                stonyman.calcMask(current_img,row*col,mask,&mask_base);
                Serial.println("FPN Mask done");  
                break;   

                //optical flow type  
            case 'o':
                OFType=commandArgument;
                break;

                //change chip select
            case 's':
                inputPin=commandArgument;
                sprintf(charbuf,"input pin= %d",inputPin);
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

    //get an image from the stonyman chip
    stonyman.getImageAnalog(current_img,sr,row,skiprow,sc,col,skipcol,inputPin);

    //apply an FPNMask to the image.  This needs to be calculated with the "f" command
    //while the vision chip is covered with a white sheet of paper to expose it to 
    //uniform illumination.  Once calculated, it will remove fixed-pattern noise  
    stonyman.applyMask(current_img,row*col,mask,mask_base);

    //if GUI is enabled then send image for display
    gui.sendImage(row,col,current_img,row*col);

    //calculate optical flow

    //Image Interpolation 2D with standard "plus" shifting
    if(OFType==0)
        IIA_Plus_2D(current_img,last_img,row,col,200,&OFX,&OFY);
    //Image Interpolation 2D with compact "square" shifting
    if(OFType==1)
        IIA_Square_2D(current_img,last_img,row,col,200,&OFX,&OFY);
    //Lucas Kanade 2D with standard "plus" shifting
    if(OFType==2)
        LK_Plus_2D(current_img,last_img,row,col,200,&OFX,&OFY);
    //Lucas Kanade 2D with compact "square" shifting
    if(OFType==3)
        LK_Square_2D(current_img,last_img,row,col,200,&OFX,&OFY);

    //low pass filter the X shift
    LPF(&filtered_OFX,&OFX,0.35);

    //low pass filter the Y shift
    LPF(&filtered_OFY,&OFY,0.35);

    //put filtered shifts into array to send to GUI
    vectors[0]=filtered_OFX;    //vector1 x
    vectors[1]=filtered_OFY;     //vector1 y

    //send shifts to be displayed on GUI
    gui.sendVectors(1,1,vectors,1);

    //copy current_img to last_img so two frames are kept
    //for optical flow calculation
    imgCopy(current_img,last_img,row*col);

    //small delay
    delay(5);
}


