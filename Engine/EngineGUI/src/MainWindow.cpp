#include "MainWindow.h"
#include "ui_mainwindow.h"

#include <QIcon>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<cv::Mat>("cv::Mat");

    setWindowIcon(QIcon("./Data/icon.png"));

    m_camera = new QCameraCapture();

    m_model = new AppModel();

    m_camera->moveToThread(new QThread(this));

    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::openCamera);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::closeCamera);
    connect(ui->videoTestButton, &QPushButton::clicked, this, &MainWindow::videoTest);
    connect(ui->stopVideoTest, &QPushButton::clicked, this, [this] {
        m_model->stopPocessVideo();
        disconnect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
    });

    connect(ui->detecCheck, &QCheckBox::clicked, this, &MainWindow::slot_detectCheckbox);
    connect(ui->countCheck, &QCheckBox::clicked, this, &MainWindow::slot_countCheckbox);

    connect(this, &MainWindow::stateChanged, m_model, &AppModel::setState);

    connect(this, &MainWindow::cameraOpened, this, [this] {
       m_model->m_detAndTrack->loadModel();
    });
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

    emit cameraOpened();
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

    emit cameraClosed();
}

void MainWindow::slot_detectCheckbox()
{
    if(ui->detecCheck->isChecked() && !ui->countCheck->isChecked()){

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::DETECTING_STATE);
    }
    else if (!ui->detecCheck->isChecked() && !ui->countCheck->isChecked()){

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

void MainWindow::slot_countCheckbox()
{
    if(ui->countCheck->isChecked() && !ui->detecCheck->isChecked()){

        disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);
        connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);

        emit stateChanged(AppModel::APP_STATE::COUNTING_STATE);
    }
    else if (!ui->detecCheck->isChecked() && !ui->countCheck->isChecked()){

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

void MainWindow::videoTest()
{
    connect(m_model, &AppModel::imageReady, ui->imageView, &QLabel::setPixmap);
    m_model->processVideo();
//    m_model->processCamera();
}
