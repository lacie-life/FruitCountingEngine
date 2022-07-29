#ifndef LOCAL_TRACKER_H
#define LOCAL_TRACKER_H

#include "defines.h"
#include "track.h"

#include <cstring> // for memset
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/tracking.hpp>

#include <vpi/OpenCVInterop.hpp>
#include <vpi/Array.h>
#include <vpi/Image.h>
#include <vpi/Status.h>
#include <vpi/Stream.h>
#include <vpi/algo/KLTFeatureTracker.h>

// ------------------------------------------------------------------------------------
// Tracking only founded regions between two frames (now used LK optical flow with VPI)
// ------------------------------------------------------------------------------------

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

class LocalTracker
{
public:
    LocalTracker();
    ~LocalTracker(void);

    cv::Mat preprocessImage(cv::Mat _frame);

    void Update(tracks_t& tracks, cv::UMat prevFrame, cv::UMat currFrame);
    void VPIUpdate(tracks_t& tracks, cv::UMat prevFrame, cv::UMat currFrame);


private:

    cv::Mat last_frame_;
    cv::Mat vpiCvTemplate_;

    // Arrays that will store our input bboxes and predicted transform.
    VPIArray inputBoxList = NULL;
    VPIArray inputPredList = NULL;

    VPIBackend backend;

    // Other VPI objects that will be used
    VPIStream stream         = NULL;
    VPIArray outputBoxList   = NULL;
    VPIArray outputEstimList = NULL;
    VPIPayload klt           = NULL;
    VPIImage imgReference    = NULL;
    VPIImage imgTemplate     = NULL;

    // Stores how many bboxes there are in each frame. Only
    // stores when the bboxes count change.
    std::map<int, size_t> bboxes_size_at_frame; // frame -> bbox count
};

#endif // LOCAL_TRACKER_H
