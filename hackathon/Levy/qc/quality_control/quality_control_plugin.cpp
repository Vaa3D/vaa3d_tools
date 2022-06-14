/* quality_control_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-15 : by ZSJ
 */
 
#include "quality_control_plugin.h"
#include "feature_analysis.h"

using namespace std;
Q_EXPORT_PLUGIN2(quality_control, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("arbor_based_QC")
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
    if (menu_name == tr("arbor_based_QC"))
	{
        arbor_main(callback, parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by YiweiLi, 2022-2-15"));
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
        cout<<"check1"<<endl;
        QList<int> result;
        result = loop_detection(nt_unsorted);
        QList<CellAPO> apo;
        QString filename = "/home/penglab/Desktop/test2/1.apo";
        for (int i=0;i<result.size();i++){
            CellAPO m;
            m.x = nt_unsorted.listNeuron.at(result.at(i)).x;
            m.y = nt_unsorted.listNeuron.at(result.at(i)).y;
            m.z = nt_unsorted.listNeuron.at(result.at(i)).z;
            m.color.r=255;
            m.color.g=0;
            m.color.b=0;
            m.volsize = 50;
            apo.push_back(m);
        }
        writeAPO_file(filename, apo);
	}
    else if (func_name == tr("tree_feature"))
    {
        tree_structure(input, output,callback);
    }
    else if (func_name == tr("arbor_qc"))
    {
        vector<char*> in, inparas, outfiles;
        if(input.size() >= 1) in = *((vector<char*> *)input.at(0).p);

        if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
        int input_flag=TRUE;
        bool flag_sort;
        if (inparas.size() == 1)
        {
            input_flag = atof(inparas.at(0));
            if(input_flag==1){
                flag_sort=TRUE;
            }
            else{
                flag_sort=FALSE;
            }
         }
        else if(inparas.size()>1){
            cout<<"Illegal parameter list."<<endl;
            return false;
        }

        QString outfolder;
        if(output.size() >= 1) {outfiles = *((vector<char*> *)output.at(0).p);outfolder= QString(outfiles.at(0));}
        QString inputfolder=QString::fromStdString(in[0]);
        QString type_csv=inparas.at(0);
        arbor_qc(inputfolder, type_csv, flag_sort, outfolder);

    }
    else if (func_name == tr("truncate"))
    {
        arbor_truncate(input, output,callback);
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

