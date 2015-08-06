/* NeuronPopulator_plugin.h
 * Use this plugin to simulate a group of neurons with desired density. This simulator also counts the contacts between axons and dendrites.
 * 2015-8-5 : by Xiaoxiao Liu
 */
 
#ifndef __NEURONPOPULATOR_PLUGIN_H__
#define __NEURONPOPULATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class NeuronPopulator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    bool func_populate( const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback);
    bool func_detect( const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback);


    void menu_populate( V3DPluginCallback2 &callback, QWidget *parent);
    void menu_detect( V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

