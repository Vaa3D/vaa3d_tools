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
	/**/iom::debug(iom::LEV3, iom::strprintf("files_size = %d, path = %s, first = %d, last = %d, is_sparse = %s, chan = %d, params = \"%s\"",
		files_size, path ? path : "null", first, last, is_sparse ? "true" : "false", chan, params.c_str()).c_str(), __iom__current__function__);


	throw iom::exception(iom::strprintf("not implemented yet (params = \"%s\")", params.c_str()), __iom__current__function__);
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
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = %s, img_height = %d, img_width = %d, y0 = %d, y1 = %d, x0 = %d, x1 = %d, bpp = %d, params = \"%s\"", 
		img_path.c_str(), img_height, img_width, y0, y1, x0, x1, bpp, params.c_str()).c_str(), __iom__current__function__);


	throw iom::exception(iom::strprintf("not implemented yet (params = \"%s\")", params.c_str()), __iom__current__function__);
}