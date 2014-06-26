/* neurontracing_mip_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-06-17 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_mip_plugin.h"
#include "my_surf_objs.h"
#include "smooth_curve.h"


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
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

void autotrace_largeScale_mip(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &p);
QString getAppPath();

 
QStringList neurontracing_mip::menulist() const
{
	return QStringList() 
		<<tr("trace")
		<<tr("about");
}

QStringList neurontracing_mip::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neurontracing_mip::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("trace"))
	{
        APP2_LS_PARA P;
        mipTracingeDialog dialog(callback, parent);
        if (!dialog.image)
            return;
        if(dialog.listLandmarks.count() ==0)
            return;

        LocationSimple tmpLocation(0,0,0);
        tmpLocation = dialog.listLandmarks.at(0);
        tmpLocation.getCoord(P.root_1st[0],P.root_1st[1],P.root_1st[2]);

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

        autotrace_largeScale_mip(callback,parent,P);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2014-06-17"));
	}
}

bool neurontracing_mip::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void autotrace_largeScale_mip(V3DPluginCallback2 &callback, QWidget *parent,APP2_LS_PARA &Para)
{
    v3dhandle curwin = callback.currentImageWindow();
    Image4DSimple* p4DImage = callback.getImage(curwin);
    unsigned char* data1d = p4DImage->getRawData();
    QString image_name = p4DImage->getFileName();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG C = p4DImage->getCDim();

    int th = Para.bkg_thresh;

    QString tmpfolder = QFileInfo(image_name).path()+("/tmp");
    system(qPrintable(QString("mkdir %1").arg(tmpfolder.toStdString().c_str())));
    if(tmpfolder.isEmpty())
    {

        printf("Can not create a tmp folder!\n");
        return;
    }

    int tmpx,tmpy,tmpz;
    tmpx = Para.root_1st[0];
    tmpy = Para.root_1st[1];
    tmpz = Para.root_1st[2];



    V3DLONG pagesz_mip = N*M;
    unsigned char *image_mip=0;
    try {image_mip = new unsigned char [pagesz_mip];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}

    for(V3DLONG iy = 0; iy < M; iy++)
    {
        V3DLONG offsetj = iy*N;
        for(V3DLONG ix = 0; ix < N; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                if(data1d[offsetk + offsetj + ix] > max_mip)
                {
                    image_mip[offsetj + ix] = data1d[offsetk + offsetj + ix];
                    max_mip = data1d[offsetk + offsetj + ix];
                }

            }
        }
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

    for(V3DLONG iz = 0; iz < 1; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                V3DLONG xb = ix-1; if(xb<0) xb = 0;
                V3DLONG xe = ix+1; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy-1; if(yb<0) yb = 0;
                V3DLONG ye = iy+1; if(ye>=M-1) ye = M-1;
                V3DLONG zb = iz-1; if(zb<0) zb = 0;
                V3DLONG ze = iz+1; if(ze>=0) ze = 0;
                ii = 0;

                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
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
    in_sz[2] = 1;
    in_sz[3] = 1;
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

   unsigned char *image_region_one = new unsigned char [pagesz_mip];

   for(V3DLONG i = 0; i < pagesz_mip*datatype; i++)
   {
       if(image_region[i] == image_region[tmpy*N + tmpx])
           image_region_one[int(i/datatype)] = image_mip[int(i/datatype)];
       else
           image_region_one[int(i/datatype)] = 0;
   }

   if(image_mip) {delete []image_mip; image_mip = 0;}
   if(image_region) {delete []image_region; image_region = 0;}

   ImageMarker S;
   QList <ImageMarker> marklist;
   S.x = tmpx;
   S.y = tmpy;
   S.z = 0;
   marklist.append(S);
   QString markerpath = tmpfolder +("/root.marker");
   writeMarker_file(markerpath.toStdString().c_str(),marklist);

   QString APP2_image_name = tmpfolder + "/group_one.raw";
   simple_saveimage_wrapper(callback, APP2_image_name.toStdString().c_str(),  (unsigned char *)image_region_one, in_sz, V3D_UINT8);
   if(image_region_one) {delete []image_region_one; image_region_one = 0;}


    #if  defined(Q_OS_LINUX)
    QString cmd_APP2 = QString("%1/vaa3d -x Vaa3D_Neuron2 -f app2 -i %2 -p %3 %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(markerpath.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh - 5).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
    #elif defined(Q_OS_MAC)
        QString cmd_APP2 = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 -x Vaa3D_Neuron2 -f app2 -i %2 -p %3 %4 %5 %6 %7 %8 %9 %10").arg(getAppPath().toStdString().c_str()).arg(APP2_image_name.toStdString().c_str()).arg(markerpath.toStdString().c_str())
                .arg(Para.channel-1).arg(Para.bkg_thresh).arg(Para.b_256cube).arg(Para.b_RadiusFrom2D).arg(Para.is_gsdt).arg(Para.is_break_accept).arg(Para.length_thresh);
        system(qPrintable(cmd_APP2));
    #else
             v3d_msg("The OS is not Linux or Mac. Do nothing.");
             return;
    #endif

   QString APP2_swc = APP2_image_name + QString("_x%1_y%2_z%3_app2.swc").arg(S.x-1).arg(S.y-1).arg(S.z);
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
   vector<MyMarker*> outswc_final;
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
               loc1_t = XYZ(node->x, node->y,  P-1);

               XYZ loc0 = loc0_t;
               XYZ loc1 = loc1_t;

               nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
               farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
           }

           fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
           smooth_curve(outswc,5);


           for(V3DLONG d = 0; d <outswc.size(); d++)
           {
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
                   loc1_t = XYZ(node1->x, node1->y,  P-1);
               }
               else if(j ==1)
               {
                   loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
                   loc1_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  P-1);
               }
               else
               {
                   loc0_t = XYZ(node2->x, node2->y,  node2->z);
                   loc1_t = XYZ(node2->x, node2->y,  P-1);
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
               outswc_final.push_back(outswc[d]);

           }
           outswc.clear();
       }


   }

   QString swc_2D = image_name + "_XY_2D_mip.swc";
   system(qPrintable(QString("mv %1 %2").arg(APP2_swc.toStdString().c_str()).arg(swc_2D.toStdString().c_str())));
   system(qPrintable(QString("rm -r %1").arg(tmpfolder.toStdString().c_str())));



   QString final_swc = image_name + "_XY_3D_mip.swc";
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
   arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("30");arg.p = (void *) & arg_sort_para; input_sort << arg;
   QString full_plugin_name_sort = "sort_neuron_swc";
   QString func_name_sort = "sort_swc";
   callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

   vector<MyMarker*> temp_out_swc = readSWC_file(final_swc.toStdString());
   saveSWC_file(final_swc.toStdString(), temp_out_swc);

   v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()));

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
