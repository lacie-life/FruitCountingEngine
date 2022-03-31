#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>
#include <caffe/caffe.hpp>
#include "pipeline.h"
#include <glog/logging.h>
// ----------------------------------------------------------------------

const char* keys =
        {
                "{help h usage ?  |                    | Print usage| }"
                "{ @input_video   |../data/VideoD.mkv  | Input video file | }"
                "{ e  example     |0                   | Number of example: 0 - SSD }"
                "{ ocl opencl     |1                   | Flag to use opencl | }"

                "{ sf start_frame |0                   | Frame modification parameter: Start a video from this position | }"
                "{ ef end_frame   |100000              | Frame modification parameter: Play a video to this position (if 0 then played to the end of file) | }"
                "{ crop           |1                   | Frame modification parameter: Flag to use location of interest | }"
                "{ crop_x         |960                   | Frame modification parameter: x coordinate of location of interest | }"
                "{ crop_y         |1100                   | Frame modification parameter: y coordinate of location of interest | }"
                "{ crop_width     |600                 | Frame modification parameter: width of location of interest | }"
                "{ crop_height    |400                 | Frame modification parameter: height of location of interest | }"

                //  07+12 SSD300
                "{ m model        |../models/VGGNet/VOC0712/SSD_300x300/deploy.prototxt | Detection parameter: Model file | }"
                "{ w weight       |../models/VGGNet/VOC0712/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel  | Detection parameter: Weight file | }"
//                 07+12 SSD512
//                "{ m model        |../models/VGGNet/VOC0712/SSD_512x512/deploy.prototxt | Detection parameter: Model file | }"
//                "{ w weight       |../models/VGGNet/VOC0712/SSD_512x512/VGG_VOC0712_SSD_512x512_iter_120000.caffemodel  | Detection parameter: Weight file | }"
                // 07++12 SSD300
//                    "{ m model        |../models/VGGNet/VOC0712Plus/SSD_300x300/deploy.prototxt | Detection parameter: Model file | }"
//                    "{ w weight       |../models/VGGNet/VOC0712Plus/SSD_300x300/VGG_VOC0712Plus_SSD_300x300_iter_240000.caffemodel  | Detection parameter: Weight file | }"
                // 07++12 SSD512
//                "{ w weight       |../models/VGGNet/VOC0712Plus/SSD_512x512/VGG_VOC0712Plus_SSD_512x512_iter_240000.caffemodel  | Detection parameter: Weight file | }"
//                "{ m model        |../models/VGGNet/VOC0712Plus/SSD_512x512/deploy.prototxt | Detection parameter: Model file | }"
                // 07+12+COCO SSD300
//                "{ m model        |../models/VGGNet/VOC0712/SSD_300x300_ft/deploy.prototxt | Detection parameter: Model file | }"
//                "{ w weight       |../models/VGGNet/VOC0712/SSD_300x300_ft/VGG_VOC0712_SSD_300x300_ft_iter_120000.caffemodel  | Detection parameter: Weight file | }"
                // 07+12+COCO SSD512

//                "{ m model        |../models/VGGNet/VOC0712/SSD_512x512_ft/deploy.prototxt | Detection parameter: Model file | }"
//                "{ w weight       |../models/VGGNet/VOC0712/SSD_512x512_ft/VGG_VOC0712_SSD_512x512_ft_iter_120000.caffemodel  | Detection parameter: Weight file | }"
                // 07++12+COCO SSD300
//                "{ m model        |../models/VGGNet/VOC0712Plus/SSD_300x300_ft/deploy.prototxt | Detection parameter: Model file | }"
//                "{ w weight       |../models/VGGNet/VOC0712Plus/SSD_300x300_ft/VGG_VOC0712Plus_SSD_300x300_ft_iter_160000.caffemodel  | Detection parameter: Weight file | }"
                // 07++12+COCO SSD512
//                "{ m model        |../models/VGGNet/VOC0712Plus/SSD_512x512_ft/deploy.prototxt | Detection parameter: Model file | }"
//                "{ w weight       |../models/VGGNet/VOC0712Plus/SSD_512x512_ft/VGG_VOC0712Plus_SSD_512x512_ft_iter_160000.caffemodel  | Detection parameter: Weight file | }"

                "{ lm label_map   |../models/VGGNet/VOC0712/SSD_300x300/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel  | Detection parameter: Label map  file  | }"
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

// ----------------------------------------------------------------------

int main(int argc, char** argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    // Log set up
    ::google::InitGoogleLogging(argv[0]);
    // Print output to stderr (while still logging)
    FLAGS_alsologtostderr = true;

    bool useOCL = parser.get<int>("opencl") != 0;
    cv::ocl::setUseOpenCL(useOCL);
    LOG(INFO) << (cv::ocl::useOpenCL() ? "OpenCL is enabled" : "OpenCL not used") << std::endl;

    auto exampleNum = parser.get<int>("example");

    switch (exampleNum)
    {
        case 0:
        {
            SSDExample ssdExample(parser);
            ssdExample.Process();
            break;
        }
        default:
            std::cerr << "Wrong example number!" << std::endl;
            break;
    }
    
    cv::destroyAllWindows();
    return 0;
}
