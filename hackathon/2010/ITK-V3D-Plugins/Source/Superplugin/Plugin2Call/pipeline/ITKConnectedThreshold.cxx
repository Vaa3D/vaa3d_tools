/* ITKConnectedThreshold.cxx
 * 2010-06-03: create this program by Yang Yu
 */


#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ITKConnectedThreshold.h"
#include "V3DITKFilterSingleImage.h"
// ITK Header Files

#include "itkConnectedThresholdImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCurvatureFlowImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(ITKConnectedThreshold, ITKConnectedThresholdPlugin)

void itkConnectedThresholdPlugin(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "ITK ConnectedThreshold";
QStringList ITKConnectedThresholdPlugin::menulist() const
{
	return QStringList() << QObject::tr("ITK ConnectedThreshold")
	<< QObject::tr("about this plugin");
}
QStringList ITKConnectedThresholdPlugin::funclist() const
{
	return QStringList() << QObject::tr("ITK ConnectedThreshold")
	<< QObject::tr("about this plugin");
}

void ITKConnectedThresholdPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == QObject::tr("ITK ConnectedThreshold"))
    {
    	itkConnectedThresholdPlugin(callback, parent);
    }
	else if (menu_name == QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "ITK Connected Threshold 1.0 (2010-June-03): this plugin is developed by Yang Yu.");
	}
}


template <typename TInputPixelType, typename TOutputPixelType>
class ITKConnectedThresholdSpecializaed:public V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>
{
public:
	typedef V3DITKFilterSingleImage<TInputPixelType,TOutputPixelType>  Superclass;
	typedef typename Superclass ::Input3DImageType InputImageType;
	typedef typename Superclass ::Output3DImageType OutputImageType;
	typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
	typedef itk::CastImageFilter< OutputImageType,InputImageType>  CastOutputType;

	typedef itk::CurvatureFlowImageFilter< OutputImageType, OutputImageType > CurvatureFlowImageFilterType;

	typedef itk::ConnectedThresholdImageFilter<OutputImageType, OutputImageType>   ConnectedFilterType;

ITKConnectedThresholdSpecializaed(V3DPluginCallback*callback):Superclass(callback)
{
	castImageFilter = CastImageFilterType::New();
	smoothing = CurvatureFlowImageFilterType::New();
	connectedThreshold = ConnectedFilterType::New();
	castOutputFilter=CastOutputType::New();
	this->RegisterInternalFilter(this->connectedThreshold,0.7);
	this->RegisterInternalFilter(this->smoothing,0.2);
	this->RegisterInternalFilter(this->castImageFilter,0.1);

}
	 	
	
	void Execute(V3DPluginCallback &callback, QWidget *parent)
	{
		
			v3dhandleList win_list = callback.getImageWindowList();
			
			LandmarkList list_landmark_sub=callback.getLandmark(win_list[0]);
			typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]
			if(list_landmark_sub.size()<1)
			{
				v3d_msg(QObject::tr("You should select one seed from your image."));
				return;
			}
			else
			{
			    //seeds
				for(int i=0;  i<list_landmark_sub.size(); i++)
				{
					//
					seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
					seedPosition[1] = list_landmark_sub[i].y -1;
					seedPosition[2] = list_landmark_sub[i].z -1;
					
					connectedThreshold->AddSeed( seedPosition );
				}
				V3DITKGenericDialog dialog("ITKConnectedThreshold");

    			 	dialog.AddDialogElement("lowerThreshold",210.0, 0.0, 255.0);
   				dialog.AddDialogElement("upperThreshold",255.0, 0.0, 255.0);

    				if( dialog.exec() == QDialog::Accepted )
      				{
  
				const TOutputPixelType lowerThreshold = dialog.GetValue("lowerThreshold"); //mean
				const TOutputPixelType upperThreshold = dialog.GetValue("upperThreshold");
			  	smoothing->SetNumberOfIterations( 5 );
				smoothing->SetTimeStep( 0.125 );
					
				connectedThreshold->SetLower(  lowerThreshold  );
				connectedThreshold->SetUpper(  upperThreshold  );
					
				connectedThreshold->SetReplaceValue( 255 );
				this->Compute();
				}
			}
			
	}
	virtual void ComputeOneRegion()
	{
				
		castImageFilter->SetInput(this->GetInput3DImage());
		smoothing->SetInput( castImageFilter->GetOutput() );
		connectedThreshold->SetInput( smoothing->GetOutput() );
		try
			{
						
			 connectedThreshold->Update();
			}
			catch( itk::ExceptionObject & excp)
			{
				std::cerr << "Error run this filter." << std::endl;
				std::cerr << excp << std::endl;
				return;
			}
		this->SetOutputImage(this->connectedThreshold->GetOutput());
	}
void ComputeOneRegion(const V3DPluginArgList & input, V3DPluginArgList & output)
    {

	v3dhandleList win_list = this->m_V3DPluginCallback->getImageWindowList();
			
	LandmarkList list_landmark_sub=this->m_V3DPluginCallback->getLandmark(win_list[0]);
	typename OutputImageType::IndexType  seedPosition; // seedPosition[0]  seedPosition[1]  seedPosition[2]
	if(list_landmark_sub.size()<1)
	{
		v3d_msg(QObject::tr("You should select one seed from your image."));
		return;
	}
	else
	{

	 for(int i=0;  i<list_landmark_sub.size(); i++)
	   {
					
		seedPosition[0] = list_landmark_sub[i].x -1; // notice 0-based and 1-based difference
		seedPosition[1] = list_landmark_sub[i].y -1;
		seedPosition[2] = list_landmark_sub[i].z -1;
					
		connectedThreshold->AddSeed( seedPosition );
	    }
		V3DITKGenericDialog dialog("ITKConnectedThreshold");

    		dialog.AddDialogElement("lowerThreshold",210.0, 0.0, 255.0);
   		dialog.AddDialogElement("upperThreshold",255.0, 0.0, 255.0);

    		if( dialog.exec() == QDialog::Accepted )
      		{
  
			const TOutputPixelType lowerThreshold = dialog.GetValue("lowerThreshold"); //mean
			const TOutputPixelType upperThreshold = dialog.GetValue("upperThreshold");
					
			connectedThreshold->SetLower(  lowerThreshold  );
			connectedThreshold->SetUpper(  upperThreshold  );
					
			connectedThreshold->SetReplaceValue( 255 );
				
       			V3DITKProgressDialog progressDialog( this->GetPluginName().toStdString().c_str() );

       			this->AddObserver( progressDialog.GetCommand() );
       			progressDialog.ObserveFilter( this->m_ProcessObjectSurrogate );
       			progressDialog.show();
			this->RegisterInternalFilter( this->connectedThreshold, 1.0 );

			void * p=NULL;
			p=(void*)input.at(0).p;
			if(!p)perror("errro");
			this->castImageFilter->SetInput((InputImageType*) p );
			this->connectedThreshold->SetInput(this->castImageFilter->GetOutput());
			this->castOutputFilter->SetInput(this->connectedThreshold->GetOutput());

			this->castOutputFilter->Update();
			V3DPluginArgItem arg;
			arg.p=castOutputFilter->GetOutput();
			arg.type="UINT8Image";
			output.replace(0,arg);
		}
	}
    }	
	
private:
	typename CastImageFilterType::Pointer castImageFilter;
	typename CurvatureFlowImageFilterType::Pointer smoothing;
	typename ConnectedFilterType::Pointer connectedThreshold;
	typename CastOutputType::Pointer	castOutputFilter;
	
};


#define EXECUTE( v3d_pixel_type, input_pixel_type, output_pixel_type ) \
	case v3d_pixel_type: \
	{ \
		ITKConnectedThresholdSpecializaed< input_pixel_type, output_pixel_type > runner(&callback); \
		runner.Execute( callback, parent ); \
		break; \
	} 

#define EXECUTE_ALL_PIXEL_TYPES \
	if (! p4DImage) return; \
	ImagePixelType pixelType = p4DImage->getDatatype(); \
	switch( pixelType )  \
	{  \
		EXECUTE( V3D_UINT8, unsigned char, float );  \
		EXECUTE( V3D_UINT16, unsigned short int, float );  \
		EXECUTE( V3D_FLOAT32, float, float );  \
		case V3D_UNKNOWN:  \
		{  \
		}  \
	}  
bool ITKConnectedThresholdPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "New Pugin for Other(developed by Yu Ping");
    return false ;
    }
	ITKConnectedThresholdSpecializaed<unsigned char,float> *runner=new ITKConnectedThresholdSpecializaed<unsigned char,float>(&v3d);
	runner->ComputeOneRegion(input, output); 
	return true;
}


void itkConnectedThresholdPlugin(V3DPluginCallback &callback, QWidget *parent)
{
	Image4DSimple* p4DImage = callback.getImage(callback.currentImageWindow());
	if (!p4DImage)
    {
		v3d_msg(QObject::tr("You don't have any image open in the main window."));
		return;
    }
	
	EXECUTE_ALL_PIXEL_TYPES;
}


