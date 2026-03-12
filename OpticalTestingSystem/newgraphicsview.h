//自定义GraphicsView，方便图像的缩放、拖拽等操作
#ifndef NEWGRAPHICSVIEW_H
#define NEWGRAPHICSVIEW_H
#include <QGraphicsView>
#include <QWidget>

#include <QWheelEvent>
#include <QMouseEvent>

class NewGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit NewGraphicsView(QWidget *parent = nullptr); //声明构造函数
    //在头文件中，你只需声明构造函数 NewGraphicsView(QWidget *parent = nullptr)，不需要在这里定义它的实现。

    // 我需要在这个cpp文件中使用mainwindow.cpp中的pixmapItem，需要创建一个指针，然后将位于mainwindow.cpp中的
    // pixmapItem 的指针传递给位于此处的pixmapItem指针，也就是将此处的pixmapItem指针指向mainwindow.cpp中的Item
    QGraphicsPixmapItem* pixmapItem= nullptr; //这个指针只是用来引用 mainwindow.cpp 中的 pixmapItem，而不是创建一个新的实例。
    void setPixmapItem(QGraphicsPixmapItem* item) {
        pixmapItem = item;
    }

private:
    bool isDragging;  // 记录是否处于拖动状态
    QPoint dragStartPos;  // 记录拖动的起始位置
    QPointF scenePos;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void adjustSceneRect();


signals:
    void mousePositionChanged(int x,int y,QPointF scenePos); // 鼠标光标位置变化时发送的信号
    void mouseClicked(int pixelValue,QPointF scenePos); // 鼠标点击时发送的信号
};

#endif // NEWGRAPHICSVIEW_H
