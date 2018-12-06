/* ML_get_sample_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by OYQ
 */
 
#include "v3d_message.h"
#include <vector>
#include "ML_get_sample_plugin.h"
#include "get_tip_block.h"
using namespace std;
Q_EXPORT_PLUGIN2(ML_get_sample, ML_sample);
 
QStringList ML_sample::menulist() const
{
	return QStringList() 
        <<tr("get_ML_sample")
		<<tr("about");
}

QStringList ML_sample::funclist() const
{
	return QStringList()
        <<tr("get_ML_sample")
		<<tr("help");
}

void ML_sample::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_ML_sample"))
	{
        v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by OYQ, 2018-12-6"));
	}
}

bool ML_sample::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

    if (func_name == tr("get_ML_sample"))
	{
        get_terminal(image, swc, output_dir,block_size, callback);
	}
	else if (func_name == tr("help"))
	{
        printHelp(input,output);
	}
	else return false;

	return true;
}

