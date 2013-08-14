

/****************************************************************************
 pointcloudatlas_linker_loader_dialog.h
 by Fuhui Long
 Jan 19, 2011
****************************************************************************/

#ifndef __POINTCLOUNDATLAS_LINKER_LOADER_DIALOG_H__
#define __POINTCLOUNDATLAS_LINKER_LOADER_DIALOG_H__

#include <QDialog>
#include <QList>

#include "ui_dialog_pointcloudatlas_linkerloader.h"

#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"

class PointCloudAtlas_LinkerLoadDialog : public QDialog, private Ui_Dialog_pointcloudatlas_linkerloader
{
    Q_OBJECT

public:
    PointCloudAtlas_LinkerLoadDialog(apoAtlasLinkerInfoAll &p);
	void fetchData(apoAtlasLinkerInfoAll &p);

protected:
	int n_apo_ref, n_apo_sig;
	QStringList nameFilters;
	QFileInfoList fileList_apo_ref, fileList_apo_sig; //FL 20091028
	
private:
	void create(apoAtlasLinkerInfoAll &p);
	
public slots:
	void done() {QDialog::done(2);}
	void select_sigfolder();
	void select_reffolder();
	void change_sigfolder(const QString & s);
	void change_reffolder(const QString & s);
    //	void change_targetref_file(const QString & s); 
    //	void select_targetref_file();     
    //	void select_output_file(); 
    //	void change_output_file(const QString & s); 
    
	void check_completeness_of_info();
};


#endif
