/* 时间：2025.2.27
 * 作者：XuXiaohan
 * 功能：海康机器人相机类，相机控制指令、原始图像获取等函数
 * 笔记：这个cpp文件中含有两个类的构造，一个是HikCameraController，一个是ImageGrabber
 * 在图像的显示过程中，前者可实现相机的开始和取流，后者抓取相机数据并发送给ui界面
 * 所以总的子线程的逻辑流程为：
 * ①创建并初始化HikCameraController对象
 * ②槽函数：相机开启按钮->运行HikCameraController对象的opencamera函数，实现取流，注意，此时只是相机开始输出视频，并没有开始抓取
 * ③创建并初始化ImageGrabber对象，该对象内部的startGrabbing()是子线程启动后要持续运行的、创建线程对象t2
 * ④槽函数：t2->启动时->ImageGrabber->startGrabbing()
 * ⑤ImageGrabber->moveTo(t2)
 * ⑥t2->start()
 * 下面的流程是不指定相机的，是可以复用的
 * ⑦创建并初始化ImageProcessThread对象,其中执行灰度质心计算等图像处理操作,注意，不同于笔记本相机，这里的ImageProcessThread具有标准的QImage的输入方式，可以为不同的相机所公用
 * ⑧槽函数：ImageGrabber->图像信号emit imageGrabbed(image.copy())->ImageProcessThread->执行其中的图像处理函数onFrameAvailable
 * ⑧new：注意，上面第⑧条使用槽函数是不合适的，特别是当这个槽函数在主程序mainwindow.cpp中，会导致ImageGrabber类的信号被ImageProcessThread类接收后执行onFrameAvailable()过
 *              程全程在主线程中完成，导致界面卡顿，解决方案是，直接在ImageGrabber类中调用ImageProcessThread类的函数onFrameAvailable()，这样，整个函数的执行过程都在
 *              ImageGrabber类所在的子线程中完成。
 *              但是有一个问题☆☆☆：由于执行完onFrameAvailable()后，ImageProcessThread类需要发送信号给主线程，所以在主线程中我们创建了一个ImageProcessThread类对象imageProcessThread，
 *              用于连接到主ui，这个imageProcessThread必须是唯一的，也就是说，ImageGrabber类在执行ImageProcessThread类中的函数onFrameAvailable()时，一般通过创建对象的方式调用的，但这次
 *              创建对象和主程序的对象不是同一个，会导致这个对象发送的信号无法被主程序接收，进一步导致没有画面显示。解决方案是将主程序的那个唯一可行的对象imageProcessThread通过参数的方式传给
 *              startGrabbing()——————————>如：grabber->startGrabbing(grayThre,imageProcessThread);
 *              这样ImageGrabber类的startGrabbing()函数在需要调用来自ImageProcessThread类的onFrameAvailable()时可以直接通过这个传入的已有对象而不是新建对象。
 *              从而实现整个图像的抓取和处理流程都在子线程中完成，且图像的处理过程不是单独的线程，而是属于图像接收ImageGrabber的过程后半部分，且其处理完数据后可以直接发送信号到主界面。
 * ⑨槽函数：ImageProcessThread->计算以后的图像信号sendSignalToUi->主ui界面this->onFrameAvailable1函数
*/

#include "hikcameracontroller.h"
// #include "hikcameradevice.h"
#include <QDebug>
#include "imageprocessthread.h"

//////////////////////////////// 1. ImageGrabber的构造函数////////////////////////////////
ImageGrabber::ImageGrabber(void* handle, QObject *parent)
    : QObject(parent), handle(handle), isGrabbing(false)
{
}

void ImageGrabber::startGrabbing(ImageProcessThread *imageProcessThread)
{
    isGrabbing = true;
    MV_FRAME_OUT stOutFrame = {0};
    // qDebug() << "Sub-thread ID:" << QThread::currentThreadId();
    while(isGrabbing)
    {
        int nRet = MV_CC_GetImageBuffer(handle, &stOutFrame, 1000);
        // qDebug()<<"while函数下的句柄"<<handle;
        if (nRet == MV_OK)
        {
            // 处理图像数据
            QImage image(stOutFrame.pBufAddr, stOutFrame.stFrameInfo.nWidth,
                        stOutFrame.stFrameInfo.nHeight, QImage::Format_Grayscale8);

            // 发送图像信号
            // emit imageGrabbed(image.copy()); // 创建深拷贝以确保线程安全
            // 这里不通过信号传输给图像处理单元，而是直接调用其公共函数
            imageProcessThread->onFrameAvailable(image,grayThreHik);

            // 释放图像缓存
            nRet = MV_CC_FreeImageBuffer(handle, &stOutFrame);
            if (nRet != MV_OK)
            {
                qDebug("Free Image Buffer fail! nRet [0x%x]\n", nRet);
            }
        }
        else
        {
            qDebug("Get Image fail! nRet [0x%x]\n", nRet);
            break;
        }
    }
    emit finished(); // 暂时没用，注意，接收到这个信号时不能直接删除线程，否则再次点击打开相机时还需要重新创建线程和连接槽函数
}

void ImageGrabber::stopGrabbing()
{
    isGrabbing = false;
}

void ImageGrabber::getGrayThreUi(QString &grayThreUi) {
    qDebug() << "getGrayThreUi called in thread:" << QThread::currentThread();
    grayThreHik = grayThreUi;
    qDebug() << "Updated grayThreHik:" << grayThreHik;
}

//////////////////////// 2. HikCameraController的构造函数/////////////////////////////
HikCameraController::HikCameraController(QObject *parent)
    : QObject(parent), grabber(nullptr), grabberThread(nullptr)
{
}

// 添加析构函数
HikCameraController::~HikCameraController()
{
    closeCamera();
}

void HikCameraController::scanCamera()
{
    // 初始化hik相机的库文件
    MV_CC_Initialize();
    int nRet = MV_OK;
    // unsigned int nIndex = 0;

    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);

    if(nRet != MV_OK){
        qDebug() << "Enum Devices fail! nRet [0x%x]" << nRet;
    }
    if (stDeviceList.nDeviceNum > 0)
    {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
        {
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
            if (pDeviceInfo != NULL){
                QString modelName = QString::fromUtf8((char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
                qDebug() << "[device" << i << "]: Model Name = " << modelName;
                if (modelName.contains("MV-CS", Qt::CaseInsensitive)) // 仅允许 Hikrobot 的MV-CS系列相机
                {
                    PrintDeviceInfo(pDeviceInfo);
                }
                else
                {
                    qDebug() << "该设备不是 Hikrobot 相机，跳过！";
                }
                // PrintDeviceInfo(pDeviceInfo);
            }
            else{
                qDebug()<<"pDeviceInfo == NULL";
            }
        }
        unsigned int nIndex = 0;
        // ch:选择设备并创建句柄 | en:Select device and create handle
        int nRet0 = MV_OK;
        nRet0 = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
        if(nRet0 != MV_OK){
            qDebug()<<"创建句柄失败！";
        }
    }
    else
    {
        qDebug() << "Find No Devices!";
    }
}

// 输出设备信息
void HikCameraController::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
    if (pstMVDevInfo == NULL)
    {
        qDebug()<<"The Pointer of pstMVDevInfo is NULL!";
    }
    /* if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
        printf("CurrentIp: %d.%d.%d.%d\n" , nIp1, nIp2, nIp3, nIp4);
        printf("UserDefinedName: %s\n\n" , pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
    } */
    if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        /// 在Windows上使用Qt创建GUI项目时默认不显示控制台。因此，printf的输出无法看到
        qDebug("    Device Name: %s", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName);
        qDebug("    Serial Number: %s", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
        qDebug("    Device Number: %d\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);

        // 将相机信息输出到combbox中
        QString chModelName = QString::fromUtf8(reinterpret_cast<const char*>(pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName));
        qDebug()<<chModelName;
        emit cameraInfoSignal(chModelName);
    }
    /* else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chSerialNumber);
        printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chSerialNumber);
        printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stCMLInfo.chModelName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chSerialNumber);
        printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stCXPInfo.chModelName);
    }
    else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE)
    {
        printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chUserDefinedName);
        printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chSerialNumber);
        printf("Model Name: %s\n\n", pstMVDevInfo->SpecialInfo.stXoFInfo.chModelName);
    } */
    else
    {
        qDebug()<<"Not USB3.0 support";
    }
}


// 相机参数设置
void HikCameraController::setCameraPara(float expTimeUs, float gainDb, float frameRate)
{
    int nRet = MV_OK;

    // 1. 设置曝光时间（单位：微秒）
    nRet = MV_CC_SetFloatValue(handle, "ExposureTime", expTimeUs);
    if (MV_OK != nRet)
    {
        qDebug("Set ExposureTime fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 2. 设置增益（单位：dB）
    nRet = MV_CC_SetFloatValue(handle, "Gain", gainDb);
    if (MV_OK != nRet)
    {
        qDebug("Set Gain fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 3. 设置帧率（单位：fps）
    nRet = MV_CC_SetFloatValue(handle, "AcquisitionFrameRate", frameRate);
    if (MV_OK != nRet)
    {
        qDebug("Set FrameRate fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 4. 设置自动曝光模式（关闭自动曝光）
    nRet = MV_CC_SetEnumValue(handle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    if (MV_OK != nRet)
    {
        qDebug("Set ExposureAuto fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 5. 设置自动增益模式（关闭自动增益）
    nRet = MV_CC_SetEnumValue(handle, "GainAuto", MV_GAIN_MODE_OFF);
    if (MV_OK != nRet)
    {
        qDebug("Set GainAuto fail! nRet [0x%x]\n", nRet);
        return;
    }

    qDebug("Camera parameters set successfully!");
    qDebug() << "ExposureTime:" << expTimeUs << "us";
    qDebug() << "Gain:" << gainDb << "dB";
    qDebug() << "FrameRate:" << frameRate << "fps";
}

// 打开相机
void HikCameraController::openCamera()
{
    /* bool initSDKSucess = hikCamera->SDK_Init();
    if(initSDKSucess){
        qDebug()<<"SDK初始化成功";
        return true;
    }
    else{
        qDebug()<<"SDK初始化失败";
        return false;
    }*/
    int nRet0 = MV_OK;
    unsigned int nIndex = 0;
    // ch:选择设备并创建句柄 | en:Select device and create handle
    nRet0 = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
    qDebug()<<"创建的句柄："<<handle;
    if (MV_OK != nRet0)
    {
        qDebug("Create Handle fail! nRet [0x%x]\n", nRet0);
    }

    // // 打开相机前先设置相机参数
    // setCameraPara(expTimeUs, gainDb, frameRate);

    // 打开相机
    int nRet = MV_CC_OpenDevice(handle);
    if (MV_OK != nRet)
    {
        qDebug("Open Device fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 设置相机参数
    nRet = MV_CC_SetEnumValue(handle, "PixelFormat", PixelType_Gvsp_Mono8); // 设置像素格式
    if (MV_OK != nRet)
    {
        qDebug("Set PixelFormat fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 设置触发模式为off，使用连续采集
    nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
    if (MV_OK != nRet)
    {
        qDebug("Set TriggerMode fail! nRet [0x%x]\n", nRet);
        return;
    }

    // 开始取流
    nRet = MV_CC_StartGrabbing(handle);
    if (MV_OK != nRet)
    {
        qDebug("Start Grabbing fail! nRet [0x%x]\n", nRet);
        return;
    }

}

// 关闭相机
void HikCameraController::closeCamera()
{
    /* bool FinalizeSDKSucess = hikCamera->SDK_Finalize();
    if(FinalizeSDKSucess){
        qDebug()<<"SDK反初始化成功";
        return true;
    }
    else{
        qDebug()<<"反SDK初始化失败";
        return false;
    } */

    // 停止图像采集
    // if (grabber)
    // {
    //     grabber->stopGrabbing();
    // }

    // if (grabberThread)
    // {
    //     grabberThread->quit();
    //     grabberThread->wait();
    //     grabber = nullptr;
    //     grabberThread = nullptr;
    // }

    // 停止采集
    if (handle)
    {
        int nRet = MV_CC_StopGrabbing(handle);
        if (MV_OK != nRet)
        {
            qDebug("Stop Grabbing fail! nRet [0x%x]\n", nRet);
        }

        // 关闭设备
        nRet = MV_CC_CloseDevice(handle);
        if (MV_OK != nRet)
        {
            qDebug("Close Device fail! nRet [0x%x]\n", nRet);
        }

        // 销毁句柄
        nRet = MV_CC_DestroyHandle(handle);
        if (MV_OK != nRet)
        {
            qDebug("Destroy Handle fail! nRet [0x%x]\n", nRet);
        }
        handle = NULL;
    }
}


