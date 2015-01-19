/* neurontracing_region_app2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-01-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_region_app2_plugin.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"


using namespace std;
Q_EXPORT_PLUGIN2(neurontracing_region_app2, neurontracing_region_app2);

struct APP2_LS_PARA
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
    int root_1st[3];
    int region_number;

    Image4DSimple * image;

    QString inimg_file;
};


void autotrace_region_app2(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &p,bool bmenu);
QString getAppPath();

 
QStringList neurontracing_region_app2::menulist() const
{
	return QStringList() 
        <<tr("trace")
		<<tr("about");
}

QStringList neurontracing_region_app2::funclist() const
{
	return QStringList()
        <<tr("trace")
		<<tr("help");
}

void neurontracing_region_app2::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace"))
	{
        APP2_LS_PARA P;
        bool bmenu = true;
        regionAPP2Dialog dialog(callback, parent);
        if (!dialog.image)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;
        P.inimg_file = dialog.image->getFileName();

        P.is_gsdt = dialog.is_gsdt;
        P.is_break_accept = dialog.is_break_accept;
        P.bkg_thresh = dialog.bkg_thresh;
        P.length_thresh = dialog.length_thresh;
        P.cnn_type = dialog.cnn_type;
        P.channel = dialog.channel;
        P.SR_ratio = dialog.SR_ratio;
        P.b_256cube = dialog.b_256cube;
        P.b_RadiusFrom2D = dialog.b_RadiusFrom2D;
        P.region_number = dialog.region_number;

        autotrace_region_app2(callback,parent,P,bmenu);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-01-16"));
	}
}

bool neurontracing_region_app2::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("trace"))
	{
        APP2_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value in the PARA_APP2 constructor as possible
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : 5; k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        P.cnn_type = 2;
        P.SR_ratio = 3.0/9.0;
        P.b_RadiusFrom2D = 1;

        autotrace_region_app2(callback,parent,P,bmenu);

    }
	else if (func_name == tr("help"))
	{
        printf("\n**** Usage of Region based APP2 tracing ****\n");
        printf("vaa3d -x plugin_name -f trace -i <inimg_file> -p  <channel> <bkg_thresh> <b_256cube> <is_gsdt> <is_gap> <length_thresh> <region_number>\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10 (is specified as -1 then auto-thresolding)\n");

        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 0.)\n");
        printf("is_gsdt          If use gray-scale distance transform (1 for yes and 0 for no. Default 0.)\n");
        printf("is_gap           If allow gap (1 for yes and 0 for no. Default 0.)\n");
        printf("length_thresh    Default 5\n");
        printf("region_number    Default 10000\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void autotrace_region_app2(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &Para,bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,C;
    QString image_name = Para.inimg_file;

    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        Image4DSimple* p4DImage = callback.getImage(curwin);
        data1d = p4DImage->getRawData();

        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        C = p4DImage->getCDim();
    }
    else
    {
        V3DLONG *im_sz = 0;
        int datatype = 0;
        if (loadImage(const_cast<char *>(image_name.toStdString().c_str()), data1d, im_sz, datatype)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",image_name.toStdString().c_str());
            return;
        }
        N = im_sz[0];
        M = im_sz[1];
        P = im_sz[2];
        C = im_sz[3];

    }

    int th = Para.bkg_thresh;
    QString tmpfolder = QFileInfo(image_name).path()+("/") + QFileInfo(image_name).completeBaseName()+("_tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return;
    }

    V3DLONG pagesz = N*M*P;
    unsigned char *image_binary=0;
    try {image_binary = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_binary."); return;}
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if(data1d[i] > th)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }

    V3DLONG in_sz[4];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = 1;

    QString input_image_name = tmpfolder + "/binary.raw";
    simple_saveimage_wrapper(callback, input_image_name.toStdString().c_str(),  (unsigned char *)image_binary, in_sz, V3D_UINT8);

    if(image_binary) {delete []image_binary; image_binary = 0;}

    QString output_image_name = tmpfolder +"/region.raw";

    #if  defined(Q_OS_LINUX)
        QString cmd_region = QString("%1/vaa3d -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.region_number);
        system(qPrintable(cmd_region));
    #elif defined(Q_OS_MAC)
        QString cmd_region = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 %4").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.region_number);
        system(qPrintable(cmd_region));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

   unsigned char * image_region = 0;
   int datatype;
   V3DLONG in_zz[4];
   if(!simple_loadimage_wrapper(callback, output_image_name.toStdString().c_str(), image_region, in_zz, datatype))
   {
       v3d_msg("Fail to load image");
       return;
   }

   int groupNum = 0;
   for(V3DLONG i = 0; i < pagesz; i++)
   {
       if(image_region[i] > groupNum)
           groupNum = image_region[i];
   }

   int *groupArray = new int[groupNum];
   int *groupIndex = new int[groupNum];

   for(int i = 0; i < groupNum; i++)
   {
       groupArray[i] = 0;
       groupIndex[i] = i+1;
   }

   for(V3DLONG i = 0; i < pagesz; i++)
   {
       if(image_region[i] > 0)
           groupArray[image_region[i] - 1] += 1;
   }

   int tmp,tmp_index;
   for(V3DLONG i = 0; i < groupNum; i++)
   {
       if (i > 0)
       {
           V3DLONG j = i;
           while(j > 0 && groupArray[j-1]<groupArray[j])
           {
               tmp = groupArray[j];
               groupArray[j] = groupArray[j-1];
               groupArray[j-1] = tmp;

               tmp_index = groupIndex[j];
               groupIndex[j] = groupIndex[j-1];
               groupIndex[j-1] = tmp_index;

               j--;
           }
       }
   }


   int groupmax = groupNum;
   vector<MyMarker*> outswc_final;

   for(int dd = 0; dd < groupmax; dd++)
   {
       unsigned char *image_region_one = new unsigned char [pagesz];
       V3DLONG group_type = groupIndex[dd];


       for(V3DLONG i = 0; i < pagesz*datatype; i++)
       {

           if(image_region[i] == group_type)
               image_region_one[int(i/datatype)] = data1d[int(i/datatype)];
           else
               image_region_one[int(i/datatype)] = 0;
       }


       QString APP2_image_name = tmpfolder + "/group_one.raw";
       simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
       if(image_region_one) {delete []image_region_one; image_region_one = 0;}

       QString APP2_swc =  APP2_image_name + QString("_group_%1.swc").arg(group_type);


        #if  defined(Q_OS_LINUX)
            QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
            system(qPrintable(cmd_APP2));
        #elif defined(Q_OS_MAC)
            QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
            system(qPrintable(cmd_APP2));
        #else
                 v3d_msg("The OS is not Linux or Mac. Do nothing.");
                 return;
        #endif

       vector<MyMarker*> temp_out_swc = readSWC_file(APP2_swc.toStdString());

       for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
       {
           outswc_final.push_back(temp_out_swc[d]);

       }

    }

    if(image_region) {delete []image_region; image_region = 0;}
    if(~bmenu)
        if(data1d) {delete []data1d; data1d = 0;}


    QString final_swc = image_name + "_region_APP2.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

   return;
}


QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}
