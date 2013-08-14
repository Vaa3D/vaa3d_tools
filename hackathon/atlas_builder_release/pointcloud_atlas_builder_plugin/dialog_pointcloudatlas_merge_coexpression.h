/*
 * Copyright (c)2011 Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


#ifndef __POINTCLOUNDATLAS_MERGE_COEXPRESSION_DIALOG_H__
#define __POINTCLOUNDATLAS_MERGE_COEXPRESSION_DIALOG_H__

#include <QDialog>
#include <QList>

#include "ui_dialog_pointcloudatlas_merge_coexpression.h"

#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"

class PointCloudAtlas_MergeCoexpressionDialog : public QDialog, private Ui_Dialog_pointcloudatlas_merge_coexpression
{
    Q_OBJECT
    
public:
    PointCloudAtlas_MergeCoexpressionDialog(apoAtlasCoexpressionMerge &p, atlasConfig &q);
	void fetchData(apoAtlasCoexpressionMerge &p);
    
protected:
	int n_apo;
	QStringList nameFilters;
	QFileInfoList fileList_apo;
	
private:
	void create(apoAtlasCoexpressionMerge &p, atlasConfig &q);
	
public slots:
	void done() {QDialog::done(2);}
    
	void select_input_atlas_file(); 
    void select_input_atlas_filelist();
    
	void change_input_atlas_file(const QString & s);    
    void remove_input_atlas_file();    
    
//	void select_output_atlas_file(); 
//	void change_output_atlas_file(const QString & s);

//	void select_output_folder(); 
//	void change_output_folder(const QString & s);

    
    void change_output_prefix(const QString & s);
    
	void select_coexpression_file(); 
	void change_coexpression_file(const QString & s);
    
	void check_completeness_of_info();
};


#endif
