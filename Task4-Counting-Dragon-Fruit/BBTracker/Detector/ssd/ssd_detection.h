#ifndef SSD_DETECTION_H
#define SSD_DETECTION_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <dirent.h>

#include "NvInfer.h"
#include "cuda_runtime_api.h"
#include "logging.h"


namespace SSD
{
    static const int INPUT_H = 300;
    static const int INPUT_W = 300;
    static const int INPUT_C = 3;

    static const int LOCATIONS = 4;
    static const int NUM_DETECTIONS = 3000;
    static const int NUM_CLASSES = 21;
}

#define CHECK(status)                                    \
  do                                                     \
  {                                                      \
    auto ret = (status);                                 \
    if (ret != 0)                                        \
    {                                                    \
      std::cerr << "Cuda failure: " << ret << std::endl; \
      abort();                                           \
    }                                                    \
  } while (0)


#define USE_FP16  // comment out this if want to use FP32
#define DEVICE 0 // GPU id
#define NMS_THRESH 0.45
#define BBOX_CONF_THRESH 0.5

using namespace nvinfer1;

// stuff we know about the network and the input/output blobs
static const int OUTPUT_SIZE_CNF = SSD::NUM_CLASSES * SSD::NUM_DETECTIONS;
static const int OUTPUT_SIZE_BX = SSD::LOCATIONS * SSD::NUM_DETECTIONS;
const char *INPUT_BLOB_NAME = "data";
const char *OUTPUT_BLOB_NAME_CNF = "confidences";
const char *OUTPUT_BLOB_NAME_BX = "locations";
static Logger gLogger;

static float data[SSD::INPUT_C * SSD::INPUT_H * SSD::INPUT_W];
static float prob[OUTPUT_SIZE_CNF], locations[OUTPUT_SIZE_BX];

struct SSDObject{
    std::vector<float> bbox; // x1 y1 x2 y2
    float class_id;          // 0 background
    float conf;              // classification confidence
};


class SSDObjectDetection
{
    public:
        SSDObjectDetection(const std::string _model_path);
        std::vector<cv::Rect> detectObject(const cv::Mat& _frame);

    private:
        void doInference(IExecutionContext &context, float *input, float *output_cnf, float *output_bx, int batchSize);
        std::vector<SSDObject> post_process_output(float *prob, float *locations, float conf_thresh, float nms_thresh);
        
    private:
        IRuntime* runtime;
        ICudaEngine* engine;
        IExecutionContext* context;
};

#endif
