#include <opencv2/opencv.hpp>
#include <iostream>
//#include <typeinfo> // 包含 typeid 所需的头文件
#include<QMessageBox>
#include <QObject>
#include "Grayscale_centroid.h"

cv::Point2d grayCenter_equal(const cv::Mat& TheImage, int GrayThreshold){
    // 定义返回的形心点
    cv::Point2d Center_equal(0, 0);
    double totalGray_equal = 0;
    double x = 0;
    double y = 0;

    // 遍历图像的每个像素
    for (int i = 0; i < TheImage.cols; ++i)
    {
        for (int j = 0; j < TheImage.rows; ++j)
        {
            double pixelValue = TheImage.at<uchar>(j, i);
            if (pixelValue >= GrayThreshold)
            {
                x += i ;
                y += j ;
                totalGray_equal += 1; // 用于计算形心
            }
        }
    }

    if (totalGray_equal > 0)
    {
        // 基础形心计算
        Center_equal.x = x / totalGray_equal;
        Center_equal.y = y / totalGray_equal;

    }
    // 返回形心坐标
    return Center_equal;
}

cv::Point2d grayCenter(const cv::Mat& TheImage, int GrayThreshold)  //二维点类型，cv::Mat&  cv::Mat 存储图像数据的矩阵类 & 按引用传递参数，避免复制整个图像数据，提高效率
    //声明一个名为grayCenter的函数，该函数接受一个只读的、按引用传递的OpenCV图像对象（cv::Mat），返回一个表示图像中某个点坐标的cv::Point对象
{
    //    // 检查输入图像是否为灰度图像
    //    if (TheImage.channels() != 1) {
    //        throw std::invalid_argument("The input image must be a grayscale image.");
    //    }
    // qDebug()<<"图像宽度："<<TheImage.cols;
    // qDebug()<<"图像长度："<<TheImage.rows;
    // 创建带边界扩展的图像
    int padding = 50; // 边界扩展大小，可以根据光斑大小调整
    cv::Mat paddedImage;
    cv::copyMakeBorder(TheImage, paddedImage, padding, padding, padding, padding, cv::BORDER_REPLICATE);

    // 定义返回的质心点
    cv::Point2d Center(0, 0);
    double totalGray = 0;
    double x = 0;
    double y = 0;


    // 记录边缘情况
    bool touchLeft = false, touchRight = false;
    bool touchTop = false, touchBottom = false;
    int edgePixelCount = 0;
    // double pixelValue_equal = 1.0;
    
    // 遍历图像的每个像素
    for (int i = padding; i < paddedImage.cols - padding; ++i)
    {
        for (int j = padding; j < paddedImage.rows - padding; ++j)
        {
            double pixelValue = paddedImage.at<uchar>(j, i);
            if (pixelValue >= GrayThreshold)
            {
                // 检查是否触及边缘
                if (i - padding == 0) touchLeft = true;
                if (i - padding == TheImage.cols - 1) touchRight = true;
                if (j - padding == 0) touchTop = true;
                if (j - padding == TheImage.rows - 1) touchBottom = true;
                
                // 统计边缘像素
                if (i - padding == 0 || i - padding == TheImage.cols - 1 || 
                    j - padding == 0 || j - padding == TheImage.rows - 1)
                {
                    edgePixelCount++;
                }

                x += (i - padding) * pixelValue;
                y += (j - padding) * pixelValue;
                totalGray += pixelValue; // 用于计算质心

            }
        }
    }

    if (totalGray > 0)
    {
        // 基础质心计算
        Center.x = x / totalGray;
        Center.y = y / totalGray;

        // 根据边缘情况进行补偿
        double compensationFactor = 1.0;
        
        // 如果光斑触及边缘，进行补偿
        if (touchLeft || touchRight || touchTop || touchBottom)
        {
            // 根据边缘像素数量估算光斑被截断的程度
            double visible_ratio = 1.0 - static_cast<double>(edgePixelCount)/totalGray;
            compensationFactor = std::clamp(1.0/visible_ratio, 1.0, 3.0);
            // double visibleRatio = 1.0 - (static_cast<double>(edgePixelCount) / (totalGray / 255.0));
            // compensationFactor = std::min(2.0, 1.0 / visible_ratio); // 限制最大补偿因子为2.0

            // 应用补偿
            if (touchLeft)
            {
                Center.x = std::max(0.0, Center.x - (TheImage.cols - Center.x) * (compensationFactor - 1.0));
            }
            else if (touchRight)
            {
                Center.x = std::min(static_cast<double>(TheImage.cols - 1), 
                                  Center.x + Center.x * (compensationFactor - 1.0));
            }

            if (touchTop)
            {
                Center.y = std::max(0.0, Center.y - (TheImage.rows - Center.y) * (compensationFactor - 1.0));
            }
            else if (touchBottom)
            {
                Center.y = std::min(static_cast<double>(TheImage.rows - 1), 
                                  Center.y + Center.y * (compensationFactor - 1.0));
            }
        }
    }

    // 返回形心坐标
    return Center;
}


// struct GrayCenterResult {
//     cv::Point2d center;
//     double totalGray;
// };

GrayCenterResult grayCenterLimit(const cv::Mat& TheImage, int GrayThreshold, int leftUpX, int leftUpY, int rightDownX, int rightDownY, int officialNoiseLevel,
                                 int testNoiseLevel){
    {
        GrayCenterResult result;
        result.center = cv::Point2d(0, 0);
        result.totalGray = 0;
        result.totalGray_official = 0;
        result.totalGray_test = 0;

        // 限定坐标边界在图像范围内
        leftUpX = std::max(0, leftUpX);
        leftUpY = std::max(0, leftUpY);
        rightDownX = std::min(TheImage.cols - 1, rightDownX);
        rightDownY = std::min(TheImage.rows - 1, rightDownY);
        // qDebug()<<leftUpX;
        // qDebug()<<leftUpY;
        // qDebug()<<rightDownX;
        // qDebug()<<rightDownY;

        // 定义返回的质心点
        // cv::Point2d Center(0, 0);
        // double totalGray = 0;
        double x = 0;
        double y = 0;

        // 遍历图像的每个像素
        for (int i = leftUpX; i < rightDownX; ++i)
        {
            for (int j = leftUpY; j < rightDownY; ++j)
            {
                double pixelValue = static_cast<double>(TheImage.at<uchar>(j, i));
                if (pixelValue >= GrayThreshold)
                {
                    // qDebug()<<pixelValue;
                    // x += i * pixelValue;
                    // y += j * pixelValue;
                    // result.totalGray += pixelValue;
                    x += i ;
                    y += j ;
                    result.totalGray += 1; // 用于计算形心
                    result.totalGray_official += pixelValue - officialNoiseLevel; // 参考镜头去噪后灰度和
                    result.totalGray_test += pixelValue - testNoiseLevel; // 测试镜头去噪后灰度和
                }
            }
        }

        if (result.totalGray > 0)
        {
            result.center.x = x / result.totalGray + 0.5;
            result.center.y = y / result.totalGray + 0.5;
        }

        return result;
    }
}
