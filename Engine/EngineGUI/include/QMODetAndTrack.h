#ifndef QMODETANDTRACK_H
#define QMODETANDTRACK_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/face.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/opencv.hpp>

#include "yolov5_detection.h"
#include "MO-Tracker/defines.h"
#include "MO-Tracker/Ctracker.h"

class QMODetAndTrack : public QObject
{
    Q_OBJECT

public:
    explicit QMODetAndTrack(QObject *parent = nullptr);
    ~QMODetAndTrack();

    void Process();

    void DrawTrack(cv::Mat frame,
                   int resizeCoeff,
                   const CTrack& track,
                   bool drawTrajectory = true,
                   bool isStatic = false);

    double CalculateRelativeSize(int frame_width, int frame_height);

    std::vector<std::vector<float>> detectframe(cv::Mat frame);
    std::vector<Object> detectframev2(cv::Mat frame);
    void DrawData(cv::Mat frame, int framesCounter, double fontScale);
    void CounterUpdater(cv::Mat frame,
                        std::map<std::string, int> &countObjects_LefttoRight,
                        std::map<std::string, int> &countObjects_RighttoLeft);
    void DrawCounter(cv::Mat frame,
                     double fontScale,
                     std::map <std::string, int> &countObjects_LefttoRight,
                     std::map <std::string, int> &countObjects_RighttoLeft);

signals:

private:
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

    std::string modelFile;
    YoLoObjectDetection* detector;

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

};

#endif // QMODETANDTRACK_H

