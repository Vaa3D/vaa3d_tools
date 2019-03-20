#include "get_image_block_from_apo.h"

bool crop_by_apo(QString image_file, QString apo_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback)
{
    printf("welcome to use crop_by_apo\n");
    QList<CellAPO> apo_list = readAPO_file(apo_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }

    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("nrrd"));

    int ct=0;
    for(int i=0; i<apo_list.size(); i++){
//        if(ct>=10){break;}
        ct++;

        // create a tip-centered block
        XYZ shift;
        shift.x = (int)apo_list.at(i).x;
        shift.y = (int)apo_list.at(i).y;
        shift.z = (int)apo_list.at(i).z;

        block crop_block = offset_block(zcenter_block, shift);
        crop_block.name = QString::number(apo_list.at(i).n);
        // crop image
        qDebug()<<crop_block.name;
        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

        crop_img(image_file, crop_block, output_dir, callback, "."+output_suffix.at(0));
    }
}
