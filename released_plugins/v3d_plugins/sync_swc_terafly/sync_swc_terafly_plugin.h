/* sync_swc_terafly_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-8-29 : by Zhi Zhou
 */
 
#ifndef __SYNC_SWC_TERAFLY_PLUGIN_H__
#define __SYNC_SWC_TERAFLY_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class sync_swc_terafly : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

class lookPanel: public QDialog
{
    Q_OBJECT

public:
    lookPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~lookPanel();

    QGridLayout *gridLayout;
    V3DPluginCallback2 & m_v3d;


private slots:
    void _slot_sync_onetime();
    void _slot_set_markers();

};

#endif

