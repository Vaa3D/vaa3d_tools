/* detect_bouton_plugin.cpp
 * This plugin detects bouton along a neuron reconstruction
 * 2018-11-20 : by Peng Xie
 */
 
#include "detect_bouton_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(detect_bouton, detect_bouton_plugin);
 
QStringList detect_bouton_plugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList detect_bouton_plugin::funclist() const
{
	return QStringList()
        <<tr("detect_bouton")
        <<tr("get_terminal")
        <<tr("get_terminal_signal")
		<<tr("help");
}

void detect_bouton_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
		v3d_msg(tr("This plugin detects bouton along a neuron reconstruction. "
			"Developed by Peng Xie, 2018-11-20"));
	}
}

bool detect_bouton_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString image=infiles.at(0);
    QString swc=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_apo;
    XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }

    if (func_name == tr("detect_bouton"))
    {
        if(output_apo.size()==0){return true;}
        detect_bouton_pipeline(image, swc, output_dir, output_apo, callback);
	}
    else if (func_name == tr("get_terminal"))
	{
        get_terminal(image, swc, output_dir, block_size, callback);
	}
    else if (func_name == tr("get_terminal_signal"))
    {
        get_terminal_signal(swc, image, callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

