#include "clicklabel.h"
#include <QMouseEvent>
ClickLabel::ClickLabel(QString name,QWidget *parent):QLabel(parent)
{
    setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    setText(name);
    status=false;
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mousePressEvent(QMouseEvent *ev)
{
    if(!status)
    {
        status=true;
    }

}

void ClickLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(status)
    {
        emit clicked();
        status=false;
    }
	 Q_UNUSED(ev);
   

}

