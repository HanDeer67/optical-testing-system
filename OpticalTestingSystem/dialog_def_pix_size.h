#ifndef DIALOG_DEF_PIX_SIZE_H
#define DIALOG_DEF_PIX_SIZE_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    QVector<double> getLineEditText() const;
    Ui::Dialog *ui;

private:


};

#endif // DIALOG_DEF_PIX_SIZE_H
