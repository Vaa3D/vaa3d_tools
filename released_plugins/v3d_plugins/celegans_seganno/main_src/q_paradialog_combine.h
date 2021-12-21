//q_paradialog_combine.h
//by Lei Qu
//2010-10-10

#ifndef __Q_PARADIALOG_COMBINE_H__
#define __Q_PARADIALOG_COMBINE_H__

#include <QDialog>
#include <v3d_interface.h>

#include "ui_paradialog_combine.h"


class ParaDialogCombine : public QDialog, public Ui::ParaDialogCombine
{
	Q_OBJECT

public:
	ParaDialogCombine(V3DPluginCallback &callback,QWidget *parent);

private slots:
	void _slots_openAtlasDlg();
	void _slots_saveAtlasDlg();

public:
	void IniDialog(V3DPluginCallback &callback);

	v3dhandleList h_wndlist;

};


#endif
