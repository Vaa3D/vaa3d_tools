//q_paradialog_rigidaffine.h
//by Lei Qu
//2011-04-11

#ifndef __Q_PARADIALOG_RIGIDAFFINE_H__
#define __Q_PARADIALOG_RIGIDAFFINE_H__

#include <QDialog>

#include <v3d_interface.h>

#include "ui_paradialog_rigidaffine.h"


class CParaDialog_rigidaffine : public QDialog, public Ui::Paradialog_rigidaffine
{
	Q_OBJECT

public:
	CParaDialog_rigidaffine(V3DPluginCallback &callback,QWidget *parent);
	~CParaDialog_rigidaffine();

	QStringList m_qsSelectedFiles_sub;

private slots:
	void _slots_openDlg_tar();
	void _slots_openDlg_sub();
	void _slots_saveDlg_sub2tar();
	void _slots_saveDlg_grid();

public:
	void IniDialog();

	v3dhandleList h_wndlist;

};


#endif
