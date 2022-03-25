#include <iostream>

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/cvconfig.h>
#include <signal.h>

#include "detectNet.h"

const int nmsMaxOut = 300;

// stuff we know about the network and the caffe input/output blobs
static const int INPUT_C = 3;
static const int INPUT_H = 300;
static const int INPUT_W = 300;
static const int IM_INFO_SIZE = 3;
static const int OUTPUT_CLS_SIZE = 5;
static const int OUTPUT_BBOX_SIZE = OUTPUT_CLS_SIZE * 4;

const std::string CLASSES[OUTPUT_CLS_SIZE]{ "background", "snake fruit", "dragon fruit", "banana", "pineapple"};

const char* INPUT_BLOB_NAME0 = "data";
const char* INPUT_BLOB_NAME1 = "im_info";
const char* OUTPUT_BLOB_NAME0 = "bbox_pred";
const char* OUTPUT_BLOB_NAME1 = "cls_prob";
const char* OUTPUT_BLOB_NAME2 = "rois";

using namespace sl;
cv::Mat slMat2cvMat(Mat& input);
cv::cuda::GpuMat slMat2cvMatGPU(Mat& input);

#ifdef HEADLESS
	#define IS_HEADLESS() "headless"	// run without display
#else
	#define IS_HEADLESS() (const char*)NULL
#endif

bool signal_recieved = false;

void sig_handler(int signo)
{
	if( signo == SIGINT )
	{
		LogVerbose("received SIGINT\n");
		signal_recieved = true;
	}
}

/**
* This function displays help in console
**/
int printHelp() {
    printf("usage: detectnet [--help] [--network=NETWORK] [--threshold=THRESHOLD] ...\n");
	printf("                 input_URI [output_URI]\n\n");
	printf("Locate objects in a video/image stream using an object detection DNN.\n");
	printf("See below for additional arguments that may not be shown above.\n\n");
	printf("positional arguments:\n");
	printf("    input_URI       resource URI of input stream  (see videoSource below)\n");
	printf("    output_URI      resource URI of output stream (see videoOutput below)\n\n");

	printf("%s", detectNet::Usage());
    printf("%s", Log::Usage());

	return 0;
}

int main(int argc, char** argv)
{
    commandLine cmdLine(argc, argv, IS_HEADLESS());

    if( cmdLine.GetFlag("help") )
		return printHelp();

	/*
	 * attach signal handler
	 */
	if( signal(SIGINT, sig_handler) == SIG_ERR )
		LogError("can't catch SIGINT\n");

    // Create a ZED camera object
    Camera zed;

    // Set configuration parameters
    InitParameters init_params;
    init_params.camera_resolution = RESOLUTION::HD1080;
    init_params.depth_mode = DEPTH_MODE::ULTRA;
    init_params.coordinate_units = UNIT::METER;
        
    // Open the camera
    ERROR_CODE err = zed.open(init_params);
    if (err != ERROR_CODE::SUCCESS) {
        printf("%s\n", toString(err).c_str());
        zed.close();
        return 1; // Quit if an error occurred
    }

    // Set runtime parameters after opening the camera
    RuntimeParameters runtime_parameters;
    runtime_parameters.sensing_mode = SENSING_MODE::STANDARD;

    // Prepare new image size to retrieve half-resolution images
    Resolution image_size = zed.getCameraInformation().camera_resolution;
    int new_width = image_size.width / 2;
    int new_height = image_size.height / 2;

    Resolution new_image_size(new_width, new_height);

    // To share data between sl::Mat and cv::Mat, use slMat2cvMat()
    // Only the headers and pointer to the sl::Mat are copied, not the data itself
    Mat image_zed(new_width, new_height, MAT_TYPE::U8_C4);
    cv::Mat image_ocv = slMat2cvMat(image_zed);
    // cv::cuda::GpuMat image_cuda = slMat2cvMatGPU(image_zed);

    // TODO: Convert to GPU memory => run detectnet
    /*
	 * create detection network
	 */
	detectNet* net = detectNet::Create(cmdLine);

    const int N = 1;
	
	if( !net )
	{
		LogError("detectnet:  failed to load detectNet model\n");
		return 0;
	}

	// parse overlay flags
	const uint32_t overlayFlags = detectNet::OverlayFlagsFromStr(cmdLine.GetString("overlay", "box,labels,conf"));


    float* data = new float[N*INPUT_C*INPUT_H*INPUT_W];
	// pixel mean used by the Faster R-CNN's author
	float pixelMean[3]{ 102.9801f, 115.9465f, 122.7717f };  // also in BGR order

    // Loop until 'q' is pressed
    char key = ' ';

    while (key != 'q') {

        if (zed.grab(runtime_parameters) == ERROR_CODE::SUCCESS) {

            // Retrieve the left image, depth image in half-resolution
            zed.retrieveImage(image_zed, VIEW::LEFT, MEM::CPU, new_image_size);

            if (image_ocv.empty())
			{
				cv::waitKey(1);
				break;
			}

            cv::Size size(INPUT_W,INPUT_H);	//the dst image size,e.g.100x100
			cv::Mat image_ocv_scaled;	
			cv::resize(image_ocv,image_ocv_scaled,size);	//resize image from src->dst

            cv::Mat_<cv::Vec3f>::iterator it;
			unsigned volChl = INPUT_H*INPUT_W;

			for (int c = 0; c < INPUT_C; ++c)                              
			{
				cv::Mat_<cv::Vec3b>::iterator it = image_ocv_scaled.begin<cv::Vec3b>();	//cv::Vec3f not working - reason still unknown...
				// the color image to input should be in BGR order
				for (unsigned j = 0; j < volChl; ++j)
				{
					//OpenCV read in image_ocv as BGR format, by default, thus need only deduct the mean value
					data[c*volChl + j] = float((*it)[c]) - pixelMean[c];
					it++;
				}
			}

            // detect objects in the frame
		    detectNet::Detection* detections = NULL;

            const int numDetections = net->Detect(data, image_ocv_scaled.height(), image_ocv_scaled.height(), &detections, overlayFlags);
            
		
		    if( numDetections > 0 )
		    {
			    LogVerbose("%i objects detected\n", numDetections);
		
			    for( int n=0; n < numDetections; n++ )
			    {
				    LogVerbose("detected obj %i  class #%u (%s)  confidence=%f\n", n, detections[n].ClassID, net->GetClassDesc(detections[n].ClassID), detections[n].Confidence);
				    LogVerbose("bounding box %i  (%f, %f)  (%f, %f)  w=%f  h=%f\n", n, detections[n].Left, detections[n].Top, detections[n].Right, detections[n].Bottom, detections[n].Width(), detections[n].Height()); 
			    }
		    }

            // Display image and depth using cv:Mat which share sl:Mat data
            cv::imshow("Image", image_ocv_scaled);

            // Handle key event
            key = cv::waitKey(10);
        }
    }

    zed.close();
    return 0;
}

// Mapping between MAT_TYPE and CV_TYPE
int getOCVtype(sl::MAT_TYPE type) {
    int cv_type = -1;
    switch (type) {
        case MAT_TYPE::F32_C1: cv_type = CV_32FC1; break;
        case MAT_TYPE::F32_C2: cv_type = CV_32FC2; break;
        case MAT_TYPE::F32_C3: cv_type = CV_32FC3; break;
        case MAT_TYPE::F32_C4: cv_type = CV_32FC4; break;
        case MAT_TYPE::U8_C1: cv_type = CV_8UC1; break;
        case MAT_TYPE::U8_C2: cv_type = CV_8UC2; break;
        case MAT_TYPE::U8_C3: cv_type = CV_8UC3; break;
        case MAT_TYPE::U8_C4: cv_type = CV_8UC4; break;
        default: break;
    }
    return cv_type;
}

/**
* Conversion function between sl::Mat and cv::Mat
**/
cv::Mat slMat2cvMat(Mat& input) {
    // Since cv::Mat data requires a uchar* pointer, we get the uchar1 pointer from sl::Mat (getPtr<T>())
    // cv::Mat and sl::Mat will share a single memory structure
    return cv::Mat(input.getHeight(), input.getWidth(), getOCVtype(input.getDataType()), input.getPtr<sl::uchar1>(MEM::CPU), input.getStepBytes(sl::MEM::CPU));
}

/**
* Conversion function between sl::Mat and cv::Mat
**/
cv::cuda::GpuMat slMat2cvMatGPU(Mat& input) {
    // Since cv::Mat data requires a uchar* pointer, we get the uchar1 pointer from sl::Mat (getPtr<T>())
    // cv::Mat and sl::Mat will share a single memory structure
    return cv::cuda::GpuMat(input.getHeight(), input.getWidth(), getOCVtype(input.getDataType()), input.getPtr<sl::uchar1>(MEM::GPU), input.getStepBytes(sl::MEM::GPU));
}

