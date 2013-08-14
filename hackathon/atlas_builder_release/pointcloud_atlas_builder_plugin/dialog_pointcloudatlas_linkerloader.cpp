/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/****************************************************************************
 **
 dialog_pointcloudatlas_linkerloader.cpp
 by Hanchuan Peng
 2009_May-17
 
 revised by Fuhui Long on Oct 26, 2009, changed the user interface to make it
 more intuitive and straightforward
 **
 ****************************************************************************/

#include "dialog_pointcloudatlas_linkerloader.h"

#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

PointCloudAtlas_LinkerLoadDialog::PointCloudAtlas_LinkerLoadDialog(apoAtlasLinkerInfoAll &p)
{
	n_apo_sig = n_apo_ref = 0;
	nameFilters << "*.apo" << "*.ano.txt";

	create(p);
}

void PointCloudAtlas_LinkerLoadDialog::fetchData(apoAtlasLinkerInfoAll &p)
{
	apoAtlasLinkerInfo a;
	a.className = lineEdit_category_name->text().trimmed();
	a.sigFolder = lineEdit_anofolder_sig->text().trimmed();
	a.refFolder = lineEdit_anofolder_ref->text().trimmed();
//	a.registeredFile = lineEdit_output_file->text().trimmed();
	//	a.registeredFile = "";
	
	QFileInfo info;
	QDir d;
	QFileInfoList refFolderFileListTmp;
	QFileInfoList sigFolderFileListTmp;
	
	info.setFile(a.sigFolder);
	if (info.isDir())
	{
		d.setPath(info.absoluteFilePath());
		a.sigFolderFileList = d.entryList(nameFilters, QDir::Files, QDir::Name);
		sigFolderFileListTmp = d.entryInfoList(nameFilters, QDir::Files, QDir::Name);

	}
	else
		a.sigFolderFileList.clear();

	info.setFile(a.refFolder);
	if (info.isDir())
	{

		a.refFolderFileList.clear(); // FL 20091103
		
		d.setPath(info.absoluteFilePath());
//		a.refFolderFileList = d.entryList(nameFilters, QDir::Files, QDir::Name);
		
		refFolderFileListTmp = d.entryInfoList(nameFilters, QDir::Files, QDir::Name);

		// FL, 20091103 
		bool matchTag = 1;
		
		for (int i=0; i<sigFolderFileListTmp.size(); i++)
		{
			
//			bool hitTag = 0;
			for (int j=0; j<refFolderFileListTmp.size(); j++)
			{
				if(sigFolderFileListTmp.at(i).baseName().compare(refFolderFileListTmp.at(j).baseName())==0)			
				{
//					hitTag = 1; 
					a.refFolderFileList << refFolderFileListTmp.at(j).fileName();

					break;
					printf("%s, %s\n", qPrintable(a.sigFolderFileList.at(i)), qPrintable(a.refFolderFileList.at(i)));
				}
			}
		}
		
	}
	else
		a.refFolderFileList.clear();

	//now append the new data
//	p.regTargetFileName = lineEdit_targetref_file->text().trimmed();	
	p.referenceMarkerName = lineEdit_common_referencemarker->text().trimmed();
	
	p.items.append(a);
}

void PointCloudAtlas_LinkerLoadDialog::create(apoAtlasLinkerInfoAll &p)
{
	setupUi(this);
	
	connect(pushButton_finish, SIGNAL(clicked()), this, SLOT(accept())); //this will get a result code "Accepted"
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(pushButton_addanother, SIGNAL(clicked()), this, SLOT(done())); //call the overloaded done() slot, which will return a rescode 2

	//default values and events
	lineEdit_category_name->setText("");
	lineEdit_anofolder_sig->setText("");
	lineEdit_anofolder_ref->setText("");
//	lineEdit_output_file->setText("");
	lineEdit_common_referencemarker->setText("");

	connect(lineEdit_common_referencemarker, SIGNAL(textChanged(const QString &)), this, SLOT(check_completeness_of_info()));	
	connect(lineEdit_category_name, SIGNAL(textChanged(const QString &)), this, SLOT(check_completeness_of_info()));
//	connect(lineEdit_output_file, SIGNAL(textChanged(const QString &)), this, SLOT(check_completeness_of_info()));
	
	label_info_num_apo_files->hide();

	connect(lineEdit_anofolder_sig, SIGNAL(textChanged(const QString &)), this, SLOT(change_sigfolder(const QString &)));
	connect(lineEdit_anofolder_ref, SIGNAL(textChanged(const QString &)), this, SLOT(change_reffolder(const QString &)));

	connect(pushButton_sig_folder, SIGNAL(clicked()), this, SLOT(select_sigfolder()));
	connect(pushButton_ref_folder, SIGNAL(clicked()), this, SLOT(select_reffolder()));
//	connect(pushButton_output_file, SIGNAL(clicked()), this, SLOT(select_output_file()));
	
	//set the common reference file
	QFileInfo info;

//	connect(lineEdit_targetref_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_targetref_file(const QString &)));
//	connect(pushButton_common_ref_file, SIGNAL(clicked()), this, SLOT(select_targetref_file()));
	
//	lineEdit_targetref_file->setText(p.regTargetFileName);
//	info.setFile(p.regTargetFileName);
//	if (!p.regTargetFileName.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
////		label_common_targetref->setText("<font color=\"red\">Common target pointcloud reference file</font>");
//		label_common_targetref->setText("<font color=\"red\">Target file for registration</font>");	//FL 20091027	
//	else
////		label_common_targetref->setText("Common target pointcloud reference file");
//		label_common_targetref->setText("Target file for registration "); // FL 20091027
	
	//set the color of the text labels

	//FL, 20091030
	if (lineEdit_common_referencemarker->text().trimmed().isEmpty())
		label_common_referencemarker->setText("<font color=\"red\">Common reference (e.g. gene name EVE)</font>"); 
	else
		label_common_referencemarker->setText("<font color=\"black\">Common reference (e.g. gene name EVE)</font>"); 
	
	if (lineEdit_category_name->text().trimmed().isEmpty())
//		label_classname->setText("<font color=\"red\">Class (e.g. gene name)</font>");
		label_classname->setText("<font color=\"red\">Signal name (e.g. gene name)</font>"); //FL, 20091026
	else
//		label_classname->setText("<font color=\"black\">Class (e.g. gene name)</font>");
		label_classname->setText("<font color=\"black\">Signal name (e.g. gene name)</font>"); //FL 20091026

	info.setFile(lineEdit_anofolder_sig->text().trimmed());
	if (!info.exists() || !info.isDir())
//		label_sigclass->setText("<font color=\"red\">Folder of signal annotation of this class</font>");
		label_sigclass->setText("<font color=\"red\">Selected signal channel folder</font>"); //FL 20091026		
	else
//		label_sigclass->setText("<font color=\"black\">Folder of signal annotation of this class</font>");
		label_sigclass->setText("<font color=\"black\">Selecteed signal folder</font>"); //FL 20091026

	info.setFile(lineEdit_anofolder_ref->text().trimmed());
	if (!info.exists() || !info.isDir())
//		label_refclass->setText("<font color=\"red\">Folder of reference annotations of this class</font>");
		label_refclass->setText("<font color=\"red\">Reference channel folder</font>"); // FL 20091026
	else
//		label_refclass->setText("<font color=\"black\">Folder of reference annotations of this class</font>");
		label_refclass->setText("<font color=\"black\">Selected reference folder</font>"); //FL 20091026

//	info.setFile(lineEdit_output_file->text().trimmed());
//	if (lineEdit_output_file->text().trimmed().isEmpty() && (!info.dir().exists())) //since the output file may or may not exist, thus only verify if the parent dir exists
////		label_output_file->setText("<font color=\"red\">Output pointcloud file of this class</font>");
//		label_output_file->setText("<font color=\"red\">Output pointcloud file of this signal</font>");	// FL 20091027
//	else
////		label_output_file->setText("Output pointcloud file of this class");
//		label_output_file->setText("Output pointcloud file of this signal");
	
	//set the # of apo file lines
	lineEdit_anofolder_sig_nfiles->setReadOnly(true);
	lineEdit_anofolder_ref_nfiles->setReadOnly(true);
	
	//enable or not the finish button
	check_completeness_of_info();
	
	//finally display some information of the existing atlas info structure
	QString s_exist_class, tmps;
	for (int i=0;i<p.items.size();i++)
	{
		if (i==0) s_exist_class = p.items.at(i).className;
		else
			s_exist_class.append(", ").append(p.items.at(i).className);
	}
//	tmps.setNum(p.items.size()); tmps.prepend("Previous existing (").append(") class(es) are: ");
	tmps.setNum(p.items.size()); tmps.prepend("Previous existing (").append(") signal(s) are: "); //FL, 20091027
	
	s_exist_class.prepend(tmps);
	label_info_existing_classes->setText(s_exist_class);
}

void PointCloudAtlas_LinkerLoadDialog::change_sigfolder(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!info.exists() || !info.isDir())
	{	
//		label_sigclass->setText("<font color=\"red\">Annotation folder of this category</font>");
		label_sigclass->setText("<font color=\"red\">Signal folder </font>"); // FL 20091026
		lineEdit_anofolder_sig_nfiles->setText("Folder is invalid.");
		n_apo_sig=0;
	}
	else
	{
//		label_sigclass->setText("<font color=\"black\">Annotation folder of this category</font>");
		label_sigclass->setText("<font color=\"black\">Signal folder </font>");		//FL 20091026
		QDir d(info.absoluteFilePath());
		n_apo_sig = d.entryList(nameFilters, QDir::Files, QDir::Name).size();
		
		fileList_apo_sig = d.entryInfoList(nameFilters, QDir::Files, QDir::Name); //FL 20091028
		
		QString tmps; tmps.setNum(n_apo_sig);
		tmps.prepend("Contains ").append(" point cloud (*.apo) files");
		lineEdit_anofolder_sig_nfiles->setText(tmps);
	}	

	// PHC's method, check if the number of stacks in the signal and reference channels match
	// It has problem when the number of stacks in the signal channel is less than the number of 
	// stacks in the reference stack
//	if (n_apo_sig>0 && n_apo_ref>0 && n_apo_sig==n_apo_ref)
////		label_info_num_apo_files->setText("<font color=\"black\">Now the # of annotation files match!</font>");
//		label_info_num_apo_files->setText("<font color=\"black\">Now the # of files match!</font>"); //FL 20091026	
//	else
////		label_info_num_apo_files->setText("<font color=\"red\">The # of annotation files do NOT match!</font>");
//		label_info_num_apo_files->setText("<font color=\"red\">The # of files do NOT match!</font>");	// FL 20091026	
//	label_info_num_apo_files->show();

	// FL, 20091028, check if the name of files match
	bool matchTag = 1;
	
	for (int i=0; i<n_apo_sig; i++)
	{
		
		bool hitTag = 0;
		for (int j=0; j<n_apo_ref; j++)
		{
//			if(fileList_apo_sig.at(i).fileName().compare(fileList_apo_ref.at(j).fileName())==0)
			if(fileList_apo_sig.at(i).baseName().compare(fileList_apo_ref.at(j).baseName())==0)			
			{
				hitTag = 1; break;
			}
		}
		
		if (hitTag == 0)
			matchTag = 0;
	}
	
	if (matchTag ==0)
		label_info_num_apo_files->setText("<font color=\"red\">Some signal files cannnot find matched reference files!</font>");
	else
		label_info_num_apo_files->setText("<font color=\"black\">Signal files and reference files match!</font>");
	label_info_num_apo_files->show();
	
	//enable or not the finish button
	check_completeness_of_info();
}

void PointCloudAtlas_LinkerLoadDialog::change_reffolder(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!info.exists() || !info.isDir())
	{	
//		label_refclass->setText("<font color=\"red\">Annotation folder of this category</font>");
		label_refclass->setText("<font color=\"red\">Reference folder</font>");		// FL 20091026
		lineEdit_anofolder_ref_nfiles->setText("Folder is invalid");
		n_apo_ref=0;
	}
	else
	{
//		label_refclass->setText("<font color=\"black\">Annotation folder of the reference class</font>");
		label_refclass->setText("<font color=\"black\">Reference folder </font>");	// FL 20091026
		QDir d(info.absoluteFilePath());
		n_apo_ref = d.entryList(nameFilters, QDir::Files, QDir::Name).size();
		
		fileList_apo_ref = d.entryInfoList(nameFilters, QDir::Files, QDir::Name); //FL 20091028
		
		QString tmps; tmps.setNum(n_apo_ref);
		tmps.prepend("Contains ").append(" point cloud (*.apo) files");
		lineEdit_anofolder_ref_nfiles->setText(tmps);
	}	

//	if (n_apo_sig>0 && n_apo_ref>0 && n_apo_sig==n_apo_ref)
////		label_info_num_apo_files->setText("<font color=\"black\">Now the # of annotation files match!</font>");
//		label_info_num_apo_files->setText("<font color=\"black\">Now the # of files match!</font>"); //FL 20091026	
//	else
////		label_info_num_apo_files->setText("<font color=\"red\">The # of annotation files do NOT match!</font>");
//		label_info_num_apo_files->setText("<font color=\"red\">The # of files do NOT match!</font>");// FL 20091026	
//	label_info_num_apo_files->show();

	// FL, 20091028, check if the name of files match
	bool matchTag = 1;

	for (int i=0; i<n_apo_sig; i++)
	{

		bool hitTag = 0;
		for (int j=0; j<n_apo_ref; j++)
		{
			if(fileList_apo_sig.at(i).baseName().compare(fileList_apo_ref.at(j).baseName())==0)
			{
				hitTag = 1; break;
			}
		}
		
		if (hitTag == 0)
			matchTag = 0;
	}
	
	if (matchTag ==0)
		label_info_num_apo_files->setText("<font color=\"red\">Some signal files cannnot find matched reference files!</font>");
	else
		label_info_num_apo_files->setText("<font color=\"black\">Signal files and reference files match!</font>");
	label_info_num_apo_files->show();
	
	//enable or not the finish button
	
	check_completeness_of_info();
}

//void PointCloudAtlas_LinkerLoadDialog::change_targetref_file(const QString & s) 
//{
//	QFileInfo info(s.trimmed());
//	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
////		label_common_targetref->setText("<font color=\"red\">Common target pointcloud reference file</font>");
//		label_common_targetref->setText("<font color=\"red\">Target file for registration </font>"); //FL 20091027
//	else
////		label_common_targetref->setText("Common target pointcloud reference file");
//		label_common_targetref->setText("Target file for registration "); //FL 20091027
//	
//	//enable or not the finish button
//	check_completeness_of_info();
//}

//void PointCloudAtlas_LinkerLoadDialog::change_output_file(const QString & s) 
//{
//	QFileInfo info(s.trimmed());
//	if (!s.trimmed().isEmpty() && (!info.dir().exists()))
////		label_output_file->setText("<font color=\"red\">Output pointcloud file of this class</font>");
//		label_output_file->setText("<font color=\"red\">Output file of this signal </font>");	//FL 20091027	
//	else
////		label_output_file->setText("Output pointcloud file of this class");
//		label_output_file->setText("Output file of this signal"); //FL 20091027
//	
//	//enable or not the finish button
//	check_completeness_of_info();
//}


void PointCloudAtlas_LinkerLoadDialog::select_sigfolder() 
{
	QString s;
	QFileInfo info(lineEdit_anofolder_sig->text().trimmed());
//	s = QFileDialog::getExistingDirectory(0, tr("select the folder of the annotation files of this class"), info.dir().path());
	s = QFileDialog::getExistingDirectory(0, tr("select the folder of the annotation files of this signal"), info.dir().path()); //FL, 20091026
	if (!s.isEmpty())
	{
		lineEdit_anofolder_sig->setText(s);
	}
}

void PointCloudAtlas_LinkerLoadDialog::select_reffolder() 
{
	QString s;
	QFileInfo info(lineEdit_anofolder_ref->text().trimmed());
//	s = QFileDialog::getExistingDirectory(0, tr("select the folder of the annotation files of reference class"), info.dir().path());
	s = QFileDialog::getExistingDirectory(0, tr("select the folder of the annotation files of the reference"), info.dir().path()); //FL, 20091026
	if (!s.isEmpty())
	{
		lineEdit_anofolder_ref->setText(s);
	}
}

//void PointCloudAtlas_LinkerLoadDialog::select_targetref_file()
//{
//	QString s;
//	QFileInfo info(lineEdit_targetref_file->text().trimmed());
////	s = QFileDialog::getOpenFileName(0, tr("select the common reference pointcloud annotation file"), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
//	s = QFileDialog::getOpenFileName(0, tr("select the common reference file for registration "), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));	//FL 20091027
//	if (!s.isEmpty())
//	{
//		lineEdit_targetref_file->setText(s);
//	}
//}
//
//void PointCloudAtlas_LinkerLoadDialog::select_output_file()
//{
//	QString s;
//	QFileInfo info(lineEdit_output_file->text().trimmed());
////	s = QFileDialog::getOpenFileName(0, tr("select the output pointcloud file for registered annotation"), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
//	s = QFileDialog::getOpenFileName(0, tr("select the output file for registered point cloud"), info.dir().path(), tr("Pointcloud file (*.apo *.txt)")); //FL 20091027
//	
//	if (!s.isEmpty())
//	{
//		lineEdit_output_file->setText(s);
//	}
//}

void PointCloudAtlas_LinkerLoadDialog::check_completeness_of_info()
{
	bool b_info_complete=true;
	QFileInfo info;

	if (lineEdit_category_name->text().isEmpty()) b_info_complete=false;
	else
	{
		if (lineEdit_common_referencemarker->text().isEmpty()) b_info_complete=false; //FL 20091030
		else
		{
			info.setFile(lineEdit_anofolder_sig->text().trimmed());
			if (!info.isDir()) b_info_complete=false;
			else
			{
				info.setFile(lineEdit_anofolder_ref->text().trimmed());
				if (!info.isDir()) b_info_complete=false;
				else
				{
	//				if (n_apo_sig>0 && n_apo_ref>0 && n_apo_sig==n_apo_ref)
	//					b_info_complete=true;
	//				else
	//					b_info_complete=false;
					

					// FL, 20091028, check if the name of files match
					b_info_complete=true;
					
					for (int i=0; i<n_apo_sig; i++)
					{
						
						bool hitTag = 0;
						for (int j=0; j<n_apo_ref; j++)
						{
//							if(fileList_apo_sig.at(i).fileName().compare(fileList_apo_ref.at(j).fileName())==0)
							printf("%s, %s\n", qPrintable(fileList_apo_sig.at(i).baseName()), qPrintable(fileList_apo_ref.at(j).baseName())); 
							if(fileList_apo_sig.at(i).baseName().compare(fileList_apo_ref.at(j).baseName())==0)							
							{
								hitTag = 1; break;
							}
						}
						
						if (hitTag == 0)
							b_info_complete = false;
					}
					
				}
			}
		}
	}
	
	printf("b_info_complete=%d\n", b_info_complete);

	//set the color of the category_name edit, FL 20091030
	if (lineEdit_common_referencemarker->text().trimmed().isEmpty())
		label_common_referencemarker->setText("<font color=\"red\">Common reference (e.g. gene name EVE)</font>");	//FL 20091026	
	else
		label_common_referencemarker->setText("<font color=\"black\">Common reference (e.g. gene name EVE)</font>");	//FL 20091026	
	
	//set the color of the category_name edit
	if (lineEdit_category_name->text().trimmed().isEmpty())
//		label_classname->setText("<font color=\"red\">Class (e.g. gene name)</font>");
		label_classname->setText("<font color=\"red\">Signal name (e.g. gene name)</font>");	//FL 20091026	
	else
//		label_classname->setText("<font color=\"black\">Class (e.g. gene name)</font>");
		label_classname->setText("<font color=\"black\">Signal name (e.g. gene name)</font>");	//FL 20091026	

//	//check if the output name has been set
//	if (lineEdit_output_file->text().trimmed().isEmpty())
//	{
////		label_output_file->setText("<font color=\"red\">Output pointcloud file of this class</font>");
//		label_output_file->setText("<font color=\"red\">Output file for registered point cloud </font>");
//		
//		b_info_complete = false;
//	}
//	else
////		label_output_file->setText("Output pointcloud file of this class");
//		label_output_file->setText("Output file for registered point cloud");

	printf("b_info_complete=%d\n", b_info_complete);
	
	//set the two push button states
	pushButton_finish->setEnabled(b_info_complete);
	pushButton_addanother->setEnabled(b_info_complete);
}
