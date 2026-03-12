#include "chartsdialog.h"
#include "ui_chartsdialog.h"
#include<QDir>
#include<QDateTime>
// # include"mainwindow.h"
#include"utils.h"


ChartsDialog::ChartsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChartsDialog)
{
    ui->setupUi(this);
    // 设置槽函数，当点击按钮时，保存chart图像
    connect(ui->downChartButton,&QPushButton::clicked,this,&ChartsDialog::saveChartImage);
}

ChartsDialog::~ChartsDialog()
{
    delete ui;
}


void ChartsDialog::addWidgetToLayout(QWidget *widget) {
    // ui->groupBox->addWidget(widget);
    ui->verticalLayout_2->addWidget(widget);  // 将 widget 添加到 verticalLayout
}

// void ChartsDialog::closeEvent(QCloseEvent *event) {
//     qDebug()<<"清理子窗口";
//     QLayoutItem *item;
//     // 清除布局中的所有控件，避免下次显示时出现重复叠加
//     while ((item = layout()->takeAt(0)) != nullptr) {
//         delete item->widget();
//         delete item;
//     }
//     QDialog::closeEvent(event);  // 调用基类的关闭事件处理
// }

void ChartsDialog::closeEvent(QCloseEvent *event) {
    // 假设 chartView 是显示图表的视图控件指针
    qDebug()<<"清理子窗口";
    emit dialogClosed();  // 发出关闭信号
    QDialog::closeEvent(event);
}


// void ChartsDialog::showEvent(QShowEvent *event) {
//     qDebug()<<"展示子窗口";
//     emit showDialog();  // 发出关闭信号
//     QDialog::showEvent(event);
// }

// 在 ChartDialog 类中添加一个公共方法  注意：后续发现在对话框自带的buttonbox中加入新的按钮不是一个好的选择，可编辑性不强
QPushButton* ChartsDialog::getButton() const {
    return ui->downChartButton;
}

void ChartsDialog::saveChartImage(){
    // 保存当前chartView中的内容到图像
    QString chartImageFolder = "/chartImages";
    emit saveChartSignal(chartImageFolder);
}

