/* pattern_search_plugin.h
 * finding substructure in a whole neuron with certain morpological pattern.
 * 2017-6-29 : by He Yishan
 */
 
#ifndef __PATTERN_SEARCH_PLUGIN_H__
#define __PATTERN_SEARCH_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class pattern_search : public QObject, public V3DPluginInterface2_1
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

