//plugin_atlasguided_seganno_prior.h
// by Lei Qu
//2010-09-27

#ifndef __PLUGIN_ATLASGUIDED_SEGANNO_PRIOR_H__
#define __PLUGIN_ATLASGUIDED_SEGANNO_PRIOR_H__

#include <v3d_interface.h>


class AtlasGuidedSegAnnoPriorPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};


#endif
