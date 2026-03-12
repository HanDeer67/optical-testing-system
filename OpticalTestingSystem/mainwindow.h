#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialog_def_pix_size.h"
// #include "hikcameracontroller.h"
#include "image_thread.h"
#include "newgraphicsview.h" // 引入自定义的 GraphicsView 类
#include "chartsdialog.h"
// #include "hikparasetdialog.h"
#include <opencv2/opencv.hpp>
// #include "Grayscale_centroid.h"
#include "chartviewnew.h"

//相机图像头文件
#include <QCamera>            //启动或停止摄像头、设置摄像头属性（如分辨率、帧率等），并通过它捕获实时视频流。
// #include <QCameraViewfinder>  //显示相机的实时视频流。它可以作为相机的取景器，将视频内容显示在界面上 适用于qt5版本
// #include <QCameraImageCapture>
// #include <QVideoWidget>
#include <QMediaCaptureSession>  //qt6
#include <QVideoSink>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QPixmap>
#include <QLabel>
#include <QTableWidget>
#include <QtCharts>
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QCursor>

#include "baslercameracontroller.h"
// 包含 Sapera 头文件
#include "SapClassBasic.h"
#include "saperacameracontroller.h"
#include "newgrayscalecentroid.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 在 MainWindow 类定义之前添加这个结构
struct TestReportData {
    // 基本信息
    QString objectName;
    QString objectProperties;
    QString testDate;
    
    // 视场数据
    struct {
        QString up;
        QString down;
        QString left;
        QString right;
        QString elevViewAng;
        QString horiViewAng;
    } fov;
    
    // 焦距数据
    struct {
        QString average;
        QStringList samples;
    } focalLength;
    
    // 畸变数据
    struct {
        QStringList parameters;
        QList<QStringList> tableData;
    } distortion;
    
    // MTF数据
    struct {
        QStringList parameters;
        QList<QStringList> tableData;
        QString cameraImage;
        QStringList chartImages;
    } mtf;
    
    // 透过率数据
    QStringList transmittance;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void aboutToClose(); // 用于在程序关闭时终止线程的信号，即程序每次退出并析构时，发送该信号
    void min2max(bool action1);

    void updateGrayThre(QString &grayThreUi);
    // void updateGrayThreSapera(double grayThreUi, int leftUpXUi, int leftUpYUi, int rightDownXUi, int rightDownYUi);
    void updateGrayThreSapera(double grayThreUi);

    void ccfPathSignal(QString ccfPath);
    void saveRawSignal(int saveRawNum, QString rawFilesPath);

public:
    MainWindow(QWidget *parent = nullptr);
// public:
//     explicit MainWindow(QWidget *parent = nullptr) : QMainWindow(parent), resizing(false) {
        // setMouseTracking(true); // 开启鼠标追踪
    // }
    ~MainWindow();

    void onPointHovered(const QPointF &point, bool state);  // 槽函数
    // // 自定义的窗口控制按钮
    // void minimizeWindow();
    // void maximizeWindow();
    // void closeWindow();
    void handleError(const QString &message);
    int windowWidthTrans0 = 1920;
    int windowHeightTrans0 = 1080;
    int windowWidthTrans = 1920 * 0.8;
    int windowHeightTrans = 1080 * 0.8;


private:
    QTimer *timer1 = new QTimer;
    QElapsedTimer lastImageUpdateTimer; // 记录最后图像更新时间
    bool timerStopped;            // 标志定时器是否已停止
    enum ResizeDirection { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeDirection resizeDirection = None;
    bool resizing;
    QPoint lastMousePos;

    float circleRect = 20;
    float circleRectLimit = 20;
    float rectLeftUpX = 0;
    float rectLeftUpY = 0;
    float rectRightDownX = 0;
    float rectRightDownY = 0;

    // 每次收集10个质心求均值然后输出
    QList<double> delayCenterX;
    QList<double> delayCenterY;

    QVector<QPair<double, double>> dataPairs; // 存放波长和透过率对应关系的容器

    void maxMinSwitch(bool minMax);

    void updateCursorShape(const QPoint &pos) {
        const int margin = 5; // 边界大小
        QRect rect = this->rect();

        if (pos.x() <= margin && pos.y() <= margin) {
            setCursor(Qt::SizeFDiagCursor); // 左上角
        } else if (pos.x() >= rect.width() - margin && pos.y() <= margin) {
            setCursor(Qt::SizeBDiagCursor); // 右上角
        } else if (pos.x() <= margin && pos.y() >= rect.height() - margin) {
            setCursor(Qt::SizeBDiagCursor); // 左下角
        } else if (pos.x() >= rect.width() - margin && pos.y() >= rect.height() - margin) {
            setCursor(Qt::SizeFDiagCursor); // 右下角
        } else if (pos.x() <= margin) {
            setCursor(Qt::SizeHorCursor); // 左边
        } else if (pos.x() >= rect.width() - margin) {
            setCursor(Qt::SizeHorCursor); // 右边
        } else if (pos.y() <= margin) {
            setCursor(Qt::SizeVerCursor); // 上边
        } else if (pos.y() >= rect.height() - margin) {
            setCursor(Qt::SizeVerCursor); // 下边
        } else {
            setCursor(Qt::ArrowCursor); // 默认
        }
    }

    void setResizeMode(const QPoint &pos) {
        const int margin = 5;
        QRect rect = this->rect();
        resizeDirection = None;

        if (pos.x() <= margin && pos.y() <= margin) {
            resizeDirection = TopLeft; // 左上角
        } else if (pos.x() >= rect.width() - margin && pos.y() <= margin) {
            resizeDirection = TopRight; // 右上角
        } else if (pos.x() <= margin && pos.y() >= rect.height() - margin) {
            resizeDirection = BottomLeft; // 左下角
        } else if (pos.x() >= rect.width() - margin && pos.y() >= rect.height() - margin) {
            resizeDirection = BottomRight; // 右下角
        } else if (pos.x() <= margin) {
            resizeDirection = Left; // 左边
        } else if (pos.x() >= rect.width() - margin) {
            resizeDirection = Right; // 右边
        } else if (pos.y() <= margin) {
            resizeDirection = Top; // 上边
        } else if (pos.y() >= rect.height() - margin) {
            resizeDirection = Bottom; // 下边
        }

        if (resizeDirection != None) {
            resizing = true;
            lastMousePos = mapToGlobal(pos);
        }
    }

    void resizeWindow(const QPoint &globalPos) {
        if (resizeDirection == None)
            return;

        QRect geometry = this->geometry();
        QPoint delta = globalPos - lastMousePos;
        lastMousePos = globalPos;

        switch (resizeDirection) {
        case TopLeft:
            geometry.setTopLeft(geometry.topLeft() + delta);
            break;
        case TopRight:
            geometry.setTopRight(geometry.topRight() + delta);
            break;
        case BottomLeft:
            geometry.setBottomLeft(geometry.bottomLeft() + delta);
            break;
        case BottomRight:
            geometry.setBottomRight(geometry.bottomRight() + delta);
            break;
        case Left:
            geometry.setLeft(geometry.left() + delta.x());
            break;
        case Right:
            geometry.setRight(geometry.right() + delta.x());
            break;
        case Top:
            geometry.setTop(geometry.top() + delta.y());
            break;
        case Bottom:
            geometry.setBottom(geometry.bottom() + delta.y());
            break;
        default:
            break;
        }

        this->setGeometry(geometry);
    }


private:
    bool isDragging = false;  // 标记是否正在拖动。注意：这里一定要初始化为false，否则，当我打开程序并首先点击的是dock窗口时会
    // 将重写的鼠标拖动程序应用在dock窗口上，这不是我们希望的
    // QPoint lastMousePos;  // 上一次鼠标位置
    QPoint dragStartPos;   // 窗口拖动开始位置


/// 四个相机的启动函数
private:
    // 创建HIK相机对象
    // HikCameraController *hikCameraController1;
    // 创建Basler对象
    CGuiCamera *baslerCameraController1;
    // 创建Sapera对象
    SaperaCameraController *saperaCameraController1;
    // 创建灰度计算对象
    newGrayscaleCentroid *newGrayscaleCentroid1;

    void openHikCamera(); // hik相机
    void closeHikCamera();
    void openBaslerCamera();
    void closeBaslerCamera();
    void updateImage(const QImage& image);

    // Basler相机


protected:
    void changeEvent (QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
/* 注意：当使用上述代码时，也就是重写了鼠标的点击、拖动、释放行为，可以实现鼠标点在ui界面的任意位置拖动主窗口，但是带来了一个问题:
当光标放在dock窗口的标题栏并拖动时我的本意是让dock界面悬浮以调整位置，但是实际使用时，主窗口会随着dock窗口的悬浮而消失，这是因为、
bool isDragging初始化为true。所以我们改用只有鼠标点击到ui界面的最上方时才能拖动界面，且isDragging初始化为false*/

private:
    Ui::MainWindow *ui;
    Dialog *diaUi; //声明对话框diaUi
    ChartsDialog *chartDiaUi; //声明子窗口弹窗
    ChartViewNew *chartViewNew; // 声明图表视图
    // hikParaSetDialog *hikCameraParaSetUi; // 声明相机参数设置窗口

    // 为透过率测试模块添加一个表格，注意是动态表格，用于存储不同波长测试条件下的镜头透过率指标
    QTableWidget *tableWidget_listTrans;
    // 灵活表格右键函数
    void createContextMenu();
    // 添加进入表格按钮槽函数
    // void addToListTrans();
    void addToListTransOffi();
    void addToListTransTest();
    // 情况列表Trans
    void clearTableTrans();
    // 绘制曲线
    void plotLine();

    QString buttonStyle = "QPushButton {"
                          "font-size: 14px; "
                          "font-weight: bold; "
                          "box-shadow: 0px 1px 2px rgba(0, 0, 0, 0.1);"
                          "}"; // 按钮风格，正常
    // QString buttonStyleHover = "border: none; background-color: #e0e0e0; font-size: 14px; font-weight: bold;"; // 按钮风格，悬停
    QString buttonStyleClicked =
                            "QPushButton {"
                            // "background-color: #b0b0b0; "  // 更深的灰色，确保视觉变化显著
                            // "border: 1px solid #808080; "   // 深色边框
                            "font-size: 14px; "
                            // "border-radius: 5px; "
                            "padding: 5px;"
                            "color: #333; "
                            "font-weight: bold; "
                            "box-shadow: 0px 1px 2px rgba(0, 0, 0, 0.1);"
                            "}"
                            "QPushButton:hover {"
                            "background-color: #dcdcdc; "
                            "border: 2px solid #d0d0d0; "
                             "}";
                            // "QPushbutton{border: none; "
                            //  "background-color: #d0d0d0; "
                            //  "font-size: 14px; "
                            //  "font-weight: bold; "
                            //  "padding: 2px;}"; // 按钮风格，点击

    QString buttonStyle2 = "QPushButton {"
                           "font-size: 14px; "
                           "color:#333333;"
                           "border: none; "
                           "border-radius: 5px;"
                           "padding: 5px;"
                           "background-color: rgba(255,255,255,50);"
                           "font-weight: bold; "
                           "box-shadow: 0px 1px 2px rgba(0, 0, 0, 0.1);"
                           "}"

                           "QPushButton:hover {"
                           "color:#333333;"
                           "border: 1px solid #8c8c8c;"
                           "border-radius: 5px;"
                           "padding: 5px;"
                           "background-color: qlineargradient("
                           "spread: pad,"
                           "x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 #b2d4f5,"
                           "stop: 1 #d7e0fd"
                           ");"
                           "}" // 按钮风格，正常
        ;

    QString buttonStyle_mainBtn = "QPushButton {"
                           "font-size: 14px; "
                           "color:#333333;"
                           "border: none; "
                           "border-radius: 5px;"
                           "padding: 5px;"
                           "background-color: rgba(255,255,255,0);"
                           "font-weight: bold; "
                           "box-shadow: 0px 1px 2px rgba(0, 0, 0, 0.1);"
                           "}"

                           "QPushButton:hover {"
                           "color:#333333;"
                           "border: 1px solid #8c8c8c;"
                           "border-radius: 5px;"
                           "padding: 5px;"
                           "background-color: qlineargradient("
                           "spread: pad,"
                           "x1: 0, y1: 0, x2: 0, y2: 1,"
                           "stop: 0 #b2d4f5,"
                           "stop: 1 #d7e0fd"
                           ");"
                           "}" // 按钮风格，正常
        ;

    // QString buttonStyleHover = "border: none; background-color: #e0e0e0; font-size: 14px; font-weight: bold;"; // 按钮风格，悬停
    QString buttonStyleClicked2 =
                            "QPushButton {"
                            // "background-color: #b0b0b0; "  // 更深的灰色，确保视觉变化显著
                            "background-color: #b2d4f5; "
                            // "border: 1px solid #808080; "   // 深色边框
                            "font-size: 14px; "
                            // "border-radius: 5px; "
                            "padding: 5px;"
                            "color: #333; "
                            "font-weight: bold; "
                            "box-shadow: 0px 1px 2px rgba(0, 0, 0, 0.1);"
                            "}"
                            "QPushButton:hover {"
                            "background-color: #b2d4f5; "
                            "border: 2px solid #d0d0d0; "
                            "}";
                        // "QPushbutton{border: none; "
                        //  "background-color: #d0d0d0; "
                        //  "font-size: 14px; "
                        //  "font-weight: bold; "
                        //  "padding: 2px;}"; // 按钮风格，点击

    // void onCheckBoxClicked();
    void setEnabled(bool tvf);
    void setEnabledMTF(bool tvf);
    void onCheckboxLensClicked();
    void onPushButtonDefPixSizeClicked();
    void onCheckboxOverallClicked();
    void changeButtonPowerColor();
    void changeButtonIcon();
    void ButtonStart();
    void fieldOfView();//视场角
    void focalLength();//焦距
    double focalLength2(double x,double x_true,double y,double y_true,double moveMm);//焦距通用函数
    void addToList();
    void aveFocalLength();//平均焦距
    void clearXYInput();
    void clearXYInputMTF();
    void init(); // 点击初始化按钮时初始化视场点坐标
    void initMTF(); // 点击初始化按钮时初始化视场点坐标
    void distortion();//畸变
    void MTF();//MTF
    void transmission();
    void drawCurve(QVector<double> vector); // 绘制曲线
    bool isColumnFilled(QTableWidget* tableWidget, int columnIndex, int rowCount);
    double MTF_Func(double DNmax,double DNmin,double DN); //MTF计算公式
    void onButtonClicked();
    void downImg();
    void saveRaw();
    QString rawFilesPath = QCoreApplication::applicationDirPath() + "/RAW/";
    void onChartDialogClosed();
    void openChartDialog();
    void testReport();
    void MTF_TableAddButtons(); // 在MTF列表的最后一列添加按钮
    void initTableCell(); // 初始化每一个单元格

    void updateTransmittance(); //根据excel表格更新相应的参考镜头透过率

    QValueAxis *axisX;
    QValueAxis *axisY;
    // QVector<double> *xNum;
    QVector<double> *xNum = new QVector<double>();  // 初始化一个空的 QVector<double>,注意：这里是声明和初始化一起做了，这样就不用在mainwindow中初始化了
    QChartView *chartView;
    QChart *chart;
    QVector<double> vectorCalMTF; // 第六列 MTF结果
    int lastTooltipX = -1; // 记录最后显示 `QToolTip` 的 X 值

    QLineSeries *series; // 将 series 声明为成员变量
    // QScatterSeries *highlightSeries;    // 高亮点的散点系列
    QGraphicsEllipseItem* highlightMarker = nullptr;
    // void onPointHovered(const QPointF &point, bool state);  // 槽函数

    void calculateCentroid(); //计算质心
    cv::Mat QImageToMat(const QImage &image);
    QGraphicsRectItem *rectItem; //用于绘制区域矩形
    QGraphicsEllipseItem *centroidItem;  // 用于绘制质心的图层
    QGraphicsEllipseItem *centroidItemLimit;  // 用于绘制质心的图层
    void updateCentroid(const cv::Point2d &centroid);
    void updateCentroidLimit(const cv::Point2d &centroid);
    void updateMousePosition(int x,int y);


    QStringList dataListFocalLength = QStringList(8, "");  // 存放焦距数据
    QStringList dataListDis = QStringList(7, "");  // 存放畸变参数数据
    QList<QStringList> tabDisAll; // 存放畸变测试表格的内容
    QStringList dataListMTF; // 存放MTF参数数据
    QList<QStringList> tabMTFAll; // 存放MTF测试表格的内容

    // QString getCurrentTimestamp();
    QPointF lastHoveredPoint = QPointF(-1, -1);



    int currentRow; // 当前要插入的行

    QImage currentImage; // 保存最近的一帧图像
    // 声明相机和捕获会话等成员变量
    QCamera *camera;
    QMediaCaptureSession *captureSession;
    QVideoSink *videoSink;

    QGraphicsScene *scene_hik; // 这个是hik相机scene
    QGraphicsPixmapItem *pixmapItem_hik;

    QGraphicsScene *scene; // 这个是原始的笔记本自带相机scene
    QGraphicsPixmapItem *pixmapItem;

    void startCamera();

    void onFrameAvailable1(const QPixmap &pixmap,const cv::Point2d &centerPoint,const cv::Point2d &centerPointLimit);
    QLabel *statusLabel;
    NewGraphicsView *view;

    //线程声明，便于析构，只有类成员变量可以被析构
    QThread *t1;
    image_Thread *imageThread;
    // QThread *t2;

    // 添加以下函数声明
    TestReportData collectTestData();
    bool validateTestData(const TestReportData& data);
    bool generatePDF(const QString& filePath, const TestReportData& data);
    
    // 辅助函数声明
    QStringList collectDistortionParameters();
    QList<QStringList> collectDistortionTableData();
    QStringList collectMTFParameters();
    QList<QStringList> collectMTFTableData();
    QString getMostRecentImage(const QString& folderPath);
    QStringList getAllImages(const QString& folderPath);
    QStringList collectTransmittanceData();

};
#endif // MAINWINDOW_H
