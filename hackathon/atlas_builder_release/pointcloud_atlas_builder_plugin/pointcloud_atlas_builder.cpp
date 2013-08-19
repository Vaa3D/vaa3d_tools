//by Hanchuan Peng
// 091022

#include <QtGui>
#include <QFileInfo>
#include <QFileDialog>

#include <math.h>
#include <stdlib.h>

#include "pointcloud_atlas_builder.h"

#include "v3d_message.h"
#include "basic_surf_objs.h"

#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"
#include "../pointcloud_atlas_builder/FL_atlasBuilder.h"
#include "../pointcloud_atlas_builder/converter_pcatlas_data.h"

#define V3DFOLDER "/plugins/atlas_builder/atlas_config.txt"

QStringList pointcloud_atlas_builderPlugin::menulist() const
{
    return QStringList() << tr("Adjust preference")
                        <<tr("Create a new point cloud atlas linker file")
                         << tr("Edit an existing point cloud atlas linker file")
						 << tr("Build the atlas")
                         << tr("Detect coexpressing cells")
//                         << tr("Edit coexpressing cells")
                         << tr("Merge coexpressing cells")
//                         << tr("Display atlas")
						 << tr("About this plugin");
}

void pointcloud_atlas_builderPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    
    atlasConfig cfginfo;
    QDir curdir; 
    
//    v3d_msg(curdir.currentPath());
//    
//    printf("%s", curdir.currentPath().toStdString().c_str());

    QString fileName = curdir.currentPath() + V3DFOLDER;
      
	if (!loadPointCloudAtlasConfigFile(qPrintable(fileName.trimmed()), cfginfo))
	{
		v3d_msg("Fail to load the configuration file atlas_config.txt");
		return;
	}    
    
    
    
    if (menu_name == tr("Edit an existing point cloud atlas linker file"))
    {
		fl_procPC_Atlas_edit_atlaslinkerfile();
	}
	else if (menu_name == tr("Create a new point cloud atlas linker file"))
	{
		fl_procPC_Atlas_create_atlaslinkerfile();
	}
	else if (menu_name == tr("Build the atlas"))
	{
		fl_procPC_Atlas_build_atlas(cfginfo);
	}	
    else if (menu_name == tr("Detect coexpressing cells"))
    {
        fl_procPC_atlas_detect_coexpress_cells(cfginfo);
    }
//    else if (menu_name == tr("Edit coexpressing cells"))
//    {
//        fl_procPC_atlas_edit_coexpress_cells();
//    }
    else if (menu_name == tr("Merge coexpressing cells"))
    {
//        fl_procPC_atlas_merge_coexpress_cells(callback);
        fl_procPC_atlas_merge_coexpress_cells(cfginfo);
        
    }
//    else if (menu_name == tr("Display atlas"))
//    {
//        fl_procPC_atlas_display();
//    }    
    else if (menu_name == tr("Adjust preference"))
    {
        fl_procPC_adjust_preference();
    }
	else if (menu_name == tr("About this plugin"))
	{
		QMessageBox::information(parent, "Version info", "Point cloud atlas builder v1.0. Developed by Fuhui Long .");
	}
	else
		return;
}	


// adjust preference

void fl_procPC_adjust_preference()
{
	atlasConfig cfginfo;
//	QString fileName = "atlas_config.txt";
//    QString fileName = "/Users/longf/work/v3d_internal/long_fuhui/atlas_builder/pointcloud_atlas_builder_plugin/atlas_config.txt";	
    
    QDir curdir; 
//    v3d_msg(curdir.currentPath());
//    
//    printf("%s", curdir.currentPath().toStdString().c_str());
    
//    QString fileName = curdir.currentPath() + "/plugins/atlas_builder/atlas_config.txt";
    QString fileName = curdir.currentPath() + V3DFOLDER;
    
    
	if (!loadPointCloudAtlasConfigFile(qPrintable(fileName.trimmed()), cfginfo))
	{
		v3d_msg("Fail to load the configuration file atlas_config.txt");
		return;
	}
	
	fl_func_procIO_adjust_preference(fileName, cfginfo);
    

}

void fl_func_procIO_adjust_preference(QString fileName, atlasConfig & cfginfo)
{

    
	while (1)
	{
		PointCloudAtlas_AdjustPreferenceDialog d(cfginfo); //everytime create a new dialog instance, so that apoinfo becomes longer and longer
		int rescode = d.exec();
		
		if (rescode==QDialog::Accepted)
		{
			d.fetchData(cfginfo);
			break;
		}
		else if (rescode==QDialog::Rejected)
		{
			return; //break;
		}
		else
		{
			v3d_msg("invalid rescode returned in PointCloudAtlas_AdjustPreferenceDialog(). Check your program.");
			break;
		}
	}

    
    if (!savePointCloudAtlasConfigFile(qPrintable(fileName), cfginfo))
    {
        QString errinfo = "Fail to save the configuration file: " + fileName; 
        v3d_msg(qPrintable(errinfo));
    }
    
}



// edit linker file
void fl_procPC_Atlas_edit_atlaslinkerfile()
{
	apoAtlasLinkerInfoAll apoinfo;
	QString fileName = QFileDialog::getOpenFileName(0, QString("Open a Point Cloud Atlas File"),
													"",
													QString("point cloud atlas linker file (*.pc_atlas)"));
    if (fileName.isEmpty())
	{
		v3d_msg("No file is selected. Do nothing.");
		return;
	}
	
	if (!loadPointCloudAtlasInfoListFromFile(qPrintable(fileName.trimmed()), apoinfo))
	{
		v3d_msg("Fail to load the specified point cloud atlas file.");
		return;
	}
	
	fl_func_procIO_import_atlas_apofolder(apoinfo);//continue to edit
}

// create a new linker file
void fl_procPC_Atlas_create_atlaslinkerfile()
{
	apoAtlasLinkerInfoAll apoinfo;
	fl_func_procIO_import_atlas_apofolder(apoinfo);
}

// display atlas
void fl_procPC_atlas_display()
{
}

//void fl_procPC_atlas_merge_coexpress_cells(V3DPluginCallback &callback)
void fl_procPC_atlas_merge_coexpress_cells(atlasConfig & cfginfo)
{
    
    
    apoAtlasCoexpressionMerge apoCoExpressInfo;
    
    fl_func_procIO_merge_coexpress_cells(apoCoExpressInfo, cfginfo);

//    // temporary for debug purpose
//    apoCoExpressInfo.apoAtlasFilelist.clear();
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_EVE.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_EN.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_REPO.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_PS1.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_ISLET.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_HB9.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_DIMM.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_BAR.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_AP.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_A227.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_11F02.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_10D02.apo");
//    apoCoExpressInfo.apoAtlasFilelist.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_9-58.apo");
//    
//    apoCoExpressInfo.coExpressionFile.clear();
//    apoCoExpressInfo.coExpressionFile.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellname_mapping.txt");
//    
////    apoCoExpressInfo.apoFinalAtlasFile.clear();
////    apoCoExpressInfo.apoFinalAtlasFile.append("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/");
    
    
    
    
    for (int i=0; i<apoCoExpressInfo.apoAtlasFilelist.size(); i++)
        printf("%s\n", qPrintable(apoCoExpressInfo.apoAtlasFilelist.at(i)));
    
    
    printf("%s\n", qPrintable(apoCoExpressInfo.coExpressionFile));
//    printf("%s\n", qPrintable(apoCoExpressInfo.apoFinalAtlasFile));
    
    if (apoCoExpressInfo.apoMergedAtlasFilePrefix.compare("")!=0) // otherwise, you have selected cancel, no need to merge
    {
//        apoCoExpressInfo.apoFinalAtlasFile.append("/");
        fl_func_procCompute_atlas_merge_coexpress_cells(apoCoExpressInfo, cfginfo);
        v3d_msg("Results can be found in the output folder that you've set up through adjust preference function. End of coexpression merging.");
    }
    else
        v3d_msg("You canceled merging function. No results generated. End of coexpression merging.");
    
    
//    v3d_msg("The final atlas and cell statistics have been generated.");    

//    QString apofile = apoCoExpressInfo.apoFinalAtlasFile.append("atla\s_all_final.apo");
//    printf("%s\n", qPrintable(apofile));
//    callback.newImageWindow(apofile);
//    callback.open3DWindow(callback.newImageWindow("/Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_all_final.apo"));

//    system("/Users/longf/work/v3d_external/v3d/vaa3d64.app/Contents/MacOS/vaa3d64 -f /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_all_final.apo");
}

void fl_procPC_atlas_edit_coexpress_cells()
{
}

void fl_procPC_atlas_detect_coexpress_cells(atlasConfig & cfginfo)
{
    
    apoAtlasCoexpressionDetect apoCoExpressInfo;
    
    fl_func_procIO_detect_coexpress_cells(apoCoExpressInfo, cfginfo);
    
    for (int i=0; i<apoCoExpressInfo.apoAtlasFilelist.size(); i++)
        printf("%s\n", qPrintable(apoCoExpressInfo.apoAtlasFilelist.at(i)));
    printf("%s\n", qPrintable(apoCoExpressInfo.coExpressionOutputFilePrefix));
    
    if (apoCoExpressInfo.coExpressionOutputFilePrefix.compare("")!=0)
        fl_func_procCompute_atlas_detect_coexpress_cells(apoCoExpressInfo, cfginfo);
    
//    v3d_msg("The coexpression files have been generated [fl_procPC_atlas_detect_coexpress_cells() done].");
    v3d_msg("End of coexpression detection.");
  
    
    
}

void fl_func_procIO_merge_coexpress_cells(apoAtlasCoexpressionMerge & apoInfoCoexpression, atlasConfig & cfginfo)
{
    
	//input interface
    
	while (1)
	{
		PointCloudAtlas_MergeCoexpressionDialog d(apoInfoCoexpression, cfginfo); //everytime create a new dialog instance, so that apoinfo becomes longer and longer
        
		int rescode = d.exec();
		
		if (rescode==QDialog::Accepted)
		{
			d.fetchData(apoInfoCoexpression);
			printf("Now you've selected %d atlas files. \n", apoInfoCoexpression.apoAtlasFilelist.size());
			break;
		}
		else if (rescode==QDialog::Rejected)
		{
			v3d_msg("The merging operation has been canceled.");
			return; //break;
		}
		else
		{
			v3d_msg("invalid rescode returned in PointCloudAtlas_MergeCoexpressionDialog(). Check your program.");
			break;
		}
	}
    
     
}


void fl_func_procIO_detect_coexpress_cells(apoAtlasCoexpressionDetect & apoInfoCoexpression, atlasConfig & cfginfo)
{

	//input interface
    
	while (1)
	{
		PointCloudAtlas_DetectCoexpressionDialog d(apoInfoCoexpression, cfginfo); //everytime create a new dialog instance, so that apoinfo becomes longer and longer
        
//        printf("symmetryRule = %s, bilateralRule = %s\n", apoInfoCoexpression.symmetryRule? "True" : "False", apoInfoCoexpression.bilateralRule? "True" : "False");

        printf("symmetryRule = %s\n", apoInfoCoexpression.symmetryRule? "true" : "false"); 
        printf("bilateralRule = %s\n", apoInfoCoexpression.bilateralRule? "true" : "false");
        
        
		int rescode = d.exec();
		
		if (rescode==QDialog::Accepted)
		{
			d.fetchData(apoInfoCoexpression);

            printf("symmetryRule = %s\n", apoInfoCoexpression.symmetryRule? "true" : "false"); 
            printf("bilateralRule = %s\n", apoInfoCoexpression.bilateralRule? "true" : "false");
            
			printf("Now you've selected %d atlas files. \n", apoInfoCoexpression.apoAtlasFilelist.size());
			break;
		}
		else if (rescode==QDialog::Rejected)
		{
			v3d_msg("The coexpression detection is canceled.");
			return; //break;
		}
//		else if (rescode==2) //"2" is a special code I define in dialog_pointcloudatlas_linkerload.cpp for "add another"
//		{
//			d.fetchData(apoInfoCoexpression);
//			printf("Now you've selected %d atlas files\n", apoInfoCoexpression.apoAtlasCoexpressionFilelist.size());
//		}
		else
		{
			v3d_msg("Invalid rescode returned in PointCloudAtlas_DetectDialog(). Check your program.");
			break;
		}
	}
    

}

void fl_func_procCompute_atlas_detect_coexpress_cells(apoAtlasCoexpressionDetect & p, atlasConfig & cfginfo)
{
    
    QString outprefix = cfginfo.outputFolder + "/" + p.coExpressionOutputFilePrefix;
    coLocalizationDetection(p.apoAtlasFilelist, outprefix, p.distanceVal, p.symmetryRule, p.bilateralRule);

}

void fl_func_procCompute_atlas_merge_coexpress_cells(apoAtlasCoexpressionMerge & p, atlasConfig & q)
{
    QString dir_output = q.outputFolder + "/";
//    QString markermap_file_name = dir_output + q.markerMapFileName;
    QStringList celltype_file_name_list = q.cellTypeFileName;
    
    merge_atlas(p.apoAtlasFilelist, p.coExpressionFile, dir_output, q.cellStatFileSurfix, q.markerMapFileName, p.apoMergedAtlasFilePrefix, celltype_file_name_list);
        
}

// build atlas
void fl_procPC_Atlas_build_atlas(atlasConfig & cfginfo)
{
    
    apoAtlasBuilderInfo atlasbuiderinfo;
	
    fl_func_procIO_build_atlas(atlasbuiderinfo);
    
    apoAtlasLinkerInfoAll apoinfo;
    
    if (atlasbuiderinfo.linkerFileName.compare("")!=0) // otherwise, you have selected cancel, no need to merge
    {
//        atlasbuiderinfo.linkerFileName.append("/");
        fl_func_procCompute_build_atlas(atlasbuiderinfo, cfginfo, apoinfo);
//        v3d_msg("Results can be found in the output folder that you've set up through adjust preference function. End of coexpression merging.");
    }
    else
        v3d_msg("You canceled atlas building function. No results generated. End of atlas building.");  

    
}

void fl_func_procIO_build_atlas(apoAtlasBuilderInfo & atlasbuiderinfo)
{
 
	//input interface
    
	while (1)
	{
		PointCloudAtlas_BuildAtlasDialog d(atlasbuiderinfo); //everytime create a new dialog instance, so that apoinfo becomes longer and longer
        
		int rescode = d.exec();
		
		if (rescode==QDialog::Accepted)
		{
			d.fetchData(atlasbuiderinfo);
			break;
		}
		else if (rescode==QDialog::Rejected)
		{
			v3d_msg("The selection of atlas files is canceled.");
			return; //break;
		}
        //		else if (rescode==2) //"2" is a special code I define in dialog_pointcloudatlas_linkerload.cpp for "add another"
        //		{
        //			d.fetchData(apoInfoCoexpression);
        //			printf("Now you've selected %d atlas files\n", apoInfoCoexpression.apoAtlasCoexpressionFilelist.size());
        //		}
		else
		{
			v3d_msg("Invalid rescode returned in PointCloudAtlas_BuildAtlasDialog(). Check your program.");
			break;
		}
	}
    
}

void fl_func_procCompute_build_atlas(apoAtlasBuilderInfo & atlasbuiderinfo, atlasConfig & cfginfo, apoAtlasLinkerInfoAll &apoinfo)
{
    
    
    QString fileName = atlasbuiderinfo.linkerFileName; 
    printf("%s\n", qPrintable(atlasbuiderinfo.linkerFileName));
    
    if (!loadPointCloudAtlasInfoListFromFile(qPrintable(fileName.trimmed()), apoinfo))    
    {
        v3d_msg("Fail to load the specified point cloud atlas file.");
        return;
    }
    
    buildAtlas(atlasbuiderinfo, cfginfo, apoinfo);

 //    v3d_msg(QString("The target file for registration is  ****: %1. \nAtlas is sucessfully built").arg(dfile_target.c_str()));	
    v3d_msg("Altas is successfully built. Results can be found in the output folder that you've set up through adjust preference function.");
    
}

void fl_func_procIO_import_atlas_apofolder() //overload for convenience
{
	apoAtlasLinkerInfoAll apoinfo;
	fl_func_procIO_import_atlas_apofolder(apoinfo);
}

void fl_func_procIO_import_atlas_apofolder(apoAtlasLinkerInfoAll & apoinfo)
{
	//	QMessageBox::information(0, "Information about import pointcloud APO files to an .atlas linker file",
	//							 "To import a series of images to an .atlas linker file, these files should be put into the same folder and have the same size (X,Y,Z dimensions and number of color channels). <br><br>"
	//							 "You will be first asked to specify one of these files, and you can define a string filter of their file names.<br><br>"
	//							 "Normally these files should be aligned/registered images; but this function can also be used to produce a linker file just for screening a series of image stacks.<br><br>"
	//							 "Note that these files must be valid .tif files (or another image file format supported in V3D).<br><br>"
	//							 "You will be then asked to specify a file name of the output .atlas linker file.<br><br>"
	//							 );
	
	//first load a series of annotation files and build the record
	
	while (1)
	{
		PointCloudAtlas_LinkerLoadDialog d(apoinfo); //everytime create a new dialog instance, so that apoinfo becomes longer and longer
		int rescode = d.exec();
		
		if (rescode==QDialog::Accepted)
		{
			d.fetchData(apoinfo);
			printf("Now apoinfo has the %d classes\n", apoinfo.items.size());
			break;
		}
		else if (rescode==QDialog::Rejected)
		{
			v3d_msg("The point cloud atlas linker file editing is canceled.");
			return; //break;
		}
		else if (rescode==2) //"2" is a special code I define in dialog_pointcloudatlas_linkerload.cpp for "add another"
		{
			d.fetchData(apoinfo);
			printf("Now apoinfo has the %d classes\n", apoinfo.items.size());
		}
		else
		{
			v3d_msg("invalid rescode returned in PointCloudAtlas_LinkerLoadDialog(). Check your program.");
			break;
		}
	}
	
	//now determine the output .atlas file
	QString fileName = QFileDialog::getSaveFileName(0, QString("Save Atlas File (*.pc_atlas)"),
													"import.pc_atlas",
													QString("point cloud atlas linker file (*.pc_atlas)"));
    if (fileName.isEmpty())
	{
		v3d_msg("No output file is selected. Do nothing.");
		return;
	}

	
	
	//append(".pc_atlas");
		
	
	//now save the record to a pointcloud atlas file
	
	if (savePointCloudAtlasInfoListToFile(qPrintable(fileName), apoinfo))
		v3d_msg(qPrintable(fileName.prepend("The point cloud atlas linker file [").append("] has been created.")));
	else
		v3d_msg("Fail to save the point cloud atlas file.");
	
}


//void fl_procPC_atlas_detect_coexpress_cells()


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(pointcloud_atlas_builder, pointcloud_atlas_builderPlugin)

