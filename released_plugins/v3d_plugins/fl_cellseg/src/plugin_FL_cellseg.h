/* plugin_FL_cellseg.h
 * cell seg  plugin based on Fuhui Long's code
 * by Hanchuan Peng, 2011-02-22
 */


#ifndef __PLUGIN_FL_CELLSEG_H__
#define __PLUGIN_FL_CELLSEG_H__

#include <QtGui>

#include "v3d_interface.h"

#include "label_object_dialog.h"

class FLCellSegPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
    float getPluginVersion() const {return 0.91f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{return false;}
	
	
};


#endif

