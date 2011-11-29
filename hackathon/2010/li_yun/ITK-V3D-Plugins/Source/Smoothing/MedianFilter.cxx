/* MedianFilter.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "MedianFilter.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(MedianFilter, ITKMedianFilterPlugin)

QStringList ITKMedianFilterPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Median Filter ...")
		 	     << QObject::tr("about this plugin");
}

template<typename TPixelType>
class ITKMedianFilterSpecializaed: public V3DITKFilterSingleImage< TPixelType,TPixelType >
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >  				Superclass;
	typedef TPixelType  									PixelType;
	typedef itk::Image< PixelType,  3 > 							ImageType_input;
	typedef itk::MedianImageFilter<ImageType_input,ImageType_input> FilterType;
public:
	ITKMedianFilterSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->filter 	      = FilterType::New();
		this->RegisterInternalFilter( this->filter, 1.0 );    		
	}

 	virtual ~ITKMedianFilterSpecializaed() {};

	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		typename ImageType_input::SizeType indexRadius;
		indexRadius[0] = 1; // radius along x
		indexRadius[1] = 1; // radius along y
		indexRadius[2] = 1; // radius along y
		filter->SetRadius( indexRadius );

		this->Compute();
		//------------------------------------------------------------------
		//setup filter: write processed image to disk
		//typedef itk::ImageFileWriter< ImageType_input >  WriterType;
		//typename WriterType::Pointer writer = WriterType::New();
		//writer->SetFileName("output.tif");		
	}
	virtual void ComputeOneRegion()
	{		
		//------------------------------------------------------------------
		//build pipeline
		filter->SetInput( this->GetInput3DImage() );
		//writer->SetInput(filter->GetOutput());

		//------------------------------------------------------------------
		//update the pixel value
		filter->Update();
		this->SetOutputImage(filter->GetOutput());
	}
private:
	typename FilterType::Pointer	filter;
};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    ITKMedianFilterSpecializaed< c_pixel_type > runner(&callback); \
    runner.Execute( callback, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
    if (! p4DImage) return; \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      EXECUTE( V3D_UINT8, unsigned char );  \
      EXECUTE( V3D_UINT16, unsigned short int );  \
      EXECUTE( V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }  

void ITKMedianFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	if (menu_name == QObject::tr("about this plugin"))
	{
	QMessageBox::information(parent, "Version info", "ITKMedianFilter...");
	return;
	}

	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
	v3d_msg(tr("You don't have any image open in the main window."));
	return;
	}

	Image4DSimple *p4DImage = callback.getImage(curwin);
	if (!p4DImage)
	{
	v3d_msg(tr("The input image is null."));
	return;
	}
	EXECUTE_ALL_PIXEL_TYPES; 
}
