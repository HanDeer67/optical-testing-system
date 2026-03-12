/*2025.2.26
 * 作者：XuXiaohan
 * 该文件是对QImage数据进行处理的类，其中包括项目所需的格式转换、灰度重心计算等函数
*/
#include "imageprocessthread.h"
// #include "hikcameracontroller.h"

ImageProcessThread::ImageProcessThread(QObject *parent)
    : QObject{parent}
{
    // QPixmap pixmap2 = QPixmap();
}

ImageProcessThread::~ImageProcessThread()
{
    // 清理资源
}
void ImageProcessThread::initialize()
{
    // 可以进行一些初始化工作
}

// 当有新的视频帧时调用
void ImageProcessThread::onFrameAvailable(QImage& image , QString &grayThre, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi,int officialNoiseLevel, int testNoiseLevel) {
    // qDebug() << "Sub-thread ID:" << QThread::currentThreadId();
    // qDebug() << "Frame format:" << frame.pixelFormat()
    //          << "Size:" << frame.width() << "x" << frame.height()
    //          << "Valid:" << frame.isValid();
    // qDebug()<<image;
    // QImage(QSize(1280, 1024),format=QImage::Format_Grayscale16,depth=16,devicePixelRatio=1,bytesPerLine=2560,sizeInBytes=2621440)
    // QImage(QSize(1920, 1200),format=QImage::Format_Grayscale8,depth=8,devicePixelRatio=1,bytesPerLine=1920,sizeInBytes=2304000)
    try{
        if (image.isNull()) {
            emit error("Input image is null");
            return;
        }

        // qDebug() << "Image format:" << image.format()
        //          << "Size:" << image.width() << "x" << image.height()
        //          << "Depth:" << image.depth()
        //          << "BytesPerLine:" << image.bytesPerLine();

        // 创建图像的深拷贝，确保线程安全
        QImage imageCopy = image.copy();
        if (imageCopy.isNull()) {
            emit error("Failed to create image copy");
            return;
        }

        QPixmap pixmap3 = QPixmap::fromImage(imageCopy);
        if (pixmap3.isNull()) {
            emit error("Failed to convert image to pixmap");
            return;
        }
        // qDebug()<<"后："<<pixmap3;

        bool ok;
        double threshold = grayThre.toDouble(&ok);
        // qDebug()<<"灰度阈值："<<threshold;
        if (!ok) {
            emit error("Invalid gray threshold value: " + grayThre);
            return;
        }
        // qDebug() << "Processing with threshold:" << threshold;

        // 计算质心
        calculateCentroidNew(imageCopy, threshold, pixmap3, leftUpXUi, leftUpYUi, rightDownXUi, rightDownYUi,officialNoiseLevel, testNoiseLevel);
        // qDebug()<<"计算质心完成";
        emit processingFinished();

    } catch (const std::exception &e) {
        emit error(QString("Exception in frame processing: %1").arg(e.what()));
    } catch (...) {
        emit error("Unknown exception in frame processing");
    }
}

// 质心计算函数
void ImageProcessThread::calculateCentroidNew(const QImage &image, const double &grayThre, QPixmap &pixmapNew, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi
                                              ,int officialNoiseLevel, int testNoiseLevel)
{
    try {
        if (image.isNull()) {
            emit error("Input image is null");
            return;
        }

        QImage grayImage;
        if (image.format() != QImage::Format_Grayscale8) {
            grayImage = image.convertToFormat(QImage::Format_Grayscale8);
        } else {
            grayImage = image;
        }

        if (grayImage.isNull()) {
            emit error("Failed to convert image to grayscale");
            return;
        }


        QImage imageCopy = grayImage.copy();
        cv::Mat matImage = QImageToMat(imageCopy);
        if (matImage.empty()) {
            emit error("Failed to convert QImage to cv::Mat");
            return;
        }

        QImage imageCopy1 = grayImage.copy();
        cv::Mat matImage1 = QImageToMat(imageCopy1);
        if (matImage1.empty()) {
            emit error("Failed to convert QImage to cv::Mat");
            return;
        }

        cv::Point2d centerPoint = grayCenter_equal(matImage, grayThre);
        // cv::Point2d centerPointLimit = grayCenterLimit(matImage1, grayThre, leftUpXUi, leftUpYUi, rightDownXUi, rightDownYUi);
        GrayCenterResult res = grayCenterLimit(matImage1, grayThre, leftUpXUi, leftUpYUi, rightDownXUi, rightDownYUi,officialNoiseLevel,testNoiseLevel); //方框区域内的质心计算

        
        // 检查质心计算结果是否有效
        if (centerPoint.x < 0 || centerPoint.y < 0 || 
            centerPoint.x >= image.width() || centerPoint.y >= image.height()) {
            emit error("Invalid centroid coordinates calculated");
            return;
        }

        if (pixmapNew.isNull()) {
            emit error("Pixmap is null");
            return;
        }

        // 发送结果到UI
        // cv::Point2d pt = res.center;
        // qDebug() << "center: (" << pt.x << "," << pt.y << ")";
        // qDebug()<<res.totalGray;
        emit sendSignalToUi(pixmapNew, centerPoint,res.center,res.totalGray_official,res.totalGray_test);

    } catch (const std::exception &e) {
        emit error(QString("Exception in centroid calculation: %1").arg(e.what()));
    } catch (...) {
        emit error("Unknown exception in centroid calculation");
    }
}

cv::Mat ImageProcessThread::QImageToMat(const QImage &image)
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
