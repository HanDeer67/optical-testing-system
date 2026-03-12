#ifndef BASLERCAMERACONTROLLER_H
#define BASLERCAMERACONTROLLER_H

#include <QObject>
#include <QMutex>
#include <QImage>

// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>
#include <pylon/BaslerUniversalInstantCamera.h>
#include "imageprocessthread.h"


class CGuiCamera : public QObject
    , public Pylon::CImageEventHandler             // Allows you to get notified about images received and grab errors.
    , public Pylon::CConfigurationEventHandler     // Allows you to get notified about device removal.
    , public Pylon::CCameraEventHandler            // Allows you to get notified about camera events and GenICam node changes.
{
    Q_OBJECT
public:
    explicit CGuiCamera(QObject *parent = nullptr);
    ~CGuiCamera(); //析构

    void Close();
    void StopGrab();

    int EnumerateDevices();
    void initBaslerCamera();
    void scanCamera();
    void openCamera();
    void closeCamera();
    void ContinuousGrab();
    const QImage& GetImage() const;
    void startGrabbing(ImageProcessThread *imageProcessThread); //这是要放进线程中的函数
    void stopGrabbing();
    void getGrayThreUi(QString &grayThreUi);
    // 设备信息列表
    Pylon::DeviceInfoList_t devices_Basler;

    void getLimitArea(int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi);
    int leftUpX = 0;
    int leftUpY = 0;
    int rightDownX = 0;
    int rightDownY = 0;
    int officialNoiseLevel = 0;
    int testNoiseLevel = 0;
    void getNoiseLevel(int officialNoiseLevelUi, int testNoiseLevelUi);


private:
    // Protects members.
    mutable QMutex m_MemberLock;
private:
    // The pylon camera object.
    Pylon::CBaslerUniversalInstantCamera m_camera;
    // The grab result retrieved from the camera.
    Pylon::CGrabResultPtr m_ptrGrabResult;
    // Buffer used for grabbed images
    Pylon::CPylonImage m_ptrGrabBuffer;

    static const int MaxCamera = 1;
    QImage m_image;
    QString grayThreHik = "20"; // 存放来自主Ui的灰度阈值

    // The format converter to convert a grab result into a QImage.
    Pylon::CImageFormatConverter m_formatConverter;

    // Smart pointer to camera features
    Pylon::CIntegerParameter m_exposureTime; // 曝光时间
    Pylon::CIntegerParameter m_gain; // 增益

    ImageProcessThread* m_imageProcessThread = nullptr;

protected:
    // 重写 CImageEventHandler 的虚函数
    virtual void OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult) override;

signals:
    void cameraInfoSignal(QString chModelName);
    void imageGrabbed(const QImage& image);
};

#endif // BASLERCAMERACONTROLLER_H
