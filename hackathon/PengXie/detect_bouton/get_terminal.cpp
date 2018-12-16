#include "get_terminal.h"

void get_terminal(QString image_file, QString swc_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback)
{
    printf("welcome to use get_termial\n");
    NeuronTree nt = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }
    QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
    cell_name = cell_name.left(cell_name.indexOf("."));

    // Find tips
    QList<int> tip_list = get_tips(nt, false);
    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;

    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("nrrd"));
    output_suffix.append(QString("swc"));

    for(int i=0; i<tip_list.size(); i++){
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        qDebug()<<node.n;
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = cell_name + "_"+QString::number(i);
        // crop image
        qDebug()<<crop_block.name;
        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
        // crop swc
        QString output_swc = output_dir+crop_block.name+".swc";
        crop_swc(swc_file, output_swc, crop_block);
//        my_saveANO(output_dir, crop_block.name, output_suffix);
    }
    return;
}
