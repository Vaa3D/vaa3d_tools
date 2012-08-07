/* MeanFilter.cpp
 * 2010-06-03: create this program by Lei Qu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "MeanFilter.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files

#include "itkMeanImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(MeanFilter, ITKMeanFilterPlugin)

QStringList ITKMeanFilterPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK Mean Filter ...");
}

template<typename TPixelType>
class ITKMeanFilterSpecializaed:public V3DITKFilterSingleImage< TPixelType,TPixelType>
{
public:
  	typedef V3DITKFilterSingleImage< TPixelType, TPixelType >   Superclass;
  	typedef typename Superclass::Input3DImageType               ImageType;
	typedef itk::MeanImageFilter<ImageType,ImageType> FilterType;

	ITKMeanFilterSpecializaed( V3DPluginCallback * callback ): Superclass(callback)
	{
		m_Filter=FilterType::New();
		this->RegisterInternalFilter(this->m_Filter,1.0);
	}
	virtual ~ITKMeanFilterSpecializaed(){}
	
	void Execute(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
	{
		
		typename ImageType::SizeType indexRadius;
		indexRadius[0] = 1; // radius along x
		indexRadius[1] = 1; // radius along y
		indexRadius[2] = 1; // radius along y
		m_Filter->SetRadius( indexRadius );
		this->Compute();
	}
	virtual void ComputeOneRegion()
	{
	 	this->m_Filter->SetInput(this->GetInput3DImage());
 		this->m_Filter->Update();

               this->SetOutputImage( this->m_Filter->GetOutput() );
	}
private:
	typename FilterType ::Pointer m_Filter;
		

};

#define EXECUTE( v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
	ITKMeanFilterSpecializaed< c_pixel_type > runner(&callback); \
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

void ITKMeanFilterPlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return;
    }

  EXECUTE_ALL_PIXEL_TYPES;
}
