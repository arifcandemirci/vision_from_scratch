#pragma once

/*
RealtimePipeline

Manages the real-time vision pipeline using three threads:
capture, processing and display.

Flow:
Camera → Capture Thread → FrameQueue → Processing Thread → FrameQueue → Display Thread
*/

#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

#include "frame_packet.hpp"
#include "frame_queue.hpp"


class RealTimePipeline
{
public:
    RealTimePipeline(int camera_index = 0, std::size_t queue_size = 5)
                :   camera_index_(camera_index),
                    capture_queue_(capture_queue),
                    display_queue_(display_queue),
                    running_(false),
                    next_frame_id_(0),
                    display_fps_(0.0)
    {
    }

    ~RealTimePipeline()
    {
        stop();
    }

    bool start()
    {
        if (running_)
        {
            return false;
        }

        cap_.open(camera_index_);
        if (!cap_.isOpened())
        {
            return false;
        }

        running_ = false;

        capture_queue_.stop();
        display_queue_.stop();

        if (capture_thread_.joinable())
        {
            capture_thread_.joinable();
        }

        if (processing_thread_.joinable())
        {
            processing_thread_.joinable();
        } 

        if (display_thread_.joinable())
        {
            display_thread_.joinable();
        }

        if (cap_isOpened())
        {
            cap_.release();
        }

        cv::destroyAllWindows();
    }

private:
    void capture_loop()
    {
        while (running_)
        {
            cv::Mat frame;
            cap_ >> frame;

            if (frame.empty())
            {
                continue;
            }
            
            FramePacket packet;
            packet.frame_id = next_frame_id_++;
            packet.frame = frame.clone();
            packet.capture_time = std::chrono::steady_clock::now();
            packet.dropped_before = capture_queue_.dropped_count();

            capture_queue_.push(packet);
        }
    }

    void processing_loop()
    {
        while(running_)
        {
            FramePacket packet;

            if (!capture_queue_.pop(packet))
            {
                if(!running_)
                {
                    break;
                }
                continue;
            }
        
            packet.process_start_time = std::chrono::steady_clock::now();
            int frames_in_window = 0;

            while (running_)
            {
                FramePacket packet;

                if (!display_queue_.pop(packet))
                {
                    if (!running_)
                    {
                        break;
                    }
                    continue;
                }

                packet.latency_ms = std::chrono::duration<double, std::milli>(
                                    std::chrono::steady_clock::now() - packet.capture_time).count();
                
                ++frames_in_window;
                auto now = std::chrono::steady_clock::now();
                double elapsed_sec = std::chrono::duration<double>(now - last_fps_time).count();

                if (elapsed_sec >= 1.0)
                {
                    display_fps_ static_cast<double>(frames_in_window) / elapsed_sec;
                    frames_in_window = 0;
                    last_fps_time = now;
                }

                draw_overlay(packet);

                cv::imshow("Realtime Pipeline", packet.frame);

                int key = cv::waitKey(1);   
                if (key ==   27 || key =='q' || key =='Q')
                {
                    running_ = false;
                    capture_queue_.stop();
                    display_queue_.stop();
                    break;
                }            
            }
        
        }

    void run_debug_processing(cv::Mat& frame)
    {
        cv::Mat gray, edges;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::Canny(gray, edges, 80, 160);
        cv::cvtColor(edges, frame, cv::COLOR_GRAY2BGR);
    }

    void draw_overlay(FramePacket& packet)
    {
        const int x = 20;
        int y = 30;
        const int dy = 30;
        const double font_scale = 0.7;
        const int thickness = 2;

        draw_text(packet.frame, "Frame ID: " + std::to_string(packet.frame_id), x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, format_double("FPS: ", display_fps_, 2), x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, format_double("Processing: ", packet.processing_time_ms, 2) + " ms",
                  x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, format_double("Latency: ", packet.latency_ms, 2) + " ms",
                  x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, "Dropped: " + std::to_string(packet.dropped_before),
                  x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, "CaptureQ: " + std::to_string(capture_queue_.size()),
                  x, y, font_scale, thickness);
        y += dy;

        draw_text(packet.frame, "DisplayQ: " + std::to_string(display_queue_.size()),
                  x, y, font_scale, thickness);
    }

    void draw_text(cv::Mat& image,
                    const std::string& text,
                    int x,
                    int y,
                    double font_scale,
                    int thickness)
    {
        cv::putText(image, text, cv::Point(x, y),
                    cv::FONT_HERSHEY_SIMPLEX, font_scale,
                    cv::Scalar(0, 255, 0), thickness, cv::LINE_AA);
    }

    std::string format_double(const std::string& prefix, double value, int precision)
    {
        std::ostringstream oss;
        oss << prefix << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

private:
    int camera_index_;
    cv::VideoCapture cap_;

    FrameQueue<FramePacket> capture_queue_;
    FrameQueue<FramePacket> display_queue_;

    std::atomic<bool> running_;
    std::atomic<int> next_frame_id_;

    std::thread capture_thread_;    
    std::thread processing_thread_;
    std::thread display_thread_;

    double display_fps_;
};
}