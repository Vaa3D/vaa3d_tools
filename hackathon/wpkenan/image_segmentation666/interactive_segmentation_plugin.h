
#ifndef __NEURON_CONNECTOR_PLUGIN_H__
#define __NEURON_CONNECTOR_PLUGIN_H__

#include <QtGui>
#include <QDialog>
#include <v3d_interface.h>


//template <class T1, class T2> bool assign_val(T1 *dst, T2 *src, V3DLONG total_num);
//template <class T1, class T2> bool assign_val_condition(T1 *dst, T2 *src, V3DLONG total_num, T2 condition_val, T1 condition_yes_val, T1 condition_no_val);
//template <class T1, class T2> bool do_seg(T1 * inputData,
//                               T2 * outputData,
//                               V3DLONG sx, V3DLONG sy, V3DLONG sz,
//                               int & iVesCnt, bool b_binarization=true);



class image_segmentation : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

};


void image_IO(V3DPluginCallback2 &callback, QWidget *parent);
bool do_seg(short *pData, V3DLONG sx, V3DLONG sy, V3DLONG sz, int & iVesCnt, bool b_binarization);

#endif

