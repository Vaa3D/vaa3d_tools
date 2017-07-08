#include"resampling.h"
#include"batch_preprocess.h"
#include"overlap/overlap_gold.h"
#include"pre-process.h"
#include<math.h>
using namespace std;

bool batch_preprocess(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    char * folder_path;
    QString consensus_in;
    QString fileSaveFolder;
    if(inlist->size() == 2)
    {
        folder_path = inlist->at(0);
        consensus_in = inlist->at(1);
    }
    else{printf("You need input a folder path and a raw image.");return false; }

    if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        fileSaveFolder = QString(outlist->at(0));
    }
    else {printf("You must  specify one  result folder.\n");return false; }

    double resample_step = 3;

    // read files from a folder
    QStringList swcList = importFileList_addnumbersort(QString(folder_path));
    vector<NeuronTree> nt_list;
    for(V3DLONG i = 2; i < swcList.size(); i++)     // the first two are not files
    {
        QString curPathSWC = swcList.at(i);
        NeuronTree temp = readSWC_file(curPathSWC);
        nt_list.push_back(temp);
    }
    cout << "The number of in_swc is " << nt_list.size() << endl;
    if(nt_list.size()==0) return false;
    NeuronTree nt_consensus =readSWC_file(consensus_in);

    // select file
    double size_mean;
    double size_std;
    double size_total=0;
    double size_var=0;
    for(int i=0; i<nt_list.size();i++)
        size_total += nt_list[i].listNeuron.size();
    size_mean = size_total/nt_list.size();
    for(int i=0; i<nt_list.size();i++)
    {
        size_var+=pow(double(nt_list[i].listNeuron.size() - size_mean),2)/nt_list.size();
    }
    size_std=sqrt(size_var);
    // resample and sort
    cout<<"pre-process begin"<<endl;
    vector<NeuronTree> nt_list2;
    vector<QString>swcList2;
    for(int i=0; i<nt_list.size();i++)
    {
        if(fabs(nt_list[i].listNeuron.size()-size_mean) < 1.5*size_std)
        {
            NeuronTree resample_result;
            resample_result=resample(nt_list[i],resample_step);
            nt_list2.push_back(resample_result);
            swcList2.push_back(swcList.at(i+2));
        }
    }
    for(int i=0; i<nt_list2.size();i++)
    {
        QList<NeuronSWC> sort_result;
        if(!sort_with_standard(nt_list2[i].listNeuron, nt_consensus.listNeuron,sort_result))
        {
            cout <<"fail to sort neuron (idx starts at 1):" << i+1 <<endl;
        }
        nt_list2[i].listNeuron=sort_result;
    }

    // save result 2
    for(int i=0; i<nt_list2.size();i++)
    {
        QList<NeuronSWC> neuron= nt_list2[i].listNeuron;
        QFileInfo fi = QFileInfo(swcList2[i]);
        QString file_name = fi.fileName();
        QString save_name = fileSaveFolder+ "/" + file_name+"_sort.swc";
        cout<<"file_name="<<file_name.toStdString()<<endl;
        //QString save_name = swcList.at(i).chop(4)+"_sort.swc";
        export_neuronList2file(neuron,save_name);
    }
    return true;
}
