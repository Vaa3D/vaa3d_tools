// header file of FL_atlasBuilder.cpp
// FL
// 20090515

#ifndef __ATLAS_BUILDER__
#define __ATLAS_BUILDER__

#include "../../../../v3d_external/v3d_main/basic_c_fun/color_xyz.h"
#include "../pointcloud_atlas_builder/pointcloud_atlas_io.h"


#include <vector>
#include <string>
#include <qstringlist.h>

using namespace std;

// define number of fields other than cell name and x,y,z positions, these include intensity terms and size term
#define FIELDNUM 5 

class MarkerStacks
{
public:
	string markername;
	string registered_file;
	vector <string> signal_ch_filename;
	vector <string> reference_ch_filename;
};

class MarkerGeneInfo
{
public: 
	string reference_stack_filename; //for registration
	string reference_markername;
	
	int markernum() {return marker_stacks.size();}
	vector <MarkerStacks> marker_stacks;
	
	MarkerGeneInfo()
	{
		reference_stack_filename = "";
	}
	
	void append(MarkerGeneInfo newinfo)
	{
		for (int i = 0; i<newinfo.markernum(); i++)
			marker_stacks.push_back(newinfo.marker_stacks.at(i));
		
		reference_stack_filename = newinfo.reference_stack_filename;
	}
	
	void print()
	{
		int i,j;
		
		printf("reference_stack_filename = %s\n", reference_stack_filename.c_str());
		printf("reference_markername = %s\n", reference_markername.c_str());
		
		printf("markernum = %d\n", markernum());
		
		for (i=0; i<markernum(); i++)
		{
			printf("marker name = %s\n", marker_stacks.at(i).markername.c_str());
			printf("marker registered file = %s\n", marker_stacks.at(i).registered_file.c_str());
			
			for (j=0; j<marker_stacks.at(i).signal_ch_filename.size(); j++)
				printf("signal channel filename = %s\n", marker_stacks.at(i).signal_ch_filename.at(j).c_str());
				
			for (j=0; j<marker_stacks.at(i).reference_ch_filename.size(); j++)
				printf("reference channel filename = %s\n", marker_stacks.at(i).reference_ch_filename.at(j).c_str());
			
			printf("\n");
		}
	}
};


//revised from CellAPO in basic_surf_objs.h
class CellAPO_FL
{
public:
	int n;				// index
	string orderinfo;
	string name;
	string comment;
	float x, y, z;		// point coordinates
	float intensity;
	float sdev, pixmax, mass;
	float volsize;		// volume size
	RGBA8 color;
	bool on;
	bool selected;
	
	CellAPO_FL()
	{
		n=x=y=z=intensity=volsize=sdev=pixmax=mass=0; 
		color.r=color.g=color.b=color.a=0; 
		on=selected=false;
		name=comment=orderinfo="";
	}
	
};


//functions related to t-test, copied from elementfunc/stat/tutest.cpp
void avevar(float data[], unsigned long n, float *ave, float *var);
void tutest(float data1[], unsigned long n1, float data2[], unsigned long n2, float *t, float *prob);

// read in lines of a txt format file, and put each line into a vector of string
void readLines(string filename, vector <string> &namelist, int &num);

// read in lines of a txt format file, and put each line into QStringList
void readLines(string filename, QStringList &namelist, int &num);

void readLines(string filename, vector <QStringList> &namelist, int &num);

void getCells(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, float *markerGeneCellArrayPos, float *markerGeneCellArraryOtherFields);//get cell matriices from a number of stacks


////generate matrices needed for registration and statistical analysis, all the cells in the reference channels are used for registration
//void genMatrices(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, 
//				float *&markerGeneCellArrayPos, float *&markerGeneCellArraryOtherFields, float *&refGeneCellArrayPos, float *&refGeneCellArrayOtherFields, 
//				int *&marker_ref_map, bool *&markerTag, int &stacknum, int &refstacknum, vector <string> &reference_stack_list);

// generate matrices needed for registration and statistical analysis, this function allows not all cells in reference channel to be used for registration
// parameters ref_gene_controlpoints_cellnamelist and refGeneCptCellArrayPos are added
void genMatrices(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				 float *&markerGeneCellArrayPos, float *&markerGeneCellArrayOtherFields, 
				 float *&refGeneCellArrayPos, float *&refGeneCellArrayOtherFields,  float *&refGeneCptCellArrayPos,
			int *&marker_ref_map, bool *&markerTag, int &stacknum, int &refstacknum, vector <string> &reference_stack_list);


// get atlas cells from an APO file, select those that have been annotated, and save them in float arrays
void getAtlasCell(string filename, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, float *&cellArray, float *&cptCellArray, float *&cellArrayOtherFields);

//compute the mean and standard deviation of each cell of multiple annotated stacks
//void computeCellStat(float *subjectCellArrayReg, float *subjectCellArrayOtherFields, int annocellnum, int stacknum, float *&cellMean, float *&cellStd);
void computeCellStat(float *markerGeneCellArray, float *refGeneCellArray, int *ref_gene_idx, int *dim_marker_gene, int *dim_ref_gene, float *&cellMean, float *&cellStd, int *&observed);

void mergeCell(float *markerGeneCellArray, float *refGeneCellArray, int *ref_gene_idx, int *dim_marker_gene, int *dim_ref_gene, 
			   vector <QStringList> final_cellnamelist, int final_cellnum, vector <string> cellnamelist, int *observed, int *expected, bool *markerTag, int markercnt,
			   float *&cellMean_final, float *&cellStd_final, int *&observed_final, int *&expected_final, bool *&markerTag_final);

// compute for each cell its expected value (i.e., in how many stacks it should express)
void computeExpectedValue(MarkerGeneInfo marker_gene_info, string dfile_outputdir, int refstacknum, int *&expected);

// called by v3d plug-in
void computeExpectedValue(MarkerGeneInfo marker_gene_info, string dfile_inputdir,  string dfile_outputdir, 
                          string cellnamelist_prefix, string cellnamelist_surfix, 
                          int refstacknum, int *&expected);


void registerStacks(float *targetCptCellArrayPos, float *markerGeneCellArrayPos, float *refGeneCellArrayPos, int *marker_ref_map, int *dim_marker_gene, int *dim_ref_gene,
					float *&markerGeneCellArrayPosNew, float *&refGeneCellArrayPosNew);

void registerStacks(float *targetCptCellArrayPos, float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *refGeneCptCellArrayPos, int *marker_ref_map, int *dim_marker_gene, int *dim_ref_gene, int *dim_ref_gene_cpt, float *&markerGeneCellArrayPosNew, float *&refGeneCellArrayPosNew);


//rebuild the atlas, target known for registration, allow co-expresssion analysis for only two apo files
void rebuildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				  vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 
				  int *ref_gene_idx, string dfile_outputdir, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum, string atlas_subject_filename, string atlas_target_filename);

//rebuild the atlas,  target unknown for registration
void rebuildAtlas(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				 vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 				  
				  int *ref_gene_idx, string dfile_outputdir, string &dfile_target, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum);

//rebuild the atlas,  target known for registration, allow co-expression analysis for any number of apo files
void rebuildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				  vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 
				  int *ref_gene_idx, string dfile_outputdir, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum, string test_colocalization_atlas_apo_filename);


// for v3d plug-in
// old version
//void buildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
//                vector <string> forceAddCellnamelist,  vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, int *ref_gene_idx, string dfile_outputdir, float threratio);

void buildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist,
                vector <string> forceAddCellnamelist,  vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, int *ref_gene_idx,  
                atlasConfig & cfginfo, apoAtlasBuilderInfo & atlasbuiderinfo, float threratio);

// new version called by v3d plugin
void buildAtlas(apoAtlasBuilderInfo & atlasbuiderinfo, atlasConfig & cfginfo, apoAtlasLinkerInfoAll &apoinfo);

// detect outlier cell positions
void outlierDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, bool *markerTag, 
                     float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, 
					 int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list, int *ref_gene_idx);



// detect localized cells, co-localized cells are potentially the same cell that should be combined in the atlas
void coLocalizationDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist,  vector <string> ref_gene_cellnamelist, bool *markerTag, 
			  float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, 
			  int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, int *ref_gene_idx);

void coLocalizationDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, bool *markerTag, 
							 float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd,
							 int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, int *ref_gene_idx, float threratio, int *observed, int *expected);

void coLocalizationDetection(string atlas_subject_filename, string atlas_target_filename,  string dfile_outputdir);

void coLocalizationDetection(string test_colocalization_atlas_apo_filename, string dfile_outputdir);
//void coLocalizationDetection(vector <string> atlas_apo_filelist, string dfile_output);

//void coLocalizationDetection(QStringList atlas_apo_filelist, QString dfile_output);
// used by v3d plugin
void coLocalizationDetection(QStringList atlas_apo_filelist, QString dfile_output, float thredis, bool symmetryRule, bool bilateralRule);

//void merge_atlas(QStringList atlas_apo_filelist, QString dfile_merge_cells, QString dfile_output);
void merge_atlas(QStringList atlas_apo_filelist, QString dfile_merge_cells, QString dir_output, QString cellstat_file_surfix, 
                 QString markermap_file_name, QString merged_atlas_prefix, QStringList celltype_file_name_list);

// save atlas
//void saveAtlas(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, bool *markerTag, 
//			   float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellMean_vol, float *cellStd_vol, int *observed, int *expected, 
//			   int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list,int *ref_gene_idx, float threratio, bool final_tag);			  

void saveAtlas(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, bool *markerTag, vector <string> forceAddCellnamelist, 
			   vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames, 
			   float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellMean_vol, float *cellStd_vol, int *observed, int *expected, 
			   int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list,int *ref_gene_idx, float threratio, bool final_tag);			  


void saveAtlasBeforeMerge(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, bool *markerTag, vector <string> forceAddCellnamelist, 
                          vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames,
                          float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellMean_vol, float *cellStd_vol, int *observed, int *expected, 
                          int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list,int *ref_gene_idx, float threratio);		  

void saveAtlasAfterMerge(float *cellMean_final, float *cellMean_vol_final, int *observed_final, int *expected_final, unsigned char *markerTag_final, 
                         int final_cellnum, vector <QStringList> final_cellnamelist, vector <vector <string> > markerName, vector <string> allMarkerSet, 
                         vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames, string dfile_outputdir, QString cellstat_file_surfix,QString merged_atlas_prefix);		  

#endif