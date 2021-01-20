/* BoutonDectection_plugin.cpp
 * designed by shengdian
 * 2020-7-29 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "BoutonDectection_plugin.h"
#include <QHash>
#include "volimg_proc.h"
#include <fstream>
#include "boutonDetection_fun.h"

using namespace std;
Q_EXPORT_PLUGIN2(BoutonDectection, BoutonDectectionPlugin);
 
QStringList BoutonDectectionPlugin::menulist() const
{
	return QStringList() 
        <<tr("BoutonDection_Img")
		<<tr("about");
}

QStringList BoutonDectectionPlugin::funclist() const
{
	return QStringList()
        <<tr("BoutonDection_terafly")
        <<tr("BoutonDection_Img")
        <<tr("BoutonDection_filter")
          <<tr("BoutonDection_filter_toSWC")
       <<tr("ReconstructionComplexity")
        <<tr("RecontructionIntensity_terafly")
       <<tr("Crop_terafly_block")
         <<tr("mask_img_from_swc")
		<<tr("help");
}

void BoutonDectectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("BoutonDection_Img"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        V3DLONG  in_sz[4];
        unsigned char* inimg1d = p4DImage->getRawData();

        in_sz[0] = p4DImage->getXDim();
        in_sz[1] = p4DImage->getYDim();
        in_sz[2] = p4DImage->getZDim();
        in_sz[3] = p4DImage->getCDim();

        cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
        long sz01 = in_sz[0] * in_sz[1];
        long sz0 = in_sz[0];
        double imgave,imgstd;
        V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
        mean_and_std(inimg1d,total_size,imgave,imgstd);
        NeuronTree nt =callback.getSWC(curwin);
        QList<NeuronSWC>& listNeuron =  nt.listNeuron;

        V3DLONG siz = nt.listNeuron.size();
        cout<<"SWC size:"<<siz<<endl;
//        int max_intensity=1;
        QHash <int, int>  hashNeuron;
        hashNeuron.clear();
        for (V3DLONG i=0;i<siz;i++)
        {
            listNeuron[i].level=1;
            hashNeuron.insert(listNeuron[i].n,i);
        }

        QDialog * dialog = new QDialog();
        QLineEdit * savePath_box = new QLineEdit("");
        QLineEdit * thresh_box = new QLineEdit("18");
        QCheckBox * useNeighborArea_checkbox = new QCheckBox("Use Neighbor Area");
        useNeighborArea_checkbox->setChecked(true);
        {
            QGridLayout * layout = new QGridLayout;
            layout->addWidget(new QLabel("Save path"), 0, 0, 1, 1);
            layout->addWidget(savePath_box, 0, 1, 1, 5);
            layout->addWidget(new QLabel("Threshold"), 1, 0, 1, 1);
            layout->addWidget(thresh_box, 1, 1, 1, 5);
            layout->addWidget(useNeighborArea_checkbox, 2, 0, 1, 6);
            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 5, 0, 1, 3);
            layout->addWidget(cancel, 5, 3, 1, 3);
            dialog->setLayout(layout);
            connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        }
        if(dialog->exec() != QDialog::Accepted) return;
        string save_path = savePath_box->text().toStdString();
        bool useNeighborArea = useNeighborArea_checkbox->isChecked();
        int threshold = atof(thresh_box->text().toStdString().c_str());
        QString savepath=QString::fromStdString(save_path);
        QDir path(savepath);
        if(!path.exists())
        {
            path.mkpath(savepath);
        }

        for(V3DLONG i=0;i<siz;i++)
        {
            NeuronSWC s = listNeuron[i];
            if(listNeuron[i].level==1)
            {

                int thisx,thisy,thisz;
                thisx=s.x;
                thisy=s.y;
                thisz=s.z;

                if(useNeighborArea)
                {
                    listNeuron[i].level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    NeuronSWC out=nodeRefine(inimg1d,listNeuron[i].x,listNeuron[i].y,listNeuron[i].z,in_sz);
                    if(listNeuron[i].level+imgave+imgstd<out.level)
                    {
                        cout<<"update intensity:old="<<listNeuron[i].level<<";New="<<out.level<<endl;
                        cout<<"Distance changes:x="<<(listNeuron[i].x-out.x)<<";y="<<(listNeuron[i].y-out.y)<<";z="<<(listNeuron[i].z-out.z)<<endl;
                        listNeuron[i].level=out.level;
                        listNeuron[i].x=out.x;
                        listNeuron[i].y=out.y;
                        listNeuron[i].z=out.z;
                    }
                }
                else
                {
                    listNeuron[i].level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    float childIntensity=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                    //get their parent node intensity

                    if(s.parent>0)
                    {
                        long pid=hashNeuron.value(s.parent);
                        NeuronSWC sp=listNeuron[pid];
                        //consider the distance of child-parent node
                        float distanceThre=5;
                        float child_parent_distance=(sp.x-s.x)*(sp.x-s.x)+(sp.y-s.y)*(sp.y-s.y)+(sp.z-s.z)*(sp.z-s.z);
                        if(child_parent_distance<distanceThre*distanceThre)
                        {

                            NeuronSWC sm;
                            sm.x=(sp.x-s.x)/2+s.x;
                            sm.y=(sp.y-s.y)/2+s.y;
                            sm.z=(sp.z-s.z)/2+s.z;
                            float parentIntensity=inimg1d[int(sp.z) * sz01 + int(sp.y) * sz0 + int(sp.x)];
                            float middleIntensity=inimg1d[int(sm.z) * sz01 + int(sm.y) * sz0 + int(sm.x)];
                            //this may need another threshold
                            int updateThre=10;
                            if((middleIntensity-childIntensity>updateThre)&&(middleIntensity-parentIntensity>updateThre))
                            {
                                cout<<"update child intensity:old = "<<childIntensity<<";New="<<middleIntensity<<". this node id:"<<s.n<<endl;

                                //                    update child node Intensity
                                listNeuron[i].level=middleIntensity;
                            }
                        }
                    }

                }
            }
        }
        getNodeRadius(inimg1d,in_sz,nt);
        QString outswc_file =savepath+"/"+"IntensityResult_original.eswc";
        writeESWC_file(outswc_file,nt);
        QList <CellAPO> apolist=getBouton(nt,threshold,1);
        QString apo_file_path = savepath +"/"+ "bouton.apo";
        writeAPO_file(apo_file_path,apolist);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
        callback.setSWC(curwin,nt);
	}
	else
	{
        v3d_msg(tr("Email: shengdianjiang@seu.edu.cn"
			"Developed by SD-Jiang, 2020-7-29"));
	}
}

bool BoutonDectectionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    if (func_name == tr("BoutonDection_terafly"))
	{
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        QString inswc_file = infiles[1];
        QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(inswc_file).path());
        int useNeighborArea=(inparas.size()>=1)?atoi(inparas[0]):2;
        int allnode=(inparas.size()>=2)?atoi(inparas[1]):0;

        NeuronTree nt = readSWC_file(inswc_file);
//        cout<<"out path "<<out_path.toStdString()<<endl;
        getBoutonInTerafly(callback,inimg_file,nt,allnode,useNeighborArea);
        QString outswc_file =out_path+"/"+QFileInfo(inswc_file).baseName()+"_IntensityResult.eswc";
        if(useNeighborArea)
            outswc_file = out_path+"/"+QFileInfo(inswc_file).baseName() +"_"+QString::number(useNeighborArea)+ "_pixels_IntensityResult.eswc";
        cout<<"save to "<<outswc_file.toStdString()<<endl;
        writeESWC_file(outswc_file,nt);
	}
    else if(func_name ==tr("BoutonDection_filter_toSWC"))
    {
        //this version will output the detected boutons to swc file
        string inswc_file;
        if(infiles.size()==1)
        {
            inswc_file = infiles[0];
        }
        else
        {
            printHelp();
            return false;
        }

        float dis_thre=(inparas.size()>=1)?atoi(inparas[0]):2.0;
        int threshold=(inparas.size()>=2)?atoi(inparas[1]):40;
        int allnode=(inparas.size()>=3)?atoi(inparas[2]):1;
        int renderingType=(inparas.size()>=4)?atoi(inparas[3]):0;

        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        V3DLONG siz = nt.listNeuron.size();
        if(renderingType==1)
        {
            for (V3DLONG i=0;i<siz;i++)
            {
                NeuronSWC s = nt.listNeuron[i];
                nt.listNeuron[i].radius=s.type;
            }
        }
        if(nt.listNeuron.size()==0)
            return false;
        NeuronTree nt_out=getBouton_toSWC(nt,threshold,allnode,dis_thre);
        if(nt_out.listNeuron.size()==0)
            qDebug()<<"Can't find bouton at input swc file";
        else
        {
            QString outswc_file = QString::fromStdString(inswc_file) +"_bouton.eswc";
            writeESWC_file(outswc_file,nt_out);
        }
    }
    else if (func_name == tr("BoutonDection_filter"))
    {
        //this version will output the detected boutons to apo file
        string inswc_file,inimg_file;
        if(infiles.size()==1)
        {
            inswc_file = infiles[0];
        }
        else if(infiles.size()==2)
        {
            inimg_file = infiles[0];
            inswc_file = infiles[1];
        }
        else
        {
            printHelp();
            return false;
        }
        int threshold=(inparas.size()>=1)?atoi(inparas[0]):40;
        int allnode=(inparas.size()>=2)?atoi(inparas[1]):1;
        int renderingType=(inparas.size()>=3)?atoi(inparas[2]):0;
        int crop_block_size=(inparas.size()>=4)?atoi(inparas[3]):16;
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        V3DLONG siz = nt.listNeuron.size();
        if(renderingType==1)
        {
            for (V3DLONG i=0;i<siz;i++)
            {
                NeuronSWC s = nt.listNeuron[i];
                nt.listNeuron[i].radius=s.type;
            }
        }
        QList <CellAPO> apolist=getBouton(nt,threshold,allnode);
        //remove duplicated bouton at branch point
        apolist=removeBoutons(apolist);
        string apo_file_path = inswc_file + "_bouton.apo";
        writeAPO_file(QString::fromStdString(apo_file_path),apolist);
        if(infiles.size()==2&&outfiles.size()==1)
        {
            string out_path=outfiles[0];
            getBoutonBlock(callback,inimg_file,apolist,out_path,crop_block_size);
        }
        else if(outfiles.size()>1)
            printHelp();return false;
    }
    else if (func_name==tr("mask_img_from_swc"))
    {
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        /*20200903:there is a v3dplugin: swc_to_mask is just like this
         *according to swc, mask img block
         *Input img block
         *Input swc file
         *Input para for mask size of x, y and z
         *output path is the save path for processed img block.
        */
        string inimg_file = infiles[0];
        QString inswc_file = infiles[1];
        int maskRadius=(inparas.size()>=1)?atoi(inparas[0]):12;
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
        QString save_path_img =out_path+"/"+QFileInfo(inswc_file).baseName()+"_maskRadius_"+QString::number(maskRadius)+".v3draw";
        cout<<"save img path:"<<save_path_img.toStdString()<<endl;
        maskImg(callback,inimg1d,save_path_img,in_sz,nt,maskRadius);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
    }
    else if (func_name==tr("Crop_terafly_block"))
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
         *output path is the save path for img block*/
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
    else if (func_name == tr("BoutonDection_Img"))
    {
        /*Input1: img block path; Input2: SWC;*/
        if(infiles.size() != 2)
        {
            cerr<<"Invalid input"<<endl;
            cout<<"Input file size="<<infiles.size()<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];

        int threshold=(inparas.size()>=1)?atoi(inparas[0]):40;
        int useNeighborArea=(inparas.size()>=2)?atoi(inparas[1]):0;
        int allnode=(inparas.size()>=3)?atoi(inparas[2]):1;
        //read img
        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
        //read swc
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        getBoutonInImg(callback,inimg1d,in_sz,nt,useNeighborArea);
        nt=removeDupNodes(nt);
        getNodeRadius(inimg1d,in_sz,nt);
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
        string outswc_file = inswc_file + "_IntensityResult.eswc";
        writeESWC_file(QString::fromStdString(outswc_file),nt);
        QList <CellAPO> apolist=getBouton(nt,threshold,allnode);
        //remove duplicated bouton at branch point
        apolist=removeBoutons(apolist);
        string apo_file_path = inswc_file + "_bouton.apo";
        writeAPO_file(QString::fromStdString(apo_file_path),apolist);
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
    else if (func_name == tr("help"))
    {
        /* Version one,20200801
        1.Basic idea: get the intensity value of the reconstruction nodes and consider big intensity changes among child-parent nodes as bouton.
        2.If threshold of the intensity changes is low, there will be more bouton results.
        3.Input swc need to be refined, if the reconstructed nodes don't place at the center of the signal, the performance will be extremly ugly.
        (Maybe consider to add refinement in future version)
        Future:
        1. For testing purpose:
               1)crop img blocks and swc_in_block,in this way, the performance is easy to compute and visualize;
               2)(Vertical)move reconstruction nodes to signal, which is so-called refinement;
               3)(Horizontal)move reconstruction nodes to a local maxmimal intensity point, need to define the surrounding region, and may also need a purning afterwards.
               4) The threshold parameter is a place to improve the overall performance. (maybe the avearge of the intensity of listneuron)
        */
        /*
         *Version two:20200828
         *1.split swc into block, mask the signal,refine the swc into center line: refinement
         *2.get the candidate bouton
         *3.use the way smartTracing did to decompose
         *4. use wavelet representation
         *5.use mRMR get features
         *6.Use SVM or CNN classifier the subvolume into bouton and non-bouton
        */
        printHelp();
	}
	else return false;

	return true;
}
void printHelp()
{
    qDebug()<<"Function for terafly";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_terafly -i <input_image_terafly> <input_swc>";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_terafly -i <input_image_terafly> <input_swc> -p <all nodes in swc are used: 0 means only type 2; greater than 0 means use all the nodes and will use this para to refine swc node to its local maximal intensity>";
    qDebug()<<"Usage_basic: get bouton apo file and crop img out";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_filter -i <input_image_terafly> <input_swc> -o <out_path> -p <threshold> <allnode> <crop_size>";
    qDebug()<<"Usage_simple: only get bouton apo file.";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_filter -i <input_swc>";
    qDebug()<<"Function for Img block";
    qDebug()<<"Usage: get bouton in using inputImg and inputSWC";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonDection_Img -i <input_image> <input_swc> -p <threshold> <useNeighborArea: 0 or 1>";
}
