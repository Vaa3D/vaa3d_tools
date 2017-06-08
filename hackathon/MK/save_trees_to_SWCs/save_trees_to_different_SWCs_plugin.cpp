/* save_trees_to_different_SWCs_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-7 : by MK
 */
 
#include <iostream>
#include <fstream>
#include "basic_surf_objs.h"
#include "sort_swc.h"
#include "v3d_message.h"
#include <vector>
#include "save_trees_to_different_SWCs_plugin.h"

using namespace std;
Q_EXPORT_PLUGIN2(save_trees_to_different_SWCs, saveSeparateTrees);
 
QStringList saveSeparateTrees::menulist() const
{
	return QStringList() 
		<<tr("save separated trees")
		<<tr("about");
}

QStringList saveSeparateTrees::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void saveSeparateTrees::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("save separated trees"))
	{
		QList <V3dR_MainWindow*> allWindowList = callback.getListAll3DViewers();
		V3dR_MainWindow* v3dwin;
		QList<NeuronTree>* ntTreeList;

		int winid;
		qDebug("search for 3D windows");
		for (V3DLONG i=0; i<allWindowList.size(); ++i)
		{
			QString fname_win=callback.getImageName(allWindowList[i]);
			if(fname_win.contains("3D View [") && fname_win.contains("]")) v3dwin = allWindowList[i];
		}
		qDebug("match and select 3D windows");
		if(allWindowList.size() < 1)
		{
			v3d_msg("Cannot find 3D viewer.");
			return;
		}
		ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(v3dwin);
		//qDebug() << ntTreeList->size();

		QList<NeuronSWC> inputNeurons, sortedNeurons;
		QList<NeuronTree> treeList;
		inputNeurons = ntTreeList->at(0).listNeuron;
		
		SortSWC(inputNeurons, sortedNeurons, VOID, 0);

		int count = 0;
		for (QList<NeuronSWC>::iterator it=sortedNeurons.begin(); it!=sortedNeurons.end(); ++it)
		{
			if (it->parent == -1)
			{
				++count;
				//cout << count << " " << endl;
				NeuronTree newTree;
				treeList.push_back(newTree);
				treeList[count-1].listNeuron.push_back(*it);
				//qDebug() << treeList.size();
			}
			treeList[count-1].listNeuron.push_back(*it);
		}
		qDebug() << treeList.size();
		QStringList nameList;
		nameList << callback.getImageName(v3dwin);
		QString windowName = nameList[0];
		QStringList parsedName1 = windowName.split("[");
		QStringList parsedName2 = parsedName1[1].split("]");
		QString fileFullName = parsedName2[0];
		qDebug() << fileFullName;

		int treeNumber = 0;
		for (int i=0; i<treeList.size(); ++i)
		{
			if (treeList[i].listNeuron.size() == 0) continue;
			++treeNumber;
			QString treeNum = QString::number(treeNumber);
			QString saveFileName = fileFullName + "_tree" + treeNum + ".swc";
			writeSWC_file(saveFileName, treeList[i]);
		}

		return;
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2017-6-7"));
	}
}

bool saveSeparateTrees::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

