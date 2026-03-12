#ifndef CHARTVIEWNEW_H
#define CHARTVIEWNEW_H

#include <QChartView>  // ChartViewNew 继承自 QChartView，因此需要包含它


class MainWindow; // 前向声明

class ChartViewNew : public QChartView {
    Q_OBJECT

public:
    explicit ChartViewNew(MainWindow *mainWindow, QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    MainWindow *mainWindow;
};

#endif // CHARTVIEWNEW_H
