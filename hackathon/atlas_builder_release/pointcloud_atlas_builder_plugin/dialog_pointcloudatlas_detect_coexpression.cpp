/*
 * Copyright (c)2011  Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


#include "dialog_pointcloudatlas_detect_coexpression.h"


#include <QtGui>

#include <QFileInfo>
#include <QFile>
#include <QFileDialog>

// -----------------------------------
// functions of the input interface
// -----------------------------------

PointCloudAtlas_DetectCoexpressionDialog::PointCloudAtlas_DetectCoexpressionDialog(apoAtlasCoexpressionDetect &p, atlasConfig &q)
{
	n_apo = 0;
	nameFilters << "*.apo" << "*.ano.txt";

	create(p, q);
}

void PointCloudAtlas_DetectCoexpressionDialog::fetchData(apoAtlasCoexpressionDetect &p)
{
    
    p.apoAtlasFilelist.clear();
    
    int num = listWidget_info_existing_atlases->count();
    
    for (int i=0; i<num; i++)
    {

        QString a;
        a = listWidget_info_existing_atlases->item(i)->text().trimmed();
        printf("%s\n",qPrintable(a));
        p.apoAtlasFilelist.append(a);
        
    }
    
    p.coExpressionOutputFilePrefix = lineEdit_output_file->text().trimmed();
    p.distanceVal = atof(lineEdit_distance_rule->text().trimmed().toStdString().c_str());
    
    if (checkBox_symmetry_rule->isChecked())
        p.symmetryRule = true;
    else
        p.symmetryRule = false;

    if (checkBox_bilateral_rule->isChecked())
        p.bilateralRule = true;
    else
        p.bilateralRule = false;
    

}

void PointCloudAtlas_DetectCoexpressionDialog::create(apoAtlasCoexpressionDetect &p, atlasConfig &q)
{
	setupUi(this);

	//default values and events
	lineEdit_atlas_file->setText("");
	lineEdit_output_file->setText("");
	label_output_folder->setText("output folder: " + q.outputFolder);
    lineEdit_distance_rule->setText("5");
    checkBox_symmetry_rule->setChecked(true);
    checkBox_bilateral_rule->setChecked(true);
    
//    p.distanceVal = 5;
//    p.symmetryRule = true;
//    p.bilateralRule = true;
//    
    //set the atlas apo file
    
//	QFileInfo info;
    
	connect(lineEdit_atlas_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_atlas_file(const QString &)));
	connect(pushButton_atlas_file, SIGNAL(clicked()), this, SLOT(select_atlas_file()));
    connect(pushButton_atlas_file_remove, SIGNAL(clicked()), this, SLOT(remove_atlas_file()));

    // set the output file
 	connect(lineEdit_output_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_prefix(const QString &)));
    
// 	connect(lineEdit_output_file, SIGNAL(textChanged(const QString &)), this, SLOT(change_output_file(const QString &)));
//	connect(pushButton_output_file, SIGNAL(clicked()), this, SLOT(select_output_file()));
    
    // button to end the session
	connect(pushButton_finish, SIGNAL(clicked()), this, SLOT(accept())); //this will get a result code "Accepted"
	connect(pushButton_cancel, SIGNAL(clicked()), this, SLOT(reject()));
	    
    //enable or not the finish button
	check_completeness_of_info();
    
}


void PointCloudAtlas_DetectCoexpressionDialog::change_atlas_file(const QString & s) 
{
	QFileInfo info(s.trimmed());
	if (!s.trimmed().isEmpty() && (!info.exists() || !info.isFile()))
		label_atlas_file->setText("<font color=\"red\">Select an atlas file: (.apo)  </font>"); //FL 20091027  
	else
		label_atlas_file->setText("Select an atlas file: (.apo) "); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


void PointCloudAtlas_DetectCoexpressionDialog::select_atlas_file()
{
	QString s;
	QFileInfo info(lineEdit_atlas_file->text().trimmed());
	s = QFileDialog::getOpenFileName(0, tr("Select an atlas file for coexpression detection "), info.dir().path(), tr("Pointcloud file (*.apo *.txt)"));
	if (!s.isEmpty())
	{
		lineEdit_atlas_file->setText(s);
        listWidget_info_existing_atlases->addItem(s);
	}
}


void PointCloudAtlas_DetectCoexpressionDialog::remove_atlas_file()
{
    
//    printf("start to remove\n");
    int a=listWidget_info_existing_atlases->currentRow();
//    printf("current item in list widget = %d\n", a);
    delete listWidget_info_existing_atlases->takeItem(a);
    
    
}


void PointCloudAtlas_DetectCoexpressionDialog::change_output_prefix(const QString & s) 
{
	
    if (lineEdit_output_file->text().trimmed().isEmpty())
        label_output_file->setText("<font color=\"red\">Prefix of the names of output coexpression files: </font>");	//FL 20091027	
	else
        //		label_output_file->setText("Output pointcloud file of this class");
		label_output_file->setText("Prefix of the names of output coexpression files:"); //FL 20091027
	
	//enable or not the finish button
	check_completeness_of_info();
}


//void PointCloudAtlas_DetectCoexpressionDialog::change_output_prefix(const QString & s) 
//{
//	QFileInfo info(s.trimmed());
//	if (!s.trimmed().isEmpty() && (!info.dir().exists()))
//        //		label_output_file->setText("<font color=\"red\">Output pointcloud file of this class</font>");
//		label_output_file->setText("<font color=\"red\">Prefix of the names of output coexpression files: </font>");	//FL 20091027	
//	else
//        //		label_output_file->setText("Output pointcloud file of this class");
//		label_output_file->setText("Prefix of the names of output coexpression files:"); //FL 20091027
//	
//	//enable or not the finish button
//	check_completeness_of_info();
//}


//void PointCloudAtlas_DetectCoexpressionDialog::select_output_prefix()
//{
//	QString s;
//	QFileInfo info(lineEdit_output_file->text().trimmed());
//	s = QFileDialog::getOpenFileName(0, tr("select the output file"), info.dir().path(), tr("Pointcloud file (*.apo *.txt)")); //FL 20091027
//	
//	if (!s.isEmpty())
//	{
//		lineEdit_output_file->setText(s);
//	}
//}

void PointCloudAtlas_DetectCoexpressionDialog::check_completeness_of_info()
{
	bool b_info_complete=true;
	QFileInfo info;
    
	if (lineEdit_atlas_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_atlas_file->setText("<font color=\"red\">Select an atlas file: (.apo) </font>");	//FL 20091026	
    }
    else
    {
 		label_atlas_file->setText("<font color=\"black\">Select an atlas file: (.apo) </font>");	//FL 20091026	
    }  
    
	printf("b_info_complete=%d\n", b_info_complete);
    
    
	if (lineEdit_output_file->text().trimmed().isEmpty()) 
    {
        b_info_complete=false;
		label_output_file->setText("<font color=\"red\">Prefix of the names of output coexpression files:</font>");		
    }
    else
    {
 		label_output_file->setText("<font color=\"black\">Prefix of the names of output coexpression files:</font>");		
    }  
    
	printf("b_info_complete=%d\n", b_info_complete);
	
	//set the two push button states
	pushButton_finish->setEnabled(b_info_complete);
    //	pushButton_addanother->setEnabled(b_info_complete);
    

    
//    printf("symmetryRule = %s, bilateralRule = %s\n", apoInfoCoexpression.symmetryRule? "True" : "False", apoInfoCoexpression.bilateralRule? "True" : "False");
    
}




