#ifndef NEURON_SEPERATOR_EXPLORER_H
#define NEURON_SEPERATOR_EXPLORER_H

#include <QDialog>
#include <QtGui>
#include <v3d_interface.h>

#define NAME_INWIN "Neuron_Seperator_Result_Explorer_OrginalImage"
#define NAME_OUTWIN "Neuron_Seperator_Result_Explorer_OverlapView"
#define NAME_EXTWIN "Neuron_Seperator_Result_Explorer_SeperateNeuron"

typedef struct{
    int status; //0: not checked; 1: checked; 2: outlier
    QList<QString> fnames_extract;
    QList<int> status_extract; //0: not checked; 1: accept; 2: reject
}extraction;

class neuron_seperator_explorer : public QDialog
{
    Q_OBJECT
public:
    neuron_seperator_explorer(V3DPluginCallback2 * cb, QWidget *parent = 0);
    ~neuron_seperator_explorer() {}

signals:

public slots:
    void acceptExt();
    void weakAcceptExt();
    void rejectExt();
    void rejectImg();
    void needRerunImg();
    void preExt();
    void preImg();
    void nextImg();
    void save();
    void loadPoj();
    void screenShot();

private:
    void creat();
    void checkButton();
    void updateAll();
    void updateOutputWindow();
    void updateInputWindow();
    void updateExtractWindow(unsigned char *image1Dc, V3DLONG size[4], int type);
    void save(QString fname);
    void keyReleaseEvent(QKeyEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    V3DPluginCallback2 * callback;
    QString anopath;
    QList<QString> fnames_ano;
    QList<extraction> imgs;
    int idx_img, idx_ext, pre_img, pre_ext;
    unsigned char *image1Dc_in;
    unsigned char *image1Dc_out;
    V3DLONG sz_img[4];
    V3DLONG sz_out[4];
    int intype, outtype;
    bool keyPressMask;


    QGraphicsScene * scene_output;
    QGraphicsView * view_output;
    QGraphicsScene * scene_input;
    QGraphicsView * view_input;
    QGraphicsScene * scene_extract;
    QGraphicsView * view_extract;

public:
    QLineEdit *edit_dir,*edit_curimg,*edit_curext;
    QPushButton *btn_acceptExt, *btn_rejectExt, *btn_rejectImg, *btn_weakAcceptExt;
    QPushButton *btn_preExt, *btn_preImg, *btn_nextImg, *btn_rerunImg;
    QPushButton *btn_save, *btn_quit, *btn_screenshot;
    QPushButton *btn_loadDir, *btn_loadPoj;
    QCheckBox *check_myview;
};

#endif // NEURON_SEPERATOR_EXPLORER_H
