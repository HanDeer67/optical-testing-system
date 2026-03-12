#include "newdockwidgettitlebar.h"
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QIcon>
#include<QLoggingCategory>
#include "loadfileqss.h"
#include <QDir>

void newDockWidgetTitleBar::newDockBar(QDockWidget *dockWidget , const QString &titleText){
    // 创建自定义标题栏
    QWidget *customTitleBar = new QWidget(dockWidget);
    customTitleBar->setStyleSheet("background-color: rgb(192,220,245); font: bold 14px; color: black;");
    // customTitleBar->setStyleSheet("background-color: #cfe7fa; font: bold 14px; color: black;");
    customTitleBar->setFixedHeight(27); // 设置标题栏高度

    // 标题文本
    QLabel *titleLabel = new QLabel(titleText, customTitleBar);
    titleLabel->setStyleSheet("margin-left: 10px;");

    // 悬浮按钮
    QToolButton *floatButton = new QToolButton(customTitleBar);
    // floatButton->setIcon(QIcon::fromTheme("window-restore")); // 使用图标主题  失效
    QIcon icon = QIcon(":/Icon/Icon_images/button-restore.png"); // 设置自定义图片为icon
    floatButton->setIcon(icon);
    floatButton->setFixedSize(25, 25); // 设置按钮宽25像素，高25像素
    floatButton->setIconSize(QSize(15, 15));
    floatButton->setStyleSheet("margin-right: 5px;");

    // 关闭按钮
    QToolButton *closeButton = new QToolButton(customTitleBar);
    closeButton->setIcon(QIcon::fromTheme("window-close"));
    closeButton->setFixedSize(25, 25); //
    closeButton->setIconSize(QSize(15, 15));
    closeButton->setStyleSheet("margin-right: 5px;");
    //                            "border: 1px solid lightblue;"
    //                             "background: white;"
    //                             "padding: 2px;"
    //                             "border-radius: 5px;");
    // 按钮点击事件
    // connect(floatButton, &QToolButton::clicked, [=]() {
    //     if (floatButton->isChecked()) {
    //         qDebug()<<"12334";
    //         // 如果按钮已经选中，恢复原背景
    //         floatButton->setChecked(false); // 重置为未选中状态
    //     } else {
    //         qDebug()<<"78945";
    //         // 如果按钮未选中，改变按钮状态
    //         // floatButton->setChecked(false); // 设置为选中状态
    //         // 设置按钮的颜色恢复为浅蓝色，且没有边框显示
    //     }
    // });

    /// 直接设计style
    // floatButton->setStyleSheet("color: black; border: 1px solid lightblue; background: white; padding: 2px; border-radius: 5px;");
    // closeButton->setStyleSheet("color: black; border: 1px solid lightblue; background: white; padding: 2px; border-radius: 5px;");
    /// 加载qss文件的内容
    QString qssFilePath = QCoreApplication::applicationDirPath() + "/../../../QssFiles/dockTitleQss.qss";
    QFile qssFile(qssFilePath);
    if (qssFile.open(QFile::ReadOnly)) {
        QString qssContent = QLatin1String(qssFile.readAll());
        floatButton->setStyleSheet(qssContent);
        closeButton->setStyleSheet(qssContent);
        qssFile.close();
    } else {
        qDebug() << "Failed to load QSS file:" << qssFilePath;
    }

    // 动态创建控件后，重新应用全局样式
    // qApp->setStyleSheet(qApp->styleSheet());

    // // 设置工作目录为 main.cpp 所在的目录
    // QDir::setCurrent(QCoreApplication::applicationDirPath() + "/../../../");
    // LoadFileQss::setStyle("QssFiles/mainQss.qss");

    // 布局设置
    QHBoxLayout *layout = new QHBoxLayout(customTitleBar);
    layout->addWidget(titleLabel);
    layout->addStretch(); // 添加弹性空间，按钮靠右
    layout->addWidget(floatButton);
    layout->addWidget(closeButton);
    layout->setContentsMargins(0, 0, 0, 0);
    customTitleBar->setLayout(layout);

    // 设置标题栏
    dockWidget->setTitleBarWidget(customTitleBar);

    // 悬浮和关闭按钮的行为
    QObject::connect(floatButton, &QToolButton::clicked, [dockWidget]() {
        dockWidget->setFloating(!dockWidget->isFloating());
    });
    QObject::connect(closeButton, &QToolButton::clicked, [dockWidget]() {
        dockWidget->close();
    });
};
