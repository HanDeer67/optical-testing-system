#include "chartviewnew.h"
#include "mainwindow.h"

ChartViewNew::ChartViewNew(MainWindow *mainWindow, QWidget *parent)
    : QChartView(parent), mainWindow(mainWindow){

}

void ChartViewNew::mouseMoveEvent(QMouseEvent *event) {
    QPointF mousePos = event->pos();
    QPointF chartPos = chart()->mapToValue(mousePos);

    // 调用 MainWindow 的 onPointHovered 函数，并传入 chartPos
    if (mainWindow) {
        mainWindow->onPointHovered(chartPos, true);
    }
    QChartView::mouseMoveEvent(event);  // 保留默认行为
}
