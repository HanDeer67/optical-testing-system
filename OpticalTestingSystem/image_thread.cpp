#include "image_thread.h"
#include<QThread>
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
#include <QMutexLocker>

image_Thread::image_Thread(QObject *parent)
    : QObject{parent}
{}

image_Thread::~image_Thread()
{
    // 清理资源
}
void image_Thread::initialize()
{
    qDebug() << "Image processor initialized in thread:" << QThread::currentThreadId();
    // 可以进行一些初始化工作
}

// 当有新的视频帧时调用
void image_Thread::onFrameAvailable(const QVideoFrame &frame , const QString &grayThre) {
    // qDebug() << "Sub-thread ID:" << QThread::currentThreadId();
    // qDebug() << "Frame format:" << frame.pixelFormat()
    //          << "Size:" << frame.width() << "x" << frame.height()
    //          << "Valid:" << frame.isValid();
    if (!frame.isValid()) {
        emit error("Invalid frame received");
        return;
    }
    try{
        QImage image = frame.toImage();
        if (image.isNull()) {
            emit error("Converted image is null");
            return;
        }
        // 使用互斥锁保护共享资源，使用花括号限制互斥锁的作用域
        {
            QMutexLocker locker(&mutex);
            pixmap = QPixmap::fromImage(image);
        } // 互斥锁在这里自动释放

        // 转换灰度阈值
        bool ok;
        double threshold = grayThre.toDouble(&ok);
        if (!ok) {
            emit error("Invalid gray threshold value");
            return;
        }
        // 计算质心
        calculateCentroidNew(image, threshold);
        emit processingFinished();

    } catch (const std::exception &e) {
        emit error(QString("Exception in frame processing: %1").arg(e.what()));
    }
}
// 质心计算函数
void image_Thread::calculateCentroidNew(const QImage &image, const double &grayThre)
{
    QImage grayImage;
    if (image.format() != QImage::Format_Grayscale8) {
        grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    } else {
        grayImage = image;
    }

    cv::Mat matImage = QImageToMat(grayImage);
    if (matImage.empty()) {
        emit error("Failed to convert QImage to cv::Mat");
        return;
    }

    cv::Point2d centerPoint = grayCenter(matImage, grayThre);

    // 发送结果到UI
    {
        QMutexLocker locker(&mutex);
        QPixmap localPixmap = pixmap;  // 读操作
        // emit sendSignalToUi(localPixmap, centerPoint);
    }
    /* 这里的互斥锁保护了：
        防止在更新 pixmap 的过程中被读取
        确保读取到的是完整的 pixmap 数据
    */
}



cv::Mat image_Thread::QImageToMat(const QImage &image)
{
    if (image.isNull()) {
        return cv::Mat();
    }

    switch (image.format()) {
    case QImage::Format_Grayscale8: {
        cv::Mat mat(image.height(), image.width(), CV_8UC1,
                    (void *)image.constBits(), image.bytesPerLine());
        return mat.clone();
    }
    case QImage::Format_RGB888: {
        // RGB
        cv::Mat mat(image.height(), image.width(), CV_8UC3, (void *)image.bits(), image.bytesPerLine());
        return mat.clone();
    }
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied: {
        cv::Mat mat(image.height(), image.width(), CV_8UC4,
                    (void *)image.constBits(), image.bytesPerLine());
        return mat.clone();
    }
    default:
        qWarning() << "Unsupported image format:" << image.format();
        return cv::Mat();
    }
}
