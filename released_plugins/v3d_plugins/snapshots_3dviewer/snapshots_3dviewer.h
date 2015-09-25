#ifndef __PLUGIN_SNAPSHOTS_3DVIEWER_H__
#define __PLUGIN_SNAPSHOTS_3DVIEWER_H__

#include <QtGui>
#include "v3d_interface.h"

class SnapShots_3Dviewer: public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
        {return false;}
        float getPluginVersion() const {return 1.1f;}
};

class controlPanel: public QDialog
{
	Q_OBJECT

public:
	controlPanel(V3DPluginCallback2 &v3d, QWidget *parent);
	~controlPanel();

public:
	long m_lframeind;
	QLineEdit *m_pLineEdit_filepath;
	QLineEdit *m_pLineEdit_fps;
	V3DPluginCallback2 &m_v3d;
	static controlPanel*m_pLookPanel;
	QTimer *m_pTimer;

private slots:
	void _slot_start();
	void _slot_stop();
	void _slots_openFileDlg_output();
	void _slot_timerupdate();

};

#endif

