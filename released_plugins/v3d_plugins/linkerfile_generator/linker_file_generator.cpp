// generate an ano file for all swc files in the given directory
// by Lei Qu
// 2009-12-30
// rewritten by Hanchuan Peng, 2010-04-20. add other V3D file types


#include <QtGui>
#include <stdlib.h>
#include "v3d_message.h"

#include "linker_file_generator.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(linker_file_generator, LinkerPlugin);

void GeneratorAno4FilesInDir(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

QStringList LinkerPlugin::menulist() const
{
    return QStringList()
	<< tr("for all SWC (e.g. neuron) files under a directory")
	<< tr("for all APO (point cloud) files under a directory")
	<< tr("for all TIF images under a directory")
	<< tr("for all LSM images under a directory")
	<< tr("for all V3D Raw images under a directory")
	<< tr("for all images (*.tif, *.tiff, *.raw) under a directory")
	<< tr("for all V3D-recognizable files under a directory");
}


void LinkerPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("for all SWC (e.g. neuron) files under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 1);
	else if (menu_name == tr("for all APO (point cloud) files under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 2);
	else if (menu_name == tr("for all TIF images under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 3);
	else if (menu_name == tr("for all LSM images under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 4);
	else if (menu_name == tr("for all V3D Raw images under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 5);
	else if (menu_name == tr("for all images (*.tif, *.tiff, *.lsm, *.raw) under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 6);
	else if (menu_name == tr("for all V3D-recognizable files under a directory"))
		GeneratorAno4FilesInDir(callback, parent, 7);
	else
		v3d_msg("Un-recognizable menu detected");
}

void GeneratorAno4FilesInDir(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	//choose a directory that contain swc files
	QString qs_dir_swc;
	qs_dir_swc=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the directory that contains the specified files")));
	
	//find all swc filename in given directory
	QDir dir(qs_dir_swc);
	QStringList qsl_filelist,qsl_filters;
	
	switch (method_code)
	{
		case 1:
			qsl_filters+="*.swc";
			break;
		case 2:
			qsl_filters+="*.apo";
			break;
		case 3:
			qsl_filters+="*.tif";
			qsl_filters+="*.tiff";
			break;
		case 4:
			qsl_filters+="*.lsm";
			break;
		case 5:
			qsl_filters+="*.raw";
			break;
		case 6:
			qsl_filters+="*.tif";
			qsl_filters+="*.tiff";
			qsl_filters+="*.lsm";
			qsl_filters+="*.raw";
			break;
		case 7:
			qsl_filters+="*.tif";
			qsl_filters+="*.tiff";
			qsl_filters+="*.lsm";
			qsl_filters+="*.raw";
			qsl_filters+="*.swc";
			qsl_filters+="*.apo";
			qsl_filters+="*.v3ds";
			qsl_filters+="*.obj";
			break;
		default:
			v3d_msg("You should never see this, - check with the developer of this plugin.");
			break;
	}
	
	foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
	{
		qsl_filelist+=file;
	}
	
	if(qsl_filelist.size()==0)
	{
		v3d_msg("Cannot find the respective files in the given directory!\nTry another diretory");
		return;
	}
	
	//generator ano file
	QString qs_filename_out=qs_dir_swc+"/mylinker.ano";
	QFile qf_anofile(qs_filename_out);
	if(!qf_anofile.open(QIODevice::WriteOnly))
	{
		v3d_msg("Cannot open file for writing!");
		return;
	}
	
	QTextStream out(&qf_anofile);
	for(V3DLONG i=0;i<qsl_filelist.size();i++)
	{
		QString temp;
			
		//see ../basic_c_fun/basic_surf_objs.cpp for keywords
	
		QFileInfo curfile_info(qsl_filelist[i]);
		if (curfile_info.suffix().toUpper()=="SWC")
			temp = qsl_filelist[i].prepend("SWCFILE=");
		else if (curfile_info.suffix().toUpper()=="APO")
			temp = qsl_filelist[i].prepend("APOFILE=");
		else if (curfile_info.suffix().toUpper()=="V3DS")
			temp = qsl_filelist[i].prepend("SURFILE=");
		else if (curfile_info.suffix().toUpper()=="OBJ")
			temp = qsl_filelist[i].prepend("SURFILE=");
		else if (curfile_info.suffix().toUpper()=="RAW")
			temp = qsl_filelist[i].prepend("RAWIMG=");
		else if (curfile_info.suffix().toUpper()=="TIF")
			temp = qsl_filelist[i].prepend("RAWIMG=");
		else if (curfile_info.suffix().toUpper()=="TIFF")
			temp = qsl_filelist[i].prepend("RAWIMG=");
		else if (curfile_info.suffix().toUpper()=="LSM")
			temp = qsl_filelist[i].prepend("RAWIMG=");
		else
			v3d_msg("You should never see this, - check with the developer of this plugin.");
				
		out << temp << endl;
		v3d_msg(qPrintable(temp), 0);
	}
	
	//show message box
	v3d_msg(QString("Save the linker file to: \n\n%1\n\nComplete!").arg(qs_filename_out));
}

