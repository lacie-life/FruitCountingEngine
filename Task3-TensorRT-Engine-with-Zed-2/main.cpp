#include <iostream>

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/cvconfig.h>
#include <signal.h>

#include "detectNet.h"

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
void printHelp() {
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


    Mat depth_image_zed(new_width, new_height, MAT_TYPE::U8_C4);
    cv::Mat depth_image_ocv = slMat2cvMat(depth_image_zed);

    Mat point_cloud;

    // TODO: Convert to GPU memory => run detectnet

    // Loop until 'q' is pressed
    char key = ' ';
    while (key != 'q') {

        if (zed.grab(runtime_parameters) == ERROR_CODE::SUCCESS) {

            // Retrieve the left image, depth image in half-resolution
            zed.retrieveImage(image_zed, VIEW::LEFT, MEM::CPU, new_image_size);

            // retrieve CPU -> the ocv reference is therefore updated
            zed.retrieveImage(depth_image_zed, VIEW::DEPTH, MEM::CPU, new_image_size);

            // Retrieve the RGBA point cloud in half-resolution
            // To learn how to manipulate and display point clouds, see Depth Sensing sample
            zed.retrieveMeasure(point_cloud, MEASURE::XYZRGBA, MEM::CPU, new_image_size);

            // Display image and depth using cv:Mat which share sl:Mat data
            cv::imshow("Image", image_ocv);

            // download the Ocv GPU data from Device to Host to be displayed
            depth_image_ocv_gpu.download(depth_image_ocv);

            cv::imshow("Depth", depth_image_ocv);

            // Handle key event
            key = cv::waitKey(10);
        }
    }

    // sl::Mat GPU memory needs to be free before the zed
    depth_image_zed_gpu.free();

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

