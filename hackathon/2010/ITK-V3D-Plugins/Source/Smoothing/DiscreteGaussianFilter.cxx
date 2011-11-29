/* DiscreteGaussianFilter.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "DiscreteGaussianFilter.h"

// ITK Header Files
#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkImageFileWriter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(DiscreteGaussianFilter, ITKDiscreteGaussianFilterPlugin)

QStringList ITKDiscreteGaussianFilterPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK DiscreteGaussian Filter ...");
}

template<typename TPixelType>
class ITKDiscreteGaussianFilterSpecializaed
{
public:
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandle oldwin = callback.currentImageWindow();
		Image4DSimple* p4DImage = callback.getImage(oldwin);
		V3D_GlobalSetting globalSetting = callback.getGlobalSetting();

		const unsigned int Dimension = 3;
	    int channelToFilter = globalSetting.iChannel_for_plugin;
	    if( channelToFilter >= p4DImage->getCDim() )
	      {
	      v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
	      return;
	      }

		//------------------------------------------------------------------
		//import image from V3D
		typedef TPixelType PixelType;
		typedef itk::Image< PixelType,  Dimension > ImageType_input;
		typedef itk::ImportImageFilter<PixelType, Dimension> ImportFilterType;

		typename ImportFilterType::Pointer importFilter = ImportFilterType::New();

		//set ROI region
		typename ImportFilterType::RegionType region;
		typename ImportFilterType::IndexType start;
		start.Fill(0);
		typename ImportFilterType::SizeType size;
		size[0] = p4DImage->getXDim();
		size[1] = p4DImage->getYDim();
		size[2] = p4DImage->getZDim();
		region.SetIndex(start);
		region.SetSize(size);
		importFilter->SetRegion(region);

		//set image Origin
		typename ImageType_input::PointType origin;
		origin.Fill(0.0);
		importFilter->SetOrigin(origin);
		//set spacing
		typename ImportFilterType::SpacingType spacing;
		spacing.Fill(1.0);
		importFilter->SetSpacing(spacing);

		//set import image pointer
		PixelType * data1d = reinterpret_cast<PixelType *> (p4DImage->getRawData());
		unsigned long int numberOfPixels = p4DImage->getTotalBytes();
		const bool importImageFilterWillOwnTheBuffer = false;
		importFilter->SetImportPointer(data1d, numberOfPixels,importImageFilterWillOwnTheBuffer);

		//------------------------------------------------------------------
		//setup filter: cast datatype to float for anisotropic process
		typedef itk::Image< float, Dimension >   	ImageType_mid;
		typedef itk::RescaleIntensityImageFilter<ImageType_input, ImageType_mid > RescaleFilterType_input;

		typename RescaleFilterType_input::Pointer rescaler_8u_32f = RescaleFilterType_input::New();
		rescaler_8u_32f->SetOutputMinimum(   0 );
		rescaler_8u_32f->SetOutputMaximum( 255 );

		//------------------------------------------------------------------
		//setup filter: Mean Filter
		typedef itk::DiscreteGaussianImageFilter<ImageType_mid,ImageType_mid> FilterType;
		typename FilterType::Pointer filter = FilterType::New();

		//set paras
		float gaussianVariance=1.0;
		unsigned int maxKernelWidth=3;
		filter->SetVariance( gaussianVariance );
		filter->SetMaximumKernelWidth( maxKernelWidth );

		//------------------------------------------------------------------
		//setup filter: cast datatype back to PixelType for output
		typedef itk::RescaleIntensityImageFilter<ImageType_mid,ImageType_input> RescaleFilterType_output;

		typename RescaleFilterType_output::Pointer rescaler_32f_8u = RescaleFilterType_output::New();
		rescaler_32f_8u->SetOutputMinimum(   0 );
		rescaler_32f_8u->SetOutputMaximum( 255 );

		//------------------------------------------------------------------
		//setup filter: write processed image to disk
		typedef itk::ImageFileWriter< ImageType_input >  WriterType;
		typename WriterType::Pointer writer = WriterType::New();
		writer->SetFileName("output.tif");

		//------------------------------------------------------------------
		//build pipeline
		rescaler_8u_32f->SetInput(importFilter->GetOutput());
		filter->SetInput(rescaler_8u_32f->GetOutput());
		rescaler_32f_8u->SetInput(filter->GetOutput());
		writer->SetInput(rescaler_32f_8u->GetOutput());

		//------------------------------------------------------------------
		//update the pixel value
		if (menu_name == QObject::tr("ITK DiscreteGaussian Filter ..."))
		{
			ITKDiscreteGaussianFilterDialog d(p4DImage, parent);

			if (d.exec() != QDialog::Accepted)
			{
				return;
			}
			else
			{
				try
				{
					writer->Update();
				}
				catch(itk::ExceptionObject &excp)
				{
					std::cerr<<excp<<std::endl;
					return;
				}
			}

		}
		else
		{
			return;
		}

		//------------------------------------------------------------------
		typedef itk::ImageRegionConstIterator<ImageType_input> IteratorType;
		IteratorType it(rescaler_32f_8u->GetOutput(), rescaler_32f_8u->GetOutput()->GetRequestedRegion());
		it.GoToBegin();

		if(!globalSetting.b_plugin_dispResInNewWindow)
		{
			printf("display results in a new window\n");
			//copy data back to V3D
			while(!it.IsAtEnd())
			{
				*data1d=it.Get();
				++it;
				++data1d;
			}

			callback.setImageName(oldwin, callback.getImageName(oldwin)+"_new");
			callback.updateImageWindow(oldwin);
		}
		else
		{
			printf("display results in current window\n");
			long N = p4DImage->getTotalBytes();
			unsigned char* newdata1d = new unsigned char[N];
			Image4DSimple tmp;
			tmp.setData(newdata1d, p4DImage );

			//copy data back to the new image
			while(!it.IsAtEnd())
			{
				*newdata1d=it.Get();
				++it;
				++newdata1d;
			}

			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, &tmp);
			callback.setImageName(newwin, callback.getImageName(oldwin)+"_new");
		    callback.updateImageWindow(newwin);
		}
	}

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKDiscreteGaussianFilterSpecializaed< c_pixel_type > runner; \
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

void ITKDiscreteGaussianFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
