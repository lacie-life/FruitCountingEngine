#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>

#include "MO-Tracker/pipeline.h"

const char* keys =
        {
                "{help h usage ?  |                    | Print usage| }"
                "{ @input_video   |/home/jun/Github/Master-Thesis/Task4-Counting-Dragon-Fruit/BBTracker/MOT17-11.mp4  | Input video file | }"
                "{ e  example     |0                   | Number of example: 0 - SSD }"
                "{ ocl opencl     |1                   | Flag to use opencl | }"

                "{ sf start_frame |0                   | Frame modification parameter: Start a video from this position | }"
                "{ ef end_frame   |100000              | Frame modification parameter: Play a video to this position (if 0 then played to the end of file) | }"
                "{ crop           |1                   | Frame modification parameter: Flag to use location of interest | }"
                "{ crop_x         |960                   | Frame modification parameter: x coordinate of location of interest | }"
                "{ crop_y         |1100                   | Frame modification parameter: y coordinate of location of interest | }"
                "{ crop_width     |600                 | Frame modification parameter: width of location of interest | }"
                "{ crop_height    |400                 | Frame modification parameter: height of location of interest | }"

                "{ m model        |/home/jun/Github/Master-Thesis/Task4-Counting-Dragon-Fruit/BBTracker/yolo.engine | Detection parameter: Model file | }"

                "{ th threshold   |0.5                 | Detection parameter: Confidence percentage of detected objects must exceed this value to be reported as a detected object. | }"
                "{ dd desired_detect |1                | Detection Parameter: Flag to detect only desired objects | }"
                "{ dd desired_objects |15,2,12   | Detection Parameter: list of desired objects to detect | }"

                "{ co count       |1                   | Counting parameter: Flag to use counting  | }"
                "{ d direction    |2                   | Counting parameter: Variable to allow counting in a certain direction. 0 - left to right, 1 - right to left, 2 - both | }"
                "{ l1p1_x         |290                   | Counting parameter: line 1 point 1 x coordinate  | }"
                "{ l1p1_y         |0                  | Counting parameter: line 1 point 1 y coordinate  | }"
                "{ l1p2_x         |290                   | Counting parameter: line 1 point 2 x coordinate  | }"
                "{ l1p2_y         |600                   | Counting parameter: line 1 point 2 y coordinate  | }"
                "{ l2p1_x         |310                  | Counting parameter: line 2 point 1 x coordinate  | }"
                "{ l2p1_y         |0                  | Counting parameter: line 2 point 1 y coordinate  | }"
                "{ l2p2_x         |310                   | Counting parameter: line 2 point 2 x coordinate  | }"
                "{ l2p2_y         |600                   | Counting parameter: line 2 point 2 y coordinate  | }"

                "{ dc draw_count  |1                   | Counting parameter: Flag to enable drawing info for count  | }"
                "{ do draw_other  |1                   | Counting parameter: Flag to enable drawing info other component  | }"

                "{ o output       |../data/D2.avi      | Writing parameter: Name of output video file | }"
                "{ save_video     |1                   | Writing parameter: Flag to enable writing to file | }"
};

int main(int argc, char** argv)
{
    std::cout << "Hello world" << std::endl;

    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }


    bool useOCL = parser.get<int>("opencl") != 0;
    cv::ocl::setUseOpenCL(useOCL);

    auto exampleNum = parser.get<int>("example");

    switch (exampleNum)
    {
        case 0:
        {
            MODetAndTrack MOExample(parser);
            MOExample.Process();
            break;
        }
        default:
            std::cerr << "Wrong example number!" << std::endl;
            break;
    }


    cv::destroyAllWindows();
    return 0;
}