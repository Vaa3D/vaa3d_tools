#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include "opencv2D.h"

// just call this macro to register your plugin
TERASTITCHER_REGISTER_IO_PLUGIN_2D(opencv2D)

// insert here your plugin description that will be displayed on the user interface
std::string iomanager::opencv2D::desc()
{
	return	"******************************************************\n"
			"* opencv2D v.1.0                                     *\n"
			"******************************************************\n"
			"*                                                    *\n"
			"* 2D image-based I/O plugin that uses the OpenCV li- *\n"
			"* brary to read/write 2D image files.                *\n"
			"*                                                    *\n"
			"* Supported image format extentions:                 *\n"
			"*  - BMP                                             *\n"
			"*  - DIB                                             *\n"
			"*  - JPEG                                            *\n"
			"*  - JPG                                             *\n"
			"*  - JPE                                             *\n"
			"*  - PNG                                             *\n"
			"*  - PBM                                             *\n"
			"*  - PGM                                             *\n"
			"*  - PPM                                             *\n"
			"*  - SR                                              *\n"
			"*  - RAS                                             *\n"
			"*  - TIFF                                            *\n"
			"*  - TIF                                             *\n"
			"*                                                    *\n"
			"* Accepted configuration parameters:                 *\n"
			"*  - none                                            *\n"
			"*                                                    *\n"
			"******************************************************\n";
}

// read image metadata from a 2D image file
void 
	iomanager::opencv2D::readMetadata(
	std::string img_path,			// (INPUT)  image filepath
	int & img_width,				// (OUTPUT) image width  (in pixels)
	int & img_height,				// (OUTPUT) image height (in pixels)
	int & img_bytes_x_chan,			// (OUTPUT) number of bytes per channel
	int & img_chans,				// (OUTPUT) number of channels
	const std::string & params)		// (INPUT)  additional parameters <param1=val, param2=val, ...> 
throw (iom::exception)
{
	/**/iom::debug(iom::LEV3, iom::strprintf("img_path = \"%s\", params = \"%s\"",img_path.c_str(), params.c_str()).c_str(), __iom__current__function__);

	// try to read image
	cv::Mat img = cv::imread(img_path);
	if(!img.data)
		throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF",
		img_path.c_str()), __iom__current__function__);

	// read image metadata
	img_width  = img.rows;
	img_height = img.cols;
	img_chans  = img.channels();
	switch(img.depth())
	{
		case CV_8U:
			img_bytes_x_chan = 1; break;
		case CV_8S:
			img_bytes_x_chan = 1; break;
		case CV_16U:
			img_bytes_x_chan = 2; break;
		case CV_16S:
			img_bytes_x_chan = 2; break;		
		case CV_32S:
			img_bytes_x_chan = 4; break;
		case CV_32F:
			img_bytes_x_chan = 4; break;	
		case CV_64F:
			img_bytes_x_chan = 8; break;
		default:
			throw iom::exception(iom::strprintf("unable determine bitdepth of image at \"%s\"",	img_path.c_str()), __iom__current__function__);
	}
}


// read 3D image data from a stack of (2D) image files
iom::real_t*						// (OUTPUT) a [0.0,1.0]-valued array storing the 3D image in channel->slice->row order
	iomanager::opencv2D::readData(
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

	iom::real_t *image_stack = 0;
	uint64 image_stack_width=0, image_stack_height=0, z=0;

	// check for valid file list
	if(!files || files_size <= 0)
		throw iom::exception("invalid file list (null pointer or 0-sized)", __iom__current__function__);

	// check and adjust file selection
	first = (first == -1 ? 0 : first);
	last  = (last  == -1 ? files_size - 1 : last );
	first = std::min(first, files_size-1);
	last = std::min(last, files_size-1);

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

		// load image
		cv::Mat image;
		double proctime = -TIME(0);
		if(chan == iom::ALL)
			image = cv::imread(image_path, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);   // pack all channels into grayscale
		else
			image = cv::imread(image_path, CV_LOAD_IMAGE_ANYCOLOR);                             // load individual channels
		if(!image.data)
			throw iom::exception(iom::strprintf("unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", image_path.c_str()), __iom__current__function__);

		// select channel (if requested)
		if(image.channels() == 3 && chan != iom::ALL)
		{
			cv::Mat imageChannels[3];
			cv::split(image, imageChannels);
			image = imageChannels[3-CHANS]; // OpenCV uses BGR !
		}

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
			image_stack_height = (uint64) image.rows;
			image_stack_width  = (uint64) image.cols;
			uint64 image_stack_size = image_stack_width * image_stack_height * (last-first+1);
			image_stack = new iom::real_t[image_stack_size];
			for(uint64 j=0; j < image_stack_size; j++)
				image_stack[j] = 0;		// default is 0 (black)
		}
		else if(image_stack_width != (uint64)image.cols || image_stack_height != (uint64)image.rows)
			throw iom::exception("images in stack have not the same dimensions", __iom__current__function__);

		// convert image to [0.0,1.0]-valued array
		iom::real_t *raw_data = &image_stack[z*image.rows*image.cols];
		if(image.depth() == CV_8U)
		{
			for(int i=0; i<image.rows; i++)
			{
				uint8* img_data = image.ptr<uint8>(i);
				for(int j=0; j<image.cols; j++, raw_data++)
					*raw_data = img_data[j]/255.0f;
			}
		}
		else if (image.depth() == CV_16U)
		{
			for(int i=0; i<image.rows; i++)
			{
				uint16* img_data = image.ptr<uint16>(i);
				for(int j=0; j<image.cols; j++, raw_data++)
					*raw_data = img_data[j]/65535.0f;
			}
		}
		else
			throw iom::exception("unsupported image depth", __iom__current__function__);

		// update time
		if(TIME_CALC)
		{
			proctime += TIME(0);
			time_conversions+=proctime;
		}
	}

	// cannot load any data from this stack
	if(!image_stack)
		throw iom::exception(iom::strprintf("stack (%s) does not contain loadable data in [%d,%d]", path ? path : "0", first, last), __iom__current__function__);

	return image_stack;
}


// write 2D image data into a single (2D) image file
void 
	iomanager::opencv2D::writeData(
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
	cv::Mat image(ROI_height, ROI_width, bpp == 8 ? CV_8U : CV_16U, cv::Scalar(0));
	if(bpp == 8)
	{
		for(int i = 0; i <ROI_height; i++)
		{
			uint8* img_data = image.ptr<uint8>(i);
			for(int j = 0; j < ROI_width; j++)
				img_data[j] = static_cast<uint8>(raw_img[(i+y0)*img_width+j+x0] * 255.0f + 0.5f);
		}
	}
	else
	{
		for(int i = 0; i <ROI_height; i++)
		{
			uint16* img_data = image.ptr<uint16>(i);
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
	try
	{
		cv::imwrite(img_path, image);
	}
	catch(...)
	{
		throw iom::exception(iom::strprintf("unable to save image at \"%s\". Unsupported format or wrong path.\n", img_path.c_str()), __iom__current__function__);
	}

	// update IO time
	if(TIME_CALC)
	{
		proctime += TIME(0);
		time_IO+=proctime;
	}
}