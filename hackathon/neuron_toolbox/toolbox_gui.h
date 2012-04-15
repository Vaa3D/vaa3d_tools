#ifndef __TOOLBOX_GUI_H__
#define __TOOLBOX_GUI_H__
#include <QtGui>
#include "v3d_interface.h"

class SelectPluginDlg : public QDialog
{
	Q_OBJECT

public:
	SelectPluginDlg(QWidget * parent, const V3DPluginCallback2 & _callback);
	
	QWidget * parent;
	V3DPluginCallback2 * callback;
	QTreeWidget * pluginTreeWidget;
	QString root_path;


public slots:
	bool runPlugin();
};

#endif
