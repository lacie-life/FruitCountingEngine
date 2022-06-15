#ifndef QMODETANDTRACK_H
#define QMODETANDTRACK_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <iostream>
#include <dirent.h>
#include <fstream>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/face.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/opencv.hpp>

#include "MO-Tracker/defines.h"
#include "MO-Tracker/Ctracker.h"

class QMODetAndTrack : public QObject
{
    Q_OBJECT

public:
    explicit QMODetAndTrack(QObject *parent = nullptr);
    ~QMODetAndTrack();

signals:

};

#endif // QMODETANDTRACK_H

