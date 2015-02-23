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
#include "tiff2D.h"
#include "Tiff3DMngr.h"
#include "VirtualFmtMngr.h"

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_2D(tiff2D)

// insert here your plugin description that will be displayed on the user interface
std::string iomanager::tiff2D::desc()
{
	return	"******************************************************\n"
			"* tiff2D v.1.0                                     *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* 2D image-based I/O plugin that uses the libtiff li- *\n"
			"* brary to read/write 2D image files.                *\n"
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
	iomanager::tiff2D::isChansInterleaved( ) 
{
	return true;
}


// read image metadata from a 2D image file
void 
	iomanager::tiff2D::readMetadata(
	std::string img_path,			// (INPUT)  image filepath
	int & img_width,				// (OUTPUT) image width  (in pixels)
	int & img_height,				// (OUTPUT) image height (in pixels)
	int & img_bytes_x_chan,			// (OUTPUT) number of bytes per channel
	int & img_chans,				// (OUTPUT) number of channels
	const std::string & params)		// (INPUT)  additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = \"%s\", params = \"%s\"",img_path.c_str(), params.c_str()).c_str(), __iom__current__function__);

	uint16 bpp;
	uint16 spp;

	 // disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	TIFF* input=TIFFOpen(img_path.c_str(),"r");
	if (!input)
		throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported format is 2DTIFF", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &img_width))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGEWIDTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &img_height))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGELENGTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_BITSPERSAMPLE' from image \"%s\". ", img_path.c_str()), __iom__current__function__);
	img_bytes_x_chan = bpp/8;

	if (!TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp)) 
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_SAMPLESPERPIXEL' from image \"%s\". ", img_path.c_str()), __iom__current__function__);
	img_chans = spp;

	// Onofri
	int img_depth;			// image depth (in pixels)
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
	
	// check the exception after closing the file
	if ( img_depth > 1 ) 
		throw iom::exception(iom::strprintf("image \"%s\" has more than one page.\nSupported format is 2DTIFF", img_path.c_str()), __iom__current__function__);
}


// Read 2D image data
unsigned char *						// (OUTPUT) a buffer storing the 2D image
	iomanager::tiff2D::readData(
	std::string img_path,			// (INPUT)	image filepath
	int & img_width,				// (INPUT/OUTPUT) image width  (in pixels)
	int & img_height,				// (INPUT/OUTPUT) image height (in pixels)
	int & img_bytes_x_chan,			// (INPUT/OUTPUT) number of bytes per channel
	int & img_chans,				// (INPUT/OUTPUT) number of channels to be read
	unsigned char *data,			// (INPUT) image data
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_width 0 %d, img_height 0 %d, img_bytes_x_chan = %d, img_chans = %d, params = \"%s\"", 
		img_path.c_str(), img_width, img_height, img_bytes_x_chan, img_chans, params.c_str()).c_str(), __iom__current__function__);

	char *err_Tiff3Dfmt;

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	if ( !data ) { // recover the metadata, allocate the buffer and set parameters
		unsigned int _width;
		unsigned int _height;
		unsigned int _depth;
		int _bytes_x_chan;
		unsigned int _chans;
		int b_swap;
		void *fhandle;
		int header_len;

		if ( (err_Tiff3Dfmt = loadTiff3D2Metadata((char *)img_path.c_str(),_width,_height,_depth,_chans,_bytes_x_chan,b_swap,fhandle,header_len)) != 0 ) {
			throw iom::exception(iom::strprintf("(%s) unable to read meta data of tiff file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
		}
		closeTiff3DFile(fhandle);

		data = new unsigned char[_width * _height * _chans * _bytes_x_chan];
		img_width        = _width;
		img_height       = _height;
		img_bytes_x_chan = _bytes_x_chan;
		img_chans        = _chans;
	}

	// load 2D image 
	if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer((char *)img_path.c_str(),data,img_width,img_height,0,0)) != 0 ) {
		throw iom::exception(iom::strprintf("(%s) unable to read tiff file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
	}

	return data;
}


// Write 2D image data into a single (2D) image file
void 
	iomanager::tiff2D::writeData(
	std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
	unsigned char *raw_img,			// (INPUT)	image data to be saved into the file
	int img_height,					// (INPUT)	image height
	int img_width,					// (INPUT)	image width
	int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
	int img_chans,					// (INPUT)	number of channels
	int y0,							// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int y1,							// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int x0,							// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int x1,							// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	//throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_height = %d, img_width = %d, img_bytes_x_chan = %d, img_chans = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, params = \"%s\"", 
		img_path.c_str(), img_height, img_width, img_bytes_x_chan, img_chans, y0, y1, x0, x1, params.c_str()).c_str(), __iom__current__function__);

	// correct default parameters
	y0 = (y0 < 0) ? 0: y0;
	y1 = (y1 < 0) ? img_height : y1;
	x0 = (x0 < 0) ? 0: x0;
	x1 = (x1 < 0) ? img_width  : x1;

	// compute ROI dimensions
	int ROI_height = y1 - y0;
	int ROI_width  = x1 - x0;

	// precondition checks
	if(! (y0>=0 && y1>y0 && y1<=img_height && x0>=0 && x1>x0 && x1<=img_width) )
		throw iom::exception(iom::strprintf("invalid ROI [%d,%d](X) x [%d,%d](Y) on image %d(X) x %d(Y)", x0, x1, y0, y1, img_width, img_height), __iom__current__function__);
	if(img_bytes_x_chan != 1 && img_bytes_x_chan != 2)
		throw iom::exception(iom::strprintf("unsupported bitdepth %d\n", img_bytes_x_chan*8), __iom__current__function__);
	if(img_chans != 1)
		throw iom::exception(iom::strprintf("unsupported number of channels = %d\n. Only single-channel images are supported", img_chans), __iom__current__function__);

	// convert raw data to image data
	double proctime = -TIME(0);

	char *err_Tiff3Dfmt;

	// creates the file (2D image: depth is 1)
	if ( (err_Tiff3Dfmt = initTiff3DFile((char *)img_path.c_str(),ROI_width,ROI_height,1,img_chans,img_bytes_x_chan)) != 0 ) {
		throw iom::exception(iom::strprintf("unable to create tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
	}

	// update conversion time
	if(TIME_CALC)
	{
		proctime += TIME(0);
		time_conversions+=proctime;
		proctime = -TIME(0);
	}

	// save image

	if ( x0 == 0 && x1 == img_width && y0 == 0 && y1 == img_height ) { // all buffer must be written
		if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),0,raw_img,ROI_width,ROI_height)) != 0 ) {
			throw iom::exception(iom::strprintf("unable to save image at \"%s\". Unsupported format or wrong path.\n", img_path.c_str()), __iom__current__function__);
		}
	}
	else { // copy to a sub buffer before writing
		iim::sint64 stridex  = img_width * img_chans * img_bytes_x_chan;
		unsigned char *buf; // to scan the input buffer
		iim::sint64 stridex_ROI    = (x1-x0) * img_chans * img_bytes_x_chan;
		unsigned char *raw_img_ROI = new unsigned char[(y1-y0) * (x1-x0) * img_chans * img_bytes_x_chan];

		buf = raw_img + x0*stridex + y0*img_chans*img_bytes_x_chan; // buf points to the first byte to be written

		iim::VirtualFmtMngr::copyBlock2SubBuf(buf,raw_img_ROI,(y1-y0),(x1-x0),1,img_bytes_x_chan,stridex,0,stridex_ROI,0); // xy strides are 0 since the buffer is 2D

		if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),0,raw_img_ROI,(x1-x0),(y1-y0))) != 0 ) {
			throw iom::exception(iom::strprintf("(%s) unable to write 2D image into file %s",err_Tiff3Dfmt,img_path.c_str()), __iom__current__function__);
		}

		delete []raw_img_ROI;
	}

	// update IO time
	if(TIME_CALC)
	{
		proctime += TIME(0);
		time_IO+=proctime;
	}
}


// read 3D image data from a stack of (2D) image files
iom::real_t*						// (OUTPUT) a [0.0,1.0]-valued array storing the 3D image in channel->slice->row order
	iomanager::tiff2D::readData(
	char **files,					// (INPUT)	array of C-strings storing image filenames
	int files_size,					// (INPUT)	size of 'files'
	const char *path /*= 0*/,		// (INPUT)	path to be concatenated to the i-th entry of 'files'
	int first /*= -1*/,				// (INPUT)	selects a range [first, last] of files to be loaded
	int last /*= -1*/,				// (INPUT)	selects a range [first, last] of files to be loaded	
	bool is_sparse /*= false*/,		// (INPUT)	if true, 'files' is a sparse array and null entries should be treated as empty (black) images
	iom::channel chan,				// (INPUT)	channel selection { ALL, R, G, B }. 
	const std::string & params)		// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	throw iom::exception(iom::strprintf("no more available"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("files_size = %d, path = %s, first = %d, last = %d, is_sparse = %s, chan = %d, params = \"%s\"",
		files_size, path ? path : "null", first, last, is_sparse ? "true" : "false", chan, params.c_str()).c_str(), __iom__current__function__);

	char *err_Tiff3Dfmt;

	unsigned int rows;
	unsigned int cols;
	unsigned int n_slices;
	unsigned int channels;
	int depth;
	int swap;
	void *dummy;
	int dummy_len;

	unsigned char *data;
 	iom::real_t *raw_data;
 
	iom::real_t *image_stack = 0;
	uint64 image_stack_width=0, image_stack_height=0, z=0;
	
	// check for valid file list
	if(!files || files_size <= 0)
		throw iom::exception("invalid file list (null pointer or 0-sized)", __iom__current__function__);

	// check and adjust file selection
	first = (first == -1 ? 0 : first);
	last  = (last  == -1 ? files_size - 1 : last );
	first = min(first, files_size-1);
	last = min(last, files_size-1);

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	// build absolute image path of first file in the stack
	int i = 0;
	while ( is_sparse && i<files_size && files[i]==0 ) // look for existing file
		i++;
	if ( i == files_size )
		throw iom::exception("stack is entirely empty: cannot recover slice dimension", __iom__current__function__);
	std::string image_path = path ? std::string(path) + "/" + files[i] : files[i];
	
	// load metadata
	//iomanager::tiff2D::readMetadata(image_path,cols,rows,depth,channels,params);
	if ( (err_Tiff3Dfmt = loadTiff3D2Metadata((char *)image_path.c_str(),cols,rows,n_slices,channels,depth,swap,dummy,dummy_len)) != 0 ) {
		throw iom::exception(iom::strprintf("unable to read tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
	}
	closeTiff3DFile(dummy);
	
	// allocate buffer
	data = new unsigned char[cols * rows * depth * channels];

	// loop over files
	for(int file_i = first; file_i <= last; file_i++, z++)
	{
		// skip missing slices if stack is sparse
		if(is_sparse && !files[file_i])
			continue;

		// if stack is not sparse, a missing slice must throw an iom::exception
		if(!files[file_i])
			throw iom::exception("invalid slice filename in non-sparse tile", __iom__current__function__);

		// build absolute image path
		std::string image_path = path ? std::string(path) + "/" + files[file_i] : files[file_i];

		// load 2D image 
		if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer((char *)image_path.c_str(),data,cols,rows,0,0)) != 0 ) {
			throw iom::exception(iom::strprintf("unable to read tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
		}

		double proctime = -TIME(0);
// 		if(chan == iom::ALL)
// 			image = cv::imread(image_path, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);   // pack all channels into grayscale
// 		else
// 			image = cv::imread(image_path, CV_LOAD_IMAGE_ANYCOLOR);                             // load individual channels
// 		if(!image.data)
// 			throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", image_path.c_str()), __iom__current__function__);

		// select channel (if requested)
// 		if(image.channels() == 3 && chan != iom::ALL)
// 		{
// 			cv::Mat imageChannels[3];
// 			cv::split(image, imageChannels);
// 			image = imageChannels[3-CHANS]; // OpenCV uses BGR !
// 		}

		// update time
		if(TIME_CALC)
		{
			proctime += TIME(0);
			time_IO+=proctime;
			proctime = -TIME(0);
		}

		// initialize output data and image dimensions
		if(!image_stack)
		{
			image_stack_height = (uint64) rows;
			image_stack_width  = (uint64) cols;
			uint64 image_stack_size = image_stack_width * image_stack_height * (last-first+1);
			image_stack = new iom::real_t[image_stack_size];
			for(uint64 j=0; j < image_stack_size; j++)
				image_stack[j] = 0;		// default is 0 (black)
		}
		else if(image_stack_width != (uint64)cols || image_stack_height != (uint64)rows)
			throw iom::exception("images in stack have not the same dimensions", __iom__current__function__);

// 		iom::real_t *raw_data = &image_stack[z*image.rows*image.cols];
// 		if(image.depth == CV_8U)
// 		{
// 			for(int i=0; i<image.rows; i++)
// 			{
// 				uint8* img_data = image.ptr<uint8>(i);
// 				for(int j=0; j<image.cols; j++, raw_data++)
// 					*raw_data = img_data[j]/255.0f;
// 			}
// 		}
// 		else if (image.depth() == CV_16U)
// 		{
// 			for(int i=0; i<image.rows; i++)
// 			{
// 				uint16* img_data = image.ptr<uint16>(i);
// 				for(int j=0; j<image.cols; j++, raw_data++)
// 					*raw_data = img_data[j]/65535.0f;
// 			}
// 		}
// 		else
// 			throw iom::exception("unsupported image depth", __iom__current__function__);

		int offset;

		// convert image to [0.0,1.0]-valued array
		if ( channels == 1 ) {
			raw_data = &image_stack[z*rows*cols];
			if ( depth == 1 ) {
				for(int i = 0; i < (rows * cols); i++)
					raw_data[i] = (iom::real_t) data[i]/255.0f;
			}
			else { // depth == 2
				for(int i = 0; i < (rows * cols); i++)
					raw_data[i] = (iom::real_t) ((uint16 *)data)[i]/65535.0f; // data must be interpreted as a uint16 array
			}
		}
		else { // conversion to an intensity image
			if ( chan == iom::ALL ) {
				throw iom::exception("conversion from multi-channel to intensity images not supported.", __iom__current__function__);
			}
			else if ( chan == iom::R ) {
				offset = 0;
			}
			else if ( chan == iom::G ) {
				offset = 1;
			}
			else if ( chan == iom::B ) {
				offset = 2;
			}
			else {
				throw iom::exception("wrong value for parameter iom::CHANNEL_SELECTION.", __iom__current__function__);
			}
			raw_data = &image_stack[z*rows*cols];
			if ( depth == 1 )
				for(int i = 0; i < (rows * cols); i++)
					raw_data[i] = (iom::real_t) data[3*i + offset]/255.0f;
			else // depth == 2
				for(int i = 0; i < (rows * cols); i++)
					raw_data[i] = (iom::real_t) ((uint16 *)data)[3*i + offset]/65535.0f; // data must be interpreted as a uint16 array
		}

		// update time
		if(TIME_CALC)
		{
			proctime += TIME(0);
			time_conversions+=proctime;
		}
	}
	
	delete []data;

	// cannot load any data from this stack
	if(!image_stack)
		throw iom::exception(iom::strprintf("stack (%s) does not contain loadable data in [%d,%d]", path ? path : "0", first, last), __iom__current__function__);

	return image_stack;
}


// write 2D image data into a single (2D) image file
void 
	iomanager::tiff2D::writeData(
	std::string img_path,		// (INPUT)	image filepath (it includes the file extension)
	iom::real_t* raw_img,			// (INPUT)	a [0.0,1.0]-valued array storing the 2D image in channel->row order
	int img_height,				// (INPUT)	image height
	int img_width,				// (INPUT)	image width
	int img_chans,				// (INPUT)	number of channels
	int y0,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int y1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int x0,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int x1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
	int bpp,					// (INPUT)	color depth (bits per pixel)
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	throw iom::exception(iom::strprintf("no more available"), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_height = %d, img_width = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, bpp = %d, params = \"%s\"", 
		img_path.c_str(), img_height, img_width, y0, y1, x0, x1, bpp, params.c_str()).c_str(), __iom__current__function__);


	// correct default parameters
	y1 = (y1 == -1 ? img_height - 1 : y1);
	x1 = (x1 == -1 ? img_width  - 1 : x1 );

	// compute ROI dimensions
	int ROI_height = y1 - y0 + 1;
	int ROI_width  = x1 - x0 + 1;

	// precondition checks
	if(! (y0>=0 && y1>y0 && y1<img_height && x0>=0 && x1>x0 && x1<img_width) )
		throw iom::exception(iom::strprintf("invalid ROI [%d,%d](X) x [%d,%d](Y) on image %d(X) x %d(Y)", x0, x1, y0, y1, img_width, img_height), __iom__current__function__);
	if(bpp != 8 && bpp != 16)
		throw iom::exception(iom::strprintf("unsupported bitdepth %d\n", bpp), __iom__current__function__);
	if(img_chans != 1)
		throw iom::exception(iom::strprintf("unsupported number of channels = %d\n. Only single-channel images are supported", img_chans), __iom__current__function__);

	// convert raw data to image data
	double proctime = -TIME(0);

	char *err_Tiff3Dfmt;

	// creates the file (2D image: depth is 1)
	if ( (err_Tiff3Dfmt = initTiff3DFile((char *)img_path.c_str(),ROI_width,ROI_height,1,img_chans,bpp/8)) != 0 ) {
		throw iom::exception(iom::strprintf("unable to create tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
	}

	unsigned char *buffer = new unsigned char[ROI_height * ROI_width * img_chans * (bpp/8)];

	if(bpp == 8)
	{
		for(int i = 0; i <ROI_height; i++)
		{
			uint8* img_data = buffer + i*ROI_width*img_chans;
			for(int j = 0; j < ROI_width; j++)
				img_data[j] = static_cast<uint8>(raw_img[(i+y0)*img_width+j+x0] * 255.0f + 0.5f);
		}
	}
	else // bpp == 16
	{
		for(int i = 0; i <ROI_height; i++)
		{
			uint16* img_data = ((uint16 *)buffer) + i*ROI_width*img_chans; // the cast to uint16* guarantees the right offset
			for(int j = 0; j < ROI_width; j++)
				img_data[j] = static_cast<uint16>(raw_img[(i+y0)*img_width+j+x0] * 65535.0f + 0.5f);
		}
	}

	// update conversion time
	if(TIME_CALC)
	{
		proctime += TIME(0);
		time_conversions+=proctime;
		proctime = -TIME(0);
	}

	// save image
	if ( (err_Tiff3Dfmt = appendSlice2Tiff3DFile((char *)img_path.c_str(),0,buffer,ROI_width,ROI_height)) != 0 ) {
		throw iom::exception(iom::strprintf("unable to save image at \"%s\". Unsupported format or wrong path.\n", img_path.c_str()), __iom__current__function__);
	}

	delete []buffer;

	// update IO time
	if(TIME_CALC)
	{
		proctime += TIME(0);
		time_IO+=proctime;
	}
}