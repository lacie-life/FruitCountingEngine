/**
 * @file
 * 
 * @author      Noah van der Meer
 * @brief       Object detection with YOLOv5, TensorRT and Stereolabs ZED
 * 
 * 
 * Note: this example is based on the process_live example from the
 *   yolov5-tensorrt library, see: https://github.com/noahmr/yolov5-tensorrt
 * 
 * For the most up-to-date version of this ZED-YOLOv5 example, see:
 * https://github.com/noahmr/zed-yolov5
 * 
 * 
 * 
 * Copyright (c) 2021, Noah van der Meer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * 
 */

/*  yolov5-tensorrt library */
#include <yolov5_detector.hpp>

/*  opencv  */
#include <opencv2/highgui.hpp>

/*  ZED */
#include <sl/Camera.hpp>

char* getCmdOption(char** begin, char** end, const std::string& option)
{
    /*  From https://stackoverflow.com/questions/865668/parsing-
        command-line-arguments-in-c */
    char** itr = std::find(begin, end, option);
    if(itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option, 
                    bool value = false)
{
    /*  From https://stackoverflow.com/questions/865668/parsing-
        command-line-arguments-in-c */
    char** itr = std::find(begin, end, option);
    if(itr == end)
    {
        return false;
    }
    if(value && itr == end-1)
    {
        std::cout << "Warning: option '" << option << "'"
                << " requires a value" << std::endl;
        return false;
    }
    return true;
}

void printHelp()
{
    std::cout << "Options:\n"
                "-h --help :       show this help menu\n"
                "--engine :        [mandatory] Engine to be used for "
                    "inference\n"
                "--classes :       [optional] specify list of class names\n"
                "--gui :           [optional] display results visually\n"
                "--svo :           [optional] load a ZED SVO instead of "
                    "connecting to a live sensor\n"
                "Example usage:\n"
                "detect --engine yolov5s.engine --classes coco_names.txt "
                    "--gui" << std::endl;
}

int main(int argc, char* argv[])
{
    /*  
        Handle arguments
    */
    if(cmdOptionExists(argv, argv+argc, "--help") || 
        cmdOptionExists(argv, argv+argc, "-h"))
    {
        printHelp();
        return 0;
    }

    if(!cmdOptionExists(argv, argv+argc, "--engine", true))
    {
        std::cout << "Missing mandatory argument" << std::endl;
        printHelp();
        return 1;
    }
    const std::string engineFile(getCmdOption(argv, argv+argc, "--engine"));
  
    std::string classesFile;
    if(cmdOptionExists(argv, argv+argc, "--classes", true))
    {
        classesFile = getCmdOption(argv, argv+argc, "--classes");
    }

    bool gui = false;
    if(cmdOptionExists(argv, argv+argc, "--gui", false))
    {
        gui = true;
    }

    std::string svo;
    if(cmdOptionExists(argv, argv+argc, "--svo", true))
    {
        svo = getCmdOption(argv, argv+argc, "--svo");
    }


    /*
        Create the YoloV5 Detector object.
    */
    yolov5::Detector detector;


    /*
        Initialize the YoloV5 Detector. This should be done first, before
        loading the engine.
    */
    yolov5::Result r = detector.init();
    if(r != yolov5::RESULT_SUCCESS)
    {
        std::cout << "init() failed: " << yolov5::result_to_string(r) 
                    << std::endl;
        return 1;
    }


    /*
        Load the engine from file.
    */
    r = detector.loadEngine(engineFile);
    if(r != yolov5::RESULT_SUCCESS)
    {
        std::cout << "loadEngine() failed: " << yolov5::result_to_string(r)
                    << std::endl;
        return 1;
    }
    

    /*
        Load the Class names from file, and pass these on to the Detector
    */
    if(classesFile.length() > 0)
    {
        yolov5::Classes classes;
        classes.setLogger(detector.logger());
        r = classes.loadFromFile(classesFile);
        if(r != yolov5::RESULT_SUCCESS)
        {
            std::cout << "classes.loadFromFile() failed: " 
                    << yolov5::result_to_string(r) << std::endl;
            return 1;
        }
        detector.setClasses(classes);
    }


    if(gui)
    {
        try
        {
            cv::namedWindow("color");
        }
        catch(const std::exception& e)
        {
            std::cout << "CV namedWindow() exception: " << e.what() << std::endl;
            return 1;
        }
    }


    /*
        Both the ZED SDK and yolov5-tensorrt library are using CUDA
        internally. Although this is just my speculation (since the SDK is
        closed-source), it appears that ZED SDK will create its
        own CUDA context and push it to the context stack.
        
        On the other hand, the yolov5-tensorrt and OpenCV libraries simply
        use the context bound to the calling CPU thread. This poses a problem
        for this code, since the Detector is initialized and loaded first, then
        the ZED Camera is opened, and then detection is performed again.

        See for more details:
        https://docs.nvidia.com/cuda/cuda-driver-api/group__CUDA__CTX.html


        In this case, there are two possible solutions to avoid problems:
        - First open the ZED camera, and after this initialize and load the
            detector, perform detection tasks etc.
        - First start with the detector, and before opening the ZED camera,
            indicate that it should use the same CUDA context, for instance
            obtained through cuCtxGetCurrent().


        The first solution is fairly simple to implement. I have opted to use
        the 2nd solution here to demonstrate how it would work.
    */
    CUcontext cuContext;
    if(cuCtxGetCurrent(&cuContext) != 0)
    {
        std::cout << "Could not get current CUDA context" << std::endl;
        return 1;
    }
    CUdevice cuDevice;
    if(cuCtxGetDevice(&cuDevice) != 0)
    {
        std::cout << "Could not obtain CUdevice from CUDA context" << std::endl;
        return 1;
    }


    /*
        Set up the ZED camera
    */
    sl::InitParameters initParameters;
    initParameters.sdk_verbose = true;
    initParameters.depth_mode = sl::DEPTH_MODE::PERFORMANCE;
    /*  note: if you are loading from an SVO, you might need to change the
            resolution  */
    initParameters.camera_resolution = sl::RESOLUTION::HD720;
    initParameters.sdk_cuda_ctx = cuContext;
    initParameters.sdk_gpu_id = cuDevice;

    if(svo.length() > 0)
    {
        initParameters.input.setFromSVOFile(sl::String(svo.c_str()));
        initParameters.svo_real_time_mode = true;
    }


    sl::Camera zed;
    sl::ERROR_CODE error = zed.open(initParameters);
    if(error != sl::ERROR_CODE::SUCCESS)
    {
		std::cout << "Error opening camera: " << sl::toVerbose(error) << std::endl;
        return 1;
	}

    sl::RuntimeParameters runtimeParameters;

    /*  Set up the Mat in which the frames are to be placed. An OpenCV Mat
        is set up to share memory with the SL mat   */
    sl::Mat imageSl(sl::getResolution(initParameters.camera_resolution), 
                        sl::MAT_TYPE::U8_C4, sl::MEM::CPU);
    cv::Mat imageCv(cv::Size(imageSl.getWidth(), imageSl.getHeight()),
                        CV_8UC4, imageSl.getPtr<sl::uchar1>(sl::MEM::CPU));
    cv::Mat imageBgr;

    while(true)
    {
        /*
            Grab & retrieve image from ZED
        */
		error = zed.grab(runtimeParameters);
        if(error != sl::ERROR_CODE::SUCCESS)
        {
            std::cout << "Error grabbing frames: " << sl::toVerbose(error) << std::endl;
            continue;
        }
        zed.retrieveImage(imageSl, sl::VIEW::LEFT);


        /*  ZED outputs BGRA, while the yolov5-tensorrt library takes
            either BGR or RGB   */
        cv::cvtColor(imageCv, imageBgr, cv::COLOR_BGRA2BGR);

        /*
            Perform object detection through YOLOv5
        */
        std::vector<yolov5::Detection> detections;
        r = detector.detect(imageBgr, &detections, yolov5::INPUT_BGR);
        if(r != yolov5::RESULT_SUCCESS)
        {
            std::cout << "yolov5 detect() failed: " << yolov5::result_to_string(r) 
                        << std::endl;
            continue;
        }

        /*
            Visualize the objects
        */
        for(const yolov5::Detection& object : detections)
        {
            const cv::Scalar magenta(255, 51, 153); /*  BGR */
            yolov5::visualizeDetection(object, &imageBgr, magenta, 1.0);
        }

        if(gui)
        {
            cv::imshow("color", imageBgr);
            const int key = cv::waitKey(1);
            if(key == 'e')
            {
                break;
            }
        }
    }
    zed.close();

    if(gui)
    {
        cv::destroyAllWindows();
    }
    return 0;
}