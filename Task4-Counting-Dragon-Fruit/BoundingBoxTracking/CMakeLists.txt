cmake_minimum_required(VERSION 3.2)

project(vpi-bbtracker)

find_package(vpi REQUIRED)
find_package(OpenCV 4 REQUIRED)

set(
	CUDA_NVCC_FLAGS
	-O3
	-gencode arch=compute_72,code=sm_72
)

cuda_add_library(cuda-utils SHARED cuda_utils.cu cuda_utils.cuh)

cuda_add_executable(vpi-bbtracker main.cpp)

target_link_libraries(vpi-bbtracker vpi opencv_core opencv_imgproc opencv_imgcodecs cuda-utils)

