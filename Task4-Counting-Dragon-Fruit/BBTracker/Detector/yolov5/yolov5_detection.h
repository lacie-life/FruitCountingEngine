#ifndef YOLOV5_DETECTION_H
#define YOLOV5_DETECTION_H

#include <opencv2/core/utility.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <cmath>

#include "include/cuda_utils.h"
#include "include/logging.h"
#include "include/common.hpp"
#include "include/utils.h"
#include "include/calibrator.h"
#include "include/GLViewer.hpp"

#include <sl/Camera.hpp>

#define USE_FP16  // set USE_INT8 or USE_FP16 or USE_FP32
#define DEVICE 0  // GPU id
#define NMS_THRESH 0.4
#define CONF_THRESH 0.5
#define BATCH_SIZE 1

// stuff we know about the network and the input/output blobs
static const int INPUT_H = Yolo::INPUT_H;
static const int INPUT_W = Yolo::INPUT_W;
static const int CLASS_NUM = Yolo::CLASS_NUM;
static const int OUTPUT_SIZE = Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof (Yolo::Detection) / sizeof (float) + 1; // we assume the yololayer outputs no more than MAX_OUTPUT_BBOX_COUNT boxes that conf >= 0.1
const char* INPUT_BLOB_NAME = "data";
const char* OUTPUT_BLOB_NAME = "prob";
static Logger gLogger;

static float data[BATCH_SIZE * 3 * INPUT_H * INPUT_W];
static float prob[BATCH_SIZE * OUTPUT_SIZE];

static int get_width(int x, float gw, int divisor = 8) {
    return int(ceil((x * gw) / divisor)) * divisor;
}

static int get_depth(int x, float gd) {
    if (x == 1) return 1;
    int r = round(x * gd);
    if (x * gd - int(x * gd) == 0.5 && (int(x * gd) % 2) == 0) {
        --r;
    }
    return std::max<int>(r, 1);
}

struct Object{
    cv::Rect rec;
    float prob;
    int label;
};

class YoLoObjectDetection
{
    public:
        YoLoObjectDetection(const std::string _model_path);
        std::vector<cv::Rect> detectObject(const cv::Mat& _frame);
        std::vector<Object> detectObjectv2(const cv::Mat& _frame);
        
    private:
        std::vector<sl::uint2> cvt(const cv::Rect &bbox_in);
        void doInference(IExecutionContext& context, cudaStream_t& stream, void **buffers, float* input, float* output, int batchSize);

    private:
        IRuntime* runtime;
        ICudaEngine* engine;
        IExecutionContext* context;
        cudaStream_t stream;
        void* buffers[2];

        cv::Mat left_cv_rgb;
};

#endif

