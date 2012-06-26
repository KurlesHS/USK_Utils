#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
#include <QEvent>

class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEdit(QWidget *parent = 0);


protected:
    bool eventFilter(QObject *obj, QEvent *ev);

signals:
    void focusOut();

public slots:

};

#endif // MYLINEEDIT_H
