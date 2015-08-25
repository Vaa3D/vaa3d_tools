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

#ifndef _IOM_IMAGE_FORMAT_H
#define _IOM_IMAGE_FORMAT_H

/************************************************************************************************
* General information
*
* Different plugins are available for managing 2D image formats and 3D image formats. 
* Nevertheless 3D plugins may occasionally manage images which third dimension is a 
* singleton.
*
* All plugins are assumed to manage images that can be in compressed format. 
* In case of 3D plugins it is assumed that the image format can extract efficiently 2D slices 
* from files containing 3D images (e.g. multi-page tiff format).
*
* Plugins can manage multi-channel images in two ways: interleaving channels (i.e. channels are 
* the first dimension) or splitting channels (i.e. channels are the last dimension). 
* Plugins inform client code on how channels are managed through the method 'isChansInterleaved',
* returning true if channels are interleaved, false if they are splitted.
* Client code must check which representation is used and correctly process image data returned 
* by the 'read' methods, as well as pass image data to write methods in the right format.
*
* Intervals used to specify ROIs refer to pixel indices and do not include the upper limit, 
* i.e. the interval [x0,x1) includes the (x1-x0) indices from x0 to x1-1, included.
*************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
* 2015-01-02. Giulio.     @ADDED new plugins interface
*/

#include <string>
#include <map>
#include "iomanager.config.h"

// top-level image i/o plugin
class iomanager::IOPlugin
{
	public:

		double time_IO;					//time employed in Input/Output 
		double time_conversions;		//time employed in converting raw data to/from image data

		IOPlugin() : time_IO(0), time_conversions(0){}

		// return plugin description
		virtual std::string desc() = 0;
};



// abstract 2Dimage-based IO plugin: a new plugin should implement just its abstract methods (see exampleplugin2D)
class iomanager::IOPlugin2D : public iomanager::IOPlugin
{
	public:

		/************************************************************************************************
		* Read image metadata from a 2D file
		*************************************************************************************************/
		virtual void 
			readMetadata(
			std::string img_path,			// (INPUT)  image filepath
			int & img_width,				// (OUTPUT) image width  (in pixels)
			int & img_height,				// (OUTPUT) image height (in pixels)
			int & img_bytes_x_chan,			// (OUTPUT) number of bytes per channel
			int & img_chans,				// (OUTPUT) number of channels
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Read 2D image data
		* If data in not NULL, parameters img_width, img_height, img_bytes_x_chan, img_chans
		* must be passed by the caller otherwise a buffer is internally allocated and the above parameters 
		* are set with metadata recovered from the image. 
		* If data in not NULL data must point to a buffer of:
		*                    img_height x img_width x img_bytes_x_chan x img_chans
		* bytes.
		* In all cases the buffer with read data is returned to the caller.
		* Bytes swapping, if needed is performed before returning the buffer filled with the image data
		*************************************************************************************************/
		virtual unsigned char *						// (OUTPUT) a buffer storing the 2D image
			readData(
			std::string img_path,			// (INPUT)	image filepath
			int & img_width,				// (INPUT/OUTPUT) image width  (in pixels)
			int & img_height,				// (INPUT/OUTPUT) image height (in pixels)
			int & img_bytes_x_chan,			// (INPUT/OUTPUT) number of bytes per channel
			int & img_chans,				// (INPUT/OUTPUT) number of channels to be read
			unsigned char *data = 0,		// (INPUT) image data
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;
		
		/************************************************************************************************
		* Write 2D image data into a single (2D) image file
		* The ROI specifies a subvolume into the buffer pointed by raw_img which are the only data saved 
		* into the file. This means that the image actually saved has dimensions:
		*     (y1-y0) x (x1-x0)
		* Default values for ROI limits correspond to the whole buffer.
		*************************************************************************************************/
        virtual void 
			writeData(
			std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
			unsigned char *raw_img,			// (INPUT)	image data to be saved into the file
			int img_height,					// (INPUT)	image height
			int img_width,					// (INPUT)	image width
			int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
			int img_chans,					// (INPUT)	number of channels
			int y0 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int y1 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x0 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x1 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Return plugin description
		*************************************************************************************************/
		virtual std::string desc() = 0;
		
		/************************************************************************************************
		* Return if channels are interleaved (in case the image has just one channel return value is 
		* indefinite)
		*************************************************************************************************/
		virtual bool isChansInterleaved() = 0;


		/************************************************************************************************
		* Methods deprecated
		*************************************************************************************************/		

		// read 3D image data from a stack of (2D) image files
		virtual iom::real_t*				// (OUTPUT) a [0.0,1.0]-valued array storing the 3D image in channel->slice->row order
			readData(
			char **files,					// (INPUT)	array of C-strings storing image filenames
			int files_size,					// (INPUT)	size of 'files'
			const char *path = 0,			// (INPUT)	path to be concatenated to the i-th entry of 'files'
			int first = -1,					// (INPUT)	selects a range [first, last] of files to be loaded
			int last = -1,					// (INPUT)	selects a range [first, last] of files to be loaded	
			bool is_sparse = false,			// (INPUT)	if true, 'files' is a sparse array and null entries should be treated as empty (black) images
			iom::channel chan = CHANS,		// (INPUT)	channel selection { ALL, R, G, B }. 
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		
		// write 2D image data into a single (2D) image file
        virtual void 
			writeData(
			std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
			iom::real_t *raw_img,			// (INPUT)	a [0.0,1.0]-valued array storing the 2D image in channel->row order
			int img_height,					// (INPUT)	image height
			int img_width,					// (INPUT)	image width
			int img_chans,					// (INPUT)	number of channels
			int y0 = -1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int y1 = -1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x0 = -1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x1 = -1,						// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int bpp = iom::DEF_BPP,			// (INPUT)	color depth (bits per pixel)
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;
};



// abstract 3D IO plugin: a new plugin should implement just its abstract methods (see ExamplePlugin)
class iomanager::IOPlugin3D : public iomanager::IOPlugin
{
	public:

		/************************************************************************************************
		* Read image metadata from a 3D file
		*************************************************************************************************/
		virtual void 
			readMetadata(
			std::string img_path,			// (INPUT)	image filepath
			int & img_width,				// (OUTPUT) image width  (in pixels)
			int & img_height,				// (OUTPUT) image height (in pixels)
			int & img_depth,				// (OUTPUT) image depth (in pixels)
			int & img_bytes_x_chan,			// (OUTPUT) number of bytes per channel
			int & img_chans,				// (OUTPUT) number of channels
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Read 3D image data
		* A subset of slices can be specified by parameters z0 and z1.  
		* Default values correspond to z0 = 0 and z1 = total number o slices in the file.
		* If data in not NULL, parameters img_width, img_height, img_depth, img_bytes_x_chan, img_chans
		* must be passed by the caller and the data must point to a buffer of 
		*     (img_width * img_height * (z1-z0) * img_bytes_x_chan * img_chans)
		* bytes, otherwise a buffer is internally allocated and the above parameters are set with metadata
		* recovered from the image. 
		* In all cases the buffer with read data is returned to the caller.
		* Bytes swapping, if needed is performed before returning the buffer filled with the image data
		*************************************************************************************************/
		virtual unsigned char *				// (OUTPUT) buffer containing the read image data
			readData(
			std::string img_path,			// (INPUT) image filepath
			int & img_width,				// (INPUT/OUTPUT) image width  (in pixels)
			int & img_height,				// (INPUT/OUTPUT) image height (in pixels)
			int & img_depth,				// (INPUT/OUTPUT) image depth (in pixels)
			int & img_bytes_x_chan,			// (INPUT/OUTPUT) number of bytes per channel
			int & img_chans,				// (INPUT/OUTPUT) number of channels to be read
			unsigned char *data = 0,		// (INPUT) image data
			int z0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int z1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Write 3D image data into a single (3D) image file
		* If there is more than one channel, channel layout must be coherent with the value returned by 
		* method isChansInterleaved().
		* The ROI specifies a subvolume into the buffer pointed by raw_img which are the only data to be 
		* saved into the file. This means that the image actually saved has dimensions:
		*     (z1-z0) x (y1-y0) x (x1-x0)
		* Default values for ROI limits correspond to the whole buffer.
		*************************************************************************************************/
        virtual void 
			writeData(
			std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
			unsigned char * raw_img,		// (INPUT)	image data to be saved into the file
			int img_height,					// (INPUT)	image height (in pixels)
			int img_width,					// (INPUT)	image width (in pixels)
			int img_depth,					// (INPUT)  image depth (in pixels)
			int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
			int img_chans,					// (INPUT)	number of channels
			int z0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int z1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int y0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int y1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int x0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			int x1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1)[z0,z1) to be set on the image
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Create an empty 3D image 
		* Initialize a file that has to contain a 3D image. The newly created file is initially empty 
		* and it has to be filled adding slices with method 'appendSlice'.
		* WARNING: exactly 'img_depth' slices have to be written in the newly created file before it can be read.
		*************************************************************************************************/
        virtual void 
			create3Dimage(
			std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
			int img_height,					// (INPUT)	image height (in pixels)
			int img_width,					// (INPUT)	image width (in pixels)
			int img_depth,					// (INPUT)  image depth (in pixels)
			int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
			int img_chans,					// (INPUT)	number of channels
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Append a single slice at the bottom of a 3D image file
		* The ROI specify a subarea into the buffer pointed by raw_img which are the only data appended 
		* to the file. This means that the saved slice actually saved has dimensions:
		*     (y1-y0) x (x1-x0)
		* Default values for ROI limits correspond to the whole buffer.
		* If the value of parameter slice is not the default value, its value must be equal to the 
		* number of slices already stored into the file.
		* WARNING: if more slices than were specified when the file was created (see 'create3Dimage'), 
		* the file might be corrupted.
		*************************************************************************************************/
        virtual void 
			appendSlice(
			std::string img_path,			// (INPUT)	image filepath (it includes the file extension)
			unsigned char * raw_img,		// (INPUT)	slice to be saved into the file
			int img_height,					// (INPUT)	slice height (in pixels)
			int img_width,					// (INPUT)	slice width (in pixels)
			int img_bytes_x_chan,			// (INPUT)  number of bytes per channel
			int img_chans,					// (INPUT)	number of channels
			int y0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
			int y1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
			int x0 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
			int x1 = -1,					// (INPUT)	region of interest [x0,x1)[y0,y1) to be set on the image
			int slice = -1,					// (INPUT)  slice index 
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		/************************************************************************************************
		* Return plugin description
		*************************************************************************************************/
		virtual std::string desc() = 0;
		
		/************************************************************************************************
		* return if channels are interleaved (in case the image has just one channel return value is 
		* indefinite)
		*************************************************************************************************/
		virtual bool isChansInterleaved() = 0;


		/************************************************************************************************
		* Methods deprecated
		*************************************************************************************************/

		// read 3D image data
		virtual void
			readData(
			char *finName,					// (INPUT)	image filepath
			int XSIZE,						// (INPUT)	image width (in pixels)
			int YSIZE,						// (INPUT)	image height (in pixels)
			unsigned char *data,			// (OUTPUT) image data
			int first,						// (INPUT)	selects a range [first, last] of files to be loaded 
			int last,						// (INPUT)	selects a range [first, last] of files to be loaded 
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;
};


// define new type: Format2D plugin creator function
typedef iomanager::IOPlugin2D* (*IOPlugin2DCreator)(void);

// define new type: Format3D plugin creator function
typedef iomanager::IOPlugin3D* (*IOPlugin3DCreator)(void);

// Factory for plugins' registration and instantiation
class iomanager::IOPluginFactory 
{
	private:

		// map of registered I/O plugins
		std::map<std::string, IOPlugin2DCreator> registry2D; 
		std::map<std::string, IOPlugin3DCreator> registry3D; 

	public:

		// singleton
		static IOPluginFactory* instance()
		{
			static IOPluginFactory* uniqueInstance = new IOPluginFactory();
			return uniqueInstance;
		}

		// plugin registration
		static std::string registerPlugin2D(IOPlugin2DCreator creator, std::string id) 
		{
			instance()->registry2D[id] = creator;
			return id;
		}		
		static std::string registerPlugin3D(IOPlugin3DCreator creator, std::string id) 
		{
			instance()->registry3D[id] = creator;
			return id;
		}

		// plugin instantiation
		static IOPlugin2D* getPlugin2D(std::string id) throw (iom::exception)
		{ 
			if(instance()->registry2D.find(id) == instance()->registry2D.end())
				throw iom::exception(iom::strprintf("Cannot find 2D I/O plugin \"%s\" or it is not a 2D I/O plugin", id.c_str()).c_str());
			return (instance()->registry2D[id])(); 
		}
		static IOPlugin3D* getPlugin3D(std::string id) throw (iom::exception)
		{ 
			if(instance()->registry3D.find(id) == instance()->registry3D.end())
				throw iom::exception(iom::strprintf("Cannot find 3D I/O plugin \"%s\" or it is not a 3D I/O plugin", id.c_str()).c_str());
			return (instance()->registry3D[id])(); 
		}

		// get list of registered plugins
		static std::string registeredPlugins2D(){
			std::string plugins;
			for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
				plugins += "\"" + it->first + "\", ";
			plugins = plugins.substr(0, plugins.find_last_of(","));
			return plugins;
		}		
		static std::string registeredPlugins3D(){
			std::string plugins;
			for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
				plugins += "\"" + it->first + "\", ";
			plugins = plugins.substr(0, plugins.find_last_of(","));
			return plugins;
		}
		static std::string registeredPlugins(){
			std::string plugins;
			for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
				plugins += "\"" + it->first + "\", ";
			for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
				plugins += "\"" + it->first + "\", ";
			plugins = plugins.substr(0, plugins.find_last_of(","));
			return plugins;
		}
        static std::vector< std::string > registeredPluginsList(){
            std::vector< std::string > plugins;
            for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
                plugins.push_back(it->first);
            for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
                plugins.push_back(it->first);
            return plugins;
        }

		// get plugins info
		static std::string pluginsInfo(){
			std::string info;
			for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
				info += "\n" + it->second()->desc() + "\n";
			for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
				info += "\n" + it->second()->desc() + "\n";
			return info;
		}

		// get total time
		static double getTimeConversions(){
			double sum = 0;
			for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
				sum += it->second()->time_conversions;
			for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
				sum += it->second()->time_conversions;
			return sum;
		}
		static double getTimeIO(){
			double sum = 0;
			for(std::map<std::string, IOPlugin3D* (*)(void)>::iterator it = instance()->registry3D.begin(); it != instance()->registry3D.end(); it++)
				sum += it->second()->time_IO;
			for(std::map<std::string, IOPlugin2D* (*)(void)>::iterator it = instance()->registry2D.begin(); it != instance()->registry2D.end(); it++)
				sum += it->second()->time_IO;
			return sum;
		}
};

// macro for automated 2D IO plugin definition
#define TERASTITCHER_CREATE_IO_PLUGIN_2D(classname)						\
namespace iomanager														\
{																		\
	class classname : public IOPlugin2D									\
	{																	\
		private:														\
																		\
			static const std::string _id;								\
			static IOPlugin2D* create() { return instance(); }			\
																		\
		public:															\
																		\
			static classname* instance(){								\
				static classname * uniqueInstance = new classname ();   \
				return uniqueInstance;									\
			}															\
																		\
			virtual void												\
				readMetadata(											\
				std::string img_path,									\
				int & img_width,										\
				int & img_height,										\
				int & img_bytes_x_chan,									\
				int & img_chans,										\
				const std::string & params = iom::IMIN_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
																		\
			virtual unsigned char *                                     \
				readData(                                               \
				std::string img_path,                                   \
				int & img_width,                                        \
				int & img_height,                                       \
				int & img_bytes_x_chan,                                 \
				int & img_chans,                                        \
				unsigned char *data = 0,                                \
				const std::string & params = iom::IMIN_PLUGIN_PARAMS)   \
			throw (iom::exception);                                     \
			                                                            \
	        virtual void                                                \
				writeData(                                              \
				std::string img_path,                                   \
				unsigned char *raw_img,                                 \
				int img_height,	                                        \
				int img_width,	                                        \
				int img_bytes_x_chan,                                   \
				int img_chans,                                          \
				int y0 = -1,                                            \
				int y1 = -1,                                            \
				int x0 = -1,												\
				int x1 = -1,                                            \
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)  \
			throw (iom::exception);                                     \
                                                                        \
			virtual std::string desc();									\
																		\
			virtual bool isChansInterleaved();                          \
																		\
			virtual iom::real_t*										\
				readData(												\
				char **files,											\
				int files_size,											\
				const char *path = 0,									\
				int first = -1,											\
				int last = -1,											\
				bool is_sparse = false,									\
				iom::channel chan = CHANS,								\
				const std::string & params = iom::IMIN_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
																		\
			virtual void												\
				writeData(												\
				std::string img_path,									\
				iom::real_t *raw_img,									\
				int img_height,											\
				int img_width,											\
				int img_chans,											\
				int y0 = -1,												\
				int y1 = -1,												\
				int x0 = -1,												\
				int x1 = -1,												\
				int bpp = iom::DEF_BPP,									\
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
	};																	\
}																		\
																		\
namespace{																\
	const iomanager::classname* object##classname = iomanager::classname::instance(); \
} 

// macro for automated 3D IO plugin definition
#define TERASTITCHER_CREATE_IO_PLUGIN_3D(classname)						\
namespace iomanager														\
{																		\
	class classname : public IOPlugin3D									\
	{																	\
		private:														\
																		\
			static const std::string _id;								\
			static IOPlugin3D* create() { return instance(); }			\
																		\
		public:															\
																		\
			static classname* instance(){								\
				static classname * uniqueInstance = new classname ();	\
				return uniqueInstance;									\
			}															\
																		\
			virtual void												\
				readMetadata(											\
				std::string img_path,									\
				int & img_width,										\
				int & img_height,										\
				int & img_depth,										\
				int & img_bytes_x_chan,									\
				int & img_chans,										\
				const std::string & params = IMIN_PLUGIN_PARAMS)		\
			throw (iom::exception);										\
																		\
			virtual unsigned char *	                                    \
				readData(                                               \
				std::string img_path,	                                \
				int & img_width,	                                    \
				int & img_height,	                                    \
				int & img_depth,	                                    \
				int & img_bytes_x_chan,	                                \
				int & img_chans,                                        \
				unsigned char *data = 0,                                \
				int z0 = -1,                                            \
				int z1 = -1,                                            \
				const std::string & params = iom::IMIN_PLUGIN_PARAMS)   \
			throw (iom::exception);                                     \
																		\
	        virtual void												\
				writeData(												\
				std::string img_path,									\
				unsigned char * raw_img,								\
				int img_height,											\
				int img_width,											\
				int img_depth,											\
				int img_bytes_x_chan,									\
				int img_chans,											\
				int z0 = -1,												\
				int z1 = -1,												\
				int y0 = -1,												\
				int y1 = -1,												\
				int x0 = -1,												\
				int x1 = -1,												\
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
																		\
	        virtual void												\
				create3Dimage(											\
				std::string img_path,									\
				int img_height,											\
				int img_width,											\
				int img_depth,											\
				int img_bytes_x_chan,									\
				int img_chans,											\
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)  \
			throw (iom::exception);                                     \
																		\
	        virtual void 												\
				appendSlice(											\
				std::string img_path,									\
				unsigned char * raw_img,								\
				int img_height,											\
				int img_width,											\
				int img_bytes_x_chan,									\
				int img_chans,											\
				int y0 = -1,												\
				int y1 = -1,												\
				int x0 = -1,												\
				int x1 = -1,												\
				int slice = -1,											\
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
																		\
			virtual std::string desc();									\
																		\
			virtual bool isChansInterleaved();                          \
																		\
			virtual void												\
				readData(												\
				char *finName,											\
				int XSIZE,												\
				int YSIZE,												\
				unsigned char *data,									\
				int first,												\
				int last,												\
				const std::string & params = IMIN_PLUGIN_PARAMS)		\
			throw (iom::exception);										\
	};																	\
}																		\
																		\
namespace{																\
	const iomanager::classname* object##classname = iomanager::classname::instance(); \
} 

// macro for automated 2D IO plugin registration
#define TERASTITCHER_REGISTER_IO_PLUGIN_2D(classname) \
	const std::string iomanager::classname::_id = iomanager::IOPluginFactory::registerPlugin2D(&create, #classname);

// macro for automated 3D IO plugin registration
#define TERASTITCHER_REGISTER_IO_PLUGIN_3D(classname) \
	const std::string iomanager::classname::_id = iomanager::IOPluginFactory::registerPlugin3D(&create, #classname);

// include the header automatically generated by CMakeFile to import all the available plugins
#include "ioplugins.h"

#endif // _IOM_IMAGE_FORMAT_H
