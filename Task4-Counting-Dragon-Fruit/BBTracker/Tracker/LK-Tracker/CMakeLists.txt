cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_package(Threads REQUIRED)
add_definitions(-std=c++11)

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)

find_package(ZED 3 REQUIRED)
include_directories(${ZED_INCLUDE_DIRS})
link_directories(${ZED_LIBRARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Detector/yolov5)

add_library(lk-tracker SHARED lk_tracker.cpp munkres.cpp det_and_track.cpp)
target_link_libraries(lk-tracker ${OpenCV_LIBS} 
                                 yolov5-detector 
                                 ${CUDA_CUDA_LIBRARY}
                                 ${ZED_LIBRARIES})
