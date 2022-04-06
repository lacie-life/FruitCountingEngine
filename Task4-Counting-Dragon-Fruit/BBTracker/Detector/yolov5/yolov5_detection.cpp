#include "object_detection.h"
#include "net.h"

static ncnn::Net mobilenet;

ObjectDetection::ObjectDetection(const std::string _model_path)
{   
    // deserialize the .engine and run inference
    std::ifstream file(engine_name, std::ios::binary);
    if (!file.good()) {
        std::cerr << "read " << engine_name << " error!" << std::endl;
        return -1;
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

    
}

std::vector<cv::Rect> ObjectDetection::detectObject(const cv::Mat& _frame)
{
    std::vector<cv::Rect> boxes;
    
    int img_h = _frame.size().height;
    int img_w = _frame.size().width;

    int input_size = 300;
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(_frame.data, ncnn::Mat::PIXEL_BGR, _frame.cols, _frame.rows, input_size, input_size);

    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
    const float norm_vals[3] = {1.0/127.5,1.0/127.5,1.0/127.5};
    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Mat out;

    ncnn::Extractor ex = mobilenet.create_extractor();
    ex.set_light_mode(true);
    //ex.set_num_threads(8);
    ex.input("data", in);
    ex.extract("detection_out",out);

    std::vector<Object> objects;
    for (int iw=0;iw<out.h;iw++)
    {
        Object object;
        const float *values = out.row(iw);
        //object.class_id = values[0];
        object.prob = values[1];
        object.rec.x = values[2] * img_w;
        object.rec.y = values[3] * img_h;
        object.rec.width = values[4] * img_w - object.rec.x;
        object.rec.height = values[5] * img_h - object.rec.y;
        objects.push_back(object);
    
        // Threshold
        if(object.prob > 0.2)
        {
            boxes.push_back(object.rec);
        }
    }

    return boxes;
}
