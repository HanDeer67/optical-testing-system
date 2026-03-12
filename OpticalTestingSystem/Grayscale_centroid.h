#ifndef GRAYSCALE_CENTROID_H
#define GRAYSCALE_CENTROID_H
#include<opencv2/opencv.hpp>
#include<QObject>

cv::Point2d grayCenter_equal(const cv::Mat& TheImage, int GrayThreshold);

cv::Point2d grayCenter(const cv::Mat& TheImage, int GrayThreshold);

// 定义结果结构体
struct GrayCenterResult {
    cv::Point2d center;
    double totalGray;
    double totalGray_official;
    double totalGray_test;
};

// 声明函数
GrayCenterResult grayCenterLimit(const cv::Mat& TheImage, int GrayThreshold,
                                 int leftUpX, int leftUpY, int rightDownX, int rightDownY,int officialNoiseLevel,
                                 int testNoiseLevel);
// cv::Point2d grayCenterLimit(const cv::Mat& TheImage, int GrayThreshold, int leftUpX, int leftUpY, int rightDownX, int rightDownY);

#endif // GRAYSCALE_CENTROID_H

