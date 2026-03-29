# Week 04 — `pipeline.hpp` Code Walkthrough

This note explains **what each important line does** in your uploaded `pipeline.hpp` code, from the point of view of a **software / image processing engineer**.

Source code analyzed: the uploaded `Pasted code.cpp`. fileciteturn1file0

---

# 1. First, what this file is supposed to do

This file is trying to build a **real-time vision pipeline** with three stages:

```text
Camera
  ↓
Capture Thread
  ↓
Capture Queue
  ↓
Processing Thread
  ↓
Display Queue
  ↓
Display Thread
```

The idea is correct.

The file is trying to manage:

- camera input
- frame transport
- multithreading
- image processing
- overlay drawing
- display

So conceptually, this is the **control layer** of the whole Week 4 system.

---

# 2. Very important: your uploaded code has mistakes

Before explaining the lines, I need to be honest:

your uploaded code has several **syntax errors** and also some **logic errors**.

That is normal at this stage.

You understood the pipeline idea, but the code still needs correction.

So while teaching this file, I will do two things:

1. explain what each line is **trying** to do  
2. point out where the code is wrong and what the correct intention should be

That way you learn both:

- C++ meaning
- engineering/debugging mindset

---

# 3. Header guard and file comment

```cpp
#pragma once
```

## What it does

This tells the compiler:

> include this header file only once

## Why we use it

If the same header is included multiple times, the compiler may see the same class definition again and again.

`#pragma once` prevents that problem.

## Engineering view

This is standard in modern C++ headers.

---

```cpp
/*
RealtimePipeline

Manages the real-time vision pipeline using three threads:
capture, processing and display.

Flow:
Camera → Capture Thread → FrameQueue → Processing Thread → FrameQueue → Display Thread
*/
```

## What it does

This is only a comment.

It explains the role of the file.

## Why it matters

In real projects, a short file-level comment helps future you and teammates quickly understand the file.

---

# 4. Include lines

```cpp
#include <opencv2/opencv.hpp>
```

## What it does

This includes OpenCV types and functions.

## Why needed

Because this file uses:

- `cv::Mat`
- `cv::VideoCapture`
- `cv::imshow`
- `cv::waitKey`
- `cv::putText`
- `cv::cvtColor`
- `cv::Canny`

Without this include, OpenCV symbols would be unknown.

---

```cpp
#include <thread>
```

## What it does

Includes C++ thread support.

## Why needed

Because the code creates:

- `std::thread capture_thread_`
- `std::thread processing_thread_`
- `std::thread display_thread_`

Without `<thread>`, multithreading code would not compile.

---

```cpp
#include <atomic>
```

## What it does

Provides atomic variables.

## Why needed

Because the file uses:

- `std::atomic<bool> running_`
- `std::atomic<int> next_frame_id_`

Atomic variables are used when multiple threads access the same variable safely.

---

```cpp
#include <chrono>
```

## What it does

Provides time utilities.

## Why needed

Because the code uses timestamps such as:

- `std::chrono::steady_clock::now()`
- duration calculations for latency and processing time

This is essential in real-time systems.

---

```cpp
#include <string>
#include <sstream>
#include <iomanip>
```

## What they do

These are used for text formatting.

## Why needed

Because overlay strings like FPS and latency are built as formatted text.

For example:

- `std::string`
- `std::ostringstream`
- `std::setprecision`

---

```cpp
#include "frame_packet.hpp"
#include "frame_queue.hpp"
```

## What they do

These include your own project modules.

## Why needed

This file depends on:

- `FramePacket` → the data unit travelling through the pipeline
- `FrameQueue` → the thread-safe queue between threads

So `pipeline.hpp` cannot work without them.

---

# 5. Class definition

```cpp
class RealTimePipeline
{
```

## What it does

This starts the class definition.

## Why a class?

Because the pipeline is not one small function.

It contains:

- camera object
- queues
- threads
- configuration
- helper functions
- start/stop control

A class groups them into one system.

## Engineering view

This is the correct design direction.

---

# 6. Constructor

```cpp
RealTimePipeline(int camera_index = 0, std::size_t queue_size = 5)
```

## What it does

This is the constructor.

It creates a pipeline object and allows two settings:

- `camera_index`
- `queue_size`

## Meaning

- `camera_index = 0` means default camera
- `queue_size = 5` means queues can hold up to 5 frames

This is good because it makes the class configurable.

---

```cpp
:   camera_index_(camera_index),
    capture_queue_(capture_queue),
    display_queue_(display_queue),
    running_(false),
    next_frame_id_(0),
    display_fps_(0.0)
```

## What this syntax is

This is the **member initializer list**.

It initializes member variables before the constructor body runs.

## What each line is trying to do

### `camera_index_(camera_index)`

Store the constructor argument into the class member.

Correct.

---

### `capture_queue_(capture_queue)`

This line is wrong.

## Why wrong

`capture_queue` is not a constructor parameter.

You probably meant:

```cpp
capture_queue_(queue_size)
```

because `FrameQueue` needs a max size.

---

### `display_queue_(display_queue)`

This is also wrong for the same reason.

It should likely be:

```cpp
display_queue_(queue_size)
```

---

### `running_(false)`

Good.

At the start, the pipeline is not running yet.

---

### `next_frame_id_(0)`

Good.

Frame numbering starts at zero.

---

### `display_fps_(0.0)`

Good.

Initial FPS is zero before frames are displayed.

---

# 7. Destructor

```cpp
~RealTimePipeline()
{
    stop();
}
```

## What it does

When the object is destroyed, it automatically calls `stop()`.

## Why important

This is a resource cleanup pattern.

The pipeline owns:

- threads
- camera
- windows

If the object dies, it should shut everything down cleanly.

## Engineering view

Good idea.

This is RAII-style cleanup behavior.

---

# 8. `start()` function

```cpp
bool start()
{
```

## What it does

This function is supposed to:

- open the camera
- start threads
- set `running_ = true`
- return success/failure

That is the intended meaning.

---

```cpp
if (running_)
{
    return false;
}
```

## What it does

If the pipeline is already running, do not start again.

## Why useful

Starting twice would be dangerous:

- duplicate threads
- duplicated camera access
- inconsistent state

So this guard is correct.

---

```cpp
cap_.open(camera_index_);
if (!cap_.isOpened())
{
    return false;
}
```

## What it does

Try to open the selected camera.

If camera opening fails, return failure.

## Why important

A real pipeline cannot work without its input source.

This is one of the first health checks in vision systems.

---

## Problem after this point

Your `start()` function becomes logically wrong here.

You wrote code that belongs to `stop()` inside `start()`.

---

```cpp
running_ = false;
```

## Why wrong

This should be:

```cpp
running_ = true;
```

because `start()` should activate the pipeline.

Setting it to false would immediately keep loops from running.

---

```cpp
capture_queue_.stop();
display_queue_.stop();
```

## Why wrong here

This is shutdown behavior.

It does not belong in `start()`.

When starting, queues should be ready to work, not stopped.

---

```cpp
if (capture_thread_.joinable())
{
    capture_thread_.joinable();
}
```

## Why wrong

`joinable()` only checks if a thread can be joined.

It does **not** join it.

You probably intended:

```cpp
capture_thread_.join();
```

But even that would belong in `stop()`, not in `start()`.

The same mistake exists for:

- `processing_thread_`
- `display_thread_`

---

```cpp
if (cap_isOpened())
{
    cap_.release();
}
```

## Why wrong

This should be:

```cpp
if (cap_.isOpened())
```

and again, this belongs in `stop()`, not `start()`.

Also `cap_isOpened()` is not a valid function.

---

```cpp
cv::destroyAllWindows();
```

## Why wrong here

This closes OpenCV windows.

That is shutdown behavior, so it should be in `stop()`, not `start()`.

---

## Engineering conclusion about `start()`

Your `start()` currently mixes:

- start logic
- stop logic

That means the intent is understood, but the implementation is not correctly separated yet.

---

# 9. `capture_loop()`

```cpp
void capture_loop()
{
    while (running_)
    {
```

## What it does

This is the capture thread loop.

As long as the pipeline is running, it keeps capturing frames.

## Meaning

This loop is the **producer** side of the producer-consumer model.

---

```cpp
cv::Mat frame;
cap_ >> frame;
```

## What it does

Create a frame variable and read one image from the camera into it.

## Meaning

`cap_` is your camera stream object.

`>>` grabs the next frame.

---

```cpp
if (frame.empty())
{
    continue;
}
```

## What it does

If the camera failed to deliver a valid frame, skip this iteration.

## Why important

In real camera systems, frame reads can fail temporarily.

Never assume every read is valid.

This is a standard sanity check.

---

```cpp
FramePacket packet;
```

## What it does

Create a packet object that will carry the frame and metadata through the pipeline.

## Meaning

This is where raw image data becomes a tracked pipeline object.

---

```cpp
packet.frame_id = next_frame_id_++;
```

## What it does

Assign the current frame ID, then increment the counter for the next frame.

## Meaning

- current frame gets a unique number
- next frame will get the next number

This is useful for debugging and tracing.

---

```cpp
packet.frame = frame.clone();
```

## What it does

Store a deep copy of the frame into the packet.

## Why `clone()`?

Because camera frame buffers may be reused internally.

If you pass a shared image carelessly, another part of the program may overwrite it.

Using `clone()` makes the packet own its own image data.

## Engineering note

This is safer, though a little more expensive.

For Week 4, this is a good tradeoff.

---

```cpp
packet.capture_time = std::chrono::steady_clock::now();
```

## What it does

Store the exact moment the frame was captured.

## Why needed

Later, this timestamp is used to compute:

- latency
- queue wait
- timing metrics

---

```cpp
packet.dropped_before = capture_queue_.dropped_count();
```

## What it does

Stores how many frames had already been dropped before this packet was pushed.

## Why useful

This gives debug information about queue overload.

---

```cpp
capture_queue_.push(packet);
```

## What it does

Push the packet into the capture queue.

## Meaning

This sends the frame from the capture stage to the processing stage.

So this line is literally the handoff between two pipeline stages.

---

# 10. `processing_loop()`

```cpp
void processing_loop()
{
    while(running_)
    {
```

## What it does

This is supposed to be the processing thread loop.

It should:

- pop from capture queue
- process the frame
- measure processing time
- push to display queue

That is the intended architecture.

---

```cpp
FramePacket packet;
```

Create a local packet variable that will receive a frame from the queue.

---

```cpp
if (!capture_queue_.pop(packet))
{
    if(!running_)
    {
        break;
    }
    continue;
}
```

## What it does

Try to pop a packet from the capture queue.

- if no packet is available and system is stopping, exit loop
- otherwise continue waiting/trying

This is valid queue-consumer logic.

---

```cpp
packet.process_start_time = std::chrono::steady_clock::now();
```

## What it does

Store the moment processing starts.

## Why needed

Later used to compute processing duration.

---

## Major problem here

After this point, your code accidentally contains display-loop logic **inside** `processing_loop()`.

That means your thread roles got mixed.

You started a nested loop that should actually belong to a separate `display_loop()` function.

---

```cpp
int frames_in_window = 0;

while (running_)
{
    FramePacket packet;
```

## Why this is a problem

Inside `processing_loop()`, you opened another `while (running_)` and started popping from `display_queue_`.

That means:

- processing logic is incomplete
- display logic is inside processing logic
- packet variable is shadowed by another packet variable

This is not the intended architecture.

---

```cpp
if (!display_queue_.pop(packet))
```

## Why wrong here

The processing thread should not pop from `display_queue_`.

It should push **to** `display_queue_`.

Correct idea should be:

1. processing thread pops from `capture_queue_`
2. processing thread runs image processing
3. processing thread computes processing time
4. processing thread pushes to `display_queue_`

So this line belongs to `display_loop()`, not here.

---

```cpp
packet.latency_ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - packet.capture_time).count();
```

## What it does

Computes latency in milliseconds.

## Meaning

How much time has passed since the frame was captured until now.

## Engineering meaning

This is a key real-time performance metric.

Good metric, wrong location in current structure.

It should usually be updated close to display.

---

```cpp
++frames_in_window;
auto now = std::chrono::steady_clock::now();
double elapsed_sec = std::chrono::duration<double>(now - last_fps_time).count();
```

## What it does

This is trying to compute FPS over a time window.

## But there is a problem

`last_fps_time` is not declared in this scope.

So this would not compile.

You need something like:

```cpp
auto last_fps_time = std::chrono::steady_clock::now();
```

in the correct place.

---

```cpp
if (elapsed_sec >= 1.0)
{
    display_fps_ static_cast<double>(frames_in_window) / elapsed_sec;
    frames_in_window = 0;
    last_fps_time = now;
}
```

## What it is trying to do

Every ~1 second, update the FPS value.

## Error

This line is invalid:

```cpp
display_fps_ static_cast<double>(frames_in_window) / elapsed_sec;
```

You forgot the assignment operator.

It should be:

```cpp
display_fps_ = static_cast<double>(frames_in_window) / elapsed_sec;
```

---

```cpp
draw_overlay(packet);
```

## What it does

Draw debugging information on the image.

## Meaning

This converts raw metrics into visual feedback on the frame.

Good idea.

---

```cpp
cv::imshow("Realtime Pipeline", packet.frame);
```

## What it does

Show the frame in an OpenCV window.

## Meaning

This is the final visible output of the pipeline.

---

```cpp
int key = cv::waitKey(1);
```

## What it does

Wait 1 ms for a keyboard event.

## Why needed

OpenCV windows need `waitKey()` for event handling and refresh.

Without it, `imshow()` windows often do not update correctly.

---

```cpp
if (key == 27 || key =='q' || key =='Q')
{
    running_ = false;
    capture_queue_.stop();
    display_queue_.stop();
    break;
}
```

## What it does

Allows the user to quit with:

- `ESC`
- `q`
- `Q`

## Meaning

This is a user-controlled shutdown path.

Good idea.

But again, this logic belongs more naturally in a proper `display_loop()`.

---

# 11. Missing processing step

Your `processing_loop()` currently does not actually call the image processing function.

You defined:

```cpp
void run_debug_processing(cv::Mat& frame)
```

but you never called it inside the correct processing path.

The intended line should be something like:

```cpp
run_debug_processing(packet.frame);
```

followed by:

```cpp
packet.process_end_time = std::chrono::steady_clock::now();
packet.processing_time_ms = ...
display_queue_.push(packet);
```

That is currently missing from the real flow.

---

# 12. `run_debug_processing()`

```cpp
void run_debug_processing(cv::Mat& frame)
{
    cv::Mat gray, edges;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 80, 160);
    cv::cvtColor(edges, frame, cv::COLOR_GRAY2BGR);
}
```

## What it does step by step

### `cv::Mat gray, edges;`

Create two temporary images:

- `gray` for grayscale
- `edges` for edge result

---

### `cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);`

Convert the original color frame into grayscale.

## Why?

Canny edge detection typically works on grayscale input.

---

### `cv::Canny(gray, edges, 80, 160);`

Apply edge detection.

## Meaning of 80 and 160

These are the low and high thresholds for Canny.

This highlights image edges.

---

### `cv::cvtColor(edges, frame, cv::COLOR_GRAY2BGR);`

Convert the edge image back into BGR.

## Why?

Because later drawing functions and display code often expect a 3-channel image.

This lets you draw green overlay text on the result.

---

## Engineering meaning

This function is your placeholder processing stage.

Later you can replace it with:

- HSV segmentation
- contour detection
- ArUco detection
- optical flow
- deep learning inference

The pipeline structure stays the same.

Only this processing function changes.

---

# 13. `draw_overlay()`

```cpp
void draw_overlay(FramePacket& packet)
```

## What it does

Draws debugging text on top of the frame.

## Why useful

Real-time systems should be observable.

If you cannot see metrics, debugging gets much harder.

---

```cpp
const int x = 20;
int y = 30;
const int dy = 30;
const double font_scale = 0.7;
const int thickness = 2;
```

## What these do

These are layout parameters for text drawing.

- `x` = horizontal text position
- `y` = vertical start position
- `dy` = line spacing
- `font_scale` = text size
- `thickness` = text thickness

This creates a neat text stack on the frame.

---

```cpp
draw_text(packet.frame, "Frame ID: " + std::to_string(packet.frame_id), x, y, font_scale, thickness);
```

## What it does

Draw the frame ID on the image.

## Why useful

Helps track whether frames are skipped or delayed.

---

```cpp
draw_text(packet.frame, format_double("FPS: ", display_fps_, 2), x, y, font_scale, thickness);
```

## What it does

Draw the current display FPS.

## Meaning

Shows how fast images are reaching the screen.

---

```cpp
draw_text(packet.frame, format_double("Processing: ", packet.processing_time_ms, 2) + " ms", ...)
```

## What it does

Draw the processing time in milliseconds.

## Meaning

Shows how long the algorithm took for this frame.

---

```cpp
draw_text(packet.frame, format_double("Latency: ", packet.latency_ms, 2) + " ms", ...)
```

## What it does

Draw total frame latency.

## Meaning

Shows how old the frame is when displayed.

---

```cpp
draw_text(packet.frame, "Dropped: " + std::to_string(packet.dropped_before), ...)
```

## What it does

Shows how many frames had been dropped.

## Meaning

Useful for diagnosing overload.

---

```cpp
draw_text(packet.frame, "CaptureQ: " + std::to_string(capture_queue_.size()), ...)
draw_text(packet.frame, "DisplayQ: " + std::to_string(display_queue_.size()), ...)
```

## What they do

Draw current queue sizes.

## Meaning

This helps see if queues are filling up.

If queues stay high, the system is falling behind.

---

# 14. `draw_text()`

```cpp
void draw_text(cv::Mat& image,
               const std::string& text,
               int x,
               int y,
               double font_scale,
               int thickness)
```

## What it does

Small helper function that wraps `cv::putText()`.

## Why useful

Instead of repeating the same OpenCV text-drawing call everywhere, you centralize it in one helper.

This improves readability.

---

```cpp
cv::putText(image, text, cv::Point(x, y),
            cv::FONT_HERSHEY_SIMPLEX, font_scale,
            cv::Scalar(0, 255, 0), thickness, cv::LINE_AA);
```

## What it does

Actually draws text on the image.

### `cv::Point(x, y)`
Text position

### `cv::FONT_HERSHEY_SIMPLEX`
Font type

### `cv::Scalar(0, 255, 0)`
Green color in BGR

### `cv::LINE_AA`
Anti-aliased line for smoother text

---

# 15. `format_double()`

```cpp
std::string format_double(const std::string& prefix, double value, int precision)
```

## What it does

Converts a floating-point value into a nicely formatted string.

Example:

- `"FPS: 29.84"`
- `"Latency: 14.22"`

---

```cpp
std::ostringstream oss;
oss << prefix << std::fixed << std::setprecision(precision) << value;
return oss.str();
```

## Meaning

- build text in a stream
- force fixed decimal format
- keep a chosen number of digits after the decimal point
- return the final string

This is cleaner than rough string concatenation.

---

# 16. Private member variables

```cpp
int camera_index_;
```

Selected camera index.

---

```cpp
cv::VideoCapture cap_;
```

The camera/video stream object.

---

```cpp
FrameQueue<FramePacket> capture_queue_;
FrameQueue<FramePacket> display_queue_;
```

Two queues used between pipeline stages.

- capture queue: capture → processing
- display queue: processing → display

---

```cpp
std::atomic<bool> running_;
```

Global running flag used by all threads.

---

```cpp
std::atomic<int> next_frame_id_;
```

Shared frame counter.

---

```cpp
std::thread capture_thread_;    
std::thread processing_thread_;
std::thread display_thread_;
```

Thread objects for each pipeline stage.

---

```cpp
double display_fps_;
```

Stores displayed FPS.

## Small engineering note

Because multiple threads may access this, making it atomic or protecting it carefully would be cleaner in a stronger version.

For this learning project, the simple version is acceptable.

---

# 17. Final extra problem at the end of your file

Your uploaded file ends with an extra `}`.

So the class/file closing structure is also currently broken.

That is another syntax issue.

---

# 18. What the correct logical structure should be

Your intended structure should be:

## `start()`
- open camera
- set `running_ = true`
- start capture thread
- start processing thread
- start display thread

## `stop()`
- set `running_ = false`
- stop queues
- join threads
- release camera
- destroy windows

## `capture_loop()`
- read frame
- create packet
- push to capture queue

## `processing_loop()`
- pop from capture queue
- run processing
- measure processing time
- push to display queue

## `display_loop()`
- pop from display queue
- compute latency
- compute FPS
- draw overlay
- show image
- handle quit key

Right now your file mixes these responsibilities.

---

# 19. Most important lesson as an engineer

The biggest lesson from this file is not only syntax.

It is **responsibility separation**.

Each part of the pipeline should have one job:

- capture thread captures
- processing thread processes
- display thread displays
- start starts
- stop stops

When these responsibilities get mixed, bugs appear fast.

That is exactly what happened in your current version.

---

# 20. Short summary

Your code shows that you already understand the **architecture idea**:

- pipeline
- queues
- threads
- timing
- overlay

That is the hard conceptual part.

The remaining problem is **implementation discipline**:

- function boundaries
- correct initialization
- correct start/stop separation
- correct queue direction
- correct syntax

That is normal and fixable.

---

# 21. What you should remember from this file

## Conceptually correct ideas you already have

- using a class for the pipeline
- using thread-safe queues
- using metadata per frame
- measuring latency and FPS
- using overlay for debugging
- separating processing into a helper function

## Main mistakes to fix

- constructor queue initialization
- `start()` vs `stop()` confusion
- missing thread creation in `start()`
- missing real `stop()` function
- display logic accidentally placed inside `processing_loop()`
- missing processing timing completion
- missing push to `display_queue_`
- undeclared `last_fps_time`
- missing `=` in FPS assignment
- invalid `cap_isOpened()`
- wrong `joinable()` usage
- mismatched braces

---

# 22. Recommended next step

The best next step is:

1. cleanly rewrite `pipeline.hpp`
2. then write `main.cpp`
3. then build and test one stage at a time

That is how an engineer debugs systems:

- first structure
- then compile
- then run
- then measure

Not all at once.
