#include "yolov5_detection.h"


ObjectDetection::ObjectDetection(const std::string _model_path)
{   
    // deserialize the .engine and run inference
    std::ifstream file(_model_path, std::ios::binary);
    if (!file.good()) {
        std::cerr << "read " << _model_path << " error!" << std::endl;
        exit(0);
    }
    char *trtModelStream = nullptr;
    size_t size = 0;
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();
 
}

std::vector<cv::Rect> ObjectDetection::detectObject(const cv::Mat& _frame)
{
    std::vector<cv::Rect> boxes;

    return boxes;
}
