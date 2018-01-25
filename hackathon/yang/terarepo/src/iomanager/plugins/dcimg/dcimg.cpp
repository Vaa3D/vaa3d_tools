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
* 2017-07-11. Giulio.     @CREATED
*/

#define _FILE_OFFSET_BITS 64

#include <cstddef>			// 2014-09-18. Alessandro. @FIXED compilation issue on gcc compilers.
#include "dcimg.h"
#include "IM_config.h"

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_3D(dcimg)


// insert here your plugin description that will be displayed on the user interface
std::string iomanager::dcimg::desc()
{
	return	"******************************************************\n"
			"* Hamamatsu DCIMG v.1.0                              *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* 3D image-based I/O plugin that implements          *\n"
			"* the format used by Hamamtsu cameras                *\n"
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
	iomanager::dcimg::isChansInterleaved( ) 
{
	return false;
}


// read image metadata
void 
	iomanager::dcimg::readMetadata(
	std::string img_path,		// (INPUT)	image filepath
	int & img_width,			// (OUTPUT) image width  (in pixels)
	int & img_height,			// (OUTPUT) image height (in pixels)
	int & img_depth,			// (OUTPUT) image depth (in pixels)
	int & img_bytes_x_chan,	    // (OUTPUT) number of bytes per channel
	int & img_chans,			// (OUTPUT) number of channels
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);

	unsigned char dummy[256];
	char file_format[9];
	iim::uint32 format_version;
	iim::uint32 nsess;
	iim::uint32 nfrms;
	iim::uint32 header_size;
	//iim::uint64 file_size;
	//iim::uint64 file_size2;

	iim::uint64 session_size;
	iim::uint32 nfrms2;
	iim::uint32 byte_depth;
	iim::uint32 xsize;
	iim::uint32 byte_per_row;
	iim::uint32 ysize;
	//iim::uint32 byte_per_img;
	//iim::uint32 header_size2;
	//iim::uint64 session_data_size;

	FILE *fin;

	if ( (fin = fopen(img_path.c_str(),"rb")) == NULL ) 
		throw iom::exception(iom::strprintf("cannot open file %s",img_path.c_str()), __iom__current__function__);

	fread(file_format,sizeof(char),8,fin);
	file_format[8] = '\0';
	if ( strcmp(file_format,"DCIMG") != 0 )
		throw iom::exception(iom::strprintf("invalid DCIMG file (%s)",file_format), __iom__current__function__);

	fread(&format_version,sizeof(iim::uint32),1,fin);
	fread(dummy,sizeof(iim::uint32),5,fin);
	fread(&nsess,sizeof(iim::uint32),1,fin);
	fread(&nfrms,sizeof(iim::uint32),1,fin);
	fread(&header_size,sizeof(iim::uint32),1,fin);
	//fread(dummy,sizeof(iim::uint32),1,fin);
	//fread(&file_size,sizeof(iim::uint64),1,fin);
	//fread(dummy,sizeof(iim::uint32),2,fin);
	//fread(&file_size2,sizeof(iim::uint64),1,fin);

	fseek(fin,header_size,SEEK_SET);

	fread(&session_size,sizeof(iim::uint64),1,fin);
	fread(dummy,sizeof(iim::uint32),6,fin);
	fread(&nfrms2,sizeof(iim::uint32),1,fin);
	fread(&byte_depth,sizeof(iim::uint32),1,fin);
	fread(dummy,sizeof(iim::uint32),1,fin);
	fread(&xsize,sizeof(iim::uint32),1,fin);
	fread(&byte_per_row,sizeof(iim::uint32),1,fin);
	fread(&ysize,sizeof(iim::uint32),1,fin);
	//fread(&byte_per_img,sizeof(iim::uint32),1,fin);
	//fread(dummy,sizeof(iim::uint32),2,fin);
	//fread(&header_size2,sizeof(iim::uint32),1,fin);
	//fread(&session_data_size,sizeof(iim::uint64),1,fin);

	img_width        = ysize;
	img_height       = xsize;
	img_depth        = nfrms;
	img_bytes_x_chan = byte_depth;   
	img_chans        = 1;
	
	fclose(fin);		
}


// Read 3D image data
unsigned char *						// (OUTPUT) buffer containing the read image data
	iomanager::dcimg::readData(
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

//printf("--> params = \"%s\"\n",params.c_str());
	int chID = -1; // invalid value: return all channels
	std::string str_value;

	// set the ROI
	z0 = (z0 < 0) ? 0: z0;
	z1 = (z1 < 0) ? img_depth : z1;

	unsigned int _width;
	unsigned int _height;
	unsigned int _depth;
	int _bytes_x_chan;

	FILE *fin;

	if ( (fin = fopen(img_path.c_str(),"rb")) == NULL ) 
		throw iom::exception(iom::strprintf("cannot open file %s",img_path.c_str()), __iom__current__function__);

	if ( !data ) { // recover the metadata, allocate the buffer and set parameters

		iim::uint32 header_size;
		iim::uint32 dummy;

		fseek(fin,36,SEEK_SET); // jump to nfrms field of the header

		fread(&_depth,sizeof(iim::uint32),1,fin);
		fread(&header_size,sizeof(iim::uint32),1,fin);

		fseek(fin,header_size,SEEK_SET);

		fread(&_bytes_x_chan,sizeof(iim::uint32),1,fin);
		fread(&dummy,sizeof(iim::uint32),1,fin);
		fread(&_width,sizeof(iim::uint32),1,fin);
		fread(&dummy,sizeof(iim::uint32),1,fin);
		fread(&_height,sizeof(iim::uint32),1,fin);

		data = new unsigned char[((sint64)_width) * ((sint64)_height) * ((sint64)_depth) * _bytes_x_chan];
		img_width        = _width;
		img_height       = _height;
		img_depth        = _depth;
		img_bytes_x_chan = _bytes_x_chan;
	}

	if ( z0 >= z1 )
		throw iom::exception(iom::strprintf("wrong slice indices (z0 = %d, z1 = %d)",z0, z1), __iom__current__function__);

	iim::uint64 V0 = 0;
	iim::uint64 V1 = img_height;
	iim::uint64 H0 = 0;
	iim::uint64 H1 = img_width;

	if ( getParamValue(params,"V0",str_value) )
		V0 = (iim::uint64) atoi(str_value.c_str());
	if ( getParamValue(params,"V1",str_value) )
		V1 = (iim::uint64) atoi(str_value.c_str());
	if ( getParamValue(params,"H0",str_value) )
		H0 = (iim::uint64) atoi(str_value.c_str());
	if ( getParamValue(params,"H1",str_value) )
		H1 = (iim::uint64) atoi(str_value.c_str());

	iim::uint64 file_offset = (iim::uint64) 232 + (iim::uint64)img_width * (iim::uint64)img_height  * (iim::uint64)z0 * (iim::uint64)img_bytes_x_chan;

	iim::uint64 size;

	if ( V0 == 0 && V1 == img_height && H0 == 0 && H1 == img_width ) { // load the whole substack
		size = ((iim::uint64)img_width * (iim::uint64)img_height * (iim::uint64)img_bytes_x_chan) * (iim::uint64)(z1 - z0);
#ifdef WIN32
		_fseeki64(fin,file_offset*sizeof(unsigned char),SEEK_SET);
#else
		fseeko(fin,file_offset*sizeof(unsigned char),SEEK_SET);
#endif

		fread(data,sizeof(unsigned char),size,fin);
	}
	else { // load selected data row by row
		iim::uint64 rowStride = (iim::uint64)img_width * (iim::uint64)img_bytes_x_chan;
		iim::uint64 rowSize = (iim::uint64)(img_width - H0) * (iim::uint64)img_bytes_x_chan;
		iim::uint64 sliceStride = (iim::uint64)img_height * rowStride;
//printf("--> rowStride = %lld, sliceStride = %lld\n",rowStride,sliceStride);

		unsigned char *pSlice = data + ((iim::uint64)img_width * V0 + H0) * (iim::uint64)img_bytes_x_chan;
		iim::uint64 sliceOffs = file_offset + ((iim::uint64)img_width * V0 + H0) * (iim::uint64)img_bytes_x_chan;
		for ( int z=0; z<(z1-z0); z++, pSlice+=sliceStride, sliceOffs+=sliceStride ) {
			unsigned char *pRow = pSlice;
			iim::uint64 rowOffs = sliceOffs;
			for ( int v=0; v<(V1-V0); v++, pRow+=rowStride, rowOffs+=rowStride ) {
#ifdef WIN32
				_fseeki64(fin,rowOffs*sizeof(unsigned char),SEEK_SET);
#else
//printf("--> v = %d, rowOffs = %lld\n",v,rowOffs);
				fseeko(fin,rowOffs*sizeof(unsigned char),SEEK_SET);
#endif
//printf("--> pRow - data = %ld, rowSize = %lld\n",pRow-data,rowStride);
				fread(pRow,sizeof(unsigned char),rowSize,fin);
			}
		}
	}
//printf("--> after loop\n");

	fclose(fin);
			
	return (unsigned char *) data;
}


// Write 3D image data into a single (3D) image file
void 
	iomanager::dcimg::writeData(
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
	iomanager::dcimg::create3Dimage(
	std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
	int img_height,					// (INPUT)	image height (in pixels)
	int img_width,					// (INPUT)	image width (in pixels)
	int img_depth,					// (INPUT)  image depth (in pixels)
	int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
	int img_chans,					// (INPUT)	number of channels
	const std::string & params)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);
}


// Append a single slice at the bottom of a 3D image file
void 
	iomanager::dcimg::appendSlice(
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
		throw iom::exception(iom::strprintf("ioplugin dcimg supports only the append of the whole buffer"), __iom__current__function__);
}


// read image data
void
	iomanager::dcimg::readData(
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