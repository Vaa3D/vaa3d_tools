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
* 2017-04-20. Giulio      @CHANGED calls to 'IMS_HDF5init' 
* 2017-04-20. Giulio      @FIXED a missing parameter in the call to 'IMS_HDF5init' in 'appendSlice'
* 2017-04-17. Giulio.     @ADDED the generation of IMS files with default metadata
* 2017-04-07. Giulio.     @ADDED ability to return a selected channel instead of all channels
* 2016-10-27. Giulio.     @ADDED routines for extracting additional parameters
* 2016-10-16. Giulio.     @CREATED
*/

#include <cstddef>			// 2014-09-18. Alessandro. @FIXED compilation issue on gcc compilers.
#include "IMS_HDF5.h"
#include "IMS_HDF5Mngr.h"
#include "hdf5.h"

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_3D(IMS_HDF5)


// insert here your plugin description that will be displayed on the user interface
std::string iomanager::IMS_HDF5::desc()
{
	return	"******************************************************\n"
			"* Imaris IMS HDF5 v.1.1                              *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* 3D image-based I/O plugin that uses the HDF5       *\n"
			"* library to implement the format used by Imaris     *\n"
			"*                                                    *\n"
			"* Accepted configuration parameters:                 *\n"
			"*  - resolution=R                                    *\n"
			"*  - timepoint=T                                     *\n"
			"*  - channel=C                                       *\n"
			"*                                                    *\n"
			"******************************************************\n";
}


// Return if channels are interleaved (in case the image has just one channel return value is indefinite)
bool 
	iomanager::IMS_HDF5::isChansInterleaved( ) 
{
	return false;
}


// read image metadata
void 
	iomanager::IMS_HDF5::readMetadata(
	std::string img_path,		// (INPUT)	image filepath
	int & img_width,			// (OUTPUT) image width  (in pixels)
	int & img_height,			// (OUTPUT) image height (in pixels)
	int & img_depth,			// (OUTPUT) image depth (in pixels)
	int & img_bytes_x_chan,	// (OUTPUT) number of bytes per channel
	int & img_chans,			// (OUTPUT) number of channels
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);

	void *file_descr;
	void *volume_descr;
	float vxl1;
	float vxl2;
	float vxl3;
	float org1;
	float org2;
	float org3;
	int n_timepoints;
	int t0;
	int t1;
	iim::uint32 width;
	iim::uint32 height;
	iim::uint32 depth;

	int res = 0;
	int tp  = 0;
	std::string str_value;

	if ( getParamValue(params,"resolution",str_value) )
		res = atoi(str_value.c_str());
	if ( getParamValue(params,"timepoint",str_value) )
		tp = atoi(str_value.c_str());

	IMS_HDF5init(img_path,file_descr);
	IMS_HDF5getVolumeInfo(file_descr,tp,res,volume_descr,vxl1,vxl2,vxl3,org1,org2,org3,height,width,depth,img_chans,img_bytes_x_chan,n_timepoints,t0,t1);
	IMS_HDF5closeVolume(volume_descr);
	IMS_HDF5close(file_descr);

	img_width  = width;
	img_height = height;
	img_depth  = depth;
	
}


// Read 3D image data
unsigned char *						// (OUTPUT) buffer containing the read image data
	iomanager::IMS_HDF5::readData(
	std::string img_path,			// (INPUT) image filepath
	int & img_width,				// (INPUT/OUTPUT) image width  (in pixels)
	int & img_height,				// (INPUT/OUTPUT) image height (in pixels)
	int & img_depth,				// (INPUT/OUTPUT) image depth (in pixels)
	int & img_bytes_x_chan,			// (INPUT/OUTPUT) number of bytes per channel
	int & img_chans,				// (INPUT/OUTPUT) number of channels to be read
	unsigned char *data,			// (INPUT) image data
	int z0,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int z1,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, data = %p, z0 = %d, z1 = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, data, z0, z1, params.c_str()).c_str(), __iom__current__function__);

	void *file_descr;
	void *volume_descr;
	float vxl1;
	float vxl2;
	float vxl3;
	float org1;
	float org2;
	float org3;
	int n_timepoints;
	int t0;
	int t1;
	iim::uint32 width;
	iim::uint32 height;
	iim::uint32 depth;
	int c;
	iim::uint8 *ptr;
	iim::sint64 chan_size;

	int res = 0;
	int tp  = 0;
	int chID = -1; // invalid value: return all channels
	std::string str_value;

	// set the ROI
	z0 = (z0 < 0) ? 0: z0;
	z1 = (z1 < 0) ? img_depth : z1;

	if ( z0 >= z1 )
		throw iom::exception(iom::strprintf("wrong slice indices (z0 = %d, z1 = %d)",z0, z1), __iom__current__function__);

	if ( getParamValue(params,"resolution",str_value) )
		res = atoi(str_value.c_str());
	if ( getParamValue(params,"timepoint",str_value) )
		tp = atoi(str_value.c_str());
	if ( getParamValue(params,"channel",str_value) )
		chID = atoi(str_value.c_str());

	IMS_HDF5init(img_path,file_descr);
	IMS_HDF5getVolumeInfo(file_descr,tp,res,volume_descr,vxl1,vxl2,vxl3,org1,org2,org3,height,width,depth,img_chans,img_bytes_x_chan,n_timepoints,t0,t1);
	if ( chID == -1 ) {
		chan_size = ((iim::sint64)height) * ((iim::sint64)width) * (z1-z0) * img_bytes_x_chan;
		for ( c=0, ptr=data; c<img_chans; c++, ptr+=chan_size ) {
			IMS_HDF5getSubVolume(volume_descr,0,height,0,width,z0,z1,c,ptr);
			//printf("---> [multiple] c=%d z=%d\n",c,z0);
		}
	}
	else {
		IMS_HDF5getSubVolume(volume_descr,0,height,0,width,z0,z1,chID,data);
		//printf("---> [single] c=%d z=%d\n",chID,z0);
	}
	IMS_HDF5closeVolume(volume_descr);
	IMS_HDF5close(file_descr);

	img_width  = width;
	img_height = height;
	img_depth  = depth;

	return (unsigned char *) data;
}


// Write 3D image data into a single (3D) image file
void 
	iomanager::IMS_HDF5::writeData(
	std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
	unsigned char * raw_img,		// (INPUT)	image data to be saved into the file
	int img_height,					// (INPUT)	image height (in pixels)
	int img_width,					// (INPUT)	image width (in pixels)
	int img_depth,					// (INPUT)  image depth (in pixels)
	int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
	int img_chans,					// (INPUT)	number of channels
	int z0,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int z1,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int y0,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int y1,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int x0,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	int x1,							// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, z0 = %d, z1 = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, z0, z1, y0, y1, x0, x1, params.c_str()).c_str(), __iom__current__function__);
}


// Create an empty 3D image 
void 
	iomanager::IMS_HDF5::create3Dimage(
	std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
	int img_height,					// (INPUT)	image height (in pixels)
	int img_width,					// (INPUT)	image width (in pixels)
	int img_depth,					// (INPUT)  image depth (in pixels)
	int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
	int img_chans,					// (INPUT)	number of channels
	const std::string & params)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);

	void *file_descr;

	const char *sPtr;
	int sLen;
	char *src_value;

	// load the metadata (mandatory)
	if ( (sPtr = strstr(params.c_str(),"source_metadata")) == 0 )
		throw iom::exception(iom::strprintf("source metadata parameter missing"), __iom__current__function__);

	sPtr = strstr(sPtr,"=") + 1; // start of source path
	sLen = (int) (strstr(sPtr,",") ? strstr(sPtr,",") - sPtr : strlen(sPtr));
	src_value = new char[sLen + 1];
	strncpy(src_value,sPtr,sLen);
	src_value[sLen] = '\0';

	void *olist;
	void *rootalist;
	if ( strcmp(src_value,"default") == 0 || strcmp(src_value,"null") == 0 ) {
		olist = IMS_HDF5get_olist((void *)0,img_path,img_height,img_width,img_depth,img_chans,1); // assumes 1 timepoint
		rootalist = IMS_HDF5get_rootalist((void *)0);
	}
	else {
		IMS_HDF5init(src_value,file_descr,true);
		olist = IMS_HDF5get_olist(file_descr); // assumes 1 timepoint
		rootalist = IMS_HDF5get_rootalist(file_descr);
		IMS_HDF5close(file_descr);
	}

	delete src_value;

	IMS_HDF5init(img_path,file_descr,false,img_bytes_x_chan,olist);
	olist = (void *) 0;
	
	// set the resolutions
	if ( (sPtr = strstr(params.c_str(),"resolutions")) == 0 ) { // use default
		IMS_HDF5addResolution(0,img_height,img_width,img_depth,img_chans);
	}
	else {
		sPtr = strstr(sPtr,"=") + 1; // start of source path
		sLen = (int) (strstr(sPtr,",") ? strstr(sPtr,",") - sPtr : strlen(sPtr));
		src_value = new char[sLen + 1];
		strncpy(src_value,sPtr,sLen);
		src_value[sLen] = '\0';

		for ( int i=0; i<sLen; i++ ) {
			IMS_HDF5addResolution(file_descr,img_height,img_width,img_depth,img_chans,(src_value[i] - '0'));
		}

		delete src_value;
	}

	IMS_HDF5addTimepoint(file_descr);

	IMS_HDF5close(file_descr); 
}


// Append a single slice at the bottom of a 3D image file
void 
	iomanager::IMS_HDF5::appendSlice(
	std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
	unsigned char * raw_img,		// (INPUT)	slice to be saved into the file
	int img_height,					// (INPUT)	slice height (in pixels)
	int img_width,					// (INPUT)	slice width (in pixels)
	int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
	int img_chans,					// (INPUT)	number of channels
	int y0,							// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
	int y1,							// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
	int x0,							// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
	int x1,							// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
	int slice,						// (INPUT)  slice index 
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_bytes_x_chan = %d, img_chans = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_bytes_x_chan, img_chans, y0, y1, x0, x1, params.c_str()).c_str(), __iom__current__function__);

	y0 = (y0 < 0) ? 0: y0;
	y1 = (y1 < 0) ? img_height : y1;
	x0 = (x0 < 0) ? 0: x0;
	x1 = (x1 < 0) ? img_width  : x1;

	if ( y0 >= y1 || x0 >= x1 )
		throw iom::exception(iom::strprintf("wrong ROI (y0 = %d, y1 = %d, x0 = %d, x1 = %d)", y0, y1, x0, x1), __iom__current__function__);

	if ( !( x0 == 0 && x1 == img_width && y0 == 0 && y1 == img_height) )  // not all buffer must be written
		throw iom::exception(iom::strprintf("ioplugin IMS_HDF5 supports only the append of the whole buffer"), __iom__current__function__);

	const char *sPtr;
	int sLen;
	char *src_value;
	int tp;

	if ( (sPtr = strstr(params.c_str(),"timepoint")) == 0 ) { // use default
		tp = 0;
	}
	else {
		sPtr = strstr(sPtr,"=") + 1; // start of source path
		sLen = (int) (strstr(sPtr,",") ? strstr(sPtr,",") - sPtr : strlen(sPtr));
		src_value = new char[sLen + 1];
		strncpy(src_value,sPtr,sLen);
		src_value[sLen] = '\0';
		tp = atoi(src_value);
		delete src_value;
	}

	void *file_descr;
	sint64 *hyperslab_descr = new sint64[4*3]; // four 3-valued parameters: [ start(offset), stride count(size), block ]
	memset(hyperslab_descr,0,4*3*sizeof(sint64));
	sint64 *buf_dims = new sint64[3];  // dimensions of the buffer in which the subvolume is stored at a given resolution
	memset(buf_dims,0,3*sizeof(sint64));

	buf_dims[1] = img_height; 
	buf_dims[2] = img_width; 
	buf_dims[0] = 1; // append just one slice
	// start
	hyperslab_descr[0] = 0; // [0][0]
	hyperslab_descr[1] = 0; // [0][1]
	hyperslab_descr[2] = 0; // [0][2]
	// stride
	hyperslab_descr[3] = 1;  // [1][0]
	hyperslab_descr[4] = 1;  // [1][1]
	hyperslab_descr[5] = 1;  // [1][2]
	// count
	hyperslab_descr[6] = buf_dims[0]; //height/(powInt(2,i)); // [2][0]
	hyperslab_descr[7] = buf_dims[1]; //width/(powInt(2,i));  // [2][1]
	hyperslab_descr[8] = buf_dims[2]; //z_size/(powInt(2,i)); // [2][2]
	// block
	hyperslab_descr[9]  = 1; // [3][0]
	hyperslab_descr[10] = 1; // [3][1]
	hyperslab_descr[11] = 1; // [3][2]

	IMS_HDF5init(img_path,file_descr,false,img_bytes_x_chan);

	for ( int c=0; c<img_chans; c++ ) {
		IMS_HDF5writeHyperslab(file_descr,raw_img,buf_dims,hyperslab_descr,tp,c);
		raw_img += img_height * img_width * img_bytes_x_chan;
	}

	IMS_HDF5close(file_descr); 
}


// read image data
void
	iomanager::IMS_HDF5::readData(
	char *finName,					// image filepath
	int XSIZE,						// image width (in pixels)
	int YSIZE,						// image height (in pixels)
	unsigned char *data,			// (OUTPUT) image data
	int first,						// selects a range [first, last] of files to be loaded 
	int last,						// selects a range [first, last] of files to be loaded 
	const std::string & params)		// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	throw iom::exception(iom::strprintf("no more available"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("finName = %s, XSIZE = %d, YSIZE = %d, first = %d, last = %d, params = \"%s\"",
		finName, XSIZE, YSIZE, first, last, params.c_str()).c_str(), __iom__current__function__);
}