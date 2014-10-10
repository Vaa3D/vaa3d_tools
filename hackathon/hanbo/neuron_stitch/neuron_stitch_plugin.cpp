/* neuron_stitch_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_stitch_plugin.h"
#include "neuron_stitch_func.h"

using namespace std;

Q_EXPORT_PLUGIN2(neuron_stitch, neuron_stitch);
 
QStringList neuron_stitch::menulist() const
{
	return QStringList() 
        <<tr("adjust_neurons")
		<<tr("about");
}

QStringList neuron_stitch::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuron_stitch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("adjust_neurons"))
    {
        doadjust(callback, parent);
    }
	else
	{
		v3d_msg(tr("This plugin is for link neuron segments across stacks.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool neuron_stitch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void doadjust(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()==2)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with only 2 SWC file. Please load the two SWC file you want to stitch in the same 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //to-do: pop up a window to select
        v3dwin = selectWindowList[0];
    }else{
        v3dwin = selectWindowList[0];
    }

    //call dialog
    NeuronGeometryDialog myDialog(&callback, v3dwin);
    int res=myDialog.exec();
}
