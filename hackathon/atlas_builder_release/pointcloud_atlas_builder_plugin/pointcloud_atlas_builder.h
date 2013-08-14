// note that current version of pointcloud_atlas_builder.h as of 20091027
// does not use the interface and the function in dialog_pointcloudatlas_buildatlas.h
// which tries to separate parameters of target file for registration and output folder
// from the linker file. In the current version of parameters are all collected
// in the linker file

#ifndef _pointcloud_atlas_builder_h_
#define _pointcloud_atlas_builder_h_


#include <v3d_interface.h>

#include "dialog_pointcloudatlas_linkerloader.h"
#include "dialog_pointcloudatlas_build_atlas.h" // interface and functions are not used
//#include "dialog_pointcloudatlas_detect_coexpression_input.h"
//#include "dialog_pointcloudatlas_detect_coexpression_output.h"
#include "dialog_pointcloudatlas_detect_coexpression.h"
#include "dialog_pointcloudatlas_merge_coexpression.h"
#include "dialog_pointcloudatlas_adjust_preference.h"


class pointcloud_atlas_builderPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {};
};



void fl_func_procIO_import_atlas_apofolder();
void fl_func_procIO_import_atlas_apofolder(apoAtlasLinkerInfoAll & apoinfo);

void fl_procPC_Atlas_edit_atlaslinkerfile();
void fl_procPC_Atlas_create_atlaslinkerfile();

void fl_procPC_Atlas_build_atlas(atlasConfig & cfginfo);
void fl_func_procIO_build_atlas(apoAtlasBuilderInfo & atlasbuiderinfo);
void fl_func_procCompute_build_atlas(apoAtlasBuilderInfo & atlasbuiderinfo, atlasConfig & cfginfo, apoAtlasLinkerInfoAll &apoinfo);


void fl_procPC_atlas_detect_coexpress_cells(atlasConfig & cfginfo);
void fl_func_procIO_detect_coexpress_cells(apoAtlasCoexpressionDetect & apoAtlasCoexpression, atlasConfig & cfginfo);
void fl_func_procCompute_atlas_detect_coexpress_cells(apoAtlasCoexpressionDetect & apoAtlasCoexpression, atlasConfig & cfginfo);


//void fl_procPC_atlas_merge_coexpress_cells(V3DPluginCallback &callback);
void fl_procPC_atlas_merge_coexpress_cells(atlasConfig & cfginfo);
void fl_func_procIO_merge_coexpress_cells(apoAtlasCoexpressionMerge & apoInfoCoexpression, atlasConfig & cfginfo);
void fl_func_procCompute_atlas_merge_coexpress_cells(apoAtlasCoexpressionMerge & apoInfoCoexpression, atlasConfig & cfginfo);

void fl_procPC_adjust_preference();
void fl_func_procIO_adjust_preference(QString fileName, atlasConfig & cfginfo);


void fl_procPC_atlas_edit_coexpress_cells();
void fl_procPC_atlas_display();



//void fl_procPC_Atlas_view_atlas_computeVanoObjStat();


#endif
