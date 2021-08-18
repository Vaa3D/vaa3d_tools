/* BoutonDetection_plugin.cpp
 * designed by shengdian
 * 2020-7-29 : by SD-Jiang
 */
 
#include "v3d_message.h"
#include <vector>
#include "BoutonDetection_plugin.h"
#include "boutonDetection_fun.h"

using namespace std;
Q_EXPORT_PLUGIN2(BoutonDetection, BoutonDetectionPlugin);
 
QStringList BoutonDetectionPlugin::menulist() const
{
	return QStringList() 
        <<tr("BoutonDection_Img")
		<<tr("about");
}

QStringList BoutonDetectionPlugin::funclist() const
{
    return QStringList()
            <<tr("BoutonDetection_terafly")
           <<tr("BoutonDetection_image")
          <<tr("RefinementAll_terafly")
         <<tr("RefinementAll_image")
          << tr("Refinement_terafly")
          <<tr("Refinement_image")
         << tr("NodeRefinement_terafly")
         <<tr("NodeRefinement_image")
        <<tr("SWC_profile_terafly")
       <<tr("SWC_profile")
       <<tr("Bouton_filter")
      << tr("TeraImage_SWC_Crop")
      <<tr("BoutonSWC_pruning")
     <<tr("CCF_profile")
    <<tr("Bouton_feature")
    <<tr("UpsampleImage")
    <<tr("SWC_Analysis")
    <<tr("help");
}

void BoutonDetectionPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
                    NeuronSWC out=nodeRefine(inimg1d,listNeuron.at(i),in_sz);
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
//        getNodeRadius(inimg1d,in_sz,nt);
        QString outswc_file =savepath+"/"+"IntensityResult_original.eswc";
        writeESWC_file(outswc_file,nt);
        QList <CellAPO> apolist=getBouton_1D_filter(nt);
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
bool BoutonDetectionPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    if (func_name == tr("BoutonDetection_terafly"))
    {
        boutonDetection_dofunc(callback,input,output,true);
    }
    else if (func_name == tr("BoutonDetection_image"))
    {
        boutonDetection_dofunc(callback,input,output,false);
    }
    else if(func_name == tr("Bouton_filter"))
    {
        boutonFilter_dofunc(callback,input,output);
    }
    else if(func_name == tr("RefinementAll_terafly"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,2,true);
    }
    else if(func_name == tr("RefinementAll_image"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,2,false);
    }
    else if(func_name == tr("Refinement_terafly"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,0,true);
    }
    else if(func_name == tr("Refinement_image"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,0,false);
    }
    else if(func_name == tr("NodeRefinement_terafly"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,1,true);
    }
    else if(func_name == tr("NodeRefinement_image"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,1,false);
    }
    else if(func_name == tr("SWC_profile_terafly"))
    {
        swc_profile_dofunc(callback,input,output,true);
    }
    else if(func_name == tr("SWC_profile"))
    {
        swc_profile_dofunc(callback,input,output,false);
    }
    else if (func_name == tr("BoutonSWC_pruning"))
    {
        boutonswc_pruning_dofunc(callback,input,output);
    }
    else if (func_name == tr("CCF_profile"))
    {
        ccf_profile_dofunc(callback,input,output);
    }
    else if (func_name == tr("Bouton_feature"))
    {
        bouton_feature_dofunc(callback,input,output);
    }
    else if (func_name == tr("SWC_Analysis"))
    {
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nt.listNeuron.size()) return false;
         V_NeuronSWC_list nt_nslist=NeuronTree__2__V_NeuronSWC_list(nt);
         cout<<"seg list size: "<<nt_nslist.seg.size()<<endl;
          string filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_statistics.csv");
         QFile tofile(QString::fromStdString(filename));
         if(tofile.exists())
             cout<<"File overwrite to "<<filename<<endl;
         QString confTitle="nodes,mean,std\n";
         if(tofile.open(QIODevice::WriteOnly | QIODevice::Text))
         {
              tofile.write(confTitle.toAscii());
         }
         for(int i=0;i<nt_nslist.seg.size();i++){
             V_NeuronSWC curseg=nt_nslist.seg.at(i);
             vector<double> seg_levels; seg_levels.clear();
             seg_levels=get_sorted_fea_of_seg(curseg);
             long seg_nodes=seg_levels.size();
             int seg_level_mean=0;
             for(int j=0;j<seg_nodes;j++){
                 seg_level_mean+=seg_levels[j];
             }
             seg_level_mean/=seg_nodes;
             int seg_level_std=0;
             for(int j=0;j<seg_nodes;j++){
                 seg_level_std+=(seg_levels[j]-seg_level_mean)*(seg_levels[j]-seg_level_mean);
             }
             seg_level_std/=seg_nodes;             seg_level_std=sqrt(seg_level_std);
             QString outline=(QString::number(seg_nodes)+","
                              +QString::number(seg_level_mean)+","
                              +QString::number(seg_level_std)+"\n");
             tofile.write(outline.toAscii());
         }
         tofile.close();
    }
    else if (func_name == tr("UpsampleImage"))
    {

        string inimg_file;
        if(infiles.size()>=1) {
            inimg_file = infiles[0];
            double x_rez=(inparas.size()>=1)?atoi(inparas[0]):4;
            double y_rez=(inparas.size()>=2)?atoi(inparas[1]):4;
            double z_rez=(inparas.size()>=3)?atoi(inparas[2]):1;

            //get out_image_path and out_swc_path
            string out_image_file=(outfiles.size()>=1)?outfiles[0]:(inimg_file+"_up.v3draw");

            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;
            if(!simple_loadimage_wrapper(callback, (char*)inimg_file.c_str(), data1d, in_sz, datatype))
            {
                cerr<<"load image "<<inimg_file<<" error!"<<endl;
                return false;
            }

            V3DLONG N = in_sz[0];
            V3DLONG M = in_sz[1];
            V3DLONG P = in_sz[2];
            V3DLONG pagesz = N*M*P;
            in_sz[3] = 1;

            double dfactor[3];
            dfactor[0] = x_rez; dfactor[1] = y_rez;  dfactor[2] = z_rez;

            V3DLONG pagesz_resample = (V3DLONG)(ceil(x_rez*y_rez*z_rez*pagesz));

            unsigned char * image_resampled = 0;
            try {image_resampled = new unsigned char [pagesz_resample];}
            catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}

            V3DLONG out_sz[4];
            out_sz[0] = (V3DLONG)(ceil(dfactor[0]*N));
            out_sz[1] = (V3DLONG)(ceil(dfactor[1]*M));
            out_sz[2] = (V3DLONG)(ceil(dfactor[2]*P));
            out_sz[3] = 1;
            upsampleImage(data1d,image_resampled,in_sz,out_sz,dfactor);
            simple_saveimage_wrapper(callback, (char*)out_image_file.c_str(),(unsigned char *)image_resampled, out_sz, 1);
            if(data1d) {delete []data1d; data1d =0;}
            if(image_resampled) {delete []image_resampled; image_resampled =0;}
            return true;
        }
        else
            printHelp();
        return true;
    }
    else if (func_name == tr("TeraImage_SWC_Crop"))
    {
        string inimg_file,inswc_file,inapo_file;
        if(infiles.size()>=3) {
            inimg_file = infiles[0];
            inswc_file = infiles[1];
            inapo_file = infiles[2];
            int cropx=(inparas.size()>=1)?atoi(inparas[0]):256;
            int cropy=(inparas.size()>=2)?atoi(inparas[1]):256;
            int cropz=(inparas.size()>=3)?atoi(inparas[2]):256;
            //get out_image_path and out_swc_path
            QString out_path=(outfiles.size()>=1)?outfiles[0]:(QFileInfo(QString::fromStdString(inswc_file)).path());
            teraImage_swc_crop(callback,inimg_file,inswc_file,inapo_file,out_path,cropx,cropy,cropz);
        }
        else
            printHelp();
    }
    else if (func_name == tr("help"))
        printHelp();
	else return false;
    return true;
}
void printHelp()
{
    cout<<"Please find README.md in project folder"<<endl;
}
