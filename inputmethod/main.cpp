#include "im.h"
#include <QtGui>
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("GB2312");//这种GB2312很方便的实现中文短信发送
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
    QApplication a(argc, argv);
    
    QFile file(":/qss/inputmethod.qss");//qss文件实现漂亮的皮肤
    file.open(QFile::ReadOnly);    
    QString styleSheet=QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);
    
    IMFrame *im=new IMFrame();
    QWSServer::setCurrentInputMethod(im);
    
    return a.exec();
}
