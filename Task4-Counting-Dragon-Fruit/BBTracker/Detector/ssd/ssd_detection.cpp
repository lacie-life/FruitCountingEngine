#include "ssd_detection.h"

#include <algorithm>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>

using namespace std;

float clamp(float x)
{
  return std::max(std::min(x, 1.f), 0.f);
}

std::vector<std::vector<float>> generate_ssd_priors()
{
  // SSD specifications as feature map size, shrinkage, box min, box max
  float specs[6][4] = {{19, 16, 60, 105},
                       {10, 32, 105, 150},
                       {5, 64, 150, 195},
                       {3, 100, 195, 240},
                       {2, 150, 240, 285},
                       {1, 300, 285, 330}};
  float aspect_ratios[2] = {2, 3};
  float image_size = 300;
  float x_center, y_center, scale, h, w, size, ratio;
  std::vector<std::vector<float>> priors;

  for (size_t i = 0; i < 6; i++)
  {
    scale = image_size / specs[i][1];
    for (size_t j = 0; j < specs[i][0]; j++)
    {
      for (size_t k = 0; k < specs[i][0]; k++)
      {
        x_center = clamp((j + 0.5) / scale);
        y_center = clamp((k + 0.5) / scale);

        // small sized square box
        w = clamp(specs[i][2] / image_size);
        h = w;
        std::vector<float> v1 = {x_center, y_center, w, h};
        priors.push_back(v1);

        // big sized square box
        size = sqrt(specs[i][3] * specs[i][2]);
        w = clamp(size / image_size);
        h = w;
        std::vector<float> v2 = {x_center, y_center, w, h};
        priors.push_back(v2);

        // change h/w ratio of the small sized box
        w = specs[i][2] / image_size;
        h = w;
        for (float rt : aspect_ratios)
        {
          ratio = sqrt(rt);
          std::vector<float> v3 = {x_center, y_center, clamp(w * ratio), clamp(h / ratio)};
          priors.push_back(v3);
          std::vector<float> v4 = {x_center, y_center, clamp(w / ratio), clamp(h * ratio)};
          priors.push_back(v4);
        }
      }
    }
  }

  return priors;
}

std::vector<float> convert_locations_to_boxes(std::vector<float> prior, float *location)
{
  float center_variance = 0.1;
  float size_variance = 0.2;

  float bx_cx, bx_cy, bx_h, bx_w;
  float bx_x1, bx_y1, bx_x2, bx_y2;

  // x_center, y_center, w, h
  bx_cx = location[0] * center_variance * prior[2] + prior[0];
  bx_cy = location[1] * center_variance * prior[3] + prior[1];
  bx_w = exp(location[2] * size_variance) * prior[2];
  bx_h = exp(location[3] * size_variance) * prior[3];

  // x1, y1, x2, y2
  bx_x1 = bx_cx - bx_w / 2.0;
  bx_y1 = bx_cy - bx_h / 2.0;
  bx_x2 = bx_cx + bx_w / 2.0;
  bx_y2 = bx_cy + bx_h / 2.0;
  std::vector<float> box = {bx_x1, bx_y1, bx_x2, bx_y2};
  return box;
}

/* Post processing script borrowed from ../yolo5/common.h model */
float iou(std::vector<float> lbox, std::vector<float> rbox)
{
    float interBox[] = {
      std::max(lbox[0], rbox[0]), //left
      std::min(lbox[2], rbox[2]), //right
      std::max(lbox[1], rbox[1]), //top
      std::min(lbox[3], rbox[3]), //bottom
    };

  if (interBox[2] > interBox[3] || interBox[0] > interBox[1])
    return 0.0f;

  float interBoxS = (interBox[1] - interBox[0]) * (interBox[3] - interBox[2]);
  return interBoxS / ((lbox[2] - lbox[0]) * (lbox[3] - lbox[1]) + (rbox[2] - rbox[0]) * (rbox[3] - rbox[1]) - interBoxS);
}

std::vector<SSDObject> nms(std::map<float, std::vector<SSDObject>> m, float nms_thresh)
{
  // NMS on single image of NUM_DETECTIONS detections
  std::vector<SSDObject> res;
  for (auto it = m.begin(); it != m.end(); it++)
  {
    //std::cout << it->second[0].class_id << " --- " << std::endl;
    auto &dets = it->second;
    std::sort(dets.begin(), dets.end(), cmp);
    for (size_t m = 0; m < dets.size(); ++m)
    {
      auto &item = dets[m];
      res.push_back(item);
      for (size_t n = m + 1; n < dets.size(); ++n)
      {
        if (iou(item.bbox, dets[n].bbox) > nms_thresh)
        {
          dets.erase(dets.begin() + n);
          --n;
        }
      }
    }
  }
  return res;
}

SSDObjectDetection::SSDObjectDetection(const std::string _model_path)
{   
    // create a model using the API directly and serialize it to a stream
    char *trtModelStream{nullptr};
    size_t size{0};

    std::ifstream file(_model_path, std::ios::binary);
    if (file.good())
    {
      file.seekg(0, file.end);
      size = file.tellg();
      file.seekg(0, file.beg);
      trtModelStream = new char[size];
      assert(trtModelStream);
      file.read(trtModelStream, size);
      file.close();
      std::cout << "Engine file read successful" << std::endl;
    }

    runtime = createInferRuntime(gLogger);
    assert(runtime != nullptr);
    engine = runtime->deserializeCudaEngine(trtModelStream, size, nullptr);
    assert(engine != nullptr);
    context = engine->createExecutionContext();
    assert(context != nullptr);
    delete[] trtModelStream;

}

void SSDObjectDetection::doInference(IExecutionContext &context, float *input, float *output_cnf, float *output_bx, int batchSize)
{
    const ICudaEngine &engine = context.getEngine();

    // Pointers to input and output device buffers to pass to engine.
    // Engine requires exactly IEngine::getNbBindings() number of buffers.
    assert(engine.getNbBindings() == 3);
    void *buffers[3];

    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()
    const int inputIndex = engine.getBindingIndex(INPUT_BLOB_NAME);
    const int outputIndexCnf = engine.getBindingIndex(OUTPUT_BLOB_NAME_CNF);
    const int outputIndexBx = engine.getBindingIndex(OUTPUT_BLOB_NAME_BX);

    // Create GPU buffers on device
    CHECK(cudaMalloc(&buffers[inputIndex], batchSize * ssd::INPUT_C * ssd::INPUT_H * ssd::INPUT_W * sizeof(float)));
    CHECK(cudaMalloc(&buffers[outputIndexCnf], batchSize * OUTPUT_SIZE_CNF * sizeof(float)));
    CHECK(cudaMalloc(&buffers[outputIndexBx], batchSize * OUTPUT_SIZE_BX * sizeof(float)));

    // Create stream
    cudaStream_t stream;
    CHECK(cudaStreamCreate(&stream));

    // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
    CHECK(cudaMemcpyAsync(buffers[inputIndex], input, batchSize * ssd::INPUT_C * ssd::INPUT_H * ssd::INPUT_W * sizeof(float), cudaMemcpyHostToDevice, stream));
    context.enqueue(batchSize, buffers, stream, nullptr);
    CHECK(cudaMemcpyAsync(output_cnf, buffers[outputIndexCnf], batchSize * OUTPUT_SIZE_CNF * sizeof(float), cudaMemcpyDeviceToHost, stream));
    CHECK(cudaMemcpyAsync(output_bx, buffers[outputIndexBx], batchSize * OUTPUT_SIZE_BX * sizeof(float), cudaMemcpyDeviceToHost, stream));
    cudaStreamSynchronize(stream);

    // Release stream and buffers
    cudaStreamDestroy(stream);
    CHECK(cudaFree(buffers[inputIndex]));
    CHECK(cudaFree(buffers[outputIndexCnf]));
}

std::vector<SSDObject> SSDObjectDetection::detectObject(const cv::Mat& _frame)
{
    cv::Mat pr_img = _frame.clone();
    cv::resize(pr_img, pr_img, cv::Size(SSD::INPUT_H, SSD::INPUT_W));

    for (int i = 0; i < SSD::INPUT_H * SSD::INPUT_W; i++)
    {
      data[i] = (pr_img.at<cv::Vec3b>(i)[2] - 127) / 128.0;
      data[i + SSD::INPUT_H * SSD::INPUT_W] = (pr_img.at<cv::Vec3b>(i)[1] - 127) / 128.0;
      data[i + 2 * SSD::INPUT_H * SSD::INPUT_W] = (pr_img.at<cv::Vec3b>(i)[0] - 127) / 128.0;
    }

    auto start = std::chrono::system_clock::now();
    doInference(*context, data, prob, locations, 1);
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    std::vector<SSDObject> dt = post_process_output(prob, locations, BBOX_CONF_THRESH, NMS_THRESH);

    return dt;
}

std::vector<SSDObject> SSDObjectDetection::post_process_output(float *prob, float *locations, float conf_thresh, float nms_thresh)
{
  // Process the detections on a single image
  std::vector<std::vector<float>> priors = generate_ssd_priors();
  std::map<float, std::vector<SSDObject>> m;
  float class_id;
  float *conf;

  // map from class_id : detections
  for (int i = 0; i < SSD::NUM_DETECTIONS; i++)
  {
    conf = std::max_element(prob + i * SSD::NUM_CLASSES, prob + (i + 1) * SSD::NUM_CLASSES);
    class_id = std::distance(prob + i * SSD::NUM_CLASSES, conf);
    if (*conf <= conf_thresh)
      continue;
    if (class_id == 0.0)
      continue;
    std::vector<float> box = convert_locations_to_boxes(priors[i], locations + i * 4);
    SSDObject det = {box, class_id, *conf};
    // if (m.count(det.class_id) == 0) m.emplace(det.class_id, std::vector<ssd::Detection>());
    m[det.class_id].push_back(det);
  }

  return nms(m, nms_thresh);
}
