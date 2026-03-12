#ifndef SAPERACAMERACONTROLLER_H
#define SAPERACAMERACONTROLLER_H

#include <QObject>
#include <QGraphicsView>
// 包含 Sapera 头文件
#include "SapClassBasic.h"
#include "imageprocessthread.h"

class SaperaCameraController : public QObject
{
    Q_OBJECT
public:
    explicit SaperaCameraController(QObject *parent = nullptr);

signals:
    void updateCameraListSignal(QStringList cameraList);
    void warningSaperaSignal(QString warningText);
    void barShowSaperaSignal(QString warningText);

private slots:
    // 用于初始化和刷新相机列表
    void refreshCameraList();

    // 开始和停止采集
    void startAcquisition();
    void stopAcquisition();
    // 用于定时获取帧
    void grabFrame();
    // // 单次采集
    // 更新图像显示  测试
    // void updateDisplay(const QImage& image);

    QImage convertTo8Bit(const QImage& image16 , int minValUi, int maxValUi);



public:
    int minValGet = 0;
    int maxValGet = 65535;
    // 获取ui界面最大最小灰度限制
    void getMinMaxVal(int minValUiSent, int maxValUiSent);
    void getLimitArea(int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi);
    double grayThre = 0.0;
    // void updateGrayThre(double grayThreUi, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi);
    void updateGrayThre(double grayThreUi);
    int leftUpX = 0;
    int leftUpY = 0;
    int rightDownX = 0;
    int rightDownY = 0;
    int officialNoiseLevel = 0;
    int testNoiseLevel = 0;
    void getNoiseLevel(int officialNoiseLevelUi, int testNoiseLevelUi);

    // 当用户选择新相机时
    void onCameraSelected(int index);
    // 测试
    void setGraphicsView(QGraphicsView* view);

    // QTimer *m_grabTimer;
    // 提供公有方法访问 m_cameraNames
    void clearCameraNames() {
        m_cameraNames.clear();
    }
    void updateCameraList() {  // 提供公有接口,目的是不暴漏函数的内部实现
        refreshCameraList();
    }
    void startAcq(){
        startAcquisition();
    }
    void stopAcq(){
        stopAcquisition();
    }
    void grabFra(){
        grabFrame();
    }
    void onCameraSel(int index){
        onCameraSelected(index);
    }
    void destroySap(){
        destroySaperaObjects();
    }
    void setImageProcessThread(ImageProcessThread *thread);
    void setGrayThreshold(const QString &threshold);

    void getGrayThreUi(QString &grayThreUi); // 获取主Ui的灰度阈值

    void getCcfPath(QString ccfPath);

    cv::Mat QImageToMat(const QImage &image);

    void saveRaw(int saveRawNum, QString rawFilesPathGet);
    int remainingRawNum = 0;
    int totalRawNum = 0;
    QString rawFilesPath;

    void simulateFrame(); // 临时测试
private:


    // 用于显示图像
    QGraphicsScene *m_scene;
    QGraphicsView *m_graphicsView;

    QTimer *m_grabTimer;
    // 自定义成员
    QString ccfPathRec;
    // Sapera 相关成员 - 根据官方示例修改类型
    SapAcquisition *m_Acq;
    SapBuffer *m_Buffers;
    SapAcqToBuf *m_Xfer;  // 修改为 SapAcqToBuf 类型
    SapView *m_View;      // 添加 SapView 对象

    // 保存可用的相机名称列表
    QStringList m_cameraNames;
    // 保存相机是否正在运行
    bool m_isAcquiring;

    // 初始化 Sapera 资源
    bool createSaperaObjects(const QString &cameraName);
    // 释放 Sapera 资源
    void destroySaperaObjects();
    // 检查 Sapera 资源是否已创建
    bool isSaperaObjectsCreated() const;

    // 静态回调函数，用于 Sapera 回调
    static void XferCallback(SapXferCallbackInfo *pInfo);
    // 信号状态回调
    static void SignalCallback(SapAcqCallbackInfo *pInfo);
    // 实际处理帧的函数
    void processFrame(void *pData);
    //
    // 模拟调用的重载
    void processFrameTest(void *pData, UINT32 width, UINT32 height, SapFormat format);

    // 获取信号状态
    void getSignalStatus();
    void getSignalStatus(SapAcquisition::SignalStatus signalStatus);

    // // 记录 Sapera 错误信息
    // void logSaperaError(const QString &operation);

    ImageProcessThread *m_imageProcessThread = nullptr; // 创建一个图像处理的对象
    QString grayThreHik = "20"; // 存放来自主Ui的灰度阈值

    QMutex rawMutex; // 添加互斥锁

signals:
    void imageReady(const QImage& image);
    void cameralinkSignal(const QImage& image, cv::Point2d centerPoint, cv::Point2d centerPointLimit, double totalGray_official, double totalGray_test);
};

#endif // SAPERACAMERACONTROLLER_H
