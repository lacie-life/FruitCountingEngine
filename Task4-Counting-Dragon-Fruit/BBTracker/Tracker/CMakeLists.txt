if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package(Threads REQUIRED)
add_definitions(-std=c++11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Detector/yolov5)

add_library(tracker SHARED lk_tracker.cpp munkres.cpp det_and_track.cpp)
target_link_libraries(tracker ${OpenCV_LIBS} detector)
