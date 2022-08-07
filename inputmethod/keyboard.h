
#include "key.h"
#include "chnpanel.h"
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QSignalMapper>

const static QString english[26]={"q","w","e","r","t","y","u","i","o","p",
                                  "a","s","d","f","g","h","j","k","l",
                                  "z","x","c","v","b","n","m"};

const static QString englishCaps[26]={"Q","W","E","R","T","Y","U","I","O","P",
                                  "A","S","D","F","G","H","J","K","L",
                                  "Z","X","C","V","B","N","M"};

const static QString number[26]={"0","1","2","3","4","5","6","7","8","9",
                                 "#","$","%","&&","=","+","-","*","/",
                                 "?","!",":",";","\"","(",")"};

class Keyboard : public QWidget
{
    Q_OBJECT
public:
    Keyboard();
    ~Keyboard();
    ChnPanel *chnPanel;
    QWidget *keyBoard;
    Key *key[26];
    Key *keyShift;
    Key *keyDelete;
    Key *keyComma;
    Key *keyPeriod;
    Key *keyChaNum;
    Key *keyEngChn;
    Key *keySpace;
    Key *keyGo;
	bool isMoving;
    QPoint lastPnt;

	bool m_shift;
	bool m_chaNum;
	bool m_chn;
protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private slots:
	void setShift();
	void setChaNum();
};
