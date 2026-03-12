#ifndef UTILS_H
#define UTILS_H
#include<QString>
#include<QDateTime>

namespace utils {  // 这里使用命名空间是为了避免在大型项目中，两个模块同时定义了相同名字的函数
    //具体意义，比如我有两个不同的模块中都想用这个函数名字，只不过函数内部的逻辑不一样，函数的用处不一样，此时适合使用命名空间
    QString getCurrentTimestamp();
    void dataToCsv(QString csvPathSupply,QString csvFileName,QString dataTitle,QStringList dataList);
}

#endif // UTILS_H
