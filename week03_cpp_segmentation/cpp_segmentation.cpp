    #include <opencv2/opencv.hpp>
    #include <iostream>
    #include <chrono>

    static inline double now_sec() {
        using clock = std::chrono::high_resolution_clock;
        return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
    }


    int main() {    
        cv::VideoCapture cap(0, cv::CAP_V4L2);
        cap.set(cv::CAP_PROP_FPS, 60);
        if (!cap.isOpened()) {
            std::cerr << "ERROR: Camera could not be opened!\n"; 
            return 1;
        }

        // HSV color range for green 
        int h_low = 100, s_low = 100, v_low=50;
        int h_high = 130, s_high = 255, v_high = 255;

        int kernel_size = 3; //should be odd number
        int area_min = 800; 

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
        cv::createTrackbar("area_min","mask_clean",&area_min, 20000);

        cv::Mat frame, hsv, mask, cleaned;

        double t_prev = now_sec();
        double fps = 0.0;

        cv::Rect last_box;
        bool has_last_box= false;

        while(true) {
            cap >> frame;
            if (frame.empty()) 
                break;
            //FPS 
            double t_now = now_sec();
            double dt = t_now - t_prev;
            t_prev = t_now;
            if (dt > 0)
                fps = 1.0 / dt;

            cv::Mat view = frame.clone();

            //ROI determination
            cv::Rect roi_rect(0, 0, frame.cols, frame.rows);

            if (has_last_box) {
                int pad = 80;

                int x1 = std::max(0, last_box.x - pad);
                int y1 = std::max(0, last_box.y - pad);
                int x2 = std::min(frame.cols, last_box.x + last_box.width + pad);
                int y2 = std::min(frame.rows, last_box.y + last_box.height + pad);

                roi_rect = cv::Rect(x1, y1, x2 - x1, y2 - y1);
            }

            cv::rectangle(view, roi_rect, cv::Scalar(255, 0, 0), 2);

            cv::Mat roi_frame = frame(roi_rect);

            //BGR to HSV
            cv::cvtColor(roi_frame, hsv, cv::COLOR_BGR2HSV);

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
            cv::morphologyEx(cleaned, cleaned, cv::MORPH_CLOSE, kernel);

            //Contours
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(cleaned, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


            int best_idx = -1;

            double best_area = 0.0;
            for (int i = 0; i < (int)contours.size(); i++) {
                double a = cv::contourArea(contours[i]);

                if (a > best_area){
                    best_area = a;
                    best_idx = i;

                }
            }

            //find best contour and bbox it 
            if (best_idx >= 0 && best_area >= area_min) {

                cv::Rect local_box = cv::boundingRect(contours[best_idx]);

                cv::Rect box(
                    local_box.x + roi_rect.x,
                    local_box.y + roi_rect.y,
                    local_box.width,
                    local_box.height
                );

                cv::rectangle(view, box, cv::Scalar(0, 0, 255), 2);

                //better centroid
                cv::Moments m = cv::moments(contours[best_idx]);
                int cx_local = (m.m00 !=0.0) ? (int)(m.m10 / m.m00) : (local_box.x + local_box.width / 2);
                int cy_local = (m.m00 !=0.0) ? (int)(m.m01 / m.m00) : (local_box.y + local_box.height / 2);
            
                int cx = cx_local + roi_rect.x;
                int cy = cy_local + roi_rect.y;
                
                cv::circle(view, cv::Point(cx, cy), 5, cv::Scalar(255, 255, 255), -1);

                char info[160];
                std::snprintf(info, sizeof(info),
                            "area: %.0f cx:%d cy:%d", best_area, cx, cy);
                cv::putText(view, info, cv::Point(20,40),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7,
                            cv::Scalar(0, 255, 255), 2, cv::LINE_AA);

                last_box = box;
                has_last_box = true;
            }

            else {
                has_last_box = false;

                cv::putText(view, "target: None", cv::Point(20, 40),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7,
                            cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
            }

            char fps_info[80];
            std::snprintf(fps_info, sizeof(fps_info),
                            "FPS: %.1f", fps);
            cv::putText(view, fps_info,cv::Point(20,80),
                            cv::FONT_HERSHEY_SIMPLEX, 0.7,
                        cv::Scalar(0, 255, 255), 2, cv::LINE_AA);

            cv::imshow("camera", view);
            cv::imshow("mask_clean", cleaned);
        
            int key = cv::waitKey(1);
            if (key == 'q' || key == 27) //'q' or ESC
                break; 
        }

        cap.release();
        cv::destroyAllWindows();
        return 0;
    }



