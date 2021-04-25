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
#include <sstream>

using namespace std;
Q_EXPORT_PLUGIN2(UnsortedPlugin, UnsortedPlugin);

QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
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
         <<tr("RadiusContour")
        <<tr("getTipBlock")
       <<tr("getTipComponent")
      <<tr("renderingSWC")
            <<tr("SomaRefinement")
           <<tr("somaBlockCrop")
          <<tr("MIP_Zslices")
         <<tr("swc_combine")
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
    else if (func_name==tr("swc_combine"))
    {
        /*this fun will combine swc files in one dir*/
        if (input.size() < 1) return false;
        /*
         *1. read swc list
         * 2.
        */
        string inswc_path;
        if(infiles.size()>=1) {inswc_path = infiles[0];}
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_path+"/combine.eswc");
        QStringList swcList = importFileList_addnumbersort(QString::fromStdString(inswc_path), 1);
        NeuronTree outswc;outswc.listNeuron.clear();outswc.hashNeuron.clear();
        QList<V3DLONG> out_id;out_id.clear();
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
            NeuronTree curSwc = readSWC_file(swcList.at(i));
            if(!curSwc.listNeuron.size()) continue;
            QHash<V3DLONG,V3DLONG> cur_id;cur_id.clear();
            for(V3DLONG d = 0; d < curSwc.listNeuron.size(); d++)
                cur_id.insert(curSwc.listNeuron[d].n,d+1);
            V3DLONG last_out_size=out_id.size();
             for(V3DLONG d = 0; d < curSwc.listNeuron.size(); d++)
             {
                 NeuronSWC s=curSwc.listNeuron[d];
                 s.n=last_out_size+d+1;
                 if(s.parent>0&&cur_id.contains(s.parent))
                     s.parent=last_out_size+cur_id.value(s.parent);
                 out_id.append(s.n);
                 outswc.listNeuron.append(s);
                 outswc.hashNeuron.insert(s.n,outswc.listNeuron.size()-1);
             }
        }
        writeESWC_file(QString::fromStdString(out_swc_file),outswc);
        return true;
    }
    else if (func_name==tr("renderingSWC"))
    {
        /*scan all the swc files in a folder
         *rendering into differenct colors
        */
        //for all the swc files
        QString swcpath = infiles[0];
        QString outpath=outfiles[0];
        QDir dir(swcpath);
        QStringList qsl_filelist,qsl_filters;
        qsl_filters+="*.swc";
        qsl_filters+="*.eswc";
        foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
        {
            qsl_filelist+=file;
        }

        if(qsl_filelist.size()==0)
        {
            v3d_msg("Cannot find the respective files in the given directory!\nTry another diretory");
        }

        //generator ano file
        QString ano_file=outfiles[1];
        QString qs_filename_out;
         if(ano_file.isEmpty())
              qs_filename_out = outpath+"/mylinker.ano"; // for domenu()
         else
              qs_filename_out =  ano_file;

        QFile qf_anofile(qs_filename_out);
        if(!qf_anofile.open(QIODevice::WriteOnly))
        {
            v3d_msg("Cannot open file for writing!");
        }

        QTextStream out(&qf_anofile);
        for(V3DLONG i=0;i<qsl_filelist.size();i++)
        {
            QString temp;
            QFileInfo curfile_info(qsl_filelist[i]);
            if (curfile_info.suffix().toUpper()=="SWC")
                temp = qsl_filelist[i].prepend("SWCFILE=");
            else if (curfile_info.suffix().toUpper()=="ESWC")
                temp = qsl_filelist[i].prepend("SWCFILE=");
            else
                v3d_msg("You should never see this, - check with the developer of this plugin.");

            out << temp << endl;
            v3d_msg(qPrintable(temp), 0);
        }

        //show message box
         if(ano_file.isEmpty())
              v3d_msg(QString("Save the linker file to: \n\n%1\n\nComplete!").arg(qs_filename_out));
         else
              v3d_msg(QString("Save the linker file to: \n\n%1\n\nComplete!").arg(qs_filename_out), 0);

         //change type: from 2-100
         int colortype=2;
         foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
         {
             QString thisswcfile=swcpath+"/"+file;
             NeuronTree nt = readSWC_file(thisswcfile);
             for(V3DLONG i=0;i<nt.listNeuron.size();i++)
             {
                 nt.listNeuron[i].type=colortype;
             }
             //write swc file
             QString outswc_file = outpath +"/"+file;
             writeESWC_file(outswc_file,nt);
             colortype++;
             colortype=(colortype>22)?2:colortype;
         }
    }
    else if (func_name==tr("getTipBlock"))
    {
        /*
         *Input: swc/eswc file
         *get all the tips and it's tip block
         *default crop size: 256*256*256
        */
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        QString inswc_file = infiles[1];
        int cropx=(inparas.size()>=1)?atoi(inparas[0]):256;
        int cropy=(inparas.size()>=2)?atoi(inparas[1]):256;
        int cropz=(inparas.size()>=3)?atoi(inparas[2]):256;
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
        getTipBlock(callback,inimg_file,inswc_file,out_path, cropx, cropy, cropz);

    }
    else if (func_name==tr("getTipComponent"))
    {
        /*According to the tip node, this function will crop a connected tree within an input block size.
         *Input: swc/eswc file
         *get all the tips and it's tip swc block
         *filter unrelated swc components
         *default crop size: 256*256*256
        */
        QString inswc_file = infiles[0];
        int cropx=(inparas.size()>=1)?atoi(inparas[0]):256;
        int cropy=(inparas.size()>=2)?atoi(inparas[1]):256;
        int cropz=(inparas.size()>=3)?atoi(inparas[2]):256;
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
        getTipComponent(inswc_file,out_path, cropx, cropy, cropz);
    }
    else if (func_name==tr("RadiusContour"))
    {
        /*
         *analysis of radius and get the contour of node's Radius
         *interval:(0-0.01-1]
         *out is a csv file: Interval,Number,Ratio(Number/Total)
         *
        */
        QString swcfile = infiles[0];
        NeuronTree nt = readSWC_file(swcfile);
        const char* outcsvFile = outfiles[0];
        ofstream csvFile;
        csvFile.open(outcsvFile,ios::out);
        csvFile<<"contourInterval,Number,Ratio"<<endl;
        //from:(0,0.01],(0.01,0.02],...,(0.99,1]
        float cInterval[100];
        long ciNum[100];
        float stepCI=0.01;
        for(int ci=0;ci<100;ci++)
        {
            cInterval[ci]=float(ci*stepCI);
            ciNum[ci]=0;
        }
        long totalNum=0;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            NeuronSWC s=nt.listNeuron[i];
            float sRadius=float(1/s.radius);
//            cout<<"R="<<sRadius<<endl;
            for(int ci=0;ci<100;ci++)
            {
                float startci=cInterval[ci];
                float endci=startci+stepCI;
                if(startci<sRadius&&sRadius<=endci)
                {
                    ciNum[ci]+=1;
                    totalNum+=1;
                    break;
                }
            }
        }
        cout<<"total num :"<<totalNum<<endl;
        //out to file
        for(int ci=0;ci<100;ci++)
        {

            float contourInterval=cInterval[ci];
            long number=ciNum[ci];
            double ratioTmp=double(100*number/totalNum);
//            cout<<"interval="<<contourInterval<<",number="<<number<<",ratio="<<ratioTmp<<endl;
            csvFile<<contourInterval<<','<<number<<','<<ratioTmp<<endl;
        }
//        cout<<"move out"<<endl;
        csvFile.close();
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
        int dis_thre=(inparas.size()>=1)?atoi(inparas[0]):64;

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
    else if (func_name==tr("somaBlockCrop"))
    {
        /*Develop by shengdian.
         *You can use this plugin get the soma feature:
         * ------volume,radius,intensity,#pixel?
         *Input:
         *      1.Img
         *      2.Apo file with soma list inside
         *output:
         *
         *      results will directly save at the same path of the input apo file.
         *this version could have the problems, because:
         *  1. bkg_thre
         *  2. soma position is not at the center point, need find a flexible radius computing way
         *  3. ration, bkg/total
        */
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        QString inapo_file = infiles[1];
        int cropx=(inparas.size()>=1)?atoi(inparas[0]):128;
        int cropy=(inparas.size()>=2)?atoi(inparas[1]):128;
        int cropz=(inparas.size()>=3)?atoi(inparas[2]):128;
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inapo_file).path());
        getSomaBlock(callback,inimg_file,inapo_file,out_path,cropx,cropy,cropz);
        cout<<"done"<<endl;
    }
    else if (func_name == tr("MIP_Zslices"))
    {
        unsigned char * data1d = 0;
        int datatype;
//        V3DLONG in_sz[4];
        V3DLONG *in_sz = new V3DLONG[4];
        if (!simple_loadimage_wrapper(callback, infiles.at(0), data1d, in_sz, datatype))
        {
            v3d_msg("Fail to load image");
            return false;
        }
        if(datatype==2)
        {
            convert_data_to_8bit((void *&)data1d,in_sz,datatype);
        }
        V3DLONG N = in_sz[0];
        V3DLONG M = in_sz[1];
        V3DLONG P = in_sz[2];
        V3DLONG mip_sz[4];
        mip_sz[0] = N;
        mip_sz[1] = M;
        mip_sz[2] = 1;
        mip_sz[3] = 1;

        cout<<"size x="<<N<<" , size y= "<<M<<endl;
        V3DLONG pagesz_mip = mip_sz[0]*mip_sz[1]*mip_sz[2];
        unsigned char *image_mip=0;
        try {image_mip = new unsigned char [pagesz_mip];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

        for(V3DLONG ix = 0; ix < N; ix++)
        {
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                int max_mip = 0;
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    if(data1d[N*M*iz + N*iy + ix] >= max_mip)
                    {
                        image_mip[iy*N + ix] = data1d[N*M*iz + N*iy + ix];
                        max_mip = data1d[N*M*iz + N*iy + ix];
                    }
                }
                if(datatype==2&&max_mip>0)
                {
                    cout<<"max mip= "<<max_mip<<endl;
                    image_mip[iy*N + ix]=200;
                }
            }
        }
        simple_saveimage_wrapper(callback, outfiles.at(0), (unsigned char *)image_mip, mip_sz, 1);
        if(data1d) {delete []data1d; data1d = 0;}
        if(image_mip) {delete []image_mip; image_mip = 0;}
    }
    else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
void getTipComponent(QString inswc_file, QString outpath, int cropx, int cropy, int cropz)
{
    QDir path(outpath);
    if(!path.exists())
        path.mkpath(outpath);
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz = nt.listNeuron.size();
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    if(siz==0)
        return;
    //get the tips
    listNeuron=nt.listNeuron;
    QHash <long, long>  hashNeuron,hashchild;
    hashNeuron.clear(); hashchild.clear();
    //hashchild: value is #child of node, key is node id.
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        hashNeuron.insert(s.n,i);
        hashchild.insert(s.n,0);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        //get parent node
        if(s.parent>0)
        {
            //this node is not soma.
            hashchild[s.parent]+=1;
        }
    }
    QHash <long, long>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        //this node is tip node
        if(hcit.value()==0)
        {
            V3DLONG tipID=hcit.key();
            V3DLONG tipIndex=hashNeuron.value(tipID);
            NeuronSWC s = listNeuron[tipIndex];

            //this tip block boundary
            long start_x,start_y,start_z,end_x,end_y,end_z;
            start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
            end_x = s.x + cropx/2; /*if(end_x > in_zz[0]) end_x = in_zz[0];*/
            start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
            end_y = s.y + cropy/2;/*if(end_y > in_zz[1]) end_y = in_zz[1];*/
            start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
            end_z = s.z + cropz/2;/*if(end_z > in_zz[2]) end_z = in_zz[2];*/
            //get the raw cropped swc
            NeuronTree nt_corped_raw=NeuronTree();
            long count = 0;
            for(long i=0; i<siz; i++)
            {
                NeuronSWC thiss = listNeuron[i];
                if(thiss.x>=start_x&&thiss.x<end_x
                        &&thiss.y>=start_y&&thiss.y<end_y
                        &&thiss.z>=start_z&&thiss.z<end_z)
                {

//                    thiss.x-=start_x;
//                    thiss.y-=start_y;
//                    thiss.z-=start_z;
                    nt_corped_raw.listNeuron.push_back(thiss);
                    nt_corped_raw.hashNeuron.insert(thiss.n,count);
                    count++;
                }
            }
            //save to file
//            if(nt_corped_raw.listNeuron.size()>0)
//            {
//                QString tmpstr = "";
//                tmpstr.append("_x_").append(QString("%1").arg(s.x));
//                tmpstr.append("_y_").append(QString("%1").arg(s.y));
//                tmpstr.append("_z_").append(QString("%1").arg(s.z));
//                QString default_name = "RawTip"+tmpstr+".swc";
//                QString save_croped_path =outpath+"/"+default_name;
//                writeESWC_file(save_croped_path,nt_corped_raw);
//            }

            //if node is related to this tip node, keep it
            NeuronTree nt_corped_final=NeuronTree();
            long count_final = 0;
            //start from this tip, iterative push node into new swc neuron tree. tipID
            QHash <int, int> cropedHashNeuron=nt_corped_raw.hashNeuron;
            int crop_tipIndex=cropedHashNeuron.value(tipID);
            NeuronSWC start_tip = nt_corped_raw.listNeuron[crop_tipIndex];
            nt_corped_final.listNeuron.push_back(start_tip);
            nt_corped_final.hashNeuron.insert(start_tip.n,count_final);
            count_final++;
            //for the start tip, get its' parent node in this block
            V3DLONG stPid=start_tip.parent;
            while(stPid>0)
            {
                if(!cropedHashNeuron.contains(stPid))
                    break;
                int stP_Index=cropedHashNeuron.value(stPid);
                NeuronSWC stP = nt_corped_raw.listNeuron[stP_Index];
                nt_corped_final.listNeuron.push_back(stP);
                nt_corped_final.hashNeuron.insert(stP.n,count_final);
                count_final++;
                stPid=stP.parent;
            }
            //for all the tips in this cropped swc, get the brother tips
            for(long i=0; i<nt_corped_raw.listNeuron.size(); i++)
            {
                NeuronSWC crops = nt_corped_raw.listNeuron[i];
                //is this tip node?
                bool brotip=false;
//                if(/*hashchild.value(crops.n)==0*/true)
                if(true)
                {
                    //is parent node in nt_corped_final
                    //parent node must in cropedHashNeuron
                    V3DLONG scPid=crops.parent;
                    while(scPid>0)
                    {
                        if(!cropedHashNeuron.contains(scPid))
                            break;
                        if(nt_corped_final.hashNeuron.contains(scPid))
                        {
                            //this is a brother tips
                            brotip=true;
                            break;
                        }
                        int stP_Index=cropedHashNeuron.value(scPid);
                        NeuronSWC scP = nt_corped_raw.listNeuron[stP_Index];

                        scPid=scP.parent;
                    }
                }
                if(brotip)
                {
                    if(!nt_corped_final.hashNeuron.contains(crops.n))
                    {
                        nt_corped_final.listNeuron.push_back(crops);
                        nt_corped_final.hashNeuron.insert(crops.n,count_final);
                        count_final++;
                    }
                    V3DLONG scPid=crops.parent;
                    while(scPid>0)
                    {
                        if(!cropedHashNeuron.contains(scPid)||nt_corped_final.hashNeuron.contains(scPid))
                            break;
                        int stP_Index=cropedHashNeuron.value(scPid);
                        NeuronSWC stP = nt_corped_raw.listNeuron[stP_Index];
                        nt_corped_final.listNeuron.push_back(stP);
                        nt_corped_final.hashNeuron.insert(stP.n,count_final);
                        count_final++;
                        scPid=stP.parent;
                    }
                }
            }
            //
            //save to file
            if(nt_corped_final.listNeuron.size()>0)
            {
                QString tmpstr = "";
                tmpstr.append("_x_").append(QString("%1").arg(s.x));
                tmpstr.append("_y_").append(QString("%1").arg(s.y));
                tmpstr.append("_z_").append(QString("%1").arg(s.z));
                QString default_name = "Tip"+tmpstr+".swc";
                QString save_croped_path =outpath+"/"+default_name;
                writeESWC_file(save_croped_path,nt_corped_final);
            }
        }
    }
}
void getTipBlock(V3DPluginCallback2 &callback, string imgPath, QString inswc_file, QString outpath, int cropx, int cropy, int cropz)
{

    QDir path(outpath);
    if(!path.exists())
        path.mkpath(outpath);
    //read swc
    NeuronTree nt = readSWC_file(inswc_file);
    V3DLONG siz = nt.listNeuron.size();
    QList<NeuronSWC> listNeuron =  nt.listNeuron;
    if(siz==0)
        return;
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
    //get the tips
    listNeuron=nt.listNeuron;
    QHash <long, long>  hashNeuron,hashchild;
    hashNeuron.clear(); hashchild.clear();
    //hashchild: value is #child of node, key is node id.
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        hashNeuron.insert(s.n,i);
        hashchild.insert(s.n,0);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = listNeuron[i];
        //get parent node
        if(s.parent>0)
        {
            //this node is not soma.
            hashchild[s.parent]+=1;
        }
    }
    cout<<"Input crop size x="<<cropx<<";y="<<cropy<<";z="<<cropz<<endl;
    long tipTotal=0;int refinedBlockSize=16;
    QHash <long, long>::const_iterator hcit;
    for(hcit=hashchild.begin();hcit!=hashchild.end();hcit++)
    {
        //this node is tip node
        if(hcit.value()==0)
        {
            tipTotal++;
            V3DLONG tipID=hcit.key();
            V3DLONG tipIndex=hashNeuron.value(tipID);
            NeuronSWC s = listNeuron[tipIndex];
            //refine tip node to local max intensity pixel
            long refined_start_x,refined_start_y,refined_start_z,refined_end_x,refined_end_y,refined_end_z;
            refined_start_x = s.x - refinedBlockSize/2; if(refined_start_x<0) refined_start_x = 0;
            refined_end_x = s.x + refinedBlockSize/2; if(refined_end_x > in_zz[0]) refined_end_x = in_zz[0];
            refined_start_y =s.y - refinedBlockSize/2;if(refined_start_y<0) refined_start_y = 0;
            refined_end_y = s.y + refinedBlockSize/2;if(refined_end_y > in_zz[1]) refined_end_y = in_zz[1];
            refined_start_z = s.z - refinedBlockSize/2;if(refined_start_z<0) refined_start_z = 0;
            refined_end_z = s.z + refinedBlockSize/2;if(refined_end_z > in_zz[2]) refined_end_z = in_zz[2];
            V3DLONG *in_sz_refined = new V3DLONG[4];
            in_sz_refined[0] = refinedBlockSize;
            in_sz_refined[1] = refinedBlockSize;
            in_sz_refined[2] = refinedBlockSize;
            in_sz_refined[3]=in_zz[3];
            unsigned char * im_refined_block = 0;
            V3DLONG pagesz_refined;
            pagesz_refined = (refined_end_x-refined_start_x+1)*(refined_end_y-refined_start_y+1)*(refined_end_z-refined_start_z+1);
            try {im_refined_block = new unsigned char [pagesz_refined];}
            catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

            im_refined_block = callback.getSubVolumeTeraFly(imgPath,refined_start_x,refined_end_x,refined_start_y,refined_end_y,refined_start_z,refined_end_z);
            if(im_refined_block==NULL){
                continue;
            }
            V3DLONG thisx,thisy,thisz;
            thisx=s.x-refined_start_x;
            thisy=s.y-refined_start_y;
            thisz=s.z-refined_start_z;
            NeuronSWC s_refined=nodeRefine(im_refined_block,thisx,thisy,thisz,in_sz_refined,5);
            QString tmpstr_raw = "";
            tmpstr_raw.append("_rawx_").append(QString("%1").arg(s.x));
            tmpstr_raw.append("_rawy_").append(QString("%1").arg(s.y));
            tmpstr_raw.append("_rawz_").append(QString("%1").arg(s.z));

            s.x=refined_start_x+s_refined.x;
            s.y=refined_start_y+s_refined.y;
            s.z=refined_start_z+s_refined.z;
            if(im_refined_block) {delete []im_refined_block; im_refined_block = 0;}
            //get img block
            //read img
            long start_x,start_y,start_z,end_x,end_y,end_z;
            start_x = s.x - cropx/2; if(start_x<0) start_x = 0;
            end_x = s.x + cropx/2; if(end_x > in_zz[0]) end_x = in_zz[0];
            start_y =s.y - cropy/2;if(start_y<0) start_y = 0;
            end_y = s.y + cropy/2;if(end_y > in_zz[1]) end_y = in_zz[1];
            start_z = s.z - cropz/2;if(start_z<0) start_z = 0;
            end_z = s.z + cropz/2;if(end_z > in_zz[2]) end_z = in_zz[2];
//            cout<<"crop size x="<<start_x<<";y="<<start_y<<";z="<<start_z<<endl;
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
            QString default_name = "Img"+tmpstr+tmpstr_raw+".v3draw";
            QString save_path_img =outpath+"/"+default_name;
            simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
            if(im_cropped) {delete []im_cropped; im_cropped = 0;}
        }
    }
    cout<<"# Tip Block: "<<tipTotal<<endl;
    cout<<"Save to "<<outpath.toStdString()<<endl;
}
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size)
{
    //return the intensity of the input node
//    int outRadius=0;
    cout<<"---Node refine to the local maximal intensity----"<<endl;
    NeuronSWC out;
    V3DLONG maxIntensity=0;
    V3DLONG thisx,thisy,thisz;
    out.x=thisx=nodex;
    out.y=thisy=nodey;
    out.z=thisz=nodez;

    V3DLONG sz01 = sz[0] * sz[1];
//    cout<<"----------node new------------"<<endl;
    for(V3DLONG iz=((thisz-neighbor_size)>=0)?(thisz-neighbor_size):0;(iz<thisz+neighbor_size+1)&&(iz<sz[2]);iz++)
    {
        for( V3DLONG iy=((thisy-neighbor_size)>=0)?(thisy-neighbor_size):0;(iy<thisy+neighbor_size+1)&&(iy<sz[1]);iy++)
        {
            for(V3DLONG ix=((thisx-neighbor_size)>=0)?(thisx-neighbor_size):0;(ix<thisx+neighbor_size+1)&&(ix<sz[0]);ix++)
            {

                V3DLONG pos = iz*sz01 + iy * sz[0] + ix;
                V3DLONG thisIntensity=inimg1d[pos];
                if(thisIntensity>maxIntensity)
                {
//                    cout<<"update to intensity:"<<thisIntensity<<endl;
                    maxIntensity=thisIntensity;
                    out.x=ix;
                    out.y=iy;
                    out.z=iz;
                }
            }
        }
    }
    return out;
}
void getSomaBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo_file, QString outpath, int cropx, int cropy, int cropz)
{
    QList <CellAPO> apolist=readAPO_file(inapo_file);
    if(apolist.size()==0)
        return;
    V3DLONG siz = apolist.size();
    V3DLONG *in_zz = 0;
    if(!callback.getDimTeraFly(imgPath,in_zz))
    {
        cout<<"can't load terafly img"<<endl;
        return;
    }
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
        //below is croped img saving and marker saving
        QString tmpstr =s.name;
        tmpstr.append("-x_").append(QString("%1").arg(s.x));
        tmpstr.append("_y_").append(QString("%1").arg(s.y));
        tmpstr.append("_z_").append(QString("%1").arg(s.z));
        QString default_name = "ImgSoma_"+tmpstr+".v3draw";
        QString save_path_img =outpath+"/"+default_name;
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),im_cropped,in_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
}
void getMarkerRadius(unsigned char *&inimg1d, long in_sz[], NeuronSWC& s)
{
    long sz01 = in_sz[0] * in_sz[1];
    long sz0 = in_sz[0];
    double max_r = in_sz[0]/2;
    if (max_r > in_sz[1]/2) max_r = in_sz[1]/2;
    if (max_r > (in_sz[2])/2) max_r = (in_sz[2])/2;

    //get the background threshold
    double imgave,imgstd;
    V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
    mean_and_std(inimg1d,total_size,imgave,imgstd);
//    double td= (imgstd<10) ? 10:imgstd;
    double bkg_thresh= (imgave+0.5*imgstd)>20?20:(imgave+0.5*imgstd);
    cout<<"Img mean= "<<imgave<<" ,std="<<imgstd<<" ,thresh="<<bkg_thresh<<endl;
    int thisx,thisy,thisz;
    thisx=s.x;
    thisy=s.y;
    thisz=s.z;

    double total_num, background_num,total_intensity;
    double ir;
    total_intensity=0;
    for (ir=1.0; ir<=max_r; ir++)
    {
        total_num = background_num = 0;
        double dz, dy, dx;
        double zlower = -ir, zupper = +ir;
        for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; ++dy)
                for (dx= -ir; dx <= +ir; ++dx)
                {
                    total_num++;
                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                    if (r>ir-1 && r<=ir)
                    {
                        V3DLONG i = thisx+dx;   if (i<0 || i>=in_sz[0]) goto end2;
                        V3DLONG j = thisy+dy;   if (j<0 || j>=in_sz[1]) goto end2;
                        V3DLONG k = thisz+dz;   if (k<0 || k>=in_sz[2]) goto end2;

                        if (inimg1d[k * sz01 + j * sz0 + i] <= bkg_thresh)
                        {
                            background_num++;
                            inimg1d[k * sz01 + j * sz0 + i]=0;
                            if ((background_num/total_num) > 0.01) goto end2;
                        }
                        else
                        {
                            total_intensity+=inimg1d[k * sz01 + j * sz0 + i];
                        }
                    }
                }
    }
end2:
    cout<<"radius="<<ir<<", totalnum="<<total_num<<endl;
    s.radius= float(ir);
    s.timestamp=total_num;
    s.tfresindex=total_intensity/total_num;
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
        pagesz = (end_x-start_x)*(end_y-start_y)*(end_z-start_z);
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
