// basic functions of atlas builder
// FL
// 20090515
// 20110722, add trim space in cell names to avoid inconsistency in manual annotation and cell name list

// read each line in a text file, and put them into a string vector
// input: filename -- the name of the text file
// output: namelist -- the string vector that saves each line of the text file
//         num --- number of lines

#include <QtGui>

#include "../../../../v3d_external/v3d_main/basic_c_fun/color_xyz.h"
#include "FL_registerAffine.h"
#include "FL_atlasBuilder.h"
//#include "converter_pcatlas_data.h"
//#include "../../elementfunc/stat/pbetai.cpp"
#include "pbetai.cpp"

#include "../../../../v3d_external/v3d_main/cellseg/FL_sort2.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <stdlib.h>

#include "converter_pcatlas_data.h"

#undef  DEBUG

using namespace std;


static float sqrarg;
#define SQR(a) ((sqrarg=(a))==0.0 ? 0.0: sqrarg*sqrarg)



//
// read in lines of a txt format file, each line contains one string
// put the string in the entire file into a vector of string
void readLines(string filename, vector <string> &namelist, int &num)
{
	QString name;
	char tmps[1024];
	
	ifstream ifscn (filename.c_str(), ifstream::in);
	int cnt = 0;
	
	while (!ifscn.eof())
	{
		ifscn.getline(tmps, 1024);
//		printf("tmps = %s\n", tmps);
		name = tmps;
		if (name.trimmed().isEmpty())
			continue;
		printf("%s\n", (char *)qPrintable(name));
		namelist.push_back((char *)qPrintable(name));
		cnt++;
	}	
	
	
//	for (int i=0; i<cnt; i++)
//		printf("%d, %s\n", i, namelist.at(i).c_str());
		
	num = (int)namelist.size(); //number of elements	
	ifscn.close();
}


// read in lines of a txt format file, each line contains one string
// put the string in the entire file into QStringList
void readLines(string filename, QStringList &namelist, int &num)
{
	QString name;
	char tmps[1024];
	
	ifstream ifscn (filename.c_str(), ifstream::in);
	int cnt = 0;
	
	while (!ifscn.eof())
	{
		ifscn.getline(tmps, 1024);
		name = tmps;
		if (name.trimmed().isEmpty())
			continue;
		namelist.push_back((char *)qPrintable(name));
		cnt++;
	}	
	
	num = (int)namelist.size(); //number of elements	
	ifscn.close();
}


// read in lines of a txt format file, each line contain multiple strings separated by ','
// put each line into a QStringList, and the entire content of the file into a QStringList array
void readLines(string filename, vector <QStringList> &namelist, int &num)
{
	QString name;
	QStringList namelist_tmp;
	char tmps[1024];
	
	long i,j;
	
	ifstream ifscn (filename.c_str(), ifstream::in);
	
	if (!ifscn.is_open())
	{
		printf("file open error (file might not exist).");
		return;
	}
	

	int cnt = 0;
	
	while (!ifscn.eof())
	{
		ifscn.getline(tmps, 1024);
		name = tmps;
		if (name.trimmed().isEmpty())
			continue;
		namelist_tmp.push_back((char *)qPrintable(name));
		cnt++;
	}	
	
	num = (int)namelist_tmp.size(); //number of lines	
	ifscn.close();
	
//	namelist = new QStringList [num];
	char sep_char[] = ":, ";
	
	for (long i=0; i<num; i++)
	{
		QStringList mytmp;
		
		j = 0;
		int startpos = j;
		int endpos = j;
		
		
		while (j<namelist_tmp.at(i).size())
		{
			if ((namelist_tmp.at(i).at(j) == (QChar)sep_char[0])||(namelist_tmp.at(i).at(j) == (QChar)sep_char[1]))
//			if ((namelist_tmp.at(i).at(j).compare(":")  == 0)||(namelist_tmp.at(i).at(j).compare(",") == 0))
			
			{
				endpos = j-1;
				mytmp.push_back((char *)qPrintable(namelist_tmp.at(i).mid(startpos, (endpos-startpos+1))));
				
//				namelist[i].push_back((char *)qPrintable(namelist_tmp.at(i).mid(startpos, (endpos-startpos+1))));
				
				while ((j<namelist_tmp.at(i).size()) && ((namelist_tmp.at(i).at(j) == (QChar)sep_char[0]) || (namelist_tmp.at(i).at(j) == (QChar)sep_char[1])|| (namelist_tmp.at(i).at(j) == (QChar)sep_char[2])))
//				while ((j<namelist_tmp.at(i).size()) && ((namelist_tmp.at(i).at(j).compare(":") == 0) || (namelist_tmp.at(i).at(j).compare(",") == 0)|| (namelist_tmp.at(i).at(j).compare(" ") == 0)))
				
					j++;
				
				startpos = j;
				endpos = startpos;
			}
			else
				j++;
			
		}
		
		namelist.push_back(mytmp);
		
//		// print result to check if the read function works
//		for (j=0; j<namelist[i].size())
//		{
//			printf("%s, ", qPrintable(namelist(i).at(j)))
//		}
//		printf("\n");
	}
}



void trimStringSpace(string str, string &strnew)
{
	long cnt = 0;
	strnew = str;
	
	for (int i=0; i<str.length(); i++)
	{
		if ((str[i]!=' ') && (str[i]!='\t') && (str[i]!='\n') && (str[i]!='\v') && (str[i]!='\f') && (str[i]!='\r'))
		{
			strnew[cnt] = str[i]; 
			cnt++;
		}
	}
	strnew = strnew.substr(0, cnt);
	
//	printf("%s\n", str.c_str());
//	printf("%s\n", strnew.c_str());
	
	
//	return(strnew);
}



//**************************************************************************
// codes called by atlas builder plugin
//**************************************************************************

void buildAtlas(apoAtlasBuilderInfo & atlasbuiderinfo, atlasConfig & cfginfo, apoAtlasLinkerInfoAll &apoinfo)
{

    int i,j,k,m;


    // ---------------------------------------------------------------
    // load and parse linker file, generate marker_gene_info
    // ---------------------------------------------------------------

//    apoAtlasLinkerInfoAll apoinfo;

    //	QString fileName = QFileDialog::getOpenFileName(0, QString("Select an atlas linker file (*.pc_atlas"),
    //													"",
    //													QString("point cloud atlas linker file (*.pc_atlas)"));
    //    if (fileName.isEmpty())
    //	{
    //		v3d_msg("No file is selected. Do nothing.");
    //		return;
    //	}
    //	else
    //	{
    //		v3d_msg("You've selected a linker file. An atlas will be built.");
    //	}

    //    if (!loadPointCloudAtlasInfoListFromFile(qPrintable(fileName.trimmed()), apoinfo))

//    QString fileName = atlasbuiderinfo.linkerFileName; 
//
//    if (!loadPointCloudAtlasInfoListFromFile(qPrintable(fileName.trimmed()), apoinfo))    
//    {
////        v3d_msg("Fail to load the specified point cloud atlas file.");
//        printf("Fail to load the specified point cloud atlas file.");
//        return;
//    }

    QString fileName = atlasbuiderinfo.linkerFileName;     
    MarkerGeneInfo marker_gene_info;
    int markerNum; //number of new markers

    convert_FLAtlasBuilderInfo_from_apoAtlasLinkerInfo(marker_gene_info, apoinfo);	

    markerNum = marker_gene_info.markernum();
    marker_gene_info.print();

    printf("parse linker file finish\n");

    //    QFileInfo info(fileName);	
    //    QString dfile_outputdir_tmp = info.dir().path(); //temporary	
    //    string dfile_outputdir = qPrintable(dfile_outputdir_tmp+"/");


    // ---------------------------------------------------------------
    // assign file names
    // ---------------------------------------------------------------
    string dfile_inputdir;
    string dfile_outputdir;
    
    if (cfginfo.inputFolder.mid(cfginfo.inputFolder.size()-1,1).compare("/")==0)
    {
        dfile_inputdir = cfginfo.inputFolder.toStdString();
        dfile_outputdir = cfginfo.outputFolder.toStdString();
    }
    else
    {
        dfile_inputdir = (cfginfo.inputFolder + "/").toStdString();
        dfile_outputdir = (cfginfo.outputFolder + "/").toStdString();
    }
    


    //    string refgene_cellnamelist_filename = dfile_outputdir + "cellnamelist_eve.txt";
    //    string global_cellnamelist_filename = dfile_outputdir + "cellnamelist_atlas.txt"; //name of the cell name list file used to build the atlas	
    //    string dfile_force_add = dfile_outputdir + "forceAddCells.txt"; //name of the file containing cells that must be included in the atlas	
    //    string refgene_controlpoints_cellnamelist_filename = dfile_outputdir + "cellnamelist_eve_nocluster.txt";
    //   string refgene_controlpoints_cellnamelist_filename = dfile_inputdir + cfginfo.refMarkerCptCellNameFile;
    //	string dfile_marker_info = dfile_inputdir + "import.pc_atlas"; //name of the linker file saving the marker information
    //    string dfile_marker_info = qPrintable(fileName); //name of the linker file saving the marker information
    //    string dfile_target = marker_gene_info.reference_stack_filename;

    string refgene_cellnamelist_filename = dfile_inputdir + cfginfo.cellNameFilePrefix.toStdString() + "_" 
                                            + apoinfo.referenceMarkerName.toStdString() + cfginfo.cellNameFileSurfix.toStdString();
    string global_cellnamelist_filename = dfile_inputdir + cfginfo.cellNameFilePrefix.toStdString() + "_atlas" 
                                            + cfginfo.cellNameFileSurfix.toStdString(); //name of the cell name list file used to build the atlas	
    
    string dfile_force_add = atlasbuiderinfo.forceAddCellFileName.toStdString(); //name of the file containing cells that must be included in the atlas	
//    string refgene_controlpoints_cellnamelist_filename = cfginfo.refMarkerCptCellNameFile.toStdString(); // refMarkerCptCellNameFile has path information

    string refgene_controlpoints_cellnamelist_filename = atlasbuiderinfo.refMarkerCptCellNameFile.toStdString(); // refMarkerCptCellNameFile has path information
    
    string dfile_marker_info = fileName.toStdString(); //name of the linker file saving the marker information
    string dfile_target = atlasbuiderinfo.regTargetFileName.toStdString();
    
    
    string cellnamelist_prefix = cfginfo.cellNameFilePrefix.toStdString();
    string cellnamelist_surfix = cfginfo.cellNameFileSurfix.toStdString();

    printf("%s\n", refgene_cellnamelist_filename.c_str());
    printf("%s\n", global_cellnamelist_filename.c_str());
    printf("%s\n", dfile_force_add.c_str());
    printf("%s\n", refgene_controlpoints_cellnamelist_filename.c_str());
    printf("%s\n", dfile_marker_info.c_str());
    printf("%s\n", dfile_target.c_str());
    printf("%s\n", cellnamelist_prefix.c_str());
    printf("%s\n", cellnamelist_surfix.c_str());
    
    

    // ---------------------------------------------------------------
    // generate cellnamelist_atlas.txt (contain the name of all the cells)
    // ---------------------------------------------------------------
    QStringList cellNamesAll, namelist;
    //	string cellnamelist_filename = dfile_inputdir + "cellnamelist_eve.txt";
    string cellnamelist_filename = refgene_cellnamelist_filename;

    int num;

    //	if (cellnamelist_filename.empty())
    //	{
    //		v3d_msg(QString("The cell name list file : %1. does not exist under the output folder\n").arg(cellnamelist_filename.c_str()));	
    //		return;
    //	}

    readLines(cellnamelist_filename, namelist, num);	
    cellNamesAll << namelist;

    for (i=0; i<markerNum; i++)
    {
        printf("i=%d\n", i);
        
        string cellnamelist_filename = dfile_inputdir + cfginfo.cellNameFilePrefix.toStdString() + "_" + marker_gene_info.marker_stacks.at(i).markername+".txt";
        
        readLines(cellnamelist_filename, namelist, num);	
        cellNamesAll << namelist;
        
    }

    // write the cellnamelist_atlas.txt
    writeCellNameList2File((const QString)(global_cellnamelist_filename.c_str()), uniqueCellNameList((const QStringList)cellNamesAll));

    printf("finish writing global cellname list file.\n");
    
    // ---------------------------------------------------------------
    // read cell name file, get the number of annotated cells in total
    // ---------------------------------------------------------------

    vector <string> cellnamelist;
    int annocellnum;
    readLines(global_cellnamelist_filename, cellnamelist, annocellnum);

//	//output cellnamelist to check if it is correct
//	for (i=1; i<=annocellnum; i++)
//	{
//		printf("%d, %s\n", i, cellnamelist.at(i-1).c_str());
//	}

    printf("finish reading global cellname list file.\n");

    // -----------------------------------------------------------------------------
    // read reference gene cell name file, get the number of annotated marker cells in total
    // -----------------------------------------------------------------------------	

    printf("%s\n", refgene_cellnamelist_filename.c_str());
    
    vector <string> ref_gene_cellnamelist;
    int ref_gene_annocellnum;
    readLines(refgene_cellnamelist_filename, ref_gene_cellnamelist, ref_gene_annocellnum);
    printf("finish reading refgene_cellnamelist_filename.\n");
    
    // -----------------------------------------------------------------------------
    // read control point cells in the reference gene, get the number of control points
    // -----------------------------------------------------------------------------		
    vector <string> ref_gene_controlpoints_cellnamelist;
    int ref_gene_controlpoints_annocellnum =0;

    readLines(refgene_controlpoints_cellnamelist_filename, ref_gene_controlpoints_cellnamelist, ref_gene_controlpoints_annocellnum);

    //#ifdef DEBUG	
    //output cellnamelist to check if it is correct
    for (i=1; i<ref_gene_controlpoints_annocellnum; i++)
    {
        printf("%d, %s\n", i, ref_gene_controlpoints_cellnamelist.at(i-1).c_str());
    }
    //#endif

    printf("finish reading ref_gene_controlpoints_cellnamelist.\n");

    // -----------------------------------------------------------------------------
    // read force add cell names
    // -----------------------------------------------------------------------------		
    vector <string> forceAddCellnamelist;
    int forceaddCellNum = 0;

    readLines(dfile_force_add, forceAddCellnamelist, forceaddCellNum);

    //	//output cellnamelist to check if it is correct
    //	for (i=1; i<forceaddCellNum; i++)
    //	{
    //		printf("%d, %s\n", i, forceAddCellnamelist.at(i-1).c_str());
    //	}
    
    printf("finish reading dfile_force_add.\n");
    

    // --------------------------------------------
    // read cell name list of different cell types 
    // --------------------------------------------

    vector <string> cellTypeNames;
    //
    //    cellTypeNames.push_back("interneuron");
    //    cellTypeNames.push_back("motoneuron");
    //    cellTypeNames.push_back("secretory");
    //    cellTypeNames.push_back("galia");
    //
    //
    int cellTypeNum[cfginfo.cellTypeFileName.size()];
    //
    vector < vector <string> > cellTypeNamelist;

    for(i=0; i<cfginfo.cellTypeFileName.size(); i++)
    {
//        string dfile_cellTypes = dfile_inputdir + cfginfo.cellTypeNames.at(i) + cfginfo.cellNameFileSurfix;
        string dfile_cellTypes = cfginfo.cellTypeFileName.at(i).toStdString(); //cellTypeNames has folder and surfix information
        
        printf("%s\n", dfile_cellTypes.c_str());
        
        vector <string> tmp;
        readLines(dfile_cellTypes, tmp, cellTypeNum[i]);
        cellTypeNamelist.push_back(tmp);
        
        // assign value to cellTypeNames
        int pos1 = cfginfo.cellTypeFileName.at(i).lastIndexOf("_", -1);        
        int pos2 = cfginfo.cellTypeFileName.at(i).lastIndexOf(".", -1);
        
        QString strtmp = cfginfo.cellTypeFileName.at(i).mid(pos1+1, pos2-pos1-1);
        cellTypeNames.push_back(strtmp.toStdString());
        
        
        //		//#ifdef DEBUG	
        //		//output cellnamelist to check if it is correct
        //		for (j=0; j<cellTypeNum[i]; j++)
        //		{
        //			printf("%d, %s\n", j, cellTypeNamelist[i].at(j).c_str());
        //		}
        //		
        //		printf("\n");
        //		
        //		//#endif
    }

    printf("finish reading cell type files.\n");

    // -----------------------------------------------------------------------------
    // set up the corresponding index for the cells of cellnamelist 
    // in ref_gene_cellnamelist, this is used for cell position statistics analysis
    // -----------------------------------------------------------------------------	

    int *ref_gene_idx = new int [annocellnum];

    for (i=0; i<annocellnum; i++)
    {
        ref_gene_idx[i] = -1;
        for (j=0; j<ref_gene_annocellnum; j++)
        {
            if (cellnamelist.at(i).compare(ref_gene_cellnamelist.at(j))==0)
            {
                ref_gene_idx[i] = j;
                break;
            }
        }
        //		printf("i = %d, %d\n", i, ref_gene_idx[i]);
    }


    // -----------------------
    // build the atlas 
    // -----------------------
    
    printf("begin to build atlas\n");


    //	vector <string> ref_gene_controlpoints_cellnamelist;

//    printf("\n**** in pointcloud_atlas_builder.cpp: reference stack: %s\n", dfile_target.c_str());


    //    float threratio = 0.3;
    //    buildAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
    //               forceAddCellnamelist,  cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, threratio);
    
    float threratio = atlasbuiderinfo.ratio;
    printf("*******  ratio = %f *********\n", threratio);
    
    
//    buildAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
//               forceAddCellnamelist,  cellTypeNamelist, cellTypeNames, ref_gene_idx, cfginfo, atlasbuiderinfo, threratio);
//

//    printf("\n**** in pointcloud_atlas_builder.cpp: reference stack: %s\n", dfile_target.c_str());


	
	FILE *file; 
	string outfilename;
	int fieldnum = FIELDNUM + 3;
	
	// -----------------------------------------------------------
	// generate matrices for registration and statistical analysis
	// -----------------------------------------------------------	
	bool *markerTag;	
	float *markerGeneCellArrayPos = 0; //marker gene arrary containing only (x,y,z), for registration	
	float *markerGeneCellArrayOtherFields = 0;	//marker gene arrary containing intesity and volume information	
	float *refGeneCellArrayPos = 0; //reference gene (e.g., EVE) arrary containing only (x,y,z) 
	float *refGeneCptCellArrayPos = 0; // reference gene array containing only control points for registration, note refGeneCptCellArrayPos cab be a subset of refGeneCptCellArrayPos
	float *refGeneCellArrayOtherFields = 0; //the reference gene (e.g., EVE) arrary containing intensity and volume information
	int *marker_ref_map =0; // a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
	int stacknum, refstacknum;
	vector <string> reference_stack_list;
	
	// for debug purpose
#ifdef DEBUG
	
	MarkerStacks markerstack;
	int cnt = 0;
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.reference_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			printf("i=%d, j=%d, cnt=%d, stack= %s, ref= %s\n", i, j, cnt, markerstack.signal_ch_filename.at(j).c_str(), markerstack.reference_ch_filename.at(j).c_str());
			cnt++;
		}
	}
#endif
	
	//	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, 
	//				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  
	//				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist,
				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  refGeneCptCellArrayPos, 
				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
	
	// --------------
	// registration
	// --------------	
	
	// read cell position in target file
	float *targetCellArrayPos = 0; // all annotated cell array					
	float *targetCptCellArrayPos =0; // control point (annotated) array
	float *targetCellArrayOtherFields = 0; 
	
	printf("target file = %s\n", dfile_target.c_str());
	
	
	//	getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	getAtlasCell(dfile_target, cellnamelist, ref_gene_controlpoints_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	
	// targetCellArrayPos and targetCellArrayOtherFields are no use
	if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
	if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}
	
	// do registration
	int dim_marker_gene[3], dim_ref_gene[3], dim_ref_gene_cpt[3];
	
	dim_marker_gene[0] = 3;
	dim_marker_gene[1] = cellnamelist.size();
	dim_marker_gene[2] = stacknum;
	
	dim_ref_gene[0] = 3;
	dim_ref_gene[1] = ref_gene_cellnamelist.size();
	dim_ref_gene[2] = refstacknum;
	
	dim_ref_gene_cpt[0] = 3;
	dim_ref_gene_cpt[1] = ref_gene_controlpoints_cellnamelist.size();
	dim_ref_gene_cpt[2] = refstacknum;
	
	float *refGeneCellArrayPosNew=0, *markerGeneCellArrayPosNew=0;
	
	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
	
	
	// ------------------------------------------------
	// save data before registration 
	// ------------------------------------------------
    
    if (atlasbuiderinfo.saveRegDataTag)
    {
        FILE *regfile1, *regfile2;
        string registeredfilename1 = dfile_outputdir + "before_registration_marker.txt";
        regfile1 = fopen(registeredfilename1.c_str(), "wt");
        
        string registeredfilename2 = dfile_outputdir + "before_registration_ref.txt";
        regfile2 = fopen(registeredfilename2.c_str(), "wt");
        
        // write regfile1
        for (i=0; i<marker_gene_info.markernum(); i++)
        {
            
            MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
            int markerstacknum = (markerstack.signal_ch_filename).size();
            
            for (j=0; j<markerstacknum; j++)
            {
                
                fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
                
                for (m=0; m<dim_marker_gene[1]; m++)
                {
                    int mm = m*3;
                    int qq = 3*dim_marker_gene[1]*j + mm;
                    if ((markerGeneCellArrayPos[qq]==0)&(markerGeneCellArrayPos[qq+1]==0)&(markerGeneCellArrayPos[qq+2]==0))
                        continue;
                    else
                        fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
                }
                
                
            } // for j end
        }
        fclose(regfile1);
        
        // write regfile2
        for (j=0; j<refstacknum; j++)
        {
            
            fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
            
            for (m=0; m<dim_ref_gene[1]; m++)
            {
                int mm = m*3;
                int qq = 3*dim_ref_gene[1]*j + mm;
                if ((refGeneCellArrayPos[qq]==0)&(refGeneCellArrayPos[qq+1]==0)&(refGeneCellArrayPos[qq+2]==0))
                    continue;
                else
                    fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPos[qq], refGeneCellArrayPos[qq+1], refGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
                
            }
        }				
        fclose(regfile2);
        
        // ------------------------------------------------
        // save data after registration 
        // ------------------------------------------------
        
        
        registeredfilename1 = dfile_outputdir + "after_registration_marker.txt";
        regfile1 = fopen(registeredfilename1.c_str(), "wt");
        
        registeredfilename2 = dfile_outputdir + "after_registration_ref.txt";
        regfile2 = fopen(registeredfilename2.c_str(), "wt");
        
        
        // write regfile1
        for (i=0; i<marker_gene_info.markernum(); i++)
        {
            
            MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
            int markerstacknum = (markerstack.signal_ch_filename).size();
            
            for (j=0; j<markerstacknum; j++)
            {
                
                fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
                
                for (m=0; m<dim_marker_gene[1]; m++)
                {
                    int mm = m*3;
                    int qq = 3*dim_marker_gene[1]*j + mm;
                    if ((markerGeneCellArrayPosNew[qq]==0)&(markerGeneCellArrayPosNew[qq+1]==0)&(markerGeneCellArrayPosNew[qq+2]==0))
                        continue;
                    else
                        fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
                }
                
                
            } // for j end
        }
        fclose(regfile1);
        
        // write regfile2
        for (j=0; j<refstacknum; j++)
        {
            
            fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
            
            for (m=0; m<dim_ref_gene[1]; m++)
            {
                int mm = m*3;
                int qq = 3*dim_ref_gene[1]*j + mm;
                if ((refGeneCellArrayPosNew[qq]==0)&(refGeneCellArrayPosNew[qq+1]==0)&(refGeneCellArrayPosNew[qq+2]==0))
                    continue;
                else
                    fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
                
            }
        }				
        fclose(regfile2);
 
        
        
        
//        // ------------------------------------------------        
//        // save registered reference stack to .apo files,
//        // this is for paper purpose, no need for the final tools
//        // ------------------------------------------------
//        
//        for (j=0; j<refstacknum; j++)
//        {
//            
//            size_t found = 0;
//            found=reference_stack_list.at(j).find_last_of("/");
//            string reference_stack_name = reference_stack_list.at(j).substr(found+1,reference_stack_list.at(j).size()-found);
//
//            
//            string registeredReferenceApo = dfile_outputdir + "reference_apo_4paper/" + reference_stack_name.c_str();
//            FILE *f = fopen(registeredReferenceApo.c_str(), "wt");
//            
////            fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
//            
//            for (m=0; m<dim_ref_gene[1]; m++)
//            {
//                int mm = m*3;
//                int qq = 3*dim_ref_gene[1]*j + mm;
//                int p = j*FIELDNUM*dim_ref_gene[1]+FIELDNUM*m;
//                string tmp = "<$EVE>";
//                
////                fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
////                        m,m,ref_gene_cellnamelist.at(m).c_str(),tmp.c_str(),int(refGeneCellArrayPosNew[qq]+0.5), int(refGeneCellArrayPosNew[qq＋1]+0.5), int(refGeneCellArrayPosNew[qq＋2]+0.5),refGeneCellArrayOtherFields[p], refGeneCellArrayOtherFields[p+1], refGeneCellArrayOtherFields[p+2], refGeneCellArrayOtherFields[p+3], refGeneCellArrayOtherFields[p+4]);
//                
////                printf("j=%d, m=%d, refGeneCellArrayOtherFields = %ld\n", j,m, refGeneCellArrayOtherFields);
////                printf("%5.3f\n",refGeneCellArrayOtherFields[p]);
////                printf("%5.3f\n",refGeneCellArrayOtherFields[p+1]);
////                printf("%5.3f\n",refGeneCellArrayOtherFields[p+2]);
////                printf("%5.3f\n",refGeneCellArrayOtherFields[p+3]);
////                printf("%5.3f\n",refGeneCellArrayOtherFields[p+4]);
//                
//                fprintf(f, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",m,m,ref_gene_cellnamelist.at(m).c_str(),tmp.c_str(),int(refGeneCellArrayPosNew[qq+2]+0.5), int(refGeneCellArrayPosNew[qq]+0.5), int(refGeneCellArrayPosNew[qq+1]+0.5),refGeneCellArrayOtherFields[p], refGeneCellArrayOtherFields[p+1], refGeneCellArrayOtherFields[p+2], refGeneCellArrayOtherFields[p+3],refGeneCellArrayOtherFields[p+4]);
//                
//                
//            }
//            
//            fclose(f);
//
//        }				
      
        
    }
	
    
    
    
	
	// ------------------------------------------------
	// analyze cell statistics
	// note that reference channel (e.g., EVE) cell statistics 
	// need to be combined with the same cells in marker channel 
	// ------------------------------------------------
	float *cellMean = 0;
	float *cellStd = 0;
	int *observed = 0;
	int *expected = 0;
	
	//position mean and standard deviation
	//	computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
	computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
	
	//as observed and expected will be computed when analyzing volume statistcs as below, they are deleted here to avoid memory leak
	if (observed) {delete []observed; observed=0;}
	//	if (expected) {delete []expected; expected=0;}
	
	int lenn = dim_marker_gene[1]*dim_marker_gene[0];
	float aveCellStd=0;
	
	for (j=0; j<lenn; j++) // compute the sum of std along each dimension and for each cell
	{
		aveCellStd += cellStd[j];
	}
	
	aveCellStd /= lenn;
	
	printf("aveCellStd = %f\n", aveCellStd);
	
	
	//size mean and standard deviation
	float *cellMean_vol;
	float *cellStd_vol;
	
	int dim_marker_gene_vol[3], dim_ref_gene_vol[3];
	
	dim_marker_gene_vol[0] = 1;
	dim_marker_gene_vol[1] = cellnamelist.size();
	dim_marker_gene_vol[2] = stacknum;
	
	dim_ref_gene_vol[0] = 1;
	dim_ref_gene_vol[1] = ref_gene_cellnamelist.size();
	dim_ref_gene_vol[2] = refstacknum;
	
	
	float *markerGeneCellArray = new float [dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*dim_marker_gene_vol[2]];
	
	for (j=0; j<dim_marker_gene_vol[2]; j++)
	{
		int mm1 = dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_marker_gene[1]*j;
		
		for (i=0; i<dim_marker_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_marker_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			markerGeneCellArray[nn1] = markerGeneCellArrayOtherFields[nn2+3];
		}
	}
	
	float *refGeneCellArray = new float [dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*dim_ref_gene_vol[2]];
	
	for (j=0; j<dim_ref_gene_vol[2]; j++)
	{
		int mm1 = dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_ref_gene[1]*j;
		
		for (i=0; i<dim_ref_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_ref_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			refGeneCellArray[nn1] = refGeneCellArrayOtherFields[nn2+3];
		}
	}
	
	//	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed, expected);
	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed);
	
	// -----------------------------------------------------------------------------------------------
	// compute for each cell its expected value (i.e., in how many stacks it should express
	// -----------------------------------------------------------------------------------------------
//	computeExpectedValue(marker_gene_info, dfile_outputdir, refstacknum, expected);
	computeExpectedValue(marker_gene_info,  dfile_inputdir, dfile_outputdir, cellnamelist_prefix, cellnamelist_surfix, refstacknum, expected);
	
	// ------------------
	// save atlas files
	// ------------------	
    //	bool final_tag = 0;
    
    
    //output cellnamelist to check if it is correct
	for (i=1; i<=annocellnum; i++)
	{
		printf("%d, %s\n", i, cellnamelist.at(i-1).c_str());
	}
    
    printf("check cellnamelist before saveAtlasBeforeMerge.\n");

	
    // 20111206 comment, need it    
	saveAtlasBeforeMerge(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
                         markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
                         dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio);	
    
    
    //----------------------------------------------------------------------------------------
    // generate temporary data for Ellie to check if merge function work correctly, when generating
    // the non-merged atlas, need to set genMergeStdev =0
    //----------------------------------------------------------------------------------------
    
    int genMergeStdev = 0;
    
    if (genMergeStdev==1)
    {
        
        float *cellMean_final = 0;
        float *cellStd_final= 0;
        
        float *cellMean_vol_final = 0;
        float *cellStd_vol_final = 0;
        
        int *observed_final = 0;
        bool *markerTag_final = 0;
        int *expected_final = 0;
        
        int markercnt = marker_gene_info.markernum() + 1; // other markers plus eve
        
        // read in the file final_cellnamelist
                
        vector <QStringList> final_cellnamelist;
        int final_cellnum = 0;
//        string dfile_merge_cells = dfile_outputdir + "cellname_type_mapping.txt"; // name of the file containing the final cell names and their old correspondence

        string dfile_merge_cells = dfile_inputdir + "cellname_mapping_8TF_oct10.txt";
//        string dfile_merge_cells = dfile_inputdir + "cellname_mapping_75gal4_oct10.txt";
//        string dfile_merge_cells = dfile_inputdir + "cellname_mapping_8TF_75gal4_oct10.txt";
        
        
         readLines(dfile_merge_cells, final_cellnamelist, final_cellnum);
        
        //#ifdef DEBUG	
        //output cellnamelist to check if it is correct
        for (i=0; i<final_cellnum; i++)
        {
            for (j=0; j<final_cellnamelist[i].size(); j++)
                printf("%s, ", qPrintable(final_cellnamelist[i].at(j)));
            printf("\n");
			
        }

        
		mergeCell(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
				  cellMean_final, cellStd_final, observed_final, expected_final, markerTag_final);
        
        
		if (observed_final) {delete []observed_final; observed_final=0;}
		if (expected_final) {delete []expected_final; expected_final=0;}
		if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
		
        
		// merge cell volumes
		mergeCell(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
                  cellMean_vol_final, cellStd_vol_final, observed_final, expected_final, markerTag_final);
   
        // ------------------------------------------------------------------------
        // save the final atlas
        // ------------------------------------------------------------------------	
//        final_tag = 1;
        
        vector <string> cellnamelist_f; 
        
        for (i=0; i<final_cellnum; i++)
        {
            cellnamelist_f.push_back(final_cellnamelist[i].at(0).toStdString());
            printf("%d, %s\n", i, cellnamelist_f.at(i).c_str());
        }
        
        int dim_marker_gene_final[3];
        
        dim_marker_gene_final[0] = 3;
        dim_marker_gene_final[1] = final_cellnum;
        dim_marker_gene_final[2] = stacknum;
        

        vector <string> forceAddCellnamelist_final;
        
        saveAtlasBeforeMerge(marker_gene_info, cellnamelist_f, ref_gene_cellnamelist, markerTag_final, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, 
                  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean_final, cellStd_final, cellMean_vol_final, cellStd_vol_final, observed_final, expected_final,
                  dim_marker_gene_final, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio);	
        
//        saveAtlas(marker_gene_info, cellnamelist_f, ref_gene_cellnamelist, markerTag_final, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, 
//                  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean_final, cellStd_final, cellMean_vol_final, cellStd_vol_final, observed_final, expected_final,
//                  dim_marker_gene_final, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
//        
//        saveAtlasBeforeMerge(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
//                             markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
//                             dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio);	
        
        
		if (observed_final) {delete []observed_final; observed_final=0;}
		if (expected_final) {delete []expected_final; expected_final=0;}
		if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
    
        if (cellMean_final) {delete []cellMean_final; cellMean_final=0;}
        if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
        if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
        if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
        
    }
    
    
    
    
    
	
	// ------------------
	// delete pointers
	// ------------------	
	
	
	if (markerTag) {delete []markerTag; markerTag=0;}
    //	if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
	
	
	if (refGeneCellArrayPos) {delete []refGeneCellArrayPos; refGeneCellArrayPos=0;}
	if (refGeneCellArrayOtherFields) {delete []refGeneCellArrayOtherFields; refGeneCellArrayOtherFields = 0;}
	if (refGeneCptCellArrayPos) {delete []refGeneCptCellArrayPos; refGeneCptCellArrayPos = 0;}
	if (refGeneCellArray) {delete []refGeneCellArray; refGeneCellArray = 0;}
	if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew=0;}
	
	if (markerGeneCellArray) {delete []markerGeneCellArray; markerGeneCellArray = 0;}
	if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
	if (markerGeneCellArrayPos) {delete []markerGeneCellArrayPos; markerGeneCellArrayPos=0;}		
	if (markerGeneCellArrayOtherFields) {delete []markerGeneCellArrayOtherFields; markerGeneCellArrayOtherFields=0;}
	
	if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}
	
	if (marker_ref_map) {delete []marker_ref_map; marker_ref_map=0;}
	
	if (cellMean) {delete []cellMean; cellMean = 0;}
	if (cellStd) {delete []cellStd; cellStd = 0;}	
	if (cellMean_vol) {delete []cellMean_vol; cellMean_vol = 0;}
	if (cellStd_vol) {delete []cellStd_vol; cellStd_vol = 0;}
	if (observed) {delete []observed; observed = 0;}
	if (expected) {delete []expected; expected = 0;}
	
    //	if (cellMean_final) {delete []cellMean_final; cellMean_final = 0;}
    //	if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
    //	if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
    //	if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
    //	if (observed_final) {delete []observed_final; observed_final = 0;}
    //	if (expected_final) {delete []observed_final; observed_final = 0;}
	
}



//// 20111206
////build the atlas, target known for registration
//void buildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, 
//                vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
//                vector <string> forceAddCellnamelist,  vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 
//                int *ref_gene_idx, string dfile_outputdir, float threratio)
//{
//	
//	int i,j,k,m;
//	FILE *file; 
//	string outfilename;
//	int fieldnum = FIELDNUM + 3;
//	
//	// -----------------------------------------------------------
//	// generate matrices for registration and statistical analysis
//	// -----------------------------------------------------------	
//	bool *markerTag;	
//	float *markerGeneCellArrayPos = 0; //marker gene arrary containing only (x,y,z), for registration	
//	float *markerGeneCellArrayOtherFields = 0;	//marker gene arrary containing intesity and volume information	
//	float *refGeneCellArrayPos = 0; //reference gene (e.g., EVE) arrary containing only (x,y,z) 
//	float *refGeneCptCellArrayPos = 0; // reference gene array containing only control points for registration, note refGeneCptCellArrayPos cab be a subset of refGeneCptCellArrayPos
//	float *refGeneCellArrayOtherFields = 0; //the reference gene (e.g., EVE) arrary containing intensity and volume information
//	int *marker_ref_map =0; // a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
//	int stacknum, refstacknum;
//	vector <string> reference_stack_list;
//	
//	// for debug purpose
//#ifdef DEBUG
//	
//	MarkerStacks markerstack;
//	int cnt = 0;
//	for (i=0; i<marker_gene_info.markernum(); i++)
//	{
//		markerstack = marker_gene_info.marker_stacks.at(i);
//		int markerstacknum = (markerstack.reference_ch_filename).size();
//		
//		for (j=0; j<markerstacknum; j++)
//		{
//			printf("i=%d, j=%d, cnt=%d, stack= %s, ref= %s\n", i, j, cnt, markerstack.signal_ch_filename.at(j).c_str(), markerstack.reference_ch_filename.at(j).c_str());
//			cnt++;
//		}
//	}
//#endif
//	
//	//	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, 
//	//				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  
//	//				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
//	
//	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist,
//				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  refGeneCptCellArrayPos, 
//				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
//	
//	
//	// --------------
//	// registration
//	// --------------	
//	
//	// read cell position in target file
//	float *targetCellArrayPos = 0; // all annotated cell array					
//	float *targetCptCellArrayPos =0; // control point (annotated) array
//	float *targetCellArrayOtherFields = 0; 
//	
//	printf("target file = %s\n", dfile_target.c_str());
//	
//	
//	//	getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
//	getAtlasCell(dfile_target, cellnamelist, ref_gene_controlpoints_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
//	
//	// targetCellArrayPos and targetCellArrayOtherFields are no use
//	if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
//	if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}
//	
//	// do registration
//	int dim_marker_gene[3], dim_ref_gene[3], dim_ref_gene_cpt[3];
//	
//	dim_marker_gene[0] = 3;
//	dim_marker_gene[1] = cellnamelist.size();
//	dim_marker_gene[2] = stacknum;
//	
//	dim_ref_gene[0] = 3;
//	dim_ref_gene[1] = ref_gene_cellnamelist.size();
//	dim_ref_gene[2] = refstacknum;
//	
//	dim_ref_gene_cpt[0] = 3;
//	dim_ref_gene_cpt[1] = ref_gene_controlpoints_cellnamelist.size();
//	dim_ref_gene_cpt[2] = refstacknum;
//	
//	float *refGeneCellArrayPosNew=0, *markerGeneCellArrayPosNew=0;
//	
//	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
//				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
//	
//	
//	// ------------------------------------------------
//	// save data before registration 
//	// ------------------------------------------------
//	FILE *regfile1, *regfile2;
//	string registeredfilename1 = dfile_outputdir + "before_registration_marker.txt";
//	regfile1 = fopen(registeredfilename1.c_str(), "wt");
//	
//	string registeredfilename2 = dfile_outputdir + "before_registration_ref.txt";
//	regfile2 = fopen(registeredfilename2.c_str(), "wt");
//	
//	// write regfile1
//	for (i=0; i<marker_gene_info.markernum(); i++)
//	{
//		
//		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
//		int markerstacknum = (markerstack.signal_ch_filename).size();
//		
//		for (j=0; j<markerstacknum; j++)
//		{
//			
//			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
//			
//			for (m=0; m<dim_marker_gene[1]; m++)
//			{
//				int mm = m*3;
//				int qq = 3*dim_marker_gene[1]*j + mm;
//				if ((markerGeneCellArrayPos[qq]==0)&(markerGeneCellArrayPos[qq+1]==0)&(markerGeneCellArrayPos[qq+2]==0))
//					continue;
//				else
//					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
//			}
//			
//			
//		} // for j end
//	}
//	fclose(regfile1);
//	
//	// write regfile2
//	for (j=0; j<refstacknum; j++)
//	{
//		
//		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
//		
//		for (m=0; m<dim_ref_gene[1]; m++)
//		{
//			int mm = m*3;
//			int qq = 3*dim_ref_gene[1]*j + mm;
//			if ((refGeneCellArrayPos[qq]==0)&(refGeneCellArrayPos[qq+1]==0)&(refGeneCellArrayPos[qq+2]==0))
//				continue;
//			else
//				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPos[qq], refGeneCellArrayPos[qq+1], refGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
//			
//		}
//	}				
//	fclose(regfile2);
//	
//	// ------------------------------------------------
//	// save data after registration 
//	// ------------------------------------------------
//	
//	
//	registeredfilename1 = dfile_outputdir + "after_registration_marker.txt";
//	regfile1 = fopen(registeredfilename1.c_str(), "wt");
//	
//	registeredfilename2 = dfile_outputdir + "after_registration_ref.txt";
//	regfile2 = fopen(registeredfilename2.c_str(), "wt");
//	
//	
//	// write regfile1
//	for (i=0; i<marker_gene_info.markernum(); i++)
//	{
//		
//		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
//		int markerstacknum = (markerstack.signal_ch_filename).size();
//		
//		for (j=0; j<markerstacknum; j++)
//		{
//			
//			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
//			
//			for (m=0; m<dim_marker_gene[1]; m++)
//			{
//				int mm = m*3;
//				int qq = 3*dim_marker_gene[1]*j + mm;
//				if ((markerGeneCellArrayPosNew[qq]==0)&(markerGeneCellArrayPosNew[qq+1]==0)&(markerGeneCellArrayPosNew[qq+2]==0))
//					continue;
//				else
//					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
//			}
//			
//			
//		} // for j end
//	}
//	fclose(regfile1);
//	
//	// write regfile2
//	for (j=0; j<refstacknum; j++)
//	{
//		
//		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
//		
//		for (m=0; m<dim_ref_gene[1]; m++)
//		{
//			int mm = m*3;
//			int qq = 3*dim_ref_gene[1]*j + mm;
//			if ((refGeneCellArrayPosNew[qq]==0)&(refGeneCellArrayPosNew[qq+1]==0)&(refGeneCellArrayPosNew[qq+2]==0))
//				continue;
//			else
//				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
//			
//		}
//	}				
//	fclose(regfile2);
//	
//	
//	// ------------------------------------------------
//	// analyze cell statistics
//	// note that reference channel (e.g., EVE) cell statistics 
//	// need to be combined with the same cells in marker channel 
//	// ------------------------------------------------
//	float *cellMean = 0;
//	float *cellStd = 0;
//	int *observed = 0;
//	int *expected = 0;
//	
//	//position mean and standard deviation
//	//	computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
//	computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
//	
//	//as observed and expected will be computed when analyzing volume statistcs as below, they are deleted here to avoid memory leak
//	if (observed) {delete []observed; observed=0;}
//	//	if (expected) {delete []expected; expected=0;}
//	
//	int lenn = dim_marker_gene[1]*dim_marker_gene[0];
//	float aveCellStd=0;
//	
//	for (j=0; j<lenn; j++) // compute the sum of std along each dimension and for each cell
//	{
//		aveCellStd += cellStd[j];
//	}
//	
//	aveCellStd /= lenn;
//	
//	printf("aveCellStd = %f\n", aveCellStd);
//	
//	
//	//size mean and standard deviation
//	float *cellMean_vol;
//	float *cellStd_vol;
//	
//	int dim_marker_gene_vol[3], dim_ref_gene_vol[3];
//	
//	dim_marker_gene_vol[0] = 1;
//	dim_marker_gene_vol[1] = cellnamelist.size();
//	dim_marker_gene_vol[2] = stacknum;
//	
//	dim_ref_gene_vol[0] = 1;
//	dim_ref_gene_vol[1] = ref_gene_cellnamelist.size();
//	dim_ref_gene_vol[2] = refstacknum;
//	
//	
//	float *markerGeneCellArray = new float [dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*dim_marker_gene_vol[2]];
//	
//	for (j=0; j<dim_marker_gene_vol[2]; j++)
//	{
//		int mm1 = dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*j;
//		int mm2 = FIELDNUM*dim_marker_gene[1]*j;
//		
//		for (i=0; i<dim_marker_gene_vol[1]; i++)
//		{
//			int nn1 = mm1 + i*dim_marker_gene_vol[0];
//			int nn2 = mm2 + i*FIELDNUM;
//			markerGeneCellArray[nn1] = markerGeneCellArrayOtherFields[nn2+3];
//		}
//	}
//	
//	float *refGeneCellArray = new float [dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*dim_ref_gene_vol[2]];
//	
//	for (j=0; j<dim_ref_gene_vol[2]; j++)
//	{
//		int mm1 = dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*j;
//		int mm2 = FIELDNUM*dim_ref_gene[1]*j;
//		
//		for (i=0; i<dim_ref_gene_vol[1]; i++)
//		{
//			int nn1 = mm1 + i*dim_ref_gene_vol[0];
//			int nn2 = mm2 + i*FIELDNUM;
//			refGeneCellArray[nn1] = refGeneCellArrayOtherFields[nn2+3];
//		}
//	}
//	
//	//	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed, expected);
//	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed);
//	
//	// -----------------------------------------------------------------------------------------------
//	// compute for each cell its expected value (i.e., in how many stacks it should express
//	// -----------------------------------------------------------------------------------------------
//	computeExpectedValue(marker_gene_info, dfile_outputdir, refstacknum, expected);
//	
//	// ------------------
//	// save atlas files
//	// ------------------	
//    //	bool final_tag = 0;
//    
//	
//    // 20111206 comment, need it    
//	saveAtlasBeforeMerge(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
//                         markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
//                         dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio);	
//	
//	// ------------------
//	// delete pointers
//	// ------------------	
//	
//	
//	if (markerTag) {delete []markerTag; markerTag=0;}
//    //	if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
//	
//	
//	if (refGeneCellArrayPos) {delete []refGeneCellArrayPos; refGeneCellArrayPos=0;}
//	if (refGeneCellArrayOtherFields) {delete []refGeneCellArrayOtherFields; refGeneCellArrayOtherFields = 0;}
//	if (refGeneCptCellArrayPos) {delete []refGeneCptCellArrayPos; refGeneCptCellArrayPos = 0;}
//	if (refGeneCellArray) {delete []refGeneCellArray; refGeneCellArray = 0;}
//	if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew=0;}
//	
//	if (markerGeneCellArray) {delete []markerGeneCellArray; markerGeneCellArray = 0;}
//	if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
//	if (markerGeneCellArrayPos) {delete []markerGeneCellArrayPos; markerGeneCellArrayPos=0;}		
//	if (markerGeneCellArrayOtherFields) {delete []markerGeneCellArrayOtherFields; markerGeneCellArrayOtherFields=0;}
//	
//	if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}
//	
//	if (marker_ref_map) {delete []marker_ref_map; marker_ref_map=0;}
//	
//	if (cellMean) {delete []cellMean; cellMean = 0;}
//	if (cellStd) {delete []cellStd; cellStd = 0;}	
//	if (cellMean_vol) {delete []cellMean_vol; cellMean_vol = 0;}
//	if (cellStd_vol) {delete []cellStd_vol; cellStd_vol = 0;}
//	if (observed) {delete []observed; observed = 0;}
//	if (expected) {delete []expected; expected = 0;}
//	
//    //	if (cellMean_final) {delete []cellMean_final; cellMean_final = 0;}
//    //	if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
//    //	if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
//    //	if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
//    //	if (observed_final) {delete []observed_final; observed_final = 0;}
//    //	if (expected_final) {delete []observed_final; observed_final = 0;}
//	
//}


// generate matrices needed for registration and statistical analysis
//
// input parameters:
// 1) marker_gene_info: of class MarkerGeneInfo, carries the inforamtion of markers and stacks in the atals
// 2) cellnamelist: list of all cell names in the current atlas 
// 3) ref_gene_cellnamelist: list of all reference cell names used for registration
//
// output parameters:
// 1) markerGeneCellArrayPos: the marker gene arrary containing only (x,y,z), 
// the size is: total number of cells in the atlas * 3 * number of stacks (note one marker 
// constains multiple stacks). This array is used for registration and statistics analysis
// 2) markerGeneCellArrayOtherFields: the marker gene arrary containing intesity and volume information, 
// the size is total number of cells in the atlas * 5 * number of stacks (note one marker constains multiple stacks)
// This arrary is not used for registration, but for later statistic analaysis
// 3) refGeneCellArrayPos is the reference gene (e.g., EVE) arrary containing only (x,y,z), 
// the size is: total number of reference gene cells in the atlas * 3 * number of stacks (note one marker 
// constains multiple stacks).
// 4) refGeneCellArrayyOtherFields is the reference gene (e.g., EVE) arrary containing intensity and volume information, 
// the size is: total number of reference gene cells in the atlas * 3 * number of stacks (note one marker 
// constains multiple stacks). This array is not used for registration but for later statistics analysis	
// 5) marker_ref_map: a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
// 6) refGeneCellCptArrayPos is added on 20100903, to allow subset of reference gene be used for registration

void genMatrices(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
                 float *&markerGeneCellArrayPos, float *&markerGeneCellArrayOtherFields, 
                 float *&refGeneCellArrayPos, float *&refGeneCellArrayOtherFields,  float *&refGeneCptCellArrayPos,
                 int *&marker_ref_map, bool *&markerTag, int &stacknum, int &refstacknum, vector <string> &reference_stack_list)
{
    
	int i,j,k,m;
	
	int cellnum = cellnamelist.size();
	int refcellnum = ref_gene_cellnamelist.size();
	int refcptcellnum = ref_gene_controlpoints_cellnamelist.size();
	
	MarkerStacks markerstack;
	
    
	// compute the total number of stacks used to build the atlas
	stacknum =0;
    
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
        
		markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		stacknum += markerstacknum;
        //		printf("%s, %d\n", markerstack.markername.c_str(), markerstacknum);
	}
    
	// compute marker_ref_map
	// Since 2 (or multiple) marker gene stacks share the same reference stack	
	// generate a list of reference stacks, with each stack appear only once
    
	marker_ref_map = new int [stacknum];
	
	
	int cnt = -1;
	
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
        
		markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.reference_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			cnt++;
			bool existflg = 0;
			
			for (k=0; k<reference_stack_list.size(); k++)
			{
				if (markerstack.reference_ch_filename.at(j).compare(reference_stack_list.at(k)) ==0)
				{
					marker_ref_map[cnt] = k;
					existflg = 1;
					break;
				}
			}
			
			if (existflg ==0) // expand reference_stack_list
			{
				reference_stack_list.push_back(markerstack.reference_ch_filename.at(j));
				marker_ref_map[cnt] = reference_stack_list.size()-1;
			}
			
            //			printf("i = %d, j = %d, cnt = %d, marker_ref_map[cnt] = %d\n", i, j, cnt, marker_ref_map[cnt]);
            
            //			printf("i = %d, j = %d\n, signal stack = %s\n, reference stack = %s\n", i, j, 
            //				   marker_gene_info.marker_stacks.at(i).signal_ch_filename.at(j).c_str(), 
            //				   reference_stack_list.at(marker_ref_map[cnt]).c_str());
			
		} // for j end
	}
    
	// compute the total number of stacks in the reference
	refstacknum = reference_stack_list.size();
	
	//allocate memoery for matrices	
	int len1 = cellnum*3*stacknum;
	int len2 = cellnum*FIELDNUM*stacknum;
	int len3 = refcellnum*3*refstacknum;
	int len4 = refcellnum*FIELDNUM*refstacknum;
	int len5 = cellnum*(marker_gene_info.markernum()+1);
	int len6 = refcptcellnum*3*refstacknum; //20100903
	
	markerGeneCellArrayPos = new float [len1];
	markerGeneCellArrayOtherFields = new float [len2];
	refGeneCellArrayPos = new float [len3];
	refGeneCellArrayOtherFields = new float [len4];
	markerTag = new bool [len5];
	refGeneCptCellArrayPos = new float [len6]; //20100903
	
	
	// initialize 
	for (i=0; i<len1; i++)
		markerGeneCellArrayPos[i] = 0;
    
	for (i=0; i<len2; i++)
		markerGeneCellArrayOtherFields[i] = 0;
    
	for (i=0; i<len3; i++)
		refGeneCellArrayPos[i] = 0;
    
	for (i=0; i<len4; i++)
		refGeneCellArrayOtherFields[i] = 0;
    
	for (i=0; i<len5; i++)
		markerTag[i] = 0;
	
	for (i=0; i<len6; i++)
		refGeneCptCellArrayPos[i] = 0;
	
    
    //	getCells(marker_gene_info, cellnamelist, markerGeneCellArrayPos, markerGeneCellArrayOtherFields);
    //	getCells(marker_gene_info, ref_gene_cellnamelist, refGeneCellArrayPos, refGeneCellArrayOtherFields);
    
    //	getCells(marker_gene_info, cellnamelist, ref_gene_cellnamelist, marker_ref_map,
    //			 markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields, markerTag);
    // no need to put a function here, can directly insert code
    
	// assign values to markerGeneCellArrayPos, markerGeneCellArrayOtherFields, markerTag
	
	string apoline;
	char tmps[1024];
	int stackcnt = -1;
	
	for (k=0; k<marker_gene_info.markernum(); k++)
	{
		printf("k = %d\n", k);
		
		markerstack = marker_gene_info.marker_stacks.at(k);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
            
			stackcnt++;
            
#ifdef DEBUG
			printf("stackcnt = %d\n", stackcnt);
#endif
            //			if (stackcnt == 115)
            //				printf("stackcnt = 115, %s\n", markerstack.signal_ch_filename.at(j).c_str());
			ifstream ifs ((markerstack.signal_ch_filename.at(j)).c_str(), ifstream::in);
			
//            			printf("%d\n", j);
//            			printf("signal filename = %s\n", markerstack.signal_ch_filename.at(j).c_str());
//            			printf("reference filename = %s\n", markerstack.reference_ch_filename.at(j).c_str());
			
			while (!ifs.eof())
			{
				ifs.getline(tmps, 1024);
				apoline = tmps;
				int s_start = 0;
				int s_length = 0;
				CellAPO_FL curcell;
				int item = 0;
                
				// parse the information in each line and save it in curcell
                
				for (i=0; i<apoline.size(); i++)
				{
					if ((apoline.at(i)==',')||(i==(apoline.size()-1)))
					{
						// process the current item
						if (i==(apoline.size()-1))
							s_length++;
                        
                        //						printf("k=%d, j=%d, i=%d, s_start=%d, s_length=%d\n", k,j,i,s_start, s_length);
						string itemstr = apoline.substr(s_start, s_length);
                        //						printf("item = %d, s_start = %d, s_length = %d, itemstr = %s\n", item, s_start, s_length, itemstr.c_str());
						
						if (item==0) curcell.n = atoi(itemstr.c_str());
						else if (item==1) curcell.orderinfo = itemstr;
                        //						else if (item==2) curcell.name = itemstr;
						else if (item==2) curcell.name = qPrintable(QString(itemstr.c_str()).trimmed());						
						else if (item==3) curcell.comment = itemstr;
						else if (item==4) curcell.z = atof(itemstr.c_str()); 
						else if (item==5) curcell.x = atof(itemstr.c_str());
						else if (item==6) curcell.y = atof(itemstr.c_str());
						else if (item==7) curcell.pixmax = atof(itemstr.c_str());
						else if (item==8) curcell.intensity = atof(itemstr.c_str());
						else if (item==9) curcell.sdev = atof(itemstr.c_str());
						else if (item==10) curcell.volsize = atof(itemstr.c_str());
						else if (item==11) curcell.mass =atof(itemstr.c_str());
						else if (item==15) curcell.color.r = (unsigned char) atoi(itemstr.c_str());
						else if (item==16) curcell.color.g = (unsigned char) atoi(itemstr.c_str());
						else if (item==17) curcell.color.b = (unsigned char) atoi(itemstr.c_str());
						
						// move pointer
						item++;
						s_start = s_start+s_length+1; 
						s_length = 0;
					}
					else
						s_length++;
				} // for i end
                
                
				// test if the current cell belongs to those annotated
				for (i=0; i<cellnum; i++)
				{
                    //			printf("%s\n",cellnamelist.at(i).c_str());
                    
					
                    //					printf("curcell.name = %s, cellnamelist = %s\n", curcell.name.c_str(), cellnamelist.at(i).c_str());
					
                    //					string str1 = trimStringSpace(curcell.name);
                    //					printf("curcell.name trimed = %s\n",str1.c_str());
                    //										
                    //					string str2 = trimStringSpace(cellnamelist.at(i));
                    //					printf("cellnamelsit trimed = %s\n",str2.c_str());
                    
					string str1, str2;
					trimStringSpace(curcell.name, str1);
                    //					printf("curcell.name trimed = [%s]\n",str1.c_str());
					
					trimStringSpace(cellnamelist.at(i), str2);
                    //					printf("cellnamelsit trimed = [%s]\n",str2.c_str());
					
                    //					printf("curcell.name trimed = %s, cellnamelist trimmed = %s\n",str1.c_str(), str2.c_str());
                    
					if (str1.compare(str2)==0)
						break;
				}
                
                
				if (i<cellnum) // the current cell is an annotated cell
				{
                    
					int p = stackcnt*3*cellnum + 3*i;
					markerGeneCellArrayPos[p] = curcell.x; 
					markerGeneCellArrayPos[p+1] = curcell.y; 
					markerGeneCellArrayPos[p+2] = curcell.z;
                    
                    //					if (i==1503)
                    //					if (i==18)					
                    //						printf("i = %d, %s, stacknum = %d, %f, %f, %f\n ", i, cellnamelist.at(i).c_str(), j, markerGeneCellArrayPos[p], markerGeneCellArrayPos[p+1], markerGeneCellArrayPos[p+2]);
					
                    
					p = stackcnt*FIELDNUM*cellnum + FIELDNUM*i; 
					markerGeneCellArrayOtherFields[p] = curcell.pixmax;
					markerGeneCellArrayOtherFields[p+1] = curcell.intensity;
					markerGeneCellArrayOtherFields[p+2] = curcell.sdev;
					markerGeneCellArrayOtherFields[p+3] = curcell.volsize;
					markerGeneCellArrayOtherFields[p+4] = curcell.mass;
					
					markerTag[i*(marker_gene_info.markernum()+1) + k+1] = 1; // k+1 because the first element is saved for reference (EVE)
                    
                    //					if (i==18)					
                    //						printf("%f, %f, %f, %f, %f\n", markerGeneCellArrayOtherFields[p], markerGeneCellArrayOtherFields[p+1],
                    //							markerGeneCellArrayOtherFields[p+2], markerGeneCellArrayOtherFields[p+3], markerGeneCellArrayOtherFields[p+4]);
					
				}
                
			} //while end
			ifs.close();
		} // for j end
	} // for k end
	
	// assign values to refGeneCellArrayPos, refGeneCellArrayOtherFields, markerTag
	stackcnt = -1;
	
	for (j=0; j<refstacknum; j++)
	{
        
		printf("j = %d, %s\n", j, reference_stack_list.at(j).c_str());
		
		stackcnt++;
		ifstream ifs (reference_stack_list.at(j).c_str(), ifstream::in);
		
		while (!ifs.eof())
		{
			ifs.getline(tmps, 1024);
			apoline = tmps;
			int s_start = 0;
			int s_length = 0;
			CellAPO_FL curcell;
			int item = 0;
            
			// parse the information in each line and save it in curcell
            
			for (i=0; i<apoline.size(); i++)
			{
				if ((apoline.at(i)==',')||(i==(apoline.size()-1)))
				{
					// process the current item
					if (i==(apoline.size()-1))
						s_length++;
                    
					string itemstr = apoline.substr(s_start, s_length);
                    //				printf("item = %d, s_start = %d, s_length = %d, itemstr = %s\n", item, s_start, s_length, itemstr.c_str());
					
					if (item==0) curcell.n = atoi(itemstr.c_str());
					else if (item==1) curcell.orderinfo = itemstr;
                    //					else if (item==2) curcell.name = itemstr;
					else if (item==2) curcell.name = qPrintable(QString(itemstr.c_str()).trimmed());
					else if (item==3) curcell.comment = itemstr;
					else if (item==4) curcell.z = atof(itemstr.c_str()); 
					else if (item==5) curcell.x = atof(itemstr.c_str());
					else if (item==6) curcell.y = atof(itemstr.c_str());
					else if (item==7) curcell.pixmax = atof(itemstr.c_str());
					else if (item==8) curcell.intensity = atof(itemstr.c_str());
					else if (item==9) curcell.sdev = atof(itemstr.c_str());
					else if (item==10) curcell.volsize = atof(itemstr.c_str());
					else if (item==11) curcell.mass =atof(itemstr.c_str());
					else if (item==15) curcell.color.r = (unsigned char) atoi(itemstr.c_str());
					else if (item==16) curcell.color.g = (unsigned char) atoi(itemstr.c_str());
					else if (item==17) curcell.color.b = (unsigned char) atoi(itemstr.c_str());
					
					// move pointer
					item++;
					s_start = s_start+s_length+1; 
					s_length = 0;
				}
				else
					s_length++;
			} // for i end
            
            //			printf("%s\n", reference_stack_list.at(j).c_str());
            //			printf("i = %d, j = %d, %s\n", i, j, curcell.name.c_str());
            
			// test if the current cell belongs to those annotated reference cells
			for (i=0; i<refcellnum; i++)
			{
                //			printf("%s\n",cellnamelist.at(i).c_str());
                //				printf("%s, %s\n", curcell.name.c_str(), ref_gene_cellnamelist.at(i).c_str()); 
                
				string str1, str2;
				
				trimStringSpace(curcell.name, str1);
				trimStringSpace(ref_gene_cellnamelist.at(i), str2);
				
				if (str1.compare(str2)==0)
					break;
			}
            
            
			if (i<refcellnum) // the current cell is an annotated cell
			{
                
				int p = stackcnt*3*refcellnum + 3*i;
				refGeneCellArrayPos[p] = curcell.x; 
				refGeneCellArrayPos[p+1] = curcell.y; 
				refGeneCellArrayPos[p+2] = curcell.z;
                
                //				printf("i = %d, %f, %f, %f, ", i, refGeneCellArrayPos[p], refGeneCellArrayPos[p+1], refGeneCellArrayPos[p+2]);
                
                
				p = stackcnt*FIELDNUM*refcellnum + FIELDNUM*i; 
				refGeneCellArrayOtherFields[p] = curcell.pixmax;
				refGeneCellArrayOtherFields[p+1] = curcell.intensity;
				refGeneCellArrayOtherFields[p+2] = curcell.sdev;
				refGeneCellArrayOtherFields[p+3] = curcell.volsize;
				refGeneCellArrayOtherFields[p+4] = curcell.mass;
                
                //				printf("%f, %f, %f, %f, %f\n", refGeneCellArrayOtherFields[p], refGeneCellArrayOtherFields[p+1],
                //					refGeneCellArrayOtherFields[p+2], refGeneCellArrayOtherFields[p+3], refGeneCellArrayOtherFields[p+4]);
				
				//assign values in markerTag
				for (m=0; m<cellnum; m++)
				{
					string str1, str2;
					
					trimStringSpace(curcell.name, str1); 
					trimStringSpace(cellnamelist.at(m), str2);
					
					if (str1.compare(str2)==0)
						break;
				}
				markerTag[m*(marker_gene_info.markernum()+1)] = 1;
                
			}
			
			// test if the current cell belongs to those control point reference cells used for registration
			for (i=0; i<refcptcellnum; i++)
			{
				//			printf("%s\n",cellnamelist.at(i).c_str());
                //				printf("%s, %s\n", curcell.name.c_str(), ref_gene_cellnamelist.at(i).c_str()); 
                //				
                //				printf("%s, %s, %s, %s\n", curcell.name.c_str(), trimStringSpace(curcell.name).c_str(), ref_gene_cellnamelist.at(i).c_str(), trimStringSpace(ref_gene_controlpoints_cellnamelist.at(i)).c_str()); 
                
				string str1, str2;
				trimStringSpace(curcell.name, str1);
				trimStringSpace(ref_gene_controlpoints_cellnamelist.at(i), str2);
				
				if (str1.compare(str2)==0)
					break;
			}
			
			
			if (i<refcptcellnum) // the current cell is a control point cell used for registration
			{
				
				int p = stackcnt*3*refcptcellnum + 3*i;
				refGeneCptCellArrayPos[p] = curcell.x; 
				refGeneCptCellArrayPos[p+1] = curcell.y; 
				refGeneCptCellArrayPos[p+2] = curcell.z;
				
				//				printf("i = %d, %f, %f, %f, ", i, refGeneCellArrayPos[p], refGeneCellArrayPos[p+1], refGeneCellArrayPos[p+2]);
			}
			
			
			
		} //while end
		ifs.close();
	} // for j end
    
    //	//check markerTag values
    //	for (i=0; i<cellnum; i++)
    //	{
    //		int tmp = (marker_gene_info.markernum()+1);
    //		printf("i=%d: ", i);
    //		
    //		for (j=0; j<marker_gene_info.markernum()+1; j++)
    //			printf("%d ", markerTag[i*tmp+j]);
    //		printf("\n");
    //	}
	
	return;
}


// get atlas cells from an APO file, select those that have been annotated, and save them in float arrays
// input: filename, cellnamelist, ref_gene_cellnamelist
// output: cellArray and cptCellArray. cellArray is the array of all annotated cells, it has cellnamelist.size()*3 elemenets;
//		   cptCellArray is the array of annoated marker cells, it has ref_gene_cellnamelist.size()*3 elements

void getAtlasCell(string filename, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, float *&cellArray, float *&cptCellArray, float *&cellArrayOtherFields)
{		
	int i,j;
	
	
	string apoline;
	ifstream ifs (filename.c_str(), ifstream::in);
	char tmps[1024];
    
#ifdef DEBUG
	printf("tagetfile name in getAtlasCell %s\n", filename.c_str());
#endif
	
    //	cellArray = new CellAPO_FL [cellnamelist.size()];
    //	cptCellArray = new CellAPO_FL [ref_gene_cellnamelist.size()];
    
	int cellnum = cellnamelist.size();
	int refcellnum = ref_gene_cellnamelist.size();
	
    
	int len1 = cellnum*3;
	int len2 = refcellnum*3;
	int len3 = cellnum*FIELDNUM;
	
	cellArray = new float [len1]; // used for registration
	cptCellArray = new float [len2]; // used for registration
	cellArrayOtherFields = new float [len3];// together with registered cell positions, used for atlas statistic analysis, other fields of the cell, including pixmax, intensity, sdev, volsize, mass
    
	// initialize cellArray and cptCellArray
	
	for (i=0; i<len1; i++)
		cellArray[i] = 0;
    
	for (i=0; i<len2; i++)
		cptCellArray[i] = 0;
    
	for (i=0; i<len3; i++)
		cellArrayOtherFields[i] = 0;
    
	int linenum = -1;
	while (!ifs.eof())
	{
		ifs.getline(tmps, 1024);
		apoline = tmps;
		int s_start = 0;
		int s_length = 0;
		CellAPO_FL curcell;
		int item = 0;
        
		
		linenum++;
        
#ifdef DEBUG
		printf("linenum = %d, linesize = %d \n", linenum, apoline.size());
		printf("%s\n", apoline.c_str());
#endif		
		
		// parse the information in each line and save it in curcell
        
		
		for (i=0; i<apoline.size(); i++)
		{
			if ((apoline.at(i)==',')||(i==(apoline.size()-1)))
			{
				// process the current item
				if (i==(apoline.size()-1))
					s_length++;
                
				string itemstr = apoline.substr(s_start, s_length);
                //				printf("item = %d, s_start = %d, s_length = %d, itemstr = %s\n", item, s_start, s_length, itemstr.c_str());
				
				if (item==0) curcell.n = atoi(itemstr.c_str());
				else if (item==1) curcell.orderinfo = itemstr;
                //				else if (item==2) curcell.name = itemstr;
				else if (item==2) curcell.name = qPrintable(QString(itemstr.c_str()).trimmed());
				else if (item==3) curcell.comment = itemstr;
				else if (item==4) curcell.z = atof(itemstr.c_str()); 
				else if (item==5) curcell.x = atof(itemstr.c_str());
				else if (item==6) curcell.y = atof(itemstr.c_str());
				else if (item==7) curcell.pixmax = atof(itemstr.c_str());
				else if (item==8) curcell.intensity = atof(itemstr.c_str());
				else if (item==9) curcell.sdev = atof(itemstr.c_str());
				else if (item==10) curcell.volsize = atof(itemstr.c_str());
				else if (item==11) curcell.mass =atof(itemstr.c_str());
				else if (item==15) curcell.color.r = (unsigned char) atoi(itemstr.c_str());
				else if (item==16) curcell.color.g = (unsigned char) atoi(itemstr.c_str());
				else if (item==17) curcell.color.b = (unsigned char) atoi(itemstr.c_str());
				
				// move pointer
				item++;
				s_start = s_start+s_length+1; 
				s_length = 0;
			}
			else
				s_length++;
		} // for i end
		
        //		printf("%d, %s, %s, %s, %f, %f, %f, %f, %f, %f, %f, %d, %f, %d, %d\n", curcell.n, curcell.orderinfo, curcell.name, curcell.comment, curcell.z, curcell.x, curcell.y, curcell.pixmax, curcell.intensity,
        //			   curcell.sdev, curcell.volsize, curcell.mass, curcell.color.r, curcell.color.g, curcell.color.b);
        
        //		printf("%d\n", curcell.n);
		
		// test if the current cell belongs to those annotated
		for (i=0; i<cellnum; i++)
		{
            //			printf("i=%d, %s\n", i, cellnamelist.at(i).c_str());
			
			string str1, str2;
			trimStringSpace(curcell.name, str1);
			trimStringSpace(cellnamelist.at(i), str2);
			
			if (str1.compare(str2)==0)
				break;
		}
        
        
		if (i<cellnum) // the current cell is an annotated cell
		{
            
			int p =  3*i;
			cellArray[p] = curcell.x; 
			cellArray[p+1] = curcell.y; 
			cellArray[p+2] = curcell.z;
            
			p = FIELDNUM*i;
			cellArrayOtherFields[p] = curcell.pixmax;
			cellArrayOtherFields[p+1] = curcell.intensity;
			cellArrayOtherFields[p+2] = curcell.sdev;
			cellArrayOtherFields[p+3] = curcell.volsize;
			cellArrayOtherFields[p+4] = curcell.mass;
			
		}
		
		
		// test if the current cell belongs to those annotated marker cells
		for (i=0; i<refcellnum; i++)
		{
			string str1, str2;
			trimStringSpace(curcell.name, str1);
			trimStringSpace(ref_gene_cellnamelist.at(i), str2);
			
			if (str1.compare(str2)==0)
				break;
		}
        
        
		if (i<refcellnum) // the current cell is an annotated marker cell
		{
            
			int p =  3*i;
			
			cptCellArray[p] = curcell.x; 
			cptCellArray[p+1] = curcell.y; 
			cptCellArray[p+2] = curcell.z; 
			
            //			// check the values of cptCellArray
            //			printf("i=%d, %f, %f, %f\n", i, cptCellArray[p], cptCellArray[p+1], cptCellArray[p+2]);  
		}	
		
	} //while end
	
	ifs.close();
	return;
}


// register stacks, target file known
// parameters:
// targetCptCellArrayPos: control point position of target stack
// markerGeneCellArrayPos: marker gene cell position array, size is  3 * number of cells in the atlas * total number of stacks (for multiple markers)
// refGeneCellArrayPos: reference gene (e.g., EVE) cell position array, size is the 3 * number of EVE cells in the atlas * total number of EVE stacks (note this number is smaller than
//						the total number of stacks of marker genes, as usually two marker genes share the same reference channel
// marker_ref_map: indicate for each marker stack, what is its corresponding reference stack
// dim_marker_gene: dimension of marker gene cell position array, dim_marker_gene[0~2] are 3, # of all cells in the atlas, # of stack
// dim_marker_gene: dimension of marker gene cell position array, dim_marker_gene[0~2] are 3, # of all reference cells, # of stack
// markerGeneCellArrayPosNew: registered result of markerGeneCellArrayPos
// refGeneCellArrayPosNew: registered result of refGeneCellArrayPos
// refGeneCptCellArrayPos is added on 20100903, to allow subset of reference gene cells used for as control points for registration

void registerStacks(float *targetCptCellArrayPos, float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *refGeneCptCellArrayPos, int *marker_ref_map, int *dim_marker_gene, int *dim_ref_gene, int *dim_ref_gene_cpt, float *&markerGeneCellArrayPosNew, float *&refGeneCellArrayPosNew)
{
	
	int k, m;
    
	int ref_gene_annocellnum = dim_ref_gene[1];
	int ref_gene_stacknum = dim_ref_gene[2];
	int marker_gene_annocellnum = dim_marker_gene[1];
	int marker_gene_stacknum = dim_marker_gene[2];
	int ref_gene_annocellnum_cpt = dim_ref_gene_cpt[1];
	
    //	float *targetCptCellArrayTmp = new float [ref_gene_annocellnum*3]; // temporary files for saving valid controling points in the target stack for registration
    //	float *subjectCptCellArrayTmp = new float [ref_gene_annocellnum*3]; // temporary files for saving valid controling points in the subject stack for registration
    
	float *targetCptCellArrayTmp = new float [ref_gene_annocellnum_cpt*3]; // temporary files for saving valid controling points in the target stack for registration, 20100903
	float *subjectCptCellArrayTmp = new float [ref_gene_annocellnum_cpt*3]; // temporary files for saving valid controling points in the subject stack for registration, 20100903
	
	float *cur_refGeneArray = new float [ref_gene_annocellnum*3]; // reference cell position array for the current reference stack 
	float *cur_markerGeneArray = new float [marker_gene_annocellnum*3]; // maker cell position array for the current marker gene stack which is under registration
	
	bool *existTag1 = new bool [ref_gene_annocellnum]; //tag indicating if a particular cell exist in a reference gene stack
	bool *existTag2 = new bool [marker_gene_annocellnum]; //tag indicating if a particular cell exist in a marker gene stack
    
	int len1 = marker_gene_annocellnum*3*marker_gene_stacknum;
	int len2 = ref_gene_annocellnum*3*ref_gene_stacknum;
	
	markerGeneCellArrayPosNew = new float [len1];
	refGeneCellArrayPosNew = new float [len2];
	
	// initialize 
	for (k=0; k<len1; k++)
		markerGeneCellArrayPosNew[k] = 0;
	
	for (k=0; k<len2; k++)
		refGeneCellArrayPosNew[k] = 0;
	
	// register for each marker gene stack
	for (k=0; k<marker_gene_stacknum; k++)
	{
		
		printf("%d marker of all %d\n", k, marker_gene_stacknum);
		
		// set controlling points, remove those that are not annotated in either the subject or target files
		long existCptNum = -1;
        
		// 20100903 comment
        //		for (m=0; m<ref_gene_annocellnum; m++)
        //		{
        //			int mm = m*3;
        //			int qq = 3*ref_gene_annocellnum*marker_ref_map[k] + mm;
        //			
        ////			printf("k = %d, marker_ref_map[k] = %d, mm = %d, qq = %d \n", k, marker_ref_map[k], mm, qq);
        //				   
        //			if ((!((refGeneCellArrayPos[qq]==0)&&(refGeneCellArrayPos[qq+1]==0)&&(refGeneCellArrayPos[qq+2]==0)))&&
        //				(!((targetCptCellArrayPos[mm]==0)&&(targetCptCellArrayPos[mm+1]==0)&&(targetCptCellArrayPos[mm+2]==0))))
        //			{
        //				existCptNum++;
        //				
        //				int nn = existCptNum*3;
        //				
        //				subjectCptCellArrayTmp[nn] = refGeneCellArrayPos[qq];
        //				subjectCptCellArrayTmp[nn+1] = refGeneCellArrayPos[qq+1];
        //				subjectCptCellArrayTmp[nn+2] = refGeneCellArrayPos[qq+2];
        //				
        //				targetCptCellArrayTmp[nn] = targetCptCellArrayPos[mm];
        //				targetCptCellArrayTmp[nn+1] = targetCptCellArrayPos[mm+1];
        //				targetCptCellArrayTmp[nn+2] = targetCptCellArrayPos[mm+2];
        //				
        //			}
        //		} // for m end
		
		for (m=0; m<ref_gene_annocellnum_cpt; m++)
		{
			int mm = m*3;
			int qq = 3*ref_gene_annocellnum_cpt*marker_ref_map[k] + mm;
			
			//			printf("k = %d, marker_ref_map[k] = %d, mm = %d, qq = %d \n", k, marker_ref_map[k], mm, qq);
			
			if ((!((refGeneCptCellArrayPos[qq]==0)&&(refGeneCptCellArrayPos[qq+1]==0)&&(refGeneCptCellArrayPos[qq+2]==0)))&&
				(!((targetCptCellArrayPos[mm]==0)&&(targetCptCellArrayPos[mm+1]==0)&&(targetCptCellArrayPos[mm+2]==0))))
			{
				existCptNum++;
				
				int nn = existCptNum*3;
				
				subjectCptCellArrayTmp[nn] = refGeneCptCellArrayPos[qq];
				subjectCptCellArrayTmp[nn+1] = refGeneCptCellArrayPos[qq+1];
				subjectCptCellArrayTmp[nn+2] = refGeneCptCellArrayPos[qq+2];
				
				targetCptCellArrayTmp[nn] = targetCptCellArrayPos[mm];
				targetCptCellArrayTmp[nn+1] = targetCptCellArrayPos[mm+1];
				targetCptCellArrayTmp[nn+2] = targetCptCellArrayPos[mm+2];
				
			}
		} // for m end
		
		existCptNum++;
        
		// do registration for reference gene
		// identify not annotated cells in refGeneCellArrayPos of the current stack
		for (m=0; m<ref_gene_annocellnum; m++)
		{
			int mm = m*3;
            //			int qq = m*3*marker_ref_map[k];
			int qq = 3*ref_gene_annocellnum*marker_ref_map[k] + mm;
			
			cur_refGeneArray[mm] = refGeneCellArrayPos[qq];
			cur_refGeneArray[mm+1] = refGeneCellArrayPos[qq+1];
			cur_refGeneArray[mm+2] = refGeneCellArrayPos[qq+2];
			
			if (!((cur_refGeneArray[mm]==0)&&(cur_refGeneArray[mm+1]==0)&&(cur_refGeneArray[mm+2]==0)))
				existTag1[m] = 1;
			else
				existTag1[m] = 0;
            
#ifdef DEBUG			
			// check cur_refGeneArray for debug purpose
			printf("m=%d, %f, %f, %f\n", m, cur_refGeneArray[mm], cur_refGeneArray[mm+1], cur_refGeneArray[mm+2]);
#endif
			
		}
        
        //		printf("%d affine reference registration starts\n", k);
		// affine transformation
#ifdef DEBUG		
		printf("stacknum = %d\n", k);
#endif
		
		registerAffine(cur_refGeneArray, ref_gene_annocellnum, subjectCptCellArrayTmp, targetCptCellArrayTmp, existCptNum, 3);
        
        //		printf("%d affine reference registration ends\n", k);
		
        //		// check cur_refGeneArray for debug purpose
        //		printf("\n\n subjectCptCellArrayTmp\n");
        //		for (m=0; m<ref_gene_annocellnum; m++)
        //		{
        //			int mm = m*3;
        //			printf("m=%d, %f, %f, %f\n", m, subjectCptCellArrayTmp[mm], subjectCptCellArrayTmp[mm+1], subjectCptCellArrayTmp[mm+2]);
        //		}
        //
        //		printf("\n\n targetCptCellArrayTmp\n");		
        //		for (m=0; m<ref_gene_annocellnum; m++)
        //		{
        //			int mm = m*3;
        //			printf("m=%d, %f, %f, %f\n", m, targetCptCellArrayTmp[mm], targetCptCellArrayTmp[mm+1], targetCptCellArrayTmp[mm+2]);
        //		}		
        //
        //		printf("\n\n after registration\n");
        //		printf("\n\n cur_refGeneArray\n");
        //		
        //		for (m=0; m<ref_gene_annocellnum; m++)
        //		{
        //			int mm = m*3;
        //			printf("m=%d, %f, %f, %f\n", m, cur_refGeneArray[mm], cur_refGeneArray[mm+1], cur_refGeneArray[mm+2]);
        //		}
        
		// assign values to refGeneCellArrayPosNew using registered values
		// set those cells in refGeneCellArrayPos that are not annotated to (0,0,0)
        //		printf("\n\n refGeneCellArrayPosNew\n");
		
		for (m=0; m<ref_gene_annocellnum; m++)
		{
			int mm = m*3;
            //			int qq = m*3*marker_ref_map[k];
			int qq = 3*ref_gene_annocellnum*marker_ref_map[k] + mm;
			
			if (existTag1[m] == 0)
				refGeneCellArrayPosNew[qq] = refGeneCellArrayPosNew[qq+1] = refGeneCellArrayPosNew[qq+2] = 0; 
			else
			{
				refGeneCellArrayPosNew[qq] = cur_refGeneArray[mm];
				refGeneCellArrayPosNew[qq+1] = cur_refGeneArray[mm+1];
				refGeneCellArrayPosNew[qq+2] = cur_refGeneArray[mm+2];
			}
			
            //			printf("m=%d, %f, %f %f\n", m, refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]);
			
		}
        
		
		// do registration for marker gene,
		// identify not annotated cells in refGeneCellArrayPos of the current stack
		for (m=0; m<marker_gene_annocellnum; m++)
		{
			int mm = m*3;
            //			int qq = k;
			int qq = 3*marker_gene_annocellnum*k + mm;
			
			cur_markerGeneArray[mm] = markerGeneCellArrayPos[qq];
			cur_markerGeneArray[mm+1] = markerGeneCellArrayPos[qq+1];
			cur_markerGeneArray[mm+2] = markerGeneCellArrayPos[qq+2];
			
			if (!((cur_markerGeneArray[mm]==0)&&(cur_markerGeneArray[mm+1]==0)&&(cur_markerGeneArray[mm+2]==0)))
				existTag2[m] = 1;
			else
				existTag2[m] = 0;
			
		}
        
        //		printf("%d affine marker gene registration starts\n", k);
		
		// affine transformation
		registerAffine(cur_markerGeneArray, marker_gene_annocellnum, subjectCptCellArrayTmp, targetCptCellArrayTmp, existCptNum, 3);
        
        //		printf("%d affine marker gene registration starts\n", k);
		
		// assign values to markerGeneCellArrayPosNew using registered values
		// set those cells in markerGeneCellArrayPos that are not annotated to (0,0,0)
		for (m=0; m<marker_gene_annocellnum; m++)
		{
			int mm = m*3;
            //			int qq = k;
			int qq = 3*marker_gene_annocellnum*k + mm;
			
			if (existTag2[m] == 0)
				markerGeneCellArrayPosNew[qq] = markerGeneCellArrayPosNew[qq+1] = markerGeneCellArrayPosNew[qq+2] = 0; 
			else
			{
				markerGeneCellArrayPosNew[qq] = cur_markerGeneArray[mm];
				markerGeneCellArrayPosNew[qq+1] = cur_markerGeneArray[mm+1];
				markerGeneCellArrayPosNew[qq+2] = cur_markerGeneArray[mm+2];
			}
		}
        
		// for debug purpose
        //		if (k==115)
        //		{
        //			for (m=0; m<marker_gene_annocellnum; m++)
        //			{
        //				int mm = m*3;
        //				int qq = 3*marker_gene_annocellnum*k + mm;
        //				
        //				if (!((markerGeneCellArrayPos[qq]==0)&&(markerGeneCellArrayPos[qq+1]==0)&&(markerGeneCellArrayPos[qq+2]==0)))
        //				{
        //					printf("before m=%d, %f, %f, %f\n", m, markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]);
        //					printf("after m=%d, %f, %f, %f\n", m, markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]);
        //				}
        //			}
        //			printf("\n");
        //			
        //		}
        
        
		
	} // for k end
	
	// clean all workspace variables 
	if (subjectCptCellArrayTmp) {delete []subjectCptCellArrayTmp; subjectCptCellArrayTmp=0;}
	if (targetCptCellArrayTmp) {delete []targetCptCellArrayTmp; targetCptCellArrayTmp=0;}
	
	if (existTag1) {delete []existTag1; existTag1=0;}
	if (existTag2) {delete []existTag2; existTag2=0;}
	
	if (cur_refGeneArray) {delete []cur_refGeneArray; cur_refGeneArray =0;}
	if (cur_markerGeneArray) {delete []cur_markerGeneArray; cur_markerGeneArray =0;}
    
}


// compute the mean and stdev of cells in the atlas,
// reference gene and marker genes are combined to generate a unique statistics for each cell
void computeCellStat(float *markerGeneCellArray, float *refGeneCellArray, int *ref_gene_idx, int *dim_marker_gene, int *dim_ref_gene, float *&cellMean, float *&cellStd, int *&observed)
{
	int i,j,k;
	
    //	int fieldnum = 3+FIELDNUM;	
	int fieldnum = dim_marker_gene[0];		
	int annocellnum = dim_marker_gene[1]; // note that the number of cells in markerGeneCellArray equals the total number of cells in the atlas (including all the cells in the reference marker)
	int ref_annocellnum = dim_ref_gene[1];	
	int marker_stacknum = dim_marker_gene[2];
	int ref_stacknum = dim_ref_gene[2];
	
    //	int *cnt = new int [annocellnum];
	observed = new int [annocellnum]; //how many times a given cell is observed or annotated
    //	expected = new int [annocellnum]; //how many stacks a given cell should appear
    
	int len = annocellnum*fieldnum; 
	cellMean = new float [len];
	cellStd = new float [len];
    
	// initialize cellMean and cellStd
	for (i=0; i<len; i++)
	{
		cellMean[i] = 0;
		cellStd[i] = 0;
	}
	
	
	// compute cellMean 
	for (i=0; i<annocellnum; i++)
	{
		observed[i] = 0;
		int nn = i*fieldnum;
		
		
		for (j=0; j<marker_stacknum; j++)
		{
			int mm = j*annocellnum*fieldnum + nn;
			
			bool existflag = 0;
			for (int pp = 0; pp<fieldnum; pp++)
			{
				if (markerGeneCellArray[mm+pp]!=0)
				{
					existflag = 1;
					break;
				}
			}
			
			if (existflag == 1)
			{
				
				observed[i]++;
				for (k=0; k<fieldnum; k++)
					cellMean[nn+k] += markerGeneCellArray[mm+k];
			}
		}
		
        //		if (i<ref_annocellnum) // for reference gene cells, need also to combine reference gene stacks
		if (ref_gene_idx[i]!=-1) // the cell also express reference gene, need also to combine reference gene stack for position statistics
		{
			int qq = ref_gene_idx[i]*fieldnum;
			
			for (j=0; j<ref_stacknum; j++)
			{
				int mm = j*ref_annocellnum*fieldnum + qq;
                
				bool existflag = 0;
				for (int pp = 0; pp<fieldnum; pp++)
				{
					if (refGeneCellArray[mm+pp]!=0)
					{
						existflag = 1;
						break;
					}
				}
				
				if (existflag == 1)				
                    //				if (!((refGeneCellArray[mm]==0)&&(refGeneCellArray[mm+1]==0)&&(refGeneCellArray[mm+2]==0)))
				{
					observed[i]++;
					
                    //				if (i==408)
                    //					printf("****i=%d, j=%d, %f, %f, %f\n", i,j, refGeneCellArray[mm], refGeneCellArray[mm+1], refGeneCellArray[mm+2]);
                    
                    //				if (i==51)
                    //				{
                    //					printf("*****i=%d, j=%d, ", i,j);
                    //					for (int pp=0; pp<fieldnum; pp++)
                    //						printf("%f  ",refGeneCellArray[mm+pp]);
                    //					printf("\n");
                    //				}
                    //
                    //				if (i==967)
                    //				{
                    //					printf("*****i=%d, j=%d, ", i,j);
                    //					for (int pp=0; pp<fieldnum; pp++)
                    //						printf("%f  ",refGeneCellArray[mm+pp]);
                    //					printf("\n");
                    //				}
					
					for (k=0; k<fieldnum; k++)
						cellMean[nn+k] += refGeneCellArray[mm+k];
				}
			}		
		}
		
        //		printf("**********\n");		
	}
	
    //	for (i=0; i<annocellnum; i++)
    //		printf("observed[%d] = %d\n", i, observed[i]);
    
    //	printf("i=408, %d\n", observed[408]);
	
	for (i=0; i<annocellnum; i++)
	{
		if (observed[i]>0)
		{
			int nn = i*fieldnum;
			
			for (k=0; k<fieldnum; k++)
				cellMean[nn+k] /= observed[i];
            
            //			printf("i=%d, mean:", i);
            //			for (int pp=0; pp<fieldnum; pp++)
            //				printf("%f  ",cellMean[nn+pp]);
            //			printf("\n");
            
            
            
            //			if (i==51)
            //			{
            //				printf("i=%d, mean:", i);
            //				for (int pp=0; pp<fieldnum; pp++)
            //					printf("%f  ",cellMean[nn+pp]);
            //				printf("\n");
            //			}			
            //			
            //			if (i==967)
            //			{
            //				printf("i=%d, mean:", i);
            //				for (int pp=0; pp<fieldnum; pp++)
            //					printf("%f  ",cellMean[nn+pp]);
            //					
            //				printf("\n");
            //			}						
		}
		
        //		for debug purpose
        //		for(j=0; j<fieldnum; j++)
        //			if (cellMean[i*fieldnum+j]<0)
        //				printf("i=%d, %f\n", i, cellMean[i*fieldnum+j]);  
		
	}
    
	// compute cellStd
	
	for (i=0; i<annocellnum; i++)
	{
        
		int nn = i*fieldnum;
		
		for (j=0; j<marker_stacknum; j++)
		{
			int mm = j*annocellnum*fieldnum + nn;
            
			bool existflag = 0;
			
			for (int pp = 0; pp<fieldnum; pp++)
			{
				if (markerGeneCellArray[mm+pp]!=0)
				{
					existflag = 1;
					break;
				}
			}
            
			if (existflag==1)
			{
				for (k=0; k<fieldnum; k++)
					cellStd[nn+k] += pow((markerGeneCellArray[mm+k]-cellMean[nn+k]),2);
			}
            
            //			if (!((markerGeneCellArray[mm]==0)&&(markerGeneCellArray[mm+1]==0)&&(markerGeneCellArray[mm+2]==0)))
            //			{
            //				for (k=0; k<fieldnum; k++)
            //					cellStd[nn+k] += pow((markerGeneCellArray[mm+k]-cellMean[nn+k]),2);
            //			}
		}
		
        //		if (i<ref_annocellnum) // for reference gene cells, need also to combine reference gene stacks
		if (ref_gene_idx[i]!=-1) // the cell also express reference gene, need also to combine reference gene stack for position statistics
		{
            
			int qq = ref_gene_idx[i]*fieldnum;
            
			for (j=0; j<ref_stacknum; j++)
			{
				int mm = j*ref_annocellnum*fieldnum + qq;
				
				bool existflag = 0;
				
				for (int pp = 0; pp<fieldnum; pp++)
				{
					if (refGeneCellArray[mm+pp]!=0)
					{
						existflag = 1;
						break;
					}
				}
				
				if (existflag==1)
				{
					for (k=0; k<fieldnum; k++)
						cellStd[nn+k] += pow((refGeneCellArray[mm+k]-cellMean[nn+k]),2);
				}
				
				
                //				if (!((refGeneCellArray[mm]==0)&&(refGeneCellArray[mm+1]==0)&&(refGeneCellArray[mm+2]==0)))
                //				{
                //					for (k=0; k<fieldnum; k++)
                //						cellStd[nn+k] += pow((refGeneCellArray[mm+k]-cellMean[nn+k]),2);
                //				}
			}		
		}
	}
	
	
	
	for (i=0; i<annocellnum; i++)
	{
		if (observed[i]>0)
		{
			int nn = i*fieldnum;
			
			for (k=0; k<fieldnum; k++)
				cellStd[nn+k] = sqrt(cellStd[nn+k]/observed[i]);
            
		}
		
	}
	
}

// called by v3d-plugin
// compute for each cell its expected value (i.e., in how many stacks it should express)
void computeExpectedValue(MarkerGeneInfo marker_gene_info, string dfile_inputdir, string dfile_outputdir,
                          string cellnamelist_prefix, string cellnamelist_surfix, 
                          int refstacknum, int *&expected)
{
    
	int i,j,k;
	QStringList cellNamesAll, namelist;
	int cellnum, num;
	MarkerStacks markerstack;
	int num_old = 0;
	string cellnamelist_filename;
	int markerstacknum;
	
	// read cell names in cellnamelist_atlas.txt
	string cellnamelist_atlas_filename = dfile_inputdir + cellnamelist_prefix + "_atlas" + cellnamelist_surfix;		
	readLines(cellnamelist_atlas_filename, cellNamesAll, cellnum);	
    
	//allocate memory and initialize expected
	expected = new int [cellnum];
	for (i=0; i<cellnum; i++)
		expected[i] = 0;
	
	// compute expected
	for (i=0; i<marker_gene_info.markernum()+1; i++)
	{
        
		if (i==marker_gene_info.markernum())
		{
			cellnamelist_filename = dfile_inputdir + cellnamelist_prefix + "_" + marker_gene_info.reference_markername+ cellnamelist_surfix;			
            //			cellnamelist_filename = dfile_outputdir + "cellnamelist_eve.txt"; //reference marker	
			
			markerstacknum = refstacknum;
		}
		else
		{
//			cellnamelist_filename = dfile_outputdir + cellnamelist_prefix + "_"  + marker_gene_info.marker_stacks.at(i).markername + cellnamelist_surfix;	
            
			cellnamelist_filename = dfile_inputdir + cellnamelist_prefix + "_"  + marker_gene_info.marker_stacks.at(i).markername + cellnamelist_surfix;	
            
			//get the number of stacks for that marker
			markerstack = marker_gene_info.marker_stacks.at(i);
			markerstacknum = (markerstack.signal_ch_filename).size();
		}
		
		//read cell names in the cellnamelist of each marker
		printf("%s\n", 	cellnamelist_filename.c_str());
		readLines(cellnamelist_filename, namelist, num);	
        
        //		//debug
        //		for (j=0; j<num; j++)
        //			printf("%d, %s\n", j, qPrintable(namelist.at(j)));
        
		
		// compute expected
		for (j=num_old; j<num; j++)
            for (k=0; k<cellnum; k++)
            {
                //			if ((namelist.at(j)).compare(cellNamesAll.at(k))==0)
                if (((namelist.at(j).trimmed()).compare(cellNamesAll.at(k).trimmed()))==0)			
                {
                    //				if ((j==0)&&(k==646))
                    //				printf("i=%d, j=%d, k=%d, %s, %s, %d\n", i, j, k, qPrintable(namelist.at(j)), qPrintable(cellNamesAll.at(k)), markerstacknum);
                    expected[k] += markerstacknum;
                    break;
                }
            }
		
		num_old = num;
        
	}
	
    //	//print expected values for debug purpose
    //	for (i=0; i<cellnum; i++)
    //	{
    //		printf("i = %d, expected value = %d\n", i, expected[i]);
    //	}
}


// compute for each cell its expected value (i.e., in how many stacks it should express)
void computeExpectedValue(MarkerGeneInfo marker_gene_info, string dfile_inputdir, int refstacknum, int *&expected)
{
    
	int i,j,k;
	QStringList cellNamesAll, namelist;
	int cellnum, num;
	MarkerStacks markerstack;
	int num_old = 0;
	string cellnamelist_filename;
	int markerstacknum;
	
	// read cell names in cellnamelist_atlas.txt
	string cellnamelist_atlas_filename = dfile_inputdir + "cellnamelist_atlas.txt";		
	readLines(cellnamelist_atlas_filename, cellNamesAll, cellnum);	
    
	//allocate memory and initialize expected
	expected = new int [cellnum];
	for (i=0; i<cellnum; i++)
		expected[i] = 0;
	
	// compute expected
	for (i=0; i<marker_gene_info.markernum()+1; i++)
	{
        
		if (i==marker_gene_info.markernum())
		{
			cellnamelist_filename = dfile_inputdir + "cellnamelist_"+ marker_gene_info.reference_markername+ ".txt";			
            //			cellnamelist_filename = dfile_outputdir + "cellnamelist_eve.txt"; //reference marker	
			
			markerstacknum = refstacknum;
		}
		else
		{
			cellnamelist_filename = dfile_inputdir + "cellnamelist_" + marker_gene_info.marker_stacks.at(i).markername+".txt";	
			//get the number of stacks for that marker
			markerstack = marker_gene_info.marker_stacks.at(i);
			markerstacknum = (markerstack.signal_ch_filename).size();
		}
		
		//read cell names in the cellnamelist of each marker
		printf("%s\n", 	cellnamelist_filename.c_str());
		readLines(cellnamelist_filename, namelist, num);	
        
        //		//debug
        //		for (j=0; j<num; j++)
        //			printf("%d, %s\n", j, qPrintable(namelist.at(j)));
        
		
		// compute expected
		for (j=num_old; j<num; j++)
            for (k=0; k<cellnum; k++)
            {
                //			if ((namelist.at(j)).compare(cellNamesAll.at(k))==0)
                if (((namelist.at(j).trimmed()).compare(cellNamesAll.at(k).trimmed()))==0)			
                {
                    //				if ((j==0)&&(k==646))
                    //				printf("i=%d, j=%d, k=%d, %s, %s, %d\n", i, j, k, qPrintable(namelist.at(j)), qPrintable(cellNamesAll.at(k)), markerstacknum);
                    expected[k] += markerstacknum;
                    break;
                }
            }
		
		num_old = num;
        
	}
	
    //	//print expected values for debug purpose
    //	for (i=0; i<cellnum; i++)
    //	{
    //		printf("i = %d, expected value = %d\n", i, expected[i]);
    //	}
}


// used by the final interface
// save the atlas before co-localization detection

// note that this function is also called in buildAtlas to generate the final merged atlas with stdev values for Ellie to check

void saveAtlasBeforeMerge(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, bool *markerTag, 
                          vector <string> forceAddCellnamelist, vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames,
                          float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellMean_vol, float *cellStd_vol, 
                          int *observed, int *expected, 
                          int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list,int *ref_gene_idx, float threratio)			  
{
	// ----------------------------------------------------------------------------------
	// save mean and std of each cell into one file, which is the basic inforamtion of the atlas
	// ----------------------------------------------------------------------------------	
	string outfilename;
	int i,j,k,m;
	FILE *file;
    
    outfilename = dfile_outputdir + "atlas_all.apo_cellStatistics.txt";
    
	file = fopen(outfilename.c_str(), "wt");	
	
	fprintf(file, "#cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t std_x,\t std_y,\t std_z,\t cellstd_vol,\t observed, \t expected, \t ratio\n");
    
	bool *forceAddTag = new bool [dim_marker_gene[1]];
	
	for (j=0; j<dim_marker_gene[1]; j++)
	{
		forceAddTag[j] = 0;
		for (k=0; k<forceAddCellnamelist.size(); k++)
			if (cellnamelist.at(j).compare(forceAddCellnamelist.at(k))==0)
			{
				forceAddTag[j] = 1;
                //				printf("%d, %d\n", j, forceAddTag[j]);
				break;
			}
		
	}
    
    printf("########## threratio = %f ###############\n", threratio);
    
	for (j=0; j<dim_marker_gene[1]; j++)
	{
		int tmp2 = j*3;
		
        //		printf("observed [%d] = %d\n", j, observed[j]);
        
#ifdef DEBUG		
		if (observed[j]==0)
			printf("observed %d =0, %s\n", j, cellnamelist.at(j).c_str());
		
		if (((float)observed[j]/(float)expected[j])<=1)
			printf("observed = %d, expected = %d, ratio = %f\n", observed[j], expected[j], ((float)observed[j]/(float)expected[j]));
        
		if ((float)observed[j]/(float)expected[j]>1)
		{
			printf("obsevered = %d, expected = %d\n", observed[j], expected[j]);
			printf("j=%d, %s, observed value is bigger than expected value\n", j, cellnamelist.at(j).c_str());
		}
		
#endif
        
		float ratio = (float)observed[j]/(float)expected[j];
		
		if ((ratio>=threratio)||(forceAddTag[j] == 1)) //20091124 add according to Chris
			fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", cellnamelist.at(j).c_str(), cellMean[tmp2], cellMean[tmp2+1], cellMean[tmp2+2], cellMean_vol[j], cellStd[tmp2], cellStd[tmp2+1], cellStd[tmp2+2], cellStd_vol[j], observed[j], expected[j], ratio);
        
	}
	
	fclose(file);
    
	//--------------------------------------------------------------------
	// save mean and std of cells into different files based on markers 
	//--------------------------------------------------------------------
	
	for (j=0; j<marker_gene_info.markernum(); j++)
	{
		string markername;
		markername = marker_gene_info.marker_stacks[j].markername;
		
        outfilename = dfile_outputdir + "atlas_" + markername + ".apo_cellStatistics.txt";
		
		file = fopen(outfilename.c_str(), "wt");
        
        fprintf(file, "#cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t std_x,\t std_y,\t std_z,\t cellstd_vol,\t observed, \t expected, \t ratio\n");
        
        
		for (i=0; i<dim_marker_gene[1]; i++)
		{
            
            float ratio = (float)observed[i]/(float)expected[i];
            
			if ((markerTag[i*(marker_gene_info.markernum()+1)+(j+1)] == 1)&&((ratio>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
                
			{
				int tmp2 = i*3;
                
                fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", cellnamelist.at(i).c_str(), cellMean[tmp2], cellMean[tmp2+1], cellMean[tmp2+2], cellMean_vol[i], cellStd[tmp2], cellStd[tmp2+1], cellStd[tmp2+2], cellStd_vol[i], observed[i], expected[i], ratio);
                
                
			}
		}
		fclose(file);
	}
    
	//--------------------------------------------
	// save mean and std of cells into EVE file 
	//--------------------------------------------
	
    //	printf("%s\n", marker_gene_info.reference_markername.c_str());
	
    outfilename = dfile_outputdir + "atlas_" + marker_gene_info.reference_markername + ".apo_cellStatistics.txt";
    
	printf("%s\n", outfilename.c_str());
	
	file = fopen(outfilename.c_str(), "wt");
    
    fprintf(file, "#cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t std_x,\t std_y,\t std_z,\t cellstd_vol,\t observed, \t expected, \t ratio\n");
    
    
    
	//printf("dim_marker_gene[1] = %d\n", dim_marker_gene[1]);
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		
        float ratio = (float)observed[i]/(float)expected[i];
        
		if ((markerTag[i*(marker_gene_info.markernum()+1)] == 1)&&((ratio>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
		{
            int tmp2 = i*3;
            
            fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", cellnamelist.at(i).c_str(), cellMean[tmp2], cellMean[tmp2+1], cellMean[tmp2+2], cellMean_vol[i], cellStd[tmp2], cellStd[tmp2+1], cellStd[tmp2+2], cellStd_vol[i], observed[i], expected[i], ratio);
			
		}
	}
	fclose(file);
    
    
	
	// ----------------------------------------------------
	// generate .apo file (including the reference marker)
	// ----------------------------------------------------
    
    
    //	float pixmax = 100.0, intensity = 100.0, sdev = 100.0, volsize = 105.0, mass = 100.0;
	float pixmax = 100.0, intensity = 100.0, sdev = 100.0, mass = 100.0;
	float volsize;
	
	// overall apo containing all markers
    outfilename = dfile_outputdir + "atlas_all.apo";
	file = fopen(outfilename.c_str(), "wt");	
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
        
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
		{
			volsize = cellMean_vol[i];
			string markernamelist = "<";
			bool filledTag = 0;
			
			if (markerTag[i*(marker_gene_info.markernum()+1)]==1) //the cell express the reference marker EVE
			{
				markernamelist.append("$").append(marker_gene_info.reference_markername.c_str());
				filledTag = 1; // for print format purpose
			}
			
            //			printf("%d\n", marker_gene_info.markernum());
			for (j=0; j<marker_gene_info.markernum();j++)
			{
				if (markerTag[i*(marker_gene_info.markernum()+1)+j+1]==1) //the cell express that marker
				{
					if (filledTag == 1)
						markernamelist.append("_$").append(marker_gene_info.marker_stacks.at(j).markername);
					else
					{
						markernamelist.append("$").append(marker_gene_info.marker_stacks.at(j).markername);						
						filledTag = 1;
					}
                    //					printf("%s\n", markernamelist.c_str());
				}
			}
			
            //			markernamelist += ">";	
			markernamelist.append(">");	
			
			// write the cell into the atlas
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
					pixmax, intensity, sdev, volsize, mass);
		}
	}
	fclose(file);
    
	//---------------------------------------
	// write apo file for each marker
	//---------------------------------------
	
	for (j=0; j<marker_gene_info.markernum(); j++)
	{
		string markername;
		markername = marker_gene_info.marker_stacks[j].markername;
		
        outfilename = dfile_outputdir + "atlas_" + markername + ".apo";
		
		file = fopen(outfilename.c_str(), "wt");
        
#ifdef DEBUG
		printf("%s, %d\n", outfilename.c_str(), marker_gene_info.markernum());	
#endif
        
		for (i=0; i<dim_marker_gene[1]; i++)
		{
            //			if (markerTag[i*(marker_gene_info.markernum()+1)+(j+1)] == 1) //current cell expresses this marker
			if ((markerTag[i*(marker_gene_info.markernum()+1)+(j+1)] == 1)&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
                
			{
				int nn = i*3;
				volsize = cellMean_vol[i];
                
				// write the cell into the atlas
				string markernamelist = "<$";
				markernamelist.append(marker_gene_info.marker_stacks.at(j).markername).append(">");
                //				printf("%s\n", markernamelist.c_str());
				fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
						i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
						pixmax, intensity, sdev, volsize, mass);
			}
		}
		fclose(file);
	}
	
	//--------------------------------------------
	// write apo file for reference gene (eve)
	//--------------------------------------------
	
	printf("%s\n", marker_gene_info.reference_markername.c_str());
	
    outfilename = dfile_outputdir + "atlas_"+marker_gene_info.reference_markername+".apo";
    
	printf("%s\n", outfilename.c_str());
	
	file = fopen(outfilename.c_str(), "wt");
	printf("dim_marker_gene[1] = %d\n", dim_marker_gene[1]);
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		
        //		if (markerTag[i*(marker_gene_info.markernum()+1)] == 1) //current cell expresses this marker
		if ((markerTag[i*(marker_gene_info.markernum()+1)] == 1)&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
		{
			int nn = i*3;
			volsize = cellMean_vol[i];
			
			// write the cell into the atlas
            //			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
            //					i,i,cellnamelist.at(i).c_str(),"<$EVE>", int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
            //					pixmax, intensity, sdev, volsize, mass);
            
			string markernamelist = "<$";
			markernamelist.append(marker_gene_info.reference_markername.c_str()).append(">");
			
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
					pixmax, intensity, sdev, volsize, mass);
			
		}
	}
	fclose(file);
    
    //	printf("\n");
	
	
//	//--------------------------------------------
//	// write apo file for different cell types
//    // in fact, no need to write to cell types before merging
//  // in addition, no appropriate cell name list is provide before merging
//	//--------------------------------------------
//	
//	
//    for (i=0; i<cellTypeNames.size(); i++) // different cell types
//    {
//        
//        outfilename = dfile_outputdir + "atlas_" + cellTypeNames.at(i) + ".apo";
//        file = fopen(outfilename.c_str(), "wt");
//        
//        for (j=0; j<dim_marker_gene[1]; j++)
//        {
//            bool foundflg = 0;
//            for (k=0; k<cellTypeNamelist[i].size(); k++)
//            {
//                if (cellnamelist.at(j).compare(cellTypeNamelist[i].at(k))==0)
//                {
//                    foundflg = 1;
//                    break;
//                }
//            }
//            
//            if (((((float)observed[j]/(float)expected[j])>threratio)||(forceAddTag[j]==1))&&(foundflg==1)) //20091124 according to Chris		
//            {
//                int nn = j*3;
//                volsize = cellMean_vol[j];
//                
//                string markernamelist = "<$";
//                
//                if (markerTag[j*(marker_gene_info.markernum()+1)] == 1) // eve
//                    markernamelist.append("EVE_");
//                
//                for (m=0; m<marker_gene_info.markernum(); m++) // other markers
//                {
//                    if (markerTag[j*(marker_gene_info.markernum()+1)+m+1] == 1)
//                        markernamelist.append(marker_gene_info.marker_stacks[m].markername.c_str()).append("_");
//                }
//                
//                markernamelist.append(">");
//                
//                fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
//                        j,j,cellnamelist.at(j).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
//                        pixmax, intensity, sdev, volsize, mass);
//                
//            }
//        }
//        
//        fclose(file);
//        
//    }
	
	
	if (forceAddTag) {delete []forceAddTag, forceAddTag = 0;}
}


// used by the plugin interface

void coLocalizationDetection(QStringList atlas_apo_filelist, QString dfile_output, float thredis, bool symmetryRule, bool bilateralRule)
{
	
    
    long int i,j, k,m, n, p, q;
 	int collocalCellNum = -1;	
    //	float thredis = 6; // distance threshold which determines potential cell colocalization	
//	float thredis = 10; // distance threshold which determines potential cell colocalization	
    
    
	// ---------------------------------------------------------------------------------
	// read the contents of each atlas apo, and concatenate them into one file
	// ---------------------------------------------------------------------------------
	
	vector <QStringList> atlas_apo_combined;
	int num_cell = 0; // total number of cells when cancatenate all the apo files
    int num_apo_file = atlas_apo_filelist.size(); // number of apo atlas files
	int *num_each_apo = new int [num_apo_file]; // number of cells in each apo file
	
	for (i=0; i<num_apo_file; i++)
	{
		vector <QStringList> tmp;
        //		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_each_apo[i]);
		readLines(qPrintable(atlas_apo_filelist.at(i)), tmp, num_each_apo[i]);
		
		for (j=0; j<num_each_apo[i]; j++)		
			atlas_apo_combined.push_back(tmp[j]);
		
		num_cell += num_each_apo[i];
	}
	
	
//#ifdef DEBUG	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j<atlas_apo_combined[i].size(); j++)
			printf("%s, ", qPrintable(atlas_apo_combined[i].at(j)));		
		printf("\n");
	}
//#endif
	
    
    // indicate for each cell which apo file it comes from
    // use this to replace the marker rule, i.e., cells from the 
    // same apo file cannot colocalize; in comparison to: cells
    // express the same marker cannot be colocalized.
    
    short int *same_file_tag = new short int [num_cell];
    int cnt = 0;
    
    for (i=0; i<num_apo_file; i++)
    {
        for (j=0; j<num_each_apo[i]; j++)
            same_file_tag[cnt+j] = i;
        
        cnt+=num_each_apo[i];
        
    }
    
#ifdef DEBUG    
    for (i=0; i<num_cell; i++)
        printf("%d, %d\n", i, same_file_tag[i]);
#endif
    
	// -------------------------
	// parse atlas files
	// -------------------------
	
	float *cellMean = new float [num_cell*3];
	
	vector <vector <string> > markerName;
	
	
	// get information of cell position and markers
	
	for (i=0; i<num_cell; i++)
	{
		
		cellMean[i*3] = atof(qPrintable(atlas_apo_combined[i].at(5)));
		cellMean[i*3+1] = atof(qPrintable(atlas_apo_combined[i].at(6)));
		cellMean[i*3+2] = atof(qPrintable(atlas_apo_combined[i].at(4)));		
		
		
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_apo_combined[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring2; 
        tmpstring2.clear();
		
        //		printf("length of tmpstring2 = %d\n", tmpstring2.size());
		
		while (j<atlas_apo_combined[i].at(3).size()) // at(3) saves marker information
		{	
			
			if (atlas_apo_combined[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				
				tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				
				start_pos = j+2;				
				cnt++;
			}
			
			j++;
		}
		tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos)));
		markerName.push_back(tmpstring2); 
		
#ifdef DEBUG		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, ", i, markerName[i].at(j).c_str(),  markerName[i].size());
		
		printf("\n");
#endif				
		
	}
	
#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName[i].size());
		
		for (j=0; j< markerName[i].size(); j++)
		{
			
            //			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName[i].at(j).c_str());
			
		}
		printf("\n");
	}
#endif	
	
	
	
	// -----------------------------------------	 
	// compute pair-wise distance between cells
	// -----------------------------------------	 
	
	float *celldis = new float [num_cell*num_cell];
	
	
	for (i=0; i<num_cell; i++)
	{
		
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))) 		
		{
			for (j=0; j<num_cell; j++)
			{
				int mm = j*3;
				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))) 		
				{
					celldis[i*num_cell+j] = sqrt((cellMean[nn]-cellMean[mm])*(cellMean[nn]-cellMean[mm]) 
                                                 + (cellMean[nn+1]-cellMean[mm+1])*(cellMean[nn+1]-cellMean[mm+1]) 
                                                 + (cellMean[nn+2]-cellMean[mm+2])*(cellMean[nn+2]-cellMean[mm+2]));
                    //					printf("%d, %d, %f\n", i, j, celldis[i*num_cell+j]);
				}
				else
					celldis[i*num_cell+j] = 9999;
				
			}
		}
		else
			for (j=0; j<num_cell; j++)
			{
				celldis[i*num_cell+j] = 9999;
			}
	}
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// 1): cells from the same apo file cannot be colocalized
    // 2): distance rule
    // 3): L/R rule
	// ---------------------------------------------------------------------
	
	unsigned char *prediction = new unsigned char [num_cell*num_cell];
	unsigned char shareMarkerTag = 0;
	
	
    for (i=0; i<num_cell; i++)
    {
        int nn = i*num_cell;
        
        for (j=0; j<num_cell; j++)
            prediction[nn + j] = 0;

    }
    
#ifdef DEBUG		
	
	for (i=0; i<num_cell-1; i++)
	{
		for (j=0; j< markerName[i].size(); j++)
			printf("%d, %d, %s, %d", i, j, markerName[i].at(j).c_str(),  markerName[i].size());
	}
	
#endif
	
	for (i=0; i<num_cell-1; i++)
	{
		int nn = i*num_cell;
		
		size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
		//		string surfix_i = atlas_apo_combined[i].at(2).substr(len_i-2, len_i-1);
		string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // determine if it is right or left
		
		// select those whose mean distance with respect to cell i is less than thredis
		for (j=i+1; j<num_cell; j++)
		{
			
			k = 0;
			int n2 = 0;
			
			shareMarkerTag = 0;
			prediction[nn+j] = 0;
			
            //			if (j!=i)
			{
                
                //                // method 1: marker rule: check if cell i and cell j express the same marker gene
                //				for (p=0; p<markerName[i].size(); p++)
                //				{
                //					for (q=0; q<markerName[j].size(); q++)
                //					{
                ////                        printf("%d, %d, %d, %d, %s, %s\n", i, j, p, q, markerName[i].at(p).c_str(),  markerName[j].at(q).c_str());
                //						
                //						if (markerName[i].at(p).compare(markerName[j].at(q))==0)
                //						{
                //							shareMarkerTag = 1;
                //							break;
                //						}
                //					} // for q end
                //					if (shareMarkerTag == 1)
                //						break;
                //				} // for p end
                
				
                // method 2: same apo file rule: check if cell i and cell j are from the same apo file 
                if (same_file_tag[i]==same_file_tag[j])
                    shareMarkerTag = 1;
                
//				if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene
				if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene, 
				{
                   
                    
                    // check if cell i and cell j satisfy the Left/right rule
                    
//                    printf("symmetryRule = %s\n", symmetryRule? "true" : "false"); 

                    if (symmetryRule == true) // applies summetry rule
                    {
//                        printf("Now symmetryRule is true\n");
                               
                        size_t len_j = atlas_apo_combined[j].at(2).size();
                        string surfix_j = qPrintable(atlas_apo_combined[j].at(2).mid(len_j-1, 1));

//                        printf("prediction = %d\n", prediction[nn+j]);
//                        printf("surfix_i = %s\n", surfix_i.c_str());
//                        printf("surfix_j = %s\n", surfix_j.c_str());
                        
                        
                        if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
                            || ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
                        {
                            //						printf("%d, %d, %f, %s, %s\n", i,j,celldis[nn+j],surfix_i.c_str(), surfix_j.c_str());
                            if (celldis[nn+j]<thredis) // distance rule
                                prediction[nn+j] = 1; 
                            else
                                prediction[nn+j]=0; 
                        }
                        
                    }
                    else
                    {
//                        prediction[nn+j] = 1;
                        
                        if (celldis[nn+j]<thredis) // apply distance rule directly without considering symmetry rule
                            prediction[nn+j] = 1; 
                        else
                            prediction[nn+j]=0; 
                        
                    };
				}
			} // if (j!=i) end
		} // for j end
	} // for i end
	
    
    //#ifdef DEBUG
    for (i=0; i<num_cell; i++)
    {
        int nn = i*num_cell;
        for (j=0; j<num_cell; j++)
        {
            if (prediction[nn+j] == 1)
                printf("%d, %d, %d\n", i,j, prediction[nn+j]);
        }
    }
    //#endif
    
	// --------------------
	// check bilateral rule
	// --------------------
	
    // find for each cell, its ipsilateral parterner
    
    if (bilateralRule == true)
    {
        QString search_str;
        int *LR_parterner_idx = new int [num_cell];
        
        for (i=0; i<num_cell; i++)
        {
            
//            LR_parterner_idx[i] = 0;
            LR_parterner_idx[i] = -1;
            
            // determine the name of the ipsilabtetal parterner
            
            size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
            string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // tell if it is left or right
            
            search_str = "";
            
            string tmp_str1 = "L";
            string tmp_str2 = "R";
            int len = atlas_apo_combined[i].at(2).size();
            
            if (surfix_i.compare(tmp_str1)==0)
                search_str = atlas_apo_combined[i].at(2).mid(0,len_i-1) + "R";
            else if (surfix_i.compare(tmp_str2)==0)
                search_str = atlas_apo_combined[i].at(2).mid(0,len_i-1) + "L";
            
            // search for the ipsilater paterner
            
            for (j=0; j<num_cell; j++)
            {
                if (atlas_apo_combined[j].at(2).trimmed().compare(search_str.trimmed())==0)
                {
                    LR_parterner_idx[i] = j;
                    
//                    QString hh1 = atlas_apo_combined[j].at(2).trimmed();
//                    QString hh2 = search_str.trimmed();
//                    
//                    printf("atlas_apo_combined[i]=%s, hh1=%s, hh2=%s\n", atlas_apo_combined[i].at(2).trimmed().toStdString().c_str(), hh1.toStdString().c_str(), hh2.toStdString().c_str());
//                    printf("LR_parterner_idx = %d\n", LR_parterner_idx[i]);
                    break;
                }
                
            }
        }
        
        
        //for each detect coexpression pair, check if its  ipsilateral parterners also coexpress, if not discard the pair as coexpression
        
//        for (i=0; i<num_cell-1; i++)
//        {
//            
//            int nn = i*num_cell;
//            
//            for (j=i+1; j<num_cell; j++)
//            {
//                
//                if ((prediction[nn+j] == 1)&&(LR_parterner_idx[i]!=0)&&(LR_parterner_idx[j]!=0))
//                {
//                    int pp = LR_parterner_idx[i]*num_cell + LR_parterner_idx[j];
//                    int qq = LR_parterner_idx[j]*num_cell + LR_parterner_idx[i];
//                    
////                    if ((prediction[pp]==0)&&(prediction[qq]==0))
//                    
//                    if ((prediction[pp]==0)||(prediction[qq]==0))                    
//                        prediction[nn+j] = 0;
//                }
//                
//            }
//        }
        
        
        for (i=0; i<num_cell; i++)
        {
            
            int nn = i*num_cell;
            
            for (j=0; j<num_cell; j++)
            {
                
                
                if ((j!=i)&&(prediction[nn+j] == 1)&&(LR_parterner_idx[i]!=-1)&&(LR_parterner_idx[j]!=-1))
                {
                    int pp = LR_parterner_idx[i]*num_cell + LR_parterner_idx[j];

//                    printf("(%s, %s), prediction[i,j]=%d, (%s, %s), prediction(i2, j2)=%d\n", 
//                           atlas_apo_combined[i].at(2).trimmed().toStdString().c_str(), atlas_apo_combined[j].at(2).trimmed().toStdString().c_str(), prediction[nn+j], 
//                           atlas_apo_combined[LR_parterner_idx[i]].at(2).trimmed().toStdString().c_str(), atlas_apo_combined[LR_parterner_idx[j]].at(2).trimmed().toStdString().c_str(), prediction[pp]);
                                        
                    
                    if (prediction[pp]==0)                    
                        prediction[nn+j] = 0;
                }
                
            }
        }
        
        if (LR_parterner_idx) {delete []LR_parterner_idx; LR_parterner_idx=0;}
        
    }
    
    
	// ---------------------------------------------------------------------
	// sort candidates
	// ---------------------------------------------------------------------
	
    //	string filename = dfile_output + "colocalizedCells.txt";
    
	string filename = qPrintable(dfile_output + ".coexpress.txt");
    
	printf("%s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(),"wt");
	
    
	
    
    ////	filename = test_colocalization_atlas_apo_filename + ".coexpress.apo";
    //	filename = dfile_output + ".coexpress.apo";    
    //	FILE *file2 = fopen(filename.c_str(), "wt");
    
	bool *coexpress_flg = new bool [num_cell];
	
	for (i=0; i<num_cell; i++)
        coexpress_flg[i] = 0;
	
    float *sortidx2 = new float [num_cell+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *sortval2 = new float [num_cell+1];
	
    
	for (i=0; i<num_cell; i++)
	{
		
		int candcnt = 0;
		
		// sort each cell based on its distance to the cell in consideration
		
		sortval2[0] = -999; // the 0th element is not sorted in sort2			
		sortidx2[0] = -999;
		
		for (j=0; j<num_cell; j++)
		{	
			sortval2[j+1] = celldis[i*num_cell+j];
			sortidx2[j+1] = j;
		}
		
		sort2(num_cell, sortval2, sortidx2);// the first element is not sorted
		
		j = 0;
		bool print_tag = 0;
		bool cellname_printed_tag = 0;
		
		
		while (sortval2[j+1] < thredis)
		{
			if ((sortidx2[j+1]!=i) && (prediction[i*num_cell+(long)sortidx2[j+1]]==1))
			{
                //				fprintf(file, "cellname: %s, Candidate: %s , meanx=%f, meany=%f, meanz=%f\n", 
                //						qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
                
				if (cellname_printed_tag == 0)
				{
					fprintf(file, "\ncellname: %s, meanx=%f, meany=%f, meanz=%f\n", 
							qPrintable(atlas_apo_combined[i].at(2)), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2]);
					
					cellname_printed_tag = 1;
				}
				
				fprintf(file, "Candidate:%s , meanx=%f, meany=%f, meanz=%f\n", 
						qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
				
				
				if (print_tag == 0)
				{
					print_tag = 1;
					coexpress_flg[i] = 1;
					coexpress_flg[(long)sortidx2[j+1]] = 1;
				}
                
			}
			j++;
			
		}; // while (sortval2[j+1] < thredis) end
		
	} // for i end
	
	fclose(file);
    //	fclose (file2);
	
	// ---------------------------------------------------------------------
	// create .apo files for visualizing co-localization
	// ---------------------------------------------------------------------
	
    //	filename = dfile_output + ".coexpress.apo";
	filename = qPrintable(dfile_output + ".coexpress.apo");
    
	file = fopen(filename.c_str(), "wt");
	
    
	unsigned char *mycolor = new unsigned char [num_cell*3]; // color of cells to be rendered
    
	unsigned char colorval[60] = {255, 0, 0, //red
		0, 255, 0, //green
		0, 0, 255, //blue
		255, 255, 0, //yellow
		255, 0, 255, //purple
		0, 255, 255, //cyan        
		0, 0, 0, //black        
		128, 128, 255, 
		128, 255, 128, 
		255, 128, 128, 
		128, 196, 0, 
		0, 128, 196,
		196, 0, 128,
		128, 0, 196,
		0, 196, 128,
		196, 128, 0,
		64, 128, 128,
		128, 64, 128,
		128, 128, 64,
        128, 196, 255};
	
	
	
	int num =0;
    printf("number of colors = %d\n", atlas_apo_filelist.size());
    
	for (i=0; i<atlas_apo_filelist.size(); i++)
	{
		
		for (j=0; j<num_each_apo[i]; j++)
		{
			mycolor[(num+j)*3] = colorval[i*3];
			mycolor[(num+j)*3+1] = colorval[i*3+1];
			mycolor[(num+j)*3+2] = colorval[i*3+2];
		}
		num += num_each_apo[i];
	}
    
    
	for (i=0; i<num_cell; i++)
	{
		if (coexpress_flg[i] == 1)	
		{
			mycolor[i*3] = mycolor[i*3+1] = mycolor[i*3+2] = 255;
		}
	}
	
	for (i=0; i<num_cell; i++)
	{
        
		fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,,%d,%d,%d\n",
				i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
				atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))), mycolor[i*3], mycolor[i*3+1], mycolor[i*3+2]);
		
	}
	
	
	fclose (file);
	
//    if (LR_parterner_idx) {delete []LR_parterner_idx; LR_parterner_idx=0;}
	if (same_file_tag) {delete []same_file_tag; same_file_tag=0;}
    
	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
	if (sortval2) {delete []sortval2; sortval2 = 0;}
	if (celldis) {delete []celldis; celldis = 0;}
	if (prediction) {delete []prediction; prediction = 0;}
	if (cellMean) {delete []cellMean; cellMean =0;}
	if (coexpress_flg) {delete []coexpress_flg; coexpress_flg = 0;}
	
	if (num_each_apo) {delete []num_each_apo; num_each_apo=0;}
	if (mycolor) {delete []mycolor; mycolor = 0;}
    
}


// merge co-expressed cells and generate final atlas, cell names are replace and statistics (mean) recomputed 
// since inidividual stacks will not be provided, there is no way to compute new stdev
//used in final atlas builder plugin

void merge_atlas(QStringList atlas_apo_filelist, QString dfile_merge_cells, QString dir_output, QString cellstat_file_surfix, QString markermap_file_name, QString merged_atlas_prefix, QStringList celltype_file_name_list)
{
	// ------------------------------------------
	// initializing variables and pointers
	// ------------------------------------------
    
    long int i,j, k,m, n, p, q;
    
	float *cellMean_final = 0;
    //	float *cellStd_final= 0;
	
	float *cellMean_vol_final = 0;
    //	float *cellStd_vol_final = 0;
	
	int *observed_final = 0;
	int *expected_final = 0;    
	unsigned char *markerTag_final = 0;
    
	int *observed = 0;
	int *expected = 0;
    
    float *cellMean = 0;
    
	// ---------------------------------------------------------------------------------
	// read the contents of each atlas apo, and concatenate them into one file
	// ---------------------------------------------------------------------------------
	
	vector <QStringList> atlas_apo_combined;
	int num_cell = 0; // total number of cells when cancatenate all the apo files
    int num_apo_file = atlas_apo_filelist.size(); // number of apo atlas files
	int *num_each_apo = 0;
    
    if (num_apo_file>0) 
        num_each_apo = new int [num_apo_file]; // number of cells in each apo file
    else
        qDebug("the num_apo_file <=0. return");
	
    
    
	for (i=0; i<num_apo_file; i++)
	{
		vector <QStringList> tmp;
        int tmp_num;
        
        //		readLines(qPrintable(atlas_apo_filelist.at(i)), tmp, num_each_apo[i]);
		readLines(qPrintable(atlas_apo_filelist.at(i)), tmp, tmp_num);
        
		
        int startline = 0;
        
        if (tmp[0].at(0).trimmed().startsWith("#")) // the first line is comment line
        {
            startline = 1;
            num_each_apo[i] = tmp_num-1;
        }
        else
        {
            startline = 0;
            num_each_apo[i] = tmp_num;
            
        }
        
		for (j=startline; j<tmp_num; j++)	
			atlas_apo_combined.push_back(tmp[j]);
        
		
		num_cell += num_each_apo[i];
	}
	
	
#ifdef DEBUG	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j<atlas_apo_combined[i].size(); j++)
			printf("%s, ", qPrintable(atlas_apo_combined[i].at(j)));		
		printf("\n");
	}
#endif
	
    
	// -------------------------
	// parse atlas files
	// -------------------------
	
	cellMean = new float [num_cell*3];
	
	vector <vector <string> > markerName; // markers that each cell in the cancatenated set expresses, note that when building the final atlas from individual markers, it is not the list of markers of each final cell, therefore, each cell has only one marker; but when building atlas by combining individual marker with mother atlas, each cell in the mother atlas may express a list of cells
	
	// get information of cell position and markers
	
	for (i=0; i<num_cell; i++)
	{
		
		cellMean[i*3] = atof(qPrintable(atlas_apo_combined[i].at(5)));
		cellMean[i*3+1] = atof(qPrintable(atlas_apo_combined[i].at(6)));
		cellMean[i*3+2] = atof(qPrintable(atlas_apo_combined[i].at(4)));		
		
		// assign value to markerName[i]
        
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_apo_combined[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring2; 
        tmpstring2.clear();
		
        //		printf("length of tmpstring2 = %d\n", tmpstring2.size());
		
		while (j<atlas_apo_combined[i].at(3).size()) // at(3) saves marker information
		{	
			
			if (atlas_apo_combined[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				
				tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				
				start_pos = j+2;				
				cnt++;
			}
			
			j++;
		}
		tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos)));
		markerName.push_back(tmpstring2); 
		
//#ifdef DEBUG		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, %d", i, markerName[i].at(j).c_str(),  markerName[i].size());
		
		printf("\n");
//#endif				
		
	}
    
	
#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName[i].size());
		
		for (j=0; j< markerName[i].size(); j++)
		{
			
            //			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName[i].at(j).c_str());
			
		}
		printf("\n");
	}
#endif	
	
    printf("finish parse atlas.\n");
    
    // -----------------------------------------------
    // generate the overall set of markers
    // -----------------------------------------------
    
    vector <string> allMarkerSet;
    
    for (i=0; i<num_cell; i++)
    {
        for (j=0; j<markerName[i].size(); j++)
        {
            m = 0;
            
            while(m<allMarkerSet.size())
            {
                if (markerName[i].at(j).compare(allMarkerSet.at(m))!=0)
                    m++;
                else
                    break;
                
            }
            
            if (m>=allMarkerSet.size()) // did not find match, add the marker to allMarkerSet
                allMarkerSet.push_back(markerName[i].at(j));
            
        }
        
    }
    
    int markercnt = allMarkerSet.size();
    
    printf("marker number = %d\n", markercnt);
    printf("finish generate allMarkerSet.\n");
    
    // -------------------------------------------------------------------------------------------------------------------------
    // parse the file containing all the cell names before and after merging (generated by coexpression detection (+ manual curation)
    // generate the file containing the cells only in selected .apo files
    // -------------------------------------------------------------------------------------------------------------------------
    
	vector <QStringList> final_cellnamelist_all;
	int final_cellnum_all = 0;
	
    printf("%s\n", dfile_merge_cells.toStdString().c_str());
    
	readLines(dfile_merge_cells.toStdString(), final_cellnamelist_all, final_cellnum_all);
	
	
    //output cellnamelist to check if it is correct
#ifdef DEBUG	
    
	for (i=0; i<final_cellnum_all; i++)
	{
		for (j=0; j<final_cellnamelist_all[i].size(); j++)
			printf("%s, ", qPrintable(final_cellnamelist_all[i].at(j)));
		printf("\n");
        
	}
    
#endif
    
    // generate the file containing the cells only in selected .apo files
    
	vector <QStringList> final_cellnamelist;
    //	int final_cellnum = num_cell;
	int final_cellnum = 0;
    int nn_cnt = 0;
    
    
    unsigned char *already_accessed_tag = new unsigned char [final_cellnum_all];
    
    for (j=0; j<final_cellnum_all; j++)
        already_accessed_tag[j] = 0;
    
	for (i=0; i<num_cell; i++)
	{
        //		for (j=0; j<atlas_apo_combined[i].size(); j++)
        //			printf("%s, ", qPrintable(atlas_apo_combined[i].at(2)));		
        unsigned char foundtag = 0;
        
        for (j=0; j<final_cellnum_all; j++)
        {
            for (k=1; k<final_cellnamelist_all[j].size(); k++)
            {
                if (atlas_apo_combined[i].at(2).trimmed().compare(final_cellnamelist_all[j].at(k).trimmed())==0)
                {
                    foundtag = 1;
                    break;
                }
            }
            
            if (foundtag == 1)
                break;
            
        }
        
        
        //        printf("i=%d, j=%d\n", i, j);
        //        printf("%s\n", qPrintable(atlas_apo_combined[i].at(2).trimmed()));
        //        printf("%s\n", qPrintable(final_cellnamelist_all[j].at(k).trimmed()));
        
        if ((foundtag == 1) & (already_accessed_tag[j] ==0))
        {
            nn_cnt++;
            
            QStringList tmpstrlist;
            
            for (k=0; k<final_cellnamelist_all[j].size(); k++)
                tmpstrlist.push_back(final_cellnamelist_all[j].at(k));
            
            final_cellnamelist.push_back(tmpstrlist);
            
            already_accessed_tag[j] = 1;
            
            
        }
        
	}
    
    final_cellnum = final_cellnamelist.size();
    
#ifdef DEBUG	
    
	for (i=0; i<final_cellnum; i++)
	{
        printf("%d, %d, %d, %d, %d, %d\n", i, final_cellnum, final_cellnamelist.size(), final_cellnum_all, final_cellnamelist_all.size(), nn_cnt);
		for (j=0; j<final_cellnamelist[i].size(); j++)
			printf("%s, ", qPrintable(final_cellnamelist[i].at(j)));
		printf("\n");
        
	}
    
#endif
    
    
    printf("finish read coexpression file.\n");
    
    //    goto Label_exit_merge_atlas;
    
	// --------------------------------------------
	// read cell name list of different cell types 
	// --------------------------------------------
	{
//        vector <string> cellTypeNames;
//        
//        cellTypeNames.push_back("interneuron");
//        cellTypeNames.push_back("motoneuron");
//        cellTypeNames.push_back("secretory");
//        cellTypeNames.push_back("galia");
//        
//        
//        int cellTypeNum[cellTypeNames.size()];
//        
//        vector < vector <string> > cellTypeNamelist;
//        
//        for(i=0; i<cellTypeNames.size(); i++)
//        {
//            //		string dfile_cellTypes = dir_output.append(QString("cellname_type_")).append(cellTypeNames[i]).append(".txt");
//            string dfile_cellTypes = dir_output.toStdString() + "cellname_type_" + cellTypeNames[i] + ".txt";
//            
//            printf("%s\n", dfile_cellTypes.c_str());
//            
//            vector <string> tmp;
//            readLines(dfile_cellTypes, tmp, cellTypeNum[i]);
//            cellTypeNamelist.push_back(tmp);
//            
//        }
//        
//        printf("finish generate celltype name.\n");

        
        vector <string> cellTypeNames;
        int cellTypeNum[celltype_file_name_list.size()];
        //
        vector < vector <string> > cellTypeNamelist;
        
//        for(i=0; i<cfginfo.cellTypeFileName.size(); i++)
        for(i=0; i<celltype_file_name_list.size(); i++)
        {
            //        string dfile_cellTypes = dfile_inputdir + cfginfo.cellTypeNames.at(i) + cfginfo.cellNameFileSurfix;
            string dfile_cellTypes = celltype_file_name_list.at(i).toStdString(); //cellTypeNames has folder and surfix information
            
            printf("%s\n", dfile_cellTypes.c_str());
            
            vector <string> tmp;
            readLines(dfile_cellTypes, tmp, cellTypeNum[i]);
            
            cellTypeNamelist.push_back(tmp);
            
            // assign value to cellTypeNames
            int pos1 = celltype_file_name_list.at(i).lastIndexOf("_", -1);        
            int pos2 = celltype_file_name_list.at(i).lastIndexOf(".", -1);
            
            QString strtmp = celltype_file_name_list.at(i).mid(pos1+1, pos2-pos1-1);
            cellTypeNames.push_back(strtmp.toStdString());
            
        }
        
        printf("finish reading cell type files.\n");
        
        
        // --------------------------------------------------------------
        // read in observed and expected values
        // --------------------------------------------------------------
        
        if (num_cell>0)
        {
            qDebug()<<"num_cell = " << num_cell;
            observed = new int [num_cell];
            expected = new int [num_cell];
        }
        else
        {
            qDebug()<<"num_cell < 0";
            goto Label_exit_merge_atlas;
        }
        
        //test passed here
        
        int cnum=0;
        printf("num_apo_file = %d\n", num_apo_file);
        
        for (i=0, cnum=0; i<num_apo_file; i++)
        {
            vector <QStringList> tmp;
            tmp.clear();
            
            //		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_each_apo[i]);
            string fn = atlas_apo_filelist.at(i).toStdString() + "_" + cellstat_file_surfix.toStdString();
            
            printf("%s\n", fn.c_str());
            
            int num_row;
            
            readLines(fn, tmp, num_row);
            
            int startline = 0;
            
//            printf("num_row = %d\n", num_row);
//            printf("%s\n", qPrintable(tmp[0].at(0)));
            
            if (tmp.size()>0)
            {
                if (tmp[0].size()>0)
                    if (tmp[0].at(0).trimmed().startsWith("#")) // the first line is comment line
                    {
                        startline = 1;
                    }
            }
            else
                continue;
            
            
            if (tmp[0].size()>8) // std info are saved
            {
                int mycnt = 0;
                for (j=startline; j<num_row; j++)	// j=0 is the comment line
                {
                    observed[cnum+mycnt] = atoi(tmp[j].at(9).toStdString().c_str());
                    expected[cnum+mycnt] = atoi(tmp[j].at(10).toStdString().c_str());
                    mycnt++;
                    
                }
            }
            else
            {
                int mycnt = 0;
                for (j=startline; j<num_row; j++)
                {
                    observed[cnum+mycnt] = atoi(tmp[j].at(5).toStdString().c_str());
                    expected[cnum+mycnt] = atoi(tmp[j].at(6).toStdString().c_str());
                    mycnt++;
                }
                
            }
            
            cnum += num_each_apo[i];
            qDebug() << "cnum=" << cnum;
        }
        
        printf("finish compute observed and expected.\n");
        
        
        // -----------------------------------------------------------------
        // assigne values for cellMean_final, markerTag_final, etc
        // -----------------------------------------------------------------
        
        // allocate memory for pointers
        
        observed_final = new int [final_cellnum]; //how many times a given cell is observed or annotated	
        expected_final = new int [final_cellnum];
        markerTag_final = new unsigned char [final_cellnum * markercnt];
        
        int len_final = final_cellnum*3; 
        
        cellMean_final = new float [len_final];
        //	cellStd_final = new float [len_final];
        
        cellMean_vol_final = new float [final_cellnum];
        //    cellStd_vol_final = new float [final_cellnum];
        
        // initialize cellMean_final and cellStd_final, markerTaag_final
        
        for (i=0; i<len_final; i++)
        {
            cellMean_final[i] = 0;
            //		cellStd_final[i] = 0;
        }
        
        for (i=0; i<final_cellnum; i++)
        {
            cellMean_vol_final[i] = 0;
        }
        
        
        for (i=0; i<final_cellnum*markercnt; i++)
        {
            markerTag_final[i] = 0;
        }
        
        //--------------------------------------------------------------------
        // compute merged_cell_idx,  observed_final, expected_final,  markerTag_final, cellMean_final
        //--------------------------------------------------------------------
        
        vector <int> merged_cell_idx;
        vector <string> thisFinalCellMarker;
        for (i=0; i<final_cellnum; i++)
        {
            if (final_cellnamelist[i].size()>0)
                printf("i=%d, %s\n", i, qPrintable(final_cellnamelist[i].at(0)));
            else
                qDebug() << "length < 0!!";
            
            //		int ss = i*3;
            
            // compute merged_cell_idx,  observed_final, expected_final, markerTag_final
            
            merged_cell_idx.clear(); // indicate which cell should be merged;
            
            observed_final[i] = 0;
            expected_final[i] = 0;
            
            thisFinalCellMarker.clear(); // the marker set of the current cell in the final atlas
            
            // parse which cells co-express therefore should be merged
            //		printf("%d\n", final_cellnamelist.size());
            //		printf("size = %d\n", final_cellnamelist[i].size());
            
            for (j=1; j<final_cellnamelist[i].size(); j++) //j=0 is the final name of the cell
            {
                QString tmp = final_cellnamelist[i].at(j).trimmed();
                
                //          printf("tmp = %s\n", qPrintable(tmp));
                
                for (k=0; k<num_cell; k++)
                {
                    QString tmp2 = atlas_apo_combined[k].at(2).trimmed(); // at(2) is the name of the cell 
                    
                    //                printf("tmp2 = %s\n", qPrintable(tmp2));
                    if (tmp.compare(tmp2)==0)
                    {
                        //					printf("%d, ", k);
                        //					printf("%s, %s\n", qPrintable(tmp), qPrintable(tmp2));
                        merged_cell_idx.push_back(k);
                        observed_final[i] += observed[k];
                        expected_final[i] += expected[k];
                        
                        for (m=0; m<markerName[k].size(); m++)
                            thisFinalCellMarker.push_back(markerName[k].at(m));                        
                    
                        break;
                        
                    }
                    
                }
                //			printf("\n");
            } // for j end
            
            // compute markerTag_final
            
            //        for (p = 0; p<markercnt; p++)
            //        {
            //            int cnt = i*markercnt + p;
            //            markerTag_final[cnt] = markerTag[cnt] || markerTag[k*markercnt + p];
            //            printf("%d, ", markerTag_final[cnt]);
            //        }
            //        
            //        printf("\n");
            
            
            for (j=0; j<thisFinalCellMarker.size(); j++)
            {
                for (m=0; m<markercnt; m++)
                {
                    if (thisFinalCellMarker.at(j).compare(allMarkerSet.at(m))==0)
                    {
                        int cnt = i*markercnt + m;
                        markerTag_final[cnt] = 1;
                        break;
                    }
                    
                }
            }
            
            
            //--------------------------------------------------
            // compute cellMean_final and cellMean_vol_final
            //--------------------------------------------------
            
            
            for (p=0; p<merged_cell_idx.size(); p++)
            {		
                
                //            printf("%s\n", atlas_apo_combined[merged_cell_idx.at(p)].at(5).toStdString().c_str());
                //            printf("%s\n", atlas_apo_combined[merged_cell_idx.at(p)].at(6).toStdString().c_str());
                //            printf("%s\n", atlas_apo_combined[merged_cell_idx.at(p)].at(4).toStdString().c_str());
                //            printf("%s\n", atlas_apo_combined[merged_cell_idx.at(p)].at(10).toStdString().c_str());
                //            
                //            printf("%d\n", observed[merged_cell_idx.at(p)]);
                
                
                cellMean_final[i*3] += (atof(atlas_apo_combined[merged_cell_idx.at(p)].at(5).toStdString().c_str())*observed[merged_cell_idx.at(p)]);
                cellMean_final[i*3+1] += (atof(atlas_apo_combined[merged_cell_idx.at(p)].at(6).toStdString().c_str())*observed[merged_cell_idx.at(p)]); 
                cellMean_final[i*3+2] += (atof(atlas_apo_combined[merged_cell_idx.at(p)].at(4).toStdString().c_str())*observed[merged_cell_idx.at(p)]); 
                
                cellMean_vol_final[i] += (atof(atlas_apo_combined[merged_cell_idx.at(p)].at(10).toStdString().c_str())*observed[merged_cell_idx.at(p)]);
                
                //  			observed_final[i] += observed[merged_cell_idx.at(p)];
            }// for p end
            
            if (observed_final[i]>0)
            {
                
                int nn = i*3;
                
                for (k=0; k<3; k++)
                {
                    cellMean_final[nn+k] /= observed_final[i];	
                    
                    //				printf("%f, ", cellMean_final[nn+k]);
                }
                //			printf("\n");
                
                cellMean_vol_final[i] /= observed_final[i];
            }
            
        } // for i end
        
        
        
        
        // ------------------------------------------------------------------------
        // save the gene marker file
        // ------------------------------------------------------------------------	
        
        
        // write gene expression map
        
//        string file_genemap = dir_output.toStdString() + "genemap.txt";        
        string file_genemap = dir_output.toStdString() + markermap_file_name.toStdString();
        
        printf("%s\n", file_genemap.c_str());
        
        FILE *filetmp = fopen(file_genemap.c_str(), "wt"); // file to save gene expression map
        
        
        // write file head
        printf("%d\n", markercnt);
        
        
        for (j=0; j<markercnt; j++)
        {
            printf("marker = %s\n", allMarkerSet.at(j).c_str());
            
            fprintf(filetmp, "%s, ", allMarkerSet.at(j).c_str());
            printf("****\n");
            
        }
        
        fprintf(filetmp, "\n");
        
        // write the matrix
        for (i=0; i<final_cellnum;i++)
        {
            //			printf("%d, %s\n", i, qPrintable(final_cellnamelist[i].at(0)));
            
            fprintf(filetmp, "%s: ", qPrintable(final_cellnamelist[i].at(0)));
            int nn= i*markercnt; 
            for (j=0; j<markercnt; j++)
                fprintf(filetmp, "%d,", markerTag_final[nn+j]);
            fprintf(filetmp, "\n");
        }
        
        fclose(filetmp);
        
        
        // ------------------------------------------------------------------------
        // save the atlas file
        // ------------------------------------------------------------------------	
        
        string doutput = dir_output.toStdString();
//        QString cellstat_file_surfix = ;
//        QString merged_atlas_prefix;
        
        saveAtlasAfterMerge(cellMean_final, cellMean_vol_final, observed_final, expected_final, markerTag_final,
                            final_cellnum, 
                            final_cellnamelist, 
                            markerName, 
                            allMarkerSet, 
                            cellTypeNamelist, 
                            cellTypeNames, 
                            doutput,
                            cellstat_file_surfix,
                            merged_atlas_prefix);
        
    }
    
    //clean variables
Label_exit_merge_atlas:
    
    if (observed) {delete []observed; observed=0;}
    if (expected) {delete []expected; expected=0;}
    
    if (observed_final) {delete []observed_final; observed_final=0;}
    if (expected_final) {delete []expected_final; expected_final=0;}
    if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
    
    if (cellMean) {delete []cellMean; cellMean = 0;}
    //    if (cellStd) {delete []cellStd; cellStd = 0;}
    
    if (cellMean_final) {delete []cellMean_final; cellMean_final = 0;}
    //    if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
    
    if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
    //    if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
    
    if (num_each_apo) {delete []num_each_apo; num_each_apo = 0;}
    
    if (already_accessed_tag) {delete []already_accessed_tag; already_accessed_tag = 0;}
    
}


// use by the final interface
// save the atlas after merging co-localized cells
void saveAtlasAfterMerge(float *cellMean_final, float *cellMean_vol_final, int *observed_final, int *expected_final, 
                         unsigned char *markerTag_final, int final_cellnum, vector <QStringList> final_cellnamelist, 
                         vector <vector <string> > markerName, vector <string> allMarkerSet, 
                         vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames, string dfile_outputdir,
                         QString cellstat_file_surfix,QString merged_atlas_prefix)			  
{
	// ----------------------------------------------------------------------------------
	// save mean and std of each cell, which is the basic inforamtion of the atlas
    // do not save std info
	// ----------------------------------------------------------------------------------	
	string outfilename;
	int i,j,k,m;
    
    int markercnt = allMarkerSet.size();
    
	FILE *file = 0;
    
//    outfilename = dfile_outputdir + "atlas_all_final.apo_cellStatistics.txt";
    outfilename = dfile_outputdir + merged_atlas_prefix.toStdString() + ".apo_" + cellstat_file_surfix.toStdString();
    
	file = fopen(outfilename.c_str(), "wt");	
	
	fprintf(file, "#cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t cellmean_vol,\t observed, \t expected, \t ratio\n");
    
    
	for (j=0; j<final_cellnum; j++)
	{
        //		int tmp2 = j*3;
		
        
		float ratio = (float)observed_final[j]/(float)expected_final[j];
        int nn = j*3;
		
        fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", 
                qPrintable(final_cellnamelist[j].at(0)), cellMean_final[nn], cellMean_final[nn+1], cellMean_final[nn+2], cellMean_vol_final[j],  
                observed_final[j], expected_final[j], ratio);
        
	}
	
	fclose(file);
	
    printf("save 1 done\n");
    
    // -------------------------------
    // save statistics of each marker
    // -------------------------------
    
	
	for (j=0; j<allMarkerSet.size(); j++)
	{
		string thismarkername;
		thismarkername = allMarkerSet.at(j);
		
//        outfilename = dfile_outputdir + "cellStatistics_" + thismarkername + "_final.txt";

//        outfilename = dfile_outputdir + "atlas_" + thismarkername + "_final.apo_cellStatistics.txt";
        outfilename = dfile_outputdir + "atlas_" + thismarkername + "_merged.apo_" + cellstat_file_surfix.toStdString();

		
		file = fopen(outfilename.c_str(), "wt");
        
        fprintf(file, "#cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t observed,\t expected,\t ratio\n");
        
        
		for (i=0; i<final_cellnum; i++)
		{
            
            float ratio = (float)observed_final[i]/(float)expected_final[i];
            
			if (markerTag_final[i*markercnt+j] == 1) 		
                
			{
				int tmp2 = i*3;
                
                fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", 
                        qPrintable(final_cellnamelist[i].at(0)), 
                        cellMean_final[tmp2], cellMean_final[tmp2+1], cellMean_final[tmp2+2], cellMean_vol_final[i], 
                        observed_final[i], expected_final[i], ratio);
                
			}
		}
		fclose(file);
	}
    
    printf("save 2 done\n");
    
	// ----------------------------------------------------
	// generate .apo file (including the reference marker)
	// ----------------------------------------------------
    
    
	float pixmax = 100.0, intensity = 100.0, sdev = 100.0, mass = 100.0;
	float volsize;
	
	// overall apo containing all markers
//    outfilename = dfile_outputdir + "atlas_all_final.apo";
    
    outfilename = dfile_outputdir + merged_atlas_prefix.toStdString() + ".apo";

    
	file = fopen(outfilename.c_str(), "wt");	
	
	for (i=0; i<final_cellnum; i++)
	{
        
        
        
		int nn = i*3;
		if (!((cellMean_final[nn]==0)&&(cellMean_final[nn+1]==0)&&(cellMean_final[nn+2]==0))) 		
		{
			volsize = cellMean_vol_final[i];
            
//            for (j=0; j<markerName[i].size(); j++)
//                printf("%s,", markerName[i].at(j).c_str());
//            printf("\n");
            
			string markernamelist = "<$";
            
            int tt = i*allMarkerSet.size();
            unsigned char accessflg = 0;
            
            for (j=0; j<allMarkerSet.size(); j++)
            {
                if (markerTag_final[tt+j] == 1)
                { 
                    if (accessflg ==0)
                    {
                        markernamelist.append(allMarkerSet.at(j));
                        accessflg = 1;
                    }
                    else
                        markernamelist.append("_").append(allMarkerSet.at(j));
                }

            }
            
//            for (j=1; j<markerName[i].size(); j++)
//                markernamelist.append("_$").append(markerName[i].at(j));
            
            markernamelist.append(">");
            
			// write the cell into the atlas
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,qPrintable(final_cellnamelist[i].at(0)), markernamelist.c_str(), int(cellMean_final[nn+2]+0.5), int(cellMean_final[nn]+0.5), int(cellMean_final[nn+1]+0.5),
					pixmax, intensity, sdev, volsize, mass);
		}
	}
	fclose(file);
    
    
    printf("save 3 done\n");
    
	//---------------------------------------
	// write apo file for each marker
	//---------------------------------------
	
	for (j=0; j<allMarkerSet.size(); j++)
	{
		string markername;
		markername = allMarkerSet.at(j);
		
//        outfilename = dfile_outputdir + "atlas_" + markername + "_final.apo";
        outfilename = dfile_outputdir + "atlas_" + markername + "_merged.apo";
		
		file = fopen(outfilename.c_str(), "wt");
        
#ifdef DEBUG
		printf("%s, %d\n", outfilename.c_str(), marker_gene_info.markernum());	
#endif
        
		for (i=0; i<final_cellnum; i++)
		{
			if (markerTag_final[i*markercnt+j] == 1) 
                
			{
				int nn = i*3;
				volsize = cellMean_vol_final[i];
                
				// write the cell into the atlas
				string markernamelist = "<$";
				markernamelist.append(allMarkerSet.at(j)).append(">");
                
				fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
						i,i,qPrintable(final_cellnamelist[i].at(0)), markernamelist.c_str(), int(cellMean_final[nn+2]+0.5), int(cellMean_final[nn]+0.5), int(cellMean_final[nn+1]+0.5),
						pixmax, intensity, sdev, volsize, mass);
			}
		}
		fclose(file);
	}
	
    printf("save 4 done\n");
	
	//--------------------------------------------
	// write apo file for different cell types
	//--------------------------------------------
	
	
    for (i=0; i<cellTypeNames.size(); i++) // different cell types
    {
        
//        outfilename = dfile_outputdir + "atlas_" + cellTypeNames.at(i) + "_final.apo";
        outfilename = dfile_outputdir + "atlas_" + cellTypeNames.at(i) + "_merged.apo"; // note that cell type name file should be: cellname_type_secretory.txt, cannot be cellname_type_secretory_oct10.txt, because the program tries to extract the string between the last '_' and '.', if _oct10 is added, then CellTypeNames will all be 'oct10', this is a bug, should be changed to allow the flexibility
        
        file = fopen(outfilename.c_str(), "wt");
        
        for (j=0; j<final_cellnum; j++)
        {
            bool foundflg = 0;
            
            for (k=0; k<cellTypeNamelist[i].size(); k++)
            {
                
                if (final_cellnamelist[j].at(0).compare((QString)(cellTypeNamelist[i].at(k).c_str()))==0)
                {
                    foundflg = 1;
                    break;
                }
            }
            
            if (foundflg==1) 		
            {
                int nn = j*3;
                volsize = cellMean_vol_final[j];
                
                string markernamelist = "<$";
                
                
//                markernamelist.append(markerName[j].at(0));
//                
//                for (k=1; k<markerName[j].size(); k++)
//                    markernamelist.append("_$").append(markerName[j].at(k));

                int tt = j*allMarkerSet.size();
                unsigned char accessflg = 0;
                
                for (m=0; m<allMarkerSet.size(); m++)
                {
                    if (markerTag_final[tt+m] == 1)
                    { 
                        if (accessflg ==0)
                        {
                            markernamelist.append(allMarkerSet.at(m));
                            accessflg = 1;
                        }
                        else
                            markernamelist.append("_").append(allMarkerSet.at(m));
                    }
                    
                }
                
                
                markernamelist.append(">");
                
                // write the cell into the atlas
                fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
                        i,i,qPrintable(final_cellnamelist[j].at(0)), markernamelist.c_str(), int(cellMean_final[nn+2]+0.5), int(cellMean_final[nn]+0.5), int(cellMean_final[nn+1]+0.5),
                        pixmax, intensity, sdev, volsize, mass);
                
                
            }
        }
        
        fclose(file);
        
    }
    
    printf("save 5 done\n");
    
	
}













// *********************************************************************
// codes used in early version of command-line program debug
// *********************************************************************


//detect outlier cells and save in a txt file 
void outlierDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, bool *markerTag, 
                     float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, 
					 int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list, int *ref_gene_idx)
{
	
	int i,j,k,m,n;
	string outfilename = dfile_outputdir+"outlier.txt";
	FILE *file = fopen(outfilename.c_str(), "wt");
	MarkerStacks markerstack;
	
	// compute the mean and std of cellStd
	float mean_cellStd[3], std_cellStd[3];

	for (i=0; i<3; i++)
	{
		mean_cellStd[i] = 0;
		std_cellStd[i] = 0;
		
		int cnt = 0;
		for (j=0; j<dim_marker_gene[1]; j++)
		{
			
			if (!((cellStd[j*3]==0)&&(cellStd[j*3+1]==0)&&(cellStd[j*3+2]==0))) // otherwise, only one cell is annotated, do not use those to supress std_cellStd
			{
//				printf("%f, %f, %f\n", cellStd[j*3], cellStd[j*3+1], cellStd[j*3+2]);
			
				mean_cellStd[i] += cellStd[j*3+i];
				cnt++;
			}
		}
		
//		mean_cellStd[i] /=dim_marker_gene[1];
		mean_cellStd[i] /=cnt;
//		printf("%f, ", mean_cellStd[i]);
		
		for (j=0; j<dim_marker_gene[1]; j++)
		{	
			if (!((cellStd[j*3]==0)&&(cellStd[j*3+1]==0)&&(cellStd[j*3+2]==0))) // otherwise, only one cell is annotated, do not use those to supress std_cellStd
				std_cellStd[i] += pow((cellStd[j*3+i]-mean_cellStd[i]),2);
		}
		
//		std_cellStd[i]  = sqrt(std_cellStd[i])/dim_marker_gene[1];
		std_cellStd[i]  = sqrt(std_cellStd[i]/cnt);

		printf("%f, ", std_cellStd[i]);
	}
	
	printf("\n");
	
	// find cells that fall out of  2*std_cellStd

	for (j=0; j<dim_marker_gene[1]; j++)
	{
		for (i=0; i<3; i++)
		{
			if (fabs(cellStd[j*3+i]-mean_cellStd[i])>2*std_cellStd[i]) // an outlier
			{
				
				int stacknum = 0;
				
				// determine which stack falls out of 2*cellStd[j] in signal channels
				for (m=0; m<marker_gene_info.markernum(); m++)
				{

					markerstack = marker_gene_info.marker_stacks.at(m);
					int markerstacknum = (markerstack.signal_ch_filename).size();
					
					if (markerTag[j*(marker_gene_info.markernum()+1)+m+1]==1)
					{
						for (n=0; n<markerstacknum; n++)
						{
							int p = (n+stacknum)*3*dim_marker_gene[1]+3*j;
							
//							if (p==67983)
//								printf("%f, %f, %f\n", markerGeneCellArrayPos[p], markerGeneCellArrayPos[p+1], markerGeneCellArrayPos[p+2]);
							
							if (!((markerGeneCellArrayPos[p]==0)&&(markerGeneCellArrayPos[p+1]==0)&&(markerGeneCellArrayPos[p+2]==0))) // only consider cells annotated
							{
							
								if (fabs(markerGeneCellArrayPos[p+i]-cellMean[3*j+i])>2*cellStd[j*3+i])
								{
									fprintf(file, "%s: ", cellnamelist.at(j).c_str());
									switch (i)
									{
										case 0:
											fprintf(file, "x\n");
											break;
										case 1:
											fprintf(file, "y\n");
											break;					
										case 2:
											fprintf(file, "z\n");
											break;
									}
									
									fprintf(file, "    %s (mean = %f, stdev = %f, pos = %f)\n", markerstack.signal_ch_filename.at(n).c_str(), cellMean[3*j+i], cellStd[j*3+i], markerGeneCellArrayPos[p+i]);
								}
							}
						
						}
					}
					
//					printf("j=%d, i=%d, m=%d, %d, %d\n", j,i,m,markerstacknum, stacknum);
					
					stacknum += markerstacknum;
				}
				
				// determine which stack falls out of 2*cellStd[j] in reference channel
								
				if (markerTag[j*(marker_gene_info.markernum()+1)]==1) // cell express reference gene
				{
					for (m=0; m<reference_stack_list.size(); m++)
					{
						int p = m*3*dim_ref_gene[1]+3*ref_gene_idx[j];
						
						if (!((refGeneCellArrayPos[p]==0)&&(refGeneCellArrayPos[p+1]==0)&&(refGeneCellArrayPos[p+2]==0))) // only consider cells annotated
						{
						
							if (fabs(refGeneCellArrayPos[p+i]-cellMean[3*j+i])>2*cellStd[j*3+i])
								fprintf(file, "    %s (mean = %f, stdev = %f, pos = %f)\n", reference_stack_list.at(m).c_str(), cellMean[3*j+i], cellStd[j*3+i], refGeneCellArrayPos[p+i]);
						}
						
					}
				}
								
			}
		}
	}
	
	fclose(file);
}


// merge cells based on co-expression of cells to generate the final atlas. It fullfils:
// 1. replace cell names
// 2. generate new statistics of cells in termns of mean and stdev of cell locations OR volumes

//keep this function. This is used to generate the merged atlas with stdev values for Ellie to check
// merge_atlas is used for other labs to genreate the final merged atlas, consequntially, stdev are not saved

void mergeCell(float *markerGeneCellArray, float *refGeneCellArray, int *ref_gene_idx, int *dim_marker_gene, int *dim_ref_gene, 
			   vector <QStringList> final_cellnamelist, int final_cellnum, vector <string> cellnamelist, int *observed, int *expected, bool *markerTag, int markercnt,
			   float *&cellMean_final, float *&cellStd_final, int *&observed_final, int *&expected_final, bool *&markerTag_final)
{
	int i,j,k,p;
	
	//	int fieldnum = 3+FIELDNUM;	
	int fieldnum = dim_marker_gene[0];		
	int annocellnum = dim_marker_gene[1]; // note that the number of cells in markerGeneCellArray equals the total number of cells in the atlas (including all the cells in the reference marker)
	int marker_stacknum = dim_marker_gene[2];

	int ref_annocellnum = dim_ref_gene[1];	
	int ref_stacknum = dim_ref_gene[2];
	
	observed_final = new int [final_cellnum]; //how many times a given cell is observed or annotated	
	expected_final = new int [final_cellnum];
	
	markerTag_final = new bool [final_cellnum*markercnt];
	
	int len_final = final_cellnum*fieldnum; 
	cellMean_final = new float [len_final];
	cellStd_final = new float [len_final];
	
	
	// initialize cellMean and cellStd
	for (i=0; i<len_final; i++)
	{
		cellMean_final[i] = 0;
		cellStd_final[i] = 0;
	}
	
	for (i=0; i<final_cellnum*markercnt; i++)
	{
		markerTag_final[i] = 0;
	}
	
//	vecor <int> *merged_cell_idx = new vector <int> [final_cellnum];
	
	
	
	for (i=0; i<final_cellnum; i++)
	{
		
//		printf("i=%d\n", i);
//		if (i==666)
//			printf("i=%d, %s\n", i, qPrintable(final_cellnamelist[i].at(0)));
		
		int ss = i*fieldnum;

		//------------------------------------------
		// compute merged_cell_idx and observed_final
		//------------------------------------------
		vector <int> merged_cell_idx; // indicate which cell should be merged;
		
		observed_final[i] = 0;
		expected_final[i] = 0;
		
		// parse which cells co-express therefore should be merged
		printf("%d\n", final_cellnamelist.size());
		printf("size = %d\n", final_cellnamelist[i].size());
		
		for (j=1; j<final_cellnamelist[i].size(); j++)
		{
			QString tmp = final_cellnamelist[i].at(j);
			
			for (k=0; k<annocellnum; k++)
			{
//				QString tmp2 = (QString)cellnamelist.at(k);
				QString tmp2 = cellnamelist.at(k).c_str();
				
				if (tmp.compare(tmp2)==0)
				{
//					printf("%d, ", k);
					printf("%s, %s\n", qPrintable(tmp), qPrintable(tmp2));
					merged_cell_idx.push_back(k);
					observed_final[i] += observed[k];
					expected_final[i] += expected[k];
					
					for (p = 0; p<markercnt; p++)
					{
						int cnt = i*markercnt + p;
						markerTag_final[cnt] = markerTag_final[cnt] || markerTag[k*markercnt + p];
						printf("%d, ", markerTag_final[cnt]);
					}
					
					printf("\n");
					
					break;
					
				}
					
			}
//			printf("\n");
		} // for j end
		
		
		//--------------------------
		// compute cellMean_final
		//--------------------------
			
		
		for (p=0; p<merged_cell_idx.size(); p++)
		{		
		
			int nn = merged_cell_idx.at(p)*fieldnum;
			
			for (j=0; j<marker_stacknum; j++)
			{
				int mm = j*annocellnum*fieldnum + nn;

				bool existflag = 0;
				for (int pp = 0; pp<fieldnum; pp++)
				{
					if (markerGeneCellArray[mm+pp]!=0)
					{
						existflag = 1;
						break;
					}
				}
				
				if (existflag == 1)	
				{
					for (k=0; k<fieldnum; k++)
						cellMean_final[ss+k] += markerGeneCellArray[mm+k];
				}	
					
				
//				if (!((markerGeneCellArray[mm]==0)&&(markerGeneCellArray[mm+1]==0)&&(markerGeneCellArray[mm+2]==0)))
//				{
//					for (k=0; k<fieldnum; k++)
//						cellMean_final[ss+k] += markerGeneCellArray[mm+k];
//				}
			}
				
			// for reference gene cells, need also to combine reference gene stacks
			
			int tt = merged_cell_idx.at(p);
			
			if (ref_gene_idx[tt]!=-1) // the cell also express reference gene, need also to combine reference gene stack for position statistics
			{
				int qq = ref_gene_idx[tt]*fieldnum;
				
				for (j=0; j<ref_stacknum; j++)
				{
					int mm = j*ref_annocellnum*fieldnum + qq;
					
					bool existflag = 0;
					for (int pp = 0; pp<fieldnum; pp++)
					{
						if (refGeneCellArray[mm+pp]!=0)
						{
							existflag = 1;
							break;
						}
					}
					
					if (existflag == 1)	
					{
						for (k=0; k<fieldnum; k++)
							cellMean_final[ss+k] += refGeneCellArray[mm+k];
					}
						
						
//					if (!((refGeneCellArray[mm]==0)&&(refGeneCellArray[mm+1]==0)&&(refGeneCellArray[mm+2]==0)))
//					{
//						for (k=0; k<fieldnum; k++)
//							cellMean_final[ss+k] += refGeneCellArray[mm+k];
//					}
				}		
			}
			
		}// for p end
		
		if (observed_final[i]>0)
		{
			int nn = i*fieldnum;
			
			for (k=0; k<fieldnum; k++)
			{
				cellMean_final[nn+k] /= observed_final[i];			
				printf("%f, ", cellMean_final[nn+k]);
			}
			printf("\n");
		}
	
		//--------------------------
		// compute cellStd_final
		//--------------------------
		for (p=0; p<merged_cell_idx.size(); p++)
		{
			
			int nn = merged_cell_idx.at(p)*fieldnum;
			
			for (j=0; j<marker_stacknum; j++)
			{
				int mm = j*annocellnum*fieldnum + nn;

				bool existflag = 0;
				for (int pp = 0; pp<fieldnum; pp++)
				{
					if (markerGeneCellArray[mm+pp]!=0)
					{
						existflag = 1;
						break;
					}
				}
				
				if (existflag == 1)	
				{
					for (k=0; k<fieldnum; k++)
						cellStd_final[ss+k] += pow((markerGeneCellArray[mm+k]-cellMean_final[ss+k]),2);
				}
				
//				if (!((markerGeneCellArray[mm]==0)&&(markerGeneCellArray[mm+1]==0)&&(markerGeneCellArray[mm+2]==0)))
//				{
//					for (k=0; k<fieldnum; k++)
//						cellStd_final[ss+k] += pow((markerGeneCellArray[mm+k]-cellMean_final[ss+k]),2);
//				}
			}
			
			//		if (i<ref_annocellnum) // for reference gene cells, need also to combine reference gene stacks
			int tt = merged_cell_idx.at(p);
			
			if (ref_gene_idx[tt]!=-1) // the cell also express reference gene, need also to combine reference gene stack for position statistics
			{
				
				int qq = ref_gene_idx[tt]*fieldnum;
				
				for (j=0; j<ref_stacknum; j++)
				{
					int mm = j*ref_annocellnum*fieldnum + qq;
					
					bool existflag = 0;
					for (int pp = 0; pp<fieldnum; pp++)
					{
						if (refGeneCellArray[mm+pp]!=0)
						{
							existflag = 1;
							break;
						}
					}
					
					if (existflag == 1)	
					{
						for (k=0; k<fieldnum; k++)
							cellStd_final[ss+k] += pow((refGeneCellArray[mm+k]-cellMean_final[ss+k]),2);
					}
						
						
//					if (!((refGeneCellArray[mm]==0)&&(refGeneCellArray[mm+1]==0)&&(refGeneCellArray[mm+2]==0)))
//					{
//						for (k=0; k<fieldnum; k++)
//							cellStd_final[ss+k] += pow((refGeneCellArray[mm+k]-cellMean_final[ss+k]),2);
//					}
				}		
			}
		} // for p end
			
		if (observed_final[i]>0)
		{
			int nn = i*fieldnum;
			
			for (k=0; k<fieldnum; k++)
				cellStd_final[nn+k] = sqrt(cellStd_final[nn+k]/observed_final[i]);
			
		}
		
			
	} // for i end
	
//	if merged_cell_idx {delete []merged_cell_idx; merged_cell_idx =0;}
	return;
}



// detect co-localized cells based on 7 rules:
//1. A cell and a candidate co-localized cell must appear in more than a given threshold (0.33 in our case) of the stacks annotated. 
//2. If two cells express the same marker, they cannot be co-localized.
//3. If test cell name ends in L, candidate cell must end in L (Left/Right rule)
//4. If test cell name does not end in L or R, candidate cell must not end in L or R (unpaired midline rule)
//5. Cells with distance more than 6 pixels cannot be co-localized (distance rule)
//6. If the test cell is OBSERVED in 4-6 hemisegs, the candidate cell must satisfy other co-localization rules in 2
//   or more hemisegments; if the test cell is OBSERVED in 2-3 hemisegs, the candidate cell must satisfy other co-localization rules
//   in 2 or more hemisegments. If candidate cell is observed in 1 hemiseg, then exclude it from colocalization list.
//7. Candidates with 2/2, 2/4, 2/6 ratios (i.e., observed hemisegs/ expected hemisegs) must have both positives in the same seg to pass (bilateral rule).

void coLocalizationDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, bool *markerTag, 
							 float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellvol,
							 int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, int *ref_gene_idx, float threratio, int *observed, int *expected)
{
	
	long int i,j, k,m, n;
	int markernum = marker_gene_info.markernum()+1;
	
	int collocalCellNum = -1;	
	//	float thredis = 1000; // distance threshold which determines potential cell colocalization
	float thredis = 6; // distance threshold which determines potential cell colocalization
	int topn = 8; // only consider the topn candidates
	
	int *colocal_candidate = new int [dim_marker_gene[1]]; 	
	float *celldis = new float [dim_marker_gene[1]*dim_marker_gene[1]];
	
	float *sortidx2 = new float [dim_marker_gene[1]+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *sortval2 = new float [dim_marker_gene[1]+1];
	
	// read in ground-truth colocalization and adjacent cells
	unsigned char *groundtruth = new unsigned char [dim_marker_gene[1]*dim_marker_gene[1]];
	char tmps[1024];
	string cell_line;
	size_t pos_blank;
	
	// -----------------------------
	// load ground truth file
	//------------------------------
	ifstream ifs ("/Users/longf/work/fly_oregan/data/final_data/co_localization_ground_truth/coloc_cell_new_list.txt", ifstream::in); // hard code the path
	
	long linecount = 0;
	
	while (!ifs.eof())
	{
		ifs.getline(tmps, 1024);
		cell_line = tmps;
		int s_start = 0;
		int s_length = 0;
		
		// parse the information in each line, get the two strings in each line
		
		string itemstr1, itemstr2;
		
		
		for (i=0; i<cell_line.size(); i++)
		{
			if (cell_line.at(i)==',') // the first item
			{
				// process the current item
				itemstr1 = cell_line.substr(s_start, s_length);
				s_start = s_start+s_length+1; 
				s_length = 0;
			}
			else if (i==(cell_line.size()-1)) // the second item
			{
				s_length++;
				itemstr2 = cell_line.substr(s_start, s_length);
			}
			else
				s_length++;
		}
		
		printf("linecnt = %d, itemstr1 = %s, itemstr2 = %s\n", linecount, itemstr1.c_str(), itemstr2.c_str());
		
		if ((itemstr1.compare("")==0) || (itemstr2.compare("")==0))
			break;
		
		linecount++;	
		
		// search full names of cells, assign values to the grountruth matrix
		
		unsigned char item = 0;
		
		// test if the current cell belongs to those annotated
		for (m=0; m<dim_marker_gene[1]; m++)
		{
			//			printf("%s\n",cellnamelist.at(i).c_str());

			// before 20110722
//			pos_blank = cellnamelist.at(m).find(" ");
//			string cellname_prefix = cellnamelist.at(m).substr(0, pos_blank);
//			string cellname_surfix = cellnamelist.at(m).substr(pos_blank+1, cellnamelist.at(m).length()-1);

			
			// 20110722
						
			string cellname_prefix;
			trimStringSpace(cellnamelist.at(m).substr(0, cellnamelist.at(m).length()-3), cellname_prefix);
			string cellname_surfix = cellnamelist.at(m).substr(cellnamelist.at(m).length()-3,3);
			
			if (itemstr1.compare(cellname_prefix)==0)
			{
				string searchstr = itemstr2 + " " + cellname_surfix;
				for (n=0; n<dim_marker_gene[1]; n++)
				{
					//					printf("%s, %s\n", searchstr.c_str(), cellnamelist.at(n).c_str());
//					if (searchstr.compare(cellnamelist.at(n))==0)
					
					string str1, str2;
					trimStringSpace(searchstr, str1);
					trimStringSpace(cellnamelist.at(n), str2);
					
					if (str1.compare(str2)==0) //20110722
					
					{
						//						printf("%s, %s, %s\n", itemstr1.c_str(), searchstr.c_str(), cellnamelist.at(n).c_str());						
						//						printf("m=%d, n=%d\n", m,n);
						groundtruth[m*dim_marker_gene[1]+n] = 1; //colocalized cells
						break;
					}
				}
				
			}		
		}
	} // while end
	
	ifs.close();
	
	
	// -----------------------------------------	 
	// compute pair-wise distance between cells
	// -----------------------------------------	 
	
	string filename = dfile_outputdir + "colocalizedCells.txt";
	printf("%s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(),"wt");
	
	int fpos=0, fneg=0, tpos=0, tneg=0; // number of false positive, false negative, true positive, true negative
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))&&(((float)observed[i]/(float)expected[i])>threratio)) 		
		{
			for (j=0; j<dim_marker_gene[1]; j++)
			{
				
				int mm = j*3;
				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))&&(((float)observed[j]/(float)expected[j])>threratio)) 		
					celldis[i*dim_marker_gene[1]+j] = sqrt((cellMean[i*3]-cellMean[j*3])*(cellMean[i*3]-cellMean[j*3]) 
														   + (cellMean[i*3+1]-cellMean[j*3+1])*(cellMean[i*3+1]-cellMean[j*3+1]) 
														   + (cellMean[i*3+2]-cellMean[j*3+2])*(cellMean[i*3+2]-cellMean[j*3+2]));
				else
					celldis[i*dim_marker_gene[1]+j] = 9999;
				
			}
		}
		else
			for (j=0; j<dim_marker_gene[1]; j++)
			{
				celldis[i*dim_marker_gene[1]+j] = 9999;
			}
	}
	
	
	// ----------------------------------------------------
	// find for each cell, the cells that share the prefix
	// this is to generate the ratio value (see below)
	// ----------------------------------------------------
	short int *cell_same_prefix_idx = new short int [dim_marker_gene[1]*6];
	
	for (i=0; i< dim_marker_gene[1]; i++)
	{	
		long ii = i*3;
		
		// initialize
		for (j=0; j<6; j++)
		{
			//			printf("%d, %d\n", i,j);
			cell_same_prefix_idx[i*6+j] = -1;
		}
		
		if (!((cellMean[ii]==0)&&(cellMean[ii+1]==0)&&(cellMean[ii+2]==0)))
		{
			
			// keep these two lines
//			size_t pos_blankk = cellnamelist.at(i).find(" ");
//			string cellname_prefixx = cellnamelist.at(i).substr(0, pos_blankk);

			//20110722
			string cellname_prefixx;
			trimStringSpace(cellnamelist.at(i).substr(0, cellnamelist.at(i).length()-3), cellname_prefixx);
			
			//	unsigned char num1 = 0;
			
			cell_same_prefix_idx[i*6] = i;
			//		printf("%s, ", cellnamelist.at(cell_same_prefix_idx[i*6]).c_str());
			
			unsigned char cc = 0;
			
			for (j=0; j<dim_marker_gene[1]; j++)
			{
				if (j!=i)
				{
//					size_t pos_blankk2 = cellnamelist.at(j).find(" ");
//					string tmp = cellnamelist.at(j).substr(0, pos_blankk2);

				  //20110722
					string tmp;
					trimStringSpace(cellnamelist.at(j).substr(0, cellnamelist.at(j).length()-3), tmp);
													  
					long jj = j*3;
//					if ((cellname_prefixx.compare(tmp)==0) && (!((cellMean[jj]==0)&&(cellMean[jj+1]==0)&&(cellMean[jj+2]==0))))
					if ((cellname_prefixx.compare(tmp)==0) && (!((cellMean[jj]==0)&&(cellMean[jj+1]==0)&&(cellMean[jj+2]==0))))
											   
					{
						//				num1 += 1;					
						cc++;				
						cell_same_prefix_idx[i*6 + cc] = j;
						//					printf("%s, ", cellnamelist.at(cell_same_prefix_idx[i*6+cc]).c_str());
						
					}
				}
				
			}
		}
		//		printf("\n");
	}
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// distance and L/R rules
	// ---------------------------------------------------------------------
	
//	unsigned char *prediction = new unsigned char [dim_marker_gene[1]*dim_marker_gene[1]];
	bool *prediction = new bool [dim_marker_gene[1]*dim_marker_gene[1]];
	bool *prediction_topn = new bool [dim_marker_gene[1]*topn];
	
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		int nn = i*dim_marker_gene[1];
		//		int candcnt = 0;
		
		size_t len_i = cellnamelist.at(i).size();
		string surfix_i = cellnamelist.at(i).substr(len_i-2, len_i-1);
		
		// select those whose mean distance with respect to cell i is less than thredis
		for (j=0; j<dim_marker_gene[1]; j++)
		{
			
			k = 0;
			int n2 = 0;
			
			// if cell i and cell j express the same marker, they can't be potentially the same co-localized cell, since two names are given to them in that marker stacks 
			while (k<markernum)
			{ 
				if (!((markerTag[i*markernum+k]==1)&&(markerTag[j*markernum+k]==1)))
					k++;
				else
					break;
			}
			
			if (k>=markernum) // cell i and cell j do not express the same marker gene
			{
				size_t len_j = cellnamelist.at(j).size();
				string surfix_j = cellnamelist.at(j).substr(len_j-2, len_j-1);
				
				if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
				    || ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
				{
					if ((celldis[nn+j]<thredis)&&(j!=i)) // distance rule
						prediction[i*dim_marker_gene[1]+j] = 1; // this value is temporary, will be changed by other rules
					else
						prediction[i*dim_marker_gene[1]+j]=0; 
				}
			}
		} // for j end
	} // for i end
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// other rules
	// ---------------------------------------------------------------------
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		
		int candcnt = 0;
	
		// sort each cell based on its distance to the cell in consideration
		
		sortval2[0] = -999; // the 0th element is not sorted in sort2			
		sortidx2[0] = -999;
		
		for (j=0; j<dim_marker_gene[1]; j++)
		{	
			if (j==i)
				sortval2[j+1] = 10000; // do not consider the cell itself
			else
				sortval2[j+1] = celldis[i*dim_marker_gene[1]+j];
		
			sortidx2[j+1] = j;
		}
		
		sort2(dim_marker_gene[1], sortval2, sortidx2);// the first element is not sorted
		
		// get the topn candidates in terms of distance
		// initialize predication_topn		
		long tmp1 = i*topn;
		long tmp2 = i*dim_marker_gene[1];
		
		for (j=0; j<topn; j++)
		{
			colocal_candidate[j] = sortidx2[j+1]; 
			prediction_topn[tmp1+j] = prediction[tmp2+colocal_candidate[j]];
		};
		
		//	unsigned char expectednum_i = 0;
		unsigned char realnum_i = 0;
		
		for (m=0; m<6; m++) 
		{
			short int idx1 = cell_same_prefix_idx[i*6+m];
			if (idx1>=0)
				realnum_i++;
		}
		
//		fprintf(file, "cellname: %s, Candidate:        , meanx=%f, meany=%f, meanz=%f, stdx=%f, stdy=%f, stdz=%f, size=%f, obs_stack=%d, exp_stack=%d, obs_hemiseg=%d\n", 
//				cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2], cellvol[i], observed[i], expected[i], realnum_i);

		fprintf(file, "cellname: %s, Candidate:        , meanx=%f, meany=%f, meanz=%f, obs_stack=%d, exp_stack=%d, obs_hemiseg=%d\n", 
				cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], observed[i], expected[i], realnum_i);
		
		if (realnum_i>0)
		{
			
			for (j=0; j<topn; j++) 
			{
				//	printf("%d, %d\n", j, candcnt);
				
				// determine prediction ratio
				unsigned char expectednum_j = 0;
				unsigned char realnum_j = 0;
				float totaldistance = 0;
				
				unsigned char cnt_T2 = 0, cnt_T3 = 0, cnt_A1 = 0; // number of co-localized cells in each segment
				for (m=0; m<6; m++) 
				{
					short int idx1 = cell_same_prefix_idx[i*6+m];
					
					if (idx1>0)
					{
//						size_t pos_blank1= cellnamelist.at(idx1).find(" ");					
//						//						string cellname_sufix1 = cellnamelist.at(idx1).substr(0, pos_blank1);
//						string cellname_sufix1 = cellnamelist.at(idx1).substr(pos_blank1+1, cellnamelist.at(idx1).length());

											   
					   //20110722
					   string cellname_surfix1 = cellnamelist.at(idx1).substr(cellnamelist.at(idx1).length()-3, 3);
											   
						for (n=0; n<6; n++)
						{
							short int idx2 = cell_same_prefix_idx[colocal_candidate[j]*6+n];
							
							if (idx2>0)
							{
//								size_t pos_blank2= cellnamelist.at(idx2).find(" ");					
//								//								string cellname_sufix2 = cellnamelist.at(idx2).substr(0, pos_blank2);
//								string cellname_sufix2 = cellnamelist.at(idx2).substr(pos_blank2+1, cellnamelist.at(idx2).length());

								//20110722
							   string cellname_surfix2 = cellnamelist.at(idx2).substr(cellnamelist.at(idx2).length()-3, 3);
											   
								if (cellname_surfix1.compare(cellname_surfix2)==0)
								{
									expectednum_j++; // number of colocalization in all the 6 hemisegments
									if (prediction[idx1*dim_marker_gene[1]+idx2]==1)
									{
										realnum_j++; // the predicted number of colocalization in all the 6 
										totaldistance += celldis[idx1*dim_marker_gene[1]+idx2];
										
										//											if(cellname_sufix2.substr(0,2).compare(toupper("A1"))==0)
										if(cellname_surfix2.substr(0,2).compare("A1")==0)											
											cnt_A1++;
										
										if(cellname_surfix2.substr(0,2).compare("T2")==0)
											cnt_T2++;
										
										if(cellname_surfix2.substr(0,2).compare("T3")==0)
											cnt_T3++;
										
									}
									
								}
							}
							
						}
					}
				} //for m end
				
				// Chris rule 3, 4
				if (prediction[i*dim_marker_gene[1]+colocal_candidate[j]]==1)
				{
//					if ((realnum_i>=4)&&(realnum_j<3)) // old rule
					if ((realnum_i>=4)&&(realnum_j<2))
					{
//						prediction[i*dim_marker_gene[1]+colocal_candidate[j]] = 0;
//						prediction[colocal_candidate[j]*dim_marker_gene[1]+i] = 0; // symmetrical principle
						prediction_topn[tmp1+j] = 0;
						prediction_topn[j*topn+i] = 0; // symmetrical principle
						
					}
						
					
					if (((realnum_i==2)||(realnum_i==3))&&(realnum_j<2))
					{
//						prediction[i*dim_marker_gene[1]+colocal_candidate[j]] = 0;
//						prediction[colocal_candidate[j]*dim_marker_gene[1]+i] = 0; // symmetrical principle
						prediction_topn[tmp1+j] = 0;
						prediction_topn[j*topn+i] = 0; // symmetrical principle
					}
						
					
					if (realnum_j==1)
					{
//						prediction[i*dim_marker_gene[1]+colocal_candidate[j]] = 0;
//						prediction[colocal_candidate[j]*dim_marker_gene[1]+i] = 0; // symmetrical principle
						prediction_topn[tmp1+j] = 0;
						prediction_topn[j*topn+i] = 0; // symmetrical principle
						
					}
						
					
//					//bilateral rule
//					if (((expectednum_j==2)||(expectednum_j==4)||(expectednum_j==6))&&(realnum_j==2)&&((cnt_T3!=2)&&(cnt_T2!=2)&(cnt_A1!=2)))
//					{
//						printf("%s, %s, expectednum_j=%d, realnum_j=%d, cnt_T3=%d, cnt_T2=%d, cnt_A1=%d\n", cellnamelist.at(i).c_str(), cellnamelist.at(colocal_candidate[j]).c_str(), expectednum_j, realnum_j, cnt_T3, cnt_T2, cnt_A1);
////						prediction[i*dim_marker_gene[1]+colocal_candidate[j]] = 0;
////						prediction[colocal_candidate[j]*dim_marker_gene[1]+i] = 0; // symmetrical principle
//						prediction_topn[tmp1+j] = 0;
//						prediction_topn[j*topn+i] = 0; // symmetrical principle
//						
//					}
//
				}
				
				string tag;
				
				
				//				fprintf(file, "Candidate: %s,\t mean_x=%f,\t mean_y=%f,\t mean_z=%f,\t std_x=%f,\t std_y=%f,\t std_z=%f, \t distance=%f, \t size=%f, \t %d/%d, \t observed=%d,\t expected=%d,\t type=%s\n", 
				//						cellnamelist.at(colocal_candidate[j]).c_str(),  
				//						cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
				//						cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2], sortval[colocal_candidate[j]], 
				//						cellvol[colocal_candidate[j]], realnum_j, expectednum_j, observed[colocal_candidate[j]], expected[colocal_candidate[j]],tag.c_str()); 
				
				//					if (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==1) 
				//						tag = "***POSITIVE***";
				//					else
				//						tag = "";
				//							
				//					fprintf(file, "Candidate: %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d,  distance=%f, ratio=%d/%d, %s\n", 
				//							cellnamelist.at(colocal_candidate[j]).c_str(),  
				//							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
				//							cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2],  
				//							cellvol[colocal_candidate[j]], observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval[j], realnum_j, expectednum_j, tag.c_str()); 
				
				

				
////				if (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==1) 
//				if (prediction_topn[i*dim_marker_gene[1]+j]==1) 				
//				{
//					fprintf(file, "Candidate: %s, meanx=%f, meany=%f, meanz=%f, stdx=%f, stdy=%f, stdz=%f, size=%f, obs_stack=%d, exp_stack=%d,  distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, T2=%d, T3=%d, A1=%d, **POSITIVE**\n", 
//							cellnamelist.at(colocal_candidate[j]).c_str(),  
//							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//							cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2],  
//							cellvol[colocal_candidate[j]], observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1], realnum_j, expectednum_j, cnt_T2, cnt_T3, cnt_A1, tag.c_str()); 
//					
//				}
//				else
//				{
//					fprintf(file, "Candidate: %s, meanx=%f, meany=%f, meanz=%f, stdx=%f, stdy=%f, stdz=%f, size=%f, obs_stack=%d, exp_stack=%d, distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, T2=%d, T3=%d, A1=%d\n", 
//							cellnamelist.at(colocal_candidate[j]).c_str(),  
//							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//							cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2],  
//							cellvol[colocal_candidate[j]], observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1], realnum_j, expectednum_j, cnt_T2, cnt_T3, cnt_A1); 
//					
//				}
				
				if (prediction_topn[tmp1+j]==1) 				
				{
//					fprintf(file, "Cellname: %s, Candidate: %s, meanx=%f, meany=%f, meanz=%f, stdx=%f, stdy=%f, stdz=%f, size=%f, obs_stack=%d, exp_stack=%d,  distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, T2=%d, T3=%d, A1=%d, **POSITIVE**\n", 
//							cellnamelist.at(i).c_str(), cellnamelist.at(colocal_candidate[j]).c_str(), 
//							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//							cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2],  
//							cellvol[colocal_candidate[j]], observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1], realnum_j, expectednum_j, cnt_T2, cnt_T3, cnt_A1, tag.c_str()); 

					fprintf(file, "Cellname: %s, Candidate: %s, meanx=%f, meany=%f, meanz=%f, obs_stack=%d, exp_stack=%d,  distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, distance/(obs/exp)=%f, T2=%d, T3=%d, A1=%d, **POSITIVE**\n", 
							cellnamelist.at(i).c_str(), cellnamelist.at(colocal_candidate[j]).c_str(), 
							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
							observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1],  
							realnum_j, expectednum_j, sortval2[j+1]/((double)realnum_j/ (double)expectednum_j), cnt_T2, cnt_T3, cnt_A1, tag.c_str()); 
					
				}
				else
				{
//					fprintf(file, "Cellname: %s, Candidate: %s, meanx=%f, meany=%f, meanz=%f, stdx=%f, stdy=%f, stdz=%f, size=%f, obs_stack=%d, exp_stack=%d, distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, T2=%d, T3=%d, A1=%d\n", 
//							cellnamelist.at(i).c_str(), cellnamelist.at(colocal_candidate[j]).c_str(),  
//							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//							cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2],  
//							cellvol[colocal_candidate[j]], observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1], realnum_j, expectednum_j, cnt_T2, cnt_T3, cnt_A1); 

					fprintf(file, "Cellname: %s, Candidate: %s, meanx=%f, meany=%f, meanz=%f, obs_stack=%d, exp_stack=%d,  distance=%f, obs_coloc_hemiseg/exp_coloc_hemiseg=%d/%d, distance/(obs/exp)=%f, T2=%d, T3=%d, A1=%d\n", 
							cellnamelist.at(i).c_str(), cellnamelist.at(colocal_candidate[j]).c_str(), 
							cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
							observed[colocal_candidate[j]], expected[colocal_candidate[j]], sortval2[j+1], 
							realnum_j, expectednum_j, sortval2[j+1]/((double)realnum_j/ (double)expectednum_j), cnt_T2, cnt_T3, cnt_A1, tag.c_str()); 
					
				}				
				
			} // for j end
			
		} // if(realnum_i>0) end
		
		// print other top candidates that are close to the cell in consideration, but do not pass the rules
		
		fprintf(file, "\n\n");
		
		//----------------------------------------------
		// get the number of tpos, fpos, tneg, fneg
		//----------------------------------------------
		
		
		//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[j]]==1) & (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==1)) tag = "true pos";
		//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[j]]==1) & (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==0)) tag = "false neg";
		//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[j]]==0) & (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==1)) tag = "false pos";
		//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[j]]==0) & (prediction[i*dim_marker_gene[1] +colocal_candidate[j]]==0)) tag = "true neg";
		
		
	} // for i end
	
	
	fclose(file);
	
//	printf("true positive = %d, true negative = %d, false positive = %d, false negative = %d, score = %d\n", tpos, tneg, fpos, fneg, tpos+tneg-fpos-fneg);	
//	printf("Number of cells that can find potentially colocalized cells is: %d", collocalCellNum);
	
	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
	if (sortval2) {delete []sortval2; sortval2 = 0;}
	if (colocal_candidate) {delete []colocal_candidate; colocal_candidate = 0;}
	if (celldis) {delete []celldis; celldis = 0;}
	if (groundtruth) {delete []groundtruth; groundtruth = 0;}
	if (prediction) {delete []prediction; prediction = 0;}
	if (prediction_topn) {delete []prediction_topn; prediction_topn = 0;}	
	if (cell_same_prefix_idx) {delete []cell_same_prefix_idx; cell_same_prefix_idx = 0;}
	
	
}


// simplified co-localization detection for third party labs based on only 3 rules:
// 1. within one cell diameter; 
// 2. both cells can't be the same marker; 
// 3. both cells must be in the same hemisegment. 
// This function can only detect colocalization of two atlas apo files

void coLocalizationDetection(string atlas_subject_filename, string atlas_target_filename,  string dfile_outputdir)
{
	
	long int i,j, k,m, n, p, q;
	
	int collocalCellNum = -1;	
	float thredis = 6; // distance threshold which determines potential cell colocalization	
	
	// read in ground-truth colocalization and adjacent cells
	string cell_line;
	size_t pos_blank;
	
	
	// -----------------------------------------	 
	// load atlas files
	// -----------------------------------------
	vector <QStringList> atlas_subject;
	int num_subject = 0;
	
	vector <QStringList> atlas_target;
	int num_target = 0;
	
	readLines(atlas_subject_filename, atlas_subject, num_subject);
	readLines(atlas_target_filename, atlas_target, num_target);

#ifdef DEBUG	
	for (i=0; i<num_subject; i++)
	{
		for (j=0; j<atlas_subject[i].size(); j++)
			printf("%s, ", qPrintable(atlas_subject[i].at(j)));
		printf("\n");
		
	}
	
	for (i=0; i<num_target; i++)
	{
		for (j=0; j<atlas_target[i].size(); j++)
			printf("%s, ", qPrintable(atlas_target[i].at(j)));
		printf("\n");
		
	}	
	
	printf("%d, %d\n", num_subject, num_target);
	
#endif

	float *celldis = new float [num_subject*num_target];
	
	float *sortidx2 = new float [num_target+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *sortval2 = new float [num_target+1];
	
	// -------------------------
	// parse atlas files
	// -------------------------
	
	float *cellMean_subject = new float [num_subject*3];
	float *cellMean_target = new float [num_target*3];
	
//	<vector <string>> *markerName_subject = new vector <string> [num_subject];
//	vector <string> *markerName_target = new vector <string> [num_target];
	
	vector <vector <string> > markerName_subject;
	vector <vector <string> > markerName_target;
	
	
	// parse target atlas
	
	for (i=0; i<num_target; i++)
	{
		
		cellMean_target[i*3] = atof(qPrintable(atlas_target[i].at(5)));
		cellMean_target[i*3+1] = atof(qPrintable(atlas_target[i].at(6)));
		cellMean_target[i*3+2] = atof(qPrintable(atlas_target[i].at(4)));		
		
		
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_target[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring2; tmpstring2.clear();
		
		printf("length of tmpstring2 = %d\n", tmpstring2.size());
		
		while (j<atlas_target[i].at(3).size())
		{	
			
			if (atlas_target[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				
				tmpstring2.push_back(qPrintable(atlas_target[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				
				start_pos = j+2;				
				cnt++;
			}
			
			j++;
		}
		tmpstring2.push_back(qPrintable(atlas_target[i].at(3).mid(start_pos, end_pos-start_pos)));
		
		markerName_target.push_back(tmpstring2); 

#ifdef DEBUG		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, %d", i, markerName_target[i].at(j).c_str(),  markerName_target[i].size());
		
		printf("\n");
#endif				
		
	}
	
#ifdef DEBUG		
	
	for (i=0; i<num_target; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName_target[i].size());
		
		for (j=0; j< markerName_target[i].size(); j++)
		{
			
			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName_target[i].at(j).c_str());
			
		}
		printf("\n");
	}
#endif	
	
	
	// parse subject atlas
	for (i=0; i<num_subject; i++)
	{
		cellMean_subject[i*3] = atof(qPrintable(atlas_subject[i].at(5)));
		cellMean_subject[i*3+1] = atof(qPrintable(atlas_subject[i].at(6)));
		cellMean_subject[i*3+2] = atof(qPrintable(atlas_subject[i].at(4)));		
		
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_subject[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring;
		
		while (j<atlas_subject[i].at(3).size())
		{	
			printf("%s\n", qPrintable(atlas_subject[i].at(3)));
			if (atlas_subject[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				tmpstring.push_back(qPrintable(atlas_subject[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				start_pos = j+2;
				cnt++;

			}
			
			j++;
		}
		tmpstring.push_back(qPrintable(atlas_subject[i].at(3).mid(start_pos, end_pos-start_pos)));
		
		markerName_subject.push_back(tmpstring);

#ifdef DEBUG		
		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, %d", i, markerName_subject[i].at(j).c_str(), markerName_subject[i].size());
		printf("\n");
#endif
		
	}
	
#ifdef DEBUG		
	for (i=0; i<num_target; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName_target[i].size());
		
		for (j=0; j< markerName_target[i].size(); j++)
		{
			//			printf("%d, %d", i, j, markerName_target[i].at(j).c_str(),  markerName_target[i].size());
			
			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName_target[i].at(j).c_str());
			
		}
		printf("\n");
	}
	
	
	for (i=0; i<num_subject; i++)
	{
		printf("***** %d, %d, ", i, markerName_subject[i].size());
		
		for (j=0; j< markerName_subject[i].size(); j++)
		{
			//			printf("%d, %d", i, j, markerName_subject[i].at(j).c_str(),  markerName_subject[i].size());
			
			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName_subject[i].at(j).c_str());
			
		}
		printf("\n");
	}
#endif	
	
	
	// -----------------------------------------	 
	// compute pair-wise distance between cells
	// -----------------------------------------	 
	
	string filename = dfile_outputdir + "colocalizedCells.txt";
	printf("%s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(),"wt");
	
	
	for (i=0; i<num_subject; i++)
	{
		
		int nn = i*3;
		if (!((cellMean_subject[nn]==0)&&(cellMean_subject[nn+1]==0)&&(cellMean_subject[nn+2]==0))) 		
		{
			for (j=0; j<num_target; j++)
			{
				
				int mm = j*3;
				if (!((cellMean_target[mm]==0)&&(cellMean_target[mm+1]==0)&&(cellMean_target[mm+2]==0))) 		
				{
					celldis[i*num_target+j] = sqrt((cellMean_subject[nn]-cellMean_target[mm])*(cellMean_subject[nn]-cellMean_target[mm]) 
														   + (cellMean_subject[nn+1]-cellMean_target[mm+1])*(cellMean_subject[nn+1]-cellMean_target[mm+1]) 
														   + (cellMean_subject[nn+2]-cellMean_target[mm+2])*(cellMean_subject[nn+2]-cellMean_target[mm+2]));
					printf("%d, %d, %f\n", i, j, celldis[i*num_target+j]);
				}
				else
					celldis[i*num_target+j] = 9999;
				
			}
		}
		else
			for (j=0; j<num_target; j++)
			{
				celldis[i*num_target+j] = 9999;
			}
	}
	
		
	
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// distance and L/R rules
	// ---------------------------------------------------------------------
	
	bool *prediction = new bool [num_subject*num_target];
	bool shareMarkerTag = 0;
	
	
#ifdef DEBUG		
	
	for (i=0; i<num_target; i++)
	{
		for (j=0; j< markerName_target[i].size(); j++)
			printf("%d, %d, %s, %d", i, j, markerName_target[i].at(j).c_str(),  markerName_target[i].size());
	}
	
#endif
	
	for (i=0; i<num_subject; i++)
	{
		int nn = i*num_target;
		
		size_t len_i = atlas_subject[i].at(2).size(); // atlas_subject[i].at(2) is the name of the ith cell
//		string surfix_i = atlas_subject[i].at(2).substr(len_i-2, len_i-1);
		string surfix_i = qPrintable(atlas_subject[i].at(2).mid(len_i-1, 1)); // get the hemisegment name
		
		// select those whose mean distance with respect to cell i is less than thredis
		for (j=0; j<num_target; j++)
		{
			
			k = 0;
			int n2 = 0;
			
			shareMarkerTag = 0;
			prediction[nn+j] = 0;
			
				
			for (p=0; p<markerName_subject[i].size(); p++)
			{
				for (q=0; q<markerName_target[j].size(); q++)
				{
//					printf("%d, %d, %d, %d, %s, %s\n", i, j, p, q, markerName_subject[i].at(p).c_str(),  markerName_target[j].at(q).c_str());
					
					if (markerName_subject[i].at(p).compare(markerName_target[j].at(q))==0)
					{
						shareMarkerTag = 1;
						break;
					}
				} // for q end
				if (shareMarkerTag == 1)
					break;
			} // for p end
			
			if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene
			{
				size_t len_j = atlas_target[j].at(2).size();
//				string surfix_j = atlas_target[j].at(2).substr(len_j-2, len_j-1);
				string surfix_j = qPrintable(atlas_target[j].at(2).mid(len_j-1, 1));
//				printf("%s, %s\n", surfix_i.c_str(), surfix_j.c_str());
					   
					   
				if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
				    || ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
				{
					printf("%d, %d, %f, %s, %s\n", i,j,celldis[nn+j],surfix_i.c_str(), surfix_j.c_str());
					if (celldis[nn+j]<thredis) // distance rule
						prediction[nn+j] = 1; 
					else
						prediction[nn+j]=0; 
				}
			}
		} // for j end
	} // for i end
	
	
	// ---------------------------------------------------------------------
	// sort candidates
	// ---------------------------------------------------------------------
	
	for (i=0; i<num_subject; i++)
	{
		
		int candcnt = 0;
		
		// sort each cell based on its distance to the cell in consideration
		
		sortval2[0] = -999; // the 0th element is not sorted in sort2			
		sortidx2[0] = -999;
		
		for (j=0; j<num_target; j++)
		{	
			sortval2[j+1] = celldis[i*num_target+j];
			sortidx2[j+1] = j;
		}
		
		sort2(num_target, sortval2, sortidx2);// the first element is not sorted
		
		j = 0;
		while (sortval2[j+1] < thredis)
		{
			fprintf(file, "cellname: %s, Candidate:        , meanx=%f, meany=%f, meanz=%f\n", 
					qPrintable(atlas_target[i].at(1)), cellMean_target[(long)sortidx2[j+1]*3], cellMean_target[(long)sortidx2[j+1]*3+1], cellMean_target[(long)sortidx2[j+1]*3+2]);
			j++;
						
		};
				
		
	} // for i end
	
	fclose(file);
	
	// ---------------------------------------------------------------------
	// create .apo files for visualizing co-localization
	// ---------------------------------------------------------------------
	
	filename = atlas_subject_filename + ".coexpress.apo";
	file = fopen(filename.c_str(), "wt");

	for (i=0; i<num_subject; i++)
	{
		bool cotag = 0;
		
		for (j=0; j<num_target; j++)
		{
			if (prediction[i*num_target+j]==1)
			{
				cotag =1;
				break;
			}
		}
		
		if (cotag == 1)	
		{
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,255,255,255\n",
							i,i,qPrintable(atlas_subject[i].at(2)), qPrintable(atlas_subject[i].at(3)), atoi(qPrintable(atlas_subject[i].at(4))), atoi(qPrintable(atlas_subject[i].at(5))), atoi(qPrintable(atlas_subject[i].at(6))), 
							atof(qPrintable(atlas_subject[i].at(7))), atof(qPrintable(atlas_subject[i].at(8))), atof(qPrintable(atlas_subject[i].at(9))), atof(qPrintable(atlas_subject[i].at(10))));
		}
		else
		{
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,qPrintable(atlas_subject[i].at(2)), qPrintable(atlas_subject[i].at(3)), atoi(qPrintable(atlas_subject[i].at(4))), atoi(qPrintable(atlas_subject[i].at(5))), atoi(qPrintable(atlas_subject[i].at(6))), 
					atof(qPrintable(atlas_subject[i].at(7))), atof(qPrintable(atlas_subject[i].at(8))), atof(qPrintable(atlas_subject[i].at(9))), atof(qPrintable(atlas_subject[i].at(10))));
		}
		
		
	}
	
	fclose (file);
	
	
	filename = atlas_target_filename + ".coexpress.apo";
	file = fopen(filename.c_str(), "wt");
	
	for (i=0; i<num_target; i++)
	{
		bool cotag = 0;
		
		for (j=0; j<num_subject; j++)
		{
			if (prediction[j*num_target+i]==1)
			{
			
				cotag =1;
				break;
			}
		}
		
		if (cotag == 1)		
		{
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,0,0,0\n",
					i,i,qPrintable(atlas_target[i].at(2)), qPrintable(atlas_target[i].at(3)), atoi(qPrintable(atlas_target[i].at(4))), atoi(qPrintable(atlas_target[i].at(5))), atoi(qPrintable(atlas_target[i].at(6))), 
					atof(qPrintable(atlas_target[i].at(7))), atof(qPrintable(atlas_target[i].at(8))), atof(qPrintable(atlas_target[i].at(9))), atof(qPrintable(atlas_target[i].at(10))));
		}
		else
		{
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,qPrintable(atlas_target[i].at(2)), qPrintable(atlas_target[i].at(3)), atoi(qPrintable(atlas_target[i].at(4))), atoi(qPrintable(atlas_target[i].at(5))), atoi(qPrintable(atlas_target[i].at(6))), 
					atof(qPrintable(atlas_target[i].at(7))), atof(qPrintable(atlas_target[i].at(8))), atof(qPrintable(atlas_target[i].at(9))), atof(qPrintable(atlas_target[i].at(10))));
						
		}
		
		
	}
	
	fclose (file);
	
	
	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
	if (sortval2) {delete []sortval2; sortval2 = 0;}
	if (celldis) {delete []celldis; celldis = 0;}
	if (prediction) {delete []prediction; prediction = 0;}
	if (cellMean_subject) {delete []cellMean_subject; cellMean_subject =0;}
	if (cellMean_target) {delete []cellMean_target; cellMean_target =0;}
	
//	if (atlas_subject) {delete []atlas_subject; atlas_subject = 0;}
//	if (atlas_target) {delete []atlas_target; atlas_target = 0;}
//	if (markerName_subject) {delete []markerName_subject; markerName_subject =0;}
//	if (markerName_target) {delete []markerName_target; markerName_target=0;}
	
}


// simplified co-localization detection for third party labs based on only 3 rules:
// 1. within one cell diameter; 
// 2. both cells can't be the same marker; 
// 3. both cells must be in the same hemisegment. 
// This function can detect colocalization of any number of apo files

void coLocalizationDetection(string test_colocalization_atlas_apo_filename, string dfile_outputdir)
{
	
	long int i,j, k,m, n, p, q;
	
	int collocalCellNum = -1;	
//	float thredis = 6; // distance threshold which determines potential cell colocalization	
	float thredis = 10; // distance threshold which determines potential cell colocalization	

	// -------------------------------------------------
	// load  the file containing a list of apo files
	// -------------------------------------------------
	vector <string> atlas_apo_filelist;
	int num_apo_file = 0;
	
	readLines(test_colocalization_atlas_apo_filename, atlas_apo_filelist, num_apo_file);
	
//#ifdef DEBUG	
	for (i=0; i<num_apo_file; i++)
	{
		printf("%s\n", atlas_apo_filelist.at(i).c_str());		
	}
	
//#endif

	// ---------------------------------------------------------------------------------
	// read each the contents of each atlas apo, and concatenate them into one file
	// ---------------------------------------------------------------------------------
	
	vector <QStringList> atlas_apo_combined;
	int num_cell = 0;
	int *num_each_apo = new int [atlas_apo_filelist.size()];
	
	for (i=0; i<num_apo_file; i++)
	{
		vector <QStringList> tmp;
//		int num_item;
		
//		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_item);
		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_each_apo[i]);
		
//		for (j=0; j<num_item; j++)
		for (j=0; j<num_each_apo[i]; j++)		
			atlas_apo_combined.push_back(tmp[j]);
		
//		num_cell += num_item;
		num_cell += num_each_apo[i];
	}
	
	
//#ifdef DEBUG	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j<atlas_apo_combined[i].size(); j++)
			printf("%s, ", qPrintable(atlas_apo_combined[i].at(j)));		
		printf("\n");
	}
//#endif
	
	
	float *celldis = new float [num_cell*num_cell];
	
	float *sortidx2 = new float [num_cell+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *sortval2 = new float [num_cell+1];
	
	// -------------------------
	// parse atlas files
	// -------------------------
	
	float *cellMean = new float [num_cell*3];
	
	vector <vector <string> > markerName;
	
	
	// parse target atlas
	
	for (i=0; i<num_cell; i++)
	{
		
		cellMean[i*3] = atof(qPrintable(atlas_apo_combined[i].at(5)));
		cellMean[i*3+1] = atof(qPrintable(atlas_apo_combined[i].at(6)));
		cellMean[i*3+2] = atof(qPrintable(atlas_apo_combined[i].at(4)));		
		
		
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_apo_combined[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring2; tmpstring2.clear();
		
		printf("length of tmpstring2 = %d\n", tmpstring2.size());
		
		while (j<atlas_apo_combined[i].at(3).size())
		{	
			
			if (atlas_apo_combined[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				
				tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				
				start_pos = j+2;				
				cnt++;
			}
			
			j++;
		}
		tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos)));
		markerName.push_back(tmpstring2); 
		
#ifdef DEBUG		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, %d", i, markerName[i].at(j).c_str(),  markerName[i].size());
		
		printf("\n");
#endif				
		
	}
	
//#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName[i].size());
		
		for (j=0; j< markerName[i].size(); j++)
		{
			
			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName[i].at(j).c_str());
			
		}
		printf("\n");
	}
//#endif	
	
	
	
	// -----------------------------------------	 
	// compute pair-wise distance between cells
	// -----------------------------------------	 
	
	
	
	for (i=0; i<num_cell; i++)
	{
		
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))) 		
		{
			for (j=0; j<num_cell; j++)
			{
				int mm = j*3;
				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))) 		
				{
					celldis[i*num_cell+j] = sqrt((cellMean[nn]-cellMean[mm])*(cellMean[nn]-cellMean[mm]) 
												   + (cellMean[nn+1]-cellMean[mm+1])*(cellMean[nn+1]-cellMean[mm+1]) 
												   + (cellMean[nn+2]-cellMean[mm+2])*(cellMean[nn+2]-cellMean[mm+2]));
					printf("%d, %d, %f\n", i, j, celldis[i*num_cell+j]);
				}
				else
					celldis[i*num_cell+j] = 9999;
				
			}
		}
		else
			for (j=0; j<num_cell; j++)
			{
				celldis[i*num_cell+j] = 9999;
			}
	}
	
	
	
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// distance and L/R rules
	// ---------------------------------------------------------------------
	
	bool *prediction = new bool [num_cell*num_cell];
	bool shareMarkerTag = 0;
	
	
#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j< markerName[i].size(); j++)
			printf("%d, %d, %s, %d", i, j, markerName[i].at(j).c_str(),  markerName[i].size());
	}
	
#endif
	
	for (i=0; i<num_cell; i++)
	{
		int nn = i*num_cell;
		
		size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
		//		string surfix_i = atlas_apo_combined[i].at(2).substr(len_i-2, len_i-1);
		string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // get the hemisegment name
		
		// select those whose mean distance with respect to cell i is less than thredis
		for (j=0; j<num_cell; j++)
		{
			
			k = 0;
			int n2 = 0;
			
			shareMarkerTag = 0;
			prediction[nn+j] = 0;
			
			if (j!=i)
			{
			
				for (p=0; p<markerName[i].size(); p++)
				{
					for (q=0; q<markerName[j].size(); q++)
					{
						//					printf("%d, %d, %d, %d, %s, %s\n", i, j, p, q, markerName[i].at(p).c_str(),  markerName[j].at(q).c_str());
						
						if (markerName[i].at(p).compare(markerName[j].at(q))==0)
						{
							shareMarkerTag = 1;
							break;
						}
					} // for q end
					if (shareMarkerTag == 1)
						break;
				} // for p end
				
				if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene
				{
					size_t len_j = atlas_apo_combined[j].at(2).size();
					//				string surfix_j = atlas_apo_combined[j].at(2).substr(len_j-2, len_j-1);
					string surfix_j = qPrintable(atlas_apo_combined[j].at(2).mid(len_j-1, 1));
					//				printf("%s, %s\n", surfix_i.c_str(), surfix_j.c_str());
					
					
					if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
						|| ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
					{
						printf("%d, %d, %f, %s, %s\n", i,j,celldis[nn+j],surfix_i.c_str(), surfix_j.c_str());
						if (celldis[nn+j]<thredis) // distance rule
							prediction[nn+j] = 1; 
						else
							prediction[nn+j]=0; 
					}
				}
			} // if (j!=i) end
		} // for j end
	} // for i end
	
	
	// ---------------------------------------------------------------------
	// sort candidates
	// ---------------------------------------------------------------------
	
	string filename = dfile_outputdir + "coexpress.txt";
	printf("%s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(),"wt");
	
	filename = test_colocalization_atlas_apo_filename + ".coexpress.apo";
	FILE *file2 = fopen(filename.c_str(), "wt");

	bool *coexpress_flg = new bool [num_cell];
	
	for (i=0; i<num_cell; i++)
		 coexpress_flg[i] = 0;
	
	for (i=0; i<num_cell; i++)
	{
		
		int candcnt = 0;
		
		// sort each cell based on its distance to the cell in consideration
		
		sortval2[0] = -999; // the 0th element is not sorted in sort2			
		sortidx2[0] = -999;
		
		for (j=0; j<num_cell; j++)
		{	
			sortval2[j+1] = celldis[i*num_cell+j];
			sortidx2[j+1] = j;
		}
		
		sort2(num_cell, sortval2, sortidx2);// the first element is not sorted
		
		j = 0;
		bool print_tag = 0;
		bool cellname_printed_tag = 0;
		
		
		while (sortval2[j+1] < thredis)
		{
			if ((sortidx2[j+1]!=i) && (prediction[i*num_cell+(long)sortidx2[j+1]]==1))
			{
//				fprintf(file, "cellname: %s, Candidate: %s , meanx=%f, meany=%f, meanz=%f\n", 
//						qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);

				if (cellname_printed_tag == 0)
				{
					fprintf(file, "\ncellname: %s, meanx=%f, meany=%f, meanz=%f\n", 
							qPrintable(atlas_apo_combined[i].at(2)), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2]);
					
					cellname_printed_tag = 1;
				}
				
				fprintf(file, "Candidate:%s , meanx=%f, meany=%f, meanz=%f\n", 
						qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
				
				
				if (print_tag == 0)
				{
					print_tag = 1;
					coexpress_flg[i] = 1;
					coexpress_flg[(long)sortval2[j+1]] = 1;
				}
								
			}
			j++;
			
		}; // while (sortval2[j+1] < thredis) end
		
	} // for i end
	
	fclose(file);
	fclose (file2);
	
	// ---------------------------------------------------------------------
	// create .apo files for visualizing co-localization
	// ---------------------------------------------------------------------
	
	filename = test_colocalization_atlas_apo_filename + ".coexpress.apo";
	file = fopen(filename.c_str(), "wt");
	
//	for (i=0; i<num_cell; i++)
//	{
//		
//		if (coexpress_flg[i] == 1)	
//		{
//			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,255,255,255\n",
//					i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
//					atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))));
//		}
//		else
//		{
//			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
//					i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
//					atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))));
//		}
//		
//		
//	}

	unsigned char *mycolor = new unsigned char [num_cell*3]; // color of cells to be rendered

	unsigned char colorval[60] = {255, 0, 0, //red
		0, 255, 0, //green
		0, 0, 255, //blue
		255, 255, 0, //yellow
		255, 0, 255, //purple
		0, 255, 255, //cyan
		0, 0, 0, //black
		128, 128, 255, 
		128, 255, 128, 
		255, 128, 128, 
		128, 196, 0, 
		0, 128, 196,
		196, 0, 128,
		128, 0, 196,
		0, 196, 128,
		196, 128, 0,
		64, 128, 128,
		128, 64, 128,
		128, 128, 64,
	128, 196, 255};
	
	
	
	int num =0;
	for (i=0; i<atlas_apo_filelist.size(); i++)
	{
		
		for (j=0; j<num_each_apo[i]; j++)
		{
			mycolor[(num+j)*3] = colorval[i*3];
			mycolor[(num+j)*3+1] = colorval[i*3+1];
			mycolor[(num+j)*3+2] = colorval[i*3+2];
		}
		num += num_each_apo[i];
	}
				

	for (i=0; i<num_cell; i++)
	{
		if (coexpress_flg[i] == 1)	
		{
			mycolor[i*3] = mycolor[i*3+1] = mycolor[i*3+2] = 255;
		}
	}
	
	for (i=0; i<num_cell; i++)
	{
	
		fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,%d,%d,%d\n",
				i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
				atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))), mycolor[i*3], mycolor[i*3+1], mycolor[i*3+2]);
		
	}
	
	
	fclose (file);
	
	
	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
	if (sortval2) {delete []sortval2; sortval2 = 0;}
	if (celldis) {delete []celldis; celldis = 0;}
	if (prediction) {delete []prediction; prediction = 0;}
	if (cellMean) {delete []cellMean; cellMean =0;}
	if (coexpress_flg) {delete []coexpress_flg; coexpress_flg = 0;}
	
	if (num_each_apo) {delete []num_each_apo; num_each_apo=0;}
	if (mycolor) {delete []mycolor; mycolor = 0;}
	
	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
	//	if (markerName) {delete []markerName; markerName =0;}
	//	if (markerName) {delete []markerName; markerName=0;}
	
}


// void coLocalizationDetection(vector <string> atlas_apo_filelist, string dfile_output)
// {
// 	
//     
//     long int i,j, k,m, n, p, q;
//  	int collocalCellNum = -1;	
//     //	float thredis = 6; // distance threshold which determines potential cell colocalization	
// 	float thredis = 10; // distance threshold which determines potential cell colocalization	
//     
//    
// 	// ---------------------------------------------------------------------------------
// 	// read the contents of each atlas apo, and concatenate them into one file
// 	// ---------------------------------------------------------------------------------
// 	
// 	vector <QStringList> atlas_apo_combined;
// 	int num_cell = 0; // total number of cells when cancatenate all the apo files
//     int num_apo_file = atlas_apo_filelist.size(); // number of apo atlas files
// 	int *num_each_apo = new int [num_apo_file]; // number of cells in each apo file
// 	
// 	for (i=0; i<num_apo_file; i++)
// 	{
// 		vector <QStringList> tmp;
// 		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_each_apo[i]);
// 		
// 		for (j=0; j<num_each_apo[i]; j++)		
// 			atlas_apo_combined.push_back(tmp[j]);
// 		
// 		num_cell += num_each_apo[i];
// 	}
// 	
// 	
//     //#ifdef DEBUG	
// 	for (i=0; i<num_cell; i++)
// 	{
// 		for (j=0; j<atlas_apo_combined[i].size(); j++)
// 			printf("%s, ", qPrintable(atlas_apo_combined[i].at(j)));		
// 		printf("\n");
// 	}
//     //#endif
// 	
// 		
// 	// -------------------------
// 	// parse atlas files
// 	// -------------------------
// 	
// 	float *cellMean = new float [num_cell*3];
// 	
// 	vector <vector <string> > markerName;
// 	
// 	
// 	// get information of cell position and markers
// 	
// 	for (i=0; i<num_cell; i++)
// 	{
// 		
// 		cellMean[i*3] = atof(qPrintable(atlas_apo_combined[i].at(5)));
// 		cellMean[i*3+1] = atof(qPrintable(atlas_apo_combined[i].at(6)));
// 		cellMean[i*3+2] = atof(qPrintable(atlas_apo_combined[i].at(4)));		
// 		
// 		
// 		j=2; // j=0 and j=1 correspond to "<" and "$"
// 		int start_pos = j;
// 		int end_pos = atlas_apo_combined[i].at(3).size()-1;
// 		int cnt = 0;
// 		
// 		vector <string> tmpstring2; 
//         tmpstring2.clear();
// 		
// 		printf("length of tmpstring2 = %d\n", tmpstring2.size());
// 		
// 		while (j<atlas_apo_combined[i].at(3).size()) // at(3) saves marker information
// 		{	
// 			
// 			if (atlas_apo_combined[i].at(3).mid(j,1)=="_")			
// 			{
// 				end_pos = j-1;
// 				
// 				tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos+1)));
// 				
// 				start_pos = j+2;				
// 				cnt++;
// 			}
// 			
// 			j++;
// 		}
// 		tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos)));
// 		markerName.push_back(tmpstring2); 
// 		
// #ifdef DEBUG		
// 		for (j=0; j<=cnt; j++)
// 			printf("%d, %s, %d", i, markerName[i].at(j).c_str(),  markerName[i].size());
// 		
// 		printf("\n");
// #endif				
// 		
// 	}
// 	
//     //#ifdef DEBUG		
// 	
// 	for (i=0; i<num_cell; i++)
// 	{
// 		printf("^^^^^ %d, %d, ", i, markerName[i].size());
// 		
// 		for (j=0; j< markerName[i].size(); j++)
// 		{
// 			
// 			printf("%d, %d, ", i, j);			
// 			printf("%s, ", markerName[i].at(j).c_str());
// 			
// 		}
// 		printf("\n");
// 	}
//     //#endif	
// 	
// 	
// 	
// 	// -----------------------------------------	 
// 	// compute pair-wise distance between cells
// 	// -----------------------------------------	 
// 	
// 	float *celldis = new float [num_cell*num_cell];
// 	
// 	
// 	for (i=0; i<num_cell; i++)
// 	{
// 		
// 		int nn = i*3;
// 		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))) 		
// 		{
// 			for (j=0; j<num_cell; j++)
// 			{
// 				int mm = j*3;
// 				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))) 		
// 				{
// 					celldis[i*num_cell+j] = sqrt((cellMean[nn]-cellMean[mm])*(cellMean[nn]-cellMean[mm]) 
//                                                  + (cellMean[nn+1]-cellMean[mm+1])*(cellMean[nn+1]-cellMean[mm+1]) 
//                                                  + (cellMean[nn+2]-cellMean[mm+2])*(cellMean[nn+2]-cellMean[mm+2]));
// 					printf("%d, %d, %f\n", i, j, celldis[i*num_cell+j]);
// 				}
// 				else
// 					celldis[i*num_cell+j] = 9999;
// 				
// 			}
// 		}
// 		else
// 			for (j=0; j<num_cell; j++)
// 			{
// 				celldis[i*num_cell+j] = 9999;
// 			}
// 	}
// 	
// 	
// 	// ---------------------------------------------------------------------
// 	// compute prediction, i.e., canndidate co-localized cells, based on
// 	// distance and L/R rules
// 	// ---------------------------------------------------------------------
// 	
// 	bool *prediction = new bool [num_cell*num_cell];
// 	bool shareMarkerTag = 0;
// 	
// 	
// #ifdef DEBUG		
// 	
// 	for (i=0; i<num_cell-1; i++)
// 	{
// 		for (j=0; j< markerName[i].size(); j++)
// 			printf("%d, %d, %s, %d", i, j, markerName[i].at(j).c_str(),  markerName[i].size());
// 	}
// 	
// #endif
// 	
// 	for (i=0; i<num_cell; i++)
// 	{
// 		int nn = i*num_cell;
// 		
// 		size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
// 		//		string surfix_i = atlas_apo_combined[i].at(2).substr(len_i-2, len_i-1);
// 		string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // determine if it is right or left
// 		
// 		// select those whose mean distance with respect to cell i is less than thredis
// 		for (j=i+1; j<num_cell; j++)
// 		{
// 			
// 			k = 0;
// 			int n2 = 0;
// 			
// 			shareMarkerTag = 0;
// 			prediction[nn+j] = 0;
// 			
// //			if (j!=i)
// 			{
//                 
//                 // check if cell i and cell j share express the same marker gene
// 				for (p=0; p<markerName[i].size(); p++)
// 				{
// 					for (q=0; q<markerName[j].size(); q++)
// 					{
// 						//					printf("%d, %d, %d, %d, %s, %s\n", i, j, p, q, markerName[i].at(p).c_str(),  markerName[j].at(q).c_str());
// 						
// 						if (markerName[i].at(p).compare(markerName[j].at(q))==0)
// 						{
// 							shareMarkerTag = 1;
// 							break;
// 						}
// 					} // for q end
// 					if (shareMarkerTag == 1)
// 						break;
// 				} // for p end
// 				
// 				if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene
// 				{
//                     
//                     // check if cell i and cell j satisfy the Left/right rule
// 					size_t len_j = atlas_apo_combined[j].at(2).size();
// 					string surfix_j = qPrintable(atlas_apo_combined[j].at(2).mid(len_j-1, 1));
// 					
// 					
// 					if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
// 						|| ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
// 					{
// 						printf("%d, %d, %f, %s, %s\n", i,j,celldis[nn+j],surfix_i.c_str(), surfix_j.c_str());
// 						if (celldis[nn+j]<thredis) // distance rule
// 							prediction[nn+j] = 1; 
// 						else
// 							prediction[nn+j]=0; 
// 					}
// 				}
// 			} // if (j!=i) end
// 		} // for j end
// 	} // for i end
// 	
// 	// --------------------
// 	// check bilateral rule
// 	// --------------------
// 	
//     // find for each cell, its ipsilabtetal parterner
//     
//     QString search_str;
//     int *LR_parterner_idx = new int [num_cell];
//     
// 	for (i=0; i<num_cell; i++)
// 	{
// 	
//         LR_parterner_idx[i] = 0;
//         
// 		// determine the name of the ipsilabtetal parterner
//         
//         size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
// 		string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // tell if it is left or right
//         
//         search_str = "";
//         
//         if (surfix_i.compare('L')==0)
//             search_str = atlas_apo_combined[i].at(2).mid(0,len-i-1) + "R";
//         else if (surfix_i.compare('R')==0)
//             search_str = atlas_apo_combined[i].at(2).mid(0,len-i-1) + "L";
//         
//         // search for the ipsilater paterner
//         
//         for (j=0; j<num_cell; j++)
//         {
//             if (atlas_apo_combined[j].at(2).trimmed().compare(search_str.trimmed())==0)
//             {
//                 LR_parterner_idx[i] = j;
//                 break;
//             }
//         
//         }
//     }
//         
//       
//     //for each detect coexpression pair, check if its  ipsilateral parterners also coexpress, if not discard the pair as coexpression
//     
//     for (i=0; i<num_cell-1; i++)
// 	{
// 		
// 		int nn = i*num_cell;
//                         
// 		for (j=i+1; j<num_cell; j++)
// 		{
//             
//             if ((prediction[nn+j] == 1)&&(LR_parterner_idx[i]!=0)&&(LR_parterner_idx[j]!=0))
//             {
//                 int pp = LR_parterner_idx[i]*num_cell + LR_parterner_idx[j];
//                 int qq = LR_parterner_idx[j]*num_cell + LR_parterner_idx[i];
//                 
//                 if ((prediction[pp]==0)&&(prediction[qq]==0))
//                     prediction[nn+j] = 0;
//             }
//                                 
//         }
//     }
//         
//  
//         
// 	// ---------------------------------------------------------------------
// 	// sort candidates
// 	// ---------------------------------------------------------------------
// 	
// 	string filename = dfile_output + "colocalizedCells.txt";
// 	printf("%s\n", filename.c_str());
// 	FILE *file = fopen(filename.c_str(),"wt");
// 	
//     
//     ////	filename = test_colocalization_atlas_apo_filename + ".coexpress.apo";
//     //	filename = dfile_output + ".coexpress.apo";    
//     //	FILE *file2 = fopen(filename.c_str(), "wt");
//     
// 	bool *coexpress_flg = new bool [num_cell];
// 	
// 	for (i=0; i<num_cell; i++)
//         coexpress_flg[i] = 0;
// 	
//     float *sortidx2 = new float [num_cell+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
// 	float *sortval2 = new float [num_cell+1];
// 	
// 
// 	for (i=0; i<num_cell; i++)
// 	{
// 		
// 		int candcnt = 0;
// 		
// 		// sort each cell based on its distance to the cell in consideration
// 		
// 		sortval2[0] = -999; // the 0th element is not sorted in sort2			
// 		sortidx2[0] = -999;
// 		
// 		for (j=0; j<num_cell; j++)
// 		{	
// 			sortval2[j+1] = celldis[i*num_cell+j];
// 			sortidx2[j+1] = j;
// 		}
// 		
// 		sort2(num_cell, sortval2, sortidx2);// the first element is not sorted
// 		
// 		j = 0;
// 		bool print_tag = 0;
// 		bool cellname_printed_tag = 0;
// 		
// 		
// 		while (sortval2[j+1] < thredis)
// 		{
// 			if ((sortidx2[j+1]!=i) && (prediction[i*num_cell+(long)sortidx2[j+1]]==1))
// 			{
//                 //				fprintf(file, "cellname: %s, Candidate: %s , meanx=%f, meany=%f, meanz=%f\n", 
//                 //						qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
//                 
// 				if (cellname_printed_tag == 0)
// 				{
// 					fprintf(file, "\ncellname: %s, meanx=%f, meany=%f, meanz=%f\n", 
// 							qPrintable(atlas_apo_combined[i].at(2)), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2]);
// 					
// 					cellname_printed_tag = 1;
// 				}
// 				
// 				fprintf(file, "Candidate:%s , meanx=%f, meany=%f, meanz=%f\n", 
// 						qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
// 				
// 				
// 				if (print_tag == 0)
// 				{
// 					print_tag = 1;
// 					coexpress_flg[i] = 1;
// 					coexpress_flg[(long)sortval2[j+1]] = 1;
// 				}
//                 
// 			}
// 			j++;
// 			
// 		}; // while (sortval2[j+1] < thredis) end
// 		
// 	} // for i end
// 	
// 	fclose(file);
// 	fclose (file2);
// 	
// 	// ---------------------------------------------------------------------
// 	// create .apo files for visualizing co-localization
// 	// ---------------------------------------------------------------------
// 	
// 	filename = dfile_output + ".coexpress.apo";
// 	file = fopen(filename.c_str(), "wt");
// 	
//     
// 	unsigned char *mycolor = new unsigned char [num_cell*3]; // color of cells to be rendered
//     
// 	unsigned char colorval[60] = {255, 0, 0, //red
// 		0, 255, 0, //green
// 		0, 0, 255, //blue
// 		255, 255, 0, //yellow
// 		255, 0, 255, //purple
// 		0, 255, 255, //cyan
// 		0, 0, 0, //black
// 		128, 128, 255, 
// 		128, 255, 128, 
// 		255, 128, 128, 
// 		128, 196, 0, 
// 		0, 128, 196,
// 		196, 0, 128,
// 		128, 0, 196,
// 		0, 196, 128,
// 		196, 128, 0,
// 		64, 128, 128,
// 		128, 64, 128,
// 		128, 128, 64,
//         128, 196, 255};
// 	
// 	
// 	
// 	int num =0;
// 	for (i=0; i<atlas_apo_filelist.size(); i++)
// 	{
// 		
// 		for (j=0; j<num_each_apo[i]; j++)
// 		{
// 			mycolor[(num+j)*3] = colorval[i*3];
// 			mycolor[(num+j)*3+1] = colorval[i*3+1];
// 			mycolor[(num+j)*3+2] = colorval[i*3+2];
// 		}
// 		num += num_each_apo[i];
// 	}
//     
//     
// 	for (i=0; i<num_cell; i++)
// 	{
// 		if (coexpress_flg[i] == 1)	
// 		{
// 			mycolor[i*3] = mycolor[i*3+1] = mycolor[i*3+2] = 255;
// 		}
// 	}
// 	
// 	for (i=0; i<num_cell; i++)
// 	{
//         
// 		fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,%d,%d,%d\n",
// 				i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
// 				atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))), mycolor[i*3], mycolor[i*3+1], mycolor[i*3+2]);
// 		
// 	}
// 	
// 	
// 	fclose (file);
// 	
//     if (LR_parterner_idx) {delete []LR_parterner_idx; LR_parterner_idx=0;}
// 	
// 	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
// 	if (sortval2) {delete []sortval2; sortval2 = 0;}
// 	if (celldis) {delete []celldis; celldis = 0;}
// 	if (prediction) {delete []prediction; prediction = 0;}
// 	if (cellMean) {delete []cellMean; cellMean =0;}
// 	if (coexpress_flg) {delete []coexpress_flg; coexpress_flg = 0;}
// 	
// 	if (num_each_apo) {delete []num_each_apo; num_each_apo=0;}
// 	if (mycolor) {delete []mycolor; mycolor = 0;}
// 	
// 	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
// 	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
// 	//	if (markerName) {delete []markerName; markerName =0;}
// 	//	if (markerName) {delete []markerName; markerName=0;}
// 	
// }


// final_tag indicates it is to save the final version of the atlas (after combining co-express cells), note that stdev is not saved, as we don't want to share the annotation file for each stack

void saveAtlas(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, bool *markerTag, vector <string> forceAddCellnamelist, 
			   vector < vector <string> >cellTypeNamelist, vector <string> cellTypeNames,
			  float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellMean_vol, float *cellStd_vol, int *observed, int *expected, 
			  int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, vector <string> reference_stack_list,int *ref_gene_idx, float threratio, bool final_tag)			  
{
	// ----------------------------------------------------------------------------------
	// save mean and std of each cell, which is the basic inforamtion of the atlas
	// ----------------------------------------------------------------------------------	
	string outfilename;
	int i,j,k,m;
	FILE *file;

	if (final_tag == 0)
//		outfilename = dfile_outputdir + "cellMeanStd.txt";
		outfilename = dfile_outputdir + "cellStatistics.txt";
        
	else
//		outfilename = dfile_outputdir + "cellMeanStd_final.txt";
		outfilename = dfile_outputdir + "cellStatistics_final.txt";
		
	file = fopen(outfilename.c_str(), "wt");	
	
	fprintf(file, "cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t std_x,\t std_y,\t std_z,\t cellstd_vol,\t observed, \t expected, \t ratio\n");

	bool *forceAddTag = new bool [dim_marker_gene[1]];
	
	for (j=0; j<dim_marker_gene[1]; j++)
	{
		forceAddTag[j] = 0;
		for (k=0; k<forceAddCellnamelist.size(); k++)
			if (cellnamelist.at(j).compare(forceAddCellnamelist.at(k))==0)
			{
				forceAddTag[j] = 1;
//				printf("%d, %d\n", j, forceAddTag[j]);
				break;
			}
		
	}

	for (j=0; j<dim_marker_gene[1]; j++)
	{
		int tmp2 = j*3;
		
//		printf("observed [%d] = %d\n", j, observed[j]);

#ifdef DEBUG		
		if (observed[j]==0)
			printf("observed %d =0, %s\n", j, cellnamelist.at(j).c_str());
		
		if (((float)observed[j]/(float)expected[j])>1)
			printf("observed = %d, expected = %d, ratio = %f\n", observed[j], expected[j], ((float)observed[j]/(float)expected[j]));

		if ((float)observed[j]/(float)expected[j]>1)
		{
			printf("obsevered = %d, expected = %d\n", observed[j], expected[j]);
			printf("j=%d, %s, observed value is bigger than expected value\n", j, cellnamelist.at(j).c_str());
		}
		
#endif
	
		float ratio = (float)observed[j]/(float)expected[j];
		
		if ((((float)observed[j]/(float)expected[j])>=threratio)||(forceAddTag[j] == 1)) //20091124 add according to Chris
			fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", cellnamelist.at(j).c_str(), cellMean[tmp2], cellMean[tmp2+1], cellMean[tmp2+2], cellMean_vol[j], cellStd[tmp2], cellStd[tmp2+1], cellStd[tmp2+2], cellStd_vol[j], observed[j], expected[j], ratio);
			
	}
	
	fclose(file);
	
//	// ----------------------------------------------------------------------------------
//	// signify asymmetrical variaitons of cells
//	// ----------------------------------------------------------------------------------	
//	
////	bool *asymmetryflg = new bool [dim_marker_gene[1]];
//
//	if (final_tag == 0)
//		outfilename = dfile_outputdir + "cellMeanStd_asymmetrical.txt";
//	else
//		outfilename = dfile_outputdir + "cellMeanStd_asymmetrical_final.txt";
//		
//	file = fopen(outfilename.c_str(), "wt");
//	
//	fprintf(file, "cellname,\t mean_x,\t mean_y,\t mean_z,\t mean_vol,\t std_x,\t std_y,\t std_z,\t cellstd_vol,\t observed, \t expected, \t ratio\n");
//	
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//		int nn = i*3;
//		float minval = 9999;
//		
////		asymmetryflg[i] = 0;
//		bool asymmetryflg = 0;
//		
////		if (((float)observed[i]/(float)expected[i])>threratio) //20091124 add according to Chris
//		if ((((float)observed[i]/(float)expected[i])>=threratio)||(forceAddTag[i] == 1))		
//		{
//			for (j=0; j<3; j++)
//			{
//				if (cellStd[nn+j]<minval)
//					minval = cellStd[nn+j];
//			}
//			
//			for (j=0; j<3; j++)
//			{
//				if (cellStd[nn+j]>3*minval)
//					asymmetryflg = 1;
//			}
//			
//			if (asymmetryflg==1)
//					fprintf(file, "%s,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%f,\t%d,\t%d,\t%f\n", cellnamelist.at(i).c_str(), cellMean[nn], cellMean[nn+1], cellMean[nn+2], cellMean_vol[i], 
//							cellStd[nn], cellStd[nn+1], cellStd[nn+2], cellStd_vol[i], observed[i], expected[i], (float)observed[i]/(float)expected[i]);
//					
//	//				asymmetryflg[i] = 1;
//		}
//	}
//	
//	
//	
//	// ----------------------------------------------------------------
//	// compute outlier, pop up cells with big variations
//	// which cell, in which dimension, in which stack has big variation
//	// ----------------------------------------------------------------	
//	// save outlier cells (not for final atlas)
//	if (final_tag == 0)
//		outlierDetection(marker_gene_info, cellnamelist, markerTag, 
//                     markerGeneCellArrayPos, refGeneCellArrayPos, cellMean, cellStd, 
//					 dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list,ref_gene_idx);
	
	// ----------------------------------------------------
	// generate .apo file (including the reference marker)
	// ----------------------------------------------------


//	float pixmax = 100.0, intensity = 100.0, sdev = 100.0, volsize = 105.0, mass = 100.0;
	float pixmax = 100.0, intensity = 100.0, sdev = 100.0, mass = 100.0;
	float volsize;
	
	// overall apo containing all markers
	if (final_tag == 0)
		outfilename = dfile_outputdir + "atlas_all.apo";
	else
		outfilename = dfile_outputdir + "atlas_all_final.apo";
		
	file = fopen(outfilename.c_str(), "wt");	
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
	
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
		{
			volsize = cellMean_vol[i];
			string markernamelist = "<";
			bool filledTag = 0;
			
			if (markerTag[i*(marker_gene_info.markernum()+1)]==1) //the cell express the reference marker EVE
			{
				markernamelist.append("$").append(marker_gene_info.reference_markername.c_str());
				filledTag = 1; // for print format purpose
			}
			
//			printf("%d\n", marker_gene_info.markernum());
			for (j=0; j<marker_gene_info.markernum();j++)
			{
				if (markerTag[i*(marker_gene_info.markernum()+1)+j+1]==1) //the cell express that marker
				{
					if (filledTag == 1)
						markernamelist.append("_$").append(marker_gene_info.marker_stacks.at(j).markername);
					else
					{
						markernamelist.append("$").append(marker_gene_info.marker_stacks.at(j).markername);						
						filledTag = 1;
					}
//					printf("%s\n", markernamelist.c_str());
				}
			}
			
//			markernamelist += ">";	
			markernamelist.append(">");	
			
			// write the cell into the atlas
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
					pixmax, intensity, sdev, volsize, mass);
		}
	}
	fclose(file);

	//---------------------------------------
	// write apo file for each marker
	//---------------------------------------
	
	for (j=0; j<marker_gene_info.markernum(); j++)
	{
		string markername;
		markername = marker_gene_info.marker_stacks[j].markername;
		
		if (final_tag == 0)
			outfilename = dfile_outputdir + "atlas_" + markername + ".apo";
		else
			outfilename = dfile_outputdir + "atlas_" + markername + "_final.apo";
		
		file = fopen(outfilename.c_str(), "wt");
				
#ifdef DEBUG
		printf("%s, %d\n", outfilename.c_str(), marker_gene_info.markernum());	
#endif
				
		for (i=0; i<dim_marker_gene[1]; i++)
		{
//			if (markerTag[i*(marker_gene_info.markernum()+1)+(j+1)] == 1) //current cell expresses this marker
			if ((markerTag[i*(marker_gene_info.markernum()+1)+(j+1)] == 1)&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
			
			{
				int nn = i*3;
				volsize = cellMean_vol[i];
			
				// write the cell into the atlas
				string markernamelist = "<$";
				markernamelist.append(marker_gene_info.marker_stacks.at(j).markername).append(">");
//				printf("%s\n", markernamelist.c_str());
				fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
						i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
						pixmax, intensity, sdev, volsize, mass);
			}
		}
		fclose(file);
	}
	
	//--------------------------------------------
	// write apo file for reference gene (eve)
	//--------------------------------------------
	
	printf("%s\n", marker_gene_info.reference_markername.c_str());
	
	if (final_tag == 0)
		outfilename = dfile_outputdir + "atlas_"+marker_gene_info.reference_markername+".apo";
	else
		outfilename = dfile_outputdir + "atlas_"+marker_gene_info.reference_markername+"_final.apo";
		
	printf("%s\n", outfilename.c_str());
	
	file = fopen(outfilename.c_str(), "wt");
	printf("dim_marker_gene[1] = %d\n", dim_marker_gene[1]);
	
	for (i=0; i<dim_marker_gene[1]; i++)
	{
		
//		if (markerTag[i*(marker_gene_info.markernum()+1)] == 1) //current cell expresses this marker
		if ((markerTag[i*(marker_gene_info.markernum()+1)] == 1)&&((((float)observed[i]/(float)expected[i])>threratio)||(forceAddTag[i]==1))) //20091124 according to Chris		
		{
			int nn = i*3;
			volsize = cellMean_vol[i];
			
			// write the cell into the atlas
//			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
//					i,i,cellnamelist.at(i).c_str(),"<$EVE>", int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
//					pixmax, intensity, sdev, volsize, mass);

			string markernamelist = "<$";
			markernamelist.append(marker_gene_info.reference_markername.c_str()).append(">");
			
			fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
					i,i,cellnamelist.at(i).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
					pixmax, intensity, sdev, volsize, mass);
			
		}
	}
	fclose(file);

//	printf("\n");
	
	
	//--------------------------------------------
	// write apo file for different cell types
	//--------------------------------------------
	
	
	if (final_tag == 1)
	{
		for (i=0; i<cellTypeNames.size(); i++) // different cell types
		{
			
			outfilename = dfile_outputdir + "atlas_" + cellTypeNames.at(i) + ".apo";
			file = fopen(outfilename.c_str(), "wt");
			
			for (j=0; j<dim_marker_gene[1]; j++)
			{
				bool foundflg = 0;
				for (k=0; k<cellTypeNamelist[i].size(); k++)
				{
					if (cellnamelist.at(j).compare(cellTypeNamelist[i].at(k))==0)
					{
						foundflg = 1;
						break;
					}
				}
				
				if (((((float)observed[j]/(float)expected[j])>threratio)||(forceAddTag[j]==1))&&(foundflg==1)) //20091124 according to Chris		
				{
					int nn = j*3;
					volsize = cellMean_vol[j];
									
					string markernamelist = "<$";
	
					if (markerTag[j*(marker_gene_info.markernum()+1)] == 1) // eve
						markernamelist.append("EVE");
					
					 for (m=0; m<marker_gene_info.markernum(); m++) // other markers
					 {
						 if (markerTag[j*(marker_gene_info.markernum()+1)+m+1] == 1)
							 markernamelist.append(marker_gene_info.marker_stacks[m].markername.c_str()).append("_");
					 }
					
					markernamelist.append(">");
					
					fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
							j,j,cellnamelist.at(j).c_str(), markernamelist.c_str(), int(cellMean[nn+2]+0.5), int(cellMean[nn]+0.5), int(cellMean[nn+1]+0.5),
							pixmax, intensity, sdev, volsize, mass);
					
				}
			}
			
			fclose(file);
			
		}
	}
	
	
	if (forceAddTag) {delete []forceAddTag, forceAddTag = 0;}
}


//rebuild the atlas, if there are old markers in the atlas, the corresponding stacks needs to re_registered, target known for registration

//void rebuildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, int *ref_gene_idx, string dfile_outputdir,
//				 bool *&markerTag, float *&markerGeneCellArrayPos, float *&markerGeneCellArrayOtherFields, float *&refGeneCellArrayPos, float *&refGeneCellArrayOtherFields, int *&marker_ref_map,
//				 int &stacknum, int &refstacknum, vector <string> &reference_stack_list,
//				 float *&cellMean, float *&cellStd, float *&cellMean_vol, float *&cellStd_vol)

void rebuildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				  vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 
				  int *ref_gene_idx, string dfile_outputdir, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum, string atlas_subject_filename, string atlas_target_filename)
{
    
	int i,j,k,m;
	FILE *file; 
	string outfilename;
	int fieldnum = FIELDNUM + 3;
	
	// -----------------------------------------------------------
	// generate matrices for registration and statistical analysis
	// -----------------------------------------------------------	
	bool *markerTag;	
	float *markerGeneCellArrayPos = 0; //marker gene arrary containing only (x,y,z), for registration	
	float *markerGeneCellArrayOtherFields = 0;	//marker gene arrary containing intesity and volume information	
	float *refGeneCellArrayPos = 0; //reference gene (e.g., EVE) arrary containing only (x,y,z) 
	float *refGeneCptCellArrayPos = 0; // reference gene array containing only control points for registration, note refGeneCptCellArrayPos cab be a subset of refGeneCptCellArrayPos
	float *refGeneCellArrayOtherFields = 0; //the reference gene (e.g., EVE) arrary containing intensity and volume information
	int *marker_ref_map =0; // a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
	int stacknum, refstacknum;
	vector <string> reference_stack_list;
    
	// for debug purpose
#ifdef DEBUG
	
	MarkerStacks markerstack;
	int cnt = 0;
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.reference_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			printf("i=%d, j=%d, cnt=%d, stack= %s, ref= %s\n", i, j, cnt, markerstack.signal_ch_filename.at(j).c_str(), markerstack.reference_ch_filename.at(j).c_str());
			cnt++;
		}
	}
#endif
	
    //	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, 
    //				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  
    //				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
    
	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist,
				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  refGeneCptCellArrayPos, 
				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
    
	// --------------
	// registration
	// --------------	
	
	// read cell position in target file
	float *targetCellArrayPos = 0; // all annotated cell array					
	float *targetCptCellArrayPos =0; // control point (annotated) array
	float *targetCellArrayOtherFields = 0; 
	
	printf("target file = %s\n", dfile_target.c_str());
	
	
    //	getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	getAtlasCell(dfile_target, cellnamelist, ref_gene_controlpoints_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	
	// targetCellArrayPos and targetCellArrayOtherFields are no use
	if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
	if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}
	
	// do registration
	int dim_marker_gene[3], dim_ref_gene[3], dim_ref_gene_cpt[3];
	
	dim_marker_gene[0] = 3;
	dim_marker_gene[1] = cellnamelist.size();
	dim_marker_gene[2] = stacknum;
	
	dim_ref_gene[0] = 3;
	dim_ref_gene[1] = ref_gene_cellnamelist.size();
	dim_ref_gene[2] = refstacknum;
	
	dim_ref_gene_cpt[0] = 3;
	dim_ref_gene_cpt[1] = ref_gene_controlpoints_cellnamelist.size();
	dim_ref_gene_cpt[2] = refstacknum;
	
	float *refGeneCellArrayPosNew=0, *markerGeneCellArrayPosNew=0;
    
	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
    
	
	// ------------------------------------------------
	// save data before registration 
	// ------------------------------------------------
	FILE *regfile1, *regfile2;
	string registeredfilename1 = dfile_outputdir + "before_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	string registeredfilename2 = dfile_outputdir + "before_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
    
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPos[qq]==0)&(markerGeneCellArrayPos[qq+1]==0)&(markerGeneCellArrayPos[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
			}
            
			
		} // for j end
	}
	fclose(regfile1);
    
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPos[qq]==0)&(refGeneCellArrayPos[qq+1]==0)&(refGeneCellArrayPos[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPos[qq], refGeneCellArrayPos[qq+1], refGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
            
		}
	}				
	fclose(regfile2);
    
	// ------------------------------------------------
	// save data after registration 
	// ------------------------------------------------
	
	
	registeredfilename1 = dfile_outputdir + "after_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	registeredfilename2 = dfile_outputdir + "after_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
    
	
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPosNew[qq]==0)&(markerGeneCellArrayPosNew[qq+1]==0)&(markerGeneCellArrayPosNew[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			}
			
			
		} // for j end
	}
	fclose(regfile1);
	
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPosNew[qq]==0)&(refGeneCellArrayPosNew[qq+1]==0)&(refGeneCellArrayPosNew[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			
		}
	}				
	fclose(regfile2);
	
	
	// ------------------------------------------------
	// analyze cell statistics
	// note that reference channel (e.g., EVE) cell statistics 
	// need to be combined with the same cells in marker channel 
	// ------------------------------------------------
	float *cellMean = 0;
	float *cellStd = 0;
	int *observed = 0;
	int *expected = 0;
	
	//position mean and standard deviation
    //	computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
	computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
	
	//as observed and expected will be computed when analyzing volume statistcs as below, they are deleted here to avoid memory leak
	if (observed) {delete []observed; observed=0;}
    //	if (expected) {delete []expected; expected=0;}
	
	int lenn = dim_marker_gene[1]*dim_marker_gene[0];
	float aveCellStd=0;
	
	for (j=0; j<lenn; j++) // compute the sum of std along each dimension and for each cell
	{
		aveCellStd += cellStd[j];
	}
	
	aveCellStd /= lenn;
	
	printf("aveCellStd = %f\n", aveCellStd);
    
	
	//size mean and standard deviation
	float *cellMean_vol;
	float *cellStd_vol;
	
	int dim_marker_gene_vol[3], dim_ref_gene_vol[3];
	
	dim_marker_gene_vol[0] = 1;
	dim_marker_gene_vol[1] = cellnamelist.size();
	dim_marker_gene_vol[2] = stacknum;
	
	dim_ref_gene_vol[0] = 1;
	dim_ref_gene_vol[1] = ref_gene_cellnamelist.size();
	dim_ref_gene_vol[2] = refstacknum;
	
	
	float *markerGeneCellArray = new float [dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*dim_marker_gene_vol[2]];
    
	for (j=0; j<dim_marker_gene_vol[2]; j++)
	{
		int mm1 = dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_marker_gene[1]*j;
        
		for (i=0; i<dim_marker_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_marker_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			markerGeneCellArray[nn1] = markerGeneCellArrayOtherFields[nn2+3];
		}
	}
    
	float *refGeneCellArray = new float [dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*dim_ref_gene_vol[2]];
    
	for (j=0; j<dim_ref_gene_vol[2]; j++)
	{
		int mm1 = dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_ref_gene[1]*j;
        
		for (i=0; i<dim_ref_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_ref_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			refGeneCellArray[nn1] = refGeneCellArrayOtherFields[nn2+3];
		}
	}
    
    //	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed, expected);
	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed);
    
	// -----------------------------------------------------------------------------------------------
	// compute for each cell its expected value (i.e., in how many stacks it should express
	// -----------------------------------------------------------------------------------------------
	computeExpectedValue(marker_gene_info, dfile_outputdir, refstacknum, expected);
    
	// ------------------
	// save atlas files
	// ------------------	
	bool final_tag = 0;
    
	saveAtlas(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
			  dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
    
	// ---------------------------------------------------------------
	// identify co-localized cells and combine those 
	// that are supposed to be the same cell to build the final atlas
	// ---------------------------------------------------------------
	
	// note that afte co-localization detection, the input arguments changed (both pointer address and contents)	
    //	coLocalizationDetection(marker_gene_info, cellnamelist, markerTag, 
    //			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol,
    //			  dim_marker_gene, dim_ref_gene, dfile_outputdir, ref_gene_idx, threratio, observed, expected);
    
	// ------------------------------------------------------------------------------------------------------------------------------
	// merge co-expressed cells and generate final atlas, cell names are replace and statistics (mean, stdev) recomputed  
	// ------------------------------------------------------------------------------------------------------------------------------
	float *cellMean_final = 0;
	float *cellStd_final= 0;
	
	float *cellMean_vol_final = 0;
	float *cellStd_vol_final = 0;
	
	int *observed_final = 0;
	bool *markerTag_final = 0;
	int *expected_final = 0;
    
	int markercnt = marker_gene_info.markernum() + 1; // other markers plus eve
	
	if (final_cellnum >0)
	{
		// merge cell locations
		mergeCell(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
				  cellMean_final, cellStd_final, observed_final, expected_final, markerTag_final);
        
		if (observed_final) {delete []observed_final; observed_final=0;}
		if (expected_final) {delete []expected_final; expected_final=0;}
		if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
		
        
		// merge cell volumes
		mergeCell(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
                  cellMean_vol_final, cellStd_vol_final, observed_final, expected_final, markerTag_final);
	}
    
	// ------------------------------------------------------------------------
	// save the final atlas
	// ------------------------------------------------------------------------	
	final_tag = 1;
	
	vector <string> cellnamelist_f; 
	
	for (i=0; i<final_cellnum; i++)
	{
		cellnamelist_f.push_back(final_cellnamelist[i].at(0).toStdString());
		printf("%d, %s\n", i, cellnamelist_f.at(i).c_str());
	}
    
	int dim_marker_gene_final[3];
	
	dim_marker_gene_final[0] = 3;
	dim_marker_gene_final[1] = final_cellnum;
	dim_marker_gene_final[2] = stacknum;
    
	
	saveAtlas(marker_gene_info, cellnamelist_f, ref_gene_cellnamelist, markerTag_final, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, 
			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean_final, cellStd_final, cellMean_vol_final, cellStd_vol_final, observed_final, expected_final,
			  dim_marker_gene_final, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
	
	
	// ---------------------------------------------------------------
	// identify co-localized cells and combine those 
	// that are supposed to be the same cell to build the final atlas
	// ---------------------------------------------------------------
	
	coLocalizationDetection(atlas_subject_filename, atlas_target_filename, dfile_outputdir);
	
	// ------------------
	// delete pointers
	// ------------------	
    
	
	if (markerTag) {delete []markerTag; markerTag=0;}
	if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
	
	
	if (refGeneCellArrayPos) {delete []refGeneCellArrayPos; refGeneCellArrayPos=0;}
	if (refGeneCellArrayOtherFields) {delete []refGeneCellArrayOtherFields; refGeneCellArrayOtherFields = 0;}
	if (refGeneCptCellArrayPos) {delete []refGeneCptCellArrayPos; refGeneCptCellArrayPos = 0;}
	if (refGeneCellArray) {delete []refGeneCellArray; refGeneCellArray = 0;}
	if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew=0;}
	
	if (markerGeneCellArray) {delete []markerGeneCellArray; markerGeneCellArray = 0;}
	if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
	if (markerGeneCellArrayPos) {delete []markerGeneCellArrayPos; markerGeneCellArrayPos=0;}		
	if (markerGeneCellArrayOtherFields) {delete []markerGeneCellArrayOtherFields; markerGeneCellArrayOtherFields=0;}
	
	if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}
    
	if (marker_ref_map) {delete []marker_ref_map; marker_ref_map=0;}
	
	if (cellMean) {delete []cellMean; cellMean = 0;}
	if (cellStd) {delete []cellStd; cellStd = 0;}	
	if (cellMean_vol) {delete []cellMean_vol; cellMean_vol = 0;}
	if (cellStd_vol) {delete []cellStd_vol; cellStd_vol = 0;}
	if (observed) {delete []observed; observed = 0;}
	if (expected) {delete []expected; expected = 0;}
	
	if (cellMean_final) {delete []cellMean_final; cellMean_final = 0;}
	if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
	if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
	if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
	if (observed_final) {delete []observed_final; observed_final = 0;}
	if (expected_final) {delete []observed_final; observed_final = 0;}
	
}


//rebuild the atlas,  target unknown for registration


void rebuildAtlas(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				  vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames,  		  				  
				  int *ref_gene_idx, string dfile_outputdir, string &dfile_target, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum)
{
    
	int i,j,k,m;
	FILE *file; 
	string outfilename;
	int fieldnum = FIELDNUM + 3;
    //	string dfile_target;
	
	// -----------------------------------------------------------
	// generate matrices for registration and statistical analysis
	// -----------------------------------------------------------	
	bool *markerTag;	
	float *markerGeneCellArrayPos = 0; //marker gene arrary containing only (x,y,z), for registration	
	float *markerGeneCellArrayOtherFields = 0;	//marker gene arrary containing intesity and volume information	
	float *refGeneCellArrayPos = 0; //reference gene (e.g., EVE) arrary containing only (x,y,z), for registration
	float *refGeneCptCellArrayPos = 0;	
	float *refGeneCellArrayOtherFields = 0; //the reference gene (e.g., EVE) arrary containing intensity and volume information
	int *marker_ref_map =0; // a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
	int stacknum, refstacknum;
	vector <string> reference_stack_list;
	
	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist,
				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  refGeneCptCellArrayPos, 
				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
	// --------------
	// registration
	// --------------	
	
	// read cell position in target file
	float *targetCellArrayPos = 0; // all annotated cell array					
	float *targetCptCellArrayPos =0; // control point (annotated) array
	float *targetCellArrayOtherFields = 0; 
	
	float *cellMean = 0;
	float *cellStd = 0;
	float *cellMean_vol = 0;
	float *cellStd_vol = 0;
	int *observed = 0;
	int *expected = 0;
    
	// do registration
	int dim_marker_gene[3], dim_ref_gene[3], dim_ref_gene_cpt[3];
	
	dim_marker_gene[0] = 3;
	dim_marker_gene[1] = cellnamelist.size();
	dim_marker_gene[2] = stacknum;
	
	dim_ref_gene[0] = 3;
	dim_ref_gene[1] = ref_gene_cellnamelist.size();
	dim_ref_gene[2] = refstacknum;
	
    //	printf("dim_marker_gene = %d, %d, %d\n", dim_marker_gene[0], dim_marker_gene[1], dim_marker_gene[2]);
    //	printf("dim_ref_gene = %d, %d, %d\n", dim_ref_gene[0], dim_ref_gene[1], dim_ref_gene[2]);
	
    
	int dim_marker_gene_vol[3], dim_ref_gene_vol[3];
	
	dim_marker_gene_vol[0] = 1;
	dim_marker_gene_vol[1] = cellnamelist.size();
	dim_marker_gene_vol[2] = stacknum;
	
	dim_ref_gene_vol[0] = 1;
	dim_ref_gene_vol[1] = ref_gene_cellnamelist.size();
	dim_ref_gene_vol[2] = refstacknum;
    
	float minstd = 99999;
	int refstackidx = -1;	
    //	int lenn = cellnamelist.size()*3;
	int lenn = dim_marker_gene[1]*dim_marker_gene[0];
    
	float *refGeneCellArrayPosNew=0, *markerGeneCellArrayPosNew=0;
	
	
	
	for (i=0; i<reference_stack_list.size(); i++)
	{
		dfile_target = reference_stack_list.at(i);
        
		
        //		getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
		getAtlasCell(dfile_target, cellnamelist, ref_gene_controlpoints_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
        
		// targetCellArrayPos and targetCellArrayOtherFields are no use
		if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
		if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}		
		
        //		registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, marker_ref_map, dim_marker_gene, dim_ref_gene, markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
        
		registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
					   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
		
		// ------------------------------------------------
		// analyze cell statistics
		// note that reference channel (e.g., EVE) cell statistics 
		// need to be combined with the same cells in marker channel 
		// ------------------------------------------------
		
		//position mean and standard deviation
        //		computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
		computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
		float aveCellStd = 0;
        
		for (j=0; j<lenn; j++) // compute the sum of std along each dimension and for each cell
		{
			aveCellStd += cellStd[j];
		}
		
		aveCellStd /= lenn;
		
		printf("%s\n", reference_stack_list.at(i).c_str()); 
		printf("aveCellStd = %f\n", aveCellStd);
		
		if (aveCellStd<minstd)
		{
			minstd = aveCellStd;
			refstackidx = i;
		}
        
		// delete pointers
		if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
		if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew=0;}
		if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}		
		if (cellMean) {delete []cellMean; cellMean = 0;}
		if (cellStd) {delete []cellStd; cellStd = 0;}	
		if (observed) {delete []observed; observed = 0;}
		if (expected) {delete []expected; expected = 0;}
        
	}
    
	
	// ------------------------------
	// do registration using the target file
	// ------------------------------	
    
	dfile_target = reference_stack_list.at(refstackidx);
	printf("reference stack: %d, %s\n", refstackidx, dfile_target.c_str());
	
	getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	
	// targetCellArrayPos and targetCellArrayOtherFields are no use
	if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
	if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}		
	
    //	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene,
    //				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
    
	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
	
	
	// ------------------------------------------------
	// save data before registration 
	// ------------------------------------------------
	FILE *regfile1, *regfile2;
	string registeredfilename1 = dfile_outputdir + "before_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	string registeredfilename2 = dfile_outputdir + "before_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
	
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPos[qq]==0)&(markerGeneCellArrayPos[qq+1]==0)&(markerGeneCellArrayPos[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
			}
			
			
		} // for j end
	}
	fclose(regfile1);
	
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPos[qq]==0)&(refGeneCellArrayPos[qq+1]==0)&(refGeneCellArrayPos[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPos[qq], refGeneCellArrayPos[qq+1], refGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
			
		}
	}				
	fclose(regfile2);
	
	// ------------------------------------------------
	// save data before registration 
	// ------------------------------------------------
	
	
	registeredfilename1 = dfile_outputdir + "after_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	registeredfilename2 = dfile_outputdir + "after_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
	
	
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPosNew[qq]==0)&(markerGeneCellArrayPosNew[qq+1]==0)&(markerGeneCellArrayPosNew[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			}
			
			
		} // for j end
	}
	fclose(regfile1);
	
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPosNew[qq]==0)&(refGeneCellArrayPosNew[qq+1]==0)&(refGeneCellArrayPosNew[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			
		}
	}				
	fclose(regfile2);
	
	
	
	// ------------------------------------------------
	// analyze cell statistics
	// note that reference channel (e.g., EVE) cell statistics 
	// need to be combined with the same cells in marker channel 
	// ------------------------------------------------
	
	//position mean and standard deviation
    //	computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
	computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
    
	if (observed) {delete []observed; observed = 0;}
    //	if (expected) {delete []expected; expected = 0;}	
	
	//size mean and standard deviation
	
	float *markerGeneCellArray = new float [dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*dim_marker_gene_vol[2]];
    
	for (j=0; j<dim_marker_gene_vol[2]; j++)
	{
		int mm1 = dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_marker_gene[1]*j;
        
		for (i=0; i<dim_marker_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_marker_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			markerGeneCellArray[nn1] = markerGeneCellArrayOtherFields[nn2+3];
		}
	}
    
	float *refGeneCellArray = new float [dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*dim_ref_gene_vol[2]]; //note that dim_ref_gene_vol[1] is smaller than dim_marker_gene[1]
    
	for (j=0; j<dim_ref_gene_vol[2]; j++)
	{
		int mm1 = dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_ref_gene[1]*j;
        
		for (i=0; i<dim_ref_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_ref_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			refGeneCellArray[nn1] = refGeneCellArrayOtherFields[nn2+3];
		}
	}
    
	
    //	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed, expected);
	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed);
    
	
	// -----------------------------------------------------------------------------------------------
	// compute for each cell its expected value (i.e., in how many stacks it should express)
	// -----------------------------------------------------------------------------------------------
	computeExpectedValue(marker_gene_info, dfile_outputdir, refstacknum, expected);
    
	
    //	// --------------
    //	// adjust posture
    //	// --------------
    //	
    //	// identify EpCCA1L, EpCCA1R, EpCCT3L, EpCCT3R, EpCCT2L, EpCCT2R
    //	
    //	int idx;
    //	float cpts[3*6];
    //	float cptsnew[3*6];
    //	
    //	for (j=0; j<dim_marker_gene[1]; j++)
    //	{
    //		
    //		idx = -1;
    //
    ////		printf("j = %d, %s\n", j, cellnamelist.at(j).c_str());
    ////		printf("1**** j = %d, dim_marker_gene[1] = %d \n", j, dim_marker_gene[1]);
    //		
    //		if (cellnamelist.at(j).compare("EpCC A1L") ==0)
    //			idx = 0;
    //		else if (cellnamelist.at(j).compare("EpCC A1R") ==0)
    //			idx = 1;
    //		else if (cellnamelist.at(j).compare("EpCC T3L") ==0)
    //			idx = 2;
    //		else if (cellnamelist.at(j).compare("EpCC T3R") ==0)
    //			idx = 3;
    //		else if (cellnamelist.at(j).compare("EpCC T2L") ==0)
    //			idx = 4;
    //		else if (cellnamelist.at(j).compare("EpCC T2R") ==0)
    //			idx = 5;
    //
    ////		printf("2***** j = %d, dim_marker_gene[1] = %d \n", j, dim_marker_gene[1]);
    ////		printf("%s\n", cellnamelist.at(j).c_str());
    ////		
    ////		printf("idx = %d\n", idx);
    //		
    //		if (idx>=0)
    //		{
    //			cpts[idx*3] = cellMean[j*3];
    //			cpts[idx*3+1] = cellMean[j*3+1];
    //			cpts[idx*3+2] = cellMean[j*3+2];
    ////			printf("cpts_x = %f, cpts_y = %f, cpts_z = %f\n", cellMean[j*3], cellMean[j*3+1], cellMean[j*3+2]); 
    //			
    //		}
    ////		printf("3***** j = %d, dim_marker_gene[1] = %d \n", j, dim_marker_gene[1]);
    //				
    //	}
    //
    //	for (j=0; j<6; j++)
    //		printf("cpts_x = %f, cpts_y = %f, cpts_z = %f\n", cpts[j*3], cpts[j*3+1],cpts[j*3+2]); 
    //
    //	// generate new coordinates for the 6 controling cells
    //	
    //	float C0[3]; // middle point between EpCC A1L, EpCC A1R, EpCC T3L, EpCC T3R, set as new origin
    //	float C1[3]; // middle point between EpCC A1L and EpCC A1R
    //	float C2[3]; // middle point between EpCC T3L and EpCC T3R
    //	float C3[3]; // middle point between EpCC T2L and EpCC T2R
    //	
    //	for (j=0; j<3; j++)
    //		C0[j] = (cpts[0+j] + cpts[3+j] + cpts[6+j] + cpts[9+j])/4;
    //	
    //	for (j=0; j<3; j++)
    //		C1[j] = (cpts[0+j] + cpts[3+j])/2;
    //
    //	for (j=0; j<3; j++)
    //		C2[j] = (cpts[6+j] + cpts[9+j])/2;
    //	
    //	for (j=0; j<3; j++)
    //		C3[j] = (cpts[12+j] + cpts[15+j])/2;
    //	
    //
    //	// new coordinate of Epcc A1L
    //	cptsnew[0] = sqrt((C0[0]-C1[0])*(C0[0]-C1[0]) + (C0[1]-C1[1])*(C0[1]-C1[1]) + (C0[2]-C1[2])*(C0[2]-C1[2]));
    //	cptsnew[1] = -sqrt((cpts[0]-C1[0])*(cpts[0]-C1[0]) + (cpts[1]-C1[1])*(cpts[1]-C1[1]) + (cpts[2]-C1[2])*(cpts[2]-C1[2]));
    //	cptsnew[2] = 0;
    //	
    //	// new coordinate of Epcc A1R	
    //	cptsnew[3] = cptsnew[0];
    //	cptsnew[4] = sqrt((cpts[3]-C1[0])*(cpts[3]-C1[0]) + (cpts[4]-C1[1])*(cpts[4]-C1[1]) + (cpts[5]-C1[2])*(cpts[5]-C1[2]));
    //	cptsnew[5] = 0;
    //	
    //	// new coordinate of Epcc T3L
    //	cptsnew[6] = -sqrt((C0[0]-C2[0])*(C0[0]-C2[0]) + (C0[1]-C2[1])*(C0[1]-C2[1]) + (C0[2]-C2[2])*(C0[2]-C2[2]));
    //	cptsnew[7] = -sqrt((cpts[6]-C2[0])*(cpts[6]-C2[0]) + (cpts[7]-C2[1])*(cpts[7]-C2[1]) + (cpts[8]-C2[2])*(cpts[8]-C2[2]));
    //	cptsnew[8] = 0;
    //	
    //	// new coordinate of Epcc T3R
    //	cptsnew[9] = cptsnew[6];
    //	cptsnew[10] = sqrt((cpts[9]-C2[0])*(cpts[9]-C2[0]) + (cpts[10]-C2[1])*(cpts[10]-C2[1]) + (cpts[11]-C2[2])*(cpts[11]-C2[2]));
    //	cptsnew[11] = 0;
    //	
    //	// new coordinate of Epcc T2L
    //	cptsnew[12] = -sqrt((C0[0]-C3[0])*(C0[0]-C3[0]) + (C0[1]-C3[1])*(C0[1]-C3[1]) + (C0[2]-C3[2])*(C0[2]-C3[2]));
    //	cptsnew[13] = -sqrt((cpts[12]-C3[0])*(cpts[12]-C3[0]) + (cpts[13]-C3[1])*(cpts[13]-C3[1]) + (cpts[14]-C3[2])*(cpts[14]-C3[2]));
    //	cptsnew[14] = 0;
    //	
    //	// new coordinate of Epcc T2R
    //	cptsnew[15] = -sqrt((C0[0]-C3[0])*(C0[0]-C3[0]) + (C0[1]-C3[1])*(C0[1]-C3[1]) + (C0[2]-C3[2])*(C0[2]-C3[2]));
    //	cptsnew[16] = sqrt((cpts[15]-C3[0])*(cpts[15]-C3[0]) + (cpts[16]-C3[1])*(cpts[16]-C3[1]) + (cpts[17]-C3[2])*(cpts[17]-C3[2]));
    //	cptsnew[17] = 0;
    //	
    //	printf("old EpCC cell positions\n");
    //	for (i=0; i<6; i++)
    //		printf("%f, %f, %f\n", cpts[i*3+0], cpts[i*3+1], cpts[i*3+2]);
    //
    //	printf("new EpCC cell positions\n");
    //	for (i=0; i<6; i++)
    //		printf("%f, %f, %f\n", cptsnew[i*3+0], cptsnew[i*3+1], cptsnew[i*3+2]);
    //	
    //	// register cells to map them into new coordinate space
    //	
    //	registerAffine(cellMean, dim_marker_gene[1], cpts, cptsnew, 6, 3);
    
	
	
	
    //	for (i=0; i<dim_marker_gene[1]; i++)
    //		printf("**** observed[%d] = %d\n", i, observed[i]);
	
	
	
	// ------------------
	// save atlas files
	// ------------------	
	bool final_tag = 0;
	
	saveAtlas(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
			  dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
	
	// ---------------------------------------------------------------
	// identify co-localized cells and combine those 
	// that are supposed to be the same cell to build the final atlas
	// ---------------------------------------------------------------
	
	// note that afte co-localization detection, the input arguments changed (both pointer address and contents)	
	coLocalizationDetection(marker_gene_info, cellnamelist, markerTag, 
                            markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol,
                            dim_marker_gene, dim_ref_gene, dfile_outputdir, ref_gene_idx, threratio, observed, expected);
    
	// ------------------
	// delete pointers
	// ------------------	
	
	if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
	if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew = 0;}
	
	if (markerTag) {delete []markerTag; markerTag=0;}
	if (markerGeneCellArrayPos) {delete []markerGeneCellArrayPos; markerGeneCellArrayPos=0;}		
	if (markerGeneCellArrayOtherFields) {delete []markerGeneCellArrayOtherFields; markerGeneCellArrayOtherFields=0;}
	if (refGeneCellArrayPos) {delete []refGeneCellArrayPos; refGeneCellArrayPos=0;}
	if (refGeneCellArrayOtherFields) {delete []refGeneCellArrayOtherFields; refGeneCellArrayOtherFields = 0;}	
	if (refGeneCptCellArrayPos) {delete []refGeneCptCellArrayPos; refGeneCptCellArrayPos = 0;}
	
	if (marker_ref_map) {delete []marker_ref_map; marker_ref_map=0;}
	if (markerGeneCellArray) {delete []markerGeneCellArray; markerGeneCellArray = 0;}
	if (refGeneCellArray) {delete []refGeneCellArray; refGeneCellArray = 0;}
	if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}
	
	if (cellMean) {delete []cellMean; cellMean = 0;}
	if (cellStd) {delete []cellStd; cellStd = 0;}	
	if (cellMean_vol) {delete []cellMean_vol; cellMean_vol = 0;}
	if (cellStd_vol) {delete []cellStd_vol; cellStd_vol = 0;}	
	if (observed) {delete []observed; observed = 0;}
	if (expected) {delete []expected; expected = 0;}
	
    
};


//rebuild the atlas, target known for registration, allow co-expression analysis for any number of apo files

void rebuildAtlas(MarkerGeneInfo marker_gene_info, string dfile_target, vector <string> cellnamelist, vector <string> ref_gene_cellnamelist, vector <string> ref_gene_controlpoints_cellnamelist, 
				  vector <string> forceAddCellnamelist,  vector <string> forceAddCellnamelist_final, vector < vector <string> > cellTypeNamelist, vector <string> cellTypeNames, 
				  int *ref_gene_idx, string dfile_outputdir, float threratio, vector <QStringList> final_cellnamelist, int final_cellnum, string test_colocalization_atlas_apo_filename)
{
	
	int i,j,k,m;
	FILE *file; 
	string outfilename;
	int fieldnum = FIELDNUM + 3;
	
	// -----------------------------------------------------------
	// generate matrices for registration and statistical analysis
	// -----------------------------------------------------------	
	bool *markerTag;	
	float *markerGeneCellArrayPos = 0; //marker gene arrary containing only (x,y,z), for registration	
	float *markerGeneCellArrayOtherFields = 0;	//marker gene arrary containing intesity and volume information	
	float *refGeneCellArrayPos = 0; //reference gene (e.g., EVE) arrary containing only (x,y,z) 
	float *refGeneCptCellArrayPos = 0; // reference gene array containing only control points for registration, note refGeneCptCellArrayPos cab be a subset of refGeneCptCellArrayPos
	float *refGeneCellArrayOtherFields = 0; //the reference gene (e.g., EVE) arrary containing intensity and volume information
	int *marker_ref_map =0; // a vector indicating for each marker stack, the index of its reference stack in refGeneCellArrayPos and refGeneCellArrayOtherFields
	int stacknum, refstacknum;
	vector <string> reference_stack_list;
	
	// for debug purpose
#ifdef DEBUG
	
	MarkerStacks markerstack;
	int cnt = 0;
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.reference_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			printf("i=%d, j=%d, cnt=%d, stack= %s, ref= %s\n", i, j, cnt, markerstack.signal_ch_filename.at(j).c_str(), markerstack.reference_ch_filename.at(j).c_str());
			cnt++;
		}
	}
#endif
	
	//	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, 
	//				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  
	//				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
	genMatrices(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist,
				markerGeneCellArrayPos, markerGeneCellArrayOtherFields, refGeneCellArrayPos, refGeneCellArrayOtherFields,  refGeneCptCellArrayPos, 
				marker_ref_map, markerTag, stacknum, refstacknum, reference_stack_list);
	
	
	// --------------
	// registration
	// --------------	
	
	// read cell position in target file
	float *targetCellArrayPos = 0; // all annotated cell array					
	float *targetCptCellArrayPos =0; // control point (annotated) array
	float *targetCellArrayOtherFields = 0; 
	
	printf("target file = %s\n", dfile_target.c_str());
	
	
	//	getAtlasCell(dfile_target, cellnamelist, ref_gene_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	getAtlasCell(dfile_target, cellnamelist, ref_gene_controlpoints_cellnamelist, targetCellArrayPos, targetCptCellArrayPos, targetCellArrayOtherFields);// targetCellArrayPos and targetCellArrayOtherFields are useless for registration purpose
	
	// targetCellArrayPos and targetCellArrayOtherFields are no use
	if (targetCellArrayPos) {delete targetCellArrayPos; targetCellArrayPos = 0;}
	if (targetCellArrayOtherFields) {delete targetCellArrayOtherFields; targetCellArrayOtherFields = 0;}
	
	// do registration
	int dim_marker_gene[3], dim_ref_gene[3], dim_ref_gene_cpt[3];
	
	dim_marker_gene[0] = 3;
	dim_marker_gene[1] = cellnamelist.size();
	dim_marker_gene[2] = stacknum;
	
	dim_ref_gene[0] = 3;
	dim_ref_gene[1] = ref_gene_cellnamelist.size();
	dim_ref_gene[2] = refstacknum;
	
	dim_ref_gene_cpt[0] = 3;
	dim_ref_gene_cpt[1] = ref_gene_controlpoints_cellnamelist.size();
	dim_ref_gene_cpt[2] = refstacknum;
	
	float *refGeneCellArrayPosNew=0, *markerGeneCellArrayPosNew=0;
	
	registerStacks(targetCptCellArrayPos, markerGeneCellArrayPos, refGeneCellArrayPos, refGeneCptCellArrayPos, marker_ref_map, dim_marker_gene,dim_ref_gene, dim_ref_gene_cpt,
				   markerGeneCellArrayPosNew, refGeneCellArrayPosNew);
	
	
	// ------------------------------------------------
	// save data before registration 
	// ------------------------------------------------
	FILE *regfile1, *regfile2;
	string registeredfilename1 = dfile_outputdir + "before_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	string registeredfilename2 = dfile_outputdir + "before_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
	
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPos[qq]==0)&(markerGeneCellArrayPos[qq+1]==0)&(markerGeneCellArrayPos[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPos[qq], markerGeneCellArrayPos[qq+1], markerGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
			}
			
			
		} // for j end
	}
	fclose(regfile1);
	
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPos[qq]==0)&(refGeneCellArrayPos[qq+1]==0)&(refGeneCellArrayPos[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPos[qq], refGeneCellArrayPos[qq+1], refGeneCellArrayPos[qq+2]); // cell name, x, y, z positions
			
		}
	}				
	fclose(regfile2);
	
	// ------------------------------------------------
	// save data after registration 
	// ------------------------------------------------
	
	
	registeredfilename1 = dfile_outputdir + "after_registration_marker.txt";
	regfile1 = fopen(registeredfilename1.c_str(), "wt");
	
	registeredfilename2 = dfile_outputdir + "after_registration_ref.txt";
	regfile2 = fopen(registeredfilename2.c_str(), "wt");
	
	
	// write regfile1
	for (i=0; i<marker_gene_info.markernum(); i++)
	{
		
		MarkerStacks markerstack = marker_gene_info.marker_stacks.at(i);
		int markerstacknum = (markerstack.signal_ch_filename).size();
		
		for (j=0; j<markerstacknum; j++)
		{
			
			fprintf(regfile1, "%s\n", markerstack.signal_ch_filename.at(j).c_str()); 
			
			for (m=0; m<dim_marker_gene[1]; m++)
			{
				int mm = m*3;
				int qq = 3*dim_marker_gene[1]*j + mm;
				if ((markerGeneCellArrayPosNew[qq]==0)&(markerGeneCellArrayPosNew[qq+1]==0)&(markerGeneCellArrayPosNew[qq+2]==0))
					continue;
				else
					fprintf(regfile1, "%s, %f, %f %f\n", cellnamelist.at(m).c_str(), markerGeneCellArrayPosNew[qq], markerGeneCellArrayPosNew[qq+1], markerGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			}
			
			
		} // for j end
	}
	fclose(regfile1);
	
	// write regfile2
	for (j=0; j<refstacknum; j++)
	{
		
		fprintf(regfile2, "%s\n", reference_stack_list.at(j).c_str()); 
		
		for (m=0; m<dim_ref_gene[1]; m++)
		{
			int mm = m*3;
			int qq = 3*dim_ref_gene[1]*j + mm;
			if ((refGeneCellArrayPosNew[qq]==0)&(refGeneCellArrayPosNew[qq+1]==0)&(refGeneCellArrayPosNew[qq+2]==0))
				continue;
			else
				fprintf(regfile2, "%s, %f, %f %f\n", ref_gene_cellnamelist.at(m).c_str(), refGeneCellArrayPosNew[qq], refGeneCellArrayPosNew[qq+1], refGeneCellArrayPosNew[qq+2]); // cell name, x, y, z positions
			
		}
	}				
	fclose(regfile2);
	
	
	// ------------------------------------------------
	// analyze cell statistics
	// note that reference channel (e.g., EVE) cell statistics 
	// need to be combined with the same cells in marker channel 
	// ------------------------------------------------
	float *cellMean = 0;
	float *cellStd = 0;
	int *observed = 0;
	int *expected = 0;
	
	//position mean and standard deviation
	//	computeCellStat(markerGeneCellArrayPos, refGeneCellArrayPos, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed, expected);
	computeCellStat(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, cellMean, cellStd, observed);
	
	//as observed and expected will be computed when analyzing volume statistcs as below, they are deleted here to avoid memory leak
	if (observed) {delete []observed; observed=0;}
	//	if (expected) {delete []expected; expected=0;}
	
	int lenn = dim_marker_gene[1]*dim_marker_gene[0];
	float aveCellStd=0;
	
	for (j=0; j<lenn; j++) // compute the sum of std along each dimension and for each cell
	{
		aveCellStd += cellStd[j];
	}
	
	aveCellStd /= lenn;
	
	printf("aveCellStd = %f\n", aveCellStd);
	
	
	//size mean and standard deviation
	float *cellMean_vol;
	float *cellStd_vol;
	
	int dim_marker_gene_vol[3], dim_ref_gene_vol[3];
	
	dim_marker_gene_vol[0] = 1;
	dim_marker_gene_vol[1] = cellnamelist.size();
	dim_marker_gene_vol[2] = stacknum;
	
	dim_ref_gene_vol[0] = 1;
	dim_ref_gene_vol[1] = ref_gene_cellnamelist.size();
	dim_ref_gene_vol[2] = refstacknum;
	
	
	float *markerGeneCellArray = new float [dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*dim_marker_gene_vol[2]];
	
	for (j=0; j<dim_marker_gene_vol[2]; j++)
	{
		int mm1 = dim_marker_gene_vol[0]*dim_marker_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_marker_gene[1]*j;
		
		for (i=0; i<dim_marker_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_marker_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			markerGeneCellArray[nn1] = markerGeneCellArrayOtherFields[nn2+3];
		}
	}
	
	float *refGeneCellArray = new float [dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*dim_ref_gene_vol[2]];
	
	for (j=0; j<dim_ref_gene_vol[2]; j++)
	{
		int mm1 = dim_ref_gene_vol[0]*dim_ref_gene_vol[1]*j;
		int mm2 = FIELDNUM*dim_ref_gene[1]*j;
		
		for (i=0; i<dim_ref_gene_vol[1]; i++)
		{
			int nn1 = mm1 + i*dim_ref_gene_vol[0];
			int nn2 = mm2 + i*FIELDNUM;
			refGeneCellArray[nn1] = refGeneCellArrayOtherFields[nn2+3];
		}
	}
	
	//	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed, expected);
	computeCellStat(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, cellMean_vol, cellStd_vol, observed);
	
	// -----------------------------------------------------------------------------------------------
	// compute for each cell its expected value (i.e., in how many stacks it should express
	// -----------------------------------------------------------------------------------------------
	computeExpectedValue(marker_gene_info, dfile_outputdir, refstacknum, expected);
	
	// ------------------
	// save atlas files
	// ------------------	
	bool final_tag = 0;
	
	saveAtlas(marker_gene_info, cellnamelist, ref_gene_cellnamelist, markerTag, forceAddCellnamelist, cellTypeNamelist, cellTypeNames,
			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol, cellStd_vol, observed, expected,
			  dim_marker_gene, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
	
	// ---------------------------------------------------------------
	// identify co-localized cells and combine those 
	// that are supposed to be the same cell to build the final atlas
	// ---------------------------------------------------------------
	
    //	// note that afte co-localization detection, the input arguments changed (both pointer address and contents)	
    //	coLocalizationDetection(marker_gene_info, cellnamelist, markerTag, 
    //			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean, cellStd, cellMean_vol,
    //			  dim_marker_gene, dim_ref_gene, dfile_outputdir, ref_gene_idx, threratio, observed, expected);
	
	// ------------------------------------------------------------------------------------------------------------------------------
	// merge co-expressed cells and generate final atlas, cell names are replace and statistics (mean, stdev) recomputed  
	// ------------------------------------------------------------------------------------------------------------------------------
	float *cellMean_final = 0;
	float *cellStd_final= 0;
	
	float *cellMean_vol_final = 0;
	float *cellStd_vol_final = 0;
	
	int *observed_final = 0;
	bool *markerTag_final = 0;
	int *expected_final = 0;
	
	int markercnt = marker_gene_info.markernum() + 1; // other markers plus eve
	
	string file_genemap = dfile_outputdir + "genemap.txt";
	
	FILE *filetmp = fopen(file_genemap.c_str(), "wt"); // file to save gene expression map
	
	if (final_cellnum >0)
	{
		// merge cell locations
		mergeCell(markerGeneCellArrayPosNew, refGeneCellArrayPosNew, ref_gene_idx, dim_marker_gene, dim_ref_gene, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
				  cellMean_final, cellStd_final, observed_final, expected_final, markerTag_final);
        
		for (i=0; i<final_cellnum;i++)
		{
            //			printf("%d, %s\n", i, qPrintable(final_cellnamelist[i].at(0)));
			
			fprintf(filetmp, "%s: ", qPrintable(final_cellnamelist[i].at(0)));
			int nn= i*markercnt; 
			for (j=0; j<markercnt; j++)
				fprintf(filetmp, "%d,", markerTag_final[nn+j]);
			fprintf(filetmp, "\n");
		}
		
        
		if (observed_final) {delete []observed_final; observed_final=0;}
		if (expected_final) {delete []expected_final; expected_final=0;}
		if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
		
		
		// merge cell volumes
		mergeCell(markerGeneCellArray, refGeneCellArray, ref_gene_idx, dim_marker_gene_vol, dim_ref_gene_vol, final_cellnamelist, final_cellnum, cellnamelist, observed, expected, markerTag, markercnt,
				  cellMean_vol_final, cellStd_vol_final, observed_final, expected_final, markerTag_final);
	}
	
	fclose(filetmp);
	
	// ------------------------------------------------------------------------
	// save the final atlas
	// ------------------------------------------------------------------------	
	final_tag = 1;
	
	vector <string> cellnamelist_f; 
	
	for (i=0; i<final_cellnum; i++)
	{
		cellnamelist_f.push_back(final_cellnamelist[i].at(0).toStdString());
		printf("%d, %s\n", i, cellnamelist_f.at(i).c_str());
	}
	
	int dim_marker_gene_final[3];
	
	dim_marker_gene_final[0] = 3;
	dim_marker_gene_final[1] = final_cellnum;
	dim_marker_gene_final[2] = stacknum;
	
	
	saveAtlas(marker_gene_info, cellnamelist_f, ref_gene_cellnamelist, markerTag_final, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, 
			  markerGeneCellArrayPosNew, refGeneCellArrayPosNew, cellMean_final, cellStd_final, cellMean_vol_final, cellStd_vol_final, observed_final, expected_final,
			  dim_marker_gene_final, dim_ref_gene, dfile_outputdir, reference_stack_list, ref_gene_idx, threratio, final_tag);	
	
	
    //	// ---------------------------------------------------------------
    //	// identify co-localized cells and combine those 
    //	// that are supposed to be the same cell to build the final atlas
    //	// ---------------------------------------------------------------
    //	
    //	//coLocalizationDetection(atlas_subject_filename, atlas_target_filename, dfile_outputdir);
	coLocalizationDetection(test_colocalization_atlas_apo_filename, dfile_outputdir);
    
	
	// ------------------
	// delete pointers
	// ------------------	
	
	
	if (markerTag) {delete []markerTag; markerTag=0;}
	if (markerTag_final) {delete []markerTag_final; markerTag_final=0;}
	
	
	if (refGeneCellArrayPos) {delete []refGeneCellArrayPos; refGeneCellArrayPos=0;}
	if (refGeneCellArrayOtherFields) {delete []refGeneCellArrayOtherFields; refGeneCellArrayOtherFields = 0;}
	if (refGeneCptCellArrayPos) {delete []refGeneCptCellArrayPos; refGeneCptCellArrayPos = 0;}
	if (refGeneCellArray) {delete []refGeneCellArray; refGeneCellArray = 0;}
	if (refGeneCellArrayPosNew) {delete []refGeneCellArrayPosNew; refGeneCellArrayPosNew=0;}
	
	if (markerGeneCellArray) {delete []markerGeneCellArray; markerGeneCellArray = 0;}
	if (markerGeneCellArrayPosNew) {delete []markerGeneCellArrayPosNew; markerGeneCellArrayPosNew=0;}
	if (markerGeneCellArrayPos) {delete []markerGeneCellArrayPos; markerGeneCellArrayPos=0;}		
	if (markerGeneCellArrayOtherFields) {delete []markerGeneCellArrayOtherFields; markerGeneCellArrayOtherFields=0;}
	
	if (targetCptCellArrayPos) {delete []targetCptCellArrayPos; targetCptCellArrayPos=0;}
	
	if (marker_ref_map) {delete []marker_ref_map; marker_ref_map=0;}
	
	if (cellMean) {delete []cellMean; cellMean = 0;}
	if (cellStd) {delete []cellStd; cellStd = 0;}	
	if (cellMean_vol) {delete []cellMean_vol; cellMean_vol = 0;}
	if (cellStd_vol) {delete []cellStd_vol; cellStd_vol = 0;}
	if (observed) {delete []observed; observed = 0;}
	if (expected) {delete []expected; expected = 0;}
	
	if (cellMean_final) {delete []cellMean_final; cellMean_final = 0;}
	if (cellStd_final) {delete []cellStd_final; cellStd_final = 0;}
	if (cellMean_vol_final) {delete []cellMean_vol_final; cellMean_vol_final = 0;}
	if (cellStd_vol_final) {delete []cellStd_vol_final; cellStd_vol_final = 0;}
	if (observed_final) {delete []observed_final; observed_final = 0;}
	if (expected_final) {delete []observed_final; observed_final = 0;}
	
}



void coLocalizationDetection(vector <string> atlas_apo_filelist, string dfile_output)
{
	
    
    long int i,j, k,m, n, p, q;
  	int collocalCellNum = -1;	
 	float thredis = 10; // distance threshold which determines potential cell colocalization	
    
	// ---------------------------------------------------------------------------------
	// read each the contents of each atlas apo, and concatenate them into one file
	// ---------------------------------------------------------------------------------
	
	vector <QStringList> atlas_apo_combined;
	int num_cell = 0;
    int num_apo_file = atlas_apo_filelist.size(); // number of apo atlas files    
	int *num_each_apo = new int [atlas_apo_filelist.size()];
	
	for (i=0; i<num_apo_file; i++)
	{
		vector <QStringList> tmp;
        //		int num_item;
		
        //		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_item);
		readLines(atlas_apo_filelist.at(i).c_str(), tmp, num_each_apo[i]);
		
        //		for (j=0; j<num_item; j++)
		for (j=0; j<num_each_apo[i]; j++)		
			atlas_apo_combined.push_back(tmp[j]);
		
        //		num_cell += num_item;
		num_cell += num_each_apo[i];
	}
	
	
    //#ifdef DEBUG	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j<atlas_apo_combined[i].size(); j++)
			printf("%s, ", qPrintable(atlas_apo_combined[i].at(j)));		
		printf("\n");
	}
    //#endif
	
	
	float *celldis = new float [num_cell*num_cell];
	
	float *sortidx2 = new float [num_cell+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
	float *sortval2 = new float [num_cell+1];
	
	// -------------------------
	// parse atlas files
	// -------------------------
	
	float *cellMean = new float [num_cell*3];
	
	vector <vector <string> > markerName;
	
	
	// parse target atlas
	
	for (i=0; i<num_cell; i++)
	{
		
		cellMean[i*3] = atof(qPrintable(atlas_apo_combined[i].at(5)));
		cellMean[i*3+1] = atof(qPrintable(atlas_apo_combined[i].at(6)));
		cellMean[i*3+2] = atof(qPrintable(atlas_apo_combined[i].at(4)));		
		
		
		j=2; // j=0 and j=1 correspond to "<" and "$"
		int start_pos = j;
		int end_pos = atlas_apo_combined[i].at(3).size()-1;
		int cnt = 0;
		
		vector <string> tmpstring2; tmpstring2.clear();
		
		printf("length of tmpstring2 = %d\n", tmpstring2.size());
		
		while (j<atlas_apo_combined[i].at(3).size())
		{	
			
			if (atlas_apo_combined[i].at(3).mid(j,1)=="_")			
			{
				end_pos = j-1;
				
				tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos+1)));
				
				start_pos = j+2;				
				cnt++;
			}
			
			j++;
		}
		tmpstring2.push_back(qPrintable(atlas_apo_combined[i].at(3).mid(start_pos, end_pos-start_pos)));
		markerName.push_back(tmpstring2); 
		
#ifdef DEBUG		
		for (j=0; j<=cnt; j++)
			printf("%d, %s, %d", i, markerName[i].at(j).c_str(),  markerName[i].size());
		
		printf("\n");
#endif				
		
	}
	
    //#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		printf("^^^^^ %d, %d, ", i, markerName[i].size());
		
		for (j=0; j< markerName[i].size(); j++)
		{
			
			printf("%d, %d, ", i, j);			
			printf("%s, ", markerName[i].at(j).c_str());
			
		}
		printf("\n");
	}
    //#endif	
	
	
	
	// -----------------------------------------	 
	// compute pair-wise distance between cells
	// -----------------------------------------	 
	
	
	
	for (i=0; i<num_cell; i++)
	{
		
		int nn = i*3;
		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))) 		
		{
			for (j=0; j<num_cell; j++)
			{
				int mm = j*3;
				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))) 		
				{
					celldis[i*num_cell+j] = sqrt((cellMean[nn]-cellMean[mm])*(cellMean[nn]-cellMean[mm]) 
                                                 + (cellMean[nn+1]-cellMean[mm+1])*(cellMean[nn+1]-cellMean[mm+1]) 
                                                 + (cellMean[nn+2]-cellMean[mm+2])*(cellMean[nn+2]-cellMean[mm+2]));
					printf("%d, %d, %f\n", i, j, celldis[i*num_cell+j]);
				}
				else
					celldis[i*num_cell+j] = 9999;
				
			}
		}
		else
			for (j=0; j<num_cell; j++)
			{
				celldis[i*num_cell+j] = 9999;
			}
	}
	
	
	
	
	
	// ---------------------------------------------------------------------
	// compute prediction, i.e., canndidate co-localized cells, based on
	// distance and L/R rules
	// ---------------------------------------------------------------------
	
	bool *prediction = new bool [num_cell*num_cell];
	bool shareMarkerTag = 0;
	
	
#ifdef DEBUG		
	
	for (i=0; i<num_cell; i++)
	{
		for (j=0; j< markerName[i].size(); j++)
			printf("%d, %d, %s, %d", i, j, markerName[i].at(j).c_str(),  markerName[i].size());
	}
	
#endif
	
	for (i=0; i<num_cell; i++)
	{
		int nn = i*num_cell;
		
		size_t len_i = atlas_apo_combined[i].at(2).size(); // atlas_apo_combined[i].at(2) is the name of the ith cell
		//		string surfix_i = atlas_apo_combined[i].at(2).substr(len_i-2, len_i-1);
		string surfix_i = qPrintable(atlas_apo_combined[i].at(2).mid(len_i-1, 1)); // get the hemisegment name
		
		// select those whose mean distance with respect to cell i is less than thredis
		for (j=0; j<num_cell; j++)
		{
			
			k = 0;
			int n2 = 0;
			
			shareMarkerTag = 0;
			prediction[nn+j] = 0;
			
			if (j!=i)
			{
                
				for (p=0; p<markerName[i].size(); p++)
				{
					for (q=0; q<markerName[j].size(); q++)
					{
						//					printf("%d, %d, %d, %d, %s, %s\n", i, j, p, q, markerName[i].at(p).c_str(),  markerName[j].at(q).c_str());
						
						if (markerName[i].at(p).compare(markerName[j].at(q))==0)
						{
							shareMarkerTag = 1;
							break;
						}
					} // for q end
					if (shareMarkerTag == 1)
						break;
				} // for p end
				
				if (shareMarkerTag == 0) // cell i and cell j do not express the same marker gene
				{
					size_t len_j = atlas_apo_combined[j].at(2).size();
					//				string surfix_j = atlas_apo_combined[j].at(2).substr(len_j-2, len_j-1);
					string surfix_j = qPrintable(atlas_apo_combined[j].at(2).mid(len_j-1, 1));
					//				printf("%s, %s\n", surfix_i.c_str(), surfix_j.c_str());
					
					
					if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
						|| ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
					{
						printf("%d, %d, %f, %s, %s\n", i,j,celldis[nn+j],surfix_i.c_str(), surfix_j.c_str());
						if (celldis[nn+j]<thredis) // distance rule
							prediction[nn+j] = 1; 
						else
							prediction[nn+j]=0; 
					}
				}
			} // if (j!=i) end
		} // for j end
	} // for i end
	
	
	// ---------------------------------------------------------------------
	// sort candidates
	// ---------------------------------------------------------------------
	
	string filename = dfile_output + "colocalizedCells.txt";
	printf("%s\n", filename.c_str());
	FILE *file = fopen(filename.c_str(),"wt");
	
    ////	filename = test_colocalization_atlas_apo_filename + ".coexpress.apo";
    //	filename = dfile_output + ".coexpress.apo";    
    //	FILE *file2 = fopen(filename.c_str(), "wt");
    
	bool *coexpress_flg = new bool [num_cell];
	
	for (i=0; i<num_cell; i++)
        coexpress_flg[i] = 0;
	
	for (i=0; i<num_cell; i++)
	{
		
		int candcnt = 0;
		
		// sort each cell based on its distance to the cell in consideration
		
		sortval2[0] = -999; // the 0th element is not sorted in sort2			
		sortidx2[0] = -999;
		
		for (j=0; j<num_cell; j++)
		{	
			sortval2[j+1] = celldis[i*num_cell+j];
			sortidx2[j+1] = j;
		}
		
		sort2(num_cell, sortval2, sortidx2);// the first element is not sorted
		
		j = 0;
		bool print_tag = 0;
		bool cellname_printed_tag = 0;
		
		
		while (sortval2[j+1] < thredis)
		{
			if ((sortidx2[j+1]!=i) && (prediction[i*num_cell+(long)sortidx2[j+1]]==1))
			{
                //				fprintf(file, "cellname: %s, Candidate: %s , meanx=%f, meany=%f, meanz=%f\n", 
                //						qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
                
				if (cellname_printed_tag == 0)
				{
					fprintf(file, "\ncellname: %s, meanx=%f, meany=%f, meanz=%f\n", 
							qPrintable(atlas_apo_combined[i].at(2)), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2]);
					
					cellname_printed_tag = 1;
				}
				
				fprintf(file, "Candidate:%s , meanx=%f, meany=%f, meanz=%f\n", 
						qPrintable(atlas_apo_combined[(long)sortidx2[j+1]].at(2)), cellMean[(long)sortidx2[j+1]*3], cellMean[(long)sortidx2[j+1]*3+1], cellMean[(long)sortidx2[j+1]*3+2]);
				
				
				if (print_tag == 0)
				{
					print_tag = 1;
					coexpress_flg[i] = 1;
					coexpress_flg[(long)sortval2[j+1]] = 1;
				}
                
			}
			j++;
			
		}; // while (sortval2[j+1] < thredis) end
		
	} // for i end
	
	fclose(file);
//	fclose (file2);
	
	// ---------------------------------------------------------------------
	// create .apo files for visualizing co-localization
	// ---------------------------------------------------------------------
	
	filename = dfile_output + ".coexpress.apo";
	file = fopen(filename.c_str(), "wt");
	
    
	unsigned char *mycolor = new unsigned char [num_cell*3]; // color of cells to be rendered
    
	unsigned char colorval[60] = {255, 0, 0, //red
		0, 255, 0, //green
		0, 0, 255, //blue
		255, 255, 0, //yellow
		255, 0, 255, //purple
		0, 255, 255, //cyan
		0, 0, 0, //black
		128, 128, 255, 
		128, 255, 128, 
		255, 128, 128, 
		128, 196, 0, 
		0, 128, 196,
		196, 0, 128,
		128, 0, 196,
		0, 196, 128,
		196, 128, 0,
		64, 128, 128,
		128, 64, 128,
		128, 128, 64,
        128, 196, 255};
	
	
	
	int num =0;
	for (i=0; i<atlas_apo_filelist.size(); i++)
	{
		
		for (j=0; j<num_each_apo[i]; j++)
		{
			mycolor[(num+j)*3] = colorval[i*3];
			mycolor[(num+j)*3+1] = colorval[i*3+1];
			mycolor[(num+j)*3+2] = colorval[i*3+2];
		}
		num += num_each_apo[i];
	}
    
    
	for (i=0; i<num_cell; i++)
	{
		if (coexpress_flg[i] == 1)	
		{
			mycolor[i*3] = mycolor[i*3+1] = mycolor[i*3+2] = 255;
		}
	}
	
	for (i=0; i<num_cell; i++)
	{
        
		fprintf(file, "%ld,%ld,%s,%s,%d,%d,%d,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,%d,%d,%d\n",
				i,i,qPrintable(atlas_apo_combined[i].at(2)), qPrintable(atlas_apo_combined[i].at(3)), atoi(qPrintable(atlas_apo_combined[i].at(4))), atoi(qPrintable(atlas_apo_combined[i].at(5))), atoi(qPrintable(atlas_apo_combined[i].at(6))), 
				atof(qPrintable(atlas_apo_combined[i].at(7))), atof(qPrintable(atlas_apo_combined[i].at(8))), atof(qPrintable(atlas_apo_combined[i].at(9))), atof(qPrintable(atlas_apo_combined[i].at(10))), mycolor[i*3], mycolor[i*3+1], mycolor[i*3+2]);
		
	}
	
	
	fclose (file);
	
	
	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
	if (sortval2) {delete []sortval2; sortval2 = 0;}
	if (celldis) {delete []celldis; celldis = 0;}
	if (prediction) {delete []prediction; prediction = 0;}
	if (cellMean) {delete []cellMean; cellMean =0;}
	if (coexpress_flg) {delete []coexpress_flg; coexpress_flg = 0;}
	
	if (num_each_apo) {delete []num_each_apo; num_each_apo=0;}
	if (mycolor) {delete []mycolor; mycolor = 0;}
	
	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
	//	if (atlas_apo_combined) {delete []atlas_apo_combined; atlas_apo_combined = 0;}
	//	if (markerName) {delete []markerName; markerName =0;}
	//	if (markerName) {delete []markerName; markerName=0;}
	
}



// *************************************************************************************
// The following codes are temporary and the algorithm do not work very well
// *************************************************************************************

void avevar(float data[], unsigned long n, float *ave, float *var)
{
    unsigned long j;
    float s, ep;
    
    for (*ave=0.0, j=0; j<n; j++) *ave += data[j];
    *ave /= n;
    *var=ep=0;
    for (j=0; j<n; j++) 
    {
        s=(float)data[j] - (*ave);
        ep +=s;
        *var += s*s;
    }
    *var=(*var - ep*ep/n)/(n-1);
}

void tutest(float data1[], unsigned long n1, float data2[], unsigned long n2, float *t, float *prob)
{
    float var1,var2,se,df,ave1,ave2;
    
    avevar(data1,n1,&ave1,&var1);
    avevar(data2,n2,&ave2,&var2);
    
    // debug
	
    se = sqrt(var1/n1+var2/n2);
    //  printf("ave1 = %f, var1 = %f, ave2 = %f, var2 = %f, se = %f\n", ave1, var1, ave2, var2, se); 
    
    if (se!=0.0){
        *t = (ave1-ave2)/se;
        df = SQR(var1/n1+var2/n2)/(SQR(var1/n1)/(n1-1)+SQR(var2/n2)/(n2-1));
    }
    else{
        *t = 0.0;
        df = 1; // 1=0/0;  or maybe df should be n1+n2-2;
    }
    
    *prob = betai(0.5*df,0.5,df/(df+SQR(*t)));
    return;
}



//// detect co-localized cells using T-test, and list the potential candidates for further biological tests
//// assumption: if two cells express the same marker, they cannot be the same cell, thus cannot be co-localized
//// temporary, not used 
//
//void coLocalizationDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist,  vector <string> ref_gene_cellnamelist, bool *markerTag, 
//                             float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd,
//                             int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, int *ref_gene_idx, float threratio, int *observed, int *expected)
//{
//    
//	int i,j, k,m;
//	int markernum = marker_gene_info.markernum()+1;
//	
//	int *colocal_candidate = 0;
//	float *pvalueMatrix = 0; 
//	int collocalCellNum = -1;
//	
//	string filename = dfile_outputdir + "colocalizedCells.txt";
//	printf("%s\n", filename.c_str());
//	
//	FILE *file = fopen(filename.c_str(),"wt");
//    
//    //	fprintf(file, "cell name, candidates, pvalue_x, pvalue_y, pvalue_z, pvalue_x+pvalue_y+pvalue_z\n\n");
//    
//	float *data1_x=0, *data2_x=0, *data1_y=0, *data2_y=0, *data1_z=0, *data2_z=0;
//	unsigned long n1=0, n2=0;
//	float t_x, t_y, t_z, prob_x, prob_y, prob_z;
//    
//	// co-localization detection
//	
//    //	for (i=0; i<dim_marker_gene[1]; i++)
//    //		printf("i=%d, %d\n", i, ref_gene_idx[i]);
//    
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//        
//		n1 = 0;
//		colocal_candidate = new int [dim_marker_gene[1]];
//		pvalueMatrix = new float [dim_marker_gene[1]*4]; //x,y,z pvalues and x+y+z pvalue
//        
//		data1_x = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//		data1_y = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//		data1_z = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//        
//		int candcnt = 0;
//        
//        
//		// assign values to data1_x, data1_y, data1_z
//        
//		for (m=0; m<dim_marker_gene[2]; m++)
//		{
//			int p = m*dim_marker_gene[1]*3+i*3;
//			if (!((markerGeneCellArrayPos[p]==0)&&(markerGeneCellArrayPos[p+1]==0)&&(markerGeneCellArrayPos[p+2]==0)))
//			{
//				data1_x[n1] = markerGeneCellArrayPos[p];
//				data1_y[n1] = markerGeneCellArrayPos[p+1];
//				data1_z[n1] = markerGeneCellArrayPos[p+2];
//				n1++;
//				
//			}
//		} // for m end
//        
//		// add data in refGeneCellArrayPos
//		for (m=0; m<dim_ref_gene[2]; m++)
//		{
//			if (ref_gene_idx[i]>0)
//			{
//				int p = m*dim_ref_gene[1]*3+ref_gene_idx[i]*3;
//				if (!((refGeneCellArrayPos[p]==0)&&(refGeneCellArrayPos[p+1]==0)&&(refGeneCellArrayPos[p+2]==0)))
//				{
//					data1_x[n1] = refGeneCellArrayPos[p];
//					data1_y[n1] = refGeneCellArrayPos[p+1];
//					data1_z[n1] = refGeneCellArrayPos[p+2];
//					n1++;
//                    
//				}
//			}
//            
//		} // for m end
//        
//		// consider the second cell
//        //		for (j=i+1; j<dim_marker_gene[1]; j++) //if a and b are colocalized, only list a, b; not necessary to list b, a
//		for (j=0; j<dim_marker_gene[1]; j++) //if a and b are colocalized, only list a, b; not necessary to list b, a		
//		{
//            
//			k = 0;
//			n2 = 0;
//			
//			// if cell i and cell j express the same marker, they can't be potentially the same co-localized cell, since two names are given to them in that marker stacks 
//			while (k<markernum)
//			{ 
//				if (!((markerTag[i*markernum+k]==1)&&(markerTag[j*markernum+k]==1)))
//					k++;
//				else
//					break;
//			}
//			
//			if (k>=markernum) // cell i and cell j do not express the same marker gene
//			{
//                
//				// generate vectors for t-test, which test if two normal distribution are potentially the same
//				
//				data2_x = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//				data2_y = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//				data2_z = new float [dim_marker_gene[2]+dim_ref_gene[2]];
//				
//				// assign values to data2_x, data2_y, data2_z
//				
//				// add data in markerGeneCellArrayPos
//				for (m=0; m<dim_marker_gene[2]; m++)
//				{
//                    
//					int p = m*dim_marker_gene[1]*3+j*3;
//					if (!((markerGeneCellArrayPos[p]==0)&&(markerGeneCellArrayPos[p+1]==0)&&(markerGeneCellArrayPos[p+2]==0)))
//					{
//						data2_x[n2] = markerGeneCellArrayPos[p];
//						data2_y[n2] = markerGeneCellArrayPos[p+1];
//						data2_z[n2] = markerGeneCellArrayPos[p+2];
//						n2++;
//                        
//					}
//					
//				} // for m end
//				
//				// add data in refGeneCellArrayPos
//				for (m=0; m<dim_ref_gene[2]; m++)
//				{
//                    
//					// assign values to data2_x, data2_y, data2_z		
//					if (ref_gene_idx[j]>0)
//					{
//						int p = m*dim_ref_gene[1]*3+ref_gene_idx[j]*3;
//						if (!((refGeneCellArrayPos[p]==0)&&(refGeneCellArrayPos[p+1]==0)&&(refGeneCellArrayPos[p+2]==0)))
//						{
//							data2_x[n2] = refGeneCellArrayPos[p];
//							data2_y[n2] = refGeneCellArrayPos[p+1];
//							data2_z[n2] = refGeneCellArrayPos[p+2];
//							n2++;
//						}
//					}
//					
//				} // for m end
//                
//                //				if ((i==226)&&((j==723)||(j==724)||(j==725)||(j==726)))
//                //				if ((i==4)&&(j==951))
//                //
//                //				{
//                //					printf("n1 = %d, n2 = %d\n", n1,n2);
//                //					for (int ss=0; ss<n1; ss++)
//                //						printf("data1: %f, %f, %f\n", data1_x[ss], data1_y[ss], data1_z[ss]);
//                //
//                //					for (int ss=0; ss<n2; ss++)
//                //						printf("data2: %f, %f, %f\n", data2_x[ss], data2_y[ss], data2_z[ss]);
//                //				
//                //					printf("mean1 = %f, %f, %f\n", cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2]);
//                //					printf("mean2 = %f, %f, %f\n", cellMean[j*3], cellMean[j*3+1], cellMean[j*3+2]);
//                //					
//                //					printf("\n");
//                //					
//                //				}
//                //				
//				if ((n1>1)&&(n2>1))
//				{
//                    
//					tutest(data1_x, n1, data2_x, n2, &t_x, &prob_x);
//					tutest(data1_y, n1, data2_y, n2, &t_y, &prob_y);
//					tutest(data1_z, n1, data2_z, n2, &t_z, &prob_z);
//                    
//                    //					if ((i==226)&&((j==723)||(j==724)||(j==725)||(j==726)))
//                    //					if ((i==4)&&(j==951))
//                    //						{
//                    //							printf("%d, %d, %f, %f, %f, %f, %f, %f\n", i, j, t_x, t_y, t_z, prob_x, prob_y, prob_z);
//                    //							
//                    //						}
//                    
//                    //	for debug purpose			
//                    //					if (((cellnamelist.at(i).substr(0, 4).compare("ERP2")==0) && (cellnamelist.at(j).substr(0, 4).compare("MRP2")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP1")==0) && (cellnamelist.at(j).substr(0, 4).compare("IRP1")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP3")==0) && (cellnamelist.at(j).substr(0, 4).compare("IRP3")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP4")==0) && (cellnamelist.at(j).substr(0, 4).compare("IRP4")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP5")==0) && (cellnamelist.at(j).substr(0, 4).compare("IRP5")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP1")==0) && (cellnamelist.at(j).substr(0, 4).compare("HRP1")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP3")==0) && (cellnamelist.at(j).substr(0, 4).compare("HRP3")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP4")==0) && (cellnamelist.at(j).substr(0, 4).compare("HRP4")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("MRP5")==0) && (cellnamelist.at(j).substr(0, 4).compare("HRP5")==0))||
//                    //						((cellnamelist.at(i).substr(0, 3).compare("EU1")==0) && (cellnamelist.at(j).substr(0, 3).compare("MU1")==0))||
//                    //						((cellnamelist.at(i).substr(0, 3).compare("EU2")==0) && (cellnamelist.at(j).substr(0, 3).compare("MU2")==0))||
//                    //						((cellnamelist.at(i).substr(0, 3).compare("EU3")==0) && (cellnamelist.at(j).substr(0, 3).compare("MU3")==0))||
//                    //						((cellnamelist.at(i).substr(0, 3).compare("EU4")==0) && (cellnamelist.at(j).substr(0, 3).compare("MU4")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("HEW1")==0) && (cellnamelist.at(j).substr(0, 4).compare("IEW1")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("HEW2")==0) && (cellnamelist.at(j).substr(0, 4).compare("IEW2")==0))||
//                    //						((cellnamelist.at(i).substr(0, 4).compare("HEW3")==0) && (cellnamelist.at(j).substr(0, 4).compare("IEW3")==0)))
//                    //							printf("%s, %s, %f, %f, %f, %f, %f, %f\n", cellnamelist.at(i).c_str(), cellnamelist.at(j).c_str(), t_x, t_y, t_z, prob_x, prob_y, prob_z);
//                    
//                    //					if ((prob_x>0.2)&&(prob_y>0.2)&&(prob_z>0.2)) //potentially i and j can be co-localized, 20091119 change threshold						
//                    //					if ((prob_x>0.6)&&(prob_y>0.6)&&(prob_z>0.6)) //potentially i and j can be co-localized, 20090825 fix bugs
//					if ((prob_x>0.1)&&(prob_y>0.1)&&(prob_z>0.1)) //potentially i and j can be co-localized, 20091119 change threshold											
//					{
//                        
//						// determine the real co-localization cells from candidate list
//						colocal_candidate[candcnt] = j;
//                        
//						int tmp = candcnt*4;
//						pvalueMatrix[tmp] = prob_x;
//						pvalueMatrix[tmp+1] = prob_y;
//						pvalueMatrix[tmp+2] = prob_z;
//						pvalueMatrix[tmp+3] = prob_x + prob_y + prob_z;
//                        
//						candcnt++;
//						// find group of cells in which only one cell can be the colocalized cell with cell i
//					}
//                    
//				} //if ((n1>1)&&(n2>1)) end
//				
//                //				printf("i=%d, j=%d, %k=%d\n", i,j, k);
//				
//				if (data2_x) {delete []data2_x; data2_x=0;}
//				if (data2_y) {delete []data2_y; data2_y=0;}
//				if (data2_z) {delete []data2_z; data2_z=0;}
//                
//			} // if (k>markernum) end
//            
//		} //for j end
//		
//		if (candcnt>0)
//		{
//            
//			collocalCellNum++;
//            
//			
//			int tt = candcnt;
//            
//			// write to file
//			fprintf(file, "cellname:  %s,\t mean_x=%f,\t mean_y=%f,\t mean_z=%f,\t std_x=%f,\t std_y=%f,\t std_z=%f\n", cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2]);
//			
//			
//			for (j=0; j<tt; j++) // only print the top 3 with the biggest p values, i.e., most unlikely to be different cells
//                
//			{
//				fprintf(file, "Candidate: %s,\t mean_x=%f,\t mean_y=%f,\t mean_z=%f,\t std_x=%f,\t std_y=%f,\t std_z=%f\n", cellnamelist.at(colocal_candidate[j]).c_str(), 
//                        cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//                        cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2]); 
//				fprintf(file, "                       \t pvalue_x=%f,\t pvalue_y=%f,\t pvalue_z=%f,\t summed pvalue=%f\n", pvalueMatrix[j*4], pvalueMatrix[j*4+1], pvalueMatrix[j*4+2], pvalueMatrix[j*4+3]);
//			}
//			fprintf(file, "\n\n");
//            
//            //			// write to file
//            //			fprintf(file, "%s:                ", cellnamelist.at(i).c_str());
//            //			fprintf(file, "%f,   %f,   %f,   %f,   %f,   %f\n", cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2]);
//            //
//            //			
//            //			int tt = candcnt;
//            //			
//            //			for (j=0; j<tt; j++) // only print the top 3 with the biggest p values, i.e., most unlikely to be different cells
//            //
//            //			{
//            //				int idx = j;
//            ////				fprintf(file, "          %s,   %f,   %f,   %f,   %f\n", cellnamelist.at(colocal_candidate[j]).c_str(),pvalueMatrix[j*4], pvalueMatrix[j*4+1], pvalueMatrix[j*4+2], pvalueMatrix[j*4+3]);
//            //				fprintf(file, "          %s,   %f,   %f,   %f,   %f,   %f,   %f\n", cellnamelist.at(colocal_candidate[j]).c_str(), cellMean[colocal_candidate[j]*3], cellMean[colocal_candidate[j]*3+1], cellMean[colocal_candidate[j]*3+2], 
//            //				                                                                    cellStd[colocal_candidate[j]*3], cellStd[colocal_candidate[j]*3+1], cellStd[colocal_candidate[j]*3+2]); 
//            //				fprintf(file, "                     %f,   %f,   %f,   %f\n", pvalueMatrix[j*4], pvalueMatrix[j*4+1], pvalueMatrix[j*4+2], pvalueMatrix[j*4+3]);
//            //			}
//            //			
//		}
//		
//		if (data1_x) {delete []data1_x; data1_x=0;}
//		if (data1_y) {delete []data1_y; data1_y=0;}
//		if (data1_z) {delete []data1_z; data1_z=0;}
//        
//		if (colocal_candidate) {delete []colocal_candidate; colocal_candidate = 0;}
//		if (pvalueMatrix) {delete []pvalueMatrix; pvalueMatrix = 0;}
//		
//	} // for i end
//	
//	printf("Number of cells that can find potentially colocalized cells is: %d", collocalCellNum);
//	
//	fclose(file);
//	
//}


////detect co-localized cells based on mean and standard deviation of cell positions
//void coLocalizationDetection(MarkerGeneInfo marker_gene_info, vector <string> cellnamelist, bool *markerTag, 
//							 float *markerGeneCellArrayPos, float *refGeneCellArrayPos, float *cellMean, float *cellStd, float *cellvol,
//							 int *dim_marker_gene, int *dim_ref_gene, string dfile_outputdir, int *ref_gene_idx, float threratio, int *observed, int *expected)
//{
//	
//	long int i,j, k,m, n;
//	int markernum = marker_gene_info.markernum()+1;
//	
//	int collocalCellNum = -1;	
//	float thredis = 1000; // distance threshold which determines potential cell colocalization
////	float thredis = 6; // distance threshold which determines potential cell colocalization
//	int topn = 8; // only consider the topn candidates
//
//	int *colocal_candidate = new int [dim_marker_gene[1]]; 	
//	float *celldis = new float [dim_marker_gene[1]*dim_marker_gene[1]];
//	
//	float *sortidx2 = new float [dim_marker_gene[1]+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
//	float *sortval2 = new float [dim_marker_gene[1]+1];
//	
//	// read in ground-truth colocalization and adjacent cells
//	unsigned char *groundtruth = new unsigned char [dim_marker_gene[1]*dim_marker_gene[1]];
//	char tmps[1024];
//	string cell_line;
//	size_t pos_blank;
//	
//	// -----------------------------
//	// load ground truth file
//	//------------------------------
//	ifstream ifs ("/Users/longf/work/fly_oregan/data/final_data/co_localization_ground_truth/coloc_cell_new_list.txt", ifstream::in); // hard code the path
//		
//	long linecount = 0;
//	
//	while (!ifs.eof())
//	{
//		ifs.getline(tmps, 1024);
//		cell_line = tmps;
//		int s_start = 0;
//		int s_length = 0;
//		
//		// parse the information in each line, get the two strings in each line
//
//		string itemstr1, itemstr2;
//		
//		
//		for (i=0; i<cell_line.size(); i++)
//		{
//			if (cell_line.at(i)==',') // the first item
//			{
//				// process the current item
//				itemstr1 = cell_line.substr(s_start, s_length);
//				s_start = s_start+s_length+1; 
//				s_length = 0;
//			}
//			else if (i==(cell_line.size()-1)) // the second item
//			{
//				s_length++;
//				itemstr2 = cell_line.substr(s_start, s_length);
//			}
//			else
//				s_length++;
//		}
//
//		printf("linecnt = %d, itemstr1 = %s, itemstr2 = %s\n", linecount, itemstr1.c_str(), itemstr2.c_str());
//		
//		if ((itemstr1.compare("")==0) || (itemstr2.compare("")==0))
//			break;
//		
//		linecount++;	
//		
//		// search full names of cells, assign values to the grountruth matrix
//		
//		unsigned char item = 0;
//				
//		// test if the current cell belongs to those annotated
//		for (m=0; m<dim_marker_gene[1]; m++)
//		{
//			//			printf("%s\n",cellnamelist.at(i).c_str());
//			
//			pos_blank = cellnamelist.at(m).find(" ");
//			string cellname_prefix = cellnamelist.at(m).substr(0, pos_blank);
//			string cellname_surfix = cellnamelist.at(m).substr(pos_blank+1, cellnamelist.at(m).length()-1);
//			
//			if (itemstr1.compare(cellname_prefix)==0)
//			{
//				string searchstr = itemstr2 + " " + cellname_surfix;
//				for (n=0; n<dim_marker_gene[1]; n++)
//				{
//					printf("%s, %s\n", searchstr.c_str(), cellnamelist.at(n).c_str());
//					if (searchstr.compare(cellnamelist.at(n))==0)
//					{
//						printf("%s, %s, %s\n", itemstr1.c_str(), searchstr.c_str(), cellnamelist.at(n).c_str());						
//						printf("m=%d, n=%d\n", m,n);
//						groundtruth[m*dim_marker_gene[1]+n] = 1; //colocalized cells
//						break;
//					}
//				}
//				
//			}		
//		}
//	} // while end
//		
//	ifs.close();
//		
//		
//	// -----------------------------------------	 
//	// compute pair-wise distance between cells
//	// -----------------------------------------	 
//
//	string filename = dfile_outputdir + "colocalizedCells.txt";
//	printf("%s\n", filename.c_str());
//	FILE *file = fopen(filename.c_str(),"wt");
//	
//	int fpos=0, fneg=0, tpos=0, tneg=0; // number of false positive, false negative, true positive, true negative
//	
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//		
//		int nn = i*3;
//		if (!((cellMean[nn]==0)&&(cellMean[nn+1]==0)&&(cellMean[nn+2]==0))&&(((float)observed[i]/(float)expected[i])>threratio)) 		
//		{
//			for (j=0; j<dim_marker_gene[1]; j++)
//			{
//				
//				int mm = j*3;
//				if (!((cellMean[mm]==0)&&(cellMean[mm+1]==0)&&(cellMean[mm+2]==0))&&(((float)observed[j]/(float)expected[j])>threratio)) 		
//					celldis[i*dim_marker_gene[1]+j] = sqrt((cellMean[i*3]-cellMean[j*3])*(cellMean[i*3]-cellMean[j*3]) 
//														   + (cellMean[i*3+1]-cellMean[j*3+1])*(cellMean[i*3+1]-cellMean[j*3+1]) 
//														   + (cellMean[i*3+2]-cellMean[j*3+2])*(cellMean[i*3+2]-cellMean[j*3+2]));
//				else
//					celldis[i*dim_marker_gene[1]+j] = 9999;
//					
//			}
//		}
//		else
//			for (j=0; j<dim_marker_gene[1]; j++)
//			{
//				celldis[i*dim_marker_gene[1]+j] = 9999;
//			}
//	}
//
//		
//	// ----------------------------------------------------
//	// find for each cell, the cells that share the prefix
//	// this is to generate the ratio value (see below)
//	// ----------------------------------------------------
//	short int *cell_same_prefix_idx = new short int [dim_marker_gene[1]*6];
//	
//	for (i=0; i< dim_marker_gene[1]; i++)
//	{	
//		long ii = i*3;
//
//		// initialize
//		for (j=0; j<6; j++)
//		{
//			printf("%d, %d\n", i,j);
//			cell_same_prefix_idx[i*6+j] = -1;
//		}
//		
//		if (!((cellMean[ii]==0)&&(cellMean[ii+1]==0)&&(cellMean[ii+2]==0)))
//		{
//			size_t pos_blankk = cellnamelist.at(i).find(" ");
//			string cellname_prefixx = cellnamelist.at(i).substr(0, pos_blankk);
//			
//		//	unsigned char num1 = 0;
//			
//			cell_same_prefix_idx[i*6] = i;
//	//		printf("%s, ", cellnamelist.at(cell_same_prefix_idx[i*6]).c_str());
//			
//			unsigned char cc = 0;
//			
//			for (j=0; j<dim_marker_gene[1]; j++)
//			{
//				if (j!=i)
//				{
//					size_t pos_blankk2 = cellnamelist.at(j).find(" ");
//					string tmp = cellnamelist.at(j).substr(0, pos_blankk2);
//					
//	//				if (cellname_prefixx.compare(tmp)==0)
//					long jj = j*3;
//					if ((cellname_prefixx.compare(tmp)==0) && (!((cellMean[jj]==0)&&(cellMean[jj+1]==0)&&(cellMean[jj+2]==0))))
//					{
//		//				num1 += 1;					
//						cc++;				
//						cell_same_prefix_idx[i*6 + cc] = j;
//	//					printf("%s, ", cellnamelist.at(cell_same_prefix_idx[i*6+cc]).c_str());
//						
//					}
//				}
//				
//			}
//		}
//		printf("\n");
//	}
//	
//	// generate the txt file indicating the top 5 closet cells for each cell
//	
//	float *sortallidx = new float [dim_marker_gene[1]+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
//	float *sortallval = new float [dim_marker_gene[1]+1];
//	
//	sortallval[0] = -999; // the 0th element is not sorted in sort2			
//	sortallidx[0] = -999;
//
//	FILE *f = fopen("nearest_cell_list.txt","wt");
//	
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//		for (j=0; j<dim_marker_gene[1]; j++)
//		{	
//			sortallval[j+1] = celldis[i*dim_marker_gene[1]+j];
//			sortallidx[j+1] = j;
//		}
//		
//		sort2(dim_marker_gene, sortallval, sortallidx);// the first element is not sorted
//
//		fprintf(file, "cellname:  %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d\n", 
//				cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2], cellvol[i], observed[i], expected[i]);
//		
//
//		for (j=1;j<=5;j++)
//		{
//			
//			
//			
//			string tag;
//			
//			if ((groundtruth[i*dim_marker_gene[1] + (int)sortallidx[j]]==1) & (prediction[i*dim_marker_gene[1] + (int)sortallidx[j]]==1)) tag = "true pos";
//			if ((groundtruth[i*dim_marker_gene[1] + (int)sortallidx[j]]==1) & (prediction[i*dim_marker_gene[1] + (int)sortallidx[j]]==0)) tag = "false neg";
//			if ((groundtruth[i*dim_marker_gene[1] + (int)sortallidx[j]]==0) & (prediction[i*dim_marker_gene[1] + (int)sortallidx[j]]==1)) tag = "false pos";
//			if ((groundtruth[i*dim_marker_gene[1] + (int)sortallidx[j]]==0) & (prediction[i*dim_marker_gene[1] + (int)sortallidx[j]]==0)) tag = "true neg";
//			
//			fprintf(file, "Candidate: %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, distance=%f, size=%f, ratio=%d/%d, observed=%d, expected=%d, type=%s\n", 
//					cellnamelist.at((int)sortallidx[j]).c_str(),  
//					cellMean[(int)sortallidx[j]*3], cellMean[(int)sortallidx[j]*3+1], cellMean[(int)sortallidx[j]*3+2], 
//					cellStd[(int)sortallidx[j]*3], cellStd[(int)sortallidx[j]*3+1], cellStd[(int)sortallidx[j]*3+2], sortval[j], 
//					cellvol[(int)sortallidx[j]], realnum_j, expectednum_j, observed[(int)sortallidx[j]], expected[(int)sortallidx[j]],tag.c_str()); 
//		}
//		
//	}
//	
//	fclose(f);
//		
//	if (sortallidx) {delete []sortallidx; sortallidx=0;}
//	if (sortallval) {delete []sortallval; sortallval=0;}
//	
//	
//	// --------------------------------------------------------
//	// compute prediction, i.e., canndidate co-localized cells
//	// --------------------------------------------------------
//	
//	unsigned char *prediction = new unsigned char [dim_marker_gene[1]*dim_marker_gene[1]];
//		
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//		int nn = i*dim_marker_gene[1];
//		int candcnt = 0;
//
//		size_t len_i = cellnamelist.at(i).size();
//		string surfix_i = cellnamelist.at(i).substr(len_i-2, len_i-1);
//		
//		// select those whose mean distance with respect to cell i is less than thredis
//		for (j=0; j<dim_marker_gene[1]; j++)
//		{
//			
//			k = 0;
//			int n2 = 0;
//			
//			// if cell i and cell j express the same marker, they can't be potentially the same co-localized cell, since two names are given to them in that marker stacks 
//			while (k<markernum)
//			{ 
//				if (!((markerTag[i*markernum+k]==1)&&(markerTag[j*markernum+k]==1)))
//					k++;
//				else
//					break;
//			}
//			
//			if (k>=markernum) // cell i and cell j do not express the same marker gene
//			{
//				size_t len_j = cellnamelist.at(j).size();
//				string surfix_j = cellnamelist.at(j).substr(len_j-2, len_j-1);
//				
//				if (((surfix_i.compare("L")==0)&&(surfix_j.compare("L")==0)) || ((surfix_i.compare("R")==0)&&(surfix_j.compare("R")==0)) 
//				    || ((surfix_i.compare("L")!=0)&&(surfix_i.compare("R")!=0)&&(surfix_j.compare("L")!=0)&&(surfix_j.compare("R")!=0))) // Chris L/R rule (rule 1 and 2)
//				{
//					if ((celldis[nn+j]<thredis)&&(j!=i)) // distance rule
//					{
//	//					colocal_candidate[candcnt] = j;
//	//					candcnt++;
//						prediction[i*dim_marker_gene[1]+j] = 1; // this value is temporary, will be changed by other rules
//						
//	//					// detect the rate of false positive, true positive, true negative
//	//					if (groundtruth[i*dim_marker_gene[1]+j] == 1)
//	//						tpos += 1;
//	//					else
//	//						fpos += 1;
//						
//					}
//					else
//					{
//						prediction[i*dim_marker_gene[1]+j]=0; 
//						// detect the rate of false negative, true negative
//						
//	//					if (groundtruth[i*dim_marker_gene[1]+j] == 1)
//	//						fneg += 1;
//	//					else
//	//						tneg += 1;
//						
//					}
//				}
//			}
//		} // for j end
//	} // for i end
//
//
//	for (i=0; i<dim_marker_gene[1]; i++)
//	{
//	
//		int candcnt = 0;
//
//		// save colocal_candidate, tpos, fpos, tneg, fneg
//		for (j=0; j<dim_marker_gene[1]; j++)
//		{
//			
//			if (prediction[i*dim_marker_gene[1]+j]==1)
//			{
//					colocal_candidate[candcnt] = j;
//					candcnt++;
//				
//					if (groundtruth[i*dim_marker_gene[1]+j] == 1)
//						tpos += 1; // true positive by comparing the ground truth
//					else
//						fpos += 1; // false positive by comparing the ground truth
//				
//			}
//			else
//			{
//				if (groundtruth[i*dim_marker_gene[1]+j] == 1)
//					fneg += 1; // false negative by comparing ground truth
//				else
//					tneg += 1; // true negative by comparing ground truth
//				
//			}
//		}
//			
//		if (candcnt==0)
//		{
//			printf("no candidate cellname = %s\n", cellnamelist.at(i).c_str());
//			fprintf(file, "cellname:  %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d\n", 
//					cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2], cellvol[i], observed[i], expected[i]);
//			
//		}
//
//		
//		if (candcnt>0)
//		{
//		
//			collocalCellNum++;
//			
//			// sort
//			
//			float *sortidx = new float [candcnt+1]; // sort2 does not sort the first element, add an element so that everyone is sorted	
//			float *sortval = new float [candcnt+1];
//			
//			
//			stdev[0] = -999; // the 0th element is not sorted in sort2
//			sortval[0] = -999; // the 0th element is not sorted in sort2			
//			sortidx[0] = -999;
//			
//			for (j=0; j<candcnt; j++)
//			{	
//				int nn = colocal_candidate[j]*3;
//				sortval[j+1] = cellStd[nn] + cellStd[nn+1] + cellStd[nn+2];
//				sortval[j+1] = celldis[i*dim_marker_gene[1]+colocal_candidate[j]];
//				sortidx[j+1] = j;
//			}
//			
//			sort2(candcnt, sortval, sortidx);// the first element is not sorted
//			
//			unsigned char expectednum_i = 0;
//			unsigned char realnum_i = 0;
//
//			for (m=0; m<6; m++) 
//			{
//				short int idx1 = cell_same_prefix_idx[i*6+m];
//				if (idx1>=0)
//					realnum_i++;
//			}
//				
//
//			if (realnum_i>0)
//			{
//				// write to file
//				//			fprintf(file, "cellname:  %s,\t mean_x=%f,\t mean_y=%f,\t mean_z=%f,\t std_x=%f,\t std_y=%f,\t std_z=%f,\t size=%f, \t observed=%d,\t expected=%d\n", 
//				//					cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2], cellvol[i], observed[i], expected[i]);
//				
//				fprintf(file, "cellname:  %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d\n", 
//						cellnamelist.at(i).c_str(), cellMean[i*3], cellMean[i*3+1], cellMean[i*3+2], cellStd[i*3], cellStd[i*3+1], cellStd[i*3+2], cellvol[i], observed[i], expected[i]);
//				
//				
//				
//				if (candcnt>topn)
//					candcnt = topn;
//				
//				
//				for (j=1; j<=candcnt; j++) 
//				{
//	//				printf("%d, %d\n", j, candcnt);
//					
//					// determine prediction ratio
//					unsigned char expectednum_j = 0;
//					unsigned char realnum_j = 0;
//					float totaldistance = 0;
//					
//					unsigned char cnt_T2 = 0, cnt_T3 = 0, cnt_A1 = 0; // number of co-localized cells in each segment
//					for (m=0; m<6; m++) 
//					{
//						short int idx1 = cell_same_prefix_idx[i*6+m];
//						
//						if (idx1>0)
//						{
//							size_t pos_blank1= cellnamelist.at(idx1).find(" ");					
//	//						string cellname_sufix1 = cellnamelist.at(idx1).substr(0, pos_blank1);
//							string cellname_sufix1 = cellnamelist.at(idx1).substr(pos_blank1+1, cellnamelist.at(idx1).length());
//							
//							for (n=0; n<6; n++)
//							{
//								short int idx2 = cell_same_prefix_idx[colocal_candidate[(int)sortidx[j]]*6+n];
//								
//								if (idx2>0)
//								{
//									size_t pos_blank2= cellnamelist.at(idx2).find(" ");					
//	//								string cellname_sufix2 = cellnamelist.at(idx2).substr(0, pos_blank2);
//									string cellname_sufix2 = cellnamelist.at(idx2).substr(pos_blank2+1, cellnamelist.at(idx2).length());
//									
//									if (cellname_sufix1.compare(cellname_sufix2)==0)
//									{
//										expectednum_j++; // number of colocalization in all the 6 hemisegments
//										if (prediction[idx1*dim_marker_gene[1]+idx2]==1)
//										{
//											realnum_j++; // the predicted number of colocalization in all the 6 
//											totaldistance += celldis[idx1*dim_marker_gene[1]+idx2];
//											
//											if(cellname_sufix2.substr(0,2).compare(toupper("A1"))==0)
//											if(cellname_sufix2.substr(0,2).compare("A1")==0)											
//												cnt_A1++;
//
//											if(cellname_sufix2.substr(0,2).compare("T2")==0)
//												cnt_T2++;
//
//											if(cellname_sufix2.substr(0,2).compare("T3")==0)
//												cnt_T3++;
//											
//										}
//										
//									}
//								}
//								
//							}
//						}
//					} //for m end
//					
//					// Chris rule 3
//					if ((realnum_i>=4)&&(realnum_j<3)) // old rule
//					if ((realnum_i>=4)&&(realnum_j<2))
//						prediction[i*dim_marker_gene[1]+colocal_candidate[(int)sortidx[j]]] = 0;
//					
//					if (((realnum_i==2)||(realnum_i==3))&&(realnum_j<2))
//						prediction[i*dim_marker_gene[1]+colocal_candidate[(int)sortidx[j]]] = 0;
//						
//					if (realnum_j==1)
//						prediction[i*dim_marker_gene[1]+colocal_candidate[(int)sortidx[j]]] = 0;
//					
//					//bilateral rule
//					if (((expectednum_j==2)||(expectednum_j==4)||(expectednum_j==6))&&(realnum_j==2)&&((cnt_T3!=2)&&(cnt_T3!=2)&(cnt_T3!=2)))
//						prediction[i*dim_marker_gene[1]+colocal_candidate[(int)sortidx[j]]] = 0;
//						
//									
//					string tag;
//					
//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1) & (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1)) tag = "true pos";
//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1) & (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==0)) tag = "false neg";
//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==0) & (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1)) tag = "false pos";
//					if ((groundtruth[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==0) & (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==0)) tag = "true neg";
//
//					//				fprintf(file, "Candidate: %s,\t mean_x=%f,\t mean_y=%f,\t mean_z=%f,\t std_x=%f,\t std_y=%f,\t std_z=%f, \t distance=%f, \t size=%f, \t %d/%d, \t observed=%d,\t expected=%d,\t type=%s\n", 
//					//						cellnamelist.at(colocal_candidate[(int)sortidx[j]]).c_str(),  
//					//						cellMean[colocal_candidate[(int)sortidx[j]]*3], cellMean[colocal_candidate[(int)sortidx[j]]*3+1], cellMean[colocal_candidate[(int)sortidx[j]]*3+2], 
//					//						cellStd[colocal_candidate[(int)sortidx[j]]*3], cellStd[colocal_candidate[(int)sortidx[j]]*3+1], cellStd[colocal_candidate[(int)sortidx[j]]*3+2], sortval[colocal_candidate[(int)sortidx[j]]], 
//					//						cellvol[colocal_candidate[(int)sortidx[j]]], realnum_j, expectednum_j, observed[colocal_candidate[(int)sortidx[j]]], expected[colocal_candidate[(int)sortidx[j]]],tag.c_str()); 
//					
//					if (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1) 
//						tag = "***POSITIVE***";
//					else
//						tag = "";
//							
//					fprintf(file, "Candidate: %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d,  distance=%f, ratio=%d/%d, %s\n", 
//							cellnamelist.at(colocal_candidate[(int)sortidx[j]]).c_str(),  
//							cellMean[colocal_candidate[(int)sortidx[j]]*3], cellMean[colocal_candidate[(int)sortidx[j]]*3+1], cellMean[colocal_candidate[(int)sortidx[j]]*3+2], 
//							cellStd[colocal_candidate[(int)sortidx[j]]*3], cellStd[colocal_candidate[(int)sortidx[j]]*3+1], cellStd[colocal_candidate[(int)sortidx[j]]*3+2],  
//							cellvol[colocal_candidate[(int)sortidx[j]]], observed[colocal_candidate[(int)sortidx[j]]], expected[colocal_candidate[(int)sortidx[j]]], sortval[j], realnum_j, expectednum_j, tag.c_str()); 
//
//					if (prediction[i*dim_marker_gene[1] +colocal_candidate[(int)sortidx[j]]]==1) 
//					{
//						fprintf(file, "Candidate: %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d,  distance=%f, ratio=%d/%d, %s\n", 
//								cellnamelist.at(colocal_candidate[(int)sortidx[j]]).c_str(),  
//								cellMean[colocal_candidate[(int)sortidx[j]]*3], cellMean[colocal_candidate[(int)sortidx[j]]*3+1], cellMean[colocal_candidate[(int)sortidx[j]]*3+2], 
//								cellStd[colocal_candidate[(int)sortidx[j]]*3], cellStd[colocal_candidate[(int)sortidx[j]]*3+1], cellStd[colocal_candidate[(int)sortidx[j]]*3+2],  
//								cellvol[colocal_candidate[(int)sortidx[j]]], observed[colocal_candidate[(int)sortidx[j]]], expected[colocal_candidate[(int)sortidx[j]]], sortval[j], realnum_j, expectednum_j, tag.c_str()); 
//						
//					}
//					else
//						candcnt--;
//					
//					
//					
//				} // for j end
//	
//				if (sortidx) {delete [] sortidx; sortidx =0;}
//				if (sortval) {delete [] sortval; sortval = 0;}
//				
//									 
//			} // if(realnum_i>0) end
//		} // if cancnt>0
//		
//		// print other top candidates that are close to the cell in consideration, but do not pass the rules
//		
//		
//		sortval2[0] = -999; // the 0th element is not sorted in sort2			
//		sortidx2[0] = -999;
//		
//		for (j=0; j<dim_marker_gene[1]; j++)
//		{	
//			sortval2[j+1] = celldis[i*dim_marker_gene[1]+j];
//			sortidx2[j+1] = j;
//		}
//		
//		sort2(dim_marker_gene[1], sortval2, sortidx2);// the first element is not sorted
//		
//		j=1; 
//		long kk = 1;
//		while (kk<=topn-candcnt)
//		{
//			if ((prediction[i*dim_marker_gene[1]+(int)sortidx2[j]]==0)&&((int)sortidx2[j]!=i))
//			{
//				fprintf(file, "Candidate: %s, mean_x=%f, mean_y=%f, mean_z=%f, std_x=%f, std_y=%f, std_z=%f, size=%f, observed=%d, expected=%d, distance=%f\n", 
//						cellnamelist.at((int)sortidx2[j]).c_str(),  
//						cellMean[(int)sortidx2[j]*3], cellMean[(int)sortidx2[j]*3+1], cellMean[(int)sortidx2[j]*3+2], 
//						cellStd[(int)sortidx2[j]*3], cellStd[(int)sortidx2[j]*3+1], cellStd[(int)sortidx2[j]*3+2], 
//						cellvol[(int)sortidx2[j]], observed[(int)sortidx2[j]], expected[(int)sortidx2[j]], sortval2[j]); 
//				kk++;
//				//						printf("i=%d, j=%d\n", i, j);
//				
//			}
//			j++;
//		}
//		
//		fprintf(file, "\n\n");
//		
//	} // for i end
//
//	
//	fclose(file);
//	
//	printf("true positive = %d, true negative = %d, false positive = %d, false negative = %d, score = %d\n", tpos, tneg, fpos, fneg, tpos+tneg-fpos-fneg);
//	
//	printf("Number of cells that can find potentially colocalized cells is: %d", collocalCellNum);
//
//	if (sortidx2) {delete []sortidx2; sortidx2 = 0;}
//	if (sortval2) {delete []sortval2; sortval2 = 0;}
//	if (colocal_candidate) {delete []colocal_candidate; colocal_candidate = 0;}
//	if (celldis) {delete []celldis; celldis = 0;}
//	if (groundtruth) {delete []groundtruth; groundtruth = 0;}
//	if (prediction) {delete []prediction; prediction = 0;}
//	if (cell_same_prefix_idx) {delete []cell_same_prefix_idx; cell_same_prefix_idx = 0;}
//		
//	
//}





