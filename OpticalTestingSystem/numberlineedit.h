/*
时间：2025.3.27
作者：xxh
功能：为LineEdit单独创建一个类，用于限制lineedit的输入，例如，用户输入的内容是字母或者汉字时是无效输入。
使用：按照这种方式定义好类以后，在ui文件中右键点击一个lineedit控件，点击提升为，在下面 新建提升的类 中的 提升的类名称 中输入类名NumberLineEdit
     点击 添加。之后对于每个lineedit控件，右键点击后都可以点击 提升为->NumberLineEdit。
     这种方式可以避免需要对多个lineedit进行输入限制时重复输入代码。
*/


#ifndef NUMBERLINEEDIT_H
#define NUMBERLINEEDIT_H

#include <QObject>
#include <QLineEdit>

class NumberLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit NumberLineEdit(QWidget *parent = nullptr);
    void setup(int maxLength = 10, int decimals = 5);
};

#endif // NUMBERLINEEDIT_H
