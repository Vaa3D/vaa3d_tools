/*
 * Copyright (c)2011  Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 % Dec 30, 2011
 */


#include "dialog_pointcloudatlas_adjust_preference.h"


#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

// -----------------------------------
// functions of the input interface
// -----------------------------------

PointCloudAtlas_AdjustPreferenceDialog::PointCloudAtlas_AdjustPreferenceDialog(atlasConfig &p)
{
//	n_apo = 0;
//	nameFilters << "*.apo" << "*.ano.txt";

	create(p);
}

void PointCloudAtlas_AdjustPreferenceDialog::create(atlasConfig &p)
{
	setupUi(this);
	
	//set default values and events
	lineEdit_input_folder->setText(p.inputFolder);
	lineEdit_output_folder->setText(p.outputFolder);
	lineEdit_cellname_file_prefix->setText(p.cellNameFilePrefix);
	lineEdit_cellname_file_surfix->setText(p.cellNameFileSurfix);
//	lineEdit_refmarkercpt_file->setText(p.refMarkerCptCellNameFile);
	lineEdit_celltype_file->setText("");
    
//    if (p.saveRegDataTag == true)
//        radioButton_saveregtag->setChecked(true);
//    else
//        radioButton_saveregtag->setChecked(false);
    
    for (int i=0; i<p.cellTypeFileName.size(); i++)
        listWidget_existing_celltype_files->addItem(p.cellTypeFileName.at(i));
    
    
	lineEdit_cellstat_file_surfix->setText(p.cellStatFileSurfix);
//	lineEdit_coexp_file_surfix->setText(p.coexpressFileSurfix);    
	lineEdit_markermap_file->setText(p.markerMapFileName);
	
	//set connection
    
	QFileInfo info;
    
	connect(lineEdit_input_folder, SIGNAL(textChanged(const QString &)), this, SLOT(change_input_folder(const QString &)));
	connect(pushButton_input_folder, SIGNAL(clicked()), this, SLOT(select_input_folder()));
    
	connect(lineEdit_output_folder, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_folder(const QString &)));
	connect(pushButton_output_folder, SIGNAL(clicked()), this, SLOT(select_output_folder()));
    
//	connect(lineEdit_cellname_file_prefix, SIGNAL(textChanged(const QString &)), this, SLOT(change_cellname_file_prefix(const QString &)));
// 	connect(lineEdit_cellname_file_surfix, SIGNAL(textChanged(const QString &)), this, SLOT(change_cellname_file_surfix(const QString &)));
   

//	connect(lineEdit_refmarkercpt_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_refmarkercpt_file(const QString &)));
//	connect(pushButton_refmarkercpt_file, SIGNAL(clicked()), this, SLOT(select_refmarkercpt_file()));
    
    connect(lineEdit_celltype_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_celltype_file(const QString &)));
	connect(pushButton_celltype_file, SIGNAL(clicked()), this, SLOT(select_celltype_file()));
	connect(pushButton_remove_celltype_files, SIGNAL(clicked()), this, SLOT(remove_celltype_file()));
    
    
 //   connect(radioButton_saveregtag, SIGNAL(isChecked()), this, SLOT(change_saveregtag(const QString &)));

//	connect(lineEdit_cellstat_file_surfix, SIGNAL(textChanged(const QString &)), this, SLOT(change_cellstat_file_surfix(const QString &)));
// 	connect(lineEdit_coexp_file_surfix, SIGNAL(textChanged(const QString &)), this, SLOT(change_coexp_file_surfix(const QString &)));
// 	connect(lineEdit_markermap_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_markermap_file(const QString &)));
    
    
    // button for ending the window
	connect(pushButton_finish, SIGNAL(clicked()), this, SLOT(accept())); //this will get a result code "Accepted"
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    //chekc completeness enable or not the finish button
	check_completeness_of_info();
    
}


void PointCloudAtlas_AdjustPreferenceDialog::fetchData(atlasConfig &p)
{
    
    
    p.inputFolder = lineEdit_input_folder->text().trimmed();
    p.outputFolder = lineEdit_output_folder->text().trimmed();
    p.cellNameFilePrefix = lineEdit_cellname_file_prefix->text().trimmed();
    p.cellNameFileSurfix = lineEdit_cellname_file_surfix->text().trimmed();
//    p.refMarkerCptCellNameFile = lineEdit_refmarkercpt_file->text().trimmed();

//    if (radioButton_saveregtag->isChecked()==true)
//        p.saveRegDataTag = 1;
//    else
//        p.saveRegDataTag = 0;
    
    
    p.cellStatFileSurfix = lineEdit_cellstat_file_surfix->text().trimmed();
//    p.coexpressFileSurfix = lineEdit_coexp_file_surfix->text().trimmed();
    p.markerMapFileName = lineEdit_markermap_file->text().trimmed();
    
    
    // add cell type file
    p.cellTypeFileName.clear();
        
    int num = listWidget_existing_celltype_files->count();
    
    for (int i=0; i<num; i++)
    {
        
        QString a;
        a = listWidget_existing_celltype_files->item(i)->text().trimmed();
        printf("%s\n",qPrintable(a));
        p.cellTypeFileName.append(a);
        
    }
}


void PointCloudAtlas_AdjustPreferenceDialog::change_input_folder(const QString & s) 
{
	QFileInfo info(s.trimmed());
    
	if (!info.exists() || !info.isDir())
	{	
		label_input_folder->setText("<font color=\"red\">Input folder: </font>"); 
		lineEdit_input_folder->setText("Folder is invalid.");
	}
	else
	{
		label_input_folder->setText("<font color=\"black\">Input folder: </font>");		//FL 20091026
		QDir d(info.absoluteFilePath());
	}	
    	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_AdjustPreferenceDialog::select_input_folder()
{
	QString s;
	QFileInfo info(lineEdit_input_folder->text().trimmed());
	s = QFileDialog::getExistingDirectory(0, tr("select the input folder:"), info.dir().path()); //FL, 20091026
	
	if (!s.isEmpty())
	{
		lineEdit_input_folder->setText(s);
	}
}


void PointCloudAtlas_AdjustPreferenceDialog::change_output_folder(const QString & s) 
{
	QFileInfo info(s.trimmed());
    
	if (!info.exists() || !info.isDir())
	{	
		label_output_folder->setText("<font color=\"red\">Output folder </font>"); // FL 20091026
		lineEdit_output_folder->setText("Folder is invalid.");
	}
	else
	{
		label_output_folder->setText("<font color=\"black\">Output folder </font>");		//FL 20091026
		QDir d(info.absoluteFilePath());
	}	
    
	
	//enable or not the finish button
	check_completeness_of_info();
}

void PointCloudAtlas_AdjustPreferenceDialog::select_output_folder()
{
	QString s;
	QFileInfo info(lineEdit_output_folder->text().trimmed());
	s = QFileDialog::getExistingDirectory(0, tr("Select the output folder:"), info.dir().path()); 
	
	if (!s.isEmpty())
	{
		lineEdit_output_folder->setText(s);
	}
}




//void PointCloudAtlas_AdjustPreferenceDialog::change_refmarkercpt_file(const QString & s) 
//{
//	QFileInfo info(s.trimmed());
//	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
//		label_refmarkercpt_file->setText("<font color=\"red\">Reference marker control point file for registration: </font>");   
//	else
//		label_refmarkercpt_file->setText("Reference marker control point file for registration: "); 
//	
//	//enable or not the finish button
//	check_completeness_of_info();
//}
//
//
//void PointCloudAtlas_AdjustPreferenceDialog::select_refmarkercpt_file()
//{
//	QString s;
//	QFileInfo info(lineEdit_refmarkercpt_file->text().trimmed());
//	s = QFileDialog::getOpenFileName(0, tr("Select the reference marker control point file for registration:"));
//	if (!s.isEmpty())
//	{
//		lineEdit_refmarkercpt_file->setText(s);
//	}
//}



void PointCloudAtlas_AdjustPreferenceDialog::change_celltype_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_celltype_file->setText("<font color=\"red\">Cell type file:  </font>");  
	else
		label_celltype_file->setText("Cell type file: "); 
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_AdjustPreferenceDialog::select_celltype_file()
{
	QString s;
	QFileInfo info(lineEdit_celltype_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select the cell type file: "));
	if (!s.isEmpty())
	{
		lineEdit_celltype_file->setText(s);
        listWidget_existing_celltype_files->addItem(s);
        
	}
}



void PointCloudAtlas_AdjustPreferenceDialog::remove_celltype_file()
{
    
    int a=listWidget_existing_celltype_files->currentRow();
    delete listWidget_existing_celltype_files->takeItem(a);
    
}


void PointCloudAtlas_AdjustPreferenceDialog::check_completeness_of_info()
{
	bool b_info_complete=true;
	QFileInfo info;
    
	if (lineEdit_input_folder->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_input_folder->setText("<font color=\"red\">Input folder: </font>");	//FL 20091026	
    }
    else
    {
 		label_input_folder->setText("<font color=\"black\">Input folder: </font>");	//FL 20091026	
    }  
    
	if (lineEdit_output_folder->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_output_folder->setText("<font color=\"red\">Output folder:</font>");		
    }
    else
    {
 		label_output_folder->setText("<font color=\"black\">Output folder:</font>");		
    }  
    
    
    if (lineEdit_cellname_file_prefix->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_cellname_file_prefix->setText("<font color=\"red\">Cell name file prefix:</font>");		
    }
    else
    {
 		label_cellname_file_prefix->setText("<font color=\"black\">Cell name file prefix:</font>");		
    }  


    if (lineEdit_cellname_file_surfix->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_cellname_file_surfix->setText("<font color=\"red\">Cell name file suffix:</font>");		
    }
    else
    {
 		label_cellname_file_surfix->setText("<font color=\"black\">Cell name file suffix:</font>");		
    }  
    
//    if (lineEdit_refmarkercpt_file->text().trimmed().isEmpty()) 
//    {
//        b_info_complete=false;
//		label_refmarkercpt_file->setText("<font color=\"red\">Reference marker control point file for registration:</font>");		
//    }
//    else
//    {
// 		label_refmarkercpt_file->setText("<font color=\"black\">Reference marker control point file for registration:</font>");		
//    }  
    
    if(listWidget_existing_celltype_files->count() == 0)
    {
        b_info_complete=false;
		label_celltype_file->setText("<font color=\"red\">Cell type file: </font>");	
    }
    else
    {
 		label_celltype_file->setText("<font color=\"black\">Cell type file: </font>");		
    }  

    if (lineEdit_cellstat_file_surfix->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_cellstat_file_surfix->setText("<font color=\"red\">Cell statistics file suffix:</font>");		
    }
    else
    {
 		label_cellstat_file_surfix->setText("<font color=\"black\">Cell statistics file suffix:</font>");		
    }  

//    if (lineEdit_coexp_file_surfix->text().trimmed().isEmpty()) 
//    {
//        b_info_complete=false;
//		label_coexp_file_surfix->setText("<font color=\"red\">Coexpression file surfix:</font>");		
//    }
//    else
//    {
// 		label_coexp_file_surfix->setText("<font color=\"black\">Coexpression file surfix:</font>");		
//    }  
        
    if (lineEdit_markermap_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_markermap_file->setText("<font color=\"red\">Marker map file:</font>");		
    }
    else
    {
 		label_markermap_file->setText("<font color=\"black\">Marker map file:</font>");		
    }  
       
        
        
	printf("b_info_complete=%d\n", b_info_complete);
	
	//set the two push button states
	pushButton_finish->setEnabled(b_info_complete);
    
    
}






