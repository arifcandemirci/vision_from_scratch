#include <opencv2/opencv.hpp>
#include <iostream>

int main() {    
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "ERROR: Camera could not be opened!\n"; 
        return 1;
    }

    // HSV color range for green 
    int h_low = 35, s_low = 50, v_low;
    int h_high = 85, s_high = 255, v_high = 255;

    int kernel_size = 3; //should be odd number

    cv::namedWindow("camera", cv::WINDOW_NORMAL);
    cv::namedWindow("mask_clean", cv::WINDOW_NORMAL);

    // Trackbar

    cv::createTrackbar("H low", "mask_clean", &h_low, 179);
    cv::createTrackbar("H high", "mask_clean", &h_high, 179);
    cv::createTrackbar("S low", "mask_clean", &s_low, 255);
    cv::createTrackbar("S high", "mask_clean", &s_high, 255);
    cv::createTrackbar("V low", "mask_clean", &v_low, 255);
    cv::createTrackbar("V high", "mask_clean", &v_high, 255);
    cv::createTrackbar("ksize", "mask_clean", &kernel_size, 17);

    cv::Mat frame, hsv, mask, cleaned;

    while(true) {
        cap >> frame;
        if (frame.empty()) 
            break;
        
        //BGR to HSV
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        cv::inRange(hsv,
            cv::Scalar(h_low, s_low, v_low),
            cv::Scalar(h_high, s_high, v_high),
            mask
        );

        //noise cleaning with morphology
        int  k = std::max(1, kernel_size);
        if (k % 2 == 0)
            k += 1; //if k is not odd, make it odd 

        cv::Mat kernel = cv::getStructuringElement(
            cv::MORPH_ELLIPSE, cv::Size(k, k)
        );

        //OPEN: clean little white noise
        cv::morphologyEx(mask, cleaned, cv::MORPH_OPEN, kernel);

        //CLOSE: close the holes in the object
        cv::morphologyEx(mask, cleaned, cv::MORPH_CLOSE, kernel);

        cv::imshow("camera", frame);
        cv::imshow("mask_clean", cleaned);
    
        int key = cv::waitKey(1);
        if (key == 'q' || key == 27) //'q' or ESC
            break; 
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}



