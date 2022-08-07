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
          <<tr("Preprocess")
         << tr("Refinement_terafly")
         <<tr("Refinement_image")
        <<tr("SWC_profile_terafly")
       <<tr("SWC_profile")
      <<tr("Bouton_filter")
     << tr("TeraImage_SWC_Crop")
     <<tr("BoutonSWC_pruning")
    <<tr("CCF_profile")
    <<tr("Bouton_feature")
    <<tr("FileTo")
    <<tr("UpsampleImage")
    <<tr("Postprocess")
    << tr("to_swc")
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
        unsigned char* inimg1d_raw = p4DImage->getRawData();

        in_sz[0] = p4DImage->getXDim();
        in_sz[1] = p4DImage->getYDim();
        in_sz[2] = p4DImage->getZDim();
        in_sz[3] = p4DImage->getCDim();

        cout<<"Img size,x="<<in_sz[0]<<",y="<<in_sz[1]<<",z="<<in_sz[2]<<endl;
        NeuronTree nt =callback.getSWC(curwin);
        V3DLONG siz = nt.listNeuron.size();
        cout<<"SWC size:"<<siz<<endl;

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
//        float threshold = atof(thresh_box->text().toStdString().c_str());
        QString savepath=QString::fromStdString(save_path);
        QDir path(savepath);
        if(!path.exists())
            path.mkpath(savepath);
        if(loop_checking(nt)){return;}
        NeuronTree nt_pre=preprocess_simple(nt);
        NeuronTree nt_interpolated=node_interpolation(nt_pre,4,true);
        cout<<"get radius and intensity profile from image block"<<endl;
        QList<NeuronSWC>& listNeuron =  nt_interpolated.listNeuron;

        //get the background threshold
        V3DLONG sz01 = in_sz[0] * in_sz[1];
        V3DLONG sz0 = in_sz[0];
        V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];

        /*image enhancement*/
        unsigned char * inimg1d = 0;
        try {inimg1d = new unsigned char [total_size];}
        catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl; return;}

        if(!enhanceImage(inimg1d_raw,inimg1d,in_sz))
        {
            cout<<"adaptive thresholding and enhancement fail"<<endl;
            inimg1d_raw = p4DImage->getRawData();
            double imgave,imgstd;
            mean_and_std(inimg1d_raw,total_size,imgave,imgstd);
            double bkg_thresh=MIN(MAX(imgave+imgstd+15,30),60);
            cout<<"bkg thresh="<<bkg_thresh<<","<<imgave<<","<<imgstd<<endl;

            for(V3DLONG i=0;i<siz;i++)
            {
                //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
                NeuronSWC s = listNeuron.at(i);
                V3DLONG thisx,thisy,thisz;
                thisx=s.x;thisy=s.y;thisz=s.z;
                s.level=inimg1d_raw[thisz * sz01 + thisy * sz0 + thisx];
                //get node radius
                s.r=radiusEstimation(inimg1d_raw,in_sz,s,4,bkg_thresh);
                s.timestamp=bkg_thresh;
                listNeuron[i].level=s.level;
                listNeuron[i].x=s.x;
                listNeuron[i].y=s.y;
                listNeuron[i].z=s.z;
                listNeuron[i].r=s.r;
            }
        }
        else
        {
            cout<<"adaptive thresholding and enhancement"<<endl;
            double imgave,imgstd;
            mean_and_std(inimg1d,total_size,imgave,imgstd);
            double bkg_thresh=MIN(MAX(imgave+imgstd+15,30),60);
            cout<<"bkg thresh="<<bkg_thresh<<","<<imgave<<","<<imgstd<<endl;

            for(V3DLONG i=0;i<siz;i++)
            {
                //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
                NeuronSWC s = listNeuron.at(i);
                V3DLONG thisx,thisy,thisz;
                thisx=s.x;thisy=s.y;thisz=s.z;
                s.level=inimg1d[thisz * sz01 + thisy * sz0 + thisx];
                //get node radius
                s.r=radiusEstimation(inimg1d,in_sz,s,4,bkg_thresh);
                s.timestamp=bkg_thresh;
                listNeuron[i].level=s.level;
                listNeuron[i].x=s.x;
                listNeuron[i].y=s.y;
                listNeuron[i].z=s.z;
                listNeuron[i].r=s.r;
            }
        }

        NeuronTree nt_profiled=internode_pruning(nt_interpolated,2,true);
         cout<<"end of getting intensity and radius profile"<<endl;

         //3. get initial bouton-sites and out to list of NeuronSWC
          QList <AxonalBouton> init_bouton_sites=boutonFilter_fun(nt_profiled,1.5,1.0,1.5);
          if(!init_bouton_sites.size()) {return;}
          //map bouton-sites to swc, enlarge fea_val size to 12
          map_bouton_2_neuronTree(nt_profiled,init_bouton_sites);

          //4. filter and pruning
          float min_bouton_dist=8.0;
          nearBouton_pruning(nt_profiled,min_bouton_dist,false);

        QString outswc_file =savepath+"/"+"bouton.eswc";
        writeESWC_file(outswc_file,nt_profiled);

        QList<CellAPO> apolist=bouton_to_apo(nt_profiled);
        QString apo_file_path = savepath +"/"+ "bouton.apo";
        writeAPO_file(apo_file_path,apolist);
        if(inimg1d_raw) {delete []inimg1d_raw; inimg1d_raw=0;}
        if(inimg1d) {delete []inimg1d; inimg1d=0;}
        callback.setSWC(curwin,nt_profiled);
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
    else if(func_name == tr("Refinement_terafly"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,true);
    }
    else if(func_name == tr("Refinement_image"))
    {
        /* refine to center line, based on mean-shift*/
        refinement_dofunc(callback,input,output,false);
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
    else if (func_name == tr("FileTo"))
    {
        bouton_file_dofunc(callback,input,output);
    }
    else if (func_name == tr("Preprocess"))
    {
        preprocess_dofunc(callback,input,output);
    }
    else if (func_name == tr("Neuron_checking"))
    {
        neuron_checking_dofunc(callback,input,output);
    }
    else if (func_name == tr("Postprocess"))
    {
        postprocess_dofunc(callback,input,output);
    }
    else if (func_name == tr("UpsampleImage"))
    {

        string inimg_file;
        if(infiles.size()>=1) {
            inimg_file = infiles[0];
            double x_rez=(inparas.size()>=1)?atof(inparas[0]):4;
            double y_rez=(inparas.size()>=2)?atof(inparas[1]):4;
            double z_rez=(inparas.size()>=3)?atof(inparas[2]):1;

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
    else if (func_name == tr("to_swc"))
    {
        QString inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        NeuronTree nt = readSWC_file(inswc_file);
        if(!nt.listNeuron.size()) return false;
        int level_2_type=(inparas.size()>=1)?atoi(inparas[0]):0;
        if(level_2_type>0)
            for(V3DLONG i=0;i<nt.listNeuron.size();i++)
                nt.listNeuron[i].type=nt.listNeuron.at(i).level;
        QString out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file+"_out.swc");
        writeSWC_file(out_swc_file,nt);
    }
    else if (func_name == tr("swc_scale"))
    {
        /*scale registered file to ccf*/
        QString inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        float r_scale=(inparas.size()>=1)?atof(inparas[0]):1.0;
        int shift_pixels=(inparas.size()>=2)?atoi(inparas[1]):0;
        int scale_xyz=(inparas.size()>=3)?atoi(inparas[2]):1;
        NeuronTree nt = readSWC_file(inswc_file);
        if(!nt.listNeuron.size()) return false;
        V3DLONG siz=nt.listNeuron.size();
//        int shift_pixels=20;
//        int scale_xyz=25;
        for(V3DLONG i=0;i<siz;i++){
            nt.listNeuron[i].x-=shift_pixels;
            nt.listNeuron[i].x*=scale_xyz;
            nt.listNeuron[i].y*=scale_xyz;
            nt.listNeuron[i].z*=scale_xyz;
            nt.listNeuron[i].r*=r_scale;
        }
        QString out_nt_filename=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_ccfprofiled.eswc");
        writeESWC_file(out_nt_filename,nt);
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
