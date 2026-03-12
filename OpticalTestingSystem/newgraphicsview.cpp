#include "newgraphicsview.h"
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QColor>
#include <QScrollBar>

NewGraphicsView::NewGraphicsView(QWidget *parent)
    : QGraphicsView{parent},isDragging(false)
{
    setMouseTracking(true); // 启用鼠标追踪，用于实时捕获当前光标位置

    // setDragMode(QGraphicsView::ScrollHandDrag);  // 设置鼠标拖动模式：当有这行代码时，鼠标移动过去图像就开始移动，根本不用手动抓取图像
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // 缩放时以鼠标为中心
    // setSceneRect(-700, -500, 2200, 1500);  // 设置一个大的场景范围，注意，这句代码如果没有的话，图像拖不动，但是这个范围设置很容易不合适，比如图像超出图像框

    setDragMode(QGraphicsView::NoDrag);  // 禁用内置的拖动模式
    setInteractive(true);  // 确保自定义拖动生效
    // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 可选，禁用滚动条
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);    // 可选，禁用滚动条

    // setRenderHint(QPainter::Antialiasing);  // 提升渲染质量
} //在源文件中提供构造函数的实现。

// 实现其他事件处理函数    完全禁用自动场景范围调整
void NewGraphicsView::wheelEvent(QWheelEvent *event) {
    const double scaleFactor = 1.15;

    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);  // 放大
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);  // 缩小
    }
    // 缩放之后，调整场景的范围
    // adjustSceneRect();
    // 添加以下代码强制扩展场景范围   // 完全禁用自动调整，保持固定的大场景范围
    QRectF currentRect = sceneRect();
    setSceneRect(currentRect.adjusted(-2000, -2000, 2000, 2000));
}

// void NewGraphicsView::mouseMoveEvent(QMouseEvent *event) {
//     QGraphicsView::mouseMoveEvent(event);
//     // 注意：event 是一个指向 QMouseEvent 对象的指针，它包含与鼠标事件相关的信息（比如鼠标点击、移动、释放等）
//     // QPointF viewPos = event->pos(); //获取当前视图坐标
//     // 获取当前视图坐标,并将视图坐标转为场景坐标
//     scenePos = mapToScene(event->pos());
//     //发射鼠标坐标更新的信号，随信号附带坐标
//     emit mousePositionChanged(static_cast<int>(scenePos.x()),static_cast<int>(scenePos.y()),scenePos);

//     if (isDragging) {
//         QPointF delta = mapToScene(event->pos()) - mapToScene(dragStartPos);
//         qreal currentScale = transform().m11();
//         // 修改为不除以缩放比例，保持拖动灵敏度
//         translate(delta.x(), delta.y());
//         dragStartPos = event->pos();
//         update();
//     }
//     // else {
//     //     QGraphicsView::mouseMoveEvent(event);
//     // }
// }

void NewGraphicsView::mouseMoveEvent(QMouseEvent *event) {
    QGraphicsView::mouseMoveEvent(event);
    scenePos = mapToScene(event->pos());
    emit mousePositionChanged(static_cast<int>(scenePos.x()), static_cast<int>(scenePos.y()), scenePos);

    if (isDragging) {
        // 使用滚动条实现更平滑的拖动
        QPoint delta = event->pos() - dragStartPos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        dragStartPos = event->pos();
    }
}

// void NewGraphicsView::resizeEvent(QResizeEvent *event) {
//     QGraphicsView::resizeEvent(event);  // 调用父类的事件处理

//     // 获取当前窗口的大小
//     QSize viewportSize = viewport()->size();
//     // qDebug()<<viewportSize;

//     // 当前计算方式为窗口大小的800%，建议改为更大的倍数
//     qreal sceneWidth = viewportSize.width() * 8;  // 从 2 改为 8
//     qreal sceneHeight = viewportSize.height() * 8; // 从 2 改为 8
//     // qDebug()<<sceneWidth;
//     // qDebug()<<sceneHeight;

//     // 设置场景范围比窗口大 50%
//     // setSceneRect(-sceneWidth / 4, -sceneHeight / 4, sceneWidth, sceneHeight);
//     // setSceneRect(-sceneWidth / 2, -sceneHeight / 2, sceneWidth, sceneHeight);
//     setSceneRect(-5000, -5000, 10000, 10000);

// }

void NewGraphicsView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);

    // 合理的场景范围设置（当前视口的2倍）
    QSize viewSize = size();
    qreal sceneWidth = viewSize.width() * 2;
    qreal sceneHeight = viewSize.height() * 2;
    setSceneRect(-sceneWidth/2, -sceneHeight/2, sceneWidth, sceneHeight);
}

// void NewGraphicsView::wheelEvent(QWheelEvent *event) {
//     const double scaleFactor = 1.15;
//     // ...原有缩放逻辑...

//     // 不再强制扩展超大场景范围
//     // 改为根据实际内容调整
//     adjustSceneRect();
// }

void NewGraphicsView::adjustSceneRect() {
    qreal scenePadding = 500;  // 设置适当的边距，确保有足够的拖动空间

    // 调整场景范围
    QRectF newSceneRect = scene()->itemsBoundingRect().adjusted(-scenePadding, -scenePadding, scenePadding, scenePadding);
    setSceneRect(newSceneRect);
}


// void NewGraphicsView::mousePressEvent(QMouseEvent *event) {
//     QGraphicsView::mousePressEvent(event);
//     if (event->button() == Qt::LeftButton) {
//         isDragging = true;
//         dragStartPos = event->pos();
//         setCursor(Qt::ClosedHandCursor);
//     }

//     // 添加鼠标点击事件：点击图像时显示当前像素的灰度值
//     QPointF clickedPos = mapToScene(event->pos()); //这里的pos()就是当前鼠标点击的位置
//     // 此时pixmapItem并不是灰度格式的
//     if(pixmapItem && pixmapItem->contains(pixmapItem->mapFromScene(scenePos))){
//         // 获取点击像素灰度值
//         QImage tempImg = pixmapItem->pixmap().toImage();
//         // 使用floor确保精确到整数像素边界，避免浮点误差
//         QPoint pixelPos(floor(clickedPos.x()), floor(clickedPos.y())); /// 这一句很关键，如果没有的话，放大像素块后像素值会显示错位，比如一个像素块的右侧像素值显示的其实是其右侧像素块的像素值
//         int grayValue = qGray(tempImg.pixel(pixelPos));
//         // qDebug()<<"grayValue:"<<grayValue;
//         emit mouseClicked(grayValue,scenePos);
//         //将灰度值显示在状态框中——发射信号，信号带有grayValue的值
//     }
//     // QGraphicsView::mousePressEvent(event);

// }

void NewGraphicsView::mousePressEvent(QMouseEvent *event) {
    QGraphicsView::mousePressEvent(event);
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        dragStartPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }

    // 更精确的像素坐标计算
    QPointF scenePos = mapToScene(event->pos());
    QPointF itemPos = pixmapItem->mapFromScene(scenePos);

    if (pixmapItem->contains(itemPos)) {
        QRectF pixelRect = pixmapItem->boundingRect();
        // 确保坐标在图像范围内
        int x = qBound(0, static_cast<int>(itemPos.x()), static_cast<int>(pixelRect.width()-1));
        int y = qBound(0, static_cast<int>(itemPos.y()), static_cast<int>(pixelRect.height()-1));

        QImage tempImg = pixmapItem->pixmap().toImage();
        int grayValue = qGray(tempImg.pixel(x, y));
        emit mouseClicked(grayValue, scenePos);
    }
}

// void NewGraphicsView::mousePressEvent(QMouseEvent *event) {
//     QGraphicsView::mousePressEvent(event);
//     if (event->button() == Qt::LeftButton) {
//         isDragging = true;
//         dragStartPos = event->pos();
//         setCursor(Qt::ClosedHandCursor);
//     }

//     // 添加鼠标点击事件：点击图像时显示当前像素的灰度值
//     QPointF scenePos = mapToScene(event->pos()); // 将视图坐标转为场景坐标

//     if(pixmapItem->contains(pixmapItem->mapFromScene(scenePos))){
//         // 获取点击像素灰度值
//         QImage tempImg = pixmapItem->pixmap().toImage();

//         // 将场景坐标转换为pixmapItem内部的坐标
//         QPointF itemPos = pixmapItem->mapFromScene(scenePos);

//         // 使用floor确保精确到整数像素边界，避免浮点误差
//         QPoint pixelPos(floor(itemPos.x()), floor(itemPos.y()));

//         // 确保坐标在图像范围内
//         pixelPos.setX(qBound(0, pixelPos.x(), tempImg.width() - 1));
//         pixelPos.setY(qBound(0, pixelPos.y(), tempImg.height() - 1));

//         // 获取正确的像素位置
//         int grayValue = qGray(tempImg.pixel(pixelPos));

//         emit mouseClicked(grayValue, scenePos);
//     }
// }


void NewGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
    QGraphicsView::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        setCursor(Qt::ArrowCursor);
    }

}
