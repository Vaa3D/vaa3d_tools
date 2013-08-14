// main atlas builder program
// 1) recompute cell infomration in .apo file
// 2) reads annotated cell information 
// 3) does point-based registration, names of cells give the correspondence information
// 4) compute cell statistics and build atlas, output:
//		--- statistics 
//		--- outliear
//		--- atlas file for v3d to view and toggle on/off different markers/neurons
// 5) identify colocalized cells
// 6) combine cells of different name but co-localized, which are assumed to be the same neuron.
//    This step is needed because the intial atlas may contain the same neurons expressing different markers
//    as they name the same neuron in different words. User should provide a txt file indicating the 
//    names of neurons that should be combined

// by Fuhui Long
// latest update: 20090602
// 090605: remove load_ano_file.h as it has been merged into basic_sur_objs.h
//

// example: 
// run -R ./test_atlas_builder/cellnamelist_eve.txt -T ./test_atlas_builder/ps1_en_eve/ch3_eve/ps1_en_eve_10.ano.ano.txt -B 1 -O ./test_atlas_builder/
// run -R ./test_atlas_builder/cellnamelist_eve.txt -B 1 -O ./test_atlas_builder/

// final data of toy_ap_eve, dimm_eve
// run -R /Users/longf/work/fly_oregan/data/final_data/atlas/cellnamelist_eve.txt -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas/
// run -R /Users/longf/work/fly_oregan/data/final_data/atlas/cellnamelist_eve.txt -T /Users/longf/work/fly_oregan/data/final_data/dimm_eve_final/ch3_eve/dimm-eve-7.tif.ano_new.apo -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas/

//20091014
// run -R /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20090710/atlas/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20090710/atlas/
// run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20100308/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20100308/

// run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20100308/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20100308/ -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20100308/10D02_eve_complete/ch1_eve/10D02_eve_4.tif.ano_new.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20100805/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20100805/ -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20100805/10D02_eve/ch1_eve/10D02_eve_4.tif.adjusted.ano_new.apo -E 0.33


// run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20100805/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20100805/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20100805/cellnamelist_eve_noclusters.txt -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20100805/10D02_eve/ch1_eve/10D02_eve_4.tif.adjusted.ano_new.apo -E 0.66

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/cellnamelist_eve_noclusters.txt -E 0.66


//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20101008/cellnamelist_eve_noclusters.txt -E 0.66 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20101008/islet_eve/ch3_eve/islet_fox_eve_13.corrected.raw.ano_new_new.adjust.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve.txt -E 0.5 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110223/islet_eve/ch3_eve/islet_fox_eve_13.corrected.raw.ano_new_new.adjust.apo
//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve_noclusters.txt -E 0.5 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110223/islet_eve/ch3_eve/islet_fox_eve_13.corrected.raw.ano_new_new.adjust.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110223/cellnamelist_eve_acc_pcc_RP2_U1_U2.txt -E 0 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110223/islet_eve/ch3_eve/islet_fox_eve_13.corrected.raw.ano_new_new.adjust.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110720/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110720/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110720/cellnamelist_eve_nocluster.txt -E 0.33 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110720/ps1_en_eve/ch3_eve/ps1_en_eve_7.ano.ano_new_new_new.adjust.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve_nocluster.txt -E 0.33 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110926/ps1_en_eve/ch3_eve/ps1_en_eve_7.ano.ano_new_new_new.adjust.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve.txt  -B 1 -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/ -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve_nocluster.txt -E 0.33 -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110926/ps1_en_eve/ch3_eve/ps1_en_eve_7.ano.ano_new_new_new.adjust.apo -s /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_EVE.apo -t /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/atlas_PS1.apo

//run -R /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve.txt -T /Users/longf/work/fly_oregan/data/final_data/atlas_data_complete_20110926/ps1_en_eve/ch3_eve/ps1_en_eve_7.ano.ano_new_new_new.adjust.apo -E 0.33 -C /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/cellnamelist_eve_nocluster.txt -D /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/coexpress_detection_filelist.txt -O /Users/longf/work/fly_oregan/data/final_data/atlas_20110926/ 


//run 64 bit gdb:
// gdb atlas_builder.app -arch x86_64

// bug report
// 20111102 found the last line in cellnamelist_atlas.txt has strange letters: ÔªøBDL1/bar T2L, but BDL1/bar T2L already included
//

#include "FL_atlasBuilder.h"
#include "converter_pcatlas_data.h"

#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
#include "pointcloud_atlas_io.h"
//#include "../../wano/c++/wano_core.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include <QString>
#include <qstringlist.h>
#undef DEBUG

using namespace std;

extern char *optarg;
extern int optind, opterr;

void printHelp()
{
//	printf("\nUsage: prog_name -R <reference marker cell name file> -M <final cell name list> -T <name of target file for registration> -B -E <cut off ratio> -C <cells used as reference> -O <directory of output file>\n");
	printf("\nUsage: prog_name -R <reference marker cell name file> -T <name of target file for registration> -E <cut off ratio> -C <cells used as reference> -D <file of apo list for co-express detection> -O <directory of output file>\n");	
	printf("[R]			the name of the file that contains annotated marker cell names for registration \n");
	printf("[T]			the name of the target file for registration (optional) \n");
	printf("[E]			threshold ratio determining which cells to be included in the final atlas\n");
	printf("[C]			the name of the file that contains the cell names in reference channel (EVE for example) that are used as control points for registration");
	printf("[D]			name of the file containing the list of atlas .apo files that co-expression detection is needed");
	printf("[O]			the directory of output \n");

//	printf("[r]			recompute cell information in .apo file (optional)\n");
//	printf("[I]			original image for recomputing the cell information in .apo file (optional)\n");
//	printf("[M]			the name of the file listing final cells and cells to be merged based on co-expression file\n");
//	printf("[B]			rebuild tag\n");
//	printf("[s]			subject atlas filename (for co-localization visualization. \n");
//	printf("[t]			target atlas filename (for co-localization visualization. \n");  
	printf("[h]			help\n");
	
	return;
}

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	// -----------------
	// Read input arguments
	// -----------------
	
	string refgene_cellnamelist_filename; // name of the file that contains annoated reference gene cells
	string refgene_controlpoints_cellnamelist_filename; // name of the file that contains the cell names in the reference gene channel that are used as control points for registration. It should be a subset of those listed in refgene_cellnamelist_filename
	string dfile_target; // name of the reference target file for registration
	string dfile_outputdir; // directory of output file
	string test_colocalization_atlas_apo_filename;

	string atlas_subject_filename, atlas_target_filename;
	
//	unsigned char rebuildTag; //tag indicating whether the atlas should be rebuilt or expended, if rebuild, the registration step will be redone, otherwise just load existing file
		
	long int c;
//	static char optstring[] = "hr:R:T:B:O:C:E:M:s:t:";
	static char optstring[] = "h:R:T:E:C:D:O:";
	
	bool recomputeTag = 0;
	float threratio = 0; // default value of threratio
	
	opterr = 0;
	int i,j,k;
	
//	FILE *file;
	
	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;

			case 'R':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -R.\n");
					return 1;
				}
				refgene_cellnamelist_filename = optarg;
				break;

			case 'T':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -T.\n");
					return 1;
				}
				dfile_target = optarg;
				break;

			case 'E':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -E.\n");
					return 1;
				}
				threratio = atof(optarg);
				break;
								
			case 'C':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -C.\n");
					return 1;
				}
				refgene_controlpoints_cellnamelist_filename = optarg;
				break;

			case 'D':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -D.\n");
					return 1;
				}
				test_colocalization_atlas_apo_filename = optarg;
				break;
				
			case 'O':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -O.\n");
					return 1;
				}
				dfile_outputdir = optarg;
				break;
				
//			case 'c':
//				recomputeTag = 1;
//				break;
//
//			case 'B':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -B.\n");
//					return 1;
//				}
//				rebuildTag = atoi(optarg);
//				break;
//
//			case 's':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
//					return 1;
//				}
//				atlas_subject_filename = optarg;
//				break;
//				
//			case 't':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
//					return 1;
//				}
//				atlas_target_filename = optarg;
//				break;
				
				
        }
    }

	printf("%d, %d\n", optind, argc);
	
	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

	string global_cellnamelist_filename = dfile_outputdir + "cellnamelist_atlas.txt"; //name of the cell name list file used to build the atlas	
	string dfile_marker_info = dfile_outputdir + "import.pc_atlas"; //name of the linker file saving the marker information
	string dfile_merge_cells = dfile_outputdir + "cellname_type_mapping.txt"; // name of the file containing the final cell names and their old correspondence
	string dfile_force_add = dfile_outputdir + "forceAddCells.txt"; //name of the file containing cells that must be included in the atlas	
	string dfile_force_add_final = dfile_outputdir + "forceAddCells_final.txt"; //name of the file containing cells that must be included in the atlas

	vector <string> cellTypeNames;
	
	cellTypeNames.push_back("interneuron");
	cellTypeNames.push_back("motoneuron");
	cellTypeNames.push_back("secretory");
	cellTypeNames.push_back("galia");
	
	
	printf("%s\n", dfile_marker_info.c_str());
	
	// ---------------------------------
	// parse dfile_marker_info 
	// ---------------------------------
	MarkerGeneInfo marker_gene_info;
	int markerNum; //number of new markers
	apoAtlasLinkerInfoAll apoinfo; // struct apoAtlasLinkerInfoAll is defined in converter_pcatlas_data.h	
	loadPointCloudAtlasInfoListFromFile(dfile_marker_info.c_str(), apoinfo);
	printApo(apoinfo);
	convert_FLAtlasBuilderInfo_from_apoAtlasLinkerInfo(marker_gene_info, apoinfo);	

	markerNum = marker_gene_info.markernum();
	marker_gene_info.print();

	printf("parse linker file finish\n");
	

	// ---------------------------------------------------------------
	// generate cellnamelist_atlas.txt
	// ---------------------------------------------------------------
	QStringList cellNamesAll, namelist;
	string cellnamelist_filename = dfile_outputdir + "cellnamelist_EVE.txt";
	int num;

	readLines(cellnamelist_filename, namelist, num);	
	cellNamesAll << namelist;
	
	printf("finish reading cellnamelist_eve.txt\n");
	
	int mycnt = 0;
	for (i=0; i<markerNum; i++)
	{
		printf("i=%d\n", i);
		
		string cellnamelist_filename = dfile_outputdir + "cellnamelist_" + marker_gene_info.marker_stacks.at(i).markername+".txt";
		printf("%s\n", cellnamelist_filename.c_str());
		
		readLines(cellnamelist_filename, namelist, num);	
		cellNamesAll << namelist;

		mycnt += num;
		
		for (j=1; j<num; j++)
		{
			printf("%d, %s\n", j, qPrintable(namelist.at(j-1)));
		}
	}

	printf("mycnt= %d\n", mycnt);
	printf("finish reading cellnamelist of marker cells\n");
	
	writeCellNameList2File((const QString)(global_cellnamelist_filename.c_str()), uniqueCellNameList((const QStringList)cellNamesAll));

	//writeCellNameList2File((const QString)(global_cellnamelist_filename.c_str()), (const QStringList)cellNamesAll);
	
	printf("finish writing cellnamelist of the atlas\n");
	

	// ---------------------------------------------------------------
	// read cell name file, get the number of annotated cells in total
	// ---------------------------------------------------------------
//	string dfile_global_cellname;
//	dfile_global_cellname = dfile_outputdir + "cellnamelist_atlas.txt";
	
	vector <string> cellnamelist;
	int annocellnum = 0;
	readLines(global_cellnamelist_filename, cellnamelist, annocellnum);
	
#ifdef DEBUG
	//output cellnamelist to check if it is correct
	for (i=1; i<=annocellnum; i++)
	{
		printf("%d, %s\n", i, cellnamelist.at(i-1).c_str());
	}
#endif

//	for (i=1; i<=annocellnum; i++)
//	{
//		printf("%d, %s\n", i, cellnamelist.at(i-1).c_str());
//	}
	
	// -----------------------------------------------------------------------------
	// read reference gene cell name file, get the number of annotated marker cells in total
	// -----------------------------------------------------------------------------	
	vector <string> ref_gene_cellnamelist;
	int ref_gene_annocellnum =0;
	readLines(refgene_cellnamelist_filename, ref_gene_cellnamelist, ref_gene_annocellnum);
	
//	//output cellnamelist to check if it is correct
//	for (i=1; i<ref_gene_annocellnum; i++)
//	{
//		printf("%d, %s\n", i, ref_gene_cellnamelist.at(i-1).c_str());
//	}

	// -----------------------------------------------------------------------------
	// read control point cells in the reference gene, get the number of control points
	// -----------------------------------------------------------------------------		
	vector <string> ref_gene_controlpoints_cellnamelist;
	int ref_gene_controlpoints_annocellnum =0;
	
	//refgene_controlpoints_cellnamelist_filename = dfile_outputdir +  "cellnamelist_EVE.txt";

	printf("%s\n", cellnamelist_filename.c_str());
	
	printf("%s\n", refgene_controlpoints_cellnamelist_filename.c_str());
	
	readLines(refgene_controlpoints_cellnamelist_filename, ref_gene_controlpoints_cellnamelist, ref_gene_controlpoints_annocellnum);
	
#ifdef DEBUG	
		//output cellnamelist to check if it is correct
	for (i=1; i<ref_gene_controlpoints_annocellnum; i++)
	{
		printf("%d, %s\n", i, ref_gene_controlpoints_cellnamelist.at(i-1).c_str());
	}
#endif
	
	
	// -----------------------------------------------------------------------------
	// read force add cell names
	// -----------------------------------------------------------------------------		
	vector <string> forceAddCellnamelist, forceAddCellnamelist_final;
	int forceaddCellNum = 0;
	int forceaddCellNum_final = 0;
			
	readLines(dfile_force_add, forceAddCellnamelist, forceaddCellNum);
	readLines(dfile_force_add_final, forceAddCellnamelist_final, forceaddCellNum_final);
	
#ifdef DEBUG	
	//output cellnamelist to check if it is correct
	for (i=1; i<forceaddCellNum; i++)
	{
		printf("%d, %s\n", i, forceAddCellnamelist.at(i-1).c_str());
	}
	
	for (i=1; i<forceaddCellNum_final; i++)
	{
		printf("%d, %s\n", i, forceAddCellnamelist_final.at(i-1).c_str());
	}
	
#endif
	
	
	// --------------------------------------------
	// read cell name list of different cell types 
	// --------------------------------------------
	
	int cellTypeNum[cellTypeNames.size()];
	
	vector < vector <string> > cellTypeNamelist;
	//= new vector <string> [cellTypeNames.size()];
	
	for(i=0; i<cellTypeNames.size(); i++)
	{
		string dfile_cellTypes = dfile_outputdir + "cellname_type_" + cellTypeNames[i] + ".txt";
		
		vector <string> tmp;
		readLines(dfile_cellTypes, tmp, cellTypeNum[i]);
		cellTypeNamelist.push_back(tmp);
	
	
		//#ifdef DEBUG	
		//output cellnamelist to check if it is correct
		for (j=0; j<cellTypeNum[i]; j++)
		{
			printf("%d, %s\n", j, cellTypeNamelist[i].at(j).c_str());
		}
		
		printf("\n");
		
		//#endif
	}
	

	// ----------------------------------------------------------------------------------------------------------------------------------
	// read the file containing cells in the atlas .apo files (no matter threratio) before and after merging cells 
	// ---------------------------------------------------------------------------------------------------------------------------------		
	vector <QStringList> final_cellnamelist;
	int final_cellnum = 0;
	
//	QStringList tmp;	
//	readLines(dfile_merge_cells, tmp, final_cellnum);
//	final_cellnamelist.push_back(tmp);

	readLines(dfile_merge_cells, final_cellnamelist, final_cellnum);
	
//#ifdef DEBUG	
	//output cellnamelist to check if it is correct
	for (i=0; i<final_cellnum; i++)
	{
		for (j=0; j<final_cellnamelist[i].size(); j++)
			printf("%s, ", qPrintable(final_cellnamelist[i].at(j)));
		printf("\n");
			
	}

//#endif

	
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
	

	printf("target file = %s\n", dfile_target.c_str());
	
//	if (rebuildTag==0) // append, no need to rebuild the atlas, thus no need to redo registration
//	{
////		appendAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist);		
//	}
//	else //need to rebuild
//	{
//		if (!dfile_target.empty()) // target is given
//			rebuildAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
//						 forceAddCellnamelist, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, threratio, final_cellnamelist, final_cellnum, atlas_subject_filename, atlas_target_filename);				
//
//		
//		else // target is unknown, need to figure out the reference stack automatically
//			rebuildAtlas(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
//						 forceAddCellnamelist, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, dfile_target, threratio, final_cellnamelist, final_cellnum);
//	}

	
	if (!dfile_target.empty()) // target is given
	{
//		rebuildAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
//					 forceAddCellnamelist, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, 
//					 threratio, final_cellnamelist, final_cellnum, atlas_subject_filename, atlas_target_filename);				

		rebuildAtlas(marker_gene_info, dfile_target, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
					 forceAddCellnamelist, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, 
					 threratio, final_cellnamelist, final_cellnum, test_colocalization_atlas_apo_filename);				
		
	}
	else // target is unknown, need to figure out the reference stack automatically
	{
		rebuildAtlas(marker_gene_info, cellnamelist, ref_gene_cellnamelist, ref_gene_controlpoints_cellnamelist, 
					 forceAddCellnamelist, forceAddCellnamelist_final, cellTypeNamelist, cellTypeNames, ref_gene_idx, dfile_outputdir, 
					 dfile_target, threratio, final_cellnamelist, final_cellnum);
	}

}



