# Install script for directory: /home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/build/libyolov5-tensorrt.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyolov5-tensorrt.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/yolov5-tensorrt" TYPE FILE FILES
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_builder.hpp"
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_common.hpp"
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_detection.hpp"
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_detector.hpp"
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_detector_internal.hpp"
    "/home/jun/Github/Master-Thesis/Task3-TensorRT-Engine-with-Zed-2/yolov5/yolov5-tensorrt/include/yolov5_logging.hpp"
    )
endif()

