/* sync_z_cut_plugin.h
 * This plugin syncs the z-cut of the image and the surface.
 * 2014-07-03 : by YourName
 */
 
#ifndef __SYNC_Z_CUT_PLUGIN_H__
#define __SYNC_Z_CUT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class SyncZ : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

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
    QSpinBox* box_ZCut_Min;
    QSpinBox* box_ZCut_Max;
    QListWidget *list_anchors;
    QComboBox* combo_master;
    QLabel* label_master;
    QCheckBox* check_zed; //take this out later; unnecessary
    QGridLayout *gridLayout;
    v3dhandleList win_list;
    v3dhandleList win_list_past;
    V3DPluginCallback2 & m_v3d;
    QTimer *m_pTimer;

    View3DControl *view_master;

    bool b_autoON;

/**
virtual int zCut0() const {return _zCut0; }
virtual int zCut1() const { return _zCut1; }
virtual int zClip0() const { return _zClip0; }
virtual int zClip1() const { return _zClip1; }
**/

public slots:

/**
virtual void setZCut0(int s);
virtual void setZCut1(int s);
virtual void setZClip0(int s);
virtual void setZClip1(int s);
virtual void setZCutLock(bool); //only for image
virtual void setZSurfCutLock(bool); //for the surface
virtual void enableZSlice(bool); //only for image
**/

private slots:
    void _slot_sync_onetime();
    void reject();
    void change_zed_min();
    //void change_zed_max();

/**
signals:
void changeEnableCut0Slider(bool);
void changeEnableCut1Slider(bool);
void changeZCut0(int s);
void changeZCut1(int s);
void changeZClip0(int s);
void changeZClip1(int s);
**/

public:
//int _zCut0, _zCut1;
//int dzCut, lockZ, lockSurfZ; //added the last one
//int _zClip0, _zClip1;

int zCut0;
int zCut1;

void init_members()
{
//_zCut0 = _zCut1 = dzCut = lockZ = lockSurfZ = _zClip0 = _zClip1 = 0; //added lockSurfZ
zCut0 = zCut1 = 0;
}

};

#endif

