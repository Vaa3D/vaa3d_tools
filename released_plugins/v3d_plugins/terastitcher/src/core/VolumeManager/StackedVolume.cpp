//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include <iostream>
#include <string>
#include "StackedVolume.h"
#include "S_config.h"
#include "tinyxml.h"
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <limits>
#include <list>
#include "Stack.h"
#include "Displacement.h"

using namespace std;

const char* axis_to_str(axis ax)
{
    if(ax==axis_invalid)         return "axis_invalid";
    else if(ax==vertical)        return "vertical";
    else if(ax==inv_vertical)    return "inv_vertical";
    else if(ax==horizontal)      return "horizontal";
    else if(ax==inv_horizontal)  return "inv_horizontal";
    else if(ax==depth)           return "depth";
    else if(ax==inv_depth)       return "inv_depth";
    else                         return "unknown";
}

StackedVolume::StackedVolume(const char* _stacks_dir, ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3, bool overwrite_mdata) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::StackedVolume(_stacks_dir=%s, reference_system = {%d,%d,%d}, VXL_1 = %.2f, VXL_2 = %.2f, VXL_3 = %.2f)\n", 
		  _stacks_dir,reference_system.first, reference_system.second, reference_system.third, VXL_1, VXL_2, VXL_3);
	#endif

	this->stacks_dir = new char[strlen(_stacks_dir)+1];
	strcpy(this->stacks_dir, _stacks_dir);

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
    char mdata_filepath[VM_STATIC_STRINGS_SIZE];
    sprintf(mdata_filepath, "%s/%s", stacks_dir, VM_BIN_METADATA_FILE_NAME);
    if(fileExists(mdata_filepath) && !overwrite_mdata)
            loadBinaryMetadata(mdata_filepath);
    else
	{
		if(reference_system.first == axis_invalid ||  reference_system.second == axis_invalid ||
			reference_system.third == axis_invalid || VXL_1 == 0 || VXL_2 == 0 || VXL_3 == 0)
			throw MyException("in StackedVolume::StackedVolume(...): invalid importing parameters");
		init();
		applyReferenceSystem(reference_system, VXL_1, VXL_2, VXL_3);
		saveBinaryMetadata(mdata_filepath);
	}
}

StackedVolume::StackedVolume(const char *xml_filepath) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::StackedVolume(xml_filepath=%s)\n", xml_filepath);
	#endif

    //extracting <stacks_dir> field from XML
    TiXmlDocument xml;
    if(!xml.LoadFile(xml_filepath))
    {
        char errMsg[2000];
        sprintf(errMsg,"in StackedVolume::StackedVolume(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
        throw MyException(errMsg);
    }
    TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));
    TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
    this->stacks_dir = new char[strlen(pelem->Attribute("value"))+1];
    strcpy(this->stacks_dir, pelem->Attribute("value"));

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
	char mdata_filepath[2000];
	sprintf(mdata_filepath, "%s/%s", stacks_dir, VM_BIN_METADATA_FILE_NAME);
	if(fileExists(mdata_filepath))
	{
		// load mdata.bin content and xml content, also perform consistency check between mdata.bin and xml content
		loadBinaryMetadata(mdata_filepath);
		loadXML(xml_filepath);
	}
	else
	{
		// load xml content and generate mdata.bin
		initFromXML(xml_filepath);
		saveBinaryMetadata(mdata_filepath);
	}
}

StackedVolume::~StackedVolume()
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::~StackedVolume(void)\n");
	#endif

	if(stacks_dir)
		delete[] stacks_dir;

	if(STACKS)
	{
		for(int row=0; row<N_ROWS; row++)
		{
			for(int col=0; col<N_COLS; col++)
				delete STACKS[row][col];
			delete[] STACKS[row];
		}
		delete[] STACKS;
	}
}

//GET METHODS
float	StackedVolume::getORG_V()					{return ORG_V;}
float	StackedVolume::getORG_H()					{return ORG_H;}
float	StackedVolume::getORG_D()					{return ORG_D;}
float	StackedVolume::getABS_V(int ABS_PIXEL_V)	{return ORG_V * 1000 + ABS_PIXEL_V*this->getVXL_V();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	StackedVolume::getABS_H(int ABS_PIXEL_H)	{return ORG_H * 1000 + ABS_PIXEL_H*this->getVXL_H();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	StackedVolume::getABS_D(int ABS_PIXEL_D)	{return ORG_D * 1000 + ABS_PIXEL_D*this->getVXL_D();}	//Alessandro - 23/03/2013: removed conversion from int to float
float	StackedVolume::getVXL_V()					{return VXL_V;}
float	StackedVolume::getVXL_H()					{return VXL_H;}
float	StackedVolume::getVXL_D()					{return VXL_D;}
float	StackedVolume::getMEC_V()					{return MEC_V;}
float	StackedVolume::getMEC_H()					{return MEC_H;}
int		StackedVolume::getStacksHeight()			{return STACKS[0][0]->getHEIGHT();}
int		StackedVolume::getStacksWidth()				{return STACKS[0][0]->getWIDTH();}
int		StackedVolume::getN_ROWS()					{return this->N_ROWS;}
int		StackedVolume::getN_COLS()					{return this->N_COLS;}
int		StackedVolume::getN_SLICES()				{return this->N_SLICES;}
Stack***StackedVolume::getSTACKS()					{return this->STACKS;}
char*   StackedVolume::getSTACKS_DIR()				{return this->stacks_dir;}
int		StackedVolume::getOVERLAP_V()				{return (int)(getStacksHeight() - MEC_V/VXL_V);}
int		StackedVolume::getOVERLAP_H()				{return (int)(getStacksWidth() -  MEC_H/VXL_H);}
int		StackedVolume::getDEFAULT_DISPLACEMENT_V()	{return (int)(fabs(MEC_V/VXL_V));}
int		StackedVolume::getDEFAULT_DISPLACEMENT_H()	{return (int)(fabs(MEC_H/VXL_H));}
int		StackedVolume::getDEFAULT_DISPLACEMENT_D()	{return 0;}

void StackedVolume::init() throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::init()\n");
	#endif

	//LOCAL VARIABLES
	string tmp_path;				//string that contains temp paths during computation
	string tmp;					    //string that contains temp data during computation
	string tmp2;				    //string that contains temp data during computation
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	DIR *cur_dir_lev2;				//pointer to DIR, the data structure that represents a DIRECTORY (level 2 of hierarchical structure)
	dirent *entry_lev1;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 1)
	dirent *entry_lev2;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 2)
	int i=0,j=0;					//for counting of N_ROWS, N_COLS
	list<Stack*> stacks_list;		//each stack found in the hierarchy is pushed into this list
	list<string> entries_lev1;		//list of entries of first level of hierarchy
	list<string>::iterator entry_i;	//iterator for list 'entries_lev1'
	list<string> entries_lev2;		//list of entries of second level of hierarchy
	list<string>::iterator entry_j;	//iterator for list 'entries_lev2'
	char stack_i_j_path[S_STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to stacks_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(stacks_dir)))
	{
		char msg[S_STATIC_STRINGS_SIZE];
		sprintf(msg,"in StackedVolume::init(...): Unable to open directory \"%s\"", stacks_dir);
		throw MyException(msg);
	}

	//scanning first level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry_lev1=readdir(cur_dir_lev1)))
	{
		tmp=entry_lev1->d_name;
		if(tmp.find(".") == string::npos && tmp.find(" ") == string::npos)
			entries_lev1.push_front(entry_lev1->d_name);
	}
	closedir(cur_dir_lev1);
	entries_lev1.sort();
	N_ROWS = (uint16) entries_lev1.size();
	N_COLS = 0;
        if(N_ROWS == 0)
                throw MyException("in StackedVolume::init(...): Unable to find stacks in the given directory");


	//for each entry of first level, scanning second level
	for(entry_i = entries_lev1.begin(), i=0; entry_i!= entries_lev1.end(); entry_i++, i++)
	{
		//building absolute path of first level entry to be used for "opendir(...)"
		tmp_path=stacks_dir;
		tmp_path.append("/");
		tmp_path.append(*entry_i);
		cur_dir_lev2 = opendir(tmp_path.c_str());
		if (!cur_dir_lev2)
			throw MyException("in StackedVolume::init(...): A problem occurred during scanning of subdirectories");

		//scanning second level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
		while ((entry_lev2=readdir(cur_dir_lev2)))
		{
			tmp=entry_lev2->d_name;
			if(tmp.find(".") == string::npos && tmp.find(" ") == string::npos)
				entries_lev2.push_back(entry_lev2->d_name);
		}
		closedir(cur_dir_lev2);
		entries_lev2.sort();

		//for each entry of the second level, allocating a new Stack
		for(entry_j = entries_lev2.begin(), j=0; entry_j!= entries_lev2.end(); entry_j++, j++)
		{
			//allocating new stack
			sprintf(stack_i_j_path,"%s/%s",(*entry_i).c_str(), (*entry_j).c_str());
			Stack *new_stk = new Stack(this,i,j,stack_i_j_path);
			stacks_list.push_back(new_stk);
		}
		entries_lev2.clear();
		if(N_COLS == 0)
			N_COLS = j;
		else if(j != N_COLS)
			throw MyException("in StackedVolume::init(...): Number of second-level directories is not the same for all first-level directories!");
	}
	entries_lev1.clear();

	//intermediate check
	if(N_ROWS == 0 || N_COLS == 0)
		throw MyException("in StackedVolume::init(...): Unable to find stacks in the given directory");

	//converting stacks_list (STL list of Stack*) into STACKS (2-D array of Stack*)
	STACKS = new Stack**[N_ROWS];
	for(int row=0; row < N_ROWS; row++)
		STACKS[row] = new Stack*[N_COLS];
	for(list<Stack*>::iterator i = stacks_list.begin(); i != stacks_list.end(); i++)
		STACKS[(*i)->getROW_INDEX()][(*i)->getCOL_INDEX()] = (*i);
}

void StackedVolume::applyReferenceSystem(ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::applyReferenceSystem(reference_system = {%d,%d,%d}, VXL_1 = %.2f, VXL_2 = %.2f, VXL_3 = %.2f)\n", 
		               reference_system.first, reference_system.second, reference_system.third, VXL_1, VXL_2, VXL_3);
	#endif

	/******************* 2) SETTING THE REFERENCE SYSTEM ********************
	The entire application uses a vertical-horizontal reference system, so
	it is necessary to fit the original reference system into the new one.     
	*************************************************************************/

	//adjusting possible sign mismatch between reference system and VXL
	//in these cases VXL is adjusted to match with reference system
	if(SIGN(reference_system.first) != SIGN(VXL_1))
		VXL_1*=-1.0f;
	if(SIGN(reference_system.second) != SIGN(VXL_2))
		VXL_2*=-1.0f;
	if(SIGN(reference_system.third) != SIGN(VXL_3))
		VXL_3*=-1.0f;	

	//HVD --> VHD
	if      (abs(reference_system.first)==2 && abs(reference_system.second)==1  && reference_system.third==3)
	{
		this->rotate(90);
		this->mirror(axis(2));	

		if(reference_system.first == -2)
			this->mirror(axis(2));
		if(reference_system.second == -1)
			this->mirror(axis(1));

		int computed_ORG_1, computed_ORG_2, computed_ORG_3;
		extractCoordinates(STACKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);
		ORG_V = computed_ORG_2/10000.0F;
		ORG_H = computed_ORG_1/10000.0F;
		ORG_D = computed_ORG_3/10000.0F;
		VXL_V = VXL_2 ;
		VXL_H = VXL_1 ; 
		VXL_D = VXL_3 ;
		int tmp_coord_1, tmp_coord_2, tmp_coord_3, tmp_coord_4, tmp_coord_5, tmp_coord_6;
        extractCoordinates(STACKS[0][0], 0, &tmp_coord_1, &tmp_coord_2, &tmp_coord_3);
		if(N_ROWS > 1)
		{
            extractCoordinates(STACKS[1][0], 0, &tmp_coord_4, &tmp_coord_5, &tmp_coord_6);
			this->MEC_V = (tmp_coord_5 - tmp_coord_2)/10.0F;
		}
		else
			this->MEC_V = getStacksHeight()*VXL_V;		
		if(N_COLS > 1)
		{
            extractCoordinates(STACKS[0][1], 0, &tmp_coord_4, &tmp_coord_5, &tmp_coord_6);
			this->MEC_H = (tmp_coord_4 - tmp_coord_1)/10.0F;
		}
		else
			this->MEC_H = getStacksWidth()*VXL_H;
		this->N_SLICES = STACKS[0][0]->getDEPTH();
	}
	//VHD --> VHD
	else if (abs(reference_system.first)==1 && abs(reference_system.second)==2 && reference_system.third==3)
	{		
		if(reference_system.first == -1)
			this->mirror(axis(1));
		if(reference_system.second == -2)
			this->mirror(axis(2));

		int computed_ORG_1, computed_ORG_2, computed_ORG_3;
		extractCoordinates(STACKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);
		ORG_V = computed_ORG_1/10000.0F;
		ORG_H = computed_ORG_2/10000.0F;
		ORG_D = computed_ORG_3/10000.0F;	
		VXL_V = VXL_1;
		VXL_H = VXL_2;
		VXL_D = VXL_3;
		int tmp_coord_1, tmp_coord_2, tmp_coord_3, tmp_coord_4, tmp_coord_5, tmp_coord_6;
        extractCoordinates(STACKS[0][0], 0, &tmp_coord_1, &tmp_coord_2, &tmp_coord_3);
		
		if(N_ROWS > 1)
		{
            extractCoordinates(STACKS[1][0], 0, &tmp_coord_4, &tmp_coord_5, &tmp_coord_6);
			this->MEC_V = (tmp_coord_4 - tmp_coord_1)/10.0F;		
		}
		else
			this->MEC_V = getStacksHeight()*VXL_V;		
		if(N_COLS > 1)
		{
            extractCoordinates(STACKS[0][1], 0, &tmp_coord_4, &tmp_coord_5, &tmp_coord_6);
			this->MEC_H = (tmp_coord_5 - tmp_coord_2)/10.0F;
		}
		else
			this->MEC_H = getStacksWidth()*VXL_H;
		this->N_SLICES = STACKS[0][0]->getDEPTH();
	}	
	else //unsupported reference system
	{
		char msg[500];
		sprintf(msg, "in StackedVolume::init(...): the reference system {%d,%d,%d} is not supported.", 
			reference_system.first, reference_system.second, reference_system.third);
		throw MyException(msg);
	}

	//some little adjustments of the origin
	if(VXL_V < 0)
		ORG_V -= (STACKS[0][0]->getHEIGHT()-1)* VXL_V/1000.0F;

	if(VXL_H < 0)
		ORG_H -= (STACKS[0][0]->getWIDTH() -1)* VXL_H/1000.0F;

	//inserting motorized stages coordinates
	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
		{
			if(i!=0)
				STACKS[i][j]->setABS_V(STACKS[i-1][j]->getABS_V() + getDEFAULT_DISPLACEMENT_V());
			else
				STACKS[i][j]->setABS_V(0);
			if(j!=0)
				STACKS[i][j]->setABS_H(STACKS[i][j-1]->getABS_H() + getDEFAULT_DISPLACEMENT_H());
			else
				STACKS[i][j]->setABS_H(0);
			STACKS[i][j]->setABS_D(getDEFAULT_DISPLACEMENT_D());
		}
}

void StackedVolume::loadXML(const char *xml_filepath)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::loadXML(char *xml_filepath = %s)\n", xml_filepath);
	#endif

	TiXmlDocument xml;
	if(!xml.LoadFile(xml_filepath))
	{
		char errMsg[2000];
		sprintf(errMsg,"in StackedVolume::loadXML(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
		throw MyException(errMsg);
	}

	//setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));

	//reading fields and checking coherence with metadata previously read from VM_BIN_METADATA_FILE_NAME
	TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
	if(strcmp(pelem->Attribute("value"), stacks_dir) != 0)
	{
		char errMsg[2000];
		sprintf(errMsg, "in StackedVolume::loadXML(...): Mismatch in <stacks_dir> field between xml file (=\"%s\") and %s (=\"%s\").", pelem->Attribute("value"), VM_BIN_METADATA_FILE_NAME, stacks_dir);
		throw MyException(errMsg);
	}
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	float VXL_V_read=0.0f, VXL_H_read=0.0f, VXL_D_read=0.0f;
	pelem->QueryFloatAttribute("V", &VXL_V_read);
	pelem->QueryFloatAttribute("H", &VXL_H_read);
	pelem->QueryFloatAttribute("D", &VXL_D_read);
	if(VXL_V_read != VXL_V || VXL_H_read != VXL_H || VXL_D_read != VXL_D)
	{
		char errMsg[2000];
		sprintf(errMsg, "in StackedVolume::loadXML(...): Mismatch in <voxel_dims> field between xml file (= %.2f x %.2f x %.2f ) and %s (= %.2f x %.2f x %.2f ).", VXL_V_read, VXL_H_read, VXL_D_read, VM_BIN_METADATA_FILE_NAME, VXL_V, VXL_H, VXL_D);
		throw MyException(errMsg);
	}
	pelem = hRoot.FirstChildElement("origin").Element();
	float ORG_V_read=0.0f, ORG_H_read=0.0f, ORG_D_read=0.0f;
	pelem->QueryFloatAttribute("V", &ORG_V_read);
	pelem->QueryFloatAttribute("H", &ORG_H_read);
	pelem->QueryFloatAttribute("D", &ORG_D_read);
	/*if(ORG_V_read != ORG_V || ORG_H_read != ORG_H || ORG_D_read != ORG_D)
	{
		char errMsg[2000];
		sprintf(errMsg, "in StackedVolume::loadXML(...): Mismatch in <origin> field between xml file (= {%.7f, %.7f, %.7f} ) and %s (= {%.7f, %.7f, %.7f} ).", ORG_V_read, ORG_H_read, ORG_D_read, VM_BIN_METADATA_FILE_NAME, ORG_V, ORG_H, ORG_D);
		throw MyException(errMsg);
	} @TODO: bug with float precision causes often mismatch */ 
	pelem = hRoot.FirstChildElement("mechanical_displacements").Element();
	float MEC_V_read=0.0f, MEC_H_read=0.0f;
	pelem->QueryFloatAttribute("V", &MEC_V_read);
	pelem->QueryFloatAttribute("H", &MEC_H_read);
	if(MEC_V_read != MEC_V || MEC_H_read != MEC_H)
	{
		char errMsg[2000];
		sprintf(errMsg, "in StackedVolume::loadXML(...): Mismatch in <mechanical_displacements> field between xml file (= %.1f x %.1f ) and %s (= %.1f x %.1f ).", MEC_V_read, MEC_H_read, VM_BIN_METADATA_FILE_NAME, MEC_V, MEC_H);
		throw MyException(errMsg);
	}
	pelem = hRoot.FirstChildElement("dimensions").Element();
	int N_ROWS_read, N_COLS_read, N_SLICES_read;
	pelem->QueryIntAttribute("stack_rows", &N_ROWS_read);
	pelem->QueryIntAttribute("stack_columns", &N_COLS_read);
	pelem->QueryIntAttribute("stack_slices", &N_SLICES_read);
	if(N_ROWS_read != N_ROWS || N_COLS_read != N_COLS || N_SLICES_read != N_SLICES)
	{
		char errMsg[2000];
		sprintf(errMsg, "in StackedVolume::loadXML(...): Mismatch between in <dimensions> field xml file (= %d x %d ) and %s (= %d x %d ).", N_ROWS_read, N_COLS_read, VM_BIN_METADATA_FILE_NAME, N_ROWS, N_COLS);
		throw MyException(errMsg);
	}

	pelem = hRoot.FirstChildElement("STACKS").Element()->FirstChildElement();
	int i,j;
	for(i=0; i<N_ROWS; i++)
		for(j=0; j<N_COLS; j++, pelem = pelem->NextSiblingElement())
			STACKS[i][j]->loadXML(pelem);
}

void StackedVolume::initFromXML(const char *xml_filepath)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::initFromXML(char *xml_filename = %s)\n", xml_filename);
		#endif

	TiXmlDocument xml;
	if(!xml.LoadFile(xml_filepath))
	{
		char errMsg[2000];
		sprintf(errMsg,"in StackedVolume::initFromXML(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
		throw MyException(errMsg);
	}

	//setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));

	//reading fields
	TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	pelem->QueryFloatAttribute("V", &VXL_V);
	pelem->QueryFloatAttribute("H", &VXL_H);
	pelem->QueryFloatAttribute("D", &VXL_D);
	pelem = hRoot.FirstChildElement("origin").Element();
	pelem->QueryFloatAttribute("V", &ORG_V);
	pelem->QueryFloatAttribute("H", &ORG_H);
	pelem->QueryFloatAttribute("D", &ORG_D);
	pelem = hRoot.FirstChildElement("mechanical_displacements").Element();
	pelem->QueryFloatAttribute("V", &MEC_V);
	pelem->QueryFloatAttribute("H", &MEC_H);
	pelem = hRoot.FirstChildElement("dimensions").Element();
	int nrows, ncols, nslices;
	pelem->QueryIntAttribute("stack_rows", &nrows);
	pelem->QueryIntAttribute("stack_columns", &ncols);
	pelem->QueryIntAttribute("stack_slices", &nslices);
	N_ROWS = nrows;
	N_COLS = ncols;
	N_SLICES = nslices;

	pelem = hRoot.FirstChildElement("STACKS").Element()->FirstChildElement();
	STACKS = new Stack **[N_ROWS];
	for(int i = 0; i < N_ROWS; i++)
	{
		STACKS[i] = new Stack *[N_COLS];
		for(int j = 0; j < N_COLS; j++, pelem = pelem->NextSiblingElement())
		{
			STACKS[i][j] = new Stack(this, i, j, pelem->Attribute("DIR_NAME"));
			STACKS[i][j]->loadXML(pelem);
		}
	}
}

void StackedVolume::saveXML(const char *xml_filename, const char *xml_filepath) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::saveXML(char *xml_filename = %s)\n", xml_filename);
	#endif

	//LOCAL VARIABLES
        char xml_abs_path[S_STATIC_STRINGS_SIZE];
	TiXmlDocument xml;
	TiXmlElement * root;
	TiXmlElement * pelem;
	int i,j;

        //obtaining XML absolute path
        if(xml_filename)
            sprintf(xml_abs_path, "%s/%s.xml", stacks_dir, xml_filename);
        else if(xml_filepath)
            strcpy(xml_abs_path, xml_filepath);
        else
            throw MyException("in StackedVolume::saveXML(...): no xml path provided");

	//initializing XML file with DTD declaration
        fstream XML_FILE(xml_abs_path, ios::out);
	XML_FILE<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	XML_FILE<<"<!DOCTYPE TeraStitcher SYSTEM \"TeraStitcher.DTD\">"<<endl;
	XML_FILE.close();

	//loading previously initialized XML file 
        if(!xml.LoadFile(xml_abs_path))
	{
		char errMsg[IO_STATIC_STRINGS_SIZE];
                sprintf(errMsg, "in StackedVolume::saveToXML(...) : unable to load xml file at \"%s\"", xml_abs_path);
		throw MyException(errMsg);
	}

	//inserting root node <TeraStitcher> and children nodes
	root = new TiXmlElement("TeraStitcher");  
	xml.LinkEndChild( root );  
	pelem = new TiXmlElement("stacks_dir");
	pelem->SetAttribute("value", stacks_dir);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("voxel_dims");
	pelem->SetDoubleAttribute("V", VXL_V);
	pelem->SetDoubleAttribute("H", VXL_H);
	pelem->SetDoubleAttribute("D", VXL_D);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("origin");
	pelem->SetDoubleAttribute("V", ORG_V);
	pelem->SetDoubleAttribute("H", ORG_H);
	pelem->SetDoubleAttribute("D", ORG_D);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("mechanical_displacements");
	pelem->SetDoubleAttribute("V", MEC_V);
	pelem->SetDoubleAttribute("H", MEC_H);
	root->LinkEndChild(pelem);
	pelem = new TiXmlElement("dimensions");
	pelem->SetAttribute("stack_rows", N_ROWS);
	pelem->SetAttribute("stack_columns", N_COLS);
	pelem->SetAttribute("stack_slices", N_SLICES);
	root->LinkEndChild(pelem);

	//inserting stack nodes
	pelem = new TiXmlElement("STACKS");
	for(i=0; i<N_ROWS; i++)
		for(j=0; j<N_COLS; j++)
			pelem->LinkEndChild(STACKS[i][j]->getXML());
	root->LinkEndChild(pelem);
	//saving the file
	xml.SaveFile();
}

void StackedVolume::saveBinaryMetadata(char *metadata_filepath) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::saveBinaryMetadata(char *metadata_filepath = %s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	FILE *file;
	int i,j;

	if(!(file = fopen(metadata_filepath, "wb")))
		throw MyException("in StackedVolume::saveBinaryMetadata(...): unable to save binary metadata file");
	str_size = (uint16) strlen(stacks_dir) + 1;
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(stacks_dir, str_size, 1, file);
	fwrite(&VXL_V, sizeof(float), 1, file);
	fwrite(&VXL_H, sizeof(float), 1, file);
	fwrite(&VXL_D, sizeof(float), 1, file);
	fwrite(&ORG_V, sizeof(float), 1, file);
	fwrite(&ORG_H, sizeof(float), 1, file);
	fwrite(&ORG_D, sizeof(float), 1, file);
	fwrite(&MEC_V, sizeof(float), 1, file);
	fwrite(&MEC_H, sizeof(float), 1, file);
	fwrite(&N_ROWS, sizeof(uint16), 1, file);
	fwrite(&N_COLS, sizeof(uint16), 1, file);
	fwrite(&N_SLICES, sizeof(uint16), 1, file);

	for(i = 0; i < N_ROWS; i++)
		for(j = 0; j < N_COLS; j++)
			STACKS[i][j]->binarizeInto(file);

	fclose(file);
}

void StackedVolume::loadBinaryMetadata(char *metadata_filepath) throw (MyException)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::loadBinaryMetadata(char *metadata_filepath = %s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	FILE *file;
	int i,j;
	size_t fread_return_val;

	if(!(file = fopen(metadata_filepath, "rb")))
		throw MyException("in StackedVolume::loadBinaryMetadata(...): unable to load binary metadata file");
	str_size = (uint16) strlen(stacks_dir) + 1;
	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(stacks_dir, str_size, 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&VXL_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&VXL_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&VXL_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&ORG_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&ORG_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&ORG_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&MEC_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&MEC_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&N_ROWS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&N_COLS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	fread_return_val = fread(&N_SLICES, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in StackedVolume::loadBinaryMetadata(...) error while reading binary metadata file");

	STACKS = new Stack **[N_ROWS];
	for(i = 0; i < N_ROWS; i++)
	{
		STACKS[i] = new Stack *[N_COLS];
		for(j = 0; j < N_COLS; j++)
			STACKS[i][j] = new Stack(this, i, j, file);
	}

	fclose(file);
}

//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
void StackedVolume::rotate(int theta)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::rotate(theta = %d)\n", theta);
	#endif

	Stack*** new_STACK_2D_ARRAY = NULL;
	int new_N_ROWS=0, new_N_COLS=0;

	switch(theta)
	{
		case(0): break;

		case(90):
		{
			new_N_COLS = N_ROWS;
			new_N_ROWS = N_COLS;

			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = STACKS[N_ROWS-1-j][i];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
		case(180):
		{
			new_N_COLS=N_COLS;
			new_N_ROWS=N_ROWS;

			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = STACKS[N_ROWS-1-i][N_COLS-1-j];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
		case(270):
		{
			new_N_COLS=N_ROWS;
			new_N_ROWS=N_COLS;

			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = STACKS[j][N_COLS-1-i];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
	}

	//deallocating current STACK_2DARRAY object
	for(int row=0; row<N_ROWS; row++)
		delete[] STACKS[row];
	delete[] STACKS;

	STACKS = new_STACK_2D_ARRAY;
	N_COLS = new_N_COLS;
	N_ROWS = new_N_ROWS;
}

//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
void StackedVolume::mirror(axis mrr_axis)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::mirror(mrr_axis = %d)\n", mrr_axis);
	#endif

	if(mrr_axis!= 1 && mrr_axis != 2)
	{
		char msg[1000];
		sprintf(msg,"in StackedVolume::mirror(axis mrr_axis=%d): unsupported axis mirroring", mrr_axis);
		throw MyException(msg);
	}

	Stack*** new_STACK_2D_ARRAY;

	switch(mrr_axis)
	{
		case(1):
		{
			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Stack**[N_ROWS];
			for(int i=0; i<N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Stack*[N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<N_ROWS; i++)
				for(int j=0; j<N_COLS; j++){
					new_STACK_2D_ARRAY[i][j]=STACKS[N_ROWS-1-i][j];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}		
		case(2):
		{
			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Stack**[N_ROWS];
			for(int i=0; i<N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Stack*[N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<N_ROWS; i++)
				for(int j=0; j<N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = STACKS[i][N_COLS-1-j];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
		default: break;
	}

	//deallocating current STACK_2DARRAY object
	for(int row=0; row<N_ROWS; row++)
		delete[] STACKS[row];
	delete[] STACKS;

	STACKS = new_STACK_2D_ARRAY;
}

//extract spatial coordinates (in millimeters) of given Stack object
void StackedVolume::extractCoordinates(Stack* stk, int z, int* crd_1, int* crd_2, int* crd_3)
{
    #if VM_VERBOSE > 3
    printf("\t\t\t\tin StackedVolume::extractCoordinates(stk=\"%s\", z = %d)\n", stk->getDIR_NAME(), z);
    #endif

	bool found_ABS_X=false;
	bool found_ABS_Y=false;

	//loading estimations for absolute X and Y stack positions
	char * pch;
	char buffer[100];
	strcpy(buffer,&(stk->getDIR_NAME()[0]));
	pch = strtok (buffer,"/_");
	pch = strtok (NULL, "/_");

	while (pch != NULL)
	{
		if(!found_ABS_X)
		{
			if(sscanf(pch, "%d", crd_1) == 1)
				found_ABS_X=true;
		}
		else if(!found_ABS_Y)
		{
			if(sscanf(pch, "%d", crd_2) == 1)
				found_ABS_Y=true;
		}
		else
			break;

		pch = strtok (NULL, "/_");
	}

	if(!found_ABS_X || !found_ABS_Y)
	{
		char msg[200];
		sprintf(msg,"in StackedVolume::extractCoordinates(directory_name=\"%s\"): format 000000_000000 or X_000000_X_000000 not found", stk->getDIR_NAME());
        throw MyException(msg);
	}

	//loading estimation for absolute Z stack position
	if(crd_3!= NULL)
	{
		char* first_file_name = stk->getFILENAMES()[z];

		char * pch;
		char lastTokenized[100];
		char buffer[500];
		strcpy(buffer,&(first_file_name[0]));

		pch = strtok (buffer,"_");
		while (pch != NULL)
		{
			strcpy(lastTokenized,pch);
			pch = strtok (NULL, "_");
		}

		pch = strtok (lastTokenized,".");
		strcpy(lastTokenized,pch);

		if(sscanf(lastTokenized, "%d", crd_3) != 1)
		{
			char msg[200];
			sprintf(msg,"in StackedVolume::extractCoordinates(...): unable to extract Z position from filename %s", first_file_name);
            throw MyException(msg);
		}
	}
}

//print all informations contained in this data structure
void StackedVolume::print()
{
	printf("*** Begin printing StakedVolume object...\n\n");
	printf("\tDirectory:\t\t\t%s\n", stacks_dir);
	printf("\tDimensions of single stack:\t%d(V) x %d(H) x %d(D)\n", this->getStacksHeight(), this->getStacksWidth(), N_SLICES);
	printf("\tVoxels:\t\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", VXL_V, VXL_H, VXL_D);
	printf("\tOrigin:\t\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", ORG_V, ORG_H, ORG_D);
	printf("\tMechanical displacements:\t%.4f(V) x %.4f(H)\n", MEC_V, MEC_H);
	printf("\tStacks matrix:\t\t\t%d(V) x %d(H)\n", N_ROWS, N_COLS);
	printf("\t |\n");
	for(int row=0; row<N_ROWS; row++)
		for(int col=0; col<N_COLS; col++)
			STACKS[row][col]->print();
	printf("\n*** END printing StakedVolume object...\n\n");
}

//inserts the given displacements in the given stacks
void StackedVolume::insertDisplacement(Stack *stk_A, Stack *stk_B, Displacement *displacement)  throw (MyException)
{
	int stk_A_row = stk_A->getROW_INDEX();
	int stk_A_col = stk_A->getCOL_INDEX();
	int stk_B_row = stk_B->getROW_INDEX();
	int stk_B_col = stk_B->getCOL_INDEX();
	displacement->evalReliability(dir_vertical);
	displacement->evalReliability(dir_horizontal);
	displacement->evalReliability(dir_depth);
	if(stk_B_row == stk_A_row && stk_B_col == stk_A_col+1)
	{
		displacement->setDefaultV(0);	//we assume that adjacent tiles are aligned with respect to motorized stages coordinates
		displacement->setDefaultH(getDEFAULT_DISPLACEMENT_H());
		displacement->setDefaultD(getDEFAULT_DISPLACEMENT_D());
		stk_A->getEAST().push_back(displacement);
		stk_B->getWEST().push_back(displacement->getMirrored(dir_horizontal));
	}
	else if(stk_B_row == stk_A_row +1 && stk_B_col == stk_A_col)
	{
		displacement->setDefaultV(getDEFAULT_DISPLACEMENT_V());
		displacement->setDefaultH(0);	//we assume that adjacent tiles are aligned with respect to motorized stages coordinates					
		displacement->setDefaultD(getDEFAULT_DISPLACEMENT_D());
		stk_A->getSOUTH().push_back(displacement);
		stk_B->getNORTH().push_back(displacement->getMirrored(dir_vertical));
	}
	else
	{
		char errMsg[1000];
		sprintf(errMsg, "in StackedVolume::insertDisplacement(stk_A[%d,%d], stk_B[%d,%d], displacement): stacks are not adjacent", 
			    stk_A->getROW_INDEX(), stk_A->getCOL_INDEX(), stk_B->getROW_INDEX(), stk_B->getCOL_INDEX());
		throw MyException(errMsg);
	}
}

//extract absolute path from file path(i.e. "C:/Users/Alex/Desktop/" from "C:/Users/Alex/Desktop/text.xml")
std::string StackedVolume::extractPathFromFilePath(const char* file_path)
{
	//PRECONDITIONS: file_path contains an absolute file path (i.e. "C:/Users/Alex/Desktop/text.xml") with both separators "/" OR "\"
	//POSTCONDITIONS: the path of the directory that contains the file is returned (i.e. "C:/Users/Alex/Desktop/" from "C:/Users/Alex/Desktop/text.xml")


	//LOCAL VARIABLES
	string file_path_string =file_path;
	string file_name;
	string ris;

	//loading file name from 'file_path_string' into 'file_name' by strtoking with "/" OR "\" character
	char * tmp;
	tmp = strtok (&file_path_string[0],"/\\");
	while (tmp != NULL)
	{
		file_name = tmp;
		tmp = strtok (NULL, "/\\");
	}

	//restoring content of file_path_string due to possible alterations done by strtok
	file_path_string =file_path;

	//loading file name substring index in 'file_path_string'
	int index_of_filename= (int) file_path_string.find(file_name);

	//extract substring that contains absolute path but the filename
	ris=file_path_string.substr(0,index_of_filename);
	return ris;
}


//returns true if file exists at the given filepath
bool StackedVolume::fileExists(const char *filepath)  throw (MyException)
{
	//LOCAL VARIABLES
	string file_path_string =filepath;
	string file_name;
	string dir_path;
	bool file_exists = false;
	DIR* directory;
	dirent* dir_entry;

	//extracting dir_path and file_name from file_path
	char * tmp;
	tmp = strtok (&file_path_string[0],"/\\");
	while (tmp != NULL)
	{
		file_name = tmp;
		tmp = strtok (NULL, "/\\");
	}
	file_path_string =filepath;
	dir_path=file_path_string.substr(0,file_path_string.find(file_name));

	//obtaining DIR pointer to directory (=NULL if directory doesn't exist)
	if (!(directory=opendir(&(dir_path[0]))))
	{
		char msg[1000];
		sprintf(msg,"in fileExists(filepath=%s): Unable to open directory \"%s\"", filepath, &dir_path[0]);
		throw MyException(msg);
	}

	//scanning for given file
	while (!file_exists && (dir_entry=readdir(directory)))
	{
		//storing in tmp i-th entry and checking that it not contains '.', so that I can exclude '..', '.' and files entries
		if(!strcmp(&(file_name[0]), dir_entry->d_name))
			file_exists = true;
	}
	closedir(directory);

	return file_exists;
}

//counts the total number of displacements and the number of displacements per stack
void StackedVolume::countDisplacements(int& total, float& per_stack_pair)
{
    /* PRECONDITIONS: none */
    total = 0;
	per_stack_pair = 0.0f;
    for(int i=0; i<N_ROWS; i++)
        for(int j=0; j<N_COLS; j++)
        {
            total+= static_cast<int>(STACKS[i][j]->getEAST().size());
            total+= static_cast<int>(STACKS[i][j]->getSOUTH().size());
            per_stack_pair += static_cast<int>(STACKS[i][j]->getEAST().size());
            per_stack_pair += static_cast<int>(STACKS[i][j]->getSOUTH().size());
        }
    per_stack_pair /= 2*(N_ROWS*N_COLS) - N_ROWS - N_COLS;
}

//counts the number of single-direction displacements having a reliability measure above the given threshold
void StackedVolume::countReliableSingleDirectionDisplacements(float threshold, int& total, int& reliable)
{
    /* PRECONDITIONS:
     *   - for each pair of adjacent stacks one and only one displacement exists (CHECKED) */

    total = reliable = 0;
    for(int i=0; i<N_ROWS; i++)
        for(int j=0; j<N_COLS; j++)
        {
            if(j != (N_COLS-1) && STACKS[i][j]->getEAST().size()==1)
            {
                total+=3;
                reliable += STACKS[i][j]->getEAST()[0]->getReliability(dir_vertical) >= threshold;
                reliable += STACKS[i][j]->getEAST()[0]->getReliability(dir_horizontal) >= threshold;
                reliable += STACKS[i][j]->getEAST()[0]->getReliability(dir_depth) >= threshold;
            }
            if(i != (N_ROWS-1) && STACKS[i][j]->getSOUTH().size()==1)
            {
                total+=3;
                reliable += STACKS[i][j]->getSOUTH()[0]->getReliability(dir_vertical) >= threshold;
                reliable += STACKS[i][j]->getSOUTH()[0]->getReliability(dir_horizontal) >= threshold;
                reliable += STACKS[i][j]->getSOUTH()[0]->getReliability(dir_depth) >= threshold;
            }
        }
}

//counts the number of stitchable stacks given the reliability threshold
int StackedVolume::countStitchableStacks(float threshold)
{
    /* PRECONDITIONS:
     *   - for each pair of adjacent stacks one and only one displacement exists (CHECKED) */

    //stitchable stacks are stacks that have at least one reliable single-direction displacement
    int stitchables = 0;
    bool stitchable;
    for(int i=0; i<N_ROWS; i++)
        for(int j=0; j<N_COLS; j++)
        {
            stitchable = false;
            Stack* stk = STACKS[i][j];
            if(i!= 0 && STACKS[i][j]->getNORTH().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getNORTH()[0]->getReliability(direction(k)) >= threshold);
            if(j!= (N_COLS -1) && STACKS[i][j]->getEAST().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getEAST()[0]->getReliability(direction(k)) >= threshold);
            if(i!= (N_ROWS -1) && STACKS[i][j]->getSOUTH().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getSOUTH()[0]->getReliability(direction(k)) >= threshold);
            if(j!= 0 && STACKS[i][j]->getWEST().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getWEST()[0]->getReliability(direction(k)) >= threshold);
            stitchables += stitchable;
        }
    return stitchables;
}
