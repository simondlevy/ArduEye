/*
StonymanUtils.cpp Utilities library for the Stonyman2 Centeye Vision Chip
See Stonyman.h for documentation Copyright (c) 2012 Centeye, Inc. 
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

#include <StonymanUtils.h>

//helper class for grabbing images and storing them in an array
class ArrayFrameGrabber : public FrameGrabber {

    friend class Stonyman;

    private:

    uint16_t * _img;
    uint16_t * _pimg;

    protected:

    virtual void preProcess(void) override  
    {
        _pimg = _img;
    }

    virtual void handlePixel(uint8_t row, uint8_t col, uint16_t pixel, bool use_amp) override 
    {
        (void)row;
        (void)col;
        (void)use_amp;

        *_pimg++ = pixel;
    }

    public:

    ArrayFrameGrabber(uint16_t * img) 
    {
        _img = img;
    }

};

void stonymanGetImage(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital) 
{
    ArrayFrameGrabber fg(img);
    stonyman.processFrame(fg, input, bounds, digital);
}

void stonymanGetImage(Stonyman & stonyman, uint16_t *img, uint8_t input, bool digital) 
{
    stonymanGetImage(stonyman, img, input, stonyman.FULLBOUNDS, digital);
}

// helper class for computing row sums
class SumFrameGrabber : public FrameGrabber {

    friend class Stonyman;

    private:

    uint16_t * pimg;
    uint16_t total;

    protected:

    virtual void handleVectorStart(void) 
    { 
        total = 0;
    }

    virtual void handlePixel(uint8_t row, uint8_t col, uint16_t pixel, bool use_amp) 
    { 
        (void)row; 
        (void)col; 
        (void)use_amp;

        total += pixel;
    }

    virtual void handleVectorEnd(void) 
    {
        *pimg++ = total>>4; // store pixel divide to avoid overflow, then advance pointer
    }

    public:

    SumFrameGrabber(uint16_t * img) 
    {
        pimg = img;
    }
};


void stonymanGetRowSum(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital)
{
    SumFrameGrabber fg(img);
    stonyman.processFrame(fg, input, bounds, digital);
}

void stonymanGetColSum(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital)
{
    SumFrameGrabber fg(img);
    stonyman.processFrameVertical(fg, input, bounds, digital);
}

//helper class for finding maximum pixel values in image
class MaxFrameGrabber : public FrameGrabber {

    friend class Stonyman;

    protected:

    uint16_t minval;
    uint16_t maxval;

    virtual void preProcess(void) override
    {
        maxval=5000;
        minval=0;
        bestrow=0;
        bestcol=0;
    }

    virtual void handlePixel(uint8_t row, uint8_t col, uint16_t pixel, bool use_amp) override 
    {
        if(use_amp)	{ //amplifier is inverted
            if (pixel>minval) { 	//find max val (bright)
                bestrow=row;
                bestcol=col;
                minval=pixel;
            }
        }
        else {		//unamplified
            if (pixel<maxval) {	//find min val (bright)
                bestrow=row;
                bestcol=col;
                maxval=pixel;
            }
        }
    }

    public:

    uint16_t bestrow;
    uint16_t bestcol;
};


void stonymanFindMax(Stonyman & stonyman, uint8_t input, uint8_t *maxrow, uint8_t *maxcol, ImageBounds & bounds, bool digital)
{
    MaxFrameGrabber fg;
    stonyman.processFrame(fg, input, bounds, digital);
    *maxrow = fg.bestrow;
    *maxcol = fg.bestcol;
}
