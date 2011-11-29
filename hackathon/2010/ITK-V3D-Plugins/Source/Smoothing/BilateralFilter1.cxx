/* BilateralFilter.cxx
 * 2010-06-02: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "BilateralFilter.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files

#include "itkRescaleIntensityImageFilter.h"
#include "itkBilateralImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(BilateralFilter, ITKBilateralFilterPlugin)

QStringList ITKBilateralFilterPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Bilateral Filter (slow) ...");
}

template<typename TPixelType>
class ITKBilateralFilterSpecializaed:public V3DITKFilterSingleImage<TPixelType, TPixelType>
{
	typedef V3DITKFilterSingleImage<TPixelType, TPixelType> Superclass;
	typedef typename Superclass::Input3DImageType  InputImageType;
	typedef itk::Image< float, 3 >   	ImageType_mid;
	typedef itk::RescaleIntensityImageFilter<InputImageType, ImageType_mid > RescaleFilterType_input;
	typedef itk::BilateralImageFilter<ImageType_mid,ImageType_mid> AniFilterType;
	typedef itk::RescaleIntensityImageFilter<ImageType_mid, InputImageType> RescaleFilterType_output;
	typedef itk::ImageFileWriter< InputImageType >  WriterType;


public:
	ITKBilateralFilterSpecializaed(V3DPluginCallback *callback):Superclass(callback)
{
	rescaler_8u_32f = RescaleFilterType_input::New();
	filter = AniFilterType::New();
	rescaler_32f_8u= RescaleFilterType_output::New();
	writer = WriterType::New();
	this->RegisterInternalFilter(filter,0.8);
	this->RegisterInternalFilter(rescaler_8u_32f,0.1);
	this->RegisterInternalFilter(rescaler_32f_8u,0.1);
	
	
}
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		rescaler_32f_8u->SetOutputMinimum(   0 );
		rescaler_32f_8u->SetOutputMaximum( 255 );
		const unsigned int Dimension=3;
		double rangeSigma = 5.0;
		double domainSigmas[ Dimension ];
		for(unsigned int i=0;i<Dimension;i++)
			domainSigmas[i]=6.0;
		filter->SetDomainSigma( domainSigmas );
		filter->SetRangeSigma( rangeSigma );
		writer->SetFileName("output.tif");

		this->Compute();
	}
	virtual void ComputeOneRegion()
	{  
  
		rescaler_8u_32f->SetInput(this->GetInput3DImage());
		filter->SetInput(rescaler_8u_32f->GetOutput());
		rescaler_32f_8u->SetInput(filter->GetOutput());
		rescaler_32f_8u->Update();
		this->SetOutputImage(this->rescaler_32f_8u->GetOutput());
		writer->SetInput(rescaler_32f_8u->GetOutput());
	}
private:
	typename RescaleFilterType_input::Pointer rescaler_8u_32f;
	typename AniFilterType::Pointer filter ;
	typename RescaleFilterType_output::Pointer rescaler_32f_8u;
	typename WriterType::Pointer writer;


};


#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	  ITKBilateralFilterSpecializaed< c_pixel_type > runner(&callback); \
    runner.Execute(  menu_name, callback, parent ); \
    break; \
    }

#define EXECUTE_ALL_PIXEL_TYPES \
	Image4DSimple *p4DImage = callback.getImage(curwin); \
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

void ITKBilateralFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
