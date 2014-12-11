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
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2014-11-06. Giulio.     @ADDED saved reference system into XML file
* 2014-09-20. Alessandro. @ADDED overwrite_mdata flag to the XML-based constructor.
* 2014-09-10. Alessandro. @ADDED 'volume_format' attribute to <TeraStitcher> XML node
* 2014-09-10. Alessandro. @ADDED plugin creation/registration functions to make 'StackedVolume' a volume format plugin.
* 2014-09-09. Alessandro. @FIXED. Added default reference system if volume is imported from xml.
* 2014-09-09. Alessandro. @FIXED both 'init()' and 'initFromXML()' methods to deal with empty stacks. Added call of 'normalize_stacks_attributes()' method.
* 2014-09-05. Alessandro. @ADDED 'normalize_stacks_attributes()' method to normalize stacks attributes (width, height, etc.)
* 2014-09-02. Alessandro. @FIXED both 'loadBinaryMetadata()' and 'saveBinaryMetadata()' as 'N_SLICES' changed from 'uint16' to 'int' type. See vmVirtualVolume.h.
*/

 
#include "vmBlockVolume.h"
#include "S_config.h"
//#include <string>
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <list>
#include <fstream>
#include <sstream>
#include <set>

// 2014-09-10. Alessandro. @ADDED plugin creation/registration functions to make 'StackedVolume' a volume format plugin.
const std::string BlockVolume::id = "TiledXY|3Dseries";
const std::string BlockVolume::creator_id1 = volumemanager::VirtualVolumeFactory::registerPluginCreatorXML(&createFromXML, BlockVolume::id);
const std::string BlockVolume::creator_id2 = volumemanager::VirtualVolumeFactory::registerPluginCreatorData(&createFromData, BlockVolume::id);

using namespace std;
using namespace iom;

BlockVolume::BlockVolume(const char* _stacks_dir, vm::ref_sys _reference_system, float VXL_1, float VXL_2, float VXL_3, bool overwrite_mdata) throw (iom::exception)
	: VirtualVolume(_stacks_dir, _reference_system, VXL_1, VXL_2, VXL_3)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::BlockVolume(_stacks_dir=%s, reference_system = {%d,%d,%d}, VXL_1 = %.2f, VXL_2 = %.2f, VXL_3 = %.2f)\n", 
		  _stacks_dir,reference_system.first, reference_system.second, reference_system.third, VXL_1, VXL_2, VXL_3);
	#endif

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
    char mdata_filepath[VM_STATIC_STRINGS_SIZE];
    sprintf(mdata_filepath, "%s/%s", stacks_dir, vm::BINARY_METADATA_FILENAME.c_str());
    if(fileExists(mdata_filepath) && !overwrite_mdata)
            loadBinaryMetadata(mdata_filepath);
    else
	{
		if(_reference_system.first == vm::axis_invalid ||  _reference_system.second == vm::axis_invalid ||
			_reference_system.third == vm::axis_invalid || VXL_1 == 0 || VXL_2 == 0 || VXL_3 == 0)
			throw iom::exception("in BlockVolume::BlockVolume(...): invalid importing parameters");
		reference_system = _reference_system; // GI_140501: stores the refrence system to generate the mdata.bin file for the output volumes
		init();
		applyReferenceSystem(reference_system, VXL_1, VXL_2, VXL_3);
		saveBinaryMetadata(mdata_filepath);
	}
}

BlockVolume::BlockVolume(const char *xml_filepath, bool overwrite_mdata) throw (iom::exception)
	: VirtualVolume(xml_filepath)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::BlockVolume(xml_filepath=%s)\n", xml_filepath);
	#endif

    //extracting <stacks_dir> field from XML
    TiXmlDocument xml;
    if(!xml.LoadFile(xml_filepath))
    {
        char errMsg[2000];
        sprintf(errMsg,"in BlockVolume::BlockVolume(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
        throw iom::exception(errMsg);
    }
    TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));
    TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
    this->stacks_dir = new char[strlen(pelem->Attribute("value"))+1];
    strcpy(this->stacks_dir, pelem->Attribute("value"));

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
	char mdata_filepath[2000];
	sprintf(mdata_filepath, "%s/%s", stacks_dir, vm::BINARY_METADATA_FILENAME.c_str());

    // 2014-09-20. Alessandro. @ADDED overwrite_mdata flag
    if(fileExists(mdata_filepath) && !overwrite_mdata)
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

BlockVolume::~BlockVolume()
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::~BlockVolume(void)\n");
	#endif

	if(stacks_dir)
		delete[] stacks_dir;

	if(BLOCKS)
	{
		for(int row=0; row<N_ROWS; row++)
		{
			for(int col=0; col<N_COLS; col++)
				delete BLOCKS[row][col];
			delete[] BLOCKS[row];
		}
		delete[] BLOCKS;
	}
}



void BlockVolume::init() throw (iom::exception)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::init()\n");
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
	list<Block*> stacks_list;		//each stack found in the hierarchy is pushed into this list
	list<string> entries_lev1;		//list of entries of first level of hierarchy
	list<string>::iterator entry_i;	//iterator for list 'entries_lev1'
	list<string> entries_lev2;		//list of entries of second level of hierarchy
	list<string>::iterator entry_j;	//iterator for list 'entries_lev2'
	char stack_i_j_path[S_STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to stacks_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(stacks_dir)))
	{
		char msg[S_STATIC_STRINGS_SIZE];
		sprintf(msg,"in BlockVolume::init(...): Unable to open directory \"%s\"", stacks_dir);
		throw iom::exception(msg);
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
         throw iom::exception("in BlockVolume::init(...): Unable to find stacks in the given directory");


	//for each entry of first level, scanning second level
	for(entry_i = entries_lev1.begin(), i=0; entry_i!= entries_lev1.end(); entry_i++, i++)
	{
		//building absolute path of first level entry to be used for "opendir(...)"
		tmp_path=stacks_dir;
		tmp_path.append("/");
		tmp_path.append(*entry_i);
		cur_dir_lev2 = opendir(tmp_path.c_str());
		if (!cur_dir_lev2)
			throw iom::exception("in BlockVolume::init(...): A problem occurred during scanning of subdirectories");

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
			Block *new_stk = new Block(this,i,j,stack_i_j_path);
			stacks_list.push_back(new_stk);
		}
		entries_lev2.clear();
		if(N_COLS == 0)
			N_COLS = j;
		else if(j != N_COLS)
			throw iom::exception("in BlockVolume::init(...): Number of second-level directories is not the same for all first-level directories!");
	}
	entries_lev1.clear();

	//intermediate check
	if(N_ROWS == 0 || N_COLS == 0)
		throw iom::exception("in BlockVolume::init(...): Unable to find stacks in the given directory");

	//converting stacks_list (STL list of Stack*) into STACKS (2-D array of Stack*)
	BLOCKS = new Block**[N_ROWS];
	for(int row=0; row < N_ROWS; row++)
		BLOCKS[row] = new Block*[N_COLS];
	for(list<Block*>::iterator i = stacks_list.begin(); i != stacks_list.end(); i++)
		BLOCKS[(*i)->getROW_INDEX()][(*i)->getCOL_INDEX()] = (*i);

	// 2014-09-09. Alessandro. @FIXED both 'init()' and 'initFromXML()' methods to deal with empty stacks. Added call of 'normalize_stacks_attributes()' method.
	// make stacks have the same attributes
	normalize_stacks_attributes();
}

void BlockVolume::applyReferenceSystem(vm::ref_sys reference_system, float VXL_1, float VXL_2, float VXL_3) throw (iom::exception)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::applyReferenceSystem(reference_system = {%d,%d,%d}, VXL_1 = %.2f, VXL_2 = %.2f, VXL_3 = %.2f)\n", 
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
		this->mirror(vm::axis(2));	

		if(reference_system.first == -2)
			this->mirror(vm::axis(2));
		if(reference_system.second == -1)
			this->mirror(vm::axis(1));

		int computed_ORG_1, computed_ORG_2, computed_ORG_3;
		extractCoordinates(BLOCKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);
		ORG_V = computed_ORG_2/10000.0F;
		ORG_H = computed_ORG_1/10000.0F;
		ORG_D = computed_ORG_3/10000.0F;
		VXL_V = VXL_2 ;
		VXL_H = VXL_1 ; 
		VXL_D = VXL_3 ;
		int tmp_coord_1, tmp_coord_2, tmp_coord_4, tmp_coord_5;
        extractCoordinates(BLOCKS[0][0], 0, &tmp_coord_1, &tmp_coord_2);
		if(N_ROWS > 1)
		{
            extractCoordinates(BLOCKS[1][0], 0, &tmp_coord_4, &tmp_coord_5);
			this->MEC_V = (tmp_coord_5 - tmp_coord_2)/10.0F;
		}
		else
			this->MEC_V = getStacksHeight()*VXL_V;		
		if(N_COLS > 1)
		{
            extractCoordinates(BLOCKS[0][1], 0, &tmp_coord_4, &tmp_coord_5);
			this->MEC_H = (tmp_coord_4 - tmp_coord_1)/10.0F;
		}
		else
			this->MEC_H = getStacksWidth()*VXL_H;
		this->N_SLICES = BLOCKS[0][0]->getDEPTH();
	}
	//VHD --> VHD
	else if (abs(reference_system.first)==1 && abs(reference_system.second)==2 && reference_system.third==3)
	{		
		if(reference_system.first == -1)
			this->mirror(vm::axis(1));
		if(reference_system.second == -2)
			this->mirror(vm::axis(2));

		int computed_ORG_1, computed_ORG_2, computed_ORG_3;
		extractCoordinates(BLOCKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);
		ORG_V = computed_ORG_1/10000.0F;
		ORG_H = computed_ORG_2/10000.0F;
		ORG_D = computed_ORG_3/10000.0F;	
		VXL_V = VXL_1;
		VXL_H = VXL_2;
		VXL_D = VXL_3;
		int tmp_coord_1, tmp_coord_2, tmp_coord_4, tmp_coord_5;
        extractCoordinates(BLOCKS[0][0], 0, &tmp_coord_1, &tmp_coord_2);
		
		if(N_ROWS > 1)
		{
            extractCoordinates(BLOCKS[1][0], 0, &tmp_coord_4, &tmp_coord_5);
			this->MEC_V = (tmp_coord_4 - tmp_coord_1)/10.0F;		
		}
		else
			this->MEC_V = getStacksHeight()*VXL_V;		
		if(N_COLS > 1)
		{
            extractCoordinates(BLOCKS[0][1], 0, &tmp_coord_4, &tmp_coord_5);
			this->MEC_H = (tmp_coord_5 - tmp_coord_2)/10.0F;
		}
		else
			this->MEC_H = getStacksWidth()*VXL_H;
		this->N_SLICES = BLOCKS[0][0]->getDEPTH();
	}	
	else //unsupported reference system
	{
		char msg[500];
		sprintf(msg, "in BlockVolume::init(...): the reference system {%d,%d,%d} is not supported.", 
			reference_system.first, reference_system.second, reference_system.third);
		throw iom::exception(msg);
	}

	//some little adjustments of the origin
	if(VXL_V < 0)
		ORG_V -= (BLOCKS[0][0]->getHEIGHT()-1)* VXL_V/1000.0F;

	if(VXL_H < 0)
		ORG_H -= (BLOCKS[0][0]->getWIDTH() -1)* VXL_H/1000.0F;

	//inserting motorized stages coordinates
	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
		{
			if(i!=0)
				BLOCKS[i][j]->setABS_V(BLOCKS[i-1][j]->getABS_V() + getDEFAULT_DISPLACEMENT_V());
			else
				BLOCKS[i][j]->setABS_V(0);
			if(j!=0)
				BLOCKS[i][j]->setABS_H(BLOCKS[i][j-1]->getABS_H() + getDEFAULT_DISPLACEMENT_H());
			else
				BLOCKS[i][j]->setABS_H(0);
			BLOCKS[i][j]->setABS_D(getDEFAULT_DISPLACEMENT_D());
		}
}

void BlockVolume::saveBinaryMetadata(char *metadata_filepath) throw (iom::exception)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::saveBinaryMetadata(char *metadata_filepath = %s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	FILE *file;
	int i,j;

	if(!(file = fopen(metadata_filepath, "wb")))
		throw iom::exception("in BlockVolume::saveBinaryMetadata(...): unable to save binary metadata file");
	str_size = (uint16) strlen(stacks_dir) + 1;
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(stacks_dir, str_size, 1, file);
    fwrite(&reference_system.first, sizeof(vm::axis), 1, file);  // GI_140501
    fwrite(&reference_system.second, sizeof(vm::axis), 1, file); // GI_140501
    fwrite(&reference_system.third, sizeof(vm::axis), 1, file);  // GI_140501
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

	// 2014-09-02. Alessandro. @FIXED as 'N_SLICES' changed from 'uint16' to 'int' type. See vmVirtualVolume.h.
	fwrite(&N_SLICES, sizeof(int), 1, file);

	for(i = 0; i < N_ROWS; i++)
		for(j = 0; j < N_COLS; j++)
			BLOCKS[i][j]->binarizeInto(file);

	fclose(file);
}

void BlockVolume::loadBinaryMetadata(char *metadata_filepath) throw (iom::exception)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::loadBinaryMetadata(char *metadata_filepath = %s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	char *temp; // GI_140425
	bool regen = false;
	FILE *file;
	int i,j;
	size_t fread_return_val;

	if(!(file = fopen(metadata_filepath, "rb")))
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...): unable to load binary metadata file");
	// str_size = (uint16) strlen(stacks_dir) + 1;  // GI_140425 remodev because has with no effect 
	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	// GI_140425 a check has been introduced to avoid that an out-of-date mdata.bin contains a wrong rood directory
	temp = new char[str_size];
	fread_return_val = fread(temp, str_size, 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	if ( !strcmp(temp,stacks_dir) ) // the two strings are equal
		delete []temp;
	else { // GI_140626: allow moving mdata.bin to other machine
		delete []temp;
		regen = true;
		//fclose(file);
		//throw iom::iom::exception("in BlockVolume::loadBinaryMetadata(...): binary metadata file is out-of-date");
		#if VM_VERBOSE > 3
		printf("\t\t\t\tin BlockVolume::loadBinaryMetadata(...): binary metadata file is out-of-date\n");
		#endif
	}

	// GI_140501
    fread_return_val = fread(&reference_system.first, sizeof(vm::axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw iom::exception("in BlockVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	// GI_140501
    fread_return_val = fread(&reference_system.second, sizeof(vm::axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw iom::exception("in BlockVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

 	// GI_140501
   fread_return_val = fread(&reference_system.third, sizeof(vm::axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw iom::exception("in BlockVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&VXL_V, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&VXL_H, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&VXL_D, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&ORG_V, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&ORG_H, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&ORG_D, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&MEC_V, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&MEC_H, sizeof(float), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&N_ROWS, sizeof(uint16), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	fread_return_val = fread(&N_COLS, sizeof(uint16), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}

	// 2014-09-02. Alessandro. @FIXED as 'N_SLICES' changed from 'uint16' to 'int' type. See vmVirtualVolume.h.
	fread_return_val = fread(&N_SLICES, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in BlockVolume::loadBinaryMetadata(...) error while reading binary metadata file");
	}
	BLOCKS = new Block **[N_ROWS];
	for(i = 0; i < N_ROWS; i++)
	{
		BLOCKS[i] = new Block *[N_COLS];
		for(j = 0; j < N_COLS; j++)
			BLOCKS[i][j] = new Block(this, i, j, file);
	}

	fclose(file);

	if ( regen ) { // GI_140626: directory name is changed, mdata.bin must be regenerated
		saveBinaryMetadata(metadata_filepath);
	}
}

//rotate stacks matrix around D vm::axis (accepted values are theta=0,90,180,270)
void BlockVolume::rotate(int theta)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::rotate(theta = %d)\n", theta);
	#endif

	Block*** new_STACK_2D_ARRAY = NULL;
	int new_N_ROWS=0, new_N_COLS=0;

	switch(theta)
	{
		case(0): break;

		case(90):
		{
			new_N_COLS = N_ROWS;
			new_N_ROWS = N_COLS;

			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Block**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Block*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = BLOCKS[N_ROWS-1-j][i];
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
			new_STACK_2D_ARRAY = new Block**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Block*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = BLOCKS[N_ROWS-1-i][N_COLS-1-j];
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
			new_STACK_2D_ARRAY = new Block**[new_N_ROWS];
			for(int i=0; i<new_N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Block*[new_N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<new_N_ROWS; i++)
				for(int j=0; j<new_N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = BLOCKS[j][N_COLS-1-i];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
	}

	//deallocating current STACK_2DARRAY object
	for(int row=0; row<N_ROWS; row++)
		delete[] BLOCKS[row];
	delete[] BLOCKS;

	BLOCKS = new_STACK_2D_ARRAY;
	N_COLS = new_N_COLS;
	N_ROWS = new_N_ROWS;
}

//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
void BlockVolume::mirror(vm::axis mrr_axis)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::mirror(mrr_axis = %d)\n", mrr_axis);
	#endif

	if(mrr_axis!= 1 && mrr_axis != 2)
	{
		char msg[1000];
		sprintf(msg,"in BlockVolume::mirror(vm::axis mrr_axis=%d): unsupported vm::axis mirroring", mrr_axis);
		throw iom::exception(msg);
	}

	Block*** new_STACK_2D_ARRAY;

	switch(mrr_axis)
	{
		case(1):
		{
			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Block**[N_ROWS];
			for(int i=0; i<N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Block*[N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<N_ROWS; i++)
				for(int j=0; j<N_COLS; j++){
					new_STACK_2D_ARRAY[i][j]=BLOCKS[N_ROWS-1-i][j];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}		
		case(2):
		{
			//allocating new_STACK_2D_ARRAY
			new_STACK_2D_ARRAY = new Block**[N_ROWS];
			for(int i=0; i<N_ROWS; i++)
				new_STACK_2D_ARRAY[i] = new Block*[N_COLS];

			//populating new_STACK_2D_ARRAY
			for(int i=0; i<N_ROWS; i++)
				for(int j=0; j<N_COLS; j++){
					new_STACK_2D_ARRAY[i][j] = BLOCKS[i][N_COLS-1-j];
					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
				}
				break;
		}
		default: break;
	}

	//deallocating current STACK_2DARRAY object
	for(int row=0; row<N_ROWS; row++)
		delete[] BLOCKS[row];
	delete[] BLOCKS;

	BLOCKS = new_STACK_2D_ARRAY;
}

void BlockVolume::loadXML(const char *xml_filepath)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::loadXML(char *xml_filepath = %s)\n", xml_filepath);
	#endif

	TiXmlDocument xml;
	if(!xml.LoadFile(xml_filepath))
	{
		char errMsg[2000];
		sprintf(errMsg,"in BlockVolume::loadXML(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
		throw iom::exception(errMsg);
	}

	//setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));

	// 2014-09-10. Alessandro. @ADDED 'volume_format' attribute to <TeraStitcher> XML node
	const char *volformat = hRoot.ToElement()->Attribute("volume_format");
	if(volformat && strcmp(volformat, id.c_str()) != 0)
		throw iom::exception(vm::strprintf("in StackedVolume::initFromXML(): unsupported volume_format = \"%s\" (current format is \"%s\")", volformat, id.c_str()).c_str());

	//reading fields and checking coherence with metadata previously read from VM_BIN_METADATA_FILE_NAME
	TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
	if(strcmp(pelem->Attribute("value"), stacks_dir) != 0)
	{
		char errMsg[2000];
		sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch in <stacks_dir> field between xml file (=\"%s\") and %s (=\"%s\").", pelem->Attribute("value"), vm::BINARY_METADATA_FILENAME.c_str(), stacks_dir);
		throw iom::exception(errMsg);
	}
	// 2014-11-06. Giulio. @ADDED saved reference system into XML file
	vm::ref_sys reference_system_read;
	if ( (pelem = hRoot.FirstChildElement("ref_sys").Element()) != 0 ) { // skip if not present (for compatibility with previous versions)
		pelem->QueryIntAttribute("ref1", (int *) &reference_system_read.first);
		pelem->QueryIntAttribute("ref2", (int *) &reference_system_read.second);
		pelem->QueryIntAttribute("ref3", (int *) &reference_system_read.third);
		if (reference_system_read.first != reference_system.first || reference_system_read.second != reference_system.second || reference_system_read.third != reference_system.third ) 
		{
			char errMsg[2000];
			sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch in <erf_sys> field between xml file (= (%d,%d,%d) ) and %s (= (%d,%d,%d) ).", 
				reference_system_read.first, reference_system_read.second, reference_system_read.third, vm::BINARY_METADATA_FILENAME.c_str(), reference_system.first, reference_system.second, reference_system.third);
			throw iom::exception(errMsg);
		}
	}
	pelem = hRoot.FirstChildElement("voxel_dims").Element();
	float VXL_V_read=0.0f, VXL_H_read=0.0f, VXL_D_read=0.0f;
	pelem->QueryFloatAttribute("V", &VXL_V_read);
	pelem->QueryFloatAttribute("H", &VXL_H_read);
	pelem->QueryFloatAttribute("D", &VXL_D_read);
	if(VXL_V_read != VXL_V || VXL_H_read != VXL_H || VXL_D_read != VXL_D)
	{
		char errMsg[2000];
		sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch in <voxel_dims> field between xml file (= %.2f x %.2f x %.2f ) and %s (= %.2f x %.2f x %.2f ).", VXL_V_read, VXL_H_read, VXL_D_read, vm::BINARY_METADATA_FILENAME.c_str(), VXL_V, VXL_H, VXL_D);
		throw iom::exception(errMsg);
	}
	pelem = hRoot.FirstChildElement("origin").Element();
	float ORG_V_read=0.0f, ORG_H_read=0.0f, ORG_D_read=0.0f;
	pelem->QueryFloatAttribute("V", &ORG_V_read);
	pelem->QueryFloatAttribute("H", &ORG_H_read);
	pelem->QueryFloatAttribute("D", &ORG_D_read);
	/*if(ORG_V_read != ORG_V || ORG_H_read != ORG_H || ORG_D_read != ORG_D)
	{
		char errMsg[2000];
		sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch in <origin> field between xml file (= {%.7f, %.7f, %.7f} ) and %s (= {%.7f, %.7f, %.7f} ).", ORG_V_read, ORG_H_read, ORG_D_read, VM_BIN_METADATA_FILE_NAME, ORG_V, ORG_H, ORG_D);
		throw iom::iom::exception(errMsg);
	} @TODO: bug with float precision causes often mismatch */ 
	pelem = hRoot.FirstChildElement("mechanical_displacements").Element();
	float MEC_V_read=0.0f, MEC_H_read=0.0f;
	pelem->QueryFloatAttribute("V", &MEC_V_read);
	pelem->QueryFloatAttribute("H", &MEC_H_read);
	if(MEC_V_read != MEC_V || MEC_H_read != MEC_H)
	{
		char errMsg[2000];
		sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch in <mechanical_displacements> field between xml file (= %.1f x %.1f ) and %s (= %.1f x %.1f ).", MEC_V_read, MEC_H_read, vm::BINARY_METADATA_FILENAME.c_str(), MEC_V, MEC_H);
		throw iom::exception(errMsg);
	}
	pelem = hRoot.FirstChildElement("dimensions").Element();
	int N_ROWS_read, N_COLS_read, N_SLICES_read;
	pelem->QueryIntAttribute("stack_rows", &N_ROWS_read);
	pelem->QueryIntAttribute("stack_columns", &N_COLS_read);
	pelem->QueryIntAttribute("stack_slices", &N_SLICES_read);
	if(N_ROWS_read != N_ROWS || N_COLS_read != N_COLS || N_SLICES_read != N_SLICES)
	{
		char errMsg[2000];
		sprintf(errMsg, "in BlockVolume::loadXML(...): Mismatch between in <dimensions> field xml file (= %d x %d ) and %s (= %d x %d ).", N_ROWS_read, N_COLS_read, vm::BINARY_METADATA_FILENAME.c_str(), N_ROWS, N_COLS);
		throw iom::exception(errMsg);
	}

	pelem = hRoot.FirstChildElement("STACKS").Element()->FirstChildElement();
	int i,j;
	for(i=0; i<N_ROWS; i++)
		for(j=0; j<N_COLS; j++, pelem = pelem->NextSiblingElement())
			BLOCKS[i][j]->loadXML(pelem, N_SLICES);
}

void BlockVolume::initFromXML(const char *xml_filepath)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::initFromXML(char *xml_filename = %s)\n", xml_filename);
		#endif

	TiXmlDocument xml;
	if(!xml.LoadFile(xml_filepath))
	{
		char errMsg[2000];
		sprintf(errMsg,"in BlockVolume::initFromXML(xml_filepath = \"%s\") : unable to load xml", xml_filepath);
		throw iom::exception(errMsg);
	}

	//setting ROOT element (that is the first child, i.e. <TeraStitcher> node)
	TiXmlHandle hRoot(xml.FirstChildElement("TeraStitcher"));

	// 2014-09-10. Alessandro. @ADDED 'volume_format' attribute to <TeraStitcher> XML node
	const char *volformat = hRoot.ToElement()->Attribute("volume_format");
	if(volformat && strcmp(volformat, id.c_str()) != 0)
		throw iom::exception(vm::strprintf("in StackedVolume::initFromXML(): unsupported volume_format = \"%s\" (current format is \"%s\")", volformat, id.c_str()).c_str());

	//reading fields
	TiXmlElement * pelem = hRoot.FirstChildElement("stacks_dir").Element();
	// 2014-11-06. Giulio. @ADDED saved reference system into XML file
	if ( (pelem = hRoot.FirstChildElement("ref_sys").Element()) != 0 ) { // skip if not present (for compatibility with previous versions)
		pelem->QueryIntAttribute("ref1", (int *) &reference_system.first);
		pelem->QueryIntAttribute("ref2", (int *) &reference_system.second);
		pelem->QueryIntAttribute("ref3", (int *) &reference_system.third);
	}
	else {
		// 2014-11-06. Giulio. @MOVED in case XML is old
		// 2014-09-09. Alessandro. @FIXED. Added default reference system if volume is imported from xml.
		reference_system = vm::ref_sys(vm::vertical,vm::horizontal,vm::depth);
	}
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
	BLOCKS = new Block **[N_ROWS];
	for(int i = 0; i < N_ROWS; i++)
	{
		BLOCKS[i] = new Block *[N_COLS];
		for(int j = 0; j < N_COLS; j++, pelem = pelem->NextSiblingElement())
		{
			BLOCKS[i][j] = new Block(this, i, j, pelem->Attribute("DIR_NAME"));
			BLOCKS[i][j]->loadXML(pelem, N_SLICES);
		}
	}

	// 2014-09-09. Alessandro. @FIXED both 'init()' and 'initFromXML()' methods to deal with empty stacks. Added call of 'normalize_stacks_attributes()' method.
	// make stacks have the same attributes
	normalize_stacks_attributes();
}

void BlockVolume::saveXML(const char *xml_filename, const char *xml_filepath) throw (iom::exception)
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin BlockVolume::saveXML(char *xml_filename = %s)\n", xml_filename);
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
        throw iom::exception("in BlockVolume::saveXML(...): no xml path provided");

	//initializing XML file with DTD declaration
	fstream XML_FILE(xml_abs_path, ios::out);
	XML_FILE<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	XML_FILE<<"<!DOCTYPE TeraStitcher SYSTEM \"TeraStitcher.DTD\">"<<endl;
	XML_FILE.close();

	//loading previously initialized XML file 
    if(!xml.LoadFile(xml_abs_path))
	{
		char errMsg[5000];
                sprintf(errMsg, "in BlockVolume::saveToXML(...) : unable to load xml file at \"%s\"", xml_abs_path);
		throw iom::exception(errMsg);
	}

	//inserting root node <TeraStitcher> and children nodes
	root = new TiXmlElement("TeraStitcher");  
	xml.LinkEndChild( root );  

	// 2014-09-10. Alessandro. @ADDED 'volume_format' attribute to <TeraStitcher> XML node
	root->SetAttribute("volume_format", id.c_str());

	pelem = new TiXmlElement("stacks_dir");
	pelem->SetAttribute("value", stacks_dir);
	root->LinkEndChild(pelem);
	// 2014-11-06. Giulio. @ADDED saved reference system into XML file
	pelem = new TiXmlElement("ref_sys");
	pelem->SetAttribute("ref1", reference_system.first);
	pelem->SetAttribute("ref2", reference_system.second);
	pelem->SetAttribute("ref3", reference_system.third);
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
			pelem->LinkEndChild(BLOCKS[i][j]->getXML());
	root->LinkEndChild(pelem);
	//saving the file
	xml.SaveFile();
}

//counts the total number of displacements and the number of displacements per stack
void BlockVolume::countDisplacements(int& total, float& per_stack_pair)
{
    /* PRECONDITIONS: none */
    total = 0;
	per_stack_pair = 0.0f;
    for(int i=0; i<N_ROWS; i++)
        for(int j=0; j<N_COLS; j++)
        {
            total+= static_cast<int>(BLOCKS[i][j]->getEAST().size());
            total+= static_cast<int>(BLOCKS[i][j]->getSOUTH().size());
            per_stack_pair += static_cast<int>(BLOCKS[i][j]->getEAST().size());
            per_stack_pair += static_cast<int>(BLOCKS[i][j]->getSOUTH().size());
        }
    per_stack_pair /= 2*(N_ROWS*N_COLS) - N_ROWS - N_COLS;
}

//counts the number of single-direction displacements having a reliability measure above the given threshold
void BlockVolume::countReliableSingleDirectionDisplacements(float threshold, int& total, int& reliable)
{
    /* PRECONDITIONS:
     *   - for each pair of adjacent stacks one and only one displacement exists (CHECKED) */

    total = reliable = 0;
    for(int i=0; i<N_ROWS; i++)
        for(int j=0; j<N_COLS; j++)
        {
            if(j != (N_COLS-1) && BLOCKS[i][j]->getEAST().size()==1)
            {
                total+=3;
                reliable += BLOCKS[i][j]->getEAST()[0]->getReliability(dir_vertical) >= threshold;
                reliable += BLOCKS[i][j]->getEAST()[0]->getReliability(dir_horizontal) >= threshold;
                reliable += BLOCKS[i][j]->getEAST()[0]->getReliability(dir_depth) >= threshold;
            }
            if(i != (N_ROWS-1) && BLOCKS[i][j]->getSOUTH().size()==1)
            {
                total+=3;
                reliable += BLOCKS[i][j]->getSOUTH()[0]->getReliability(dir_vertical) >= threshold;
                reliable += BLOCKS[i][j]->getSOUTH()[0]->getReliability(dir_horizontal) >= threshold;
                reliable += BLOCKS[i][j]->getSOUTH()[0]->getReliability(dir_depth) >= threshold;
            }
        }
}

//counts the number of stitchable stacks given the reliability threshold
int BlockVolume::countStitchableStacks(float threshold)
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
            Block* stk = BLOCKS[i][j];
            if(i!= 0 && BLOCKS[i][j]->getNORTH().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getNORTH()[0]->getReliability(direction(k)) >= threshold);
            if(j!= (N_COLS -1) && BLOCKS[i][j]->getEAST().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getEAST()[0]->getReliability(direction(k)) >= threshold);
            if(i!= (N_ROWS -1) && BLOCKS[i][j]->getSOUTH().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getSOUTH()[0]->getReliability(direction(k)) >= threshold);
            if(j!= 0 && BLOCKS[i][j]->getWEST().size()==1)
                for(int k=0; k<3; k++)
                    stitchable = stitchable || (stk->getWEST()[0]->getReliability(direction(k)) >= threshold);
            stitchables += stitchable;
        }
    return stitchables;
}

// 2014-09-05. Alessandro. @ADDED 'normalize_stacks_attributes()' method to normalize stacks attributes (width, height, etc.)
void BlockVolume::normalize_stacks_attributes() throw (iom::exception)
{
	std::set<int> heights, widths, nbytes, nchans;
	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
		{
			// exclude empty stacks (that are expected to have invalid WIDTH and HEIGHT)
			if(BLOCKS[i][j]->isEmpty())
				continue;

			heights.insert(BLOCKS[i][j]->HEIGHT);
			widths.insert(BLOCKS[i][j]->WIDTH);
			nbytes.insert(BLOCKS[i][j]->N_BYTESxCHAN);
			nchans.insert(BLOCKS[i][j]->N_CHANS);

		}

	// make the checks
	if(heights.size() != 1)
		throw iom::exception("in BlockVolume::check_stacks_same_dims(...): Stacks have unequal 'HEIGHT' attribute. This feature is not supported yet.");
	if(widths.size() != 1)
		throw iom::exception("in BlockVolume::check_stacks_same_dims(...): Stacks have unequal 'WIDTH' attribute. This feature is not supported yet.");
	if(nbytes.size() != 1)
		throw iom::exception("in BlockVolume::check_stacks_same_dims(...): Stacks have unequal 'N_BYTESxCHAN' attribute. This feature is not supported yet.");
	if(nchans.size() != 1)
		throw iom::exception("in BlockVolume::check_stacks_same_dims(...): Stacks have unequal 'N_CHANS' attribute. This feature is not supported yet.");

	// make empty stacks having the same attributes of other stacks
	for(int i=0; i<N_ROWS; i++)
		for(int j=0; j<N_COLS; j++)
			if(BLOCKS[i][j]->isEmpty())
			{
				BLOCKS[i][j]->HEIGHT = *(heights.begin());
				BLOCKS[i][j]->WIDTH = *(widths.begin());
				BLOCKS[i][j]->N_CHANS = *(nchans.begin());
				BLOCKS[i][j]->N_BYTESxCHAN = *(nbytes.begin());
			}
}
