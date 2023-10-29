/* ImageLib_plugin.cpp
 * for the process of image data
 * 2023-7-5 : by SD_Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "ImageLib_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(ImageLib, ImageLib);
 
QStringList ImageLib::menulist() const
{
	return QStringList() 
        <<tr("menu1")
		<<tr("about");
}

QStringList ImageLib::funclist() const
{
	return QStringList()
        <<tr("func1")
		<<tr("help");
}

void ImageLib::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
    }
	else
	{
		v3d_msg(tr("for the process of image data. "
			"Developed by SD_Jiang, 2023-7-5"));
	}
}

bool ImageLib::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

