/* choose_marker_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2018-7-19 : by LXF
 */
 
#ifndef __CHOOSE_MARKER_PLUGIN_H__
#define __CHOOSE_MARKER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class choose_markerPlugin : public QObject, public V3DPluginInterface2_1
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
struct Coordinate
{
    int x;
    int y;
    int z;
    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};
int get_color(QString & img,QString & markerOpenName,V3DPluginCallback2 & callback);
bool export_TXT(QHash<Coordinate,int> &color,QString fileSaveName);
#endif

