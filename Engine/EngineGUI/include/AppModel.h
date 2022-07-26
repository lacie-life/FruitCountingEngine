#ifndef APPMODEL_H
#define APPMODEL_H

#include <QObject>
#include <QPixmap>
#include <QTimer>
#include <QThread>
#include <memory>
#include "QMODetAndTrack.h"
#include "AppConstants.h"


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
    void processCamera();
    void stopPocessVideo();

signals:
    void imageReady(QPixmap pixmap);

public slots:
    void processImage(cv::Mat frame);
    void setState(APP_STATE state);
    void sendImage();

public:
    APP_STATE m_state;

    QMODetAndTrack* m_detAndTrack;
    QThread m_detAndTrackThread;

    QTimer m_update;
    cv::Mat m_frame;

};

#endif // APPMODEL_H
