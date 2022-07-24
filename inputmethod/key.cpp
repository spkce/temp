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
