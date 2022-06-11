cmake_minimum_required(VERSION 3.5)

project(SmartWardrobeGUI VERSION 0.1 LANGUAGES CXX)

option(LINK_SHARED_ZED "Link with the ZED SDK shared executable" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-std=c++14 -O3)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets Sql SerialPort Svg REQUIRED)

find_package(ZED 3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CUDA REQUIRED)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES})

set(UI ${CMAKE_CURRENT_LIST_DIR}/ui/mainwindow.ui)

qt5_wrap_ui(UI_HEADERS ${UI})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(SmartWardrobeGUI
    ${HEADERS}
    ${SOURCES}
    ${UI_HEADERS}
    )

if (LINK_SHARED_ZED)
    set(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
else()
    set(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()

target_link_libraries(SmartWardrobeGUI PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Sql
    Qt5::SerialPort
    Qt5::Svg
    ${OpenCV_LIBS}
    ${ZED_LIBS})

set_target_properties(SmartWardrobeGUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

