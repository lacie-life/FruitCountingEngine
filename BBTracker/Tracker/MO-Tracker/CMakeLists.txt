cmake_minimum_required(VERSION 3.5)

find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


set(CMAKE_CXX_STANDARD 14)

include(CheckIncludeFileCXX)
check_include_file_cxx(opencv2/bgsegm.hpp HAVE_OPENCV_CONTRIB)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/HungarianAlg
    ${CMAKE_CURRENT_SOURCE_DIR}/graph
    ${CMAKE_CURRENT_SOURCE_DIR}/graph/GTL/include)

option(USE_OPENCV "Build with OpenCV support" ON)

add_definitions(-DHAVE_OPENCV_CONTRIB)

option(USE_OCV_BGFG "Should use the bgfg algorithms from opencv_contrib?" ON)
option(USE_OCV_KCF "Should use the KCF tracker from opencv_contrib for collisions resolving?" ON)
option(USE_OCV_UKF "Should use the Unscented Kalman Filterr from opencv_contrib?" ON)

if(USE_OCV_UKF)
    add_definitions(-DUSE_OCV_UKF)
endif(USE_OCV_UKF)

if(USE_OCV_KCF)
    add_definitions(-DUSE_OCV_KCF)
endif(USE_OCV_KCF)

if(USE_OCV_BGFG)
    add_definitions(-DUSE_OCV_BGFG)
endif(USE_OCV_BGFG)

set(graph_source
        graph/tokenise.cpp
        graph/mytree.cpp
        graph/mygraph.cpp
        graph/mwbmatching.cpp
        graph/mincut.cpp
        graph/gport.cpp
        graph/fheap.c)

set(graph_header
        graph/tokenise.h
        graph/mytree.h
        graph/mygraph.h
        graph/mwbmatching.h
        graph/mincut.h
        graph/gport.h
        graph/gdefs.h
        graph/fheap.h)

set(gtl_source
        graph/GTL/src/bellman_ford.cpp
        graph/GTL/src/bfs.cpp
        graph/GTL/src/biconnectivity.cpp
        graph/GTL/src/bid_dijkstra.cpp
        graph/GTL/src/components.cpp
        graph/GTL/src/debug.cpp
        graph/GTL/src/dfs.cpp
        graph/GTL/src/dijkstra.cpp
        graph/GTL/src/edge.cpp
        graph/GTL/src/embedding.cpp
        graph/GTL/src/fm_partition.cpp
        graph/GTL/src/gml_parser.cpp
        graph/GTL/src/gml_scanner.cpp
        graph/GTL/src/graph.cpp
        graph/GTL/src/maxflow_ff.cpp
        graph/GTL/src/maxflow_pp.cpp
        graph/GTL/src/maxflow_sap.cpp
        graph/GTL/src/min_tree.cpp
        graph/GTL/src/node.cpp
        graph/GTL/src/planarity.cpp
        graph/GTL/src/pq_node.cpp
        graph/GTL/src/pq_tree.cpp
        graph/GTL/src/ratio_cut_partition.cpp
        graph/GTL/src/st_number.cpp
        graph/GTL/src/topsort.cpp)

set(gtl_header
        graph/GTL/include/GTL/algorithm.h
        graph/GTL/include/GTL/bellman_ford.h
        graph/GTL/include/GTL/bfs.h
        graph/GTL/include/GTL/biconnectivity.h
        graph/GTL/include/GTL/bid_dijkstra.h
        graph/GTL/include/GTL/bin_heap.h
        graph/GTL/include/GTL/components.h
        graph/GTL/include/GTL/debug.h
        graph/GTL/include/GTL/dfs.h
        graph/GTL/include/GTL/dijkstra.h
        graph/GTL/include/GTL/edge.h
        graph/GTL/include/GTL/edge_data.h
        graph/GTL/include/GTL/edge_map.h
        graph/GTL/include/GTL/embedding.h
        graph/GTL/include/GTL/fm_partition.h
        graph/GTL/include/GTL/gml_parser.h
        graph/GTL/include/GTL/gml_scanner.h
        graph/GTL/include/GTL/graph.h
        graph/GTL/include/GTL/maxflow_ff.h
        graph/GTL/include/GTL/maxflow_pp.h
        graph/GTL/include/GTL/maxflow_sap.h
        graph/GTL/include/GTL/min_tree.h
        graph/GTL/include/GTL/ne_map.h
        graph/GTL/include/GTL/node.h
        graph/GTL/include/GTL/node_data.h
        graph/GTL/include/GTL/node_map.h
        graph/GTL/include/GTL/planarity.h
        graph/GTL/include/GTL/pq_node.h
        graph/GTL/include/GTL/pq_tree.h
        graph/GTL/include/GTL/ratio_cut_partition.h
        graph/GTL/include/GTL/st_number.h
        graph/GTL/include/GTL/symlist.h
        graph/GTL/include/GTL/topsort.h
        graph/GTL/include/GTL/version.h
        graph/GTL/include/GTL/GTL.h)

set(folder_headers
        Ctracker.h
        track.h
        HungarianAlg/HungarianAlg.h
        LocalTracker.h
        Kalman.h)

set(folder_source
        Ctracker.cpp
        track.cpp
        HungarianAlg/HungarianAlg.cpp
        LocalTracker.cpp
        Kalman.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Detector/yolov5)

add_library(mo-tracker SHARED
        defines.h
        nms.h
        pipeline.h
        ${folder_source} 
        ${folder_headers} 
        ${graph_source} 
        ${graph_header} 
        ${gtl_source} 
        ${gtl_header})

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_headers})

source_group("graph" FILES ${graph_source} ${graph_header})
source_group("GTL" FILES ${gtl_source} ${gtl_header})

# ----------------------------------------------------------------------------
# Link libraries to project
# ----------------------------------------------------------------------------
target_link_libraries(mo-tracker ${OpenCV_LIBS})

