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

        printf("\n This is a plugin for retrace in terafly  Developed by DZC, 2019-5-14\n");
        TRACE_LS_PARA P;

        P.image=0;
        P.block_size=512;
        P.soma=0;
        P.channel=1;
        P.bkg_thresh=10;
        P.resume =  1;   //add continue tracing option
        P.b_256cube = 1;
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

        qDebug()<<"imgpath: \n" <<imgpath;

        P.inimg_file=imgpath;
         bool bmenu=false;
         NeuronTree swc;

        for (int i=0; i<locationlist.size();i++)
        {
            if(locationlist.at(i).color.r==255 &&locationlist.at(i).color.g==0 && locationlist.at(i).color.b==0)
            {
                ImageMarker m;
                m.x=locationlist.at(i).x;
                m.y=locationlist.at(i).y;
                m.z=locationlist.at(i).z;
                m.color.r=255;
                m.color.g=0;
                m.color.b=0;
                QList<ImageMarker> mark;
                mark.push_back(m);
                printf("\n ++++++++++++++++++++\n");
               QString  markfn=imgpath+"/../"+QString::number(i)+".marker";
                writeMarker_file(markfn,mark);
                P.markerfilename=markfn;
                qDebug()<<"markerfilename: \n"<<markfn;
                LandmarkList indimark;

                indimark.clear();
                indimark.push_back(locationlist.at(i));
                P.listLandmarks=indimark;
                printf("-------------------------------\n");
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

    if (func_name == tr("retrace"))
	{

        TRACE_LS_PARA P;

        P.image=0;
        P.block_size=512;
        P.soma=0;
        P.channel=1;
        P.bkg_thresh=-1;
        P.resume =  0;   //add continue tracing option
        P.b_256cube = 1;
        P.b_RadiusFrom2D = 1;
        P.is_gsdt = 0;
        P.is_break_accept =  0;
        P.length_thresh =  5;
        P.adap_win= 1;
        P.tracing_3D = true;
        P.tracing_comb = false;
        P.global_name = true;
        P.method = app2;
        //LandmarkList locationlist=callback.getLandmarkTeraFly();
        //neurontracer_app2_raw dialog(callback, parent);
        //QString imgpath=callback.getPathTeraFly();
        QString imgpath= infiles[0];
        QString swcfn= infiles[1];
        NeuronTree ref_swc= readSWC_file(swcfn);
        qDebug()<<"imgpath: \n" <<imgpath;

        P.inimg_file=imgpath;
        bool bmenu=false;


        QString markerfn = inparas[0];
        QList<ImageMarker> markerlist;
        markerlist=readMarker_file(markerfn);

        qDebug()<<"markerlist size ="<< markerlist.size();

        if (outfiles.size()<1)
        {
            fprintf(stderr,"please input an output folder.");

        }

        QString fiswcfolder = QString(outfiles[0])+"\\finalswcfolder";
        qDebug()<<"fiswcfoler: \n"<<fiswcfolder;
        system(qPrintable(QString("mkdir %1").arg(fiswcfolder.toStdString().c_str())));
        P.fusion_folder=fiswcfolder;



        for (int i=0; i<markerlist.size();i++)
        {
            //            LocationSimple t;
            //            t.x= markerlist.at(i).x;
            //            t.y= markerlist.at(i).y;
            //            t.z= markerlist.at(i).z;
            //            P.listLandmarks.push_back(t);
            ImageMarker t= markerlist.at(i);
            QString indimarker ;

            if(i<9)
                indimarker=QString(outfiles[0])+QString("\\00%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);
            else if(i<99)
                indimarker=QString(outfiles[0])+QString("\\0%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);
            else
                indimarker=QString(outfiles[0])+QString("\\%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);

           //QString ref_swcfn= indimarker + QString("_nc_APP2_GD.swc");
           //writeSWC_file(ref_swcfn,ref_swc);

            QList<ImageMarker> indimarkerls;
            indimarkerls.clear();
            indimarkerls.push_back(t);
            writeMarker_file(indimarker,indimarkerls);

            P.markerfilename= indimarker;
            crawler_raw_app(callback,parent,P ,bmenu);

            QString txtfileName= indimarker+"_tmp_APP2\\scanData.txt";
            qDebug() <<"txtfileName="<< txtfileName;
            list<string> infostring;
            processSmartScan_3D(callback,infostring,txtfileName, P);

        }


        QString ref_swc_wf= QString(fiswcfolder) + "\\ori.swc";
        writeSWC_file(ref_swc_wf,ref_swc);
        QString fusedswc= QString(fiswcfolder) + "_fused.swc";

        smartFuse(callback,fiswcfolder, fusedswc);














    }
    else if (func_name == tr("generate_final_result"))
	{
        if(infiles.empty())
        {
            cerr<<"Need input txt file"<<endl;
            return false;
        }

        QString txtfilenName = infiles[0];
        TRACE_LS_PARA P;
        list<string> infostring;
        processSmartScan_3D(callback,infostring,txtfilenName,P);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
