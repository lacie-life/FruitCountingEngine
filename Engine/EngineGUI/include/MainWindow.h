#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "AppModel.h"
#include "QCameraCapture.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void stateChanged(AppModel::APP_STATE state);

public slots:
    void openCamera();
    void closeCamera();

    void slot_countCheckbox();
    void slot_detectCheckbox();
    void videoTest();

private:
    Ui::MainWindow *ui;
    AppModel* m_model;
    QCameraCapture* m_camera;
};

#endif // MAINWINDOW_H
