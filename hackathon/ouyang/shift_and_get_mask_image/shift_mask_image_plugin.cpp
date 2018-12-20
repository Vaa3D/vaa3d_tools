/* shift_mask_image_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-16 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "shift_mask_image_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(shift_mask_image, shift_mask2D);
 
QStringList shift_mask2D::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList shift_mask2D::funclist() const
{
	return QStringList()
        <<tr("shift_mask")
		<<tr("func2")
		<<tr("help");
}

void shift_mask2D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-12-16"));
	}
}

bool shift_mask2D::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
    QString output_image=outfiles.at(0);


    if (func_name == tr("shift_mask"))
	{
        get_terminal_signal(input_swc,input_image,output_image,callback);
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

