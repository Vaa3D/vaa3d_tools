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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

# include "RawVolume.h"
# include <stdio.h>


static char *get_path ( const char *_file_name ) throw (MyException);

RawVolume::RawVolume(const char* _file_name)  throw (MyException)
: VirtualVolume(get_path(_file_name),(float)1.0,(float)1.0,(float)1.0) 
{
	this->file_name = new char[strlen(_file_name)+1];
	strcpy(this->file_name, _file_name);

	init();
	initChannels();
}


RawVolume::~RawVolume(void)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin RawVolume::~SimpleVolume(void)\n");
	#endif

	if(file_name)
		delete[] file_name;
	if (img)
		delete[] img;
	if (sz)
		delete[] sz;
	if (fhandle)
		closeRawFile(fhandle);
}


void RawVolume::init ( ) throw (MyException)
{
	#if IM_VERBOSE > 3
        printf("\t\t\t\tin RawVolume::init()\n");
	#endif

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	img = (unsigned char *) 0;
	sz  = (V3DLONG *) 0;
	fhandle = (void *) 0;
	b_swap = 0;
	header_len = -1;

	//int berror = loadRaw2Stack(file_name,img,sz,datatype);

	char *internal_msg;
	if ( (internal_msg = loadRaw2Metadata(file_name,sz,datatype,b_swap,fhandle,header_len)) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"RawVolume::init: error in loading metadata - %s",internal_msg);
		throw MyException(err_msg);
	}

	DIM_V = (uint32) sz[1]; // in raw format first dimension is horizontal
	DIM_H = (uint32) sz[0];
	DIM_D = (uint32) sz[2];
}

void RawVolume::initChannels ( ) throw (MyException) {
	CHANS = (int) sz[3];
}


REAL_T *RawVolume::loadSubvolume_to_REAL_T(int V0,int V1, int H0, int H1, int D0, int D1)  throw (MyException) {
	#if IM_VERBOSE > 3
        printf("\t\t\t\tin RawVolume::loadSubvolume_to_REAL_T(V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d)\n", V0, V1, H0, H1, D0, D1);
	#endif

	char msg[IM_STATIC_STRINGS_SIZE];
	sprintf(msg,"in RawVolume::loadSubvolume_to_REAL_T: not implemented yet");
	throw MyException(msg);

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

	return subvol;
}


uint8 *RawVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels) throw (MyException) {

	// check #channels
	//if ( CHANS == 1 )
	//	*channels = 1;
	//else if ( CHANS <=3 )
	//	*channels = 3;
	//else {
	//	char err_msg[IM_STATIC_STRINGS_SIZE];
	//	sprintf(err_msg,"RawVolume::loadSubvolume_to_UINT8: too many channels [%d]",CHANS);
	//	throw MyException(err_msg);
	//}
	*channels = CHANS; 	// returns the exact number of channels;
	// WARNING: the caller must check if it suppports only 1 or 3 channels 
	
	//initializations
	V0 = (V0 == -1 ? 0	     : V0);
	V1 = (V1 == -1 ? DIM_V   : V1);
	H0 = (H0 == -1 ? 0	     : H0);
	H1 = (H1 == -1 ? DIM_H   : H1);
	D0 = (D0 == -1 ? 0		 : D0);
	D1 = (D1 == -1 ? DIM_D	 : D1);

	// allocate buffer
	sint64 sbv_height = V1 - V0;
	sint64 sbv_width  = H1 - H0;
	sint64 sbv_depth  = D1 - D0;
	sint64 total_buf_size = sbv_height * sbv_width * sbv_depth * (*channels) * datatype;
	uint8 *subvol = new uint8[total_buf_size];
	memset(subvol,0,sizeof(uint8)*total_buf_size);

	char *internal_msg;
	if ( (internal_msg = loadRaw2SubStack(fhandle,subvol,sz,H0,V0,D0,H1,V1,D1,datatype,b_swap,header_len)) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"RawVolume::init: error in loading metadata - %s",internal_msg);
		throw MyException(err_msg);
	}

	return subvol;
}



# define PATH_BUF_LEN   5000

static char path_buffer[PATH_BUF_LEN];
static char *suffixes[] = { ".raw", ".RAW", ".v3draw", ".V3DRAW", "" };
static int  suf_lens[] = {      3,      3,         6,         6     };

char *get_path ( const char *_file_name ) throw (MyException) {
	strcpy(path_buffer,_file_name);
	char *tPtr = 0;
	for ( int i=0; !tPtr && *suffixes[i] != '\0'; i++ )
		// check only if last characters match the suffix
		tPtr = strstr(path_buffer+strlen(path_buffer)-suf_lens[i]-1,suffixes[i]);
	if ( !tPtr ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"RawVolume::get_path: unknown suffix of source file");
		throw MyException(err_msg);
	}
	tPtr--;
	while ( *tPtr != '\\' && *tPtr != '/' )
		tPtr--;
	*(tPtr+1) = '\0';
	return path_buffer;
}
