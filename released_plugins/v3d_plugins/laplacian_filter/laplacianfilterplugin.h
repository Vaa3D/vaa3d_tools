/* __LAPLACIAN_FILTER_H__
 * by Hanchuan Peng, 2012-04-09
 */


#ifndef __LAPLACIAN_FILTER_H__
#define __LAPLACIAN_FILTER_H__

#include <v3d_interface.h>

class LaplacianFilterPlugin : public QObject, public V3DPluginInterface2_1
{
     Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 0.91f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
     bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif



