
#include "clicklabel.h"
#include <QWidget>
#include <QLineEdit>
#include <QToolButton>

class ChnPanel:public QWidget
{
    Q_OBJECT
public:
    ChnPanel();
    ~ChnPanel();
    QLineEdit *chnLineEdit;
    ClickLabel *chnLabel[5];//最多候选中文字的个数
    QToolButton *btnLeft;
    QToolButton *btnRight;
};
