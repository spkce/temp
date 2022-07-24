#include "im.h"
#include <QtGui>
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("GB2312");//����GB2312�ܷ����ʵ�����Ķ��ŷ���
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
    QApplication a(argc, argv);
    
    QFile file(":/qss/inputmethod.qss");//qss�ļ�ʵ��Ư����Ƥ��
    file.open(QFile::ReadOnly);    
    QString styleSheet=QLatin1String(file.readAll());
    a.setStyleSheet(styleSheet);
    
    IMFrame *im=new IMFrame();
    QWSServer::setCurrentInputMethod(im);
    
    return a.exec();
}
