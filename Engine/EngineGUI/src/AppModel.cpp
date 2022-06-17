#include "AppModel.h"

AppModel::AppModel(QObject *parent)
    : QObject{parent},
      m_state(NONE_STATE)
{
    readSettingFile("./Data/config/config.yaml");
    m_detAndTrack = new QMODetAndTrack();
}

AppModel::~AppModel()
{

}

void AppModel::readSettingFile(QString path)
{
    // Something stupid;
    QSettings gui_setting("engine_gui", "MODetAndTrack");
    gui_setting.setValue("output", "./Data/video/output.mp4");
    gui_setting.setValue("input", "./Data/video/MOT17-11.mp4");
    gui_setting.setValue("end_frame", "100000");
    gui_setting.setValue("start_frame", "0");

    gui_setting.setValue("fps", "30");
    gui_setting.setValue("save_video", false);
    gui_setting.setValue("count", true);
    gui_setting.setValue("draw_count", true);
    gui_setting.setValue("draw_other", false);

    gui_setting.setValue("direction", "2"); // 0 - left to right, 1 - right to left, 2 - both
    gui_setting.setValue("crop", "0");

    gui_setting.setValue("crop_width", "600");
    gui_setting.setValue("crop_height", "400");

    gui_setting.setValue("crop_x", "960");
    gui_setting.setValue("crop_y", "1100");

    gui_setting.setValue("threshold", "0.5");
    gui_setting.setValue("desired_detect", "1");
    gui_setting.setValue("desired_objects", "0");

    gui_setting.setValue("l1p1_x", 950);
    gui_setting.setValue("l1p1_y", 0);
    gui_setting.setValue("l1p2_x", 950);
    gui_setting.setValue("l1p2_y", 1080);
    gui_setting.setValue("l2p1_x", 970);
    gui_setting.setValue("l2p1_y", 0);
    gui_setting.setValue("l2p2_x", 970);
    gui_setting.setValue("l2p2_y", 1080);

    gui_setting.setValue("model", "./Data/model/yolo.engine");
}

void AppModel::processImage(cv::Mat frame)
{

}

void AppModel::setState(AppModel::APP_STATE state)
{
    m_state = state;
}
