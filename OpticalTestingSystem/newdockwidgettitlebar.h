#ifndef NEWDOCKWIDGETTITLEBAR_H
#define NEWDOCKWIDGETTITLEBAR_H

#include<QDockWidget>
#include<QWidget>
#include <QObject>

class newDockWidgetTitleBar: public QObject
{
    Q_OBJECT  // 启用信号与槽机制
    /// 笔记：事实上，一个普通的子类，如果想使用槽函数等qt类才具有的功能时，在这里继承QObject即可，然后启用信号与槽机制
public:
    static void newDockBar(QDockWidget *dockWidget , const QString &titleText);
    // 笔记1：这里加入static是定义该函数是一个静态方法，当该函数是静态方法时，在别的cpp文件中调用该函数时可以以静态方式调用
    // 静态方法调用：类名：：方法（）
    // 如果这里不加static，函数就不是静态函数，如果在别的cpp文件调用，需要先创建对象，通过对象调用该非静态函数

    // 笔记2：这里加入const，这样可以接受字符串常量、QString对象等各种字符串类型，并避免额外的复制开销
    // 如果不加，setCustomTitleBar 中参数类型是 QString&（引用类型），但在调用时传入的是一个字符串常量 const char[16]（比如 "主参数面板"），这两者不兼容

};

#endif // NEWDOCKWIDGETTITLEBAR_H
