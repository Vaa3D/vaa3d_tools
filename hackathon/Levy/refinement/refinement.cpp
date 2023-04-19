  /* refinement.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-6-24 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "refinement.h"
#include "n_class.h"
#include <fstream>
#include "nlohmann\json.hpp"
using namespace std;
Q_EXPORT_PLUGIN2(refine_swc, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by YiweiLi, 2021-10-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

 if(func_name==tr("refine"))  // the main function to do refinement on the whole-brain images(Terafly version)
    {
        NeuronTree nt1=readSWC_file(infiles[0]);
        QString braindir= infiles[1];
        SwcTree a;
        a.initialize(nt1);  // initialize the neuron structure
        NeuronTree refinetree = a.refine_swc_by_gd(braindir,callback);  // do the refinement based on GD
        SwcTree b;
        b.initialize(refinetree);
        string inswc_file=infiles[0];
        QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
        NeuronTree result= b.refine_bifurcation_by_gd(braindir,callback,eswcfile);
//        QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
        writeSWC_file(eswcfile,result);
 }else if(func_name==tr("refine_SIAT")){
        NeuronTree nt1=readSWC_file(infiles[0]);
        QString braindir= infiles[1];
        SwcTree a;
        int stitch_flag = atoi(inparas[0]);
        float step = atoi(inparas[1]);
        if(stitch_flag){
            a.initialize_SIAT(nt1,step);  // initialize the neuron structure with 300um stitch
        }else{
           a.initialize(nt1);  // initialize the neuron structure
        }
        int thres = atoi(inparas[2]);
        int img_size_x = atoi(inparas[3]);
        int img_size_y = atoi(inparas[4]);
        int img_size_z = atoi(inparas[5]);
        float shift_z = atof(inparas[6]);
        cout<<"initialization finished"<<endl;
        NeuronTree refinetree = a.refine_swc_by_gd_SIAT(braindir,thres,img_size_x,img_size_y,img_size_z,shift_z,callback);  // do the refinement based on GD
        cout<<"refinement finished"<<endl;
        string inswc_file=infiles[0];
        QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
        writeSWC_file(eswcfile,refinetree);
 }
    else if (func_name == tr("help"))
    {
        cout<<"usage:"<<endl;
        cout<<"v3d -x refine -f refine -i [file_swc] [brain_path] -o [txt_file]"<<endl;
        cout<<"v3d -x refine_img -f refine_img -i [file_swc] [img_path] -o [txt_file]"<<endl;
        cout<<"v3d -x refine_SAIT -f refine_SIAT -i [file_swc] [brain_path] -o [txt_file] -p stitch_flag stitch_position thres imgsize_x imgsize_y imgsize_z shift_z"<<endl;
    }else if(func_name==tr("refine_img")) // the main function to do refinement on the cropped images(tif version)
    {
     NeuronTree nt1=readSWC_file(infiles[0]);
     string inimg_file= infiles[1];
     SwcTree a;
     a.initialize(nt1);  // initialize the neuron structure
     NeuronTree refinetree = a.refine_swc_by_gd_img(inimg_file,callback);// do the refinement based on GD
     string inswc_file=infiles[0];
     QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
     writeESWC_file(eswcfile,refinetree);
 }else if(func_name==tr("feature_extraction_rate")){  // calculate the feature extraction rate
        string inimg_file= infiles[1];
        string inswc_file=infiles[0];
        NeuronTree nt=readSWC_file(infiles[0]);
        float feature_extraction_rate;
        feature_extraction_rate=get_feature_extraction_rate(inimg_file,nt,callback);
        if(feature_extraction_rate>=1){feature_extraction_rate=1;}
        QString feature_extraction_rate_txt = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_feature_extraction_rate.csv"));
        ofstream csvOutFile;
        csvOutFile.open(feature_extraction_rate_txt.toStdString().c_str(),ios::out | ios::app);
        csvOutFile<<feature_extraction_rate<<","<<inswc_file<<endl;
        csvOutFile.close();
 }else if(func_name==tr("swc_compress")){

 }else if(func_name==tr("image_snr")){  // compute the snr of the synthetic images
        QString infolder_1=infiles[0];
        QString infolder_2=infiles[1];
        QString SNr_out = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString(("image_snr.csv"));
        compute_image_snr(infolder_1,infolder_2,SNr_out,callback);
 }else if(func_name==tr("compare_2swc_change")){ // compare the difference between raw and refined swc files
//        string inswc_file=infiles[0];
        NeuronTree nt1=readSWC_file(infiles[0]);
        NeuronTree nt2=readSWC_file(infiles[1]);
        string inswc_file=infiles[0];
        QString change_out = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+".csv"));
        compare_2swc_change(nt1,nt2,callback,change_out);
 }else if(func_name==tr("blend")){
        NeuronTree nt1=readSWC_file(infiles[0]);
        NeuronTree nt2=readSWC_file(infiles[1]);
        string inswc_file=infiles[0];
        QString file_out = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+".swc"));
        blend(nt1,nt2,file_out);
        cout<<"blend done"<<endl;

 }else if(func_name==tr("refine_analysis")){   // calculate the metrics used in the manuscript
         QString infolder_1=infiles[0];
         QString infolder_2=infiles[1];
         QString refine_analysis_file = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString(("refine_analysis.csv"));
         refine_analysis(infolder_1,infolder_2,refine_analysis_file,callback);
 } else if(func_name==tr("refine_analysis_swc")){
     QString infolder_1=infiles[0];
     QString infolder_2=infiles[1];
     QString refine_analysis_folder = (outfiles.size()>=1) ? outfiles[0] : infolder_1;
     refine_analysis_swc(infolder_1,infolder_2,refine_analysis_folder,callback);
}else if(func_name==tr("MIP")){    // output the MIP files
     NeuronTree nt1=readSWC_file(infiles[0]);//after refine
     NeuronTree nt2=readSWC_file(infiles[1]);// raw swc
     QString braindir= infiles[2];
     SwcTree a;
     a.initialize(nt1);
     QString imgdir=outfiles[0];
     a.MIP_terafly(nt1,nt2,braindir,imgdir,callback);
 }else if(func_name==tr("MIP2")){
     NeuronTree nt1=readSWC_file(infiles[0]);//after refine
     NeuronTree nt2=readSWC_file(infiles[1]);// raw swc
     QString braindir= infiles[2];
     QString imgdir=outfiles[0];
     MIP_terafly2(nt1,nt2,braindir,imgdir,callback);
 }else if(func_name==tr("tapping_ratio")){
     NeuronTree nt1=readSWC_file(infiles[0]);
     string inswc_file=infiles[0];
     SwcTree a;
     qDebug()<<"1111"<<endl;
     a.initialize(nt1);
     QString tapping_ratio_txt1 = (outfiles.size()>=3) ? outfiles[0] : QString::fromStdString((inswc_file+"_taper1.csv"));
     QString tapping_ratio_txt2 = (outfiles.size()>=3) ? outfiles[1] : QString::fromStdString((inswc_file+"_taper2.csv"));
     QString tapping_ratio_txt3 = (outfiles.size()>=3) ? outfiles[2] : QString::fromStdString((inswc_file+"_Pk_classic.csv"));
     a.calculate_tapping_ratio(tapping_ratio_txt1,tapping_ratio_txt2,tapping_ratio_txt3,callback);

 }else if(func_name==tr("bouton_distribution")){
     QString infolder_1=infiles[0];
     QString bouton_distribution_file = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((infolder_1.toStdString()+"/axon_distribution.csv"));
     bouton_distribution(infolder_1,bouton_distribution_file,callback);
 }else if(func_name==tr("terminuax_bouton_count")){
     QString infolder_1=infiles[0];
     QString bouton_distribution_file = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((infolder_1.toStdString()+"/bouton_count.csv"));
     terminuax_bouton_count(infolder_1,bouton_distribution_file,callback);
 }else if(func_name==tr("bouton_seg_distribution")){
     QString infolder_1=infiles[0];
     QString outfolder = outfiles[0];
     bouton_seg_distribution(infolder_1,outfolder,callback);
 }
    else return false;

	return true;
}

