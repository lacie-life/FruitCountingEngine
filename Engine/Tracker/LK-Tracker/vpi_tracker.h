#ifndef LK_TRACKER_H
#define LK_TRACKER_H

#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <stdexcept>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>

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

#include "munkres.h"

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


class VPITracker{
    public:
        VPITracker(const cv::Mat& _frame, const cv::Rect& _bbox, const int _tracker_id, const bool _use_kf);
        void updateVPITracker(const cv::Mat& _frame);
        cv::Rect getBbox()
        {
            return bbox_;
        }
        cv::Scalar getColor()
        {
            return color_;   
        }
        std::list<cv::Point2f> getTrackPoints()
        {
            return track_points_;
        }
        bool getStatus()
        {
            return status_;
        }
        
        // kalman filter stuff
        void KalmanUpdate(const cv::Rect _new_box);
        void KalmanPredict();

    private:
        static cv::Ptr<cv::FastFeatureDetector> detector_;  
        int tracker_id_;
        cv::Rect bbox_;
        VPIKLTTrackedBoundingBox vpibbox_;
        VPIHomographyTransform2D pred_;
        bool status_; 
        int MIN_TRACK_POINTS_NUM_ = 10;
        int MAX_TRACK_POINTS_NUM_ = 60;
        std::list<cv::Point2f> track_points_;
        std::vector<cv::Point2f> old_track_points_;
        const float SCALE_THRESHOLD = 1.01;
        template <class T>
        T findMedian(std::vector<T> vec);
        int frame_width_;
        int frame_height_;

        int32_t MIN_ACCEPT_FRAMES_;
        int32_t MIN_REJECT_FRAMES_;
        bool accepted_;
        bool rejected_;
        int32_t getting_frames_;
        int32_t missing_frames_;
        
        cv::Scalar color_;

        // kalman filter stuff
        const bool USE_KF_; // if do Kalman filter or not
        cv::KalmanFilter kf_;
        cv::Mat kf_state_; // x, y, xdot, ydot, w, h
        cv::Mat kf_measure_;
        float p_cov_scalar_;
        float m_cov_scalar_;
        bool first_time_;
        double ticks_;

        friend class VPITrackerManager;
};

class VPITrackerManager{
    public:
        VPITrackerManager(cv::Mat _frame, std::vector<cv::Rect> _rois);
        void updateTrackersWithNewFrame(const cv::Mat& _frame);
        bool updateTrackersWithNewDetectionResults(const std::vector<cv::Rect>& _dets);
        std::vector<cv::Rect> getAllBox();
        std::vector<cv::Scalar> getAllColor();
        std::vector<cv::Point2f> getAllPoints();
        cv::Mat preprocessImage(cv::Mat _frame);

    private:
        std::vector<VPITracker*> tracker_ptrs_;
        
        //cv::Mat current_frame_;
        cv::Mat last_frame_;
        cv::Mat vpiCvTemplate_;

        std::vector<cv::Point2f> all_new_points_;
        int ids_;
        float getIOU(const cv::Rect _rec1, const cv::Rect _rec2);
        int getMatchingScore(const cv::Rect _rec1, const cv::Rect _rec2);
        const int COST_THRESHOLD_;
        const bool USE_KF_; // use kalman filter

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

        friend class DetAndTrack;
};

#endif
