#include "QMODetAndTrack.h"
#include "include/common.h"

std::vector<sl::uint2> cvt(const cv::Rect &bbox_in){
    std::vector<sl::uint2> bbox_out(4);
    bbox_out[0] = sl::uint2(bbox_in.x, bbox_in.y);
    bbox_out[1] = sl::uint2(bbox_in.x + bbox_in.width, bbox_in.y);
    bbox_out[2] = sl::uint2(bbox_in.x + bbox_in.width, bbox_in.y + bbox_in.height);
    bbox_out[3] = sl::uint2(bbox_in.x, bbox_in.y + bbox_in.height);
    return bbox_out;
}

QMODetAndTrack::QMODetAndTrack(QObject *parent)
    :  QObject{parent},
     modelLoaded(false)
{
    // Read setting
    QSettings gui_setting("engine_gui", "MODetAndTrack");
    outFile = gui_setting.value("output").toString().toStdString();
    inFile = gui_setting.value("input").toString().toStdString();
    endFrame = gui_setting.value("end_frame").toInt();
    startFrame = gui_setting.value("satrt_frame").toInt();

    m_fps = gui_setting.value("fps").toInt();
    saveVideo = gui_setting.value("save_video").toBool();
    enableCount = gui_setting.value("count").toBool();
    drawCount = gui_setting.value("draw_count").toBool();
    drawOther = gui_setting.value("draw_other").toBool();

    direction = gui_setting.value("direction").toInt();
    useCrop = gui_setting.value("crop").toBool();

    cropFrameWidth = gui_setting.value("crop_width").toInt();
    cropFrameHeight = gui_setting.value("crop_height").toInt();

    cropRect = cv::Rect(gui_setting.value("crop_x").toInt(),
                        gui_setting.value("crop_y").toInt(),
                        cropFrameWidth,
                        cropFrameHeight);

    detectThreshold = gui_setting.value("threshold").toFloat();
    desiredDetect = gui_setting.value("desired_detect").toBool();
    desiredObjectsString = gui_setting.value("desired_objects").toString().toStdString();

    line1_x1 = gui_setting.value("l1p1_x").toInt();
    line1_x2 = gui_setting.value("l1p2_x").toInt();
    line1_y1 = gui_setting.value("l1p1_y").toInt();
    line1_y2 = gui_setting.value("l1p2_y").toInt();
    line2_x1 = gui_setting.value("l2p1_x").toInt();
    line2_x2 = gui_setting.value("l2p2_x").toInt();
    line2_y1 = gui_setting.value("l2p1_y").toInt();
    line2_y2 = gui_setting.value("l2p2_y").toInt();

    modelFile = gui_setting.value("model").toString().toStdString();

    // Initialize the tracker
    config_t config;

    // TODO: put these variables in main
    TrackerSettings settings;
    settings.m_distType = tracking::DistRects;
    settings.m_kalmanType = tracking::KalmanLinear;
    settings.m_filterGoal = tracking::FilterRect;
    settings.m_lostTrackType = tracking::TrackKCF;       // Use KCF tracker for collisions resolving
    settings.m_matchType = tracking::MatchHungrian;      // Matching algorithm
    settings.m_dt = 0.3f;                                // Delta time for Kalman filter
    settings.m_accelNoiseMag = 0.1f;                     // Accel noise magnitude for Kalman filter
    settings.m_distThres = 100;                          // Distance threshold between region and object on two frames
    settings.m_maximumAllowedSkippedFrames = (size_t)(1 * m_fps);  // Maximum allowed skipped frames
    settings.m_maxTraceLength = (size_t)(5 * m_fps);               // Maximum trace length

    m_tracker = std::make_unique<CTracker>(settings);


    // Different color used for path lines in tracking
    // Add more if you are a colorful person.
    m_colors.emplace_back(cv::Scalar(255, 0, 0));
    m_colors.emplace_back(cv::Scalar(0, 255, 0));
    m_colors.emplace_back(cv::Scalar(0, 0, 255));
    m_colors.emplace_back(cv::Scalar(255, 255, 0));
    m_colors.emplace_back(cv::Scalar(0, 255, 255));
    m_colors.emplace_back(cv::Scalar(255, 0, 255));
    m_colors.emplace_back(cv::Scalar(255, 127, 255));
    m_colors.emplace_back(cv::Scalar(127, 0, 255));
    m_colors.emplace_back(cv::Scalar(127, 0, 127));
}

QMODetAndTrack::~QMODetAndTrack()
{
    m_update.stop();
}

void QMODetAndTrack::Process()
{
    // Prepossessing step. May be make a new function to do prepossessing (TODO)
    // Converting desired object into float.
    std::vector <float> desiredObjects;
    std::stringstream ss(desiredObjectsString);
    while( ss.good() )
    {
        std::string substring;
        getline( ss, substring, ',' );
        desiredObjects.push_back( std::stof(substring) );
    }

    std::cout << inFile << std::endl;

    // Set up input
    cv::VideoCapture cap(inFile);

    if (!cap.isOpened()) {
        std::cout << "Failed to open video: " << inFile << std::endl;
    }

    cv::Mat frame;
    int frameCount = 0;

    // video output
    cv::VideoWriter writer;
    auto frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    auto frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    writer.open(outFile, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), m_fps, cv::Size(frame_width, frame_height), true);

    std::cout << "Frame Infor: " << frame_width << " " << frame_height << std::endl;

    std::map <std::string, int> countObjects_LefttoRight;
    std::map <std::string, int> countObjects_RighttoLeft;
    double fontScale = CalculateRelativeSize(1920, 1080);

    double tFrameModification = 0;
    double tDetection = 0;
    double tTracking = 0;
    double tCounting = 0;
    double tDTC = 0;
    double tStart  = cv::getTickCount();

    isRuning = true;

    loadModel();

    // Process one frame at a time
    while (isRuning){

        double tStartFrameModification = cv::getTickCount();

        bool success = cap.read(frame);

        if (!success) {
            std::cout << "Process " << frameCount << " frames from " << inFile << std::endl;
            break;
        }

        if(frameCount < startFrame)
        {
            continue;
        }

        if (frameCount > endFrame)
        {
            std::cout << "Process: reached last " << endFrame << " frame" << std::endl;
            break;
        }

        if(frame.empty())
        {
            std::cout << "Error when read frame" << std::endl;
            // break;
        }

        // Focus on interested area in the frame
        if (useCrop)
        {
            cv::Mat copyFrame(frame, cropRect);
            // Deep copy (TODO)
            //copyFrame.copyTo(frame);
            // Shallow copy
            frame = copyFrame;
        }
        tFrameModification += cv::getTickCount() - tStartFrameModification;

        // Get all the detected objects.
        double tStartDetection = cv::getTickCount();
        regions_t tmpRegions;
        std::vector<Object> detections = detectframev2(frame);

        std::cout << "Number object in frame " << frameCount << "th: " << detections.size() << std::endl;

        // Filter out all the objects based
        // 1. Threshold
        // 2. Desired object classe
        for (auto const& detection : detections){

            const Object &d = detection;
            // Detection format: [score, label, xmin, ymin, xmax, ymax].
            const float score = d.prob;
            const float fLabel= d.label;

            if(desiredDetect)
            {
                if (!(std::find(desiredObjects.begin(), desiredObjects.end(), fLabel) != desiredObjects.end()))
                {
                    continue;
                }
            }

            std::string label;
            if (fLabel == 2.0){
                label = "Ripe";
            }
            else if (fLabel == 0.0){
                label = "Unripe";
            }else{
                label = std::to_string(static_cast<int>(fLabel));
            }

            if (score >= detectThreshold) {

                std::cout << label << std::endl;

                cv::Rect object(d.rec);
                tmpRegions.push_back(CRegion(object, label, score));
            }
        }
        tDetection += cv::getTickCount() - tStartDetection;

        double tStartTracking = cv::getTickCount();
        // Update Tracker
        cv::UMat clFrame;
        clFrame = frame.getUMat(cv::ACCESS_READ);
        m_tracker->Update(tmpRegions, clFrame, m_fps);
        tTracking += cv::getTickCount() - tStartTracking;

        if(enableCount)
        {
            double tStartCounting = cv::getTickCount();
            // Update Counter
            CounterUpdater(frame, countObjects_LefttoRight, countObjects_RighttoLeft);
            tCounting += cv::getTickCount() - tStartCounting;

            if(drawCount){
                DrawCounter(frame, fontScale, countObjects_LefttoRight, countObjects_RighttoLeft);
            }

        }

        if(drawOther){
            DrawData(frame, frameCount, fontScale);
        }

        if (writer.isOpened() && saveVideo)
        {
            writer << frame;
        }

        ++frameCount;

        emit imageResults(frame);
    }
    if (cap.isOpened()) {
        cap.release();
    }

    // Calculate Time for components
    double tEnd  = cv::getTickCount();
    double totalRunTime = (tEnd - tStart)/cv::getTickFrequency();
    double tFrameModificationRuntTime = tFrameModification/cv::getTickFrequency();
    double detectionRunTime = tDetection/cv::getTickFrequency();
    double trackingRunTime = tTracking/cv::getTickFrequency();
    double countingRunTime = tCounting/cv::getTickFrequency();
    double FDTCRuntime = tFrameModificationRuntTime + detectionRunTime + trackingRunTime + countingRunTime;

    // Display and write output
    std::ofstream csvFile;
    csvFile.open ("../D2.csv");
    csvFile << "Frame Modification time" << ",";
    csvFile << "Detection time" << ",";
    csvFile << "Tracking time" << ",";
    csvFile << "Counting time" << ",";
    csvFile << "FDTC time" << ",";
    csvFile << "Total time" << ",";
    csvFile << "FDTC frame rate" << ",";
    csvFile << "Total frame rate" << "\n";
    std::cout  << "Frame Modification time = " << tFrameModificationRuntTime << " seconds" << std::endl;
    csvFile << tFrameModificationRuntTime << ",";
    std::cout  << "Detection time = " << detectionRunTime << " seconds" << std::endl;
    csvFile << detectionRunTime << ",";
    std::cout  << "Tracking time = " << trackingRunTime << " seconds" << std::endl;
    csvFile << trackingRunTime<< ",";
    std::cout  << "Counting time = " << countingRunTime << " seconds" << std::endl;
    csvFile << countingRunTime << ",";
    std::cout  << "FDTC time = " << FDTCRuntime << " seconds " << std::endl;
    csvFile << FDTCRuntime << ",";
    std::cout  << "Total time = " << totalRunTime << " seconds " << std::endl;
    csvFile << totalRunTime << ",";
    std::cout  << " FDTC frame rate: "<< frameCount/FDTCRuntime << " fps" <<std::endl;
    csvFile << frameCount/FDTCRuntime  << ",";
    std::cout  << " Total frame rate: "<< frameCount/totalRunTime << " fps" << std::endl;
    csvFile << frameCount/totalRunTime << "\n";
    std::cout  << "Left to Right or Top to Bottom ";
    csvFile << "Object label" << "," << "count Left to Right" << "\n";
    for(auto elem : countObjects_LefttoRight)
    {
        std::cout << elem.first << " " << elem.second << "\n";
        csvFile << elem.first << "," << elem.second << "\n";
    }
    std::cout  << "Right to Left or Bottom to Top";
    csvFile << "Object label" << "," << "count Right to Left" << "\n";
    for(auto elem : countObjects_RighttoLeft)
    {
        std::cout << elem.first << " " << elem.second << "\n";
        csvFile << elem.first << "," << elem.second << "\n";
    }

    csvFile.close();
}

void QMODetAndTrack::ProcessZED()
{
    // ZED 2 Pipeline
    // TODO: Add QGLViewer
    // Prepossessing step. May be make a new function to do prepossessing (TODO)
    // Converting desired object into float.
    std::vector <float> desiredObjects;
    std::stringstream ss(desiredObjectsString);
    while( ss.good() )
    {
        std::string substring;
        getline( ss, substring, ',' );
        desiredObjects.push_back( std::stof(substring) );
    }

    // Opening the ZED camera before the model deserialization to avoid cuda context issue
    sl::Camera zed;
    sl::InitParameters init_parameters;
    init_parameters.camera_resolution = sl::RESOLUTION::HD1080;
    init_parameters.sdk_verbose = true;
    init_parameters.depth_mode = sl::DEPTH_MODE::ULTRA;
    init_parameters.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP; // OpenGL's coordinate system is right_handed

    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        std::cout << "Camera Open" << returned_state << " \n Exit program. \n";
    }

    zed.enablePositionalTracking();
    // Custom OD
    sl::ObjectDetectionParameters detection_parameters;
    detection_parameters.enable_tracking = true;
    detection_parameters.enable_mask_output = false; // designed to give person pixel mask
    detection_parameters.detection_model = sl::DETECTION_MODEL::CUSTOM_BOX_OBJECTS;
    returned_state = zed.enableObjectDetection(detection_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        std::cout << "enableObjectDetection" << returned_state << " \n Exit program. \n";
        zed.close();
    }
    auto camera_config = zed.getCameraInformation().camera_configuration;
    sl::Resolution pc_resolution(std::min((int) camera_config.resolution.width, 720), std::min((int) camera_config.resolution.height, 404));
    auto camera_info = zed.getCameraInformation(pc_resolution).camera_configuration;

    // Create OpenGL Viewer
    QGLViewer viewer;
    viewer.init(camera_info.calibration_parameters.left_cam, true);
    // ---------

    sl::Mat left_sl, point_cloud;
    cv::Mat left_cv_rgb;
    sl::ObjectDetectionRuntimeParameters objectTracker_parameters_rt;
    sl::Objects objects;
    sl::Pose cam_w_pose;
    cam_w_pose.pose_data.setIdentity();

    int frameCount = 0;

    // video output
    cv::VideoWriter writer;
    auto frame_width = static_cast<int>(std::min((int) camera_config.resolution.width, 1280));
    auto frame_height = static_cast<int>(std::min((int) camera_config.resolution.height, 720));

    writer.open(outFile, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), m_fps, cv::Size(frame_width, frame_height), true);

    std::cout << "Frame Infor: " << frame_width << " " << frame_height << std::endl;

    std::map <std::string,  int> countObjects_LefttoRight;
    std::map <std::string,  int> countObjects_RighttoLeft;
    double fontScale = CalculateRelativeSize(1920, 1080);

    double tFrameModification = 0;
    double tDetection = 0;
    double tTracking = 0;
    double tCounting = 0;
    double tDTC = 0;
    double tStart  = cv::getTickCount();

    detector = new YoLoObjectDetection(modelFile);

    // Process one frame at a time
    while (viewer.isAvailable())
    {
        double tStartFrameModification = cv::getTickCount();

        if(zed.grab() == sl::ERROR_CODE::SUCCESS)
        {
            // TODO: processing
            zed.retrieveImage(left_sl, sl::VIEW::LEFT); 

            // Preparing inference
            cv::Mat left_cv_rgba = slMat2cvMat(left_sl);

            cv::cvtColor(left_cv_rgba, left_cv_rgb, cv::COLOR_BGRA2BGR);
                
            if (left_cv_rgb.empty()) 
            {
                continue;
            }

            // Focus on interested area in the frame
            if (useCrop)
            {
                cv::Mat copyFrame(left_cv_rgb, cropRect);
                // Deep copy (TODO)
                //copyFrame.copyTo(frame);
                // Shallow copy
                left_cv_rgb = copyFrame;
            }
            tFrameModification += cv::getTickCount() - tStartFrameModification;

            // Get all the detected objects.
            double tStartDetection = cv::getTickCount();
            regions_t tmpRegions;
            std::vector<Yolo::Detection> detections = detectframev4(left_cv_rgb);

            std::cout << "Number object in frame " << frameCount << "th: " << detections.size() << std::endl;

            // Filter out all the objects based
            // 1. Threshold
            // 2. Desired object classe
            for (auto const& detection : detections){

                const Yolo::Detection &d = detection;
                // Detection format: [score, label, xmin, ymin, xmax, ymax].
                const float score = d.conf;
                const float fLabel= d.class_id;

                if(desiredDetect)
                {
                    if (!(std::find(desiredObjects.begin(), desiredObjects.end(), fLabel) != desiredObjects.end()))
                    {
                        continue;
                    }
                }

                std::string label;
                if (fLabel == 2.0){
                    label = "Bicycle";
                }
                else if (fLabel == 0.0){
                    label = "People";
                }else{
                    label = std::to_string(static_cast<int>(fLabel));
                }

                if (score >= detectThreshold) {

                    std::cout << label << std::endl;

                    float bbox[4];

                    bbox[0] = d.bbox[0];
                    bbox[1] = d.bbox[1];
                    bbox[2] = d.bbox[2];
                    bbox[3] = d.bbox[3];

                    cv::Rect object = get_rect(left_cv_rgb, bbox);
                    tmpRegions.push_back(CRegion(object, label, score));
                }
            }
            tDetection += cv::getTickCount() - tStartDetection;

            double tStartTracking = cv::getTickCount();
            // Update Tracker
            cv::UMat clFrame;
            clFrame = left_cv_rgb.getUMat(cv::ACCESS_READ);
            m_tracker->Update(tmpRegions, clFrame, m_fps);
            tTracking += cv::getTickCount() - tStartTracking;

            cv::Mat frameDraw = left_cv_rgb.clone();

            if(enableCount)
            {
                double tStartCounting = cv::getTickCount();
                // Update Counter
                CounterUpdater(frameDraw, countObjects_LefttoRight, countObjects_RighttoLeft);
                tCounting += cv::getTickCount() - tStartCounting;

                if(drawCount){
                    DrawCounter(frameDraw, fontScale, countObjects_LefttoRight, countObjects_RighttoLeft);
                }

            }

            if(drawOther){
                DrawData(frameDraw, frameCount, fontScale);
            }

            if (writer.isOpened() && saveVideo)
            {
                writer << frameDraw;
            }

            ++frameCount;

//            cv::imshow("Object", frameDraw);

            // Preparing for ZED SDK ingesting
            std::vector<sl::CustomBoxObjectData> objects_in;
            for (auto &it : detections) {
                sl::CustomBoxObjectData tmp;
                cv::Rect r = get_rect(left_cv_rgb, it.bbox);
                // Fill the detections into the correct format
                tmp.unique_object_id = sl::generate_unique_id();
                tmp.probability = it.conf;
                tmp.label = (int) it.class_id;
                tmp.bounding_box_2d = cvt(r);
                tmp.is_grounded = ((int) it.class_id == 0); // Only the first class (person) is grounded, that is moving on the floor plane
                // others are tracked in full 3D space
                objects_in.push_back(tmp);
            }
            // Send the custom detected boxes to the ZED
            zed.ingestCustomBoxObjects(objects_in);


            // Displaying 'raw' objects
            for (size_t j = 0; j < detections.size(); j++) {
                cv::Rect r = get_rect(left_cv_rgb, detections[j].bbox);
                cv::rectangle(left_cv_rgb, r, cv::Scalar(0x27, 0xC1, 0x36), 2);
                cv::putText(left_cv_rgb, std::to_string((int) detections[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
            }

            cv::Mat img;
            cv::resize(left_cv_rgb, img, cv::Size(960, 540));
            // cv::imshow("Objects", img);
            emit imageResults(left_cv_rgb);
            cv::waitKey(1);

            // Retrieve the tracked objects, with 2D and 3D attributes
            zed.retrieveObjects(objects, objectTracker_parameters_rt);
            // GL Viewer
            zed.retrieveMeasure(point_cloud, sl::MEASURE::XYZRGBA, sl::MEM::GPU, pc_resolution);
            zed.getPosition(cam_w_pose, sl::REFERENCE_FRAME::WORLD);
            viewer.updateData(point_cloud, objects.object_list, cam_w_pose.pose_data);

            if(cv::waitKey(1) == 27)
            {
                viewer.exit();
                break;
            }
        }

    }
    viewer.exit();
}

void QMODetAndTrack::init()
{
    // Convert desired object to float
    std::stringstream ss(desiredObjectsString);
    while (ss.good()) {
        std::string substring;
        getline(ss, substring, ',');
        z_desiredObjects.push_back(std::stof(substring));
    }

    // Get camera setting
    QSettings gui_setting("engine_gui", "MODetAndTrack");

    // Setting video output
    auto frame_width = static_cast<int>(std::min((int) gui_setting.value("output_width").toInt(), 1280));
    auto frame_height = static_cast<int>(std::min((int) gui_setting.value("output_heigh").toInt(), 720));
    m_writer.open(outFile, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), m_fps, cv::Size(frame_width, frame_height), true);

    z_fontScale = CalculateRelativeSize(1920, 1080);

    z_tFrameModification = 0;
    z_tDetection = 0;
    z_tTracking = 0;
    z_tCounting = 0;
    z_tDTC = 0;
    z_frameCount = 0;
    //  z_tStart  = cv::getTickCount();

    m_update.start(1000/m_fps);
}

// Model need to load after create Zed camera object 
void QMODetAndTrack::loadModel()
{
    // Setting detector
    detector = new YoLoObjectDetection(modelFile);
    CONSOLE << "Load model successfully";
    modelLoaded = true;
}

void QMODetAndTrack::processv2(cv::Mat image) 
{
    double tStartFrameModification = cv::getTickCount();
    // process single frame
    if(image.empty())
    {
        CONSOLE << "frame empty";
        return;
    }

    if(useCrop)
    {
        // TODO: Deep copy
        cv::Mat copyFrame(image, cropRect);
        image = copyFrame;
    }

    z_tFrameModification += cv::getTickCount() - tStartFrameModification;

    // Get all the detected objects.
    double tStartDetection = cv::getTickCount();
    regions_t tmpRegions;
    std::vector<Object> detections = detectframev2(image);

    CONSOLE << "Number object in frame " << z_frameCount << "th: " << detections.size();

    // Filter out all the objects based
    // 1. Threshold
    // 2. Desired object classe
    for (auto const& detection : detections){

        const Object &d = detection;
        // Detection format: [score, label, xmin, ymin, xmax, ymax].
        const float score = d.prob;
        const float fLabel= d.label;

        if(desiredDetect)
        {
            if (!(std::find(z_desiredObjects.begin(), z_desiredObjects.end(), fLabel) != z_desiredObjects.end()))
            {
                continue;
            }
        }

        std::string label;
        if (fLabel == 2.0){
            label = "Bicycle";
        }
        else if (fLabel == 0.0){
            label = "People";
        }
        else{
            label = std::to_string(static_cast<int>(fLabel));
        }

        if (score >= detectThreshold) {

            std::cout << label << std::endl;

            cv::Rect object(d.rec);
            tmpRegions.push_back(CRegion(object, label, score));
        }
    }

    z_tDetection += cv::getTickCount() - tStartDetection;

    double tStartTracking = cv::getTickCount();

    // Update Tracker
    cv::UMat clFrame;
    clFrame = image.getUMat(cv::ACCESS_READ);
    m_tracker->Update(tmpRegions, clFrame, m_fps);
    z_tTracking += cv::getTickCount() - tStartTracking;

    if(enableCount)
    {
        double tStartCounting = cv::getTickCount();
        // Update Counter
        CounterUpdater(image, z_countObjects_LefttoRight, z_countObjects_RighttoLeft);
        z_tCounting += cv::getTickCount() - tStartCounting;

        if(drawCount){
            DrawCounter(image, z_fontScale, z_countObjects_LefttoRight, z_countObjects_RighttoLeft);
        }

    }

    if(drawOther){
        DrawData(image, z_frameCount, z_fontScale);
    }

    if (m_writer.isOpened() && saveVideo)
    {
        m_writer << image;
    }

    ++z_frameCount;

    emit imageResults(image);
}

void QMODetAndTrack::DrawTrack(cv::Mat frame,
                               int resizeCoeff,
                               const CTrack &track,
                               bool drawTrajectory,
                               bool isStatic)
{
    auto ResizeRect = [&](const cv::Rect& r) -> cv::Rect
    {
        return cv::Rect(resizeCoeff * r.x, resizeCoeff * r.y, resizeCoeff * r.width, resizeCoeff * r.height);
    };
    auto ResizePoint = [&](const cv::Point& pt) -> cv::Point
    {
        return cv::Point(resizeCoeff * pt.x, resizeCoeff * pt.y);
    };


    if (track.m_lastRegion.m_type == "People")
    {
        cv::rectangle(frame, ResizeRect(track.GetLastRect()), cv::Scalar(198, 172, 75), 1, cv::LINE_AA);
    }
    else
    {
        cv::rectangle(frame, ResizeRect(track.GetLastRect()), cv::Scalar(119, 102, 39), 1, cv::LINE_AA);
    }

    if (drawTrajectory)
    {
        cv::Scalar cl = m_colors[track.m_trackID % m_colors.size()];

        for (size_t j = 0; j < track.m_trace.size() - 1; ++j)
        {
            const TrajectoryPoint& pt1 = track.m_trace.at(j);
            const TrajectoryPoint& pt2 = track.m_trace.at(j + 1);

            if (track.m_lastRegion.m_type == "People")
            {
                cv::line(frame, ResizePoint(pt1.m_prediction), ResizePoint(pt2.m_prediction), cv::Scalar(198, 172, 75), 1, cv::LINE_AA);
            }
            else
            {
                cv::line(frame, ResizePoint(pt1.m_prediction), ResizePoint(pt2.m_prediction), cv::Scalar(119, 102, 39), 1, cv::LINE_AA);
            }
            //cv::line(frame, ResizePoint(pt1.m_prediction), ResizePoint(pt2.m_prediction), cl, 3, CV_AA);
            if (!pt2.m_hasRaw)
            {
                //cv::circle(frame, ResizePoint(pt2.m_prediction), 4, cl, 1, CV_AA);
            }
        }
    }
}

double QMODetAndTrack::CalculateRelativeSize(int frame_width, int frame_height)
{
    int baseLine = 0;
    double countBoxWidth = frame_width * 0.1;
    double countBoxHeight = frame_height * 0.1;
    cv::Rect countBoxRec(0, 200, int(countBoxWidth), int(countBoxHeight));
    std::string counterLabel_Left = "Count : " + std::to_string(0);
    cv::Size rect = cv::getTextSize(counterLabel_Left, cv::FONT_HERSHEY_PLAIN, 1.0, 1, &baseLine);
    double scalex = (double)countBoxRec.width / (double)rect.width;
    double scaley = (double)countBoxRec.height / (double)rect.height;
    return std::min(scalex, scaley);
}

std::vector<std::vector<float>> QMODetAndTrack::detectframe(cv::Mat frame)
{
    std::vector<cv::Rect> boxes = detector->detectObject(frame);

    std::vector<std::vector<float>> boxes_float;

    for (int i=0; i < boxes.size(); i++) {
        std::vector<int> box_int = {boxes[i].tl().x,boxes[i].tl().y,boxes[i].br().x, boxes[i].br().y};
        std::vector<float> box_float(box_int.begin(), box_int.end());

        boxes_float.push_back(box_float);
    }

    return boxes_float;
}

std::vector<Object> QMODetAndTrack::detectframev2(cv::Mat frame)
{
    return detector->detectObjectv2(frame);
}

void QMODetAndTrack::detectframev3(cv::Mat frame)
{
    std::vector<Object> objects = detectframev2(frame);

    CONSOLE << "Object number: " << objects.size();

    // Draw object to image
    for(auto const& object : objects)
    {
        const Object &d = object;
        const float score = d.prob;
        const float fLabel = d.label;

        if(desiredDetect)
        {
            if (!(std::find(z_desiredObjects.begin(), z_desiredObjects.end(), fLabel) != z_desiredObjects.end()))
            {
                continue;
            }
        }

        std::string label;
        if (fLabel == 2.0){
            label = "Bicycle";
        }
        else if (fLabel == 0.0){
            label = "People";
        }else{
            label = std::to_string(static_cast<int>(fLabel));
        }

        if (score >= detectThreshold)
        {
            cv::Rect object(d.rec);
            cv::rectangle(frame, object, cv::Scalar(0, 0, 255));
            cv::putText(frame, label,
                        cv::Point(object.tl().x - 5, object.tl().y - 5),
                        cv::FONT_HERSHEY_COMPLEX,
                        1, cv::Scalar(0, 0, 0));
        }
    }

//    cv::imshow("Result", frame);
    emit imageResults(frame);
}

std::vector<Yolo::Detection> QMODetAndTrack::detectframev4(cv::Mat frame)
{
    return detector->detectObjectv3(frame);
}

void QMODetAndTrack::DrawData(cv::Mat frame, int framesCounter, double fontScale)
{
    for (const auto& track : m_tracker->tracks)
    {
        if (track->IsRobust(5,                           // Minimal trajectory size
                            0.2f,                        // Minimal ratio raw_trajectory_points / trajectory_lenght
                            cv::Size2f(0.1f, 8.0f))      // Min and max ratio: width / height
                )
        {
            DrawTrack(frame, 1, *track);
            std::string label = track->m_lastRegion.m_type + ": " + std::to_string((int)(track->m_lastRegion.m_confidence * 100)) + " %";
            //std::string label = std::to_string(track->m_trace.m_firstPass) + " | " + std::to_string(track->m_trace.m_secondPass);
            int baseLine = 0;
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
            auto rect(track->GetLastRect());
            cv::rectangle(frame, cv::Rect(cv::Point(rect.x, rect.y - labelSize.height), cv::Size(labelSize.width, labelSize.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
            cv::putText(frame, label, cv::Point(rect.x, rect.y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0),1);
        }
    }
}

void QMODetAndTrack::CounterUpdater(cv::Mat frame,
                                    std::map<std::string, int> &countObjects_LefttoRight,
                                    std::map<std::string, int> &countObjects_RighttoLeft)
{
    // Draw Counter line
    cv::Point polyLinePoints[1][4];
    polyLinePoints [0][0] = cv::Point (line2_x2,line2_y2);
    polyLinePoints [0][1] = cv::Point (line2_x1,line2_y1);
    polyLinePoints [0][2] = cv::Point (line1_x1,line1_y1);
    polyLinePoints [0][3] = cv::Point (line1_x2,line1_y2);
    const cv::Point* ppt[1] = { polyLinePoints[0] };
    int npt[] = { 4 };

    cv::fillPoly(frame, ppt, npt, 1, cv::Scalar( 0, 255, 255), 8);

    for (const auto& track : m_tracker->tracks)
    {
        if(track->m_trace.size() >= 2)
        {
            // Extract the last two points from trace.
            track_t pt1_x = track->m_trace.at(track->m_trace.size() - 2).m_prediction.x;
            track_t pt1_y = track->m_trace.at(track->m_trace.size() - 2).m_prediction.y;
            track_t pt2_x = track->m_trace.at(track->m_trace.size() - 1).m_prediction.x;
            track_t pt2_y = track->m_trace.at(track->m_trace.size() - 1).m_prediction.y;

            float pt1_position_line1 = (line1_y2 - line1_y1) * pt1_x + (line1_x1 - line1_x2) * pt1_y + (line1_x2 * line1_y1 - line1_x1 * line1_y2);
            float pt2_position_line1 = (line1_y2 - line1_y1) * pt2_x + (line1_x1 - line1_x2) * pt2_y + (line1_x2 * line1_y1 - line1_x1 * line1_y2);
            float pt1_position_line2 = (line2_y2 - line2_y1) * pt1_x + (line2_x1 - line2_x2) * pt1_y + (line2_x2 * line2_y1 - line2_x1 * line2_y2);
            float pt2_position_line2 = (line2_y2 - line2_y1) * pt2_x + (line2_x1 - line2_x2) * pt2_y + (line2_x2 * line2_y1 - line2_x1 * line2_y2);
            if (310 <= pt2_y and pt2_y <= 330){
                cv::fillPoly(frame, ppt, npt, 1, cv::Scalar( 0, 100, 0), 8);
            }

            // Update Counter from Left to Right
            if(direction == 0)
            {
                if(pt1_position_line1 < 0  && pt2_position_line1 >= 0)
                {
                    track->m_trace.FirstPass();
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line2 >= 0 && !track->m_trace.GetSecondPass() )
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<std::string, int>::iterator,bool> ret;
                    ret = countObjects_LefttoRight.insert ( std::pair<std::string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second) {
                        ret.first->second = ret.first->second + 1;

                    }
                }
                // Update Counter from Right to Left
            }
            else if (direction == 1)
            {
                if(pt2_position_line2 <= 0  && pt1_position_line2 > 0)
                {
                    track->m_trace.FirstPass();
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line1 <= 0 && !track->m_trace.GetSecondPass() )
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<std::string, int>::iterator,bool> ret;
                    ret = countObjects_RighttoLeft.insert ( std::pair<std::string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second) {
                        ret.first->second = ret.first->second + 1;

                    }
                }
            }// Update Counter from both directions
            else
            {
                if(pt2_position_line2 <= 0  && pt1_position_line2 > 0){
                    track->m_trace.FirstPass();
                    track->m_trace.m_directionFromLeft = true;
                }
                else if(pt1_position_line1 < 0  && pt2_position_line1 >= 0)
                {
                    track->m_trace.FirstPass();
                    track->m_trace.m_directionFromLeft = false;
                }
                if (track->m_trace.GetFirstPass() && pt2_position_line1 <= 0 && !track->m_trace.GetSecondPass() && track->m_trace.m_directionFromLeft)
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<std::string, int>::iterator,bool> ret;
                    ret = countObjects_RighttoLeft.insert ( std::pair<std::string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second) {
                        ret.first->second = ret.first->second + 1;
                    }
                }
                else if (track->m_trace.GetFirstPass() && pt2_position_line2 >= 0 && !track->m_trace.GetSecondPass() && !track->m_trace.m_directionFromLeft)
                {
                    track->m_trace.SecondPass();
                    std::pair<std::map<std::string, int>::iterator,bool> ret;
                    ret =  countObjects_LefttoRight.insert ( std::pair<std::string, int>(track->m_lastRegion.m_type, 1));
                    if (!ret.second) {
                        ret.first->second = ret.first->second + 1;
                    }
                }
            }
        }
    }
}

void QMODetAndTrack::DrawCounter(cv::Mat frame,
                                 double fontScale,
                                 std::map<std::string, int> &countObjects_LefttoRight,
                                 std::map<std::string, int> &countObjects_RighttoLeft)
{
    // Line
    cv::Point polyLinePoints[1][4];
    polyLinePoints [0][0] = cv::Point (line2_x2,line2_y2);
    polyLinePoints [0][1] = cv::Point (line2_x1,line2_y1);
    polyLinePoints [0][2] = cv::Point (line1_x1,line1_y1);
    polyLinePoints [0][3] = cv::Point (line1_x2,line1_y2);
    const cv::Point* ppt[1] = { polyLinePoints[0] };
    int npt[] = { 4 };

    cv::fillPoly(frame, ppt, npt, 1, cv::Scalar( 0, 255, 255), 8);

    // cv::line( frame, cv::Point( line2_x1, line2_y1 ), cv::Point( line2_x2, line2_y2), cv::Scalar( 120, 220, 0),  3, 8 );

    // Create Counter label
    std::string counterLabel_L = "Count --> : ";
    std::string counterLabel_R = "Count <-- : ";
    for(auto elem : countObjects_LefttoRight){
        counterLabel_L += elem.first + ": " + std::to_string(elem.second) + " | ";
    }
    for(auto elem : countObjects_RighttoLeft){
        counterLabel_R += elem.first + ": " + std::to_string(elem.second) + " | ";
    }

    // Draw counter label
    int baseLine = 0;
    float fontSize = 0.4;
    cv::Size labelSize_LR = cv::getTextSize(counterLabel_L, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    cv::Size labelSize_RL = cv::getTextSize(counterLabel_R, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    cv::rectangle(frame, cv::Rect(cv::Point(0, 400 - 30 - labelSize_LR.height), cv::Size(labelSize_LR.width, labelSize_LR.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::rectangle(frame, cv::Rect(cv::Point(0, line2_y1 + 30 - labelSize_LR.height), cv::Size(labelSize_RL.width, labelSize_RL.height + baseLine)), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::putText(frame, counterLabel_L, cv::Point(0, 400 - 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
    cv::putText(frame, counterLabel_R, cv::Point(0, line2_y1 + 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
    // cv::Size labelSize_LR = cv::getTextSize(counterLabel_L, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    // cv::Size labelSize_RL = cv::getTextSize(counterLabel_R, cv::FONT_HERSHEY_SIMPLEX, fontSize, 1, &baseLine);
    // cv::rectangle(frame, cv::Rect(cv::Point(10, 50 - 30 - labelSize_LR.height), cv::Size(labelSize_LR.width, labelSize_LR.height + baseLine)), cv::Scalar(255, 255, 255), CV_FILLED);
    // cv::rectangle(frame, cv::Rect(cv::Point(10, 600 + 30 - labelSize_LR.height), cv::Size(labelSize_RL.width, labelSize_RL.height + baseLine)), cv::Scalar(255, 255, 255), CV_FILLED);
    // cv::putText(frame, counterLabel_L, cv::Point(10, 50 - 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
    // cv::putText(frame, counterLabel_R, cv::Point(10, 600 + 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(0, 0, 0),1.5);
}

void QMODetAndTrack::resetCounter()
{
    // Reset counter
}

void QMODetAndTrack::stopProcess()
{
    isRuning = false;
}









