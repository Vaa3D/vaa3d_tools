/* Smooth_Movie_plugin.h
 * This plugin can be used to generate a smooth movie by several points
 * 2013-11-21 : by Zhi Zhou
 */
 
#ifndef __SMOOTH_MOVIE_PLUGIN_H__
#define __SMOOTH_MOVIE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class Smooth_Movie : public QObject, public V3DPluginInterface2_1
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

public:
    v3dhandle curwin;
    QGridLayout *gridLayout;
    V3DPluginCallback2 & m_v3d;
    int xRot_past, yRot_past,zRot_past;
    int xShift_past,yShift_past,zShift_past;
    int zoom_past;

private slots:
    void _slot_record();
    void _slot_generate();

};

#endif

