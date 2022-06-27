#include "MainWindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<cv::Mat>("cv::Mat");

    m_model = new AppModel();

    m_camera = new QCameraCapture();

    m_camera->moveToThread(new QThread(this));

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::openCamera);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::closeCamera);

    connect(ui->detecCheck, &QCheckBox::clicked, this, &MainWindow::slot_countCheckbox);
    connect(ui->countCheck, &QCheckBox::clicked, this, &MainWindow::slot_detectCheckbox);

    connect(this, &MainWindow::stateChanged, m_model, &AppModel::setState);

}

MainWindow::~MainWindow()
{
    m_camera->stop();
    m_camera->thread()->quit();
    m_camera->thread()->wait();

    delete ui;
}

void MainWindow::openCamera()
{
    m_camera->initCamera();

    connect(m_camera->thread(), &QThread::started, m_camera, &QCameraCapture::stream);
    connect(m_camera->thread(), &QThread::finished, m_camera, &QCameraCapture::deleteLater);
    connect(m_camera, &QCameraCapture::frameReady, m_model, &AppModel::processImage);
    if (m_model->m_state == AppModel::APP_STATE::NONE_STATE){
        connect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
    } else {
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
    }

    m_camera->thread()->start();
}

void MainWindow::closeCamera()
{
    disconnect(m_camera->thread(), &QThread::started, m_camera, &QCameraCapture::stream);
    disconnect(m_camera->thread(), &QThread::finished, m_camera, &QCameraCapture::deleteLater);
    disconnect(m_camera, &QCameraCapture::frameReady, m_model, &AppModel::processImage);
//    disconnect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
    disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);

    if (m_model->m_state == AppModel::APP_STATE::COUNTING_STATE || m_model->m_state == AppModel::APP_STATE::DETECTING_STATE){
        disconnect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
    }

    m_camera->stop();
    m_camera->thread()->quit();
    m_camera->thread()->wait();
}

void MainWindow::slot_countCheckbox()
{
    if(ui->detecCheck->isChecked() && !ui->detecCheck->isChecked()){

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::DETECTING_STATE);
    }
    else if (!ui->detecCheck->isChecked() && !ui->detecCheck->isChecked()){

        disconnect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
        connect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::NONE_STATE);
    }
    else {
        ui->countCheck->setCheckState(Qt::Unchecked);
        ui->detecCheck->setCheckState(Qt::Checked);

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::DETECTING_STATE);
    }
}

void MainWindow::slot_detectCheckbox()
{
    if(ui->countCheck->isChecked() && !ui->detecCheck->isChecked()){

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::COUNTING_STATE);
    }
    else if (!ui->detecCheck->isChecked() && !ui->detecCheck->isChecked()){

        disconnect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
        connect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::NONE_STATE);
    }
    else {
        ui->detecCheck->setCheckState(Qt::Unchecked);
        ui->countCheck->setCheckState(Qt::Checked);

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::COUNTING_STATE);
    }
}
