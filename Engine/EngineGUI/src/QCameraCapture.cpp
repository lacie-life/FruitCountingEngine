#include "QCameraCapture.h"

#include <QDebug>
#include <QString>

QCameraCapture::QCameraCapture(QObject *parent)
    : QObject{parent}
{
}

QCameraCapture::~QCameraCapture()
{
    m_camera.close();
}

bool QCameraCapture::initCamera()
{
    if(m_camera.isOpened()){
        m_camera.close();
    }

    sl::InitParameters init_params;
    init_params.camera_resolution = sl::RESOLUTION::HD720;
    init_params.depth_mode = sl::DEPTH_MODE::ULTRA;
    init_params.coordinate_units = sl::UNIT::METER;

    // Open the camera
    sl::ERROR_CODE err = m_camera.open(init_params);
    if (err != sl::ERROR_CODE::SUCCESS) {
        CONSOLE << QString(toString(err));
        m_camera.close();
        return false; // Quit if an error occurred
    }

    return true;
}

void QCameraCapture::stream()
{
    // Set runtime parameters after opening the camera
    sl::RuntimeParameters runtime_parameters;
    runtime_parameters.sensing_mode = sl::SENSING_MODE::STANDARD;

    // Prepare new image size to retrieve half-resolution images
    sl::Resolution image_size = m_camera.getCameraInformation().camera_configuration.resolution;
    int new_width = image_size.width / 2;
    int new_height = image_size.height / 2;

    sl::Resolution new_image_size(new_width, new_height);

    // To share data between sl::Mat and cv::Mat, use slMat2cvMat()
    // Only the headers and pointer to the sl::Mat are copied, not the data itself
    sl::Mat image_zed(new_width, new_height, sl::MAT_TYPE::U8_C4);

//    image_ocv = slMat2cvMat(image_zed);

    sl::Mat depth_image_zed(new_width, new_height, sl::MAT_TYPE::U8_C4);
    depth_image_ocv = slMat2cvMat(depth_image_zed);

    stopped = false;

    while(!stopped)
    {
        if (m_camera.grab(runtime_parameters) == sl::ERROR_CODE::SUCCESS) {

            // Retrieve the left image, depth image in half-resolution
            m_camera.retrieveImage(image_zed, sl::VIEW::LEFT, sl::MEM::CPU, new_image_size);
            cv::Mat img_rgba = slMat2cvMat(image_zed);
            cv::cvtColor(img_rgba, image_ocv, cv::COLOR_BGRA2BGR);

            // retrieve CPU -> the ocv reference is therefore updated
            m_camera.retrieveImage(depth_image_zed, sl::VIEW::DEPTH, sl::MEM::CPU, new_image_size);

            // Display image and depth using cv:Mat which share sl:Mat data
//            cv::imshow("Image", image_ocv);
//            cv::imshow("Depth", depth_image_ocv);

            QPixmap img = QPixmap::fromImage(QImage((uchar*)image_ocv.data,
                                                    image_ocv.cols,
                                                    image_ocv.rows,
                                                    static_cast<int>(image_ocv.step),
                                                    QImage::Format_RGB888).rgbSwapped());

            emit frameReady(image_ocv);
            emit frameUIReady(img);
        }
    }

    image_zed.free();
    depth_image_zed.free();
}

void QCameraCapture::stop()
{
    stopped = true;

    m_camera.close();

    cv::waitKey(100);
}

cv::Mat QCameraCapture::slMat2cvMat(sl::Mat &input)
{
    return cv::Mat(input.getHeight(),
                   input.getWidth(),
                   getOpenCVType(input.getDataType()),
                   input.getPtr<sl::uchar1>(sl::MEM::CPU),
                   input.getStepBytes(sl::MEM::CPU));
}

int QCameraCapture::getOpenCVType(sl::MAT_TYPE type)
{
    int cv_type = -1;
    switch (type) {
        case sl::MAT_TYPE::F32_C1: cv_type = CV_32FC1; break;
        case sl::MAT_TYPE::F32_C2: cv_type = CV_32FC2; break;
        case sl::MAT_TYPE::F32_C3: cv_type = CV_32FC3; break;
        case sl::MAT_TYPE::F32_C4: cv_type = CV_32FC4; break;
        case sl::MAT_TYPE::U8_C1: cv_type = CV_8UC1; break;
        case sl::MAT_TYPE::U8_C2: cv_type = CV_8UC2; break;
        case sl::MAT_TYPE::U8_C3: cv_type = CV_8UC3; break;
        case sl::MAT_TYPE::U8_C4: cv_type = CV_8UC4; break;
        default: break;
    }
    return cv_type;
}
