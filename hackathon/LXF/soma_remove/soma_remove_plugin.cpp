    /* soma_remove_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-7-11 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_remove_plugin.h"
#include "soma_remove_main.h"
#include "data_training.h"
using namespace std;
Q_EXPORT_PLUGIN2(soma_remove, soma_removePlugin);
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
 
QStringList soma_removePlugin::menulist() const
{
	return QStringList() 
        <<tr("soma_remove")
        <<tr("data_training")
          <<tr("distance_between_two_connected_area")
		<<tr("about");
}

QStringList soma_removePlugin::funclist() const
{
	return QStringList()
        <<tr("soma_remove")
        <<tr("data_training")
          <<tr("distance_between_two_connected_area")
		<<tr("help");
}

void soma_removePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("soma_remove"))
	{
        v3dhandle win = callback.currentImageWindow();
        Image4DSimple * img = callback.getImage(win);
        V3DLONG in_sz[4];
        in_sz[0] = img->getXDim();
        in_sz[1] = img->getYDim();
        in_sz[2] = img->getZDim();
        in_sz[3] = img->getCDim();

        //soma_remove_main(img->getRawData(),in_sz,callback);
        soma_remove_main_2(img->getRawData(),in_sz,callback);
        //v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
        v3d_msg("To be implemented.");
	}
    else if (menu_name == tr("distance_between_two_connected_area"))
    {
        v3d_msg("To be implemented.");
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-7-11"));
	}
}

bool soma_removePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("soma_remove"))
	{
		v3d_msg("To be implemented.");
	}
    else if (func_name == tr("data_training"))
	{
        //QString filelistOpenName = QString(inlist->at(0));
        //QString filelistfileOpenName2 = QString(inlist->at(1));
        //QString raw_img = QString(inlist->at(2));
        data_training(input,output,callback);
		v3d_msg("To be implemented.");
	}
    else if (func_name == tr("distance_between_two_connected_area"))
	{
        if(infiles.size()!=2)
        {
            //v3d_msg("Please input two files,first one is offset result,anther one is markerlist for all center");
            return false;
        }
        QString fileOpenName1 = QString(infiles.at(0));
        QString fileOpenName2 = QString(infiles.at(1));
        QList <ImageMarker> markerlist1 = readMarker_file(fileOpenName1);
        QList <ImageMarker> markerlist2 = readMarker_file(fileOpenName2);
        QList <ImageMarker> marker_result;
        double min_dis=100000000000;
        for(int i=0;i<markerlist1.size();i++)
        {
            for(int j=0;j<markerlist2.size();j++)
            {
                double dis = NTDIS(markerlist1[i],markerlist2[j]);
               // cout<<"dis = "<<dis<<endl;
                if(dis<min_dis)
                {
                    min_dis = dis;
                  //  cout<<min_dis<<endl;
                }
            }
        }
        cout<<"min_dis = "<<min_dis<<endl;

		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
