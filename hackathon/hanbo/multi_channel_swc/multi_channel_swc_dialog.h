#ifndef MULTI_CHANNEL_SWC_DIALOG_H
#define MULTI_CHANNEL_SWC_DIALOG_H

#include <QDialog>
#include <QtGui>
#include <v3d_interface.h>
#include <vector>

using namespace std;

void save_result(QString fname, NeuronTree* nt, vector<float>& ratio, vector<float>& mean, vector<float>& stdev);

bool compute_swc_render(QString fname, NeuronTree* nt_rend);

class multi_channel_swc_dialog : public QDialog
{
    Q_OBJECT
public:
    explicit multi_channel_swc_dialog(V3DPluginCallback2 *cb, QWidget *parent = 0);
    ~multi_channel_swc_dialog();

    QGridLayout *gridLayout;
    QLineEdit *edit_image, *edit_swc, *edit_output;
    QPushButton *btn_loadimage, *btn_loadswc, *btn_setoutput;
    QDoubleSpinBox *spin_threshold1, *spin_threshold2;
    QComboBox *combo_channel1, *combo_channel2;
    QPushButton *btn_run, *btn_quit;

private:
    void create();
    void initDlg();
    void checkbtn();

    unsigned char* p_img;
    V3DLONG sz_img[4];
    int intype;
    NeuronTree* nt;
    QString fname_img, fname_neuronTree;

    V3DPluginCallback2 *callback;

signals:

public slots:
    void load_image();
    void load_swc();
    void set_output();
    void run();
};

#endif // MULTI_CHANNEL_SWC_DIALOG_H
