/* neuron_color_display_plugin.cpp
 * This plugin will set the color of the neuron displayed
 * 2015-6-12 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_color_display_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(neuron_color_display, SetNeuronDisplayColor);

unsigned char R_table[8]={255,0,0,255,255,128,96,0};
unsigned char G_table[8]={0,0,255,0,196,0,96,128};
unsigned char B_table[8]={255,255,0,0,0,255,255,0};
 
QStringList SetNeuronDisplayColor::menulist() const
{
	return QStringList() 
		<<tr("1_click_color")
		<<tr("about");
}

QStringList SetNeuronDisplayColor::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void SetNeuronDisplayColor::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("1_click_color"))
	{
        do1clickcolor(callback, parent);
	}
	else
	{
		v3d_msg(tr("This plugin will set the color of the neuron displayed. "
			"Developed by Hanbo Chen, 2015-6-12"));
	}
}

bool SetNeuronDisplayColor::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

void SetNeuronDisplayColor::do1clickcolor(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>0)
            selectWindowList.append(allWindowList[i]);
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to color in the 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("1-click neuron color"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
        if(!ok) return;
        for(int i=0; i<selectWindowList.size(); i++){
            if(selectitem==callback.getImageName(selectWindowList[i]))
            {
                winid=i;
                break;
            }
        }
    }else{
        winid=0;
    }
    v3dwin = selectWindowList[winid];

    //load neuron tree
    ntTreeList=callback.getHandleNeuronTrees_Any3DViewer(v3dwin);
    for(int i=0; i<ntTreeList->size(); i++){
        NeuronTree* p = (NeuronTree*)&(ntTreeList->at(i));
        int k=i%8;
        p->color.r=R_table[k];
        p->color.g=G_table[k];
        p->color.b=B_table[k];
        p->color.a=255;
    }
    qDebug("Done Setting Color");
    callback.update_3DViewer(v3dwin);
}

