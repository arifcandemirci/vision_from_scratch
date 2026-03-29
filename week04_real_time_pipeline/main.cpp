/*
main.cpp

Entry point of the Week 04 realtime vision pipeline demo.

This file creates the pipeline object, starts the system and keeps the
application alive until the user closes the window or exits with a key.
*/

#include <iostream>
#include <thread>
#include <chrono>

#include "pipeline.hpp"

int main ()
{
    RealTimePipeline pipeline(0 , 5);

    if (!pipeline.start())
    {
        std::cerr << "Failed to start the realtime pipeline" << std::endl;
        return -1
    }

    std::cout <<"Realtime pipeline started"<< std::endl;
    std::cout <<"Press Q or ESC to quit"<< std::endl;

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // it checks that if user shut downs the window
        if (cv::getWindowProperty("Week04 Realtime Pipeline", cv::WND_PROP_VISIBLE) < 1)
        {
            break;
        }
    }

    pipeline.stop()
    return 0;
}