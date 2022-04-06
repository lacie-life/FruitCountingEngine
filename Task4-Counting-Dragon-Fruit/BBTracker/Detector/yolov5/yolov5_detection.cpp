#include "yolov5_detection.h"

ObjectDetection::ObjectDetection(const std::string _model_path)
{   
    // deserialize the .engine and run inference
    std::ifstream file(_model_path, std::ios::binary);
    if (!file.good()) {
        std::cerr << "read " << _model_path << " error!" << std::endl;
        exit(0);
    }
    char *trtModelStream = nullptr;
    size_t size = 0;
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();

    runtime = createInferRuntime(gLogger);
    assert(runtime != nullptr);
    
    engine = runtime->deserializeCudaEngine(trtModelStream, size);
    assert(engine != nullptr);
    
    context = engine->createExecutionContext();
    assert(context != nullptr);

    delete[] trtModelStream;

    assert(engine->getNbBindings() == 2);

    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()
    const int inputIndex = engine->getBindingIndex(INPUT_BLOB_NAME);
    const int outputIndex = engine->getBindingIndex(OUTPUT_BLOB_NAME);
    assert(inputIndex == 0);
    assert(outputIndex == 1);

    // Create GPU buffers on device
    CUDA_CHECK(cudaMalloc(&buffers[inputIndex], BATCH_SIZE * 3 * INPUT_H * INPUT_W * sizeof (float)));
    CUDA_CHECK(cudaMalloc(&buffers[outputIndex], BATCH_SIZE * OUTPUT_SIZE * sizeof (float)));

    CUDA_CHECK(cudaStreamCreate(&stream));

    assert(BATCH_SIZE == 1); // This sample only support batch 1 for now
}

std::vector<sl::uint2> ObjectDetection::cvt(const cv::Rect &bbox_in)
{
    std::vector<sl::uint2> bbox_out(4);
    bbox_out[0] = sl::uint2(bbox_in.x, bbox_in.y);
    bbox_out[1] = sl::uint2(bbox_in.x + bbox_in.width, bbox_in.y);
    bbox_out[2] = sl::uint2(bbox_in.x + bbox_in.width, bbox_in.y + bbox_in.height);
    bbox_out[3] = sl::uint2(bbox_in.x, bbox_in.y + bbox_in.height);
    return bbox_out;
}

void ObjectDetection::doInference(IExecutionContext& context, cudaStream_t& stream, void **buffers, float* input, float* output, int batchSize) {
    // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
    CUDA_CHECK(cudaMemcpyAsync(buffers[0], input, batchSize * 3 * INPUT_H * INPUT_W * sizeof (float), cudaMemcpyHostToDevice, stream));
    context.enqueue(batchSize, buffers, stream, nullptr);
    CUDA_CHECK(cudaMemcpyAsync(output, buffers[1], batchSize * OUTPUT_SIZE * sizeof (float), cudaMemcpyDeviceToHost, stream));
    cudaStreamSynchronize(stream);
}

std::vector<cv::Rect> ObjectDetection::detectObject(const cv::Mat& _frame)
{
    std::vector<cv::Rect> boxes;

    // _frame is input from ZED camera => RGBA format
    cv::cvtColor(_frame, left_cv_rgb, cv::COLOR_BGRA2BGR);

    cv::Mat pr_img = preprocess_img(left_cv_rgb, INPUT_W, INPUT_H); 

    int i = 0;
    int batch = 0;
    for (int row = 0; row < INPUT_H; ++row) {
        uchar* uc_pixel = pr_img.data + row * pr_img.step;
        for (int col = 0; col < INPUT_W; ++col) {
            data[batch * 3 * INPUT_H * INPUT_W + i] = (float) uc_pixel[2] / 255.0;
            data[batch * 3 * INPUT_H * INPUT_W + i + INPUT_H * INPUT_W] = (float) uc_pixel[1] / 255.0;
            data[batch * 3 * INPUT_H * INPUT_W + i + 2 * INPUT_H * INPUT_W] = (float) uc_pixel[0] / 255.0;
            uc_pixel += 3;
            ++i;
        }
    }

    // Running inference
    doInference(*context, stream, buffers, data, prob, BATCH_SIZE);
    std::vector<std::vector < Yolo::Detection >> batch_res(BATCH_SIZE);
    auto& res = batch_res[batch];
    nms(res, &prob[batch * OUTPUT_SIZE], CONF_THRESH, NMS_THRESH);

    std::vector<Object> objects;
    for (auto &it : res)
    {
        Object object;
        object.rec = get_rect(left_cv_rgb, it.bbox);
        object.prob = it.conf;
        object.label = it.class_id;

        objects.push_back(object);

        // Threshold
        if(object.prob > 0.2)
        {
            boxes.push_back(object.rec);
        }
    }

    return boxes;
}
