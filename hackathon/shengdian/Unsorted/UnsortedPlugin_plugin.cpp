/* UnsortedPlugin_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-8-29 : by Shengdian
 */
 
#include "v3d_message.h"
#include <vector>
#include "UnsortedPlugin_plugin.h"
#include "volimg_proc.h"
#include <QHash>
#include <fstream>

using namespace std;
Q_EXPORT_PLUGIN2(UnsortedPlugin, UnsortedPlugin);
 
QStringList UnsortedPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList UnsortedPlugin::funclist() const
{
	return QStringList()
            <<tr("ReconstructionComplexity")
           <<tr("RecontructionIntensity_terafly")
          <<tr("Crop_terafly_block")
            <<tr("SomaRefinement")
         <<tr("help");
}

void UnsortedPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Shengdian, 2020-8-29"));
	}
}

bool UnsortedPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name==tr("Crop_terafly_block"))
    {
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        /*crop img from terafly
         *Input img is highest resolution img path
         *Input apo with dst marker in it
         *Input para for crop size of x, y and z
         *output path is the save path for img block
        */
        string inimg_file = infiles[0];
        string inapo_file = infiles[1];
        int cropx=(inparas.size()>=1)?atoi(inparas[0]):1024;
        int cropy=(inparas.size()>=2)?atoi(inparas[1]):1024;
        int cropz=(inparas.size()>=3)?atoi(inparas[2]):512;
        string out_path=outfiles[0];
        QList <CellAPO> apolist=readAPO_file(QString::fromStdString(inapo_file));
        if(apolist.size()>0)
        {
            getTeraflyBlock(callback,inimg_file,apolist,out_path,cropx,cropy,cropz);
        }
        else
            cout<<"apo size is zero"<<endl;
        cout<<"done"<<endl;
    }
    else if (func_name==tr("SomaRefinement"))
    {
        /*if the distance between the two somata are less than 10, remove one of this
        */
        string inimg_file;
        string inapo_file;
//        QString::fromStdString(inapo_file)
        cout<<"size of the input file is "<<infiles.size()<<endl;
        cout<<"size of the inpara file is "<<inparas.size()<<endl;
        cout<<"size of the output file is "<<outfiles.size()<<endl;
        if(infiles.size()==1)
        {
            inapo_file = infiles[0];
        }
        else if(infiles.size()==2)
        {
            inimg_file = infiles[0];
            inapo_file = infiles[1];
        }
        else
        {
            cout<<"Input error"<<endl;
            return false;
        }
        int dis_thre=(inparas.size()>=1)?atoi(inparas[0]):10;

        QList <CellAPO> apolist=readAPO_file(QString::fromStdString(inapo_file));
        QList <CellAPO> apolist_out;apolist_out.clear();
        for(V3DLONG i=0;i<(apolist.size()-1);i++)
        {
            bool is_single=false;
            CellAPO thisapo=apolist[i];

            for(V3DLONG j=i+1;j<apolist.size();j++)
            {
                CellAPO compareapo=apolist[j];
                double distance_somata=(thisapo.x-compareapo.x)*(thisapo.x-compareapo.x)
                        +(thisapo.y-compareapo.y)*(thisapo.y-compareapo.y)
                        +(thisapo.z-compareapo.z)*(thisapo.z-compareapo.z);
                if(distance_somata<dis_thre*dis_thre)
                {
                    is_single=true;
                    break;
                }
            }
            if(is_single==false)
            {
                apolist_out.push_back(thisapo);
            }

        }
        cout<<"APO size is "<<apolist.size()<<",New APO size is "<<apolist_out.size()<<endl;
        cout<<"Remove false marker num = "<<(apolist.size()-apolist_out.size())<<endl;
        //save to file
        string save_apo_file=inapo_file+"_refined.apo";
        writeAPO_file(QString::fromStdString(save_apo_file),apolist_out);

    }
    else if(func_name ==tr("RecontructionIntensity_terafly"))
    {
        /*Input:
         *1. (highest resolution) terafly img path
         *2. <dendrite>.swc/.eswc
         *get the statistics infor of dendrite img
         *out the infor to the swc name
        */
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
        getSWCIntensityInTerafly(callback,inimg_file,QString::fromStdString(inswc_file));
    }
    else if (func_name == tr("ReconstructionComplexity"))
    {
        /*Input: img path*/
        if(infiles.size() != 1)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        int threshold=(inparas.size()>=1)?atoi(inparas[0]):1;
        cout<<"In threshold "<<threshold<<endl;
        //read img
        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
         cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
         long sz01 = in_sz[0] * in_sz[1];
         long sz0 = in_sz[0];
         double total_num, background_num;

         double imgave,imgstd;
         V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
         mean_and_std(inimg1d,total_size,imgave,imgstd);
         cout<<"img avearge intensity="<<imgave<<endl;
         cout<<"img std intensity="<<imgstd<<endl;

         //save
         unsigned char * im_cropped = 0;
         V3DLONG pagesz=total_size;
         try {im_cropped = new unsigned char [pagesz];}
         catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return false;}
         im_cropped=inimg1d;
         bool issaved=false;
         int finalth=imgave;
         for(int th=(imgave+1*imgstd);th<(imgave+7*imgstd);th++)
         {
             total_num = background_num = 0;
             for(V3DLONG ix=0;ix<in_sz[0];ix++)
             {
                 for(V3DLONG iy=0;iy<in_sz[1];iy++)
                 {
                     for(V3DLONG iz=0;iz<in_sz[2];iz++)
                     {
                         total_num++;
                         int thisx,thisy,thisz;
                         thisx=ix;
                         thisy=iy;
                         thisz=iz;
                         float thisNodeIntensity=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                         if(thisNodeIntensity<th)
                         {
                             background_num++;
                         }

                     }
                 }
             }
             float rcomplexity=1-background_num/total_num;
             cout<<"bg threhold :"<<th<<", ratio :"<<rcomplexity<<endl;
             //save the img
             if(rcomplexity*1000<threshold && !issaved)
             {
                 issaved=true;
                 for(V3DLONG ix=0;ix<in_sz[0];ix++)
                 {
                     for(V3DLONG iy=0;iy<in_sz[1];iy++)
                     {
                         for(V3DLONG iz=0;iz<in_sz[2];iz++)
                         {
                             total_num++;
                             int thisx,thisy,thisz;
                             thisx=ix;
                             thisy=iy;
                             thisz=iz;
                             float thisNodeIntensity=im_cropped[thisz * sz01 + thisy * sz0 + thisx];
                             if(thisNodeIntensity<th)
                             {
                                 background_num++;
                                 im_cropped[thisz * sz01 + thisy * sz0 + thisx]=0;
                             }

                         }
                     }
                 }
                 finalth=th;
                 cout<<"in "<<finalth<<endl;
                 break;
             }
             if(th==int(imgave+7*imgstd-1)&& !issaved)
             {
                 issaved=true;
                 for(V3DLONG ix=0;ix<in_sz[0];ix++)
                 {
                     for(V3DLONG iy=0;iy<in_sz[1];iy++)
                     {
                         for(V3DLONG iz=0;iz<in_sz[2];iz++)
                         {
                             total_num++;
                             int thisx,thisy,thisz;
                             thisx=ix;
                             thisy=iy;
                             thisz=iz;
                             float thisNodeIntensity=im_cropped[thisz * sz01 + thisy * sz0 + thisx];
                             if(thisNodeIntensity<th)
                             {
                                 background_num++;
                                 im_cropped[thisz * sz01 + thisy * sz0 + thisx]=0;
                             }

                         }
                     }
                 }
                 finalth=th;
                 cout<<"in "<<finalth<<endl;
                 break;
             }
         }

         QString tmpstr = "";
         tmpstr.append("_threshold").append(QString("%1").arg(finalth));
         QString save_path_img = QString::fromStdString(inimg_file)+tmpstr+".tif";
         cout<<"save img path:"<<save_path_img.toStdString()<<endl;
         simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);
         cout<<"Done:"<<endl;

         if(im_cropped) {delete []im_cropped; im_cropped = 0;}
         if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath, QList<CellAPO> apolist, string outpath, int cropx, int cropy, int cropz)
{
    cout<<"Welcome into terafly block crop"<<endl;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    for(V3DLONG i=0;i<siz;i++)
    {
        CellAPO s = apolist[i];
        long start_x,start_y,start_z,end_x,end_y,end_z;
        start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
        end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
        start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
        end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
        start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
        end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
        cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
        V3DLONG *in_sz = new V3DLONG[4];
        in_sz[0] = cropx;
        in_sz[1] = cropy;
        in_sz[2] = cropz;
        in_sz[3]=in_zz[3];
        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = (end_x-start_x+1)*(end_y-start_y+1)*(end_z-start_z+1);
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        im_cropped = callback.getSubVolumeTeraFly(imgPath,start_x,end_x,start_y,end_y,start_z,end_z);
        if(im_cropped==NULL){
            continue;
        }
        QString tmpstr = "";
        tmpstr.append("_x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "Img"+tmpstr+".v3draw";
        QString save_path = QString::fromStdString(outpath);
        QDir path(save_path);
        if(!path.exists())
        {
            path.mkpath(save_path);
        }
        QString save_path_img =save_path+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}
void getSWCIntensityInTerafly(V3DPluginCallback2 &callback, string imgPath, QString inswc_file)
{
    cout<<"Welcome into Intensity statistics"<<endl;
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    QList<NeuronSWC>& listNeuron =  nt.listNeuron;
    //load terafly img
    V3DLONG siz = nt.listNeuron.size();
    QHash <int, int>  hashNeuron;
    hashNeuron.clear();

    float intensityList[siz];
    float meanList[siz];
    float stdintensityList[siz];
    for (V3DLONG i=0;i<siz;i++)
    {
        listNeuron[i].radius=1;
        intensityList[i]=meanList[i]=stdintensityList[i]=0;
        hashNeuron.insert(listNeuron[i].n,i);
    }
    V3DLONG *in_zz = 0;
        //read img
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    //for wirting out
    QString inswc_file_out = inswc_file + "_intensityStatistics.csv";
    ofstream fp(inswc_file_out.toStdString().c_str());
    if(!fp.is_open()){
        cout<<"Can't create file for out"<<endl;
        exit(1);
    }
    fp<<"Intensity,Mean,Std"<<endl;
    //start from here
    for(V3DLONG i=0;i<siz;i++)
    {
        //for all the node, if is axonal node and radius=1,this is a virgin node that needs to be processed.
        NeuronSWC s = listNeuron[i];
        if(s.radius==1)
        {
            //get a block
            long start_x,start_y,start_z,end_x,end_y,end_z;
            long block_size=64;
            start_x = s.x - block_size; if(start_x<0) start_x = 0;
            end_x = s.x + block_size; if(end_x > in_zz[0]) end_x = in_zz[0];
            start_y =s.y - block_size;if(start_y<0) start_y = 0;
            end_y = s.y + block_size;if(end_y > in_zz[1]) end_y = in_zz[1];
            start_z = s.z - block_size;if(start_z<0) start_z = 0;
            end_z = s.z + block_size;if(end_z > in_zz[2]) end_z = in_zz[2];

            V3DLONG *in_sz = new V3DLONG[4];
            in_sz[0] = end_x - start_x;
            in_sz[1] = end_y - start_y;
            in_sz[2] = end_z - start_z;
            in_sz[3]=in_zz[3];
            V3DLONG *sz;
            sz=in_sz;
            long sz0 = sz[0];
            long sz01 = sz[0] * sz[1];
            unsigned char * inimg1d = 0;
            inimg1d = callback.getSubVolumeTeraFly(imgPath,start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
            //            //version one
            int thisx,thisy,thisz;
            thisx=s.x-start_x;
            thisy=s.y-start_y;
            thisz=s.z-start_z;

            listNeuron[i].level=inimg1d[thisz * sz01 + thisy* sz0 + thisx];
//            cout<<"This node intensity is "<<listNeuron[i].radius<<endl;
            double imgave,imgstd;
            V3DLONG total_size=8*block_size;
            mean_and_std(inimg1d,total_size,imgave,imgstd);
//            cout<<"here"<<endl;
            intensityList[i]=listNeuron[i].level;
            meanList[i]=imgave;
            stdintensityList[i]=imgstd;
            cout<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;
            fp<<intensityList[i]<<","<<imgave<<","<<imgstd<<endl;

            if(inimg1d) {delete []inimg1d; inimg1d=0;}
        }
    }
    fp.close();
    //out swc file
    float intensityMean,imgMean,imgstdMean;
    intensityMean=0;
    imgMean=0;
    imgstdMean=0;
    for(V3DLONG i=0;i<siz;i++)
    {
        intensityMean+=intensityList[i];
        imgMean+=meanList[i];
        imgstdMean+=stdintensityList[i];
        listNeuron[i].radius=10*listNeuron[i].level/255;
    }
    intensityMean/=siz;
    imgMean/=siz;
    imgstdMean/=siz;
    QString tmpstr = "";
    tmpstr.append("_NodeIntensity_").append(QString("%1").arg(intensityMean));
    tmpstr.append("_ImgIntensityMean_").append(QString("%1").arg(imgMean));
    tmpstr.append("_ImgIntensityStd_").append(QString("%1").arg(imgstdMean));
    QString outswc_name =inswc_file+tmpstr+".eswc";
    writeESWC_file(outswc_name,nt);
    cout<<"Done:"<<endl;
}
