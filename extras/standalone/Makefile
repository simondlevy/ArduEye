# Makefile for flow-capture program
#
# Copyright (C) 2017 Simon D. Levy
#
# Requires: OpenCV

SRC = ../../src

all: asciicap flowcap

flowcap: flowcap.o OpticalFlow.o
	g++  -g -o flowcap  flowcap.o OpticalFlow.o `pkg-config opencv --libs`

flowcap.o: flowcap.cpp $(SRC)/OpticalFlow.h Makefile
	g++  -Wall -I$(SRC) -c flowcap.cpp  `pkg-config opencv --cflags`

OpticalFlow.o: $(SRC)/OpticalFlow.cpp $(SRC)/OpticalFlow.h Makefile
	g++ -Wall -I$(SRC) -c $(SRC)/OpticalFlow.cpp

asciicap: asciicap.o ImageUtils.o
	g++  -g -o asciicap  asciicap.o ImageUtils.o `pkg-config opencv --libs`

asciicap.o: asciicap.cpp $(SRC)/ImageUtils.h Makefile
	g++  -Wall -I$(SRC) -c asciicap.cpp  `pkg-config opencv --cflags`

ImageUtils.o: $(SRC)/ImageUtils.cpp $(SRC)/ImageUtils.h Makefile
	g++  -Wall -c $(SRC)/ImageUtils.cpp


clean:
	rm -rf asciicap flowcap *.o *~ 
