
#include <map>
#include <QToolButton>
#include "thread.h"


class Key:public QToolButton
{
    Q_OBJECT
public:
    Key(QString name,int w,int h);
    ~Key();

private slots:
	void onclick();
};
