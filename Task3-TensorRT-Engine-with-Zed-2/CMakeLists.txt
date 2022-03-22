
cmake_minimum_required(VERSION 2.8)
project(trt-ssd)

# setup build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings -Wno-deprecated-declarations -Wno-format-extra-args")	# -std=c++14 
set(SPECIAL_OS_LIBS "pthread" "X11")

find_package(ZED 3 REQUIRED)
find_package(OpenCV REQUIRED)

message("-- ZED SDK: ${ZED_VERSION}")

# setup CUDA
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/jetson-utils/cuda" )
find_package(CUDA ${ZED_CUDA_VERSION} EXACT REQUIRED)
message("-- CUDA version: ${CUDA_VERSION}")

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 
	-gencode arch=compute_53,code=sm_53
	-gencode arch=compute_62,code=sm_62
)

if(CUDA_VERSION_MAJOR GREATER 9)
	message("-- CUDA ${CUDA_VERSION_MAJOR} detected, enabling SM_72")

	set(
		CUDA_NVCC_FLAGS
		${CUDA_NVCC_FLAGS}; 
		-gencode arch=compute_72,code=sm_72
	)
endif()

# setup project output paths
set(PROJECT_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
set(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)

file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR})
file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)

message("-- system arch:  ${CMAKE_SYSTEM_PROCESSOR}")
message("-- output path:  ${PROJECT_OUTPUT_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)

# build C/C++ library
include_directories(${PROJECT_INCLUDE_DIR} 
                    ${PROJECT_INCLUDE_DIR}/trt-ssd 
                    ${PROJECT_INCLUDE_DIR}/jetson-utils
                    ${PROJECT_INCLUDE_DIR}/calibration)

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

link_directories(${ZED_LIBRARY_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

include_directories(/usr/include/gstreamer-1.0 
                    /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include 
                    /usr/include/glib-2.0 
                    /usr/include/libxml2 
                    /usr/lib/aarch64-linux-gnu/glib-2.0/include/)

file(GLOB inferenceSources trt-ssd/*.cpp trt-ssd/*.cu trt-ssd/*.cpp calibration/*.cpp)
file(GLOB inferenceIncludes trt-ssd/*.h trt-ssd/*.h calibration/*.h)

link_directories(/usr/lib/aarch64-linux-gnu/tegra)	
cuda_add_library(trt-ssd SHARED ${inferenceSources})

# transfer all headers to the include directory
file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/trt-ssd)

foreach(include ${inferenceIncludes})
	message("-- Copying ${include}")
	configure_file(${include} ${PROJECT_INCLUDE_DIR}/trt-ssd COPYONLY)
endforeach()

# create symbolic link for network and image data
execute_process( COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${PROJECT_SOURCE_DIR}/data/networks" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/networks" )
execute_process( COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${PROJECT_SOURCE_DIR}/data/images" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/images" )

add_subdirectory(jetson-utils)

# set linker options
target_link_libraries(trt-ssd jetson-utils nvinfer nvinfer_plugin nvcaffe_parser)

if(CUDA_VERSION_MAJOR GREATER 9)
	target_link_libraries(trt-ssd nvonnxparser)

    if(HAS_OPENCV) 
        message("-- Linking jetson-inference with OpenCV " ${OpenCV_VERSION})
        target_link_libraries(trt-ssd opencv_core opencv_calib3d)
    endif()
endif()

add_executable(ssd-detect test/trt-test.cpp)
target_link_libraries(ssd-detect trt-ssd)

add_executable(ssd-zed test/zed-test.cpp)
target_link_libraries(ssd-zed trt-ssd 
                              ${ZED_LIBRARIES} 
                              ${OpenCV_LIBRARIES})

