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

#include "StackRaw.h"
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

StackRaw::StackRaw(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, char* _DIR_NAME) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("_ROW_INDEX=%d, _COL_INDEX=%d, _DIR_NAME=%s", _ROW_INDEX, _COL_INDEX, _DIR_NAME).c_str(), __iim__current__function__);

	this->CONTAINER = _CONTAINER;

	this->DIR_NAME = new char[strlen(_DIR_NAME)+1];
	strcpy(this->DIR_NAME, _DIR_NAME);

	this->ROW_INDEX = _ROW_INDEX;
	this->COL_INDEX = _COL_INDEX;

	STACKED_IMAGE = NULL;
	FILENAMES = NULL;
	HEIGHT = WIDTH = DEPTH = 0;
	ABS_V = ABS_H = 0;

	init();
}

StackRaw::StackRaw(VirtualVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("_ROW_INDEX=%d, _COL_INDEX=%d", _ROW_INDEX, _COL_INDEX).c_str(), __iim__current__function__);

	CONTAINER = _CONTAINER;
	ROW_INDEX = _ROW_INDEX;
	COL_INDEX = _COL_INDEX;

    DIR_NAME = 0;
    STACKED_IMAGE = 0;
    FILENAMES = 0;
	HEIGHT = WIDTH = DEPTH = 0;
	ABS_V = ABS_H = 0;

	unBinarizeFrom(bin_file);

	//initializing STACKED_IMAGE array
	this->STACKED_IMAGE = new CvMat*[DEPTH];
	for(uint32 z=0; z<DEPTH; z++)
        this->STACKED_IMAGE[z] = 0;
}

StackRaw::~StackRaw(void)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	for(uint32 z=0; z<DEPTH; z++)
	{
		if(STACKED_IMAGE[z])
			cvReleaseMat(&STACKED_IMAGE[z]);
		if(FILENAMES[z])
			delete[] FILENAMES[z];
	}

	if(STACKED_IMAGE)
		delete[] STACKED_IMAGE;
	if(FILENAMES)
		delete[] FILENAMES;
	if(DIR_NAME)
		delete[] DIR_NAME;
}

//binarizing-unbinarizing methods
void StackRaw::binarizeInto(FILE* file)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	uint16 str_size;
	uint32 i;

	fwrite(&HEIGHT, sizeof(int), 1, file);
	fwrite(&WIDTH, sizeof(int), 1, file);
	fwrite(&DEPTH, sizeof(int), 1, file);
	fwrite(&ABS_V, sizeof(int), 1, file);
	fwrite(&ABS_H, sizeof(int), 1, file);
	str_size = (uint16)(strlen(DIR_NAME) + 1);
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(DIR_NAME, str_size, 1, file);
	for(i = 0; i < DEPTH; i++)
	{
        str_size = (uint16)(strlen(FILENAMES[i]) + 1);
        fwrite(&str_size, sizeof(uint16), 1, file);
        fwrite(FILENAMES[i], str_size, 1, file);
	}
}

void StackRaw::unBinarizeFrom(FILE* file) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	uint16 str_size;
	uint32 i;
	size_t fread_return_val;

	fread_return_val = fread(&HEIGHT, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }
	fread_return_val = fread(&WIDTH, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }
	fread_return_val = fread(&DEPTH, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }
	fread_return_val = fread(&ABS_V, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }
	fread_return_val = fread(&ABS_H, sizeof(int), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }
	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }

    DIR_NAME = new char[str_size];
	fread_return_val = fread(DIR_NAME, str_size, 1, file);
    if(fread_return_val != 1)
    {
        fclose(file);
        throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
    }

	FILENAMES = new char*[DEPTH];
	for(i = 0; i < DEPTH; i++)
	{
		fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
        }

		FILENAMES[i] = new char[str_size];
		fread_return_val = fread(FILENAMES[i], str_size, 1, file);
		if(fread_return_val != 1)
        {
            fclose(file);
            throw IOException("in StackRaw::unBinarizeFrom(...): error while reading binary metadata file");
        }
	
	}
}

//Initializes all object's members given DIR_NAME
void StackRaw::init()
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d", ROW_INDEX, COL_INDEX).c_str(), __iim__current__function__);

	//LOCAL variables
	string tmp;
	DIR *cur_dir_lev3;
	dirent *entry_lev3;
	list<string> entries_lev3;
	list<string>::iterator entry_k;
	string entry;

	//building filenames_list
	char abs_path[STATIC_STRINGS_SIZE];
	sprintf(abs_path,"%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME);
	cur_dir_lev3 = opendir(abs_path);
	if (!cur_dir_lev3)
	{
		char errMsg[STATIC_STRINGS_SIZE];
		sprintf(errMsg, "in StackRaw::init(): can't open directory \"%s\"", abs_path);
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
	DEPTH = (int)entries_lev3.size();

    //----- Alessandro added on August 12, 2013
    //----- Bug fixed: exceeding the maximum number of directories opened at the same time
    closedir(cur_dir_lev3);

	//checking if current stack is not empty
	if(DEPTH == 0)
	{
		char msg[1000];
                sprintf(msg,"in StackRaw[%d,%d]::init(): stack in \"%s\" is empty", ROW_INDEX, COL_INDEX, abs_path);
        throw IOException(msg);
	}

	//converting filenames_list (STL list of C-strings) into FILENAMES (1-D array of C-strings)
	FILENAMES = new char*[DEPTH];
	for(uint32 z=0; z<DEPTH; z++)
	{
		entry = entries_lev3.front();
		FILENAMES[z] = new char[entry.size()+1];
		strcpy(FILENAMES[z], entry.c_str());
		entries_lev3.pop_front();
	}
	entries_lev3.clear();

	//initializing STACKED_IMAGE array
	this->STACKED_IMAGE = new CvMat*[DEPTH];
	for(uint32 z=0; z<DEPTH; z++)
		this->STACKED_IMAGE[z] = NULL;

	//extracting HEIGHT and WIDTH attributes from first slice
	char slice_fullpath[STATIC_STRINGS_SIZE];
	sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME, FILENAMES[0]);

    //IplImage *img_tmp = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE);

	// get file attributes
	char *err_rawfmt;
	void *fhandle;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	if ( (err_rawfmt = loadRaw2Metadata(slice_fullpath,sz,datatype,b_swap,fhandle,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in StackRaw[%d,%d]::init(): unable to open image \"%s\". Wrong path or format (%s)", 
			ROW_INDEX, COL_INDEX, slice_fullpath,err_rawfmt);
        throw IOException(msg);
	}
	closeRawFile((FILE *)fhandle);

	// I should check that sz[2] is 1

	HEIGHT = (int)sz[1];
	WIDTH  = (int)sz[0];
	//cvReleaseImage(&img_tmp);
	
	delete[] sz;
}

//PRINT method
void StackRaw::print()
{
	printf("\t |\t[%d,%d]\n", ROW_INDEX, COL_INDEX);
	printf("\t |\tDirectory:\t\t%s\n", DIR_NAME);
	printf("\t |\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", HEIGHT, WIDTH, DEPTH);
	printf("\t |\tAbsolute position:\t%d(V) x %d(H) x %d(D)\n", ABS_V, ABS_H, 0);
	/*printf("\t |\tFilenames:\t\n");
	for(int z=0; z<DEPTH; z++)
		printf("\t |\t\t%s\n",FILENAMES[z]);*/
	printf("\t |\n");
}

//loads images of current stack (from 'first_file' to 'last_file' extremes included, if not specified loads entire stack)
void StackRaw::loadStack(int first_file, int last_file)
{	
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d, first_file=%d, last_file=%d", ROW_INDEX, COL_INDEX, first_file, last_file).c_str(), __iim__current__function__);

	//LOCAL VARIABLES
	char slice_fullpath[1000];
	IplImage *slice_img_i;
	CvMat *mat_buffer;
	double scale_factor_16b, scale_factor_8b;

	//initializations
	first_file = (first_file == -1 ? 0       : first_file);
	last_file  = (last_file  == -1 ? DEPTH-1 : last_file);
	mat_buffer = cvCreateMatHeader(100,100,CV_16UC1); //NOTE: it doesn't matter what is the size or the type
	scale_factor_16b = (double) 1.0 / 65535.0;
	scale_factor_8b  = (double) 1.0 / 255.0;
	
	for(int file_i = first_file; file_i <= last_file; file_i++)
	{
		//checking if image has been previously loaded
		if(!STACKED_IMAGE[file_i])
		{
			//building image path
			sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getROOT_DIR(), DIR_NAME, FILENAMES[file_i]);
			
			//loading image
			slice_img_i = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);
			if(!slice_img_i)
			{
				char msg[1000];
				sprintf(msg,"in StackRaw[%d,%d]::loadStack(%d,%d): unable to open image \"%s\". Wrong path or format.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", 
					ROW_INDEX, COL_INDEX, first_file, last_file, slice_fullpath);
                throw IOException(msg);
			}		

			//allocating CvMat
			STACKED_IMAGE[file_i] = cvCreateMat(HEIGHT, WIDTH, CV_32FC1);

			//rescaling into 'data[sample_i]', i.e. after rescaling values are between 0.0 and 1.0
			cvConvertScale(cvGetMat(slice_img_i,mat_buffer),STACKED_IMAGE[file_i],slice_img_i->depth == IPL_DEPTH_16U ? scale_factor_16b : scale_factor_8b);
			
			//releasing image
			cvReleaseImage(&slice_img_i);
		}
	}
	cvReleaseMat(&mat_buffer);
}

//releases images of current stack (from 'first_file' to 'last_file' extremes included, if not specified loads entire stack)
void StackRaw::releaseStack(int first_file, int last_file)
{
    /**/iim::debug(iim::LEV3, strprintf("ROW_INDEX=%d, COL_INDEX=%d, first_file=%d, last_file=%d", ROW_INDEX, COL_INDEX, first_file, last_file).c_str(), __iim__current__function__);

	//initializations
	first_file = (first_file == -1 ? 0       : first_file);
	last_file  = (last_file  == -1 ? DEPTH-1 : last_file);

	//memory deallocation
	for(int file_i = first_file; file_i <= last_file; file_i++)
	{
		if(STACKED_IMAGE[file_i])
		{
			cvReleaseMat(&(STACKED_IMAGE[file_i]));
			STACKED_IMAGE[file_i] = NULL;
		}
	}
}


//returns a pointer to the intersection rectangle if the given area intersects current stack, otherwise returns NULL
Rect_t* StackRaw::Intersects(const Rect_t& area)
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
        return 0;
}
