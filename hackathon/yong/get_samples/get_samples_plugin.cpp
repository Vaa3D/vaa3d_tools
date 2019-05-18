/* get_samples_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-4-29 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include <string>
#include <iostream>
#include "basic_surf_objs.h"
#include "resampling.h"
#include "sort_swc.h"

#include "get_samples_plugin.h"
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
Q_EXPORT_PLUGIN2(get_samples, get_samplesPlugin);

using namespace std;

 
QStringList get_samplesPlugin::menulist() const
{
	return QStringList() 
        <<tr("get_train_smaples")
		<<tr("about");
}

QStringList get_samplesPlugin::funclist() const
{
	return QStringList()
        <<tr("get_train_smaples")
		<<tr("help");
}

void get_samplesPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_train_smaples"))
	{
        v3d_msg("vaa3d -x get_samples -f get_train_smaples -i <inimg_file> -p <swc_file> -o <out_file>\n"
                "inimg_file       The input image\n"
                "swc_file         The input swc\n"
                "out_file         The output image and swc block\n");
    }

    else if (menu_name == tr("get_train_smaples_soma"))
    {
        v3d_msg("vaa3d -x get_samples -f get_train_smaples_soma -i <inimg_file>  -p <swc_file> <soma_file> -o <out_file>\n"
                "inimg_file       The input image\n"
                "swc_file         The input swc\n"
                "soma_file         The input soma\n"
                "out_file         The output image and swc block\n");
    }

	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Yongzhang, 2019-4-29"));
	}
}

bool get_samplesPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    input_PARA P;
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    vector<char*>* outlist = NULL;

    P.inimg_file = infiles[0];
    if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        P.out_file = QString(outlist->at(0));
    }

    cout<<"inimg : "<<P.inimg_file.toStdString()<<endl;
    cout<<"out_file: "<<P.out_file.toStdString()<<endl;


    if (func_name == tr("get_train_smaples"))
	{   
        int k=0;
        QString swc_file = paras.empty() ? "" : paras[k]; if(swc_file == "NULL") swc_file = ""; k++;
        if(swc_file.isEmpty())
        {
            cerr<<"Need a swc file"<<endl;
            return false;
        }
        else
            P.swc_file = swc_file;

        cout<<"swc : "<<P.swc_file.toStdString()<<endl;
        get_train_smaples(callback,parent,input,output,P);
	}

    else if (func_name == tr("get_train_smaples_soma"))
    {
//        int k=0;
//        QString soma_file = paras.empty() ? "" : paras[k]; if(soma_file == "NULL") soma_file = ""; k++;
//        if(soma_file.isEmpty())
//        {
//            cerr<<"Need a soma file"<<endl;
//            return false;
//        }
//        else
//            P.soma_file = soma_file;
        P.swc_file = paras[0];
        P.soma_file = paras[1];

        cout<<"swc_file :  "<<P.swc_file.toStdString()<<endl;
        cout<<"soma_file : "<<P.soma_file.toStdString()<<endl;

        get_train_smaples_soma(callback,parent,input,output,P);
    }

    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN
		printf("**** Usage of get_samples tracing **** \n");
        printf("vaa3d -x get_samples -f get_train_smaples -i <inimg_file> -p <swc_file> -o <out_file>\n");
        printf("inimg_file       The input image\n");
        printf("swc_file         The input swc\n");
        printf("out_file         The output image and swc block\n");

	}
	else return false;

	return true;
}

bool get_train_smaples(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input, V3DPluginArgList & output,input_PARA &P)
{
    //load image
    QString imagepath = P.inimg_file;

    //load and processing swc
    QStringList swclist = importFileList_addnumbersort(P.swc_file);
    cout<<"swclist = "<<swclist.size()<<endl;
    QList<NeuronTree> All_nt;
    for(int i = 2;i < swclist.size();i++)
    {
        //cout<<"swclist : "<<swclist[i].toStdString()<<endl;
        NeuronTree nt = readSWC_file(swclist[i]);
        //resample swc
        double step=2.0,n = 0.5;
        NeuronTree nt_resample = resample(nt,step);

        NeuronTree nt_resize = resize(nt_resample,n);

        All_nt.push_back(nt_resize);

    }
    cout<<"All_nt.size = "<<All_nt.size()<<endl;

    //get samples
    for(int i = 2;i < swclist.size();i++)
    {
        //cout<<"swclist : "<<swclist[i].toStdString()<<endl;
        NeuronTree nt = readSWC_file(swclist[i]);
        //resample swc
        double step=2.0,n = 0.5;
        NeuronTree nt_resample = resample(nt,step);

        NeuronTree nt_resize = resize(nt_resample,n);

        for(V3DLONG j =0;j < nt_resize.listNeuron.size();j=j+500)
        {
            NeuronSWC cur;
            cur.x = nt_resize.listNeuron[j].x;
            cur.y = nt_resize.listNeuron[j].y;
            cur.z = nt_resize.listNeuron[j].z;

            double l_x = 64;
            double l_y = 64;
            double l_z = 32;

            V3DLONG xb = cur.x-l_x;
            V3DLONG xe = cur.x+l_x-1;
            V3DLONG yb = cur.y-l_y;
            V3DLONG ye = cur.y+l_y-1;
            V3DLONG zb = cur.z-l_z;
            V3DLONG ze = cur.z+l_z-1;


            V3DLONG im_cropped_sz[4];
            unsigned char * im_cropped = 0;
            V3DLONG pagesz;

            pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

            QList<NeuronSWC> outswc, outswc_sort;
            int lens=0;
            for(V3DLONG k = 0; k < All_nt.size();k++)
            {
                for(V3DLONG l= 0;l < All_nt[k].listNeuron.size();l++)
                {
                    NeuronSWC S;
                    if(All_nt[k].listNeuron[l].x<xe&&All_nt[k].listNeuron[l].x>xb&&All_nt[k].listNeuron[l].y<ye&&All_nt[k].listNeuron[l].y>yb&&All_nt[k].listNeuron[l].z<ze&&All_nt[k].listNeuron[l].z>zb)
                    {
                        S.x = All_nt[k].listNeuron[l].x-xb;
                        S.y = All_nt[k].listNeuron[l].y-yb;
                        S.z = All_nt[k].listNeuron[l].z-zb;
                        S.n = All_nt[k].listNeuron[l].n+lens;
                        S.pn = All_nt[k].listNeuron[l].pn+lens;
                        S.r = All_nt[k].listNeuron[l].r;
                        S.type = All_nt[k].listNeuron[l].type;

                        outswc.push_back(S);
                    }
                }
                lens=lens+All_nt[k].listNeuron.size();

            }

            V3DLONG rootid = VOID;
            V3DLONG thres = 0;
            SortSWC(outswc, outswc_sort ,rootid, thres);        

            im_cropped = callback.getSubVolumeTeraFly(imagepath.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


            QString outimg_file,outswc_file;
            outimg_file = P.out_file+"/"+QString::number(i-1)+"_"+QString("x_%1_y_%2_z_%3").arg(xb).arg(yb).arg(zb)+".tif";
            outswc_file = P.out_file+"/"+QString::number(i-1)+"_"+QString("x_%1_y_%2_z_%3").arg(xb).arg(yb).arg(zb)+".swc";
            export_list2file(outswc_sort,outswc_file,outswc_file);

            simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
            //if(im_cropped) {delete []im_cropped; im_cropped = 0;}

        }

    }

    return true;
}


bool get_train_smaples_soma(V3DPluginCallback2 &callback, QWidget *parent, const V3DPluginArgList & input, V3DPluginArgList & output,input_PARA &P)
{
    //load image
    QString imagepath = P.inimg_file;

    //load and processing swc
    QStringList swclist = importFileList_addnumbersort(P.swc_file);
    cout<<"swclist = "<<swclist.size()<<endl;
    QList<NeuronTree> All_nt;
    for(int i = 2;i < swclist.size();i++)
    {
        //cout<<"swclist : "<<swclist[i].toStdString()<<endl;
        NeuronTree nt = readSWC_file(swclist[i]);
        //resample swc
        double step=2.0,n = 0.5;
        NeuronTree nt_resample = resample(nt,step);

        NeuronTree nt_resize = resize(nt_resample,n);

        All_nt.push_back(nt_resize);

    }
    cout<<"All_nt.size = "<<All_nt.size()<<endl;

    //load soma
    double n =0.5;
    NeuronTree nt = readSWC_file(P.soma_file);
    NeuronTree nt_resize = resize(nt,n);
    LandmarkList somaMarkers;
    for(V3DLONG i = 0;i < nt_resize.listNeuron.size();i++)
    {
        LocationSimple t;
        t.x = nt_resize.listNeuron[i].x;
        t.y = nt_resize.listNeuron[i].y;
        t.z = nt_resize.listNeuron[i].z;
        //somaMarkers.push_back(t);

        V3DLONG im_cropped_sz[4];

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        double l_x = 128;
        double l_y = 128;
        double l_z = 32;

        V3DLONG xb = t.x-l_x;
        V3DLONG xe = t.x+l_x-1;
        V3DLONG yb = t.y-l_y;
        V3DLONG ye = t.y+l_y-1;
        V3DLONG zb = t.z-l_z;
        V3DLONG ze = t.z+l_z-1;
        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

        QList<NeuronSWC> outswc,outswc_sort;
        int lens=0;
        for(V3DLONG k = 0; k < All_nt.size();k++)
        {
            for(V3DLONG l= 0;l < All_nt[k].listNeuron.size();l++)
            {
                NeuronSWC S;
                if(All_nt[k].listNeuron[l].x<xe&&All_nt[k].listNeuron[l].x>xb&&All_nt[k].listNeuron[l].y<ye&&All_nt[k].listNeuron[l].y>yb&&All_nt[k].listNeuron[l].z<ze&&All_nt[k].listNeuron[l].z>zb)
                {
                    S.x = All_nt[k].listNeuron[l].x-xb;
                    S.y = All_nt[k].listNeuron[l].y-yb;
                    S.z = All_nt[k].listNeuron[l].z-zb;
                    S.n = All_nt[k].listNeuron[l].n+lens;
                    S.pn = All_nt[k].listNeuron[l].pn+lens;
                    S.r = All_nt[k].listNeuron[l].r;
                    S.type = All_nt[k].listNeuron[l].type;

                    outswc.push_back(S);
                }
            }
            lens=lens+All_nt[k].listNeuron.size();

        }

        V3DLONG rootid = VOID;
        V3DLONG thres = 0;
        SortSWC(outswc, outswc_sort ,rootid, thres);


        im_cropped = callback.getSubVolumeTeraFly(imagepath.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


        QString outimg_file,outswc_file;
        outimg_file = P.out_file+"/"+QString("x_%1_y_%2_z_%3").arg(t.x).arg(t.y).arg(t.z)+".tif";
        outswc_file = P.out_file+"/"+QString("x_%1_y_%2_z_%3").arg(t.x).arg(t.y).arg(t.z)+".swc";
        export_list2file(outswc_sort,outswc_file,outswc_file);

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    }



    return true;
}

QStringList importFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();
    // get the iamge files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";

    QDir dir(curFilePath);
    if(!dir.exists())
    {
        cout <<"Cannot find the directory";
        return myList;
    }
    foreach(QString file, dir.entryList()) // (imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }
    //print filenames
    foreach(QString qs, myList) qDebug() << qs;
    return myList;
}

NeuronTree resize(NeuronTree &nt,double n)
{
    NeuronTree nt_resize;
    for(V3DLONG i =0 ;i < nt.listNeuron.size();i++)
    {
        NeuronSWC cur;
        cur.x = n*(nt.listNeuron[i].x);
        cur.y = n*(nt.listNeuron[i].y);
        cur.z = n*(nt.listNeuron[i].z);
        cur.n = nt.listNeuron[i].n;
        cur.type = nt.listNeuron[i].type;
        cur.radius = nt.listNeuron[i].radius;
        cur.parent = nt.listNeuron[i].parent;

        nt_resize.listNeuron.push_back(cur);
    }

    return nt_resize;
}

bool sort_with_standard(QList<NeuronSWC>  & neuron1, QList<NeuronSWC> & neuron2,QList<NeuronSWC>  &result)
{
    V3DLONG siz = neuron1.size();
    V3DLONG root_id = 1;
    double dist;
    if (siz==0) return false;
    double min_dist = sqrt((neuron1[0].x-neuron2[0].x)*(neuron1[0].x-neuron2[0].x)
         +(neuron1[0].y-neuron2[0].y)*(neuron1[0].y-neuron2[0].y)
         +(neuron1[0].z-neuron2[0].z)*(neuron1[0].z-neuron2[0].z));
    for(V3DLONG i=0; i<siz; i++)
    {
         dist = sqrt((neuron1[i].x-neuron2[0].x)*(neuron1[i].x-neuron2[0].x)
                +(neuron1[i].y-neuron2[0].y)*(neuron1[i].y-neuron2[0].y)
                +(neuron1[i].z-neuron2[0].z)*(neuron1[i].z-neuron2[0].z));
         if(min_dist > dist) {min_dist = dist; root_id = i+1;}
    }
    cout<<"min_dist = "<< min_dist <<endl;
    cout<<"root_id = " << root_id <<endl;

    //sort_swc process
    double thres = 10000;
    if(!SortSWC(neuron1,result,root_id,thres))
    {\
        cout<<"Error in sorting swc"<<endl;
        return false;
    }
    return true;
}
