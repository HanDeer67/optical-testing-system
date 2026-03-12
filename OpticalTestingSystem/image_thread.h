#ifndef IMAGE_THREAD_H
#define IMAGE_THREAD_H

#include <QObject>
//相机图像头文件
#include <QCamera>            //启动或停止摄像头、设置摄像头属性（如分辨率、帧率等），并通过它捕获实时视频流。
// #include <QCameraViewfinder>  //显示相机的实时视频流。它可以作为相机的取景器，将视频内容显示在界面上 适用于qt5版本
// #include <QCameraImageCapture>
// #include <QVideoWidget>
#include <QMediaCaptureSession>  //qt6
#include <QVideoSink>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QPixmap>
#include <QThread>
#include "opencv2/opencv.hpp"
#include "Grayscale_centroid.h"
#include <QMutexLocker>

class image_Thread : public QObject
{
    Q_OBJECT
public:
    explicit image_Thread(QObject *parent = nullptr);
    ~image_Thread(); //新增  ???

public slots:
    void onFrameAvailable(const QVideoFrame &frame, const QString &grayThre);
    // 新增：初始化槽函数，可以在线程启动时调用
    void initialize(); //新增  ???

signals:
    // void sendImageToUi(const QPixmap &pixmap);
    void sendSignalToUi(const QPixmap &pixmap, const cv::Point2d &centerPoint, const cv::Point2d &centerPointLimit);
    // 新增：错误信号
    void error(const QString &message);
    // 新增：处理完成信号
    void processingFinished();

private:
    // 新增：互斥锁保护共享资源
    QMutex mutex;
    QPixmap pixmap;
    void calculateCentroidNew(const QImage &image, const double &grayThre); // 计算质心
    cv::Mat QImageToMat(const QImage &image);
    // void run() override;  // 重写 run() 方法，启动线程事件循环
};

#endif // IMAGE_THREAD_H
