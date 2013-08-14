/*
 * Copyright (c)2011 Fuhui Long (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


#ifndef __POINTCLOUNDATLAS_ADJUST_PREFERENCE_H__
#define __POINTCLOUNDATLAS_ADJUST_PREFERENCE_H__

#include <QDialog>
#include <QList>

#include "ui_dialog_pointcloudatlas_adjust_preference.h"

#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"

class PointCloudAtlas_AdjustPreferenceDialog : public QDialog, private Ui_Dialog_pointcloudatlas_adjust_preference
{
    Q_OBJECT
    
public:
    PointCloudAtlas_AdjustPreferenceDialog(atlasConfig &p);
	void fetchData(atlasConfig &p);
    
//protected:
//	int n_apo;
//	QStringList nameFilters;
//	QFileInfoList fileList_apo;
	
private:
	void create(atlasConfig &p);
	
public slots:
	void done() {QDialog::done(2);}

    void select_input_folder();
	void change_input_folder(const QString & s);
    
	void select_output_folder();
	void change_output_folder(const QString & s);

//	void change_cellname_file_prefix(const QString & s);
//	void change_cellname_file_surfix(const QString & s);
    
//	void select_refmarkercpt_file();  
//    void change_refmarkercpt_file(const QString &);    
    
//    void change_saveregtag(const QString & s);
    
    void change_celltype_file(const QString & s);
	void select_celltype_file();  
    void remove_celltype_file();
        
//    void change_cellstat_file_surfix(const QString &);
//    void change_coexp_file_surfix(const QString &);
//    void change_markermap_file(const QString &);
    
	void check_completeness_of_info();
};


#endif
