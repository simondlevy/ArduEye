/*
   ImageUtils.cpp Basic functions to handle images

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

// Support both Arduino and standard C++
#if defined(__arm__) || defined(__avr__)
#include <Arduino.h>
#define PRINTSHORT(n) Serial.print(n)
#define PRINTCHAR(c)  Serial.print(c)
#define PRINTSTR(s)   Serial.print(s) 
#define RANDOM(m)     random(m)
#else
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define PRINTSHORT(n) printf("%d ", n)
#define PRINTCHAR(c)  printf("%c ", c)
#define PRINTSTR(s)   printf("%s ", s)
#define RANDOM(m)     (random() % m)
#endif

#include "ImageUtils.h"

// These two variables define an array of uint8_tacters used for ASCII
// dumps of images to the Arduino serial monitor
uint8_t ASCII_DISP_CHARS[16] = "#@$%&x*=o+-~,. ";
uint8_t NUM_ASCII_DISP_CHARS = 15;


void imgCopy(uint16_t *src, uint16_t *dst,  uint16_t numpix) 
{
    uint16_t *pa = src;
    uint16_t *pb = dst;
    for (uint16_t pix=0; pix<numpix; ++pix) {
        *pb = *pa;
        pa++; 
        pb++;
    }
}

void imgCopy(uint8_t *src, uint8_t *dst,  uint16_t numpix) 
{
    uint8_t *pa = src;
    uint8_t *pb = dst;
    for (uint16_t pix=0; pix<numpix; ++pix) {
        *pb = *pa;
        pa++; 
        pb++;
    }
}

void imgDumpAscii(uint16_t *img, uint16_t numrows, uint16_t numcolumns, uint16_t mini, uint16_t maxi) 
{
    // if mini==0 then we compute minimum
    if (mini==0) {
        mini = *img;
        for (uint16_t i=0,*pix=img; i<numrows*numcolumns; ++i,++pix) {
            if (*pix<mini)
                mini=*pix;
        }
    }
    // if maxi==0 then we compute maximum
    if (maxi==0) {
        maxi = *img;
        for (uint16_t i=0,*pix=img; i<numrows*numcolumns; ++i,++pix) {
            if (*pix>maxi)
                maxi=*pix;
        }
    }
    // Compute delta value for display
    uint16_t delta = maxi-mini;
    delta = delta / NUM_ASCII_DISP_CHARS;
    if (delta<1)
        delta=1;

    // Loop through and dump image
    uint16_t *pix=img;
    for (uint16_t m=0; m<numrows; ++m) {
        for (uint16_t n=0; n<numcolumns; ++n) {
            // rescale pixel value to 0...9
            int16_t i = *pix - mini;
            i = i / delta;
            if (i<0)
                i=0;
            if (i>NUM_ASCII_DISP_CHARS)
                i=NUM_ASCII_DISP_CHARS;
            i = NUM_ASCII_DISP_CHARS+1-i;
            // print
            PRINTCHAR(ASCII_DISP_CHARS[i]);
            // next pixel
            pix++;
        }
        PRINTSTR(" \n");
    }
}

void imgDumpMatlab(uint16_t *img,  uint8_t numrows,  uint8_t numcols) 
{
    uint16_t *pimg = img;

    PRINTSTR("Dat = [\n");

    for (uint8_t row=0; row<numrows; ++row) {
        for (uint8_t col=0; col<numcols; ++col) {
            PRINTSHORT(*pimg);
            PRINTSTR(" ");
            pimg++;
        }
        PRINTSTR(" \n");
    }
    PRINTSTR("];\n");
}


uint16_t imgMin(uint16_t *A,  uint16_t numpix) 
{

    uint16_t *pa = A;
    uint16_t minval = *A;

    for (uint16_t pixnum=0; pixnum<numpix; ++pixnum) {
        if (*pa < minval)
            minval = *pa;
        ++pa;
    }
    return minval;
}

uint16_t imgMax(uint16_t *A,  uint16_t numpix) 
{
    uint16_t *pa = A;
    uint16_t maxval;

    maxval = *A;
    for (uint16_t pixnum=0; pixnum<numpix; ++pixnum) {
        if (*pa > maxval)
            maxval = *pa;
        ++pa;
    }
    return maxval;
}

void imgDiff(uint16_t *A, uint16_t *B, uint16_t *D,  uint16_t numpix) 
{
    uint16_t *pa = A;
    uint16_t *pb = B;
    uint16_t *pd = D;

    for (uint16_t pixnum=0; pixnum<numpix; pixnum++) {
        *pd = *pa - *pb;
        pa++;
        pb++;
        pd++;
    }
}

void imgFilter(uint16_t *I, uint16_t *L, uint16_t *H, uint16_t numpix, uint8_t shiftalpha) 
{
    uint16_t * pi = I; 
    uint16_t * pl = L; 
    uint16_t * ph = H;

    for (uint16_t pix=0; pix<numpix; ++pix) {
        // update lowpass
        uint16_t indiff = (*pi << 4) - *pl ;
        indiff = indiff >> shiftalpha;
        *pl += indiff;
        // compute highpass
        *ph = *pi - (*pl >> 4);
        // increment pointers
        pi++; pl++; ph++;
    }
}

void imgAddFpn(uint16_t *A,  uint8_t *F,  uint16_t numpix,  uint8_t s) 
{
    uint16_t * pa = A;
    uint8_t  * pf = F;

    for (uint16_t pixnum=0; pixnum<numpix; ++pixnum) {
        *pa += ((uint16_t)(*pf))*s;
        pa++;
        pf++;
    }
}

void imgMakeFpn( uint8_t *F,  uint16_t numpix,  uint8_t modval) 
{
    uint8_t *pf = F;

    for (uint16_t pixnum=0; pixnum<numpix; ++pixnum) {
        *pf = (uint8_t)(RANDOM(modval) );
        pf++;
    }
}


void SubwinShort2D(
        uint16_t *I, 
        uint16_t *S, 
        uint8_t Icols, 
        uint8_t startrow, 
        uint8_t numrows, 
        uint8_t startcol, 
        uint8_t numcols) 
{
    uint16_t * ps = S;

    for (uint8_t r=0; r<numrows; ++r) {
        uint16_t * pi = I + Icols*(startrow+r) + startcol;
        for (uint8_t c=0; c<numcols; ++c) {
            *ps = *pi;
            ps++;
            pi++;
        }
    }
}

void SubwinShort2Dto1DVertical(
        uint16_t *I, 
        uint16_t *S, 
        uint8_t Icols, 
        uint8_t subrow, 
        uint8_t subcol, 
        uint8_t Snumpix, 
        uint8_t Spixlength) 
{

    // first clear image S
    for (uint8_t r=0; r<Snumpix; ++r) {
        S[r]=0;
    }

    for (uint8_t r=0; r<Spixlength; ++r) {
        uint16_t * pi = I + Icols*(subrow+r) + subcol;
        for (uint8_t c=0; c<Snumpix; ++c) {
            S[c] += *pi;
            pi++;
        }
    }
}

void SubwinShort2Dto1DHorizontal(
        uint16_t *I, 
        uint16_t *S, 
        uint8_t Icols, 
        uint8_t subrow, 
        uint8_t subcol, 
        uint8_t Snumpix, 
        uint8_t Spixlength) 
{

    // first clear image S
    for (uint8_t r=0; r<Snumpix; ++r) {
        S[r]=0;
    }

    for (uint8_t r=0; r<Snumpix; ++r) {
        uint16_t * pi = I + Icols*(subrow+r) + subcol;
        for (uint8_t c=0; c<Spixlength; ++c) {
            S[r] += *pi;
            pi++;
        }
    }
}


