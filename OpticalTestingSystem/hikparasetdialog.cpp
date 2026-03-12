#include "hikparasetdialog.h"
#include "ui_hikparasetdialog.h"
#include <QPushButton>
#include "hikcameracontroller.h"


hikParaSetDialog::hikParaSetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::hikParaSetDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("确定");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
    ui->buttonBox->button(QDialogButtonBox::Ok)->setMinimumWidth(60);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setMinimumWidth(60);

    // 点击确定按钮时，将当前设置的相机参数发送给相机
    connect(ui->buttonBox->button(QDialogButtonBox::Ok),&QPushButton::clicked,this,[=](){
        float expoTime = ui->lineEdit_exposureTime->text().toFloat();
        float gain = ui->lineEdit_gain->text().toFloat();
        float frameRate = ui->lineEdit_frameRate->text().toFloat();
        emit cameraParametersChanged(expoTime,gain,frameRate);
    });

}

hikParaSetDialog::~hikParaSetDialog()
{
    delete ui;
}
