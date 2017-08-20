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

#define MAX_ROWS    10    
#define MAX_COLS    10
#define SKIP_PIXELS 8
#define START_ROW   16
#define START_COL   16
#define START_PIXEL 8  

#else

//If not Uno, assume a more modern board like Mega 2560,
//which can handle 48x48 array and FPN mask
//so we set SKIP_PIXELS=2 to downsample by 2
//and START_PIXELS at row/col 8.  This gives us
//a 48x48 array with superpixels of 2x2.  With the
//start row and col at 8, the 96x96 raw grid is 
//centered on the Stonyman 112x112 raw array.

#define MAX_ROWS    16
#define MAX_COLS    16
#define SKIP_PIXELS 4
#define START_ROW   24
#define START_COL   24  
#define START_PIXEL 18

#endif

#define MAX_PIXELS (MAX_ROWS*MAX_COLS)

/**
 *	A class for interacting with the Stonyman2 vision chip from Centeye, Inc.
 *  For documentation on this chip see 
 *  https://github.com/simondlevy/Centeye/blob/master/extras/docs/Stonyman_Hawksbill_ChipInstructions_Rev10_20130312.pdf
 */
class Stonyman 
{
    public:

        /**
         * Creates a Stonyman object using four digital input signals to the chip.
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
         * @param rowstart first row to acquire
         * @param numrows number of rows to acquire
         * @param rowskip skipping between rows (useful if binning is used)
         * @param colstart first column to acquire
         * @param numcols number of columns to acquire
         * @param colskip skipping between columns
         * @param input which analog input pin to use
         * 
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImageAnalog(img,16,8,1,24,8,1,0): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImageAnalog(img,0,14,8,0,14,8,2): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getImageAnalog(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
        void getImageDigital(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
         * Acquires a box section of a Stonyman or Hawksbill 
         * and saves to image array img.  However, each row of the image
         * is summed and returned as a single value.
         * Note that images are read out in 
         * raster manner (e.g. row wise) and stored as such in a 1D array. 
         * In this case the pointer img points to the output array. 
         *
         * @param img (output): pointer to image array, an array of signed uint16_ts
         * @param rowstart: first row to acquire
         * @param numrows: number of rows to acquire
         * @param rowskip: skipping between rows (useful if binning is used)
         * @param colstart: first column to acquire
         * @param numcols: number of columns to acquire
         * @param colskip: skipping between columns
         * @param input which analog input pin to use
         *
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img,16,8,1,24,8,1,0): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img,0,14,8,0,14,8,2): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getImageRowSumAnalog(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
        void getImageRowSumDigital(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
         * Acquires a box section of a Stonyman or Hawksbill 
         * and saves to image array img.  However, each col of the image
         * is summed and returned as a single value.
         * Note that images are read out in 
         * raster manner (e.g. row wise) and stored as such in a 1D array. 
         * In this case the pointer img points to the output array. 
         *
         * @param img (output) pointer to image array, an array of signed uint16_ts
         * @param rowstart first row to acquire
         * @param numrows number of rows to acquire
         * @param rowskip skipping between rows (useful if binning is used)
         * @param colstart first column to acquire
         * @param numcols number of columns to acquire
         * @param colskip skipping between columns
         * @param input which analog input pin to use
         * 
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img,16,8,1,24,8,1,0): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from chip using onboard ADC at input 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;getImage(img,0,14,8,0,14,8,2): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 2.
         */
        void getImageColSumAnalog(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
        void getImageColSumDigital(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);

        /**
         * Searches over a block section of a Stonyman or Hawksbill chip
         * to find the brightest pixel. This function is intended to be used 
         * for things like finding the location of a pinhole in response to 
         * a bright light.
         *
         * @param rowstart first row to search
         * @param numrows number of rows to search
         * @param rowskip skipping between rows (useful if binning is used)
         * @param colstart first column to search
         * @param numcols number of columns to search
         * @param colskip skipping between columns
         * @param input which analog input to use
         * @param maxrow (output) pointer to variable to write row of brightest pixel
         * @param maxcol (output) pointer to variable to write column of brightest pixel
         * @param input pin for chip-select signal
         *
         * Example:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;findMaxAnalog(8,104,1,8,104,1,0,&rowwinner, &colwinner): 
         * Search rows 8...104 and columns 8...104 for brightest pixel, using analog input 0
         */
        void findMaxAnalog(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input,
                uint8_t *maxrow, 
                uint8_t *maxcol);

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
         void findMaxDigital(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input,
                uint8_t *maxrow, 
                uint8_t *maxcol);

        /**
         * Dumps the entire contents of a Stonyman or 
         * Hawksbill chip to the Serial monitor in a form that may be copied 
         * into Matlab. The image is written be stored in matrix Img. 
         *
         * @param input which analog input pin to use
         */
        void chipToMatlabAnalog(uint8_t input);

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
        void chipToMatlabDigital(uint8_t input);

        /**
         * Dumps a box section of a Stonyman or Hawksbill 
         * to the Serial monitor in a form that may be copied into Matlab. 
         * The image is written to be stored in matrix Img. 
         *
         * @param rowstart first row to acquire
         * @param numrows number of rows to acquire
         * @param rowskip skipping between rows (useful if binning is used)
         * @param colstart first column to acquire
         * @param numcols number of columns to acquire
         * @param colskip skipping between columns
         * @param input which analog input pin to use
         *
         * Examples:
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;sectionToMatlab(16,8,1,24,8,1,0): 
         * Grab an 8x8 window of pixels at raw resolution starting at row 
         * 16, column 24, from onboard ADC at chip 0
         *
         * &nbsp;&nbsp;&nbsp;&nbsp;sectionToMatlab(0,14,8,0,14,8,2): 
         * Grab entire Stonyman chip when using 8x8 binning. Grab from input 
         * 2.
         */
        void sectionToMatlabAnalog(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);   

        /**
          * Digital (SPI) version of above.
          * @param input pin for chip-select signal
          */ 
         void sectionToMatlabDigital(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input);   
    private:

        //indicates whether amplifier is in use 
        bool use_amp;

        // input pins
        uint8_t _resp;
        uint8_t _incp;
        uint8_t _resv;
        uint8_t _incv;
        uint8_t _inphi;

        void get_image(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input,
                bool use_digital);

        void get_image_row_sum(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input, 
                bool use_digital);

        void get_image_col_sum(
                uint16_t *img, 
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input, 
                bool use_digital);

        void find_max(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input,
                uint8_t *maxrow, 
                uint8_t *maxcol,
                bool use_digital);

        void chip_to_matlab(uint8_t input, bool use_digital);

        void section_to_matlab(
                uint8_t rowstart, 
                uint8_t numrows, 
                uint8_t rowskip, 
                uint8_t colstart, 
                uint8_t numcols, 
                uint8_t colskip, 
                uint8_t input, 
                bool use_digital);

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
