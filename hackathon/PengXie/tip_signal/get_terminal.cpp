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

    // Find soma
    int soma = get_soma(nt);

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

    // Save tips into an APO file
    QString output_apo = output_dir+cell_name+".apo";
    QList<CellAPO> apo_list;

    int ct=0;
    for(int i=0; i<tip_list.size(); i++){
//        if(ct>=10){break;}

        // Skip wired node types
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        qDebug()<<node.n;
        if(node.type > 5){continue;}

        // Skip short branches
        double tip_seg_len = tip_to_branch(nt, tip_list.at(i));
        qDebug()<<QString("Distance to branch point: %1").arg(tip_seg_len);
        if(tip_seg_len < 10){
            continue;
        }

        // Skip nodes that are very close to soma
        // To be implemented
        ct++;

        // create a tip-centered block
        XYZ shift;
        shift.x = (int)node.x;
        shift.y = (int)node.y;
        shift.z = (int)node.z;

        block crop_block = offset_block(zcenter_block, shift);
        crop_block.name = cell_name + "_"+QString::number(ct);
        // crop image
        qDebug()<<crop_block.name;
        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

        crop_img(image_file, crop_block, output_dir, callback, "."+output_suffix.at(0));
        // crop swc
        QString output_swc = output_dir+crop_block.name+".swc";
        crop_swc(swc_file, output_swc, crop_block);
        my_saveANO(output_dir, crop_block.name, output_suffix);

        // apo
        CellAPO cur_apo;
        cur_apo.x = node.x;
        cur_apo.y = node.y;
        cur_apo.z = node.z;
        cur_apo.n = ct;
        cur_apo.comment = crop_block.name;
        cur_apo.volsize = 10000;
        apo_list.append(cur_apo);


//        // Generate 2D image
//        QString ano_Name = output_dir+crop_block.name+QString(".ano");
//        qDebug()<<ano_Name;
//        P_ObjectFileType cc;
//        if(! loadAnoFile(ano_Name,cc)){
//            cout <<"Fail to load ano file" <<endl;
//            return;
//        }
//        QString raw_img = cc.raw_image_file_list.at(0);
//        QString swc = cc.swc_file_list.at(0);
//        QString img_2d = output_dir + crop_block.name + ".tif";
//        V3DLONG in_sz[4];
//        generate_2d_img(callback, raw_img, swc, in_sz, img_2d);

    }
    writeAPO_file(output_apo, apo_list);
    writeSWC_file(output_dir+cell_name+".swc", nt);
    output_suffix.clear();
    output_suffix.append(QString("swc"));
    output_suffix.append(QString("apo"));
    my_saveANO(output_dir, cell_name, output_suffix);
    return;
}
