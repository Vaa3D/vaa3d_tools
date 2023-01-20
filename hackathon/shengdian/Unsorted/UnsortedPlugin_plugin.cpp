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
       <<tr("retype")
       <<tr("getTipComponent")
      <<tr("renderingSWC")
     <<tr("SomaRefinement")
    <<tr("somaBlockCrop")
    <<tr("Seg_Connection_Turn_over")
    <<tr("MIP_Zslices")
    <<tr("multi_stems")
    <<tr("get_soma_apo")
    <<tr("swc_combine")
    <<tr("hist_equal")
    <<tr("mask_img_from_swc")
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
        int sample=(inparas.size()>=4)?atoi(inparas[3]):1;
        string out_path=outfiles[0];
        QList <CellAPO> apolist=readAPO_file(QString::fromStdString(inapo_file));
        if(apolist.size()>0)
        {
            getTeraflyBlock(callback,inimg_file,apolist,out_path,cropx,cropy,cropz,sample);
        }
        else
            cout<<"apo size is zero"<<endl;
        cout<<"done"<<endl;
    }
    if (func_name==tr("get_swc_intensity"))
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
        string inswc_file = infiles[1];
        NeuronTree nt_raw = readSWC_file(QString::fromStdString(inswc_file));
        if(!nt_raw.listNeuron.size()) return false;

        //save to file: intensity_radius_profiled_file, bouton_apo_file, bouton_eswc_file
        string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_profiled.eswc");

        int half_crop_size=128;
        swc_profile_terafly_fun(callback,inimg_file,nt_raw,half_crop_size);
        writeESWC_file(QString::fromStdString(out_swc_file),nt_raw);
    }
    else if (func_name == tr("preprocess"))
    {
        /*preprocess:
         * i)to a swc/eswc file;
         * ii)to a path with neuron reconstructions
        step 1: processing of multiple -1 nodes
        For each node, if its parent index is
            1) -1; check if parent node is a duplicated node.
                        if yes, delete parent node and set to duplicated node (should not -1 node)
            2) >=0; check existence.
                        if not, record it
             out: #origin-nodes(p=-1), #after-processing-nodes(p=-1), #nodes(parent-index not existed)
        step 2: conneted tree:
        > if #after-processing-nodes(p=-1) =1 & #nodes(parent-index not existed) =1, this should be only one connected tree; sort
                *what: each node is connected, which means i can index to every node from one node.
                *should be only one connected tree, if more than one, get the permission to
                        * a) keep the biggest one; or
                        * b) highlight the small one (color=write);
         * 2. soma/root node: should be only one root
         *
        */
        QString inswc= infiles[0];
        NeuronTree nt=readSWC_file(inswc);
        V3DLONG niz=nt.listNeuron.size();
        int somaid=0;
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.type==1&&s.pn<0)
            {somaid=i;break;}
        }
        int stems=0;
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.pn>0&&somaid==nt.hashNeuron.value(s.pn))
            {
                stems++;
            }
        }
        QString outswc=(outfiles.size()>=1)?outfiles[0]:inswc+"_multi_stem.swc";
        if(stems>1)
            writeSWC_file(outswc,nt);
    }
    else if (func_name==tr("swc_combine"))
    {
        /*this fun will combine swc files in one dir*/
        if (input.size() < 1) return false;
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
    else if (func_name==tr("Seg_Connection_Turn_over"))
    {
        /*this fun will combine swc files in one dir*/
        if (input.size() < 1) return false;
        QString inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        QString out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file+"_result.eswc");
        long index_b=(inparas.size()>=1)?atoi(inparas[0]):1;
        long index_old_root=(inparas.size()>=2)?atoi(inparas[1]):1;
        long index_bp=(inparas.size()>=3)?atoi(inparas[2]):index_b;
        NeuronTree outswc;

        writeESWC_file(out_swc_file,outswc);
        return true;
    }
    else if (func_name==tr("mask_img_from_swc"))
    {
        /*20200903:there is a v3dplugin: swc_to_mask is just like this
         *according to swc, mask img block
         *Input img block
         *Input swc file
         *Input para for mask size of x, y and z
         *output path is the save path for processed img block.
        */

        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        QString inswc_file = infiles[1];
        int maskRadius=(inparas.size()>=1)?atoi(inparas[0]):12;
        int erosion_kernel_size=(inparas.size()>=2)?atoi(inparas[1]):0;
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
        //read img
        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
        //read swc
        NeuronTree nt = readSWC_file(inswc_file);
        QDir path(out_path);
        if(!path.exists())
        {
            path.mkpath(out_path);
        }
        QString save_path_img =out_path+"/"+QFileInfo(inswc_file).baseName()+"_mR_"+QString::number(maskRadius)
                +"_eR_"+QString::number(erosion_kernel_size)+".v3draw";
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        maskImg(callback,inimg1d,save_path_img,in_sz,nt,maskRadius,erosion_kernel_size);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
    else if (func_name==tr("hist_equal"))
    {
        if(input.size() < 1 || output.size() != 1)
            return false;

        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
        unsigned char lowerbound=(inparas.size()>=1)?atoi(inparas[0]):30;
        unsigned char higherbound=(inparas.size()>=2)?atoi(inparas[1]):255;
        //read img

        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,inimg_file, inimg1d, in_sz, datatype)) return false;
        V3DLONG pagesz = in_sz[0]*in_sz[1]*in_sz[2];
        hist_eq_range_uint8(inimg1d,pagesz,lowerbound,higherbound);
        simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)inimg1d, in_sz, 1);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
    else if (func_name==tr("retype"))
    {
        /*scan all the swc files in a folder
         *rendering into differenct colors
        */
        //for all the swc files
        QString swcpath = infiles[0];
        QString outpath=outfiles[0];
        int toType=(inparas.size()>=1)?atoi(inparas[0]):3;
        NeuronTree nt = readSWC_file(swcpath);
        V3DLONG siz=nt.listNeuron.size();
        for(V3DLONG i=0;i<siz;i++){
            nt.listNeuron[i].type=toType;
            if(nt.listNeuron.at(i).pn<0)
                nt.listNeuron[i].type=1;
        }
        writeSWC_file(outpath,nt);
    }
    else if (func_name==tr("record"))
    {
        /*scan all the swc files in a folder
         *rendering into differenct colors
        */
        //for all the swc files
        QString swcpath = infiles[0];
        QString outpath=outfiles[0];
        float xc=(inparas.size()>=1)?atof(inparas[0]):0.0;
        float yc=(inparas.size()>=2)?atof(inparas[1]):0.0;
        float zc=(inparas.size()>=3)?atof(inparas[2]):0.0;

        NeuronTree nt = readSWC_file(swcpath);
        V3DLONG siz=nt.listNeuron.size();
        V3DLONG somaid=-1;
         for(V3DLONG i=0;i<siz;i++)
             if(nt.listNeuron.at(i).type==1&&nt.listNeuron.at(i).pn<0)
                 somaid=i;
         if(somaid<0)
             return false;
         float xsize=nt.listNeuron.at(somaid).x;
         float ysize=nt.listNeuron.at(somaid).y;
         float zsize=nt.listNeuron.at(somaid).z;
        for(V3DLONG i=0;i<siz;i++){
            nt.listNeuron[i].x+=(xc-xsize);
            nt.listNeuron[i].y+=(yc-ysize);
            nt.listNeuron[i].z+=(zc-zsize);
        }
        writeSWC_file(outpath,nt);
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
    else if (func_name == tr("multi_stems"))
    {
        QString inswc= infiles[0];
        NeuronTree nt=readSWC_file(inswc);
        V3DLONG niz=nt.listNeuron.size();
        int somaid=0;
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.type==1&&s.pn<0)
            {somaid=i;break;}
        }
        int stems=0;
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.pn>0&&somaid==nt.hashNeuron.value(s.pn))
            {
                stems++;
            }
        }
        QString outswc=(outfiles.size()>=1)?outfiles[0]:inswc+"_multi_stem.swc";
        if(stems>1)
            writeSWC_file(outswc,nt);
    }
    else if (func_name == tr("get_soma_apo"))
    {
        QString inswc= infiles[0];
        NeuronTree nt=readSWC_file(inswc);
        V3DLONG niz=nt.listNeuron.size();
        int somaid=-1;
        for(V3DLONG i=0;i<niz;i++){
            NeuronSWC s=nt.listNeuron.at(i);
            if(s.type==1&&s.pn<0){
                if(somaid>0)
                {
                    cout<<"---------------Error: multiple soma nodes!!!-----------------------"<<endl;
                    return false;
                }else
                    somaid=i;
            }

        }
        NeuronSWC soma_node=nt.listNeuron.at(somaid);
        CellAPO soma;
        soma.x=soma_node.x;
        soma.y=soma_node.y;
        soma.z=soma_node.z;
        soma.intensity=soma.volsize=soma.sdev=soma.pixmax=soma.mass=10;
        QList<CellAPO> somaapo; somaapo.append(soma);
        QString outapo=(outfiles.size()>=1)?outfiles[0]:inswc+"_soma.apo";
        if(somaapo.size())
            writeAPO_file(outapo,somaapo);
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
