/*
 * Copyright (c)2011  Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 % Jan 20, 2012
 */


#include "dialog_pointcloudatlas_build_atlas.h"


#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

// -----------------------------------
// functions of the input interface
// -----------------------------------

PointCloudAtlas_BuildAtlasDialog::PointCloudAtlas_BuildAtlasDialog(apoAtlasBuilderInfo &p)
{

	create(p);
}

void PointCloudAtlas_BuildAtlasDialog::fetchData(apoAtlasBuilderInfo &p)
{
    
    p.linkerFileName = lineEdit_linker_file->text().trimmed();
    p.regTargetFileName = lineEdit_ref_file->text().trimmed();
    p.refMarkerCptCellNameFile = lineEdit_refmarkercpt_file->text().trimmed();
    
    p.ratio = atof(lineEdit_stack_ratio->text().trimmed().toStdString().c_str());
    
    p.forceAddCellFileName = lineEdit_forceadd_celllist_file->text().trimmed();
    
    if (radioButton_saveregtag->isChecked()==true)
        p.saveRegDataTag = 1;
    else
        p.saveRegDataTag = 0;
    
        
}



void PointCloudAtlas_BuildAtlasDialog::create(apoAtlasBuilderInfo &p)
{
	setupUi(this);
	
	//default values and events
	lineEdit_linker_file->setText("");
	lineEdit_ref_file->setText("");
	lineEdit_forceadd_celllist_file->setText("");
    lineEdit_stack_ratio->setText("0");
	   
    if (p.saveRegDataTag == true)
        radioButton_saveregtag->setChecked(true);
    else
        radioButton_saveregtag->setChecked(false);
    

    // select the input linker file
	connect(lineEdit_linker_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_linker_file(const QString &)));
	connect(pushButton_linker_file, SIGNAL(clicked()), this, SLOT(select_linker_file()));
    
    // select the registration target file     
	connect(lineEdit_ref_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_target_file(const QString &)));    
	connect(pushButton_ref_file, SIGNAL(clicked()), this, SLOT(select_target_file()));

    // select the reference marker control point file      
	connect(lineEdit_refmarkercpt_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_refmarkercpt_file(const QString &)));    
	connect(pushButton_refmarkercpt_file, SIGNAL(clicked()), this, SLOT(select_refmarkercpt_file()));
    
    // select the force added cell file      
	connect(lineEdit_forceadd_celllist_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_forceadded_cell_file(const QString &)));    
	connect(pushButton_forceadd_celllist_file, SIGNAL(clicked()), this, SLOT(select_forceadded_cell_file()));
        
    // button for ending the window
	connect(pushButton_ok, SIGNAL(clicked()), this, SLOT(accept())); //this will get a result code "Accepted"
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    //enable or not the finish button
	check_completeness_of_info();
    
}


void PointCloudAtlas_BuildAtlasDialog::change_linker_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_linker_file->setText("<font color=\"red\">Select a linker file:  </font>"); //FL 20091027  
	else
		label_linker_file->setText("Select a linker file: "); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_BuildAtlasDialog::select_linker_file()
{
	QString s;
	QFileInfo info(lineEdit_linker_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select a linker file:"), info.dir().path());
	if (!s.isEmpty())
	{
		lineEdit_linker_file->setText(s);
	}
}


void PointCloudAtlas_BuildAtlasDialog::change_target_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_ref_file->setText("<font color=\"red\">Select a target file for registration:</font>"); //FL 20091027  
	else
		label_ref_file->setText("Select a target file for registration:"); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_BuildAtlasDialog::select_target_file()
{
	QString s;
	QFileInfo info(lineEdit_ref_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select a target file for registration:"), info.dir().path());
	if (!s.isEmpty())
	{
		lineEdit_ref_file->setText(s);
	}
}


void PointCloudAtlas_BuildAtlasDialog::change_forceadded_cell_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_forceadd_celllist_file->setText("<font color=\"red\">Select force added cell file:</font>"); //FL 20091027  
	else
		label_forceadd_celllist_file->setText("Select force added cell file:"); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_BuildAtlasDialog::select_forceadded_cell_file()
{
	QString s;
	QFileInfo info(lineEdit_forceadd_celllist_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select force added cell file:"), info.dir().path());
	if (!s.isEmpty())
	{
		lineEdit_forceadd_celllist_file->setText(s);
	}
}

void PointCloudAtlas_BuildAtlasDialog::change_refmarkercpt_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_refmarkercpt_file->setText("<font color=\"red\">Reference marker control point file for registration: </font>");   
	else
		label_refmarkercpt_file->setText("Reference marker control point file for registration: "); 
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_BuildAtlasDialog::select_refmarkercpt_file()
{
	QString s;
	QFileInfo info(lineEdit_refmarkercpt_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select the reference marker control point file for registration:"));
	if (!s.isEmpty())
	{
		lineEdit_refmarkercpt_file->setText(s);
	}
}



void PointCloudAtlas_BuildAtlasDialog::check_completeness_of_info()
{
	bool b_info_complete=true;
	QFileInfo info;
    
	if (lineEdit_linker_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_linker_file->setText("<font color=\"red\">Select a linker file:</font>");	//FL 20091026	
    }
    else
    {
 		label_linker_file->setText("<font color=\"black\">Select a linker file:</font>");	//FL 20091026	
    }  
    
	if (lineEdit_ref_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_ref_file->setText("<font color=\"red\">Select a target file for registration:</font>");		
    }
    else
    {
 		label_ref_file->setText("<font color=\"black\">Select a target file for registration:</font>");		
    }  
    
	if (lineEdit_refmarkercpt_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_refmarkercpt_file->setText("<font color=\"red\">Reference marker control point file for registration:</font>");		
    }
    else
    {
 		label_refmarkercpt_file->setText("<font color=\"black\">Reference marker control point file for registration:</font>");		
    }  
    
  
	if (lineEdit_forceadd_celllist_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_forceadd_celllist_file->setText("<font color=\"red\">Select force added cell file:</font>");		
    }
    else
    {
 		label_forceadd_celllist_file->setText("<font color=\"black\">Select force added cell file:</font>");		
    }  
    
    
	printf("b_info_complete=%d\n", b_info_complete);
	
	//set the two push button states
	pushButton_ok->setEnabled(b_info_complete);
    
    
}






