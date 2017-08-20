/*
   OpticalFlow.cpp Functions to calculate optical flow and odometry

   Copyright (c) 2012 Centeye, Inc. 
   All rights reserved.

   Redistribution and use in source and binary forms, with or without 
   modification, are permitted provided that the following conditions are met

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

/**
 *	Changes current optical flow value by low-pass filter with new.
 *
 *  @param filtered_OF filtered_OF filtered optical-flow value
 *  @param new_OF new filtered_OF filtered optical-flow value
 *  @alpha filter parameter (between 0 and 1)
 */
void LPF(int16_t *filtered_OF, int16_t *new_OF, float alpha);

/**
 *	Adds new optical flow value to accumulation sum iff only new value 
 *  falls outside threshold.
 *  @param new_OF new optical-flow value
 *  @param acc_OF pointer to accumulated value
 *  @param threshold threshold value
 *  @return true if value was added, false otherwise
 */      
bool Accumulate(int16_t new_OF, int16_t *acc_OF, int16_t threshold);

/**
 *	Runs a one-dimensional version of the image interpolation algorithm (IIA) described in
 *
 *  \@article{Srinivasan1994,<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;author="Srinivasan, M. V.",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;title="An image-interpolation technique for the computation of optic flow and egomotion",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;journal="Biological Cybernetics",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;year="1994",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;month="Sep",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;day="01",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;volume="71",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;number="5",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;pages="401--415",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;issn="1432-0770",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;doi="10.1007/BF00198917",<br>
 *  &nbsp;&nbsp;&nbsp;&nbsp;url="https://doi.org/10.1007/BF00198917"<br>
 *  }
 *
 *	@param curr_img pixels of current image
 *	@param last_img pixels of previous image
 *	@param numpix number of pixels
 *	@param scale value of one pixel of motion (for scaling output)
 *	@param out pointer to integer value for output.
 */
void IIA_1D(uint8_t *curr_img, uint8_t *last_img, uint8_t numpix, uint16_t scale, int16_t *out);

/**
 * Sixteen-bit version of above
 */
void IIA_1D(uint16_t *curr_img, uint16_t *last_img, uint8_t numpix, uint16_t scale, int16_t *out);

/**
 *  Runs a two-dimensional version of the Srinivasan algorithm, using a plus-shaped configuration of pixels
 *
 *	@param curr_img pixels of current image
 *	@param last_img pixels of previous image
 *	@param rows number of rows in image
 *	@param cols number of cols in image
 *	@param scale value of one pixel of motion (for scaling output)
 *	@param ofx pointer to integer value for X shift.
 *	@param ofy pointer to integer value for Y shift.
 */

void IIA_Plus_2D(uint8_t *curr_img, uint8_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Sixteen-bit version of above.
 */
void IIA_Plus_2D(uint16_t *curr_img, uint16_t *last_img, uint16_t rows,uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Same as above, using square configuration
 */
void IIA_Square_2D(uint8_t *curr_img, uint8_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Sixteen-bit version of above.
 */
void IIA_Square_2D(uint16_t *curr_img, uint16_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 *	Computes optical flow in plus configuration between two images using the algorithm desribed in
 *
 * \@inproceedings{Lucas:1981:IIR:1623264.1623280,<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;author = {Lucas, Bruce D. and Kanade, Takeo},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;title = {An Iterative Image Registration Technique with an Application to Stereo Vision},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;booktitle = {Proceedings of the 7th International Joint Conference on Artificial Intelligence - Volume 2},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;series = {IJCAI'81},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;year = {1981},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;location = {Vancouver, BC, Canada},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;pages = {674--679},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;numpages = {6},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;url = {http://dl.acm.org/citation.cfm?id=1623264.1623280},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;acmid = {1623280},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;publisher = {Morgan Kaufmann Publishers Inc.},<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;address = {San Francisco, CA, USA}<br>
 * }
 *	
 *	This algorithm assumes that displacements are generally on the order of one pixel or less. 
 *  This version uses a plus-shaped configuration of pixels.
 *
 *	@param curr_img pixels of current image
 *	@param last_img pixels of previous image
 *	@param rows number of rows in image
 *	@param cols number of cols in image
 *	@param scale value of one pixel of motion (for scaling output)
 *	@param ofx pointer to integer value for X shift.
 *	@param ofy pointer to integer value for Y shift.
 */
void LK_Plus_2D(uint8_t *curr_img, uint8_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale, int16_t * ofx, int16_t * ofy);

/**
 *	Sixteen-bit version of above.
 */
void LK_Plus_2D(uint16_t *curr_img, uint16_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Same as above, using square pixel configuration
 */
void LK_Square_2D(uint8_t *curr_img, uint8_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Sixteen-bit version of above
 */
void LK_Square_2D(uint16_t *curr_img, uint16_t *last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);
