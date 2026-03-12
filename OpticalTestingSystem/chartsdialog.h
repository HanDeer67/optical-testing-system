#ifndef CHARTSDIALOG_H
#define CHARTSDIALOG_H

#include <QDialog>
#include <QGraphicsEllipseItem>
#include <QDialogButtonBox>
# include<QDir>


namespace Ui {
class ChartsDialog;
}

class ChartsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChartsDialog(QWidget *parent = nullptr);
    ~ChartsDialog();

    void addWidgetToLayout(QWidget *widget);  // 添加这个方法
    QPushButton* getButton() const; // 由于UI的对象是在私有区域，所以如果需要访问其中的控件，需要设置一个访问接口

private:
    Ui::ChartsDialog *ui;
    void closeEvent(QCloseEvent *event) override;
    void saveChartImage();
    // void showEvent(QShowEvent *event) override;

signals:
    void dialogClosed();
    // void showDialog();
    void messagePrint(QString message);
    void saveChartSignal(QString chartImageFolderPath);


};

#endif // CHARTSDIALOG_H
