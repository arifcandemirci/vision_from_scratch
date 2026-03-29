/*
FramePacket

A container that represents one image frame moving through the vision pipeline.

It stores:
- the image itself (cv::Mat)
- a frame ID
- timestamps for capture and processing
- latency and processing time information

This metadata allows the system to measure FPS, latency and debugging
information while the frame travels through different pipeline stages.
*/

#pragma once

#include <opencv2/opencv.hpp>
#include <chrono>

struct FramePacket
{
    int frame_id = 0;
    cv::Mat frame;

    std::chrono::steady_clock::time_point capture_time;
    std::chrono::steady_clock::time_point process_start_time;
    std::chrono::steady_clock::time_point process_end_time;

    double processing_time_ms = 0.0;
    double latency_ms = 0.0;

    std::size_t dropped_before = 0;
};