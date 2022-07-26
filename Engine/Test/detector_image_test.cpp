#include "yolov5_detection.h"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <chrono>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;


int main( int argc, char** argv )
{
    std::string folderPath = argv[1];
    std::vector<std::string> imageList;

    boost::filesystem::path p(folderPath + "/img");

    for (auto i = directory_iterator(p); i != directory_iterator(); i++)
    {
        if (!is_directory(i->path())) //we eliminate directories
        {
            std::cout << i->path().filename().string() << std::endl;
            imageList.emplace_back( i->path().filename().string());
        }
        else
            continue;
    }

    std::cout << imageList.size() << std::endl;

    cv::Mat frame;

    YoLoObjectDetection det("/home/jun/Github/FruitCountingEngine/Engine/build/Data/model/test-11.engine");

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for (int i = 0; i < imageList.size(); i++)
    {
        frame = cv::imread(folderPath + "/img/" + imageList[i]);
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        auto objects = det.detectObjectv2(frame);
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        std::chrono::duration<double> time_used = std::chrono::duration_cast<std::chrono::duration<double>>(t2-t1);
        std::cout<<"Time: " << time_used.count() << "seconds" << std::endl;

        for(auto object:objects)
        {
            cv::rectangle(frame, object.rec, cv::Scalar(0, 255, 0), 2, 1);
            cv::putText(frame, std::to_string((int) object.label), cv::Point(object.rec.x, object.rec.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255, 0, 0), 2);
        }

        cv::imshow("detection", frame);
        cv::imwrite(folderPath + "/results/" + imageList[i], frame);
        if(cv::waitKey(1) == 27)
        {
            break;
        }
    }
    std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
    std::chrono::duration<double> all_time = std::chrono::duration_cast<std::chrono::duration<double>>(stop-start);
    std::cout<<"Time: " << all_time.count() << "seconds" << std::endl;

    return 0;
}
