/* test00_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-15 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test00_plugin.h"
#include <iostream>
using namespace std;
Q_EXPORT_PLUGIN2(test00, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        //v3d_msg("To be implemented.");
        QString path1 = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file")), "/home/gyy/Desktop");
        NeuronTree nt = readSWC_file(path1);
        V3DLONG size_nt = nt.listNeuron.size();
        cout<<"size_nt="<<size_nt<<endl;
        V3DLONG max_n = size_nt, min_n = 1;
        vector<NeuronTree> vec_tree;
        //cout<<"000"<<endl;
        for(V3DLONG i = 0; i < size_nt; i ++)
        {
            if(nt.listNeuron[i].parent<min_n || nt.listNeuron[i].parent>max_n)
            {
                NeuronTree nt1;
                nt1.listNeuron.push_back(nt.listNeuron[i]);
                vec_tree.push_back(nt1);
            }

            else
            {
                vec_tree.back().listNeuron.push_back(nt.listNeuron[i]);
            }
        }
        for(V3DLONG j = 0; j < vec_tree.size(); j++)
        {
            QString path2 = "/home/balala/Desktop/data_eswc/seg";
            QString suffix = ".eswc";
            QString s = path2+"_"+QString::number((int)j,10)+suffix;
            writeESWC_file(s, vec_tree[j] );
        }
    }
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-15"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

