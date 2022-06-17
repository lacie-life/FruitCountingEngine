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
}

void AppModel::processImage(cv::Mat frame)
{

}

void AppModel::setState(AppModel::APP_STATE state)
{
    m_state = state;
}
