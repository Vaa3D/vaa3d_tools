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
            <<tr("Intensity_profile_terafly")
          <<tr("Bouton_filter")
         <<tr("BoutonAsPeak_terafly")
         <<tr("BoutonAsPeak_Image")
        <<tr("BoutonDection_filter_toSWC")
       <<tr("SWC_Analysis")
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
//        getNodeRadius(inimg1d,in_sz,nt);
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
    if (func_name == tr("Intensity_profile_terafly"))
	{
        /*get node intensity from terafly image, 2021-04-01
         * output intensity to (NeuronSWC)level feature
        */
        string inswc_file,inimg_file;
        if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
        else {  printHelp(); return false;}
        //read para list
        int Min_Interpolation_Pixels=(inparas.size()>=1)?atoi(inparas[0]):3;
        int Shift_Pixels=(inparas.size()>=2)?atoi(inparas[1]):2;
        int allnode=(inparas.size()>=3)?atoi(inparas[2]):1;

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
        getSWCIntensityInTeraflyImg(callback,inimg_file,nt_interpolated,Shift_Pixels);
        string out_intensity_swc=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_intensity.eswc");
        writeESWC_file(QString::fromStdString(out_intensity_swc),nt_interpolated);
        return true;
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
    else if (func_name == tr("Bouton_filter"))
    {
        string inswc_file;
        if(infiles.size()>=1) {inswc_file = infiles[0];}
        else {  printHelp(); return false;}
        //read para list
        int min_bouton_dist=(inparas.size()>=1)?atoi(inparas[0]):3;

        //read input swc to neuron-tree
       NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
       if(!nt.listNeuron.size()) return false;
       //get boutons
        QList <CellAPO> apolist; apolist=getBouton_1D_filter(nt);

        //5.
        // out to apo
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
                {nt_interpolated3.listNeuron[i].type=99;break;}
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
    else if (func_name == tr("BoutonAsPeak_terafly"))
    {
        /*peak detection algorithm on bouton detection,2021-04-20 ~ 2021-04-22
         * updated at 2021-07-31
        *Usage:
         * input file: <in_terafly_formatted_brain_highest_path>, <in_swc_file_path or in_eswc_file_path>
         * output file: <out_bouton_block>,<out_intensity_eswc_file_path>,<out_bouton_apo_file_path>,<out_bouton_eswc_file_path>
         * input para:
                * <Min_Interpolation_Pixels>,<Shift_Pixels>,
                * <min_bouton_dist>,<all_node or axonal part>,
                * <bouton_crop_size>,<mip>
         *
         * Workflow:
         * 0. for axonal part of neuron-tree
         * 1. (linear) interpolation of the neuron tree
                    * Min_Interpolation_Pixels=1
         * 2. shift each node to a position with maximum local intensity
                    * Shift_Pixels=1 or 2
                    * get Intensity and radius profile
                    * bkg_threshold=IMG_average+IMG_std
         * 3. neuron tree to segment list
         * 4. for each segment, use peak-detection get bouton index list
         * 5. out
                * get mip
        */
        string inimg_file,inswc_file;
        if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
        else {  printHelp(); return false;}
        //read para list
        int Min_Interpolation_Pixels=(inparas.size()>=1)?atoi(inparas[0]):3;
        int Shift_Pixels=(inparas.size()>=2)?atoi(inparas[1]):2;
        int min_bouton_dist=(inparas.size()>=3)?atoi(inparas[2]):4;
        int allnode=(inparas.size()>=4)?atoi(inparas[3]):0;

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

       //2. shift or refinement; Intensity and radius profile
        getSWCIntensityInTeraflyImg(callback,inimg_file,nt_interpolated,Shift_Pixels);
        cout<<"end of getting intensity and radius profile"<<endl;

        //3. neuron tree to segment list
        QList <CellAPO> apolist_init_boutons;        apolist_init_boutons=getBouton_1D_filter(nt_interpolated);

        //5. out
        QList <CellAPO> apo_boutons;apo_boutons.clear();
        apo_boutons=rmNearMarkers(apolist_init_boutons,min_bouton_dist);

        //bouton out to swc
        NeuronTree nt_bouton;        nt_bouton.copy(nt_interpolated);
        for(V3DLONG i=0;i<nt_bouton.listNeuron.size();i++)
        {
            for(V3DLONG b=0;b<apo_boutons.size();b++)
            {
                if(float(nt_bouton.listNeuron[i].x)==apo_boutons[b].x
                        &&float(nt_bouton.listNeuron[i].y)==apo_boutons[b].y
                        &&float(nt_bouton.listNeuron[i].z)==apo_boutons[b].z)
                {nt_bouton.listNeuron[i].type=99;break;}
            }
        }
        //crop 3D bouton block and mip image
        if(outfiles.size()==1)
        {
            //get out path
            string out_path=outfiles[0];
            int bouton_half_crop_size=(inparas.size()>=5)?atoi(inparas[4]):8;
            int get_mip=(inparas.size()>=6)?atoi(inparas[5]):0;
            getBoutonBlock(callback,inimg_file,apo_boutons,out_path,bouton_half_crop_size,get_mip);
            getBoutonBlockSWC(nt_bouton,out_path,bouton_half_crop_size);
        }
        else{
            //save to file: intensity_file, bouton_apo_file, bouton_eswc_file
            string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_intensity.eswc");
            string out_bouton_apo_file=(outfiles.size()>=2)?outfiles[1]:(inswc_file + "_bouton.apo");
            string out_bouton_swc_file=(outfiles.size()>=3)?outfiles[2]:(inswc_file + "_bouton.eswc");
            string out_init_bouton_apo_file=(outfiles.size()>=4)?outfiles[3]:(inswc_file + "_initial_bouton.eswc");
            writeESWC_file(QString::fromStdString(out_swc_file),nt_interpolated);
            writeESWC_file(QString::fromStdString(out_bouton_swc_file),nt_bouton);
            writeAPO_file(QString::fromStdString(out_bouton_apo_file),apo_boutons);
            writeAPO_file(QString::fromStdString(out_init_bouton_apo_file),apolist_init_boutons);
        }

    }
    else if (func_name == tr("BoutonAsPeak_Image"))
    {
        /*peak detection algorithm on bouton detection,2021-04-20 ~ 2021-04-22
        *Usage:
         * input file: <in_image_block_file_path>, <in_swc_file_path or in_eswc_file_path>
         * output file: <out_bouton_block>,<out_intensity_eswc_file_path>,<out_bouton_apo_file_path>,<out_bouton_eswc_file_path>
         * input para:<Min_Interpolation_Pixels>,<Shift_Pixels>,<min_bouton_dist>,<bouton_crop_size>
         *
         * Workflow:
         * 1. (linear) interpolation of the neuron tree
                    * Min_Interpolation_Pixels=1
         * 2. shift each node to a position with maximum local intensity
                    * Shift_Pixels=1 or 2
         * 3. neuron tree to segment list
         * 4. for each segment, use peak-detection get bouton index list
         * 5. out
        */
        string inswc_file,inimg_file;
        if(infiles.size()>=2) {inimg_file = infiles[0];inswc_file = infiles[1];}
        else {  printHelp(); return false;}

        //read para list
        int Min_Interpolation_Pixels=(inparas.size()>=1)?atoi(inparas[1]):2;
        int Shift_Pixels=(inparas.size()>=2)?atoi(inparas[1]):1;
        int min_bouton_dist=(inparas.size()>=3)?atoi(inparas[2]):3;
        int is2dRadius=(inparas.size()>=4)?atoi(inparas[3]):1;

         //read input swc to neuron-tree
        NeuronTree nt = readSWC_file(QString::fromStdString(inswc_file));
        if(!nt.listNeuron.size()) return false;

        //1. interpolation
        NeuronTree nt_interpolated;nt_interpolated.listNeuron.clear();nt_interpolated.hashNeuron.clear();
        nt_interpolated=linearInterpolation(nt,Min_Interpolation_Pixels);

        //2. shift or refinement
        getSWCIntensityInImg(callback,inimg_file,nt_interpolated,Shift_Pixels,is2dRadius);

        //3. neuron tree to segment list
        QList <CellAPO> apolist;        apolist=getBouton_1D_filter(nt_interpolated);

        //5.
        // out to apo
        QList <CellAPO> apolist_out;apolist_out.clear();
        apolist_out=rmNearMarkers(apolist,min_bouton_dist);

        //bouton out to swc
        NeuronTree nt_interpolated3;        nt_interpolated3.copy(nt_interpolated);
        for(V3DLONG i=0;i<nt_interpolated3.listNeuron.size();i++)
        {
            for(V3DLONG b=0;b<apolist_out.size();b++)
            {
                if(nt_interpolated3.listNeuron[i].x==apolist_out[b].x
                        &&nt_interpolated3.listNeuron[i].y==apolist_out[b].y
                        &&nt_interpolated3.listNeuron[i].z==apolist_out[b].z)
                {nt_interpolated3.listNeuron[i].type=99;break;}
            }
        }
        //crop 3D bouton block
        if(outfiles.size()==1)
        {
            //get out path
            string out_path=outfiles[0];
            int bouton_crop_size=(inparas.size()>=4)?atoi(inparas[3]):16;
            /*this function is not ready*/
            getBoutonBlock_inImg(callback,inimg_file,apolist_out,out_path,bouton_crop_size);
        }
        else{
            //save to file: intensity_file, bouton_apo_file, bouton_eswc_file
            string out_swc_file=(outfiles.size()>=1)?outfiles[0]:(inswc_file + "_intensity.eswc");
            string out_bouton_apo_file=(outfiles.size()>=2)?outfiles[1]:(inswc_file + "_bouton.apo");
            string out_bouton_swc_file=(outfiles.size()>=3)?outfiles[2]:(inswc_file + "_bouton.eswc");
            writeESWC_file(QString::fromStdString(out_swc_file),nt_interpolated);
            writeESWC_file(QString::fromStdString(out_bouton_swc_file),nt_interpolated3);
            writeAPO_file(QString::fromStdString(out_bouton_apo_file),apolist_out);
        }

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
    else if (func_name == tr("help"))
    {
        printHelp();
	}
	else return false;
    return true;
}
void printHelp()
{
    qDebug()<<"               -----Usage_basic: function for getting boutons from terafly-format datasets----              ";
    qDebug()<<"1. vaa3d -x <libname:BoutonDectection> -f BoutonAsPeak_terafly -i <input_image_terafly> <input_swc>";
    qDebug()<<"                                                                        -o <out_path> <intensity_path> <bouton_apo_path> <bouton_swc_path>";
    qDebug()<<"                                                                        -p <Min_Interpolation_Pixels> <Shift_Pixels> <min_bouton_dist> <all_node or axonal part> <bouton_crop_size> <mip>";
    qDebug()<<"e.g. vaa3d -x BoutonDectection -f BoutonAsPeak_terafly -i <input_image_terafly> <input_swc> -o <out_path> -p 2 1 3 0 8 1";
    qDebug()<<"2. vaa3d -x <libname:BoutonDectection> -f Intensity_profile_terafly -i <input_image_terafly> <input_swc>";
    qDebug()<<"                        -----Usage_basic: function for getting boutons from  vaa3d-supportted image----              ";
    qDebug()<<"Usage: get bouton in using inputImg and inputSWC";
    qDebug()<<"vaa3d -x <libname:BoutonDectection> -f BoutonAsPeak_Image -i <input_image> <input_swc>";
}
