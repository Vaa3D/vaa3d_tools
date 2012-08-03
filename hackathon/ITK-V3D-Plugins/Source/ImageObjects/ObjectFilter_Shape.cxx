#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "ObjectFilter_Shape.h"
#include "V3DITKFilterSingleImage.h"

// ITK Header Files
#include "itkLabelShapeKeepNObjectsImageFilter.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(filterObjectsByShape, ObjectFilter_ShapePlugin)


QStringList ObjectFilter_ShapePlugin::menulist() const
{
    return QStringList() << QObject::tr("Object Filter Shape")
  << QObject::tr("about this plugin");
}

QStringList ObjectFilter_ShapePlugin::funclist() const
{
    return QStringList();
}


template <typename TInPixelType, typename TOutputPixelType>
class MySpecialized : public V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >
{
  typedef V3DITKFilterSingleImage< TInPixelType, TOutputPixelType >   Superclass;
  typedef typename Superclass::Input3DImageType               ImageType;
//  typedef typename Superclass::Output3DImageType              OutputImageType;

  typedef itk::LabelShapeKeepNObjectsImageFilter< ImageType > FilterType;

public:

  MySpecialized( V3DPluginCallback * callback ): Superclass(callback)
    {
    this->m_Filter = FilterType::New();
	this->RegisterInternalFilter(this->m_Filter,1.0);
    }

  virtual ~MySpecialized() {};


  void Execute(const QString &menu_name, QWidget *parent)
    {
	this->SetupParameters();
    this->Compute();
    }

  virtual void ComputeOneRegion()
    {

    this->m_Filter->SetInput( this->GetInput3DImage() );

    if( !this->ShouldGenerateNewWindow() )
    {
      //this->m_Filter->InPlaceOn();
    }

    this->m_Filter->Update();

    this->SetOutputImage( this->m_Filter->GetOutput() );
    }

  virtual void SetupParameters()
    {
    //
    // These values should actually be provided by the Qt Dialog...
    //

    // just search the respective .h file for the itkSetMacro for parameters

//    V3D_Simple_Parameter_Dialog d;
//    d.addParameterLine( "Background Value", unsigned char);

    this->m_Filter->SetBackgroundValue( 0 );
    this->m_Filter->SetNumberOfObjects( 10 ); //only output 10 objects
    this->m_Filter->SetReverseOrdering( false );
    this->m_Filter->SetAttribute("Size");

    //the following are from itkShapeLabelObject.h

//    itkStaticConstMacro(SIZE, AttributeType, 100);
//    itkStaticConstMacro(PHYSICAL_SIZE, AttributeType, 101);
//    itkStaticConstMacro(REGION_ELONGATION, AttributeType, 102);
//    itkStaticConstMacro(SIZE_REGION_RATIO, AttributeType, 103);
//    itkStaticConstMacro(CENTROID, AttributeType, 104);
//    itkStaticConstMacro(REGION, AttributeType, 105);
//    itkStaticConstMacro(SIZE_ON_BORDER, AttributeType, 106);
//    itkStaticConstMacro(PHYSICAL_SIZE_ON_BORDER, AttributeType, 107);
//    itkStaticConstMacro(FERET_DIAMETER, AttributeType, 108);
//    itkStaticConstMacro(BINARY_PRINCIPAL_MOMENTS, AttributeType, 109);
//    itkStaticConstMacro(BINARY_PRINCIPAL_AXES, AttributeType, 110);
//    itkStaticConstMacro(BINARY_ELONGATION, AttributeType, 111);
//    itkStaticConstMacro(PERIMETER, AttributeType, 112);
//    itkStaticConstMacro(ROUNDNESS, AttributeType, 113);
//    itkStaticConstMacro(EQUIVALENT_RADIUS, AttributeType, 114);
//    itkStaticConstMacro(EQUIVALENT_PERIMETER, AttributeType, 115);
//    itkStaticConstMacro(EQUIVALENT_ELLIPSOID_RADIUS, AttributeType, 116);
//    itkStaticConstMacro(BINARY_FLATNESS, AttributeType, 117);

  }

private:

    typename FilterType::Pointer   m_Filter;

};


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE( v3d_pixel_type, c_pixel_type ) \
case v3d_pixel_type: \
{ \
MySpecialized< c_pixel_type, c_pixel_type > runner( &callback ); \
runner.Execute( menu_name, parent ); \
break; \
}


void ObjectFilter_ShapePlugin::dofunc(const QString & func_name,
                   const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void ObjectFilter_ShapePlugin::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
  if (menu_name == QObject::tr("about this plugin"))
    {
    QMessageBox::information(parent, "Version info", "ITK Filter Objects based on their shape 1.0 (2010-June-4): this plugin is developed by Hanchuan Peng.");
    return;
    }

  v3dhandle curwin = callback.currentImageWindow();
  if (!curwin)
    {
    v3d_msg(tr("You don't have any image open in the main window."));
    return;
    }

  Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage)
    {
    v3d_msg(tr("The input image is null."));
    return;
    }

  EXECUTE_PLUGIN_FOR_ALL_PIXEL_TYPES;
}

