/* saveToanoPlugin_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-01-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "saveToanoPlugin_plugin.h"
#include <iostream>
#include <fstream>

using namespace std;
Q_EXPORT_PLUGIN2(saveToanoPlugin, saveToanoPlugin);

void generatorAno43Dviewer(V3DPluginCallback2 &callback, QWidget *parent);

 
QStringList saveToanoPlugin::menulist() const
{
	return QStringList() 
        <<tr("save all SWC, APO, and Surface files to an ANO file")
		<<tr("about");
}

QStringList saveToanoPlugin::funclist() const
{
	return QStringList()
		<<tr("help");
}

void saveToanoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("save all SWC, APO, and Surface files to an ANO file"))
	{
        generatorAno43Dviewer(callback,parent);
	}
	else
	{
        v3d_msg(tr("This is a plugin to save all SWC, APO, and Surface files from a 3D viewer,"
            "Developed by Zhi Zhou, 2013-01-13"));
	}
}

bool saveToanoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void generatorAno43Dviewer(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
   QList<NeuronTree> *SWC_list;
   QList<CellAPO> *APO_list;
   QList<LabelSurf> *SURFACE_list;

   QString fileName = QFileDialog::getSaveFileName(parent, "Save Linker File",
           "linker_file.ano",
           "Linker File (*.ano)");

   if (fileName.isEmpty())
       return;

   ofstream anofile;
   anofile.open (fileName.toStdString().c_str(),ios::out | ios::app );


   if((SWC_list = callback.getHandleNeuronTrees_3DGlobalViewer(curwin)) && SWC_list->count()>0)
   {
       for(V3DLONG i = 0; i < SWC_list->count(); i++)
       {
           QString temp_swc;
           temp_swc = SWC_list->at(i).file;
           anofile << "SWCFILE=" << temp_swc.toStdString().c_str() << endl;
           anofile << "\n";
           anofile.close();

       }
   }
}
