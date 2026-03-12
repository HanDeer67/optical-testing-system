#ifndef NEWGRAYSCALECENTROID_H
#define NEWGRAYSCALECENTROID_H

#include <QObject>
#include<opencv2/opencv.hpp>

class newGrayscaleCentroid : public QObject
{
    Q_OBJECT
public:
    explicit newGrayscaleCentroid(QObject *parent = nullptr);

    cv::Point2d grayCenter(const cv::Mat& TheImage, int GrayThreshold);
    cv::Point2d grayCenterLimit(const cv::Mat& TheImage, int GrayThreshold, int leftUpX, int leftUpY, int rightDownX, int rightDownY);

signals:
};

#endif // NEWGRAYSCALECENTROID_H
