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
#include "TiledMCVolume.h"
#include "Block.h"
#include "RawFmtMngr.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
//#include <cxcore.h>
//#include <cv.h>
//#include <highgui.h>
#include <list>
#include <fstream>
#include "ProgressBar.h"

using namespace std;
using namespace iim;


TiledMCVolume::TiledMCVolume(const char* _root_dir)  throw (IOException)
: VirtualVolume(_root_dir) // iannello ADDED
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir=%s", _root_dir).c_str(), __iim__current__function__);

	// iannello this->root_dir = new char[strlen(_root_dir)+1];
	// iannello strcpy(this->root_dir,_root_dir);

	// iannello VXL_V = VXL_H = VXL_D = ORG_V = ORG_H = ORG_D = 0;
	DIM_V = DIM_H = DIM_D = 0;
	N_ROWS = N_COLS = 0;
	reference_system.first = reference_system.second = reference_system.third = axis_invalid;
	VXL_1 = VXL_2 = VXL_3 = 0;
	vol_ch = (TiledVolume **)0;
	active = (uint32 *)0;
	n_active = 0;


	//without any configuration parameter, volume import must be done from the metadata file stored in the root directory, if it exists
	char mdata_filepath[STATIC_STRINGS_SIZE];
    sprintf(mdata_filepath, "%s/%s", root_dir, MC_MDATA_BIN_FILE_NAME.c_str());
    if(iim::isFile(mdata_filepath))
	{
		load(mdata_filepath);
		initChannels();
	}
	else
	{
		char errMsg[STATIC_STRINGS_SIZE];
		sprintf(errMsg, "in TiledMCVolume::TiledMCVolume(...): unable to find metadata file at %s", mdata_filepath);
        throw IOException(errMsg);
	}
}

TiledMCVolume::TiledMCVolume(const char* _root_dir, ref_sys _reference_system, float _VXL_1, float _VXL_2, float _VXL_3, bool overwrite_mdata, bool save_mdata)  throw (IOException)
: VirtualVolume(_root_dir) // iannello ADDED
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir=%s, ref_sys reference_system={%d,%d,%d}, VXL_1=%.4f, VXL_2=%.4f, VXL_3=%.4f",
                                        _root_dir, _reference_system.first, _reference_system.second, _reference_system.third, _VXL_1, _VXL_2, _VXL_3).c_str(), __iim__current__function__);

	// iannello this->root_dir = new char[strlen(_root_dir)+1];
	// iannello strcpy(this->root_dir,_root_dir);

	// iannello VXL_V = VXL_H = VXL_D = ORG_V = ORG_H = ORG_D = 0;
	DIM_V = DIM_H = DIM_D = 0;
	N_ROWS = N_COLS = 0;
	reference_system.first = reference_system.second = reference_system.third = axis_invalid;
	VXL_1 = VXL_2 = VXL_3 = 0;
	vol_ch = (TiledVolume **)0;
	active = (uint32 *)0;
	n_active = 0;

	//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
	char mdata_filepath[STATIC_STRINGS_SIZE];
    sprintf(mdata_filepath, "%s/%s", root_dir, MC_MDATA_BIN_FILE_NAME.c_str());
    if(iim::isFile(mdata_filepath) && !overwrite_mdata)
	{
		load(mdata_filepath);
		initChannels();
	}
	else
	{
        if(_reference_system.first == axis_invalid ||  _reference_system.second == axis_invalid ||
          _reference_system.third == axis_invalid || _VXL_1 == 0 || _VXL_2 == 0 || _VXL_3 == 0)
            throw IOException("in TiledMCVolume::TiledMCVolume(...): invalid importing parameters");

        reference_system.first  = _reference_system.first;
        reference_system.second = _reference_system.second;
        reference_system.third  = _reference_system.third;
        VXL_1 = _VXL_1;
        VXL_2 = _VXL_2;
        VXL_3 = _VXL_3;
        init();
		//initChannels();
        if(save_mdata)
            save(mdata_filepath);
	}
}

TiledMCVolume::~TiledMCVolume(void)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	if ( CHDIRNAMES )
		delete[] CHDIRNAMES;
	if ( vol_ch ) {
		for ( int c=0; c<DIM_C; c++ )
			delete vol_ch[c];
		delete[] vol_ch;
	}
	if ( active )
		delete[] active;
}


void TiledMCVolume::save(char* metadata_filepath) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("metadata_filepath=%s", metadata_filepath).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	uint16 str_size;
	FILE *file;

    file = fopen(metadata_filepath, "wb");

    // --- Alessandro 2013-04-23: added exception when file can't be opened in write mode
    if(!file)
    {
        char errMsg[STATIC_STRINGS_SIZE];
        sprintf(errMsg, "in TiledMCVolume::save(): cannot write metadata binary file at \"%s\".\n\nPlease check write permissions on this storage.", metadata_filepath);
        throw IOException(errMsg);
    }

	// WARNING: check that channel subdirectories have consistent mdata files

    float mdata_version = static_cast<float>(iim::MDATA_BIN_FILE_VERSION);
    fwrite(&mdata_version, sizeof(float), 1, file); // --- Alessandro 2012-12-31: added field for metadata file version
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

	fwrite(&DIM_C, sizeof(int), 1, file);

	// save channel directories names
	int n_digits = 1;
	int _channels = DIM_C / 10;	
	while ( _channels ) {
		n_digits++;
		_channels /= 10;
	}
	for ( int c=0; c<DIM_C; c++ ) {
		str_size = (uint16)(strlen(CHDIRNAMES[c]) + 1);
		fwrite(&str_size, sizeof(uint16), 1, file);
		fwrite(CHDIRNAMES[c], str_size, 1, file);
	}

	fwrite(&BYTESxCHAN, sizeof(int), 1, file);

	fclose(file);
}

void TiledMCVolume::load(char* metadata_filepath) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("metadata_filepath=%s", metadata_filepath).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	FILE *file;
	uint16 str_size;
	//int i,j;
	size_t fread_return_val;

	file = fopen(metadata_filepath, "rb");

    // --- Alessandro 2013-04-23: added exception when file can't be opened in read mode
    if(!file)
    {
        char errMsg[STATIC_STRINGS_SIZE];
        sprintf(errMsg, "in TiledMCVolume::load(): cannot read metadata binary file at \"%s\".\n\nPlease check read permissions on this storage.", metadata_filepath);
        throw IOException(errMsg);
    }

    // --- Alessandro 2012-12-31: added field for metadata file version
    float mdata_version_read = 0;
    float mdata_version = static_cast<float>(iim::MDATA_BIN_FILE_VERSION);
    fread_return_val = fread(&mdata_version_read, sizeof(float), 1, file);
    if(fread_return_val != 1 || mdata_version_read != mdata_version)
    {
        // --- Alessandro 2013-01-06: instead of throwing an exception, it is better to mantain compatibility
//            char errMsg[STATIC_STRINGS_SIZE];
//            sprintf(errMsg, "in TiledMCVolume::unBinarizeFrom(...): metadata file version (%.2f) is different from the supported one (%.2f). "
//                    "Please re-import the current volume.", mdata_version_read, mdata_version);
//            throw MyException(errMsg);

        fclose(file);
        file = fopen(metadata_filepath, "rb");
        uint16 str_size;
        fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
        if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
        }
        char stored_root_dir[STATIC_STRINGS_SIZE];
        fread_return_val = fread(stored_root_dir, str_size, 1, file);
        if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
        }
    }

    fread_return_val = fread(&reference_system.first, sizeof(axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

    fread_return_val = fread(&reference_system.second, sizeof(axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

    fread_return_val = fread(&reference_system.third, sizeof(axis), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

    fread_return_val = fread(&VXL_1, sizeof(float), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

    fread_return_val = fread(&VXL_2, sizeof(float), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
         throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

    fread_return_val = fread(&VXL_3, sizeof(float), 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&VXL_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&VXL_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&VXL_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&ORG_V, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&ORG_H, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&ORG_D, sizeof(float), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&DIM_V, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&DIM_H, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&DIM_D, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&N_ROWS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&N_COLS, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fread_return_val = fread(&DIM_C, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	n_active = DIM_C;
	active = new uint32[n_active];
	for ( int c=0; c<DIM_C; c++ )
		active[c] = c; // all channels are assumed active

	char channel_c_path[STATIC_STRINGS_SIZE];
	CHDIRNAMES = new char*[DIM_C];
	vol_ch     = new TiledVolume *[DIM_C];
	for ( int c = 0; c < DIM_C; c++)
	{
		fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
        }

		CHDIRNAMES[c] = new char[str_size];
		fread_return_val = fread(CHDIRNAMES[c], str_size, 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
        }

		sprintf(channel_c_path,"%s/%s",root_dir,CHDIRNAMES[c]);
		vol_ch[c] = new TiledVolume(channel_c_path);
	}

	fread_return_val = fread(&BYTESxCHAN, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in TiledMCVolume::unBinarizeFrom(...): error while reading binary metadata file");
    }

	fclose(file);
}

void TiledMCVolume::init()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	//LOCAL VARIABLES
	string tmp_path;				//string that contains temp paths during computation
    string tmp;						//string that contains temp data during computation
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	dirent *entry_lev1;				//pointer to DIRENT, the data structure that represents a DIRECTORY ENTRY inside a directory (level 1)
	int c;
    list<string> entries_lev1;                      //list of entries of first level of hierarchy
    list<string>::iterator entry_i;                 //iterator for list 'entries_lev1'
	//char channel_c_path[STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(root_dir)))
	{
        char msg[STATIC_STRINGS_SIZE];
        sprintf(msg,"in TiledMCVolume::init(...): Unable to open directory \"%s\"", root_dir);
        throw IOException(msg);
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
	DIM_C = (uint16) entries_lev1.size();
	n_active = DIM_C;
	active = new uint32[n_active];
	for ( c=0; c<DIM_C; c++ )
		active[c] = c; // all channels are assumed active

	//for each entry of first level, creating channel volume
	vol_ch = new TiledVolume *[DIM_C];
	for(entry_i = entries_lev1.begin(), c=0; entry_i!= entries_lev1.end(); entry_i++, c++)
	{
		//building absolute path of first level entry to be used for "opendir(...)"
		tmp_path=root_dir;
		tmp_path.append("/");
		tmp_path.append(*entry_i);

		vol_ch[c] = new TiledVolume(tmp_path.c_str());

		if (N_ROWS == 0)
			N_ROWS = vol_ch[c]->getN_ROWS();
		else if (vol_ch[c]->getN_ROWS() != N_ROWS)
            throw IOException("in TiledMCVolume::init(...): Number of tiles is not the same for all channels!");

		if (N_COLS == 0)
			N_COLS = vol_ch[c]->getN_COLS();
		else if (vol_ch[c]->getN_COLS() != N_COLS)
            throw IOException("in TiledMCVolume::init(...): Number of tiles is not the same for all channels!");
	}

	CHDIRNAMES = new char*[DIM_C];
	for(c=0; c<DIM_C; c++)
	{
		tmp = entries_lev1.front();
		CHDIRNAMES[c] = new char[tmp.size()+1];
		strcpy(CHDIRNAMES[c], tmp.c_str());
		entries_lev1.pop_front();
	}

	entries_lev1.clear();

	//intermediate check
	if(N_ROWS == 0 || N_COLS == 0)
        throw IOException("in TiledMCVolume::init(...): Unable to initialize N_ROWS, N_COLS");

	/******************* 2) SETTING THE REFERENCE SYSTEM ********************
	The entire application uses a vertical-horizontal reference system, so
	it is necessary to fit the original reference system into the new one.     
	*************************************************************************/

	//adjusting possible sign mismatch betwwen reference system and VXL
	//in these cases VXL is adjusted to match with reference system
    if(sgn(reference_system.first) != sgn(VXL_1))
            VXL_1*=-1.0f;
    if(sgn(reference_system.second) != sgn(VXL_2))
            VXL_2*=-1.0f;
    if(sgn(reference_system.third) != sgn(VXL_3))
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

        //int computed_ORG_1, computed_ORG_2, computed_ORG_3;
        //extractCoordinates(BLOCKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);

		ORG_V = vol_ch[0]->getORG_V(); // WARNING: also in this case coordinates have to be the same of channel volumes
        ORG_H = vol_ch[0]->getORG_H();
        ORG_D = vol_ch[0]->getORG_D();
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

        //int computed_ORG_1, computed_ORG_2, computed_ORG_3;
        //extractCoordinates(BLOCKS[0][0], 0, &computed_ORG_1, &computed_ORG_2, &computed_ORG_3);

		ORG_V = vol_ch[0]->getORG_V(); 
        ORG_H = vol_ch[0]->getORG_H();
        ORG_D = vol_ch[0]->getORG_D();
        VXL_V = VXL_1;
        VXL_H = VXL_2;
        VXL_D = VXL_3;
	}
	//unsupported reference system
	else
	{
        char msg[STATIC_STRINGS_SIZE];
        sprintf(msg, "in TiledMCVolume::init(...): the reference system {%d,%d,%d} is not supported.",
                reference_system.first, reference_system.second, reference_system.third);
        throw IOException(msg);
	}

	// WARNINIG: following code should not be executed
	//some little adjustments of the origin
	//if(VXL_V < 0)
	//	ORG_V -= (BLOCKS[0][0]->getHEIGHT()-1)* VXL_V/1000.0f;

	//if(VXL_H < 0)
	//	ORG_H -= (BLOCKS[0][0]->getWIDTH() -1)* VXL_H/1000.0f;

	/******************** 3) COMPUTING VOLUME DIMENSIONS ********************  
	*************************************************************************/

	DIM_V = vol_ch[0]->getDIM_V();
	DIM_H = vol_ch[0]->getDIM_H();
	DIM_D = vol_ch[0]->getDIM_D();

	//for(int row=0; row < N_ROWS; row++) {
 //       for(int col=0; col < N_COLS; col++)
 //       {
 //           if(row==0)
	//			DIM_H+=BLOCKS[row][col]->getWIDTH();
 //           if(col==0)
	//			DIM_V+=BLOCKS[row][col]->getHEIGHT();
 //           DIM_D = BLOCKS[row][col]->getDEPTH() > DIM_D ? BLOCKS[row][col]->getDEPTH() : DIM_D;
 //       }
	//}

	/**************** 4) COMPUTING STACKS ABSOLUTE POSITIONS ****************  
	*************************************************************************/
	//for(int row=0; row < N_ROWS; row++) {
 //       for(int col=0; col < N_COLS; col++)
 //       {
 //           if(row)
	//			BLOCKS[row][col]->setABS_V(BLOCKS[row-1][col]->getABS_V()+BLOCKS[row-1][col]->getHEIGHT());
 //           else
	//			BLOCKS[row][col]->setABS_V(0);

 //           if(col)
	//			BLOCKS[row][col]->setABS_H(BLOCKS[row][col-1]->getABS_H()+BLOCKS[row][col-1]->getWIDTH());
 //           else
	//			BLOCKS[row][col]->setABS_H(0);
 //       }
	//}
}

void TiledMCVolume::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    BYTESxCHAN = vol_ch[0]->getBYTESxCHAN();
}

//PRINT method
void TiledMCVolume::print()
{
	printf("*** Begin printing StakedVolume object...\n\n");
	printf("\tDirectory:\t%s\n", root_dir);
	printf("\tDimensions:\t%d(V) x %d(H) x %d(D)\n", DIM_V, DIM_H, DIM_D);
	printf("\tVoxels:\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", VXL_V, VXL_H, VXL_D);
	printf("\tOrigin:\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", ORG_V, ORG_H, ORG_D);
	printf("\tStacks matrix:\t%d(V) x %d(H)\n", N_ROWS, N_COLS);
	//printf("\t |\n");
	//for(int row=0; row<N_ROWS; row++)
	//	for(int col=0; col<N_COLS; col++)
	//		BLOCKS[row][col]->print();
	printf("\n*** END printing StakedVolume object...\n\n");
}

//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
void TiledMCVolume::rotate(int theta)
{
    /**/iim::debug(iim::LEV3, strprintf("theta=%d", theta).c_str(), __iim__current__function__);

	char msg[STATIC_STRINGS_SIZE];
	sprintf(msg,"in TiledMCVolume::rotate: not implemented yet");
    throw IOException(msg);

//	//PRECONDITIONS:
//	//	1) current TiledMCVolume object has been initialized (init() method has been called)
//	//	2) accepted values for 'theta' are 0,90,180,270
//
//	//POSTCONDITIONS:
//	//  1) a new 2D-array of Stack* objects is created considering a rotation of 'theta' angle of current TiledMCVolume object
//
//	Stack*** new_STACK_2D_ARRAY = NULL;
//	int new_N_ROWS = 0, new_N_COLS = 0;
//
	switch(theta)
	{
		case(0): break;

		case(90):
//		{
//			new_N_COLS = N_ROWS;
//			new_N_ROWS = N_COLS;
//
//			//allocating new_STACK_2D_ARRAY
//			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
//			for(int i=0; i<new_N_ROWS; i++)
//				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];
//
//			//populating new_STACK_2D_ARRAY
//			for(int i=0; i<new_N_ROWS; i++)
//				for(int j=0; j<new_N_COLS; j++){
//					new_STACK_2D_ARRAY[i][j] = STACKS[N_ROWS-1-j][i];
//					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
//					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
//				}
			break;
//		}
		case(180):
//		{
//			new_N_COLS=N_COLS;
//			new_N_ROWS=N_ROWS;
//
//			//allocating new_STACK_2D_ARRAY
//			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
//			for(int i=0; i<new_N_ROWS; i++)
//				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];
//
//			//populating new_STACK_2D_ARRAY
//			for(int i=0; i<new_N_ROWS; i++)
//				for(int j=0; j<new_N_COLS; j++){
//					new_STACK_2D_ARRAY[i][j] = STACKS[N_ROWS-1-i][N_COLS-1-j];
//					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
//					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
//				}
			break;
//		}
		case(270):
//		{
//			new_N_COLS=N_ROWS;
//			new_N_ROWS=N_COLS;
//
//			//allocating new_STACK_2D_ARRAY
//			new_STACK_2D_ARRAY = new Stack**[new_N_ROWS];
//			for(int i=0; i<new_N_ROWS; i++)
//				new_STACK_2D_ARRAY[i] = new Stack*[new_N_COLS];
//
//			//populating new_STACK_2D_ARRAY
//			for(int i=0; i<new_N_ROWS; i++)
//				for(int j=0; j<new_N_COLS; j++){
//					new_STACK_2D_ARRAY[i][j] = STACKS[j][N_COLS-1-i];
//					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
//					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
//				}
			break;
//		}
	}
//
//
//	//deallocating current STACK_2DARRAY object
//	for(int row=0; row<N_ROWS; row++)
//		delete[] STACKS[row];
//	delete[] STACKS;
//
//	STACKS = new_STACK_2D_ARRAY;
//	N_COLS = new_N_COLS;
//	N_ROWS = new_N_ROWS;
}

//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
void TiledMCVolume::mirror(axis mrr_axis)
{
    /**/iim::debug(iim::LEV3, strprintf("mrr_axis=%d", mrr_axis).c_str(), __iim__current__function__);

//	//PRECONDITIONS:
//	//	1) current TiledMCVolume object has been initialized (init() method has been called)
//	//	2) accepted values for 'mrr_axis' are 1(V axis), 2(H axis) or 3(D axis)
//
//	//POSTCONDITIONS:
//	//  1) a new 2D-array of Stack* objects is created considering a mirrorization along 'axis' of current TiledMCVolume object
//
//	if(mrr_axis!= 1 && mrr_axis != 2)
//	{
//		char msg[1000];
//		sprintf(msg,"in TiledMCVolume::mirror(axis mrr_axis=%d): unsupported axis mirroring", mrr_axis);
//		throw MyException(msg);
//	}
//
//	Stack*** new_STACK_2D_ARRAY;
//
	switch(mrr_axis)
	{
		case(1):
//		{
//			//allocating new_STACK_2D_ARRAY
//			new_STACK_2D_ARRAY = new Stack**[N_ROWS];
//			for(int i=0; i<N_ROWS; i++)
//				new_STACK_2D_ARRAY[i] = new Stack*[N_COLS];
//
//			//populating new_STACK_2D_ARRAY
//			for(int i=0; i<N_ROWS; i++)
//				for(int j=0; j<N_COLS; j++){
//					new_STACK_2D_ARRAY[i][j]=STACKS[N_ROWS-1-i][j];
//					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
//					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
//				}
			break;
//		}		
		case(2):
//		{
//			//allocating new_STACK_2D_ARRAY
//			new_STACK_2D_ARRAY = new Stack**[N_ROWS];
//			for(int i=0; i<N_ROWS; i++)
//				new_STACK_2D_ARRAY[i] = new Stack*[N_COLS];
//
//			//populating new_STACK_2D_ARRAY
//			for(int i=0; i<N_ROWS; i++)
//				for(int j=0; j<N_COLS; j++){
//					new_STACK_2D_ARRAY[i][j] = STACKS[i][N_COLS-1-j];
//					new_STACK_2D_ARRAY[i][j]->setROW_INDEX(i);
//					new_STACK_2D_ARRAY[i][j]->setCOL_INDEX(j);
//				}
			break;
//		}
		default: break;
	}
//
//	//deallocating current STACK_2DARRAY object
//	for(int row=0; row<N_ROWS; row++)
//		delete[] STACKS[row];
//	delete[] STACKS;
//
//	STACKS = new_STACK_2D_ARRAY;
}

//loads given subvolume in a 1-D array of float
real32* TiledMCVolume::loadSubvolume(int V0,int V1, int H0, int H1, int D0, int D1, list<Block*> *involved_blocks, bool release_blocks) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, %s", V0, V1, H0, H1, D0, D1, (involved_blocks? ", involved_stacks" : "")).c_str(), __iim__current__function__);

	char msg[STATIC_STRINGS_SIZE];
	sprintf(msg,"in TiledMCVolume::loadSubvolume: not completed yet");
    throw IOException(msg);

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
    real32 *subvol = new real32[sbv_height * sbv_width * sbv_depth];

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;

	return subvol;
}

//loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
uint8* TiledMCVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

    //checking for non implemented features
	//if( this->BYTESxCHAN != 1 ) {
	//	char err_msg[STATIC_STRINGS_SIZE];
	//	sprintf(err_msg,"TiledMCVolume::loadSubvolume_to_UINT8: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
	//	throw MyException(err_msg);
	//}

    //if ( (ret_type == iim::DEF_IMG_DEPTH) && ((8 * this->BYTESxCHAN) != iim::DEF_IMG_DEPTH)  ) {
		// does not support depth conversion: 
		// return type is 8 bits, but native depth is not 8 bits
    if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"RawVolume::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
        throw IOException(err_msg);
	}

	//char *err_rawfmt;

    //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED

    //checking that the interval is valid
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw IOException("in TiledMCVolume::loadSubvolume_to_UINT8: invalid subvolume intervals");

    //computing dimensions
    sint64 sbv_height = V1 - V0;
    sint64 sbv_width  = H1 - H0;
    sint64 sbv_depth  = D1 - D0;

	sint64 sbv_ch_dim = sbv_height * sbv_width * sbv_depth;

	int dummy_ch;
    uint8 *subvol   = new uint8[n_active*sbv_ch_dim];
	uint8 *subvol_ch;

	for ( int c=0; c<n_active; c++ ) {
		subvol_ch = vol_ch[active[c]]->loadSubvolume_to_UINT8(V0,V1,H0,H1,D0,D1,&dummy_ch,ret_type);
		memcpy(subvol + c*sbv_ch_dim, subvol_ch, sbv_ch_dim*sizeof(uint8));
		delete[] subvol_ch;
	}

	//returning outputs
    if(channels)
        *channels = (int)n_active;

    return subvol;
}

// moved to VirtualVolume.cpp by Alessandro on 2014-02-20
//void TiledMCVolume::setActiveChannels ( uint32 *_active, int _n_active )
//{
//    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

//	if ( active )
//		delete[] active;
//	active   = _active;
//	n_active = _n_active;
//}


// OPERATIONS FOR STREAMED SUBVOLUME LOAD 

struct stream_MC_descr_t {
	sint64 sbv_ch_dim;
	void **descr_list;
};

void *TiledMCVolume::streamedLoadSubvolume_open ( int steps, uint8 *buf, int V0,int V1, int H0, int H1, int D0, int D1 )
{
    /**/iim::debug(iim::LEV3, strprintf("steps=%d, V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", steps, V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);

    //checking for non implemented features
	if( this->BYTESxCHAN != 1 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"TiledMCVolume::streamedLoadSubvolume_open: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
        throw IOException(err_msg);
	}

    //checks
	if ( V0>=V1 || H0>=H1 || D0>=D1 || V0<0 || H0<0 || D0<0 || V1>(int)DIM_V || H1>(int)DIM_H || D1>(int)DIM_D ) {
		char msg[1000];
		sprintf(msg,"in TiledMCVolume::streamedLoadSubvolume_open: invalid sub-block vertices");
        throw IOException(msg);
	}

	sint64 sbv_height = V1 - V0;
    sint64 sbv_width  = H1 - H0;
    sint64 sbv_depth  = D1 - D0;

	stream_MC_descr_t *stream_descr = new stream_MC_descr_t;
	stream_descr->sbv_ch_dim = sbv_height * sbv_width * sbv_depth;
	stream_descr->descr_list = new void *[n_active];

	for ( int c=0; c<n_active; c++ ) {
		stream_descr->descr_list[c] = 
			vol_ch[active[c]]->streamedLoadSubvolume_open(steps,(buf + c*(stream_descr->sbv_ch_dim)),V0,V1,H0,H1,D0,D1);
	}

	return stream_descr;
}

uint8 *TiledMCVolume::streamedLoadSubvolume_dostep ( void *stream_descr, unsigned char *buffer2 )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	uint8 *dummy;
	uint8 *buf = vol_ch[active[0]]->streamedLoadSubvolume_dostep(
					((stream_MC_descr_t *)stream_descr)->descr_list[0],
					buffer2
				 );
	for ( int c=1; c<n_active; c++ ) {
		dummy = vol_ch[active[c]]->streamedLoadSubvolume_dostep(
					((stream_MC_descr_t *)stream_descr)->descr_list[c],
					(buffer2) ? (buffer2 + c*(((stream_MC_descr_t *)stream_descr)->sbv_ch_dim)) : 0
				); 
	}

	return buf;
}

void TiledMCVolume::streamedLoadSubvolume_cpydata ( void *stream_descr, unsigned char *buffer, unsigned char *buffer2 )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	for ( int c=0; c<n_active; c++ ) {
		vol_ch[active[c]]->streamedLoadSubvolume_cpydata(
					((stream_MC_descr_t *)stream_descr)->descr_list[c],
					(buffer  + c*(((stream_MC_descr_t *)stream_descr)->sbv_ch_dim)),
					(buffer2) ? (buffer2 + c*(((stream_MC_descr_t *)stream_descr)->sbv_ch_dim)) : 0
				); 
	}
}

uint8 *TiledMCVolume::streamedLoadSubvolume_close ( void *stream_descr, bool return_buffer )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	uint8 *dummy;
	uint8 *temp = vol_ch[0]->streamedLoadSubvolume_close(((stream_MC_descr_t *)stream_descr)->descr_list[0],true);

	for ( int c=1; c<n_active; c++ ) {
		dummy = vol_ch[0]->streamedLoadSubvolume_close(((stream_MC_descr_t *)stream_descr)->descr_list[c],true);
	}

	delete[] ((stream_MC_descr_t *)stream_descr)->descr_list;
	delete ((stream_MC_descr_t *)stream_descr);

	if ( return_buffer )
		return temp;
	else {
		delete[] temp;
		return 0;
	}
}
