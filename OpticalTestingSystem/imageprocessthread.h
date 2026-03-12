#ifndef IMAGEPROCESSTHREAD_H
#define IMAGEPROCESSTHREAD_H

#include <QObject>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QPixmap>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "opencv2/opencv.hpp"
#include "Grayscale_centroid.h"

class ImageProcessThread : public QObject
{
    Q_OBJECT
public:
    explicit ImageProcessThread(QObject *parent = nullptr);
    ~ImageProcessThread();

public:
    void onFrameAvailable(QImage& image, QString &grayThre, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi,int officialNoiseLevel, int testNoiseLevel);

public slots:
    void initialize();

signals:
    void sendSignalToUi(const QPixmap &pixmap, const cv::Point2d &centerPoint, const cv::Point2d &centerPointLimit, double totalGray_official, double totalGray_test);
    void error(const QString &message);
    void processingFinished();

private:
    // mutable QMutex mutex;
    QMutex mutex;
    QPixmap pixmap;
    // QPixmap pixmap2;
    void calculateCentroidNew(const QImage &image, const double &grayThre, QPixmap &pixmapNew, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi,int officialNoiseLevel, int testNoiseLevel);
    cv::Mat QImageToMat(const QImage &image);
};

#endif // IMAGEPROCESSTHREAD_H
