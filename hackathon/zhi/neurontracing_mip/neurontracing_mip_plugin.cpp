/* neurontracing_mip_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-06-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_mip_plugin.h"
#include "my_surf_objs.h"
#include "smooth_curve.h"
#include "stackutil.h"

#include "../APP2_large_scale/readRawfile_func.h"

#include "fastmarching_linker.h"

using namespace std;
Q_EXPORT_PLUGIN2(neurontracing_mip, neurontracing_mip);

struct Point;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};

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
    int mip_plane; //0 for XY, 1 for XZ, 2 for YZ

    Image4DSimple * image;

    QString inimg_file;
    QString mip_image_file;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

void autotrace_largeScale_mip(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &p,bool bmenu);
void autotrace_largeScale_raw(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &p,bool bmenu);

QString getAppPath();

 
QStringList neurontracing_mip::menulist() const
{
	return QStringList() 
        <<tr("trace_mip")
		<<tr("about");
}

QStringList neurontracing_mip::funclist() const
{
	return QStringList()
        <<tr("trace_mip")
		<<tr("help");
}

bool saveSWC_file_TreMap(string swc_file, vector<MyMarker*> & outmarkers)
{
    if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outmarkers);

    cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<swc_file<<endl;
    map<MyMarker*, int> ind;
    ofstream ofs(swc_file.c_str());

    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<"TreMap_Tracing"<<endl;
    ofs<<"#comment "<<endl;

    ofs<<"##n,type,x,y,z,radius,parent"<<endl;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(int i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        if(parent_id == 0)  parent_id = -1;
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();
    return true;
}

void neurontracing_mip::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace_mip"))
	{
        APP2_LS_PARA P;
        bool bmenu = true;
        mipTracingeDialog dialog(callback, parent);
        if (!dialog.image)
            return;
      //  if(dialog.listLandmarks.count() ==0)
      //      return;

      //  LocationSimple tmpLocation(0,0,0);
      //  tmpLocation = dialog.listLandmarks.at(0);
      //  tmpLocation.getCoord(P.root_1st[0],P.root_1st[1],P.root_1st[2]);

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
        P.mip_plane = dialog.mip_plane;

        autotrace_largeScale_mip(callback,parent,P,bmenu);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2014-06-17"));
	}
}

bool neurontracing_mip::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("trace_mip"))
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
        P.mip_plane = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : 5; k++;

        P.cnn_type = 2;
        P.SR_ratio = 3.0/9.0;
        P.b_RadiusFrom2D = 1;

        autotrace_largeScale_mip(callback,parent,P,bmenu);

	}
    else if (func_name == tr("trace_raw"))
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
        P.mip_image_file = paras[0]; k++;
        P.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        P.bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : 10; k++;
        P.b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : 0; k++;
        P.length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : 5; k++;

        P.cnn_type = 2;
        P.SR_ratio = 3.0/9.0;
        P.b_RadiusFrom2D = 1;

        autotrace_largeScale_raw(callback,parent,P,bmenu);

    }
	else if (func_name == tr("help"))
	{

        printf("\n**** Usage of TReMAP tracing ****\n");
        printf("vaa3d -x plugin_name -f trace_mip -i <inimg_file> -p <mip_plane> <channel> <bkg_thresh> <b_256cube> <is_gsdt> <is_gap> <length_thresh>\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("mip_plane        Maximum projection plane, 0 for XY plane, 1 for XZ plane, 2 for YZ plane, Default 0\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("bkg_thresh       Default 10 (is specified as -1 then auto-thresolding)\n");

        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 0.)\n");
        printf("is_gsdt          If use gray-scale distance transform (1 for yes and 0 for no. Default 0.)\n");
        printf("is_gap           If allow gap (1 for yes and 0 for no. Default 0.)\n");
        printf("length_thresh    Default 5\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");


	}
	else return false;

	return true;
}

void autotrace_largeScale_mip(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &Para,bool bmenu)
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

    //V3DLONG tmpx,tmpy,tmpz;
    //tmpx = Para.root_1st[0];
    //tmpy = Para.root_1st[1];
    //tmpz = Para.root_1st[2];

    V3DLONG mip_sz[3];
    mip_sz[0] = N;
    mip_sz[1] = M;
    mip_sz[2] = P;
    switch (Para.mip_plane)
    {
        case 0: mip_sz[2] = 1; break;
        case 1: mip_sz[1] = 1; break;
        case 2: mip_sz[0] = 1; break;
        default:
            return;
    }
    V3DLONG pagesz_mip = mip_sz[0]*mip_sz[1]*mip_sz[2];
    unsigned char *image_mip=0;
    try {image_mip = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}

    switch (Para.mip_plane)
    {
        case 0:
                for(V3DLONG iy = 0; iy < M; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                        int max_mip = 0;
                        for(V3DLONG iz = 0; iz < P; iz++)
                        {
                            V3DLONG offsetk = iz*M*N;
                            if(data1d[offsetk + offsetj + ix] >= max_mip)
                            {
                                image_mip[iy*N + ix] = data1d[offsetk + offsetj + ix];
                                max_mip = data1d[offsetk + offsetj + ix];
                            }
                        }
                    }
                }
                break;
        case 1:
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                        int max_mip = 0;
                        for(V3DLONG iy = 0; iy < M; iy++)
                        {
                            V3DLONG offsetj = iy*N;
                            if(data1d[offsetk + offsetj + ix] >= max_mip)
                            {
                                image_mip[iz*N + ix] = data1d[offsetk + offsetj + ix];
                                max_mip = data1d[offsetk + offsetj + ix];
                            }
                        }
                    }
                }
                break;
        case 2:
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = 0; iy < N; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        int max_mip = 0;
                        for(V3DLONG ix = 0; ix < N; ix++)
                        {
                            if(data1d[offsetk + offsetj + ix] >= max_mip)
                            {
                                image_mip[iz*N + iy] = data1d[offsetk + offsetj + ix];
                                max_mip = data1d[offsetk + offsetj + ix];
                            }
                        }
                    }
                }
                break;
        default:
            return;
    }

    unsigned char *image_binary=0;
    try {image_binary = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_binary."); return;}
    for(V3DLONG i = 0; i < pagesz_mip; i++)
    {
        if(image_mip[i] > th)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }

    unsigned char *image_binary_median=0;
    try {image_binary_median = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_binary_median."); return;}

    int *arr,tmp;
    int ii,jj;
    int size = 3*3;
    arr = new int[size];

    for(V3DLONG iz = 0; iz < mip_sz[2]; iz++)
    {
        V3DLONG offsetk = iz*mip_sz[1]*mip_sz[0];
        for(V3DLONG iy = 0; iy < mip_sz[1]; iy++)
        {
            V3DLONG offsetj = iy*mip_sz[0];
            for(V3DLONG ix = 0; ix < mip_sz[0]; ix++)
            {

                V3DLONG xb = ix-1; if(xb<0) xb = 0;
                V3DLONG xe = ix+1; if(xe>=mip_sz[0]-1) xe = mip_sz[0]-1;
                V3DLONG yb = iy-1; if(yb<0) yb = 0;
                V3DLONG ye = iy+1; if(ye>=mip_sz[1]-1) ye = mip_sz[1]-1;
                V3DLONG zb = iz-1; if(zb<0) zb = 0;
                V3DLONG ze = iz+1; if(ze>=mip_sz[2]-1) ze = mip_sz[2]-1;
                ii = 0;

                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*mip_sz[1]*mip_sz[0];
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*mip_sz[0];
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                            int dataval = image_binary[offsetkl + offsetjl + i];
                            arr[ii] = dataval;
                            if (ii>0)
                            {
                                jj = ii;
                                while(jj > 0 && arr[jj-1]>arr[jj])
                                {
                                    tmp = arr[jj];
                                    arr[jj] = arr[jj-1];
                                    arr[jj-1] = tmp;
                                    jj--;
                                }
                            }
                            ii++;
                        }
                    }
                }

                V3DLONG index_pim = offsetk + offsetj + ix;
                image_binary_median[index_pim] = arr[int(0.5*ii)+1];
            }
        }
    }


    if(image_binary) {delete []image_binary; image_binary = 0;}
    V3DLONG in_sz[4];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = 1;
    switch (Para.mip_plane)
    {
        case 0: in_sz[2] = 1; break;
        case 1: in_sz[1] = 1; break;
        case 2: in_sz[0] = 1; break;
        default:
            return;
    }
    QString input_image_name = tmpfolder + "/binary_median.raw";
    simple_saveimage_wrapper(callback, input_image_name.toStdString().c_str(),  (unsigned char *)image_binary_median, in_sz, V3D_UINT8);

    if(image_binary_median) {delete []image_binary_median; image_binary_median = 0;}

    QString output_image_name = tmpfolder +"/region.raw";

    #if  defined(Q_OS_LINUX)
        QString cmd_region = QString("%1/vaa3d -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 200").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str());
        system(qPrintable(cmd_region));
    #elif defined(Q_OS_MAC)
        QString cmd_region = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x regiongrow -f rg -i %2 -o %3 -p 1 0 1 200").arg(getAppPath().toStdString().c_str()).arg(input_image_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str());
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
   for(V3DLONG i = 0; i < pagesz_mip; i++)
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

   for(V3DLONG i = 0; i < pagesz_mip; i++)
   {
       if(image_region[i] > 0)
           groupArray[image_region[i] - 1] += 1;
   }

   int tmp_index;
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


   int groupmax = 15;
   if(groupNum <= groupmax) groupmax = groupNum;
   vector<MyMarker*> outswc_final;

   for(int dd = 0; dd < groupmax; dd++)
   {
       unsigned char *image_region_one = new unsigned char [pagesz_mip];
       V3DLONG group_type = groupIndex[dd];


       for(V3DLONG i = 0; i < pagesz_mip*datatype; i++)
       {

           if(image_region[i] == group_type)
               image_region_one[int(i/datatype)] = image_mip[int(i/datatype)];
           else
               image_region_one[int(i/datatype)] = 0;
       }

       /* switch (Para.mip_plane)
        {
            case 0: group_type = image_region[tmpy*N + tmpx]; break;
            case 1: group_type = image_region[tmpz*N + tmpx]; break;
            case 2: group_type = image_region[tmpz*M + tmpy];break;
            default:
                return;
        }


       tmpx /= groupArray[0];
       tmpy /= groupArray[0];
       tmpz = 0;

       v3d_msg(QString("x is %1 and y is %2").arg(tmpx).arg(tmpy));
       return;

       if(image_mip) {delete []image_mip; image_mip = 0;}
       if(image_region) {delete []image_region; image_region = 0;}

       ImageMarker S;
       QList <ImageMarker> marklist;
       S.x = tmpx;
       S.y = tmpy;
       S.z = tmpz;
       switch (Para.mip_plane)
       {
           case 0: S.z = 0; break;
           case 1: S.y = 0; break;
           case 2: S.x = 0; break;
           default:
               return;
       }

       marklist.append(S);
       QString markerpath = tmpfolder +("/root.marker");
       writeMarker_file(markerpath.toStdString().c_str(),marklist);*/

       QString APP2_image_name = tmpfolder + "/group_one.raw";
       simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
       if(image_region_one) {delete []image_region_one; image_region_one = 0;}

       QString APP2_swc =  APP2_image_name + QString("_group_%1.swc").arg(group_type);


        #if  defined(Q_OS_LINUX)
            QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
            system(qPrintable(cmd_APP2));
            QString cmd_resample = QString("%1/vaa3d -x resample_swc -f resample_swc -i %2 -o %3 -p 2").arg(getAppPath().toStdString().c_str()).arg(APP2_swc.toStdString().c_str()).arg(APP2_swc.toStdString().c_str());
            system(qPrintable(cmd_resample));
        #elif defined(Q_OS_MAC)
            QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                    .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
            system(qPrintable(cmd_APP2));
            QString cmd_resample = QString("%1//vaa3d64.app/Contents/MacOS/vaa3d64 -x resample_swc -f resample_swc -i %2 -o %3 -p 2").arg(getAppPath().toStdString().c_str()).arg(APP2_swc.toStdString().c_str()).arg(APP2_swc.toStdString().c_str());
            system(qPrintable(cmd_resample));
        #else
                 v3d_msg("The OS is not Linux or Mac. Do nothing.");
                 return;
        #endif

       /*QString APP2_swc;
       switch (Para.mip_plane)
       {
           case 0: APP2_swc = APP2_image_name + QString("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z); break;
           case 1: APP2_swc = APP2_image_name + QString("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y).arg(S.z-1); break;
           case 2: APP2_swc = APP2_image_name + QString("_x%1_y%2_z%3_app2.swc").arg(S.x).arg(S.y-1).arg(S.z-1); break;
           default:
               return;
       }*/

       NeuronTree nt = readSWC_file(APP2_swc);

       V3DLONG siz = nt.listNeuron.size();
       Tree tree;
       for (V3DLONG i=0;i<siz;i++)
       {
           NeuronSWC s = nt.listNeuron[i];
           Point* pt = new Point;
           pt->x = s.x;
           pt->y = s.y;
           pt->z = s.z;
           pt->r = s.r;
           pt ->type = s.type;
           pt->p = NULL;
           pt->childNum = 0;
           tree.push_back(pt);
       }
       for (V3DLONG i=0;i<siz;i++)
       {
           if (nt.listNeuron[i].pn<0) continue;
           V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
           tree[i]->p = tree[pid];
           tree[pid]->childNum++;
       }
    //	printf("tree constructed.\n");
       vector<Segment*> seg_list;
       for (V3DLONG i=0;i<siz;i++)
       {
           if (tree[i]->childNum!=1)//tip or branch point
           {
               Segment* seg = new Segment;
               Point* cur = tree[i];
               do
               {
                   seg->push_back(cur);
                   cur = cur->p;
               }
               while(cur && cur->childNum==1);
               seg_list.push_back(seg);
           }
       }

       vector<MyMarker*> outswc;

       for (V3DLONG i=0;i<seg_list.size();i++)
       {
           vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
           nearpos_vec.clear();
           farpos_vec.clear();
           if(seg_list[i]->size() > 2)
           {
               for (V3DLONG j=0;j<seg_list[i]->size();j++)
               {
                   Point* node = seg_list[i]->at(j);
                   XYZ loc0_t, loc1_t;
                   loc0_t = XYZ(node->x, node->y,  node->z);
                   switch (Para.mip_plane)
                   {
                       case 0:  loc1_t = XYZ(node->x, node->y,  P-1); break;
                       case 1:  loc1_t = XYZ(node->x, M-1,  node->z); break;
                       case 2:  loc1_t = XYZ(N-1, node->y,  node->z); break;
                       default:
                           return;
                   }

                   XYZ loc0 = loc0_t;
                   XYZ loc1 = loc1_t;

                   nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
                   farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
               }

               fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
               smooth_curve(outswc,5);


               for(V3DLONG d = 0; d <outswc.size(); d++)
               {
                   outswc[d]->radius = 2;
                   outswc[d]->type = dd + 2;
                   outswc_final.push_back(outswc[d]);

               }
               outswc.clear();

           }
           else if(seg_list[i]->size() == 2)
           {
               Point* node1 = seg_list[i]->at(0);
               Point* node2 = seg_list[i]->at(1);

               for (V3DLONG j=0;j<3;j++)
               {
                   XYZ loc0_t, loc1_t;
                   if(j ==0)
                   {
                       loc0_t = XYZ(node1->x, node1->y,  node1->z);
                       switch (Para.mip_plane)
                       {
                           case 0:  loc1_t = XYZ(node1->x, node1->y,  P-1); break;
                           case 1:  loc1_t = XYZ(node1->x, M-1,  node1->z); break;
                           case 2:  loc1_t = XYZ(N-1, node1->y,  node1->z); break;
                           default:
                               return;
                       }
                   }
                   else if(j ==1)
                   {
                       loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
                       switch (Para.mip_plane)
                       {
                           case 0:  loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1); break;
                           case 1:  loc1_t = XYZ(0.5*(node1->x + node2->x), M-1,   0.5*(node1->z + node2->z)); break;
                           case 2:  loc1_t = XYZ(N-1,  0.5*(node1->y + node2->y),   0.5*(node1->z + node2->z)); break;
                           default:
                               return;
                       }
                   }
                   else
                   {
                       loc0_t = XYZ(node2->x, node2->y,  node2->z);
                       switch (Para.mip_plane)
                       {
                           case 0:  loc1_t = XYZ(node2->x, node2->y,  P-1); break;
                           case 1:  loc1_t = XYZ(node2->x, M-1,  node2->z); break;
                           case 2:  loc1_t = XYZ(N-1, node2->y,  node2->z); break;
                           default:
                               return;
                       }               }

                   XYZ loc0 = loc0_t;
                   XYZ loc1 = loc1_t;

                   nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
                   farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
                }

               fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
               smooth_curve(outswc,5);


               for(V3DLONG d = 0; d <outswc.size(); d++)
               {
                   outswc[d]->radius = 2;
                   outswc[d]->type = dd + 2;
                   outswc_final.push_back(outswc[d]);

               }
               outswc.clear();
           }
       }
    }

   QString swc_2D,final_swc;
   switch (Para.mip_plane)
   {
       case 0: swc_2D = image_name + "_XY_2D_mip.swc"; final_swc = image_name + "_XY_3D_TreMap.swc"; break;
       case 1: swc_2D = image_name + "_XZ_2D_mip.swc"; final_swc = image_name + "_XZ_3D_TreMap.swc"; break;
       case 2: swc_2D = image_name + "_YZ_2D_mip.swc"; final_swc = image_name + "_YZ_3D_TreMap.swc"; break;
       default:
           return;
   }

   saveSWC_file(final_swc.toStdString(), outswc_final);

   // system(qPrintable(QString("mv %1 %2").arg(APP2_swc.toStdString().c_str()).arg(swc_2D.toStdString().c_str())));
   system(qPrintable(QString("rm -r %1").arg(tmpfolder.toStdString().c_str())));

   V3DPluginArgItem arg;
   V3DPluginArgList input_resample;
   V3DPluginArgList input_sort;
   V3DPluginArgList output;

   arg.type = "random";std::vector<char*> arg_input_resample;
   std:: string fileName_Qstring(final_swc.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
   arg_input_resample.push_back(fileName_string);
   arg.p = (void *) & arg_input_resample; input_resample<< arg;
   arg.type = "random";std::vector<char*> arg_resample_para; arg_resample_para.push_back("10");arg.p = (void *) & arg_resample_para; input_resample << arg;
   arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;

   QString full_plugin_name_resample = "resample_swc";
   QString func_name_resample = "resample_swc";
   callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);

   arg.type = "random";std::vector<char*> arg_input_sort;
   arg_input_sort.push_back(fileName_string);
   arg.p = (void *) & arg_input_sort; input_sort<< arg;
   arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("20");arg.p = (void *) & arg_sort_para; input_sort << arg;
   QString full_plugin_name_sort = "sort_neuron_swc";
   QString func_name_sort = "sort_swc";
   callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

   vector<MyMarker*> temp_out_swc = readSWC_file(final_swc.toStdString());
   saveSWC_file_TreMap(final_swc.toStdString(), temp_out_swc);

   v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()),bmenu);

   return;
}

void autotrace_largeScale_raw(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &Para,bool bmenu)
{
    QString image_name = Para.inimg_file;
    QString image_mip_name = Para.mip_image_file;

    unsigned char *image_mip=0;
    V3DLONG *im_sz = 0;
    int datatype = 0;
    if (loadImage(const_cast<char *>(image_mip_name.toStdString().c_str()), image_mip, im_sz, datatype)!=true)
    {
        fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",image_name.toStdString().c_str());
        return;
    }

    QString tmpfolder = QFileInfo(image_name).path()+("/") + QFileInfo(image_name).completeBaseName()+("_tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return;
    }

    QString output_image_name = tmpfolder +"/region.raw";

    #if  defined(Q_OS_LINUX)
    QString cmd_region = QString("%1/vaa3d -x regiongrow -f rg -i %2 -o %3 -p 1 2 1 200 %4").arg(getAppPath().toStdString().c_str()).arg(image_mip_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.bkg_thresh);
        system(qPrintable(cmd_region));
    #elif defined(Q_OS_MAC)
        QString cmd_region = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x regiongrow -f rg -i %2 -o %3 -p 1 2 1 200 %4").arg(getAppPath().toStdString().c_str()).arg(image_mip_name.toStdString().c_str()).arg(output_image_name.toStdString().c_str()).arg(Para.bkg_thresh);
        system(qPrintable(cmd_region));
    #else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
    #endif

    unsigned char * image_region = 0;
    V3DLONG in_zz[4];
    if(!simple_loadimage_wrapper(callback, output_image_name.toStdString().c_str(), image_region, in_zz, datatype))
    {
       v3d_msg("Fail to load image");
       return;
    }

    V3DLONG pagesz_mip = in_zz[0]*in_zz[1]*in_zz[2];
    int groupNum = 0;
    for(V3DLONG i = 0; i < pagesz_mip; i++)
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

    for(V3DLONG i = 0; i < pagesz_mip; i++)
    {
        if(image_region[i] > 0)
            groupArray[image_region[i] - 1] += 1;
    }

    int tmp_index,tmp;
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


    int groupmax = 15;
    if(groupNum <= groupmax) groupmax = groupNum;
    vector<MyMarker*> outswc_final;
    for(int dd = 0; dd < groupmax; dd++)
    {
        unsigned char *image_region_one = new unsigned char [pagesz_mip];
        V3DLONG group_type = groupIndex[dd];


        for(V3DLONG i = 0; i < pagesz_mip*datatype; i++)
        {

            if(image_region[i] == group_type)
                image_region_one[int(i/datatype)] = image_mip[int(i/datatype)];
            else
                image_region_one[int(i/datatype)] = 0;
        }

        QString APP2_image_name = tmpfolder + "/group_one.raw";
        simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, im_sz, V3D_UINT8);
        if(image_region_one) {delete []image_region_one; image_region_one = 0;}

        QString APP2_swc =  APP2_image_name + QString("_group_%1.swc").arg(group_type);


#if  defined(Q_OS_LINUX)
        QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
        QString cmd_resample = QString("%1/vaa3d -x resample_swc -f resample_swc -i %2 -o %3 -p 2").arg(getAppPath().toStdString().c_str()).arg(APP2_swc.toStdString().c_str()).arg(APP2_swc.toStdString().c_str());
        system(qPrintable(cmd_resample));
#elif defined(Q_OS_MAC)
        QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -o %3 -p NULL %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(APP2_swc.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
        QString cmd_resample = QString("%1//vaa3d64.app/Contents/MacOS/vaa3d64 -x resample_swc -f resample_swc -i %2 -o %3 -p 2").arg(getAppPath().toStdString().c_str()).arg(APP2_swc.toStdString().c_str()).arg(APP2_swc.toStdString().c_str());
        system(qPrintable(cmd_resample));
#else
        v3d_msg("The OS is not Linux or Mac. Do nothing.");
        return;
#endif
        NeuronTree nt = readSWC_file(APP2_swc);

        V3DLONG siz = nt.listNeuron.size();
        Tree tree;
        for (V3DLONG i=0;i<siz;i++)
        {
            NeuronSWC s = nt.listNeuron[i];
            Point* pt = new Point;
            pt->x = s.x;
            pt->y = s.y;
            pt->z = s.z;
            pt->r = s.r;
            pt ->type = s.type;
            pt->p = NULL;
            pt->childNum = 0;
            tree.push_back(pt);
        }
        for (V3DLONG i=0;i<siz;i++)
        {
            if (nt.listNeuron[i].pn<0) continue;
            V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
            tree[i]->p = tree[pid];
            tree[pid]->childNum++;
        }
     //	printf("tree constructed.\n");
        vector<Segment*> seg_list;
        for (V3DLONG i=0;i<siz;i++)
        {
            if (tree[i]->childNum!=1)//tip or branch point
            {
                Segment* seg = new Segment;
                Point* cur = tree[i];
                do
                {
                    seg->push_back(cur);
                    cur = cur->p;
                }
                while(cur && cur->childNum==1);
                seg_list.push_back(seg);
            }
        }

        vector<MyMarker*> outswc;

        unsigned char * data1d = 0;
        V3DLONG *im3D_zz = 0;
        V3DLONG *im3D_sz = 0;

        int datatype;
        if (!loadRawRegion(const_cast<char *>(image_name.toStdString().c_str()), data1d, im3D_zz, im3D_sz,datatype,0,0,0,1,1,1))
        {
            return;
        }

        if(data1d) {delete []data1d; data1d = 0;}
        V3DLONG N = im3D_zz[0];
        V3DLONG M = im3D_zz[1];
        V3DLONG P = im3D_zz[2];


        for (V3DLONG i=0;i<seg_list.size();i++)
        {
            V3DLONG xb = N-1;
            V3DLONG xe = 0;
            V3DLONG yb = M-1;
            V3DLONG ye = 0;

            for (V3DLONG j=0;j<seg_list[i]->size();j++)
            {
                Point* node = seg_list[i]->at(j);
                if(node->x < xb) xb = node->x;
                if(node->x > xe) xe = node->x;
                if(node->y < yb) yb = node->y;
                if(node->y > ye) ye = node->y;
            }

            vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
            nearpos_vec.clear();
            farpos_vec.clear();
            if(seg_list[i]->size() > 2)
            {
                for (V3DLONG j=0;j<seg_list[i]->size();j++)
                {
                    Point* node = seg_list[i]->at(j);
                    XYZ loc0_t, loc1_t;
                    loc0_t = XYZ(node->x, node->y,  node->z);
                    loc1_t = XYZ(node->x, node->y,  P-1);
                    XYZ loc0 = loc0_t;
                    XYZ loc1 = loc1_t;

                    nearpos_vec.push_back(MyMarker(loc0.x - xb, loc0.y - yb, loc0.z));
                    farpos_vec.push_back(MyMarker(loc1.x - xb, loc1.y - yb, loc1.z));
                }

                if (!loadRawRegion(const_cast<char *>(image_name.toStdString().c_str()), data1d, im3D_zz, im3D_sz,datatype,xb,yb,0,xe+1,ye+1,P))
                {
                    printf("can not load the region");
                    if(data1d) {delete []data1d; data1d = 0;}
                    return;
                }

                fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, xe-xb+1,ye-yb+1,P, 1, 5);
                smooth_curve(outswc,5);

//                QString APP2_image_name = tmpfolder +  QString("/raw_%1.raw").arg(i);
//                simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)data1d, im3D_sz, V3D_UINT8);
//                QString final_swc = tmpfolder + QString("/raw_%1.swc").arg(i);
//                saveSWC_file(final_swc.toStdString(), outswc);

                for(V3DLONG d = 0; d <outswc.size(); d++)
                {
                    outswc[d]->radius = 2;
                    outswc[d]->type = dd + 2;
                    outswc[d]->x = outswc[d]->x + xb;
                    outswc[d]->y = outswc[d]->y + yb;
                    outswc_final.push_back(outswc[d]);
                }
                if(data1d) {delete []data1d; data1d = 0;}
                outswc.clear();
            }
            else if(seg_list[i]->size() == 2)
            {
                Point* node1 = seg_list[i]->at(0);
                Point* node2 = seg_list[i]->at(1);

                for (V3DLONG j=0;j<3;j++)
                {
                    XYZ loc0_t, loc1_t;
                    if(j ==0)
                    {
                        loc0_t = XYZ(node1->x, node1->y,  node1->z);
                        loc1_t = XYZ(node1->x, node1->y,  P-1);
                    }
                    else if(j ==1)
                    {
                        loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
                        loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1);

                    }
                    else
                    {
                        loc0_t = XYZ(node2->x, node2->y,  node2->z);
                        loc1_t = XYZ(node2->x, node2->y,  P-1);
                    }

                    XYZ loc0 = loc0_t;
                    XYZ loc1 = loc1_t;

                    nearpos_vec.push_back(MyMarker(loc0.x - xb, loc0.y - yb, loc0.z));
                    farpos_vec.push_back(MyMarker(loc1.x - xb, loc1.y - yb, loc1.z));
                 }

                if (!loadRawRegion(const_cast<char *>(image_name.toStdString().c_str()), data1d, im3D_zz, im3D_sz,datatype,xb,yb,0,xe+1,ye+1,P))
                {
                    printf("can not load the region");
                    if(data1d) {delete []data1d; data1d = 0;}
                    return;
                }

                fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, xe-xb+1,ye-yb+1,P, 1, 5);
                smooth_curve(outswc,5);


                for(V3DLONG d = 0; d <outswc.size(); d++)
                {
                    outswc[d]->radius = 2;
                    outswc[d]->type = dd + 2;
                    outswc[d]->x = outswc[d]->x + xb;
                    outswc[d]->y = outswc[d]->y + yb;
                    outswc_final.push_back(outswc[d]);
                }
                if(data1d) {delete []data1d; data1d = 0;}
                outswc.clear();
            }
        }
    }

    QString final_swc = image_name + "_raw_3D_TreMap.swc";
    system(qPrintable(QString("rm -r %1").arg(tmpfolder.toStdString().c_str())));

    saveSWC_file(final_swc.toStdString(), outswc_final);

    V3DPluginArgItem arg;
    V3DPluginArgList input_resample;
    V3DPluginArgList input_sort;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> arg_input_resample;
    std:: string fileName_Qstring(final_swc.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
    arg_input_resample.push_back(fileName_string);
    arg.p = (void *) & arg_input_resample; input_resample<< arg;
    arg.type = "random";std::vector<char*> arg_resample_para; arg_resample_para.push_back("10");arg.p = (void *) & arg_resample_para; input_resample << arg;
    arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;

    QString full_plugin_name_resample = "resample_swc";
    QString func_name_resample = "resample_swc";
    callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);

    arg.type = "random";std::vector<char*> arg_input_sort;
    arg_input_sort.push_back(fileName_string);
    arg.p = (void *) & arg_input_sort; input_sort<< arg;
    arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("20");arg.p = (void *) & arg_sort_para; input_sort << arg;
    QString full_plugin_name_sort = "sort_neuron_swc";
    QString func_name_sort = "sort_swc";
    callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

    vector<MyMarker*> temp_out_swc = readSWC_file(final_swc.toStdString());
    saveSWC_file_TreMap(final_swc.toStdString(), temp_out_swc);

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
