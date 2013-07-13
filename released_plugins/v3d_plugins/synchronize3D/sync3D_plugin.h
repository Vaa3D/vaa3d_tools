/* sync3D_plugin.h
 * 2013-07-09 : by Zhi Zhou
 */
 
//last edit: by PHC, remove the redundant data variables, 2013-07-12

#ifndef __SYNC3D_PLUGIN_H__
#define __SYNC3D_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class sync3D : public QObject, public V3DPluginInterface2_1
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



class lookPanel: public QDialog
{	
	Q_OBJECT

public:
	lookPanel(V3DPluginCallback2 &v3d, QWidget *parent);
	~lookPanel();

public:
    QComboBox* combo_master;
    QComboBox* combo_slave;
    QLabel* label_master;
    QLabel* label_slave;
	QCheckBox* check_rotation;
	QCheckBox* check_shift;
	QCheckBox* check_zoom;
	QGridLayout *gridLayout;
	v3dhandleList win_list;
	v3dhandleList win_list_past;			
    V3DPluginCallback2 & m_v3d;
	QTimer *m_pTimer;
	QPushButton* syncAuto;
    View3DControl *view_master;
    View3DControl *view_slave;
	int xRot_past, yRot_past,zRot_past;	
	int xShift_past,yShift_past,zShift_past;
	int zoom_past;
    bool b_autoON;

private:
    void resetSyncAutoState();

private slots:
	void _slot_syncAuto();
    void _slot_sync_onetime();
	void _slot_timerupdate();
    void reject();
};

#endif
