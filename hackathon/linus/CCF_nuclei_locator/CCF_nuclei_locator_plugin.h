/* CCF_nuclei_locator_plugin.h
 * This plugin creates a set of markers in specified CCF regions based on experimental density clouds.
 * 2022-7-22 : by Linus Manubens Gil
 */
 
#ifndef __CCF_NUCLEI_LOCATOR_PLUGIN_H__
#define __CCF_NUCLEI_LOCATOR_PLUGIN_H__

#include <QtGui>
#include <vector>
#include <v3d_interface.h>

class CCF_nuclei_locator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void menu_locate( V3DPluginCallback2 &callback, QWidget *parent);

    //template<class T> bool save_mask_image(T * &outimg1d, unsigned char * inimg1d, V3DLONG reg_id, int soma_radius, QString output_file,
    //                                       long sz0, long sz1, long sz2, int datatype, V3DPluginCallback2 & callback);
};

bool get_soma_coords_fixed_dens(float * inimg1d, V3DLONG reg_id, float soma_d, float fixed_dens, QString output_marker_file,
                                       long sz0, long sz1, long sz2);

#endif

