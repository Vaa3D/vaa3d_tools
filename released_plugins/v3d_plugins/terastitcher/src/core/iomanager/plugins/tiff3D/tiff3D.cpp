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
* 2015-01-02. Giulio.     @IMPLEMENTED new plugins interface
*/

#include <cstddef>			// 2014-09-18. Alessandro. @FIXED compilation issue on gcc compilers.
#include <tiffio.h>
#include "tiff3D.h"
#include "Tiff3DMngr.h"
#include "VirtualFmtMngr.h"

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_3D(tiff3D)

// insert here your plugin description that will be displayed on the user interface
std::string iomanager::tiff3D::desc()
{
	return	"******************************************************\n"
			"* tiff3D v.1.0                                       *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* 3D image-based I/O plugin that uses the libtiff    *\n"
			"* library to read/write multipage tiffs.             *\n"
			"*                                                    *\n"
			"* Supported image format extentions:                 *\n"
			"*  - TIFF                                            *\n"
			"*  - TIF                                             *\n"
			"*                                                    *\n"
			"* Accepted configuration parameters:                 *\n"
			"*  - none                                            *\n"
			"*                                                    *\n"
			"******************************************************\n";
}


// Return if channels are interleaved (in case the image has just one channel return value is indefinite)
bool 
	iomanager::tiff3D::isChansInterleaved( ) 
{
	return true;
}


// read image metadata
void 
	iomanager::tiff3D::readMetadata(
	std::string img_path,		// (INPUT)	image filepath
	int & img_width,			// (OUTPUT) image width  (in pixels)
	int & img_height,			// (OUTPUT) image height (in pixels)
	int & img_depth,			// (OUTPUT) image depth (in pixels)
	int & img_bytes_x_chan,	// (OUTPUT) number of bytes per channel
	int & img_chans,			// (OUTPUT) number of channels
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);

	// disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	TIFF* input=TIFFOpen(img_path.c_str(),"r");
	if (!input)
		throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported format is 3DTIFF", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &img_width))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGEWIDTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &img_height))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGELENGTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &img_bytes_x_chan))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_BITSPERSAMPLE' from image \"%s\". ", img_path.c_str()), __iom__current__function__);
	img_bytes_x_chan /= 8;

	if (!TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &img_chans)) 
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_SAMPLESPERPIXEL' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	// Onofri
	uint16 cpage;  // Current page. We do not actually need it.
	uint16 npages; // Number of pages. 
	int PNcheck=TIFFGetField(input, TIFFTAG_PAGENUMBER, &cpage, &npages);
	if (!PNcheck || npages==0) { // the tag has not been read correctly
		iom::warning(iom::strprintf("unable to determine 'TIFFTAG_PAGENUMBER' from image file \"%s\". ", img_path.c_str()).c_str(),__iom__current__function__);
		img_depth = 0;
		do {
			img_depth++;
		} while (TIFFReadDirectory(input));
	}
	else
		img_depth = npages;

	TIFFClose(input);
}


// Read 3D image data
unsigned char *						// (OUTPUT) buffer containing the read image data
	iomanager::tiff3D::readData(
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

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	unsigned int _width;
	unsigned int _height;
	unsigned int _depth;
	int _bytes_x_chan;
	unsigned int _chans;
	int b_swap;
	void *fhandle;
	int header_len;
	char *err_Tiff3Dfmt;

	if ( !data ) { // recover the metadata, allocate the buffer and set parameters
		if ( (err_Tiff3Dfmt = loadTiff3D2Metadata((char *)img_path.c_str(),_width,_height,_depth,_chans,_bytes_x_chan,b_swap,fhandle,header_len)) != 0 ) {
			throw iom::exception(iom::strprintf("(%s) unable to read meta data of tiff file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
		}
		closeTiff3DFile(fhandle);

		data = new unsigned char[_width * _height * _depth * _chans * _bytes_x_chan];
		img_width        = _width;
		img_height       = _height;
		img_depth        = _depth;
		img_bytes_x_chan = _bytes_x_chan;
		img_chans        = _chans;
	}

	// set the ROI
	z0 = (z0 < 0) ? 0: z0;
	z1 = (z1 < 0) ? img_depth  : z1;

	if ( z0 >= z1 )
		throw iom::exception(iom::strprintf("wrong slice indices (z0 = %d, z1 = %d)",z0, z1), __iom__current__function__);

	// get the image
	if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer((char *)img_path.c_str(),data,img_width,img_height,z0,z1-1)) != 0 ) {
		throw iom::exception(iom::strprintf("(%s) unable to read tiff file %s in page range [%d,%d]",err_Tiff3Dfmt,img_path.c_str(),z0,z1-1), __iom__current__function__);
	}

	return data;
}


// Write 3D image data into a single (3D) image file
void 
	iomanager::tiff3D::writeData(
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
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_depth = %d, img_bytes_x_chan = %d, img_chans = %d, z0 = %d, z1 = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_depth,	img_bytes_x_chan, img_chans, z0, z1, y0, y1, x0, x1, params.c_str()).c_str(), __iom__current__function__);

	iim::sint64 stridex  = img_width * img_chans * img_bytes_x_chan;
	iim::sint64 stridexy = stridex * img_height;
	unsigned char *buf; // to scan the input buffer

	char *err_Tiff3Dfmt;

	z0 = (z0 < 0) ? 0: z0;
	z1 = (z1 < 0) ? img_depth  : z1;
	y0 = (y0 < 0) ? 0: y0;
	y1 = (y1 < 0) ? img_height : y1;
	x0 = (x0 < 0) ? 0: x0;
	x1 = (x1 < 0) ? img_width  : x1;

	if ( z0 >= z1 || y0 >= y1 || x0 >= x1 )
		throw iom::exception(iom::strprintf("wrong ROI (z0 = %d, z1 = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d)",z0, z1, y0, y1, x0, x1), __iom__current__function__);

	// creates an empty file
	if ( (err_Tiff3Dfmt = initTiff3DFile((char *)img_path.c_str(),(x1-x0),(y1-y0),(z1-z0),img_chans,img_bytes_x_chan)) != 0 ) {
		throw iom::exception(iom::strprintf("(%s) unable to create an empty tiff file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
	}

	// append slice by slice

	buf = raw_img + z0*stridexy + x0*stridex + y0*img_chans*img_bytes_x_chan; // buf points to the first byte to be written

	if ( x0 == 0 && x1 == img_width && y0 == 0 && y1 == img_height ) { // all buffer must be written
		for ( int i=0; i<(z1-z0); i++, buf += stridexy ) {
			if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),i,buf,img_width,img_height)) != 0 ) {
				throw iom::exception(iom::strprintf("(%s) unable to write slice %d into file %s",err_Tiff3Dfmt,z0+i,img_path.c_str()), __iom__current__function__);
			}
		}
	}
	else { // copy to a sub buffer before writing
		iim::sint64 stridex_ROI    = (x1-x0) * img_chans * img_bytes_x_chan;
		iim::sint64 stridexy_ROI   = stridex_ROI * (y1-y0);
		unsigned char *raw_img_ROI = new unsigned char[(z1-z0) * (y1-y0) * (x1-x0) * img_chans * img_bytes_x_chan];
		unsigned char *buf_ROI;

		iim::VirtualFmtMngr::copyBlock2SubBuf(buf,raw_img_ROI,(y1-y0),(x1-x0),(z1-z0),img_bytes_x_chan,stridex,stridexy,stridex_ROI,stridexy_ROI);

		buf_ROI = raw_img_ROI;
		for ( int i=0; i<(z1-z0); i++, buf_ROI += stridexy_ROI ) {
			if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),i,buf_ROI,(x1-x0),(y1-y0))) != 0 ) {
				throw iom::exception(iom::strprintf("(%s) unable to write slice %d into file %s",err_Tiff3Dfmt,z0+i,img_path.c_str()), __iom__current__function__);
			}
		}
		delete []raw_img_ROI;
	}
}


// Create an empty 3D image 
void 
	iomanager::tiff3D::create3Dimage(
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

	char *err_Tiff3Dfmt;

	// creates an empty file
	if ( (err_Tiff3Dfmt = initTiff3DFile((char *)img_path.c_str(),img_width,img_height,img_depth,img_chans,img_bytes_x_chan)) != 0 ) {
		throw iom::exception(iom::strprintf("(%s) unable to create an empty tiff file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
	}
}


// Append a single slice at the bottom of a 3D image file
void 
	iomanager::tiff3D::appendSlice(
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
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width = %d, img_height = %d, img_bytes_x_chan = %d, img_chans = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, params = \"%s\"",
		img_path.c_str(), img_width, img_height, img_bytes_x_chan, img_chans, y0, y1, x0, x1, params.c_str()).c_str(), __iom__current__function__);

	iim::sint64 stridex  = img_width * img_chans * img_bytes_x_chan;
	unsigned char *buf; // to scan the input buffer

	char *err_Tiff3Dfmt;

	y0 = (y0 < 0) ? 0: y0;
	y1 = (y1 < 0) ? img_height : y1;
	x0 = (x0 < 0) ? 0: x0;
	x1 = (x1 < 0) ? img_width  : x1;

	if ( y0 >= y1 || x0 >= x1 )
		throw iom::exception(iom::strprintf("wrong ROI (y0 = %d, y1 = %d, x0 = %d, x1 = %d)", y0, y1, x0, x1), __iom__current__function__);

	// append a slice

	buf = raw_img + x0*stridex + y0*img_chans*img_bytes_x_chan; // buf points to the first byte to be written

	if ( x0 == 0 && x1 == img_width && y0 == 0 && y1 == img_height ) { // all buffer must be written
		if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),slice,buf,img_width,img_height)) != 0 ) {
			throw iom::exception(iom::strprintf("(%s) unable to write slice %d into file %s",err_Tiff3Dfmt,slice,img_path.c_str()), __iom__current__function__);
		}
	}
	else { // copy to a sub buffer before writing
		iim::sint64 stridex_ROI    = (x1-x0) * img_chans * img_bytes_x_chan;
		iim::sint64 stridexy_ROI   = stridex_ROI * (y1-y0); // just because required by 'copyBlock2SubBuf', not actually used
		iim::sint64 stridexy       = stridex * img_height; // just because required by 'copyBlock2SubBuf', not actually used
		unsigned char *raw_img_ROI = new unsigned char[(y1-y0) * (x1-x0) * img_chans * img_bytes_x_chan];

		iim::VirtualFmtMngr::copyBlock2SubBuf(buf,raw_img_ROI,(y1-y0),(x1-x0),1,img_bytes_x_chan,stridex,stridexy,stridex_ROI,stridexy_ROI);

		if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),slice,raw_img_ROI,(x1-x0),(y1-y0))) != 0 ) {
			throw iom::exception(iom::strprintf("(%s) unable to write slice %d into file %s",err_Tiff3Dfmt,slice,img_path.c_str()), __iom__current__function__);
		}
		delete []raw_img_ROI;
	}
}


// read image data
void
	iomanager::tiff3D::readData(
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

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	char *err_Tiff3Dfmt;
	if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer(finName,data,XSIZE,YSIZE,first,last)) != 0 ) {
		throw iom::exception(iom::strprintf("(%s) unable to read tiff file %s in page range [%d,%d]",err_Tiff3Dfmt,finName,first,last), __iom__current__function__);
	}
}