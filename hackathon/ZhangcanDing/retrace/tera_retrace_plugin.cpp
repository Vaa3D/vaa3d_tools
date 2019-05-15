/* tera_retrace_plugin.cpp
 * This is a 
 * 2019-5-13 : by DZC
 */
 
#include "v3d_message.h"
#include <string>
#include <iostream>
#include <vector>
#include "tera_retrace_plugin.h"
#include "tera_retrace_func.h"
#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "my_surf_objs.h"
using namespace std;
Q_EXPORT_PLUGIN2(tera_retrace, retrace);
 
QStringList retrace::menulist() const
{
	return QStringList() 
		<<tr("retrace")
		<<tr("about");
}

QStringList retrace::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void retrace::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("retrace"))
	{

        printf("welcome ");
        TRACE_LS_PARA P;
        P.is_gsdt=10;
        P.image=0;
        P.block_size=512;
        P.soma=0;
        P.channel=1;
        P.bkg_thresh=-1;
        P.resume =  0;   //add continue tracing option
        P.b_256cube = 0;
        P.b_RadiusFrom2D = 1;
        P.is_gsdt = 0;
        P.is_break_accept =  0;
        P.length_thresh =  5;
        P.adap_win= 1;
        P.tracing_3D = true;
        P.tracing_comb = false;
        P.global_name = true;
        P.method = app2;
        LandmarkList locationlist=callback.getLandmarkTeraFly();
        //neurontracer_app2_raw dialog(callback, parent);
        QString imgpath=callback.getPathTeraFly();

        //v3d_msg("imgpath: \n %s",imgpath);

        P.inimg_file=imgpath;
         bool bmenu=false;

        for (int i=0; i<locationlist.size();i++)
        {
            if(locationlist.at(i).color.r==255 &&locationlist.at(i).color.g==0 && locationlist.at(i).color.b==0)
            {
//                ImageMarker m;
//                m.x=locationlist.at(i).x;
//                m.y=locationlist.at(i).y;
//                m.z=locationlist.at(i).z;
//                m.color.r=255;
//                m.color.g=0;
//                m.color.b=0;
//                QList<ImageMarker> mark={};
//                mark.push_back(m);
               QString  markfn=imgpath+"/../"+QString::number(i)+".marker";
//                writeMarker_file(markfn,mark);
                P.markerfilename=markfn;
                LandmarkList indimark;
                indimark.clear();
                indimark.push_back(locationlist.at(i));
                P.listLandmarks=indimark;
                crawler_raw_app(callback,parent,P,bmenu);

            }
        }


	}
	else
	{
		v3d_msg(tr("This is a plugin for retrace in terafly. "
            "Developed by DZC, 2019-5-14"));
	}
}













bool retrace::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
