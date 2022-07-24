#include "chnpanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

ChnPanel::ChnPanel():QWidget(0,Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
{
    chnLineEdit = new QLineEdit();
    chnLineEdit->setFocusPolicy(Qt::NoFocus);
    chnLineEdit->setFont(QFont("wenquanyi",6,QFont::Bold));
    chnLineEdit->setObjectName(QString::fromUtf8("CHN"));
    chnLineEdit->setReadOnly(true);

    int i;
    for (i = 0; i < 5; i++) {
        chnLabel[i] = new ClickLabel("", 0);
        chnLabel[i]->setFont(QFont("wenquanyi", 6, QFont::Bold));
        chnLabel[i]->setObjectName(QString::fromUtf8("CHN"));
    }

    btnLeft = new QToolButton();
    btnLeft->setFont(QFont("wenquanyi", 6, QFont::Bold));
    btnLeft->setText(tr("<"));
    btnLeft->setObjectName(QString::fromUtf8("CHN"));
    btnLeft->setFixedSize(20, 20);

    btnRight = new QToolButton();
    btnRight->setFont(QFont("wenquanyi", 6, QFont::Bold));
    btnRight->setText(tr(">"));
    btnRight->setObjectName(QString::fromUtf8("CHN"));
    btnRight->setFixedSize(20, 20);

    QHBoxLayout *layoutLabel = new QHBoxLayout();
    layoutLabel->addWidget(btnLeft);

    for (i = 0; i < 5; i++)
        layoutLabel->addWidget(chnLabel[i]);
    layoutLabel->addWidget(btnRight);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(chnLineEdit);
    layout->addLayout(layoutLabel);

    setLayout(layout);
    setFixedSize(240,60);
    setVisible(false);
}

ChnPanel::~ChnPanel()
{
}
