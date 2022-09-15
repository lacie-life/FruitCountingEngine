#include "LocalTracker.h"

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
LocalTracker::LocalTracker()
{

    // backend = VPI_BACKEND_PVA;

    // CHECK_STATUS(vpiInitKLTFeatureTrackerParams(&params));
    // params.numberOfIterationsScaling  = 20;
    // params.nccThresholdUpdate         = 0.8f;
    // params.nccThresholdKill           = 0.6f;
    // params.nccThresholdStop           = 1.0f;
    // params.maxScaleChange             = 0.2f;
    // params.maxTranslationChange       = 1.5f;
    // params.trackingType               = VPI_KLT_INVERSE_COMPOSITIONAL;
}

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
LocalTracker::~LocalTracker(void)
{
}

// cv::Mat LocalTracker::preprocessImage(cv::Mat _frame)
// {
//     // We only support grayscale inputs
//     if (_frame.channels() == 3)
//     {
//         cvtColor(_frame, _frame, cv::COLOR_BGR2GRAY);
//     }

//     if (backend == VPI_BACKEND_PVA)
//     {
//         // PVA only supports 16-bit unsigned inputs,
//         // where each element is in 0-255 range, so
//         // no rescaling needed.
//         cv::Mat aux;
//         _frame.convertTo(aux, CV_16U);
//         _frame = aux;
//     }
//     else
//     {
//         assert(_frame.type() == CV_8U);
//     }

//     return _frame;
// }

// ---------------------------------------------------------------------------
//
// ---------------------------------------------------------------------------
// void LocalTracker::VPIUpdate(
//         tracks_t& tracks,
//         cv::UMat _prevFrame,
//         cv::UMat _currFrame
//         )
// {
//     // convert track rectangles to VPIKLTTrackedBoundingBox
//     std::vector<VPIKLTTrackedBoundingBox> bboxes;
//     int32_t bboxesSize = 0;
//     std::vector<VPIHomographyTransform2D> preds;
//     int32_t predsSize = 0;

//     // PVA requires that array capacity is 128.
//     bboxes.reserve(128);
//     preds.reserve(128);

//     for (auto& track : tracks)
//     {
//         cv::Rect rect = track->m_lastRegion.m_rect;
//         VPIKLTTrackedBoundingBox track = {};

//         // scale
//         track.bbox.xform.mat3[0][0] = 1;
//         track.bbox.xform.mat3[1][1] = 1;
//         // position
//         track.bbox.xform.mat3[0][2] = rect.x;
//         track.bbox.xform.mat3[1][2] = rect.y;
//         // must be 1
//         track.bbox.xform.mat3[2][2] = 1;
  
//         track.bbox.width     = rect.w;
//         track.bbox.height    = rect.h;
//         track.trackingStatus = 0; // valid tracking
//         track.templateStatus = 1; // must update
  
//         bboxes.push_back(track);
  
//         // Identity predicted transform.
//         VPIHomographyTransform2D xform = {};
//         xform.mat3[0][0]               = 1;
//         xform.mat3[1][1]               = 1;
//         xform.mat3[2][2]               = 1;
//         preds.push_back(xform);
  
//         bboxes_size_at_frame[frame] = bboxes.size();
//     }

//     // Wrap the input arrays into VPIArray's
//     VPIArrayData data           = {};
//     data.bufferType             = VPI_ARRAY_BUFFER_HOST_AOS;
//     data.buffer.aos.type        = VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX;
//     data.buffer.aos.capacity    = bboxes.capacity();
//     data.buffer.aos.sizePointer = &bboxesSize;
//     data.buffer.aos.data        = &bboxes[0];
//     CHECK_STATUS(vpiArrayCreateWrapper(&data, 0, &inputBoxList));
  
//     data.buffer.aos.type        = VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D;
//     data.buffer.aos.sizePointer = &predsSize;
//     data.buffer.aos.data        = &preds[0];
//     CHECK_STATUS(vpiArrayCreateWrapper(&data, 0, &inputPredList));

//     // Create the stream for the given backend.
//     CHECK_STATUS(vpiStreamCreate(backend, &stream));

//     cv::Mat tempPrevFrame = _prevFrame.getMat(cv::ACCESS_READ);
//     cv::Mat tempCurrFrame = _currFrame.getMat(cv::ACCESS_READ);

//     prevFrame = preprocessImage(tempPrevFrame);
//     CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(prevFrame, 0, &imgTemplate));

//     currFrame = preprocessImage(tempCurrFrame);
//     CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(currFrame, 0, &imgReference));

//     CHECK_STATUS(vpiCreateKLTFeatureTracker(backend, prevFrame.cols, prevFrame.rows, imgFormat, NULL, &klt));

//      // Output array with estimated bbox for current frame.
//     CHECK_STATUS(vpiArrayCreate(128, VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX, 0, &outputBoxList));
  
//     // Output array with estimated transform of input bbox to match output bbox.
//     CHECK_STATUS(vpiArrayCreate(128, VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D, 0, &outputEstimList));
// }

void LocalTracker::Update(
        tracks_t& tracks,
        cv::UMat prevFrame,
        cv::UMat currFrame
        )
{
    // get last region point
    std::vector<cv::Point2f> points[2];

    points[0].reserve(8 * tracks.size());
    for (auto& track : tracks)
    {
        for (const auto& pt : track->m_lastRegion.m_points)
        {
            points[0].push_back(pt);
        }
    }
    if (points[0].empty())
    {
        return;
    }



    cv::TermCriteria termcrit(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);
    cv::Size subPixWinSize(3, 3);
    cv::Size winSize(21, 21);

    cv::cornerSubPix(prevFrame, points[0], subPixWinSize, cv::Size(-1,-1), termcrit);

    std::vector<uchar> status;
    std::vector<float> err;

    cv::calcOpticalFlowPyrLK(prevFrame, currFrame, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);

    size_t i = 0;
    for (auto& track : tracks)
    {
        track->m_averagePoint = Point_t(0, 0);
        track->m_boundidgRect = cv::Rect(0, 0, 0, 0);

        for (auto it = track->m_lastRegion.m_points.begin(); it != track->m_lastRegion.m_points.end();)
        {
            if (status[i])
            {
                *it = points[1][i];
                track->m_averagePoint += *it;

                ++it;
            }
            else
            {
                it = track->m_lastRegion.m_points.erase(it);
            }

            ++i;
        }

        if (!track->m_lastRegion.m_points.empty())
        {
            track->m_averagePoint /= static_cast<track_t>(track->m_lastRegion.m_points.size());

            cv::Rect br = cv::boundingRect(track->m_lastRegion.m_points);
#if 0
			br.x -= subPixWinSize.width;
			br.width += 2 * subPixWinSize.width;
			if (br.x < 0)
			{
				br.width += br.x;
				br.x = 0;
			}
            if (br.x + br.width >= currFrame.cols)
			{
                br.x = currFrame.cols - br.width - 1;
			}

			br.y -= subPixWinSize.height;
			br.height += 2 * subPixWinSize.height;
			if (br.y < 0)
			{
				br.height += br.y;
				br.y = 0;
			}
            if (br.y + br.height >= currFrame.rows)
			{
                br.y = currFrame.rows - br.height - 1;
			}
#endif
            track->m_boundidgRect = br;
        }
    }
}
