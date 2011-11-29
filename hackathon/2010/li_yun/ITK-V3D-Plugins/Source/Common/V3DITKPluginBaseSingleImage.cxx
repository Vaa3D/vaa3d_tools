#include "V3DITKPluginBaseSingleImage.h"


// Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
// The value of PluginName should correspond to the TARGET specified in the
// plugin's project file.
Q_EXPORT_PLUGIN2(V3DITKCommon, V3DITKPluginBaseSingleImage )


V3DITKPluginBaseSingleImage::V3DITKPluginBaseSingleImage()
{
}

V3DITKPluginBaseSingleImage::~V3DITKPluginBaseSingleImage()
{
}

void V3DITKPluginBaseSingleImage::SetPluginName( const char * name )
{
  this->m_PluginName = name;
}

QString V3DITKPluginBaseSingleImage::GetPluginName() const
{
  return QObject::tr( this->m_PluginName.c_str() );
}


QStringList V3DITKPluginBaseSingleImage::menulist() const
{
    return QStringList() << QObject::tr( this->m_PluginName.c_str() )
						<< QObject::tr("about this plugin");
}

QStringList V3DITKPluginBaseSingleImage::funclist() const
{
    return QStringList();
}

void V3DITKPluginBaseSingleImage::dofunc(const QString & func_name,
		const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent)
{
  // empty by now
}


void V3DITKPluginBaseSingleImage
::domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent)
{
}

bool V3DITKPluginBaseSingleImage::Initialize( V3DPluginCallback & callback )
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
		v3d_msg(tr("You don't have any image open in the main window."));
		return false;
    }

	Image4DSimple *p4DImage = callback.getImage(curwin);
  if (! p4DImage || !p4DImage->valid() )
    {
    v3d_msg(tr("The input image is null."));
    return false;
    }

  return true;
}

