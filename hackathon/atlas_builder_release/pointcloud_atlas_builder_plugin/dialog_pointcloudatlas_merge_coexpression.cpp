/*
 * Copyright (c)2011  Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 % Dec 30, 2011
 */


#include "dialog_pointcloudatlas_merge_coexpression.h"


#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

// -----------------------------------
// functions of the input interface
// -----------------------------------

PointCloudAtlas_MergeCoexpressionDialog::PointCloudAtlas_MergeCoexpressionDialog(apoAtlasCoexpressionMerge &p, atlasConfig &q)
{
	n_apo = 0;
	nameFilters << "*.apo" << "*.ano.txt";

	create(p, q);
}

void PointCloudAtlas_MergeCoexpressionDialog::fetchData(apoAtlasCoexpressionMerge &p)
{
    
    p.apoAtlasFilelist.clear();
    
    int num = listWidget_existing_input_atlases->count();
    
    for (int i=0; i<num; i++)
    {

        QString a;
        a = listWidget_existing_input_atlases->item(i)->text().trimmed();
        printf("%s\n",qPrintable(a));
        p.apoAtlasFilelist.append(a);
        
    }
    
    p.coExpressionFile = lineEdit_coexpression_file->text().trimmed();
    p.apoMergedAtlasFilePrefix = lineEdit_output_file_prefix->text().trimmed();
    
        
}



void PointCloudAtlas_MergeCoexpressionDialog::create(apoAtlasCoexpressionMerge &p, atlasConfig &q)
{
	setupUi(this);
	
	//default values and events
	lineEdit_input_atlas_file->setText("");
//	lineEdit_output_folder->setText("");
	lineEdit_coexpression_file->setText("");
	lineEdit_output_file_prefix->setText("");
	label_output_folder->setText("Output folder: " + q.outputFolder);
    
	//set the input atlas apo file
    
	QFileInfo info;
    
	connect(lineEdit_input_atlas_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_input_atlas_file(const QString &)));
	//connect(pushButton_input_atlas_file, SIGNAL(clicked()), this, SLOT(select_input_atlas_file()));
    
    connect(pushButton_input_atlas_file, SIGNAL(clicked()), this, SLOT(select_input_atlas_filelist()));
 	connect(pushButton_input_atlas_remove, SIGNAL(clicked()), this, SLOT(remove_input_atlas_file()));
   

    // select the input coexpression file
	connect(lineEdit_coexpression_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_coexpression_file(const QString &)));
	connect(pushButton_coexpression_file, SIGNAL(clicked()), this, SLOT(select_coexpression_file()));
    
    // select the output atlas file
    
 	connect(lineEdit_output_file_prefix, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_prefix(const QString &)));
    
//	connect(lineEdit_output_folder, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_atlas_file(const QString &)));
//	connect(pushButton_output_atlas_file, SIGNAL(clicked()), this, SLOT(select_output_atlas_file()));
    
//	connect(lineEdit_output_folder, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_folder(const QString &)));    
//	connect(pushButton_output_folder, SIGNAL(clicked()), this, SLOT(select_output_folder()));

    // button for ending the window
	connect(pushButton_finish, SIGNAL(clicked()), this, SLOT(accept())); //this will get a result code "Accepted"
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    //enable or not the finish button
	check_completeness_of_info();
    
}

void PointCloudAtlas_MergeCoexpressionDialog::change_output_prefix(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_output_file_prefix->setText("<font color=\"red\">Prefix of the names of output merged files</font>"); //FL 20091027  
	else
		label_output_file_prefix->setText("Prefix of the names of output merged files"); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_MergeCoexpressionDialog::change_input_atlas_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_input_atlas_file->setText("<font color=\"red\">Select an atlas file: (.apo)  </font>"); //FL 20091027  
	else
		label_input_atlas_file->setText("Select an atlas file: (.apo) "); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}

// select one single atlas .apo at a time

void PointCloudAtlas_MergeCoexpressionDialog::select_input_atlas_file()
{
	QString s;
	QFileInfo info(lineEdit_input_atlas_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select an atlas file: "), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
	if (!s.isEmpty())
	{
		lineEdit_input_atlas_file->setText(s);
        listWidget_existing_input_atlases->addItem(s);
	}
}



// select multiple atlas .apo files, Oct 11, 2012 add

void PointCloudAtlas_MergeCoexpressionDialog::select_input_atlas_filelist()
{
	QStringList s;
//	QFileInfo info(lineEdit_input_atlas_file->text().trimmed());
    QFileInfo info;
    
    
    QFileDialog dialog(this);
 //   dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("Pointcloud file (*.apo *.txt)"));
//    QStringList fileNames;
    if (dialog.exec())
        s = dialog.selectedFiles();
    
    
//    
//	s = QFileDialog::getOpenFileName(0, tr("Select atlas file(s): "), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
  
    
    for (int i = 0; i < s.size(); i++)
    {
        if (!s.at(i).isEmpty())
        {
            lineEdit_input_atlas_file->setText(s.at(i));
            listWidget_existing_input_atlases->addItem(s.at(i));
        }
        
    }
    
    
}






void PointCloudAtlas_MergeCoexpressionDialog::change_coexpression_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_coexpression_file->setText("<font color=\"red\">Select the coexpression file:  </font>"); //FL 20091027  
	else
		label_coexpression_file->setText("Select the coexpression file: "); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}




void PointCloudAtlas_MergeCoexpressionDialog::select_coexpression_file()
{
	QString s;
	QFileInfo info(lineEdit_coexpression_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select the coexpression file: "), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
	if (!s.isEmpty())
	{
		lineEdit_coexpression_file->setText(s);
	}
	check_completeness_of_info();
    
}


void PointCloudAtlas_MergeCoexpressionDialog::remove_input_atlas_file()
{
    
    int a=listWidget_existing_input_atlases->currentRow();
    delete listWidget_existing_input_atlases->takeItem(a);
    
}


//void PointCloudAtlas_MergeCoexpressionDialog::change_output_folder(const QString & s) 
//{
//	QFileInfo info(s.trimmed());
//
//	if (!info.exists() || !info.isDir())
//	{	
//		label_output_folder->setText("<font color=\"red\">Select the output folder </font>"); // FL 20091026
//		lineEdit_output_folder->setText("Folder is invalid.");
//	}
//	else
//	{
//		label_output_folder->setText("<font color=\"black\">Select the output folder </font>");		//FL 20091026
//		QDir d(info.absoluteFilePath());
//	}	
//    
//	
//	//enable or not the finish button
//	check_completeness_of_info();
//}
//
//
//
//
//void PointCloudAtlas_MergeCoexpressionDialog::select_output_folder()
//{
//	QString s;
//	QFileInfo info(lineEdit_output_folder->text().trimmed());
//	s = QFileDialog::getExistingDirectory(0, tr("select the output folder:"), info.dir().path()); //FL, 20091026
//	
//	if (!s.isEmpty())
//	{
//		lineEdit_output_folder->setText(s);
//	}
//}


void PointCloudAtlas_MergeCoexpressionDialog::check_completeness_of_info()
{
	bool b_info_complete=true;
	QFileInfo info;
    
	if ((lineEdit_input_atlas_file->text().trimmed().isEmpty()) & (listWidget_existing_input_atlases->count()==0)) // if both are empty
    {
        b_info_complete=false;
		label_input_atlas_file->setText("<font color=\"red\">Select an atlas file: (.apo) </font>");	//FL 20091026	
    }
    else
    {
 		label_input_atlas_file->setText("<font color=\"black\">Select an atlas file: (.apo) </font>");	//FL 20091026	
    }  
    
    if (lineEdit_coexpression_file->text().trimmed().isEmpty())
    {
        b_info_complete = false;
        label_coexpression_file->setText("<font color=\"red\">Select the coexpression file:</font>");
    }
    else
    {
        label_coexpression_file->setText("<font color=\"black\">Select the coexpression file:</font>");
        
    }
    
	if (lineEdit_output_file_prefix->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_output_file_prefix->setText("<font color=\"red\">Prefix of the names of output merged files</font>");		
    }
    else
    {
 		label_output_file_prefix->setText("<font color=\"black\">Prefix of the names of output merged files</font>");		
    }  
    
    
	printf("b_info_complete=%d\n", b_info_complete);
	
	//set the two push button states
	pushButton_finish->setEnabled(b_info_complete);
    
    
}






