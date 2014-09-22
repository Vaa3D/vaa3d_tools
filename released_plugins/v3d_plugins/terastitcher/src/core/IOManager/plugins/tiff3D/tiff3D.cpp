#include <cstddef>			// 2014-09-18. Alessandro. @FIXED compilation issue on gcc compilers.
#include <tiffio.h>
#include "tiff3D.h"
#include "Tiff3DMngr.h"

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

// read image metadata
void 
	iomanager::tiff3D::readMetadata(
	std::string img_path,		// (INPUT)	image filepath
	int & img_width,			// (OUTPUT) image width  (in pixels)
	int & img_height,			// (OUTPUT) image height (in pixels)
	int & img_depth,			// (OUTPUT) image depth (in pixels)
	int & img_bytes_x_chans,	// (OUTPUT) number of bytes per channel
	int & img_chans,			// (OUTPUT) number of channels
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %d, params = \"%s\"",img_path.c_str(), params.c_str()).c_str(), __iom__current__function__);

	 // disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	TIFF* input=TIFFOpen(img_path.c_str(),"r");
	if (!input)
		throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported format is 3DTIFF", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &img_width))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGEWIDTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &img_height))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_IMAGELENGTH' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &img_bytes_x_chans))
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_BITSPERSAMPLE' from image \"%s\". ", img_path.c_str()), __iom__current__function__);
	img_bytes_x_chans /= 8;

	if (!TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &img_chans)) 
		throw iom::exception(iom::strprintf("unable to determine 'TIFFTAG_SAMPLESPERPIXEL' from image \"%s\". ", img_path.c_str()), __iom__current__function__);

	img_depth = 0;
	do {
		img_depth++;
	} while (TIFFReadDirectory(input));
	TIFFClose(input);
}

// read image data
void
	iomanager::tiff3D::readData(
	char *finName,				// image filepath
	int XSIZE,					// image width (in pixels)
	int YSIZE,					// image height (in pixels)
	unsigned char *data,		// (OUTPUT) image data
	int first,					// selects a range [first, last] of files to be loaded 
	int last,					// selects a range [first, last] of files to be loaded 
	const std::string & params)	// (INPUT)	additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("finName = %s, XSIZE = %d, YSIZE = %d, first = %d, last = %d, params = \"%s\"",
		finName, XSIZE, YSIZE, first, last, params.c_str()).c_str(), __iom__current__function__);

	//disable warning handler to avoid messages on unrecognized tags
	TIFFSetWarningHandler(0);

	char *err_Tiff3Dfmt;
	if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer(finName,data,XSIZE,YSIZE,first,last)) != 0 ) {
		throw iom::exception(iom::strprintf("unable to read tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
	}
}