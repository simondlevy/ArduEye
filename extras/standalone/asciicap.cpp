/*
ascicap.cpp illustrates Images_v1 library by displaying a live ASCII image from your webcam.

Copyright (C) 2017 Simon D. Levy

Requires: OpenCV
*/

#include <opencv2/opencv.hpp>
#include <ImageUtils.h>
#include <stdint.h>

static const int W = 80;
static const int H = 60;

int main(int argc, char** argv)
{
    // Check for camera number on command line; default to 0
    int camno = (argc > 1) ? atoi(argv[1]) : 0;

    // Open a video capture stream, exiting on failure
    cv::VideoCapture cap(camno); 
    if(!cap.isOpened()) {
        fprintf(stderr, "Unable to open camera\n");
        return -1;
    }

    while (true)
    {
        // Capture image frame-by-frame
        cv::Mat frame;
        cap >> frame; 

        // Display the image in OpenCV
        cv::imshow("Hit ESC to quit", frame);
        if(cv::waitKey(1) >= 0) break;

        // Convert frame to gray
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Scale down to image to be used for ASCII display
        cv::Mat small;
        cv::resize(gray, small, cv::Size(W,H), 0, 0);

        // Copy scaled-down grayscale to short ints
        uint16_t shortimage[W*H];
        for (int k=0; k<W*H; ++k) {
            shortimage[k] = small.data[k];
        }

        // Dump image as ASCII
        imgDumpAscii(shortimage, H, W, 0, 255);
    }

    // The camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
