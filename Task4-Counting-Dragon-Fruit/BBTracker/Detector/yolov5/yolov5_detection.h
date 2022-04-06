#ifndef OBJECT_DETECTION_H
#define OBJECT_DETECTION_H

#include "opencv2/core/utility.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>
#include "cuda_utils.h"
#include "logging.h"
#include "common.hpp"
#include "utils.h"
#include "calibrator.h"
#include "GLViewer.hpp"

#include <sl/Camera.hpp>

struct Object{
    cv::Rect rec;
    float prob;
};

class ObjectDetection
{
    public:
        ObjectDetection(const std::string _model_path);
        std::vector<cv::Rect> detectObject(const cv::Mat& _frame);
    private:
};

#endif

