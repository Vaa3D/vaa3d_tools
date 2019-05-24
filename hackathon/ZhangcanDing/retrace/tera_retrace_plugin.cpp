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
#include "vn_app2.h"

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

        // parameters set for ultratracer.
        //some are not used coz a direct app2 plugin call in crawler_raw_app and app_ada_tracing
        //if you want to change some of the parameters, pls go to app_tracing_ada_win3d to set

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

        QString zdim=imgpath.section(QRegExp("[(x)]"),-2,-2).trimmed() ;
        QString ydim=imgpath.section(QRegExp("[(x)]"),-3,-3).trimmed();
        QString xdim=imgpath.section(QRegExp("[(x)]"),-4,-4).trimmed();

        QString zdim_2nd= QString::number(zdim.toInt()/2);
        QString ydim_2nd= QString::number(ydim.toInt()/2);
        QString xdim_2nd= QString::number(xdim.toInt()/2);
        QString res2ndimgpath= imgpath.split("RES")[0]+"RES("
                +xdim_2nd +"x"+ydim_2nd+"x"+zdim_2nd+")";

        qDebug()<< "x ="<<xdim;qDebug()<< "y ="<<ydim;qDebug()<< "z ="<<zdim;
        qDebug()<< "x_2nd ="<<xdim_2nd;qDebug()<< "y_2nd ="<<ydim_2nd;qDebug()<< "z_2nd ="<<zdim_2nd;
        qDebug()<< "2ndimgpath"<<res2ndimgpath;




        //v3d_msg()<<"imgpath: \n" <<imgpath;

         P.inimg_file=res2ndimgpath;
         bool bmenu=false;

         //get referenced swc from terafly and scaling it to 1/2 to fit the img
         NeuronTree ref_swc_highres= callback.getSWCTeraFly();
         NeuronTree ref_swc;

         QList<NeuronSWC> listneuron;listneuron.clear();
         QHash<int,int> hashneuron;hashneuron.clear();

         for(int i=0; i<ref_swc_highres.listNeuron.size();i++)
         {
             NeuronSWC s;
             s.n=ref_swc_highres.listNeuron.at(i).n;
             s.x= ref_swc_highres.listNeuron.at(i).x/2;
             s.y= ref_swc_highres.listNeuron.at(i).y/2;
             s.z= ref_swc_highres.listNeuron.at(i).z/2;
             s.radius= ref_swc_highres.listNeuron.at(i).radius/2;
             s.pn=ref_swc_highres.listNeuron.at(i).pn;
             s.type=ref_swc_highres.listNeuron.at(i).type;
             listneuron.append(s);
             hashneuron.insert(s.n,listneuron.size()-1);

         }
         ref_swc.n=-1;
         ref_swc.on=true;
         ref_swc.listNeuron=listneuron;
         ref_swc.hashNeuron=hashneuron;


         QString savefolder= "D:\\retrace_tmp";
         system(qPrintable(QString("mkdir %1").arg(savefolder.toStdString().c_str())));
         QString ref_swc_highres_fn= savefolder+"\\ref_swc_highres.swc";
         writeSWC_file(ref_swc_highres_fn,ref_swc_highres);


         QString fiswcfolder = savefolder+"\\finalswcfolder";
         qDebug()<<"fiswcfoler: \n"<<fiswcfolder;
         system(qPrintable(QString("mkdir %1").arg(fiswcfolder.toStdString().c_str())));
         P.fusion_folder=fiswcfolder;

         v3d_msg("check");

        for (int i=0; i<locationlist.size();i++)
        {
            if(locationlist.at(i).color.r==255 &&locationlist.at(i).color.g==0 && locationlist.at(i).color.b==0)
            {
                ImageMarker m;
                m.x=locationlist.at(i).x/2;
                m.y=locationlist.at(i).y/2;
                m.z=locationlist.at(i).z/2;
                m.color.r=255;
                m.color.g=0;
                m.color.b=0;
                QList<ImageMarker> indimarker;
                indimarker.clear();
                indimarker.push_back(m);

                QString indimarkerfn;
                if(i<9)
                    indimarkerfn=savefolder+QString("\\00%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(m.x).arg(m.y).arg(m.z);
                else if(i<99)
                    indimarkerfn=savefolder+QString("\\0%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(m.x).arg(m.y).arg(m.z);
                else
                    indimarkerfn=savefolder+QString("\\%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(m.x).arg(m.y).arg(m.z);


                writeMarker_file(indimarkerfn,indimarker);
                qDebug()<<"markerfilename: \n"<<indimarkerfn;
                v3d_msg("check marker name.");
                QString ref_swcfn= indimarkerfn + QString("_nc_APP2_GD.swc");
                writeSWC_file(ref_swcfn,ref_swc);
                P.markerfilename = indimarkerfn ;

                printf("-------------------------------\n");
                crawler_raw_app(callback,parent,P,bmenu);

                v3d_msg("check output");
                QString txtfileName= indimarkerfn+"_tmp_APP2\\scanData.txt";
                qDebug()<<"txtfilename:"<<txtfileName;
                qDebug() <<"txtfileName="<< txtfileName;
                list<string> infostring;
                processSmartScan_3D_wofuison(callback,infostring,txtfileName);


            }
        }

        v3d_msg("check whole output without fusion");
        QString ref_swc_wf= QString(fiswcfolder) + "\\ori.swc";
        writeSWC_file(ref_swc_wf,ref_swc);
        QString fusedswc= QString(fiswcfolder) + "_fused.swc";
        smartFuse(callback,fiswcfolder, fusedswc);


        //scale the fused swc back to highest size to fit the highest resolution
        NeuronTree swc_2ndres=readSWC_file(fusedswc);
        NeuronTree swc_final;
        listneuron.clear();
        hashneuron.clear();
        for(int i=0; i<swc_2ndres.listNeuron.size();i++)
        {
            NeuronSWC s;
            s.n=swc_2ndres.listNeuron.at(i).n;
            s.x= swc_2ndres.listNeuron.at(i).x * 2;
            s.y= swc_2ndres.listNeuron.at(i).y * 2;
            s.z= swc_2ndres.listNeuron.at(i).z * 2;
            s.radius= swc_2ndres.listNeuron.at(i).radius * 2;
            s.pn=swc_2ndres.listNeuron.at(i).pn;
            s.type=swc_2ndres.listNeuron.at(i).type;
            listneuron.append(s);
            hashneuron.insert(s.n,listneuron.size()-1);

        }
        swc_final.n=-1;
        swc_final.on=true;
        swc_final.listNeuron=listneuron;
        swc_final.hashNeuron=hashneuron;

        QString swc_final_fn= QString(fiswcfolder)+"highRES_fused.swc";
        writeSWC_file(swc_final_fn,swc_final);

        callback.setSWCTeraFly(swc_final);






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
        //LandmarkList locationlist=callback.getLandmarkTeraFly();
        //neurontracer_app2_raw dialog(callback, parent);
        //QString imgpath=callback.getPathTeraFly();
        QString imgpath= infiles[0];
        qDebug()<<"input imgpath="<<imgpath;

        //        QString zdim=imgpath.section(QRegExp("[(x)]"),-2,-2).trimmed() ;
        //        QString ydim=imgpath.section(QRegExp("[(x)]"),-3,-3).trimmed();
        //        QString xdim=imgpath.section(QRegExp("[(x)]"),-4,-4).trimmed();

        //        QString zdim_2nd= QString::number(zdim.toInt()/2);
        //        QString ydim_2nd= QString::number(ydim.toInt()/2);
        //        QString xdim_2nd= QString::number(xdim.toInt()/2);
        //        QString res2ndimgpath= imgpath.split("RES")[0]+"RES("
        //                +xdim_2nd +"x"+ydim_2nd+"x"+zdim_2nd+")";

        //        qDebug()<< "x ="<<xdim;qDebug()<< "y ="<<ydim;qDebug()<< "z ="<<zdim;
        //        qDebug()<< "x_2nd ="<<xdim_2nd;qDebug()<< "y_2nd ="<<ydim_2nd;qDebug()<< "z_2nd ="<<zdim_2nd;
        //        qDebug()<< "2ndimgpath"<<res2ndimgpath;


        QString swcfn= infiles[1];
        NeuronTree ref_swc= readSWC_file(swcfn);
        //qDebug()<<"imgpath: \n" <<imgpath;


        v3d_msg("check");
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
            if ( markerlist.at(i).color.r==255 && markerlist.at(i).color.g==0 && markerlist.at(i).color.b==0)

            {
                ImageMarker t= markerlist.at(i);
                QString indimarker ;

                if(i<9)
                    indimarker=QString(outfiles[0])+QString("\\00%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);
                else if(i<99)
                    indimarker=QString(outfiles[0])+QString("\\0%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);
                else
                    indimarker=QString(outfiles[0])+QString("\\%1_x_%2_y_%3_z_%4.marker").arg(i+1).arg(t.x).arg(t.y).arg(t.z);

                QString ref_swcfn= indimarker + QString("_nc_APP2_GD.swc");
                writeSWC_file(ref_swcfn,ref_swc);

                QList<ImageMarker> indimarkerls;
                indimarkerls.clear();
                indimarkerls.push_back(t);
                writeMarker_file(indimarker,indimarkerls);

                P.markerfilename= indimarker;
                crawler_raw_app(callback,parent,P ,bmenu);

                QString txtfileName= indimarker+"_tmp_APP2\\scanData.txt";
                qDebug() <<"txtfileName="<< txtfileName;
                list<string> infostring;
                //processSmartScan_3D(callback,infostring,txtfileName);
                 processSmartScan_3D(callback,infostring,txtfileName);
            }

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
        processSmartScan_3D(callback,infostring,txtfilenName);
	}
    else if (func_name == tr("single_image"))
	{
        PARA_APP2 p2;
        p2.is_gsdt = 0;
        p2.is_coverage_prune = true;
        p2.is_break_accept = false ;
        p2.bkg_thresh = -1;//P.bkg_thresh;
        p2.length_thresh =5 ;
        p2.cnn_type = 2;
        p2.channel = 0;
        p2.SR_ratio = 0.30303;
        p2.b_256cube = 1;
        p2.b_RadiusFrom2D = 1;
        p2.b_resample = 0;
        p2.b_intensity = 0;
        p2.b_brightfiled = 0;
        p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

        p2.inimg_file=infiles[0];
        p2.inmarker_file= infiles[1];
//        p2.xc0 = p2.yc0 = p2.zc0 = 0;
//        p2.xc1 = p2.p4dImage->getXDim()-1;
//        p2.yc1 = p2.p4dImage->getYDim()-1;
//        p2.zc1 = p2.p4dImage->getZDim()-1;

        QString versionstr= "v2.621";
        proc_app2(callback,p2,versionstr);

//        QString app2pluginname= "vn2";
//        QString app2funcname="app2";
//        callback.callPluginFunc(app2pluginname,app2funcname,input,output);





    }else if (func_name=="callapp2")
    {
        QString app2pluginname= "vn2";
        QString app2funcname="app2";


        V3DPluginArgItem arg;
        V3DPluginArgList input_app2;
        arg.type = "random";
        std::vector <char *>arg_input_app2;
        QString test_img= "C:\\Users\\braincenter\\Desktop\\app2_testing\\x_7262_y_13274_z_1952app2.v3draw";
        QByteArray test_img_ba=test_img.toLatin1();
        char * test_img_string =test_img_ba.data();
        arg_input_app2.push_back(test_img_string);
        arg.p = (void *) & arg_input_app2;
        input_app2<<arg;

        arg.type="random";
        std::vector <char *> arg_para_app2;

        //parameters set for app2
        char *p= "NULL";arg_para_app2.push_back(p);
        p ="0";arg_para_app2.push_back(p);
        p= "AUTO";arg_para_app2.push_back(p);
        p ="1";arg_para_app2.push_back(p);
        p= "1";arg_para_app2.push_back(p);
        p ="0";arg_para_app2.push_back(p);
        p ="0";arg_para_app2.push_back(p);
        p ="5";arg_para_app2.push_back(p);
        p ="0";arg_para_app2.push_back(p);
        p ="0";arg_para_app2.push_back(p);
        p= "0";arg_para_app2.push_back(p);
        arg.p= (void *)& arg_para_app2;
        input_app2<<arg;

        arg.type= "random";
        std::vector <char *> arg_output_app2;





        callback.callPluginFunc(app2pluginname,app2funcname,input_app2,output);



        //p NULL 0 AUTO 1 1  0 0 5 0 0 0

    }



    else if (func_name==tr("help"))
    {
        printf("vaa3d -x tera_retrace -f retrace -i <TeraflyImage> <Reference Swc> -p <MarkerFile> -o <OutputFolder>.\n");


    }



	else return false;

	return true;
}
