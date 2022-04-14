#ifndef VPI_TRACKER_H
#define VPI_TRACKER_H

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <vpi/OpenCVInterop.hpp>

#include <vpi/Array.h>
#include <vpi/Image.h>
#include <vpi/Status.h>
#include <vpi/Stream.h>
#include <vpi/algo/KLTFeatureTracker.h>

#include <cstring> // for memset
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#define CHECK_STATUS(STMT)                                    \
    do                                                        \
    {                                                         \
        VPIStatus status = (STMT);                            \
        if (status != VPI_SUCCESS)                            \
        {                                                     \
            char buffer[VPI_MAX_STATUS_MESSAGE_LENGTH];       \
            vpiGetLastStatusMessage(buffer, sizeof(buffer));  \
            std::ostringstream ss;                            \
            ss << vpiStatusGetName(status) << ": " << buffer; \
            throw std::runtime_error(ss.str());               \
        }                                                     \
    } while (0);

// Utility to draw the bounding boxes into an image and save it to disk.
static cv::Mat WriteKLTBoxes(VPIImage img, VPIArray boxes, VPIArray preds)
{
    // Convert img into a cv::Mat
    cv::Mat out;
    {
        VPIImageData imgdata;
        CHECK_STATUS(vpiImageLock(img, VPI_LOCK_READ, &imgdata));

        int cvtype;
        switch (imgdata.format)
        {
        case VPI_IMAGE_FORMAT_U8:
            cvtype = CV_8U;
            break;

        case VPI_IMAGE_FORMAT_S8:
            cvtype = CV_8S;
            break;

        case VPI_IMAGE_FORMAT_U16:
            cvtype = CV_16UC1;
            break;

        case VPI_IMAGE_FORMAT_S16:
            cvtype = CV_16SC1;
            break;

        default:
            throw std::runtime_error("Image type not supported");
        }

        cv::Mat cvimg(imgdata.planes[0].height, imgdata.planes[0].width, cvtype, imgdata.planes[0].data,
                      imgdata.planes[0].pitchBytes);

        if (cvimg.type() == CV_16U)
        {
            cvimg.convertTo(out, CV_8U);
            cvimg = out;
            out   = cv::Mat();
        }

        cvtColor(cvimg, out, cv::COLOR_GRAY2BGR);

        CHECK_STATUS(vpiImageUnlock(img));
    }

    // Now draw the bounding boxes.
    VPIArrayData boxdata;
    CHECK_STATUS(vpiArrayLock(boxes, VPI_LOCK_READ, &boxdata));

    VPIArrayData preddata;
    CHECK_STATUS(vpiArrayLock(preds, VPI_LOCK_READ, &preddata));

    auto *pboxes = reinterpret_cast<VPIKLTTrackedBoundingBox *>(boxdata.data);
    auto *ppreds = reinterpret_cast<VPIHomographyTransform2D *>(preddata.data);

    // Use random high-saturated colors
    static std::vector<cv::Vec3b> colors;
    if ((int)colors.size() != *boxdata.sizePointer)
    {
        colors.resize(*boxdata.sizePointer);

        cv::RNG rand(1);
        for (size_t i = 0; i < colors.size(); ++i)
        {
            colors[i] = cv::Vec3b(rand.uniform(0, 180), 255, 255);
        }
        cvtColor(colors, colors, cv::COLOR_HSV2BGR);
    }

    // For each tracked bounding box...
    for (int i = 0; i < *boxdata.sizePointer; ++i)
    {
        if (pboxes[i].trackingStatus == 1)
        {
            continue;
        }

        float x, y, w, h;
        x = pboxes[i].bbox.xform.mat3[0][2] + ppreds[i].mat3[0][2];
        y = pboxes[i].bbox.xform.mat3[1][2] + ppreds[i].mat3[1][2];
        w = pboxes[i].bbox.width * pboxes[i].bbox.xform.mat3[0][0] * ppreds[i].mat3[0][0];
        h = pboxes[i].bbox.height * pboxes[i].bbox.xform.mat3[1][1] * ppreds[i].mat3[1][1];

        rectangle(out, cv::Rect(x, y, w, h), cv::Scalar(colors[i][0], colors[i][1], colors[i][2]), 2);
    }

    CHECK_STATUS(vpiArrayUnlock(preds));
    CHECK_STATUS(vpiArrayUnlock(boxes));

    return out;
}

class VPITrackerManager {
public:
    VPITracker(cv::Mat _frame, std::vector<cv::Rect> _rois);
    void updateTrackersWithNewFrame(const cv::Mat& _frame);
    bool updateTrackersWithNewDetectionResults(const std::vector<cv::Rect>& _dets);
    std::vector<cv::Rect> getAllBox();
    std::vector<cv::Scalar> getAllColor();
    std::vector<cv::Point2f> getAllPoints();

private:
    // OpenCV image that will be wrapped by a VPIImage.
    // Define it here so that it's destroyed *after* wrapper is destroyed
    cv::Mat cvTemplate;
    cv::Mat cvReference;

    VPIBackend backend;

    // Arrays that will store our input bboxes and predicted transform.
    VPIArray inputBoxList = NULL; 
    VPIArray inputPredList = NULL;

    // Other VPI objects that will be used
    VPIStream stream         = NULL;
    VPIArray outputBoxList   = NULL;
    VPIArray outputEstimList = NULL;
    VPIPayload klt           = NULL;
    VPIImage imgReference    = NULL;
    VPIImage imgTemplate     = NULL;

    // These arrays will actually wrap these vectors.
    std::vector<VPIKLTTrackedBoundingBox> bboxes;
    int32_t bboxesSize = 0;
    std::vector<VPIHomographyTransform2D> preds;
    int32_t predsSize = 0;

    std::map<int, size_t> bboxes_size_at_frame; // frame -> bbox count
};

#endif