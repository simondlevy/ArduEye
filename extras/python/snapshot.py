#!/usr/bin/env python3
'''
snapshot.py Python test program to go with Arduino example

Once you've flashed the Arudino sketch, this script will allow you to acquire a pattern-noise image
and use it to display a cleaned-up snapshot image.

Copyright (C) Simon D. Levy 2017

This file is part of Stonyman2.

Stonyman2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Stonyman2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with Stonyman2.  If not, see <http://www.gnu.org/licenses/>.
'''

from serial import Serial
import numpy as np
import matplotlib.pyplot as plt

# Changed this for your OS
PORT = '/dev/ttyACM0'

# This shouldn't need to be changed
BAUD = 115200

def getimage(stony, prompt):

    # Prompt the user for an action
    input(prompt)

    # Send command to deliver Matlab-formatted array
    stony.write('M'.encode())

    # Start with empty array
    a = []

    # Loop until EOF
    while True:

        # Grab and split a line from serial connection
        row = stony.readline().split()

        # Empty line = EOF
        if len(row) == 0:
            break

        # Lines containing 112 elements are legit
        elif len(row) == 112:
            a.append(row)

    # Return image as numpy array
    return np.array(a).astype('float')

if __name__ == '__main__':

    # Open serial connection to Arduino
    stony = Serial(PORT, BAUD, timeout=2)

    # Get pattern-noise image
    F = getimage(stony, 'Cover the lens and hit any key to acquire pattern noise ...')

    # Get actual image
    I = getimage(stony, 'Uncover the lens and hit any key to take a snapshot ...')

    # Subtract pattern noise from image and display result
    image = F - I

    plt.imshow(np.flipud(image), cmap='gray')
    plt.show()

    # Close serial connection to Arduino
    stony.close()
