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

    return auto_soma_correction(qs_file_swc, qs_file_apo);
}

bool auto_soma_correction(QString qs_file_swc, QString qs_file_apo)
{
    int suffix_len = 5;
    if(qs_file_swc.endsWith(".swc") || qs_file_swc.endsWith(".SWC")){suffix_len = 4;}
    QString prefix = qs_file_swc.left(qs_file_swc.size() - suffix_len);
    prefix = prefix.right(prefix.size() - prefix.lastIndexOf("/") - 1);
    double dist_thres = 8;

    // Step 1: remove nodes in the volume of soma.
    NeuronTree nt = readSWC_file(qs_file_swc);
    QList<CellAPO> soma_markers = readAPO_file(qs_file_apo);
    NeuronSWC soma = get_soma_from_APO(soma_markers).at(0);
    QList<int> del_list;
    for(int i=0; i<nt.listNeuron.size(); i++){
        if(computeDist2(nt.listNeuron.at(i), soma, XSCALE, YSCALE, ZSCALE) < dist_thres)
        {
            nt.listNeuron[i].level=7;
            del_list.append(i);
        }
    }
    export_listNeuron_2eswc(nt.listNeuron, "Example/color_deleted_part.swc");
    nt = rm_nodes(nt, del_list);

    // Step 2: connect branching points to soma
    QString temp_dir = "temp/";
    QString input_swc = QDir::currentPath() + "/" + temp_dir + prefix + ".clear_soma_volume.swc";
    QString input_apo = QDir::currentPath() + "/" + temp_dir + prefix + ".clear_soma_volume.apo";
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

    QString output_swc = QDir::currentPath() + "/" + prefix + ".soma_corrected.eswc";
    pre_processing(input_swc, output_swc, 0, 0.5, 10, 0, dist_thres+1);

    return 1;
}
