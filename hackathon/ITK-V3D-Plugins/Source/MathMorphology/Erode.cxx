#include <QtGui>
#include <math.h>
#include <stdlib.h>

#include "Erode.h"

// ITK Header Files
#include "itkBinaryErodeImageFilter.h"
#include "itkGrayscaleErodeImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkBinaryBallStructuringElement.h" 


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(Erode, ErodePlugin)


QStringList ErodePlugin::menulist() const
{
    return QStringList() << QObject::tr("Erode")
	<< QObject::tr("about this plugin");
}

QStringList ErodePlugin::funclist() const
{
    return QStringList();
}


template <typename TPixelType>
class ErodeSpecialized
{
public:
	void Execute(const QString &menu_name,  V3DPluginCallback & callback, QWidget *parent)
    {
		if (menu_name == QObject::tr("about this plugin"))
		{
			v3d_msg(QObject::tr("Erode 1.0 (2010-June-2)"));
		}
		else if(menu_name == QObject::tr("Erode"))
		{
			typedef TPixelType  PixelType;
			
			v3dhandle curwin = callback.currentImageWindow();
			
			V3D_GlobalSetting globalSetting = callback.getGlobalSetting();
			Image4DSimple *p4DImage = callback.getImage(curwin);
			
			PixelType * data1d = reinterpret_cast< PixelType * >( p4DImage->getRawData() );
			unsigned long int numberOfPixels = p4DImage->getTotalBytes();
			
			// long pagesz = p4DImage->getTotalUnitNumberPerChannel();
			
			V3DLONG nx = p4DImage->getXDim();
			V3DLONG ny = p4DImage->getYDim();
			V3DLONG nz = p4DImage->getZDim();
			V3DLONG nc = p4DImage->getCDim();  // Number of channels
			
			
			int channelToFilter = globalSetting.iChannel_for_plugin;
			if (channelToFilter<0) 
			{
				v3d_msg(QObject::tr("Need to add *channelToFilter* in the program!"));
				return;
			}
			
			if( channelToFilter >= nc )
			{
				v3d_msg(QObject::tr("You are selecting a channel that doesn't exist in this image."));
				return;
			}
			
			
			const unsigned int Dimension = 3;
			
			typedef itk::Image< PixelType, Dimension > ImageType;
			typedef itk::ImportImageFilter< PixelType, Dimension > ImportFilterType;
			
			typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
			
			typename ImportFilterType::SizeType size;
			size[0] = nx;
			size[1] = ny;
			size[2] = nz;
			
			typename ImportFilterType::IndexType start;
			start.Fill( 0 );
			
			typename ImportFilterType::RegionType region;
			region.SetIndex( start );
			region.SetSize(  size  );
			
			importFilter->SetRegion( region );
			
			region.SetSize( size );
			
			typename ImageType::PointType origin;
			origin.Fill( 0.0 );
			
			importFilter->SetOrigin( origin );
			
			
			typename ImportFilterType::SpacingType spacing;
			spacing.Fill( 1.0 );
			
			importFilter->SetSpacing( spacing );
			
			
			const bool importImageFilterWillOwnTheBuffer = false;
			importFilter->SetImportPointer( data1d, numberOfPixels, importImageFilterWillOwnTheBuffer );
			
			typedef itk::BinaryBallStructuringElement< 
			PixelType,
			Dimension  >             
			StructuringElementType;
			
			typedef unsigned char CharPixelType;	
			typedef itk::Image<CharPixelType, Dimension> CharImageType;	
			typedef itk::BinaryErodeImageFilter< ImageType, CharImageType, StructuringElementType > MyBinaryFilterType;
			typename MyBinaryFilterType::Pointer filter_b = MyBinaryFilterType::New();
			typedef itk::GrayscaleErodeImageFilter< ImageType, ImageType, StructuringElementType > MyGrayscaleFilterType;
			typename MyGrayscaleFilterType::Pointer filter_g = MyGrayscaleFilterType::New();
			
			
			//another way that determine b_use_binary based on histogram will be useful in the future
			bool b_use_binary = false; 
			QMessageBox mb;
			mb.setText("Use binary or grayscale filter?");
			mb.addButton(QMessageBox::Yes);
			mb.addButton(QMessageBox::No);
			mb.setButtonText(QMessageBox::Yes, "Binary");
			mb.setButtonText(QMessageBox::No, "Grayscale");
			b_use_binary = (mb.exec()==QMessageBox::Yes) ? true : false;
			
			//	
			if (b_use_binary)	
				filter_b->SetInput( importFilter->GetOutput() );
			else
				filter_g->SetInput( importFilter->GetOutput() );
			
			//filter->InPlaceOn(); // Reuse the buffer
			
			
			//set the SE	
			StructuringElementType  structuringElement;	
			structuringElement.SetRadius( 1 );  // 3x3 structuring element
			structuringElement.CreateStructuringElement();
			
			if (b_use_binary)	
				filter_b->SetKernel(  structuringElement );
			else	
				filter_g->SetKernel(  structuringElement );
			
			//PixelType background =   0;
			PixelType foreground = 255;
			if (b_use_binary)
				filter_b->SetErodeValue( foreground );
			
			//now compute
			
			try
			{
				if (b_use_binary)
				{
					v3d_msg(QObject::tr("Now use binary filter"), 0);
					filter_b->Update(); //the actual computation happens here!
				}
				else {
					v3d_msg(QObject::tr("Now use grayscale filter"), 0);
					filter_g->Update(); 
				}
				
			}
			catch ( itk::ExceptionObject & excp )
			{
				v3d_msg(QObject::tr( excp.what() )); //or use excp.GetDesciption() may do it as well. suggested by Luis!
				return;
			}
			
			//output
			
			if (b_use_binary)
			{
				typename CharImageType::PixelContainer * container;
				
				container = filter_b->GetOutput()->GetPixelContainer();
				container->SetContainerManageMemory( false );
				
				CharPixelType * output1d = container->GetImportPointer();
				
				setPluginOutputAndDisplayUsingGlobalSetting( output1d, nx, ny, nz, 1, callback );
			}
			else 
			{
				typename ImageType::PixelContainer * container;
				
				container = filter_g->GetOutput()->GetPixelContainer();
				container->SetContainerManageMemory( false );
				
				PixelType * output1d = container->GetImportPointer();
				
				setPluginOutputAndDisplayUsingGlobalSetting( output1d, nx, ny, nz, 1, callback );
			}
		}
		else
		{
			v3d_msg(QObject::tr("Unsupported menu string detected. This should never happen. Check with your developer."));
			return;
		}
    }
	
};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
case v3d_pixel_type: \
{ \
ErodeSpecialized< c_pixel_type > runner; \
runner.Execute( menu_name, callback, parent ); \
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

void ErodePlugin::dofunc(const QString & func_name,
						 const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
	// empty by now
}


void ErodePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES; 
	
}

