/*
 *  neuronseg .cpp
 *  neuronseg .h
 *
 *  Created by Yang, Jinzhu, on 04/01/11.
 */

#include <QtGlobal>
//#include <dirent.h>
#include "neuronseg.h"
#include "v3d_message.h" 
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include"../../../v3d_main/neuron_editing/neuron_format_converter.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(Neuronseg, NEURONSEGlugin);


//plugin funcs
const QString title = "neuronseg";
QStringList NEURONSEGlugin::menulist() const
{
    return QStringList() 
	<<tr("Swc segment")
	<<tr("Help");
}

void NEURONSEGlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Swc segment"))
	{
		swc_to_segments(callback, parent,1);
		
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.11)Neuronseg Plugin 1.0 developed by Jinzhu Yang. (Peng Lab, Janelia Research Farm Campus, HHMI), save swc files according to different segments of swc file");
		return;
	}
}

void swc_to_segments(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
	NeuronTree neuron;
	QString filename;
	QStringList filenames;
	///////////////////////////////////////////////////////
	if (method_code ==1)
	{
		
		filenames = QFileDialog::getOpenFileNames(0, 0,"","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);
		if(filenames.isEmpty()) 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		NeuronSWC *p_cur=0;
		
		QDir dir;
	
		for (V3DLONG i = 0; i < filenames.size();i++)//////re-search the bounding box
		{
			filename = filenames[i];
			
			QString swcname = QFileInfo(filename).fileName();
			
			QString curFilePath = QFileInfo(filename).path();
			
			qDebug()<<"filenames"<<"name"<<"path"<<filename<<swcname<<curFilePath;
			
	     	dir=QFileInfo(filename).dir();
		
			
			QString dirname= swcname + "_seg";
			
			dir.mkdir(dirname);
			
			if (filename.size()>0)
			{
				neuron = readSWC_file(filename);
				//V3DLONG seg_id, nodeinseg_id;
				
				QList <NeuronSWC> listNeuron;
				
				QString file= neuron.file;
				
				//QList<NeuronSWC> & qlist = neuron.listNeuron;
                 
				V_NeuronSWC_list NeuronList = NeuronTree__2__V_NeuronSWC_list(&neuron);
				V_NeuronSWC seg = merge_V_NeuronSWC_list(NeuronList);
				seg.name = NeuronList.name;
				seg.file = NeuronList.file;
				QString swcfile;
				for(int n =0; n < NeuronList.seg.size(); n++)
				{
					NeuronTree SS;
					QList <NeuronSWC> listNeuron;
					QHash <int, int>  hashNeuron;
					listNeuron.clear();
					hashNeuron.clear();
					for (V3DLONG i=0;i< NeuronList.seg.at(n).row.size();i++)
					{
						NeuronSWC v;
						v.n		= NeuronList.seg.at(n).row.at(i).data[0];
						v.type	= NeuronList.seg.at(n).row.at(i).data[1];
						v.x 	= NeuronList.seg.at(n).row.at(i).data[2];
						v.y 	= NeuronList.seg.at(n).row.at(i).data[3];
						v.z 	= NeuronList.seg.at(n).row.at(i).data[4];
						v.r 	= NeuronList.seg.at(n).row.at(i).data[5];
						v.pn    = NeuronList.seg.at(n).row.at(i).data[6];
						v.seg_id       = seg.row.at(n).seg_id;
						v.nodeinseg_id = seg.row.at(n).nodeinseg_id;
						listNeuron.append(v);
						hashNeuron.insert(v.n, listNeuron.size()-1);
					}
					SS.n = -1;
					SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
					SS.on = true;
					SS.listNeuron = listNeuron;
					SS.hashNeuron = hashNeuron;
					SS.name = seg.name.c_str();
					SS.file = seg.file.c_str();
					swcfile.sprintf("%d",n);
					QString curImageName1 = curFilePath +"/"+swcname + "_seg" +"/" + swcname + swcfile +".swc";
					qDebug()<<"curImage"<<curImageName1;
					writeSWC_file(curImageName1,SS);
					//printf("NeuronListsize=%ld qlistsize=%ld num=%ld\n",a,qlist.size(),num);
					//printf("name=%ls fila=%ls n=%ld\n",name,file,n);
				}
			}
			else 
			{
				v3d_msg("You don't have any SWC file open in the main window.");
				return;
			}

		}
		
	}
	
}
void SetsizeDialog::update()
{
	//get current data
	
	NX = coord_x->text().toLong()-1;
	NY = coord_y->text().toLong()-1;
	NZ = coord_z->text().toLong()-1;
	
}
