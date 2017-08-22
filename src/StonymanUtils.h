/*
   StonymanUtils.h Utilities library for the Stonyman Centeye Vision Chip

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
#include <Stonyman.h>

/**
 * Acquires a box section of an image and and saves to image array img.  Note 
 * that images are read out in 
 * raster manner (e.g. row wise) and stored as such in a 1D array. 
 * In this case the pointer img points to the output array. 
 *
 * @param stonyman a Stonyman object whose begin() method has been called
 * @param img (output) pointer to image array, an array of signed uint16_ts
 * @param input which analog input pin to use
 * @param bounds optional ImageBounds object
 * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
 */
void stonymanGetImage(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
void stonymanGetImage(Stonyman & stonyman, uint16_t *img, uint8_t input, bool digital=false);

/**
 * Acquires a box section of a Stonyman or Hawksbill 
 * and saves to image array img.  However, each row of the image
 * is summed and returned as a single value.
 * Note that images are read out in 
 * raster manner (e.g. row wise) and stored as such in a 1D array. 
 * In this case the pointer img points to the output array. 
 *
 * @param stonyman a Stonyman object whose begin() method has been called
 * @param img (output): pointer to image array, an array of signed uint16_ts
 * @param input which analog input pin to use
 * @param bounds optional ImageBounds object
 * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
 */
void stonymanGetRowSum(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
void stonymanGetRowSum(Stonyman & stonyman, uint16_t *img, uint8_t input, bool digital=false);

/**
 * Acquires a box section of a Stonyman or Hawksbill 
 * and saves to image array img.  However, each col of the image
 * is summed and returned as a single value.
 * Note that images are read out in 
 * raster manner (e.g. row wise) and stored as such in a 1D array. 
 * In this case the pointer img points to the output array. 
 *
 * @param stonyman a Stonyman object whose begin() method has been called
 * @param img (output) pointer to image array, an array of signed uint16_ts
 * @param input which analog input pin to use
 * @param bounds optional ImageBounds object
 * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
 */
void stonymanGetColSum(Stonyman & stonyman, uint16_t *img, uint8_t input, ImageBounds & bounds, bool digital=false);
void stonymanGetColSum(Stonyman & stonyman, uint16_t *img, uint8_t input, bool digital=false);


/**
 * Searches over a block section of a Stonyman chip
 * to stonymanFind the brightest pixel. This function is intended to be used 
 * for things like stonymanFinding the location of a pinhole in response to 
 * a bright light.
 *
 * @param stonyman a Stonyman object whose begin() method has been called
 * @param input pin for chip-select signal
 * @param maxrow gets row index of brightest pixel
 * @param maxcol gets column index of brightest pixel
 * @param bounds optional ImageBounds object
 * @param optional bool digital= flag for using SPI (default=false, use Arduino ADC)
 */
void stonymanFindMax(Stonyman & stonyman, uint8_t input, uint8_t *maxrow, uint8_t *maxcol, bool digital=false);
void stonymanFindMax(Stonyman & stonyman, uint8_t input, uint8_t *maxrow, uint8_t *maxcol, ImageBounds & bounds, bool digital=false);

