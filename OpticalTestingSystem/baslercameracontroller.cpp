#include "baslercameracontroller.h"
#include <QDebug>
#include <QMessageBox>

CGuiCamera::CGuiCamera(QObject *parent)
    : QObject{parent}
{
    // 设置图像格式转换器的输出格式
    m_formatConverter.OutputPixelFormat = Pylon::PixelType_Mono8;  // 使用 Mono8 格式
    // m_formatConverter.MonoConversionMethod = Pylon::MonoConversionMethod_Luminance;  // 添加这行

    // 注册事件处理器
    m_camera.RegisterImageEventHandler(this, Pylon::RegistrationMode_ReplaceAll, Pylon::Cleanup_None);
    m_camera.RegisterConfiguration(new Pylon::CAcquireContinuousConfiguration, Pylon::RegistrationMode_ReplaceAll, Pylon::Cleanup_Delete);

    // Register this object as a configuration event handler in order to get notified of camera state changes.
    // See Pylon::CConfigurationEventHandler for details.
    m_camera.RegisterConfiguration( this, Pylon::RegistrationMode_Append, Pylon::Cleanup_None );
}


CGuiCamera::~CGuiCamera()
{
    Close();
}


void CGuiCamera::Close()
{
    QMutexLocker lock( &m_MemberLock );

    // Stop the grab so the grab thread will not set new m_ptrGrabResult.
    StopGrab();

    // Free the grab result if present.
    m_ptrGrabResult.Release();

    // Free the image by swapping it with a dummy (null) image.
    QImage dummy;
    m_image.swap( dummy );

    // Remove the event handlers that will be called when the feature changes, e.g., its state or value.
    m_camera.DeregisterCameraEventHandler( this, "TriggerSource" );
    m_camera.DeregisterCameraEventHandler( this, "TriggerMode" );
    m_camera.DeregisterCameraEventHandler( this, "PixelFormat" );

    if (m_gain.IsValid())
    {
        // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
        m_camera.DeregisterCameraEventHandler( this, m_gain.GetNode()->GetName() );
    }

    if (m_exposureTime.IsValid())
    {
        // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
        m_camera.DeregisterCameraEventHandler( this, m_exposureTime.GetNode()->GetName() );
    }

    // Clear the pointers to the features we set manually in Open().
    m_exposureTime.Release();
    m_gain.Release();

    // Close the camera and free all resources.
    m_camera.DestroyDevice();
}

// Stop the continuous grab on the camera.
void CGuiCamera::StopGrab()
{
    m_camera.StopGrabbing();
}

// 扫描（枚举）当前可用的相机，返回相机数量
int CGuiCamera::EnumerateDevices()
{
    // 设备信息列表
    Pylon::DeviceInfoList_t devices;

    try
    {
        // 创建 Transport Layer Factory 实例.
        Pylon::CTlFactory& TlFactory = Pylon::CTlFactory::GetInstance();
        // 枚举可用设备.
        TlFactory.EnumerateDevices( devices );
        qDebug()<<"枚举可用设备完成!";
    }
    catch (const Pylon::GenericException& e)
    {
        PYLON_UNUSED( e );
        devices.clear();
        qDebug() <<"枚举可用设备失败，错误描述："<< e.GetDescription();
    }

    // 输出设备信息
    if (devices.empty())
    {
        qDebug()  << "未找到任何 Basler 相机!" ;
        // qDebug()  << "devices.size:"<<devices.size(); // √
    }
    else
    {
        qDebug()  << "找到 " << devices.size() << " 台相机: " ;

        for (size_t i = 0; i < devices.size(); ++i)
        {
            qDebug() << "------------------------------------";
            qDebug()  << "设备 " << i + 1 << " 信息："  ;
            qDebug()  << "厂商: " << devices[i].GetVendorName()  ; // 厂商名字
            qDebug()  << "型号: " << devices[i].GetModelName()  ; // 设备型号
            qDebug()  << "序列号: " << devices[i].GetSerialNumber()  ;
            qDebug()  << "接口类型: " << devices[i].GetDeviceClass()  ;
        }
        // 刷新全局设备信息
        devices_Basler = devices;
        // 获取设备型号
        QString chModelName = QString::fromStdString(devices[0].GetModelName().c_str()); // 这一句代码需要在检测到设备的前提下才能正常运行，否则会崩溃
        qDebug()<<chModelName;
        // 将设备型号发送出去，输出到combbox中
        emit cameraInfoSignal(chModelName);
    }

    return (int) devices.size(); // 返回设备数量
}

// 初始化相机库文件
void CGuiCamera::initBaslerCamera(){
    Pylon::PylonInitialize();  // 初始化 Pylon 库
};

void CGuiCamera::scanCamera(){
    initBaslerCamera();
    // 扫描所有的Basler相机，输出相机信息，发射带有相机信息的信号，主ui根据相机名称更新combbox
    qDebug()<<"Basler相机数量："<<EnumerateDevices();
}

// The Open Selected button for camera 1 has been clicked.
void CGuiCamera::openCamera()
{
    qDebug()<<"当前在执行openCamera函数";
    // Creates and opens the camera specified in deviceInfo.
    // Initializes the member variables to access camera features.
    // Registers camera event handler for camera features so we'll get notfied when a feature changes.
    // Registers an image event handler so we'll get notifed when a new image has been grabbed.
    // Registers a configuration event so we can configure the camrea when a grab is being started.
    // QMutexLocker lock( &m_MemberLock );

    try
    {
        if (devices_Basler.empty()) {
            qDebug() << "没有找到可用的Basler相机设备";
            return;
        }

        qDebug() << "尝试创建相机设备...";
        Pylon::IPylonDevice* pDevice = Pylon::CTlFactory::GetInstance().CreateDevice(devices_Basler[0]);
        if (!pDevice) {
            qDebug() << "创建相机设备失败";
            return;
        }

        qDebug() << "尝试附加相机设备...";
        m_camera.Attach(pDevice, Pylon::Cleanup_Delete);

        qDebug() << "尝试打开相机...";
        m_camera.Open();

        if (m_camera.IsOpen()) {
            qDebug() << "相机打开成功";
        } else {
            qDebug() << "相机打开失败";
        }

        // Get the ExposureTime feature.
        // On GigE cameras, the feature is called ExposureTimeRaw.
        // On USB cameras, it is called ExposureTime.
        if (m_camera.ExposureTime.IsValid())
        {
            // We need the integer representation because the GUI controls can only use integer values.
            // If it doesn't exist, return an empty parameter.
            m_camera.ExposureTime.GetAlternativeIntegerRepresentation( m_exposureTime );
        }
        else if (m_camera.ExposureTimeRaw.IsValid())
        {
            m_exposureTime.Attach( m_camera.ExposureTimeRaw.GetNode() );
        }

        // Get the Gain feature.
        // On GigE cameras, the feature is called GainRaw.
        // On USB cameras, it is called Gain.
        if (m_camera.Gain.IsValid())
        {
            // We need the integer representation for this sample.
            // If it doesn't exist, return an empty parameter.
            m_camera.Gain.GetAlternativeIntegerRepresentation( m_gain );
        }
        else if (m_camera.GainRaw.IsValid())
        {
            m_gain.Attach( m_camera.GainRaw.GetNode() );
        }

        // Add the event handlers that will be called when the feature changes, e.g., its state or value.

        if (m_exposureTime.IsValid())
        {
            // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
            m_camera.RegisterCameraEventHandler( this, m_exposureTime.GetNode()->GetName(), 0, Pylon::RegistrationMode_Append, Pylon::Cleanup_None, Pylon::CameraEventAvailability_Optional );
        }

        if (m_gain.IsValid())
        {
            // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
            m_camera.RegisterCameraEventHandler( this, m_gain.GetNode()->GetName(), 0, Pylon::RegistrationMode_Append, Pylon::Cleanup_None, Pylon::CameraEventAvailability_Optional );
        }

        m_camera.RegisterCameraEventHandler( this, "PixelFormat", 0, Pylon::RegistrationMode_Append, Pylon::Cleanup_None, Pylon::CameraEventAvailability_Optional );

        m_camera.RegisterCameraEventHandler( this, "TriggerMode", 0, Pylon::RegistrationMode_Append, Pylon::Cleanup_None, Pylon::CameraEventAvailability_Optional );

        m_camera.RegisterCameraEventHandler( this, "TriggerSource", 0, Pylon::RegistrationMode_Append, Pylon::Cleanup_None, Pylon::CameraEventAvailability_Optional );


    }
    catch (const Pylon::GenericException& e)
    {
        qDebug() << "打开相机时发生异常：" << e.GetDescription();
        // lock.unlock();
        Close();
        throw;
    }
}

// Perform cleanup and undo everything we did in Open():
// - Stop the grab.
// - Release all images.
// - Deregister event handlers.
// - Free the camera.
void CGuiCamera::closeCamera()
{
    QMutexLocker lock( &m_MemberLock );

    // Stop the grab so the grab thread will not set new m_ptrGrabResult.
    StopGrab();

    // Free the grab result if present.
    m_ptrGrabResult.Release();

    // Free the image by swapping it with a dummy (null) image.
    QImage dummy;
    m_image.swap( dummy );

    // Remove the event handlers that will be called when the feature changes, e.g., its state or value.
    m_camera.DeregisterCameraEventHandler( this, "TriggerSource" );
    m_camera.DeregisterCameraEventHandler( this, "TriggerMode" );
    m_camera.DeregisterCameraEventHandler( this, "PixelFormat" );

    if (m_gain.IsValid())
    {
        // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
        m_camera.DeregisterCameraEventHandler( this, m_gain.GetNode()->GetName() );
    }

    if (m_exposureTime.IsValid())
    {
        // If we must use the alternative integer representation, we don't know the name of the node as it is defined by the camera.
        m_camera.DeregisterCameraEventHandler( this, m_exposureTime.GetNode()->GetName() );
    }

    // Clear the pointers to the features we set manually in Open().
    m_exposureTime.Release();
    m_gain.Release();

    // Close the camera and free all resources.
    m_camera.DestroyDevice();
}

// Start a continuous grab on the camera.开始抓取
void CGuiCamera::ContinuousGrab()
{
    // qDebug() << "相机打开状态:" << m_camera.IsOpen();
    // qDebug() << "相机抓取状态:" << m_camera.IsGrabbing();

    if (!m_camera.IsOpen() || m_camera.IsGrabbing()) {
        qDebug() << "当前相机未打开或图像正在抓取中";
        return;
    }

    try {
        // 设置相机参数
        m_camera.AcquisitionMode.SetValue(Basler_UniversalCameraParams::AcquisitionMode_Continuous);
        
        // 设置像素格式
        if (m_camera.PixelFormat.IsWritable()) {
            m_camera.PixelFormat.SetValue(Basler_UniversalCameraParams::PixelFormat_Mono8);
        }
        
        // 其他参数设置
        m_camera.ExposureAuto.SetValue(Basler_UniversalCameraParams::ExposureAuto_Off);
        m_camera.ExposureTime.SetValue(10000.0);
        m_camera.GainAuto.SetValue(Basler_UniversalCameraParams::GainAuto_Off);
        m_camera.Gain.SetValue(0.0);

        // 开始抓取
        m_camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByInstantCamera);
        
        qDebug() << "相机开始连续采集";
    }
    catch (const Pylon::GenericException& e) {
        qDebug() << "设置相机参数时发生异常：" << e.GetDescription();
    }
}

// Return the converted bitmap.
// Called by the GUI to display the image on the screen.
const QImage& CGuiCamera::GetImage() const
{
    // No need to protect this member as it will only be accessed from the GUI thread.
    return m_image;
}

// 这是要放进线程中的函数
void CGuiCamera::startGrabbing(ImageProcessThread *imageProcessThread){
    // QMutexLocker lock(&m_MemberLock);  // 添加互斥锁保护

    try {
        qDebug() << "开始抓取前检查相机状态:";
        qDebug() << "相机打开状态:" << m_camera.IsOpen();
        qDebug() << "相机抓取状态:" << m_camera.IsGrabbing();

        qDebug() << "开始连续采集图像";
        ContinuousGrab();

        if (!m_camera.IsGrabbing()) {
            qDebug() << "启动图像抓取失败";
            return;
        }
        else{
            qDebug() << "启动图像抓取成功";  // √
        }

        m_imageProcessThread = imageProcessThread; // 保存处理线程的指针
    }
    catch (const Pylon::GenericException& e) {
        qDebug() << "图像抓取时发生异常：" << e.GetDescription();
    }
}

void CGuiCamera::stopGrabbing() {
    qDebug()<<"停止抓取";
    StopGrab();
}

void CGuiCamera::getGrayThreUi(QString &grayThreUi) {
    qDebug() << "getGrayThreUi called in thread:" << QThread::currentThread();
    grayThreHik = grayThreUi;
    qDebug() << "Updated grayThreHik:" << grayThreHik;
}

// 添加 OnImageGrabbed 事件处理函数的实现
void CGuiCamera::OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& ptrGrabResult)
{
    try {
        if (ptrGrabResult->GrabSucceeded()) {
            // 获取原始图像格式
            Pylon::EPixelType pixelType = ptrGrabResult->GetPixelType();
            // qDebug() << "原始图像格式:" << pixelType;

            // 将图像转换为所需的像素格式
            m_formatConverter.Convert(m_ptrGrabBuffer, ptrGrabResult);

            // 获取图像信息
            size_t width = m_ptrGrabBuffer.GetWidth();
            size_t height = m_ptrGrabBuffer.GetHeight();
            size_t paddingX = m_ptrGrabBuffer.GetPaddingX();
            // size_t stride = m_ptrGrabBuffer.GetStride();

            // 创建QImage时确保使用正确的stride
            QImage image(
                static_cast<uchar*>(m_ptrGrabBuffer.GetBuffer()),
                width,
                height,
                // stride,  // 使用实际的stride而不是宽度
                QImage::Format_Grayscale8
            );

            // 创建图像的深拷贝，确保数据独立
            QImage imageCopy = image.copy();

            if (m_imageProcessThread) {
                m_imageProcessThread->onFrameAvailable(imageCopy, grayThreHik, leftUpX,leftUpY,rightDownX,rightDownY,officialNoiseLevel, testNoiseLevel);
                // qDebug() << "图像已发送到处理线程";
            }
        }
        else {
            qDebug() << "Error: " << ptrGrabResult->GetErrorDescription();
        }
    }
    catch (const Pylon::GenericException& e) {
        qDebug() << "处理图像时发生异常：" << e.GetDescription();
    }
}

void CGuiCamera::getLimitArea(int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi){
    leftUpX = leftUpXUi;
    leftUpY = leftUpYUi;
    rightDownX = rightDownXUi;
    rightDownY = rightDownYUi;
}

void CGuiCamera::getNoiseLevel(int officialNoiseLevelUi, int testNoiseLevelUi){
    officialNoiseLevel = officialNoiseLevelUi;
    testNoiseLevel = testNoiseLevelUi;
}


