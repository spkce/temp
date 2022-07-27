
#include <QLabel>


class ClickLabel:public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QString name,QWidget *parent);
    virtual ~ClickLabel();

private:
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    bool status;
signals:
    void clicked();
};
