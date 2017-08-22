/*
Stonyman.h Library for the Stonyman Centeye Vision Chip

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

#pragma once

#include <stdint.h>

#if defined (__AVR_ATmega8__)||(__AVR_ATmega168__)|  	(__AVR_ATmega168P__)||(__AVR_ATmega328P__)

//uno can handle 10x10 array and FPN mask
//so we set SKIP_PIXELS=8 to downsample by 8
//and START_ROW/COL to 16 to center the resulting
//80x80 raw grid on the Stonyman raw 112x112 array.

static const uint8_t MAX_ROWS    = 10;
static const uint8_t MAX_COLS    = 10;
static const uint8_t SKIP_PIXELS = 8;
static const uint8_t START_ROW   = 16;
static const uint8_t START_COL   = 16;
static const uint8_t START_PIXEL = 8;  

#else

//If not Uno, assume a more modern board like Mega 2560,
//which can handle 48x48 array and FPN mask
//so we set SKIP_PIXELS=2 to downsample by 2
//and START_PIXELS at row/col 8.  This gives us
//a 48x48 array with superpixels of 2x2.  With the
//start row and col at 8, the 96x96 raw grid is 
//centered on the Stonyman 112x112 raw array.

static const uint8_t MAX_ROWS    = 16;
static const uint8_t MAX_COLS    = 16;
static const uint8_t SKIP_PIXELS = 4;
static const uint8_t START_ROW   = 24;
static const uint8_t START_COL   = 24; 
static const uint8_t START_PIXEL = 18;

#endif

static const uint16_t MAX_PIXELS  = MAX_ROWS * MAX_COLS;

/**
 * A helper class for handling frame-grabbing events.
 * Works row-wise or column-wise, depending on what methods you override.
 * Methods are called automatically by Stonyman object.
 */
class FrameGrabber {

    friend class Stonyman;

    protected:

    /**
     * Does something useful before frame is read.
     */
    virtual void preProcess(void) { }

    /**
     * Does something useful with the pixel just read.
     */
    virtual void handlePixel(uint8_t row, uint8_t col, uint16_t pixel, bool use_amp) 
    { 
        (void)row; (void)col; (void)pixel; (void)use_amp;
    }

    /**
     * Does something useful at the start of a row or column.
     */
    virtual void handleVectorStart(void) { }

    /**
     * Does something useful at the end of a row or coumn.
     */
    virtual void handleVectorEnd(void) { }

    /**
     * Does something useful after frame is read.
     */
    virtual void postProcess(void) { }
};

/**
 * A container class for image bounds
 */
class ImageBounds {

    friend class Stonyman;

    protected:

    uint8_t _rowstart; 
    uint8_t _numrows; 
    uint8_t _rowstride; 
    uint8_t _colstart; 
    uint8_t _numcols; 
    uint8_t _colstride;

    public:

    /**
      * Constructs an ImageBounds object of a specified size for use with Stonyman::processFrame()
      *
      * @param rowstart index of first row
      * @param numrows number of rows
      * @param rowstride stride (step size) for row
      * @param colstart index of first column
      * @param numcols number of columns
      * @param colstride stride (step size) for column
      */
    ImageBounds(uint8_t rowstart, uint8_t numrows, uint8_t rowstride, uint8_t colstart, uint8_t numcols, uint8_t colstride) :
        _rowstart(rowstart), _numrows(numrows), _rowstride(rowstride), _colstart(colstart), _numcols(numcols), _colstride(colstride) { }

    /**
      * Constructs a full-sized (112x112) ImageBounds object for use with Stonyman::processFrame()
      */
     ImageBounds(void) : _rowstart(0), _numrows(112), _rowstride(1), _colstart(0), _numcols(112), _colstride(1) { }
};

/**
 *	A class for interacting with the Stonyman2 vision chip from Centeye, Inc.
 *  For documentation on this chip see 
 *  https://github.com/simondlevy/Centeye/blob/master/extras/docs/Stonyman_Hawksbill_ChipInstructions_Rev10_20130312.pdf
 */
class Stonyman 
{
    public:

        /**
         * Creates a Stonyman object using four bool digital= input signals to the chip.
         * @param resp pin for RESP signal
         * @param incp pin for INCP signal
         * @param resv pin for RESV signal
         * @param incv pin for INCV signal
         * @param inphi pin for optional INPHI (amplifier) signal; tie signal to GND if unused
         */
        Stonyman(uint8_t resp, uint8_t incp, uint8_t resv, uint8_t incv, uint8_t inphi=0);

        /**
         * Initializes the vision chips for normal operation.  Sets vision
         * chip pins to low outputs, clears chip registers, sets biases and
         * config register.  
         * @param vref value for VREF register
         * @param nbias value for NBIAS register
         * @param aobias value for AOBIAS register
         * @param useAmp flag for using amplifier
         */
        void begin(uint8_t vref=30, uint8_t nbias=40, uint8_t aobias=40, bool useAmp=false); 

        /**
         * Sets configuration register
         *
         * @param gain amplifier gain 1-7
         * @param useAmp (0) bypasses amplifier, (1) connects it
         * @param cvdda  (1) connect vdda, always should be connected
         *
         * Example 1: To configure the chip to bypass the amplifier:
         * setConfig(0,0,1);
         *
         * Example 2: To use the amplifier and set the gain to 4:
         * setConfig(4,1,1);
         */
        void setConfig(uint8_t gain, uint8_t useAmp, uint8_t cvdda=1);

        /**
         * Provides a friendlier version of setConfig.  If amplifier gain is set to 
         * zero, amplifier is bypassed.  Otherwise the appropriate amplifier
         * gain (range 1-7) is set.
         *
         *  @param gain gain to set
         */
        void setAmpGain(uint8_t gain);

        /**
         * Configures binning in the focal plane using the VSW and HSW
         * system registers. The super pixels are aligned with the top left 
         * of the image, e.g. "offset downsampling" is not used. This 
         * function is for the Stonyman chip only. 
         * 
         * @param hbin set to 1, 2, 4, or 8 to bin horizontally by that amount
         * @param vbin set to 1, 2, 4, or 8 to bin vertically by that amount
         */
        void setBinning(uint8_t hbin, uint8_t vbin);

        /**
         * Sets the VREF register value 
         * @param vref value to put in the register (0-63)
         */
        void setVref(uint8_t vref);

        /**
         * Sets the NBIAS register value
         * @param nbias value to put in the register (0-63)
         */
        void setNbias(uint8_t nbias);

        /**
         * Sets the AOBIAS register value (0-63)
         * @param aobias value to put in the register (0-63)
         */
        void setAobias(uint8_t aobias);

        /**
         *  Sets biases based on chip voltage
         *  @param vddType
         */
        void setBiasesVdd(uint8_t vddType);

        /**
         * Sets all three biases
         * @param vref value to put in the register (0-63)
         * @param nbias value to put in the register (0-63)
         * @param aobias value to put in the register (0-63)
         */
        void setBiases(uint8_t vref, uint8_t nbias, uint8_t aobias);

        /**
         * Exposes the vision chip to uniform texture (such as a white piece
         * of paper placed over the optics).  Take an image using the 
         * getImage function.  Pass the uint16_t "img" array and the "size"
         * number of pixels, along with a uint8_t "mask" array to hold
         * the FPN mask and maskBase for the FPN mask base.  Function will
         * populate the mask array and maskBase variable with the FPN mask,
         * which can then be used with the applMask function. 
         * @param img image pixels
         * @param size number of pixels
         * @param mask the image mask (should be same size as img)
         * @param maskBase gets smallest value of any pixel in img
         */
        void calcMask(uint16_t *img, uint16_t size, uint8_t *mask, uint16_t *maskBase);

        /**
         * Given the "mask" and "maskBase" variables calculated in        
         * calcMask, and a current image, subtracts the
         * mask to provide a calibrated image.
         * @param img image pixels
         * @param size number of pixels
         * @param mask the image mask (should be same size as img)
         * @param maskBase previously-obtained smallest value of any pixel in img
          */
        void applyMask(uint16_t *img, uint16_t size, uint8_t *mask, uint16_t maskBase);

        /**
         * Acquires a box section of an image
         * from the analog output, and and saves to image array img.  Note 
         * that images are read out in 
         * raster manner (e.g. row wise) and stored as such in a 1D array. 
         * In this case the pointer img points to the output array. 
         *
         * @param img (output) pointer to image array, an array of signed uint16_ts
         * @param input which analog input pin to use
         * @param bounds optional ImageBounds object
         * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
         * 
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img, 0, ImageBounds(16,8,1,24,8,1)): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img, 2);
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getImage(uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
        void getImage(uint16_t *img, uint8_t input, bool digital=false);

        /**
         * Acquires a box section of a Stonyman or Hawksbill 
         * and saves to image array img.  However, each row of the image
         * is summed and returned as a single value.
         * Note that images are read out in 
         * raster manner (e.g. row wise) and stored as such in a 1D array. 
         * In this case the pointer img points to the output array. 
         *
         * @param img (output): pointer to image array, an array of signed uint16_ts
         * @param input which analog input pin to use
         * @param bounds optional ImageBounds object
         * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
         *
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img, 0, ImageBounds(16,8,1,24,8,1)): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img,2, ImageBounds(0,14,8,0,14,8)): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getRowSum(uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
        void getRowSum(uint16_t *img, uint8_t input, bool digital=false);


        /**
         * Acquires a box section of a Stonyman or Hawksbill 
         * and saves to image array img.  However, each col of the image
         * is summed and returned as a single value.
         * Note that images are read out in 
         * raster manner (e.g. row wise) and stored as such in a 1D array. 
         * In this case the pointer img points to the output array. 
         *
         * @param img (output) pointer to image array, an array of signed uint16_ts
         * @param input which analog input pin to use
         * @param bounds optional ImageBounds object
         * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
         * 
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;get(img, 0, ImageBounds(16,8,1,24,8,1)): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;get(img, 2, ImageBounds(0,14,8,0,14,8,2)): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getColSum(uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
        void getColSum(uint16_t *img, uint8_t input, bool digital=false);


        /**
         * Searches over a block section of a Stonyman chip
         * to find the brightest pixel. This function is intended to be used 
         * for things like finding the location of a pinhole in response to 
         * a bright light.
         *
         * @param input pin for chip-select signal
         * @param maxrow gets row index of brightest pixel
         * @param maxcol gets column index of brightest pixel
         * @param bounds optional ImageBounds object
         * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
         *
         * Example:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;findMax(0, &rowineer, & colwinner, (8,104,1,8,104,1)):
         * Search rows 8...104 and columns 8...104 for brightest pixel, using analog input 0
         */
        void findMax(uint8_t input, uint8_t *maxrow, uint8_t *maxcol, bool digital=false);
        void findMax(uint8_t input, uint8_t *maxrow, uint8_t *maxcol, ImageBounds & bounds, bool digital=false);

         /**
           * Processes one frame of image data from Stonyman2 in row-wise order.
           *
           * @param fg FrameGrabber object
           * @param input input pin number
           * @param bounds ImageBounds object
           */
         void processFrame(FrameGrabber & fg, uint8_t input, ImageBounds & bounds, bool digital=false);
         void processFrame(FrameGrabber & fg, uint8_t input, bool digital=false);

         /**
           * Processes one frame of image data from Stonyman2 in column-wise order.
           *
           * @param fg FrameGrabber object
           * @param input input pin number
           * @param bounds ImageBounds object
           */
         void processFrameVertical(FrameGrabber & fg, uint8_t input, ImageBounds & bounds, bool digital=false);
         void processFrameVertical(FrameGrabber & fg, uint8_t input, bool digital=false);

    private:

        //indicates whether amplifier is in use 
        bool use_amp;

        // input pins
        uint8_t _resp;
        uint8_t _incp;
        uint8_t _resv;
        uint8_t _incv;
        uint8_t _inphi;

        ImageBounds fullbounds;

        static void init_pin(uint8_t pin);

        /*********************************************************************/
        // Chip Register and Value Manipulation

        // Sets pointer on chip
        void set_pointer(uint8_t ptr);

        // Sets value of current register
        void set_value(uint16_t val);

        //	Sets the pointer system register to the desired value.  Value is
        //	not reset so the current value must be taken into account.
        void inc_value(uint16_t val);

        //	Operates the amplifier.  Sets inphi pin high, delays to allow
        //	value time to settle, and then brings it low.
        void pulse_inphi(uint8_t delay); 

        //	Resets the value of all registers to zero
        void clear_values(void);

        //	Sets the pointer to a register and sets the value of that        
        //	register
        void set_pointer_value(uint8_t ptr,uint16_t val);
};
