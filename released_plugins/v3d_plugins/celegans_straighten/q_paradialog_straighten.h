//q_paradialog_straighten.h
//by Lei Qu
//2010-08-11

#ifndef __Q_PARADIALOG_STRAIGHTEN_H__
#define __Q_PARADIALOG_STRAIGHTEN_H__

#include <QDialog>
#include <v3d_interface.h>

#include "ui_paradialog.h"


class ParaDialog : public QDialog, public Ui::ParaDialog
{
	Q_OBJECT

public:
	ParaDialog(V3DPluginCallback &callback,QWidget *parent);

private slots:
	void _slots_openImgDlg();
	void _slots_openMarkerDlg();
	void _slots_saveStrImgDlg();

public:
	void IniDialog(V3DPluginCallback &callback);

	v3dhandleList h_wndlist;

};


#endif
