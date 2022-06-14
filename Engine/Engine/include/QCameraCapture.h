#ifndef QCAMERACAPTURE_H
#define QCAMERACAPTURE_H

#include <QObject>
#include <QPixmap>
#include <QDebug>
#include <QMutex>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QWaitCondition>
#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <algorithm>

Q_DECLARE_METATYPE(cv::Mat)

class QCameraCapture : public QObject
{
    Q_OBJECT

public:
    explicit QCameraCapture(QObject *parent = nullptr);
    ~QCameraCapture();

    bool initCamera();

signals:
    void frameReady(cv::Mat frame);
    void frameUIReady(QPixmap pixmap);

public slots:
    void stream();
    void stop();

private:
    cv::Mat slMat2cvMat(sl::Mat& input);
    int getOpenCVType(sl::MAT_TYPE type);

private:
    bool stopped;
    sl::Camera m_camera;
    cv::Mat image_ocv;
    cv::Mat depth_image_ocv;

};

#endif // QCAMERACAPTURE_H
