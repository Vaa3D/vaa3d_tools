/****************************************************************************
 dialog_pointcloudatlas_buildatlas.h
 by Fuhui Long
 Oct 23, 2009
****************************************************************************/

#ifndef __POINTCLOUNDATLAS_BUILDATLAS_DIALOG_H__
#define __POINTCLOUNDATLAS_BUILDATLAS_DIALOG_H__

#include <QDialog>
#include <QList>

#include "ui_dialog_pointcloudatlas_build_atlas.h"
#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"

class PointCloudAtlas_BuildAtlasDialog : public QDialog, private Ui_Dialog_pointcloudatlas_buildatlas
{
    Q_OBJECT

public:
    PointCloudAtlas_BuildAtlasDialog(apoAtlasBuilderInfo &p);
	void fetchData(apoAtlasBuilderInfo &p);

protected:
//	int n_apo_ref, n_apo_sig;
//	QStringList nameFilters;
	
private:
	void create(apoAtlasBuilderInfo &p);
	
public slots:
	void done() {QDialog::done(2);}
//	void select_output_file();
	void select_linker_file();
	void change_linker_file(const QString & s);
    
    void select_target_file();
	void change_target_file(const QString & s);
    
    void select_forceadded_cell_file();
    void change_forceadded_cell_file(const QString & s);
    
	void select_refmarkercpt_file();  
    void change_refmarkercpt_file(const QString &);    
    
    void check_completeness_of_info();
    
};


#endif
