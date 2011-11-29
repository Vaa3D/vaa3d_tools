#include <QtGui>
#include <math.h>
#include <stdlib.h>

#include "Erode.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkBinaryErodeImageFilter.h"
#include "itkGrayscaleErodeImageFilter.h"

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
class ErodeSpecialized:public V3DITKFilterSingleImage<TPixelType,TPixelType>
{
	typedef V3DITKFilterSingleImage<TPixelType,TPixelType> Superclass;
	typedef typename Superclass::Input3DImageType  ImageType;
	typedef TPixelType  PixelType;
	typedef itk::BinaryBallStructuringElement< PixelType, 3  >             StructuringElementType;
	typedef itk::BinaryErodeImageFilter< ImageType, ImageType, StructuringElementType > MyBinaryFilterType;
			
	typedef itk::GrayscaleErodeImageFilter< ImageType, ImageType, StructuringElementType > MyGrayscaleFilterType;
	
public:
      ErodeSpecialized(V3DPluginCallback *callback):Superclass(callback)
{
	filter_b = MyBinaryFilterType::New();
	filter_g = MyGrayscaleFilterType::New();
	this->RegisterInternalFilter(this->filter_b,1.0);
	this->RegisterInternalFilter(this->filter_g,1.0);	
}
	
	void Execute(const QString &menu_name,  V3DPluginCallback & callback, QWidget *parent)
    {
		if (menu_name == QObject::tr("about this plugin"))
		{
			v3d_msg(QObject::tr("Erode 1.0 (2010-June-2)"));
		}
		else if(menu_name == QObject::tr("Erode"))
		{
			b_use_binary=false;
		        QMessageBox mb;
			mb.setText("Use binary or grayscale filter?");
			mb.addButton(QMessageBox::Yes);
			mb.addButton(QMessageBox::No);
			mb.setButtonText(QMessageBox::Yes, "Binary");
			mb.setButtonText(QMessageBox::No, "Grayscale");
			b_use_binary = (mb.exec()==QMessageBox::Yes) ? true : false;
			
			structuringElement.SetRadius( 1 );  // 3x3 structuring element
			structuringElement.CreateStructuringElement();
			this->Compute();
		}
  			
			
	}

virtual void ComputeOneRegion()
	{
				
			if (b_use_binary)	
				filter_b->SetInput( this->GetInput3DImage() );
			else
				filter_g->SetInput( this->GetInput3DImage() );
				

			
			if (b_use_binary)	
				filter_b->SetKernel(  structuringElement );
			else	
				filter_g->SetKernel(  structuringElement );

			PixelType foreground = 255;
			if (b_use_binary)
				filter_b->SetErodeValue( foreground );
			try
			{
				if (b_use_binary)
				{
					filter_b->Update(); //the actual computation happens here!
				}
				else {
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
				this->SetOutputImage(this->filter_b->GetOutput());
			}
			else 
			{
				this->SetOutputImage(this->filter_g->GetOutput());
			}
		
    }
private:
	bool b_use_binary; 
	typename MyBinaryFilterType::Pointer filter_b;
	typename MyGrayscaleFilterType::Pointer filter_g;
	StructuringElementType   structuringElement;
};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
case v3d_pixel_type: \
{ \
ErodeSpecialized< c_pixel_type > runner(&callback); \
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

