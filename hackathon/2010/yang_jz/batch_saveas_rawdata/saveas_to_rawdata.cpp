/*
 *  saveas_to_rawdata .cpp
 *  saveas_to_rawdata.h 
 *
 *  Created by Yang, Jinzhu, on 04/08/11.
 *  
 */

#include <QtGlobal>
#include <dirent.h>
#include "saveas_to_rawdata.h"
#include "v3d_message.h" 

//#include <fstream>
//#include <sstream>
//#include <iostream>

#include <vector>
#include <list>
#include <bitset>

#include <set>

#include <cmath>

#include <ctime>

#include <complex>
#include <fftw3.h>

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"
#include "basic_4dimage.h"

#include "imageio_mylib.h" 

#include "mg_image_lib11.h"

#include "stackutil-11.h"

// interface v3d
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#include <QBasicTimer>
#include <QPolygonF>
//#include <QMainWindow>
#include <qgroupbox.h>

#include "volimg_proc.h"

#include <QMutex>
#include <QQueue>

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(saveas_to_rawdata, SAVEAS_TO_RAWDATAlugin);


//plugin funcs
const QString title = "saveas_to_rawdata";
QStringList SAVEAS_TO_RAWDATAlugin::menulist() const
{
    return QStringList() 
	<<tr("saveas_to_rawdata")
	<<tr("Help");
}

void SAVEAS_TO_RAWDATAlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("saveas_to_rawdata"))
	{
    	saveas_to_rawdata(callback, parent,1 );
    }
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Neuronseg Plugin 1.0 developed by Jinzhu Yang. (Peng Lab, Janelia Research Farm Campus, HHMI), save data files to raw format ");
		return;
	}
}

void saveas_to_rawdata(V3DPluginCallback &callback, QWidget *parent, int method_code)
{
		
	QString fn_img = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the directory including all tiled images "),
											   QDir::currentPath(),
											   QFileDialog::ShowDirsOnly);
	QDir dir(fn_img);

	QStringList list = dir.entryList();
	
	QStringList imgSuffix;	
	
	imgSuffix<<"*.tif"<<"*.lsm"
	<<"*.TIF"<<"*.LSM";
	
//	for(int i=0; i<list.size();i++)
//	{
//		QStringList filelist;
//		
//		filelist.clear();
//		
//		// get the image files namelist in the directory
//		
//		foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
//		{
//			filelist += QFileInfo(dir, file).absoluteFilePath();
//		}
//		
//		qDebug()<<"filelist== "<<filelist;
//		
//		for(int j=0; j<filelist.size();j++)
//		{	
//			QString curFilePath = QFileInfo(filelist.at(j)).path();
//			
//			QString name = QFileInfo(filelist.at(j)).fileName(); 
//
//			void *pData = NULL;
//			
//			ImagePixelType datatype;
//			
//			string fn_sub;
//			
//			QString curPath = curFilePath+"/";
//			
//			qDebug() << "image filel: name: "<< filelist.at(j)<<name;
//			
//			qDebug() << "curPath ===== " << curPath ; // 
//			
//			string fn = filelist.at(j).toStdString();
//
//			string ff = name.toStdString();
//			
//			qDebug()<<"fn="<<fn.c_str();
//			
//			int i = ff.find(".", 0 ); 
//			
//			fn_sub=ff.substr(0,i);
//			
//			char * imgSrcFile = const_cast<char *>(fn.c_str());
//			
//			qDebug()<<"fn_sub="<<fn_sub.c_str();
//			
//			V3DLONG *sz = 0; 
//			
//			int datatype_relative = 0;
//			
//			unsigned char* tmpData = 0;
//			
//			int pixelnbits=1; //100817		
//			
//			if(loadTif2StackMylib(imgSrcFile,tmpData,sz,datatype_relative,pixelnbits))
//			//if(loadImage(imgSrcFile,tmpData,sz,datatype_relative)!=true)
//			{
//				QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
//				return;
//			}
//			if(datatype_relative ==1)
//			{
//				datatype = V3D_UINT8;
//			}
//			else if(datatype_relative == 2)
//			{
//				datatype = V3D_UINT16;
//			}
//			else if(datatype_relative==4)
//			{
//				datatype = V3D_FLOAT32;
//			}
//			
//			QString tmp_filename = curPath;
//			
//			tmp_filename.append(QString(fn_sub.c_str()));
//			
//			tmp_filename += ".raw";	
//			
//			qDebug()<<"tmp_filename=="<<tmp_filename;
//			
//			//	sz_tmp[0] = vx; sz_tmp[1] = vy; sz_tmp[2] = vz; sz_tmp[3] = vc; 
//			
//			if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)tmpData, sz, datatype)!=true)
//			{
//				fprintf(stderr, "Error happens in file writing. Exit. \n");
//				return ;
//			}
//		}
//		
//	}
	
	QStringList rootlist;
    
	foreach (QString subDir,dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot))
	{
		rootlist += QFileInfo(dir, subDir).absoluteFilePath();
	}
	
	for(int i=0; i<rootlist.size();i++)
	{
		int childCount =0;
		QString tmpdir = rootlist.at(i);
		QDir thisDir(tmpdir);
		childCount = thisDir.entryInfoList().count();
		QFileInfoList newFileList = thisDir.entryInfoList();
		if(childCount>2)
		{
			QString str = QFileInfo(rootlist.at(i)).fileName(); 
			str+=".raw";
			dir.mkdir(str);
		}
	}
	
	for(int i=0; i<rootlist.size();i++)
	{
		
		QStringList myList;
		
		myList.clear();
	
		QDir dir1(rootlist.at(i));
		
		if (!dir1.exists())
		{
			qWarning("Cannot find the directory");
			return ;
		}
		
		foreach (QString file, dir1.entryList(imgSuffix, QDir::Files, QDir::Name))
		{
			myList += QFileInfo(dir1, file).absoluteFilePath();
		}
		
		qDebug()<<"lisitsize== "<<myList.size();
		
		//qDebug()<<"dirname= "<<rootlist.at(i);
		
		foreach (QString qs, myList) qDebug() << qs;
		
	    for(int j=0; j<myList.size();j++)
		{	
			//char * curFileSuffix = getSurfix(const_cast<char *>(myList.at(j).c_str()));		
		
			QString curFilePath = QFileInfo(myList.at(j)).path();
			
			QString name = QFileInfo(myList.at(j)).fileName(); 
			
		//	curFilePath.append("/");
			
			void *pData = NULL;
			
			ImagePixelType datatype;
			
			string fn_sub;
						
			QString curPath = curFilePath;
			
			qDebug() << "image filel: name: "<< myList.at(j)<<name;
			
			//qDebug() << "suffix ... " << curFileSuffix; // 
			
			qDebug() << "curPath ... " << curPath ; // 
			
			curPath = curPath+".raw"+"/";
			
			qDebug() << "curPath ===== " << curPath ; // 
			
			string fn = myList.at(j).toStdString();
			
			string ff = name.toStdString();
			
			qDebug()<<"fn="<<fn.c_str();
			
			int i = ff.find(".", 0 ); 
			
			fn_sub=ff.substr(0,i);
			
			char * imgSrcFile = const_cast<char *>(fn.c_str());
			
			qDebug()<<"fn_sub="<<fn_sub.c_str();
			
			//printf("i=%ld\n",i);
			
			V3DLONG *sz = 0; 
			
			int datatype_relative = 0;
			
			unsigned char* tmpData = 0;
			
			int pixelnbits=1; 		
			
			if(loadTif2StackMylib(imgSrcFile,tmpData,sz,datatype_relative,pixelnbits))
			//if(loadImage(imgSrcFile,tmpData,sz,datatype_relative)!=true)
			{
				QMessageBox::information(0, "Load Image", QObject::tr("File load failure"));
				return;
			}
			if(datatype_relative ==1)
			{
				datatype = V3D_UINT8;
			}
			else if(datatype_relative == 2)
			{
				datatype = V3D_UINT16;
			}
			else if(datatype_relative==4)
			{
				datatype = V3D_FLOAT32;
			}
			
			QString tmp_filename = curPath;
			
			tmp_filename.append(QString(fn_sub.c_str()));
			
			tmp_filename += ".raw";	
			
			qDebug()<<"tmp_filename=="<<tmp_filename;
			
			if (saveImage(tmp_filename.toStdString().c_str(), (const unsigned char *)tmpData, sz, datatype)!=true)
			{
				fprintf(stderr, "Error happens in file writing. Exit. \n");
				return ;
			}
			if(tmpData) {delete []tmpData; tmpData=0;}
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
