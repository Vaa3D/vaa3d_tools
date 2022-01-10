/* neurontracing_region_advantra_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-12-15 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_region_advantra_plugin.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../v3d_main/jba/c++/convert_type2uint8.h"


using namespace std;
Q_EXPORT_PLUGIN2(neurontracing_region_advantra, neurontracing_region_advantra);

struct advantra_LS_PARA
{
    int  bkg_thresh;
    int region_number;

    int     scal;       // scale
    float   bratio;     // background ratio - stopping the trace
    int     perc;       // fg. percentile
    float   znccTh;     // correlation threshold
    int     Ndir;       // number of directions
    float   angSig;     // angular deviation
    int     Ni;         // number of iterations
    int     Ns;         // number of states
    float   zDist;      // the distance between layers in pixels

    Image4DSimple * image;

    QString inimg_file;
};

void autotrace_region_advantra(V3DPluginCallback2 &callback, QWidget *parent,advantra_LS_PARA &p,bool bmenu);

template <class T> void advantra_tracing(const T* image_region,
                                     unsigned char * data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     advantra_LS_PARA &Para,
                                     bool bmenu);


QString getAppPath();


QStringList neurontracing_region_advantra::menulist() const
{
	return QStringList() 
        <<tr("trace_advantra")
		<<tr("about");
}

QStringList neurontracing_region_advantra::funclist() const
{
	return QStringList()
        <<tr("trace_advantra")
		<<tr("help");
}

void neurontracing_region_advantra::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace_advantra"))
	{
		v3d_msg("To be implemented.");
	}
    else if (menu_name == tr("trace_advantra"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-12-15"));
	}
}

bool neurontracing_region_advantra::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("trace_advantra"))
	{
        advantra_LS_PARA P;
        bool bmenu = false;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        P.inimg_file = infiles[0];
        int k=0;
         //try to use as much as the default value in the PARA_APP2 constructor as possible
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        P.region_number = (paras.size() >= k+1) ? atoi(paras[k]) : 10000; k++;

        P.scal = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        P.bratio = (paras.size() >= k+1) ? atof(paras[k]) : 0.3; k++;
   //     P.perc = (paras.size() >= k+1) ? atoi(paras[k]) : 95; k++;
        P.znccTh = (paras.size() >= k+1) ? atof(paras[k]) : 0.6; k++;
        P.Ndir = (paras.size() >= k+1) ? atoi(paras[k]) : 15; k++;
        P.angSig = (paras.size() >= k+1) ? atof(paras[k]) : 60; k++;
        P.Ni = (paras.size() >= k+1) ? atoi(paras[k]) : 30; k++;
        P.Ns = (paras.size() >= k+1) ? atoi(paras[k]) : 5; k++;
        P.zDist = (paras.size() >= k+1) ? atof(paras[k]) : 1; k++;

        autotrace_region_advantra(callback,parent,P,bmenu);
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void autotrace_region_advantra(V3DPluginCallback2 &callback, QWidget *parent,advantra_LS_PARA &Para,bool bmenu)
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

   switch (datatype)
   {
   case 1: advantra_tracing(image_region, data1d,in_zz, callback,tmpfolder,Para,bmenu); break;
   case 2: advantra_tracing((unsigned short int *)image_region,data1d,in_zz,callback,tmpfolder,Para,bmenu); break;
   default: v3d_msg("Invalid data type. Do nothing."); return;
   }


   if(image_region) {delete []image_region; image_region = 0;}
   if(~bmenu)
       if(data1d) {delete []data1d; data1d = 0;}

   return;
}

template <class T> void advantra_tracing(const T* image_region,
                                     unsigned char* data1d,
                                     V3DLONG *in_sz,
                                     V3DPluginCallback2 &callback,
                                     QString tmpfolder,
                                     advantra_LS_PARA &Para,
                                     bool bmenu)
{

    V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz; i++)
    {
        if((unsigned short int)image_region[i] > groupNum)
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
        if((unsigned short int)image_region[i] > 0)
            groupArray[(unsigned short int)image_region[i] - 1] += 1;
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

    int groupmax = 50;
    if(groupNum <= groupmax) groupmax = groupNum;

    vector<MyMarker*> outswc_final;

    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[i] = data1d[i];
            else
                image_region_one[i] = 0;
        }

        QString advantra_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, advantra_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString advantra_swc =  advantra_image_name + "_Advantra.swc";


    #if  defined(Q_OS_LINUX)
        QString cmd_advantra = QString("%1/vaa3d -x Advantra -f advantra_func -i %2 -p %3 %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(advantra_image_name.toStdString().c_str())
                .arg(Para.scal).arg(Para.bratio).arg(Para.znccTh).arg(Para.Ndir).arg(Para.angSig).arg(Para.Ni).arg(Para.Ns).arg(Para.zDist);
        system(qPrintable(cmd_advantra));
    #else
        v3d_msg("The OS is not Linux. Do nothing.");
        return;
    #endif

        vector<MyMarker*> temp_out_swc = readSWC_file(advantra_swc.toStdString());

        for(V3DLONG d = 0; d <temp_out_swc.size(); d++)
        {
            outswc_final.push_back(temp_out_swc[d]);

        }

    }

    QString final_swc = Para.inimg_file + "_region_advantra.swc";
    saveSWC_file(final_swc.toStdString(), outswc_final);

    system(qPrintable(QString("rm -rf %1").arg(tmpfolder.toStdString().c_str())));
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

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
