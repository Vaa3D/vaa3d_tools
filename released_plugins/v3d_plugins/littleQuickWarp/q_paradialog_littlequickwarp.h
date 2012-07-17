//q_paradialog_littlequickwarp.h
//by Lei Qu
//2012-07-16

#ifndef __Q_PARADIALOG_LITTLEQUICKWARP_H__
#define __Q_PARADIALOG_LITTLEQUICKWARP_H__

#include <QDialog>
#include <v3d_interface.h>
#include "ui_q_paradialog_littlequickwarp.h"


class CParaDialog_littlequickwarp : public QDialog, public Ui::Paradialog_littlequickwarp
{
	Q_OBJECT

public:
	CParaDialog_littlequickwarp(V3DPluginCallback &callback,QWidget *parent);
	~CParaDialog_littlequickwarp();

private slots:
	void _slots_openDlg_img_sub();
	void _slots_openDlg_img_warp();
	void _slots_openDlg_marker_sub();
	void _slots_openDlg_marker_tar();

public:
	void IniDialog();

	v3dhandleList h_wndlist;

};


#endif
