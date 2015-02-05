#ifndef PAINT_DIALOG_H
#define PAINT_DIALOG_H

#include <QtGui>
#include <QDialog>
#include "scribblearea.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>


class Paint_Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Paint_Dialog(QWidget *parent = 0);

public:
    ScribbleArea* paintarea;

signals:

public slots:
    void load();
    void save();
    void penColor();
    void penWidth();

private:
    void create();
    QString fileName;
    bool maybeSave();
    bool saveFile(const QByteArray &fileFormat);


};

#endif // PAINT_DIALOG_H
