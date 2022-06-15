cmake_minimum_required(VERSION 2.8)

project(FruitCountingEngine)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

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

find_package(Qt5 COMPONENTS Core Widgets Sql SerialPort Svg REQUIRED)

find_package(OpenMP)
if(OpenMP_CXX_FOUND OR OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Detector/yolov5)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Tracker)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Engine/include)

# ==================== Engine GUI ================================== # 
# ================================================================== #
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Engine/src/*.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

set(UI ${CMAKE_CURRENT_LIST_DIR}/Engine/ui/mainwindow.ui)

qt5_wrap_ui(UI_HEADERS ${UI})

add_executable(FruitCountingEngine
    ${HEADERS}
    ${SOURCES}
    ${UI_HEADERS}
)

if (LINK_SHARED_ZED)
    set(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
else()
    set(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()

target_link_libraries(FruitCountingEngine
    Qt5::Widgets
    Qt5::Core
    Qt5::Sql
    Qt5::SerialPort
    Qt5::Svg
    ${OpenCV_LIBS}
    ${ZED_LIBS}
    yolov5-detector
    mo-tracker
    )

# ========================= Test ======================================= #
# ====================================================================== #
#add_executable(bbtracker main.cpp)

#target_link_libraries(bbtracker ${OpenCV_LIBS}
#                                yolov5-detector
#                                lk-tracker
#                                mo-tracker
#                                ${CUDA_CUDA_LIBRARY}
#                                ${ZED_LIBRARIES})

#add_executable(detector_test Test/detector_test.cpp)

#target_link_libraries(detector_test ${OpenCV_LIBS}
#                                    yolov5-detector
#                                    lk-tracker
#                                    mo-tracker
#                                    ${CUDA_CUDA_LIBRARY}
#                                    ${ZED_LIBRARIES})

#add_executable(mo_tracker_test Test/mo_tracker_test.cpp Test/zed2_pipeline.h)

#target_link_libraries(mo_tracker_test ${OpenCV_LIBS}
#                                      yolov5-detector
#                                      lk-tracker
#                                      mo-tracker
#                                      ${CUDA_CUDA_LIBRARY}
#                                      ${ZED_LIBRARIES})

# ====================== Data =========================================== #
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
