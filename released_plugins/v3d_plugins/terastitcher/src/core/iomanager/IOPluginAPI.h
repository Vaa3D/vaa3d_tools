#ifndef _IOM_IMAGE_FORMAT_H
#define _IOM_IMAGE_FORMAT_H

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

		// read image metadata from a 2D image file
		virtual void 
			readMetadata(
			std::string img_path,		// (INPUT)  image filepath
			int & img_width,			// (OUTPUT) image width  (in pixels)
			int & img_height,			// (OUTPUT) image height (in pixels)
			int & img_bytes_x_chan,		// (OUTPUT) number of bytes per channel
			int & img_chans,			// (OUTPUT) number of channels
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;


		// read 3D image data from a stack of (2D) image files
		virtual iom::real_t*					// (OUTPUT) a [0.0,1.0]-valued array storing the 3D image in channel->slice->row order
			readData(
			char **files,				// (INPUT)	array of C-strings storing image filenames
			int files_size,				// (INPUT)	size of 'files'
			const char *path = 0,		// (INPUT)	path to be concatenated to the i-th entry of 'files'
			int first = -1,				// (INPUT)	selects a range [first, last] of files to be loaded
			int last = -1,				// (INPUT)	selects a range [first, last] of files to be loaded	
			bool is_sparse = false,		// (INPUT)	if true, 'files' is a sparse array and null entries should be treated as empty (black) images
			iom::channel chan = CHANS,	// (INPUT)	channel selection { ALL, R, G, B }. 
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		
		// write 2D image data into a single (2D) image file
        virtual void 
			writeData(
			std::string img_path,		// (INPUT)	image filepath (it includes the file extension)
			iom::real_t* raw_img,			// (INPUT)	a [0.0,1.0]-valued array storing the 2D image in channel->row order
			int img_height,				// (INPUT)	image height
			int img_width,				// (INPUT)	image width
			int img_chans,				// (INPUT)	number of channels
			int y0 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int y1 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x0 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int x1 = -1,					// (INPUT)	region of interest [x0,x1][y0,y1] to be set on the image
			int bpp = iom::DEF_BPP,		// (INPUT)	color depth (bits per pixel)
			const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		// return plugin description
		virtual std::string desc() = 0;
};



// abstract 3D IO plugin: a new plugin should implement just its abstract methods (see ExamplePlugin)
class iomanager::IOPlugin3D : public iomanager::IOPlugin
{
	public:

		// read image metadata
		virtual void 
			readMetadata(
			std::string img_path,		// (INPUT)	image filepath
			int & img_width,			// (OUTPUT) image width  (in pixels)
			int & img_height,			// (OUTPUT) image height (in pixels)
			int & img_depth,			// (OUTPUT) image depth (in pixels)
			int & img_bytes_x_chan,		// (OUTPUT) number of bytes per channel
			int & img_chans,			// (OUTPUT) number of channels
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		// read 3D image data
		virtual void
			readData(
			char *finName,				// (INPUT)	image filepath
			int XSIZE,					// (INPUT)	image width (in pixels)
			int YSIZE,					// (INPUT)	image height (in pixels)
			unsigned char *data,		// (OUTPUT) image data
			int first,					// (INPUT)	selects a range [first, last] of files to be loaded 
			int last,					// (INPUT)	selects a range [first, last] of files to be loaded 
			const std::string & params = iom::IMIN_PLUGIN_PARAMS)	// (INPUT) additional parameters <param1=val, param2=val, ...> 
		throw (iom::exception) = 0;

		// return plugin description
		virtual std::string desc() = 0;
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
			iom::real_t*												\
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
				iom::real_t* raw_img,									\
				int img_height,											\
				int img_width,											\
				int img_chans,											\
				int y0 = -1,											\
				int y1 = -1,											\
				int x0 = -1,											\
				int x1 = -1,											\
				int bpp = iom::DEF_BPP,									\
				const std::string & params = iom::IMOUT_PLUGIN_PARAMS)	\
			throw (iom::exception);										\
																		\
			virtual std::string desc();									\
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
																		\
			virtual std::string desc();									\
	};																	\
}																		\
																		\
	namespace{															\
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
