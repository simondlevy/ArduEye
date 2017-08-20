/*
Stonyman.h Library for the Stonyman Centeye Vision Chip

Basic functions to operate Stonyman with Arduino boards

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

#include <Stonyman.h>
#include <SPI.h>	//SPI required for external ADC

/*********************************************************************/
//SMH System Registers

#define SMH_SYS_COLSEL 0	//select column
#define SMH_SYS_ROWSEL 1	//select row
#define SMH_SYS_VSW    2	//vertical switching
#define SMH_SYS_HSW    3	//horizontal switching
#define SMH_SYS_VREF   4	//voltage reference
#define SMH_SYS_CONFIG 5	//configuration register
#define SMH_SYS_NBIAS  6	//nbias
#define SMH_SYS_AOBIAS 7	//analog out bias

/*********************************************************************/

// Supply voltage types
// Notation: AVB is A.B volts. e.g. 5V0 is 5V, 3V3 is 3.3V, etc.
#define SMH1_VDD_5V0  1

#define SMH_VREF_5V0 30		//vref for 5 volts
#define SMH_NBIAS_5V0 55	//nbias for 5 volts
#define SMH_AOBIAS_5V0 55	//aobias for 5 volts

Stonyman::Stonyman(uint8_t resp, uint8_t incp, uint8_t resv, uint8_t incv, uint8_t inphi)
{
    _resp = resp;
    _incp = incp;
    _resv = resv;
    _incv = incv;
    _inphi = inphi;
}

void Stonyman::init_pin(uint8_t pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Stonyman::begin(uint8_t vref, uint8_t nbias, uint8_t aobias, bool selamp)
{
    //set all digital pins to output
    init_pin(_resp);
    init_pin(_incp);
    init_pin(_resv);
    init_pin(_incv);

    //clear all chip register values
    clear_values();

    //set up biases
    setBiases(vref,nbias,aobias);

    //turn chip on with config value
    set_pointer_value(SMH_SYS_CONFIG,16);

    //if amp is used, set use_amp variable
    use_amp = selamp;
}

static void pulse(int pin)
{
    digitalWrite(pin, HIGH);
    delayMicroseconds(1);
    digitalWrite(pin, LOW);
}

void Stonyman::set_pointer(uint8_t ptr)
{
    // clear pointer
    pulse(_resp);

    // increment pointer to desired value
    for (uint16_t i=0; i!=ptr; ++i) 
        pulse(_incp);
}

void Stonyman::set_value(uint16_t val) 
{
    // clear pointer
    pulse(_resv);

    // increment pointer
    for (uint16_t i=0; i!=val; ++i) 
        pulse(_incv);
}

void Stonyman::inc_value(uint16_t val) 
{
    for (uint16_t i=0; i<val; ++i) //increment pointer
        pulse(_incv);
}

void Stonyman::pulse_inphi(uint8_t delay) 
{
    (void)delay;
    pulse(_inphi);
}

void Stonyman::set_pointer_value(uint8_t ptr,uint16_t val)
{
    set_pointer(ptr);	//set pointer to register
    set_value(val);	//set value of that register
}

void Stonyman::clear_values(void)
{
    for (uint8_t i=0; i!=8; ++i)
        set_pointer_value(i,0);	//set each register to zero
}

void  Stonyman::setVref(uint8_t vref)
{
    set_pointer_value(SMH_SYS_VREF,vref);
}

void  Stonyman::setNbias(uint8_t nbias)
{
    set_pointer_value(SMH_SYS_NBIAS,nbias);
}

void  Stonyman::setAobias(uint8_t aobias)
{
    set_pointer_value(SMH_SYS_AOBIAS,aobias);
}

void Stonyman::setBiasesVdd(uint8_t vddType)
{

    // determine biases. Only one option for now.
    switch (vddType) 
    {
        case SMH1_VDD_5V0:	//chip receives 5V
        default:
            set_pointer_value(SMH_SYS_NBIAS,SMH_NBIAS_5V0);
            set_pointer_value(SMH_SYS_AOBIAS,SMH_AOBIAS_5V0);
            set_pointer_value(SMH_SYS_VREF,SMH_VREF_5V0);
            break;
    }
}

void Stonyman::setBiases(uint8_t vref, uint8_t nbias, uint8_t aobias)
{
    set_pointer_value(SMH_SYS_NBIAS,nbias);
    set_pointer_value(SMH_SYS_AOBIAS,aobias);
    set_pointer_value(SMH_SYS_VREF,vref);
}

void Stonyman::setConfig(uint8_t gain, uint8_t selamp, uint8_t cvdda) 
{
    uint16_t config=gain+(selamp*8)+(cvdda*16);	//form register value

    if(selamp==1)	//if selamp is 1, set use_amp variable to 1
        use_amp = true;
    else 
        use_amp = false;

    // Note that config will have the following form binary form:
    // 000csggg where c=cvdda, s=selamp, ggg=gain (3 bits)
    // Note that there is no overflow detection in the input values.
    set_pointer_value(SMH_SYS_CONFIG,config);
}

void Stonyman::setAmpGain(uint8_t gain)
{
    uint16_t config;

    if((gain>0)&&(gain<8))	//if gain is a proper value, connect amp
    {
        config=gain+8+16;	//gain+(selamp=1)+(cvdda=1)
        use_amp = true;	//using amplifier
    }
    else	//if gain is zero or outside range, bypass amp
    {
        config=16;	//(cvdda=1)
        use_amp = false;	//bypassing amplifier
    }

    set_pointer_value(SMH_SYS_CONFIG,config);	//set config register
}

void Stonyman::setBinning(uint8_t hbin,uint8_t vbin)
{
    uint16_t hsw,vsw;

    switch (hbin) //horizontal binning
    {
        case 2:		//downsample by 2
            hsw = 0xAA;
            break;
        case 4:		//downsample by 4
            hsw = 0xEE;
            break;
        case 8:		//downsample by 8
            hsw = 0xFE;
            break;
        default:	//no binning
            hsw = 0x00;
    }

    switch (vbin) 	//vertical binning
    {
        case 2:		//downsample by 2
            vsw = 0xAA;
            break;
        case 4:		//downsample by 4
            vsw = 0xEE;
            break;
        case 8:		//downsample by 8
            vsw = 0xFE;
            break;
        default:	//no binning
            vsw = 0x00;
    }

    //set switching registers
    set_pointer_value(SMH_SYS_HSW,hsw);
    set_pointer_value(SMH_SYS_VSW,vsw);
}

void Stonyman::calcMask(uint16_t *img, uint16_t size, uint8_t *mask,uint16_t *maskBase)
{
    *maskBase = 10000; // e.g. "high"

    for (int i=0; i<size; ++i)
        if (img[i]<(*maskBase))	//find the min value for maskBase
            *maskBase = img[i];

    // generate calibration mask
    for (int i=0; i<size; ++i)
        mask[i] = img[i] - *maskBase;	//subtract min value for mask
}

void Stonyman::applyMask(uint16_t *img, uint16_t size, uint8_t *mask, uint16_t maskBase)
{
    // Subtract calibration mask
    for (int i=0; i<size;++i) 
    {
        img[i] -= maskBase+mask[i];  //subtract FPN mask
        img[i]=-img[i];          //negate image so it displays properly
    }
}

void Stonyman::getImageAnalog(uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    get_image(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, false);
}

void Stonyman::getImageDigital(uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    get_image(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, true);
}

void Stonyman::get_image(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input, 
        bool    use_digital) 
{
    // XXX need to support digital (SPI) as well    
    (void)use_digital;

    uint16_t *pimg = img; // pointer to output image array
    uint16_t val;
    uint8_t row,col;

    // Go to first row
    set_pointer_value(SMH_SYS_ROWSEL,rowstart);

    // Loop through all rows
    for (row=0; row<numrows; ++row) {

        // Go to first column
        set_pointer_value(SMH_SYS_COLSEL,colstart);

        // Loop through all columns
        for (col=0; col<numcols; ++col) {

            // settling delay
            delayMicroseconds(1);

            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            // get data value
            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel XXX need to support digital (SPI) as well

            *pimg = val; // store pixel
            pimg++; // advance pointer
            inc_value(colskip); // go to next column
        }
        set_pointer(SMH_SYS_ROWSEL);
        inc_value(rowskip); // go to next row
    }
}

void Stonyman::getImageRowSumAnalog(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input)
{
    get_image_row_sum(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, false);
}

void Stonyman::getImageRowSumDigital(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input)
{
    get_image_row_sum(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, true);
}

void Stonyman::get_image_row_sum(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input,
        bool use_digital) 
{
    (void)use_digital;

    uint16_t *pimg = img; // pointer to output image array
    uint16_t val,total=0;
    uint8_t row,col;

    // Go to first row
    set_pointer_value(SMH_SYS_ROWSEL,rowstart);

    // Loop through all rows
    for (row=0; row<numrows; ++row) {

        // Go to first column
        set_pointer_value(SMH_SYS_COLSEL,colstart);

        total=0;

        // Loop through all columns
        for (col=0; col<numcols; ++col) {

            // settling delay
            delayMicroseconds(1);

            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            // get data value
            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel

            total+=val;	//sum values along row
            inc_value(colskip); // go to next column
        }

        *pimg = total>>4; // store pixel divide to avoid overflow
        pimg++; // advance pointer

        set_pointer(SMH_SYS_ROWSEL);
        inc_value(rowskip); // go to next row
    }
}

void Stonyman::getImageColSumAnalog(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    get_image_col_sum(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, false);
}

void Stonyman::getImageColSumDigital(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    get_image_col_sum(img, rowstart, numrows, rowskip, colstart, numcols, colskip, input, true);
}
    
void Stonyman::get_image_col_sum(
        uint16_t *img, 
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input,
        bool use_digital) 
{
    (void)use_digital;

    uint16_t *pimg = img; // pointer to output image array
    uint16_t val,total=0;
    uint8_t row,col;

    // Go to first col
    set_pointer_value(SMH_SYS_COLSEL,colstart);

    // Loop through all cols
    for (col=0; col<numcols; ++col) {

        // Go to first row
        set_pointer_value(SMH_SYS_ROWSEL,rowstart);

        total=0;

        // Loop through all rows
        for (row=0; row<numrows; ++row) {

            // settling delay
            delayMicroseconds(1);

            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            // get data value
            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel

            total+=val;	//sum value along column
            inc_value(rowskip); // go to next row
        }

        *pimg = total>>4; // store pixel
        pimg++; // advance pointer

        set_pointer(SMH_SYS_COLSEL);
        inc_value(colskip); // go to next col
    }
}


void Stonyman::findMaxAnalog(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols,
        uint8_t colskip, 
        uint8_t input,
        uint8_t *maxrow, 
        uint8_t *maxcol)
{
    find_max(rowstart, numrows, rowskip, colstart, numcols, colskip, input, maxrow, maxcol, false);
}

void Stonyman::findMaxDigital(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols,
        uint8_t colskip, 
        uint8_t input,
        uint8_t *maxrow, 
        uint8_t *maxcol)
{
    find_max(rowstart, numrows, rowskip, colstart, numcols, colskip, input, maxrow, maxcol, true);
}

void Stonyman::find_max(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols,
        uint8_t colskip, 
        uint8_t input,
        uint8_t *maxrow, 
        uint8_t *maxcol,
        bool use_digital)
{
    (void)use_digital;

    uint16_t maxval=5000,minval=0,val;
    uint8_t row,col,bestrow=0,bestcol=0;

    // Go to first row
    set_pointer_value(SMH_SYS_ROWSEL,rowstart);

    // Loop through all rows
    for (row=0; row<numrows; ++row) {

        // Go to first column
        set_pointer_value(SMH_SYS_COLSEL,colstart);

        // Loop through all columns
        for (col=0; col<numcols; ++col) {

            // settling delay
            delayMicroseconds(1);

            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            // get data value
            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel

            if(use_amp)	//amplifier is inverted
            {
                if (val>minval) 	//find max val (bright)
                {
                    bestrow=row;
                    bestcol=col;
                    minval=val;
                }
            }
            else		//unamplified
            {
                if (val<maxval) 	//find min val (bright)
                {
                    bestrow=row;
                    bestcol=col;
                    maxval=val;
                }
            }

            inc_value(colskip); // go to next column
        }
        set_pointer(SMH_SYS_ROWSEL);
        inc_value(rowskip); // go to next row
    }

    *maxrow = bestrow;
    *maxcol = bestcol;
}

void Stonyman::chipToMatlabAnalog(uint8_t input) 
{
    chip_to_matlab(input, false); 
}

void Stonyman::chipToMatlabDigital(uint8_t input) 
{
    chip_to_matlab(input, true); 
}

void Stonyman::chip_to_matlab(uint8_t input, bool use_digital) 
{
    (void)use_digital;

    uint8_t row,col;
    uint16_t val;

    Serial.println("Img = [");
    set_pointer_value(SMH_SYS_ROWSEL,0); // set row = 0
    for (row=0; row<112; ++row) {
        set_pointer_value(SMH_SYS_COLSEL,0); // set column = 0
        for (col=0; col<112; ++col) {
            // settling delay
            delayMicroseconds(1);
            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            // get data value
            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel

            // increment column
            inc_value(1);
            Serial.print(val);
            Serial.print(" ");
        }
        set_pointer(SMH_SYS_ROWSEL); // point to row
        inc_value(1); // increment row
        Serial.println(" ");
    }
    Serial.println("];");
}

void Stonyman::sectionToMatlabAnalog(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    section_to_matlab(rowstart, numrows, rowskip, colstart, numcols, colskip, input, false);
}

void Stonyman::sectionToMatlabDigital(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input) 
{
    section_to_matlab(rowstart, numrows, rowskip, colstart, numcols, colskip, input, true);
}

void Stonyman::section_to_matlab(
        uint8_t rowstart, 
        uint8_t numrows, 
        uint8_t rowskip, 
        uint8_t colstart, 
        uint8_t numcols, 
        uint8_t colskip, 
        uint8_t input,
        bool use_digital) 
{
    (void)use_digital;

    uint16_t val;
    uint8_t row,col;

    Serial.println("Img = [");
    set_pointer_value(SMH_SYS_ROWSEL,rowstart);

    for (row=0; row<numrows; row++) {

        set_pointer_value(SMH_SYS_COLSEL,colstart);

        for (col=0; col<numcols; col++) {
            // settling delay
            delayMicroseconds(1);

            // pulse amplifier if needed
            if (use_amp) 
                pulse_inphi(2);

            delayMicroseconds(1);

            val = analogRead(input); // acquire pixel

            inc_value(colskip);
            Serial.print(val);
            Serial.print(" ");
        }
        set_pointer(SMH_SYS_ROWSEL);
        inc_value(rowskip); // go to next row
        Serial.println(" ");
    }
    Serial.println("];");

}
