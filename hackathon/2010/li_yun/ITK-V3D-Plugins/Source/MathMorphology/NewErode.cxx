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
class ErodeSpecialized : public V3DITKFilterSingleImage< TPixelType, TPixelType > 
{
	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >  				Superclass;
	typedef TPixelType  									PixelType;
	typedef itk::Image< PixelType, 3 > 							ImageType;
	typedef itk::BinaryBallStructuringElement< PixelType, 3 > 				StructuringElementType;	
	typedef itk::BinaryErodeImageFilter< ImageType, ImageType, StructuringElementType > 	MyBinaryFilterType;	
	typedef itk::GrayscaleErodeImageFilter< ImageType, ImageType, StructuringElementType > 	MyGrayscaleFilterType;
	
public:
	ErodeSpecialized( V3DPluginCallback * callback ): Superclass(callback)
	{
		this->filter_b = MyBinaryFilterType::New();
		this->filter_g = MyGrayscaleFilterType::New();
		this->b_use_binary = false;

		this->RegisterInternalFilter( this->filter_b, 1.0 );
		this->RegisterInternalFilter( this->filter_g, 1.0 );
    		
	}

	virtual ~ErodeSpecialized() {};

	void Execute(const QString &menu_name, V3DPluginCallback & callback, QWidget *parent)
	{
		SetupParameters();
        	this->Compute();
	}

	virtual void CmputeOneRegion()
        {
		if (b_use_binary)	
			filter_b->SetInput( this->GetInput3DImage() );
		else
			filter_g->SetInput( this->GetInput3DImage() );
		//set the SE	
		StructuringElementType  structuringElement;	
		structuringElement.SetRadius( 1 );  // 3x3 structuring element
		structuringElement.CreateStructuringElement();
	
		if (b_use_binary)	
		{
			filter_b->SetKernel(  structuringElement );
		        filter_b->Update();
		        this->SetOutputImage( filter_b->GetOutput() );
		}
		else	
		{
			filter_g->SetKernel(  structuringElement );
		        filter_g->Update();
		        this->SetOutputImage( filter_g->GetOutput() );
	        }
 	}
	virtual void SetupParameters()
	{
	    V3DITKGenericDialog dialog("Erode");

	    dialog.AddDialogElement("UseBinary", 0.0, 0.0, 1.0);
	    if( dialog.exec() == QDialog::Accepted )
		    b_use_binary = dialog.GetValue("UseBinary");
	    if(b_use_binary)
	    {
	     	V3DITKGenericDialog dlg("SetErodeValue");

		dlg.AddDialogElement("Foreground", 255.0, 0.0, 255.0);

		if( dlg.exec() == QDialog::Accepted )
		    filter_b->SetErodeValue( dlg.GetValue("Foreground") );
	    }
	}
private:
	typename MyBinaryFilterType::Pointer filter_b;
	typename MyGrayscaleFilterType::Pointer filter_g;
	bool b_use_binary;

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    ErodeSpecialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, callback, parent ); \
    break; \
    } 

#define EXECUTE_ALL_PIXEL_TYPES \
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
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "Erode 1.0 (2010-June-2)");
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

