cmake_minimum_required(VERSION 2.8)

project(bbtracker)

add_definitions(-std=c++11)

add_subdirectory(Detector)
add_subdirectory(Tracker)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Detector/yolov5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Tracker)

add_executable(bbtracker main.cpp)
target_link_libraries(bbtracker ${OpenCV_LIBS} detector tracker)