
#include "keyboard.h"
#include <QWSInputMethod>
#include <QSignalMapper>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QTableView>
#include <QSqlRecord>
#include <QCopChannel>

class IMFrame:public QWSInputMethod
{
Q_OBJECT

public:
        IMFrame();
        ~IMFrame();
	void showKeyboard(bool flag);

private:
        Keyboard *kb;       /*virtual keyboard*/
        QSignalMapper *mapper;
	QSqlDatabase db;

        bool shift; 
        bool num;   
        bool chn;  

        int pageCount;
        int currentPage;

        QString chinese[128];
        QSqlTableModel *model;

	QCopChannel *channel;
        void setMapping();
        void setConnect();
        void listChn();

private slots:
        void sendContent(const QString&);
        void setShift();
        void setChaNum();
        void setEngChn();
        void transPy(const QString &text);
        void turnLeft();
        void turnRight();

        void handleMessageFromQCop(const QString & message, const QByteArray & data);		
};
