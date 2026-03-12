#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"ui_dialog_def_pix_size.h"
#include"dialog_def_pix_size.h"
#include"chartsdialog.h"
// #include "hikcameracontroller.h"
#include "imageprocessthread.h"
// #include<iostream>
#include <cstdlib>
#include<cmath>
#include<QButtonGroup>
#include<QMessageBox>
#include<QThread>
#include "image_thread.h"
#include <QDir>
#include<QTimer>
#include <QRegularExpressionValidator>
#include<opencv2/opencv.hpp>
// #include"Grayscale_centroid.h"

#include"newgraphicsview.h" ////鼠标拖动、滚轮缩放使用这个重定义的类
#include "newgrayscalecentroid.h"
#include"newdockwidgettitlebar.h"
#include <QLabel> // 实时显示光标所在坐标
#include <QStatusBar>
// #include <QGraphicsView> ////默认使用这个类

//相机图像头文件
#include <QCamera>            //启动或停止摄像头、设置摄像头属性（如分辨率、帧率等），并通过它捕获实时视频流。
// #include <QCameraViewfinder>  //显示相机的实时视频流。它可以作为相机的取景器，将视频内容显示在界面上 适用于qt5版本
// #include <QCameraImageCapture>
// #include <QVideoWidget>
#include <QMediaCaptureSession>  //qt6
#include <QVideoSink>

#include <QMediaDevices> // 获取相机信息
#include <QCameraDevice>
#include <QVideoFrameFormat>
#include <QSize>

#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPixmap>

#include <QtCharts>
#include"chartsdialog.h"
#include"utils.h"
#include <QTextStream>

// 直接加载Excel数据到容器（假设数据已排序）
#include <QVector>
#include <QPair>

#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QTextDocument> // 使用HTML和css修饰文本时使用
#include <QDoubleValidator>
#include "baslercameracontroller.h"
// 包含 Sapera 头文件
// #include "SapClassBasic.h"
#include "saperacameracontroller.h"
#include "Grayscale_centroid.h"
#include "xlsxdocument.h"  // QXlsx


QVector<double> vectorPixelList;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

    // 海康相机
    // , hikCameraController1(new HikCameraController(this))
    // Basler相机
    , baslerCameraController1(new CGuiCamera(this))
    // Sapera相机（cameralink）
    , saperaCameraController1(new SaperaCameraController(this))

    , ui(new Ui::MainWindow)
    , diaUi(new Dialog(this)) //初始化diaUi,注意，这里的diaUi是指针①
    , chartDiaUi(new ChartsDialog(this))
    , chartViewNew(new ChartViewNew(this))
    // , hikCameraParaSetUi(new hikParaSetDialog(this)) // 初始化相机参数设置窗口

    , axisX(new QValueAxis(this))
    , axisY(new QValueAxis(this))  //// 注意：这里之所以有这么长的初始化，是因为在声明这些变量并将这些变量加入到成员变量时没有定义
    , chart(new QChart)  // 注意：这里如果不对chart进行初始化的话，程序会崩溃：成员变量在构造函数中应进行初始化，否则会导致空指针访问
    , series(new QLineSeries(this))
    // , highlightSeries(new QScatterSeries(chartDiaUi))
    , currentRow(0)
{
    ui->setupUi(this);

    // 创建状态栏 显示作者、版本、日期等信息
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    // 创建标签并设置信息
    QLabel *infoLabel = new QLabel(this);
    infoLabel->setText("上海国科航星量子科技有限公司 | XXH | Version 1.0.1 | 2025.09.18");
    infoLabel->setFont(QFont("Times New Roman", 2));
    infoLabel->setStyleSheet("color: gray;");  // 设置为灰色
    statusBar->addPermanentWidget(infoLabel);


    // 获取主屏幕对象
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    // 获取屏幕完整分辨率（包括任务栏）
    QRect screenGeometry = primaryScreen->geometry();
    // 获取可用分辨率（排除任务栏）
    // QRect availableGeometry = primaryScreen->availableGeometry();
    int windowWidth0 = screenGeometry.width();
    int windowHeight0 = screenGeometry.height();
    int windowWidth = screenGeometry.width() * 0.8;
    int windowHeight = screenGeometry.height() * 0.8;
    windowWidthTrans0 = windowWidth0;
    windowHeightTrans0 = windowHeight0;
    windowWidthTrans = windowWidth;
    windowHeightTrans = windowHeight;

    this->resize(windowWidth, windowHeight);
    ui->dockWidget_paraSetting->setMaximumWidth(windowWidth*0.23); // 窗口化时中间区域最大宽度
    ui->dockWidget_dashBoard->setMaximumWidth(windowWidth*0.23); // 窗口化时中间区域最大宽度
    ui->dockWidget_statusOutput->setMaximumWidth(windowWidth*0.23); // 窗口化时中间区域最大宽度
    ui->widget_Center->setMinimumWidth(windowWidth*0.36); // 窗口化时中间区域最大宽度
    ui->dockWidget_imageShow->setMaximumWidth(windowWidth*0.5); // 侧边栏最小宽度
    ui->dockWidget_imageShow->setMinimumWidth(windowWidth*0.3); // 侧边栏最小宽度

    // ui->widget_Center->setMaximumWidth(550); // 窗口化时中间区域最大宽度
    // ui->dockWidget_imageShow->setMinimumWidth(450); // 侧边栏最小宽度

    // 创建阴影效果 ——注意：下面这段代码会导致主界面非常卡顿，非常卡顿，非常卡顿！！！！！！！
    /* QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(3, 3);  // 阴影的偏移量
    shadowEffect->setBlurRadius(15);  // 阴影的模糊半径
    shadowEffect->setColor(QColor(0, 0, 0, 160));  // 阴影的颜色和透明度（例如黑色，透明度160）
    // 将阴影效果应用到窗口
    setGraphicsEffect(shadowEffect); */


    // 设置只读
    ui->lineEdit_centroidX->setReadOnly(true);
    ui->lineEdit_centroidY->setReadOnly(true);

    // 初始化拖动状态
    // isDragging = false;
    // 设置中心窗口
    this->setCentralWidget(ui->widget_Center);

    // 设置窗口为无边框模式
    this->setWindowFlags(Qt::FramelessWindowHint);
    // 设置窗口颜色
    // this->setStyleSheet("background-color: #90CEF5;"); // 已经在qss中设置过了，这里就可以省略。注意：这里设置和qss中设置是有区别的
    // 这里设置的话，整个界面中的所有控件在默认情况下都是这个样式，而在qss中设置时，只有mainwindow是这个样式，其他每个类别的模组都需要单独设置
    // this->setStyleSheet("background-color: #EAF7FF");
    // 单独设置对话框中groupbox的样式
    // 设置QCheckBox的图像样式

    // 创建Logo控件
    QLabel *logoLabel = new QLabel;
    QPixmap logoPixmap(":/Icon/Icon_images/logo_hx.png");
    logoLabel->setPixmap(logoPixmap.scaled(169, 45, Qt::KeepAspectRatio)); // 调整Logo大小

    QLabel *titleLabel = new QLabel("光学模组与整机标准化测试系统");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; margin-left: 20px;");

    // 创建Logo和标题的布局
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(titleLabel);
    logoLayout->addStretch();
    logoLayout->setContentsMargins(10, 0, 0, 5);  // 设置布局与控件边缘的间距
    QWidget *logoWidget = new QWidget;
    logoWidget->setLayout(logoLayout); /// ☆2

    // 在顶部的logo区域中添加最小化、最大化和关闭按钮
    QPushButton *btn_min = new QPushButton;
    QPushButton *btn_max = new QPushButton;
    QPushButton *btn_close = new QPushButton;
    // 在logo区域中添加三个按钮，但是单独创建一个widget，这个widget与logo的widget垂直排列
    QWidget *btnWinWidget = new QWidget;
    btnWinWidget->setFixedHeight(24);  // 设置固定高度为24像素
    QHBoxLayout *btnWinLayout = new QHBoxLayout;
    QSpacerItem *btnWinSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnWinLayout->setContentsMargins(0, 0, 0, 0); // 左、上、右、下边距。
    btnWinLayout->addItem(btnWinSpacer);
    btnWinLayout->addWidget(btn_min);
    btnWinLayout->addWidget(btn_max);
    btnWinLayout->addWidget(btn_close);
    btnWinLayout->setSpacing(0);
    btnWinWidget->setLayout(btnWinLayout); /// ☆1
    // 标准库的Icon，并不是很好看
    // btn_min->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    // btn_max->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    // btn_close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    // 自定义Icon
    QIcon icon_creatPDF(":/Icon/Icon_images/creat_PDF_icon.png");
    QIcon icon_testGO(":/Icon/Icon_images/go_icon.png");
    // btn_min->setIcon(":/Icon/Icon_images/window_minimize_icon.png");
    QIcon icon_min(":/Icon/Icon_images/window_minimize_icon.png");
    QIcon icon_max(":/Icon/Icon_images/window_maximize_icon.png");
    QIcon icon_close(":/Icon/Icon_images/window_close_icon.png");
    ui->pushButton_print->setIcon(icon_creatPDF);
    ui->pushButton_start->setIcon(icon_testGO);
    btn_min->setIcon(icon_min);
    btn_max->setIcon(icon_max);
    btn_close->setIcon(icon_close);
    // 设置按钮的尺寸
    btn_min->setFixedWidth(46);
    btn_max->setFixedWidth(46);
    btn_close->setFixedWidth(46);
    // 上述代码等同于 btn_min->setIcon(QIcon(":/Icon/Icon_images/window_minimize_icon.png"));
    btn_min->setIconSize(QSize(26, 26));
    btn_max->setIconSize(QSize(26, 26));
    btn_close->setIconSize(QSize(26, 26));
    // 设置按钮透明背景并在鼠标悬停时显示背景
    btn_min->setFlat(true);
    btn_max->setFlat(true);
    btn_close->setFlat(true);
    btn_min->setStyleSheet( "QPushButton { background: transparent; border: none; }"
                            "QPushButton:hover { background: rgba(255, 255, 255, 0.4); }");
    btn_max->setStyleSheet( "QPushButton { background: transparent; border: none; }"
                           "QPushButton:hover { background: rgba(255, 255, 255, 0.4); }");
    btn_close->setStyleSheet( "QPushButton { background: transparent; border: none; }"
                             "QPushButton:hover { background: rgba(255, 255, 255, 0.4); }");
    // 设置按钮槽函数
    connect(btn_min,&QPushButton::clicked,this,[=](){
        this->showMinimized();
    });
    // ui->widget_Center->setMaximumWidth(600);
    // ui->dockWidget_imageShow->setMinimumWidth(QWIDGETSIZE_MAX);
    /// ☆☆☆☆☆注意：这里的600和500必须和下面的if语句中的600和500对应起来，否则会导致dock无法自由停靠，原因是点击按钮后的尺寸参数发生了变更☆☆☆☆☆
    connect(btn_max,&QPushButton::clicked,this,[=](){
        if (this->isMaximized()) {
            // 还原时的尺寸
            this->showNormal();
            maxMinSwitch(true);
            // 设置Icon的变化，此时显示最大化Icon
            btn_max->setIcon(QIcon(":/Icon/Icon_images/window_maximize_icon.png"));
        } else {
            this->showMaximized();
            maxMinSwitch(false);
            // 设置Icon的变化，此时显示缩小化Icon
            btn_max->setIcon(QIcon(":/Icon/Icon_images/window_restore_icon.png"));
        }
    });

    connect(btn_close,&QPushButton::clicked,this,[=](){
        // this->close();
        QApplication::quit();
    });


    // 创建按钮和logo的widget
    QWidget *btn_logoWidget = new QWidget;
    QVBoxLayout *btn_logoLayout = new QVBoxLayout;
    btn_logoLayout->addWidget(btnWinWidget);
    btn_logoLayout->addWidget(logoWidget);
    btn_logoWidget->setLayout(btn_logoLayout);
    btn_logoLayout->setContentsMargins(10, 0, 0, 0);
    btn_logoLayout->setSpacing(0);
    // 创建顶部区域
    QWidget *topWidget = new QWidget;
    QVBoxLayout *topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(0, 0, 0, 10); // 左、上、右、下边距。
    // QSpacerItem* spacerLogMenu = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum); // 10 是间距，可以根据需要调整
    topLayout->addWidget(btn_logoWidget);
    topLayout->addWidget(menuBar());  // 添加菜单栏

    topLayout->setSpacing(10); // 设置控件之间的间距为10
    topLayout->setContentsMargins(2,0,0,2);

    // 添加顶部区域到主窗口
    this->setMenuWidget(topWidget); /// 这样设置的话，dock窗口就不会挤压top窗口的空间，top窗口可以独立存在于最上方

    // 设置中心窗口
    this->setCentralWidget(ui->widget_Center);
    // 设置主窗口的窗口化和全屏状态下的交互
    // if(QEvent::WindowStateChange){

    // 添加左侧DockWidget
    this->addDockWidget(Qt::LeftDockWidgetArea, ui->dockWidget_dashBoard);
    this->addDockWidget(Qt::LeftDockWidgetArea, ui->dockWidget_paraSetting);
    this->addDockWidget(Qt::LeftDockWidgetArea, ui->dockWidget_statusOutput);

    // this->splitDockWidget(ui->dockWidget_dashBoard, ui->dockWidget_paraSetting, Qt::Vertical);
    // this->splitDockWidget(ui->dockWidget_paraSetting, ui->dockWidget_statusOutput, Qt::Vertical);

    // 添加右侧DockWidget
    // this->addDockWidget(Qt::RightDockWidgetArea, ui->dockWidget_dashBoard);
    this->addDockWidget(Qt::RightDockWidgetArea, ui->dockWidget_imageShow);
    // this->splitDockWidget(ui->dockWidget_dashBoard, ui->dockWidget_imageShow, Qt::Vertical);
    // 调整上下顺序
    // splitDockWidget(ui->dockWidget_dashBoard, ui->dockWidget_imageShow, Qt::Vertical); // dockWidget_imageShow 在 dockWidget_dashBoard 下方
    // splitDockWidget(ui->dockWidget_paraSetting, ui->dockWidget_statusOutput, Qt::Vertical); // dockWidget_statusOutput 在 dockWidget_paraSetting 下方

    ui->dockWidget_dashBoard->setMaximumHeight(110);
    ui->dockWidget_dashBoard->setMinimumHeight(110);

    /// 显示和隐藏测试模块
    ui->action_dashBoard->setCheckable(true);
    ui->action_dashBoard->setChecked(true);
    ui->action_imageShow->setCheckable(true);
    ui->action_imageShow->setChecked(true);
    ui->action_paraSetting->setCheckable(true);
    ui->action_paraSetting->setChecked(true);
    ui->action_statusOutput->setCheckable(true);
    ui->action_statusOutput->setChecked(true);
    connect(ui->action_dashBoard,&QAction::toggled,this, [this](bool checked){
        if(checked){
            ui->dockWidget_dashBoard->show();
        }
        else ui->dockWidget_dashBoard->hide();
    });
    connect(ui->action_imageShow,&QAction::toggled,this, [this](bool checked){
        if(checked){
            ui->dockWidget_imageShow->show();
        }
        else ui->dockWidget_imageShow->hide();
    });
    connect(ui->action_paraSetting,&QAction::toggled,this, [this](bool checked){
        if(checked){
            ui->dockWidget_paraSetting->show();
        }
        else ui->dockWidget_paraSetting->hide();
    });
    connect(ui->action_statusOutput,&QAction::toggled,this, [this](bool checked){
        if(checked){
            ui->dockWidget_statusOutput->show();
        }
        else ui->dockWidget_statusOutput->hide();
    });
    // 将dockWidget右上角的叉叉和QAction中的选项链接起来，以同步模块的显示状态
    connect(ui->dockWidget_dashBoard, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // 检查主窗口是否被最小化
        if (this->windowState() & Qt::WindowMinimized) {
            return; // 忽略最小化状态下的信号
        }
        ui->action_dashBoard->setChecked(visible);
    });
    connect(ui->dockWidget_imageShow, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // 检查主窗口是否被最小化
        if (this->windowState() & Qt::WindowMinimized) {
            return; // 忽略最小化状态下的信号
        }
        ui->action_imageShow->setChecked(visible);
    });
    connect(ui->dockWidget_paraSetting, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // 检查主窗口是否被最小化
        if (this->windowState() & Qt::WindowMinimized) {
            return; // 忽略最小化状态下的信号
        }
        ui->action_paraSetting->setChecked(visible);
    });
    connect(ui->dockWidget_statusOutput, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // 检查主窗口是否被最小化
        if (this->windowState() & Qt::WindowMinimized) {
            return; // 忽略最小化状态下的信号
        }
        ui->action_statusOutput->setChecked(visible);
    });
    /* 笔记：Lambda 内需要访问 this 对象的成员（ui->action_dashBoard），因此用 [this] 捕获 this 指针
    信号 visibilityChanged 传递一个 bool 参数
    上述代码可以实现点击dockwidget右上角的缩小按钮时，窗口悬浮，点击右上角叉叉时，窗口隐藏，同时点击视图中的相应模块，也能改变dockwidget的可视状态
    但是遇到新的问题：当点击主窗口UI界面右上角的最小化按钮时，四个dockwidget同时隐藏，原因是，点击最小化按钮时，QDockWidget 的 visibilityChanged(bool) 信号
    也会被触发，当主窗口最小化时，所有子窗口（包括 QDockWidget）的可见性实际上都被 Qt 设置为 false,在改变dockwidget可视化之前需要检测当前主窗口的状态 */

    // 设置自定义标题栏
    newDockWidgetTitleBar::newDockBar(ui->dockWidget_paraSetting, "主参数面板");
    newDockWidgetTitleBar::newDockBar(ui->dockWidget_dashBoard, "主控制面板");
    newDockWidgetTitleBar::newDockBar(ui->dockWidget_imageShow, "图像显示窗口");
    newDockWidgetTitleBar::newDockBar(ui->dockWidget_statusOutput, "测试状态输出");
    // 设置dock窗口的初始宽度
    // ui->dockWidget_imageShow->setMinimumWidth(1000);
    // setDockOptions(QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks);

    // 设置项目命名框的输入限制，使用QValidator  注意：QRegularExpressionValidator是用于输入验证的一个类，它基于正则表达式
    QRegularExpression regExp("[^<>:\"/\\|?*]*"); // 定义 Win10 文件夹合法字符
    QValidator *validator = new QRegularExpressionValidator(regExp, ui->lineEdit_objectName);
    ui->lineEdit_objectName->setValidator(validator);
    connect(ui->lineEdit_objectName,&QLineEdit::inputRejected,this,[=](){
        QString toolTipText = "文件名不能包含下列任何字符：\n           /:*?<>|";
        // ui->lineEdit_objectName->setToolTip(toolTipText); // 注意：使用这种方式，只能在鼠标悬停在目标单位上时才显示tooltip
        QToolTip::showText(ui->lineEdit_objectName->mapToGlobal(QPoint()), toolTipText); // 使用showText可以即时显示tooltip，但是第一个参数位需要提供位置信息
    });

    // 设置表格的宽度调整策略：自动填充整个表格宽度
    ui->tableWidget_MTF->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_distortion->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //设置表格的样式
    ui->tableWidget_distortion->setStyleSheet("QWidget { background-color: rgba(255,255,255,50); }");
    ui->tableWidget_MTF->setStyleSheet("QWidget { background-color: rgba(255,255,255,50); }");
    ui->tableWidget_aveFocalLength->setStyleSheet("QWidget { background-color: rgba(255,255,255,50); }");

    // 在表格的最后一列添加按钮，将下面这部分操作封装成函数，便于在修改表格行数时随时使用
    MTF_TableAddButtons();

    // 为透过率测试模块添加一个表格，注意是动态表格，用于存储不同波长测试条件下的镜头透过率指标
    tableWidget_listTrans = new QTableWidget(this); // 初始化，直接使用指针，但没有先初始化它会崩溃
    // 将表格加入到父容器中
    QVBoxLayout* vLayout;
    vLayout = new QVBoxLayout(ui->groupBox_21);
    // 添加表格到垂直布局
    vLayout->addWidget(tableWidget_listTrans);
    // 将现有的gridLayout_4添加到新的垂直布局中
    // 创建一个新的 QWidget 容器
    QWidget *gridContainer = new QWidget();
    // 将 gridLayout_4 设置为这个容器的布局
    gridContainer->setLayout(ui->gridLayout_4);
    // 将容器添加到新布局
    vLayout->addWidget(gridContainer);
    // 设置表格基本属性
    tableWidget_listTrans->setColumnCount(5);
    tableWidget_listTrans->setHorizontalHeaderLabels(QStringList() << "波长（nm）" << "参照光功率"<<"参照透过率"<< "测试光功率"<< "测试透过率");
    // 设置表格自动填充可用空间
    tableWidget_listTrans->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 允许右键菜单
    tableWidget_listTrans->setContextMenuPolicy(Qt::CustomContextMenu);
    // 连接右键菜单信号
    connect(tableWidget_listTrans, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::createContextMenu);
    // 加入列表按钮槽函数
    connect(ui->pushButton_addListOffi,&QPushButton::clicked,this,&MainWindow::addToListTransOffi);
    connect(ui->pushButton_addListTest,&QPushButton::clicked,this,&MainWindow::addToListTransTest);
    // 清空表格按钮槽函数
    connect(ui->pushButton_clearTableTrans,&QPushButton::clicked,this,&MainWindow::clearTableTrans);
    // 生成图像按钮槽函数
    connect(ui->pushButton_plotLine,&QPushButton::clicked,this,&MainWindow::plotLine);

    // 设置主窗口按钮
    ui->pushButton_init_3->setFixedSize(70,32);
    ui->pushButton_init_3->setStyleSheet(buttonStyle2);
    ui->pushButton_init->setFixedSize(70,32);
    ui->pushButton_init->setStyleSheet(buttonStyle2);
    ui->pushButton_addToList->setFixedSize(32,32);
    ui->pushButton_addToList->setStyleSheet(buttonStyle2); // 将按钮一个一个地设置状态有点麻烦，是否使用全局样式更简单？
    ui->pushButton_aveFocalLength->setFixedSize(70,32);
    ui->pushButton_aveFocalLength->setStyleSheet(buttonStyle2);
    ui->pushButton_clearDistortion->setFixedSize(70,32);
    ui->pushButton_clearDistortion->setStyleSheet(buttonStyle2);
    ui->pushButton_clearDistortion_3->setFixedSize(70,32);
    ui->pushButton_clearDistortion_3->setStyleSheet(buttonStyle2);
    ui->pushButton_clearTable->setFixedSize(70,32);
    ui->pushButton_clearTable->setStyleSheet(buttonStyle2);
    ui->pushButton_defaultPixelSize->setFixedSize(70,32);
    ui->pushButton_defaultPixelSize->setStyleSheet(buttonStyle2);
    ui->pushButton_downImg->setFixedSize(38,38);
    ui->pushButton_downImg->setStyleSheet(buttonStyle2);

    ui->pushButton_addGrayscale1->setStyleSheet(buttonStyle2);
    ui->pushButton_addGrayscale2->setStyleSheet(buttonStyle2);
    ui->pushButton_plotLine->setStyleSheet(buttonStyle2);
    ui->pushButton_clearTableTrans->setStyleSheet(buttonStyle2);
    // ui->pushButton_addToList_2->setStyleSheet(buttonStyle2);
    ui->pushButton_addListOffi->setStyleSheet(buttonStyle2);
    ui->pushButton_addListTest->setStyleSheet(buttonStyle2);
    ui->pushButton_addToList_MTF->setStyleSheet(buttonStyle2);
    ui->pushButton_addToList_Dis->setStyleSheet(buttonStyle2);

    ui->pushButton_refreshCamera->setToolTip("重载相机");
    // ui->pushButton_refreshCamera->setFixedSize(42,42);
    ui->pushButton_refreshCamera->setStyleSheet(buttonStyle_mainBtn);
    // 设置大小策略
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->pushButton_refreshCamera->setSizePolicy(sizePolicy);
    ui->pushButton_power->setToolTip("打开电源");
    // ui->pushButton_power->setFixedSize(42,42);
    ui->pushButton_power->setStyleSheet(buttonStyle_mainBtn);
    ui->pushButton_power->setSizePolicy(sizePolicy);
    ui->pushButton_power->setVisible(false); //由于暂时不用电源按钮，所以隐藏
    ui->pushButton_openCamera->setToolTip("相机启动");
    // ui->pushButton_openCamera->setFixedSize(42,42);
    ui->pushButton_openCamera->setStyleSheet(buttonStyle_mainBtn);
    ui->pushButton_openCamera->setSizePolicy(sizePolicy);
    ui->groupBox_5->layout()->setContentsMargins(0, 0, 0, 0);
    ui->groupBox_5->layout()->setSpacing(0);

    ui->pushButton_start->setToolTip("开始测试");
    // ui->pushButton_start->setFixedSize(42,42);
    ui->pushButton_start->setStyleSheet(buttonStyle_mainBtn);
    ui->pushButton_start->setSizePolicy(sizePolicy);
    ui->pushButton_dataToExcel->setToolTip("保存数据到Excel");
    // ui->pushButton_dataToExcel->setFixedSize(42,42);
    ui->pushButton_dataToExcel->setStyleSheet(buttonStyle_mainBtn);
    ui->pushButton_dataToExcel->setSizePolicy(sizePolicy);
    ui->pushButton_print->setToolTip("打印PDF");
    // ui->pushButton_print->setFixedSize(42,42);
    ui->pushButton_print->setStyleSheet(buttonStyle_mainBtn);
    ui->pushButton_print->setSizePolicy(sizePolicy);


    // 要在MTF曲线上画标记，这里是初始化标记
    highlightMarker = new QGraphicsEllipseItem();
    highlightMarker->setRect(-3, -3, 6, 6); // 设置标记大小：绘制以 (0, 0) 为中心，半径为 5 的圆形标记。
    highlightMarker->setBrush(Qt::red);
    // highlightMarker->setPen(Qt::NoPen);

    // 子窗口中自带的ButtonBox中是否可以添加Button
    // QPushButton *downChartButton = new QPushButton(); // 创建新的按钮
    // 设置子窗口按钮上
    QIcon icon = QIcon(":/Icon/Icon_images/downloadIcon.png"); // 设置图片为icon
    chartDiaUi->getButton()->setIcon(icon);
    chartDiaUi->getButton()->setFixedSize(32, 32); // 设置按钮宽80像素，高30像素
    chartDiaUi->getButton()->setIconSize(QSize(27, 27));
    // chartDiaUi->getButton()->setStyleSheet(buttonStyle2);
    // 设置按钮样式
    chartDiaUi->getButton()->setStyleSheet(R"(
    QPushButton {
        border: 2px solid #8f8f91;
        border-radius: 5px;
        background-color: #dfdfdf;
        padding: 5px;
    }
    QPushButton:pressed {
        background-color: #bfbfbf;
        border-style: inset;
        padding-left: 6px;
        padding-top: 6px;
    }
    QPushButton:hover {
        background-color: #e6e6e6;
    }
)");

    // chartDiaUi->getButtonBox()->addButton(downChartButton,QDialogButtonBox::ActionRole); // 将按钮加进buttonbox，注意：第二个参数需要设置按钮的规则


    connect(chartDiaUi, &ChartsDialog::dialogClosed, this, &MainWindow::onChartDialogClosed);
    // connect(chartDiaUi, &ChartsDialog::showDialog, this, &MainWindow::openChartDialog);

    // 鼠标移动到图标上的样本点时显示坐标的槽函数
    connect(series,&QLineSeries::hovered,this,&MainWindow::onPointHovered);

    // 图像下面的下载按钮的点击槽函数
    connect(ui->pushButton_downImg,&QPushButton::clicked,this,&MainWindow::downImg);

    // 保存原始RAW格式数据
    connect(ui->pushButton_startSaveRaw,&QPushButton::clicked,this,&MainWindow::saveRaw);

    // 创建chartImages文件夹消息打印
    connect(chartDiaUi, &ChartsDialog::messagePrint,this,[=](QString message){
        ui->plainTextEdit->appendPlainText(message);
    });
    // 保存chart为图像
    connect(chartDiaUi, &ChartsDialog::saveChartSignal,this,[=](QString chartImageFolder){
        QString appDir = QCoreApplication::applicationDirPath();
        QString objectName  = ui->lineEdit_objectName->text();
        QString chartImageFolderPath = appDir + "/" +objectName + chartImageFolder;
        QDir chartDir(chartImageFolderPath);
        // 检查图像保存路径是否存在
        if(!chartDir.exists()){
            // 通知输出框
            QString message = QString("[%1] 初始化文件夹chartImages完成").arg(utils::getCurrentTimestamp());
            // 向主窗口发送一个信号，从而在输出框中弹出上述提示
            ui->plainTextEdit->appendPlainText(message);
            chartDir.mkpath(".");
        }

        // qDebug()<<"正在保存chart图像";
        // 抓取chart并生成图像，保存jpg格式到chartDir

        // 注意：确保chartImageFolderPath是一个有效的文件路径，包含完整的文件名和扩展名,而此时的chartImageFolderPath只是一个路径
        //（例如chartImageFolderPath = "C:/path/to/folder/chart_image.jpg"）。如果路径只包含文件夹而不包含文件名，保存会失败。
        QString currentTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

        // 给即将保存的文件进行命名
        QString filePath;
        if (ui->lineEdit_objectName->text() != "") {
            filePath = QString("%1/%2-%3.jpg").arg(chartImageFolderPath, ui->lineEdit_objectName->text(), currentTime);
        } else {
            filePath = QString("%1/%2.jpg").arg(chartImageFolderPath, currentTime);
        }

        QGraphicsScene * grabChartScene = chartViewNew->scene();
        if(!grabChartScene) return;
        // 暂时隐藏标记
        highlightMarker->setVisible(false);
        // QPixmap chartPixmap = chartView->grab();
        QPixmap chartPixmap = chartViewNew->grab();
        QImage chartImage = chartPixmap.toImage();
        // 保存图像
        if (chartImage.save(filePath, "JPG")) {
            // 将信息同步到输出窗口
            // 获取当前时间并格式化为字符串
            QString message = QString("[%1] 图像已保存到: %2").arg(utils::getCurrentTimestamp(),chartImageFolderPath);
            ui->plainTextEdit->appendPlainText(message);
        } else {
            QMessageBox::warning(this,"Error","图像保存失败！");
            // 注意：下面这三行可以用于所有的需要输出到输出窗口的地方
            QString message = QString("[%1] 图像保存失败！").arg(utils::getCurrentTimestamp());
            ui->plainTextEdit->appendPlainText(message);
        }
        // 恢复标记的可见性
        highlightMarker->setVisible(true);
    });

    // 两个表格，当点击添加按钮时，添加行数
    connect(ui->pushButton_addToList_Dis, &QPushButton::clicked, this, [=](){
        int currentRowCount = ui->tableWidget_distortion->rowCount();
        ui->tableWidget_distortion->insertRow(currentRowCount);
        //注意：这里添加行后，其实表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        int columnCount = ui->tableWidget_distortion->columnCount();
        for (int col = 0; col < columnCount; ++col) {
            QTableWidgetItem* item = new QTableWidgetItem(""); // 初值为空
            ui->tableWidget_distortion->setItem(currentRowCount, col, item);
        }
    });

    connect(ui->pushButton_addToList_MTF, &QPushButton::clicked, this, [=](){
        int currentRowCount = ui->tableWidget_MTF->rowCount();
        ui->tableWidget_MTF->insertRow(currentRowCount);
        //注意：这里添加行后，其实表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        int columnCount = ui->tableWidget_MTF->columnCount();
        for (int col = 0; col < columnCount; ++col) {
            QTableWidgetItem* item = new QTableWidgetItem("");
            ui->tableWidget_MTF->setItem(currentRowCount, col, item);
        }
        // 最后一列添加按钮
        MTF_TableAddButtons();
    });

    //设置表格不可编辑部分
    setEnabled(false); //启动软件时默认设置表格x和y不可编辑
    //设置选择自定义checkbox时恢复表格的可编辑属性
    connect(ui->checkBox_customize,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为0
        ui->tableWidget_distortion->setRowCount(0);
        setEnabled(true); //当选择自定义视场点时，恢复x和y可编辑
    });
    connect(ui->checkBox_2v2,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为9
        ui->tableWidget_distortion->setRowCount(9);
        //注意：这里设置完表格的行数后，表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        for (int row = 0; row < 9; ++row) {
            for(int col = 0; col < 6; ++col){
                QTableWidgetItem *item = ui->tableWidget_distortion->item(row, col);
                if(!item){
                    QTableWidgetItem* item = new QTableWidgetItem("");
                    ui->tableWidget_distortion->setItem(row, col, item);
                }
            }
        }
        setEnabled(false); //当选择2*2视场点时，x和y不可编辑
    });
    // 打开程序的时候就要初始化一次表格
    initTableCell();
    connect(ui->checkBox_3v3,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为16
        ui->tableWidget_distortion->setRowCount(16);
        //注意：这里设置完表格的行数后，表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        initTableCell();
        setEnabled(false); //当选择3*3视场点时，x和y不可编辑
    });

    setEnabledMTF(false);
    connect(ui->checkBox_customize_3,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为0
        ui->tableWidget_MTF->setRowCount(0);
        setEnabledMTF(true); //当选择自定义视场点时，恢复x和y可编辑
    });

    connect(ui->checkBox_2v2_3,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为9
        ui->tableWidget_MTF->setRowCount(9);
        //注意：这里设置完表格的行数后，表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        for (int row = 0; row < 9; ++row) {
            for(int col = 0; col < 6; ++col){
                QTableWidgetItem *item = ui->tableWidget_MTF->item(row, col);
                if(!item){
                    QTableWidgetItem* item = new QTableWidgetItem("");
                    ui->tableWidget_MTF->setItem(row, col, item);
                }
            }
        }
        // 最后一列添加按钮
        MTF_TableAddButtons();
        setEnabledMTF(false); //当选择2*2视场点时，x和y不可编辑
    });
    connect(ui->checkBox_3v3_3,&QCheckBox::clicked,this,[=](){
        // 设置此时的列表行数为16
        ui->tableWidget_MTF->setRowCount(16);
        //注意：这里设置完表格的行数后，表格的单元格是没有初始化的，会出现空指针的情况，所以需要及时初始化
        for (int row = 0; row < 16; ++row) {
            for(int col = 0; col < 6; ++col){
                QTableWidgetItem *item = ui->tableWidget_MTF->item(row, col);
                if(!item){
                    QTableWidgetItem* item = new QTableWidgetItem("");
                    ui->tableWidget_MTF->setItem(row, col, item);
                }
            }
        }
        // 最后一列添加按钮
        MTF_TableAddButtons();
        setEnabledMTF(false); //当选择3*3视场点时，x和y不可编辑
    });

    // 额外添加操作：当选择自定义模式时，初始化按钮不可点击
    connect(ui->checkBox_customize,&QCheckBox::checkStateChanged,this,[=](int state){
        if(state == Qt::Checked){
            ui->pushButton_init->setEnabled(false);
            ui->pushButton_addToList_Dis->setEnabled(true);
        }
        else{
            ui->pushButton_init->setEnabled(true);
            ui->pushButton_addToList_Dis->setEnabled(false);
        }
    });
    connect(ui->checkBox_customize_3,&QCheckBox::checkStateChanged,this,[=](int state){
        if(state == Qt::Checked){
            ui->pushButton_init_3->setEnabled(false);
            ui->pushButton_addToList_MTF->setEnabled(true);
        }
        else{
            ui->pushButton_init_3->setEnabled(true);
            ui->pushButton_addToList_MTF->setEnabled(false);
        }
    });

    // 程序启动时检查点击状态
    if(ui->checkBox_fixedDistsnce->isChecked()){
        ui->lineEdit_reticleMoveMm->setReadOnly(false);
    }else{
        ui->lineEdit_reticleMoveMm->setReadOnly(true);
    }
    // 是否锁定当前分划板移动距离
    connect(ui->checkBox_fixedDistsnce,&QCheckBox::checkStateChanged,this,[=](int state){

        ui->lineEdit_reticleMoveMm->setReadOnly(state != Qt::Checked);
        if(state == Qt::Checked){
            // 弹出一个窗口询问用户 “是否统一设置所有测试点的分划板移动距离，该设置会稍微影响测试精度”
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          "操作提示",
                                          "是否统一设置所有测试点的分划板移动距离？\n该设置会稍微影响测试精度。",
                                          QMessageBox::Yes | QMessageBox::No);
            // 如果用户在弹出的窗口中点击确认，则将表格的第五列全部设置为ui->checkBox_fixedDistsnce的内容
            if (reply == QMessageBox::Yes) {
                QString value = ui->lineEdit_reticleMoveMm->text();

                // 表格控件是 ui->tableWidget_distortion，并且第五列是索引 4（列索引从0开始）
                int rowCount = ui->tableWidget_distortion->rowCount();
                for (int row = 0; row < rowCount; ++row) {
                    QTableWidgetItem* item = ui->tableWidget_distortion->item(row, 4);
                    if (!item) {
                        item = new QTableWidgetItem();
                        ui->tableWidget_distortion->setItem(row, 4, item);
                    }
                    item->setText(value);
                }
            }
            // 如果用户在弹出的窗口中点击取消，则没有变化
        }
    });


    /*// 设置LineEdit输入限制。已经单独创建了一个类用来管理lineedit的输入限制函数
    // ①输入数字（包括整数和小数）
    QDoubleValidator *doubleValidator = new QDoubleValidator(this);
    // doubleValidator->setDecimals(4); // 小数点后最多可输入4位
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    ui->lineEdit_originalOpticalPower->setValidator(doubleValidator);
    ui->lineEdit_originalOpticalPower->setMaxLength(10);
    ui->lineEdit_officialLensOpticalPower->setValidator(doubleValidator);
    ui->lineEdit_officialLensOpticalPower->setMaxLength(10);
    ui->lineEdit_officialLensTransmission->setValidator(doubleValidator);
    ui->lineEdit_officialLensTransmission->setMaxLength(10);
    ui->lineEdit_testLensOpticalPower->setValidator(doubleValidator);
    ui->lineEdit_testLensOpticalPower->setMaxLength(10);
    ui->lineEdit_testLensTransmission->setValidator(doubleValidator);
    ui->lineEdit_testLensTransmission->setMaxLength(10);

    ui->lineEdit_pixelSize->setValidator(doubleValidator);
    ui->lineEdit_pixelSize->setMaxLength(10);
    ui->lineEdit_colliFoci->setValidator(doubleValidator);
    ui->lineEdit_colliFoci->setMaxLength(10);
    ui->lineEdit_grayThre->setValidator(doubleValidator);
    ui->lineEdit_grayThre->setMaxLength(10);
    ui->lineEdit_circleRect->setValidator(doubleValidator);
    ui->lineEdit_circleRect->setMaxLength(10);*/

    // 单独创建一些qss暂时无法改变的样式
    ui->lineEdit_wavelengthTrans->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    // ui->lineEdit_officialLensTransStand->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    // ui->lineEdit_originalOpticalPower->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    // ui->lineEdit_officialLensOpticalPower->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_officialLensOpticalPower->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    // ui->lineEdit_officialLensTrans->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_officialLensTrans->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_testLensOpticalPower->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    // ui->lineEdit_testLensTrans->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->pushButton_addListOffi->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->pushButton_addListTest->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_minGray->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_maxGray->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidX->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidY->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidX_Limit->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidY_Limit->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidX_Limit2->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_centroidY_Limit2->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_leftUpPointX->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_leftUpPointY->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_rightDownPointX->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_rightDownPointY->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_officialNoiseLevel->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_totalGray1->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_testNoiseLevel->setStyleSheet("background-color: rgba(255, 255, 255, 125);");
    ui->lineEdit_totalGray2->setStyleSheet("background-color: rgba(255, 255, 255, 125);");

    updateTransmittance();
    QDoubleValidator *validatorWavelength = new  QDoubleValidator(0.0,20000.0,4,this);
    validatorWavelength->setNotation(QDoubleValidator::StandardNotation);
    ui->lineEdit_wavelengthTrans->setValidator(validatorWavelength);
    ///当用户修改lineEdit_wavelengthTrans的值时，lineEdit_officialLensTrans自动从excel表格中获取相应数值
    connect(ui->lineEdit_wavelengthTrans,&QLineEdit::editingFinished,this,&MainWindow::updateTransmittance);

    // 创建一个 Dialog 对象
    // Dialog dialog(this);//注意，这里的diaUi是对象②
    // diaUi = new Dialog(this);//注意，这里的diaUi是对象③
    vectorPixelList = diaUi->getLineEditText(); //使用vector列表装像元尺寸,注意，每次打开对话框并闭关时要重复执行该操作
    //设置checkbox排他性
    // 创建一个QButtonGroup并将checkBox添加进去
    QButtonGroup *checkGroupLight = new QButtonGroup(this);
    checkGroupLight->setExclusive(true);  // 设置排他性
    QButtonGroup *checkGroupObject = new QButtonGroup(this);
    checkGroupObject->setExclusive(true);
    QButtonGroup *checkGroupDistortion = new QButtonGroup(this);
    checkGroupDistortion->setExclusive(true);
    QButtonGroup *checkGroupMTF = new QButtonGroup(this);
    checkGroupMTF->setExclusive(true);
    // 将界面上的QCheckBox添加到组

    ///////////////// 2025.2.26 设置光源和相机之间的绑定/////////////////////////////////////
    checkGroupLight->addButton(ui->checkBox_kjg);
    checkGroupLight->addButton(ui->checkBox_jhw);
    checkGroupLight->addButton(ui->checkBox_dbhw);
    checkGroupLight->addButton(ui->checkBox_zbhw);
    checkGroupLight->addButton(ui->checkBox_cbhw);
    checkGroupLight->addButton(ui->checkBox_externalDetector);
    // checkGroupLight->addButton(ui->checkBox_test);
    ui->comboBox_camera->addItem("可见光、近红外探测器（未连接）", 0); // 可见光、近红外
    ui->comboBox_camera->addItem("短波红外探测器（未连接）", 1); // 短波红外
    ui->comboBox_camera->addItem("中波红外探测器（未连接）", 2); // 中波红外
    ui->comboBox_camera->addItem("长波红外探测器（未连接）", 3); // 长波红外
    // ui->comboBox_camera->addItem("测试用hik可见光相机（未连接）", 4); // 长波红外
    // 根据当前选中的光源设置默认相机
    auto updateCameraIndex = [=]() {
        if (ui->checkBox_kjg->isChecked() || ui->checkBox_jhw->isChecked()) {
            ui->comboBox_camera->setCurrentIndex(0); // 可见光和近红外共用一个可见光相机0
        } else if (ui->checkBox_dbhw->isChecked()) { // 短波红外一个相机1
            ui->comboBox_camera->setCurrentIndex(1);
        } else if (ui->checkBox_zbhw->isChecked()) { // 中波红外一个相机2
            ui->comboBox_camera->setCurrentIndex(2);
        } else if (ui->checkBox_cbhw->isChecked()) { // 长波红外一个相机3
            ui->comboBox_camera->setCurrentIndex(3);
        }
        // else if (ui->checkBox_test->isChecked()) { // 测试相机4
        //     ui->comboBox_camera->setCurrentIndex(4);
        // }
    };
    // 连接所有checkbox的信号到更新函数
    connect(ui->checkBox_kjg, &QCheckBox::toggled, this, updateCameraIndex);
    connect(ui->checkBox_jhw, &QCheckBox::toggled, this, updateCameraIndex);
    connect(ui->checkBox_dbhw, &QCheckBox::toggled, this, updateCameraIndex);
    connect(ui->checkBox_zbhw, &QCheckBox::toggled, this, updateCameraIndex);
    connect(ui->checkBox_cbhw, &QCheckBox::toggled, this, updateCameraIndex);
    // connect(ui->checkBox_test, &QCheckBox::toggled, this, updateCameraIndex);
    // 添加 comboBox 切换时自动选择对应的 checkbox
    connect(ui->comboBox_camera, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int index) {
                switch(index) {
                case 0:
                    ui->checkBox_kjg->setChecked(true);
                    break;
                case 1:
                    ui->checkBox_dbhw->setChecked(true);
                    break;
                case 2:
                    ui->checkBox_zbhw->setChecked(true);
                    break;
                case 3:
                    ui->checkBox_cbhw->setChecked(true);
                    break;
                // case 4:
                //     ui->checkBox_test->setChecked(true);
                //     break;
                }
            });
    // 相机参数设置按钮
    // connect(ui->pushButton_setCameraPara,&QPushButton::clicked,this,[=](){
    //     hikCameraParaSetUi->setWindowTitle("相机参数设置");
    //     hikCameraParaSetUi->show();
    // });
    // // 创建HIK相机对象，已经移到成员变量中去了
    // HikCameraController *hikCameraController1 = new HikCameraController(this);
    // 接收来自子UI的相机参数设置
    // connect(hikCameraParaSetUi, &hikParaSetDialog::cameraParametersChanged,
    //         this, [=](float expoTimeUs, float gainDb, float frameRate){
    //             if (hikCameraController1 && ui->comboBox_camera->currentIndex() == 0) {
    //                 hikCameraController1->setCameraPara(expoTimeUs, gainDb, frameRate);
    //             }
    //         });
    // 将扫描到的相机添加到combobox中
    // connect(hikCameraController1,&HikCameraController::cameraInfoSignal,this,[=](QString cameraName){
    //     qDebug()<<"刷新后的相机名"<<cameraName;
    //     // 根据相机名称判断类型并添加到对应位置
    //     if (cameraName.contains("MV-CS020-10UM")) {  // 假设这是可见光相机的型号
    //         ui->comboBox_camera->setItemText(4, cameraName);}
    // });
    connect(baslerCameraController1,&CGuiCamera::cameraInfoSignal,this,[=](QString cameraName){

        // 根据相机名称判断类型并添加到对应位置
        if (cameraName.contains("a2A1920-160umBAS")) {  // 假设这是可见光相机的型号
            ui->comboBox_camera->setItemText(0, cameraName);}
    });

    // 程序打开时就运行☆☆☆每个相机☆☆的扫描函数，注意，仅仅是运行扫描函数并将可用相机添加到相机的combobox中，并不打开相机
    // hikCameraController1->scanCamera(); // 这里可能导致崩溃，因为hikCameraController1未初始化
    baslerCameraController1->scanCamera(); // 初始化Basler相机
    // // 初始化时刷新相机列表
    // saperaCameraController1->updateCameraList(); // 初始化Sapera相机
    // 当点击刷新相机按钮时，每个相机的扫描函数
    connect(ui->pushButton_refreshCamera,&QPushButton::clicked,this,[=](){
        // 如果当前是镜头测试模式，使用自带相机
        // if(ui->checkBox_lens->isChecked()){
        if(!ui->checkBox_externalDetector->isChecked()){
            // //海康相机
            // if (hikCameraController1) {
            //     hikCameraController1->scanCamera();
            // } else {
            //     qDebug() << "Camera controller not initialized!";
            // }
            // Basler相机
            if(baslerCameraController1) {
                baslerCameraController1->scanCamera(); // 初始化Basler相机
            }
        }
        // 如果当前是整机测试模式，开始调用cameralink采集卡函数
        else{
            // 清空现有列表
            ui->comboBox_camera_external->clear();
            saperaCameraController1->clearCameraNames();
            // 扫描相机
            saperaCameraController1->updateCameraList();
        }
    });
    // 刷新cameralink相机列表
    connect(saperaCameraController1,&SaperaCameraController::updateCameraListSignal,this,[=](QStringList cameraList){
        for(int i = 0; i < cameraList.size(); i++){
            ui->comboBox_camera_external->addItem(cameraList[i]);
        }
    });
    // // 连接相机选择信号
    // connect(ui->comboBox_camera_external, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         saperaCameraController1,&SaperaCameraController::onCameraSel);

    // 初始化时刷新相机列表
    // saperaCameraController1->updateCameraList(); // 初始化Sapera相机 注意：这一行必须位于上面槽函数的后面才能正常执行



    // 来自Sapera的警告信号槽函数
    ////////////////////////////////////////////////////////////////////////////

    checkGroupObject->addButton(ui->checkBox_lens);
    checkGroupObject->addButton(ui->checkBox_overall);
    checkGroupDistortion->addButton(ui->checkBox_2v2);
    checkGroupDistortion->addButton(ui->checkBox_3v3);
    checkGroupDistortion->addButton(ui->checkBox_customize);
    checkGroupMTF->addButton(ui->checkBox_2v2_3);
    checkGroupMTF->addButton(ui->checkBox_3v3_3);
    checkGroupMTF->addButton(ui->checkBox_customize_3);

    // 设置打开程序时默认情况下的用户对于像元尺寸的可修改权限
    if(ui->checkBox_lens->isChecked()){
        //当默认选择镜头测试时，可以点击修改按钮，但无法自定义编辑像元尺寸
        ui->pushButton_defaultPixelSize->setEnabled(true);
        ui->lineEdit_pixelSize->setEnabled(false);
    }
    else{
        //当默认选择镜头测试时，可以点击修改按钮，但无法自定义编辑像元尺寸
        ui->pushButton_defaultPixelSize->setEnabled(false);
        ui->lineEdit_pixelSize->setEnabled(true);
    }
    // 设置点击镜头和整机checkbox时的不同操作
    connect(ui->checkBox_lens,&QCheckBox::clicked,this,&MainWindow::onCheckboxLensClicked);
    connect(ui->checkBox_overall,&QCheckBox::clicked,this,&MainWindow::onCheckboxOverallClicked);
    if(ui->checkBox_overall->isChecked()){
        ui->pushButton_defaultPixelSize->setEnabled(false);
        ui->pushButton_defaultPixelSize->setToolTip("整机测试状态下不可修改相元");
    }
    else{
        ui->pushButton_defaultPixelSize->setEnabled(true);
        ui->pushButton_defaultPixelSize->setToolTip("修改相机相元尺寸");
    }
    // 点击修改按钮时，弹出对话框，修改默认的像元尺寸
    connect(ui->pushButton_defaultPixelSize,&QPushButton::clicked,this,&MainWindow::onPushButtonDefPixSizeClicked);
    // 点击对话框确认按钮时自动修改像元显示,    //点击确定按钮时所有的像元lineEdit刷新
    connect(diaUi->ui->buttonBox,&QDialogButtonBox::accepted,this,[=](){
        vectorPixelList = diaUi->getLineEditText(); //使用vector列表装像元尺寸,注意，每次打开对话框并闭关时要重复执行该操作，实现及时刷新
        if(ui->checkBox_kjg->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[0]));
        }
        else if(ui->checkBox_jhw->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[1]));
        }
        else if(ui->checkBox_dbhw->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[2]));
        }
        else if(ui->checkBox_zbhw->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[3]));
        }
        else{
             ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[4]));
        }
    });
    //点击波长checkbox时也同时修改像元
    //①方法一，使用Lambda函数
    //获取对话框中的数据设置
    connect(ui->checkBox_kjg,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_lens->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[0]));
        }
    });
    connect(ui->checkBox_jhw,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_lens->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[1]));
        }
    });
    connect(ui->checkBox_dbhw,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_lens->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[2]));
        }
    });
    connect(ui->checkBox_zbhw,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_lens->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[3]));
        }
    });
    connect(ui->checkBox_cbhw,&QCheckBox::clicked,this,[=](){
        if(ui->checkBox_lens->isChecked()){
            ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[4]));
        }
    });
    //方法二，使用统一的槽函数
    connect(ui->pushButton_power,&QPushButton::clicked,this,&MainWindow::changeButtonPowerColor);// 总体电源按钮的显示颜色切换、点击按钮槽函数
    connect(ui->pushButton_start,&QPushButton::clicked,this,&MainWindow::ButtonStart);// 点击右侧开始按钮槽函数
    /// 注意，下面这行代码是用于控制按钮的Icon切换的，但是由于这个按钮点击时还连接着打开相机的另一个槽函数，那个槽函数在运行时会阻塞线程，导致按钮状态更新迟滞
    // connect(ui->pushButton_openCamera,&QPushButton::clicked,this,&MainWindow::changeButtonIcon);// 1. 总体启动按钮的显示图标、颜色切换、点击按钮槽函数    2. 开启和关闭图像显示
    connect(ui->pushButton_addToList,&QPushButton::clicked,this,&MainWindow::addToList);//加入列表
    connect(ui->pushButton_aveFocalLength,&QPushButton::clicked,this,&MainWindow::aveFocalLength);//计算平均焦距
    connect(ui->pushButton_clearDistortion,&QPushButton::clicked,this,&MainWindow::clearXYInput); // 表格清空按钮
    connect(ui->pushButton_clearDistortion_3,&QPushButton::clicked,this,&MainWindow::clearXYInputMTF); // 表格清空按钮MTF
    connect(ui->pushButton_init,&QPushButton::clicked,this,&MainWindow::init);//点击初始化按钮时产生不同的视场点
    connect(ui->pushButton_init_3,&QPushButton::clicked,this,&MainWindow::initMTF);//点击初始化按钮时产生不同的视场点


    /////////////////////////////////////////////////Hik相机///////////////////////////////////////////////////////
    // 初始化图像场景和视图
    pixmapItem = new QGraphicsPixmapItem();
    scene = new QGraphicsScene(this);
    scene->addItem(pixmapItem);

    // 初始化区域矩形框图层
    rectItem = new QGraphicsRectItem(); // 绘制椭圆或圆形图层
    rectItem->setPen(QPen(Qt::white,0.5)); // 使用白色画笔
    // 矩形需要4个参数：x，y，长，宽
    rectLeftUpX = ui->lineEdit_leftUpPointX->text().toFloat();
    rectLeftUpY = ui->lineEdit_leftUpPointY->text().toFloat();
    rectRightDownX = ui->lineEdit_rightDownPointX->text().toFloat();
    rectRightDownY = ui->lineEdit_rightDownPointY->text().toFloat();
    rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
    connect(ui->lineEdit_leftUpPointX,&QLineEdit::editingFinished,this,[=](){
        rectLeftUpX = ui->lineEdit_leftUpPointX->text().toFloat();
    rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
    });
    connect(ui->lineEdit_leftUpPointY,&QLineEdit::editingFinished,this,[=](){
        rectLeftUpY = ui->lineEdit_leftUpPointY->text().toFloat();
    rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
    });
    connect(ui->lineEdit_rightDownPointX,&QLineEdit::editingFinished,this,[=](){
        rectRightDownX = ui->lineEdit_rightDownPointX->text().toFloat();
    rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
    });
    connect(ui->lineEdit_rightDownPointY,&QLineEdit::editingFinished,this,[=](){
        rectRightDownY = ui->lineEdit_rightDownPointY->text().toFloat();
    rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
    });
    rectItem->setVisible(false);  // 开始时隐藏
    scene->addItem(rectItem);  // 将质心图层添加到场景中

    // 初始化噪声


    // 初始化质心图层
    /// 注意：这里的QGraphicsEllipseItem（圆环）在固定在特定坐标位置时是以其外接矩形的左上角坐标为基准的而不是圆心，这也就导致了显示的标记的圆心并不在质心上
    /// 解决方案：当设置圆环显示的坐标时，在质心坐标的基础上，x和y都要减去一个半径circleRect/2
    centroidItem = new QGraphicsEllipseItem(); // 绘制椭圆或圆形图层
    centroidItem->setPen(QPen(Qt::red, 3));  // 使用红色画笔
    circleRect = ui->lineEdit_circleRect->text().toFloat();
    centroidItem->setRect(0, 0, circleRect, circleRect);  // 初始大小和位置，这里要设计成ui界面可以实时刷新
    connect(ui->lineEdit_circleRect,&QLineEdit::editingFinished,this,[=](){ // 实时根据用户输入而刷新
        circleRect = ui->lineEdit_circleRect->text().toFloat();
    });
    centroidItem->setVisible(false);  // 开始时隐藏
    scene->addItem(centroidItem);  // 将质心图层添加到场景中

    // 初始化区域质心图层
    centroidItemLimit = new QGraphicsEllipseItem(); // 绘制椭圆或圆形图层
    centroidItemLimit->setPen(QPen(Qt::blue, 0.5));  // 使用蓝色画笔
    circleRectLimit = ui->lineEdit_circleRectLimit->text().toFloat();
    centroidItemLimit->setRect(0, 0, circleRectLimit, circleRectLimit);  // 初始大小和位置，这里要设计成ui界面可以实时刷新
    connect(ui->lineEdit_circleRectLimit,&QLineEdit::editingFinished,this,[=](){ // 实时根据用户输入而刷新
        circleRectLimit = ui->lineEdit_circleRectLimit->text().toFloat();
    });
    centroidItemLimit->setVisible(false);  // 开始时隐藏
    scene->addItem(centroidItemLimit);  // 将质心图层添加到场景中

    ui->graphicsView->setScene(scene);
    // 获取 handle
    // void* cameraHandle1 = hikCameraController1->getHandle(); // 注意，句柄是在openCamera执行以后才产生，这个函数如果执行过早获取的句柄就是0x0
    // qDebug()<<"句柄："<<cameraHandle1;
    // 创建图像采集线程和对象
    QThread *t1_hikGrabberThread = new QThread(this);
    // ImageGrabber *grabber = new ImageGrabber(cameraHandle1);
    // CGuiCamera *grabberBasler = new CGuiCamera;


    /// 实现对灰度阈值的实时检测，手动修改后自动应用于图像处理而无需重启相机
    connect(ui->lineEdit_grayThre,&QLineEdit::editingFinished,[=](){
        QString grayThre = ui->lineEdit_grayThre->text();
        qDebug()<<"输入框最新灰度阈值："<<grayThre;
        emit updateGrayThre(grayThre);
    });

    // 更新灰度阈值信号
    connect(this,&MainWindow::updateGrayThre,saperaCameraController1,[=](QString &grayThre){
        qDebug()<<"信号被成功接收sapera";
        qDebug()<<grayThre;
        // grabber->getGrayThreUi(grayThre);
        saperaCameraController1->getGrayThreUi(grayThre);
    },Qt::DirectConnection); // 如果使用 Qt::QueuedConnection，需要确保grabber对象在与信号发射的对象不同的线程中。如果它们在同一个线程中，需要使用 Qt::DirectConnection
    // 更新灰度最大最小限制
    saperaCameraController1->getMinMaxVal(ui->lineEdit_minGray->text().toInt(), ui->lineEdit_maxGray->text().toInt());
    connect(ui->lineEdit_minGray,&QLineEdit::editingFinished,[=](){
        saperaCameraController1->getMinMaxVal(ui->lineEdit_minGray->text().toInt(), ui->lineEdit_maxGray->text().toInt());
    });
    connect(ui->lineEdit_maxGray,&QLineEdit::editingFinished,[=](){
        saperaCameraController1->getMinMaxVal(ui->lineEdit_minGray->text().toInt(), ui->lineEdit_maxGray->text().toInt());
    });
    // 更新矩形区域限制
    int leftUpXGet = ui->lineEdit_leftUpPointX->text().toInt();
    int leftUpYGet = ui->lineEdit_leftUpPointY->text().toInt();
    int rightDownXGet = ui->lineEdit_rightDownPointX->text().toInt();
    int rightDownYGet = ui->lineEdit_rightDownPointY->text().toInt();
    int officialNoiseLevel = ui->lineEdit_officialNoiseLevel->text().toInt();
    int testNoiseLevel = ui->lineEdit_testNoiseLevel->text().toInt();

    saperaCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    baslerCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    saperaCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
    baslerCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
    connect(ui->lineEdit_officialNoiseLevel,&QLineEdit::editingFinished,[=](){
        int officialNoiseLevel = ui->lineEdit_officialNoiseLevel->text().toInt();
        int testNoiseLevel = ui->lineEdit_testNoiseLevel->text().toInt();
        saperaCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
        baslerCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
    });
    connect(ui->lineEdit_testNoiseLevel,&QLineEdit::editingFinished,[=](){
        int officialNoiseLevel = ui->lineEdit_officialNoiseLevel->text().toInt();
        int testNoiseLevel = ui->lineEdit_testNoiseLevel->text().toInt();
        saperaCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
        baslerCameraController1->getNoiseLevel(officialNoiseLevel,testNoiseLevel);
    });

    connect(ui->lineEdit_leftUpPointX,&QLineEdit::editingFinished,[=](){
        // 更新矩形区域限制
        int leftUpXGet = ui->lineEdit_leftUpPointX->text().toInt();
        int leftUpYGet = ui->lineEdit_leftUpPointY->text().toInt();
        int rightDownXGet = ui->lineEdit_rightDownPointX->text().toInt();
        int rightDownYGet = ui->lineEdit_rightDownPointY->text().toInt();
        saperaCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
        baslerCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    });
    connect(ui->lineEdit_leftUpPointY,&QLineEdit::editingFinished,[=](){
        // 更新矩形区域限制
        int leftUpXGet = ui->lineEdit_leftUpPointX->text().toInt();
        int leftUpYGet = ui->lineEdit_leftUpPointY->text().toInt();
        int rightDownXGet = ui->lineEdit_rightDownPointX->text().toInt();
        int rightDownYGet = ui->lineEdit_rightDownPointY->text().toInt();
        saperaCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
        baslerCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    });
    connect(ui->lineEdit_rightDownPointX,&QLineEdit::editingFinished,[=](){
        // 更新矩形区域限制
        int leftUpXGet = ui->lineEdit_leftUpPointX->text().toInt();
        int leftUpYGet = ui->lineEdit_leftUpPointY->text().toInt();
        int rightDownXGet = ui->lineEdit_rightDownPointX->text().toInt();
        int rightDownYGet = ui->lineEdit_rightDownPointY->text().toInt();
        saperaCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
        baslerCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    });
    connect(ui->lineEdit_rightDownPointY,&QLineEdit::editingFinished,[=](){
        // 更新矩形区域限制
        int leftUpXGet = ui->lineEdit_leftUpPointX->text().toInt();
        int leftUpYGet = ui->lineEdit_leftUpPointY->text().toInt();
        int rightDownXGet = ui->lineEdit_rightDownPointX->text().toInt();
        int rightDownYGet = ui->lineEdit_rightDownPointY->text().toInt();
        saperaCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
        baslerCameraController1->getLimitArea(leftUpXGet, leftUpYGet ,rightDownXGet ,rightDownYGet);
    });


    connect(this,&MainWindow::updateGrayThre,baslerCameraController1,[=](QString &grayThre){
        qDebug()<<"信号被成功接收basler";
        qDebug()<<grayThre;
        // grabber->getGrayThreUi(grayThre);
        baslerCameraController1->getGrayThreUi(grayThre);
    },Qt::DirectConnection); // 如果使用 Qt::QueuedConnection，需要确保grabber对象在与信号发射的对象不同的线程中。如果它们在同一个线程中，需要使用 Qt::DirectConnection

    // 程序刚打开时就读取ui界面的默认灰度阈值设置
    QString grayThre = ui->lineEdit_grayThre->text();
    emit updateGrayThre(grayThre);

    // 设置对象和线程的关联
    // grabber->moveToThread(t1_hikGrabberThread);
    baslerCameraController1->moveToThread(t1_hikGrabberThread); // 暂时共用一个线程，因为同时只能打开一个相机

    //连接信号和槽
    // connect(t1_hikGrabberThread, &QThread::started, grabber, &ImageGrabber::startGrabbing);
    ImageProcessThread *imageProcessThread = new ImageProcessThread(this);


    // connect(t1_hikGrabberThread, &QThread::started, grabber, [=](){
    //     // QString grayThre = ui->lineEdit_grayThre->text();
    //     // 获取当前相机的选择
    //     QString currentCamera = ui->comboBox_camera->currentText();
    //     if(currentCamera.contains("MV-CS020-10UM")){
    //         grabber->startGrabbing(imageProcessThread);
    //     }
    //     else{
    //         qDebug()<<"当前相机不是MV-CS020-10UM";
    //     }
    // });
    connect(t1_hikGrabberThread, &QThread::started, baslerCameraController1, [=](){
        // QString grayThre = ui->lineEdit_grayThre->text();
        // 获取当前相机的选择
        QString currentCamera = ui->comboBox_camera->currentText();
        if(currentCamera.contains("a2A1920-160umBAS")){
            baslerCameraController1->startGrabbing(imageProcessThread);
        }
        else{
            qDebug()<<"当前相机不是a2A1920-160umBAS";
        }
    });

    // connect(grabber, &ImageGrabber::finished, t1_hikGrabberThread, &QThread::quit);
    // connect(grabber, &ImageGrabber::finished, grabber, &ImageGrabber::deleteLater);
    // connect(t1_hikGrabberThread, &QThread::finished, t1_hikGrabberThread, &QThread::deleteLater);
    // connect(grabber, &ImageGrabber::imageGrabbed, this, &HikCameraController::newImageCaptured);
    // 连接来自startGrabbing()的QImage数据到ImageProcessThread->执行其中的图像处理函数onFrameAvailable

    /// 这一段代码在这里是无法加入子线程的
    /*connect(grabber,&ImageGrabber::imageGrabbed,imageProcessThread,[=](const QImage& image){
        QString grayThre = ui->lineEdit_grayThre->text();
        imageProcessThread->onFrameAvailable(image,grayThre);
    },Qt::QueuedConnection);*/

    // 连接来自imageProcessThread处理后的图像信号sendSignalToUi和主ui界面
    connect(imageProcessThread,&ImageProcessThread::sendSignalToUi,this,[=](const QPixmap &pixmap, const cv::Point2d &centerPoint, const cv::Point2d &centerPointLimit, double totalGray_official, double totalGray_test){
        // 将灰度和先输出
        // ui->lineEdit_totalGray1->setText(QString::number(totalGray));
        // ui->lineEdit_totalGray2->setText(QString::number(totalGray));
        // 将灰度和先输出
        ui->lineEdit_totalGray1->setText(QString::number(totalGray_official));
        ui->lineEdit_totalGray2->setText(QString::number(totalGray_test));
        onFrameAvailable1(pixmap,centerPoint, centerPointLimit);
    },Qt::QueuedConnection);  // 必须指定连接类型);

    // 给cameralink相机单独创建一个槽函数的链接
    /// 注意（笔记）：Qt 为了避免不可预测的副作用和引用生命周期问题，在使用 lambda 作为槽函数时，不允许将非常量引用参数传入 lambda。
    /// 所以下面的槽函数中：
    /// 如果信号是 void signal(QImage &)：
    /// 不能绑定到 [](QImage &)（lambda 参数必须是 const QImage& 或 QImage）
    /// 可以绑定到 [](const QImage &)（只读引用）
    /// 可以绑定到 [](QImage)（值拷贝）
    /// 可以理解为const和&同时存在才行，否则这个槽函数根本无法被调用，没有任何反应
    connect(saperaCameraController1,&SaperaCameraController::cameralinkSignal,this,[=](const QImage &qimageNew, const cv::Point2d centerPoint, const cv::Point2d centerPointLimit,double totalGray_official, double totalGray_test){
        // 先将灰度总和显示出来
        // ui->lineEdit_totalGray1->setText(QString::number(totalGray));
        // ui->lineEdit_totalGray2->setText(QString::number(totalGray));
        ui->lineEdit_totalGray1->setText(QString::number(totalGray_official));
        ui->lineEdit_totalGray2->setText(QString::number(totalGray_test));
        // qDebug()<<"开始cameralinkSignal";
        // 基于QImage计算质心
        /// 笔记：这里将QImage直接加载进信号传递给主线程，成功显示出图像，但是主UI会卡顿，原因是下面的质心计算在主线程中执行
        QImage grayImage;
        if (qimageNew.format() != QImage::Format_Grayscale8) {
            grayImage = qimageNew.copy().convertToFormat(QImage::Format_Grayscale8);
        } else {
            grayImage = qimageNew.copy();
        }

        if (grayImage.isNull()) {
            QMessageBox::warning(this,"Error","Failed to convert image to grayscale！");
            return;
        }

        cv::Mat matImage = QImageToMat(grayImage);
        if (matImage.empty()) {
            QMessageBox::warning(this,"Error","Failed to convert QImage to cv::Mat！");
            return;
        }

        // 获取ui界面灰度阈值
        double grayScaleUi = ui->lineEdit_grayThre->text().toDouble();

        emit updateGrayThreSapera(grayScaleUi);
        // cv::Point2d centerPoint = grayCenter(matImage, grayScaleUi);

        // grayImage.setColorSpace(QColorSpace(QColorSpace::SRgbLinear)); // 设置颜色空间
        // 将qimageNew转换为QPixmap格式
        QPixmap pixmapNew = QPixmap::fromImage(grayImage);

        // 直接将pixmap显示在ui界面中
        onFrameAvailable1(pixmapNew,centerPoint, centerPointLimit);
    });

    connect(this,&MainWindow::updateGrayThreSapera,saperaCameraController1,[=](double grayThreUi){
        saperaCameraController1->updateGrayThre(grayThreUi);
    });

    // 计算矩形区域的灰度和，用于透过率的测试中
    connect(ui->pushButton_addGrayscale1,&QPushButton::clicked,this,[=](){
        ui->lineEdit_officialLensOpticalPower->setText(ui->lineEdit_totalGray1->text());
    });
    connect(ui->pushButton_addGrayscale2,&QPushButton::clicked,this,[=](){
        ui->lineEdit_testLensOpticalPower->setText(ui->lineEdit_totalGray2->text());
    });

    // 接收来自hik相机的携带有QImage数据的信号
    // connect(hikCameraController1,&HikCameraController::newImageCaptured,this,[=](const QImage& image){
    //     QMetaObject::invokeMethod(this, [this, image](){
    //         // 确保UI操作在主线程
    //         QPixmap pix = QPixmap::fromImage(image);
    //         if(!pix.isNull()){
    //             pixmapItem_hik->setPixmap(pix);
    //         }
    //     });
    // }, Qt::QueuedConnection);
    // 添加连接

    // // 初始化图像场景和视图
    // pixmapItem_hik = new QGraphicsPixmapItem();
    // scene_hik = new QGraphicsScene(this);
    // scene_hik->addItem(pixmapItem_hik);
    // connect(hikCameraController1, &HikCameraController::newImageCaptured, this, [=](const QImage& image){
    //     // QGraphicsScene* scene = new QGraphicsScene(this);
    //     scene_hik->addPixmap(QPixmap::fromImage(image));
    //     // ui->graphicsView->setScene(scene);
    //     ui->graphicsView->fitInView(scene_hik->sceneRect(), Qt::KeepAspectRatio);
    // });

    /////////////////////////////////////////////////笔记本相机///////////////////////////////////////////////////////
    //图像窗口 需要放入单独的线程中
    // 创建摄像头和媒体捕获会话
    camera = new QCamera(this);  //摄像头对象,从设备中捕获视频流  注意：this指的是MainWindow的实例，
    // QCamera会把MainWindow实例作父对象，这样当Mainwindow被销毁时，QCamera也会自动销毁
    videoSink = new QVideoSink(this); //视频接收器，起到接收端的作用
    captureSession = new QMediaCaptureSession(this);//将摄像头和接收器关联起来，确保视频流能够正确传递，管理视频流传递，起到纽带的作用

    // 设置摄像头和视频输出
    captureSession->setCamera(camera);
    captureSession->setVideoSink(videoSink);

    // // 初始化图像场景和视图
    // pixmapItem = new QGraphicsPixmapItem();
    // scene = new QGraphicsScene(this);
    // scene->addItem(pixmapItem);

    // // 初始化质心图层
    // centroidItem = new QGraphicsEllipseItem(); // 绘制椭圆或圆形图层
    // centroidItem->setPen(QPen(Qt::red, 3));  // 使用红色画笔
    // centroidItem->setRect(0, 0, 20, 20);  // 初始大小和位置
    // centroidItem->setVisible(false);  // 开始时隐藏
    // scene->addItem(centroidItem);  // 将质心图层添加到场景中

    // ui->graphicsView->setScene(scene);
    /*
    笔记：QCamera是相机对象，是图像视频流的输出端，但其只负责产生视频帧并输出
    QVideoSink作为接收端，相当于一个容器，接收来自QCamera的视频流
    由于这里的QCamera可以有很多个，QVideoSink也可以有很多个，所以，为了将特定的相机输出到特定的容器中，我们需要QMediaCaptureSession来绑定QCamera和QVideoSink
    对于每一帧图像，首先必须转换成QPixmap的格式，转换后的图像放在pixmapItem中，pixmapItem可以看成一个pixmap的容器
    我们需要一个场景来展示容器中的pixmap图像，于是有了QGraphicsScene
    但是一个场景也需要放进ui界面中的一个组件中才能展示，这个组件叫graphicsView
    */

    qDebug() << "Main thread ID:" << QThread::currentThreadId();

    //创建线程
    //方法1：使用这种方法时没有指定父对象，所以需要手动管理内存的释放，需要在析构函数中加上delete语句，当变量 t1 没有被定义为类的成员，无法析构
    // QThread *t1 = new QThread(this);
    // image_Thread *imageThread = new image_Thread(this);
    //方法2：使用这种方法时指定了父对象，则不用手动管理内存释放，父对象析构时一起自动析构，则在析构函数中无需加入delete t1 和delete imageThread
    t1 = new QThread();
    imageThread = new image_Thread();

    // 在 MainWindow 构造函数中连接信号槽
    // connect(this, &MainWindow::aboutToClose, t1, &QThread::quit);
    connect(this, &MainWindow::aboutToClose, t1, [=](){
        // 关闭自带相机线程
        qDebug() << "Thread t1 state before quit:" << t1->isRunning();
        disconnect(t1, nullptr, this, nullptr); // 断开所有信号槽连接
        t1->quit();      // 请求线程退出
        t1->wait();
        qDebug() << "Thread t1 state after quit:" << t1->isRunning();


    });
    /// 关闭主程序时的线程销毁
    connect(this, &MainWindow::aboutToClose, this, [=](){

        qDebug() << "Thread t1_hikGrabberThread state before quit:" << t1_hikGrabberThread->isRunning();
        // disconnect(t1_hikGrabberThread, nullptr, this, nullptr); // 断开所有信号槽连接 在停止时调用了 disconnect，重新启动时需要 重新连接信号槽，否则通信会失效。

        ///停止线程
        t1_hikGrabberThread->quit();      // 请求线程退出
        t1_hikGrabberThread->wait();
        disconnect(t1_hikGrabberThread, nullptr, this, nullptr); // 断开所有信号槽连接

        qDebug() << "Thread t1_hikGrabberThread state after quit:" << t1_hikGrabberThread->isRunning();
    });

    // connect(this, &MainWindow::aboutToClose, t1, &QThread::wait);
    // connect(this, &MainWindow::aboutToClose, imageThread, &QObject::deleteLater);
    // 将 imageThread 移动到 t1 子线程，并确保在 t1 启动后再执行 moveToThread
    // connect(t1, &QThread::started, imageThread, &image_Thread::);  // 启动线程事件循环
    // 连接信号槽
    connect(t1, &QThread::started, imageThread, &image_Thread::initialize); //t1线程启动时会进行初始化操作
    connect(imageThread, &image_Thread::error, this, &MainWindow::handleError); // 线程错误时的操作
    // connect(t1_hikGrabberThread, &QThread::started, imageProcessThread, &ImageProcessThread::initialize); //t1线程启动时会进行初始化操作
    connect(imageProcessThread, &ImageProcessThread::error, this, &MainWindow::handleError); // 线程错误时的操作

    // 启动线程
    imageThread->moveToThread(t1);
    /// t1->start(); // 电脑自带相机的线程
    // 当新的视频帧可用时，连接到槽函数
    // connect(videoSink, &QVideoSink::videoFrameChanged, imageThread, &image_Thread::onFrameAvailable);
    connect(videoSink, &QVideoSink::videoFrameChanged, imageThread, [=](const QVideoFrame &frame) {
        QString grayThre = ui->lineEdit_grayThre->text(); // 获取 QLineEdit 数据
        imageThread->onFrameAvailable(frame, grayThre);
    },Qt::QueuedConnection);

    // connect(videoSink, &QVideoSink::videoFrameChanged, imageThread, [=](){
    //     image_Thread-onFrameAvailable;
    // });
    //☆☆☆注意：当连接 videoSink 的 videoFrameChanged 信号和 image_Thread::onFrameAvailable 槽时，Qt 会自动将信号中提供的参数（也就是代表视频帧的frame参数）传递给槽函数。☆☆☆
    //☆☆☆这样就回答了：为什么&image_Thread::onFrameAvailable并没有显式提供frame参数，但是却可以正常捕获到视频帧☆☆☆

    // connect(videoSink, &QVideoSink::videoFrameChanged, this, [=](const QVideoFrame &frame){ //注意此处的videoFrameChanged信号默认只发送给ui主线程，this处不能使用子线程对象
    //     //调用子线程中的函数image_Thread::onFrameAvailable
    //     // imageThread->onFrameAvailable(frame); //使用这行代码时，其实根本没有调用子线程，还是在主线程中运行的，只是调用了子线程类中的函数onFrameAvailable
    //     QMetaObject::invokeMethod(imageThread, "onFrameAvailable", Qt::QueuedConnection, Q_ARG(QVideoFrame, frame)); //
    // });

    // lastImageUpdateTimer.start();  // 初始化记录器
    // 设置定时器，每秒计算一次质心
    // QTimer *timer1 = new QTimer(this);
    // 定时器的超时检查
    // connect(timer1, &QTimer::timeout, this, [=]() {
    //     // 如果超过 1 秒未收到信号，则停止定时器
    //     if (lastImageUpdateTimer.elapsed() > 1000) { // 超过 1 秒未更新
    //         timer1->stop(); // 停止定时器
    //         qDebug() << "关闭 timer1: 超过 1 秒未收到信号";
    //     } else {
    //         // qDebug() << "定时器触发，继续工作...";
    //         // 执行质心计算逻辑
            // calculateCentroid();
    //     }
    // });
    // connect(timer1, &QTimer::timeout, this, &MainWindow::calculateCentroid);  // 定时器触发时调用质心计算
    //当接收到子线程图像发射信号时，显示图像
    // connect(imageThread,&image_Thread::sendSignalToUi,this,[=](const QPixmap &pixmap, const cv::Point2d &centerPoint , const cv::Point2d &centerPointLimit){
    //     qDebug()<<"sssss";
    //     onFrameAvailable1(pixmap,centerPoint, centerPointLimit);
    // });

    // 设置实时刷新光标坐标的状态显示
    // statusLabel = new QLabel(this);
    // statusBar()->addWidget(statusLabel); //设置状态栏，这里的状态栏显示在主窗口左下角，并不是图像窗口左下角


    // view = new NewGraphicsView(this);  // 这行在这里不对，初始化 view 此时是新建了一个对象，不是现有的相机窗口
    // 之前通过 Qt Designer 已经创建了一个 QGraphicsView，名为 graphicsView
    view = findChild<NewGraphicsView*>("graphicsView"); // ☆☆☆☆从现有的 UI 中找到 QGraphicsView☆☆☆☆
    // 注意：这里的view并不一定能指向某个目标，如果findChild失败，view将是一个空指针
    if (view && pixmapItem) {
        view->setPixmapItem(pixmapItem); // 假设你在 NewGraphicsView 中有这个方法
    }
    else{
        qDebug() << "view not found";
        return;  // 如果没有找到，退出函数（注意：是所在的整个函数）以防止空指针调用
    }
    // 创建 QLabel 来显示鼠标坐标
    QLabel *viewStatusLabel = new QLabel(view);  // 把 QLabel 放到 QGraphicsView 内
    viewStatusLabel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 128); }");
    viewStatusLabel->setFixedSize(200, 20);  // 设置标签大小
    // qDebug()<<"view->height:"<<view->height();
    viewStatusLabel->move(10, view->height()-30);  // 将 QLabel 放在视图的左下角

    // 创建 QLabel 来显示鼠标点击处的灰度值
    QLabel *viewStatusLabelPixel = new QLabel(view);  // 把 QLabel 放到 QGraphicsView 内
    viewStatusLabelPixel->setStyleSheet("QLabel { color: white; background-color: rgba(0, 0, 0, 128); }");
    viewStatusLabelPixel->setFixedSize(200, 20);  // 设置标签大小
    viewStatusLabelPixel->move(240, view->height()-30);  // 将 QLabel 放在视图的左下角

    // connect(view,&NewGraphicsView::mousePositionChanged,this,&MainWindow::updateMousePosition);
    connect(view,&NewGraphicsView::mousePositionChanged,this,[=](int x,int y,QPointF scenePos){
    // statusLabel->setText(QString("Mouse Position: [%1, %2]").arg(x).arg(y));
        if(pixmapItem->contains(pixmapItem->mapFromScene(scenePos))){  // ☆☆☆判断当前鼠标光标是否在pixmapItem内☆☆☆
        //解读：pixmapItem->mapFromScene(scenePos)：将大的场景坐标转变为以小的Item内容为基准的坐标
        //pixmapItem->contains():在这个总的坐标中，位于Item内部的点是满足if条件的
            viewStatusLabel->setText(QString("Mouse Position: [%1, %2]").arg(x).arg(y));
        }
    });

    connect(view,&NewGraphicsView::mouseClicked,this,[=](int pixelValue,QPointF scenePos){
        if(pixmapItem->contains(pixmapItem->mapFromScene(scenePos))){
            viewStatusLabelPixel->setText(QString("Pixel Value: [%1]").arg(pixelValue));
        }
    });

    /*
     * 笔记：
    mapFromScene
        功能: 将场景坐标转换为当前项（QGraphicsItem）的局部坐标。
        用法: 通过调用 item->mapFromScene(scenePos)，你可以将一个在场景中的坐标点转换为相对于该项的坐标。
        场景与局部坐标关系: 场景坐标是全局坐标系，而局部坐标是相对于某个特定图形项的坐标。因此，使用这个方法可以确定在特定图形项内部的坐标位置。
    mapToScene
        功能: 将当前项的局部坐标转换为场景坐标。
        用法: 通过调用 item->mapToScene(localPos)，你可以将一个在该项内部的局部坐标转换为全局场景坐标。
        局部与场景坐标关系: 这个方法允许你将图形项内部的操作或位置转换为场景中的位置，以便于与其他图形项或场景元素进行交互。
    */

    //注意：statusBar() 是 QMainWindow 的一部分，可以直接在继承自 QMainWindow 的类中使用，而不需要显式声明或创建状态栏

    // 连接按钮点击信号,根据光源的选择，执行不同的相机开启函数
    // connect(ui->pushButton_openCamera, &QPushButton::clicked, this, &MainWindow::startCamera);

    // 1. 点击打开  usb相机
    // 2. 点击关闭  usb相机
    // 3. 点击打开  sapera相机
    // 4. 点击关闭  sapera相机

    connect(ui->pushButton_openCamera, &QPushButton::clicked, this, [=](){ // 打开相机并进行图形抓取按钮
        changeButtonIcon();
        // if(ui->checkBox_lens->isChecked()){
        // 当选择非cameralink相机时
        if(!ui->checkBox_externalDetector->isChecked()){
            // 先把cameralink相机关了
            if(saperaCameraController1){
                saperaCameraController1->stopAcq();
            }
            // 获取当前相机的选择
            /// 问题1：这里有一个问题，就是打开和关闭相机是基于当前ui界面的相机选择，所以如果用户在相机打开的状态下切换了相机选择，那么在关闭相机时就会发出不是当前相机的关闭指令
            QString currentCamera = ui->comboBox_camera->currentText();
            // if(currentCamera.contains("MV-CS020-10UM")){
            //     if(ui->pushButton_openCamera->toolTip()=="相机关闭"){
            //         openHikCamera(); // 注意：经过测试，这个函数的运行是会阻塞主程序的，会导致相机开启按钮从未开启到开启的状态切换有迟滞，虽然代码行数不多，但这
            //             // 些相机操作都是同步的硬件通信操作，每一步都可能需要较长时间，这些操作都是阻塞式的，每个函数都会等待硬件响应完成才返回
            //         void* cameraHandle = hikCameraController1->getHandle(); // 注意，句柄是在openCamera执行以后才产生，这个函数如果执行过早获取的句柄就是0x0
            //         qDebug()<<"启动线程前的句柄："<<cameraHandle;
            //         // 设置句柄
            //         grabber->setHandle(cameraHandle);
            //         qDebug()<<"设置句柄成功";
            //         qDebug()<<t1_hikGrabberThread;
            //         t1_hikGrabberThread->start(); // 启动数据流抓取线程
            //         /// 其他相机的启动函数......
            //     }
            //     else{
            //         qDebug() << "Thread t1_hikGrabberThread state before quit:" << t1_hikGrabberThread->isRunning();
            //         // disconnect(t1_hikGrabberThread, nullptr, this, nullptr); // 断开所有信号槽连接 在停止时调用了 disconnect，重新启动时需要 重新连接信号槽，否则通信会失效。
            //         ///解决问题1： 关闭的时候全部设备都要关

            //         ///1. 停止抓取
            //         grabber->stopGrabbing();
            //         baslerCameraController1->stopGrabbing();  //明明关闭相机的函数中自带停止抓取，为什么还要这两句呢，因为如果不先停止抓取，那么在下面线程退出时会卡死

            //         ///2. 停止线程
            //         t1_hikGrabberThread->quit();      // 请求线程退出
            //         t1_hikGrabberThread->wait();

            //         ///3. 停止相机
            //         closeHikCamera(); // 关闭相机
            //         closeBaslerCamera(); // 关闭相机

            //         qDebug() << "Thread t1_hikGrabberThread state after quit:" << t1_hikGrabberThread->isRunning();
            //     }
            // }
            // Basler相机
            if(currentCamera.contains("a2A1920-160umBAS")){
                qDebug()<<"当前相机为：a2A1920-160umBAS"; //√
                if(ui->pushButton_openCamera->toolTip()=="相机关闭"){
                    openBaslerCamera(); // 注意：经过测试，这个函数的运行是会阻塞主程序的，会导致相机开启按钮从未开启到开启的状态切换有迟滞，虽然代码行数不多，但这
                        // 些相机操作都是同步的硬件通信操作，每一步都可能需要较长时间，这些操作都是阻塞式的，每个函数都会等待硬件响应完成才返回
                    t1_hikGrabberThread->start(); // 启动数据流抓取线程
                    // baslerCameraController1->startGrabbing(imageProcessThread); // 启动basler相机的图像抓取操作，注意，这里不需要手动编写子线程，其SDK内部会自动创建子线程
                }
                else{
                    qDebug() << "Thread t1_hikGrabberThread state before quit:" << t1_hikGrabberThread->isRunning();
                    ///解决问题1： 关闭的时候全部都要关

                    ///1. 停止抓取
                    // grabber->stopGrabbing();

                    baslerCameraController1->stopGrabbing();


                    ///2. 停止线程
                    t1_hikGrabberThread->quit();      // 请求线程退出
                    t1_hikGrabberThread->wait();

                    ///3. 停止相机
                    // closeHikCamera(); // 关闭相机
                    closeBaslerCamera(); // 关闭相机

                    qDebug() << "Thread t1_hikGrabberThread state after quit:" << t1_hikGrabberThread->isRunning();

                }
            }
        }
        // 整机测试模式cameralink
        else{
            // 先把usb相机关了
            if(baslerCameraController1){
                baslerCameraController1->stopGrabbing();
            }

            if(ui->pushButton_openCamera->toolTip()=="相机关闭"){
                // t1_hikGrabberThread->start();
                // 添加 CCF 文件支持
                QString ccfPath = QFileDialog::getOpenFileName(this, "选择相机配置文件",
                                                               QString(), "相机配置文件 (*.ccf)");
                // 将ccf文件路径传递给相机
                emit ccfPathSignal(ccfPath);
                saperaCameraController1->onCameraSelected(ui->comboBox_camera_external->currentIndex());
                saperaCameraController1->startAcq();
                // 这里没必要一定要将imageProcessThread移动到sapera中去以便于调用其中的图像处理函数，由于本来sapera内部就自己创建子线程，所以完全可以在
                //sapera内部创建imageProcessThread对象
            }
            else{
                // qDebug() << "Thread t1_hikGrabberThread state before quit:" << t1_hikGrabberThread->isRunning();
                saperaCameraController1->stopAcq();
                //2. 停止线程
                // t1_hikGrabberThread->quit();      // 请求线程退出
                // t1_hikGrabberThread->wait();
                // qDebug() << "Thread t1_hikGrabberThread state after quit:" << t1_hikGrabberThread->isRunning();
            }
        }

    });

    // // 连接相机选择信号
    // connect(ui->comboBox_camera_external, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         saperaCameraController1, &SaperaCameraController::onCameraSelected);
    // // 获取cff文件路径
    // connect(this,&MainWindow::ccfPathSignal,saperaCameraController1,&SaperaCameraController::getCcfPath);
    // 获取cff文件路径
    connect(this,&MainWindow::ccfPathSignal,saperaCameraController1,[=](QString ccfPath){
        qDebug()<<"ccf:"<<ccfPath;
        saperaCameraController1->getCcfPath(ccfPath);
    });

    // 打印测试报告，输出PDF
    connect(ui->pushButton_print,&QPushButton::clicked,this,&MainWindow::testReport);

    // 窗口的最大化和窗口切换
    connect(this,&MainWindow::min2max,this,[=](bool action1){
        if(action1){
            // 设置此时的中心窗口的尺寸
            ui->widget_Center->setMaximumWidth(600);
            ui->dockWidget_imageShow->setMinimumWidth(500);
            btn_max->setIcon(QIcon(":/Icon/Icon_images/window_maximize_icon.png"));
        }
        else{
            ui->widget_Center->setMaximumWidth(800);
            ui->dockWidget_imageShow->setMinimumWidth(800);
            btn_max->setIcon(QIcon(":/Icon/Icon_images/window_restore_icon.png"));
        }
    });

    // 在构造函数或初始化函数中添加以下连接
    connect(baslerCameraController1, &CGuiCamera::imageGrabbed, this, [=](const QImage& image){
        QPixmap pixmap = QPixmap::fromImage(image);
        pixmapItem->setPixmap(pixmap);
        scene->setSceneRect(pixmap.rect());
        ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    });

    // 设置 Sapera 相机的 GraphicsView
    // saperaCameraController1->setGraphicsView(ui->graphicsView);

    // 保存raw信号发送给SaperaCameraController
    connect(this,&MainWindow::saveRawSignal,saperaCameraController1,&SaperaCameraController::saveRaw);
    // 获取当前系统时间

    QDir rawPath(rawFilesPath);
    if(!rawPath.exists()){
        if(!rawPath.mkpath(".")){
            QMessageBox::warning(this,"错误","无法初始化RAW文件夹！");
            return;
        }
    }

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){
        saperaCameraController1->simulateFrame();
    });
    timer->start(1000); // 每50ms一帧
}

MainWindow::~MainWindow()
{
    /*if (t1 && t1->isRunning()) {
        t1->quit();      // 请求线程退出
        t1->wait();      // 等待线程完全退出
    }
    delete t1; // 释放线程对象
    t1 = nullptr; // 防止悬空指针
    确保 imageThread 被安全销毁
    if (imageThread) {
        imageThread->deleteLater(); // 确保在事件循环中销毁
        imageThread = nullptr;     // 防止悬空指针
    }
    disconnect(imageThread, nullptr, this, nullptr); // 断开 imageThread 的信号槽
    disconnect(t1, nullptr, this, nullptr);          // 断开 t1 的信号槽
    上面那些代码会导致程序在退出时发生崩溃，解决方案是通过下面的信号和槽函数来关闭t1线程*/

    // if (hikCameraController1) {
    //     hikCameraController1->closeCamera(); //执行该函数会自动停止抓取
    // }
    if(baslerCameraController1){
        baslerCameraController1->closeCamera(); //执行该函数会自动停止抓取
    }
    emit aboutToClose(); // 发出信号关闭线程

    // 确保停止采集并释放资源
    saperaCameraController1->stopAcq();
    saperaCameraController1->destroySap();

    delete ui;
}


// 拖动主窗口
// 窗口状态变化处理
void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMaximized()) {
            ui->widget_Center->setMaximumWidth(QWIDGETSIZE_MAX); // 最大化时解除宽度限制
            ui->dockWidget_imageShow->setMinimumWidth(300); // 调整侧边栏最小宽度
        } else {
            ui->widget_Center->setMaximumWidth(400); // 恢复窗口化约束
            ui->dockWidget_imageShow->setMinimumWidth(200);
        }
    }
    QMainWindow::changeEvent(event);
}

// 重写鼠标按下事件
void MainWindow::mousePressEvent(QMouseEvent *event) {
    int dragAreaHeight = 80; // 仅在页面上方可以拖动
    if (event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight) {
        isDragging = true; //只有当鼠标存在在正确的位置，才能触发拖动，防止其他控件喧宾夺主
        lastMousePos = event->globalPosition().toPoint();
        // 检查是否为全屏状态
    // 实现缩放
    if (!resizing) {
        updateCursorShape(event->pos()); // 根据鼠标位置更新光标形状
    } else {
        resizeWindow(event->globalPosition().toPoint()); // 实现窗口缩放
    }
    }

    // 当在全屏状态下拖动窗口时，切换到窗口模式
    // if (isFullScreen() && event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight) {
    //     // 检查鼠标是否位于标题栏区域
    //     if (event->pos().y() < 80) { // 假设标题栏的高度为 80 像素
    //         isDragging = true;
    //         lastMousePos = event->globalPosition().toPoint();
    //     }
    //     QMainWindow::mousePressEvent(event);
    // }

}
    // if(event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight){

    // }

// 重写鼠标移动事件
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    // if (this->isMaximized()) {
    //     this->showNormal();
        // qDebug() << "Exiting full screen";
        // 发送一个信号，告诉界面，该切换Icon了
        // emit min2max(true);
    // }
    if (isDragging && !this->isMaximized()) {
        QPoint delta = event->globalPosition().toPoint() - lastMousePos;
        this->move(this->pos() + delta);
        lastMousePos = event->globalPosition().toPoint();
    }
}
// 重写鼠标释放事件
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
    }
}

void MainWindow::maxMinSwitch(bool minMax){
    if(minMax){
        // 正常
        ui->dockWidget_paraSetting->setMaximumWidth(windowWidthTrans*0.23); // 窗口化时中间区域最大宽度
        ui->dockWidget_dashBoard->setMaximumWidth(windowWidthTrans*0.23); // 窗口化时中间区域最大宽度
        ui->dockWidget_statusOutput->setMaximumWidth(windowWidthTrans*0.23); // 窗口化时中间区域最大宽度
        ui->widget_Center->setMinimumWidth(windowWidthTrans*0.36); // 窗口化时中间区域最大宽度
        ui->dockWidget_imageShow->setMaximumWidth(windowWidthTrans*0.5); // 侧边栏最小宽度
        ui->dockWidget_imageShow->setMinimumWidth(windowWidthTrans*0.3); // 侧边栏最小宽度
    }
    else{
        // 最大化
        ui->dockWidget_paraSetting->setMaximumWidth(windowWidthTrans0*0.2); // 窗口化时中间区域最大宽度
        ui->dockWidget_dashBoard->setMaximumWidth(windowWidthTrans0*0.2); // 窗口化时中间区域最大宽度
        ui->dockWidget_statusOutput->setMaximumWidth(windowWidthTrans0*0.2); // 窗口化时中间区域最大宽度
        // ui->widget_Center->setMinimumWidth(windowWidth0*0.3); // 窗口化时中间区域最大宽度
        ui->dockWidget_imageShow->setMaximumWidth(windowWidthTrans0*0.6); // 侧边栏最小宽度
        ui->dockWidget_imageShow->setMinimumWidth(windowWidthTrans0*0.3); // 侧边栏最小宽度
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    int dragAreaHeight = 80; // 指定顶部区域的高度
    if (event->button() == Qt::LeftButton && event->pos().y() <= dragAreaHeight) {
        // 检查窗口当前状态并切换
        if (this->isMaximized()) {
            this->showNormal(); // 如果已最大化，切换到普通窗口
            maxMinSwitch(true);
            // 发送一个信号，告诉界面，该切换Icon了
            emit min2max(true);
        } else {
            this->showMaximized(); // 如果不是最大化，切换到最大化
            // 发送一个信号，告诉界面，该切换Icon了
            maxMinSwitch(false);
            emit min2max(false);
        }
    }
    // QMainWindow::mouseDoubleClickEvent(event); // 调用父类实现，确保其他功能正常
}


void MainWindow::createContextMenu(){
    QMenu menu(this);
    QAction *deleteAction = menu.addAction("删除行");

    connect(deleteAction, &QAction::triggered, this, [this](){
        int currentRow = tableWidget_listTrans->currentRow();
        if(currentRow >= 0) {
            tableWidget_listTrans->removeRow(currentRow);
        }
    });

    menu.exec(QCursor::pos());
}

// void MainWindow::addToListTrans(){
//     // 添加新行
//     int row = tableWidget_listTrans->rowCount(); // 获取当前行数
//     tableWidget_listTrans->insertRow(row); // 在行号为 row 的位置插入新行
//     // 获取左侧数据
//     QString wavelengthTrans =  ui->lineEdit_wavelengthTrans->text();
//     QString testLensTrans = ui->lineEdit_testLensTrans->text();
//     // 可选：添加新的QTableWidgetItem
//     tableWidget_listTrans->setItem(row, 0, new QTableWidgetItem(wavelengthTrans));
//     tableWidget_listTrans->setItem(row, 1, new QTableWidgetItem(testLensTrans));
// }

void MainWindow::addToListTransOffi(){
    // 获取当前波长信息
    QString currentWavelens = ui->lineEdit_wavelengthTrans->text();
    // 获取当前参考透过率信息
    QString offiTrans = ui->lineEdit_officialLensTrans->text();
    // 获取当前行数
    int row = tableWidget_listTrans->rowCount(); // 获取当前行数
    // if(row==0){
    //     tableWidget_listTrans->insertRow(row); // 在行号为 row 的位置插入新行
    //     for(int j = 0; j<4;j++){
    //         tableWidget_listTrans->setItem(row, j, new QTableWidgetItem("")); //初始化行
    //     }
    // }
    // row = tableWidget_listTrans->rowCount(); // 获取当前行数
    // 循环检查列表第一列是否有该波长，如果有就直接添加数值，没有就新建一行
    // 获取左侧参照镜头光功率
    QString opticalPowerOffi = ui->lineEdit_officialLensOpticalPower->text();
    bool isExist = false;
    for(int i = 0;i<row;i++){
        if(tableWidget_listTrans->item(i,0)->text() == currentWavelens){
            tableWidget_listTrans->setItem(i, 1, new QTableWidgetItem(opticalPowerOffi));
            tableWidget_listTrans->setItem(i, 2, new QTableWidgetItem(offiTrans));
            isExist = true;
        }
    }
    if(!isExist){
        tableWidget_listTrans->insertRow(row); // 在行号为 row 的位置插入新行
        tableWidget_listTrans->setItem(row, 0, new QTableWidgetItem(currentWavelens));
        tableWidget_listTrans->setItem(row, 1, new QTableWidgetItem(opticalPowerOffi));
        tableWidget_listTrans->setItem(row, 2, new QTableWidgetItem(offiTrans));
        tableWidget_listTrans->setItem(row, 3, new QTableWidgetItem(""));
        tableWidget_listTrans->setItem(row, 4, new QTableWidgetItem(""));
    }
}

void MainWindow::addToListTransTest(){
    // 获取当前波长信息
    QString currentWavelens = ui->lineEdit_wavelengthTrans->text();
    // 获取当前参考透过率信息
    QString offiTrans = ui->lineEdit_officialLensTrans->text();
    // 获取当前行数
    int row = tableWidget_listTrans->rowCount(); // 获取当前行数
    // if(row==0){
    //     tableWidget_listTrans->insertRow(row); // 在行号为 row 的位置插入新行
    //     for(int j = 0; j<4;j++){
    //         tableWidget_listTrans->setItem(row, j, new QTableWidgetItem("")); //初始化行
    //     }
    // }
    // row = tableWidget_listTrans->rowCount(); // 获取当前行数
    // 循环检查列表第一列是否有该波长，如果有就直接添加数值，没有就新建一行
    // 获取左侧测试镜头光功率
    QString opticalPowerTest = ui->lineEdit_testLensOpticalPower->text();
    bool isExist = false;
    for(int i = 0;i<row;i++){
        if(tableWidget_listTrans->item(i,0)->text() == currentWavelens){
            tableWidget_listTrans->setItem(i, 3, new QTableWidgetItem(opticalPowerTest));
            tableWidget_listTrans->setItem(i, 4, new QTableWidgetItem(""));
            isExist = true;
        }
    }
    if(!isExist){
        tableWidget_listTrans->insertRow(row); // 在行号为 row 的位置插入新行
        tableWidget_listTrans->setItem(row, 0, new QTableWidgetItem(currentWavelens));
        tableWidget_listTrans->setItem(row, 1, new QTableWidgetItem(""));
        tableWidget_listTrans->setItem(row, 2, new QTableWidgetItem(""));
        tableWidget_listTrans->setItem(row, 3, new QTableWidgetItem(opticalPowerTest));
        tableWidget_listTrans->setItem(row, 4, new QTableWidgetItem(""));
    }
}

void MainWindow::clearTableTrans(){
    qDebug()<<"正在重置列表";
    // tableWidget_listTrans->clearContents();  // 清空单元格内容
    tableWidget_listTrans->setRowCount(0);   // 删除所有行
}

// 绘制透过率曲线，这整个函数都是ai写的，可以学一下
// 修改想法：创建一个专门的chart类，用于绘制曲线，调用绘制函数时需要输入参数：横轴标题、纵轴标题、横轴数据、纵轴数据
void MainWindow::plotLine(){
    qDebug()<<"正在绘制透过率曲线";
    // 获取表格中的数据
    int rowCount = tableWidget_listTrans->rowCount();
    if (rowCount == 0) {
        QMessageBox::warning(this, "警告", "没有数据可以绘制！");
        return;
    }

    // 创建数据点
    QVector<double> wavelengths;
    QVector<double> transmissions;

    for (int i = 0; i < rowCount; i++) {
        QTableWidgetItem* wavelengthItem = tableWidget_listTrans->item(i, 0);
        QTableWidgetItem* transmissionItem = tableWidget_listTrans->item(i, 4);

        if (wavelengthItem && transmissionItem) {
            bool waveOk, transOk;
            double wavelength = wavelengthItem->text().toDouble(&waveOk);
            double transmission = transmissionItem->text().toDouble(&transOk);

            if (waveOk && transOk) {
                wavelengths.append(wavelength);
                transmissions.append(transmission);
            }
        }
    }

    if (wavelengths.isEmpty()) {
        QMessageBox::warning(this, "警告", "没有有效的数据可以绘制！");
        return;
    }

    // 创建图表
    QChart *chart = new QChart();
    chart->setTitle("透过率曲线");

    // 创建数据系列
    QLineSeries *series = new QLineSeries();
    series->setName("透过率");
    // 添加数据点
    for (int i = 0; i < wavelengths.size(); i++) {
        series->append(wavelengths[i], transmissions[i]);
    }

    // 添加系列到图表
    chart->addSeries(series);

    // 创建坐标轴
    chart->createDefaultAxes();
    QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());

    if (axisX && axisY) {
        axisX->setTitleText("波长 (nm)");
        axisY->setTitleText("透过率 (%)");
        axisY->setRange(0, 100);
    }

    // 创建图表视图
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // 显示图表
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("透过率曲线");
    dialog->resize(600, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(chartView);

    dialog->exec();
}

/* 这段代码并没有运行，已经是放到了imageprocessthread.cpp中了
 * void MainWindow::calculateCentroid(){

    double grayThre = ui->lineEdit_grayThre->text().toDouble();
    //首先需要明确currentImage的当前格式是QImage，QImage::Format_RGB32
    //将QImage的Format_RGB32格式转为mat格式
    if (currentImage.format() != QImage::Format_Grayscale8) {
        // QMessageBox::warning(this, "Image Format Error", "The extracted image is not in grayscale format.");
        // Optional: Convert the image back to grayscale
        currentImage = currentImage.convertToFormat(QImage::Format_Grayscale8);  //这里将彩色图像转为灰度图，以便于后面计算灰度重心
    }
    cv::Mat matImage =  QImageToMat(currentImage);
    cv::Point2d centerPoint= grayCenter(matImage,grayThre);
    //刷新界面的lineEdit
    // std::cout <<"Center: (" << centerPoint.x << ", " << centerPoint.y << ")" << std::endl;
    ui->lineEdit_centroidX->setText(QString::number(centerPoint.x));
    ui->lineEdit_centroidY->setText(QString::number(centerPoint.y));
    //在图像中标出质心的位置
    // 更新质心标记在图层中的显示
    // 判断主页面是否勾选需要质心标记
    if(ui->checkBox_circleStatus->isChecked()){
        qDebug("需要标记");
        updateCentroid(centerPoint);
    }
    else centroidItem->setVisible(false);  // 隐藏质心图层
}*/

void MainWindow::updateCentroid(const cv::Point2d &centroid){
    if (centroid.x < 0 || centroid.y < 0) {
        centroidItem->setVisible(false);  // 无效质心，隐藏
    } else {
        // 更新质心图层的位置
        centroidItem->setRect(centroid.x-circleRect / 2, centroid.y-circleRect / 2, circleRect, circleRect);  // 设置质心圆形的中心
        centroidItem->setVisible(true);  // 显示质心图层
    }
}
void MainWindow::updateCentroidLimit(const cv::Point2d &centroid){
    if (centroid.x < 0 || centroid.y < 0) {
        centroidItemLimit->setVisible(false);  // 无效质心，隐藏
        rectItem ->setVisible(false);
    } else {
        // 更新质心图层的位置
        centroidItemLimit->setRect(centroid.x-circleRectLimit / 2, centroid.y-circleRectLimit / 2, circleRectLimit, circleRectLimit);  // 设置质心圆形的中心
        centroidItemLimit->setVisible(true);  // 显示质心图层
        rectItem->setRect(rectLeftUpX,rectLeftUpY,rectRightDownX - rectLeftUpX < 0 ? 0 : rectRightDownX - rectLeftUpX, rectRightDownY - rectLeftUpY < 0 ? 0 : rectRightDownY - rectLeftUpY);
        rectItem ->setVisible(true);
    }
}

/* 这段代码并没有运行，已经是放到了imageprocessthread.cpp中了
 * //将QImage格式改为mat格式
cv::Mat MainWindow::QImageToMat(const QImage &image) {
    // 判断 QImage 是否有效
    if (image.isNull()) {
        return cv::Mat();
    }
    // 转换 QImage 为 cv::Mat
    // 根据 QImage 的格式，选择不同的转换方法
    switch (image.format()) {
    case QImage::Format_RGB32: {
        // RGBA (含 alpha 通道)
        cv::Mat mat(image.height(), image.width(), CV_8UC4, (void *)image.bits(), image.bytesPerLine());
        return mat.clone();  // 复制数据，避免使用指针
    }
    case QImage::Format_RGB888: {
        // RGB
        cv::Mat mat(image.height(), image.width(), CV_8UC3, (void *)image.bits(), image.bytesPerLine());
        return mat.clone();  // 复制数据，避免使用指针
    }
    case QImage::Format_Grayscale8: {
        // 灰度图
        cv::Mat mat(image.height(), image.width(), CV_8UC1, (void *)image.bits(), image.bytesPerLine());
        return mat.clone();  // 复制数据，避免使用指针
    }
    default: {
        qWarning("Unsupported QImage format: %d", image.format());
        return cv::Mat();  // 返回空的 Mat
    }
    }
}*/


/// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆hik相机☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
// void MainWindow::openHikCamera(){
//     hikCameraController1->openCamera();
// }

// void MainWindow::closeHikCamera(){
//     hikCameraController1->closeCamera();
// }

void MainWindow::openBaslerCamera() {

    baslerCameraController1->openCamera();
}

void MainWindow::closeBaslerCamera(){
    // 停止图像采集
    baslerCameraController1->stopGrabbing();
    // 关闭相机
    baslerCameraController1->closeCamera();
};
/// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆电脑自带相机↓☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
// // 点击按钮启动摄像头
// void MainWindow::startCamera() {

//     camera->start();  // 启动相机
// }

// // 当有新的视频帧时调用
// void MainWindow::onFrameAvailable(const QVideoFrame &frame) {
//     if (frame.isValid()) {
//         QImage image = frame.toImage();  // 将视频帧转换为图像
//         if (!image.isNull()) {
//             QPixmap pixmap = QPixmap::fromImage(image);  // 将 QImage 转换为 QPixmap
//             pixmapItem->setPixmap(pixmap);  // 在 QGraphicsView 中显示图像
//             ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);  // 适应视图
//         }
//     }
// }

///☆☆☆☆☆多线程操作    注意：就是这里，下面这些图像的处理必须找机会放进子线程中，不然还是在主线程中运行会阻塞UI界面
void MainWindow::handleError(const QString &message)
{
    // 在状态栏显示错误信息
    statusBar()->showMessage(tr("Error: %1").arg(message), 5000);  // 显示5秒

    // 在输出框中显示错误信息（如果你有plainTextEdit）
    if (ui->plainTextEdit) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        ui->plainTextEdit->appendPlainText(QString("[%1] Error: %2").arg(timestamp, message));
    }

    // 输出到调试控制台
    qDebug() << "Error:" << message;
}

cv::Mat MainWindow::QImageToMat(const QImage &image)
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


// 当有新的视频帧时调用
void MainWindow::onFrameAvailable1(const QPixmap &pixmap,const cv::Point2d &centerPoint, const cv::Point2d &centerPointLimit) {
    /* qDebug() << "Current Thread ID:" << QThread::currentThreadId(); //主线程
    currentImage = pixmap.toImage();//保存最近的帧到currentImage，以用于计算和标记质心
    获取图像格式
    QImage::Format format = currentImage.format();
    打印图像格式
    qDebug() << "Image format:" << format;  //QImage::Format_RGB32
    注意：上面这个currentImage需要在不同的函数中被使用，所以应该将其声明为类的成员变量，并不建议随意使用全局变量*/
    currentImage = pixmap.toImage();//保存最近的帧到currentImage，以用于计算和标记质心
    // 获取图像格式
    // QImage::Format format = currentImage.format();
    // qDebug() << "Image format:" << format;  //QImage::Format_RGB32

    // 保存当前视图的变换状态
    QTransform currentTransform = ui->graphicsView->transform();
    pixmapItem->setPixmap(pixmap);  // 在 QGraphicsView 中显示图像
    // ui->graphicsView->fitInView(pixmapItem, Qt::KeepAspectRatio);  // 适应视图:fitInView 会强制每次图像刷新时都将视图缩放到适应图像大小的状态，这样会覆盖掉之前用户手动缩放和拖动的效果。
    // 恢复之前的变换状态，保持当前的缩放和平移
    ui->graphicsView->setTransform(currentTransform);

    // 对于全局质心计算
    ui->lineEdit_centroidX->setText(QString::number(centerPoint.x));
    ui->lineEdit_centroidY->setText(QString::number(centerPoint.y));
    //在图像中标出质心的位置
    // 更新质心在图层中的显示
    if(ui->checkBox_circleStatus->isChecked()){
        // qDebug("需要标记");
        updateCentroid(centerPoint);
    }
    else centroidItem->setVisible(false);  // 隐藏质心图层

    //对于矩形框中的的质心计算
    ui->lineEdit_centroidX_Limit->setText(QString::number(centerPointLimit.x));
    ui->lineEdit_centroidY_Limit->setText(QString::number(centerPointLimit.y));

    delayCenterX.append(centerPointLimit.x);
    delayCenterY.append(centerPointLimit.y);

    // 每次积攒20帧图像的质心数据然后计算均值
    if(delayCenterX.size()==20){
        // 方法1：手动遍历计算
       double sumX = 0.0;
       for (double value : delayCenterX) {
           sumX += value;
       }
       double manualAverageX = delayCenterX.isEmpty() ? 0 : sumX / delayCenterX.size();
       ui->lineEdit_centroidX_Limit2->setText(QString::number(manualAverageX));
       delayCenterX.clear();

       double sumY = 0.0;
       for (double value : delayCenterY) {
           sumY += value;
       }
       double manualAverageY = delayCenterY.isEmpty() ? 0 : sumY / delayCenterY.size();
       ui->lineEdit_centroidY_Limit2->setText(QString::number(manualAverageY));
       delayCenterY.clear();
    }

    if(ui->checkBox_circleStatusLimit->isChecked()){
        // qDebug("需要标记");
        updateCentroidLimit(centerPointLimit);
    }
    else{
        centroidItemLimit->setVisible(false);  // 隐藏质心图层
        rectItem->setVisible(false);
    }
}
/// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆相机↑☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆


void MainWindow::setEnabled(bool tvf){
    for (int row = 0; row < ui->tableWidget_distortion->rowCount(); ++row) {
        // 创建或获取单元格项，如果不存在则创建新的单元格项
        if (!ui->tableWidget_distortion->item(row, 0)) {
            ui->tableWidget_distortion->setItem(row, 0, new QTableWidgetItem());
        }
        if (!ui->tableWidget_distortion->item(row, 1)) {
            ui->tableWidget_distortion->setItem(row, 1, new QTableWidgetItem());
        }
        if (!ui->tableWidget_distortion->item(row, 5)) {
            ui->tableWidget_distortion->setItem(row, 5, new QTableWidgetItem());
        }
        QTableWidgetItem* itemx = ui->tableWidget_distortion->item(row, 0);
        QTableWidgetItem* itemy = ui->tableWidget_distortion->item(row, 1);
        QTableWidgetItem* itemd = ui->tableWidget_distortion->item(row, 5);
        if(!tvf){
            itemx->setFlags(itemx->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
            itemy->setFlags(itemy->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
            itemd->setFlags(itemd->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
        }
        else{
            itemx->setFlags(itemx->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
            itemy->setFlags(itemy->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
            // itemd->setFlags(itemd->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
        }
    }
}

void MainWindow::setEnabledMTF(bool tvf){
    for (int row = 0; row < ui->tableWidget_MTF->rowCount(); ++row) {
        // 创建或获取单元格项，如果不存在则创建新的单元格项
        if (!ui->tableWidget_MTF->item(row, 0)) {
            ui->tableWidget_MTF->setItem(row, 0, new QTableWidgetItem());
        }
        if (!ui->tableWidget_MTF->item(row, 1)) {
            ui->tableWidget_MTF->setItem(row, 1, new QTableWidgetItem());
        }
        if (!ui->tableWidget_MTF->item(row, 5)) {
            ui->tableWidget_MTF->setItem(row, 5, new QTableWidgetItem());
        }
        QTableWidgetItem* itemx = ui->tableWidget_MTF->item(row, 0);
        QTableWidgetItem* itemy = ui->tableWidget_MTF->item(row, 1);
        QTableWidgetItem* itemd = ui->tableWidget_MTF->item(row, 5);
        if(!tvf){
            itemx->setFlags(itemx->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
            itemy->setFlags(itemy->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
            itemd->setFlags(itemd->flags() & ~Qt::ItemIsEditable);  // 移除可编辑标志
        }
        else{
            itemx->setFlags(itemx->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
            itemy->setFlags(itemy->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
            // itemd->setFlags(itemd->flags() | Qt::ItemIsEditable);  // 恢复可编辑标志
        }
    }
}

//方法二，使用统一的槽函数
// void MainWindow::onCheckBoxClicked(){
//     if(ui->checkBox_lens->isChecked()){
//         QCheckBox* senderCheckBox = qobject_cast<QCheckBox*>(sender()); // 获取信号发送者
//         if (senderCheckBox == ui->checkBox_kjg ||senderCheckBox == ui->checkBox_jhw||senderCheckBox == ui->checkBox_dbhw) {
//             ui->lineEdit_pixelSize->setText("3.45");
//         } else if (senderCheckBox == ui->checkBox_zbhw || senderCheckBox == ui->checkBox_cbhw) {
//             ui->lineEdit_pixelSize->setText("12");
//         }
//     }
// }

//当选择镜头测试时
void MainWindow::onCheckboxLensClicked(){
    // "修改"按钮可以点击
    ui->pushButton_defaultPixelSize->setEnabled(true);
    ui->pushButton_defaultPixelSize->setToolTip("修改相机相元尺寸");
    ui->lineEdit_pixelSize->setEnabled(false);
    // 对于不同的波长的光，选择不同的像元尺寸
    if(ui->checkBox_kjg->isChecked()){
        ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[0])); //测试镜头时的默认像元尺寸
    }
    else if(ui->checkBox_jhw->isChecked()){
        ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[1])); //测试镜头时的默认像元尺寸
    }
    else if(ui->checkBox_dbhw->isChecked()){
        ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[2])); //测试镜头时的默认像元尺寸
    }
    else if(ui->checkBox_zbhw->isChecked()){
        ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[3])); //测试镜头时的默认像元尺寸
    }
    else{
        ui->lineEdit_pixelSize->setText(QString::number(vectorPixelList[4])); //测试镜头时的默认像元尺寸
    }
}

void MainWindow::onPushButtonDefPixSizeClicked(){
    diaUi->exec(); //打开对话框
    vectorPixelList = diaUi->getLineEditText(); //使用vector列表装像元尺寸,注意，每次打开对话框并闭关时要重复执行该操作
    // diaUi->ui->buttonBox->accepted(); //accepted() 是 QDialogButtonBox 的一个内置信号，用于表示"接受"按钮（通常是"OK"或"确定"按钮）被点击时发出的信号
}


//当选择整机测试时
void MainWindow::onCheckboxOverallClicked(){
    ui->pushButton_defaultPixelSize->setEnabled(false);
    ui->pushButton_defaultPixelSize->setToolTip("整机测试状态下不可修改相元");
    ui->lineEdit_pixelSize->setEnabled(true);
}

void MainWindow::changeButtonPowerColor(){
    //颜色切换
    QString currentStyle = ui->pushButton_power->styleSheet();
    // if(currentStyle.contains("background-color: rgba(255, 130, 108, 100);")){
    if(currentStyle == buttonStyle_mainBtn){
        ui->pushButton_power->setStyleSheet(buttonStyleClicked2);
        // ui->pushButton->setStyleSheet("QPushButton { background-color: rgba(158, 189, 255, 100);}"); //当关闭总电源时，当前的总体进程也同时断开
        ui->pushButton_power->setToolTip("关闭电源");
    }
    else{
        // ui->pushButton_power->setStyleSheet("QPushButton { background-color: rgba(255, 130, 108, 100);}");
        ui->pushButton_power->setStyleSheet(buttonStyle_mainBtn);
        ui->pushButton_power->setToolTip("打开电源");
    }
    //开始执行槽函数
}

void MainWindow::changeButtonIcon(){
    //图标切换
    QIcon currentIcon = ui->pushButton_openCamera->icon();
    QIcon playIcon = QIcon::fromTheme("media-playback-start");
    QIcon stopIcon = QIcon::fromTheme("media-playback-stop");
    if(currentIcon.cacheKey()==stopIcon.cacheKey()){
        ui->pushButton_openCamera->setIcon(playIcon);
        //关闭图像显示
        // camera->stop();  // 关闭相机
        ui->pushButton_openCamera->setToolTip("相机启动");
    }
    else{
        ui->pushButton_openCamera->setIcon(stopIcon);
        //开启图像显示
        // camera->start();  // 启动相机
        ui->pushButton_openCamera->setToolTip("相机关闭");
    }
    //颜色切换
    QString currentStyle = ui->pushButton_openCamera->styleSheet();
    if(currentStyle == buttonStyle_mainBtn){
        ui->pushButton_openCamera->setStyleSheet(buttonStyleClicked2);
    }
    else{
        ui->pushButton_openCamera->setStyleSheet(buttonStyle_mainBtn);
    }
}

// 初始化表格单元格
void MainWindow::initTableCell(){
    // 获取当前表格的行数
    int tempRowCountDis = ui->tableWidget_distortion->rowCount();
    int tempColCountDis = ui->tableWidget_distortion->columnCount();
    int tempRowCountMTF = ui->tableWidget_MTF->rowCount();
    int tempColCountMTF = ui->tableWidget_MTF->columnCount();

    for (int row = 0; row < tempRowCountDis; ++row) {
        for(int col = 0; col < tempColCountDis; ++col){
            QTableWidgetItem *item = ui->tableWidget_distortion->item(row, col);
            if(!item){
                QTableWidgetItem* item = new QTableWidgetItem("");
                ui->tableWidget_distortion->setItem(row, col, item);
            }
        }
    } // 记号
    for (int row = 0; row < tempRowCountMTF; ++row) {
        for(int col = 0; col < tempColCountMTF; ++col){
            QTableWidgetItem *item = ui->tableWidget_MTF->item(row, col);
            if(!item){
                QTableWidgetItem* item = new QTableWidgetItem("");
                ui->tableWidget_MTF->setItem(row, col, item);
            }
        }
    } // 记号
}


// 总体启动按钮的显示图标切换，控制测试系统的总体开关，比如图像显示、各个模块当前参数显示等
void MainWindow::ButtonStart(){
    //开始执行槽函数
    //①判断当前测试的是哪个参数，并执行对应的操作
    if(ui->tabWidget->currentIndex()==0){
        // 将当前按钮变成不可选取状态
        ui->pushButton_start->setEnabled(false);
        // qDebug()<<"当前正在测试视场角...";
        QString message = QString("[%1] 当前正在测试视场角...").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        fieldOfView();
    }

    if(ui->tabWidget->currentIndex()==1){
        // 将当前按钮变成不可选取状态
        ui->pushButton_start->setEnabled(false);
        // qDebug()<<"当前正在测试焦距...";
        QString message = QString("[%1] 当前正在测试焦距...").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        focalLength();
    }

    if(ui->tabWidget->currentIndex()==2){
        // 将当前按钮变成不可选取状态
        ui->pushButton_start->setEnabled(false);
        // qDebug()<<"当前正在测试畸变...";
        QString message = QString("[%1] 当前正在测试畸变...").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        distortion();
    }

    if(ui->tabWidget->currentIndex()==3){
        // 将当前按钮变成不可选取状态
        ui->pushButton_start->setEnabled(false);
        // qDebug()<<"当前正在测试MTF...";
        QString message = QString("[%1] 当前正在测试MTF...").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        MTF();
    }

    if(ui->tabWidget->currentIndex()==4){
        // 将当前按钮变成不可选取状态
        ui->pushButton_start->setEnabled(false);
        // qDebug()<<"当前正在测试透过率（镜头）...";
        QString message = QString("[%1] 当前正在测试透过率（镜头）...").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        transmission();
    }
}

//视场角
void MainWindow::fieldOfView(){
    double upAng = ui->lineEdit_up->text().toDouble();
    double downAng = ui->lineEdit_down->text().toDouble();
    double leftAng = ui->lineEdit_left->text().toDouble();
    double rightAng = ui->lineEdit_right->text().toDouble();

    double horiViewAng = 0.0;
    double elevViewAng = 0.0;


    // 准备参数
    double imageX = ui->lineEdit_imageX->text().toDouble();
    double imageY = ui->lineEdit_imageY->text().toDouble();

    double tanglePixelLRX = ui->lineEdit_tanglePixelLRX->text().toDouble();
    double tanglePixelLRY = ui->lineEdit_tanglePixelLRY->text().toDouble();
    double tanglePixelLRX2 = ui->lineEdit_tanglePixelLRX2->text().toDouble();
    double tanglePixelLRY2 = ui->lineEdit_tanglePixelLRY2->text().toDouble();

    double tanglePixelUDX = ui->lineEdit_tanglePixelUDX->text().toDouble();
    double tanglePixelUDY = ui->lineEdit_tanglePixelUDY->text().toDouble();
    double tanglePixelUDX2 = ui->lineEdit_tanglePixelUDX2->text().toDouble();
    double tanglePixelUDY2 = ui->lineEdit_tanglePixelUDY2->text().toDouble();

    // 计算角度和像素的对应关系
    double LRTangle  = abs(rightAng - leftAng);// 水平角
    double UDTangle = abs(upAng - downAng); //俯仰角
    double tanglePerPixelLR = 0.0;
    double tanglePerPixelUD = 0.0;
    double horiViewPixel = 0.0;
    double elevViewPixel = 0.0;
    if(ui->checkBox_misCompensation->isChecked()){
        horiViewPixel = sqrt((tanglePixelLRX2 - tanglePixelLRX)*(tanglePixelLRX2 - tanglePixelLRX) + (tanglePixelLRY2 - tanglePixelLRY) *(tanglePixelLRY2 - tanglePixelLRY));
        elevViewPixel = sqrt((tanglePixelUDX2 - tanglePixelUDX)*(tanglePixelUDX2 - tanglePixelUDX) + (tanglePixelUDY2 - tanglePixelUDY) *(tanglePixelUDY2 - tanglePixelUDY));
        tanglePerPixelLR = LRTangle / horiViewPixel;
        tanglePerPixelUD = UDTangle / elevViewPixel;
    }
    else{
        horiViewPixel = (tanglePixelLRX2 - tanglePixelLRX) ;
        elevViewPixel = (tanglePixelUDY2 - tanglePixelUDY) ;
        tanglePerPixelLR = LRTangle / horiViewPixel;
        tanglePerPixelUD = UDTangle / elevViewPixel;
    }

    // 计算需要补偿的角度
    double tangleAddX = (imageX - abs(tanglePixelLRX2 - tanglePixelLRX)) * tanglePerPixelLR;// 水平补偿角度
    double tangleAddY  =( imageY - abs(tanglePixelUDY - tanglePixelUDY2)) * tanglePerPixelUD;// 俯仰补偿角度

    horiViewAng = tangleAddX + LRTangle;
    elevViewAng = tangleAddY + UDTangle;


    ui->lineEdit_elevViewAng->setText(QString::number(elevViewAng));
    ui->lineEdit_horiViewAng->setText(QString::number(horiViewAng));

    // 准备数据
    QString timestamp = utils::getCurrentTimestamp();
    QString upperPole = ui->lineEdit_up->text();
    QString lowerPole = ui->lineEdit_down->text();
    QString leftPole = ui->lineEdit_left->text();
    QString rightPole = ui->lineEdit_right->text();
    QString horizontalFOV = ui->lineEdit_horiViewAng->text();
    QString verticalFOV = ui->lineEdit_elevViewAng->text();
    // 要写进csv文件的数据
    QStringList dataFOV = {timestamp, upperPole, lowerPole, leftPole, rightPole, horizontalFOV, verticalFOV};
    // 开始准备要输入dataToCsv函数的参数
    QString objectName = ui->lineEdit_objectName->text();
    QString csvPathSupply = "/"+objectName+"/DataExport/FOV";
    QString csvFileName = "/FOV.csv";
    QString dataTitle = "时间戳,上极,下极,左极,右极,水平视场角,俯仰视场角\n";

    /// 将创建csv文件并向其中写文件的代码段写成可复用的函数
    /// dataTitle：csv文件的各列标题 格式：QString  例如："时间戳,上极,下极,左极,右极,水平视场角,俯仰视场角\n";
    /// dataList : 要写入csv文件的数据 格式: QStringList  例如：QStringList dataFOV = {timestamp, upperPole};
    /// csvPathSupply：要补充的csv路径及名称 格式：QString  例如："/DataExport/FOV";
    /// csvFileName：要补充的csv的名称 格式：QString 例如："/FOV.csv"
    // dataToCsv(csvPathSupply,csvFileName,dataTitle,dataList);

    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataFOV);

    // 将当前按钮恢复可选取状态
    ui->pushButton_start->setEnabled(true);
    QString message = QString("[%1] 视场角计算完成").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);

}

//焦距
void MainWindow::focalLength(){
    double h = ui->lineEdit_moveMm->text().toDouble(); //分划板移动距离h
    double pixelNumX = ui->lineEdit_xEnd->text().toDouble() - ui->lineEdit_xBegin->text().toDouble();;
    double pixelNumY = ui->lineEdit_yEnd->text().toDouble() - ui->lineEdit_yBegin->text().toDouble();
    double n = sqrt(pixelNumX * pixelNumX + pixelNumY * pixelNumY);//移动像元数量n
    double a = ui->lineEdit_pixelSize->text().toDouble(); //像元尺寸a
    double f = ui->lineEdit_colliFoci->text().toDouble(); //光管焦距f
    double singleFoci =  n*a*f/h/1000;
    ui->lineEdit_movePixelNum->setText(QString::number(n));
    ui->lineEdit_singleFocalLength->setText(QString::number(singleFoci));
    // 将当前按钮恢复可选取状态
    ui->pushButton_start->setEnabled(true);
    QString message = QString("[%1] 样本点处焦距计算完成").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);

    // // 准备数据
    // QString MoveFHB = ui->lineEdit_moveMm->text();
    // QString xBegin = ui->lineEdit_xBegin->text();
    // QString yBegin = ui->lineEdit_yBegin->text();
    // QString xEnd = ui->lineEdit_xEnd->text();
    // QString yEnd = ui->lineEdit_yEnd->text();
    // QString movePixelNum = ui->lineEdit_movePixelNum->text();
    // QString singleFocalLength = ui->lineEdit_singleFocalLength->text();
    // QStringList dataList = {utils::getCurrentTimestamp(),MoveFHB,xBegin,yBegin,xEnd,yEnd,movePixelNum,singleFocalLength};
    // // 开始准备要输入dataToCsv函数的参数
    // QString objectName = ui->lineEdit_objectName->text();
    // QString csvPathSupply = "/"+objectName+"/DataExport/focalLength";
    // QString csvFileName = "/focalLength.csv";
    // QString dataTitle = "时间戳,分划板移动距离(mm),起始质心X,起始质心Y,终止质心X,终止质心Y,分划板移动像素(个),单点焦距\n";
    // utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataList);

}

//平均焦距①加入列表
void MainWindow::addToList(){
    ui->pushButton_addToList->setEnabled(false);
    //判断lineEdit内容是否为空
    if(ui->lineEdit_singleFocalLength->text().isEmpty()){
        QMessageBox::warning(this, "Error", "添加失败，当前无有效焦距！");
    }
    else{
        // 准备数据
        QString MoveFHB = ui->lineEdit_moveMm->text();
        QString xBegin = ui->lineEdit_xBegin->text();
        QString yBegin = ui->lineEdit_yBegin->text();
        QString xEnd = ui->lineEdit_xEnd->text();
        QString yEnd = ui->lineEdit_yEnd->text();
        QString movePixelNum = ui->lineEdit_movePixelNum->text();
        QString singleFocalLength = ui->lineEdit_singleFocalLength->text();
        QStringList dataList = {utils::getCurrentTimestamp(),MoveFHB,xBegin,yBegin,xEnd,yEnd,movePixelNum,singleFocalLength};
        // 开始准备要输入dataToCsv函数的参数
        QString objectName = ui->lineEdit_objectName->text();
        QString csvPathSupply = "/"+objectName+"/DataExport/focalLength";
        QString csvFileName = "/focalLength.csv";
        QString dataTitle = "时间戳,分划板移动距离(mm),起始质心X,起始质心Y,终止质心X,终止质心Y,分划板移动像素(个),单点焦距\n";
        utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataList);

        // 这是最开始的版本，固定表格8行
        // 创建一个新项，并插入到当前行的第0列
        QTableWidgetItem *newItem = new QTableWidgetItem(ui->lineEdit_singleFocalLength->text()); // 示例数据
        bool tempBool = false;
        //寻找空位
        for(int i = 0;i<8;i++){
            QTableWidgetItem *item = ui->tableWidget_aveFocalLength->item(i, 0);
            //当项目指针是空的（还没有创建过对象）时或者单元格是空的时
            if (item == nullptr || item->text().isEmpty()) {  //使用 if (item->text().isEmpty()) 会有什么问题？
                // 如果单元格还没有创建 QTableWidgetItem 对象，item() 会返回 nullptr。在这种情况下，如果直接调用 item->text()，就会尝试对空指针调用方法，导致程序崩溃（未定义行为）。
                ui->tableWidget_aveFocalLength->setItem(i, 0, newItem); // 设置第currentRow行的内容;
                tempBool = true;
                break;
            }
        }
        if(!tempBool){
            QMessageBox::warning(this, "Error", "表格已满！");
        }
        //将lineEdit_single的内容加入到列表

        // 全新版本，灵活增加或删除列表行数
    }
    ui->pushButton_addToList->setEnabled(true);
    QString message = QString("[%1] 添加样本点焦距值到焦距列表").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);

}

//②计算平均焦距
void MainWindow::aveFocalLength(){
    ui->pushButton_aveFocalLength->setEnabled(false);
    int cellNum = 0;
    int rowCount = ui->tableWidget_aveFocalLength->rowCount();
    double cellTotal = 0.0;
    // QStringList dataList;  // 将这里的局部临时变量变更为全局的成员变量，这样，在输出测试报告时
    // dataList.fill("",8);
    //读取所有表格中单元格的数
    for(int i=0;i<rowCount;i++){
        QTableWidgetItem *item = ui->tableWidget_aveFocalLength->item(i, 0);
        if(item != nullptr && !item->text().isEmpty()){
            cellNum+=1;
            cellTotal +=item->text().toDouble();
            // dataList.append(item->text());
            dataListFocalLength[i] = item->text();
        }
    }
    double aveCellTotal = cellTotal/cellNum;
    ui->lineEdit_aveFocalLength->setText(QString::number(aveCellTotal));

    // 准备数据
    QString focalLengthAve = QString::number(aveCellTotal);  // 平均焦距
    dataListFocalLength.append(focalLengthAve);
    dataListFocalLength.prepend(utils::getCurrentTimestamp());
    // 读取表格中的第一列全部数据，放进csv文件的一行
    // 开始准备要输入dataToCsv函数的参数
    QString objectName = ui->lineEdit_objectName->text();
    QString csvPathSupply = "/"+objectName+"/DataExport/focalLengthAve";
    QString csvFileName = "/focalLengthAve.csv";
    QString dataTitle = "时间戳,样本点1,样本点2,样本点3,样本点4,样本点5,样本点6,样本点7,样本点8,平均焦距\n";
    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataListFocalLength);
    ui->pushButton_aveFocalLength->setEnabled(true);
    QString message = QString("[%1] 平均焦距值计算完成").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);

}

//清空手动输入的内容
void MainWindow::clearXYInput(){
    for (int row = 0; row < 16; ++row) {
        QTableWidgetItem *itemX = ui->tableWidget_distortion->item(row, 2);
        QTableWidgetItem *itemY = ui->tableWidget_distortion->item(row, 3);
        QTableWidgetItem *itemZ = ui->tableWidget_distortion->item(row, 4);
        QTableWidgetItem *itemM = ui->tableWidget_distortion->item(row, 5);
        if (itemX) {
            itemX->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemY) {
            itemY->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemZ) {
            itemZ->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemM) {
            itemM->setText("");  // 将该单元格的文本内容设置为空
        }
    }
}
//清空手动输入的内容
void MainWindow::clearXYInputMTF(){
    for (int row = 0; row < 16; ++row) {
        QTableWidgetItem *itemX = ui->tableWidget_MTF->item(row, 2);
        QTableWidgetItem *itemY = ui->tableWidget_MTF->item(row, 3);
        QTableWidgetItem *itemZ = ui->tableWidget_MTF->item(row, 4);
        QTableWidgetItem *itemL = ui->tableWidget_MTF->item(row, 5);
        if (itemX) {
            itemX->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemY) {
            itemY->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemZ) {
            itemZ->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemL) {
            itemL->setText("");  // 将该单元格的文本内容设置为空
        }
    }
}

//畸变初始化
void MainWindow::init(){
    double tempAxisX = ui->lineEdit_axisBeginX->text().toDouble(); // 起始点
    double tempAxisY = ui->lineEdit_axisBeginY->text().toDouble(); // 终止点
    double pixelGap = ui->lineEdit_pixelGap->text().toDouble(); // 像素间距
    int tempRowCount = ui->tableWidget_distortion->rowCount(); // 当前列表行数
    //创建两个vector容器用于存放x和y的坐标
    QVector<double> vectorX;
    QVector<double> vectorY;

    //清空数据
    for (int row = 0; row < tempRowCount; ++row) {
        QTableWidgetItem *itemX = ui->tableWidget_distortion->item(row, 0);
        QTableWidgetItem *itemY = ui->tableWidget_distortion->item(row, 1);
        if (itemX) {
            itemX->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemY) {
            itemY->setText("");  // 将该单元格的文本内容设置为空
        }
    }

    //自动生成视场点坐标填入表格
    if(ui->checkBox_2v2->isChecked()){
        // 设置表格的行数
        ui->tableWidget_distortion->setRowCount(9);
        for(int i = 0;i<3;i++){
            for(int j = 0;j<3;j++){
                vectorX.append(tempAxisX+j*pixelGap);
            }
        }
        for(int i = 0;i<3;i++){
            for(int j = 0;j<3;j++){
                vectorY.append(tempAxisY+i*pixelGap);
            }
        }
    }

    else if(ui->checkBox_3v3->isChecked()){
        ui->tableWidget_distortion->setRowCount(16);
        for(int i = 0;i<4;i++){
            for(int j = 0;j<4;j++){
                vectorX.append(tempAxisX+j*pixelGap);
            }
        }
        for(int i = 0;i<4;i++){
            for(int j = 0;j<4;j++){
                vectorY.append(tempAxisY+i*pixelGap);
            }
        }
    }
    else{
        ui->tableWidget_distortion->setRowCount(0);
    }

    //将vector中的元素放进列表，X和Y分别放在前两列
    for(int i = 0;i<tempRowCount;i++){
        QTableWidgetItem *newItemX = new QTableWidgetItem(QString::number(vectorX[i])); // 示例数据
        QTableWidgetItem *itemX = ui->tableWidget_distortion->item(i, 0);
        if (itemX == nullptr || itemX->text().isEmpty()) {  //使用 if (item->text().isEmpty()) 会有什么问题？
            // 如果单元格还没有创建 QTableWidgetItem 对象，item() 会返回 nullptr。在这种情况下，如果你直接调用 item->text()，就会尝试对空指针调用方法，导致程序崩溃（未定义行为）。
            ui->tableWidget_distortion->setItem(i, 0, newItemX); // 设置第currentRow行的内容;
        }
    }
    for(int i = 0;i<tempRowCount;i++){
        QTableWidgetItem *newItemY = new QTableWidgetItem(QString::number(vectorY[i])); // 示例数据
        QTableWidgetItem *itemY = ui->tableWidget_distortion->item(i, 1);
        if (itemY == nullptr || itemY->text().isEmpty()) {  //使用 if (item->text().isEmpty()) 会有什么问题？
            // 如果单元格还没有创建 QTableWidgetItem 对象，item() 会返回 nullptr。在这种情况下，如果你直接调用 item->text()，就会尝试对空指针调用方法，导致程序崩溃（未定义行为）。
            ui->tableWidget_distortion->setItem(i, 1, newItemY); // 设置第currentRow行的内容;
        }
    }
}

//畸变计算
void MainWindow::distortion(){
    QString modeSelection;
    int rowCount = ui->tableWidget_distortion->rowCount();
    int colCount = ui->tableWidget_distortion->columnCount(); // 6

    QVector<double> vectorCalDisX0;
    QVector<double> vectorCalDisY0;
    QVector<double> vectorCalDisX;
    QVector<double> vectorCalDisY;
    QVector<double> vectorCalDisD;
    QVector<double> vectorCalDis;

    // 不用分三种情况了，直接根据行数自动操作
    for(int i =0;i<rowCount;i++){
        QTableWidgetItem *itemX0 = ui->tableWidget_distortion->item(i, 0);
        if(itemX0 && !itemX0->text().isEmpty()){
            vectorCalDisX0.append(itemX0->text().toDouble());
        }
        else if(itemX0->text().isEmpty()){
            vectorCalDisX0.append(qQNaN()); // 当单元格内容是空时，默认值为NaN,即无效数据
        }

        QTableWidgetItem *itemX1 = ui->tableWidget_distortion->item(i, 2);
        if(itemX1 && !itemX1->text().isEmpty()){
            vectorCalDisX.append(itemX1->text().toDouble());
        }
        else{
            vectorCalDisX.append(qQNaN());
        }

        QTableWidgetItem *itemY0 = ui->tableWidget_distortion->item(i, 1);
        if(itemY0 && !itemY0->text().isEmpty()){
            vectorCalDisY0.append(itemY0->text().toDouble());
        }
        else{
            vectorCalDisY0.append(qQNaN());
        }

        QTableWidgetItem *itemY1 = ui->tableWidget_distortion->item(i, 3);
        if(itemY1 && !itemY1->text().isEmpty()){
            vectorCalDisY.append(itemY1->text().toDouble());
        }
        else{
            vectorCalDisY.append(qQNaN());
        }

        QTableWidgetItem *itemD = ui->tableWidget_distortion->item(i, 4);
        if(itemD && !itemD->text().isEmpty()){
            vectorCalDisD.append(itemD->text().toDouble());
        }
        else if(itemD && itemD->text().isEmpty()){
            vectorCalDisD.append(qQNaN());
        }
        else if(!itemD){
            QString message = QString("[%1] 不存在项目").arg(utils::getCurrentTimestamp());
            ui->plainTextEdit->appendPlainText(message);
            // qDebug()<<"不存在项目";
        }
    }
    qDebug()<<vectorCalDisX0
             <<vectorCalDisX
             <<vectorCalDisY0
             <<vectorCalDisY
             <<vectorCalDisD;
    //开始计算
    for(int i=0;i<rowCount;i++){
        // 只有在一行的前5个数据都不为空时，才进行后续的计算，注意，这里不再使用append，改用索引数组方式
        // qDebug()<<vectorCalDisX0[i]<<vectorCalDisX[i]<<vectorCalDisY0[i]<<vectorCalDisY[i]<<vectorCalDisD[i];
        vectorCalDis.append(focalLength2(vectorCalDisX0[i],vectorCalDisX[i],vectorCalDisY0[i],vectorCalDisY[i],vectorCalDisD[i]));
        // qDebug()<<vectorCalDisX0[i]<<" "<<vectorCalDisY0[i]<<" "<<vectorCalDisX[i]<<" "<<vectorCalDisY[i]<<" "<<vectorCalDisD[i];
    }
    for(int i = 0;i<rowCount;i++){
        QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(vectorCalDis[i])); // 示例数据
        // QTableWidgetItem *item = ui->tableWidget_distortion->item(i, 4);
        ui->tableWidget_distortion->setItem(i, 5, newItem); // 设置第currentRow行的内容;
    }

    /*
            /// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆空指针解引用问题☆☆☆☆☆☆☆☆☆☆☆☆☆
        for(int i =0;i<16;i++){
            QTableWidgetItem *itemX1 = ui->tableWidget_distortion->item(i, 2);
            /// 这里是创建指针指向表格中的第3列的每一个单元格，但是当单元格是空的时，指针也变成了空指针，指向Nullptr
            /// 于是，在下面的if条件语句中，itemX1->text().isEmpty()尝试判断当前单元格是否是空单元格，但是问题来了：其本身就存在"->"，这句话本身就是 ☆☆☆对空指针调用成员函数☆☆☆，
            /// 这是不被允许的，程序会崩溃，因为 nullptr 没有有效的成员函数
            if(itemX1 && !itemX1->text().isEmpty()){  ///这种逻辑称为 ☆短路求值☆，即在逻辑表达式的 && 运算中，如果前面的条件为 false，则不会再去检查后面的条件，避免了对空指针的解引用。
                vectorCalDisX.append(itemX1->text().toDouble());
            }
            ///笔记：循环访问 0 到 15 行的单元格（即总共 16 行），但表格实际只有前 9 行。对于超出行数范围的单元格，item() 方法会
            /// 返回 nullptr，导致后续调用 itemX1->text() 时发生崩溃，因为 itemX1 是一个空指针。
            /// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆空指针解引用问题☆☆☆☆☆☆☆☆☆☆☆☆
    */

    // 准备数据存进csv文件
    QString objectName = ui->lineEdit_objectName->text();
    QString csvPathSupply = "/"+objectName+"/DataExport/Distortion";
    QString csvFileName = "/Distortion.csv";
    QString dataTitle = "时间戳,模式,有效焦距,起始坐标X,起始坐标Y,视场点间距(像素),分划板移动(mm)\n";
    QString lineEdi_fc = ui->lineEdi_fc->text();
    QString lineEdit_axisBeginX = ui->lineEdit_axisBeginX->text();
    QString lineEdit_axisBeginY = ui->lineEdit_axisBeginY->text();
    QString lineEdit_pixelGap = ui->lineEdit_pixelGap->text();
    QString lineEdit_reticleMoveMm = ui->lineEdit_reticleMoveMm->text();
    // 当前测试的模式
    if(ui->checkBox_2v2->isChecked()) modeSelection = "9点模式";
    else if(ui->checkBox_3v3->isChecked()) modeSelection = "16点模式";
    else modeSelection = "自定义模式";
    dataListDis = {utils::getCurrentTimestamp(),modeSelection,lineEdi_fc,lineEdit_axisBeginX,lineEdit_axisBeginY,lineEdit_pixelGap,lineEdit_reticleMoveMm};
    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataListDis);

    // 同时，还需要在上述内容的下面存放表格内容
    // QString dataTitleTable = ",,,,\n"; // 空数据
    QStringList dataListTable = {"X0","Y0","X","Y","移动距离","相对畸变"};
    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataListTable);
    // 开始逐行存放table中的数据
    // QList<QStringList> dataTable;
    QStringList dataPerRow;
    dataPerRow.fill("",colCount);
    tabDisAll.clear(); // 先清空当前的表格存储变量
    ///注意：表格中有时候是前9行有内容，有时候是整个16行都有内容，当后面7行没有内容时，很容易崩溃
    for(int i = 0 ; i < rowCount ; ++i){
        for(int j = 0 ; j < colCount ; ++j){
            QTableWidgetItem *item = ui->tableWidget_distortion->item(i, j);
            if(!item->text().isEmpty()){
                dataPerRow[j]=item->text();
            }
        }

        // 对于每一行，将数据存进csv文件
        // dataListTable.append(dataPerRow);
        // qDebug()<<"dataPerRow:"<<dataPerRow;
        utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataPerRow);
        tabDisAll.append(dataPerRow); // 按行存储进表格数据存储变量
        // dataPerRow.clear(); // 注意：这里不能使用clear()是因为，我们先前确定过dataPerRow的元素数量，使用clear()后就没有这
        // 些数量的空腔了，在后续其他行的计算中会超出索引
        dataPerRow.fill("",colCount);
    }
    // 将当前按钮恢复可选取状态
    ui->pushButton_start->setEnabled(true);
    QString message = QString("[%1] 畸变计算完成").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);
}

//计算畸变的通用函数
double MainWindow::focalLength2(double x,double x_true,double y,double y_true,double moveMm){
    // double h = ui->lineEdit_reticleMoveMm->text().toDouble(); //分划板移动真实距离h，单位mm
    double h = moveMm;
    //下面的n就是根据移动前后的x的坐标计算的
    double n = sqrt((x_true - x) * (x_true - x) + (y_true - y) * (y_true - y));//移动像元数量
    double a = ui->lineEdit_pixelSize->text().toDouble(); //像元尺寸a，微米
    double f = ui->lineEdit_colliFoci->text().toDouble(); //光管焦距f
    double singleFociTrue =  n*a*f/h/1000; //该视场点的焦距，除1000是因为像元尺寸单位是微米
    //下面开始计算畸变
    //公式：d=(fc-f)/f,其中fc是有效焦距（图像中心点处的焦距，图像512*512时就是（256,256）处的焦距），f是实际焦距（singleFociTrue）
    double fc = ui->lineEdi_fc->text().toDouble();
    // qDebug()<<"分划板移动距离："<<h;
    // qDebug()<<"移动像素数量："<<n;
    // qDebug()<<"相元尺寸："<<a;
    // qDebug()<<"视场点焦距："<<singleFociTrue;
    double d = (fc-singleFociTrue)/singleFociTrue;
    return d;
}

// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆MTF☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
//MTF初始化
void MainWindow::initMTF(){
    QVector<double> vectorX;
    QVector<double> vectorY;
    // 获取当前行数
    int rowCount = ui->tableWidget_MTF->rowCount();
    //清空数据
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *itemX = ui->tableWidget_MTF->item(row, 0);
        QTableWidgetItem *itemY = ui->tableWidget_MTF->item(row, 1);
        if (itemX) {
            itemX->setText("");  // 将该单元格的文本内容设置为空
        }
        if (itemY) {
            itemY->setText("");  // 将该单元格的文本内容设置为空
        }
    }

    //判断checkBox是哪个，如果是2或者3，则自动生成8组或16组视场点坐标填入表格
    if(ui->checkBox_2v2_3->isChecked()){
        //创建两个vector容器用于存放x和y的坐标
        double tempAxisX = ui->lineEdit_axisBeginX_3->text().toDouble();
        double tempAxisY = ui->lineEdit_axisBeginY_3->text().toDouble();
        double pixelGap = ui->lineEdit_pixelGap_3->text().toDouble();
        for(int i = 0;i<3;i++){
            for(int j = 0;j<3;j++){
                vectorX.append(tempAxisX+j*pixelGap);
            }
        }
        for(int i = 0;i<3;i++){
            for(int j = 0;j<3;j++){
                vectorY.append(tempAxisY+i*pixelGap);
            }
        }
    }
    else{

        //创建两个vector容器用于存放x和y的坐标
        double tempAxisX = ui->lineEdit_axisBeginX_3->text().toDouble();
        double tempAxisY = ui->lineEdit_axisBeginY_3->text().toDouble();
        double pixelGap = ui->lineEdit_pixelGap_3->text().toDouble();
        for(int i = 0;i<4;i++){
            for(int j = 0;j<4;j++){
                vectorX.append(tempAxisX+j*pixelGap);
            }
        }
        for(int i = 0;i<4;i++){
            for(int j = 0;j<4;j++){
                vectorY.append(tempAxisY+i*pixelGap);
            }
        }
    }
    //将vector中的元素放进列表，X和Y分别放在前两列
    for(int i = 0;i<rowCount;i++){
        QTableWidgetItem *newItemX = new QTableWidgetItem(QString::number(vectorX[i])); // 示例数据
        QTableWidgetItem *itemX = ui->tableWidget_MTF->item(i, 0);
        if (itemX == nullptr || itemX->text().isEmpty()) {  //使用 if (item->text().isEmpty()) 会有什么问题？
            // 如果单元格还没有创建 QTableWidgetItem 对象，item() 会返回 nullptr。在这种情况下，如果你直接调用 item->text()，就会尝试对空指针调用方法，导致程序崩溃（未定义行为）。
            ui->tableWidget_MTF->setItem(i, 0, newItemX); // 设置第currentRow行的内容;
        }
    }
    for(int i = 0;i<rowCount;i++){
        QTableWidgetItem *newItemY = new QTableWidgetItem(QString::number(vectorY[i])); // 示例数据
        QTableWidgetItem *itemY = ui->tableWidget_MTF->item(i, 1);
        if (itemY == nullptr || itemY->text().isEmpty()) {  //使用 if (item->text().isEmpty()) 会有什么问题？
            // 如果单元格还没有创建 QTableWidgetItem 对象，item() 会返回 nullptr。在这种情况下，如果你直接调用 item->text()，就会尝试对空指针调用方法，导致程序崩溃（未定义行为）。
            ui->tableWidget_MTF->setItem(i, 1, newItemY); // 设置第currentRow行的内容;
        }
    }
}

// MTF最后一列添加按钮
void MainWindow::MTF_TableAddButtons(){
    for(int i=0;i<ui->tableWidget_MTF->rowCount();++i){
        // 尝试从系统图标主题加载"emblem-downloads"图标
        QIcon downloadIcon = QIcon::fromTheme("emblem-downloads");
        QPushButton* button = new QPushButton();
        button->setCheckable(true);  // 设置按钮为可切换状态
        button->setIcon(downloadIcon);
        button->setStyleSheet(buttonStyle2);

        // 按钮从按下状态恢复到正常状态
        button->setContextMenuPolicy(Qt::CustomContextMenu); // 设置右键菜单策略。注意：设置按钮的上下文菜单策略为自定义菜单
        // 这样当用户在按钮上右键单击时，会触发customContextMenuRequested信号
        QMenu *menu = new QMenu; // 创建右键菜单
        QAction * actionRenew = new QAction("初始化"); // 创建菜单内容
        menu->addAction(actionRenew);
        // 连接右键菜单触发信号到槽函数
        connect(button, &QPushButton::customContextMenuRequested, this, [=](const QPoint &pos){
            menu->exec(button->mapToGlobal(pos)); // 将按钮的局部坐标pos转换为屏幕上的全局坐标。这样，菜单会在右键单击的位置弹出，而不是显示在窗口的其他位置
        }); // 这里的代码可以作为参考，在后续屏幕的任何位置点击鼠标时执行操作的位置定位
        connect(actionRenew,&QAction::triggered,this,[=](){
            button->setStyleSheet(buttonStyle2);
        });

        ui->tableWidget_MTF->setCellWidget(i, 6, button);
        // 将行号作为属性附加到按钮上
        button->setProperty("row", i);
        // 连接按钮的点击信号到槽函数
        connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
    }
}

//MTF计算
void MainWindow::MTF(){
    QVector<double> vectorCalDNmax; // 第三列 亮条纹灰度
    QVector<double> vectorCalDNmin; // 第四列 暗条纹灰度
    QVector<double> vectorCalDN; // 第五列 底噪
    QString modeSelection;
    int rowCount = ui->tableWidget_MTF->rowCount(); // 16
    int colCount = ui->tableWidget_MTF->columnCount(); // 5
    // QVector<double> vectorCalMTF; // 第六列 MTF结果
    // 判断当前列表中的数据是否完整，如果不完整，提示错误，否则会崩溃

    //读取列表中的值并存放进vector
    for(int i =0;i<rowCount;i++){
        QTableWidgetItem *itemX1 = ui->tableWidget_MTF->item(i, 2); //第三列
        if(itemX1 && !itemX1->text().isEmpty()){
            vectorCalDNmax.append(itemX1->text().toDouble());
        }else{
            vectorCalDNmax.append(qQNaN());
        }
    }

    for(int i =0;i<rowCount;i++){
        QTableWidgetItem *itemX2 = ui->tableWidget_MTF->item(i, 3); //第四列
        if(itemX2 && !itemX2->text().isEmpty()){
            vectorCalDNmin.append(itemX2->text().toDouble());
        }else{
            vectorCalDNmin.append(qQNaN());
        }
    }
    for(int i =0;i<rowCount;i++){
        QTableWidgetItem *itemX3 = ui->tableWidget_MTF->item(i, 4); //第五列
        if(itemX3 && !itemX3->text().isEmpty()){
            vectorCalDN.append(itemX3->text().toDouble());
        }else{
            vectorCalDN.append(qQNaN());
        }
    }
    // qDebug()<<vectorCalDNmax;
    // qDebug()<<vectorCalDNmin;
    // qDebug()<<vectorCalDN;
    //开始计算
    for(int i=0;i<rowCount;i++){
        vectorCalMTF.append(MTF_Func(vectorCalDNmax[i],vectorCalDNmin[i],vectorCalDN[i]));
    }
    for(int i = 0;i<rowCount;i++){
        QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(vectorCalMTF[i])); // 示例数据
        // QTableWidgetItem *item = ui->tableWidget_distortion->item(i, 4);
        ui->tableWidget_MTF->setItem(i, 5, newItem); // 设置第currentRow行的内容;
    }

    // 绘制曲线
    drawCurve(vectorCalMTF);
    // vectorCalMTF.clear();

    // 准备数据存进csv文件
    QString objectName = ui->lineEdit_objectName->text();
    QString csvPathSupply = "/"+objectName+"/DataExport/MTF";
    QString csvFileName = "/MTF.csv";
    QString dataTitle = "时间戳,模式,起始坐标X,起始坐标Y,区间边长(像素)\n";
    QString lineEdit_axisBeginX_3 = ui->lineEdit_axisBeginX_3->text();
    QString lineEdit_axisBeginY_3 = ui->lineEdit_axisBeginY_3->text();
    QString lineEdit_pixelGap_3 = ui->lineEdit_pixelGap_3->text();

    dataListMTF = {utils::getCurrentTimestamp(),modeSelection,lineEdit_axisBeginX_3,lineEdit_axisBeginY_3,lineEdit_pixelGap_3};
    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataListMTF);

    // 同时，还需要在上述内容的下面存放表格内容
    // QString dataTitleTable = ",,,,\n"; // 空数据
    QStringList dataListTable = {"X0","Y0","DNmax","DNmin","DN","MTF"};
    utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataListTable);
    // 开始逐行存放table中的数据
    // QList<QStringList> dataTable;
    QStringList dataPerRow;
    dataPerRow.fill("",colCount);
    tabMTFAll.clear();

    ///注意：表格中有时候是前9行有内容，有时候是整个16行都有内容，当后面7行没有内容时，很容易崩溃
    for(int i = 0 ; i < rowCount ; ++i){
        for(int j = 0 ; j < colCount ; ++j){
            QTableWidgetItem *item = ui->tableWidget_MTF->item(i, j);
            if(item && !item->text().isEmpty()){
                dataPerRow[j]=item->text();
            }
        }

        // 对于每一行，将数据存进csv文件
        // dataListTable.append(dataPerRow);
        // qDebug()<<"dataPerRow:"<<dataPerRow;
        utils::dataToCsv(csvPathSupply,csvFileName,dataTitle,dataPerRow);
        tabMTFAll.append(dataPerRow);
        // dataPerRow.clear(); // 注意：这里不能使用clear()是因为，我们先前确定过dataPerRow的元素数量，使用clear()后就没有这
        // 些数量的空腔了，在后续其他行的计算中会超出索引
        dataPerRow.fill("",colCount);
    }
    // 将当前按钮恢复可选取状态
    ui->pushButton_start->setEnabled(true);
    QString message = QString("[%1] MTF计算完成").arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);
}

bool MainWindow::isColumnFilled(QTableWidget* tableWidget, int columnIndex, int rowCount) {
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem* item = tableWidget->item(row, columnIndex);
        if (!item || item->text().isEmpty()) {
            return false;  // 如果有空单元格，则未填满
        }
    }
    return true;  // 全部填满
}


double MainWindow::MTF_Func(double DNmax,double DNmin,double DN){
    // MTF计算公式
    // π/4
    double factor = M_PI / 4.0;

    // MTF formula
    double mtf = factor * (DNmax - DNmin) / (DNmax + DNmin - 2 * DN);

    return mtf;
}

// MTF中的按钮的点击槽函数,保存的图像用于生成测试报告
void MainWindow::onButtonClicked(){
    // 检查当前路径是否有名为ImagesMTF的文件夹，没有则创建一个
    // 获取当前应用程序所在的目录
    QString appDir = QCoreApplication::applicationDirPath();
    // 设置目标文件夹路径
    // 获取项目名称
    QString objectName = ui->lineEdit_objectName->text();
    QString imagesFolderPath = appDir + "/" +objectName+"/ImagesMTF";
    // 创建 QDir 对象
    QDir dir(imagesFolderPath);
    // 检查文件夹是否存在，如果不存在则创建
    if (!dir.exists()) {
        QString message = QString("[%1] 初始化文件夹ImagesMTF完成").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        // qDebug()<<"正在创建文件夹";
        dir.mkpath(".");  // 创建目标文件夹
    }
    // 抓取当前QGraphicsView的内容到Images文件夹，格式为JPG，命名规则：质心X-质心Y-时间.jpg
    QGraphicsScene *grabScene = ui->graphicsView->scene(); // 获取 QGraphicsView 的场景内容
    if (!grabScene) return;

    // 查找场景中的 QGraphicsPixmapItem（假设是第一个PixmapItem）
    QGraphicsPixmapItem *pixmapItem = nullptr;
    for (QGraphicsItem *item : grabScene->items()) {
        pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item);
        if (pixmapItem) break;  // 找到第一个 QGraphicsPixmapItem 后退出循环
    }
    if (!pixmapItem) return;  // 如果未找到PixmapItem，则返回

    QPixmap pixmap = pixmapItem->pixmap();
    // 将 QPixmap 转换为 QImage，并转换为灰度图像,注意：只能先转为QImage，才能再转为灰度
    QImage image = pixmapItem->pixmap().toImage().convertToFormat(QImage::Format_Grayscale8);

    // 获取发送信号的按钮
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        // 获取按钮的行索引
        int row = button->property("row").toInt();
        QString message = QString("[%1] Button clicked in row:" + QString::number(row)).arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        // qDebug() << "Button clicked in row:" << row;
        // 判断单元格是否有内容，如果没有内容的话，报错
        QTableWidgetItem *item1 = ui->tableWidget_MTF->item(row, 0);
        QTableWidgetItem *item2 = ui->tableWidget_MTF->item(row, 1);
        // if(!item1 || !item2){
        // 注意：这里只是判断指针是否有效，并没有判断单元格中是否有内容
        // 如果表格单元格为空，即使没有内容，QTableWidget::item 方法仍然返回一个有效指针，但该单元格的文本内容可能是空字符串，这时 toInt 会将空字符串转换为 0
        if(!item1 || !item2 || item1->text().isEmpty() || item2->text().isEmpty()){
            QMessageBox::warning(this,"Error","数据不完整！某个单元格为空或无效。");
            QString message = QString("[%1] 某个单元格为空或无效").arg(utils::getCurrentTimestamp());
            ui->plainTextEdit->appendPlainText(message);
            // qDebug() << "某个单元格为空或无效";
            return;
        }

        QTableWidgetItem *itemX = ui->tableWidget_MTF->item(row, 0); // 获取单元格项目
        QTableWidgetItem *itemY = ui->tableWidget_MTF->item(row, 1); // 获取单元格项目
        int Xmtf = itemX->text().toInt();
        int Ymtf = itemY->text().toInt();
        // 构建完整文件路径和名称
        QString filePath;
        if(ui->lineEdit_objectName->text()!=""){
            filePath = QString("%1/%2-%3-%4.jpg").arg(imagesFolderPath).arg(ui->lineEdit_objectName->text()).arg(Xmtf).arg(Ymtf);
        }
        else{
            filePath = QString("%1/%2-%3.jpg").arg(imagesFolderPath).arg(Xmtf).arg(Ymtf);
        }

        // 保存图像
        if (image.save(filePath, "JPG")) {
            QString message = QString("图像已保存到: %1").arg(filePath);
            ui->plainTextEdit->appendPlainText(message);
            button->setStyleSheet(buttonStyleClicked2); // 只有当图像正常保存时，按钮的状态才发生变化
        } else {
            QMessageBox::warning(this,"Error","图像保存失败，请打开相机！");
            // qDebug() << "图像保存失败！" << filePath;
            // 注意：下面这三行可以用于所有的需要输出到输出窗口的地方
            QString message = QString("[%1] 图像保存失败，请打开相机！").arg(utils::getCurrentTimestamp());
            ui->plainTextEdit->appendPlainText(message);
        }
    }
}

void MainWindow::downImg(){
    // 获取当前应用程序所在的目录
    QString appDir = QCoreApplication::applicationDirPath(); //文本
    // qDebug()<<"当前应用程序所在的目录"<<appDir;
    // 设置目标文件夹路径
    QString objectName = ui->lineEdit_objectName->text();
    QString imagesFolderPath = appDir + "/" +objectName+"/Images";
    // 创建 QDir 对象
    QDir dir(imagesFolderPath); // QDir对象
    // 检查文件夹是否存在，如果不存在则创建
    if (!dir.exists()) {
        QString message = QString("[%1] 初始化文件夹Images完成").arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        // qDebug()<<"正在创建文件夹";
        dir.mkpath(".");  // 创建目标文件夹
    }

    // 抓取当前QGraphicsView的内容到Images文件夹，格式为JPG，命名规则：质心X-质心Y-时间.jpg
    QGraphicsScene *grabScene = ui->graphicsView->scene(); // 获取 QGraphicsView 的场景内容
    if (!grabScene) return;

    // 查找场景中的 QGraphicsPixmapItem（假设是第一个PixmapItem）
    QGraphicsPixmapItem *pixmapItem = nullptr;
    for (QGraphicsItem *item : grabScene->items()) {
        pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(item);
        if (pixmapItem) break;  // 找到第一个 QGraphicsPixmapItem 后退出循环
    }
    if (!pixmapItem) return;  // 如果未找到PixmapItem，则返回

    QPixmap pixmap = pixmapItem->pixmap();
    // 将 QPixmap 转换为 QImage，并转换为灰度图像,注意：只能先转为QImage，才能再转为灰度
    QImage image = pixmapItem->pixmap().toImage().convertToFormat(QImage::Format_Grayscale8);

    // 获取当前时间，用于生成文件名
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    // 获取当前视场点的质心坐标
    double centroidX = ui->lineEdit_centroidX->text().toDouble();  // 替换为实际质心X
    double centroidY = ui->lineEdit_centroidY->text().toDouble();  // 替换为实际质心Y
    // qDebug()<<ui->lineEdit_centroidX;
    // qDebug()<<centroidX;

    // 构建完整文件路径和名称
    // qDebug()<<timeStr;
    // qDebug()<<utils::getCurrentTimestamp();
    QString filePath;
    if(ui->lineEdit_objectName->text()!=""){
        filePath = QString("%1/%2-%3-%4-%5.jpg").arg(imagesFolderPath).arg(ui->lineEdit_objectName->text()).arg(timeStr).arg(centroidX).arg(centroidY);
    }
    else{
        filePath = QString("%1/%2-%3-%4.jpg").arg(imagesFolderPath).arg(timeStr).arg(centroidX).arg(centroidY);
    }
    // filePath = QString("%1/%2-%3-%4.jpg").arg(imagesFolderPath).arg(centroidX).arg(centroidY).arg(timeStr);
    // 保存图像
    if (image.save(filePath, "JPG")) {
        // qDebug() << "图像已保存到" << filePath;
        // 将信息同步到输出窗口
        // 获取当前时间并格式化为字符串
        QString message = QString("[%1] 图像已保存到: %2").arg(utils::getCurrentTimestamp()).arg(filePath);
        ui->plainTextEdit->appendPlainText(message);
    } else {
        QMessageBox::warning(this,"Error","图像保存失败！");
        // qDebug() << "图像保存失败！" << filePath;
        // 注意：下面这三行可以用于所有的需要输出到输出窗口的地方
        QString message = QString("[%1] 图像保存失败！").arg(utils::getCurrentTimestamp());  // 使用utils中的命名空间utils中的函数
        ui->plainTextEdit->appendPlainText(message);
    }
}

void MainWindow::saveRaw()
{

    if(ui->checkBox_externalDetector->isChecked()){
        qDebug()<<"开始保存raw文件";
        // 获取保存帧数
        int rawSaveNum = ui->lineEdit_saveRawNum->text().toInt();
        // 添加输入验证
        if(rawSaveNum <= 0) {
            QMessageBox::warning(this, "错误", "请输入有效的保存帧数!");
            return;
        }

        QString curTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
        QString rawFilesPathTemp = rawFilesPath + curTime;
        QDir rawPath(rawFilesPathTemp);
        if(!rawPath.exists()){
            if(!rawPath.mkpath(".")){
                QMessageBox::warning(this,"错误","无法初始化即时保存RAW文件夹！");
                return;
            }
        }
        emit saveRawSignal(rawSaveNum, rawFilesPathTemp);
    }
}

// 绘制曲线，输入参数是一个vector
void MainWindow::drawCurve(QVector<double> vector){
    // xNum = vector; // 注意：xNum = 由于在声明时，xNum是指针，所以不能直接赋值
    xNum = new QVector<double>(vector);  // 让 xNum 指向 vector 的一个副本,使用xNum来存放样本数据，因为 vectorCalMTF.clear(); 在每次弹出子窗口的瞬间，vectorCalMTF就被清零了
    // 设置 X 和 Y 坐标轴的范围
    // qDebug()<<"vector.size(): "<<vector.size();
    // axisX->setRange(0, vector.size()-1);  // 刚开始我使用这个代码，当vector为0时会崩溃，所以需要再执行前判断vector的size
    if (!vector.isEmpty()) {
        // 设置 X 轴的范围
        axisX->setRange(0, vector.size()-1);
        // 获取 Y 轴的最小值和最大值
        double minY = *std::min_element(vector.begin(), vector.end());
        double maxY = *std::max_element(vector.begin(), vector.end());
        double instance = (maxY-minY)*0.1;
        axisY->setRange(minY-instance, maxY+instance);

    }
    else {
        // 当 vector 为空时，设置默认的范围，避免越界
        axisX->setRange(0, 8); // 默认 X 轴范围
        axisY->setRange(0, 1); // 默认 X 轴范围
    }

    // 清除 series 和 highlightSeries 中的旧数据
    series->clear();
    // highlightSeries->clear();
    // 初始化界面
    // ChartsDialog *chartsdialog = new ChartsDialog;
    // 自动检测vector的大小并绘制曲线，展示在弹出的子窗口chartsdialog上
    // QLineSeries *series = new QLineSeries();
    // qDebug()<<vector.size();
    QString message = QString("[%1] 向量尺寸：" + QString::number(vector.size())).arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);
    for(int i=0;i<vector.size();++i){
        series->append(i,vector[i]);
    }
    // 创建Charts
    // QChart *chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("MTF Line Chart");

    QFont titleFont;
    titleFont.setPointSize(11);  // 调整标题字体大小
    titleFont.setBold(true);      // 使标题加粗
    chart->setTitleFont(titleFont);
    chart->setTitleBrush(QBrush(Qt::black));  // 设置标题颜色为黑色

    // chart->createDefaultAxes(); // 这里已经自动为 chart 创建了默认的坐标轴
    // 删除图例
    chart->legend()->hide();
    // 设置坐标轴X
    QFont axisTitleFont;
    axisTitleFont.setPointSize(10);
    axisTitleFont.setBold(true);      // 使标题加粗

    // QValueAxis *axisX = new QValueAxis;
    axisX->setLabelFormat("%d");
    // axisX->setRange(1, vector.size());
    axisX->setTitleText("Index");
    axisX->setTitleFont(axisTitleFont);
    axisX->setTickInterval(1); // 设置刻度间隔为1

    // 设置坐标轴Y
    // QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%.4f");
    axisY->setTitleText("MTF Value");
    axisY->setTitleFont(axisTitleFont);
    // // 设置X轴和Y轴网格线可见
    // axisX->setGridLineVisible(true);
    // axisY->setGridLineVisible(true);

    // 将坐标轴添加到图表中，并将系列附加到坐标轴
    chart->addAxis(axisX,Qt::AlignBottom);
    chart->addAxis(axisY,Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // highlightSeries->attachAxis(axisX); // 将 highlightSeries 附加到坐标轴
    // highlightSeries->attachAxis(axisY);
    // chart->addSeries(highlightSeries); // 添加到图表中

    // 创建图表视图
    // QChartView *chartView = new QChartView(chart);
    // 将 chart 添加到 chartView 中
    // chartView->setChart(chart);
    // chartView->scene()->addItem(highlightMarker);
    // chartView->setRenderHint(QPainter::Antialiasing);

    chartViewNew->setChart(chart);
    chartViewNew->scene()->addItem(highlightMarker);
    chartViewNew->setRenderHint(QPainter::Antialiasing);

    // 将chart添加到布局中
    // chartDiaUi->verticalLayout->addWidget(chartView);
    chartDiaUi->addWidgetToLayout(chartViewNew);

    // 设置窗口标题
    chartDiaUi->setWindowTitle("MTF Analysis");
    // chartDiaUi->addWidgetToLayout(chartView);
    chartDiaUi->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 展示子窗口界面
    chartDiaUi->show();

    vectorCalMTF.clear(); // 注意，在每次弹出子窗口的瞬间，vectorCalMTF就被清零了
}


// 透过率计算
void MainWindow::updateTransmittance(){
    if (ui->lineEdit_wavelengthTrans->text().toDouble() > 20000.0 || ui->lineEdit_wavelengthTrans->text().toDouble() < 0) {
        QMessageBox::warning(this, "输入错误", "请输入范围在 0 ~ 20000.0 之间的小数！");
        ui->lineEdit_wavelengthTrans->setFocus();
        ui->lineEdit_wavelengthTrans->clear();
        return;
    }
    //加载波长和透过率对应关系的excel文件，这个文件已经嵌入到软件了
    QString excelFilePath = ":/ExcelData/ExcelFiles/wavelength_transmittance.xlsx";
    QFile file(excelFilePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Error","无法打开资源文件wavelength_transmittance.xlsx！");
        return;
    }
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        QMessageBox::warning(this,"Error","无法创建用于拷贝wavelength_transmittance.xlsx的临时文件！");
        return;
    }
    tempFile.write(file.readAll());
    tempFile.flush();  // 保证数据写入
    file.close();
    // 用 QXlsx 打开临时文件
    QXlsx::Document xlsxDoc(tempFile.fileName());
    int row = 1;
    while (true) {
        QVariant xVar = xlsxDoc.read(row, 1); // 第一列
        QVariant yVar = xlsxDoc.read(row, 2); // 第二列

        if (xVar.isNull() || yVar.isNull())
            break;  // 到文件末尾了

        bool okX = false, okY = false;
        double x = xVar.toDouble(&okX);
        double y = yVar.toDouble(&okY);

        if (okX && okY) {
            dataPairs.append(qMakePair(x, y));
        }

        row++;
    }

    // // 可选：打印检查
    // for (const auto& pair : dataPairs) {
    //     qDebug() << "Wavelength:" << pair.first << "Transmittance:" << pair.second;
    // }

    // 开始检索列表，查找与当前输入波长最接近的透过率
    // 读取波长
    double wavelengthInput = ui->lineEdit_wavelengthTrans->text().toDouble();
    if (dataPairs.isEmpty())
        ui->lineEdit_officialLensTrans->setText("error");
        // return 0.0; // 或者 throw/返回 NAN

    double minDiff = std::abs(dataPairs[0].first - wavelengthInput);
    double nearestY = dataPairs[0].second;

    for (const auto &pair : dataPairs)
    {
        double diff = std::abs(pair.first - wavelengthInput);
        if (diff < minDiff)
        {
            minDiff = diff;
            nearestY = pair.second;
        }
    }

    ui->lineEdit_officialLensTrans->setText(QString::number(nearestY,'f',4));
    // return nearestY;

}

void MainWindow::transmission(){
    // 查询表格的行数
    int rowCount = tableWidget_listTrans->rowCount();
    // 循环计算表格中的每一行的透过率
    for(int i = 0;i<rowCount;i++){
        // double wavelengthTrans = tableWidget_listTrans->item(i,0)->text().toDouble();
        double officialLenslPower = tableWidget_listTrans->item(i,1)->text().toDouble();
        double officialLensTrans = tableWidget_listTrans->item(i,2)->text().toDouble();
        double testLensPower = tableWidget_listTrans->item(i,3)->text().toDouble();
        // 开始计算透过率
        double testLensTrans = testLensPower / officialLenslPower * officialLensTrans; // 测试镜头透过率
        // 写入表格
        QTableWidgetItem *newItem = new QTableWidgetItem(QString::number(testLensTrans)); // 示例数据
        tableWidget_listTrans->setItem(i,4,newItem);
    }

    // double wavelengthTrans1         = ui->lineEdit_wavelengthTrans->text().toDouble(); // 波长
    // double officialLensTrans1       = ui->lineEdit_officialLensTrans->text().toDouble(); // 参照镜头透过率
    // double originalOpticalPower         = ui->lineEdit_originalOpticalPower->text().toDouble(); // 原始光功率
    // double officialLensTransStand       = ui->lineEdit_officialLensTransStand->text().toDouble(); // 参照镜头透过率（标准，官方参数）
    // double officialLensOpticalPower     = ui->lineEdit_officialLensOpticalPower->text().toDouble(); // 参照镜头光功率（校正前）
    // double officialLensTrans            = ui->lineEdit_officialLensTrans->text().toDouble(); // 参照镜头透过率（校准前）

    // double officialLensOpticalPower = ui->lineEdit_officialLensOpticalPower->text().toDouble(); // 参照镜头光功率
    // double testLensOpticalPower     = ui->lineEdit_testLensOpticalPower->text().toDouble(); // 测试镜头光功率
    // double testLensTrans            = ui->lineEdit_testLensTrans->text().toDouble(); // 测试镜头透过率
    // qDebug()<<wavelengthTrans;

    // 开始计算透过率
    // testLensTrans = testLensOpticalPower / officialLensOpticalPower * officialLensTrans;
    // officialLensTrans       = officialLensOpticalPower / originalOpticalPower * 100; // 参照镜头透过率（校准前）
    // officialLensTransAfter  = officialLensOpticalPowerAfter / originalOpticalPower * 100; // 参照镜头透过率（校准后）
    // testLensTrans           = testLensOpticalPower / originalOpticalPower * 100; // 测试镜头透过率


    // 刷新计算结果
    // ui->lineEdit_officialLensTrans->setText(QString::number(officialLensTrans));
    // ui->lineEdit_officialLensTrans->setText(QString::number(officialLensTransAfter));
    // ui->lineEdit_testLensTrans->setText(QString::number(testLensTrans));
    // 将当前按钮恢复可选取状态
    ui->pushButton_start->setEnabled(true);
}


void MainWindow::onPointHovered(const QPointF &point, bool state) {

    if (state) { // 鼠标刚刚悬停到该数据点上
        // 检查 x,y 值是否接近整数
        qreal xValue = point.x();
        int roundedX = qRound(xValue); // 将 x 值四舍五入为最近的整数
        // 如果 x 值和最近的整数差小于 0.2，则显示坐标
        if (abs(xValue - roundedX)< 0.1 && roundedX>=0 && roundedX<xNum->size()) {
            if (lastHoveredPoint == QPointF(roundedX, xNum->at(roundedX))) return;
            lastHoveredPoint = QPointF(roundedX, xNum->at(roundedX));  // 更新上次悬停的点

            QString coordText = QString("X: %1, Y: %2").arg(roundedX).arg(xNum->at(roundedX)); ////注意，在每次弹出子窗口的瞬间，vectorCalMTF就被清零了,这里不能使用vectorCalMTF[roundedX]
            //// 注意：xNum->at(roundedX)：由于xNum是指针，所以读取其指向的vector中的数据时使用的是      ☆☆☆☆☆指针名->at(索引)☆☆☆☆☆
            //注意：这里要显示的并不是当前鼠标光标的坐标值
            // 而应该是鼠标想要靠近的那个样本点的真实y值，是独一无二的
            QToolTip::showText(QCursor::pos(), coordText);  // 注意：☆☆☆☆默认在显示几秒后就会自动消失☆☆☆☆，后续可选解决策略：在图表上添加一个持久的 QLabel 来替代 QToolTip

            QPointF chartPoint = chart->mapToPosition(QPointF(roundedX, xNum->at(roundedX))); ////注意，在每次弹出子窗口的瞬间，vectorCalMTF就被清零了,这里不能使用vectorCalMTF[roundedX]

            // 更新标记位置
            highlightMarker->setPos(chartPoint);
            highlightMarker->setVisible(true);
        }
        else {
            QString message = QString("[%1] 超出索引").arg(utils::getCurrentTimestamp());
            ui->plainTextEdit->appendPlainText(message);
            // qDebug()<<"超出索引";
            QToolTip::hideText();
        }
    }
    else {
        QToolTip::hideText();
        lastHoveredPoint = QPointF();  // 重置上次悬停点
    }
}

void MainWindow::onChartDialogClosed(){
    // if (chartView) {
    if (chartViewNew) {
        // 假设 chartDiaUi 是指向布局的指针
        // 移除 chartView 之前，先移除坐标轴
        chart->removeAxis(axisX);
        chart->removeAxis(axisY); // 移除坐标轴的刻度显示，不然，重复运行时会在窗口不断堆叠刻度

        // qDebug()<<"开始移除";
        QLayout *layout = chartDiaUi->layout(); // 获取布局指针
        if (layout) {
            // layout->removeWidget(chartView); // 从布局中移除 chartView，下次打开子窗口时会再把chartView恢复的： chartDiaUi->addWidgetToLayout(chartView);
            layout->removeWidget(chartViewNew); // 从布局中移除 chartView，下次打开子窗口时会再把chartView恢复的： chartDiaUi->addWidgetToLayout(chartView);
            // qDebug()<<"移除完成";
        }
    }
}

// QString getCurrentTimestamp() {  // 在C++中，如果函数的定义在调用之后，则需要先声明函数。
//     return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
// }

void MainWindow::testReport(){
    // 测试对象代号，只能输入字母和数字
    QString objectName = ui->lineEdit_objectName->text(); // %%%测试对象
    // 测试对象属性
    QString objectProperties;
    if(ui->checkBox_lens->isChecked()){
        objectProperties = "光学镜头"; // %%%测试对象种类
    }
    else{
        objectProperties = "光学整机";
    }

    /// ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆ 开始绘制PDF格式的测试报告 ☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
    // 获取当前时间，用于对输出报告进行命名
    QString reportTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    // 1. 打开文件保存对话框
    // QString filePath = QFileDialog::getSaveFileName(this, "Save PDF", "C:/Users/P14S/Reports"+ reportTime, "PDF Files (*.pdf)"); // 打开对话框并获取保存路径
    QString filePath = QFileDialog::getSaveFileName(this, "Save PDF", reportTime, "PDF Files (*.pdf)"); // 打开对话框并获取保存路径
    // 笔记：这里是为了调试方便，将默认路径设置为当前电脑的桌面，后续需要将路径删去
    if(filePath.isEmpty())
        return;

    // 2. 创建打印机设置
    // QPrinter printer(QPrinter::ScreenResolution);
    QPrinter printer(QPrinter::PrinterResolution);
    // 笔记： 上述的输出质量选择很重要，如果选择ScreenResolution，会面临打印出的pdf难以居中的问题，会发生错位，推荐使用PrinterResolution
    printer.setOutputFormat(QPrinter::PdfFormat); // 设置打印机的输出模式是pdf模式

    // 3. 设置pdf格式
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setFullPage(true); /// 文本始终无法居中对齐的根本原因，默认情况下，QPrinter 会预留打印机的边距。如果希望完全消除这些边距，使用 setFullPage(true) 方法：
    /// 这个边距不消除，生成的pdf，文字始终是错位的，因为无论是paperRage还是pageRage，都会偏右
    // pdfWriter.setPageMargins(QMarginsF(20,20,20,20)); // 设置页边距
    ///笔记：QPrinter: 提供输出设备的信息，输出格式（打印机或pdf）、文件路径、页面设置、分辨率和质量 注意：QPrinter更侧重于输出到实际的物理设备
    ///     QPdfWriter：更侧重于直接输出到PDF文件
    ///     QPainter：在输出设备（窗口、图像、打印机、文件（如pdf））上绘制具体内容

    // 4. 开始绘制
    QPainter painter; // 创建画笔
    // QPainter painter_text; // 注意： 在同一个作用域中尝试同时创建并使用了两个 QPainter 对象，并且它们都试图操作同一个打印设备（printer 或 pdfWriter）。这是不被允许的
    if(!painter.begin(&printer)){ // QPainter 的绘制目标设置为 printer,参数（printer）是一个指向QPaintDevice（painter）的指针
        QMessageBox::warning(this,"Error","无法打开打印设备！");
        return;
    }

    // 5. 设置页边距，即设置哪些区域可以用于绘制内容
    QRectF paperRect = printer.paperRect(QPrinter::Point); // 获取整个页面矩形区域  单位是点 (pt) ，1 点 (pt) = 1/72 英寸（大约 0.3528 毫米）。
    double margin_up = 72; // 设置上下边距为2.54cm，这里的72的单位是 点，1点 = 1/72 英寸，1英寸=2.54cm
    double margin_left = 3.18 / 2.54 * 72; // 设置左右边距为3.18cm
    // qDebug()<<"paperRect.width() "<<paperRect.width(); // 标准的A4纸的尺寸,宽595点,210mm
    // qDebug()<<"paperRect.height() "<<paperRect.height(); // 标准的A4纸的尺寸，高842点，297mm
    // 笔记：如果使用的不是paperRect而是pageRect，发现这两个数比标准的595和842少20，这是因为我们上一句使用的是page而不是paper，所以QPrinter默认是有页边距20点的
    double paperWidth = paperRect.width() - 2 * margin_left; // 手动设置存放文字的区域
    double paperHeight = paperRect.height() - 2 * margin_up;
    // qDebug()<<"paperWidth:"<<paperWidth; // 415
    // qDebug()<<"paperHeight:"<<paperHeight; // 698
    // 笔记：点 (pt): A4 纸张宽度是 595pt，高度是 842pt（1 英寸 = 72 点，1 英寸 = 25.4mm，计算方法是 210mm / 25.4mm * 72 和 297mm / 25.4mm * 72）

    // 6. 设置字体集合
    QFont fontBlack_B("黑体",QFont::Bold); // 黑体加粗
    fontBlack_B.setPointSizeF(19); // 19号字体，用于主标题
    QFont fontTimesNewRoman_B("Times New Roman"); // 新罗马加粗
    fontTimesNewRoman_B.setPointSizeF(19); // 19号字体，用于主标题
    QFont fontSong("宋体");
    fontSong.setPointSizeF(14);

    // ***测试报告标题***
    QString titleText = "光学模组与整机标准化测试报告";
    painter.setFont(fontBlack_B); // 给画笔附加属性——黑体加粗19号
    // 使用水平居中对齐
    QRectF targetRect(margin_left, margin_up, paperWidth, paperHeight);
    painter.drawText(targetRect, Qt::AlignHCenter, titleText); /// 这种居中方式是最合适的
    /// 注意：Qt::AlignHCenter 无法感知逻辑宽度，只会以包围矩形（280）为参考，但实际视觉上的居中对齐，需要使用逻辑宽度QFontMetricsF::horizontalAdvance,364

    // ***公司名称***
    QString companyName = "上海国科航星量子科技有限公司";
    painter.setFont(fontSong);
    // 设置行距
    QFontMetrics metrics(painter.font());
    float lineSpacing = metrics.height() * 2; // 行距为字体高度的 2 倍
    // 设置第二行文字的目标矩形
    QRectF targetRect2(margin_left, margin_up + lineSpacing, paperWidth, paperHeight - lineSpacing);
    painter.drawText(targetRect2, Qt::AlignHCenter, companyName);

    // ***测试对象***
    QString testObjectNameHeader = "测试对象：";
    QString testObjectName = ui->lineEdit_objectName->text();
    fontTimesNewRoman_B.setPointSize(14);
    painter.setFont(fontTimesNewRoman_B);
    float currentY = margin_up + 3 * lineSpacing; // 当前的起始 y 坐标
    QRectF targetRect3(margin_left, currentY, paperWidth, paperHeight);
    painter.drawText(targetRect3, Qt::AlignLeft, testObjectNameHeader + testObjectName);

    // 设置报告正文内容
    QStringList textLines = {
        "一、基本参数",
        "   光源：",
        "   像元尺寸：",
        "   灰度阈值：",
        "   平行光管焦距：",
        "",
        "二、视场",
        "   1.测试过程",
        "   上极：",
        "   下极：",
        "   左极：",
        "   右极：",
        "",
        "   2.测试结果",
        "   俯仰视场：",
        "   水平视场：",
        "",
        "三、焦距",
        "   单视场点焦距：",
        "",
        "   平均焦距：",
        "",
        "四、畸变",
        "   1.参数设置",
        "   模式：",
        "   有效焦距：",
        "   起始坐标：",
        "   视场点间距（像素）：",
        "   分划板移动距离（mm）：",
        "",
        "   2.测试结果",
        "   相对畸变数值表：",
    };

    // 设置测试结果
    QString lightSourse = "";
    if(ui->checkBox_kjg->isChecked()){
        lightSourse = "可见光";
    }
    else if(ui->checkBox_jhw->isChecked()){
        lightSourse = "近红外";
    }
    else if(ui->checkBox_dbhw->isChecked()){
        lightSourse = "短波红外";
    }
    else if(ui->checkBox_zbhw->isChecked()){
        lightSourse = "中波红外";
    }
    else{
        lightSourse = "长波红外";
    }
    QString model_2v3;
    if(ui->checkBox_2v2->isChecked()){
        model_2v3 = "9点模式";
    }
    else if(ui->checkBox_3v3->isChecked()) model_2v3 = "16点模式";
    else model_2v3 = "自定义模式";

    // 获取ui->tableWidget_aveFocalLength第一列的全部数据，存进列表
    QStringList tabAveFocList;
    tabAveFocList.fill("",8);
    for(int i  = 0 ; i < ui->tableWidget_aveFocalLength->rowCount(); ++i){
        QTableWidgetItem *Item = ui->tableWidget_aveFocalLength->item(i,0);
        if(Item && !Item->text().isEmpty()){
            tabAveFocList[i] = Item->text();
        }
    }
    // 创建一个两行8列的表格，存放样本点索引及其焦距
    QString tabAveFocPrintA = "";
    QString tabAveFocPrintB = "";
    for(int i = 0 ; i < 8 ; ++i){
        if(i<4){
            tabAveFocPrintA += QString("%1、%2 ").arg(QString::number(i+1)).arg( tabAveFocList[i]);
        }
        else tabAveFocPrintB += QString("%1、%2 ").arg(QString::number(i+1)).arg( tabAveFocList[i]);
    }

    QStringList textLines_result = {
        "",
        lightSourse,
        ui->lineEdit_pixelSize->text(),
        ui->lineEdit_grayThre->text(),
        ui->lineEdit_colliFoci->text(),
        "",
        "",
        "",
        ui->lineEdit_up->text(),
        ui->lineEdit_down->text(),
        ui->lineEdit_left->text(),
        ui->lineEdit_right->text(),
        "",
        "",
        ui->lineEdit_elevViewAng->text(),
        ui->lineEdit_horiViewAng->text(),
        "",
        "",
        tabAveFocPrintA, /// 单点焦距，需要将表格中的八个数摆出来
        tabAveFocPrintB,
        ui->lineEdit_aveFocalLength->text(),
        "",
        "",
        "",
        model_2v3,
        ui->lineEdi_fc->text(),
        ui->lineEdit_axisBeginX->text() + " " + ui->lineEdit_axisBeginY->text() ,
        ui->lineEdit_pixelGap->text(),
        ui->lineEdit_reticleMoveMm->text(),
        "",
        "",
        "",
    };


    currentY += 2 * lineSpacing; // 当前的起始 y 坐标
    // for (const QString& line : textLines) { // 这种方式可以直接对列表内容进行操作，无需创建索引
    for (int i = 0 ; i < textLines.size(); ++i) {

        // 检查是否超出页面底部
        if (currentY + lineSpacing > paperRect.height() - margin_up) {
            // 如果超出页面底部，结束当前页面并创建新页面
            printer.newPage();
            currentY = margin_up; // 重置起始 y 坐标
        }
        QRectF lineRect(margin_left, currentY, paperWidth, paperHeight);
        // 计算附加文本的 x 坐标，紧随原始文本之后
        int textWidth = painter.fontMetrics().horizontalAdvance(textLines[i]);
        QRectF lineRectResult(margin_left + textWidth + 8, currentY, paperWidth - textWidth - 8, paperHeight);
        painter.drawText(lineRect, Qt::AlignLeft, textLines[i]);
        painter.drawText(lineRectResult, Qt::AlignLeft, textLines_result[i]);
        // 计算下一行的 y 坐标
        currentY += lineSpacing; // 根据字体高度和行距动态调整
    }

    // 获取下一个y坐标为currentY，下面需要放置的是相对畸变数值表
    int tableWidth = paperWidth - 40; // 表格宽度 = 页面宽度-40
    int columnCountDis = ui->tableWidget_distortion->columnCount(); // // 畸变表格列数
    int rowCountDis = ui->tableWidget_distortion->rowCount(); // 畸变表格行数
    int columnCountMTF = ui->tableWidget_MTF->columnCount(); // // MTF表格列数
    int rowCountMTF = ui->tableWidget_MTF->rowCount(); // MTF表格行数

    // 动态计算每列宽度（等分页面宽度）
    QVector<int> columnWidthsDis(columnCountDis, tableWidth / columnCountDis); // 畸变表格打印列宽
    QVector<int> columnWidthsMTF(columnCountDis, tableWidth / columnCountDis); // MTF表格打印列宽

    // 绘制畸变表格头部
    for (int row = -1; row < rowCountDis; ++row) {
        int currentX = margin_left; // 当前 X 坐标
        bool isHeader = (row == -1);
        int cellPadding = 5; // 单元格内部边距
        int cellHeight = lineSpacing/1.2;
        for (int col = 0; col < columnCountDis; ++col) {
            QRectF cellRect(currentX + 20, currentY, columnWidthsDis[col], cellHeight); // +20表示缩进20，所以前面tableWidth-40
            // 绘制单元格边框
            painter.drawRect(cellRect);
            // 绘制单元格内容
            QString text = isHeader ? ui->tableWidget_distortion->horizontalHeaderItem(col)->text()
                                    : ui->tableWidget_distortion->item(row, col)->text();
            // qDebug()<<text;
            painter.drawText(cellRect.adjusted(cellPadding, cellPadding, -cellPadding, -cellPadding),
                             Qt::AlignLeft | Qt::AlignVCenter, text);
            currentX += columnWidthsDis[col];
        }
        currentY += cellHeight;
        // 如果当前 Y 坐标超过页面高度，分页
        if (currentY + cellHeight > paperRect.height() - margin_up) {
            printer.newPage();
            currentY = margin_up; // 重置 Y 坐标
        }
    }

    // 继续基于currentY绘制后续内容
    currentY += lineSpacing;
    QStringList textLines2 = {
        "五、MTF",
        "   1.参数设置",
        "   模式：",
        "   起始坐标：",
        "   区间边长（像素）：",
        "",
        "   2.测试结果",
        "   MTF数值表格：",
    };
    QString model_2v3_3;
    if(ui->checkBox_2v2_3->isChecked()){
        model_2v3_3 = "9点模式";
    }
    else if(ui->checkBox_3v3_3->isChecked()) model_2v3_3 = "16点模式";
    else model_2v3_3 = "自定义模式";

    QStringList textLines2_result = {
        "",
        "",
        model_2v3_3,
        ui->lineEdit_axisBeginX_3->text() + " " + ui->lineEdit_axisBeginY_3->text() ,
        ui->lineEdit_pixelGap_3->text(),
        "",
        "",
        "",
    };

    for (int i = 0 ; i < textLines2.size() ; ++i) {
        // 检查是否超出页面底部
        if (currentY + lineSpacing > paperRect.height() - margin_up) {
            // 如果超出页面底部，结束当前页面并创建新页面
            printer.newPage();
            currentY = margin_up; // 重置起始 y 坐标
        }
        QRectF lineRect(margin_left, currentY, paperWidth, paperHeight);
        // 计算附加文本的 x 坐标，紧随原始文本之后
        int textWidth = painter.fontMetrics().horizontalAdvance(textLines2[i]);
        QRectF lineRectResult(margin_left + textWidth + 8, currentY, paperWidth - textWidth - 8, paperHeight);
        painter.drawText(lineRect, Qt::AlignLeft, textLines2[i]);
        painter.drawText(lineRectResult, Qt::AlignLeft, textLines2_result[i]);

        // 计算下一行的 y 坐标
        currentY += lineSpacing; // 根据字体高度和行距动态调整
    }


    // 开始绘制MTF表格，注意，需要在最后一列放入测试过程中截取的图像
    // int tableWidth2  = paperWidth; // 设置表格宽度

    // 获取ImagesMTF文件夹中的最新的图片的尺寸比例，而后根据这个比例，设置图片单元格的宽和高
    // 获取当前程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    // qDebug()<<"appDir"<<appDir;
    QString imagesFolderPath = appDir + "/" +objectName+"/ImagesMTF";
    // qDebug()<<"imagesFolderPath"<<imagesFolderPath;
    // 检查文件夹是否存在
    QDir dir(imagesFolderPath);
    if (!dir.exists()) {
        qDebug() << "Folder does not exist:" << imagesFolderPath;
        qDebug() << "临时创建目标文件夹";
        dir.mkpath(".");  // 创建目标文件夹
    }

    // 设置过滤器（仅选择 JPG 文件）
    QStringList filters;
    filters << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Time | QDir::Reversed); // 按时间排序，最新的文件在第一个

    // 获取文件列表
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);

    if (fileList.isEmpty()) {
        QString message = QString("[%1] No JPG files found in the folder:" + imagesFolderPath).arg(utils::getCurrentTimestamp());
        ui->plainTextEdit->appendPlainText(message);
        // qDebug() << "No JPG files found in the folder:" << imagesFolderPath;
        // 当文件夹中没有一张图片时，此时加载系统中自带的默认图片☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
        fileList.append(QFileInfo(":/Icon/Icon_images/noImage.jpg"));
        // return;
    }

    QFileInfo latestFile = fileList.first();
    QString message = QString("[%1] Latest JPG file:" + latestFile.fileName()).arg(utils::getCurrentTimestamp());
    ui->plainTextEdit->appendPlainText(message);
    // qDebug() << "Latest JPG file:" << latestFile.fileName();

    // // 加载图片并获取尺寸
    QImage image(latestFile.absoluteFilePath());
    // if (image.isNull()) {
    //     qDebug() << "Failed to load image:" << latestFile.absoluteFilePath();
    //     // 当文件夹中根本就没有图片时，需要设置默认图片尺寸信息1920*1200
    //     // return;
    // }
    // 输出图片尺寸
    qDebug() << "Image dimensions:" << image.width() << "x" << image.height();

    // 为最后一列的图像设置宽度，单位为点，共595点，70*5 90 155
    // 动态计算每列宽度（等分页面宽度）
    // QVector<int> columnWidths2(columnCount2, tableWidth2 / columnCount2);
    // QVector<int> columnWidths2(7,70); // 初始化一个QVector，为每个元素设置相同的值
    int cellImageWidth = 85;
    // float cellHeight = image.height() / image.width() * cellImageWidth;
    float cellHeight = static_cast<float>(image.height()) / image.width() * cellImageWidth; // 根据实际图片比例动态调整图片单元格高度
    // int cellHeight = static_cast<int>(cellHeight_f); // 上一句转化为浮点的目的是不让/的结果是0，这一句转为int的目的是，后面在画单元格时的高度cellHeight会自动变整数，这就会产生误差
    // 即，如果currentY += cellHeight;中没有将currentY从int转为float类型，cellHeight会被自动转为int，这就是误差的来源
    // 在 C++ 中，整数相除的结果默认是取整的（也称为截断小数部分的结果），即只保留商的整数部分，
    // 所以这里image.height()和image.width()都是int类型的，故其除完的结果也是默认取整的，将其中一个或两个操作数转换为浮点数类型即可得到浮点类型的计算结果
    // static_cast<float>(a) 是一种 显式类型转换，用于将整数 a 转换为浮点数 float
    // qDebug() << "image.height():" << image.height();
    // qDebug() << "image.width():" << image.width();
    // qDebug() << "cellImageWidth:" << cellImageWidth;
    // qDebug() << "image.height() / image.width():" << image.height() / image.width();
    // qDebug() << "cellHeight:" << cellHeight;
    QVector<int> columnWidths2 = {55,55,55,55,55,55,cellImageWidth}; // 初始化一个QVector，为每个元素设置不同的值
    // int cellHeight = 80 ; // 设置表格的高度80
    /// 注意：这里的cellImageWidth = 85和 cellHeight = ? 代表的是图像存放单元格的尺寸比例，我们希望这个比例和相机的尺寸一致

    /// 获取相机支持的所有分辨率和帧率、最大帧率信息
    // auto cameras = QMediaDevices::videoInputs();
    // for (const QCameraDevice &camera : cameras) {
    //     qDebug() << "Camera:" << camera.description();

    //     // 遍历支持的格式
    //     const QList<QCameraFormat> formats = camera.videoFormats();
    //     for (const QCameraFormat &format : formats) {
    //         qDebug() << "Resolution:" << format.resolution()
    //         << "Frame rate (min-max):" << format.minFrameRate() << "-" << format.maxFrameRate();
    //     }
    // }

    // 绘制表格头部
    for (int row = -1; row < rowCountMTF; ++row) {
        int currentX = margin_left; // 当前 X 坐标
        bool isHeader = (row == -1); // 当row==-1时，isHeader == true，否则，isHeader == false
        int cellPadding = 5; // 单元格内部边距
        for (int col = 0; col < columnCountMTF; ++col) {
            QRectF cellRect(currentX, currentY, columnWidths2[col], cellHeight); // 这里的cellHeight可以是浮点数，没有影响
            // 绘制单元格边框
            painter.drawRect(cellRect);
            qDebug()<<"单元格宽度： "<<cellRect.size(); // 63.75
            // 绘制单元格内容
            // QString text = isHeader ? ui->tableWidget_distortion->horizontalHeaderItem(col)->text()
            //                         : ui->tableWidget_distortion->item(row, col)->text();
            // /// 语法解析：上述代码使用了三元运算符：condition ? expression_if_true : expression_if_false;
            // QString text;
            // if(isHeader){
            //     text= ui->tableWidget_distortion->horizontalHeaderItem(col)->text();
            // } // 这一段代码会导致程序崩溃

            QString text;
            // qDebug()<<ui->tableWidget_MTF->columnCount();
            if (isHeader) {
                if (col >= 0 && col < columnCountMTF) { // 确保 col 在有效范围内
                    QTableWidgetItem *headerItem = ui->tableWidget_MTF->horizontalHeaderItem(col);
                    if (headerItem) { // 确保头部项不为空
                        text = headerItem->text();
                    } else {
                        text = ""; // 或者设置默认值
                        qDebug() << "Header item for column" << col << "is not set.";
                    }
                } else {
                    qDebug() << "Invalid column index:" << col;
                }
            }
            else{
                QTableWidgetItem *item = ui->tableWidget_MTF->item(row, col);
                if(item != nullptr && !item->text().isEmpty()){
                    text = ui->tableWidget_MTF->item(row, col)->text();
                }
            }
            painter.drawText(cellRect.adjusted(cellPadding, cellPadding, -cellPadding, -cellPadding),
                             Qt::AlignLeft | Qt::AlignVCenter, text); // 绘制表格文本内容

            qDebug()<<"column"<<columnCountMTF-1;
            qDebug()<<"row"<<row;

            /// 开始绘制图片，在最后一列
            if(col==columnCountMTF-1 && row != -1){
                // 当前的行坐标是 row ,当前的列坐标分别是0和1, 所以图片的文件名为 项目名-ui->tableWidget_MTF->item(row, 0)->text()-ui->tableWidget_MTF->item(row, 1)->text().jpg
                QString mtf_X = ui->tableWidget_MTF->item(row, 0)->text();
                QString mtf_Y = ui->tableWidget_MTF->item(row, 1)->text();
                qDebug()<<"mtf_X"<<mtf_X;
                qDebug()<<"mtf_Y"<<mtf_Y;
                // 读取路径中是否有上述图片，有的话打印进pdf
                // // 获取当前程序所在目录——这里注释掉是因为在前文设置图像单元格尺寸时已经获取过了
                // QString appDir = QCoreApplication::applicationDirPath();
                // qDebug()<<"appDir"<<appDir;
                // QString imagesFolderPath = appDir + "/" +objectName+"/ImagesMTF";
                // qDebug()<<"imagesFolderPath"<<imagesFolderPath;
                QString imageName = QString("%1-%2-%3.jpg").arg(objectName).arg(mtf_X).arg(mtf_Y);
                qDebug()<<"imageName"<<imageName;
                // 检查imagesFolderPath中是否有imageName
                // 拼接图片的完整路径
                QString imagePath = QDir(imagesFolderPath).filePath(imageName);
                QString message = QString("[%1] imagePath:" + imagePath).arg(utils::getCurrentTimestamp());
                ui->plainTextEdit->appendPlainText(message);
                // qDebug()<<"imagePath"<<imagePath;
                // 检查文件是否存在
                if (!QFile::exists(imagePath)) {
                    QString message = QString("[%1] Image not found:" + imagePath).arg(utils::getCurrentTimestamp());
                    ui->plainTextEdit->appendPlainText(message);
                    qWarning() << "Image not found:" << imagePath;
                }
                else{
                    // 加载图片
                    QImage mtfImage(imagePath);
                    if (mtfImage.isNull()) {
                        qWarning() << "Failed to load image:" << imagePath;
                    }
                    else{
                        ///尝试1 不失真，但由于相机图像尺寸和列表图像栏尺寸不匹配，导致拉伸
                        // 计算目标矩形的尺寸
                        QSize targetSize = cellRect.size().toSize();
                        // 根据目标矩形尺寸缩放图片，保持宽高比
                        QImage scaledImage = mtfImage.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        // 计算绘制时的目标位置，使图片居中
                        QRect targetRect(cellRect.topLeft().toPoint(), scaledImage.size());
                        targetRect.moveCenter(cellRect.center().toPoint());
                        // 绘制缩放后的图片
                        // painter.drawImage(targetRect, scaledImage);
                        painter.drawImage(cellRect,mtfImage);// 绘制表格图像内容

                    }
                }
            }
            currentX += columnWidths2[col];
        }
        currentY += cellHeight;  // 表格在绘制时，如果cellHeight是浮点数，比如63.75，在这种 ☆☆☆int和float之间的运算下会将63.75变成63☆☆☆，这就导致了绘制的表格中行与行之间出现了重叠线
        /// 在 C++ 中，如果 currentY 是 int，而 cellHeight 是 float，执行 currentY += cellHeight 时，cellHeight 会被转换为 int，即其小数部分会被截断

        qDebug()<<"单元格y坐标递增"<<currentY;
        // 如果当前 Y 坐标超过页面高度，分页
        if (currentY + cellHeight > paperRect.height() - margin_up) {
            printer.newPage();
            currentY = margin_up; // 重置 Y 坐标
        }
    }

    // 绘制chart图像
    currentY += lineSpacing; // 根据字体高度和行距动态调整
    QString MTF_chart = "MTF折线图";
    // QRectF lineRect(margin_left, currentY, paperWidth, paperHeight);
    // painter.drawText(lineRect, Qt::AlignLeft, MTF_chart);
    /// 之所以不在这里绘制文字，是想在后续判断图像是否超出本页，如果是，则文字也放进新页
    // 计算下一行的 y 坐标
    // currentY += lineSpacing; // 根据字体高度和行距动态调整
    if (currentY + lineSpacing > paperRect.height() - margin_up) {
        // 如果超出页面底部，结束当前页面并创建新页面
        printer.newPage();
        currentY = margin_up; // 重置起始 y 坐标
    }

    QString chartFolderPath = appDir + "/" +objectName+"/chartImages";
    // 打开文件夹
    QDir dirChart(chartFolderPath);
    // 过滤设置，仅匹配 jpg 文件
    dirChart.setFilter(QDir::Files | QDir::NoSymLinks);
    dirChart.setNameFilters(QStringList() << "*.jpg");
    // 按修改时间排序
    dirChart.setSorting(QDir::Time | QDir::Reversed);

    // 获取文件列表
    QFileInfoList fileListChart = dirChart.entryInfoList(QDir::Files);
    if (fileListChart.isEmpty()) {
        qDebug() << "No JPG files found in the folder:" << chartFolderPath;
        return;
    }
    // 获取最新的文件
    QFileInfo latestChartFile = fileListChart.first();
    qDebug() << "latestChartFile JPG file:" << latestChartFile.fileName();

    // 加载图片
    QImage imageChart(latestChartFile.absoluteFilePath());
    if (image.isNull()) {
        qDebug() << "Failed to load image:" << latestChartFile.absoluteFilePath();
        return;
    }
    int currentX = margin_left; // 当前 X 坐标
    // 获取图像的尺寸
    float chartImageW = imageChart.width();
    float chartImageH = imageChart.height();
    float cellRectHeight = chartImageH / chartImageW * paperWidth;
    if(currentY + cellRectHeight > paperRect.height() - margin_up){
        // 如果超出页面底部，结束当前页面并创建新页面
        printer.newPage();
        currentY = margin_up + lineSpacing; // 重置起始 y 坐标，留出标题的文本空间
    }
    QRectF cellRectChart(currentX, currentY, paperWidth,cellRectHeight); // 这里的cellHeight可以是浮点数，没有影响
    QSize targetSizeChart = cellRectChart.size().toSize();
    QImage scaledImageChart = imageChart.scaled(targetSizeChart, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 计算绘制时的目标位置，使图片居中
    QRect targetRectChart(cellRectChart.topLeft().toPoint(), scaledImageChart.size());
    targetRect.moveCenter(cellRectChart.center().toPoint());
    // 绘制缩放后的图片
    // painter.drawImage(targetRect, scaledImage);
    QRectF lineRect(margin_left, currentY - lineSpacing, paperWidth, paperHeight);
    painter.drawText(lineRect, Qt::AlignLeft, MTF_chart);
    painter.drawImage(cellRectChart,imageChart);// 绘制表格图像内容
    currentY += cellRectHeight;

    // 开始绘制透过率计算的相关数据
    // 继续基于currentY绘制后续内容
    currentY += lineSpacing;
    QStringList textLines3 = {
        "六、透过率",
        "   1.参数设置",
        "   光源波长（nm）：",
        "   参照镜头透过率：",
        "",
        "   2.测试结果",
        // "   参照镜头噪声灰度值：",
        // "   参照镜头光功率：",
        // "   测试镜头噪声灰度值：",
        // "   测试镜头光功率：",
        // "   测试镜头透过率：",
    };

    QStringList textLines3_result = {
        "",
        "",
        ui->lineEdit_wavelengthTrans->text(),
        ui->lineEdit_officialLensTrans->text(),
        "",
        "",
        // ui->lineEdit_officialNoiseLevel->text(),
        // ui->lineEdit_officialLensOpticalPower->text(),
        // ui->lineEdit_testNoiseLevel->text(),
        // ui->lineEdit_testLensOpticalPower->text(),
        // ui->lineEdit_testLensTrans->text(),
    };

    for (int i = 0 ; i < textLines3.size() ; ++i) {
        // 检查是否超出页面底部
        if (currentY + lineSpacing > paperRect.height() - margin_up) {
            // 如果超出页面底部，结束当前页面并创建新页面
            printer.newPage();
            currentY = margin_up; // 重置起始 y 坐标
        }
        QRectF lineRect(margin_left, currentY, paperWidth, paperHeight);
        // 计算附加文本的 x 坐标，紧随原始文本之后
        int textWidth = painter.fontMetrics().horizontalAdvance(textLines3[i]);
        QRectF lineRectResult(margin_left + textWidth + 8, currentY, paperWidth - textWidth - 8, paperHeight);
        painter.drawText(lineRect, Qt::AlignLeft, textLines3[i]);
        painter.drawText(lineRectResult, Qt::AlignLeft, textLines3_result[i]);

        // 计算下一行的 y 坐标
        currentY += lineSpacing; // 根据字体高度和行距动态调整
    }

    // // 开始绘制透过率表格
    // // 绘制表格头部
    // for (int row = -1; row < rowCountMTF; ++row) {
    //     int currentX = margin_left; // 当前 X 坐标
    //     bool isHeader = (row == -1); // 当row==-1时，isHeader == true，否则，isHeader == false
    //     int cellPadding = 5; // 单元格内部边距
    //     for (int col = 0; col < columnCountMTF; ++col) {
    //         QRectF cellRect(currentX, currentY, columnWidths2[col], cellHeight); // 这里的cellHeight可以是浮点数，没有影响
    //         // 绘制单元格边框
    //         painter.drawRect(cellRect);
    //         qDebug()<<"单元格宽度： "<<cellRect.size(); // 63.75
    //         // 绘制单元格内容
    //         // QString text = isHeader ? ui->tableWidget_distortion->horizontalHeaderItem(col)->text()
    //         //                         : ui->tableWidget_distortion->item(row, col)->text();
    //         // /// 语法解析：上述代码使用了三元运算符：condition ? expression_if_true : expression_if_false;
    //         // QString text;
    //         // if(isHeader){
    //         //     text= ui->tableWidget_distortion->horizontalHeaderItem(col)->text();
    //         // } // 这一段代码会导致程序崩溃

    //         QString text;
    //         // qDebug()<<ui->tableWidget_MTF->columnCount();
    //         if (isHeader) {
    //             if (col >= 0 && col < columnCountMTF) { // 确保 col 在有效范围内
    //                 QTableWidgetItem *headerItem = ui->tableWidget_MTF->horizontalHeaderItem(col);
    //                 if (headerItem) { // 确保头部项不为空
    //                     text = headerItem->text();
    //                 } else {
    //                     text = ""; // 或者设置默认值
    //                     qDebug() << "Header item for column" << col << "is not set.";
    //                 }
    //             } else {
    //                 qDebug() << "Invalid column index:" << col;
    //             }
    //         }
    //         else{
    //             QTableWidgetItem *item = ui->tableWidget_MTF->item(row, col);
    //             if(item != nullptr && !item->text().isEmpty()){
    //                 text = ui->tableWidget_MTF->item(row, col)->text();
    //             }
    //         }
    //         painter.drawText(cellRect.adjusted(cellPadding, cellPadding, -cellPadding, -cellPadding),
    //                          Qt::AlignLeft | Qt::AlignVCenter, text); // 绘制表格文本内容



    /// 抬眼望去的日光
    /// 席帘遮蔽温晴
    /// 六棱灯下着雨
    /// 浸溺一个冗长而空洞的梦
    /// 时间随光影迟暮
    /// 西风
    /// 为高楼遮挡
    /// 经半路折返
    /// 还是在初雪融化之前
    /// 弥留
    /// 身体想要撑伞
    /// 灵魂从未如此渴望狂奔
    /// 狂奔
    /// 积攒了一春的江水
    /// 想要击碎这黑的礁石
    /// 白的泡沫
    /// 绚烂的沉沦
    /// 只一个波光流转的澄澈
    /// 狂奔



    painter.end();
}

