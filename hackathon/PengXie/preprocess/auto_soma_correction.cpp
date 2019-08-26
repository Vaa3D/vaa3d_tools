#include "auto_soma_correction.h"

double xs = 0.2; // x-scale
double ys = 0.2; // y-scale
double zs = 1; // z-scale

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
//    QString qs_file_swc="/home/pengx/Desktop/soma_check/test.eswc";
//    QString qs_file_apo="/home/pengx/Desktop/soma_check/test.apo";
//    QString qs_dir_output="/home/pengx/Desktop/soma_check/Soma_Corrected";
    if((qs_dir_output.size()>0) && (!qs_dir_output.endsWith("/"))){qs_dir_output = qs_dir_output + "/";}

    return auto_soma_correction(qs_file_swc, qs_file_apo, qs_dir_output);
}
bool auto_soma_correction_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) {
        infiles = *((vector<char*> *)input.at(0).p);
        qDebug()<<input.size()<<infiles.size();
    }
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString qs_file_swc = infiles.at(0);
    QString qs_file_apo = infiles.at(1);
    QString qs_dir_output = outfiles.at(0);

    return auto_soma_correction(qs_file_swc, qs_file_apo, qs_dir_output);
}
bool auto_soma_correction(QString qs_file_swc, QString qs_file_apo, QString qs_dir_output)
{
    bool skip_existing = true;
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
    QString output_swc = qs_dir_output + prefix + ".soma_reassigned.eswc";
    if((fexists(output_swc)) && (skip_existing)){
        qDebug()<<output_swc;
        return 1;
    }

    // Set parameters
    double dist_thres = 50;  // Currently this is hard coded
    double candidate_radius = 50;
    // Parameters for calling "preprocess".
    double prune_size = 0;
    double step_size = 0;
    double short_thres = 0;
    double soma_thres = dist_thres + 1;
    double long_thres = 0;

    NeuronTree nt = readSWC_file(qs_file_swc);
    NeuronTree nt_sorted;
    QList<CellAPO> soma_markers = readAPO_file(qs_file_apo);
    QList<NeuronSWC> soma_list = get_soma_from_APO(soma_markers);
    if(soma_list.isEmpty()){
        qDebug()<<(QString("APO file is given but don't know which is soma.\n"
                        "Process terminated.\n"
                        "Please double check %1").arg(qs_file_apo));
        return 0;
    }
    NeuronSWC soma = soma_list.at(0);

    bool whether_crop = false;
    if(whether_crop){
        // For testing purpose only
        // Crop the swc file
        // BEGIN
        qDebug()<<"Begin cropping";

        // Finding center for cropping
        double min_dist = 1000;
        int center_idx = 0;
        for(int i=0; i<nt.listNeuron.size(); i++){
            double cur_dist = computeDist2(nt.listNeuron.at(i), soma, XSCALE, YSCALE, ZSCALE);
            if(cur_dist < min_dist)
            {
                min_dist = cur_dist;
                center_idx = i;
            }
        }
        QString crop_dir = cur_dir + "crop/";
        QString cropped_swc = crop_dir + prefix + ".crop.swc";
        QString cropped_apo = crop_dir + prefix + ".crop.apo";
        if(!QDir::current().mkdir(crop_dir)){
    //        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(temp_dir).arg(temp_dir));
        }

        crop_swc(qs_file_swc, cropped_swc, 1000, center_idx, 0, -1, 0, 0, 0, 0, 0);
        QList<CellAPO> cropped_apolist;
        CellAPO tp_apo;
        tp_apo.x = soma.x;
        tp_apo.y = soma.y;
        tp_apo.z = soma.z;
        tp_apo.volsize = 10;
        cropped_apolist.append(tp_apo);
        writeAPO_file(cropped_apo, cropped_apolist);

        nt = readSWC_file(cropped_swc);
        soma_markers = readAPO_file(cropped_apo);
        soma_list = get_soma_from_APO(soma_markers);
        if(soma_list.isEmpty()){
            qDebug()<<(QString("APO file is given but don't know which is soma.\n"
                            "Process terminated.\n"
                            "Please double check %1").arg(cropped_apo));
            return 0;
        }
        soma = soma_list.at(0);
        // END
    }
    else{
        nt = my_SortSWC(nt, nt.listNeuron.at(0).n, 0); // Sorting is necessary for find_soma_by_apo function, otherwise node_degree calculation is inaccurate.
    }

    // Step 1: can we find the unique soma candidate in swc?

    // ------------- Case 1: Soma can be found in the swc ---------------
    qDebug()<<"Begin finding soma candidate";
    V3DLONG soma_id = find_soma_by_apo(nt, soma, candidate_radius);
//    v3d_msg(QString("%1\nsoma_id%2").arg(qs_file_swc).arg(soma_id));
    if(soma_id>=0){
        nt.listNeuron[soma_id].type=1;
        nt_sorted = my_SortSWC(nt, nt.listNeuron.at(soma_id).n, 0);
        output_swc = qs_dir_output + prefix + ".soma_reassigned.eswc";
        export_listNeuron_2eswc(nt_sorted.listNeuron, qPrintable(output_swc));
        return 1;
    }

    qDebug()<<QString("%1\n"
                      "No soma candidate found in swc.\n"
                      "Will perform automatic soma correction.").arg(qs_file_swc);
    return 0;


    // ------------- Case 2: Soma can not be found in the swc ---------------
    // Step 1: remove nodes in the volume of soma.
    QList<int> del_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
//        if(computeDist2(nt.listNeuron.at(i), soma, xs, ys, zs) < dist_thres)
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

    // Need optimization, just connect to soma will be fine.
    output_swc = qs_dir_output + prefix + ".soma_replaced.eswc";
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

bool auto_soma_correction_batch_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) {
        infiles = *((vector<char*> *)input.at(0).p);
        qDebug()<<input.size()<<infiles.size();
    }
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString qs_dir_input = infiles.at(0);
    QString qs_dir_output = outfiles.at(0);

    return auto_soma_correction_files_in_dir(qs_dir_input, qs_dir_output);
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
        qDebug()<<qs_file_swc;
        auto_soma_correction(qs_file_swc, qs_file_apo, qs_dir_output);
    }
    return 1;
}

QList<int> node_degree(QList<NeuronSWC> nlist){
    QList<int> degree;
    QList<int> name_list;
    for(int i=0; i<nlist.size(); i++){
        degree.append(0);
        name_list.append(nlist.at(i).n);
    }
    for(int i=0; i<nlist.size(); i++){
        int parent = nlist.at(i).pn;
        if(parent == -1){continue;}
        degree[i] += 1;
        degree[name_list.indexOf(parent)] += 1;
    }
    return degree;
}

V3DLONG find_soma_by_apo(NeuronTree nt, NeuronSWC soma, double detection_radius){

    int candidate;
    int max_score = 0;
    int score_thres = 5;
    QList<int> degree=node_degree(nt.listNeuron);
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(computeDist2(nt.listNeuron.at(i), soma, XSCALE, YSCALE, ZSCALE) < detection_radius)
        {
            // Found candidate within certain distance
            int score = degree.at(i);
            if(nt.listNeuron.at(i).type==1){score += 3;}
            if(score > max_score){
                candidate = i;
                max_score = score;
            }
        }
    }
    if(max_score>=score_thres){
        return candidate;
    }
    else{
        return -1;
    }
}
