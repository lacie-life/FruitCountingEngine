cmake_minimum_required(VERSION 3.1)
project(zed-yolov5 VERSION 0.1 DESCRIPTION "Object detection with YOLOv5, TensorRT and Stereolabs ZED ")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -Wall -Wextra -Wno-deprecated -fPIC")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -pthread")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


## Imports
#####################

find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)
find_package(ZED 3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_YOLOV5 REQUIRED yolov5-tensorrt)


## Build
#####################

#
# detect
#
add_executable(detect detect.cpp)
target_include_directories(detect PUBLIC
    ${CUDA_INCLUDE_DIRS}
    ${PC_YOLOV5_INCLUDE_DIRS}
    ${ZED_INCLUDE_DIRS}
)
target_link_libraries(detect 
    yolov5-tensorrt 
    nvinfer
    nvonnxparser
    ${ZED_LIBRARIES}
    ${CUDA_CUDART_LIBRARY}
    ${CUDA_CUDA_LIBRARY}
    ${OpenCV_LIBRARIES}
)
