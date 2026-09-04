cmake_minimum_required(VERSION 3.10)
project(week03_cpp_segmentation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(app cpp_segmentation.cpp)
target_link_libraries(app PRIVATE ${OpenCV_LIBS})