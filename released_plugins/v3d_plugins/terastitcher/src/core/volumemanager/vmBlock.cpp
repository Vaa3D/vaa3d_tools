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

/******************
*    CHANGELOG    *
*******************
* 2015-08-05. Giulio.     @ADDED detailed error messages in loadImageStack and compute_z_ranges
* 2015-08-01. Giulio.     @FIXED bugs in sparse data management (compute_z_ranges)
* 2015-07-22. Giluio.     @ADDED support for spase data.
* 2015-02-13. Giulio.     @CHANGED 3D ioplugin is called instead of Tiff3DMngr functions
* 2015-01-17. Alessandro. @ADDED constructor for initialization from XML.
* 2015-01-17. Alessandro. @ADDED support for all-in-one-folder data (import from xml only).
* 2015-01-17. Alessandro. @FIXED missing throw(iom::exception) declaration in many methods.
* 2014-11-04. Giulio.     @FIXED bug in conversion from pixel uint16 to float
* 2014-09-09. Alessandro. @FIXED 'getXML()' method to deal with empty stacks.
* 2014-09-09. Alessandro. @BUG in 'loadImageStack()'. 'first_file' and 'last_file' are set to '-1' by default. But here, they are never checked nor corrected. 
* 2014-09-09. Alessandro. @FIXED 'loadImageStack()' method to deal with empty tiles.
* 2014-09-09. Alessandro. @TODO add support for sparse tiles.
* 2014-09-09. Alessandro. @FIXED 'init()' method to deal with empty tiles.
* 2014-09-05. Alessandro. @ADDED 'z_end' parameter in 'loadXML()' method to support sparse data feature.
* 2014-09-05. Alessandro & Iannello. @MODIFIED 'init()' and 'loadImageStack()' methods to deal with IO plugins
* 2014-08-30. Alessandro. @ADDED regular expression based filenames matching in 'init()' method.
* 2014-08-30. Alessandro. @FIXED all error messages starting with 'Stack...' and corrected to 'Block...'.
* 2014-08-30. Alessandro. @FIXED error messages in the 'init()' method (see checks of N_BYTESxCHAN and N_CHANS).
* 2014-08-25. Alessandro. @ADDED missing 'throw (iom::iom::exception)' statement in the 'loadImageStack()' method's signature.
* 2014-08-25. Alessandro. @REMOVED unused 'entry_k' variable declared in 'init()'.
*/


#ifdef _WIN32
	#include "dirent_win.h"
#else
	#include <dirent.h>
#endif
#include <string>
#include <list>
#include <stdlib.h>
#include <boost/xpressive/xpressive.hpp>
#include "vmBlock.h"
#include "vmBlockVolume.h"
#include "vmVirtualVolume.h"
#include "StackStitcher.h"
#include "Displacement.h"
#include "IOPluginAPI.h"


using namespace std;
using namespace iom;
using namespace vm;

//CONSTRUCTOR WITH ARGUMENTS
Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, const char* _DIR_NAME) throw (iom::exception)
	: VirtualStack()
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX=%d, int _COL_INDEX=%d, char* _DIR_NAME=%s)\n",
		_ROW_INDEX, _COL_INDEX, _DIR_NAME);
	#endif

    CONTAINER = _CONTAINER;
    DIR_NAME = new char[strlen(_DIR_NAME)+1];
    strcpy(DIR_NAME, _DIR_NAME);
    ROW_INDEX = _ROW_INDEX;
    COL_INDEX = _COL_INDEX;


	N_BLOCKS = -1;
    BLOCK_SIZE = 0;
    BLOCK_ABS_D = 0;

	init();
}

// 2015-01-17. Alessandro. @ADDED constructor for initialization from XML.
Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, TiXmlElement* stack_node, int z_end) throw (iom::exception)
	: VirtualStack()
{
	#if VM_VERBOSE > 3
	printf("\t\t\t\tin Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX=%d, int _COL_INDEX=%d, TiXmlElement*, int z_end=%d)\n",
		_ROW_INDEX, _COL_INDEX, z_end);
	#endif

	// check for valid stack node
	if(!stack_node)
		throw iom::exception("not an xml node", __iom__current__function__);
	if( strcmp(stack_node->ToElement()->Value(), "Stack") != 0)
		throw iom::exception(iom::strprintf("invalid xml node name: expected \"Stack\", found \"%s\"", stack_node->ToElement()->Value()), __iom__current__function__);

	CONTAINER = _CONTAINER;
	DIR_NAME = new char[strlen(stack_node->Attribute("DIR_NAME"))+1];
	strcpy(DIR_NAME, stack_node->Attribute("DIR_NAME"));
	ROW_INDEX = _ROW_INDEX;
	COL_INDEX = _COL_INDEX;

	N_BLOCKS = -1;
	BLOCK_SIZE = 0;
	BLOCK_ABS_D = 0;

	// first read image regex field (if any) from xml node
	readImgRegex(stack_node);

	// then scan folder for images
	init();

	// finally load other xml attributes
	loadXML(stack_node, z_end);
}

Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX, int _COL_INDEX, FILE* bin_file) throw (iom::exception)
	: VirtualStack()
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block::Block(BlockVolume* _CONTAINER, int _ROW_INDEX=%d, int _COL_INDEX=%d, FILE* bin_file)\n",
		_ROW_INDEX, _COL_INDEX);
	#endif

	CONTAINER = _CONTAINER;
	ROW_INDEX = _ROW_INDEX;
	COL_INDEX = _COL_INDEX;

	N_BLOCKS = -1;
    BLOCK_SIZE = 0;
    BLOCK_ABS_D = 0;

	unBinarizeFrom(bin_file);
}

Block::~Block(void)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin Block[%d,%d]::~Block()\n",ROW_INDEX, COL_INDEX);
	#endif

	if (BLOCK_SIZE)
		delete[] BLOCK_SIZE;
	if (BLOCK_ABS_D)
		delete[] BLOCK_ABS_D;

	for(int z=0; z<N_BLOCKS; z++)
	{
		if(FILENAMES[z])
			delete[] FILENAMES[z];
	}

	NORTH.clear();
	EAST.clear();
	SOUTH.clear();
	WEST.clear();

	if(STACKED_IMAGE)
		delete[] STACKED_IMAGE;
	if(FILENAMES)
		delete[] FILENAMES;
	if(DIR_NAME)
		delete[] DIR_NAME;
}

void Block::init() throw (iom::exception)
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block[%d,%d]::init()\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL variables
	string tmp;
	DIR *cur_dir_lev3;
	dirent *entry_lev3;
    list<string> entries_lev3;
	string entry;

	//opening stack directory
	char abs_path[S_STATIC_STRINGS_SIZE];
	sprintf(abs_path,"%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME);
	cur_dir_lev3 = opendir(abs_path);
	if (!cur_dir_lev3)
	{
		char errMsg[S_STATIC_STRINGS_SIZE];
        sprintf(errMsg, "in Block::init(): can't open directory \"%s\"", abs_path);
		throw iom::exception(errMsg);
	}

	//scanning third level of hierarchy which entries need to be ordered alphabetically. This is done using STL.
	while ((entry_lev3=readdir(cur_dir_lev3)))
	{
		tmp = entry_lev3->d_name;

		// 2015-01-17. Alessandro. @ADDED support for all-in-one-folder data (import from xml only).
		if(img_regex.empty())
        {
            if(tmp.compare(".") != 0 && tmp.compare("..") != 0 && tmp.find(".") != string::npos)
                entries_lev3.push_back(tmp);
        }
        else
        {
            boost::xpressive::sregex rex = boost::xpressive::sregex::compile(img_regex.c_str());
            boost::xpressive::smatch what;
            if(boost::xpressive::regex_match(tmp, what, rex))
               entries_lev3.push_back(tmp);
        }
	}
	entries_lev3.sort();
	N_BLOCKS = (int)entries_lev3.size();

	//closing dir
	closedir(cur_dir_lev3);

	// 2014-09-09. Alessandro. @FIXED to deal with empty tiles.
	// if stack is empty...
	if(N_BLOCKS == 0)
	{
		// ...and SPARSE_DATA option is active, then exit
		if(vm::SPARSE_DATA)
			return;
		// ...otherwise throw an exception
		else
			throw iom::exception(vm::strprintf("in Block[%s]::init(): stack is empty", DIR_NAME).c_str());
	}

	//converting filenames_list (STL list of <string> objects) into FILENAMES (1-D array of C-strings)
	FILENAMES = new char*[N_BLOCKS];
	for(int z=0; z<N_BLOCKS; z++)
	{
		entry = entries_lev3.front();
		FILENAMES[z] = new char[entry.size()+1];
		strcpy(FILENAMES[z], entry.c_str());
		entries_lev3.pop_front();
	}
	entries_lev3.clear();

	// allocate memory
	BLOCK_SIZE = new int[N_BLOCKS];
	BLOCK_ABS_D = new int[N_BLOCKS];

	// get blocks dimensions and compute DEPTH
	// 2014-09-05. Alessandro & Iannello. @MODIFIED to deal with IO plugins
	DEPTH=0;
	for ( int ib=0; ib<(int)N_BLOCKS; ib++ ) 
	{
		try {
			iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->readMetadata(
				iom::strprintf("%s/%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME, FILENAMES[ib]), 
				WIDTH, HEIGHT, BLOCK_SIZE[ib], N_BYTESxCHAN, N_CHANS);
		}
		catch ( iom::exception& exception ) {
			if ( strstr(exception.what(),"unable to open image") ) // the image is corrupted: capture the exception
				BLOCK_SIZE[ib] = 0;
			else // raise the exception again
				throw iom::exception(exception.what());
		}
        BLOCK_ABS_D[ib] = DEPTH; 
		DEPTH += BLOCK_SIZE[ib];
	}

	// 2015-07-22. Giulio. TO BE CHECKED: it should be subsitituted by sparse support (see above)
	// 2014-09-09. Alessandro. @FIXED to deal with empty tiles.
	// add to 'z_ranges' the full range
	// @TODO: support sparsity along Z.
	z_ranges.clear();
	z_ranges.push_back(vm::interval<int>(0, DEPTH));
}



//binarizing-unbinarizing methods
void Block::binarizeInto(FILE* file) throw (iom::exception)
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block[%d,%d]::binarizeInto(...)\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	int i;

	fwrite(&HEIGHT, sizeof(int), 1, file);
	fwrite(&WIDTH, sizeof(int), 1, file);
	fwrite(&DEPTH, sizeof(int), 1, file);
	fwrite(&ABS_V, sizeof(int), 1, file);
	fwrite(&ABS_H, sizeof(int), 1, file);
	fwrite(&ABS_D, sizeof(int), 1, file);
	str_size = (uint16)strlen(DIR_NAME) + 1;
	fwrite(&str_size, sizeof(uint16), 1, file);
	fwrite(DIR_NAME, str_size, 1, file);

	fwrite(&N_BLOCKS, sizeof(int), 1, file);
	for(i = 0; i < N_BLOCKS; i++)
	{
		// 2015-07-25. Giulio. support for sparse data: missing blocks have store an empty string as filename
		str_size = static_cast<uint16>( FILENAMES[i] ? strlen(FILENAMES[i]) + 1 : 0);
		fwrite(&str_size, sizeof(uint16), 1, file);
		if(FILENAMES[i])
			fwrite(FILENAMES[i], str_size, 1, file);
		fwrite(BLOCK_SIZE+i, sizeof(int), 1, file);
		fwrite(BLOCK_ABS_D+i, sizeof(int), 1, file);
	}
	
	fwrite(&N_CHANS, sizeof(int), 1, file);
	fwrite(&N_BYTESxCHAN, sizeof(int), 1, file);
}

void Block::unBinarizeFrom(FILE* file) throw (iom::exception)
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block[%d,%d]::unBinarizeFrom(...)\n",ROW_INDEX, COL_INDEX);
	#endif

	//LOCAL VARIABLES
	uint16 str_size;
	int i;
	size_t fread_return_val;

	fread_return_val = fread(&HEIGHT, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	fread_return_val = fread(&WIDTH, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	fread_return_val = fread(&DEPTH, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	fread_return_val = fread(&ABS_V, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	fread_return_val = fread(&ABS_H, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	fread_return_val = fread(&ABS_D, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
        throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}

	fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	DIR_NAME = new char[str_size];
	fread_return_val = fread(DIR_NAME, str_size, 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}

	fread_return_val = fread(&N_BLOCKS, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}
	FILENAMES = new char*[N_BLOCKS];
	BLOCK_SIZE = new int[N_BLOCKS];
	BLOCK_ABS_D = new int[N_BLOCKS];
	for(i = 0; i < N_BLOCKS; i++)
	{
		fread_return_val = fread(&str_size, sizeof(uint16), 1, file);
		if(fread_return_val != 1) {
			fclose(file);
			throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
		}
		// 2015-07-25. Giulio. support for sparse data: missing blocks have an empty string as filename in the binarized file
		FILENAMES[i] = str_size ? new char[str_size] : 0;		
		if(str_size)
		{
			fread_return_val = fread(FILENAMES[i], str_size, 1, file);
			if(fread_return_val != 1) {
				fclose(file);
				throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
			}
		}
	
		fread_return_val = fread(BLOCK_SIZE+i, sizeof(int), 1, file);
		if(fread_return_val != 1) {
			fclose(file);
			throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
		}

		fread_return_val = fread(BLOCK_ABS_D+i, sizeof(int), 1, file);
		if(fread_return_val != 1) {
			fclose(file);
			throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
		}
	}

	fread_return_val = fread(&N_CHANS, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}

	fread_return_val = fread(&N_BYTESxCHAN, sizeof(int), 1, file);
	if(fread_return_val != 1) {
		fclose(file);
		throw iom::exception("in Block::unBinarizeFrom(...): error while reading binary metadata file");
	}

	// 2015-07-22. Giulio. @ADDED support for sparse data.
	compute_z_ranges();
}


//loads image stack from <first_file> to <last_file> extremes included, if not specified loads entire Stack
iom::real_t* Block::loadImageStack(int first_file, int last_file) throw (iom::exception)
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block[%d,%d]::loadImageStack(first_file = %d, last_file = %d)\n",ROW_INDEX, COL_INDEX, first_file, last_file);
	#endif
	int first, last;
	float scale_factor;
	char slice_fullpath[2000];

	// 2014-09-09. Alessandro. @BUG. 'first_file' and 'last_file' are set to '-1' by default. But here, they are never checked nor corrected. 
	// I added a very simple (but not complete) check here.
	// Be careful if you want to adjust 'last_file' to 'DEPTH' when 'last_file == -1'. 'DEPTH' could be 0 if stack is empty.
	if(first_file < 0 || last_file < 0 || first_file > last_file)
		throw iom::exception(vm::strprintf("in Block[%s]::loadImageStack(): invalid file selection [%d,%d]", DIR_NAME, first_file, last_file).c_str());


	// 2014-09-09. Alessandro. @FIXED 'loadImageStack()' method to deal with empty tiles.
	// if stack is empty in the given range, just return a black image
	if(isEmpty(first_file, last_file))
	{
		// allocate and initialize a black stack
		uint64 image_size = static_cast<uint64>(WIDTH) * static_cast<uint64>(HEIGHT) * static_cast<uint64>(last_file-first_file+1);
		STACKED_IMAGE = new iom::real_t[image_size];
		for(uint64 k=0; k<image_size; k++)
			STACKED_IMAGE[k] = 0;

		return STACKED_IMAGE;
	}

	Segm_t *intersect_segm = Intersects(first_file, last_file+1); // the second parameter should be the last slice + 1
	unsigned char *data = new unsigned char[HEIGHT * WIDTH * (last_file-first_file+1) * N_BYTESxCHAN * N_CHANS];
	unsigned char *temp = data;

	for(int i = intersect_segm->ind0; i <= intersect_segm->ind1; i++)
	{
		//if ( temp - data > (HEIGHT * WIDTH * (last_file-first_file+1) * N_BYTESxCHAN * N_CHANS) )
		//	throw iom::exception(vm::strprintf("in Block[%s]::loadImageStack(): buffer overrun at block %d", DIR_NAME, i-1).c_str());
			
		first = (first_file > BLOCK_ABS_D[i]) ? first_file-BLOCK_ABS_D[i] : 0 ;
		last = (last_file < BLOCK_ABS_D[i]+BLOCK_SIZE[i]-1) ?  last_file-BLOCK_ABS_D[i] : BLOCK_SIZE[i]-1 ;
		if ( FILENAMES[i] ) { // 2015-07-26. Giulio. @ADDED sparsedata support
			sprintf(slice_fullpath, "%s/%s/%s", CONTAINER->getSTACKS_DIR(), DIR_NAME, FILENAMES[i]);

			// 2014-09-05. Alessandro & Iannello. @MODIFIED to deal with IO plugins
			//iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->readData(slice_fullpath, WIDTH, HEIGHT, temp, first, last);
			iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->readData(slice_fullpath,WIDTH,HEIGHT,BLOCK_SIZE[i],N_BYTESxCHAN,N_CHANS,temp,first,last+1);
		}
		temp +=  HEIGHT * WIDTH * (last-first+1) * N_BYTESxCHAN * N_CHANS;
	}

	//conversion from unsigned char to iom::real_t
	if (N_CHANS == 2 || N_CHANS > 3) // only monocromatic or RGB images are supported
	{
		char errMsg[2000];
		sprintf(errMsg, "in Block[%d,%d]::loadImageStack(...): %d channels are not supported.", ROW_INDEX, COL_INDEX, N_CHANS);
		throw iom::exception(errMsg);
	}

	if ( N_BYTESxCHAN == 1 )
		scale_factor  = 255.0F;
	else if ( N_BYTESxCHAN == 2 )
		scale_factor = 65535.0F;
	else
	{
		char errMsg[2000];
		sprintf(errMsg, "in Block[%d,%d]::loadImageStack(...): Too many bytes per channel (%d).", ROW_INDEX, COL_INDEX, N_BYTESxCHAN);
		throw iom::exception(errMsg);
	}

	STACKED_IMAGE = new iom::real_t[HEIGHT * WIDTH * (last_file-first_file+1)]; // this image is an intensity image (only one channel)

	int offset;

	if ( N_CHANS == 1 ) {
		// 2014-11-04. Giulio. @FIXED
		if ( N_BYTESxCHAN == 1 )
			for(int i = 0; i <HEIGHT * WIDTH * (last_file-first_file+1); i++)
				STACKED_IMAGE[i] = (iom::real_t) data[i]/scale_factor;
		else // N_BYTESxCHAN == 2
			for(int i = 0; i <HEIGHT * WIDTH * (last_file-first_file+1); i++)
				STACKED_IMAGE[i] = (iom::real_t) ((iom::uint16 *)data)[i]/scale_factor; // data must be interpreted as a uint16 array
	}
	else { // conversion to an intensity image
		// test how channel are stored in the returned buffer 'data'
		if ( iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->isChansInterleaved() ) {
			if ( iom::CHANS == iom::ALL ) {
				char errMsg[2000];
				sprintf(errMsg, "in Block[%d,%d]::loadImageStack(...): conversion from multi-channel to intensity images not supported.", ROW_INDEX, COL_INDEX);
				throw iom::exception(errMsg);
			}
			else if ( iom::CHANS == iom::R ) {
				offset = 0;
			}
			else if ( iom::CHANS == iom::G ) {
				offset = 1;
			}
			else if ( iom::CHANS == iom::B ) {
				offset = 2;
			}
			else {
				char errMsg[2000];
				sprintf(errMsg, "in Block[%d,%d]::loadImageStack(...): wrong value for parameter iom::CHANNEL_SELECTION.", ROW_INDEX, COL_INDEX);
				throw iom::exception(errMsg);
			}
			// 2014-11-04. Giulio. @FIXED
			if ( N_BYTESxCHAN == 1 )
				for(int i = 0; i <HEIGHT * WIDTH * (last_file-first_file+1); i++)
					STACKED_IMAGE[i] = (iom::real_t) data[3*i + offset]/scale_factor;
			else // N_BYTESxCHAN == 2
				for(int i = 0; i <HEIGHT * WIDTH * (last_file-first_file+1); i++)
					STACKED_IMAGE[i] = (iom::real_t) ((iom::uint16 *)data)[3*i + offset]/scale_factor; // data must be interpreted as a uint16 array
		}
		else {
			char errMsg[2000];
			sprintf(errMsg, "in Block[%d,%d]::loadImageStack(...): channels are not interleaved in the returned buffer 'data', conversion to intensity images not supported yet.", ROW_INDEX, COL_INDEX);
			throw iom::exception(errMsg);
		}
	}

	delete [] data;


	delete intersect_segm;

	return STACKED_IMAGE;
}

//deallocates memory used by STACKED_IMAGE
void Block::releaseImageStack()
{
	if(STACKED_IMAGE)
		delete[] STACKED_IMAGE;
	STACKED_IMAGE=NULL;
}


//XML methods
TiXmlElement* Block::getXML()
{
	#if VM_VERBOSE > 3
    printf("......in Block[%d,%d]::getXML()\n",ROW_INDEX, COL_INDEX);
	#endif

	string blockSizes, blocksAbsD;
	TiXmlElement *xml_representation = new TiXmlElement("Stack");

	xml_representation->SetAttribute("N_BLOCKS",N_BLOCKS);

	// 2014-09-09. Alessandro. @FIXED 'getXML()' method to deal with empty stacks.
	char str_buf[1000];
	if(N_BLOCKS)
	{
		#if __cplusplus == 201103L // C++11 compliant compiler
			blockSizes.append(to_string(BLOCK_SIZE[0]));
			blocksAbsD.append(to_string(BLOCK_ABS_D[0]));
		#else
			sprintf(str_buf,"%i",BLOCK_SIZE[0]);
			blockSizes.append(str_buf);
			sprintf(str_buf,"%i",BLOCK_ABS_D[0]);
			blocksAbsD.append(str_buf);
		#endif
	}

	for(int j =1;j<N_BLOCKS;j++){
		blockSizes.append(",");
		blocksAbsD.append(",");
		
#if __cplusplus == 201103L // C++11 compliant compiler
		blockSizes.append(to_string(BLOCK_SIZE[j]));
		blocksAbsD.append(to_string(BLOCK_ABS_D[j]));
#else
		sprintf(str_buf,"%i",BLOCK_SIZE[j]);
		blockSizes.append(str_buf);
		sprintf(str_buf,"%i",BLOCK_ABS_D[j]);
		blocksAbsD.append(str_buf);
#endif
    }
    
	char *BLOCK_SIZES = (char*)blockSizes.c_str();
	char *BLOCKS_ABS_D = (char*)blocksAbsD.c_str();

	xml_representation->SetAttribute("BLOCK_SIZES",BLOCK_SIZES);
	xml_representation->SetAttribute("BLOCKS_ABS_D",BLOCKS_ABS_D);

	xml_representation->SetAttribute("N_CHANS",N_CHANS);
	xml_representation->SetAttribute("N_BYTESxCHAN",N_BYTESxCHAN);
	xml_representation->SetAttribute("ROW",ROW_INDEX);
	xml_representation->SetAttribute("COL",COL_INDEX);
	xml_representation->SetAttribute("ABS_V",ABS_V);
	xml_representation->SetAttribute("ABS_H",ABS_H);
	xml_representation->SetAttribute("ABS_D",ABS_D);
	xml_representation->SetAttribute("STITCHABLE",stitchable ? "yes" : "no");
	xml_representation->SetAttribute("DIR_NAME",DIR_NAME);

	// 2015-07-22. Giulio. @ADDED 'Z_RANGES' attribute in the xml node
	std::string z_ranges_string;
	for(int k=0; k<z_ranges.size(); k++)
		z_ranges_string += vm::strprintf("[%d,%d)%s", z_ranges[k].start, z_ranges[k].end, k == z_ranges.size()-1 ? "" : ";");
	xml_representation->SetAttribute("Z_RANGES",z_ranges_string.c_str());
	writeImgRegex(xml_representation);

	vector<Displacement*>::iterator i;
	TiXmlElement *NORTH_displacements = new TiXmlElement("NORTH_displacements");
	for(i = NORTH.begin(); i != NORTH.end(); i++)
		NORTH_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *EAST_displacements = new TiXmlElement("EAST_displacements");
	for(i = EAST.begin(); i != EAST.end(); i++)
		EAST_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *SOUTH_displacements = new TiXmlElement("SOUTH_displacements");
	for(i = SOUTH.begin(); i != SOUTH.end(); i++)
		SOUTH_displacements->LinkEndChild((*i)->getXML());
	TiXmlElement *WEST_displacements = new TiXmlElement("WEST_displacements");
	for(i = WEST.begin(); i != WEST.end(); i++)
		WEST_displacements->LinkEndChild((*i)->getXML());
	xml_representation->LinkEndChild(NORTH_displacements);
	xml_representation->LinkEndChild(EAST_displacements);
	xml_representation->LinkEndChild(SOUTH_displacements);
	xml_representation->LinkEndChild(WEST_displacements);

	return xml_representation;
}

void Block::loadXML(
	TiXmlElement *stack_node,
	int z_end)					// 2014-09-05. Alessandro. @ADDED 'z_end' parameter to support sparse data feature
								//			   Here 'z_end' identifies the range [0, z_end) that slices can span
throw (iom::exception)
{
	#if VM_VERBOSE > 3
    printf("\t\t\t\tin Block[%d,%d]::loadXML(TiXmlElement *stack_node)\n",ROW_INDEX, COL_INDEX);
	#endif

	stack_node->QueryIntAttribute("N_BLOCKS",&N_BLOCKS);

	const char *BLOCK_SIZES=stack_node->Attribute("BLOCK_SIZES");
	char *BLOCK_SIZES2 = new char [strlen (BLOCK_SIZES) + 1];
	strcpy (BLOCK_SIZES2,BLOCK_SIZES);
	char * pch=strtok (BLOCK_SIZES2,",");
	int j=0;
	while (pch != NULL)
	{
		BLOCK_SIZE[j]=atoi(pch);
		j++; //141027_Onofri: added missing increment
		pch = strtok (NULL, ",");
	}
	delete[] BLOCK_SIZES2; 

	const char *BLOCKS_ABS_D=stack_node->Attribute("BLOCKS_ABS_D");
	char *BLOCKS_ABS_D2 = new char [strlen (BLOCKS_ABS_D) + 1];
	strcpy (BLOCKS_ABS_D2,BLOCKS_ABS_D);
	pch=strtok (BLOCKS_ABS_D2,",");
	j=0;
	while (pch != NULL)
	{
		BLOCK_ABS_D[j]=atoi(pch);
		j++;
		pch = strtok (NULL, ",");
	}
	delete[] BLOCKS_ABS_D2;
	stack_node->QueryIntAttribute("N_CHANS",&N_CHANS);
	stack_node->QueryIntAttribute("N_BYTESxCHAN",&N_BYTESxCHAN);
	stack_node->QueryIntAttribute("ABS_V", &ABS_V);
	stack_node->QueryIntAttribute("ABS_H", &ABS_H);
	stack_node->QueryIntAttribute("ABS_D", &ABS_D);
	stitchable = strcmp(stack_node->Attribute("STITCHABLE"),"yes")==0 ? true : false;
	if(strcmp(stack_node->Attribute("DIR_NAME"), DIR_NAME) != 0)
	{
		char errMsg[2000];
        sprintf(errMsg, "in Block[%d,%d]::loadXML(...): Mismatch between xml file and %s in <DIR_NAME> field.", ROW_INDEX, COL_INDEX, vm::BINARY_METADATA_FILENAME.c_str());
		throw iom::exception(errMsg);
	}

	// 2015-01-17. Alessandro. @ADDED support for all-in-one-folder data (import from xml only).
	readImgRegex(stack_node);

	// 2015-07-22. Alessandro. @ADDED 'Z_RANGES' attribute in the xml node
	const char* z_ranges_c = stack_node->Attribute("Z_RANGES");
	if( z_ranges_c )	// field is present: we can run 'sparse data' code
	{
		// nonempty tile
		if ( strlen(z_ranges_c) != 0 )
		{
			// parse 'Z_RANGES' field
			z_ranges.clear();
			std::string z_ranges_c_str = z_ranges_c;
			std::string z_ranges_string = vm::cls(z_ranges_c_str);
			std::vector<std::string> tokens;
			vm::split(z_ranges_string, ";", tokens);
			for(int i=0; i<tokens.size(); i++)
			{
				// ignore empty tokens
				if(tokens[i].empty())
					continue;

				// parse [start, end) interval
				std::vector<std::string> extremes;
				vm::split(tokens[i].substr(1, tokens[i].size()-2), ",", extremes);

				// check correct parsing
				if(extremes.size() != 2)
					throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): cannot parse 'Z_RANGES' subentry \"%s\"", DIR_NAME, tokens[i].c_str()).c_str());

				// get integral range
				int start = vm::str2num<int>(extremes[0]);
				int end   = vm::str2num<int>(extremes[1]);

				// check valid range
				if(start < 0 ||  start >= end ||  end > z_end)
					throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): 'Z_RANGES' subentry \"%s\" is out of range [%d,%d) ", DIR_NAME, tokens[i].c_str(), 0, z_end).c_str());

				// push range
				z_ranges.push_back(vm::interval<int>(start, end));
			}

			// check precondition: z_ranges should contain consecutive but not contiguous intervals [a_1,b_1), [a_2,b_2), ... such that a_n > b_(n-1)
			for(int i=1; i<z_ranges.size(); i++)
				if(z_ranges[i].start <= z_ranges[i-1].end)
					throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): wrong sequence in 'Z_RANGES' attribute. 'Z-RANGES' should contain "
					"consecutive but not contiguous intervals [a_1,b_1), [a_2,b_2), ... such that a_n > b_(n-1). "
					"Found a_%d(%d) <= b_%d(%d)", DIR_NAME, i, z_ranges[i].start, i-1, z_ranges[i-1].end).c_str());

			// if 'FILENAMES' is not a sparse list...
			if(DEPTH != z_end)
			{
				// make 'FILENAMES' a sparse list
				char **FILENAMES_sparse = new char*[z_end];
				for(int z=0; z<z_end; z++)
					FILENAMES_sparse[z] = 0;
				int i=0;
				for(int k=0; k<z_ranges.size(); k++)
					for(int z=z_ranges[k].start; z<z_ranges[k].end; z++)
					{
						// if we have used all the available slices and the current 'z_range' asks for others, throw an exception
						if(i >= DEPTH)
							throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): no more slices available to cover z-range [%d,%d)", 
							DIR_NAME, z_ranges[k].start, z_ranges[k].end).c_str());

						FILENAMES_sparse[z] = FILENAMES[i++];
					}
				
				// substitute list of filenames with its sparse version
				if(FILENAMES)
					delete[] FILENAMES;
				FILENAMES = FILENAMES_sparse;
				DEPTH = z_end;
			}
			// ...otherwise check if the sparse list matches with z_ranges
			else
			{
				for(int i=0; i<N_BLOCKS; i++)
					if(FILENAMES[i] && isEmpty(BLOCK_ABS_D[i],BLOCK_ABS_D[i]+BLOCK_SIZE[i]-1))
						throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): mismatch between sparse list loaded from mdata.bin and 'Z_RANGES' loaded from xml. Please delete mdata.bin and re-load the xml.", DIR_NAME));
					else if(FILENAMES[i] == 0 && isComplete(BLOCK_ABS_D[i],BLOCK_ABS_D[i]+BLOCK_SIZE[i]-1))
						throw iom::exception(vm::strprintf("in Block(%s)::loadXML(): mismatch between sparse list loaded from mdata.bin and 'Z_RANGES' loaded from xml. Please delete mdata.bin and re-load the xml.", DIR_NAME));
			}
		}
		// 'Z_RANGES' field is present, but is empty: assume empty tile
		else
		{
			// make 'FILENAMES' a sparse list
			if(FILENAMES)
				delete[] FILENAMES;
			FILENAMES = new char*[z_end];
			for(int z=0; z<z_end; z++)
				FILENAMES[z] = 0;
			DEPTH = z_end;
		}
	}
	// no 'Z_RANGES' field (old or incomplete XML)
	else
	{
		// if 'sparse data' option is active, 'Z_RANGES' must be provided (then throw an exception)
		if(SPARSE_DATA)
			throw iom::exception(vm::strprintf("in Block::loadXML(): cannot find 'Z_RANGES' attribute in stack \"%s\" xml node ('sparse data' option is active). "
			"Please edit your XML file accordingly or disable 'sparse data' option.", DIR_NAME).c_str());
		
		// 'sparse data' option is not active and 'Z_RANGES' field is not present, but DEPTH does not match with z_end: throw an exception
		if(DEPTH != z_end)
			throw iom::exception(vm::strprintf("in Block::loadXML(): stack %s has %d slices, but volume has %d. If that was your intent, please use "
			"'sparse data' option.", DIR_NAME, DEPTH, z_end).c_str());

		// all checks were successful: assume tile is not sparse
        z_ranges.clear();
        z_ranges.push_back(vm::interval<int>(0, DEPTH));
	}
	

	TiXmlElement *NORTH_displacements = stack_node->FirstChildElement("NORTH_displacements");
	for(TiXmlElement *displ_node = NORTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		NORTH.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *EAST_displacements = stack_node->FirstChildElement("EAST_displacements");
	for(TiXmlElement *displ_node = EAST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		EAST.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *SOUTH_displacements = stack_node->FirstChildElement("SOUTH_displacements");
	for(TiXmlElement *displ_node = SOUTH_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		SOUTH.push_back(Displacement::getDisplacementFromXML(displ_node));
	TiXmlElement *WEST_displacements = stack_node->FirstChildElement("WEST_displacements");
	for(TiXmlElement *displ_node = WEST_displacements->FirstChildElement("Displacement"); displ_node; displ_node = displ_node->NextSiblingElement())
		WEST.push_back(Displacement::getDisplacementFromXML(displ_node));

	// 2015-07-22. Giulio. TO BE CHECKED: it should be subsitituted by sparse support (see above)
	// 2014-09-09. Alessandro. @FIXED to deal with empty tiles.
	// if tile is not empty, add to 'z_ranges' the full range
	// @TODO: support sparsity along Z.
	//if(N_BLOCKS > 0)
	//{
	//	z_ranges.clear();
	//	z_ranges.push_back(vm::interval<int>(0, z_end));
	//}
}

Segm_t* Block::Intersects(int D0, int D1) {

	if ( D0 >= BLOCK_ABS_D[N_BLOCKS-1]+BLOCK_SIZE[N_BLOCKS-1] || D1 <= 0 )
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
		if ( D1 > BLOCK_ABS_D[i1] ) // GI_141110 re-changed '>=' to '>' (D1 is the last index + 1)
			found1 = true;
		else
			i1--;
	// !found1 -> i1 = 0

	Segm_t *intersect_segm = new Segm_t;
	intersect_segm->D0 = max(D0,0);
	intersect_segm->D1 = min(D1,(int)DEPTH);
	intersect_segm->ind0 = i0;
	intersect_segm->ind1 = i1;

	return intersect_segm;
}

// compute 'z_ranges'
void 
	Block::compute_z_ranges(
	std::pair<int,int> const * z_coords /*= 0*/)	// pair of z-coordinates corresponding to the whole volume depth
	throw (iom::exception)							// if null, 'z_ranges' will be compute based on 'FILENAMES' vector
{
	// if 'z_coords' has been provided, we use it to associate each file in 'FILENAMES' to the correspondent z-coordinate
	if(z_coords)
	{
		// compute the number of slices corresponding to the volume
		int n_slices = (int) floor((float)(z_coords->second - z_coords->first) / (10 * CONTAINER->getVXL_D()) + 0.5);
		// check non-zero N_SLICES
		if (CONTAINER->getN_SLICES() != n_slices) {
			char msg[S_STATIC_STRINGS_SIZE];
			sprintf(msg,"in Block::compute_z_ranges(...): in stack [%d,%d] error in the number of slices (CONTAINER->getN_SLICES()=%d, z_coords=[%d,%d], CONTAINER->getVXL_D()=%f, n_slices=%d)",
				ROW_INDEX, COL_INDEX, CONTAINER->getN_SLICES(), z_coords->first, z_coords->second, CONTAINER->getVXL_D(), n_slices);
			throw iom::exception(msg);
		}

		char **FILENAMES_temp  = new char*[2*N_BLOCKS+1];
		int *BLOCK_SIZE_temp   = new int[2*N_BLOCKS+1];
		int *BLOCK_ABS_D_temp  = new int[2*N_BLOCKS+1];

		int n_blocks = 0;
		int n = 0;  // index of the first slice of the block to be processed
		int z_next; // D coordinate of block to be processed
		int z = z_coords->first; // current D coordinate 
		for ( int i=0; i<N_BLOCKS; i++ ) {
			z_next = atoi(VirtualVolume::name2coordZ(FILENAMES[i]).c_str());
			if ( z_next > z ) {
				FILENAMES_temp[n_blocks]   = 0;
				BLOCK_SIZE_temp[n_blocks]  = (int) floor((float)(z_next - z)/(10 * CONTAINER->getVXL_D())); // 2015-08-01. Giulio. floor is used to avoid the introduction of one more empty slice; if some slice is missing it will be added at the bottom of the stack
				BLOCK_ABS_D_temp[n_blocks] = n;
				n += BLOCK_SIZE_temp[n_blocks];
				n_blocks++;
			}
			FILENAMES_temp[n_blocks]   = FILENAMES[i];
			BLOCK_SIZE_temp[n_blocks]  = BLOCK_SIZE[i];
			BLOCK_ABS_D_temp[n_blocks] = n;
			n += BLOCK_SIZE_temp[n_blocks];
			n_blocks++;
			z = (int) floor(z_next + BLOCK_SIZE[i] * 10 * CONTAINER->getVXL_D() + 0.5);
		}
		if ( n < n_slices ) {
			FILENAMES_temp[n_blocks]   = 0;
			BLOCK_SIZE_temp[n_blocks]  = n_slices - n;
			BLOCK_ABS_D_temp[n_blocks] = n;
			n_blocks++;
		}
		else if ( n > n_slices ) {
			char msg[S_STATIC_STRINGS_SIZE];
			sprintf(msg,"in Block::compute_z_ranges(...): in stack [%d,%d] too many slices (%d instead of %d)", ROW_INDEX, COL_INDEX, n, n_slices);
			throw iom::exception(msg);
		}

		if ( N_BLOCKS ) {
			delete[] FILENAMES;
			delete[] BLOCK_SIZE;
			delete[] BLOCK_ABS_D;
		}

		FILENAMES = new char *[n_blocks];
		memcpy(FILENAMES,FILENAMES_temp,n_blocks*sizeof(char *));
		delete[] FILENAMES_temp;

		BLOCK_SIZE = new int[n_blocks];
		memcpy(BLOCK_SIZE,BLOCK_SIZE_temp,n_blocks*sizeof(int));
		delete[] BLOCK_SIZE_temp;

		BLOCK_ABS_D = new int[n_blocks];
		memcpy(BLOCK_ABS_D,BLOCK_ABS_D_temp,n_blocks*sizeof(int));
		delete[] BLOCK_ABS_D_temp;

		N_BLOCKS = n_blocks;
		DEPTH    = n_slices;
	}

	// compute 'z_range' from 'FILENAMES'
	bool interval_start = true;
	bool interval_stop = false;
	int last_slice;
    z_ranges.clear();
	for(int k=0; k<N_BLOCKS; k++)
	{
		if(FILENAMES[k] && interval_stop)
		{
			last_slice = BLOCK_ABS_D[k] + BLOCK_SIZE[k];
		}
		if(FILENAMES[k] && interval_start)
		{
			interval_start = false;
			interval_stop  = true;
			z_ranges.push_back(vm::interval<int>(BLOCK_ABS_D[k], -1));
			last_slice = BLOCK_ABS_D[k] + BLOCK_SIZE[k];
		}
		if(FILENAMES[k] == 0 && interval_stop)
		{
			interval_start = true;
			interval_stop  = false;
			z_ranges.back().end = last_slice;
		}
	}
	if(!z_ranges.empty() && z_ranges.back().end == -1)
		z_ranges.back().end = DEPTH;
}