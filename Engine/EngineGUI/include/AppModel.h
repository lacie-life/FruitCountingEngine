#ifndef APPMODEL_H
#define APPMODEL_H

#include <QObject>
#include <QPixmap>
#include "QMODetAndTrack.h"

#ifndef MACRO_DEFINE
#define MACRO_DEFINE

#define CONSOLE qDebug() << "[" << __FUNCTION__ << "] "

#endif

class AppModel : public QObject
{
    Q_OBJECT

    Q_ENUMS(APP_STATE)

public:

    enum APP_STATE {
        NONE_STATE,
        DETECTING_STATE,
        COUNTING_STATE,
        END_STATE,
    };

    explicit AppModel(QObject *parent = nullptr);
    ~AppModel();

    void readSettingFile(QString path);
    void processVideo();
    void stopPocessVideo();

signals:
    void imageReady(QPixmap pixmap);

public slots:
    void processImage(cv::Mat frame);
    void setState(APP_STATE state);

public:
    APP_STATE m_state;
    QMODetAndTrack* m_detAndTrack;

};

#endif // APPMODEL_H
