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
        <<tr("Intensity_profile_terafly")
       <<tr("Bouton_filter")
      << tr("TeraImage_SWC_Crop")
      <<tr("BoutonSWC_Compress")
     <<tr("Scale_registered_swc")
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
    else if(func_name == tr("Intensity_profile_terafly"))
    {
        /*get node intensity from terafly image, 2021-04-01
         * output intensity to (NeuronSWC)level feature
        */
        string inswc_file,inimg_file;
        if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
        else {  printHelp(); return false;}
        //read para list
        int Shift_Pixels=(inparas.size()>=1)?atoi(inparas[0]):2;
        int Min_Interpolation_Pixels=(inparas.size()>=2)?atoi(inparas[1]):4;
        int allnode=(inparas.size()>=3)?atoi(inparas[2]):1;
        long crop_size=(inparas.size()>=4)?atoi(inparas[3]):128;
        int bkg_thre_bias=(inparas.size()>=5)?atoi(inparas[4]):20;

        //read input swc to neuron-tree
       NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
       if(!nt.listNeuron.size()) return false;
        // 0. axonal part of the neuron-tree
       NeuronTree nt_p;
       if(!allnode)
       {
            for(V3DLONG i=0;i<nt.listNeuron.size();i++)
            {
                if(nt.listNeuron[i].type==2){
                    nt_p.listNeuron.append(nt.listNeuron[i]);
                    nt_p.hashNeuron.insert(nt.listNeuron[i].n,nt_p.listNeuron.size()-1);}
            }
       }
       else
           nt_p.deepCopy(nt);
       //1. interpolation
       NeuronTree nt_interpolated;       nt_interpolated=linearInterpolation(nt_p,Min_Interpolation_Pixels);
       //2. shift or refinement
        boutonDetection_terafly_fun(callback,inimg_file,nt_interpolated,Shift_Pixels,crop_size,bkg_thre_bias);
        string out_intensity_swc=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_intensity.eswc");
        writeESWC_file(QString::fromStdString(out_intensity_swc),nt_interpolated);
    }
    else if(func_name == tr("Bouton_filter"))
    {
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        double radius_delta=(inparas.size()>=1)?atoi(inparas[0]):1.3;
        double intensity_delta=(inparas.size()>=2)?atoi(inparas[1]):0.05;
        double axon_trunk_radius=(inparas.size()>=3)?atoi(inparas[2]):3;

        //read input swc to neuron-tree
       NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
       if(!nt.listNeuron.size()) return false;
       //get boutons
        QList <CellAPO> apolist; apolist=getBouton_1D_filter(nt,radius_delta,intensity_delta,axon_trunk_radius);

        // out to apo
        double min_bouton_dist=3.0;
        QList <CellAPO> apolist_out;apolist_out.clear();
        apolist_out=rmNearMarkers(apolist,min_bouton_dist);
        //bouton out to swc
        NeuronTree nt_interpolated3;        nt_interpolated3.copy(nt);
        for(V3DLONG i=0;i<nt_interpolated3.listNeuron.size();i++)
        {
            for(V3DLONG b=0;b<apolist_out.size();b++)
            {
                if(nt_interpolated3.listNeuron[i].x==apolist_out[b].x
                        &&nt_interpolated3.listNeuron[i].y==apolist_out[b].y
                        &&nt_interpolated3.listNeuron[i].z==apolist_out[b].z)
                {
                    nt_interpolated3.listNeuron[i].type=BoutonType;
                    nt_interpolated3.listNeuron[i].r=apolist_out[b].volsize;
                    nt_interpolated3.listNeuron[i].level=apolist_out[b].intensity;
                    nt_interpolated3.listNeuron[i].timestamp=apolist_out[b].sdev;
                    nt_interpolated3.listNeuron[i].tfresindex=apolist_out[b].mass;
                    break;
                }
            }
        }
        //crop 3D bouton block and mip image
        if(outfiles.size()==1&&infiles.size()==2)
        {
            //get out path
            string out_path=outfiles[0];
            int bouton_crop_size=(inparas.size()>=2)?atoi(inparas[1]):8;
            int get_mip=(inparas.size()>=3)?atoi(inparas[2]):0;
            string inimg_file = infiles[1];
            getBoutonBlock(callback,inimg_file,apolist_out,out_path,bouton_crop_size,get_mip);
        }
        else{
            //save to file: intensity_file, bouton_apo_file, bouton_eswc_file
    //        string out_swc_file=(outfiles.size()>=2)?outfiles[1]:(inswc_file + "_intensity.eswc");
            string out_bouton_apo_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_bouton.apo");
            string out_bouton_swc_file=(outfiles.size()>=2)?outfiles[1]:(inswc_file + "_bouton.eswc");

    //        writeESWC_file(QString::fromStdString(out_swc_file),nt_interpolated2);
            writeESWC_file(QString::fromStdString(out_bouton_swc_file),nt_interpolated3);
            writeAPO_file(QString::fromStdString(out_bouton_apo_file),apolist_out);
        }
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
    else if (func_name == tr("BoutonSWC_Compress"))
    {
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        int pruning_thre=(inparas.size()>=1)?atoi(inparas[0]):5;

        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nt.listNeuron.size()) return false;
        string out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_compressed.eswc");
        NeuronTree compressed_nt=boutonSWC_internode_pruning(nt,pruning_thre);
        writeESWC_file(QString::fromStdString(out_nt_filename),compressed_nt);
    }
    else if (func_name == tr("Scale_registered_swc"))
    {
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        float shift_pixels=(inparas.size()>=1)?atoi(inparas[0]):20.0;
        float scale_para=(inparas.size()>=2)?atoi(inparas[1]):25.0;

        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nt.listNeuron.size()) return false;
        string out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_scaled.eswc");
        NeuronTree scaled_nt=scale_registered_swc(nt,shift_pixels,scale_para);
        writeESWC_file(QString::fromStdString(out_nt_filename),scaled_nt);
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
    cout<<"need to complete"<<endl;
}
