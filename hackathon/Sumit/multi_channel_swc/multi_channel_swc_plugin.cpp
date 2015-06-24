/* multi_channel_swc_plugin.cpp
 * This plugin will creat swc files based on multiple channel information in the neuron image.
 * 2015-6-18 : by Sumit and Hanbo
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "multi_channel_swc_plugin.h"
#include "multi_channel_swc_dialog.h"
#include "multi_channel_swc_func.h"
#include "multi_channel_swc_template.h"

using namespace std;

Q_EXPORT_PLUGIN2(multi_channel_swc, MultiChannelSWC);
 
QStringList MultiChannelSWC::menulist() const
{
	return QStringList() 
        <<tr("multi_channel_compute")
       <<tr("multi_channel_render")
		<<tr("about");
}

QStringList MultiChannelSWC::funclist() const
{
	return QStringList()
        <<tr("compute_swc")
        <<tr("render_swc_by_eswc")
		<<tr("help");
}

void MultiChannelSWC::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("multi_channel_compute"))
    {
        multi_channel_swc_dialog dialog(&callback);
        dialog.exec();
	}
    else if (menu_name == tr("multi_channel_render"))
    {
        do_calculate_render_eswc();
    }
	else
	{
        v3d_msg(tr("This plugin will create swc files based on multiple channel information in the neuron image.."
                   "Developed by Sumit Nanda (snanda2@gmu.edu) and Hanbo Chen (cojoc.chen@gmail.com), 2015-6-18"));
	}
}

bool MultiChannelSWC::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("compute_swc"))
	{
        //load input
        if(infiles.size()!=2 || outfiles.size()!=1)
        {
            qDebug()<<"ERROR: please set input and output! "<<infiles.size()<<":"<<outfiles.size();
            printHelp();
            return false;
        }
        //load neurons
        QString fname_swc = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_img = ((vector<char*> *)(input.at(0).p))->at(1);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);
        NeuronTree* nt = new NeuronTree();
        if (fname_swc.toUpper().endsWith(".SWC") || fname_swc.toUpper().endsWith(".ESWC")){
            *nt = readSWC_file_multichannel(fname_swc);
        }
        if(nt->listNeuron.size()<=0){
            qDebug()<<"ERROR: failed to read SWC file: "<<fname_swc;
            return false;
        }
        unsigned char* p_img;
        V3DLONG sz_img[4];
        int intype;
        if(!simple_loadimage_wrapper(callback, fname_img.toStdString().c_str(), p_img, sz_img, intype))
        {
            qDebug()<<"ERROR failed to read image file: "<<fname_img;
            return false;
        }
        //check image channel information
        if(sz_img[3]<2){
            qDebug()<<"ERROR: image only has 1 channel!";
            return false;
        }

        //get parameters
        float thr1=30, thr2=30;
        int ch1=1, ch2=0;
        if(input.size() >= 2){
            vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
            if(paras.size()>0){
                int tmp=atoi(paras.at(0));
                if(tmp>=0 && tmp<sz_img[3])
                    ch1=tmp;
                else
                    cerr<<"error: primary channel is larger than image dimension: "<<tmp<<"; use default value "<<ch1<<endl;
            }
            if(paras.size()>1){
                int tmp=atoi(paras.at(1));
                if(tmp>=0 && tmp<sz_img[3])
                    ch2=tmp;
                else
                    cerr<<"error: secondary channel is larger than image dimension: "<<tmp<<"; use default value "<<ch2<<endl;
            }
            if(paras.size()>2){
                float tmp=atof(paras.at(2));
                thr1=tmp;
            }
            if(paras.size()>3){
                float tmp=atof(paras.at(3));
                thr2 = tmp;
            }
        }
        cout<<"primary channel="<<ch1<<"(start from 0); threshold="<<thr1<<endl;
        cout<<"secondary channel="<<ch2<<"(start from 0); threshold="<<thr2<<endl;
        cout<<"performing calculation:";
        //do calculate
        vector<float> N, mean, stdev, ratio, N_P, ratio_P, mean_P, stdev_P;
        if(intype==2)
            calculate_info((unsigned short *)p_img, sz_img, nt, ch1, thr1, ch2, thr2, N, ratio, mean, stdev, N_P, ratio_P, mean_P, stdev_P);
        else if(intype==4)
            calculate_info((float *)p_img, sz_img, nt, ch1, thr1, ch2, thr2, N, ratio, mean, stdev, N_P, ratio_P, mean_P, stdev_P);
        else
            calculate_info(p_img, sz_img, nt, ch1, thr1, ch2, thr2, N, ratio, mean, stdev, N_P, ratio_P, mean_P, stdev_P);

        qDebug()<<"output result: "<<fname_output;
        //output result
        save_result(fname_output, nt, N, ratio, mean, stdev, N_P, ratio_P, mean_P, stdev_P);
	}
    else if (func_name == tr("render_swc_by_eswc"))
    {
        //load input
        if(infiles.size()!=1 || outfiles.size()!=1)
        {
            qDebug()<<"ERROR: please set input and output! "<<infiles.size()<<":"<<outfiles.size();
            printHelp();
            return false;
        }
        //load neurons
        QString fname_swc = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);
        NeuronTree nt;
        nt = readSWC_file_multichannel(fname_swc);
        if(nt.listNeuron.size()<=0){
            qDebug()<<"ERROR: failed to read SWC file: "<<fname_swc;
            return false;
        }
        //do the calculation
        NeuronTree nt_new = convert_SWC_to_render_ESWC(nt);
        //save eswc
        writeESWC_file(fname_output,nt_new);
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
    qDebug()<<"\n";
    qDebug()<<"============Multi Channel SWC Computation===========";
    qDebug()<<"Usage 1: compute swc";
    qDebug()<<"v3d -x multi_channel_swc -f compute_swc -i file_neuron file_image -o file_output -p <primary channel (1)> <secondary channel (0)> <primary threshould (30)> <secondary threshould (30)>";
    qDebug()<<"Note: channel ID starts from 0";
    qDebug()<<"";
    qDebug()<<"Usage 2: compute eswc for render";
    qDebug()<<"v3d -x multi_channel_swc -f render_swc_by_eswc -i file_neuron -o file_output";
    qDebug()<<"Note: file_swc should be the output from compute_swc";
    qDebug()<<"\n";
}

void do_calculate_render_eswc()
{
    QSettings settings("V3D plugin","multiChannelSWC");
    QString fname_neuronTree;
    if(settings.contains("fname_swc"))
        fname_neuronTree=settings.value("fname_swc").toString();

    //read swc
    QString fileOpenName=fname_neuronTree;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
            fileOpenName,
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    NeuronTree nt;
    if(!fileOpenName.isEmpty()){
        nt = readSWC_file_multichannel(fileOpenName);
    }else{
        return;
    }
    if(nt.listNeuron.size()>0){
        fname_neuronTree=fileOpenName;
    }else{
        v3d_msg("Error: cannot read file "+fileOpenName);
        return;
    }
    if(nt.listNeuron.at(0).fea_val.size()==0){
        v3d_msg("Error: could not find feature informations from the file "+fileOpenName);
    }
    settings.setValue("fname_swc",fname_neuronTree);

    //do the calculation
    NeuronTree nt_new = convert_SWC_to_render_ESWC(nt);

    //save eswc
    QString fileSaveName=fname_neuronTree+"_render.eswc";
    fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                    fileSaveName,
                QObject::tr("Supported file (*.eswc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
    if(fileOpenName.isEmpty()){
        return;
    }
    writeESWC_file(fileSaveName,nt_new);
}
