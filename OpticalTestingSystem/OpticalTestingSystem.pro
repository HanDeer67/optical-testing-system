QT       += core gui multimedia multimediawidgets charts
QT       += printsupport
# 外部库
include(QXlsx/QXlsx.pri)
# 如果只需要相机功能并不需要在 UI 中显示摄像头的实时视频流，那么你只需要 multimedia 模块。
# 如果需要将摄像头的实时视频显示在界面上（例如，显示在 QGraphicsView 中），就需要同时使用 multimedia 和 multimediawidgets。

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# UTF-8 编码设置，如果没有下面两行，会弹出上万条字符偏移警告
QMAKE_CXXFLAGS += /utf-8
CONFIG += utf8_source

#######################海康机器人相机库文件######################
# # Add include path
# INCLUDEPATH += $$PWD/Includes  # 添加头文件搜索路径
# # Add library path
# LIBS += -L$$PWD/Libraries/win64 -lMvCameraControl # 添加库文件路径和链接库

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Grayscale_centroid.cpp \
    baslercameracontroller.cpp \
    beifen.cpp \
    chartsdialog.cpp \
    chartviewnew.cpp \
    dialog_def_pix_size.cpp \
    image_Thread.cpp \
    # hikcameracontroller.cpp \# 海康机器人相机控制类
    # hikparasetdialog.cpp \ # 海康机器人相机对话框
    imageprocessthread.cpp \
    main.cpp \
    mainwindow.cpp \
    newdockwidgettitlebar.cpp \
    newgraphicsview.cpp \
    newgrayscalecentroid.cpp \
    numberlineedit.cpp \
    saperacameracontroller.cpp \
    utils.cpp

HEADERS += \
    Grayscale_centroid.h \
    baslercameracontroller.h \
    chartsdialog.h \
    chartviewnew.h \
    dialog_def_pix_size.h \
    image_Thread.h \
    # hikcameracontroller.h \
    # hikparasetdialog.h \
    imageprocessthread.h \
    loadfileqss.h \
    mainwindow.h \
    newdockwidgettitlebar.h \
    newgraphicsview.h \
    newgrayscalecentroid.h \
    numberlineedit.h \
    saperacameracontroller.h \
    utils.h

FORMS += \
    chartsdialog.ui \
    dialog_def_pix_size.ui \
    hikparasetdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


# win32:CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc16/release/ -lopencv_world4100d
# else:win32:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc16/debug/ -lopencv_world4100d
# win32:CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world4100
# else:win32:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world4100d
# else:unix: LIBS += -LD:/opencv/build/x64/vc16/ -lopencv_world4100d

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/release/libopencv_world4100d.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/debug/libopencv_world4100d.a

# INCLUDEPATH += D:/opencv/build/include
# DEPENDPATH += D:/opencv/build/include
# INCLUDEPATH += D:\opencv-4.10.0\opencv\build\include
# DEPENDPATH += D:\opencv-4.10.0\opencv\build\include

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/release/libopencv_world4100d.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/debug/libopencv_world4100d.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/release/opencv_world4100d.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += D:/opencv/build/x64/vc16/debug/opencv_world4100d.lib
# else:unix: PRE_TARGETDEPS += D:/opencv/build/x64/vc16/libopencv_world4100d.a

DISTFILES +=

RESOURCES += \
    resource.qrc





# 添加这行来打印环境变量值
message(">>> 1调试信息：当前 PYLON_DEV_DIR 环境变量 = $$(PYLON_DEV_DIR) <<<")
# Add Pylon
win32 {
    INCLUDEPATH += "$$(PYLON_DEV_DIR)include" # 注意，当环境变量的路径无法识别时就打印出来看看问题出在哪，这里曾出现的问题是在环境变量路径和include之间有两个斜杠
    INCLUDEPATH += "$$(PYLON_DEV_DIR)include/pylon"
    message(">>> 2调试信息：当前 PYLON_DEV_DIR 环境变量 = $$(PYLON_DEV_DIR)include <<<")
    # INCLUDEPATH += "D:/APPs/pylon8/Development/include"
    message(">>> 3调试信息：当前 PYLON_DEV_DIR 环境变量 = D:/APPs/pylon8/Development/include <<<")
    # INCLUDEPATH += "D:/APPs/pylon8/Development/include/pylon"

    contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L"$$(PYLON_DEV_DIR)/lib/x64"
        # LIBS += -L"D:/APPs/pylon8/Development/lib/x64"
    } else {
        LIBS += -L"$$(PYLON_DEV_DIR)/lib/win32"
        # LIBS += -L"D:/APPs/pylon8/Development/lib/Win32"
    }
}

# 添加这行来打印环境变量值
message(">>> 调试信息：当前 SAPERADIR 环境变量 = $$(SAPERADIR) <<<")
# 添加 Sapera SDK 路径，请根据您的实际安装路径进行修改
# SAPERA_PATH = "C:/Program Files/Teledyne DALSA/Sapera"
INCLUDEPATH += "$$(SAPERADIR)\Include"
INCLUDEPATH += "$$(SAPERADIR)\Classes\Basic"
message(">>> 调试信息：当前Include路径 = $$(SAPERADIR)\Include <<<")

# 添加 Sapera 库文件
LIBS += -L"$$(SAPERADIR)\Lib\Win64" -lSapClassBasic
LIBS += -L"$$(SAPERADIR)\Lib\Win64" -lcorapi
LIBS += -L"$$(SAPERADIR)\Lib\Win64" -lcorppapi


# ①航星台式机opencv路径
win32:CONFIG(release, debug|release): LIBS += -LD:/opencv-4.10.0/opencv/build/x64/vc16/lib/ -lopencv_world4100
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/opencv-4.10.0/opencv/build/x64/vc16/lib/ -lopencv_world4100d

INCLUDEPATH += D:/opencv-4.10.0/opencv/build/include
DEPENDPATH += D:/opencv-4.10.0/opencv/build/include

# ②航星笔记本opencv路径
# win32:CONFIG(release, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world4100
# else:win32:CONFIG(debug, debug|release): LIBS += -LD:/opencv/build/x64/vc16/lib/ -lopencv_world4100d

# INCLUDEPATH += D:/opencv/build/include
# DEPENDPATH += D:/opencv/build/include

# ③工控机opencv路径
# win32:CONFIG(release, debug|release): LIBS += -LD:/app/opencv/opencv/build/x64/vc16/lib/ -lopencv_world4100
# else:win32:CONFIG(debug, debug|release): LIBS += -LD:/app/opencv/opencv/build/x64/vc16/lib/ -lopencv_world4100d

# INCLUDEPATH += D:/app/opencv/opencv/build/include
# DEPENDPATH += D:/app/opencv/opencv/build/include

