/* batch_main.cpp
 * This is batch program for applying blastneuron_bjut
 * 2017-03-01 : by Yishan He
 */
#include "batch_main.h"
#include "overlap/overlap_gold.h"
#include"resampling.h"
#include"pre-process.h"
#include"local_alignment.h"
#include"prune_alignment.h"

using namespace std;

bool GreaterSort (QList<NeuronSWC> a, QList<NeuronSWC> b) { return (a.size()>b.size()); }
char*num2str(int i)
{
    char* str = new char[5];
    char buf[10];
    string num;
    sprintf(buf, "%d", i);
    num = buf;
    strcpy(str,num.c_str());
    return str;
}
//char*  num2str(double i)
//{
//    char* str = new char[5];
//    char buf[10];
//    string num;
//    sprintf(buf, "%d", i);
//    num = buf;
//    strcpy(str,num.c_str());
//    return str;
//}

bool batch_main(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    // parameters
    int resample_step=3;
    double prune_alignment_thres=20;
    int prune_tree_thres=3;
    int file_num=10;
    int method;
    int dist_para;
    int meth_para;

    // load data
    char *neuron_folder_path;
    QString raw_img;
    QString consensus_file;
    QString result_folder;
    QString result_temp;
    QString v3d;

    if(inlist->size()==4)
    {
        v3d = inlist->at(0);
        neuron_folder_path = inlist->at(1);
        raw_img = inlist->at(2);
        consensus_file=inlist->at(3);
    }
    else {printf("Wrong input in batch_main.\n");return false;}

    paralist=(vector<char*>*)(input.at(1).p);

    if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        result_folder = QString(outlist->at(0));
        result_temp=result_folder+"/tempfile.swc";

    }
    else{printf("Please specify one output directory.\n");return false;}
    QString select_folder=result_folder+"/select";

    // read files from a folder
    QStringList swcList = importFileList_addnumbersort(QString(neuron_folder_path));
    vector<NeuronTree> nt_list;
    for(V3DLONG i = 2; i < swcList.size(); i++)     // the first two are not files
    {
        QString curPathSWC = swcList.at(i);
        NeuronTree temp = readSWC_file(curPathSWC);
        nt_list.push_back(temp);
    }

    // conver eswc to swc   -- the function will generate a swc file beside the eswc file
    if (consensus_file.toUpper().endsWith(".ESWC"))
    {
        V3DPluginArgItem arg;
        V3DPluginArgList input_eswc_converter;
        V3DPluginArgList output_eswc_converter;

        arg.type="random";vector<char*> arg_input_eswc_converter;
        string fileName_Qstring(consensus_file.toStdString());char* fileName_string = new char[fileName_Qstring.length() +1]; strcpy(fileName_string, fileName_Qstring.c_str());
        arg_input_eswc_converter.push_back(fileName_string);
        arg.p = (void*) & arg_input_eswc_converter; input_eswc_converter<< arg;
        QString tmpname = consensus_file;tmpname.chop(4);QString swcname=tmpname+"swc";
        string swcname_Qstring(swcname.toStdString());char * swcname_string= new char[swcname_Qstring.length() +1]; strcpy(swcname_string, swcname_Qstring.c_str());
        arg.type="random";vector<char*>arg_output_eswc_converter;arg_output_eswc_converter.push_back(swcname_string);arg.p=(void *) & arg_output_eswc_converter; output_eswc_converter<<arg;

        QString plugin_name_eswc_converter = "eswc_converter";
        QString func_name_eswc_converter= "eswc_to_swc";
        callback.callPluginFunc(plugin_name_eswc_converter,func_name_eswc_converter,input_eswc_converter,output_eswc_converter);
        consensus_file=swcname;
    }
    NeuronTree nt_consensus = readSWC_file(consensus_file);
    vector<MyMarker*> mm_consensus = readSWC_file(consensus_file.toStdString());

    // resample and sort
    cout<<"pre-process begin"<<endl;
    for(int i=0; i<nt_list.size();i++)
    {
        NeuronTree resample_result;
        resample_result=resample(nt_list[i],resample_step);
        resample_result.file = nt_list[i].file;
        nt_list[i].copy(resample_result);
    }
    for(int i=0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> sort_result;
        if(!sort_with_standard(nt_list[i].listNeuron, nt_consensus.listNeuron,sort_result))
        {
            cout <<"fail to sort neuron (idx starts at 1):" << i+1 <<endl;
        }
        nt_list[i].listNeuron=sort_result;
    }
    cout<<"pre-process done"<<endl;

    // new local alignment
    vector<QList<NeuronSWC> > p_align_list;
    for(int i=0; i<nt_list.size();i++)
    {
        QList<NeuronSWC> neuron=nt_list[i].listNeuron;
        char buf[10];
        string num_i;
        sprintf(buf, "%d", i+1);
        num_i = buf;
        QString sort_out = result_folder + "/sort_" + QString::fromStdString(num_i) + ".swc";
        export_neuronList2file(neuron,sort_out);

        QString alignment_result;
        alignment_result = result_folder + "/alignment_" + QString::fromStdString(num_i) + ".swc";

        #if  defined(Q_OS_LINUX)
            QString cmd_LA = QString("%1/vaa3d -x blastneuron_bjut -f apply_blastneuron -i %2 %3 -o %4").arg(v3d.toStdString().c_str()).arg(sort_out.toStdString().c_str()).arg(consensus_file.toStdString().c_str()).arg(alignment_result.toStdString().c_str());
            system(qPrintable(cmd_LA));
        #elif defined(Q_OS_MAC)
            QString cmd_LA = QString("%1/vaa3d64.app/Contents/MacOS/vaa3d64 --x blastneuron_bjut -f apply_blastneuron -i %2 %3 -o %4").arg(getAppPath().toStdString().c_str()).arg(sort_out.toStdString().c_str()).arg(consensus_file.toStdString().c_str()).arg(alignment_result.toStdString().c_str());
            system(qPrintable(cmd_LA));
        #else
            v3d_msg("The OS is not Linux or Mac. Do nothing.");
            return;
        #endif
       QFile::remove(sort_out);

       // prune alignment
       QFile file(alignment_result);
       if(file.exists())
       {
           NeuronTree align_temp = readSWC_file(alignment_result);
           if(align_temp.listNeuron.size()!=0)
           {
               QList<NeuronSWC> pruned_alignment=prune_long_alignment(align_temp.listNeuron,prune_alignment_thres);
               p_align_list.push_back(pruned_alignment);
           }
           QFile::remove(alignment_result);
       }
    }


    // old local alignment and prune alignment
    int BLOCK=1;
    if(BLOCK == 0)
    {
    vector<vector<MyMarker*> > mm_list;
    vector<vector<MyMarker*> > alignment_list;
    for(int i =0; i<nt_list.size();i++)
    {
        vector<MyMarker*> mm_temp = nt2mm(nt_list[i].listNeuron,result_temp);
        mm_list.push_back(mm_temp);
    }
    vector<NeuronTree>(nt_list).swap(nt_list);  // release Mermory

    for(int i = 0; i<mm_list.size();i++)
    {
        int jump=0;
        vector<map<MyMarker*, MyMarker*> > map_result;
        if(!neuron_mapping_dynamic(mm_list[i],mm_consensus,map_result))
        {
            cout<<"error in neuron_mapping"<<endl;
            jump=1;
        }
        if(jump==0)
        {
        vector<MyMarker*> map_swc;
        convert_matchmap_2swc(map_result, map_swc);
        alignment_list.push_back(map_swc);

        char buf[10];
        string num_i;
        sprintf(buf, "%d", i);
        num_i = buf;
        string name_result=result_temp.toStdString()+num_i+".swc";
        saveSWC_file(name_result, map_swc);

        for (int j=0;j<map_swc.size();j++)
            if (map_swc[j]) {delete(map_swc[j]); map_swc[j]=NULL;}
        }
        for (int j=0;j<mm_list[i].size();j++)
            if (mm_list[i][j]) {delete(mm_list[i][j]); mm_list[i][j]=NULL;}

    }
    saveSWC_file(result_temp.toStdString(), mm_consensus);
    cout<<"local_alginment done"<<endl;

    // prune tree
    vector<QList<NeuronSWC> > p_align_list;
    cout<<"alignment_list size="<<alignment_list.size()<<endl;
    for (int i =0; i<alignment_list.size();i++)
    {
        cout<<i<<endl;
        NeuronTree nt_alignment= mm2nt(alignment_list[i],result_temp);
        cout<<i<<endl;
        QList<NeuronSWC> pruned_alignment=prune_long_alignment(nt_alignment.listNeuron,prune_alignment_thres);
        p_align_list.push_back(pruned_alignment);
//        cout<<i<<endl;
//        for (int j=0;j<alignment_list[i].size();j++)
//        {cout<<j<<endl;
//            if (alignment_list[i][j]) {delete(alignment_list[i][j]); alignment_list[i][j]=NULL;}}
    }
    }

    // sort and pick out file_num largest alignments.
    sort(p_align_list.begin(),p_align_list.end(),GreaterSort);
    if(p_align_list.size()<=file_num) file_num=p_align_list.size();
    for (int i=0;i<file_num;i++)
    {
        char buf[10];
        string num_i;
        sprintf(buf, "%d", i+1);
        num_i = buf;
        QString p_align_out = select_folder + "/select_" + QString::fromStdString(num_i) + ".swc";
        export_neuronList2file(p_align_list[i],p_align_out);
    }
    // overlap with consensus
    {
        method = 2;

        for(dist_para=3;dist_para<6;dist_para+=2)
        {
            for(meth_para=4;meth_para<10;meth_para+=2)
            {
                V3DPluginArgItem arg;
                V3DPluginArgList input_overlap;
                V3DPluginArgList output_overlap;
                cout<<"method="<<method<<"    dist_para="<<dist_para<<"   meth_para="<<meth_para<<endl;
                arg.type="random";vector<char*> arg_input_overlap;
                string folderName_Qstring(select_folder.toStdString());char* folderName_string = new char[folderName_Qstring.length() +1]; strcpy(folderName_string, folderName_Qstring.c_str());
                arg_input_overlap.push_back(folderName_string);
                string inimg_Qsting(raw_img.toStdString());char* inimg_string = new char[inimg_Qsting.length() +1]; strcpy(inimg_string, inimg_Qsting.c_str());
                arg_input_overlap.push_back(inimg_string);
                arg.p = (void*) & arg_input_overlap; input_overlap<< arg;

                char* method_string=num2str(method);char* dist_para_string=num2str(dist_para);char* meth_para_string=num2str(meth_para);char* prune_tree_thres_string=num2str(prune_tree_thres);
                arg.type="random";vector<char*> arg_para_overlap;arg_para_overlap.push_back(method_string);arg_para_overlap.push_back(dist_para_string);arg_para_overlap.push_back(meth_para_string);arg_para_overlap.push_back(prune_tree_thres_string);
                arg.p = (void*) & arg_para_overlap; input_overlap << arg;

                QString savename = result_folder; savename += "/consensus_"+QString::fromStdString(dist_para_string)+"_"+QString::fromStdString(meth_para_string)+".swc";
                string savename_Qstring(savename.toStdString());char * savename_string= new char[savename_Qstring.length() +1]; strcpy(savename_string, savename_Qstring.c_str());
                arg.type="random";vector<char*>arg_output_overlap;arg_output_overlap.push_back(savename_string);arg.p=(void *) & arg_output_overlap; output_overlap<<arg;

                QString plugin_blastneuron_bjut="blastneuron_bjut";
                QString func_name_overlap="overlap_gold";
                callback.callPluginFunc(plugin_blastneuron_bjut,func_name_overlap,input_overlap,output_overlap);
            }
        }
    }
    {
        method = 3;
        dist_para=0;
        for(meth_para=4;meth_para<10;meth_para+=2)
        {
            V3DPluginArgItem arg;
            V3DPluginArgList input_overlap;
            V3DPluginArgList output_overlap;
            cout<<"method="<<method<<"    dist_para="<<dist_para<<"   meth_para="<<meth_para<<endl;
            arg.type="random";vector<char*> arg_input_overlap;
            string folderName_Qstring(select_folder.toStdString());char* folderName_string = new char[folderName_Qstring.length() +1]; strcpy(folderName_string, folderName_Qstring.c_str());
            arg_input_overlap.push_back(folderName_string);
            string inimg_Qsting(raw_img.toStdString());char* inimg_string = new char[inimg_Qsting.length() +1]; strcpy(inimg_string, inimg_Qsting.c_str());
            arg_input_overlap.push_back(inimg_string);
            arg.p = (void*) & arg_input_overlap; input_overlap<< arg;

            char* method_string=num2str(method);char* dist_para_string=num2str(dist_para);char* meth_para_string=num2str(meth_para);char* prune_tree_thres_string=num2str(prune_tree_thres);
            arg.type="random";vector<char*> arg_para_overlap;arg_para_overlap.push_back(method_string);arg_para_overlap.push_back(dist_para_string);arg_para_overlap.push_back(meth_para_string);arg_para_overlap.push_back(prune_tree_thres_string);
            arg.p = (void*) & arg_para_overlap; input_overlap << arg;

            QString savename = result_folder; savename += "/consensus_mean_"+QString::fromStdString(dist_para_string)+"_"+QString::fromStdString(meth_para_string)+".swc";
            string savename_Qstring(savename.toStdString());char * savename_string= new char[savename_Qstring.length() +1]; strcpy(savename_string, savename_Qstring.c_str());
            arg.type="random";vector<char*>arg_output_overlap;arg_output_overlap.push_back(savename_string);arg.p=(void *) & arg_output_overlap; output_overlap<<arg;

            QString plugin_blastneuron_bjut="blastneuron_bjut";
            QString func_name_overlap="overlap_gold";
            callback.callPluginFunc(plugin_blastneuron_bjut,func_name_overlap,input_overlap,output_overlap);
        }
    }
    return true;
}

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}
