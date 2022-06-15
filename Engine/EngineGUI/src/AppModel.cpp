#include "AppModel.h"

AppModel::AppModel(QObject *parent)
    : QObject{parent},
      m_state(NONE_STATE)
{

}

AppModel::~AppModel()
{

}

void AppModel::processImage(cv::Mat frame)
{

}

void AppModel::setState(AppModel::APP_STATE state)
{
    m_state = state;
}
