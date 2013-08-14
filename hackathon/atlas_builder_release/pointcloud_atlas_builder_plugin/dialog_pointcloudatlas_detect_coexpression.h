/*
 * Copyright (c)2011 Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


#ifndef __POINTCLOUNDATLAS_DETECT_COEXPRESSION_DIALOG_H__
#define __POINTCLOUNDATLAS_DETECT_COEXPRESSION_DIALOG_H__

#include <QDialog>
#include <QList>

#include "ui_dialog_pointcloudatlas_detect_coexpression.h"

#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"

class PointCloudAtlas_DetectCoexpressionDialog : public QDialog, private Ui_Dialog_pointcloudatlas_detect_coexpression
{
    Q_OBJECT
    
public:
    PointCloudAtlas_DetectCoexpressionDialog(apoAtlasCoexpressionDetect &p, atlasConfig &q);
	void fetchData(apoAtlasCoexpressionDetect &p);
    
protected:
	int n_apo;
	QStringList nameFilters;
	QFileInfoList fileList_apo;
	
private:
	void create(apoAtlasCoexpressionDetect &p, atlasConfig &q);
	
    public slots:
	void done() {QDialog::done(2);}
    
	void select_atlas_file(); 
    void remove_atlas_file();        
	void change_atlas_file(const QString & s);
 
//	void select_output_file(); 
//	void change_output_file(const QString & s);

	void change_output_prefix(const QString & s);
    
	void check_completeness_of_info();
};


#endif
