#include "auto_soma_correction.h"


bool auto_soma_correction_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //choose an swc files
    QString qs_file_swc = QFileDialog::getOpenFileName(
                            parent,
                            "Select a reconstruction (.swc/.eswc) file",
                            "",
                            "Annotation (*.swc *.eswc)");
    QString qs_file_apo = QFileDialog::getOpenFileName(
                            parent,
                            "Select a soma (.apo) file",
                            "",
                            "Annotation (*.apo)");
    QString qs_dir_output = QFileDialog::getExistingDirectory(parent,
                                                              QString(QObject::tr("Choose the output directory."))
                                                              );
    if((qs_dir_output.size()>0) && (!qs_dir_output.endsWith("/"))){qs_dir_output = qs_dir_output + "/";}

    return auto_soma_correction(qs_file_swc, qs_file_apo, qs_dir_output);
}

bool auto_soma_correction(QString qs_file_swc, QString qs_file_apo, QString qs_dir_output)
{
    // Step 0: initialization

    // Set input/output files and folders
    QString cur_dir = qs_file_swc.left(qs_file_swc.lastIndexOf("/")+1);
    if(qs_dir_output.size()==0){qs_dir_output = cur_dir + "Soma_Corrected/";}
    if(!QDir::current().mkdir(qs_dir_output)){
//        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(qs_dir_output).arg(qs_dir_output));
    }

    int suffix_len = 5;
    if(qs_file_swc.endsWith(".swc") || qs_file_swc.endsWith(".SWC")){suffix_len = 4;}  // swc file header
    QString prefix = qs_file_swc.left(qs_file_swc.size() - suffix_len);
    prefix = prefix.right(prefix.size() - prefix.lastIndexOf("/") - 1);

    // Set parameters
    double dist_thres = 8;  // Currently this is hard coded
    // Parameters for calling "preprocess".
    double prune_size = 0;
    double step_size = 0;
    double short_thres = 0;
    double soma_thres = dist_thres + 1;
    double long_thres = 0;


    // Step 1: remove nodes in the volume of soma.
    NeuronTree nt = readSWC_file(qs_file_swc);
    QList<CellAPO> soma_markers = readAPO_file(qs_file_apo);
    QList<NeuronSWC> soma_list = get_soma_from_APO(soma_markers);
    if(soma_list.isEmpty()){
        v3d_msg(QString("APO file is given but don't know which is soma.\n"
                        "Process terminated.\n"
                        "Please double check %1").arg(qs_file_apo));
        return 0;
    }
    NeuronSWC soma = soma_list.at(0);

    QList<int> del_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(computeDist2(nt.listNeuron.at(i), soma, XSCALE, YSCALE, ZSCALE) < dist_thres)
        {
            nt.listNeuron[i].level=7;
            del_list.append(i);
        }
    }
//    export_listNeuron_2eswc(nt.listNeuron, "Example/color_deleted_part.swc");
    nt = rm_nodes(nt, del_list);

    // Step 2: connect branching points to soma
    QString temp_dir = cur_dir + "temp/";
    QString input_swc = temp_dir + prefix + ".clear_soma_volume.swc";
    QString input_apo = temp_dir + prefix + ".clear_soma_volume.apo";
    if(!QDir::current().mkdir(temp_dir)){
//        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(temp_dir).arg(temp_dir));
    }
    // Save intermediate files;
    export_listNeuron_2eswc(nt.listNeuron, qPrintable(input_swc));
    QList<CellAPO> soma_apolist;
    CellAPO soma_apo;
    soma_apo.x = soma.x;
    soma_apo.y = soma.y;
    soma_apo.z = soma.z;
    soma_apo.volsize = 10;
    soma_apolist.append(soma_apo);
    writeAPO_file(input_apo, soma_apolist);

    QString output_swc = qs_dir_output + prefix + ".soma_corrected.eswc";
    pre_processing(input_swc, output_swc, prune_size, short_thres, long_thres, step_size, soma_thres);

    return 1;
}


bool auto_soma_correction_batch_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //choose a directory that contain swc files
    QString qs_dir_swc = QFileDialog::getExistingDirectory(parent,
                                                           QString(QObject::tr("Choose the directory that contains files to be soma-corrected"))
                                                           );
    if(qs_dir_swc.size()==0){
        v3d_msg("Empty input folder.\n Let the developer know if you see this message.");
        return 0;
    }
    if(!qs_dir_swc.endsWith("/")){qs_dir_swc = qs_dir_swc + "/";}
    QString qs_dir_output = QFileDialog::getExistingDirectory(parent,
                                                              QString(QObject::tr("Choose the output directory."))
                                                              );
    if((qs_dir_output.size()>0) && (!qs_dir_output.endsWith("/"))){qs_dir_output = qs_dir_output + "/";}

    return auto_soma_correction_files_in_dir(qs_dir_swc, qs_dir_output);
}

bool auto_soma_correction_files_in_dir(QString qs_dir_input, QString qs_dir_output)
{
    qDebug()<<"Welcome to use the \"batch auto_soma_correction\" plugin";
    // Step 1: Find swc files under a folder;
    QDir dir(qs_dir_input);
    QStringList qsl_filelist, qsl_filters;
    qsl_filters << "*.swc" << "*.eswc";

    foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
    {
        qsl_filelist+=file;
    }

    if(qsl_filelist.size()==0)
    {
        v3d_msg("Cannot find swc files in the given directory!\nTry another diretory");
        return 0;
    }

    // Step 2: Specify output folder
    if(qs_dir_output.size()==0){qs_dir_output = qs_dir_input + "Soma_Corrected/";}
    qDebug()<<QString("Output folder is %1.").arg(qs_dir_output);
    if(!QDir::current().mkdir(qs_dir_output)){
//        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(qs_dir_output).arg(qs_dir_output));
    }

    // Step 3: Perform auto_soma_correction through a loop
    for(int i=0; i<qsl_filelist.size(); i++){
        // input swc and apo
        int suffix_len = 5;
        if(qsl_filelist.at(i).endsWith(".swc") || qsl_filelist.at(i).endsWith(".SWC")){suffix_len = 4;}
        QString qs_file_swc = qs_dir_input + qsl_filelist.at(i);
        QString qs_file_apo = qs_dir_input + qsl_filelist.at(i).left(qsl_filelist.at(i).size()-suffix_len)+".apo";
        auto_soma_correction(qs_file_swc, qs_file_apo, qs_dir_output);
    }
    return 1;
}
