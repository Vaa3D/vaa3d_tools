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

    XYZ block_size=XYZ(50,50,50);

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
            else if(tip_type = 1 && node.type == 3){  //only get branch points from dendrite


                QString output_swc = output_dir+"branch_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "branch_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type = 1 && node.type == 2){  //only get branch points from axon


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
            if(tip_type == 2){  //get branch points from both dendrite and axon

                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type = 1 && node.type == 3){  //only get branch points from dendrite


                QString output_swc = output_dir+"tip_"+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                    // crop image
                {
                    QString output_image = "tip_"+flag1+"_"+num_cnt+cordinates;
                    crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
                }
            }
            else if(tip_type = 1 && node.type == 2){  //only get branch points from axon


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

