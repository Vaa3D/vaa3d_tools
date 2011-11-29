#ifndef __V3DITKPluginBaseSingleImage_H__
#define __V3DITKPluginBaseSingleImage_H__


#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>
#include "V3DITKFilterSingleImage.h"


#define V3DITK_PLUGIN_EXECUTE( PluginName, v3d_pixel_type, c_pixel_type ) \
  case v3d_pixel_type: \
    { \
    PluginName##Specialized< c_pixel_type > runner( &callback ); \
    runner.Execute( menu_name, parent ); \
    break; \
    }

#define EXECUTE_ALL_PIXEL_TYPES( PluginName ) \
    v3dhandle curwin = callback.currentImageWindow(); \
    Image4DSimple *p4DImage = callback.getImage(curwin); \
    ImagePixelType pixelType = p4DImage->getDatatype(); \
    switch( pixelType )  \
      {  \
      V3DITK_PLUGIN_EXECUTE( PluginName, V3D_UINT8, unsigned char );  \
      V3DITK_PLUGIN_EXECUTE( PluginName, V3D_UINT16, unsigned short int );  \
      V3DITK_PLUGIN_EXECUTE( PluginName, V3D_FLOAT32, float );  \
      case V3D_UNKNOWN:  \
        {  \
        }  \
      }

class V3DITKPluginBaseSingleImage : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	V3DITKPluginBaseSingleImage();
	virtual ~V3DITKPluginBaseSingleImage();

	void SetPluginName( const char * name );
        QString GetPluginName() const;

        QStringList menulist() const;
	QStringList funclist() const;

	virtual void dofunc(const QString & func_name,const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);

	virtual void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent);

        bool Initialize( V3DPluginCallback & callback );

private:

  std::string    m_PluginName;

};

#endif
