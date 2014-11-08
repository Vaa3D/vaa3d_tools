/* cellSegmentation.cpp
 * It aims to automatically segment cells;
 * 2014-10-12 : by Xiang Li (lindbergh.li@gmail.com);
 */
 

#include "v3d_message.h"
#include "cellSegmentation_plugin.h"
#include <vector>
#include <cassert>
#include <math.h>
#include "string"
#include "sstream"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <basic_landmark.h>
#include "compute_win_pca.h"
#include "convert_type2uint8.h"

#pragma region "UI-related functions and function defination"
Q_EXPORT_PLUGIN2(cellSegmentation, cellSegmentation);

QStringList cellSegmentation::menulist() const
{
    return QStringList()
		<<tr("Initialization")
        <<tr("Exemplar defination")
		<<tr("Exemplar propagation")
		<<tr("Further segmentation")
        <<tr("About");
}

QStringList cellSegmentation::funclist() const
{
	return QStringList()
		<<tr("cellsegmentation")
		<<tr("help");
}

void cellSegmentation::domenu(const QString &menu_name, V3DPluginCallback2 &V3DPluginCallback2_currentCallback, QWidget *QWidget_parent)
{
	if (menu_name == tr("Initialization"))
	{
		cellSegmentation::interface_initialization(V3DPluginCallback2_currentCallback,QWidget_parent);
	}
    if (menu_name == tr("Exemplar defination"))
	{
        cellSegmentation::interface_exemplarDefination(V3DPluginCallback2_currentCallback,QWidget_parent);
    }
	else if (menu_name == tr("Exemplar propagation"))
	{
		cellSegmentation::interface_exemplarPropagation(V3DPluginCallback2_currentCallback,QWidget_parent);
	}
	else if (menu_name == tr("Further segmentation"))
	{
		cellSegmentation::interface_furtherSegmentation(V3DPluginCallback2_currentCallback,QWidget_parent);
	}
	//else
	//{ v3d_msg(tr("Segmenting neurons\nby Xiang Li (lindbergh.li@gmail.com);")); }
}

bool cellSegmentation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & V3DPluginCallback2_currentCallback,  QWidget * QWidget_parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("cellsegmentation")) {}
	else if (func_name == tr("help")){}
	else {return false; }
	return true;
}
#pragma endregion

#pragma region "Segmentation interface"

#pragma endregion