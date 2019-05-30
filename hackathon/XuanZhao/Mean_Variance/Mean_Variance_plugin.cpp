/* Mean_Variance_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-28 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "Mean_Variance_plugin.h"
#include <v3d_interface.h>
#include <fstream>

using namespace std;

Q_EXPORT_PLUGIN2(Mean_Variance, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("Mean_Variance")
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
    if (menu_name == tr("Mean_Variance"))
	{
        ofstream out;
        out.open("E://sucai//Mean_Variance.txt",ios::app);

        QString dir=QFileDialog::getExistingDirectory(parent);
        QDir dir_0(dir);
        QStringList filter;
        filter<<"*.tif";
        QStringList tiffiles=dir_0.entryList(filter);

        for(int i=0;i<tiffiles.size();++i)
        {
            QString tiffile_0=dir+"/"+tiffiles[i];
            QFileInfo tiffile(tiffile_0);

            unsigned char* p=0;
            V3DLONG sz[4]={0,0,0,0};
            int datatype=1;
            simple_loadimage_wrapper(callback,tiffile.absoluteFilePath().toStdString().c_str(),p,sz,datatype);

            V3DLONG num=sz[0]*sz[1]*sz[2];
            vector<double> hdz(num,0);
            for(V3DLONG j=0;j<num;++j)
            {
                hdz[j]=(double)p[j];
            }
            double mean=0,variance=0,all=0,all_0=0;
            for(V3DLONG j=0;j<num;++j)
            {
                all+=hdz[j];
            }
            mean=all/num;
            for(V3DLONG j=0;j<num;++j)
            {
                all_0+=(hdz[j]-mean)*(hdz[j]-mean);
            }
            variance=all_0/num;


            out<<tiffile.baseName().toStdString()<<" "
              <<"mean:"<<mean<<" "<<"variance:"<<variance<<endl;
            delete p;
        }

	}
	else if (menu_name == tr("menu2"))
	{
        QString tif=QFileDialog::getOpenFileName(parent);
        QString tif_l=QFileDialog::getOpenFileName(parent);
        unsigned char* p0=0;
        V3DLONG sz0[4]={0,0,0,0};
        int datatype0=0;
        unsigned char* p1=0;
        V3DLONG sz1[4]={0,0,0,0};
        int datatype1=0;
        simple_loadimage_wrapper(callback,tif.toStdString().c_str(),p0,sz0,datatype0);
        simple_loadimage_wrapper(callback,tif_l.toStdString().c_str(),p1,sz1,datatype1);
        V3DLONG num=sz0[0]*sz0[1]*sz0[2];
        unsigned char* p=new unsigned char[num];
        for(V3DLONG i=0;i<num;++i)
        {
            p[i]=((p0[i]+p1[i])>0)?(p0[i]-p1[i]):0;
        }
        int datatype=1;
        V3DLONG sz[4]={sz0[0],sz0[1],sz0[2],sz0[3]};
        string path="E://sucai//l.tif";
        simple_saveimage_wrapper(callback,path.c_str(),p,sz,datatype);
        delete p0;
        delete p1;
        delete p;
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-28"));
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

