#include "ctime.h"
#include "message.h"
#include "key.h"

Key::Key(QString name, int w, int h)
{
    setText(name);
    setFixedSize(w,h);
    setFont(QFont("helvetica", 6, QFont::Bold));
    setObjectName(QString::fromUtf8("KEYBOARD"));

}

Key::~Key()
{
}

void Key::onclick()
{
	QString text = this->text();
	std::string str = text.toStdString();
	printf("onclick %s\n", str.c_str());
	CKBMessage::instance()->send(str);
}
