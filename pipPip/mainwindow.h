#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class QTemporaryFile;

namespace Ui {
class MainWindow;
}

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    Delegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class ItemModel : public QStandardItemModel
{
    Q_OBJECT
public:

    enum base {
        baseBin,
        baseDec,
        baseHex
    };

    ItemModel(QObject *parent);
    ~ItemModel();
    void setLenghtData(const int &lenght);
    void setBase(const base &newBase);
    void recalculate();
    base getBase() const {return currentBase;}
    QByteArray getData();

private:
    base currentBase;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QByteArray generateWavFile(const QByteArray &data);
    void appendByte(QByteArray &array, char byte);

private slots:
    void onSaveButtonPushed();
    void onPlayButtonPushed();
    void onSpinBoxValueChanged(int value);
    void onComboBoxItemChanged(int index);
    void onTimer();

signals:
    void playFinished();



private:
    Ui::MainWindow *ui;
    QByteArray s1, s0;
    QByteArray header;
    ItemModel *model;
    QList<QTemporaryFile*> listOftp;

};

#endif // MAINWINDOW_H
