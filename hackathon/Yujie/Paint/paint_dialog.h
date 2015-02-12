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
    void clearimage();
    void zoomin();
    void zoomout();
    //void inserttext();

private:
    void create();
    bool maybeSave();
    bool saveFile(const QByteArray &fileFormat);
    //unsigned char * datacopy(unsigned char *data,long size);
    void savezimage(int z);
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);
    QString fileName;
    V3DPluginCallback2 *callback;
    QPlainTextEdit *edit;
    QSpinBox *spin;
    V3DLONG sz_img[4];
    int intype;

    v3dhandle curwin;
    ImagePixelType pixeltype;
    bool zoominflag;
    int previousz;
    unsigned char *paint_1DC;
    unsigned char *backupdata;
    unsigned char *image1Dc_in;


};

#endif // PAINT_DIALOG_H
