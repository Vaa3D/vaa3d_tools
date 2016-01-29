#ifndef NOTETAKER_H
#define NOTETAKER_H

#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QFileInfo>
#include <QPushButton>
#include <QGridLayout>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
class NoteTaker : public QWidget
{
    Q_OBJECT
public:
    explicit NoteTaker(QWidget *parent = 0);

signals:

public slots:
    void status(QString statString);
    void setSaveDir(QDir saveDir);
    void save();
    void updateStuff();
private:
    QTextEdit * notes;
    QPushButton *saveButton;
    QPushButton *updateButton;
    QGridLayout *mainLayout;
    void connectStuff();
    QDateTime *myTime;
    QString saveFileString;
    void closeEvent(QCloseEvent *event);
};

#endif // NOTETAKER_H
