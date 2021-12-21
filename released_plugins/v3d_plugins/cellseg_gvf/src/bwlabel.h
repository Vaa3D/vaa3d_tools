/* bwlabeln.h
 * label all the N objects in a BW image: by Hanchuan Peng
 * a wrapper program for Fuhui Long's bwlabeln function
 */


#ifndef __BWLABELN_H__
#define __BWLABELN_H__

#include <QtGui>

#include "v3d_interface.h"
#include "label_object_dialog.h"

class BWLabelNPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	float getPluginVersion() const {return 0.9f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
	{return false;}
	
	
};



#endif

