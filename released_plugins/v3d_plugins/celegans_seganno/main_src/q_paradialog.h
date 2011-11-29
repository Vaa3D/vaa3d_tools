//q_paradialog.h
//by Lei Qu
//2010-07-31

#ifndef __Q_PARADIALOG_H__
#define __Q_PARADIALOG_H__

#include <QDialog>
#include <v3d_interface.h>

#include "ui_paradialog.h"


class ParaDialog : public QDialog, public Ui::ParaDialog
{
	Q_OBJECT

public:
	ParaDialog(V3DPluginCallback &callback,QWidget *parent);
	~ParaDialog();

private slots:
	void _slots_openImgDlg();
	void _slots_openAtlasDlg();
	void _slots_saveAtlasDlg();

public:
	void IniDialog(V3DPluginCallback &callback);

	v3dhandleList h_wndlist;

};


#endif
