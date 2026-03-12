#ifndef HIKCAMERACONTROLLER_H
#define HIKCAMERACONTROLLER_H

#include <QObject>
#include <QThread>
#include "MvCameraControl.h"
#include <QImage>
#include "imageprocessthread.h"

// 创建图像采集工作类
class ImageGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ImageGrabber(void* handle, QObject *parent = nullptr);
    void setHandle(void* handle) { this->handle = handle; }


public slots:
    void startGrabbing(ImageProcessThread *imageProcessThread);
    void stopGrabbing();
    void getGrayThreUi(QString &grayThreUi); // 获取主Ui的灰度阈值

private:
    void* handle;
    bool isGrabbing;
    QString grayThreHik = "20"; // 存放来自主Ui的灰度阈值


signals:
    void imageGrabbed(const QImage& image);
    void finished();
};

class HikCameraController : public QObject
{
    Q_OBJECT
public:
    explicit HikCameraController(QObject *parent = nullptr);
    ~HikCameraController();

    // 扫描相机，返回相机列表
    void scanCamera();
    void PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);

    void setCameraPara(float expTimeUs, float gainDb = 0.0, float frameRate = 30.0);
    void openCamera();
    void closeCamera();

    void *getHandle()const { return handle; } // 这是个什么语法？

private:
    void* handle = NULL; // 句柄
    MV_CC_DEVICE_INFO_LIST stDeviceList; // 枚举
    ImageGrabber* grabber;
    QThread* grabberThread;

signals:
    void cameraInfoSignal(QString cameraName);
    void newImageCaptured(const QImage& image);
};

#endif // HIKCAMERACONTROLLER_H
