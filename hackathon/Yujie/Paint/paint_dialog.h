#ifndef PAINT_DIALOG_H
#define PAINT_DIALOG_H

#include <QtGui>
#include <QDialog>
#include "scribblearea.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QToolButton>
#include <v3d_interface.h>

class Paint_Dialog : public QDialog
{
    Q_OBJECT
public:
    explicit Paint_Dialog(V3DPluginCallback2 *callback, QWidget *parent = 0);

public:
    ScribbleArea* paintarea;

signals:

public slots:
    bool load();
    void save();
    void penColor();
    void penWidth();
    void zdisplay(int);
    void fetch();
    void pushback();

private:
    void create();
    QString fileName;
    V3DPluginCallback2 *callback;
    bool maybeSave();
    bool saveFile(const QByteArray &fileFormat);
    QPlainTextEdit *edit;
    QSpinBox *spin;
    unsigned char *image1Dc_in;
    V3DLONG sz_img[4];
    int intype;
    v3dhandle curwin;
    ImagePixelType pixeltype;

};

#endif // PAINT_DIALOG_H
