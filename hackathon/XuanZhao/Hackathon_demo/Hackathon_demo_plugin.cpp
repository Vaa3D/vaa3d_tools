/* Hackathon_demo_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-14 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <QString>

#include "some_function.h"

#include <v3d_interface.h>
#include <basic_surf_objs.h>
#include "Hackathon_demo_plugin.h"
#include "some_class.h"

using namespace std;
Q_EXPORT_PLUGIN2(Hackathon_demo, TestPlugin);



 
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
        NeuronTree nt;
        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".","*.eswc");
        nt=readSWC_file(eswcfile);
        int size=nt.listNeuron.size();
        size_t min_x,max_x,min_y,max_y,min_z,max_z;
        //V3DLONG o_x,o_y,o_z;
        point o_xyz;
        vector<point> o_up,o_down,o_right,o_left,o_front,o_back;

        V3DLONG dx=64;
        V3DLONG dy=64;
        V3DLONG dz=32;


        o_xyz.nswc.x=nt.listNeuron[0].x;
        o_xyz.nswc.y=nt.listNeuron[0].y;
        o_xyz.nswc.z=nt.listNeuron[0].z;
        min_x=o_xyz.nswc.x-dx/2;
        max_x=o_xyz.nswc.x+dx/2;
        min_y=o_xyz.nswc.y-dy/2;
        max_y=o_xyz.nswc.y+dy/2;
        min_z=o_xyz.nswc.z-dz/2;
        max_z=o_xyz.nswc.z+dz/2;



        QString dir0;
        //QString suffix0=".eswc";
        QString path0="D://shucai//swc_manual//";
        QString suffix1=".tif";


        QString center="_c_";
        dir0=QFileDialog::getExistingDirectory(parent);
        const string dir=dir0.toStdString();
        unsigned char* imgblock;
        imgblock=callback.getSubVolumeTeraFly(dir,min_x,max_x,min_y,max_y,min_z,max_z);


        QString si0=path0+center+"x_"+QString::number((qlonglong)o_xyz.nswc.x,10)+"_y_"+QString::number((qlonglong)o_xyz.nswc.y,10)+"_z_"+QString::number((qlonglong)o_xyz.nswc.z,10)+suffix1;
        const char* si=si0.toStdString().c_str();

        V3DLONG sz0[4]={dx,dy,dz,1};
        int datatype=1;
        if(!simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype))
        {
            cout<<"000"<<endl;

        }

        delete imgblock;

        o_up=get_up(nt,min_x,max_x,min_y,max_y,max_z);
        o_down=get_down(nt,min_x,max_x,min_y,max_y,min_z);
        o_left=get_left(nt,min_x,max_x,min_z,max_z,min_y);
        o_right=get_right(nt,min_x,max_x,min_z,max_z,max_y);
        o_front=get_front(nt,min_y,max_y,min_z,max_z,max_x);
        o_back=get_back(nt,min_y,max_y,min_z,max_z,min_x);

        for(int i=0;i<o_up.size();++i)
        {
            cut_image(callback,nt,o_up[i],dir0);
        }
        for(int i=0;i<o_down.size();++i)
        {
            cut_image(callback,nt,o_down[i],dir0);
        }
        for(int i=0;i<o_left.size();++i)
        {
            cut_image(callback,nt,o_left[i],dir0);
        }
        for(int i=0;i<o_right.size();++i)
        {
            cut_image(callback,nt,o_right[i],dir0);
        }
        for(int i=0;i<o_front.size();++i)
        {
            cut_image(callback,nt,o_front[i],dir0);
        }
        for(int i=0;i<o_back.size();++i)
        {
            cut_image(callback,nt,o_back[i],dir0);
        }


	}
	else if (menu_name == tr("menu2"))
	{
        blockTree bt;
        const int dx=512,dy=512,dz=128;
        NeuronTree nt;
        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".","*.eswc");
        nt=readSWC_file(eswcfile);
        QString dir0;
        dir0=QFileDialog::getExistingDirectory(parent);

        vector<point> pts;
        cout<<"before trans"<<endl;
        pts=trans(nt);
        cout<<"after trans"<<endl;

        bt=getBlockTree(pts,dx,dy,dz);
        cout<<bt.blocklist.size()<<endl;

        for(int i=0;i<bt.blocklist.size();++i)
        {

            //QString suffix0=".eswc";
            QString path0="D://shucai//swc_manual//";
            QString suffix1=".tif";


            //QString center="_c_";

            const string dir=dir0.toStdString();
            unsigned char* imgblock;
            imgblock=callback.getSubVolumeTeraFly(dir,bt.blocklist[i].min_x,bt.blocklist[i].max_x,
                                                  bt.blocklist[i].min_y,bt.blocklist[i].max_y,
                                                  bt.blocklist[i].min_z,bt.blocklist[i].max_z);


            QString si0=path0+QString::number(bt.blocklist[i].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[i].o.nswc.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[i].o.nswc.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[i].o.nswc.z,10)+suffix1;
            const char* si=si0.toStdString().c_str();

            V3DLONG sz0[4]={dx,dy,dz,1};
            int datatype=1;
            if(!simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype))
            {
                cout<<"000"<<endl;
            }
            delete imgblock;
        }



	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-14"));
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

