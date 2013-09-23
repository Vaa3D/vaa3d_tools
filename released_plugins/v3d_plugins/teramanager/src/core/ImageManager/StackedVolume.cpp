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

#include <iostream>
#include <string>
#include "StackedVolume.h"
#include "Stack.h"
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include <list>
#include <fstream>
#include "ProgressBar.h"

using namespace std;

StackedVolume::StackedVolume(const char* _root_dir)  throw (MyException)
: VirtualVolume(_root_dir) // iannello ADDED
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::StackedVolume(_root_dir=%s)\n",	_root_dir);
	#endif

	// iannello this->root_dir = new char[strlen(_root_dir)+1];
	// iannello strcpy(this->root_dir,_root_dir);

	// iannello VXL_V = VXL_H = VXL_D = ORG_V = ORG_H = ORG_D = 0;
	DIM_V = DIM_H = DIM_D = 0;
	N_ROWS = N_COLS = 0;
	STACKS = NULL;
	reference_system.first = reference_system.second = reference_system.third = axis_invalid;
	VXL_1 = VXL_2 = VXL_3 = 0;

	//without any configuration parameter, volume import must be done from the metadata file stored in the root directory, if it exists
	char mdata_filepath[IM_STATIC_STRINGS_SIZE];
	sprintf(mdata_filepath, "%s/%s", root_dir, IM_METADATA_FILE_NAME);
	if(fileExists(mdata_filepath)) 
	{
		load(mdata_filepath);
		initChannels();
	}
	else
	{
		char errMsg[IM_STATIC_STRINGS_SIZE];
		sprintf(errMsg, "in StackedVolume::StackedVolume(...): unable to find metadata file at %s", mdata_filepath);
        throw MyException(errMsg);
	}
}

StackedVolume::StackedVolume(const char* _root_dir, ref_sys _reference_system, float _VXL_1, float _VXL_2, float _VXL_3, bool overwrite_mdata, bool save_mdata)  throw (MyException)
: VirtualVolume(_root_dir) // iannello ADDED
{
    #if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::StackedVolume(_root_dir=%s, ref_sys reference_system={%d,%d,%d}, VXL_1=%.4f, VXL_2=%.4f, VXL_3=%.4f)\n",
                          _root_dir, _reference_system.first, _reference_system.second, _reference_system.third, _VXL_1, _VXL_2, _VXL_3);
	#endif

	// iannello this->root_dir = new char[strlen(_root_dir)+1];
	// iannello strcpy(this->root_dir,_root_dir);

	// iannello VXL_V = VXL_H = VXL_D = ORG_V = ORG_H = ORG_D = 0;
	DIM_V = DIM_H = DIM_D = 0;
	N_ROWS = N_COLS = 0;
	STACKS = NULL;
	reference_system.first = reference_system.second = reference_system.third = axis_invalid;
	VXL_1 = VXL_2 = VXL_3 = 0;

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
	char mdata_filepath[IM_STATIC_STRINGS_SIZE];
	sprintf(mdata_filepath, "%s/%s", root_dir, IM_METADATA_FILE_NAME);
	if(fileExists(mdata_filepath) && !overwrite_mdata)
		load(mdata_filepath);
	else
	{
        if(_reference_system.first == axis_invalid ||  _reference_system.second == axis_invalid ||
          _reference_system.third == axis_invalid || _VXL_1 == 0 || _VXL_2 == 0 || _VXL_3 == 0)
            throw MyException("in StackedVolume::StackedVolume(...): invalid importing parameters");

        reference_system.first  = _reference_system.first;
        reference_system.second = _reference_system.second;
        reference_system.third  = _reference_system.third;
        VXL_1 = _VXL_1;
        VXL_2 = _VXL_2;
        VXL_3 = _VXL_3;
        init();
        if(save_mdata)
            save(mdata_filepath);
	}
	initChannels();
}

StackedVolume::~StackedVolume(void)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::~StackedVolume(void)\n");
	#endif

	// iannello if(root_dir)
	// iannello 	delete[] root_dir;

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


int StackedVolume::getStacksHeight(){return STACKS[0][0]->getHEIGHT();}
int StackedVolume::getStacksWidth(){return STACKS[0][0]->getWIDTH();}

void StackedVolume::save(char* metadata_filepath) throw (MyException)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::save(metadata_filepath=%s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	FILE *file;
	int i,j;

        file = fopen(metadata_filepath, "wb");

        // --- Alessandro 2013-04-23: added exception when file can't be opened in write mode
        if(!file)
        {
            char errMsg[IM_STATIC_STRINGS_SIZE];
            sprintf(errMsg, "in StackedVolume::save(): cannot write metadata binary file at \"%s\".\n\nPlease check write permissions on this storage.", metadata_filepath);
            throw MyException(errMsg);
        }

        float mdata_version = static_cast<float>(IM_METADATA_FILE_VERSION);
        fwrite(&mdata_version, sizeof(float), 1, file); // --- Alessandro 2012-12-31: added field for metadata file version
        //str_size = (uint16)(strlen(root_dir) + 1);    // --- Alessandro 2012-12-31: absolute filepaths in mdata.bin eliminated
        //fwrite(&str_size, sizeof(uint16), 1, file);   // --- Alessandro 2012-12-31: absolute filepaths in mdata.bin eliminated
        //fwrite(root_dir, str_size, 1, file);          // --- Alessandro 2012-12-31: absolute filepaths in mdata.bin eliminated
        fwrite(&reference_system.first, sizeof(axis), 1, file);
        fwrite(&reference_system.second, sizeof(axis), 1, file); // iannello CORRECTED
        fwrite(&reference_system.third, sizeof(axis), 1, file);  // iannello CORRECTED
        fwrite(&VXL_1, sizeof(float), 1, file);
        fwrite(&VXL_2, sizeof(float), 1, file);
        fwrite(&VXL_3, sizeof(float), 1, file);
	fwrite(&VXL_V, sizeof(float), 1, file);
	fwrite(&VXL_H, sizeof(float), 1, file);
	fwrite(&VXL_D, sizeof(float), 1, file);
	fwrite(&ORG_V, sizeof(float), 1, file);
	fwrite(&ORG_H, sizeof(float), 1, file);
	fwrite(&ORG_D, sizeof(float), 1, file);
	fwrite(&DIM_V, sizeof(uint32), 1, file);
	fwrite(&DIM_H, sizeof(uint32), 1, file);
	fwrite(&DIM_D, sizeof(uint32), 1, file);
	fwrite(&N_ROWS, sizeof(uint16), 1, file);
	fwrite(&N_COLS, sizeof(uint16), 1, file);

	for(i = 0; i < N_ROWS; i++)
		for(j = 0; j < N_COLS; j++)
			STACKS[i][j]->binarizeInto(file);

	fclose(file);
}

void StackedVolume::load(char* metadata_filepath) throw (MyException)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::load(metadata_filepath=%s)\n", metadata_filepath);
	#endif

	//LOCAL VARIABLES
	FILE *file;
	int i,j;
	size_t fread_return_val;

	file = fopen(metadata_filepath, "rb");

        // --- Alessandro 2013-04-23: added exception when file can't be opened in read mode
        if(!file)
        {
            char errMsg[IM_STATIC_STRINGS_SIZE];
            sprintf(errMsg, "in StackedVolume::load(): cannot read metadata binary file at \"%s\".\n\nPlease check read permissions on this storage.", metadata_filepath);
            throw MyException(errMsg);
        }

        // --- Alessandro 2012-12-31: added field for metadata file version
        float mdata_version_read = 0;
        float mdata_version = static_cast<float>(IM_METADATA_FILE_VERSION);
        fread_return_val = fread(&mdata_version_read, sizeof(float), 1, file);
        if(fread_return_val != 1 || mdata_version_read != mdata_version)
        {
            // --- Alessandro 2013-01-06: instead of throwing an exception, it is better to mantain compatibility
//            char errMsg[IM_STATIC_STRINGS_SIZE];
//            sprintf(errMsg, "in Stack::unBinarizeFrom(...): metadata file version (%.2f) is different from the supported one (%.2f). "
//                    "Please re-import the current volume.", mdata_version_read, mdata_version);
//            throw MyException(errMsg);

            fclose(file);
            file = fopen(metadata_filepath, "rb");
            uint16 str_size;
            fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
            if(fread_return_val != 1)
                    throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
            char stored_root_dir[IM_STATIC_STRINGS_SIZE];
            fread_return_val = fread(stored_root_dir, str_size, 1, file);
            if(fread_return_val != 1)
                    throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");
        }



        fread_return_val = fread(&reference_system.first, sizeof(axis), 1, file);
        if(fread_return_val != 1)
                throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

        fread_return_val = fread(&reference_system.second, sizeof(axis), 1, file);
        if(fread_return_val != 1)
                throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

        fread_return_val = fread(&reference_system.third, sizeof(axis), 1, file);

        fread_return_val = fread(&VXL_1, sizeof(float), 1, file);
        if(fread_return_val != 1)
                throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

        fread_return_val = fread(&VXL_2, sizeof(float), 1, file);
        if(fread_return_val != 1)
                throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

        fread_return_val = fread(&VXL_3, sizeof(float), 1, file);
        if(fread_return_val != 1)
                throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&VXL_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&VXL_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&VXL_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&ORG_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&ORG_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&ORG_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&DIM_V, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&DIM_H, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&DIM_D, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&N_ROWS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");

	fread_return_val = fread(&N_COLS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
		throw MyException("in Stack::unBinarizeFrom(...): error while reading binary metadata file");


	STACKS = new Stack **[N_ROWS];
	for(i = 0; i < N_ROWS; i++)
	{
		STACKS[i] = new Stack *[N_COLS];
		for(j = 0; j < N_COLS; j++)
			STACKS[i][j] = new Stack(this, i, j, file);
	}

	fclose(file);
}

void StackedVolume::init()
{
	#if IM_VERBOSE > 3
        printf("\t\t\t\tin StackedVolume::init()\n");
	#endif

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	//LOCAL VARIABLES
	string tmp_path;				//string that contains temp paths during computation
        string tmp;						//string that contains temp data during computation
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	DIR *cur_dir_lev2;				//pointer to DIR, the data structure that represents a DIRECTORY (level 2 of hierarchical structure)
	dirent *entry_lev1;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 1)
	dirent *entry_lev2;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 2)
	int i=0,j=0;					//for counting of N_ROWS, N_COLS
        list<Stack*> stacks_list;                       //each stack found in the hierarchy is pushed into this list
        list<string> entries_lev1;                      //list of entries of first level of hierarchy
        list<string>::iterator entry_i;                 //iterator for list 'entries_lev1'
        list<string> entries_lev2;                      //list of entries of second level of hierarchy
        list<string>::iterator entry_j;                 //iterator for list 'entries_lev2'
	char stack_i_j_path[IM_STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(root_dir)))
	{
            char msg[IM_STATIC_STRINGS_SIZE];
            sprintf(msg,"in StackedVolume::init(...): Unable to open directory \"%s\"", root_dir);
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

	//for each entry of first level, scanning second level
	for(entry_i = entries_lev1.begin(), i=0; entry_i!= entries_lev1.end(); entry_i++, i++)
	{
		//building absolute path of first level entry to be used for "opendir(...)"
		tmp_path=root_dir;
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

	/******************* 2) SETTING THE REFERENCE SYSTEM ********************
	The entire application uses a vertical-horizontal reference system, so
	it is necessary to fit the original reference system into the new one.     
	*************************************************************************/

	//adjusting possible sign mismatch betwwen reference system and VXL
	//in these cases VXL is adjusted to match with reference system
	if(SIGN(reference_system.first) != SIGN(VXL_1))
            VXL_1*=-1.0f;
	if(SIGN(reference_system.second) != SIGN(VXL_2))
            VXL_2*=-1.0f;
	if(SIGN(reference_system.third) != SIGN(VXL_3))
            VXL_3*=-1.0f;

	//HVD --> VHD
        if  (abs(reference_system.first)==2 && abs(reference_system.second)==1  && reference_system.third==3)
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
	}
	//unsupported reference system
	else
	{
            char msg[IM_STATIC_STRINGS_SIZE];
            sprintf(msg, "in StackedVolume::init(...): the reference system {%d,%d,%d} is not supported.",
                    reference_system.first, reference_system.second, reference_system.third);
            throw MyException(msg);
	}

	//some little adjustments of the origin
	if(VXL_V < 0)
            ORG_V -= (STACKS[0][0]->getHEIGHT()-1)* VXL_V/1000.0f;

	if(VXL_H < 0)
            ORG_H -= (STACKS[0][0]->getWIDTH() -1)* VXL_H/1000.0f;

	/******************** 3) COMPUTING VOLUME DIMENSIONS ********************  
	*************************************************************************/
	for(int row=0; row < N_ROWS; row++)
            for(int col=0; col < N_COLS; col++)
            {
                if(row==0)
                        DIM_H+=STACKS[row][col]->getWIDTH();
                if(col==0)
                        DIM_V+=STACKS[row][col]->getHEIGHT();
                DIM_D = STACKS[row][col]->getDEPTH() > DIM_D ? STACKS[row][col]->getDEPTH() : DIM_D;
            }

	/**************** 4) COMPUTING STACKS ABSOLUTE POSITIONS ****************  
	*************************************************************************/
	for(int row=0; row < N_ROWS; row++)
            for(int col=0; col < N_COLS; col++)
            {
                if(row)
                        STACKS[row][col]->setABS_V(STACKS[row-1][col]->getABS_V()+STACKS[row-1][col]->getHEIGHT());
                else
                        STACKS[row][col]->setABS_V(0);

                if(col)
                        STACKS[row][col]->setABS_H(STACKS[row][col-1]->getABS_H()+STACKS[row][col-1]->getWIDTH());
                else
                        STACKS[row][col]->setABS_H(0);
            }
}

void StackedVolume::initChannels ( ) throw (MyException) {
	char slice_fullpath[IM_STATIC_STRINGS_SIZE];

	sprintf(slice_fullpath, "%s/%s/%s", root_dir, STACKS[0][0]->getDIR_NAME(), STACKS[0][0]->getFILENAMES()[0]);
	IplImage* slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
	if(!slice)
		throw MyException(std::string("Unable to load slice at \"").append(slice_fullpath).append("\"").c_str());
    CHANS = slice->nChannels;
	if ( slice->depth == IPL_DEPTH_8U )
		BYTESxCHAN = 1; 
	else if ( slice->depth == IPL_DEPTH_16U )
		BYTESxCHAN = 2; 
	else if ( slice->depth == IPL_DEPTH_32F )
		BYTESxCHAN = 4;
	else {
		char msg[IM_STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolume::initChannels: unknown color depth");
		throw MyException(msg);
	}

	cvReleaseImage(&slice);
}

//PRINT method
void StackedVolume::print()
{
	printf("*** Begin printing StakedVolume object...\n\n");
	printf("\tDirectory:\t%s\n", root_dir);
	printf("\tDimensions:\t%d(V) x %d(H) x %d(D)\n", DIM_V, DIM_H, DIM_D);
	printf("\tVoxels:\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", VXL_V, VXL_H, VXL_D);
	printf("\tOrigin:\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", ORG_V, ORG_H, ORG_D);
	printf("\tStacks matrix:\t%d(V) x %d(H)\n", N_ROWS, N_COLS);
	printf("\t |\n");
	for(int row=0; row<N_ROWS; row++)
		for(int col=0; col<N_COLS; col++)
			STACKS[row][col]->print();
	printf("\n*** END printing StakedVolume object...\n\n");
}

//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
void StackedVolume::rotate(int theta)
{
	//PRECONDITIONS:
	//	1) current StackedVolume object has been initialized (init() method has been called)
	//	2) accepted values for 'theta' are 0,90,180,270

	//POSTCONDITIONS:
	//  1) a new 2D-array of Stack* objects is created considering a rotation of 'theta' angle of current StackedVolume object

	Stack*** new_STACK_2D_ARRAY = NULL;
	int new_N_ROWS = 0, new_N_COLS = 0;

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
	//PRECONDITIONS:
	//	1) current StackedVolume object has been initialized (init() method has been called)
	//	2) accepted values for 'mrr_axis' are 1(V axis), 2(H axis) or 3(D axis)

	//POSTCONDITIONS:
	//  1) a new 2D-array of Stack* objects is created considering a mirrorization along 'axis' of current StackedVolume object

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
		throw msg;
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
			throw msg;
		}
	}
}

//loads given subvolume in a 1-D array of float
REAL_T* StackedVolume::loadSubvolume(int V0,int V1, int H0, int H1, int D0, int D1, list<Stack*> *involved_stacks, bool release_stacks) throw (MyException)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin StackedVolume::loadSubvolume(V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d%s)\n", V0, V1, H0, H1, D0, D1, (involved_stacks? ", involved_stacks" : ""));
	#endif

    //checking for non implemented features
	if( this->BYTESxCHAN != 1 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"StackedVolume::loadSubvolume: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
		throw MyException(err_msg);
	}

	//initializations
	V0 = (V0 == -1 ? 0	     : V0);
	V1 = (V1 == -1 ? DIM_V   : V1);
	H0 = (H0 == -1 ? 0	     : H0);
	H1 = (H1 == -1 ? DIM_H   : H1);
	D0 = (D0 == -1 ? 0		 : D0);
	D1 = (D1 == -1 ? DIM_D	 : D1);

	//allocation
	sint64 sbv_height = V1 - V0;
	sint64 sbv_width  = H1 - H0;
	sint64 sbv_depth  = D1 - D0;
	REAL_T *subvol = new REAL_T[sbv_height * sbv_width * sbv_depth];

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;
	for(int row=0; row<N_ROWS; row++)
		for(int col=0; col<N_COLS; col++)
		{
			Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
			if(intersect_area)
			{
				#if IM_VERBOSE > 4
				printf("\t\t\t\tin StackedVolume::loadSubvolume(): using STACK[%d,%d] for area %d-%d(V) x %d-%d(H)\n", row, col, intersect_area->V0-V0, intersect_area->V1-V0, intersect_area->H0-H0, intersect_area->H1-H0);
				#endif

				STACKS[row][col]->loadStack(D0, D1-1);
				if(involved_stacks)
					involved_stacks->push_back(STACKS[row][col]);

				for(int k=0; k<sbv_depth; k++)
				{
					CvMat *slice = STACKS[row][col]->getSTACKED_IMAGE()[D0+k];
					int   step  = slice->step/sizeof(float);
					float *data = slice->data.fl;
					int ABS_V_stk = STACKS[row][col]->getABS_V();
					int ABS_H_stk = STACKS[row][col]->getABS_H();

					for(int i=intersect_area->V0-V0; i<intersect_area->V1-V0; i++)
						for(int j=intersect_area->H0-H0; j<intersect_area->H1-H0; j++)
							subvol[k*sbv_height*sbv_width + i*sbv_width + j] = (data+(i-ABS_V_stk+V0)*step)[j-ABS_H_stk+H0];
				}

				if(release_stacks)
					STACKS[row][col]->releaseStack();
			}
		}
	return subvol;
}

//loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
uint8* StackedVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type) throw (MyException)
{
    #if IM_VERBOSE > 3
    printf("\t\t\t\tin StackedVolume::loadSubvolume_to_UINT8(V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d)\n", V0, V1, H0, H1, D0, D1);
    #endif

    //checking for non implemented features
	if( this->BYTESxCHAN != 1 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"StackedVolume::loadSubvolume_to_UINT8: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
		throw MyException(err_msg);
	}

	if ( (ret_type == IM_DEF_IMG_DEPTH) && ((8 * this->BYTESxCHAN) != IM_DEF_IMG_DEPTH)  ) {
		// return type is 8 bits, but native depth is not 8 bits
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"RawVolume::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
		throw MyException(err_msg);
	}

    //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED
    uint8 *subvol = 0;

    //checking that the interval is valid
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw MyException("in StackedVolume::loadSubvolume_to_UINT8: invalid subvolume intervals");

    //computing dimensions
    sint64 sbv_height = V1 - V0;
    sint64 sbv_width  = H1 - H0;
    sint64 sbv_depth  = D1 - D0;

    //initializing the number of channels with an undefined value (it will be detected from the first slice read)
    sint64 sbv_channels = -1;

    //scanning of stacks matrix for data loading and storing into subvol
    Rect_t subvol_area;
    subvol_area.H0 = H0;
    subvol_area.V0 = V0;
    subvol_area.H1 = H1;
    subvol_area.V1 = V1;
    char slice_fullpath[IM_STATIC_STRINGS_SIZE];
    bool first_time = true;
    for(int row=0; row<N_ROWS; row++)
        for(int col=0; col<N_COLS; col++)
        {
            Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
            if(intersect_area)
            {
                #if IM_VERBOSE > 3
                printf("\t\t\t\tin StackedVolume::loadSubvolume_to_UINT8(): using STACK[%d,%d] for area %d-%d(V) x %d-%d(H)\n", row, col, intersect_area->V0-V0, intersect_area->V1-V0, intersect_area->H0-H0, intersect_area->H1-H0);
                #endif

                for(int k=0; k<sbv_depth; k++)
                {
                    //loading slice
                    sprintf(slice_fullpath, "%s/%s/%s", root_dir, STACKS[row][col]->getDIR_NAME(), STACKS[row][col]->getFILENAMES()[D0+k]);
                    IplImage* slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
                    if(!slice)
                        throw MyException(std::string("Unable to load slice at \"").append(slice_fullpath).append("\"").c_str());

                    //if this is the first time a slice is loaded, detecting the number of channels and safely allocating memory for data
                    if(first_time)
                    {
                        first_time = false;
                        sbv_channels = slice->nChannels;
                        if(sbv_channels != 1 && sbv_channels != 3)
                            throw MyException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());

                        try
                        {
                            subvol = new uint8[sbv_height * sbv_width * sbv_depth * sbv_channels];
                        }
                        catch(...){throw MyException("in StackedVolume::loadSubvolume_to_UINT8: unable to allocate memory");}
                    }
                    //otherwise checking that all the other slices have the same bitdepth of the first one
                    else if(slice->nChannels != sbv_channels)
                        throw MyException(std::string("Image depth mismatch at slice at \"").append(slice_fullpath).append("\": all slices must have the same bitdepth").c_str());


                    //computing offsets
                    int slice_step = slice->widthStep / sizeof(uint8);
                    int ABS_V_offset = V0 - STACKS[row][col]->getABS_V();
                    int ABS_H_offset = (H0 - STACKS[row][col]->getABS_H())*((int)sbv_channels);

                    //different procedures for 1 and 3 channels images
                    int istart, iend, jstart, jend;
                    istart  = intersect_area->V0-V0;
                    iend    = intersect_area->V1-V0;
                    jstart  = intersect_area->H0-H0;
                    jend    = intersect_area->H1-H0;
                    if(sbv_channels == 1)
                    {
                        sint64 k_offset = k*sbv_height*sbv_width;
                        for(int i = istart; i < iend; i++)
                        {
                            uint8* slice_row = ((uint8*)slice->imageData) + (i+ABS_V_offset)*slice_step;
                            for(int j = jstart; j < jend; j++)
                                subvol[k_offset + i*sbv_width + j] = slice_row[j+ABS_H_offset];
                        }
                    }
                    else if(sbv_channels == 3)
                    {

                        sint64 offset1 =                                     k*sbv_height*sbv_width;
                        sint64 offset2 =   sbv_height*sbv_width*sbv_depth  + offset1;
                        sint64 offset3 = 2*sbv_height*sbv_width*sbv_depth  + k*sbv_height*sbv_width;
                        for(int i = istart; i < iend; i++)
                        {
                            uint8* slice_row = ((uint8*)slice->imageData) + (i+ABS_V_offset)*slice_step;
                            for(int j1 = jstart, j2 = jstart*3; j1 < jend; j1++, j2+=3)
                            {
                                subvol[offset1 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset + 2];
                                subvol[offset2 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset + 1];
                                subvol[offset3 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset];
                            }
                        }
                    }
                    else
                        throw MyException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());

                    cvReleaseImage(&slice);
                }
            }
        }

    //returning outputs
    if(channels)
        *channels = (int)sbv_channels;
    return subvol;
}

//show the selected slice with a simple GUI
void StackedVolume::show(REAL_T *vol, int vol_DIM_V, int vol_DIM_H, int D_index, int window_HEIGHT, int window_WIDTH)
{
	#if IM_VERBOSE > 2
	printf("\t\t\tin StackedVolume::show(REAL_T *vol, vol_DIM_V=%d, vol_DIM_H=%d, D_index=%d, win_height=%d, win_width=%d)\n",
		vol_DIM_V, vol_DIM_H, D_index, window_HEIGHT, window_WIDTH);
	#endif

	//converting selected slice of vol (1-D array of REAL_T) into a CvMat
	CvMat *slice = cvCreateMat(vol_DIM_V, vol_DIM_H, CV_32FC1);
	for(int i=0; i<slice->rows; i++)
	{
		float *row_ptr = (float*)(slice->data.ptr+slice->step*i);
		for(int j=0; j<slice->cols; j++)
			row_ptr[j] = vol[D_index*vol_DIM_V*vol_DIM_H + i*vol_DIM_H +j];
	}

	//showing slice
	CvSize window_dims;
	window_dims.height = window_HEIGHT ? window_HEIGHT : vol_DIM_V;
	window_dims.width  = window_WIDTH  ? window_WIDTH  : vol_DIM_H;
	char buffer[200];
	CvMat* mat_rescaled = cvCreateMat(vol_DIM_V, vol_DIM_H, CV_16UC1);
	IplImage* image_resized = cvCreateImage(window_dims, IPL_DEPTH_16U, 1);
	IplImage* img_buffer = cvCreateImageHeader(window_dims, IPL_DEPTH_16U, 1);

	cvConvertScale(slice,mat_rescaled, 65535);
	cvResize(cvGetImage(mat_rescaled,img_buffer), image_resized, CV_INTER_CUBIC);

	sprintf(buffer,"SLICE %d of volume %d x %d",D_index, vol_DIM_V, vol_DIM_H);
	cvNamedWindow(buffer,1);
	cvShowImage(buffer,image_resized);
	while(1)
	{
		if(cvWaitKey(100)==27) break;
	}

	cvDestroyWindow(buffer);

	cvReleaseMat(&slice);
	cvReleaseMat(&mat_rescaled);
	cvReleaseImage(&image_resized);
	cvReleaseImageHeader(&img_buffer);
}

//delete slices from disk from first_file to last_file, extremes included
void StackedVolume::deleteSlices(int first_file, int last_file)
{
	for(int row=0; row<N_ROWS; row++)
		for(int col=0; col<N_COLS; col++)
			STACKS[row][col]->deleteSlices(first_file,last_file);
}

//saves in 'dir_path' the current volume (from D0 to D1) in stacked format, with the given stacks dimensions
void StackedVolume::saveVolume(const char* dir_path, uint32 max_slice_height, uint32 max_slice_width, uint32 V0, uint32 V1, uint32 H0, uint32 H1, uint32 D0, uint32 D1, const char* img_format, int img_depth) throw (MyException)
{
	//**LOCAL VARIABLES**
	interval_t *subvols_V;					//1D array of subvolumes V intervals that partition volume along V axis
	interval_t *subvols_H;					//1D array of subvolumes H intervals that partition volume along H axis
	uint32 subvols_V_size;					//size of subvols_V array
	uint32 subvols_H_size;					//size of subvols_H array
	uint32 vxl_count;						
	uint32 tmp_dim;							//TMP variable used for volume partitioning
	float V_crd, H_crd, D_crd;				
	char dir_name[IM_STATIC_STRINGS_SIZE];	//
	char err_msg[IM_STATIC_STRINGS_SIZE];	//
	REAL_T *subvol;
	uint32 dim_v, dim_h, dim_d;

	//checking and adjusting default variables
	//V0 = (V0 < 0 ? 0 : V0); // uint32: cannot be negative
	//H0 = (H0 < 0 ? 0 : H0);
	//D0 = (D0 < 0 ? 0 : D0);
	V1 = ((V1 == 0 || V1 > DIM_V) ? DIM_V : V1);
	H1 = ((H1 == 0 || H1 > DIM_H) ? DIM_H : H1);
	D1 = ((D1 == 0 || D1 > DIM_D) ? DIM_D : D1);
	if(V0 >= V1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. V0(%d) >= V1(%d)!", V0, V1);
		throw MyException(err_msg);
	}
	if(H0 >= H1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. H0(%d) >= H1(%d)!", H0, H1);
		throw MyException(err_msg);
	}
	if(D0 >= D1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. D0(%d) >= D1(%d)!", D0, D1);
		throw MyException(err_msg);
	}
	max_slice_height = (max_slice_height == 0 ? DIM_V : max_slice_height);
	max_slice_width  = (max_slice_width  == 0 ? DIM_H : max_slice_width);

	//initializations
	dim_v = V1 - V0;
	dim_h = H1 - H0;
	dim_d = D1 - D0;

	
	//computing partition of volume into overlapping subvolumes
	subvols_V_size = (int) ceil (dim_v / (float) max_slice_height);
	subvols_H_size = (int) ceil (dim_h / (float) max_slice_width);
	subvols_V = new interval_t[subvols_V_size];
	subvols_H = new interval_t[subvols_H_size];
	vxl_count = V0;
	for(uint32 i=0; i<subvols_V_size; i++)
	{
		tmp_dim = (i < dim_v % subvols_V_size ? dim_v/subvols_V_size +1 :  dim_v/subvols_V_size);
		subvols_V[i].start = vxl_count;
		subvols_V[i].end   = vxl_count + tmp_dim;
		vxl_count+=tmp_dim;
	}	
	vxl_count = H0;
	for(uint32 i=0; i<subvols_H_size; i++)
	{
		tmp_dim = (i < dim_h % subvols_H_size ? dim_h/subvols_H_size +1 :  dim_h/subvols_H_size);
		subvols_H[i].start = vxl_count;
		subvols_H[i].end   = vxl_count + tmp_dim;
		vxl_count+=tmp_dim;
	}	

	//progress bar initialization
	char progressBarBuff[2000];
        sprintf(progressBarBuff, "Volume conversion from %s-stack(height=%d, width=%d) to %s-stack(height=%d, width=%d)",
		    ((N_ROWS == 1 && N_COLS == 1)				  ? "MONO" :"MULTI"), STACKS[0][0]->getHEIGHT(), STACKS[0][0]->getWIDTH(),
			((subvols_V_size == 1 && subvols_H_size == 1) ? "MONO" :"MULTI"), subvols_V[0].end - subvols_V[0].start, subvols_H[0].end - subvols_H[0].start);
        ProgressBar::getInstance()->start(progressBarBuff);
        ProgressBar::getInstance()->update(0,"Initializing...");
        ProgressBar::getInstance()->show();

	
	//creating root directory	
	make_dir(dir_path);

	//saving stacked volume
	D_crd = ORG_D + D0 * (VXL_D/1000);
	for(uint32 k=D0; k<D1; k++, D_crd += VXL_D/1000)
	{
		sprintf(progressBarBuff, "Converting slice %d of %d", (k-D0+1), dim_d);
                ProgressBar::getInstance()->update((((REAL_T)100/dim_d)*(k-D0+1)), progressBarBuff);
                ProgressBar::getInstance()->show();

		for(uint32 i=0; i<subvols_V_size; i++)
		{
			V_crd = ORG_V + subvols_V[i].start * (VXL_V/1000);
			if(VXL_V < 0)
				V_crd+= (subvols_V[i].end - subvols_V[i].start -1)*(VXL_V/1000);
			sprintf(dir_name, "%s/%06d", dir_path, ((int)(V_crd*10000)) );
			make_dir(dir_name);

			for(uint32 j=0; j<subvols_H_size; j++)
			{
				H_crd = ORG_H + subvols_H[j].start * (VXL_H/1000);
				if(VXL_H < 0)
					H_crd+= (subvols_H[i].end - subvols_H[i].start -1)*(VXL_H/1000);
				sprintf(dir_name, "%s/%06d/%06d_%06d", dir_path, ((int)(V_crd*10000)), ((int)(V_crd*10000)), ((int)(H_crd*10000)) );
				make_dir(dir_name);

				subvol = loadSubvolume(subvols_V[i].start, subvols_V[i].end, subvols_H[j].start, subvols_H[j].end, k, k+1);
				sprintf(dir_name, "%s/%06d/%06d_%06d/%06d_%06d_%06d", dir_path, ((int)(V_crd*10000)), ((int)(V_crd*10000)), ((int)(H_crd*10000)),
																	((int)(V_crd*10000)), ((int)(H_crd*10000)), ((int)(D_crd*10000)));
				
				saveImage(dir_path, subvol, subvols_V[i].end-subvols_V[i].start, subvols_H[j].end-subvols_H[j].start, 0, -1, 0, -1, img_format, img_depth);

				delete[] subvol;	
			}
		}
		releaseStacks(k,k);
	}
	delete[] subvols_V;
	delete[] subvols_H;	
}

//saves in 'dir_path' the selected subvolume in multi-stack format, with the exact given stacks dimensions and the given overlap between adjacent stacks
void StackedVolume::saveOverlappingStacks(char* dir_path, uint32 slice_height, uint32 slice_width,	uint32 overlap_size, uint32 V0, uint32 V1, uint32 H0, uint32 H1, uint32 D0, uint32 D1) throw (MyException)
{
	//**LOCAL VARIABLES**
	interval_t *subvols_V;			//1D array of subvolumes V intervals that partition volume along V axis
	interval_t *subvols_H;			//1D array of subvolumes H intervals that partition volume along H axis
	int subvols_V_size;				//size of subvols_V array
	int subvols_H_size;				//size of subvols_H array
	int vxl_count;					//TMP variable used for volume partitioning
	float V_crd, H_crd, D_crd;		//TMP variable used for voxel coordinates
	char dir_name[IM_STATIC_STRINGS_SIZE];
	char err_msg[IM_STATIC_STRINGS_SIZE];
	REAL_T *subvol;
	IplImage *slice;
	uint32 dim_d;

	//checking and adjusting default variables
	//V0 = (V0 < 0 ? 0 : V0); // uint32: cannot be negative
	//H0 = (H0 < 0 ? 0 : H0);
	//D0 = (D0 < 0 ? 0 : D0);
	V1 = ((V1 == 0 || V1 > DIM_V) ? DIM_V : V1);
	H1 = ((H1 == 0 || H1 > DIM_H) ? DIM_H : H1);
	D1 = ((D1 == 0 || D1 > DIM_D) ? DIM_D : D1);
	if(V0 >= V1){
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. V0(%d) >= V1(%d)!", V0, V1);
		throw MyException(err_msg);
	}
	if(H0 >= H1){
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. H0(%d) >= H1(%d)!", H0, H1);
		throw MyException(err_msg);
	}
	if(D0 >= D1){
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. D0(%d) >= D1(%d)!", D0, D1);
		throw MyException(err_msg);
	}

	//initializations
	dim_d = D1 - D0;
	subvols_V_size = 0;
	subvols_H_size = 0;

	//computing dimensions along V and H as well as number of stacks
	for(uint32 dim_v_acc=overlap_size; dim_v_acc<= V1 - V0; dim_v_acc += slice_height-overlap_size)
		subvols_V_size++;
	for(uint32 dim_h_acc=overlap_size; dim_h_acc<= H1 - H0; dim_h_acc += slice_width-overlap_size)
		subvols_H_size++;

	//computing partition of volume into overlapping subvolumes
	subvols_V = new interval_t[subvols_V_size];
	subvols_H = new interval_t[subvols_H_size];
	vxl_count = V0;
	for(int i=0; i<subvols_V_size; i++)
	{
		subvols_V[i].start = vxl_count;
		subvols_V[i].end   = vxl_count + slice_height;
		vxl_count += slice_height - overlap_size;
	}	
	vxl_count = H0;
	for(int i=0; i<subvols_H_size; i++)
	{
		subvols_H[i].start = vxl_count;
		subvols_H[i].end   = vxl_count + slice_width;
		vxl_count += slice_width - overlap_size;
	}	

	//progress bar initialization
	char progressBarBuff[IM_STATIC_STRINGS_SIZE];
        sprintf(progressBarBuff, "Volume conversion from %s-stack(height=%d, width=%d) to %s-stack(height=%d, width=%d)",
		    ((N_ROWS == 1 && N_COLS == 1)				  ? "MONO" :"MULTI"), STACKS[0][0]->getHEIGHT(), STACKS[0][0]->getWIDTH(),
                        ((subvols_V_size == 1 && subvols_H_size == 1) ? "MONO" :"MULTI"), subvols_V[0].end - subvols_V[0].start, subvols_H[0].end - subvols_H[0].start);
        ProgressBar::getInstance()->start(progressBarBuff);
        ProgressBar::getInstance()->update(0,"Initializing...");
        ProgressBar::getInstance()->show();

	
	//creating root directory	
	make_dir(dir_path);

	//saving stacked volume
	D_crd = ORG_D + D0 * (VXL_D/1000.0f);
	for(uint32 k=D0; k<D1; k++, D_crd += VXL_D/1000.0f)
	{
		sprintf(progressBarBuff, "Converting slice %d of %d", (k-D0+1), dim_d);
                ProgressBar::getInstance()->update((((REAL_T)100/dim_d)*(k-D0+1)), progressBarBuff);
                ProgressBar::getInstance()->show();

		for(int i=0; i<subvols_V_size; i++)
		{
			V_crd = ORG_V + subvols_V[i].start * (VXL_V/1000.0f);
			if(VXL_V < 0)
				V_crd+= (subvols_V[i].end - subvols_V[i].start -1)*(VXL_V/1000.0f);
			sprintf(dir_name, "%s/%06d", dir_path, ((int)(V_crd*10000.0f)) );
			make_dir(dir_name);

			for(int j=0; j<subvols_H_size; j++)
			{
				H_crd = ORG_H + subvols_H[j].start * (VXL_H/1000.0f);
				if(VXL_H < 0)
					H_crd+= (subvols_H[i].end - subvols_H[i].start -1)*(VXL_H/1000.0f);
				sprintf(dir_name, "%s/%06d/%06d_%06d", dir_path, ((int)(V_crd*10000.0f)), ((int)(V_crd*10000.0f)), ((int)(H_crd*10000.0f)) );
				make_dir(dir_name);

				subvol = loadSubvolume(subvols_V[i].start, subvols_V[i].end, subvols_H[j].start, subvols_H[j].end, k, k+1);
				sprintf(dir_name, "%s/%06d/%06d_%06d/%06d_%06d_%06d.tif", dir_path, ((int)(V_crd*10000.0f)), ((int)(V_crd*10000.0f)), ((int)(H_crd*10000.0f)),
																	((int)(V_crd*10000.0f)), ((int)(H_crd*10000.0f)), ((int)(D_crd*10000.0f)));
				
				uint32 width  = subvols_H[j].end-subvols_H[j].start;
				uint32 height = subvols_V[i].end-subvols_V[i].start;

				slice = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
				uchar* slice_data_row_ptr;
				int slice_step = slice->widthStep/sizeof(uchar);
				for(uint32 y=0; y<height; y++)
				{
					slice_data_row_ptr = ((uchar*)(slice->imageData)) + y*slice_step;
					for(uint32 x=0; x<width; x++)
						slice_data_row_ptr[x]=(uchar)(subvol[y*width + x]*255);
				}

				cvSaveImage(dir_name,slice);
				cvReleaseImage(&slice);
				delete[] subvol;	
			}
		}
		releaseStacks(k,k);
	}
	delete[] subvols_V;
	delete[] subvols_H;	
}

//save given subvolume as a stack of 8-bit grayscale images in a directory created in the default path
void StackedVolume::saveSubVolume(REAL_T* subvol, int V0, int V1, int H0, int H1, int D0, int D1, int V_idx, int H_idx, int D_idx)
{
	int height = V1-V0;
	int width  = H1-H0;
	int depth  = D1-D0;
	char tmp[1000];
	sprintf(tmp,"%02d_%02d_%02d",V_idx,H_idx,D_idx);
	make_dir(tmp);
	for(int z=0; z<depth; z++)
	{
		sprintf(tmp,"%02d_%02d_%02d/%02d_%02d_%04d.tif",V_idx,H_idx,D_idx,V_idx,H_idx,D0+z);

		if(!fileExists(tmp))
		{
			IplImage *slice = cvCreateImage(cvSize(H1-H0,V1-V0),IPL_DEPTH_8U,1);
			uchar* slice_data = (uchar*)slice->imageData;
			int slice_step = slice->widthStep;
			for(int x=0; x<width; x++)
				for(int y=0; y<height; y++)
					slice_data[y*slice_step+x]=(uchar)(subvol[z*width*height + y*width + x]*255);		
			
			cvSaveImage(tmp,slice);
			cvReleaseImage(&slice);
		}
	}
}

//returns true if file exists at the given filepath
bool StackedVolume::fileExists(const char *filepath)
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
		sprintf(msg,"in fileExists(filepath=%s): Unable to open directory \"%s\"", filepath, dir_path.c_str());
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

//releases allocated memory of stacks
void StackedVolume::releaseStacks(int first_file, int last_file)
{
	first_file = (first_file == -1 ? 0		: first_file);
	last_file  = (last_file  == -1 ? DIM_D	: last_file);
	for(int row_index=0; row_index<N_ROWS; row_index++)
		for(int col_index=0; col_index<N_COLS; col_index++)
			STACKS[row_index][col_index]->releaseStack(first_file,last_file);
}

//saves the Maximum Intensity Projections (MIP) of the selected subvolume along the selected direction into the given paths
void StackedVolume::saveMIPs(bool direction_V, bool direction_H, bool direction_D, char* MIP_V_path, char* MIP_H_path, char* MIP_D_path,
							 uint32 V0, uint32 V1, uint32 H0, uint32 H1, uint32 D0, uint32 D1) throw (MyException)
{
	//LOCAL VARIABLES
	char err_msg[2000];
	REAL_T *MIP_V, *MIP_H, *MIP_D, *slice;
	IplImage *MIP_V_img, *MIP_H_img, *MIP_D_img;
	uint32 dim_v, dim_h, dim_d;
	uint32 i, j, k;
	uchar* img_data_row_ptr;
	int img_step;

	//checking and adjusting default variables
	//V0 = (V0 < 0 ? 0 : V0); // uint32: cannot be negative
	//H0 = (H0 < 0 ? 0 : H0);
	//D0 = (D0 < 0 ? 0 : D0);
	V1 = ((V1 == 0 || V1 > DIM_V) ? DIM_V : V1);
	H1 = ((H1 == 0 || H1 > DIM_H) ? DIM_H : H1);
	D1 = ((D1 == 0 || D1 > DIM_D) ? DIM_D : D1);
	if(V0 >= V1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. V0(%d) >= V1(%d)!", V0, V1);
		throw MyException(err_msg);
	}
	if(H0 >= H1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. H0(%d) >= H1(%d)!", H0, H1);
		throw MyException(err_msg);
	}
	if(D0 >= D1)
	{
		sprintf(err_msg, "in StackedVolume::saveVolume(...): invalid interval selected. D0(%d) >= D1(%d)!", D0, D1);
		throw MyException(err_msg);
	}
	if(MIP_V_path == NULL && direction_V)
	{
		MIP_V_path = new char[strlen(root_dir)+11];
		sprintf(MIP_V_path, "%s/MIP_V.png", root_dir);
	}	
	if(MIP_H_path == NULL && direction_H)
	{
		MIP_H_path = new char[strlen(root_dir)+11];
		sprintf(MIP_H_path, "%s/MIP_H.png", root_dir);
	}	
	if(MIP_D_path == NULL && direction_D)
	{
		MIP_D_path = new char[strlen(root_dir)+11];
		sprintf(MIP_D_path, "%s/MIP_D.png", root_dir);
	}

	//initializations and memory allocations
	dim_v = V1 - V0;
	dim_h = H1 - H0;
	dim_d = D1 - D0;
	MIP_V = MIP_H = MIP_D = NULL;
	MIP_V_img = MIP_H_img = MIP_D_img = NULL;
	if(direction_V)
	{
		MIP_V = new REAL_T[dim_h * dim_d];
		for(i=0; i<dim_h * dim_d; i++)
			MIP_V[i] = 0.0F;
		MIP_V_img = cvCreateImage(cvSize(dim_h, dim_d), IPL_DEPTH_8U, 1);
	}
	if(direction_H)
	{
		MIP_H = new REAL_T[dim_d * dim_v];
		for(i=0; i<dim_d * dim_v; i++)
			MIP_H[i] = 0.0F;
		MIP_H_img = cvCreateImage(cvSize(dim_d, dim_v), IPL_DEPTH_8U, 1);
	}
	if(direction_D)
	{
		MIP_D = new REAL_T[dim_h * dim_v];
		for(i=0; i<dim_h * dim_v; i++)
			MIP_D[i] = 0.0F;
		MIP_D_img = cvCreateImage(cvSize(dim_h, dim_v), IPL_DEPTH_8U, 1);
	}

	//progress bar initialization
	char progressBarBuff[2000];
	sprintf(progressBarBuff, "Extraction of MIPs from V[%d - %d], H[%d - %d], D[%d - %d]", V0, V1, H0, H1, D0, D1);
        ProgressBar::getInstance()->start(progressBarBuff);
        ProgressBar::getInstance()->update(0,"Initializing...");
        ProgressBar::getInstance()->show();

	//MIPs computation
	for(k = 0; k < dim_d; k++)
	{
		sprintf(progressBarBuff, "Processing slice %d of %d", (k+1), dim_d);
                ProgressBar::getInstance()->update((((REAL_T)100/dim_d)*(k+1)), progressBarBuff);
                ProgressBar::getInstance()->show();

		slice = loadSubvolume(V0, V1, H0, H1, k+D0, k+D0+1, NULL, true);
		for(i = 0; i < dim_v; i++)
			for(j = 0; j < dim_h; j++)
			{
				if(MIP_V)
					MIP_V[k*dim_h+j] = MAX(MIP_V[k*dim_h+j], slice[i*dim_h+j]);
				if(MIP_H)
					MIP_H[i*dim_d+k] = MAX(MIP_H[i*dim_d+k], slice[i*dim_h+j]);
				if(MIP_D)
					MIP_D[i*dim_h+j] = MAX(MIP_D[i*dim_h+j], slice[i*dim_h+j]);
			}
		delete[] slice;
	}

	//converting array to <IplImage> and saving
	if(MIP_V)
	{
		img_step = MIP_V_img->widthStep/sizeof(uchar);
		for(i = 0; i < (uint32)MIP_V_img->height; i++)
		{
			img_data_row_ptr = ((uchar*)(MIP_V_img->imageData)) + i*img_step;
			for(j = 0; j < (uint32)MIP_V_img->width; j++)
				img_data_row_ptr[j] = (uchar)(MIP_V[i*MIP_V_img->width+j]*255);
		}
		cvSaveImage(MIP_V_path, MIP_V_img);
	}	
	if(MIP_H)
	{
		img_step = MIP_H_img->widthStep/sizeof(uchar);
		for(i = 0; i < (uint32)MIP_H_img->height; i++)
		{
			img_data_row_ptr = ((uchar*)(MIP_H_img->imageData)) + i*img_step;
			for(j = 0; j < (uint32)MIP_H_img->width; j++)
				img_data_row_ptr[j] = (uchar)(MIP_H[i*MIP_H_img->width+j]*255);
		}
		cvSaveImage(MIP_H_path, MIP_H_img);
	}	
	if(MIP_D)
	{
		img_step = MIP_D_img->widthStep/sizeof(uchar);
		for(i = 0; i < (uint32)MIP_D_img->height; i++)
		{
			img_data_row_ptr = ((uchar*)(MIP_D_img->imageData)) + i*img_step;
			for(j = 0; j < (uint32)MIP_D_img->width; j++)
				img_data_row_ptr[j] = (uchar)(MIP_D[i*MIP_D_img->width+j]*255);
		}
		cvSaveImage(MIP_D_path, MIP_D_img);
	}


	//deallocations
	if(MIP_V)
		delete[] MIP_V;
	if(MIP_H)
		delete[] MIP_H;
	if(MIP_D)
		delete[] MIP_D;
	if(MIP_V_img)
		cvReleaseImage(&MIP_V_img);
	if(MIP_H_img)
		cvReleaseImage(&MIP_H_img);
	if(MIP_D_img)
		cvReleaseImage(&MIP_D_img);
}

const char* axis_to_str(axis ax)
{
	if(ax == 1)
		return "Vertical";
	else if(ax == -1)
		return "Inverse Vertical";
	else if(ax == 2)
		return "Horizontal";
	else if(ax == -2)
		return "Inverse Horizontal";
	else if(ax == 3)
		return "Depth";
	else if(ax == -3)
		return "Inverse Depth";
	else return "<unknown>";
}
