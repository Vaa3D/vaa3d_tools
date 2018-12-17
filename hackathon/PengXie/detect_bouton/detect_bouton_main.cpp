#include "detect_bouton_main.h"



void detect_bouton_pipeline(QString image, QString input_swc, QString output_dir, QString output_apo, V3DPluginCallback2 & callback)
{

    if(!output_dir.endsWith('/')){output_dir.append('/');}
    my_mkdir(output_dir);
    QList<block> block_list = get_blocks(input_swc);
    QList<CellAPO> apo_list;
    for(int i=0;i<block_list.size(); i++){
        block crop_block = block_list.at(i);
        QString output_dir_block = output_dir+crop_block.name+"/";
        my_mkdir(output_dir_block);
        // 1. Image
        QString output_dir_img = output_dir_block+"Image/";
        my_mkdir(output_dir_img);
        crop_img(image, crop_block, output_dir_img, callback, ".tiff"); // Will shift the image by crop_block.small
        // 2. SWC
        QString output_swc = output_dir_block+crop_block.name+".swc";
        crop_swc(input_swc, output_swc, crop_block);  // Will generate .swc file under output_dir/;
        // 3. Decompose SWC
        QString output_dir_swc = output_dir_block+"Traces/";
        my_mkdir(output_dir_swc);
        QString decompose_cmd = "vaa3d -x decompose_swc -f to_multiple_swc -i " +output_swc + " -o " +output_dir_swc;
        system(qPrintable(decompose_cmd));  // Generate cropped .swc files under output_dir/block_name/;
        // 4. run_matlab
        run_matlab(output_dir, crop_block.name); // Will generate .apo file under output_dir/block_name/;
        // 5. Re-shift .apo file
        QString cur_apo=output_dir+crop_block.name+"/"+crop_block.name+".apo";
        qDebug()<<cur_apo;
        apo_list.append(offset_apo(cur_apo, crop_block.small));
    }
    writeAPO_file(output_apo, apo_list);
    return;
}
