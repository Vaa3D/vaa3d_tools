/* tip_signal_plugin.cpp
 * This plugin detects tip signal along a neuron reconstruction
 * 2018-11-20 : by Peng Xie
 */
 
#include "tip_signal_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(tip_signal, tip_signal_plugin);
 
QStringList tip_signal_plugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList tip_signal_plugin::funclist() const
{
	return QStringList()
        <<tr("get_terminal")
        <<tr("get_terminal_signal")
        <<tr("get_all_terminal_signal")
        <<tr("help");
}

void tip_signal_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
        v3d_msg(tr("This plugin detects tip signal along a neuron reconstruction. "
			"Developed by Peng Xie, 2018-11-20"));
	}
}

bool tip_signal_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) {
        infiles = *((vector<char*> *)input.at(0).p);
        qDebug()<<input.size()<<infiles.size();
    }
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    XYZ block_size=XYZ(100,100,20);
    cout<<"tip_signal input_size\t"<<input.size()<<endl;

    if (func_name == tr("get_terminal_by_apo"))
	{
        QString image=infiles.at(0);
        QString apo=infiles.at(1);
        QString output_dir=outfiles.at(0);
        crop_by_apo(image, apo, output_dir, block_size, callback);
	}
    else if (func_name == tr("get_terminal"))
    {
        QString image=infiles.at(0);
        QString swc=infiles.at(1);
        QString output_dir=outfiles.at(0);
        get_terminal(image, swc, output_dir, block_size, callback);
    }
    else if (func_name == tr("get_terminal_signal"))
    {
        QString image=infiles.at(0);
        QString swc=infiles.at(1);
        QList<double> terminal_signal = get_terminal_signal(swc, image, callback);
    }
    else if (func_name == tr("get_all_terminal_signal"))
    {
        QString input_folder = infiles.at(0);
        QString output_file = outfiles.at(0);
        qDebug()<<input_folder<<output_file;
        get_all_terminal_signal(input_folder, output_file, callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

