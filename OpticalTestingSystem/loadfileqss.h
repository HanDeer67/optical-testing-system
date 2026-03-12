#ifndef LOADFILEQSS_H
#define LOADFILEQSS_H

#include<QApplication>
#include<QFile>


class LoadFileQss
{
public:
    static void setStyle(const QString& filename){
        // static 使得 setStyle 可以直接通过类名调用，适合全局的、与实例无关的操作。
        // const 确保了 fileName 在函数内部不被修改，增强了安全性和性能。
        QFile qssFile(filename);
        if (!qssFile.open(QFile::ReadOnly)) {
            qDebug() << "Failed to open QSS file:" << filename;
            return;
        }
        qssFile.open(QFile::ReadOnly);
        qApp->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }
};



#endif // LOADFILEQSS_H
