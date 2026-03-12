#ifndef HIKPARASETDIALOG_H
#define HIKPARASETDIALOG_H

#include <QDialog>

namespace Ui {
class hikParaSetDialog;
}

class hikParaSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit hikParaSetDialog(QWidget *parent = nullptr);
    ~hikParaSetDialog();

signals:
    void cameraParametersChanged(float expoTime, float gain, float frameRate);

private:
    Ui::hikParaSetDialog *ui;


};

#endif // HIKPARASETDIALOG_H
