/*
===============================================================================
ImagesUtils.h Basic functions to handle images

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

/**
 * @file ImageUtils.h
 * 
 * Generally images are stored as either signed uint16_ts (16 bits), signed uint8_ts (8 bits), or  uint8_ts (8 bits). 
 * Signed numbers are generally preferred to allow for negative values due to frame differences. 8 bits is adequate
 * for some applications in particular when bit depth is not needed and/or speed and memory is a consideration. 
 * 
 *  Although images are generally accepted as two dimensional arrays, for this library they are stored in a
 *  one dimensional array row-wise. So to store a 4x6 image, we would first declare a variable:

 *  &nbsp;&nbsp;&nbsp;&nbsp;<tt>uint16_t A[24]; // 24 = 4 rows * 6 columns</tt>

 * and then store the first row in the first six elements of A, the second row in the second six elements of A,
 * and so on. So A[0] = row 0 column 0, A[1] = row 0 column 1, A[5] = row 0 column 5, A[6] = row 1 column 0,
 * A[11] = row 1 column 5, etc. This row-wise format speeds up acquisition of pixels, in particular if pointer
 * are used for different arrays, or even for different regions within the same region. Note that it is up to 
 * the programmer to ensure that arrays are properly allocated and that indices do not go out of bounds.

 * The image functions are then written so that you will need to provide either the number of rows and columns,
 * or just the total number of pixels (number of rows * number of columns). This is because some functions
 * (like direct frame adding or finding the maximum or minimum values in an image) don't require knowledge
 * of the row or column of each pixel, whereas other functions (like printing images or more advanced
 * operations) do require the dimensions of the image.
 */

/**
  * Copy one image to another.
  * @param src source image
  * @param dst destination image
  * @param numpix number of pixels
  */
void imgCopy(uint16_t * src, uint16_t * dst, uint16_t numpix);

/**
  * Eight-bit version of above.
  */
void imgCopy(uint8_t * src, uint8_t * dst, uint16_t numpix);

/**
  * Dumps image to serial output as ASCII characters. 
  * Darker characters correspond to brighter pixels.
  * @param img image pixels
  * @param numrows number of rows
  * @param numcols number of columns
  * @param mini value less than this are considered black
  * @param maxi value greater than this are considered white
  */
void imgDumpAscii(uint16_t * img, uint16_t numrows, uint16_t numcolumns, uint16_t mini, uint16_t maxi);

/**
  * Dumps image to serial output in Matlab format
  * @param img image pixels
  * @param numrows number of rows
  * @param numcols number of columns
  *
  */
void imgDumpMatlab(uint16_t * img, uint8_t numrows, uint8_t numcols);

/**
  * Gets smallest pixel value in image.
  * @param img image pixels
  * @param numpix number of pixels
  * @return minimum value
  */
uint16_t imgMin(uint16_t * img, uint16_t numpix);

/**
  * Gets largest pixel value in image.
  * @param img image pixels
  * @param numpix number of pixels
  * @return maximum value
  */
uint16_t imgMax(uint16_t * img, uint16_t numpix);

/**
  * Subtracts to images to produce a third: D = A - B
  * @param a pixels of image A (input)
  * @param b pixels of image B (input)
  * @param d pixels of image D (output)
  * @param numpix number of pixels
  *
  * NB: Untested
  */
void imgDiff(uint16_t * a, uint16_t * b, uint16_t * d, uint16_t numpix);

/**
  * Runs low- and high-pass filters on an image.
  * @param img image pixels
  * @param lo gets pixels of low-pass-filtered image
  * @param hi gets pixels of high-pass-filtered image
  * @param numpix number of pixels
  * @param shiftalpha shifting amount used to implement time constant
  *
  * NB: Untested
  */
void imgFilter(uint16_t * img, uint16_t * lo, uint16_t * hi, uint16_t numpix, uint8_t shiftalpha);

/**
  * Adds a fixed-pattern-noise image to an image with optional scaling factor: 
  * img = img * f + s 
  * @param img image pixels
  * @param f fpn
  * @param numpix number of pixels
  * @s optional scaling factor
  * NB: Untested
  */
void imgAddFpn(uint16_t * img, uint8_t * f, uint16_t numpix, uint8_t s=1);

/**
 * Generates a random fixed pattern noise (Fpn) to 
 * be used to add still texture to an image. This can be added to high
 * passed images so that when there is no motion there is still some 
 * texture so that motion sensing algorithms measure zero motion. Yes, this
 * is a bit of a kludge... The fixed pattern noise is stored as
 * uint8_ts to make room. 
 *
 * @param f fpn image to be output
 * @param numpix number of pixels
 * @param modvaldetermines the strength of the FPN
 *
 * NB: Untested
 */
void imgMakeFpn(uint8_t * f, uint16_t numpix, uint8_t modval);

/**
 * Extracts a 2D subwindow from a 2D image of short unsigned integers. 
 *
 * @param src input image
 * @param dst output image
 * @param srccols number of columns of src
 * @param subrow,subcol row and column of upper left pixel of subwindow
 * @param startrow starting row of window
 * @param numrows number of rows of subwindow
 * @param startcol starting column of window
 * @param numcols number of columns of subwindow
 * NB: Untested
 */
void Subwin2D(
        uint16_t * src, 
        uint16_t * dst, 
        uint8_t srccols, 
        uint8_t startrow, 
        uint8_t numrows, 
        uint8_t startcol, 
        uint8_t numcols);

/**
 * Extracts a subwindow from a 2D image and then sums columns to form a 1D image. 
 * This is a simple way to generate 1D images from a 2D image, using a mathematical equivalent
 * of on-chip binning. Note that a SUM is used for the resulting pixels, not an AVERAGE.
 *
 * @param src input image
 * @param dst output image
 * @param srccold number of columns of src
 * @param subrow row of upper-left pixel of subwindow
 * @param subcol column of upper-left pixel of subwindow
 * @param dstnumpix number of pixels in resulting 1D image
 * @param dstpixlength length of virtual "rectangles" used to implement super pixels
 * 
 * NB: Untested
 */
void Subwin2Dto1DHorizontal(
        uint16_t * src, 
        uint16_t * dst, 
        uint8_t srccols, 
        uint8_t subrow, 
        uint8_t subcol, 
        uint8_t dstnumpix, 
        uint8_t dstpixlength);

/**
 * Extracts a subwindow from a 2D image and then sums rows to form a 1D image. 
 * See <b>Subwin2Dto1DHorizontal</b> for details.
 * 
 * NB: Untested
 */
void Subwin2Dto1DVertical(
        uint16_t * src, 
        uint16_t * dst, 
        uint8_t srccols, 
        uint8_t subrow, 
        uint8_t subcol, 
        uint8_t dstnumpix, 
        uint8_t dstpixlength);
