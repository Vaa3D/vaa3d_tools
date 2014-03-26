//last change: by Hanchuan Peng. 2012-09-21

#ifndef __APP2_PLUGIN_H__
#define __APP2_PLUGIN_H__

#include <v3d_interface.h>

//NTApp2: NeuronTracing-APP2

class NTApp2Plugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;

	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;

	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};


#endif

