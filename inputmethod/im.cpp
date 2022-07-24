#include "im.h"
#include <QMessageBox>
#include <QCopChannel>
IMFrame::IMFrame()
{
    kb = new Keyboard();

    mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(QString)), this, SLOT(sendContent(QString)));

    shift = num = chn = false;
    pageCount = currentPage = 0;

    int i;
    for (i = 0; i < 128; i++)
        chinese[i] = "";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./data.db");
    if (!db.open())
        QMessageBox::information(0, tr("中文字库date.db无法打开"), tr("请查看相应文件的路经"));

    model = new QSqlTableModel();
    model->setTable("pingyin");
    model->select();

    channel = new QCopChannel("InputMethod", this);
    connect(channel, SIGNAL(received(QString,QByteArray)), this, SLOT(handleMessageFromQCop(QString,QByteArray)));

    setMapping();
    setConnect();
}

IMFrame::~IMFrame()
{
}

void IMFrame::sendContent(const QString &content)
{
    if (content == "GO")
    {
        QWSServer::sendKeyEvent(0x01000005, Qt::Key_Enter, Qt::NoModifier, true, false);
    }
    else if (content == "DEL")
    {
        if (!chn || num || (kb->chnPanel->isVisible()&&kb->chnPanel->chnLineEdit->text().isEmpty())) {
            QWSServer::sendKeyEvent(0x01000003, Qt::Key_Backspace, Qt::NoModifier, true, false);
        }
        else {
            QString temp = kb->chnPanel->chnLineEdit->text();
            temp.chop(1);
            kb->chnPanel->chnLineEdit->setText(temp);
        }
    }
    else if (content == "SPACE") {
        if (!chn || num) {
            sendCommitString(content);
        }
        else {
            sendCommitString(kb->chnPanel->chnLabel[0]->text());
            kb->chnPanel->chnLineEdit->setText("");
        }
    }
    else if (content == "SFT") {
        setShift();
    }
    else if (content == "CLR") {
        kb->chnPanel->chnLineEdit->setText("");
    }
    else if (content == "CHN" || content == "ENG") {
        setEngChn();
    }
    else if (content == ".?123" || content == "abc") {
        setChaNum();
    }
    else {
        if (!chn || num || content=="," || content==".") {
	    QString temp=content;
            if (temp == "&&") {
		temp.chop(1);
	    }
            if (!shift) {
                temp = temp.toLower();
            }
            sendCommitString(temp);
        }
        else {
            int i;
            bool isChn = true;
            for (i = 0; i < 26; i++) {
                if (content == english[i]) {
                    isChn = false;
                    break;
                }
            }
            if (isChn) {
                sendCommitString(content);
                kb->chnPanel->chnLineEdit->setText("");
            }
            else {
                QString temp = kb->chnPanel->chnLineEdit->text();
                kb->chnPanel->chnLineEdit->setText(temp+content.toLower());
            }
        }
    }
}

void IMFrame::showKeyboard(bool flag)
{
    kb->setVisible(flag);
}

void IMFrame::setMapping()
{
    int i;
    for (i = 0; i < 26; i++)
        mapper->setMapping(kb->key[i],kb->key[i]->text());

    mapper->setMapping(kb->keyShift, kb->keyShift->text());
    mapper->setMapping(kb->keyDelete, kb->keyDelete->text());
    mapper->setMapping(kb->keyComma, ",");
    mapper->setMapping(kb->keyPeriod, ".");
    mapper->setMapping(kb->keySpace, " ");
    mapper->setMapping(kb->keyGo, kb->keyGo->text());
}

void IMFrame::setConnect()
{
    int i;
    for (i = 0; i < 26; i++)
        connect(kb->key[i], SIGNAL(clicked()), mapper, SLOT(map()));

    connect(kb->keyShift, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(kb->keyChaNum, SIGNAL(clicked()), this, SLOT(setChaNum()));
    connect(kb->keyEngChn, SIGNAL(clicked()), this, SLOT(setEngChn()));
    connect(kb->keyComma, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(kb->keyPeriod, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(kb->keyDelete, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(kb->keySpace, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(kb->keyGo, SIGNAL(clicked()), mapper, SLOT(map()));

    connect(kb->chnPanel->chnLineEdit, SIGNAL(textChanged(QString)), this, SLOT(transPy(QString)));
    connect(kb->chnPanel->btnLeft, SIGNAL(clicked()), this, SLOT(turnLeft()));
    connect(kb->chnPanel->btnRight, SIGNAL(clicked()), this, SLOT(turnRight()));

    for (i = 0; i < 5; i++)
        connect(kb->chnPanel->chnLabel[i], SIGNAL(clicked()), mapper, SLOT(map()));
}

void IMFrame::setShift()
{
    shift = !shift;
}

void IMFrame::setChaNum()
{
    num = !num;
    shift = false;
    int i;
    if (num) {
        kb->keyChaNum->setText(tr("abc"));
        kb->keyShift->setText(tr("@"));
        kb->chnPanel->chnLineEdit->setText("");

        kb->setGeometry(0, 180, 240, 140);
        kb->chnPanel->setVisible(false);
        kb->keyBoard->setGeometry(0, 0, 240, 140);
        for (i = 0; i < 26; i++)
            kb->key[i]->setText(number[i]);
    }
    else {
        kb->keyChaNum->setText(tr(".?123"));
        if (!chn) {
            kb->setGeometry(0, 180, 240, 140);
            kb->chnPanel->setVisible(false);
	    kb->keyBoard->setGeometry(0,0,240,140);

            kb->keyShift->setText(tr("SFT"));
	}
        else {
            kb->setGeometry(0, 120, 240, 200);
            kb->chnPanel->setVisible(true);
            kb->keyBoard->setGeometry(0, 60, 240, 140);

            kb->keyShift->setText(tr("CLR"));
	}
        for (i = 0; i < 26; i++) {
            kb->key[i]->setText(english[i]);
        }
    }

    setMapping();
}

void IMFrame::setEngChn()
{
    if (num)
	return;

    chn = !chn;
    shift = false;

    kb->chnPanel->chnLineEdit->setText("");

    if (chn) {
        kb->setGeometry(0, 120, 240, 200);
        kb->chnPanel->setVisible(true);
        kb->keyBoard->setGeometry(0, 60, 240, 140);

        kb->keyShift->setText(tr("CLR"));
	kb->keyEngChn->setText(tr("ENG"));
    }
    else {
        kb->setGeometry(0, 180, 240, 140);
        kb->chnPanel->setVisible(false);
        kb->keyBoard->setGeometry(0, 0, 240, 140);

        kb->keyShift->setText(tr("SFT"));
	kb->keyEngChn->setText(tr("CHN"));
    }

    setMapping();
}

void IMFrame::transPy(const QString &text)
{
    model->setFilter("1");
    pageCount = 0;
    currentPage = 0;

    int i;
    for (i = 0; i < 128; i++)
        chinese[i] = "";

    if (text == "") {
	listChn();
    }
    else {
        QString py = text;
        py.prepend("'");
	py.append("'");
        model->setFilter("py == " + py);
        int n = model->rowCount();
        for (i = 0; i < n; i++)
            chinese[i] = model->record(i).value(1).toString();
        pageCount = n/5;
        if (n%5 == 0) {
            pageCount--;
        }
        listChn();
    }
}

void IMFrame::turnLeft()
{
    if (currentPage == 0)
        return;

    currentPage--;
    listChn();
}

void IMFrame::turnRight()
{
    if (currentPage == pageCount)
        return;

    currentPage++;
    listChn();
}

void IMFrame::listChn()
{
    int i;
    for (i = 0; i < 5; i++) {
        kb->chnPanel->chnLabel[i]->setText(chinese[5 * currentPage + i]);
        mapper->setMapping(kb->chnPanel->chnLabel[i], kb->chnPanel->chnLabel[i]->text());
    }
}
/*********实现隐藏和显示键盘，这是和其他程序的一个接口**********/
void IMFrame::handleMessageFromQCop(const QString &message, const QByteArray &data)
{
    if (message == "show") {
        kb->setVisible(true);
    }
    else if (message == "hide"){
        kb->setVisible(false);
    }
}


