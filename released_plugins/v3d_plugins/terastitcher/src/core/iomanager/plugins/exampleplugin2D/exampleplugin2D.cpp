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

#include "exampleplugin2D.h"

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_2D(exampleplugin2D)

// insert here your plugin description that will be displayed on the user interface
std::string iomanager::exampleplugin2D::desc()
{
	return	"******************************************************\n"
			"* exampleplugin2D v.1.0                              *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* A dummy 2D image-based I/O plugin to demonstrate   *\n"
			"* the extensibility of TeraStitcher I/O.             *\n"
			"*                                                    *\n"
			"* Supported image formats:                           *\n"
			"*  - none                                            *\n"
			"*                                                    *\n"
			"* Accepted configuration parameters:                 *\n"
			"*  - astring:   example parameter to pass a string   *\n"
			"*  - aninteger: example parameter to pass an integer *\n"
			"*                                                    *\n"
			"******************************************************\n";
}


// Return if channels are interleaved (in case the image has just one channel return value is indefinite)
bool 
	iomanager::exampleplugin2D::isChansInterleaved( ) 
{
	return true;
}


// read image metadata from a 2D image file
void 
	iomanager::exampleplugin2D::readMetadata(
	std::string img_path,			// (INPUT)  image filepath
	int & img_width,				// (OUTPUT) image width  (in pixels)
	int & img_height,				// (OUTPUT) image height (in pixels)
	int & img_bytes_x_chan,			// (OUTPUT) number of bytes per channel
	int & img_chans,				// (OUTPUT) number of channels
	const std::string & params)		// (INPUT)  additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = \"%s\", params = \"%s\"",img_path.c_str(), params.c_str()).c_str(), __iom__current__function__);

	// parse plugin configuration parameters, if any
	std::string params_nospaces = iom::cls(params);	// remove tabs and spaces
	std::string astring;
	int aninteger = 0;
	if(!params_nospaces.empty())
	{
		std::vector<std::string> entries;
		iom::split(params_nospaces, ",", entries);
		for(int i=0; i<entries.size(); i++)
		{
			std::vector<std::string> tokens;
			iom::split(entries[i], "=", tokens);
			if(tokens.size() != 2)
				throw iom::exception(iom::strprintf("cannot parse parameter entry \"%s\". Expected \"param=value\" format", entries[i].c_str()), __iom__current__function__);
			if(tokens[0].compare("astring")==0)
				astring = tokens[1];
			else if(tokens[0].compare("aninteger")==0)
				aninteger = iom::str2num<int>(tokens[1]);
			else
				throw iom::exception(iom::strprintf("cannot parse parameter entry \"%s\". Unrecognized parameter \"%s\"", entries[i].c_str(), tokens[0].c_str()), __iom__current__function__);
		}
	}

	throw iom::exception(iom::strprintf("not implemented yet (astring = \"%s\", aninteger = %d)", astring.c_str(), aninteger), __iom__current__function__);
}


// Read 2D image data
unsigned char *						// (OUTPUT) a buffer storing the 2D image
	iomanager::exampleplugin2D::readData(
	std::string img_path,			// (INPUT)	image filepath
	int & img_width,				// (INPUT/OUTPUT) image width  (in pixels)
	int & img_height,				// (INPUT/OUTPUT) image height (in pixels)
	int & img_bytes_x_chan,			// (INPUT/OUTPUT) number of bytes per channel
	int & img_chans,				// (INPUT/OUTPUT) number of channels to be read
	unsigned char *data,			// (INPUT) image data
	const std::string & params)		// (INPUT) additional parameters <param1=val, param2=val, ...> 
throw (iom::exception) 
{
	throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);

	return 0;
}


// Write 2D image data into a single (2D) image file
void 
	iomanager::exampleplugin2D::writeData(
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
	throw iom::exception(iom::strprintf("not implemented yet"), __iom__current__function__);
}


// read 3D image data from a stack of (2D) image files
iom::real_t*						// (OUTPUT) a [0.0,1.0]-valued array storing the 3D image in channel->slice->row order
	iomanager::exampleplugin2D::readData(
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
	throw iom::exception(iom::strprintf("not implemented yet (params = \"%s\")", params.c_str()), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("files_size = %d, path = %s, first = %d, last = %d, is_sparse = %s, chan = %d, params = \"%s\"",
		files_size, path ? path : "null", first, last, is_sparse ? "true" : "false", chan, params.c_str()).c_str(), __iom__current__function__);
}


// write 2D image data into a single (2D) image file
void 
	iomanager::exampleplugin2D::writeData(
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
	throw iom::exception(iom::strprintf("not implemented yet (params = \"%s\")", params.c_str()), __iom__current__function__);

	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_height = %d, img_width = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, bpp = %d, params = \"%s\"", 
		img_path.c_str(), img_height, img_width, y0, y1, x0, x1, bpp, params.c_str()).c_str(), __iom__current__function__);
}