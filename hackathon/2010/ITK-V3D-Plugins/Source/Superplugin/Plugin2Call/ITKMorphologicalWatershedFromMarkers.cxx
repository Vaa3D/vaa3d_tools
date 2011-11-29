/* ITKMorphologicalWatershedFromMarkers.cxx
 * 2010-06-04: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKMorphologicalWatershedFromMarkers.h"
#include "V3DITKFilterDualImage.h"

// ITK Header Files
#include "itkCastImageFilter.h"
#include "itkRelabelComponentImageFilter.h"

#include "itkMorphologicalWatershedFromMarkersImageFilter.h"

// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKMorphologicalWatershedFromMarkers, ITKMorphologicalWatershedFromMarkersPlugin)

//plugin funcs
const QString title = "ITK MorphologicalWatershedFromMarkers";
QStringList ITKMorphologicalWatershedFromMarkersPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK MorphologicalWatershedFromMarkers")
						 << QObject::tr("about this plugin");
}

QStringList ITKMorphologicalWatershedFromMarkersPlugin::funclist() const
{
    return QStringList()<< QObject::tr("ITK MorphologicalWatershedFromMarkers")
						 << QObject::tr("about this plugin");
}


template <typename TInputPixelType>
class ITKMorphologicalWatershedFromMarkersSpecializaed  : public V3DITKFilterDualImage< TInputPixelType, TInputPixelType >
{
public:
	typedef V3DITKFilterDualImage< TInputPixelType,TInputPixelType >    Superclass;
	typedef typename Superclass:: Input3DImageType 		InputImageType;
	typedef itk::Image<float,3> 				FImageType;
	typedef itk::CastImageFilter< InputImageType, FImageType>  CastImageFilterType;
	typedef itk::MorphologicalWatershedFromMarkersImageFilter< FImageType, FImageType>  MorphologicalWatershedFromMarkersType;
	typedef typename MorphologicalWatershedFromMarkersType::LabelImageType  LabeledImageType;
			
	typedef itk::RelabelComponentImageFilter< LabeledImageType, InputImageType > RelabelComponentImageFilterType;
	
       ITKMorphologicalWatershedFromMarkersSpecializaed( V3DPluginCallback * callback ): Superclass(callback) 
	{
		castImageFilter1 = CastImageFilterType::New();
		castImageFilter2 = CastImageFilterType::New();
		watershedFilter = MorphologicalWatershedFromMarkersType::New();
		relabelComponent = RelabelComponentImageFilterType::New();
		this->RegisterInternalFilter(this->watershedFilter,0.7);
		this->RegisterInternalFilter(this->relabelComponent,0.2);
		this->RegisterInternalFilter(this->castImageFilter1,0.05);
		this->RegisterInternalFilter(this->castImageFilter1,0.05);
	}
	virtual ~ITKMorphologicalWatershedFromMarkersSpecializaed() {};
	
	//
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		this->SetImageSelectionDialogTitle("ITKMorphologicalWatershed");
		this->AddImageSelectionLabel("Image 1");
		this->AddImageSelectionLabel("Image 2");
		this->m_ImageSelectionDialog.SetCallback(this->m_V3DPluginCallback);
		watershedFilter->SetMarkWatershedLine( true ); // true: Meyer, false: Beucher.
		watershedFilter->SetFullyConnected( true );
		this->Compute();
	}
	virtual void ComputeOneRegion()
	{
		castImageFilter1->SetInput( this->GetInput3DImage1());
		castImageFilter2->SetInput( this->GetInput3DImage2());
		watershedFilter->SetInput( castImageFilter1->GetOutput() );
		watershedFilter->SetMarkerImage( castImageFilter2->GetOutput() );
		relabelComponent->SetInput( watershedFilter->GetOutput() );
		relabelComponent->Update();
		this->SetOutputImage(relabelComponent->GetOutput());
	}
void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {
       	V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

       	this->AddObserver( progressDialog.GetCommand() );
       	progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
       	progressDialog.show();
	this->RegisterInternalFilter(this->watershedFilter,0.7);
	this->RegisterInternalFilter(this->relabelComponent,0.2);
	this->RegisterInternalFilter(this->castImageFilter1,0.05);
	this->RegisterInternalFilter(this->castImageFilter1,0.05);
	watershedFilter->SetMarkWatershedLine( true ); // true: Meyer, false: Beucher.
	watershedFilter->SetFullyConnected( true );


	void * p1=NULL;
	void * p2=NULL;
	p1=input.at(0).p;
	p2=input.at(1).p;
	
        this->castImageFilter1->SetInput((InputImageType*) p1 );
	this->castImageFilter2->SetInput((InputImageType*) p2 );
	watershedFilter->SetInput( castImageFilter1->GetOutput() );
	watershedFilter->SetMarkerImage( castImageFilter2->GetOutput() );
	relabelComponent->SetInput( watershedFilter->GetOutput() );
	relabelComponent->Update();

	V3DPluginArgItem arg;
	arg.p=relabelComponent->GetOutput();
	arg.type="outputImage";
	output.replace(0,arg);
    }

		
private:
	typename CastImageFilterType::Pointer castImageFilter1 ;
	typename CastImageFilterType::Pointer castImageFilter2 ;
	typename MorphologicalWatershedFromMarkersType::Pointer watershedFilter ;
	typename RelabelComponentImageFilterType::Pointer relabelComponent ;
		
	
};

#define EXECUTE( v3d_pixel_type, input_pixel_type) \
	case v3d_pixel_type: \
	{ \
		ITKMorphologicalWatershedFromMarkersSpecializaed< input_pixel_type > runner(&callback); \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
		EXECUTE( V3D_UINT8, unsigned char);  \
		EXECUTE( V3D_UINT16, unsigned short int);  \
		EXECUTE( V3D_FLOAT32, float);  \
		case V3D_UNKNOWN:  \
		{  \
		}  \
	}  

bool ITKMorphologicalWatershedFromMarkersPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
  if (func_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
    return false ;
    }
	ITKMorphologicalWatershedFromMarkersSpecializaed <unsigned char> *runner=new ITKMorphologicalWatershedFromMarkersSpecializaed<unsigned char>(&v3d);
	runner->ComputeOneRegion(input, output); 
 
	return true;
}

void ITKMorphologicalWatershedFromMarkersPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (menu_name == QObject::tr("ITK MorphologicalWatershedFromMarkers"))
    {
    	v3dhandle curwin = callback.currentImageWindow();
		if (!curwin)
		{
			v3d_msg(QObject::tr("You don't have any image open in the main window."));
			return;
		}
		
		Image4DSimple *p4DImage = callback.getImage(curwin);
		if (! p4DImage)
		{
			v3d_msg(QObject::tr("The input image is null."));
			return;
		}	
		
		EXECUTE_ALL_PIXEL_TYPES;
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Morphological Watershed From Markers 1.0 (2010-June-04): this plugin is developed by Yang Yu.");
	}
}



