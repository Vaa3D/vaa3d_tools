/* neuronPicker_plugin.h
 * 
 * 2014-12-01 : by Xiang Li, Hanbo Chen
 */
 
#ifndef __NEURONPICKER_PLUGIN_H__
#define __NEURONPICKER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
#include "neuronpicker_func.h"

using namespace std;

#define NAME_INWIN "Input_Neuron_Picker"
#define NAME_OUTWIN "Output_Neuron_Picker"

class neuronPicker : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	bool interface_run(V3DPluginCallback2 &_V3DPluginCallback2_currentCallback, QWidget *_QWidget_parent);
	

	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

private:
    void printHelp();
};


class neuronPickerDialog : public QDialog
{
    Q_OBJECT

public:
    neuronPickerDialog(V3DPluginCallback2 * cb);
    ~neuronPickerDialog();
    void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
    void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);

private:
    V3DPluginCallback2 * callback;

    neuronPickerMain2 pickerObj;
    QString fname_previnput;
    unsigned char *image1Dc_in;
    unsigned char *image1Dc_out;
    V3DLONG sz_img[4];
    V3DLONG sz_out[4];
    int intype;
    LandmarkList LList;
    vector<V3DLONG> poss_landmark;
    V3DLONG pos_out;

private:
    void creat();
    void checkButtons();
    void initDlg();
    void updateInputWindow();
    void updateOutputWindow();

public slots:
    bool load();
    void output();
    int loadMarkers();
    void extract();
    void syncMarkers(int c);
    void saveFile();
    void skip();
    void reject();
    void runall();
    void autoSeeds();

public:
    QComboBox *cb_marker;
    QPushButton *btn_update, *btn_extract, *btn_save, *btn_next, *btn_quit, *btn_load, *btn_output, *btn_runall, *btn_autoMarkers;
    QSpinBox *spin_distance, *spin_bgthr, *spin_conviter, *spin_fgthr, *spin_sizethr, *spin_sizemargin;
    QLineEdit *edit_load, *edit_output;
};


#endif

