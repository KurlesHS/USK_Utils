#include "mylineedit.h"

MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    installEventFilter(this);
}

bool MyLineEdit::eventFilter(QObject *obj, QEvent *ev)
{

    if (ev->type() == QEvent::FocusOut) emit focusOut();
    return QLineEdit::eventFilter(obj,ev);
}
