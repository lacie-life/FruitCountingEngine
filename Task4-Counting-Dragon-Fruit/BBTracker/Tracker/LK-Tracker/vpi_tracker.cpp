#include "vpi_tracker.h"

VPITracker::VPITracker(cv::Mat _frame, std::vector<cv::Rect> _rois)
    : ids_(0),
{
    // Convert cv::Rect vector to VPIBoundingBox vector
    int i = 0;
    for (i = 0; i < _rois.size(); i++)
    {
        // Convert the axis-aligned bounding box into our tracking
        // structure.

        VPIKLTTrackedBoundingBox track = {};
        // scale
        track.bbox.xform.mat3[0][0] = 1;
        track.bbox.xform.mat3[1][1] = 1;
        // position
        track.bbox.xform.mat3[0][2] = _rois[i].x;
        track.bbox.xform.mat3[1][2] = _rois[i].y;
        // must be 1
        track.bbox.xform.mat3[2][2] = 1;

        track.bbox.width = _rois[i].width;
        track.bbox.height = _rois[i].height;
        track.trackingStatus = 0; // valid tracking
        track.templateStatus = 1; // must update

        bboxes.push_back(track);

        // Identity predicted transform.
        VPIHomographyTransform2D xform = {};
        xform.mat3[0][0] = 1;
        xform.mat3[1][1] = 1;
        xform.mat3[2][2] = 1;
        preds.push_back(xform);

        bboxes_size_at_frame[frame] = bboxes.size();
    }

    _frame.copyTo(last_frame_);

    // Allocate memory and config KLTFeatureTracker

    backend = VPI_BACKEND_PVA;
    // Create the stream for the given backend.
    CHECK_STATUS(vpiStreamCreate(backend, &stream));

    cv::Mat frame = preprocessImage(_frame.clone());

    // Convention between OpenCV image and VPI Image
    CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(frame, 0, &imgTemplate));

    // Create the reference image wrapper. Let's wrap the cvTemplate for now just
    // to create the wrapper. Later we'll set it to wrap the actual reference image.
    CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(frame, 0, &imgReference));

    VPIImageFormat imgFormat;
    CHECK_STATUS(vpiImageGetFormat(imgTemplate, &imgFormat));

    // Using this first frame's characteristics, create a KLT Bounding Box Tracker payload.
    // We're limiting the template dimensions to 64x64.
    CHECK_STATUS(vpiCreateKLTFeatureTracker(backend, frame.cols, frame.rows, imgFormat, NULL, &klt));

    // Parameters we'll use. No need to change them on the fly, so just define them here.
    VPIKLTFeatureTrackerParams params;
    CHECK_STATUS(vpiInitKLTFeatureTrackerParams(&params));

    // Output array with estimated bbox for current frame.
    CHECK_STATUS(vpiArrayCreate(128, VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX, 0, &outputBoxList));

    // Output array with estimated transform of input bbox to match output bbox.
    CHECK_STATUS(vpiArrayCreate(128, VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D, 0, &outputEstimList));
}

VPITracker::~VPITracker()
{
    vpiStreamDestroy(stream);
    vpiPayloadDestroy(klt);
    vpiArrayDestroy(inputBoxList);
    vpiArrayDestroy(inputPredList);
    vpiArrayDestroy(outputBoxList);
    vpiArrayDestroy(outputEstimList);
    vpiImageDestroy(imgReference);
    vpiImageDestroy(imgTemplate);
}

cv::Mat VPITracker::preprocessImage(cv::Mat &_frame)
{
    // Convert image to correct format (grayscale)
    if (_frame.channels() == 3)
    {
        cv::cvtColor(_frame, _frame, cv::COLOR_BGR2GRAY);
    }

    if (backend == VPI_BACKEND_PVA)
    {
        // PVA only supports 16-bit unsigned inputs,
        // where each element is in 0-255 range, so
        // no rescaling needed.
        cv::Mat aux;
        _frame.convertTo(aux, CV_16U);
        _frame = aux;
    }
    else
    {
        assert(_frame.type() == CV_8U);
    }

    return _frame;
}

void VPITracker::updateTrackersWithNewFrame(const cv::Mat &_frame)
{
    // Update all bounding box tracked
    cv::Mat frame = _frame.clone();

    // Wrap the input arrays into VPIArray's
    VPIArrayData data = {};
    data.bufferType = VPI_ARRAY_BUFFER_HOST_AOS;
    data.buffer.aos.type = VPI_ARRAY_TYPE_KLT_TRACKED_BOUNDING_BOX;
    data.buffer.aos.capacity = bboxes.capacity();
    data.buffer.aos.sizePointer = &bboxesSize;
    data.buffer.aos.data = &bboxes[0];
    CHECK_STATUS(vpiArrayCreateWrapper(&data, 0, &inputBoxList));

    data.buffer.aos.type = VPI_ARRAY_TYPE_HOMOGRAPHY_TRANSFORM_2D;
    data.buffer.aos.sizePointer = &predsSize;
    data.buffer.aos.data = &preds[0];
    CHECK_STATUS(vpiArrayCreateWrapper(&data, 0, &inputPredList));

    // TODO Line 378 https://docs.nvidia.com/vpi/sample_klt_tracker.html
    // What happend after recieved new frame ?
    //

    // Make the reference wrapper point to the reference frame
    CHECK_STATUS(vpiImageSetWrappedOpenCVMat(imgReference, cvReference));

    // Estimate the bounding boxes in current frame (reference) given their position in previous
    // frame (template).
    CHECK_STATUS(vpiSubmitKLTFeatureTracker(stream, backend, klt, imgTemplate, inputBoxList, inputPredList,
                                            imgReference, outputBoxList, outputEstimList, &params));

    // Wait for processing to finish.
    CHECK_STATUS(vpiStreamSync(stream));

    // ============================ Update ==================================== //
    // Now the input and output arrays are locked to properly set up the input for the next iteration.
    // Input arrays will be updated based on tracking information produced in this iteration.
    VPIArrayData updatedBBoxData;
    CHECK_STATUS(vpiArrayLockData(outputBoxList, VPI_LOCK_READ, VPI_ARRAY_BUFFER_HOST_AOS, &updatedBBoxData));

    VPIArrayData estimData;
    CHECK_STATUS(vpiArrayLockData(outputEstimList, VPI_LOCK_READ, VPI_ARRAY_BUFFER_HOST_AOS, &estimData));

    // Since these arrays are actually wrappers of external data, we don't need to retrieve
    // the VPI array contents, the wrapped buffers will be updated directly. The arrays must
    // be locked for read/write anyway.
    CHECK_STATUS(vpiArrayLock(inputBoxList, VPI_LOCK_READ_WRITE));
    CHECK_STATUS(vpiArrayLock(inputPredList, VPI_LOCK_READ_WRITE));

    auto *updated_bbox = reinterpret_cast<VPIKLTTrackedBoundingBox *>(updatedBBoxData.buffer.aos.data);
    auto *estim = reinterpret_cast<VPIHomographyTransform2D *>(estimData.buffer.aos.data);

    // TODO: Need remove bad track
    for (size_t b = 0; b < bboxes.size(); b++)
    {
        // Did tracking failed?
        if (updated_bbox[b].trackingStatus)
        {
            // Do we have to update the input bbox's tracking status too?
            if (bboxes[b].trackingStatus == 0)
            {
                // std::cout << curFrame << " -> dropped " << b << std::endl;
                bboxes[b].trackingStatus = 1;
            }

            continue;
        }

        // Must update template for this bounding box??
        if (updated_bbox[b].templateStatus)
        {
            // std::cout << curFrame << " -> update " << b << std::endl;

            // There are usually two approaches here:
            // 1. Redefine the bounding box using a feature detector such as
            //    \ref algo_harris_corners "Harris keypoint detector", or
            // 2. Use updated_bbox[b], which is still valid, although tracking
            //    errors might accumulate over time.
            //
            // We'll go to the second option, less robust, but simple enough
            // to implement.
            bboxes[b] = updated_bbox[b];

            // Signal the input that the template for this bounding box must be updated.
            bboxes[b].templateStatus = 1;

            // Predicted transform is now identity as we reset the tracking.
            preds[b] = VPIHomographyTransform2D{};
            preds[b].mat3[0][0] = 1;
            preds[b].mat3[1][1] = 1;
            preds[b].mat3[2][2] = 1;
        }
        else
        {
            // Inform that the template for this bounding box doesn't need to be pdated.
            bboxes[b].templateStatus = 0;

            // We just update the input transform with the estimated one.
            preds[b] = estim[b];
        }
    }

    // We're finished working with the input and output arrays.
    CHECK_STATUS(vpiArrayUnlock(inputBoxList));
    CHECK_STATUS(vpiArrayUnlock(inputPredList));
  
    CHECK_STATUS(vpiArrayUnlock(outputBoxList));
    CHECK_STATUS(vpiArrayUnlock(outputEstimList));

    // Next's reference frame is current's template.
    // TODO: Check variable
    std::swap(imgTemplate, imgReference);
    std::swap(cvTemplate, cvReference);
}

bool VPITracker::updateTrackersWithNewDetectionResults(const std::vector<cv::Rect>& _dets)
{
    // What is here ????????????
    // matching tracker with detection results
    int box_num = bboxes.size();
    int dets_num = _dets.size();

    
}

float VPITracker::getIOU(const cv::Rect _rec1, const cv::Rect _rec2)
{
    auto max = [](int a, int b){return a>b?a:b;};
    auto min = [](int a, int b){return a<b?a:b;};

    int xA = max(_rec1.x, _rec2.x);
    int yA = max(_rec1.y, _rec2.y);
    int xB = min(_rec1.x+_rec1.width, _rec2.x+_rec2.width);
    int yB = max(_rec1.y+_rec1.height, _rec2.y+_rec2.height);
    int interArea = 0;
    if(xB <= xA || yB <= yA)
    {
        interArea = 0;
    }
    else
    {
        interArea = (xB - xA +1)*(yB - yA +1);
    }
    int boxAArea = (_rec1.width+1)*(_rec1.height+1);
    int boxBArea = (_rec2.width+1)*(_rec2.height+1);
    float iou = float(interArea) / float(boxAArea + boxBArea - interArea);

    return iou;
}

int VPITracker::getMatchingScore(const cv::Rect _rec1, const cv::Rect _rec2)
{
    // score = (1 - iou) * dx/width * dy/height * 100
    float iou = getIOU(_rec1, _rec2);

    int x1 = _rec1.x + _rec1.width/2;
    int x2 = _rec2.x + _rec2.width/2;
    int y1 = _rec1.y + _rec1.height/2;
    int y2 = _rec2.y + _rec2.height/2;
    auto abs = [](float x){return x>0?x:-x;};
    float dx = abs(static_cast<float>(x2-x1));
    float dy = abs(static_cast<float>(y2-y1));

    int score = static_cast<int>((1.0-iou) * dx * dy * 100.0 / (_rec1.width * _rec1.height));

    //std::cout << "rec1" << std::endl << _rec1 << std::endl;
    //std::cout << "rec2" << std::endl << _rec2 << std::endl;
    //std::cout << "score:" << score << std::endl;
    
    if(score == 0)
    {
        score = 1;
    }
    return score;
}

