/* get_sub_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-6-12 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "basic_surf_objs.h"
#include "get_sub_terafly_plugin.h"
#include "algorithm"
#include <v3d_interface.h>
#include <QtGui>
#include <string>
using namespace std;
Q_EXPORT_PLUGIN2(get_sub_terafly, GETBLOCK);
struct TRACE_LS_PARA
{
    int is_gsdt;
    int is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    int b_RadiusFrom2D;
    int block_size;
    int adap_win;
    int tracing_3D;
    int tracing_comb;
    int grid_trace;
    int global_name;
    int soma;

    V3DLONG in_sz[3];

    int  visible_thresh;//for APP1 use only

    int  seed_win; //for MOST use only
    int  slip_win; //for MOST use only


    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString tcfilename,inimg_file,rawfilename,markerfilename,swcfilename,inimg_file_2nd,inimg_file1,inimg_file2,swc_file2;
};


bool get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent,QString outimg_file);
 
QStringList GETBLOCK::menulist() const
{
	return QStringList() 
        <<tr("get_block")
		<<tr("about");
}

QStringList GETBLOCK::funclist() const
{
	return QStringList()
        <<tr("get_block")
		<<tr("help");
}

void GETBLOCK::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_block"))
	{
        QString outimg_file;
        get_sub_terafly(callback,parent,outimg_file);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-6-12"));
	}
}

bool GETBLOCK::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    TRACE_LS_PARA P;
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * image = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    //vector<char*> * image2 = (input.size() >= 3) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> paras = (image != 0) ? * image : vector<char*>();
    //vector<char*> paras2 = (image2 != 0) ? * image2 : vector<char*>();
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> outlist = (poutfiles != 0) ? * poutfiles : vector<char*>();
    //vector<char*>* outlist = NULL;

    cout<< "++++++++++++++++++++++++++++++++"<<endl;

    if (func_name == tr("get_block"))
	{

        if(infiles.empty())
        {
            cerr<<"Need input two image and one swc files"<<endl;
            return false;
        }

        P.swc_file2 = infiles[0];

        P.inimg_file1 =paras[0];
        P.inimg_file2=paras[1];

        //P.inimg_file2 =paras2[0];
        P.image = 0;

        QString swc_folder_path;
        QString image1_folder_path;
        QString image2_folder_path;
        QString fileSaveFolder;
        swc_folder_path=P.swc_file2;
        image1_folder_path=P.inimg_file1;
        image2_folder_path=P.inimg_file2;

        //fileSaveFolder="/mnt/DATA_DRIVE/mouseID_321237-17302/test";
        if (output.size()==1)
        {
            //outlist = (vector<char*>*)(output.at(0).p);
            fileSaveFolder = outlist[0];
        }
        else {printf("You must specify one result folder.\n");return false; }


        // read swc files from a folder
    cout<< "0++++++++++++++++++++++++++++++++"<<endl;
        QStringList swcList = importFileList_addnumbersort(QString(swc_folder_path));
        vector<NeuronTree> nt_list;

        cout<< "1++++++++++++++++++++++++++++++++"<<endl;

        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
            cout<< "2++++++++++++++++++++++++++++++++"<<endl;
            QString curPathSWC = swcList.at(i);

            NeuronTree temp = readSWC_file(curPathSWC);
            QList<NeuronSWC> currswc=temp.listNeuron;

            /*for(int i=0;i<currswc.size();i++)
            {
              alltheN.push_back(currswc.at(i).n);
            }
            random_shuffle(alltheN.begin(),alltheN.end());*/
            int totall=currswc.size();
            //int maxnumN=currswc.at(totall-1).n;
            cout<<"max number:\t"<<maxnumN<<endl;
            cout<< "3++++++++++++++++++++++++++++++++"<<endl;
            int arr[10]={ 0 };

            srand((unsigned)time(NULL));

            for(int i=0;i<10;i++)
            {
              arr[i]=rand() % maxnumN+0;

              QList<CellAPO> file_inmarkers;
              CellAPO t;
                  t.x = currswc.at(arr[i]).x;
                  t.y = currswc.at(arr[i]).y;
                  t.z = currswc.at(arr[i]).z;
                  t.volsize = 50;
                  t.color.r = 0;
                  t.color.g = 0;
                  t.color.b = 255;
                  file_inmarkers.push_back(t);

              QString apo_name = curPathSWC + ".random.apo";
              writeAPO_file(apo_name,file_inmarkers);
              QString linker_name = curPathSWC + ".random.ano";
              QFile qf_anofile(linker_name);
              if(!qf_anofile.open(QIODevice::WriteOnly))
              {
                  v3d_msg("Cannot open file for writing!");
                  return false;
              }
              cout<< "3++++++++++++++++++++++++++++++++"<<endl;

              QTextStream out(&qf_anofile);
              out << "SWCFILE=" << QFileInfo(curPathSWC).fileName()<<endl;
              out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
              //cout<< "Save the linker file to:"<< linker_name << endl;

              V3DLONG im_cropped_sz[4];
              unsigned char * im_cropped = 0;
              double xe=currswc.at(arr[i]).x;
              double ye=currswc.at(arr[i]).y;
              double ze=currswc.at(arr[i]).z;
              double l_x = 128;
              double l_y = 128;
              double l_z = 64;

              V3DLONG xb = xe-l_x;
              V3DLONG xn = xe+l_x-1;
              V3DLONG yb = ye-l_y;
              V3DLONG yn = ye+l_y-1;
              V3DLONG zb = ze-l_z;
              V3DLONG zn = ze+l_z-1;

              V3DLONG pagesz;
              pagesz = (xn-xb+1)*(yn-yb+1)*(zn-zb+1);
              im_cropped_sz[0] = xn-xb+1;
              im_cropped_sz[1] = yn-yb+1;
              im_cropped_sz[2] = zn-zb+1;
              im_cropped_sz[3] = 1;
              cout<< "4++++++++++++++++++++++++++++++++"<<endl;


              try {im_cropped = new unsigned char [pagesz];}
              catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return false;}
              cout<< "5++++++++++++++++++++++++++++++++"<<endl;


              im_cropped = callback.getSubVolumeTeraFly(image1_folder_path.toStdString(),xb,xn+1,
                                                        yb,yn+1,zb,zn+1);


              cout<< "6++++++++++++++++++++++++++++++++"<<endl;
              QString outimg_file;
              QString numofrandom=QString("%1").arg(arr[i]);
              QString outimg_file1= "random_check.tif";


             // outswc_file = "test.swc";
             // writeSWC_file(outswc_file,outswc);

             // export_list2file(outswc,outswc_file,outswc_file);
              //cout<<"im_cropped:\t"<<im_cropped[0]<<im_cropped[100]<<im_cropped[2]<<im_cropped[31]<<endl;
              //outimg_file=QFileInfo(fileSaveFolder).append("/").append(QString(numofrandom)).append(QString(outimg_file1));

              outimg_file=fileSaveFolder+"/"+QString(numofrandom)+QString(outimg_file1);
//              cout<<"numofrandom:\t"<<numofrandom.toStdString().c_str()<<endl;
//              cout<<"outimg_file1:\t"<<outimg_file1.toStdString().c_str()<<endl;
//              cout<<"outimg_file:\t"<<outimg_file.toStdString().c_str()<<endl;


              simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
              if(im_cropped) {delete []im_cropped; im_cropped = 0;}
            }
         }
       }
    else if (func_name == tr("help"))
      {
        v3d_msg("To be implemented.");
      }
      else return false;
      return true;
}
QStringList importFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    QDir dir(curFilePath);
    if(!dir.exists())
    {
        cout <<"Cannot find the directory";
        return myList;
    }
    foreach(QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name)) // matching the filename to file
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }
    foreach(QString qs, myList) qDebug() << qs;
    return myList;
}

