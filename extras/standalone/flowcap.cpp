/*
flowcap.cpp illustrates ArduEye_OFO_v1 library by running 
optical flow on images from your webcam.

Copyright (C) 2017 Simon D. Levy

Requires: OpenCV
*/

#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include <OpticalFlow.h>

static const uint8_t  SCALEDOWN = 8;

// Flow display
static const cv::Scalar LINECOLOR = cv::Scalar(0, 255, 0); // green
static const cv::Scalar CIRCCOLOR = cv::Scalar(0, 0, 255); // red
static const uint8_t LINESCALE  = 20;
static const uint8_t CIRCRADIUS = 3;

static long millis() 
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void computeFlow(cv::Mat & prev, cv::Mat & curr, cv::Mat & colorimage)
{
    // Grab flow for this patch from the scaled-down previous and current images
    //rect_t rect = {x, y, PATCHSIZE, PATCHSIZE};
    int16_t ofx=0, ofy=0;
    LK_Plus_2D((uint8_t *)curr.data, (uint8_t *)prev.data, curr.rows, curr.cols, 10, &ofx, &ofy);

    // Add flow arrows to the scaled-up display image
    int cx = colorimage.cols/2;
    int cy = colorimage.rows/2;
    cv::Point ctr = cv::Point(cx,cy);
    cv::Point end = cv::Point(cx+ofx*LINESCALE,cy+ofy*LINESCALE);
    cv::line(colorimage, ctr, end, LINECOLOR);
    cv::circle(colorimage, end, CIRCRADIUS, CIRCCOLOR);
}

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

    // Start a timer and counter to report frames per second
    long count = 0;
    long start = millis();

    // We will use the previous and current frame to compute optical flow
    cv::Mat prev;

    while (true)
    {
        // Capture frame-by-frame
        cv::Mat frame;
        cap >> frame; 

        // Convert frame to gray
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Scale down to image to be used as current for optical flow
        cv::Mat curr;
        cv::resize(gray, curr, cv::Size(), 1./SCALEDOWN, 1./SCALEDOWN);

        // Scale back up for display
        cv::Mat display;
        cv::resize(curr, display, cv::Size(), SCALEDOWN, SCALEDOWN, cv::INTER_NEAREST);

        // Convert display image to color to support flow arrows
        cv::Mat cdisplay;
        cv::cvtColor(display, cdisplay, cv::COLOR_GRAY2BGR);

        // Compute and optical flow when previous image available
        if (!prev.empty()) {
            computeFlow(prev, curr, cdisplay);
        }

        // Display the image with flow arrows
        char windowname[100];
        sprintf(windowname, "flow: %d x %d", curr.cols, curr.rows);
        cv::imshow(windowname, cdisplay);
        if(cv::waitKey(1) >= 0) break;

        // Current image becomes previous for next iteration
        curr.copyTo(prev);

        // Increment the FPS counter
        count++;
    }

    // Report FPS
    float elapsed = (millis() - start) / 1000.;
    printf("%ld frames in %3.2f seconds = %3.2f fps\n", count, elapsed, count/elapsed);

    // The camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
