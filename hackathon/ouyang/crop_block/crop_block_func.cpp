#include "crop_block_func.h"

#include "../../../../vaa3d_tools/hackathon/ouyang/ML_get_samples/tip_main.cpp"
#include "../../../../vaa3d_tools/hackathon/yimin/old_vr_codes/Vaa3DVR(ver 0.1)/vrminimal/matrix.h"
using namespace std;


void crop_bt_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_dir_axon=outfiles.at(1);//needed only when seperate axon and dendrite nodes
    QString output_apo;

    XYZ block_size=XYZ(100,100,100);

    int branch_or_tip_node=atoi(inparas.at(0));//1:branch;2:tip;3:both
    int tip_type=atoi(inparas.at(1));//1:seperate axon and dendrite;2:not seperate
    bool condition=false;
    if(branch_or_tip_node==3) condition=true;


    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_branch plugin\n");
    NeuronTree sorted_tree = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }
    if(!output_dir_axon.endsWith("/")){
        output_dir_axon = output_dir_axon+"/";
    }

    // Find branch and points
    QList<int> tip_point_list = get_tips(sorted_tree, false);
    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_point_list.size()<<endl;
    QList<int> branch_point_list = get_branch(sorted_tree);
    cout<<"=========================================================Number_of_branch_points\t"<<qPrintable(swc_file)<<"\t"<<branch_point_list.size()<<endl;
    QList<int> branch_and_tip_point_list;
    branch_and_tip_point_list.append(tip_point_list);
    branch_and_tip_point_list.append(branch_point_list);
    // Crop branch points-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    printf("welcome to use get_termial\n");
    if(branch_or_tip_node==1 || condition)
    {
        for(int i=0; i<branch_point_list.size(); i++){
            int tipnum=i;
            NeuronSWC node = sorted_tree.listNeuron.at(branch_point_list.at(i));
            if(node.type > 5) continue;

            block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
            crop_block.name = QString::number(i);
            XYZ tip=XYZ(node.x, node.y, node.z);
            // crop swc
            QString num_cnt=QString("%1").arg(i);
            QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);

            // create a tip-centered block
            if(tip_type == 2){  //get branch points from both dendrite and axon

                QString output_swc = output_dir+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 3){  //only get branch points from dendrite


                QString output_swc = output_dir+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 2){  //only get branch points from axon


                QString output_swc = output_dir_axon+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
        }
    }
    if(branch_or_tip_node==2 || condition)
    {
        for(int i=0; i<tip_point_list.size(); i++){
            int tipnum=i;
            NeuronSWC node = sorted_tree.listNeuron.at(tip_point_list.at(i));
            if(node.type > 5) continue;

            block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
            crop_block.name = QString::number(i);
            XYZ tip=XYZ(node.x, node.y, node.z);
            // crop swc
            QString num_cnt=QString("%1").arg(i);
            QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);

            // create a tip-centered block
            if(tip_type == 2){  //get tip points from both dendrite and axon

                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 3){  //only get tip points from dendrite


                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type == 1 && node.type == 2){  //only get tip points from axon


                QString output_swc = output_dir_axon+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
        }
    }
    return;
}

void crop_defined_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);//raw image
    QString swc_folder=infiles.at(1);//all swc of same brain folder
    QString original_swc_folder=infiles.at(2);//terafly swc for croppping swc block
    NeuronTree sorted_tree = readSWC_file(original_swc_folder);
    XYZ block_size=XYZ(50,50,50);//default 50x50x50
    int crop_all_in_input_swc=atoi(inparas.at(0));//1 for crop all swc nodes,0 is constrain
    if (inparas.size() >= 3 )
    {
        int tmp1=atoi(inparas.at(0));
        int tmp2=atoi(inparas.at(1));
        int tmp3=atoi(inparas.at(2));
        crop_all_in_input_swc=atoi(inparas.at(3));
        if (tmp1 != 0 && tmp2 != 0 && tmp3 != 0)
        {
            block_size=XYZ(tmp1,tmp2,tmp3);
            printf("block size is set to:[%dx%dx%d]",tmp1,tmp2,tmp3);
        }
        else
            v3d_msg("The parameter of block size is not valid, the program will use default value of [50x50x50]",0);
    }
    if(!swc_folder.endsWith("/")){
        swc_folder = swc_folder+"/";
    }


    QDir dir_tip(swc_folder);

    QStringList namelist=dir_tip.entryList();

    QStringList all_swc;
    for(int i=0;i<namelist.size();++i)
    {
        if(namelist.at(i).endsWith("swc")) all_swc.push_back(namelist.at(i));
        else if(namelist.at(i).endsWith("eswc")) all_swc.push_back(namelist.at(i));
    }

    QStringList list=all_swc.at(0).split("_");
    QString block_folder_name_a=list.at(0)+"_all_block_after_pruning"+"/";
    QString block_folder_name_b=list.at(0)+"_all_block_before_pruning"+"/";
    QString block_folder_name_refined=list.at(0)+"_all_block_refined"+"/";

    QString dir_output_block_a=swc_folder+block_folder_name_a;
    QString dir_output_block_b=swc_folder+block_folder_name_b;
    QString dir_output_block_refined=swc_folder+block_folder_name_refined;
//    QDir dir_block_a(dir_output_block_a);
//    if(!dir_block_a.exists() && crop_all_in_input_swc)
//    {
//        dir_block_a.mkdir(dir_output_block_a);
//    }
//    else
//    {
//        dir_block_a.setFilter(QDir::Files);
//        int fileCount = dir_block_a.count();
//        for (int i = 0; i < fileCount; i++)
//            dir_block_a.remove(dir_block_a[i]);
//    }

    if(crop_all_in_input_swc==1)
    {
        QDir dir_block_b(dir_output_block_b);
        if(!dir_block_b.exists() && crop_all_in_input_swc)
        {
            dir_block_b.mkdir(dir_output_block_b);
        }
        else
        {
            dir_block_b.setFilter(QDir::Files);
            int fileCount = dir_block_b.count();
            for (int i = 0; i < fileCount; i++)
                dir_block_b.remove(dir_block_b[i]);
        }
    }

    if(crop_all_in_input_swc==0)
    {
        QDir dir_block_refined(dir_output_block_refined);
        if(!dir_block_refined.exists())
        {
            dir_block_refined.mkdir(dir_output_block_refined);
        }
        else
        {
            dir_block_refined.setFilter(QDir::Files);
            int fileCount = dir_block_refined.count();
            for (int i = 0; i < fileCount; i++)
                dir_block_refined.remove(dir_block_refined[i]);
        }
    }
    // Crop branch points-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    printf("welcome to use get_termial\n");
    for(int i=0; i<all_swc.size(); i++)
    {
        cout<<"000000000000000000000000000000000000000"<<endl;
        QString flag1=all_swc.at(i).split(".")[0];
        NeuronTree pruned_swc=readSWC_file(swc_folder+all_swc.at(i));
        if (crop_all_in_input_swc){
            for(int j=0;j<pruned_swc.listNeuron.size();j++)
            {
                QString num_cnt=QString("%1").arg(j);
                NeuronSWC node = pruned_swc.listNeuron.at(j);
                QString r=QString("%1").arg(node.r);
                block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
                QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                crop_block.name = QString::number(i);

                QString output_image = flag1+"_r"+r+cordinates;
                //crop_img(image_file, crop_block, dir_output_block_b, callback,output_image);
                // crop swc
                QString output_swc = dir_output_block_b+output_image+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                // crop image
                {
                    //QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, dir_output_block_b, callback,output_image);
                }
            }
        }
        else {
            for(int j=0;j<pruned_swc.listNeuron.size();j++)
            {
                NeuronSWC node = pruned_swc.listNeuron.at(j);
                if(node.r>=8)
                {
                    QString num_cnt=QString("%1").arg(j);
                    QString r=QString("%1").arg(node.r);
                    block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
                    QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                    crop_block.name = QString::number(i);

                    QString output_image = flag1+"_r"+r+cordinates;
                    //crop_img(image_file, crop_block, dir_output_block_refined, callback,output_image);
                    // crop swc
                    QString output_swc = dir_output_block_refined+output_image+"_croped.eswc";
                    if(crop_swc_cuboid(pruned_swc, output_swc, crop_block))
                    // crop image
                    {
                        crop_img(image_file, crop_block, dir_output_block_refined, callback,output_image);
                    }



               }
            }
        }
    }
    return;
}



QList<int> get_branch(NeuronTree nt){
    // whether a node is a branch;
    QList<int> ori_branch_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)>1){ori_branch_list.append(i);}
    }

    cout<<"the number of branch nodes:"<<ori_branch_list.size()<<endl;

    return(ori_branch_list);
}

