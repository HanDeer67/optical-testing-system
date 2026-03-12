#include "dialog_def_pix_size.h"
#include "ui_dialog_def_pix_size.h"
#include<QVector>
#include<QFile>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //单独设置对话框中groupbox的样式
    // this->setStyleSheet("background-color:red");
    QFile file("QssFiles/dialogQss.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&file);
        QString style = ts.readAll();
        this->setStyleSheet(style);
    }

}

Dialog::~Dialog()
{
    delete ui;
}

// 获取 QLineEdit 的文本
QVector<double> Dialog::getLineEditText() const
{

    QVector<double> pixelSizeList;
    // 执行一些初始化或填充操作
    pixelSizeList.append(ui->lineEdit_diaKjg->text().toDouble());
    pixelSizeList.append(ui->lineEdit_diaJhw->text().toDouble());
    pixelSizeList.append(ui->lineEdit_diaDbhw->text().toDouble());
    pixelSizeList.append(ui->lineEdit_diaZbhw->text().toDouble());
    pixelSizeList.append(ui->lineEdit_diaCbhw->text().toDouble());

    return pixelSizeList;  // 假设您的 QLineEdit 名为 lineEdit
}
