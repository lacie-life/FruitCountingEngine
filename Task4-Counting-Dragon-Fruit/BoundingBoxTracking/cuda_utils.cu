#include "cuda_utils.cuh"
#include <iostream>

__global__ void _cuda_feature2bbox(VPIKeypoint *kpts, VPIKLTTrackedBoundingBox *bboxes,
                                   VPIHomographyTransform2D *preds, uint32_t size)
{
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    if( index >= size ) return;

    VPIKeypoint kpt = kpts[index];
    VPIKLTTrackedBoundingBox *track = &bboxes[index];
    VPIHomographyTransform2D *xform = &preds[index];

    // bbox
    memset(track, 0, sizeof(VPIKLTTrackedBoundingBox));
    track->bbox.xform.mat3[0][0] = 1;
    track->bbox.xform.mat3[1][1] = 1;
    track->bbox.xform.mat3[0][2] = float(kpt.x) - 15.5f;
    track->bbox.xform.mat3[1][2] = float(kpt.y) - 15.5f;
    track->bbox.xform.mat3[2][2] = 1;

    track->bbox.width     = 32.f;
    track->bbox.height    = 32.f;
    track->trackingStatus = 0;
    track->templateStatus = 1;

    // pred
    memset(xform, 0, sizeof(VPIHomographyTransform2D));
    xform->mat3[0][0] = 1;
    xform->mat3[1][1] = 1;
    xform->mat3[2][2] = 1;
}

int cuda_feature2bbox(cudaStream_t &stream, void *kpts, void *input_box, void *input_pred, uint32_t size)
{
    _cuda_feature2bbox<<< (size+THREAD-1)/THREAD, THREAD, 0, stream >>>( (VPIKeypoint*)kpts, (VPIKLTTrackedBoundingBox*)input_box,
                                                                         (VPIHomographyTransform2D*)input_pred, size);
    cudaStreamSynchronize(stream);
    return 0;
}
