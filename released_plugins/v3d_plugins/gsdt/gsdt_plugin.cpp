/* gsdt_plugin.cpp
 * Perform distance transformation on grayscale image.
 * 2012-03-02 : by Hang Xiao
 */
 
#include <iostream>
#include "v3d_message.h"

#include "gsdt_plugin.h"
#include "gsdt_func.h"

using namespace std;

Q_EXPORT_PLUGIN2(gsdt, GrayScaleDistanceTransformationPlugin);
 
QStringList GrayScaleDistanceTransformationPlugin::menulist() const
{
	return QStringList()
		<<tr("Grayscale Distance Transformation")
		<<tr("about");
}

QStringList GrayScaleDistanceTransformationPlugin::funclist() const
{
	return QStringList()
		<<tr("gsdt")
		<<tr("help");
}

void GrayScaleDistanceTransformationPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Grayscale Distance Transformation"))
	{
		gsdt(callback,parent);
	}
	else
	{
		v3d_msg(tr("Perform distance transformation on grayscale image.. "
			"Developed by Hang Xiao, 2012-03-02"));
	}
}

bool GrayScaleDistanceTransformationPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("gsdt"))
	{
		return gsdt(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x gsdt -f gsdt -i <inimg_file> -o <outimg_file> -p <bkg_thresh_value> <cnn_type> <channel> <z_thickness>"<<endl;
		cout<<endl;
		cout<<"bkg_thresh_value         the background threshold value, default 0 and maximum 255"<<endl;
		cout<<"cnn_type                 connection type, 1 : 6 neighbors, 2 : 18 neighbors, 3 : 26 neighbors"<<endl;
		cout<<"channel                  the input channel value. 0 for red channel, 1 for green channel, 2 for blue channel"<<endl;
		cout<<"z_thickness              the thickness of z axis (double)"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x gsdt -f gsdt -i input.raw -o output.raw -p 0 1 0 1.0"<<endl;
		cout<<endl;
		return true;
	}
}

