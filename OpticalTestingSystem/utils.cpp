#include "utils.h"
#include <QDir>
#include <QApplication>

namespace utils {
    QString getCurrentTimestamp(){
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    }

    void dataToCsv(QString csvPathSupply,QString csvFileName,QString dataTitle,QStringList dataList){
        // 将上述数据存储进excel中的一行
        // 1. 检查当前软件所处路径中是否有名为DataExport的文件夹，没有则创建一个
        // 2. 检查DataExport是否有名为FOV的文件夹，没有则创建一个
        // 3. 检查FOV中是否有名为FOV.csv的文件，没有则创建一个
        // 4. 在FOV.csv中创建以下几列的标题：时间戳、上极、下极、左极、右极、水平视场角、俯仰视场角
        // 5. 将上面的数据存进FOV.csv中的最新一行，并不覆盖之前的数据
        QString appDir = QCoreApplication::applicationDirPath();
        QString fovFolderPath = appDir + csvPathSupply;
        QDir dir(fovFolderPath);
        if(!dir.exists()){
            // dir.mkdir("."); // mkdir只能创建指定目录中的单一级目录
            dir.mkpath(fovFolderPath); // 会递归创建路径中的所有缺失部分
        }
        QString csvFilePath = fovFolderPath + csvFileName;
        QFile csvFile(csvFilePath);
        QTextStream out(&csvFile);
        if(!csvFile.exists()){
            csvFile.open(QIODevice::ReadWrite);

            /// 写入 UTF-8 BOM 以确保编码被正确识别，如果没有下面两行，生成的csv文件的标题将是乱码
            out.setGenerateByteOrderMark(true); // ☆☆☆ UTF-8 BOM 是一个特殊的字符标记（0xEF,0xBB,0xBF），可以帮助某些软件识别文件为 UTF-8 编码格式  ☆☆☆
            out.setEncoding(QStringConverter::Utf8);  // 设置编码为 UTF-8

            // 写入标题作为首行
            out << dataTitle;

            csvFile.close();
        }
        // 打开文件并设置为追加模式
        if(csvFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            // 写入 CSV 文件（标题仅在文件为空时写入）
            // 写入数据，添加到文件末尾
            out << dataList.join(",") << "\n";
            csvFile.close();
        }else{
            /// 待解决。。。
            QString errorTemp = "无法打开CSV文件进行写入，请检查文件是否被占用";
            // emit messageBoxSignal(errorTemp); // 注意，没有类的cpp文件中函数是无法发射信号的
        }
    }
}

