cmake_minimum_required(VERSION 2.8)

project(bbtracker)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_subdirectory(Detector)
add_subdirectory(Tracker)


find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)

find_package(ZED 3 REQUIRED)
include_directories(${ZED_INCLUDE_DIRS})
link_directories(${ZED_LIBRARY_DIR})

find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Detector/yolov5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Tracker)

add_executable(bbtracker main.cpp)

target_link_libraries(bbtracker ${OpenCV_LIBS} 
                                detector 
                                lk-tracker
                                mo-tracker 
                                ${CUDA_CUDA_LIBRARY} 
                                ${ZED_LIBRARIES})

add_executable(detector_test Test/detector_test.cpp)

target_link_libraries(detector_test ${OpenCV_LIBS} 
                                    detector 
                                    lk-tracker
                                    mo-tracker 
                                    ${CUDA_CUDA_LIBRARY} 
                                    ${ZED_LIBRARIES})

add_executable(mo_tracker_test Test/mo_tracker_test.cpp)

target_link_libraries(mo_tracker_test ${OpenCV_LIBS} 
                                      detector 
                                      lk-tracker
                                      mo-tracker 
                                      ${CUDA_CUDA_LIBRARY} 
                                      ${ZED_LIBRARIES})

add_executable(zed2_test Test/zed2_test.cpp)

target_link_libraries(zed2_test ${OpenCV_LIBS} 
                                detector 
                                lk-tracker
                                mo-tracker 
                                ${CUDA_CUDA_LIBRARY} 
                                ${ZED_LIBRARIES})
