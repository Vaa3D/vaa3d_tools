#include <QtGui>
#include <math.h>
#include <stdlib.h>

#include "Registration.h"
#include "V3DITKFilterDualImage.h"
#include "RegistrationDlg.h"

Q_EXPORT_PLUGIN2(RegistrationIntegrated,Registration)

QStringList Registration::menulist()const
{
	return QStringList()<<QObject::tr("RegistrationIntegrated")
			    <<QObject::tr("about this plugin");
}

QStringList Registration::funclist()const
{
	return QStringList();
}


#define EXECUTE_PLUGIN_FOR_ONE_IMAGE_TYPE(v3d_pixel_type,c_pixel_type) \
	case v3d_pixel_type:\
	{ \
		PluginSpecialized<c_pixel_type> runner(&callback);\
		runner.Execute(menu_name,parent);\
		break; \
	}

void Registration::dofunc(const QString & fuc_name,const V3DPluginArgList & input,V3DPluginArgList & output,QWidget* parent)
{
}
void Registration::domenu(const QString & menu_name,V3DPluginCallback & callback,QWidget *parent)
{
	if(menu_name==QObject::tr("about this plugin"))
	{
		QMessageBox::information(parent,"Vertion info","Registration by Li Yun");
		return;
	}
	v3dhandle curwin =callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg(tr("You don't have any image open in the main window"));
		return;
	}
	Image4DSimple *p4DImage=callback.getImage(curwin);
	if(!p4DImage)
	{
		v3d_msg(tr("The input image is null."));
		return;
	}
        RegistrationDlg mydialog;        
        mydialog.SetCallback(callback);
        mydialog.exec();
}
