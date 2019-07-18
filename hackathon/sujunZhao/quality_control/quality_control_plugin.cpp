/* quality_control_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-15 : by ZSJ
 */
 
#include "quality_control_plugin.h"
#include "feature_analysis.h"

using namespace std;
Q_EXPORT_PLUGIN2(quality_control, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("quality control")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("feature_analysis")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("quality control"))
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
			"Developed by ZSJ, 2019-5-15"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
//	vector<char*> infiles, inparas, outfiles;
//	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("feature_analysis"))
	{
        Process(input,output,callback);
	}
    else if (func_name == tr("test"))
	{
        vector<char*> in, inparas, outfiles;
        if(input.size() >= 1) in = *((vector<char*> *)input.at(0).p);
        if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
        bool hasOutput;
        if(output.size() >= 1) {outfiles = *((vector<char*> *)output.at(0).p);hasOutput=true;}
        else{hasOutput=false;}
        QString swc_file = in.at(0);
        NeuronTree nt_unsorted = readSWC_file(swc_file);
        QList<int> result;
        result=get_subtree(nt_unsorted, 1);
        NeuronTree out;
        QString filename = "/home/penglab/Desktop/test2/1/test2.swc";
        for (int i=0;i<result.size();i++){
            NeuronSWC node = nt_unsorted.listNeuron.at(result.at(i));
            out.listNeuron.append(node);
        }
        writeSWC_file(filename, out);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

