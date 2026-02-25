# Week 0: Environment Setup 🏗️

Welcome to the Computer Vision course! Before diving into algorithms, we need to ensure your development environment is correctly configured. This guide covers the installation of necessary dependencies for both Python and C++ development, as well as ROS2 integration.

## 💻 Operating System
- **Recommended**: Ubuntu 22.04 LTS (Jammy Jellyfish)

## 🐍 Python Environment
Python is our primary language for prototyping and implementing most vision algorithms.

### Prerequisites
Ensure you have Python 3 and `pip` installed:
```bash
sudo apt update
sudo apt install python3-pip
```

### Required Libraries
Install the core computer vision and numerical processing libraries:
```bash
pip install opencv-python numpy matplotlib
```

## ⚙️ C++ Development
For performance-critical applications, C++ is essential.

### Toolchain
Install the standard build tools and CMake:
```bash
sudo apt install build-essential cmake
```

### OpenCV for C++
Install the OpenCV development headers and libraries:
```bash
sudo apt install libopencv-dev
```

## 🤖 ROS2 Integration
If you are working with robotics, ROS2 is our middleware of choice.

- **Distribution**: ROS2 Foxy Fitzroy
- **Setup**: To initialize your environment, source the ROS2 setup script:
  ```bash
  source /opt/ros/foxy/setup.bash
  ```
  *Tip: Add this line to your `.bashrc` for automatic sourcing in every new terminal.*

---
*Next Step: Explore [Week 1: Color Segmentation](../week01_color_segmentation/README.md)*
