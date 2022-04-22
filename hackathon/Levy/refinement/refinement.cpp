/* refinement.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-6-24 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "refinement.h"
#include "n_class.h"
#include <fstream>

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
            "Developed by YourName, 2021-10-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

 if(func_name==tr("refine"))
    {
        NeuronTree nt1=readSWC_file(infiles[0]);
        QString braindir= infiles[1];
        SwcTree a;
        a.initialize(nt1);
        NeuronTree refinetree = a.refine_swc_by_gd(braindir,callback);
        string inswc_file=infiles[0];
        QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
        writeESWC_file(eswcfile,refinetree);
    }
    else if (func_name == tr("help"))
    {
        cout<<"usage:"<<endl;
        cout<<"v3d -x refine_swc -f refine -i [file_swc] [brain_path] -o [txt_file]"<<endl;
        cout<<"v3d -x refine_swc -f refine_img -i [file_swc] [img_path] -o [txt_file]"<<endl;

    }else if(func_name==tr("refine_img"))
    {
     NeuronTree nt1=readSWC_file(infiles[0]);
     string inimg_file= infiles[1];
     SwcTree a;
     qDebug()<<"1111"<<endl;
     a.initialize(nt1);
     qDebug()<<"1111"<<endl;
     NeuronTree refinetree = a.refine_swc_by_gd_img(inimg_file,callback);
//     SwcTree b;
//     b.initialize(refinetree);
//     NeuronTree refinebranchtree=b.refine_swc_branch_by_gd_img(inimg_file,callback);
     string inswc_file=infiles[0];
     QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_refined.eswc"));
     writeESWC_file(eswcfile,refinetree);
 }else if(func_name==tr("feature_extraction_rate")){
        string inimg_file= infiles[1];
        string inswc_file=infiles[0];
//        QString inputfolder=QString::fromStdString(infiles[0]);
        NeuronTree nt=readSWC_file(infiles[0]);
        float feature_extraction_rate;
        feature_extraction_rate=get_feature_extraction_rate(inimg_file,nt,callback);
        if(feature_extraction_rate>=1){feature_extraction_rate=1;}
        QString feature_extraction_rate_txt = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+"_feature_extraction_rate.csv"));
        ofstream csvOutFile;
        //cout<<outgf.toStdString()<<endl;
        csvOutFile.open(feature_extraction_rate_txt.toStdString().c_str(),ios::out | ios::app);
        //csvOutFile<<"feature_extraction_rate,swc_file"<<endl;
        csvOutFile<<feature_extraction_rate<<","<<inswc_file<<endl;
        csvOutFile.close();
 }else if(func_name==tr("swc_compress")){

 }else if(func_name==tr("image_snr")){
        QString infolder_1=infiles[0];
        QString infolder_2=infiles[1];
        QString SNr_out = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString(("image_snr.csv"));
        compute_image_snr(infolder_1,infolder_2,SNr_out,callback);
 }else if(func_name==tr("compare_2swc_change")){
//        string inswc_file=infiles[0];
        NeuronTree nt1=readSWC_file(infiles[0]);
        NeuronTree nt2=readSWC_file(infiles[1]);
        string inswc_file=infiles[0];
        QString change_out = (outfiles.size()>=1) ? outfiles[0] : QString::fromStdString((inswc_file+".csv"));
        compare_2swc_change(nt1,nt2,callback,change_out);
 }
    else return false;

	return true;
}

