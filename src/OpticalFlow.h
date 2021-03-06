/*
   OpticalFlow.h Functions to calculate optical flow and odometry

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

// Choose one
typedef uint8_t  pixel_t;
//typedef uint16_t pixel_t;

/**
 *	Changes current optical flow value by low-pass filter with new.
 *
 *  @param filtered_OF filtered_OF filtered optical-flow value
 *  @param new_OF new filtered_OF filtered optical-flow value
 *  @alpha filter parameter (between 0 and 1)
 */
void ofoLPF(int16_t *filtered_OF, int16_t *new_OF, float alpha);

/**
 *	Adds new optical flow value to accumulation sum iff only new value 
 *  falls outside threshold.
 *  @param new_OF new optical-flow value
 *  @param acc_OF pointer to accumulated value
 *  @param threshold threshold value
 *  @return true if value was added, false otherwise
 */      
bool ofoAccumulate(int16_t new_OF, int16_t *acc_OF, int16_t threshold);

/**
 *	Runs a one-dimensional version of the image interpolation algorithm (IIA) described in
 *
 *  \@article{Srinivasan1994,<br>
 *  author="Srinivasan, M. V.",<br>
 *  title="An image-interpolation technique for the computation of optic flow and egomotion",<br>
 *  journal="Biological Cybernetics",<br>
 *  year="1994",<br>
 *  month="Sep",<br>
 *  day="01",<br>
 *  volume="71",<br>
 *  number="5",<br>
 *  pages="401--415",<br>
 *  issn="1432-0770",<br>
 *  doi="10.1007/BF00198917",<br>
 *  url={https://doi.org/10.1007/BF00198917}<br>
 *  }
 *
 *	@param curr_img pixels of current image
 *	@param last_img pixels of previous image
 *	@param numpix number of pixels
 *	@param scale value of one pixel of motion (for scaling output)
 *	@param out pointer to integer value for output.
 */
void ofoIIA_1D(pixel_t * curr_img, pixel_t * last_img, uint8_t numpix, uint16_t scale, int16_t *out);

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

void ofoIIA_Plus_2D(pixel_t * curr_img, pixel_t * last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 * Same as above, using square configuration
 */
void ofoIIA_Square_2D(pixel_t * curr_img, pixel_t * last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);

/**
 *	Computes optical flow in plus configuration between two images using the algorithm desribed in
 *
 * \@inproceedings{Lucas:1981:IIR:1623264.1623280,<br>
 * author = {Lucas, Bruce D. and Kanade, Takeo},<br>
 * title = {An Iterative Image Registration Technique with an Application to Stereo Vision},<br>
 * booktitle = {Proceedings of the 7th International Joint Conference on Artificial Intelligence - Volume 2},<br>
 * series = {IJCAI'81},<br>
 * year = {1981},<br>
 * location = {Vancouver, BC, Canada},<br>
 * pages = {674--679},<br>
 * numpages = {6},<br>
 * url = {http://dl.acm.org/citation.cfm?id=1623264.1623280},<br>
 * acmid = {1623280},<br>
 * publisher = {Morgan Kaufmann Publishers Inc.},<br>
 * address = {San Francisco, CA, USA}<br>
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
void ofoLK_Plus_2D(pixel_t * curr_img, pixel_t * last_img, uint16_t rows, uint16_t cols, uint16_t scale, int16_t * ofx, int16_t * ofy);

/**
 * Same as above, using square pixel configuration
 */
void ofoLK_Square_2D(pixel_t * curr_img, pixel_t * last_img, uint16_t rows, uint16_t cols, uint16_t scale,int16_t * ofx,int16_t * ofy);
