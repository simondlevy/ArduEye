/*
flowcap.cpp illustrates ArduEye OpticalFlow library by running optical flow on
images from your webcam.

Copyright (C) 2017 Simon D. Levy

Requires: OpenCV
*/

#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include <OpticalFlow.h>

// Standard webcam image size
static const int cam_cols = 640;
static const int cam_rows = 480;

// These params work well with the 640x480 image of a typical webcam
static const uint8_t  IMAGE_SCALEDOWN = 8;
static const uint8_t  PATCHES_PER_ROW = 8;

// Flow display
static const cv::Scalar LINECOLOR = cv::Scalar(0, 255, 0); // green
static const cv::Scalar CIRCCOLOR = cv::Scalar(0, 0, 255); // red
static const uint8_t CIRCRADIUS = 3;
static const uint16_t FLOWSCALE = 20;

static long millis() 
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void addFlow(cv::Mat & image, int16_t ofx, int16_t ofy)
{
    uint8_t patchsize = image.cols/PATCHES_PER_ROW;
    for (int y=0; y<image.rows; y+=patchsize) {
        for (int x=0; x<image.cols; x+=patchsize) {
            int cx = x + patchsize/2;
            int cy = y + patchsize/2;
            cv::Point ctr = cv::Point(cx,cy);
            cv::Point end = cv::Point(cx+ofx,cy+ofy);
            cv::line(image, ctr, end, LINECOLOR);
            cv::circle(image, end, CIRCRADIUS, CIRCCOLOR);
        }
    }
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

    while (true) {

        // Capture frame-by-frame
        cv::Mat frame;
        cap >> frame; 

        // Convert frame to gray
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // Scale down to image to be used as current for optical flow
        cv::Mat curr;
        cv::resize(gray, curr, cv::Size(), 1./IMAGE_SCALEDOWN, 1./IMAGE_SCALEDOWN);

        // Scale back up for display
        cv::Mat display;
        cv::resize(curr, display, cv::Size(), IMAGE_SCALEDOWN, IMAGE_SCALEDOWN, cv::INTER_NEAREST);

        // Convert display image to color to support flow arrows
        cv::Mat cdisplay;
        cv::cvtColor(display, cdisplay, cv::COLOR_GRAY2BGR);

        // Compute and optical flow when previous image available
        if (!prev.empty()) {

            int16_t ofx=0, ofy=0;
            ofoLK_Plus_2D((uint8_t *)curr.data, (uint8_t *)prev.data, curr.rows, curr.cols, FLOWSCALE, &ofx, &ofy);

            addFlow(cdisplay, ofx, ofy);
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
