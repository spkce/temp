
#include <QToolButton>

class Key:public QToolButton
{
    Q_OBJECT
public:
    Key(QString name,int w,int h);
    ~Key();
};
