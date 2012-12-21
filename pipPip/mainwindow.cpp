#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include "QFile"
#include <QSound>
#include <QDebug>
#include <QLineEdit>
#include <QDir>
#include <QTimer>
#include <QEventLoop>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDataStream>
const static QString toolTipTemplate =
        QObject::trUtf8("<html><body><table border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"1\" cellpadding=\"1\">"
                        "<tr><td><p align=\"right\">Offset: </p></td><td><p><span style=\" font-weight:600;\">%0 </span>"
                        "</p></td></tr><tr><td><p align=\"right\">Dec: </p></td><td><p><span style=\" font-weight:600;\">%1</span>"
                        "</p></td></tr><tr><td><p align=\"right\">Hex: </p></td><td><p><span style=\" font-weight:600;\">%2</span>"
                        "</p></td></tr><tr><td><p align=\"right\">Bin: </p></td><td><p><span style=\" font-weight:600;\">%3</span>"
                        "</p></td></tr></table></body></html>");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QFile f(":/data/1.wav");
    bool ok = true;
    if (f.open(QIODevice::ReadOnly))
    {
        header = f.read(40);
        QByteArray len = f.read(4);
        for (int i = 0; i<len.length(); ++i)
            qDebug() <<(int)(unsigned char)len.at(i);
        int l = (unsigned char)len.at(0) + ((unsigned char)len.at(1)) * 0x100;
        s1 = f.read(l);
        f.close();
    } else
        ok = false;
    f.setFileName(":/data/0.wav");
    if (f.open(QIODevice::ReadOnly))
    {
        f.seek(40);
        QByteArray len = f.read(4);
        for (int i = 0; i<len.length(); ++i)
            qDebug() <<(int)(unsigned char)len.at(i);
        int l = (unsigned char)len.at(0) + ((unsigned char)len.at(1)) * 0x100;
        s0 = f.read(l);
    } else
        ok = false;
    f.setFileName(":/data/empty.wav");
    if (f.open(QIODevice::ReadOnly))
    {
        f.seek(40);
        QByteArray len = f.read(4);
        for (int i = 0; i<len.length(); ++i)
            qDebug() <<(int)(unsigned char)len.at(i);
        int l = (unsigned char)len.at(0) + ((unsigned char)len.at(1)) * 0x100;
        empty = f.read(l);
    } else
        ok = false;
    connect(ui->pushButtonPlay, SIGNAL(clicked()), this, SLOT(onPlayButtonPushed()));
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(onSaveButtonPushed()));
    connect(ui->spinBoxLenght, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxItemChanged(int)));
    model = new ItemModel(this);
    ui->tableView->setModel(model);
    model->setLenghtData(ui->spinBoxLenght->value());
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tableView->setItemDelegate(new Delegate(this));


}

MainWindow::~MainWindow()
{
    foreach (QTemporaryFile *tp, listOftp)
        delete tp;
    delete ui;
}

QByteArray MainWindow::generateWavFile(const QByteArray &data)
{
    if (data.isEmpty())
        return QByteArray();
    QByteArray  res;
    res.append(empty);
    for (int i = 0; i < data.length(); ++i)
        appendByte(res, data.at(i));
    res.append(empty);
    int len = res.length();
    QByteArray h;
    h.append(header);
    qDebug() << trUtf8("%0, %1").arg(len).arg(len, 16);
    h.append((char)(len % 0x100));
    h.append((char)(len >> 8));
    h.append((char)(len >> 16));
    h.append((char)(len >> 24));
    res.prepend(h);
    QFile f("d:\\test.wav");
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(res);
    f.close();
    return res;
}

void MainWindow::appendByte(QByteArray &array, char byte)
{

    unsigned char mask = 0x80;
    qDebug() << "start byte";
    for (int i=0; i < 8; ++i)
    {
        if (byte & mask)
        {
            array.append(s1);
            qDebug() << 1;
        }
        else
        {
            array.append(s0);
            qDebug() << 0;
        }
        mask = mask >> 1;
    }
    qDebug() << "end byte";
    return;
}

void MainWindow::onSaveButtonPushed()
{
    QString fileName = QFileDialog::getSaveFileName(this, trUtf8("Сохранение wav файла"), "./", trUtf8("*.wav файлы(*.wav)"));
    if (fileName.isEmpty())
        return;
    QRegExp re(".wav$");
    if (re.indexIn(fileName) < 0)
        fileName += ".wav";
    QByteArray res = generateWavFile(model->getData());
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, trUtf8("Ошибка"), trUtf8("Ошибка открытия файла"));
        return;
    }
    f.write(res);
    f.close();
}

Q_DECLARE_METATYPE(QSound*)
void MainWindow::onPlayButtonPushed()
{
    ui->pushButtonPlay->setEnabled(false);
    QByteArray array = model->getData();
    QByteArray data = generateWavFile(array);
    QAudioFormat format;
    // Set up the format, eg.
    format.setFrequency(44100);
    format.setChannels(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);
    QAudioOutput *audio = new QAudioOutput( format, 0);
    QDataStream ds(&data, QIODevice::ReadOnly);
    ds.device()->open(QIODevice::ReadOnly);
    qDebug() << "bytes available" <<  ds.device()->bytesAvailable();
    audio->start(ds.device());
    while(audio->state() == QAudio::ActiveState)
        QApplication::processEvents();
    delete audio;
    ui->pushButtonPlay->setEnabled(true);
}

void MainWindow::onSpinBoxValueChanged(int value)
{
    model->setLenghtData(value);
}

void MainWindow::onComboBoxItemChanged(int index)
{
    switch (index) {
    case 0:
        model->setBase(ItemModel::baseBin);
        break;
    case 1:
        model->setBase(ItemModel::baseDec);
        break;
    case 2:
        model->setBase(ItemModel::baseHex);
        break;
    default:
        break;
    }
}

void MainWindow::onTimer()
{
    QSound *sound = sender()->property("sound").value<QSound*>();
    qDebug() << "sound" << sound;
    if (!sound)
        emit playFinished();
    else
        if (sound->isFinished())
            emit playFinished();
}

ItemModel::ItemModel(QObject *parent) :
    QStandardItemModel(parent)
{
    clear();
    currentBase = baseDec;
    setColumnCount(8);
    setLenghtData(1);
    recalculate();
}

ItemModel::~ItemModel()
{

}

void ItemModel::setLenghtData(const int &lenght)
{
    int rowC = lenght / 8;
    if (lenght % 8)
        ++rowC;
    setRowCount(rowC);

    for(int r = 0; r < rowCount(); ++r)
        for (int c = 0; c < 8; c++)
        {
            QStandardItem *item = itemFromIndex(index(r, c));
            if (!item)
                continue;
            item->setEditable((r * 8 + c) < lenght);
        }
    recalculate();
}

void ItemModel::setBase(const ItemModel::base &newBase)
{
    if (newBase == currentBase)
        return;
    currentBase = newBase;
    recalculate();
}

void ItemModel::recalculate()
{
    for (int c = 0; c < 8; c++)
        for(int r = 0; r < rowCount(); ++r)
        {
            QStandardItem *item = itemFromIndex(index(r, c));
            if (!item)
                continue;
            item->setData((int)currentBase, Qt::UserRole + 2);
            if (item->isEditable())
            {
                int val = item->data(Qt::UserRole + 1).toInt();
                QString hexVal, decVal, binVal;
                hexVal = QString("0x%0").arg(val, 2, 16, QChar('0'));
                decVal = QString("%0").arg(val);
                binVal = QString("b'%0'").arg(val, 8, 2, QChar('0'));
                int pos = r * 8 + c;
                item->setToolTip(toolTipTemplate.arg(pos).arg(decVal).arg(hexVal).arg(binVal));
                switch(currentBase){
                case baseBin:
                    item->setText(trUtf8("%1: %0").arg(binVal).arg(pos));
                    break;
                case baseDec:
                    item->setText(trUtf8("%1: %0").arg(decVal).arg(pos));
                    break;
                case baseHex:
                    item->setText(trUtf8("%1: %0").arg(hexVal).arg(pos));
                    break;
                }
            }
            else
                item->setText(trUtf8("Недоступно"));
        }
}

QByteArray ItemModel::getData()
{
    QByteArray ret;
    for(int r = 0; r < rowCount(); ++r)
        for (int c = 0; c < 8; c++)
        {
            QStandardItem *item = itemFromIndex(index(r, c));
            if (!item)
                continue;
            if (!item->isEditable())
                break;
            ret.append((char)item->data(Qt::UserRole + 1).toInt());
        }
    qDebug() << "start of array";
    for (int i = 0; i < ret.length(); ++i)
        qDebug() << (unsigned char)ret.at(i);
    qDebug() << "end of array" << ret.length();
    return ret;

}


QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit)
        return;
    ItemModel::base b = (ItemModel::base)index.data(Qt::UserRole + 2).toInt();
    switch(b)
    {
    case ItemModel::baseBin:
        lineEdit->setText(QString("%0").arg(index.data(Qt::UserRole + 1).toInt(),8, 2, QChar('0')));
        break;
    case ItemModel::baseDec:
        lineEdit->setText(QString("%0").arg(index.data(Qt::UserRole + 1).toInt()));
        break;
    case ItemModel::baseHex:
        lineEdit->setText(QString("%0").arg(index.data(Qt::UserRole + 1).toInt(),2, 16, QChar('0')));
        break;
    }
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    if (!lineEdit)
        return;
    ItemModel::base b = (ItemModel::base)index.data(Qt::UserRole + 2).toInt();
    bool ok;
    int val;
    switch(b)
    {
    case ItemModel::baseBin:
        val = lineEdit->text().toInt(&ok, 2);
        break;
    case ItemModel::baseDec:
        val = lineEdit->text().toInt(&ok, 10);
        break;
    case ItemModel::baseHex:
        val = lineEdit->text().toInt(&ok, 16);
        break;
    }
    if (ok && val <= 0xff)
        model->setData(index, val, Qt::UserRole + 1);
    else
        val = index.data(Qt::UserRole + 1).toInt();
    qDebug() << val;
    int pos = index.row() * 8 + index.column();
    QString hexVal, decVal, binVal;
    hexVal = QString("0x%0").arg(val, 2, 16, QChar('0'));
    decVal = QString("%0").arg(val);
    binVal = QString("b'%0'").arg(val, 8, 2, QChar('0'));
    QString toolTip = toolTipTemplate.arg(pos).arg(decVal).arg(hexVal).arg(binVal);

    model->setData(index, toolTip, Qt::ToolTipRole);
    switch(b)
    {
    case ItemModel::baseBin:
        model->setData(index, QString("%1: %0").arg(binVal).arg(pos));
        break;
    case ItemModel::baseDec:
        model->setData(index, QString("%1: %0").arg(decVal).arg(pos));
        break;
    case ItemModel::baseHex:
        model->setData(index, QString("%1: %0").arg(hexVal).arg(pos));
        break;
    }
}

void Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect = option.rect;
    QSize sizeHint = editor->sizeHint();
    if (rect.width()<sizeHint.width()) rect.setWidth(sizeHint.width());
    if (rect.height()<sizeHint.height()) rect.setHeight(sizeHint.height());
    editor->setGeometry(rect);
}


Delegate::Delegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}


