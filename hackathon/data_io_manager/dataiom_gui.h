#ifndef __DATAIOM_TOOLBOX_GUI_H__
#define __DATAIOM_TOOLBOX_GUI_H__
#include <QtGui>
#include "v3d_interface.h"
class SelectPluginDlg : public QDialog
{
	Q_OBJECT

public:
	SelectPluginDlg(QWidget * parent, const V3DPluginCallback2 & _callback);
	SelectPluginDlg(QWidget * parent, const V3DPluginCallback2 & _callback, const V3DPluginArgList & _input);
	
	V3DPluginArgList * input;
	QWidget * parent;
	V3DPluginCallback2 * callback;
	QTreeWidget * pluginTreeWidget;
	QString root_path;
	QHash<QTreeWidgetItem*, QString> name_table;


public slots:
	bool runMenu();
	bool runFunc();
	bool freeMem();
};

#endif
