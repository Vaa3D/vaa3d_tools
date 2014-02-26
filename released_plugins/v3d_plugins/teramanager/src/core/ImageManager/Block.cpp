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

#include "Block.h"
#include "VirtualVolume.h"
#include "RawFmtMngr.h"
#include <cxcore.h>
#include <highgui.h>
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <list>
#include <string>
#include <cv.h>

using namespace std;
using namespace iim;

Block::Block(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, char* _DIR_NAME)
{
    /**/iim::debug(iim::LEV3, strprintf("_ROW_INDEX=%d, _COL_INDEX=%d, _DIR_NAME=%s", _ROW_INDEX, _COL_INDEX, _DIR_NAME).c_str(), __iim__current__function__);

	this->CONTAINER = _CONTAINER;

	this->DIR_NAME = new char[strlen(_DIR_NAME)+1];
	strcpy(this->DIR_NAME, _DIR_NAME);

	this->ROW_INDEX = _ROW_INDEX;
	this->COL_INDEX = _COL_INDEX;

	FILENAMES = NULL;
	HEIGHT = WIDTH = DEPTH = 0;
	N_BLOCKS = 0;
	N_CHANS = 0;
	N_BYTESxCHAN = 0;
	ABS_V = ABS_H = 0;
	BLOCK_SIZE = 0;
	BLOCK_ABS_D = 0;

	init();
}

Block::Block(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("_ROW_INDEX=%d, _COL_INDEX=%d", _ROW_INDEX, _COL_INDEX).c_str(), __iim__current__function__);

	CONTAINER = _CONTAINER;
	ROW_INDEX = _ROW_INDEX;
	COL_INDEX = _COL_INDEX;

	DIR_NAME = NULL;
	FILENAMES = NULL;
	HEIGHT = WIDTH = DEPTH = 0;
	N_BLOCKS = 0;
	N_CHANS = 0;
	N_BYTESxCHAN = 0;
	ABS_V = ABS_H = 0;
	BLOCK_SIZE = 0;
	BLOCK_ABS_D = 0;

	unBinarizeFrom(bin_file);

	//initializing STACKED_IMAGE array
	//this->STACKED_IMAGE = new CvMat*[DEPTH];
	//for(uint32 z=0; z<DEPTH; z++)
	//	this->STACKED_IMAGE[z] = NULL;
}

Block::~Block(void)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	if (BLOCK_SIZE)
		delete[] BLOCK_SIZE;
	if (BLOCK_ABS_D)
		delete[] BLOCK_ABS_D;

	for(uint32 z=0; z<N_BLOCKS; z++)
	{
	//	if(STACKED_IMAGE[z])
	//		cvReleaseMat(&STACKED_IMAGE[z]);
		if(FILENAMES[z])
			delete[] FILENAMES[z];
	}

	//if(STACKED_IMAGE)
	//	delete[] STACKED_IMAGE;
	if(FILENAMES)
		delete[] FILENAMES;
	if(DIR_NAME)
		delete[] DIR_NAME;
}

//binarizing-unbinarizing methods
void Block::binarizeInto(FILE* file)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	uint16 str_size;
	uint32 i;

	fwrite(&HEIGHT, sizeof(uint32), 1, file);
	fwrite(&WIDTH, sizeof(uint32), 1, file);
	fwrite(&DEPTH, sizeof(uint32), 1, file);
	fwrite(&N_BLOCKS, sizeof(uint32), 1, file);
	fwrite(&N_CHANS, sizeof(uint32), 1, file);
	fwrite(&ABS_V, sizeof(int), 1, file);
	fwrite(&ABS_H, sizeof(int), 1, file);
	str_size = (uint16)(strlen(DIR_NAME) + 1);
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(DIR_NAME, str_size, 1, file);
	for(i = 0; i < N_BLOCKS; i++)
	{
		str_size = (uint16)(strlen(FILENAMES[i]) + 1);
        fwrite(&str_size, sizeof(uint16), 1, file);
        fwrite(FILENAMES[i], str_size, 1, file);
		fwrite(BLOCK_SIZE+i, sizeof(uint32), 1, file);
		fwrite(BLOCK_ABS_D+i, sizeof(int), 1, file);
	}
	fwrite(&N_BYTESxCHAN, sizeof(uint32), 1, file);
}

void Block::unBinarizeFrom(FILE* file) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	uint16 str_size;
	uint32 i;
	size_t fread_return_val;

	fread_return_val = fread(&HEIGHT, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (HEIGHT)");
    }
	fread_return_val = fread(&WIDTH, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (WIDTH)");
    }
	fread_return_val = fread(&DEPTH, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (DEPTH)");
    }
	fread_return_val = fread(&N_BLOCKS, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (N_BLOCKS)");
    }
	fread_return_val = fread(&N_CHANS, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (N_CHANS)");
    }
	fread_return_val = fread(&ABS_V, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (ABS_V)");
    }
	fread_return_val = fread(&ABS_H, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (ABS_H)");
    }
	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (str_size)");
    }
	DIR_NAME = new char[str_size];
    fread_return_val = fread(DIR_NAME, str_size, 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in Block::unBinarizeFrom(...): error while reading binary metadata file (DIR_NAME)");
    }

	FILENAMES = new char*[N_BLOCKS];
	BLOCK_SIZE = new uint32[N_BLOCKS];
	BLOCK_ABS_D = new int[N_BLOCKS];
	for(i = 0; i < N_BLOCKS; i++)
	{
		fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException(strprintf("in Block[%d]::unBinarizeFrom(...): error while reading binary metadata file (str_size)", i).c_str());
        }

		FILENAMES[i] = new char[str_size];
		fread_return_val = fread(FILENAMES[i], str_size, 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException(strprintf("in Block[%d]::unBinarizeFrom(...): error while reading binary metadata file (FILENAMES)", i).c_str());
        }
	
		fread_return_val = fread(BLOCK_SIZE+i, sizeof(uint32), 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException(strprintf("in Block[%d]::unBinarizeFrom(...): error while reading binary metadata file (BLOCK_SIZE)", i).c_str());
        }

		fread_return_val = fread(BLOCK_ABS_D+i, sizeof(int), 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException(strprintf("in Block[%d]::unBinarizeFrom(...): error while reading binary metadata file (BLOCK_ABS_D)", i).c_str());
        }
	}
	fread_return_val = fread(&N_BYTESxCHAN, sizeof(uint32), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException(strprintf("in Block[%d]::unBinarizeFrom(...): error while reading binary metadata file (N_BYTESxCHAN)", i).c_str());
    }

}

//Initializes all object's members given DIR_NAME
void Block::init()
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL variables
	string tmp;
	DIR *cur_dir_lev3;
	dirent *entry_lev3;
	list<string> entries_lev3;
	string entry;

	//building filenames_list
	char abs_path[STATIC_STRINGS_SIZE];
	sprintf(abs_path,"%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME);
	cur_dir_lev3 = opendir(abs_path);
	if (!cur_dir_lev3)
	{
		char errMsg[STATIC_STRINGS_SIZE];
		sprintf(errMsg, "in Block::init(): can't open directory \"%s\"", abs_path);
        throw IOException(errMsg);
	}

	//scanning third level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry_lev3=readdir(cur_dir_lev3)))
	{
		tmp = entry_lev3->d_name;
		if(tmp.compare(".") != 0 && tmp.compare("..") != 0 && tmp.find(".") != string::npos)
			entries_lev3.push_back(tmp);
	}
	entries_lev3.sort();
	N_BLOCKS = (int)entries_lev3.size();

    //----- Alessandro added on August 12, 2013
    //----- Bug fixed: exceeding the maximum number of directories opened at the same time
    closedir(cur_dir_lev3);

	//checking if current stack is not empty
	if(N_BLOCKS == 0)
	{
		char msg[1000];
                sprintf(msg,"in Block[%d,%d]::init(): stack in \"%s\" is empty", ROW_INDEX, COL_INDEX, abs_path);
        throw IOException(msg);
	}

	//converting filenames_list (STL list of C-strings) into FILENAMES (1-D array of C-strings)
	FILENAMES = new char*[N_BLOCKS];
	for(uint32 z=0; z<N_BLOCKS; z++)
	{
		entry = entries_lev3.front();
		FILENAMES[z] = new char[entry.size()+1];
		strcpy(FILENAMES[z], entry.c_str());
		entries_lev3.pop_front();
	}
	entries_lev3.clear();

	//initializing STACKED_IMAGE array
	//this->STACKED_IMAGE = new CvMat*[DEPTH];
	//for(uint32 z=0; z<DEPTH; z++)
	//	this->STACKED_IMAGE[z] = NULL;

	// declarations needed by RawFmtMngr routines
	char *err_rawfmt;
	void *dummy;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;

	//extracting HEIGHT, WIDTH and N_CHANS attributes from first slice
	char slice_fullpath[STATIC_STRINGS_SIZE];
	sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME, FILENAMES[0]);

	if ( (err_rawfmt = loadRaw2Metadata(slice_fullpath,sz,datatype,b_swap,dummy,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in Block[%d,%d]::init(): unable to open block \"%s\". Wrong path or format (%s)", 
			ROW_INDEX, COL_INDEX, slice_fullpath,err_rawfmt);
        throw IOException(msg);
	}
	closeRawFile((FILE *)dummy);

	HEIGHT       = (uint32)sz[1];
	WIDTH        = (uint32)sz[0];
	N_CHANS      = (uint32)sz[3];
	N_BYTESxCHAN = (uint32)datatype;

	//extracting DEPTH and other attributes from all blocks
	BLOCK_SIZE = new uint32[N_BLOCKS];
	BLOCK_ABS_D = new int[N_BLOCKS];

	BLOCK_ABS_D[0] = 0;
	DEPTH = BLOCK_SIZE[0] = (uint32)sz[2];
	for ( int ib=1; ib<(int)N_BLOCKS; ib++ ) {
		sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME, FILENAMES[ib]);
		if ( (err_rawfmt = loadRaw2Metadata(slice_fullpath,sz,datatype,b_swap,dummy,header_len)) != 0 ) {
			if ( sz ) delete[] sz;
			char msg[STATIC_STRINGS_SIZE];
			sprintf(msg,"in Block[%d,%d]::init(): unable to open block \"%s\". Wrong path or format (%s)", 
				ROW_INDEX, COL_INDEX, slice_fullpath,err_rawfmt);
            throw IOException(msg);
		}
		closeRawFile((FILE *)dummy);

		BLOCK_SIZE[ib] = (uint32)sz[2];
		BLOCK_ABS_D[ib] = DEPTH;
		DEPTH += BLOCK_SIZE[ib];
	}

	delete[] sz;
}

//PRINT method
void Block::print()
{
	printf("\t |\t[%d,%d]\n", ROW_INDEX, COL_INDEX);
	printf("\t |\tDirectory:\t\t%s\n", DIR_NAME);
	printf("\t |\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", HEIGHT, WIDTH, DEPTH);
	printf("\t |\tNumber of blocks:\t\t%d\n", N_BLOCKS);
	printf("\t |\tNumber of channels:\t\t%d\n", N_CHANS);
	printf("\t |\tNumber of bytes per channel:\t\t%d\n", N_BYTESxCHAN);
	printf("\t |\tAbsolute position:\t%d(V) x %d(H) x %d(D)\n", ABS_V, ABS_H, 0);
	/*printf("\t |\tFilenames:\t\n");
	for(int z=0; z<DEPTH; z++)
		printf("\t |\t\t%s\n",FILENAMES[z]);*/
	printf("\t |\n");
}

//returns a pointer to the intersection rectangle if the given area intersects current stack, otherwise returns NULL
Rect_t* Block::Intersects(const Rect_t& area)
{
	//first determining if intersection occurs
	bool intersects =  ( area.H0    < (int)(ABS_H + WIDTH)	&& 
						 area.H1    >  ABS_H			&& 
						 area.V0    < (int)(ABS_V + HEIGHT)	&& 
						 area.V1    >  ABS_V		); 

	//if intersection occurs, computing intersection area, otherwise returning NULL
	if(intersects)
	{
		Rect_t *intersect_rect = new Rect_t;
		intersect_rect->H0 = MAX(ABS_H, area.H0);
		intersect_rect->V0 = MAX(ABS_V, area.V0);
		intersect_rect->H1 = MIN((int)(ABS_H + WIDTH), area.H1);
		intersect_rect->V1 = MIN((int)(ABS_V + HEIGHT), area.V1);

		return intersect_rect;
	}
	else
		return NULL;
}


Segm_t* Block::Intersects(int D0, int D1) {

	if ( D0 >= BLOCK_ABS_D[N_BLOCKS-1]+BLOCK_SIZE[N_BLOCKS-1] || D1 < 0 )
		// there is no intersection
		return NULL;

	bool found0, found1;
	int i0, i1;

	found0 = false;
	i0     = 0;
	while ( i0<(int)(N_BLOCKS-1) && !found0 )
		if ( D0 < BLOCK_ABS_D[i0+1] )
			found0 = true;
		else
			i0++;
	// !found0 -> i0 = N_BLOCKS-1

	found1 = false;
	i1     = (int)(N_BLOCKS-1);
	while ( i1>0 && !found1 )
		if ( D1 > BLOCK_ABS_D[i1] )
			found1 = true;
		else
			i1--;
	// !found1 -> i1 = 0

	Segm_t *intersect_segm = new Segm_t;
	intersect_segm->D0 = MAX(D0,0);
	intersect_segm->D1 = MIN(D1,(int)DEPTH);
	intersect_segm->ind0 = i0;
	intersect_segm->ind1 = i1;

	return intersect_segm;
}


