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

#include "pipeline.h"
#include "detectNet.h"
#include "videoSource.h"
#include "videoOutput.h"
#include "Ctracker.h"


class BBTracker : public pipeline{
public:
    BBTracker(commandLine cmdLine, const cv::CommandLineParser &parser);

private:
    detectNet* net;
    uint32_t overlayFlags;

    std::string meanFile;
    std::string meanValue;

    int line1_x1;
    int line1_x2;
    int line1_y1;
    int line1_y2;
    int line2_x1;
    int line2_x2;
    int line2_y1;
    int line2_y2;

protected:
    std::vector<vector<float>> detectFrame(cv::Mat frame);
    void DrawData(cv::Mat frame, int framesCounter, double fontScale);
    void DrawCounter(cv::Mat frame, double fontScale, std::map <string,  int> &countObjects_LefttoRight, std::map <string,  int> &countObjects_RighttoLeft);
    void CounterUpdater(cv::Mat frame, std::map <string,  int> &countObjects_LefttoRight, std::map <string,  int> &countObjects_RighttoLeft);

};


#endif // BBTRACKER_H
