#include "mainwindow.h"
#include "loadfileqss.h"

#include <QApplication>
#include <QDebug>
#include<QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 禁用警告级别的调试输出
    QLoggingCategory::setFilterRules("*.warning=false");

    // 设置工作目录为 main.cpp 所在的目录
    QDir::setCurrent(QCoreApplication::applicationDirPath() + "/../../../");
    QApplication::setStyle("Fusion"); // 启用更接近win11的主题
    LoadFileQss::setStyle(":/Icon/QssFiles/mainQss.qss");

    MainWindow w;

    w.show();
    return a.exec();
}
