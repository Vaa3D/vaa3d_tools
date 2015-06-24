/* subtree_labelling_plugin.h
 * This tree labels dendrite id, segment id and branch order
 * 2015-6-19 : by Yujie Li
 */
 
#ifndef __SUBTREE_LABELLING_PLUGIN_H__
#define __SUBTREE_LABELLING_PLUGIN_H__

#include "subtree_dialog.h"

class subtree_label : public QObject, public V3DPluginInterface2_1
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

