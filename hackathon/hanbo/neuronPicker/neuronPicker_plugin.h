/* neuronPicker_plugin.h
 * 
 * 2014-12-01 : by Xiang Li, Hanbo Chen
 */
 
#ifndef __NEURONPICKER_PLUGIN_H__
#define __NEURONPICKER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <vector>

using namespace std;

#define NAME_INWIN "Input_Neuron_Picker"
#define NAME_OUTWIN "Output_Neuron_Picker"

class neuronPickerDialog : public QDialog
{
    Q_OBJECT

public:
    neuronPickerDialog(V3DPluginCallback2 * cb);
	void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
	void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz);

private:
    QString fname_input, fname_outbase;
    V3DPluginCallback2 * callback;

    unsigned char *image1Dc_in;
    unsigned char *image1D_out;
    V3DLONG sz_img[4];
    int intype;
    LandmarkList LList;
    vector<V3DLONG> poss_landmark;
    int* image1D_h;
    unsigned char* image1D_v;
    int preBgthr;

private:
    void creat();
    void checkButtons();
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

public:
    QComboBox *cb_marker;
    QPushButton *btn_update, *btn_extract, *btn_save, *btn_next, *btn_quit, *btn_load, *btn_output, *btn_runall;
    //QDoubleSpinBox *spin_color;
    QSpinBox *spin_distance, *spin_bgthr, *spin_huedis;
    QLineEdit *edit_load, *edit_output;
};

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

};

class neuronPickerMain
{
public:
    neuronPickerMain() {}
    ~neuronPickerMain() {}

    static unsigned char * memory_allocate_uchar1D(const V3DLONG i_size);
    static void memory_free_uchar1D(unsigned char *ptr_input);
    static int * memory_allocate_int1D(const V3DLONG i_size);
    static void memory_free_int1D(int *ptr_input);

    static vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);

    static void initChannels_rgb(unsigned char *image1Dc, int *image1D_h, unsigned char *image1D_v, V3DLONG sz_img[4], const int bg_thr);
    static void extract(int *image1D_h, unsigned char *image1D_v, unsigned char *image1D_out, V3DLONG seed, int cubSize, int colorSpan, V3DLONG sz_img[4]);
    static void saveSingleMarker(V3DLONG pos_landmark, QString fname, V3DLONG sz_img[4]);

private:
    static int huedis(int a, int b);
    static V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z);
    static vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
    static V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z);
    static int rgb2hue(const unsigned char R, const unsigned char G, const unsigned char B);
};


#endif

