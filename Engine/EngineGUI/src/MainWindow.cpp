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
    connect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);

    m_camera->thread()->start();
}

void MainWindow::closeCamera()
{
    disconnect(m_camera->thread(), &QThread::started, m_camera, &QCameraCapture::stream);
    disconnect(m_camera->thread(), &QThread::finished, m_camera, &QCameraCapture::deleteLater);
    disconnect(m_camera, &QCameraCapture::frameReady, m_model, &AppModel::processImage);
    disconnect(m_camera, &QCameraCapture::frameUIReady, ui->imageView, &QLabel::setPixmap);

    m_camera->stop();
    m_camera->thread()->quit();
    m_camera->thread()->wait();
}
