#include "saperacameracontroller.h"
// 包含 Sapera 头文件
#include "SapClassBasic.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>

SaperaCameraController::SaperaCameraController(QObject *parent)
    : QObject{parent},
    // m_grabTimer(new QTimer(this)),
    m_Acq(nullptr),
    m_Buffers(nullptr),
    m_Xfer(nullptr),
    m_View(nullptr),
    m_isAcquiring(false),
    m_scene(new QGraphicsScene(this)),  // Initialize m_scene
    m_graphicsView(nullptr)  // Initialize m_graphicsView to nullptr

{

    // 连接图像更新信号
    // connect(this, &SaperaCameraController::imageReady, this, &SaperaCameraController::updateDisplay, Qt::QueuedConnection);

    // 创建用于获取图像的定时器
    m_grabTimer = new QTimer(this);
    connect(m_grabTimer, &QTimer::timeout, this, &SaperaCameraController::grabFrame);
}
// 测试
// void SaperaCameraController::updateDisplay(const QImage& image)
// {
//     if (image.isNull())
//         return;

//     // 更新图形视图
//     m_scene->clear();
//     m_scene->addPixmap(QPixmap::fromImage(image));

//     // // 使用 Qt::KeepAspectRatio 来保持图像比例
//     m_graphicsView->setSceneRect(m_scene->itemsBoundingRect());
//     m_graphicsView->fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
// }

void SaperaCameraController::getCcfPath(QString ccfPath){
    ccfPathRec = ccfPath;
    qDebug()<<"cffPathRec:"<<ccfPathRec;
}

// 扫描系统中可用的 CameraLink 相机
void SaperaCameraController::refreshCameraList()
{
    // 停止当前采集
    stopAcquisition();
    destroySaperaObjects();

    // 使用 Sapera 获取可用设备, 获取系统中所有 Sapera 服务器（相机+采集卡组合）
    UINT32 serverCount = SapManager::GetServerCount();

    for (UINT32 i = 0; i < serverCount; i++)
    {
        char serverName[CORSERVER_MAX_STRLEN];
        if (SapManager::GetServerName(i, serverName))
        {
            // 检查服务器是否可用,即检查每个服务器是否具有采集功能 - 修改为使用 SapManager::GetResourceCount 来验证服务器可用性
            UINT32 resourceCount = SapManager::GetResourceCount(serverName, SapManager::ResourceAcq);
            if (resourceCount > 0)
            {
                // 这是一个可用的相机
                QString name = QString::fromLatin1(serverName);
                m_cameraNames.append(name);
            }
        }
    }
    // 发送加载有相机信息的信号到主ui界面
    emit updateCameraListSignal(m_cameraNames);
    if (m_cameraNames.isEmpty())
    {
        QString warningText = "未检测到任何 CameraLink 相机";
        emit warningSaperaSignal(warningText);
        qDebug()<<"未检测到任何 CameraLink 相机";
        // QMessageBox::information(this, "提示", "未检测到任何 CameraLink 相机");
    }
}


// 当用户选择不同相机时调用，初始化相机资源
void SaperaCameraController::onCameraSelected(int index)
{
    // 停止当前采集
    stopAcquisition();
    destroySaperaObjects();

    if (index >= 0 && index < m_cameraNames.size())
    {
        // 创建新的相机对象
        QString cameraName = m_cameraNames.at(index);
        if (!createSaperaObjects(cameraName))
        {
            // QMessageBox::critical(this, "错误", "创建相机资源失败");
            emit warningSaperaSignal("创建相机资源失败");
        }
        else
        {
            // 检查信号状态
            getSignalStatus();
        }
    }
}

// 为选定的相机创建必要的 Sapera 对象
bool SaperaCameraController::createSaperaObjects(const QString &cameraName)
{
    qDebug() << "开始创建 Sapera 对象...";

    // 销毁现有对象
    destroySaperaObjects();

    // 创建采集对象
    m_Acq = new SapAcquisition(cameraName.toLatin1().data());

    // // 添加 CCF 文件支持
    // QString ccfPath = QFileDialog::getOpenFileName(this, "选择相机配置文件",
    //                                                QString(), "相机配置文件 (*.ccf)");
    if (!ccfPathRec.isEmpty())
    {
        qDebug()<<"成功导入ccf文件";
        // 使用配置文件
        delete m_Acq;  // 删除原先创建的对象
        m_Acq = new SapAcquisition(cameraName.toLatin1().data(), ccfPathRec.toLatin1().data());
    }
    else{
        qDebug()<<"ccf为空";
    }

    if (!m_Acq->Create())
    {
        qDebug() << "采集对象创建失败";
        emit warningSaperaSignal("无法创建采集对象");
        // QMessageBox::critical(this, "错误", "无法创建采集对象");
        destroySaperaObjects();
        return false;
    }
    qDebug() << "采集对象创建成功";

    // 创建缓冲区 - 使用带 trash 的缓冲区，与官方示例一致
    m_Buffers = new SapBuffer(2, m_Acq); // 2个缓冲区用于双缓冲
    if (!m_Buffers->Create())
    {
        emit warningSaperaSignal("无法创建缓冲区");
        // QMessageBox::critical(this, "错误", "无法创建缓冲区");
        destroySaperaObjects();
        return false;
    }

    // 验证缓冲区尺寸
    UINT32 bufferWidth = m_Buffers->GetWidth();
    UINT32 bufferHeight = m_Buffers->GetHeight();
    qDebug() << "缓冲区尺寸:" << bufferWidth << "x" << bufferHeight;

    // 创建视图对象
    m_View = new SapView(m_Buffers);
    if (!m_View->Create())
    {
        emit warningSaperaSignal("无法创建视图对象");
        // QMessageBox::critical(this, "错误", "无法创建视图对象");
        destroySaperaObjects();
        return false;
    }

    // 创建传输对象 - 使用 SapAcqToBuf 类与官方示例一致
    m_Xfer = new SapAcqToBuf(m_Acq, m_Buffers, XferCallback, this);
    if (!m_Xfer->Create())
    {
        emit warningSaperaSignal("无法创建传输对象");
        // QMessageBox::critical(this, "错误", "无法创建传输对象");
        destroySaperaObjects();
        return false;
    }

    return true;
}

// 释放 Sapera 资源
void SaperaCameraController::destroySaperaObjects()
{
    // 按照官方示例的销毁顺序
    if (m_Xfer)
    {
        m_Xfer->Destroy();
        delete m_Xfer;
        m_Xfer = nullptr;
    }

    if (m_View)
    {
        m_View->Destroy();
        delete m_View;
        m_View = nullptr;
    }

    if (m_Buffers)
    {
        m_Buffers->Destroy();
        delete m_Buffers;
        m_Buffers = nullptr;
    }

    if (m_Acq)
    {
        m_Acq->Destroy();
        delete m_Acq;
        m_Acq = nullptr;
    }
}

// 检查 Sapera 对象是否创建成功
bool SaperaCameraController::isSaperaObjectsCreated() const
{
    return m_Acq != nullptr && m_Buffers != nullptr && m_Xfer != nullptr && m_View != nullptr;
}

// 开始连续图像采集
void SaperaCameraController::startAcquisition()
{
    if (!isSaperaObjectsCreated() || m_isAcquiring){
        qDebug()<<"startAcquisition失败";
        return;
    }

    // 开始采集 - 使用 Grab() 方法
    if (m_Xfer->Grab())
    {
        qDebug()<<"开始抓取";
        m_isAcquiring = true;
    }
    else
    {
        emit warningSaperaSignal("开始采集失败");
        qDebug()<<"开始采集失败";
        // QMessageBox::critical(this, "错误", "开始采集失败");
    }
}

// 停止图像采集
void SaperaCameraController::stopAcquisition()
{
    if (!m_isAcquiring || !m_Xfer)
        return;

    // 停止采集 - 使用 Freeze() 方法
    m_Xfer->Freeze();
    m_isAcquiring = false;
}

// 定时器触发函数，但实际上不需要操作，因为使用了回调机制
void SaperaCameraController::grabFrame()
{
    // 此函数在定时器触发时调用，但我们将依赖回调函数
    // 所以这里不需要主动获取图像
}

// 图像传输完成后的回调函数，处理新图像
void SaperaCameraController::XferCallback(SapXferCallbackInfo *pInfo)
{

    // 修正：将context转换为SaperaCameraController而非MainWindow
    SaperaCameraController *pThis = static_cast<SaperaCameraController*>(pInfo->GetContext());

    if (pThis)
    {
        // 检查是否是垃圾缓冲区采集
        if (pInfo->IsTrash())
        {
            // 如果是垃圾缓冲区采集，可以显示计数信息
            QString str = QString("在垃圾缓冲区中采集的帧数: %1").arg(pInfo->GetEventCount());
            qDebug() << str;
        }
        else
        {
            // // 正常显示图像
            // if (pThis->m_View)
            // {
            //     pThis->m_View->Show();
            // }

            // 获取图像数据进行额外处理
            SapBuffer *pBuffer = pThis->m_Buffers;
            if (pBuffer)
            {
                // 获取图像数据
                void *pData = nullptr;
                if (pBuffer->GetAddress(&pData))
                {
                    pThis->processFrame(pData);
                }
            }
        }
    }
}

// 相机信号状态变化时的回调函数
void SaperaCameraController::SignalCallback(SapAcqCallbackInfo *pInfo)
{
    // 修正：将context转换为SaperaCameraController而非MainWindow
    SaperaCameraController *pThis = static_cast<SaperaCameraController*>(pInfo->GetContext());
    if (pThis)
    {
        pThis->getSignalStatus(pInfo->GetSignalStatus());
    }
}

// 处理原始图像数据并转换为 Qt 图像用于显示
void SaperaCameraController::processFrame(void *pData)
{
    if (!pData || !m_Buffers)
        return;
    // 获取图像信息
    UINT32 width = m_Buffers->GetWidth();
    UINT32 height = m_Buffers->GetHeight();
    SapFormat format = m_Buffers->GetFormat();
    // qDebug()<<"宽："<<width;
    // qDebug()<<"高："<<height;
    int bytesPerPixel = 2; // 先设置为2，后面会根据格式精确计算

    /// **************保存原始RAW数据****************
    QMutexLocker locker(&rawMutex); // 加锁保护共享资源
    if(remainingRawNum > 0){
        qDebug()<<"remainingRawNum"<<remainingRawNum;
        // 计算正确的 bytesPerPixel 用于保存
        switch (format) {
        case SapFormatMono8:
            bytesPerPixel = 1;
            break;
        case SapFormatMono10:
        case SapFormatMono12:
        case SapFormatMono14:
        case SapFormatMono16:
            // 注意！即使是10位、12位，相机传输时通常也占用2个字节（打包或填充）
            bytesPerPixel = 2;
            break;
        case SapFormatRGB888:
            bytesPerPixel = 3;
            break;
        default:
            bytesPerPixel = 1; // 安全默认值
            break;
        }
        QString filename = rawFilesPath + QString("/frame_%1.raw").arg(totalRawNum - remainingRawNum + 1, 4, 10, QChar('0'));
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(reinterpret_cast<const char*>(pData), width * height * bytesPerPixel);
            file.close();
            qDebug() << "Saved raw frame:" << filename;
        }else {
            qDebug() << "Failed to open file for writing:" << filename;
        }
        remainingRawNum -= 1;
    }
    locker.unlock();
    /// **************保存原始RAW数据****************

    // 根据格式创建 QImage
    QImage::Format qFormat;

    switch (format)
    {
    case SapFormatMono8:
        qFormat = QImage::Format_Grayscale8;
        bytesPerPixel = 1;
        break;
    case SapFormatMono10:
    case SapFormatMono12:
    case SapFormatMono14:
    case SapFormatMono16:
        qFormat = QImage::Format_Grayscale16;
        bytesPerPixel = 2;
        // 对于10位和12位数据，需要进行位移操作以适应16位格式
        if (format == SapFormatMono10)
        {
            qDebug()<<"位深：10";
            // 10位数据左移6位
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 6;
            }
        }
        else if (format == SapFormatMono12)
        {
            // 12位数据左移4位
            qDebug()<<"位深：12";
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 4;
            }
        }
        else if (format == SapFormatMono14)
        {
            qDebug()<<"位深：14";
            // 14位数据左移2位
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 2;
            }
        }
        // else qDebug()<<"位深：16";
        break;

    case SapFormatRGB888:
        qFormat = QImage::Format_RGB888;
        bytesPerPixel = 3;
        break;

    default:
        qDebug() << "Unsupported format:" << format;
        return;
    }

    // 从原始数据创建 QImage
    // QImage img((uchar*)pData, width, height, width * bytesPerPixel, qFormat);
    // 从原始数据创建 QImage
    QImage img = QImage((uchar*)pData, width, height, width * bytesPerPixel, qFormat).copy();

    // 发送信号到主线程更新显示
    // emit imageReady(img);

    // 保存图像副本（防止数据被覆盖）
    QImage imageCopy = img.copy();
    QImage imageCopy1 = img.copy();

    // 将图像传递给图像处理线程进行处理
    // qDebug()<<"开始输出图像数据，执行后续图像处理步骤";
    // qDebug()<<"imageCopy宽"<<imageCopy.width();
    // qDebug()<<"imageCopy高"<<imageCopy.height();
    // qDebug()<<"灰度"<<grayThreHik;

    QImage displayImg = convertTo8Bit(imageCopy, minValGet, maxValGet); // 8位灰度的QImage格式数据
    QImage displayImg1 = convertTo8Bit(imageCopy1, minValGet, maxValGet); // 8位灰度的QImage格式数据
    // m_imageProcessThread->onFrameAvailable(displayImg, grayThreHik);
    // 计算图像质心
    cv::Mat matImage = QImageToMat(displayImg);
    if (matImage.empty()) {
        return;
    }
    cv::Mat matImage1 = QImageToMat(displayImg1);
    if (matImage1.empty()) {
        return;
    }
    cv::Point2d centerPoint = grayCenter(matImage, grayThre);
    GrayCenterResult res = grayCenterLimit(matImage1, grayThre, leftUpX,leftUpY,rightDownX,rightDownY,officialNoiseLevel, testNoiseLevel); //方框区域内的质心计算
    // 直接将QImage发送给ui界面
    emit cameralinkSignal(displayImg, centerPoint, res.center, res.totalGray_official,res.totalGray_test);
    // 类似于HikCamera中的处理方式
    // if (m_imageProcessThread) {
    //     qDebug()<<"开始输出图像数据，执行后续图像处理步骤";
    //     m_imageProcessThread->onFrameAvailable(imageCopy, grayThreHik);
    // }
}

// 处理原始图像数据并转换为 Qt 图像用于显示
void SaperaCameraController::processFrameTest(void *pData, UINT32 width, UINT32 height, SapFormat format)
{
    if (!pData){
        qDebug()<<"!pData";
        return;
    }

    qDebug()<<"宽："<<width;
    qDebug()<<"高："<<height;
    int bytesPerPixel = 2; // 先设置为2，后面会根据格式精确计算

    /// **************保存原始RAW数据****************
    QMutexLocker locker(&rawMutex); // 加锁保护共享资源
    if(remainingRawNum > 0){
        qDebug()<<"remainingRawNum"<<remainingRawNum;
        // 计算正确的 bytesPerPixel 用于保存
        switch (format) {
        case SapFormatMono8:
            bytesPerPixel = 1;
            break;
        case SapFormatMono10:
        case SapFormatMono12:
        case SapFormatMono14:
        case SapFormatMono16:
            // 注意！即使是10位、12位，相机传输时通常也占用2个字节（打包或填充）
            bytesPerPixel = 2;
            break;
        case SapFormatRGB888:
            bytesPerPixel = 3;
            break;
        default:
            bytesPerPixel = 1; // 安全默认值
            break;
        }
        QString filename = rawFilesPath + QString("/frame_%1.raw").arg(totalRawNum - remainingRawNum + 1, 4, 10, QChar('0'));
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(reinterpret_cast<const char*>(pData), width * height * bytesPerPixel);
            file.close();
            qDebug() << "Saved raw frame:" << filename;
        }else {
            qDebug() << "Failed to open file for writing:" << filename;
        }
        remainingRawNum -= 1;
    }
    locker.unlock();
    /// **************保存原始RAW数据****************

    // 根据格式创建 QImage
    QImage::Format qFormat;

    switch (format)
    {
    case SapFormatMono8:
        qFormat = QImage::Format_Grayscale8;
        bytesPerPixel = 1;
        break;
    case SapFormatMono10:
    case SapFormatMono12:
    case SapFormatMono14:
    case SapFormatMono16:
        qFormat = QImage::Format_Grayscale16;
        bytesPerPixel = 2;
        // 对于10位和12位数据，需要进行位移操作以适应16位格式
        if (format == SapFormatMono10)
        {
            qDebug()<<"位深：10";
            // 10位数据左移6位
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 6;
            }
        }
        else if (format == SapFormatMono12)
        {
            // 12位数据左移4位
            qDebug()<<"位深：12";
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 4;
            }
        }
        else if (format == SapFormatMono14)
        {
            qDebug()<<"位深：14";
            // 14位数据左移2位
            uint16_t* pSrc = (uint16_t*)pData;
            for (UINT32 i = 0; i < width * height; i++)
            {
                pSrc[i] = pSrc[i] << 2;
            }
        }
        // else qDebug()<<"位深：16";
        break;

    case SapFormatRGB888:
        qFormat = QImage::Format_RGB888;
        bytesPerPixel = 3;
        break;

    default:
        qDebug() << "Unsupported format:" << format;
        return;
    }

    // 从原始数据创建 QImage
    // QImage img((uchar*)pData, width, height, width * bytesPerPixel, qFormat);
    // 从原始数据创建 QImage
    QImage img = QImage((uchar*)pData, width, height, width * bytesPerPixel, qFormat).copy();

    // 发送信号到主线程更新显示
    // emit imageReady(img);

    // 保存图像副本（防止数据被覆盖）
    QImage imageCopy = img.copy();
    QImage imageCopy1 = img.copy();

    // 将图像传递给图像处理线程进行处理
    // qDebug()<<"开始输出图像数据，执行后续图像处理步骤";
    // qDebug()<<"imageCopy宽"<<imageCopy.width();
    // qDebug()<<"imageCopy高"<<imageCopy.height();
    // qDebug()<<"灰度"<<grayThreHik;

    QImage displayImg = convertTo8Bit(imageCopy, minValGet, maxValGet); // 8位灰度的QImage格式数据
    QImage displayImg1 = convertTo8Bit(imageCopy1, minValGet, maxValGet); // 8位灰度的QImage格式数据
    // m_imageProcessThread->onFrameAvailable(displayImg, grayThreHik);
    // 计算图像质心
    cv::Mat matImage = QImageToMat(displayImg);
    if (matImage.empty()) {
        return;
    }
    cv::Mat matImage1 = QImageToMat(displayImg1);
    if (matImage1.empty()) {
        return;
    }
    cv::Point2d centerPoint = grayCenter(matImage, grayThre);
    GrayCenterResult res = grayCenterLimit(matImage1, grayThre, leftUpX,leftUpY,rightDownX,rightDownY,officialNoiseLevel, testNoiseLevel); //方框区域内的质心计算
    // 直接将QImage发送给ui界面
    emit cameralinkSignal(displayImg, centerPoint, res.center, res.totalGray_official,res.totalGray_test);
    // 类似于HikCamera中的处理方式
    // if (m_imageProcessThread) {
    //     qDebug()<<"开始输出图像数据，执行后续图像处理步骤";
    //     m_imageProcessThread->onFrameAvailable(imageCopy, grayThreHik);
    // }
}

// 在 SaperaCameraController 里添加一个函数
void SaperaCameraController::simulateFrame()
{
    UINT32 width = 640;
    UINT32 height = 512;
    // int bytesPerPixel = 2; // 模拟16位数据
    SapFormat format = SapFormatMono16;

    // 分配一块内存，生成测试图像
    std::vector<uint16_t> buffer(width * height);
    for (UINT32 y = 0; y < height; y++) {
        for (UINT32 x = 0; x < width; x++) {
            buffer[y * width + x] = (x + y) % 65536; // 简单生成个梯度图
        }
    }
    qDebug()<<"模拟测试";
    // 调用真实的处理函数
    processFrameTest(buffer.data(), width, height, format);
}


// void SaperaCameraController::updateGrayThre(double grayThreUi, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi){
void SaperaCameraController::updateGrayThre(double grayThreUi){
    grayThre = grayThreUi;

}

cv::Mat SaperaCameraController::QImageToMat(const QImage &image)
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

void SaperaCameraController::saveRaw(int saveRawNum, QString rawFilesPathGet)
{
    QMutexLocker locker(&rawMutex); // 加锁
    // 缺少输入验证
    if(saveRawNum <= 0) {
        qDebug() << "Invalid save number";
        return;
    }
    qDebug()<<"saveRawNum"<<saveRawNum;
    qDebug()<<"rawFilesPathGet"<<rawFilesPathGet;
    remainingRawNum = saveRawNum;
    totalRawNum = saveRawNum;
    rawFilesPath = rawFilesPathGet;
}


void SaperaCameraController::getMinMaxVal(int minValUiSent, int maxValUiSent){
    minValGet = minValUiSent;
    maxValGet = maxValUiSent;
}

void SaperaCameraController::getLimitArea(int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi){
    leftUpX = leftUpXUi;
    leftUpY = leftUpYUi;
    rightDownX = rightDownXUi;
    rightDownY = rightDownYUi;
}

QImage SaperaCameraController::convertTo8Bit(const QImage& image16 , int minValUi, int maxValUi)
{
    if(image16.format() != QImage::Format_Grayscale16){
        return image16;
    }
    int width = image16.width();
    int height = image16.height();
    // qDebug()<<"width:"<<width; // 640
    // qDebug()<<"height:"<<height; // 512
    // qDebug()<<image16; // QImage(QSize(640, 512),format=QImage::Format_Grayscale16,
    //                     // depth=16,devicePixelRatio=1,bytesPerLine=1280,sizeInBytes=655360)
    const uchar* srcBits = image16.bits();


    QImage image8(width,height,QImage::Format_Grayscale8);


    uchar* dstBits = image8.bits();

    // 寻找最大最小值（增强对比度用）
    const quint16* src16 = reinterpret_cast<const quint16*>(srcBits);
    quint16 minVal = minValUi, maxVal = maxValUi;  //16383
    // qDebug()<<"minVal:"<<minVal;
    // qDebug()<<"maxVal:"<<maxVal;

    // for (int i = 0; i < width * height; ++i) {
    //     if (src16[i] < minVal) minVal = src16[i]; // 4624
    //     if (src16[i] > maxVal) maxVal = src16[i]; // 0
    // }

    float scale = (maxVal != minVal) ? 255.0f / (maxVal - minVal) : 1.0f;

    for (int i = 0; i < width * height; ++i) {
        quint16 val = src16[i];
        uchar val8 = static_cast<uchar>((val - minVal) * scale);
        dstBits[i] = val8;
    }

    return image8;
}

void SaperaCameraController::getGrayThreUi(QString &grayThreUi) {
    qDebug() << "getGrayThreUi called in thread:" << QThread::currentThread();
    grayThreHik = grayThreUi;
    qDebug() << "Updated grayThre:" << grayThreHik;
}

// 获取和显示相机信号状态
void SaperaCameraController::getSignalStatus()
{
    SapAcquisition::SignalStatus signalStatus;

    if (m_Acq && m_Acq->IsSignalStatusAvailable())
    {
        if (m_Acq->GetSignalStatus(&signalStatus, SignalCallback, this))
            getSignalStatus(signalStatus);
    }
}

void SaperaCameraController::getSignalStatus(SapAcquisition::SignalStatus signalStatus)
{
    bool isSignalDetected = (signalStatus != SapAcquisition::SignalNone);

    if (isSignalDetected)
        emit barShowSaperaSignal("检测到Cameralink相机信号");
    else
    {
        emit barShowSaperaSignal("未检测到相机信号");
    }
}

void SaperaCameraController::setGraphicsView(QGraphicsView* view)
{
    m_graphicsView = view;
    if (m_graphicsView) {
        m_graphicsView->setScene(m_scene);  // Set the scene for the view
    }
}

void SaperaCameraController::getNoiseLevel(int officialNoiseLevelUi, int testNoiseLevelUi){
    officialNoiseLevel = officialNoiseLevelUi;
    testNoiseLevel = testNoiseLevelUi;
}

