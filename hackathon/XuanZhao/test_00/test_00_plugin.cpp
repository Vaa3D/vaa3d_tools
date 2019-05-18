/* test_00_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-2 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>

#include "test_00_plugin.h"
#include <v3d_interface.h>
#include <basic_surf_objs.h>
#include <QString>

#include "some_function.h"


using namespace std;
Q_EXPORT_PLUGIN2(test_00, TestPlugin);

const int cc=256;
 
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
        /*unsigned char* a;
        size_t x0,x1,y0,y1,z0,z1;
        x0=12492;
        x1=13178;
        y0=11860.5;
        y1=12554;
        z0=5253.11;
        z1=5419;
        const char* file="D://v3d_2013//vaa3d_tools//hackathon//XuanZhao//test_00//test.tif";
        V3DLONG sz[4]={x1-x0,y1-y0,z1-z0,1};
        int datatype=1;
        a = callback.getSubVolumeTeraFly("Z://TeraconvertedBrain//mouse18457_teraconvert//RES(24275x37800x10955)", x0,x1,y0,y1,z0,z1);
        simple_saveimage_wrapper(callback,file,a,sz,datatype);*/

        NeuronTree nt;
        //const QString* file=
        const QString file="D://v3d_2013//vaa3d_tools//hackathon//XuanZhao//test_reconstruct//18457_00088.eswc";
        nt=readESWC_file(file);
        int size=nt.listNeuron.size();
        vector<float> xx,yy,zz;
        cout<<"000"<<endl;

        for(int i=0;i<size;++i)
        {
            if(nt.listNeuron[i].type==3){
                xx.push_back(nt.listNeuron[i].x);
                yy.push_back(nt.listNeuron[i].y);
                zz.push_back(nt.listNeuron[i].z);
            }
        }

        float max_x,max_y,max_z,min_x,min_y,min_z;

        max_x=max0<float,vector<float>::iterator>(xx.begin(),xx.end());
        max_y=max0<float,vector<float>::iterator>(yy.begin(),yy.end());
        max_z=max0<float,vector<float>::iterator>(zz.begin(),zz.end());
        min_x=min0<float,vector<float>::iterator>(xx.begin(),xx.end());
        min_y=min0<float,vector<float>::iterator>(yy.begin(),yy.end());
        min_z=min0<float,vector<float>::iterator>(zz.begin(),zz.end());
        cout<<max_x<<endl;
        cout<<max_y<<endl;
        cout<<max_z<<endl;
        cout<<min_x<<endl;
        cout<<min_y<<endl;
        cout<<min_z<<endl;

        float max_x0,max_y0,max_z0,min_x0,min_y0,min_z0;
        min_x0=((int)min_x/cc)*cc;
        min_y0=((int)min_y/cc)*cc;
        min_z0=((int)min_z/cc)*cc;
        max_x0=((int)max_x/cc+1)*cc;
        max_y0=((int)max_y/cc+1)*cc;
        max_z0=((int)max_z/cc+1)*cc;
        cout<<min_x0<<endl<<max_x0<<endl<<(max_x0-min_x0)<<endl<<min_y0<<endl<<max_y0<<endl<<(max_y0-min_y0)<<endl
           <<min_z0<<endl<<max_z0<<endl<<(max_z0-min_z0)<<endl;


        unsigned char* a;
        const char* file2="D://v3d_2013//vaa3d_tools//hackathon//XuanZhao//test_00//test.tif";
        V3DLONG sz[4]={max_x0-min_x0,max_y0-min_y0,max_z0-min_z0,1};
        int datatype=1;
        a=callback.getSubVolumeTeraFly("Z://TeraconvertedBrain//mouse18457_teraconvert//RES(24275x37800x10955)",min_x0,max_x0,min_y0,max_y0,min_z0,max_z0);
        simple_saveimage_wrapper(callback,file2,a,sz,datatype);
        delete a;

        int dd=256;
        V3DLONG sz0[4]={dd,dd,dd,1};
        QString dir="D://shucai//18457//";
        QString suffix=".tif";

        for(float j=0;j<max_x0-min_x0;j+=dd){
            for(float l=0;l<max_y0-min_y0;l+=dd)
                for(float m=0;m<max_z0-min_z0;m+=dd){
                    a=callback.getSubVolumeTeraFly(file2,j,j+dd,l,l+dd,m,m+dd);
                    QString s=dir+QString::number((int)(min_x0+j),10)+"_"+QString::number((int)(min_y0+l),10)+"_"+QString::number((int)(min_z0+m),10)+suffix;

                    string str=s.toStdString();
                    const char* s0=str.c_str();

                    simple_saveimage_wrapper(callback,s0,a,sz0,datatype);
                    delete a;
                }
        }
        delete a;
        /*const char* s0="D://shucai//18457//00.tif";
        a=callback.getSubVolumeTeraFly(file2,0,dd,0,dd,0,dd);
        simple_saveimage_wrapper(callback,s0,a,sz0,datatype);
        delete a;*/


	}
	else if (menu_name == tr("menu2"))
	{
        NeuronTree nt;
        //const QString* file=
        //const QString file="D://v3d_2013//vaa3d_tools//hackathon//XuanZhao//test_reconstruct//18457_00088.eswc";
        const QString file="C://Users//BrainCenter2//Desktop//test.tif_ini.swc";
        nt=readESWC_file(file);
        int size=nt.listNeuron.size();
        vector<float> xx,yy,zz;
        cout<<"000"<<endl;

        for(int i=0;i<size;++i)
        {
            if(nt.listNeuron[i].type==3){
                xx.push_back(nt.listNeuron[i].x);
                yy.push_back(nt.listNeuron[i].y);
                zz.push_back(nt.listNeuron[i].z);
            }
        }

        float max_x,max_y,max_z,min_x,min_y,min_z;

        max_x=max0<float,vector<float>::iterator>(xx.begin(),xx.end());
        max_y=max0<float,vector<float>::iterator>(yy.begin(),yy.end());
        max_z=max0<float,vector<float>::iterator>(zz.begin(),zz.end());
        min_x=min0<float,vector<float>::iterator>(xx.begin(),xx.end());
        min_y=min0<float,vector<float>::iterator>(yy.begin(),yy.end());
        min_z=min0<float,vector<float>::iterator>(zz.begin(),zz.end());
        cout<<max_x<<endl;
        cout<<max_y<<endl;
        cout<<max_z<<endl;
        cout<<min_x<<endl;
        cout<<min_y<<endl;
        cout<<min_z<<endl;

        float max_x0,max_y0,max_z0,min_x0,min_y0,min_z0;
        min_x0=((int)min_x/cc)*cc;
        min_y0=((int)min_y/cc)*cc;
        min_z0=((int)min_z/cc)*cc;
        max_x0=((int)max_x/cc+1)*cc;
        max_y0=((int)max_y/cc+1)*cc;
        max_z0=((int)max_z/cc+1)*cc;

        NeuronTree a1;

        QString dir="D://shucai//18457//";

        QString suffix=".eswc";

        QString au="auto";

        int dd=256;
        cout<<"000"<<endl;


        for(float j=min_x0;j<max_x0;j+=dd){
            cout<<"111"<<endl;
            for(float l=min_y0;l<max_y0;l+=dd){
                cout<<"222"<<endl;

                for(float m=min_z0;m<max_z0;m+=dd){
                    cout<<"333"<<endl;
                    a1.listNeuron.clear();
                    for(int i=0;i<size;++i){
                        if((nt.listNeuron[i].x>j)&&(nt.listNeuron[i].x<=(dd+j))
                          &&(nt.listNeuron[i].y>l)&&(nt.listNeuron[i].y<=(dd+l))
                          &&(nt.listNeuron[i].z>m)&&(nt.listNeuron[i].z<=(dd+m)))
                        {
                            nt.listNeuron[i].x-=j;
                            nt.listNeuron[i].y-=l;
                            nt.listNeuron[i].z-=m;

                            a1.listNeuron.push_back(nt.listNeuron[i]);



                        }
                    }
                    if(!a1.listNeuron.isEmpty()){
                        //const QString s=dir+QString::number((int)j,10)+"_"+QString::number((int)l,10)+"_"+QString::number((int)m,10)+suffix;
                        const QString s=dir+au+QString::number((int)j,10)+"_"+QString::number((int)l,10)+"_"+QString::number((int)m,10)+suffix;


                        if(!writeESWC_file1(s, a1))
                        {
                            cout<<"777"<<endl;
                        }
                    }else{
                        cout<<"555"<<endl;
                    }
                }
            }
        }
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-2"));
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

