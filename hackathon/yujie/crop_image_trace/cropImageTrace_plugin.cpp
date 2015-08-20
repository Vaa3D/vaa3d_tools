/* cropImageTrace_plugin.cpp
 * This plug-in can crop both image and trace
 * 2015-8-13 : by Yujie Li
 */
 
#include "v3d_message.h"
#include "cropImageTrace_plugin.h"
#include "crop_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(cropImageTrace, cropImageTrace_plugin);
 
QStringList cropImageTrace_plugin::menulist() const
{
	return QStringList() 
		<<tr("crop")
		<<tr("about");
}

QStringList cropImageTrace_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void cropImageTrace_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("crop"))
	{
        crop_dialog *dialog = new crop_dialog(&callback);
        dialog->getData();
	}
	else
	{
		v3d_msg(tr("This plug-in can crop both image and trace. "
			"Developed by Yujie Li, 2015-8-13"));
	}
}

bool cropImageTrace_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
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

