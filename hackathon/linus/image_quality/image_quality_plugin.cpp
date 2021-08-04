/* image_quality_plugin.cpp
 * Obtain image quality features
 * 2021-08-04 : by Linus Manubens-Gil
 */
 
#include "v3d_message.h"

#include "image_quality_plugin.h"
#include "image_quality_func.h"
#include <iostream>
using namespace std;
 
Q_EXPORT_PLUGIN2(imagequality, ImageQualityPlugin);
 
QStringList ImageQualityPlugin::menulist() const
{
	return QStringList() 
                <<tr("Image Quality")
		<<tr("about");
}

QStringList ImageQualityPlugin::funclist() const
{
	return QStringList()
        <<tr("image_quality")
		<<tr("help");
}

void ImageQualityPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("Image Quality"))
	{
		compute(callback,parent);
	}
	else
	{
        v3d_msg(tr("Obtain image quality features. "
            "Developed by Linus Manubens Gil, 2021-08-04"));
	}
}

bool ImageQualityPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("image_quality"))
	{
        return compute(callback, input, output);
	}
	else
	{
        cout<<"\nThis is a plugin to get image quality features. 2021-08-04 by Linus Manubens Gil"<<endl;
        cout<<"\nUsage: v3d -x image_quality -f image_quality -i <image_name> -o <outputfile_name>"<<endl;
		cout<<"\t -i <image_name> :       name of the image to be computed"<<endl;
		cout<<"\t -o <outputfile_name> :  output file name (.csv)"<<endl;
        cout<<"\nDemo v3d -x mage_quality -f mage_quality -i test.tif -o hist.csv\n"<<endl;
	}
}

