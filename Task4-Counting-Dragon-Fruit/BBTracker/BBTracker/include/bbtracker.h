//
// Created by lacie on 31/03/2022.
//

#ifndef BBTRACKER_H
#define BBTRACKER_H

#include <opencv2/opencv.hpp>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

#include "defines.h"
#include "detectNet.h"
#include "videoSource.h"
#include "videoOutput.h"
#include "Ctracker.h"

using namespace std;
using namespace cv;

DEFINE_string(mean_file, "",
              "The mean file used to subtract from the input image.");

DEFINE_string(mean_value, "104,117,123",
              "If specified, can be one value or can be same as image channels"
              " - would subtract from the corresponding channel). Separated by ','."
              "Either mean_file or mean_value should be provided, not both.");


class BBTracker{
public:
    BBTracker(commandLine cmdLine, const cv::CommandLineParser &parser);

    void Process();

private:
    detectNet* net;
    uint32_t overlayFlags;

    std::string meanFile;
    std::string meanValue;

    bool saveVideo;
    bool drawCount;
    bool drawOther;
    bool useCrop;
    int endFrame;
    int startFrame;
    cv::Rect cropRect;
    bool desiredDetect;
    int cropFrameWidth;
    std::string inFile;
    std::string outFile;
    int cropFrameHeight;
    float detectThreshold;
    std::vector<cv::Scalar> m_colors;
    std::string desiredObjectsString;

    int line1_x1;
    int line1_x2;
    int line1_y1;
    int line1_y2;
    int line2_x1;
    int line2_x2;
    int line2_y1;
    int line2_y2;

protected:
    std::unique_ptr<CTracker> m_tracker;
    float m_fps;
    bool enableCount;
    int direction;

    std::vector<vector<float>> detectFrame(cv::Mat frame);
    void DrawData(cv::Mat frame, int framesCounter, double fontScale);
    void DrawCounter(cv::Mat frame, double fontScale, std::map <string,  int> &countObjects_LefttoRight, std::map <string,  int> &countObjects_RighttoLeft);
    void CounterUpdater(cv::Mat frame, std::map <string,  int> &countObjects_LefttoRight, std::map <string,  int> &countObjects_RighttoLeft);
    void DrawTrack(cv::Mat frame,
                   int resizeCoeff,
                   const CTrack& track,
                   bool drawTrajectory = true,
                   bool isStatic = false);
    double CalculateRelativeSize(int frame_width, int frame_height);

};


#endif // BBTRACKER_H
