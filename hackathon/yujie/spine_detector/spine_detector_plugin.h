/* spine_detector_plugin.h
 * This tool detects spine
 * 2015-3-11 : by Yujie Li
 */
 
#ifndef __SPINE_DETECTOR_PLUGIN_H__
#define __SPINE_DETECTOR_PLUGIN_H__

#include "spine_detector_dialog.h"

class spine_detector : public QObject, public V3DPluginInterface2_1
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

