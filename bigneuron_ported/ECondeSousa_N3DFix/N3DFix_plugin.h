/* Created by:
*
* 				Eduardo Conde-Sousa [econdesousa@gmail.com]
*							and
*				Paulo de Castro Aguiar [pauloaguiar@ineb.up.pt]
*
* on Dez 10, 2014
* to visualize and remove artifacts resulting
* from the 3D reconstruction of dendrites / axons
*
* (last update: June 30, 2015)
*/
 
#ifndef __N3DFIX_PLUGIN_H__
#define __N3DFIX_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class N3DfixPlugin : public QObject, public V3DPluginInterface2_1
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

#endif

