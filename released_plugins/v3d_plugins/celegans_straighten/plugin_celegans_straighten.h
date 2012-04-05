//celegans straighten plugin
//by Lei Qu
//2010-08-09
// Upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05

#ifndef __PLUGIN_CELEGANS_STRAIGHTEN_H__
#define __PLUGIN_CELEGANS_STRAIGHTEN_H__

#include <v3d_interface.h>


class CElegansStraightenPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}

     bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent){return false;}
};



#endif
