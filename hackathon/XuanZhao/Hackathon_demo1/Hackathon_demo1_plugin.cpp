/* Hackathon_demo1_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-16 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <QString>
#include <iostream>
#include "Hackathon_demo1_plugin.h"

#include <basic_surf_objs.h>
#include <v3d_interface.h>

#include "some_class.h"
#include "some_function.h"


using namespace std;
Q_EXPORT_PLUGIN2(Hackathon_demo1, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("getTif")
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
    if (menu_name == tr("getTif"))
	{
        blockTree bt;
        const int dx=512,dy=512,dz=128;
        NeuronTree nt;
        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".","*.eswc");
        nt=readSWC_file(eswcfile);

        for(int i=0;i<nt.listNeuron.size();++i)
        {
            nt.listNeuron[i].on=false;
        }

        QString dir0;
        dir0=QFileDialog::getExistingDirectory(parent);

        bt=getBlockTree(nt,dx,dy,dz);

        for(int i=0;i<bt.blocklist.size();++i)
        {
            QString path0="D://shucai//swc_manual//";
            QString suffix1=".tif";

            const string dir=dir0.toStdString();
            unsigned char* imgblock;
            imgblock=callback.getSubVolumeTeraFly(dir,bt.blocklist[i].min_x,bt.blocklist[i].max_x,
                                                  bt.blocklist[i].min_y,bt.blocklist[i].max_y,
                                                  bt.blocklist[i].min_z,bt.blocklist[i].max_z);

            QString si0=path0+QString::number(bt.blocklist[i].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[i].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[i].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[i].o.z,10)+suffix1;
            const char* si=si0.toStdString().c_str();

            V3DLONG sz0[4]={dx,dy,dz,1};
            int datatype=1;
            if(!simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype))
            {
                std::cout<<"000"<<endl;
            }
            delete imgblock;
        }

	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-16"));
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

