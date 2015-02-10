/* demo_opensurfacefile_plugin.cpp
 * demo for open a surface file
 * 2015-2-10 : by Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <vector>
#include "demo_opensurfacefile_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(demo_opensurfacefile, DemoOpenSurfaceFilePlugin);
 
QStringList DemoOpenSurfaceFilePlugin::menulist() const
{
	return QStringList() 
		<<tr("open_surface_file_menu")
		<<tr("about");
}

QStringList DemoOpenSurfaceFilePlugin::funclist() const
{
	return QStringList()
		<<tr("open_surface_file_func")
		<<tr("help");
}

void DemoOpenSurfaceFilePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("open_surface_file_menu"))
	{
        QString fileName = QFileDialog::getOpenFileName(0,
            tr("Open Surface File"), "", tr("Vaa3D Neuron/Network Files (*.swc);; Vaa3D Pointcloud Files (*.apo);;  Vaa3D Irregular Surface Files (*.v3ds *.vaa3ds *.obj)"));
        if (!fileName.isEmpty())
            callback.open3DViewerForSingleSurfaceFile(fileName);
	}
	else
	{
		v3d_msg(tr("demo for open a surface file. "
			"Developed by Hanchuan Peng, 2015-2-10"));
	}
}

bool DemoOpenSurfaceFilePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("open_surface_file_func"))
	{
        for (int i=0; i<infiles.size(); i++)
        {
            QString fileName = infiles.at(i);
            if (!fileName.isEmpty())
                callback.open3DViewerForSingleSurfaceFile(fileName);
        }
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("How to use: type the command 'vaa3d -x demo_opensurfacefile -f open_surface_file_func -i <filename>'", 0);
	}
	else return false;

	return true;
}

