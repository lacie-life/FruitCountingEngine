cmake_minimum_required(VERSION 3.2)

project(bbtracker)

option(LINK_SHARED_ZED "Link with the ZED SDK shared executable" ON)

if (NOT LINK_SHARED_ZED AND MSVC)
    message(FATAL_ERROR "LINK_SHARED_ZED OFF : ZED SDK static libraries not available on Windows")
endif()

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

set(EXECUTABLE_OUTPUT_PATH ".")
set(SPECIAL_OS_LIBS "")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(ZED 3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(CUDA REQUIRED)
find_package(Threads REQUIRED)


IF(NOT WIN32)
    set(SPECIAL_OS_LIBS "pthread" "X11")
    add_definitions(-Wno-write-strings)
ENDIF()

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLUT_INCLUDE_DIR})
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Tracker)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Detector/yolov5)

link_directories(${ZED_LIBRARY_DIR})
link_directories(${GLEW_LIBRARY_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})
# cuda
include_directories(/usr/local/cuda/include)
link_directories(/usr/local/cuda/lib64)
# tensorrt
include_directories(/usr/include/x86_64-linux-gnu/)
link_directories(/usr/lib/x86_64-linux-gnu/)

FILE(GLOB_RECURSE SRC_FILES Detector/yolov5/src/*.c*
                            Detector/yolov5/yolov5_detection.cpp
                            Tracker/*.cpp
                            main.cpp)
FILE(GLOB_RECURSE HDR_FILES Detector/yolov5/include/*.h*
                            Detector/yolov5/include/yolov5_detection.h
                            Tracker/*.h)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} -std=c++11)

cuda_add_executable(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES})

add_definitions(-std=c++14 -g -O3 -D_MWAITXINTRIN_H_INCLUDED -Wno-deprecated-declarations)


if (LINK_SHARED_ZED)
    SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
else()
    SET(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()

SET(TRT_LIBS nvinfer)

target_link_libraries(${PROJECT_NAME}
                        ${TRT_LIBS}
                        ${SPECIAL_OS_LIBS}
                        ${ZED_LIBS}
                        ${OPENGL_LIBRARIES}
                        ${GLUT_LIBRARY}
                        ${GLEW_LIBRARIES}
                        ${OpenCV_LIBRARIES}
                        ${CUDA_CUDA_LIBRARY} 
                        ${CUDA_CUDART_LIBRARY})

if(INSTALL_SAMPLES)
    LIST(APPEND SAMPLE_LIST ${PROJECT_NAME})
    SET(SAMPLE_LIST "${SAMPLE_LIST}" PARENT_SCOPE)
endif()
