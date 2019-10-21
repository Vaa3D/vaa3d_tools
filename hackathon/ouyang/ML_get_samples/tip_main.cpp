
#include "tip_main.h"
#include <stdio.h>
#include <iostream>
#include "qlist.h"
#include <algorithm>
#include <iterator>
#include <cmath>
#include "APP1_pruning.h"
#include "../../../../vaa3d_tools/hackathon/yimin/old vr codes/Vaa3DVR(ver 0.1)/vrminimal/matrix.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#include "../../../../vaa3d_tools/hackathon/PengXie/preprocess/sort_swc_redefined.cpp"
#include "../../../../vaa3d_tools/hackathon/LXF/blastneuron_bjut/my_surf_objs.cpp"
#include "../../../../vaa3d_tools/v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_upSample3D.h"
#include "../../../../v3d_external/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/pca1.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp"
//#include "/home/braincenter4/v3d_external/released_plugins_more/v3d_plugins/multiscaleEnhancement/MultisclaeEnhancement_plugin.cpp"
#include "tiffio.h"
//#include "../../../released_plugins/v3d_plugins/cellseg_gvf/src/FL_downSample3D.h"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define MIN_DIST 2
#define VOID 1000000000
using namespace std;

double marker_dist(MyMarker a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

MyMarker over_traced_node(QPair< MyMarker,MyMarker> input_xyz,double over_dist){

    MyMarker output_overtraced_node;
    double dist_input=marker_dist(input_xyz.first,input_xyz.second);
    double fabs_value_x=fabs(double (input_xyz.first.x-input_xyz.second.x));
    double fabs_value_y=fabs(double (input_xyz.first.y-input_xyz.second.y));
    double fabs_value_z=fabs(double (input_xyz.first.z-input_xyz.second.z));
    double output_add_x1=input_xyz.first.x+(over_dist*fabs_value_x)/dist_input;
    double output_add_y1=input_xyz.first.y+(over_dist*fabs_value_y)/dist_input;
    double output_add_z1=input_xyz.first.z+(over_dist*fabs_value_z)/dist_input;
    double output_minus_x2=input_xyz.first.x-(over_dist*fabs_value_x)/dist_input;
    double output_minus_y2=input_xyz.first.y-(over_dist*fabs_value_y)/dist_input;
    double output_minus_z2=input_xyz.first.z-(over_dist*fabs_value_z)/dist_input;
    MyMarker direc_ori_12=MyMarker(input_xyz.first.x-input_xyz.second.x , input_xyz.first.y-input_xyz.second.y , input_xyz.first.z-input_xyz.second.z);
    MyMarker direc_after_32_add=MyMarker(output_add_x1-input_xyz.second.x , output_add_y1-input_xyz.second.y , output_add_z1-input_xyz.second.z);
    MyMarker direc_after_32_minus=MyMarker(output_minus_x2-input_xyz.second.x , output_minus_y2-input_xyz.second.y , output_minus_z2-input_xyz.second.z);
    double vector_product_add=direc_ori_12.x*direc_after_32_add.x+direc_ori_12.y*direc_after_32_add.y+direc_ori_12.z*direc_after_32_add.z;
    double vector_product_minus=direc_ori_12.x*direc_after_32_minus.x+direc_ori_12.y*direc_after_32_minus.y+direc_ori_12.z*direc_after_32_minus.z;
    if (vector_product_add>0) output_overtraced_node=MyMarker(output_add_x1,output_add_y1,output_add_z1);
    else if (vector_product_minus>0) output_overtraced_node=MyMarker(output_minus_x2,output_minus_y2,output_minus_z2);
    return output_overtraced_node;
}

MyMarker off_signal_node(QPair< MyMarker,MyMarker> input_xyz,int over_dist){

    MyMarker over_node;
    double dis_ori_over=0;
    double vector_product=0;
    while((over_dist*over_dist > dis_ori_over) || (dis_ori_over > (over_dist+1)*(over_dist+1))){  //make new node in the circle of 5 pixels radius to 6 pixels radius.

            over_node.x=random()%(2*over_dist)+int(input_xyz.first.x-over_dist);
            over_node.y=random()%(2*over_dist)+int(input_xyz.first.y-over_dist);
            over_node.z=random()%(2*over_dist)+int(input_xyz.first.z-over_dist);//make distance adjustable and random

            MyMarker direc_ori_12=MyMarker(input_xyz.second.x -input_xyz.first.x, input_xyz.second.y -input_xyz.first.y , input_xyz.second.z -input_xyz.first.z);
            MyMarker direc_ori_new=MyMarker(over_node.x-input_xyz.first.x , over_node.y-input_xyz.first.y , over_node.z-input_xyz.first.z);
            //vector_product=direc_ori_12.x*direc_after.x+direc_ori_12.y*direc_after.y+direc_ori_12.z*direc_after.z;
            double add_two_direction=fabs(double(direc_ori_12.x*direc_ori_new.x+direc_ori_12.y*direc_ori_new.y+direc_ori_12.z*direc_ori_new.z));
            double ori_ori_length=sqrt(direc_ori_12.x*direc_ori_12.x+direc_ori_12.y*direc_ori_12.y+direc_ori_12.z*direc_ori_12.z);
            double ori_new_length=sqrt(direc_ori_new.x*direc_ori_new.x+direc_ori_new.y*direc_ori_new.y+direc_ori_new.z*direc_ori_new.z);
            double add_length=ori_ori_length*ori_new_length;
            double cos_angle=add_two_direction / add_length;
            if(cos_angle > (sqrt(3) / 2)) continue;//the angle which is less than 60 will not be chosen.

            dis_ori_over=(input_xyz.first.x-over_node.x)*(input_xyz.first.y-over_node.y)+(input_xyz.first.y-over_node.y)*(input_xyz.first.y-over_node.y)
                    +(input_xyz.first.z-over_node.z)*(input_xyz.first.z-over_node.z);
    }
    return over_node;
}


void get_undertraced_sample(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir_den=outfiles.at(0);
    QString output_dir_axon=outfiles.at(1);
    QString output_apo;

    double maximum = (inparas.size() >=1) ? atoi(inparas[0]) : 30;
    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    XYZ block_size=XYZ(50,50,50);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree nt1 = readSWC_file(swc_file);
    if(!output_dir_den.endsWith("/")){
        output_dir_den = output_dir_den+"/";
    }

    if(!output_dir_axon.endsWith("/")){
        output_dir_axon = output_dir_axon+"/";
    }

//    QList<NeuronSWC> sort_swc;
      //SortSWC(nt2.listNeuron, sort_swc ,VOID, 0);
//    NeuronTree nt1;
//    QHash <int, int> hash_nt;

//    for(V3DLONG j=0; j<sort_swc.size();j++){
//        hash_nt.insert(sort_swc[j].n, j);
//    }
//    nt1.listNeuron=sort_swc;
//    nt1.hashNeuron=hash_nt;

    // Find tips
    QList<int> ori_tip_list = get_tips(nt1, false);
    cout<<"Number_of_tips:\t"<<qPrintable(swc_file)<<"\t"<<ori_tip_list.size()<<endl;
    int numtip=ori_tip_list.size();

    //get new tips after deleting nodes
    NeuronTree nt;
    nt=get_unfinished_fun(ori_tip_list,nt1,maximum);  // this fuction is to delete nodes beginning with tip nodes.NOTE:the deleted distance is adjustable and random by given range:10um to <maximum> um
    QList<int> tip_list = get_tips(nt, false);
    cout<<"Number_of_tips after deleting nodes:\t"<<tip_list.size()<<endl;

    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("tif"));
    output_suffix.append(QString("swc"));
    printf("welcome to use get_termial\n");

    int tipnum_den=1,tipnum_axon=1;
    for(int i=0; i<tip_list.size(); i++)
    {
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);

        if(nt.listNeuron.at(tip_list.at(i)).type == 3)
        {
            // crop swc
            QString num_cnt_den=QString("%1").arg(tipnum_den);
            QString output_swc_den = output_dir_den+flag1+"_"+num_cnt_den+".eswc";
            if(crop_swc_cuboid(nt, output_swc_den, crop_block))
            // crop image
            {
               QString output_image_den = flag1+"_"+num_cnt_den;
               crop_img(image_file, crop_block, output_dir_den, callback, output_image_den, output_swc_den,tipnum_den,tip,false);
            }
            tipnum_den ++;
            //my_saveANO(output_dir, crop_block.name, output_suffix);
        }
//        else if(nt.listNeuron.at(tip_list.at(i)).type == 2)//axon
//        {
//            // crop swc
//            QString num_cnt_axon=QString("%1").arg(tipnum_axon);
//            QString output_swc = output_dir_axon+flag1+"_"+num_cnt_axon+".eswc";
//            if(crop_swc_cuboid(nt, output_swc, crop_block))
//                // crop image
//            {
//                QString output_image = flag1+"_"+num_cnt_axon;
//                crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum_axon,tip,false);
//            }
//            tipnum_axon ++;
//            //my_saveANO(output_dir, crop_block.name, output_suffix);
//        }
    }
    return;
    //QString output_newswc = output_dir+"deleted"+".eswc";
    //export_list2file(nt.listNeuron,output_newswc);
}

void get_overtraced_sample(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir_den=outfiles.at(0);
    QString output_dir_axon=outfiles.at(1);
    QString output_apo;

    if(!output_dir_den.endsWith("/")){
        output_dir_den = output_dir_den+"/";
    }

    if(!output_dir_axon.endsWith("/")){
        output_dir_axon = output_dir_axon+"/";
    }

    double maximum = (inparas.size() >=1) ? atoi(inparas[0]) : 30;
    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    XYZ block_size=XYZ(50,50,50);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree sorted_tree = readSWC_file(swc_file);

//    QList<NeuronSWC> sort_swc;
//    SortSWC(nt1.listNeuron, sort_swc ,VOID, 0);
//    NeuronTree sorted_tree;
//    QHash <int, int> hash_nt;

//    for(V3DLONG j=0; j<sort_swc.size();j++){
//        hash_nt.insert(sort_swc[j].n, j);
//    }
//    sorted_tree.listNeuron=sort_swc;
//    sorted_tree.hashNeuron=hash_nt;

    // Find tips
    QList<int> ori_tip_list = get_tips(sorted_tree, false);
    cout<<"Number_of_tips:\t"<<qPrintable(swc_file)<<"\t"<<ori_tip_list.size()<<endl;

    //get new tree
    NeuronTree nt;
    nt=get_overtraced_fun(ori_tip_list,sorted_tree,maximum);  // this fuction is to produce new tip nodes.NOTE:the distance is adjustable and random by given range:10um to <maximum> um

    // Find tips
    QList<int> tip_list = get_tips(nt, false);
    cout<<"Number_of__new_tips:\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;

    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("tif"));
    output_suffix.append(QString("swc"));
    printf("welcome to use get_termial\n");
    int tipnum_den=1,tipnum_axon=1;
    for(int i=0; i<tip_list.size(); i++)
    {
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);

        if(nt.listNeuron.at(tip_list.at(i)).type == 3)
        {
            // crop swc
            QString num_cnt_den=QString("%1").arg(tipnum_den);
            QString output_swc_den = output_dir_den+flag1+"_"+num_cnt_den+".eswc";
            if(crop_swc_cuboid(nt, output_swc_den, crop_block))
            // crop image
            {
               QString output_image_den = flag1+"_"+num_cnt_den;
               crop_img(image_file, crop_block, output_dir_den, callback, output_image_den, output_swc_den,tipnum_den,tip,false);
            }
            tipnum_den ++;
            //my_saveANO(output_dir, crop_block.name, output_suffix);
        }
//        else if(nt.listNeuron.at(tip_list.at(i)).type == 2)//axon
//        {
//            // crop swc
//            QString num_cnt_axon=QString("%1").arg(tipnum_axon);
//            QString output_swc = output_dir_axon+flag1+"_"+num_cnt_axon+".eswc";
//            if(crop_swc_cuboid(nt, output_swc, crop_block))
//                // crop image
//            {
//                QString output_image = flag1+"_"+num_cnt_axon;
//                crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum_axon,tip,false);
//            }
//            tipnum_axon ++;
//            //my_saveANO(output_dir, crop_block.name, output_suffix);
//        }
    }
    return;
}


void get_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_dir_axon=outfiles.at(1);
    QString output_apo;

    XYZ block_size=XYZ(100,100,20);
    int tip_type=1;
    if (inparas.size() >= 3 )
    {
        int tmp1=atoi(inparas.at(0));
        int tmp2=atoi(inparas.at(1));
        int tmp3=atoi(inparas.at(2));
        tip_type=atoi(inparas.at(3));
        if (tmp1 != 0 && tmp2 != 0 && tmp3 != 0)
        {
            block_size=XYZ(tmp1,tmp2,tmp3);
            printf("block size is set to:[%dx%dx%d]",tmp1,tmp2,tmp3);
        }
        else
            v3d_msg("The parameter of block size is not valid, the program will use default value of [100x100x20]",0);
    }

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();

//    /XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree sorted_tree = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }
    if(!output_dir_axon.endsWith("/")){
        output_dir_axon = output_dir_axon+"/";
    }

    // Find tips
    QList<int> ori_tip_list = get_tips(sorted_tree, false);
    cout<<"Number_of_tips:\t"<<qPrintable(swc_file)<<"\t"<<ori_tip_list.size()<<endl;

    //get new tree
    NeuronTree nt;
    //nt=get_off_signal_fun(ori_tip_list,sorted_tree,5);  // this fuction is to produce new tip nodes.NOTE:the distance is adjustable and random by given range:10um to <maximum> um
    // Find tips
    QList<int> tip_list = get_tips(sorted_tree, false);
    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;
    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("tif"));
    output_suffix.append(QString("swc"));
    printf("welcome to use get_termial\n");
    for(int i=0; i<tip_list.size(); i++){
        int tipnum=i;
        NeuronSWC node = sorted_tree.listNeuron.at(tip_list.at(i));
        if(node.type > 5) continue;

        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);
        // crop swc
        QString num_cnt=QString("%1").arg(i);
        QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);

        // create a tip-centered block
        if(tip_type == 2){  //get tips from both dendrite and axon

            QString output_swc = output_dir+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
            if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                // crop image
            {
                QString output_image = flag1+"_"+num_cnt+cordinates;
                crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
            }
        }
        else if(tip_type = 1 && node.type == 3){  //only get tips from dendrite


            QString output_swc = output_dir+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
            if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                // crop image
            {
                QString output_image = flag1+"_"+num_cnt+cordinates;
                crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
            }
        }
        else if(tip_type = 1 && node.type == 2){  //only get tips from axon


            QString output_swc = output_dir_axon+flag1+"_"+num_cnt+cordinates+"_croped.eswc";
            if(crop_swc_cuboid(sorted_tree, output_swc, crop_block))
                // crop image
            {
                QString output_image = flag1+"_"+num_cnt+cordinates;
                crop_img(image_file, crop_block, output_dir_axon, callback, output_image, output_swc,tipnum,tip,false);
            }
        }

        //my_saveANO(output_dir, crop_block.name, output_suffix);
    }
    return;
}

NeuronTree get_unfinished_fun(QList<int> tip_list,NeuronTree treeswc,int maximum){

    NeuronTree nt;
    QList<int> to_delete;
    QList<int> plist;
    QList<int> alln;
    int randomnum=(rand()%(maximum-10))+10+1;//make distance adjustable and random
    int N=treeswc.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(treeswc.listNeuron.at(i).pn);
        alln.append(treeswc.listNeuron.at(i).n);
      }
    for (int i=0;i<tip_list.size();i++){

        double dis=0;
        //int step_pn=treeswc.listNeuron.at(tip_list.at(i)).pn;
        int step_n=alln.at(tip_list.at(i));
        int index_n=alln.indexOf(step_n);
        while (dis<randomnum){

            //int index_n=alln.indexOf(step_n);
            double pn=plist.at(index_n);
            if(pn==-1) break;
            int index_step_n=alln.indexOf(pn);
            dis=dist(treeswc.listNeuron.at(index_step_n),treeswc.listNeuron.at(tip_list.at(i)));//distance of tip and candidate tip

            to_delete.push_back(index_n);
            index_n=index_step_n;
        }
    }
    cout<<"size of to delete:\n"<<to_delete.size()<<endl;

    QList <NeuronSWC> newtree1=treeswc.listNeuron;

    for (int i=0;i<treeswc.listNeuron.size();i++){

        for(int j=0;j<to_delete.size();j++){

            if(i==to_delete.at(j)){
               NeuronSWC dele=treeswc.listNeuron.at(i);
              newtree1.removeOne(dele);
            }
        }
    }
    QList<int> newalln;
    QList <NeuronSWC> newtree;
    int n=newtree1.size();
    for(int i=0; i<n;i++){
        newalln.append(newtree1.at(i).n);
      }
    for (int i=0;i<newtree1.size();i++){

        NeuronSWC s;
        s.x=newtree1.at(i).x;
        s.y=newtree1.at(i).y;
        s.z=newtree1.at(i).z;
        s.type=newtree1.at(i).type;
        s.radius=newtree1.at(i).radius;
        s.n=newtree1.at(i).n;
        if((newalln.indexOf(newtree1.at(i).pn))==-1) s.pn=-1;
        else s.pn=newtree1.at(i).pn;
        newtree.push_back(s);
    }


    cout<<"size of new tree:\n"<<newtree.size()<<endl;
    NeuronTree n_t;
    QHash <int, int> hash_nt;

    for(V3DLONG j=0; j<newtree.size();j++){
        hash_nt.insert(newtree[j].n, j);
    }
    n_t.listNeuron=newtree;
    n_t.hashNeuron=hash_nt;
    return n_t;
}


NeuronTree get_overtraced_fun(QList<int> tip_list,NeuronTree sort_swc,int maximum){

    QList<int> plist;
    QList<int> alln;
    QList<NeuronSWC> sorted_listneuron=sort_swc.listNeuron;
    int N=sort_swc.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(sort_swc.listNeuron.at(i).pn);
        alln.append(sort_swc.listNeuron.at(i).n);
      }
    for (int i=0;i<tip_list.size();i++){

        int random_dis=(rand()%(maximum-10))+10+1;//make distance adjustable and random
        MyMarker tip=MyMarker(sorted_listneuron.at(tip_list.at(i)).x,sorted_listneuron.at(tip_list.at(i)).y,sorted_listneuron.at(tip_list.at(i)).z);
        int index_tip_pn=alln.indexOf(sorted_listneuron.at(tip_list.at(i)).pn);
        MyMarker tip_pn=MyMarker(sorted_listneuron.at(index_tip_pn).x,sorted_listneuron.at(index_tip_pn).y,sorted_listneuron.at(index_tip_pn).z);
        QPair<MyMarker,MyMarker> two_marker=QPair<MyMarker,MyMarker>(tip,tip_pn); //two_marker.first=tip;two_marker.second=tip_pn;
        MyMarker new_tip=over_traced_node(two_marker,random_dis);
        NeuronSWC new_line;
        new_line.x=new_tip.x; new_line.y=new_tip.y; new_line.z=new_tip.z;
        new_line.pn=sorted_listneuron.at(tip_list.at(i)).n;
        new_line.type=sorted_listneuron.at(tip_list.at(i)).type;
        new_line.radius=sorted_listneuron.at(tip_list.at(i)).radius;
        new_line.n=N+i+1;
        sorted_listneuron.append(new_line);
        cout<<"================tip"<<tip.x<<"  "<<tip.y<<"  "<<tip.z<<"  "<<endl;
        cout<<"================tip pn"<<tip_pn.x<<"  "<<tip_pn.y<<"  "<<tip_pn.z<<"  "<<endl;
    }

    NeuronTree n_t;
    QHash <int, int> hash_nt;

    for(V3DLONG j=0; j<sorted_listneuron.size();j++){
        hash_nt.insert(sorted_listneuron[j].n, j);
    }
    n_t.listNeuron=sorted_listneuron;
    n_t.hashNeuron=hash_nt;
    return n_t;
}

NeuronTree get_off_signal_fun(QList<int> tip_list,NeuronTree sort_swc,int range){

    QList<int> plist;
    QList<int> alln;
    QList<NeuronSWC> sorted_listneuron=sort_swc.listNeuron;
    int N=sort_swc.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(sort_swc.listNeuron.at(i).pn);
        alln.append(sort_swc.listNeuron.at(i).n);
      }
    for (int i=0;i<tip_list.size();i++){

        //int random_dis=(rand()%(maximum-10))+10+1;//make distance adjustable and random
        MyMarker tip=MyMarker(sorted_listneuron.at(tip_list.at(i)).x,sorted_listneuron.at(tip_list.at(i)).y,sorted_listneuron.at(tip_list.at(i)).z);
        int index_tip_pn=alln.indexOf(sorted_listneuron.at(tip_list.at(i)).pn);
        MyMarker tip_pn=MyMarker(sorted_listneuron.at(index_tip_pn).x,sorted_listneuron.at(index_tip_pn).y,sorted_listneuron.at(index_tip_pn).z);
        QPair<MyMarker,MyMarker> two_marker=QPair<MyMarker,MyMarker>(tip,tip_pn); //two_marker.first=tip;two_marker.second=tip_pn;
        MyMarker new_tip=off_signal_node(two_marker,range);
        NeuronSWC new_line;
        new_line.x=new_tip.x; new_line.y=new_tip.y; new_line.z=new_tip.z;
        new_line.pn=sorted_listneuron.at(tip_list.at(i)).n;
        new_line.type=sorted_listneuron.at(tip_list.at(i)).type;
        new_line.radius=sorted_listneuron.at(tip_list.at(i)).radius;
        new_line.n=N+i+1;
        sorted_listneuron.append(new_line);
    }

    NeuronTree n_t;
    QHash <int, int> hash_nt;

    for(V3DLONG j=0; j<sorted_listneuron.size();j++){
        hash_nt.insert(sorted_listneuron[j].n, j);
    }
    n_t.listNeuron=sorted_listneuron;
    n_t.hashNeuron=hash_nt;
    return n_t;
}

void find_short_branch_tips_menu(V3DPluginCallback2 &callback, QWidget *parent){

    NeuronTree input_tree=callback.getSWCTeraFly();
    LandmarkList ori_markers = callback.getLandmarkTeraFly();

    QList<NeuronSWC> sorted_swc;
    SortSWC(input_tree.listNeuron, sorted_swc ,VOID, 0);
    NeuronTree neurontree=neuronlist_2_neurontree(sorted_swc);

    QPair<vector<int>,int> tip_map=get_short_tips(neurontree, false);
    vector<int> fake_tip_list = tip_map.first;
    int ori_tip_num=tip_map.second;
    v3d_msg(QString("There are [%1] tips in your file,[%2] of them may be fake tips,please check!").arg(ori_tip_num).arg(fake_tip_list.size()));

    for(int i=0;i<fake_tip_list.size();i++)
       {
           LocationSimple t;
           t.x = sorted_swc.at(fake_tip_list.at(i)).x;
           t.y = sorted_swc.at(fake_tip_list.at(i)).y;
           t.z = sorted_swc.at(fake_tip_list.at(i)).z;
           t.color.r = 255;
           t.color.g = 0;
           t.color.b = 0;
           ori_markers.append(t);
       }
    callback.setLandmarkTeraFly(ori_markers);

}
void find_short_branch_tips_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString apo_file=infiles.at(1);
    QString swc_file=infiles.at(0);
    QString output_dir=outfiles.at(0);

    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    list1.removeLast();//delete the last string
    QString flag1=list1.join(".");//join the rest strings to one QString


    QList<CellAPO> file_inmarkers=readAPO_file(apo_file);
    NeuronTree input_tree=readSWC_file(swc_file);

    QList<NeuronSWC> sorted_swc;
    SortSWC(input_tree.listNeuron, sorted_swc ,VOID, 0);
    NeuronTree neurontree=neuronlist_2_neurontree(sorted_swc);
    QPair<vector<int>,int> tip_map=get_short_tips(neurontree, false);
    vector<int> fake_tip_list = tip_map.first;
    int ori_tip_num=tip_map.second;

    QList<CellAPO> listLandmarks;
    for(int i = 0; i < fake_tip_list.size(); i++)
    {
        CellAPO t;
        t.x = sorted_swc.at(fake_tip_list.at(i)).x;
        t.y = sorted_swc.at(fake_tip_list.at(i)).y;
        t.z = sorted_swc.at(fake_tip_list.at(i)).z;
        t.color.r = 255;
        t.color.g = 255;
        t.color.b = 255;
        listLandmarks.append(t);
    }
    file_inmarkers.append(listLandmarks);
    writeAPO_file(output_dir+flag1+".apo", file_inmarkers);

    vector<int> tip_two;tip_two.push_back(ori_tip_num);tip_two.push_back(fake_tip_list.size());
    double cur_per = 100*(double)fake_tip_list.size()/(double)ori_tip_num;



//************Qt write file for unexisting file***********
//record result:generate and write file

    QString out_result =QString(outfiles.at(0))+"/"+QString("tip.txt");

    QDir tempdir;//QDir for creating work directory
    QFile *tempfile=new QFile;
    tempdir.setCurrent(output_dir);//QDir for creating work directory
    if(!tempfile->exists(out_result))
    {
        tempfile->setFileName(out_result);
        if (!tempfile->open(QIODevice::WriteOnly|QIODevice::Text)) return;
        QTextStream myfile(tempfile);
        myfile<<"# generated by Vaa3D Plugin tip detection"<<endl;
        myfile<<"1.Name of file 2.Number of original tips 3.Number of possible fake tips 4.Percent of suspicious tips"<<endl;
        myfile << flag1.toStdString().c_str() <<" " << tip_two.at(0) << " "<< tip_two.at(1)<< " "<< QString::number(cur_per,'f',2).toStdString().c_str()<<"%"<<endl;
    }
//************Qt write file for existing file*************
    else export_file2record(flag1.toStdString(),tip_two,cur_per,out_result);

    tempfile->close();

//************c++ generate file******************************************************************************
//    FILE * fp=0;
//    QString out_result =QString(outfiles.at(0))+"/"+QString("tip.txt");
//    fp = fopen((char *)qPrintable(out_result), "wt");
//    qDebug("txt output dir:%s",qPrintable(out_result));
//    fprintf(fp, "1.Name of swc 2.Number of original tips 3.Number of possible fake tips\n") ;
//    fclose(fp);

//***********c++ this part for adding contain in opened original file(no coverage)***************************
//        ofstream file_result(apo_file.toStdString().c_str(),ios::app);//add contain in open file(no coverage)
//        if(file_result.fail())
//        {
//            cout<<"open marker file error"<<endl;
//        }
//        //file_result<<"#x, y, z, radius"<<endl;
//        for(int i = 0; i < listLandmarks.size(); i++)
//        {
//            file_result<<i<<",,,,"<<listLandmarks[i].z <<","<<listLandmarks[i].x <<","<<listLandmarks[i].y<<",,,,,,,,,"<<255<<","<<255<<","<<255<<endl;
//        }
//        file_result.close();
}


void get2d_label_image(NeuronTree nt_crop_sorted,V3DLONG mysz[4],unsigned char * data1d_crop,V3DPluginCallback2 & callback,QString output_format,int tipnum,XYZ tip,QList<int>mark_other_nodes,bool mark_other_nodes_or_not)
{
   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=0;//by PHC 20170531
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin,mark_other_nodes,mark_other_nodes_or_not);
   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];
   unsigned char *label_mip=0;
   label_mip = new unsigned char [stacksz];

   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           int max_label = 0;
           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
               {
                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
                   max_label = data1d_mask[offsetk + offsetj + ix];
               }
           }
       }
   }
   unsigned char* data1d_2D = 0;
   data1d_2D = new unsigned char [3*stacksz];
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i] = (label_mip[i] ==254) ? 255: image_mip[i];
       //data1d_2D[i] = image_mip[i];

   for(V3DLONG i=0; i<stacksz; i++)
   {
       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
   }
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i+2*stacksz] = image_mip[i];

   mysz[2] = 1;
   mysz[3] = 3;

   //QString mipoutpuut = output_format +QString("%1").arg(tipnum)+"_"+QString("%1_%2_%3").arg(tip.x).arg(tip.y).arg(tip.z)+ "_mip.tif";
   QString mipoutpuut = output_format + "_croped.tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}


void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    cout<<"+++++++++++"<<endl;
    QStringList list=input_swc.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();
//    QString flag=input_swc.right(input_swc.length()-43);
//    QString flag1=flag.left(flag.length()-4);
//    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_image;
    qDebug()<<input_swc;
    qDebug("number:%s",qPrintable(flag1));
    NeuronTree nt_crop_sorted1=readSWC_file(input_swc);
    NeuronTree nt_crop_sorted=revise_radius(nt_crop_sorted1,1);

    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    //printf("+++++++++++:%p\n",p4dImage);

   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=0;//by PHC 20170531
   QList<int> mark_others;
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2],margin, mark_others,false);
   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];//2D orignal image
   unsigned char *label_mip=0;
   label_mip = new unsigned char [stacksz];//2D annotation
   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           int max_label = 0;
           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
               {
                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
                   max_label = data1d_mask[offsetk + offsetj + ix];
               }
           }
       }
   }
   unsigned char* data1d_2D = 0;
   data1d_2D = new unsigned char [3*stacksz];//3 channels image
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i] = image_mip[i];

   for(V3DLONG i=0; i<stacksz; i++)
   {
       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
   }
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i+2*stacksz] = image_mip[i];

   mysz[2] = 1;
   mysz[3] = 3;
   QString mipoutpuut = output_2d_dir +flag1+".tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}

void prune_terminal_nodes(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback){

    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_folder=infiles.at(0);//swc and nrrd folder
    QString input_swc_folder=infiles.at(1);//terafly swc path
    //QString output_2d_dir=outfiles.at(0);//block marker save folder
    //QString output_2d_dir_axon=outfiles.at(1);

    if(!input_folder.endsWith("/")){
        input_folder = input_folder+"/";
    }
    if(!input_swc_folder.endsWith("/")){
        input_swc_folder = input_swc_folder+"/";
    }
    //if(!output_2d_dir.endsWith("/")){
    //    output_2d_dir = output_2d_dir+"/";
    //}


    double percent_his=atof(inparas.at(0));
    double max_length_threshold=atoi(inparas.at(1));//max length for calculating branch threshold
    double radius_nodes=atoi(inparas.at(2));//radius for calculating branch threshold
    double max_length_prunging=atoi(inparas.at(3));//max length for pruning

   printf("========================Tip pruning============================\n");
   printf("========================Tip pruning============================\n");

   //0.read terafly swc
//   QDir dir_swc(input_swc_folder);

//   QStringList namelist_swc=dir_swc.entryList();

//   QStringList all_terafly_swc_name;
//   for(int i=0;i<namelist_swc.size();++i)
//   {
//       if(namelist_swc.at(i).endsWith(".eswc")) {
//            QStringList swc_name= namelist_swc.at(i).split("_");
//            QString swc_neuron_name=swc_name.at(0)+swc_name.at(1);
//            all_terafly_swc_name.push_back(swc_neuron_name);
//       }
//       else if(namelist_swc.at(i).endsWith(".swc")){
//           QStringList swc_name= namelist_swc.at(i).split("_");
//           QString swc_neuron_name=swc_name.at(0)+swc_name.at(1);
//           all_terafly_swc_name.push_back(swc_neuron_name);
//       }
//   }

    //1.read swc and nrrd file
    QDir dir_tip(input_folder);

    QStringList namelist=dir_tip.entryList();

    QStringList all_swc,all_nrrd;
    for(int i=0;i<namelist.size();++i)
    {
        if(namelist.at(i).endsWith(".nrrd")) all_nrrd.push_back(namelist.at(i));
        else if(namelist.at(i).endsWith(".eswc")) all_swc.push_back(namelist.at(i));
        else if(namelist.at(i).endsWith(".swc")) all_swc.push_back(namelist.at(i));
    }

    //1.1 read terafly swc

    QDir dir_terafly_swc(input_swc_folder);

    QStringList namelist1=dir_terafly_swc.entryList();

    QStringList all_terafly_swc;
    for(int i=0;i<namelist1.size();++i)
    {
        if(namelist1.at(i).endsWith(".ano.eswc")) all_terafly_swc.push_back(namelist1.at(i));
        else if(namelist1.at(i).endsWith(".ano.swc")) all_terafly_swc.push_back(namelist1.at(i));
    }
    QString name=all_swc.at(0).split("_")[0]+"_"+all_swc.at(0).split("_")[1];
    //QString terafly_swc_path=input_swc_folder+name+".ano.swc";
    NeuronTree terafly_nt_input;
    if(all_terafly_swc.size()>0){
        for (int i=0;i<all_terafly_swc.size();i++){
            if (all_terafly_swc.at(i).split("/").last().split(".")[0]==name)
            {
                terafly_nt_input=readSWC_file(input_swc_folder+"/"+all_terafly_swc.at(i));
                printf("swc_path=======================:%s \n",qPrintable(all_terafly_swc.at(i)));
            }
        }
    }


    QList<int> plist;
    QList<int> alln;
    int N=terafly_nt_input.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(terafly_nt_input.listNeuron.at(i).pn);
        alln.append(terafly_nt_input.listNeuron.at(i).n);
    }

    map<int,QString> delete_index;
    QList<int> delete_index_interafly;//record deleted nodes in terafly
    for(int i=0;i<all_swc.size();i++)
    {

        //read swc
        QString swc_path=input_folder+all_swc.at(i);
        qDebug("------------------------------------------swc path:%s \n",qPrintable(swc_path));
        NeuronTree nt_input1=readSWC_file(swc_path);

        //read nrrd
        QStringList flag=all_swc.at(i).split(".");flag.removeLast();
        QString name_nrrd=flag.at(0);
        for(int i=1 ;i<flag.size();i++) name_nrrd=name_nrrd+"."+flag.at(i);
        QString nrrd_path=input_folder+name_nrrd+".nrrd";
        qDebug("------------------------------------------nrrd path:%s \n",qPrintable(nrrd_path));
        Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(nrrd_path) ));
        int nChannel = p4dImage->getCDim();

        V3DLONG mysz[4];
        mysz[0] = p4dImage->getXDim();
        mysz[1] = p4dImage->getYDim();
        mysz[2] = p4dImage->getZDim();
        mysz[3] = nChannel;
        cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
        unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
        //set threshold
        //1. block_background
        double num_intensity[256]={0};
        long sz01 = mysz[0] * mysz[1];
        long total_sz=mysz[0] * mysz[1] * mysz[2];
        for (int i=0;i<mysz[0];i++){
              for(int j=0;j<mysz[1];j++){
                   for(int k=0;k<mysz[2];k++){
                       int id = k*sz01+j*mysz[0]+i;
                       if (id<=total_sz) {
                           num_intensity[int(data1d_crop[id])]++;
                       }}}}
       //1. 0.99 threshold
       double sum_num=0;
       int index=0;
       if (sizeof (num_intensity)!=0)//0.99 threshold
       {
           while(double(sum_num/(sz01*mysz[2]))<percent_his && index<256)
           {
               sum_num=num_intensity[index]+sum_num;
               index++;
           }
       }

       double ave_signal,sdev_block;
       mean_and_std(data1d_crop,mysz[0]*mysz[1]*mysz[2],ave_signal,sdev_block);
       double block_bk=ave_signal+sdev_block;//block_background


       // find tips
       int croped_swc_tip_index1;
       croped_swc_tip_index1=find_tip(nt_input1,mysz[0],mysz[1],mysz[2]);
       int tip_n=nt_input1.listNeuron.at(croped_swc_tip_index1).n;

       QList<NeuronSWC> sorted_swc;
       SortSWC(nt_input1.listNeuron, sorted_swc ,tip_n, 0);

       NeuronTree nt_input;
       QHash <int, int> hash_nt;

       for(V3DLONG j=0; j<sorted_swc.size();j++){
           hash_nt.insert(sorted_swc[j].n, j);
       }
       nt_input.listNeuron=sorted_swc;
       nt_input.hashNeuron=hash_nt;

       // find tips
       int croped_swc_tip_index;
       croped_swc_tip_index=find_tip(nt_input,mysz[0],mysz[1],mysz[2]);


       //2.branch background

       QList<int> rollback_nodes=find_tip_and_itschild_length(nt_input,max_length_threshold,croped_swc_tip_index);//20 um
       double branch_intensity=aver_ints_around_nodes(mysz,data1d_crop,radius_nodes,nt_input,rollback_nodes);
       cout<<"=======================================cut location:"<<index<<endl;
       cout<<"================================rollback nodes size:"<<rollback_nodes.size()<<endl;
       cout<<"==================================aver_around_nodes:"<<branch_intensity<<endl;
       cout<<"=====================================aver_in_global:"<<block_bk<<endl;

       //case1. The whole branch is in a dark area.
       if(branch_intensity<block_bk) {printf("=============This branch is in a dark area!===============");continue;}
       //case2.
       double max_thres=(block_bk>branch_intensity) ? block_bk:branch_intensity;
       double thres_final=(max_thres>index) ? index:max_thres;
       cout<<"========================================final thres:"<<thres_final<<endl;
       //case2.1 Pruning length is too long
       QList<int> nodes_maybe_pruned=find_tip_and_itschild_length(nt_input,max_length_prunging,croped_swc_tip_index);//10 um
       cout<<"========================================final thres:"<<thres_final<<endl;
       QList<int> nodes_tobe_pruned;

       int tip_x=int(sorted_swc.at(nodes_maybe_pruned.at(0)).x+0.5);
       int tip_y=int(sorted_swc.at(nodes_maybe_pruned.at(0)).y+0.5);
       int tip_z=int(sorted_swc.at(nodes_maybe_pruned.at(0)).z+0.5);
       int id_intens=tip_z*sz01+tip_y*mysz[0]+tip_x;
       int j=0;
       cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<x: \n"<<tip_x<<endl;
       cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<y: \n"<<tip_y<<endl;
       cout<<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<z: \n"<<tip_z<<endl;
       while (id_intens<=total_sz && data1d_crop[id_intens]<thres_final && j<nodes_maybe_pruned.size()-1)
       {
           nodes_tobe_pruned.push_back(nodes_maybe_pruned.at(j));
           j++;
           tip_x=int(sorted_swc.at(nodes_maybe_pruned.at(j)).x+0.5);
           tip_y=int(sorted_swc.at(nodes_maybe_pruned.at(j)).y+0.5);
           tip_z=int(sorted_swc.at(nodes_maybe_pruned.at(j)).z+0.5);
           id_intens=tip_z*sz01+tip_y*mysz[0]+tip_x;
       }
       //write imagemarkers in block
       QString qs_output = input_folder+name_nrrd+".marker";
       QList <ImageMarker> imagemarks;
       if(nodes_tobe_pruned.size()!=0){
           for(int i=0;i<nodes_tobe_pruned.size();i++){
               ImageMarker marker_inblock;
               marker_inblock.x = sorted_swc.at(nodes_tobe_pruned.at(i)).x+1;
               marker_inblock.y = sorted_swc.at(nodes_tobe_pruned.at(i)).y+1;
               marker_inblock.z = sorted_swc.at(nodes_tobe_pruned.at(i)).z+1;
               marker_inblock.color.a = 0;
               marker_inblock.color.b = 0;
               marker_inblock.color.g = 0;
               marker_inblock.color.r = 255;
               imagemarks.push_back(marker_inblock);
           }
           writeMarker_file(qs_output,imagemarks);
       }
       //delete swc nodes based on terafly coodinates.
       QStringList block_name;
       for(int k=0;k<3;k++) block_name.append(all_swc.at(i).split("_")[k]);
       if(nodes_tobe_pruned.size()!=0){
           int nums=nodes_tobe_pruned.size();
           for (int j=0;j<terafly_nt_input.listNeuron.size();j++)
           {
               double x=all_swc.at(i).split("_")[3].toDouble();
               double y=all_swc.at(i).split("_")[4].toDouble();
               double z=all_swc.at(i).split("_")[5].toDouble();
               MyMarker tip_marker=MyMarker(x,y,z);
               double dist_tip_found=dist(terafly_nt_input.listNeuron.at(j),tip_marker);
               if(dist_tip_found<1)
               //if(terafly_nt_input.listNeuron.at(j).x==x && terafly_nt_input.listNeuron.at(j).y==y)
               {
                   int k;
                   int g=j;
                   while(nums>0)
                   {
                       QString block=block_name.join("_");
                       delete_index[g]=block;
                       delete_index_interafly.push_back(g);
                       //terafly_nt_input.listNeuron.removeAt(g);
                       k=alln.indexOf(terafly_nt_input.listNeuron.at(g).pn);
                       g=k;
                       nums--;
                   }
               }
           }
       }
       printf("=======================makers====================:%d \n",imagemarks.size());
   }
    //write in apo based on terafly coodinates.
    QList<CellAPO>  imagemarks_terafly_apo;
    if(delete_index.size()!=0){
        map<int, QString>::iterator iter;
            iter = delete_index.begin();
            while(iter != delete_index.end()){
            //terafly apo. file
            CellAPO apo_nodes;
            apo_nodes.x = terafly_nt_input.listNeuron.at(iter->first).x+1;
            apo_nodes.y = terafly_nt_input.listNeuron.at(iter->first).y+1;
            apo_nodes.z = terafly_nt_input.listNeuron.at(iter->first).z+1;
            apo_nodes.n=terafly_nt_input.listNeuron.at(iter->first).n;
            apo_nodes.volsize=50;//radius
            apo_nodes.comment=iter->second;
            apo_nodes.color.a = 0;
            apo_nodes.color.b = 0;
            apo_nodes.color.g = 0;
            apo_nodes.color.r = 255;
            imagemarks_terafly_apo.push_back(apo_nodes);
            iter++;
        }
    }
    //delete nodes after generating marker.
    qSort(delete_index_interafly.begin(),delete_index_interafly.end());
    for(int i=delete_index_interafly.size()-1;i>=0;i--) {terafly_nt_input.listNeuron.removeAt(delete_index_interafly.at(i));cout<<"----------------------"<<delete_index_interafly.at(i)+1<<endl;}
    QString pruned_swc_name = input_swc_folder+name+".pruned.swc";
    writeESWC_file(pruned_swc_name,terafly_nt_input);


    QString apo_name = input_swc_folder+name+".apo";
    writeAPO_file(apo_name,imagemarks_terafly_apo);
    QString linker_name =input_swc_folder+name+".ano";
    QFile qf_anofile(linker_name);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }

    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(pruned_swc_name).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
    printf("=======================delete=====================:%d \n",delete_index.size());

    return;
}
void get_tip_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=infiles.at(1);
    QString output_2d_dir=outfiles.at(0);
    QString output_2d_dir_axon=outfiles.at(1);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    if(!output_2d_dir_axon.endsWith("/")){
        output_2d_dir_axon = output_2d_dir_axon+"/";
    }
    int radius=5;
    int relocate_tip_or_not=0;//do not relocate tip position
    int tip_type=2;//get tip from both dendrite and axon
    double precent_his=0.95;//set default his-cut precent
    if (inparas.size() >= 2)
    {
        int tmp=atoi(inparas.at(0));
        relocate_tip_or_not=atoi(inparas.at(1));
        tip_type=atoi(inparas.at(2));
        precent_his=atof(inparas.at(3));
        cout<<"========================================================="<<precent_his<<endl;
        if (tmp>1 && tmp<=20)
        {
            radius=tmp;
            qDebug()<<"mean shift search window radius is set to: "<<tmp;
        }
        else
            v3d_msg("The parameter of window radius is not valid, the program will use default value of 5",0);
    }

    QStringList list=input_swc.split("/");
    QString flag1=(list.last().split("."))[0];
    qDebug()<<input_swc;
    qDebug("flag_name:%s",qPrintable(flag1));

    //1.read swc.file
    NeuronTree nt_input=readSWC_file(input_swc);
    QList<NeuronSWC> nt_crop_swc=nt_input.listNeuron;


    long mysz[4];
    mysz[0] = 50;
    mysz[1] = 50;
    mysz[2] = 50;
    mysz[3] = 1;

    // Find tips
    QList<int> ori_tip_list = get_tips(nt_input, false);
    cout<<"Number_of_tips:\t"<<qPrintable(input_swc)<<"\t"<<ori_tip_list.size()<<endl;
    XYZ block_size=XYZ(50,50,50);
    // 2.Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;

    printf("welcome to use get_relocation function!\n");
    for(int i=0; i<ori_tip_list.size(); i++)
    {
        NeuronSWC node = nt_input.listNeuron.at(ori_tip_list.at(i));
        if(node.type > 5) continue;
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        // crop swc
        QString num_cnt=QString("%1").arg(i);
        QString output_swc_crop = output_2d_dir+flag1+"_"+num_cnt+".eswc";
        QList<NeuronSWC> croped_swc;
        unsigned char * data1d_crop=0;

        // crop image
        QString output_image = flag1+"_"+num_cnt;

        int croped_swc_tip_index=0;
        NeuronTree croped;
        if(croped_swc_return(nt_input, output_swc_crop, crop_block,croped_swc,0))
        {
            croped=neuronlist_2_neurontree(croped_swc);
            croped_swc_tip_index=find_tip(croped,mysz[0],mysz[1],mysz[2]);
            cout<<"_+_+_+_+_+_+_+_+"<<croped_swc_tip_index<<endl;
            if (croped_swc_tip_index==VOID) continue;
            else
                crop_img_not_export(input_image, crop_block, output_2d_dir,callback,data1d_crop,output_image);
            if (data1d_crop == 0) continue;

        }

        //unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
        //printf("+++++++++++:%p\n",p4dImage);
        // 3.Find tip node: move tip node to the center of tip image signal
        int tip_id = ori_tip_list.at(i);
        MyMarker tip,center;
        cout<<"tip id in input swc:"<<tip_id<<endl;
        vector<MyMarker> all_cube_markers;

        tip.x=ceil(croped.listNeuron.at(croped_swc_tip_index).x);
        tip.y=ceil(croped.listNeuron.at(croped_swc_tip_index).y);
        tip.z=ceil(croped.listNeuron.at(croped_swc_tip_index).z);
        cout<<"tip location x:"<<tip.x<<" y:"<<tip.y<<" z:"<<tip.z<<endl;
        //4.tip relocation

        //--------------------------------------------------

        all_cube_markers=get_in_circle_nodes(tip,radius);//R=5
        cout<<"cube size(number of pixels):"<<all_cube_markers.size()<<endl;


        unsigned char* total1dData_mask = 0;
        total1dData_mask = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
        unsigned char* test_mask = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
        memset(total1dData_mask,0,mysz[0]*mysz[1]*mysz[2]*sizeof(unsigned char));
        QList<int> empty_markers;
        double margin=5;
        ComputemaskImage(croped, total1dData_mask, mysz[0], mysz[1], mysz[2],margin,empty_markers, false);
        double total_signal_cube=0,ave_signal_all_mask=0;
        int nt_num=0;
        for(V3DLONG j=0;j<mysz[0]*mysz[1]*mysz[2];++j)
        {
            test_mask[j] = (total1dData_mask[j] == 0)?0:data1d_crop[j];
            if(total1dData_mask[j] !=0 ) {
                total_signal_cube += test_mask[j];
                nt_num++;
            }
        }
        if(nt_num != 0) ave_signal_all_mask=total_signal_cube / nt_num;
        QString saveName = output_2d_dir +num_cnt +".nrrd";
        qDebug("--------------------------nrrd name:%s",qPrintable(saveName));
        //const char* fileName = saveName.toAscii();
        //simple_saveimage_wrapper(callback, saveName.toStdString().c_str(), test_mask , mysz, 1);

        double ave_signal_mask,sdev_block_mask,ave_signal_image_mask;
        mean_and_std(test_mask,mysz[0]*mysz[1]*mysz[2],ave_signal_mask,sdev_block_mask);
        ave_signal_image_mask=ave_signal_mask;//+sdev_block_mask*0.5;

        double ave_signal,sdev_block,ave_signal_cube,ave_signal_image;
        mean_and_std(data1d_crop,mysz[0]*mysz[1]*mysz[2],ave_signal,sdev_block);
        ave_signal_cube=get_circle_signal(all_cube_markers,data1d_crop,mysz[0],mysz[1],mysz[2]);
        ave_signal_image=ave_signal+sdev_block*0.5;

        cout<<"average signal in cube(1000):"<<ave_signal_cube<<endl;
        cout<<"average signal in block(100*100*20):"<<ave_signal_image<<endl;
        cout<<"average signal in block(100*100*20)_mask:"<<ave_signal_image_mask<<endl;
        cout<<"average signal in mask:"<<ave_signal_all_mask<<endl;

        node_and_id front_info;
        //front_info=return_signal_in_circle_nodes(radius,ave_signal_all_mask,mysz,data1d_crop,tip);
        //center=node_to_center(front_info,nt_crop_swc,mysz,data1d_crop);//fine center point based on intensity
        QList<NeuronSWC> output_swc;
        NeuronTree result_tree;
        //output_swc=change_tip_xyz(croped_swc,croped_swc_tip_index,center);
        //result_tree=neuronlist_2_neurontree(output_swc);
        //QString swc_name = output_2d_dir+flag1+"_"+num_cnt+"."+QString ("tip_reset.swc");
        //writeSWC_file(swc_name,result_tree);
        //qDebug()<<swc_name;

        //4.1 histogram
        //double *num_intensity;
        int his_radius=7;
        double num_intensity[256]={0};
        double cube_bx=tip.x-his_radius-1;
        double cube_by=tip.y-his_radius-1;
        double cube_bz=tip.z-his_radius-1;
        long sz01 = mysz[0] * mysz[1];
        long total_sz=mysz[0] * mysz[1] * mysz[2];
        int count=0;
        for (int i=0;i<2*his_radius;i++){
            MyMarker node;
              for(int j=0;j<2*his_radius;j++){
                   for(int k=0;k<2*his_radius;k++){
                       node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                       int id = node.z*sz01+node.y*mysz[0]+node.x;
                       if(id<total_sz) num_intensity[int(data1d_crop[id])]++;
                       count++;
                   }}}
        for (int  i= 0; i < 256; i++) cout<<"--------++++++++++++++--------intensity value:"<<num_intensity[i]<<endl;
        double sum_num=0;
        int index=0;
        if (sizeof (num_intensity)!=0){
            while((sum_num/(8*his_radius*his_radius*his_radius))<precent_his && index<256){
                sum_num=num_intensity[index]+sum_num;
                index++;
            }
        }
        cout<<"============================cut location:"<<index<<endl;



 //5.mean_shift tip location(1)
        LandmarkList marklist_2D;
        LocationSimple S;
        S.x = croped.listNeuron.at(croped_swc_tip_index).x;
        S.y = croped.listNeuron.at(croped_swc_tip_index).y;
        S.z = croped.listNeuron.at(croped_swc_tip_index).z;


        //4.2.calculate global threshold
        vector<MyMarker> allmarkers;
        for (int i=0;i<mysz[2];i++){
            for (int j=0;j<mysz[1];j++){
                for (int k=0;k<mysz[0];k++){
                    MyMarker node;
                    node.x=k;node.y=j;node.z=i;
                    allmarkers.push_back(node);
                }
            }
        }
        printf("pixel num of the current image:%d \n",allmarkers.size());
        double signal;
        unsigned char *data2d=0;
        signal=get_aver_signal(allmarkers, data1d_crop, data2d,mysz[0], mysz[1], mysz[2], 3);
        QList<int> plist;
        QList<int> alln;
        int N=croped.listNeuron.size();
        for(int i=0; i<N; i++){
            plist.append(croped.listNeuron.at(i).pn);
            alln.append(croped.listNeuron.at(i).n);
        }
        int back_num=0;
        int croped_swc_tip_index_for_remove=croped_swc_tip_index;
        while (data1d_crop[int(S.z*sz01+S.y*mysz[0]+S.x)]<=signal && back_num<=2){

            int index_tips_pn=alln.indexOf(croped.listNeuron.at(croped_swc_tip_index_for_remove).pn);
            if(index_tips_pn!=-1){
            croped_swc_tip_index_for_remove=index_tips_pn;
            S.x = croped.listNeuron.at(croped_swc_tip_index_for_remove).x;
            S.y = croped.listNeuron.at(croped_swc_tip_index_for_remove).y;
            S.z = croped.listNeuron.at(croped_swc_tip_index_for_remove).z;
            back_num++;}
            else back_num=3;
        }

cout<<"=============================================================================================dddddd"<<endl;
        marklist_2D.push_back(S);
        mean_shift_fun fun_obj;
        vector<V3DLONG> poss_landmark;
        vector<float> mass_center;
        //double windowradius = 5;
        V3DLONG sz_img[4];
        sz_img[0] = mysz[0]; sz_img[1] = mysz[1]; sz_img[2] = mysz[2]; sz_img[3] = 1;
        fun_obj.pushNewData<unsigned char>((unsigned char*)data1d_crop, sz_img,index);
        poss_landmark=landMarkList2poss(marklist_2D, sz_img[0], sz_img[0]*sz_img[1]);
cout<<"=============================================================================================eeeee"<<endl;
        mass_center=fun_obj.mean_shift_center_mass_thres(poss_landmark[0],radius);




 //5.mean_shift tip location(2)
        LandmarkList marklist_2D2;
        marklist_2D2.push_back(S);
        mean_shift_fun fun_obj2;
        vector<V3DLONG> poss_landmark2;
        vector<float> mass_center2;
        //double windowradius = 5;
        sz_img[0] = mysz[0]; sz_img[1] = mysz[1]; sz_img[2] = mysz[2]; sz_img[3] = 1;
        fun_obj2.pushNewData<unsigned char>((unsigned char*)data1d_crop, sz_img,index);
        poss_landmark2=landMarkList2poss(marklist_2D2, sz_img[0], sz_img[0]*sz_img[1]);
cout<<"=============================================================================================eeeee"<<endl;
        mass_center2=fun_obj2.mean_shift_center_mass(poss_landmark2[0],radius);

        //7.calculate radius

        //-------------------------------------------------------------------------------
        /*
        map<int,float> r_and_index;
        QList<NeuronSWC> tip_nodes;
        NeuronSWC S_ori,S_mean,S_re;
        S_ori.n 	= 1;
        S_ori.type= 3;
        S_ori.x 	= croped.listNeuron.at(croped_swc_tip_index).x;
        S_ori.y 	= croped.listNeuron.at(croped_swc_tip_index).y;
        S_ori.z 	= croped.listNeuron.at(croped_swc_tip_index).z;
        S_ori.r 	= 1;
        S_ori.pn 	= -1;

        S_mean.n 	= 2;
        S_mean.type= 2;
        S_mean.x 	= mass_center[0];
        S_mean.y 	= mass_center[1];
        S_mean.z 	= mass_center[2];
        S_mean.r 	= 1;
        S_mean.pn 	= 1;

        S_re.n 	= 3;
        S_re.type= 4;
        S_re.x 	= center.x;
        S_re.y 	= center.y;
        S_re.z 	= mass_center[2];//center.z; // change my result's Z to mean_shift result's Z(temp!)
        S_re.r 	= 1;
        S_re.pn 	= 2;
        tip_nodes.append(S_ori);tip_nodes.append(S_mean);tip_nodes.append(S_re);
        r_and_index[0]=5;r_and_index[1]=5;r_and_index[2]=5;

        //r_and_index=calculate_R(data1d_crop,tip_nodes,ave_signal_image_mask,5,mysz,3,20,false);

//prediction index 1:number of foreground pixels
        map<int,float> index_ratio=ratio_r(data1d_crop,tip_nodes,5,ave_signal_all_mask,mysz);
        printf("ratio_ori:%f\n ratio_mean:%f\n ratio_re:%f\n",index_ratio[0],index_ratio[1],index_ratio[2]);
        //cout<<"ratio_ori:"<<index_ratio[0]<<"ratio_mean:"<<index_ratio[1]<<"ratio_re:"<<index_ratio[2]<<endl;
//prediction index 2:number of high intensity pixels(average intensity based on samll range mask)
        // find highest intensity
        //        double max_intensity=0;
        //        for(V3DLONG j=0;j<mysz[0]*mysz[1]*mysz[2];++j)
        //        {
        //            if(test_mask[j] >= max_intensity) {
        //                max_intensity=test_mask[j];
        //            }
        //        }
        unsigned char* total1dData_mask_for_high = 0;
        total1dData_mask_for_high = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
        unsigned char* high_mask = new unsigned char [mysz[0]*mysz[1]*mysz[2]];
        memset(total1dData_mask_for_high,0,mysz[0]*mysz[1]*mysz[2]*sizeof(unsigned char));
        double margin1=2;
        ComputemaskImage(croped, total1dData_mask_for_high, mysz[0], mysz[1], mysz[2],margin1);
        double total_signal_cube_high=0,ave_signal_all_mask_high=0;
        int nt_num_high=0;
        for(V3DLONG j=0;j<mysz[0]*mysz[1]*mysz[2];++j)
        {
            high_mask[j] = (total1dData_mask_for_high[j] == 0)?0:data1d_crop[j];
            if(total1dData_mask_for_high[j] !=0 ) {
                total_signal_cube_high += high_mask[j];
                nt_num_high++;
            }
        }
        if(nt_num != 0) ave_signal_all_mask_high=total_signal_cube_high / nt_num_high;
        cout<<"--------------------------------------high intensity:"<<ave_signal_all_mask_high<<endl;
        QString saveName1 = output_2d_dir +num_cnt+"_high" +".nrrd";
        qDebug("--------------------------nrrd name:%s",qPrintable(saveName1));
        //simple_saveimage_wrapper(callback, saveName1.toStdString().c_str(), high_mask , mysz, 1);

        map<int,float> index_ratio_high=ratio_r(data1d_crop,tip_nodes,5,ave_signal_all_mask_high,mysz);
        printf("ratio_ori_high=================:%f\n ratio_mean_high================:%f\n ratio_re_high========================:%f\n",index_ratio_high[0],index_ratio_high[1],index_ratio_high[2]);
//score
        double fore_score_ori=0,fore_score_mean=0,fore_score_re=0;
        double high_score_ori=0,high_score_mean=0,high_score_re=0;
        fore_score_ori=index_ratio[0] / (index_ratio[0] + index_ratio[1] + index_ratio[2]);
        fore_score_mean=index_ratio[1] / (index_ratio[0] + index_ratio[1] + index_ratio[2]);
        fore_score_re=index_ratio[2] / (index_ratio[0] + index_ratio[1] + index_ratio[2]);

        high_score_ori=index_ratio_high[0] / (index_ratio_high[0] + index_ratio_high[1] + index_ratio_high[2]);
        high_score_mean=index_ratio_high[1] / (index_ratio_high[0] + index_ratio_high[1] + index_ratio_high[2]);
        high_score_re=index_ratio_high[2] / (index_ratio_high[0] + index_ratio_high[1] + index_ratio_high[2]);

        double score_result_ori=0,score_result_mean=0,score_result_re=0;
        score_result_ori=(fore_score_ori*0.4+high_score_ori*0.6) * 100;
        score_result_mean=(fore_score_mean*0.4+high_score_mean*0.6) * 100;
        score_result_re=(fore_score_re*0.4+high_score_re*0.6) * 100;

        double max_score=0;
        max_score= (score_result_ori >= max_score) ? score_result_ori : max_score;
        max_score= (score_result_mean >= max_score) ? score_result_mean : max_score;
        max_score= (score_result_re >= max_score) ? score_result_re : max_score;
*/
 //8.Write data in the file
        bool mark_or_not=1;
        QString qs_output;
        QString output_num=QString("%1").arg(i);
        qs_output = output_2d_dir+flag1+"_"+output_num+".reset.marker";
        // relocation
        QList <ImageMarker> imagemarks;
/*        ImageMarker rel;
        rel.x = center.x;
        rel.y = center.y;
        rel.z = center.z;
        rel.color.a = 0;
        if(max_score == score_result_re && mark_or_not){

            rel.color.b = 255;
            rel.color.g = 255;
            rel.color.r = 255;
            mark_or_not=0;
        }
        else {

            rel.color.b = 0;
            rel.color.g = 255;
            rel.color.r = 0;
        }

        rel.comment=QString("%1").arg(score_result_re);
        rel.radius=r_and_index[2];
        imagemarks.push_back(rel);
        //system("$path -f /tmp/mymarks.marker");
        //system("rm -f /tmp/tmp_out*");
*/
        //original tip
        ImageMarker ori_center;
        ori_center.x = croped.listNeuron.at(croped_swc_tip_index).x+1;
        ori_center.y = croped.listNeuron.at(croped_swc_tip_index).y+1;
        ori_center.z = croped.listNeuron.at(croped_swc_tip_index).z+1;
        ori_center.color.a = 0;
        /*if(max_score == score_result_ori && mark_or_not){

            ori_center.color.b = 255;
            ori_center.color.g = 255;
            ori_center.color.r = 255;
            mark_or_not=0;
        }
        else {
        */
            ori_center.color.b = 0;
            ori_center.color.g = 0;
            ori_center.color.r = 255;
        //}
//        ori_center.comment=QString("%1").arg(score_result_ori);
//        ori_center.radius=r_and_index[0];
        imagemarks.push_back(ori_center);

        //mean_shift tip
        ImageMarker mean;
        mean.x = mass_center[0]+1;
        mean.y = mass_center[1]+1;
        mean.z = mass_center[2]+1;
        mean.color.a = 0;
        /*if(max_score == score_result_mean && mark_or_not){

            mean.color.b = 255;
            mean.color.g = 255;
            mean.color.r = 255;
            mark_or_not=0;
        }
        else {
        */
            mean.color.b = 0;
            mean.color.g = 255;
            mean.color.r = 0;
        //}
        //mean.comment=QString("%1").arg(score_result_mean);
        //mean.radius=r_and_index[1];
        imagemarks.push_back(mean);

        ImageMarker mean_second;
        mean_second.x = mass_center2[0]+1;
        mean_second.y = mass_center2[1]+1;
        mean_second.z = mass_center2[2]+1;
        mean_second.color.a = 0;
        /*if(max_score == score_result_mean && mark_or_not){

            mean.color.b = 255;
            mean.color.g = 255;
            mean.color.r = 255;
            mark_or_not=0;
        }
        else {
        */
            mean_second.color.b = 255;
            mean_second.color.g = 0;
            mean_second.color.r = 0;
        //}
        //mean.comment=QString("%1").arg(score_result_mean);
        //mean.radius=r_and_index[1];
        imagemarks.push_back(mean_second);




        //cout<<"radius_ori:"<<r_and_index[0]<<"radius_mean:"<<r_and_index[1]<<"radius_re:"<<r_and_index[2]<<endl;
        cout<<"++++++++++++++++++++++++=========================tip ins:"<<int(data1d_crop[int(tip.z*sz01+tip.y*mysz[0]+tip.x)])<<endl;
        int re_or_not=0;
        if (int(data1d_crop[int(tip.z*sz01+tip.y*mysz[0]+tip.x)])<index) {

            writeMarker_file(qs_output,imagemarks);
            re_or_not=1;
            cout<<"++++++++++++++++++++++++Relocated tip num:"<<i<<endl;
        }

/*
//record result:generate and write file

        QString out_result =QString(outfiles.at(0))+"/"+QString("tip.csv");

        QDir tempdir;//QDir for creating work directory
        QFile *tempfile=new QFile;
        tempdir.setCurrent(output_2d_dir);//QDir for creating work directory
        if(!tempfile->exists(out_result))
        {
            tempfile->setFileName(out_result);
            if (!tempfile->open(QIODevice::WriteOnly|QIODevice::Text)) return;
            QTextStream myfile(tempfile);
            //myfile<<"# generated by tip relocation plugin"<<endl;
            myfile<<"1.Name,2.Original,3.Mean_shift,4.Relocation"<<endl;
            myfile << QString(flag1+"_"+output_num).toStdString().c_str() <<"," << QString::number(score_result_ori,'f',2).toStdString().c_str()
                   << ","<< QString::number(score_result_mean,'f',2).toStdString().c_str()<< ","<< QString::number(score_result_re,'f',2).toStdString().c_str()<<endl;
            tempfile->close();
        }
//************Qt write file for existing file*************
        else
        {
            QFile file(out_result);
            if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))//append!important!!
            {
                v3d_msg("Can not open the current recording file!Please check!");
                return;
            }

            QTextStream myfile(&file);
            //myfile<<"# generated by tip relocation plugin"<<endl;
            //myfile<<"1.Number of tip,2.Score of original tip,3.Score of mean_shift,4.Score of relocation"<<endl;
            myfile << QString(flag1+"_"+output_num).toStdString().c_str() <<"," << QString::number(score_result_ori,'f',2).toStdString().c_str()
                   << ","<< QString::number(score_result_mean,'f',2).toStdString().c_str()<< ","<< QString::number(score_result_re,'f',2).toStdString().c_str()<<endl;

            file.close();
        }
*/
        if(relocate_tip_or_not==1 && re_or_not==1)
        {
            float shift_x = mass_center[0]+1-25;
            float shift_y = mass_center[1]+1-25;
            float shift_z = mass_center[2]+1-25;
            int tipnum1=i;
            // create a tip-centered block
            if(tip_type == 2){   //get tips from dendrite and axon

                block crop_block1;
                XYZ tip_shift;
                if(node.type == 2)//shift tip from axon but not from dendrite,cas the shift function perform badly in tips from dendrite
                {
                    crop_block1 = offset_block(zcenter_block, XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z));
                    crop_block1.name = QString::number(i);
                    tip_shift=XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z);
                }
                else if(node.type == 3)
                {
                    crop_block1 = offset_block(zcenter_block, XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z));
                    crop_block1.name = QString::number(i);
                    tip_shift=XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z);
                }
                // crop swc
                QString num_cnt1=QString("%1").arg(i);
                QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                QString output_swc_shift = output_2d_dir+flag1+"_"+num_cnt1+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(nt_input, output_swc_shift, crop_block))//change block size
                // crop image
                {
                    QString output_image_shift = flag1+"_"+num_cnt1+cordinates;
                    crop_img(input_image, crop_block, output_2d_dir, callback, output_image_shift, output_swc_shift,tipnum1,tip_shift,false);//change block size
                }
            }
            else if(tip_type = 1 && node.type == 3){  //only get tips only from dendrite and do not shift these tips

                block crop_block1 = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
                crop_block1.name = QString::number(i);
                XYZ tip_shift=XYZ(node.x, node.y, node.z);
                // crop swc
                QString num_cnt1=QString("%1").arg(i);
                QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                QString output_swc_shift = output_2d_dir+flag1+"_"+num_cnt1+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(nt_input, output_swc_shift, crop_block1))
                // crop image
                {
                    QString output_image_shift = flag1+"_"+num_cnt1+cordinates;
                    crop_img(input_image, crop_block1, output_2d_dir, callback, output_image_shift, output_swc_shift,tipnum1,tip_shift,false);
                }
            }
            else if(tip_type = 1 && node.type == 2){  //get tips only from axon and do shift these tips

                block crop_block1 = offset_block(zcenter_block, XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z));
                crop_block1.name = QString::number(i);
                XYZ tip_shift=XYZ(node.x+shift_x, node.y+shift_y, node.z+shift_z);
                // crop swc
                QString num_cnt1=QString("%1").arg(i);
                QString cordinates=QString("_%1_%2_%3").arg(node.x).arg(node.y).arg(node.z);
                QString output_swc_shift = output_2d_dir_axon+flag1+"_"+num_cnt1+cordinates+"_croped.eswc";
                if(crop_swc_cuboid(nt_input, output_swc_shift, crop_block1))
                // crop image
                {
                    QString output_image_shift = flag1+"_"+num_cnt1+cordinates;
                    crop_img(input_image, crop_block1, output_2d_dir_axon, callback, output_image_shift, output_swc_shift,tipnum1,tip_shift,false);
                }
            }
        }
        //delete []total1dData_mask;
        //delete []test_mask;
//        /delete []num_intensity;
    }


}

void prediction(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback){

    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_image=infiles.at(0);
    QString input_swc=infiles.at(1);
    QString python_path=infiles.at(2);
    QString model_path=infiles.at(3);

//    const char* tmp1=QString(inparas.at(0)).toStdString().c_str();
//    const char* tmp2=QString(inparas.at(1)).toStdString().c_str();
//    const char* tmp3=QString(inparas.at(2)).toStdString().c_str();
//    const char* tmp4=QString(inparas.at(3)).toStdString().c_str();

    int tmp1=atoi(inparas.at(0));
    int tmp2=atoi(inparas.at(1));
    int tmp3=atoi(inparas.at(2));
    int tmp4=atoi(inparas.at(3));

    QString output_2d_dir_den=outfiles.at(0);//is a folder,not a file!
    QString output_2d_dir_axon=outfiles.at(0);

    QString output_2d_dir_fake_den=outfiles.at(0);
    QString output_2d_dir_fake_axon=outfiles.at(0);

    QStringList name_list=input_swc.split("/");
    QStringList basename_list=name_list.last().split("_");
    QString flag_num;
    if(basename_list.at(1).split(".").size()>1)  flag_num=basename_list.at(0)+"_"+basename_list.at(1).split(".").at(0)+"_";
    else  flag_num=basename_list.at(0)+"_"+basename_list.at(1)+"_";

    if(!output_2d_dir_den.endsWith("/") || !output_2d_dir_axon.endsWith("/")){
        output_2d_dir_den = output_2d_dir_den+"/"+flag_num+QString("den_tempfile")+"/";
        output_2d_dir_axon = output_2d_dir_axon+"/"+flag_num+QString("axon_tempfile")+"/";
        output_2d_dir_fake_den = output_2d_dir_fake_den+"/"+flag_num+QString("den_tempfile_fake")+"/";
        output_2d_dir_fake_axon = output_2d_dir_fake_axon+"/"+flag_num+QString("axon_tempfile_fake")+"/";
    }
    else {

        output_2d_dir_den = output_2d_dir_den+"/"+flag_num+QString("den_tempfile")+"/";
        output_2d_dir_axon = output_2d_dir_axon+"/"+flag_num+QString("axon_tempfile")+"/";
        output_2d_dir_fake_den = output_2d_dir_fake_den+"/"+flag_num+QString("den_tempfile_fake")+"/";
        output_2d_dir_fake_axon = output_2d_dir_fake_axon+"/"+flag_num+QString("axon_tempfile_fake")+"/";
    }

    QDir dir(output_2d_dir_den);QDir dir_axon(output_2d_dir_axon);QDir dir_fake_den(output_2d_dir_fake_den);QDir dir_fake_axon(output_2d_dir_fake_axon);
    //file 1:samples
    if(!dir.exists())
    {
        dir.mkdir(output_2d_dir_den);
    }
    else
    {
        dir.setFilter(QDir::Files);
        int fileCount = dir.count();
        for (int i = 0; i < fileCount; i++)
            dir.remove(dir[i]);
    }

    if(!dir_axon.exists())
    {
        dir_axon.mkdir(output_2d_dir_axon);
    }
    else
    {
        dir_axon.setFilter(QDir::Files);
        int fileCount = dir_axon.count();
        for (int i = 0; i < fileCount; i++)
            dir_axon.remove(dir_axon[i]);
    }
    //file 2:fake tip
    if(!dir_fake_den.exists())
    {
        dir_fake_den.mkdir(output_2d_dir_fake_den);
    }
    else
    {
        dir_fake_den.setFilter(QDir::Files);
        int fileCount = dir_fake_den.count();
        for (int i = 0; i < fileCount; i++)
            //qDebug()<<dir_fake[i];
            dir_fake_den.remove(dir_fake_den[i]);
    }
    //file 2:fake tip
    if(!dir_fake_axon.exists())
    {
        dir_fake_axon.mkdir(output_2d_dir_fake_axon);
    }
    else
    {
        dir_fake_axon.setFilter(QDir::Files);
        int fileCount = dir_fake_axon.count();
        for (int i = 0; i < fileCount; i++)
            //qDebug()<<dir_fake[i];
            dir_fake_axon.remove(dir_fake_axon[i]);
    }

    QString get_sample=QString("%1/vaa3d -x ML_get_sample -f get_2D3D_block -i '%2' %3 -p %4 %5 %6 %7 -o %8 %9").arg(getAppPath().toStdString().c_str())
            .arg(input_image.toStdString().c_str()).arg(input_swc.toStdString().c_str())
            .arg(tmp1).arg(tmp2).arg(tmp3).arg(tmp4).arg(output_2d_dir_den.toStdString().c_str()).arg(output_2d_dir_axon.toStdString().c_str());
    QString prediction_den=QString("%1/python %2/prediction_50x50x50_den.py %3 %4").arg(python_path.toStdString().c_str()).arg(model_path.toStdString().c_str())
            .arg(output_2d_dir_den.toStdString().c_str()).arg(output_2d_dir_fake_den.toStdString().c_str());
    QString prediction_axon=QString("%1/python %2/prediction_50x50x50_den.py %3 %4").arg(python_path.toStdString().c_str()).arg(model_path.toStdString().c_str())
            .arg(output_2d_dir_axon.toStdString().c_str()).arg(output_2d_dir_fake_axon.toStdString().c_str());
    //system("/home/braincenter4/anaconda3/envs/py3/bin/python /home/braincenter4/vaa3d_tools/hackathon/ouyang/ML_get_samples/prediction.py");
    system(qPrintable(get_sample));
    system(qPrintable(prediction_den));
    system(qPrintable(prediction_axon));

    //    QStringList namelist;
    //    QStringList filters;filters.append(".nrrd");
    //    //QDir dir_fake_second(output_2d_dir_fake);
    //    foreach (QString file, dir_fake_second.entryList(filters, QDir::Files))
    //        namelist += output_2d_dir_fake + '/' + file;

    map <int,vector<double> > map_tip;map_tip.clear();
    QDir dir_fake_den_second(output_2d_dir_fake_den);
    QDir dir_fake_axon_second(output_2d_dir_fake_axon);

    QStringList namelist_den=dir_fake_den_second.entryList();
    cout<<"======fake den file size======:"<<namelist_den.size()<<endl;
    QStringList namelist=dir_fake_axon_second.entryList();
    cout<<"======fake axon file size======:"<<namelist.size()<<endl;
    for (int i=0;i<namelist_den.size();i++) namelist.append(namelist_den.at(i));
    cout<<"======file size======:"<<namelist.size()<<endl;
    int k=0;
    for(int i=0;i<namelist.size();++i)
    {
        if(namelist.at(i).endsWith(".nrrd"))
        {
            vector<double> name_full_cor;name_full_cor.clear();
            QStringList name_cor=namelist.at(i).split("_");
            double name_x,name_y,name_z;
            if (name_cor.size()<10){
                name_x=name_cor.at(3).toDouble();
                name_y=name_cor.at(4).toDouble();
                name_z=name_cor.at(5).toDouble();
            }
            else {
                name_x=name_cor.at(12).toDouble();
                name_y=name_cor.at(13).toDouble();
                name_z=name_cor.at(14).toDouble();
            }
            name_full_cor.push_back(name_x);
            name_full_cor.push_back(name_y);
            name_full_cor.push_back(name_z);
            map_tip[k]=name_full_cor;
            k++;
        }
    }

    QList<CellAPO> file_inmarkers;
    cout<<"=======fake tip size======:"<<map_tip.size()<<endl;
    for(V3DLONG i = 0; i < map_tip.size(); i++)
    {

        vector<double> temp=map_tip[i];
        CellAPO t;
        t.x = temp.at(0);
        t.y = temp.at(1);
        t.z = temp.at(2);
        t.volsize = 50;
        t.color.r = 255;
        t.color.g = 255;
        t.color.b = 255;
        file_inmarkers.push_back(t);

    }

    QString apo_name = input_swc + ".apo";
    writeAPO_file(apo_name,file_inmarkers);
    QString linker_name = input_swc + ".ano";
    QFile qf_anofile(linker_name);
    if(!qf_anofile.open(QIODevice::WriteOnly))
    {
        v3d_msg("Cannot open file for writing!");
        return;
    }

    QTextStream out(&qf_anofile);
    out << "SWCFILE=" << QFileInfo(input_swc).fileName()<<endl;
    out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
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
//unsigned char * image_enhancement(V3DPluginCallback2 & callback,unsigned char* data1d,QString inimg_file,char * outimg_file2)
//{
//    cout<<"Welcome to adaptive enhancement filter"<<endl;
//    unsigned int count = 2, c=0, p = 0,q = 1;
//    double ratio = 1.0;

//    cout<<"scale = "<<count<<endl;
//    cout<<"ch = "<<c<<endl;
//    cout<<"ratio = "<<ratio<<endl;
//    cout<<"soma = "<<p<<endl;
//    cout<<"Gaussian blur = "<<q<<endl;


////  Image4DSimple *subject = callback.loadImage(inimg_file);
////    if(!subject || !subject->valid())
////    {
////         v3d_msg("Fail to load the input image.");
////         if (subject) {delete subject; subject=0;}
////         return false;
////    }

//    V3DLONG in_sz[4];
//    in_sz[0] = 100;
//    in_sz[1] = 100;
//    in_sz[2] = 20;
//    in_sz[3] = 1;
//    V3DLONG pagesz = 100*100*20;
//    V3DLONG offsetc = (c-1)*pagesz;

//    int datatype = V3D_UINT8;

//    QString temp_raw = QString(inimg_file) + "_temp.v3draw";
//    QString temp_gf = QString(inimg_file) + "_gf.v3draw";
//    QString temp_gsdt = QString(inimg_file) + "_gsdt.v3draw";
//    QString temp_gsdt_v2 = QString(inimg_file) + "_gsdt_v2.v3draw";
//    QString temp_wogf = QString(inimg_file) + "_woGf.v3draw";


//    QString temp_soma = QString(inimg_file) + "_soma.v3draw";
//    QString temp_gsdtsoma  = QString(inimg_file) + "_gsdtsoma.v3draw";
//    QString temp_ds = QString(inimg_file) + "_ds.v3draw";
//    QString temp_gsdtds = QString(inimg_file) + "_gsdtds.v3draw";


//    unsigned char *EnahancedImage_final=0;
//    try {EnahancedImage_final = new unsigned char [pagesz];}
//    catch(...)  {v3d_msg("cannot allocate memory for EnhancedImage_final."); return false;}


//    double sigma = 0.3;
//    for(unsigned int  scale = 0; scale < count; scale++)
//    {

//        unsigned char * data1d_gf = 0;
//        unsigned char * gsdtld = 0;
//        unsigned char* EnahancedImage = 0;

//        switch (datatype)
//        {
//        case V3D_UINT8:
//            if (scale==0) //do not filter for the scale 0
//            {
//                if(count ==2)
//                    sigma = 0.3;

//                data1d_gf = new unsigned char [pagesz];
//                memcpy(data1d_gf, data1d, pagesz);
//            }
//            else
//            {
//                sigma = 1.2;
////                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf);

//                callGaussianPlugin(callback,pagesz,sigma,c,(unsigned char* &)data1d_gf, temp_raw, temp_gf);

//            }
////            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld);

//            callgsdtPlugin(callback,(unsigned char *)data1d_gf, in_sz, 1,0,(unsigned char* &)gsdtld, temp_gf, temp_gsdt);
//            AdpThresholding_adpwindow((unsigned char *)data1d_gf, in_sz, 1,sigma,(unsigned char* &)EnahancedImage, gsdtld,3,ratio);
//            break;
//        default: v3d_msg("Invalid data type. Do nothing."); return false;
//        }

//         if (scale==0)
//                memcpy(EnahancedImage_final, EnahancedImage, pagesz);
//         else
//         {
//             for(V3DLONG i = 0; i<pagesz; i++)
//             {
//                 if (EnahancedImage_final[i] < EnahancedImage[i])
//                     EnahancedImage_final[i] = EnahancedImage[i];
//             }
//         }

//        if(data1d_gf) {delete []data1d_gf; data1d_gf =0;}
//        if(EnahancedImage) {delete []EnahancedImage; EnahancedImage =0;}
//        if(gsdtld) {delete []gsdtld; gsdtld =0;}
//    }

//    remove(temp_raw.toStdString().c_str());

//    unsigned char* Enhancement_soma = 0;
//    //    if(p==1)
//    //    {
//    //        LandmarkList listLandmarks;

//    //        enhancementWithsoma(callback,data1d,(unsigned char*)EnahancedImage_final,in_sz,1,(unsigned char *&)Enhancement_soma, temp_soma, temp_gsdtsoma, temp_ds, temp_gsdtds,listLandmarks);
//    //    }
//    //else
//    //{
//        Enhancement_soma = new unsigned char [pagesz];
//        memcpy(Enhancement_soma, EnahancedImage_final, pagesz);
//    //}

//    unsigned char* data1d_uint8 = 0;
//    if(q==1)
//    {
//        simple_saveimage_wrapper(callback,temp_wogf.toStdString().c_str(), (unsigned char *)Enhancement_soma, in_sz, 1);
//        V3DPluginArgItem arg;
//        V3DPluginArgList input_gf;
//        V3DPluginArgList output_gf;

//        arg.type = "random";std::vector<char*> args1;
//        char* char_temp_wogf =  new char[temp_wogf.length() + 1];strcpy(char_temp_wogf, temp_wogf.toStdString().c_str());
//        args1.push_back(char_temp_wogf);arg.p = (void *) & args1; input_gf<< arg;
//        arg.type = "random";std::vector<char*> args;
//        args.push_back("3");args.push_back("3");args.push_back("3");args.push_back("1"); args.push_back("2"); arg.p = (void *) & args; input_gf << arg;
//        char* char_temp_gsdt_v2 =  new char[temp_gsdt_v2.length() + 1];strcpy(char_temp_gsdt_v2, temp_gsdt_v2.toStdString().c_str());
//        arg.type = "random";std::vector<char*> args2;args2.push_back(char_temp_gsdt_v2); arg.p = (void *) & args2; output_gf<< arg;

//        QString full_plugin_name = "gaussian";
//        QString func_name = "gf";

//        callback.callPluginFunc(full_plugin_name,func_name, input_gf,output_gf);

////        unsigned char * data1d_float = 0;
////        V3DLONG in_zz[4];
////        datatype = 0;
////        //char * outimg_file2 = ((vector<char*> *)(output_gf.at(0).p))->at(0);
////        if(!simple_loadimage_wrapper(callback,outimg_file2, data1d_float, in_zz, datatype))
////        {
////            cerr<<"load image "<<outimg_file2<<" error!"<<endl;
////            return false;
////        }

////        remove(temp_wogf.toStdString().c_str());
////        remove(temp_gsdt_v2.toStdString().c_str());

////        double min,max;

////        data1d_uint8 = new unsigned char [pagesz];
////        rescale_to_0_255_and_copy((float *)data1d_float,pagesz,min,max,data1d_uint8);
////        if (data1d_float) { delete []data1d_float; data1d_float=0;}

//    }
//    else
//    {
//        data1d_uint8 = new unsigned char [pagesz];
//        memcpy(data1d_uint8, Enhancement_soma, pagesz);
//    }

//    //simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d_uint8, in_sz, 1);


//    if (Enhancement_soma) {delete []Enhancement_soma; Enhancement_soma=0;}
//    if (EnahancedImage_final) { delete []EnahancedImage_final; EnahancedImage_final=0;}
//    if (data1d_uint8) {delete []data1d_uint8; data1d_uint8=0;}
//    return data1d_uint8;

//}


map<int,float> ratio_r(unsigned char *data1d_crop,QList<NeuronSWC> listswc,float R,float thresh,V3DLONG sz[4]){

    map<int,float> index_ratio;
    for (int i=0;i<listswc.size();i++){

        V3DLONG g,j,k;
        V3DLONG cx, cy, cz;
        double n1x=listswc.at(i).x;
        double n1y=listswc.at(i).y;
        double n1z=listswc.at(i).z;
        double ratio=0;

        double less_num=0;
        float trace_z_thickness=(R<=5) ? 3 : 5;
        float totalsz=0;//8*R*R*R/trace_z_thickness;
        for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
        {
            cz = int(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
            for (j=-R;j<=R;j++)
            {
                cy = int(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
                for (g=-R;g<=R;g++)
                {
                    totalsz++;
                    cx = int(n1x+g+0.5); if (cx<0 || cx>=sz[0]) continue;
                    if ((cz-n1z)*(cz-n1z)+(cy-n1y)*(cy-n1y)+(cx-n1x)*(cx-n1x) <= R*R){  //make sure that the current node is in the range of R

                          V3DLONG ids=cz*sz[0]*sz[1]+cy*sz[1]+cx;
                          if(data1d_crop[ids] >=  thresh) less_num += 1;
                    }
                }
            }
        }
           if (less_num<=0) continue;
           ratio=less_num / totalsz;
           index_ratio[i]=ratio;
    printf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk____x:%f \n:",listswc.at(i).x);
    printf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk____y:%f \n",listswc.at(i).y);
    printf("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk____z:%f \n",listswc.at(i).z);
    }
      return index_ratio;

}
double get_circle_signal(vector<MyMarker> allmarkers, unsigned char * data1d,long sz0, long sz1, long sz2)
{

    double signal = 0;
    double in_block_ct = 0;
    long sz01 = sz0 * sz1;
    long total_sz = sz0 * sz1 * sz2;

    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        int id = z*sz01+y*sz0+x;
        if(id<total_sz){
            signal += data1d[id];
            //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
            in_block_ct += 1;
        }

    }
    //cout<<"total signal:"<<signal<<endl;
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }

    return signal;

}
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2)
{
    // Return the node at center of the image as tip node
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node)<MIN_DIST & childs[i].size()==0) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+1 & childs[i].size()==0) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+2 & childs[i].size()==0) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+2 & nt.listNeuron.at(i).pn==-1) {
            return i;
        }
    }
    return VOID;
    printf("No tip found!\n");
}


QList<int> find_tip_and_itschild_length(NeuronTree nt,double max_length,int tip_index){

    QList<int> all_path_nodes;
    //all_path_nodes.push_back(tip_index);
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
    }

    double length=0;
    int num=0;
    while (length<max_length)
    {
        int pn_index=plist.indexOf(nt.listNeuron.at(tip_index).n);
        if(plist.count(nt.listNeuron.at(tip_index).n)>1) break;//aviod deleting branch point
        all_path_nodes.push_back(tip_index);
        length=length+dist(nt.listNeuron.at(tip_index),nt.listNeuron.at(pn_index));
        tip_index=pn_index;
    }
    return all_path_nodes;
}

double aver_ints_around_nodes(long mysz[4],unsigned char * data1d,int radius,NeuronTree nt,QList<int> all_nodes){

     int r=int(radius+0.5);
     double  all_node_intensity=0;
     int sz1=mysz[0]*mysz[1];
     int cont=0;
     for (int i=0;i<all_nodes.size();i++){

         int x=nt.listNeuron.at(all_nodes.at(i)).x;
         int y=nt.listNeuron.at(all_nodes.at(i)).y;
         int z=nt.listNeuron.at(all_nodes.at(i)).z;
         int xa=int(nt.listNeuron.at(all_nodes.at(i)).x-0.5)-r;
         int xb=int(nt.listNeuron.at(all_nodes.at(i)).x-0.5)+r+1;
         int ya=int(nt.listNeuron.at(all_nodes.at(i)).y-0.5)-r;
         int yb=int(nt.listNeuron.at(all_nodes.at(i)).y-0.5)+r+1;
         int za=int(nt.listNeuron.at(all_nodes.at(i)).z-0.5)-r;
         int zb=int(nt.listNeuron.at(all_nodes.at(i)).z-0.5)+r+1;
         double one_node_intensity=0;
         for (int j=xa;j<xb;j++){
             for(int k=ya;k<yb;k++){
                 for(int g=za;g<zb;g++){
                     double dist_r=sqrt((j-x)*(j-x)+(k-y)*(k-y)+(g-z)*(g-z));
                     if (dist_r>r) continue;
                     int id=g*sz1+k*mysz[0]+j;
                     if(id<=sz1*mysz[2]) {
                         one_node_intensity=one_node_intensity+data1d[id];
                         cont++;
                     }
                 }
             }
         }
         all_node_intensity=all_node_intensity+one_node_intensity;
     }
     double aver_intens=0;
     if (cont!=0) aver_intens=all_node_intensity/cont;
     return aver_intens;
}

QList<int> find_tip_and_itspn(NeuronTree nt, long sz0, long sz1, long sz2)//for mark 2D image witg different coulor indifferent part
{
    // Return the node at center of the image as tip node
    QList<int> nodes_to_be_marked;
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node)<MIN_DIST & childs[i].size()==0) {
            nodes_to_be_marked.push_back(i);
        }
        else if(marker_dist(center, node)<MIN_DIST+1 & childs[i].size()==0) {
            nodes_to_be_marked.push_back(i);
        }
        else if(marker_dist(center, node)<MIN_DIST+2 & childs[i].size()==0) {
            nodes_to_be_marked.push_back(i);
        }
    }
    if(nodes_to_be_marked.size()==1){

        int index_pn=alln.indexOf(nt.listNeuron.at(nodes_to_be_marked.at(0)).pn);
        nodes_to_be_marked.push_back(index_pn);
    }
    return nodes_to_be_marked;
}

QList<NeuronSWC> change_tip_xyz(QList<NeuronSWC>input_swc,int tip_node,MyMarker center){

    QList<NeuronSWC> output_swc;
  for(int i=0;i<input_swc.size();i++){
    if(tip_node!=i){
    NeuronSWC s;
    s.x=input_swc.at(i).x;
    s.y=input_swc.at(i).y;
    s.z=input_swc.at(i).z;
    s.type=input_swc.at(i).type;
    s.radius=input_swc.at(i).radius;
    s.pn=input_swc.at(i).pn;
    s.n=input_swc.at(i).n;
    output_swc.append(s);}
    else{
        NeuronSWC s;
        s.x=center.x;
        s.y=center.y;
        s.z=center.z;
        s.type=input_swc.at(i).type;
        s.radius=input_swc.at(i).radius;
        s.pn=input_swc.at(i).pn;
        s.n=input_swc.at(i).n;
        output_swc.append(s);
    }
  }
  return output_swc;
}

NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

vector<MyMarker> get_in_circle_nodes(MyMarker center_marker, double circle_radius){
     //get nodes in cube
    vector<MyMarker> all_cube_markers;
     double length=circle_radius*2;//R=5pixels
     double total_node_num=length*length*length;
     double cube_bx=center_marker.x-circle_radius;
     double cube_by=center_marker.y-circle_radius;
     double cube_bz=center_marker.z-circle_radius;
     cout<<"start node location:"<<endl<<cube_bx<<endl<<cube_by<<endl<<cube_bz<<endl;
     for (int i=0;i<length;i++){
          MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   //cout<<"+++++++++++++"<<i<<j<<k<<endl;
                   all_cube_markers.push_back(node);}
          }
         }
     return all_cube_markers;
}

node_and_id return_signal_in_circle_nodes(int circle_radius, double ave_signal,long mysz[4],unsigned char * data1d,MyMarker center_marker){
   //get max node x,y,z,whose indensity is more than average
    node_and_id all_roi_nodes;
    vector<MyMarker> max_list;
    //unsigned char * result=0;
    double length=2*circle_radius;
    double cube_bx=center_marker.x-circle_radius;
    double cube_by=center_marker.y-circle_radius;
    double cube_bz=center_marker.z-circle_radius;
    long sz01 = mysz[0] * mysz[1];
    long total_sz = mysz[0] * mysz[1] * mysz[2];
    cout<<"input block size:"<<total_sz<<endl;
    unsigned char* result = new unsigned char[total_sz];
    int num_back=0,num_front=0;
    for (int i=0;i<length;i++){
          MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   int id = node.z*sz01+node.y*mysz[0]+node.x;
                   //printf("id in image signal arr:%d\n",id);
                   //cout<<"node location:"<<endl<<node.x<<endl<<node.y<<endl<<node.z<<endl;
                   for (long cc=0;cc<total_sz;cc++){
                       if(cc!=id) result[cc]=data1d[cc];
                       else{
                            if (id<=total_sz){
                            double min=0;double max=1;
                            if(data1d[id]<=ave_signal) {result[id]=min;num_back+=1;}
                            if(data1d[id]>ave_signal) {result[id]=max;num_front+=1;
                                all_roi_nodes.all_nodes.push_back(node);
                                all_roi_nodes.all_id.push_back(id);
                            }//get the foreground node's id,it is wroten for generating an 0-1 image firstly
                            //result[id] = (data1d[id]<=ave_signal) ? min:max;
                            //cout<<"max_id size:"<<max_list.size()<<endl;
                            }
                       }
                   }
               }
          }
         }
    cout<<"total front num :"<<num_front<<endl;
    cout<<"total back num :"<<num_back<<endl;
    delete []result;
    return all_roi_nodes;
}


MyMarker node_to_center(node_and_id all_max_nodes,QList<NeuronSWC> input_swc,long mysz[4],unsigned char * data1d){

    vector<MyMarker> nodes=all_max_nodes.all_nodes;
    vector<int> ids=all_max_nodes.all_id;
    MyMarker null_marker=MyMarker(0,0,0);
    if(ids.size()==0) return null_marker;
    node_and_id info;
//    for(int i=0;i<ids.size();i++){

//       cout<<"id:"<<ids.at(i)<<endl;
//    }
    //sort(nodes.begin(),nodes.end());
    vector<int> id_numofneibs,total_signal;
    for(int i=0;i<nodes.size();i++){
        //vector<int> result;
        info=get_26_neib_id(nodes.at(i),mysz,data1d,1.5);
        //cout<<"27 pixels' id(size):"<<neibs.size()<<endl;
        //sort(info.all_id.begin(),info.all_id.end());
        //set_intersection(ids.begin(),ids.end(),info.all_id.begin(),info.all_id.end(),back_inserter(result));
        //int num_inters=result.size();
        total_signal.push_back(info.total_signal);
        //cout<<"total signal size:"<<info.total_signal<<endl;
        //id_numofneibs.push_back(num_inters);
        //result.clear();
    }
     //method 1. using neibs total num who is in signal to define center location
//    vector<int>::iterator max=std::max_element(id_numofneibs.begin(),id_numofneibs.end());
//    int pos=distance(id_numofneibs.begin(),max);
//    MyMarker center=nodes.at(pos);
//    cout<<"original id position:"<<pos<<endl<<"max neibs size num of all cube nodes:"<<id_numofneibs.at(pos)<<endl;
//    cout<<"x:"<<center.x<<endl<<"y:"<<center.y<<endl<<"z:"<<center.z<<endl;
//    return center;
    //method 2.using 27 pixels' total signal to define center location
    //sort(total_signal.begin(),total_signal.end());
    vector<int>::iterator max=std::max_element(total_signal.begin(),total_signal.end());
    int pos=distance(total_signal.begin(),max);
    MyMarker center1=nodes.at(pos);
    //for(int i=0;i<total_signal.size();i++) qDebug()<<total_signal.at(i);
    vector<int> total_signal_after=total_signal;
    sort(total_signal_after.begin(),total_signal_after.end());
    vector<int>::iterator max2=std::find(total_signal.begin(),total_signal.end(),total_signal_after.at(total_signal_after.size()-2));
    vector<int>::iterator max3=std::find(total_signal.begin(),total_signal.end(),total_signal_after.at(total_signal_after.size()-3));
    int pos2=distance(total_signal.begin(),max2);
    int pos3=distance(total_signal.begin(),max3);
    MyMarker center2=nodes.at(pos2);
    MyMarker center3=nodes.at(pos3);

    MyMarker center_result=MyMarker((center1.x+center2.x)/2,(center1.y+center2.y)/2,(center1.z+center2.z)/2);
    //MyMarker center_result=MyMarker(center1.x,center1.y,center1.z);

    cout<<"original id position:"<<pos<<endl<<"max total 27 pixel signal of all cube nodes:"<<total_signal.at(pos)<<endl<<"max2 total 27 pixel signal of all cube nodes:"<<total_signal.at(pos2)<<endl
          <<"max3 total 27 pixel signal of all cube nodes:"<<total_signal.at(pos3)<<endl;
    cout<<"x:"<<center_result.x<<endl<<"y:"<<center_result.y<<endl<<"z:"<<center_result.z<<endl;
    return center_result;
}


node_and_id get_26_neib_id(MyMarker center_marker,long mysz[4],unsigned char * data1d,int radius){

    node_and_id info_27;
    //radius=5;//1000 cube's length
    double cube_bx=center_marker.x-radius;
    double cube_by=center_marker.y-radius;
    double cube_bz=center_marker.z-radius;
    double signal=0;
    long sz01 = mysz[0] * mysz[1];
    long total_sz=mysz[0] * mysz[1] * mysz[2];
    for (int i=0;i<2*radius;i++){
        MyMarker node;
          for(int j=0;j<2*radius;j++){
               for(int k=0;k<2*radius;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   int id = node.z*sz01+node.y*mysz[0]+node.x;
                   info_27.all_id.push_back(id);
                   if(id<total_sz){
                       signal += data1d[id];
                       //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
                   }
                   info_27.total_signal=signal;
               }}}
        //cout<<"27 pixels' id(size):"<<neibs_26_id.size()<<endl;
    return info_27;

}

double* get_histogram(MyMarker center_marker,long mysz[4],unsigned char * data1d,int radius){

    //double *num_intensity=(double *)malloc(sizeof (double)*8*radius*radius*radius);
    //static double  num_intensity[8*radius*radius*radius];
    static double num_intensity[8];
    double cube_bx=center_marker.x-radius;
    double cube_by=center_marker.y-radius;
    double cube_bz=center_marker.z-radius;
    double signal=0;
    long sz01 = mysz[0] * mysz[1];
    long total_sz=mysz[0] * mysz[1] * mysz[2];
    int count=0;
    cout<<"------------------------------------------------(size):"<<count<<endl;
    for (int i=0;i<2*radius;i++){
        MyMarker node;
          for(int j=0;j<2*radius;j++){
               for(int k=0;k<2*radius;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   int id = node.z*sz01+node.y*mysz[0]+node.x;
                   //if(id<total_sz) num_intensity[int(data1d[id])]++;
                   count++;
               }}}
    cout<<"------------------------------------------------(size):"<<count<<endl;
    return num_intensity;
}


QList<int> get_tips(NeuronTree nt, bool include_root){
    // whether a node is a tip;
    QList<int> ori_tip_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        if(include_root & nt.listNeuron.at(i).pn == -1){
            ori_tip_list.append(i);
        }
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){ori_tip_list.append(i);}
    } 

    //delete the fake tips(distance between tip and branch node is less than 10)
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    vector<int> delete_index;
    int count_num=0;
    double dis=0;
        for (int i=0;i<ori_tip_list.size();i++)
        {
            int step=ori_tip_list.at(i);
            while (childs[step].size()<2 && dis<10)
                  {
                     if (nt.listNeuron.at(step).pn!=-1)
                     {
                        int stepn=alln.indexOf(nt.listNeuron.at(step).pn);//stepn is the pn but not the i
                        dis=dist(nt.listNeuron.at(ori_tip_list.at(i)),nt.listNeuron.at(stepn));
                        step=stepn;
                        count_num++;
                     }
                     else break;
                  }
            if (dis<10) delete_index.push_back(ori_tip_list.at(i));
            dis=0;
        }
       cout<<"the number of fake tips:"<<delete_index.size()<<endl;

       sort(delete_index.rbegin(),delete_index.rend());
       for(int i=0;i<delete_index.size();i++){

                 ori_tip_list.removeOne(delete_index.at(i));
        }
       cout<<"the number of deleting fake tips):"<<delete_index.size()<<endl;

    return(ori_tip_list);
}

QPair<vector<int>,int>  get_short_tips(NeuronTree nt, bool include_root){
    // whether a node is a tip;
    QPair<vector<int>,int> result;
    QList<int> ori_tip_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        //if(include_root & nt.listNeuron.at(i).pn == -1){
          //  ori_tip_list.append(i);
        //}
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){
            ori_tip_list.append(i);
        }
    }
    cout<<"tip number:"<<ori_tip_list.size()<<endl;
    //find the fake tips(distance between tip and branch node is less than 10)
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    vector<int> result_tip;
    int count_num=0;
    double dis=0;
        for (int i=0;i<ori_tip_list.size();i++)
        {
            int step=ori_tip_list.at(i);
            while (childs[step].size()<2 && dis<10)
                  {
                     if (nt.listNeuron.at(step).pn!=-1)
                     {
                        int stepn=alln.indexOf(nt.listNeuron.at(step).pn);//stepn is the pn but not the i
                        dis=dist(nt.listNeuron.at(ori_tip_list.at(i)),nt.listNeuron.at(stepn));
                        step=stepn;
                        count_num++;
                     }
                     else break;
                  }
            if (dis<10) result_tip.push_back(ori_tip_list.at(i));
            dis=0;
        }
       cout<<"the number of fake tips:"<<result_tip.size()<<endl;
       result.first=result_tip;result.second=ori_tip_list.size();
    return(result);
}
XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}

void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format,QString input_swc,int tipnum,XYZ tip,bool mark_others_or_not)
{
    printf("welcome to use crop_img\n");
    //if(output_format.size()==0){output_format=QString(".tiff");}

    V3DLONG *in_zz;
    if(!callback.getDimTeraFly(image.toStdString(), in_zz))
    {
        v3d_msg("Cannot load terafly images.",0);
        return;
    }
    // 1. When cropping, ranges must be integers
    // pixels at large values won't be included, so set large.x/y/z as large.x/y/z + 1
    XYZ small=XYZ(crop_block.small);
    XYZ large=XYZ(crop_block.large);
    small.x = floor(small.x);
    small.y = floor(small.y);
    small.z = floor(small.z);
    large.x = floor(large.x);
    large.y = floor(large.y);
    large.z = floor(large.z);
//    large.x = ceil(large.x)+1;
//    large.y = ceil(large.y)+1;
//    large.z = ceil(large.z)+1;
    // 2. Crop image. image is stored as 1d array. 2 parameters needed for cropping:
    // 2.1. 'cropped_image' is a pointer to the beginning of the region of interest
    unsigned char * cropped_image = 0;
    qDebug()<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z;
    cropped_image = callback.getSubVolumeTeraFly(image.toStdString(),
                                                 small.x, large.x,
                                                 small.y, large.y,
                                                 small.z, large.z);
    // 2.2. 'in_sz' sets the size of the region.
    V3DLONG in_sz[4];
    in_sz[0] = large.x-small.x;
    in_sz[1] = large.y-small.y;
    in_sz[2] = large.z-small.z;
    in_sz[3] = in_zz[3];   // channel information
    // 3. Save image
    QString saveName = outputdir_img + output_format +"_croped.nrrd";
    qDebug("--------------------------nrrd name:%s",qPrintable(saveName));
    //const char* fileName = saveName.toAscii();
    simple_saveimage_wrapper(callback, saveName.toStdString().c_str(), cropped_image, in_sz, 1);
    QString save2d = outputdir_img + output_format;
    NeuronTree nt_crop_sorted;
    nt_crop_sorted=readSWC_file(input_swc);
    QList<int> mark_others_nodes;
    if (mark_others_or_not) mark_others_nodes=find_tip_and_itspn(nt_crop_sorted,in_sz[0],in_sz[1],in_sz[2]);
    else mark_others_nodes.clear();
    //get2d_label_image(nt_crop_sorted,in_sz,cropped_image,callback,save2d,tipnum,tip,mark_others_nodes,mark_others_or_not);

    return;
}
void crop_img_not_export(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, unsigned char * & cropped_image,QString output_format,QString input_swc,int tipnum,XYZ tip,bool mark_others_or_not)
{
    printf("welcome to use crop_img\n");
    //if(output_format.size()==0){output_format=QString(".tiff");}

    V3DLONG *in_zz;
    if(!callback.getDimTeraFly(image.toStdString(), in_zz))
    {
        v3d_msg("Cannot load terafly images.",0);
        return;
    }
    // 1. When cropping, ranges must be integers
    // pixels at large values won't be included, so set large.x/y/z as large.x/y/z + 1
    XYZ small=XYZ(crop_block.small);
    XYZ large=XYZ(crop_block.large);
    small.x = floor(small.x);
    small.y = floor(small.y);
    small.z = floor(small.z);
    large.x = floor(large.x);
    large.y = floor(large.y);
    large.z = floor(large.z);
//    large.x = ceil(large.x)+1;
//    large.y = ceil(large.y)+1;
//    large.z = ceil(large.z)+1;
    // 2. Crop image. image is stored as 1d array. 2 parameters needed for cropping:
    // 2.1. 'cropped_image' is a pointer to the beginning of the region of interest
    qDebug()<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z;
    cropped_image = callback.getSubVolumeTeraFly(image.toStdString(),
                                                 small.x, large.x,
                                                 small.y, large.y,
                                                 small.z, large.z);
    V3DLONG in_sz[4];
    in_sz[0] = large.x-small.x;
    in_sz[1] = large.y-small.y;
    in_sz[2] = large.z-small.z;
    in_sz[3] = in_zz[3];   // channel information
    // 3. Save image
    QString saveName = outputdir_img + output_format + ".nrrd";
    qDebug("--------------------------nrrd name:%s",qPrintable(saveName));
    //const char* fileName = saveName.toAscii();
    //simple_saveimage_wrapper(callback, saveName.toStdString().c_str(), cropped_image, in_sz, 1);

    return;
}
bool in_cuboid(NeuronSWC node, XYZ small, XYZ large){
    if((node.x>=small.x) & (node.x<=large.x) &
            (node.y>=small.y) & (node.y<=large.y) &
            (node.z>=small.z) & (node.z<=large.z)
            )
    {
        return 1;
    }
    return 0;
}
bool crop_swc_cuboid(NeuronTree nt, QString qs_output,block input_block,bool export_or_not)
{
    double xs=input_block.small.x; double ys=input_block.small.y; double zs=input_block.small.z;
    double xe=input_block.large.x; double ye=input_block.large.y; double ze=input_block.large.z;
    double xshift=input_block.small.x;double yshift=input_block.small.y; double zshift=input_block.small.z;
    printf("welcome to use crop_swc_cuboid\n");
    // 1. read input
    XYZ small(xs, ys, zs);
    XYZ large(xe, ye, ze);
    qDebug()<<small.x<<small.y<<small.z;
    qDebug()<<large.x<<large.y<<large.z;
    //NeuronTree nt = readSWC_file(qs_input);
    // 2. Decide which nodes are within the cuboid
    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(in_cuboid(node, small, large)){inside_nlist.append(node.n);}
    }
    qDebug()<<inside_nlist.size();
    //3. create a cropped tree
    NeuronTree new_tree;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        // If node is outside of the shell, skip to the next node.
        if(inside_nlist.lastIndexOf(node.n)<0){
            continue;
        }
        // If the parent node is to be deleted, put itself as a parent node.
        if(inside_nlist.lastIndexOf(node.pn)<0){
            node.pn = -1;
        }
        new_tree.listNeuron.append(node);
    }
    //if (new_tree.listNeuron.size()<5) return 0;
    nt.deepCopy(my_SortSWC(new_tree, VOID, 0));
    //4. shift if needed
    if((xshift!=0) || (yshift!=0) || (zshift!=0)){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x -= xshift;
            nt.listNeuron[i].y -= yshift;
            nt.listNeuron[i].z -= zshift;
        }
    }
    // 5. save output
    if (export_or_not)  export_list2file(nt.listNeuron, qs_output);
    return 1;
}

bool croped_swc_return(NeuronTree nt, QString qs_output,block input_block,QList<NeuronSWC>& croped_swc,bool export_or_not)
{
    double xs=input_block.small.x; double ys=input_block.small.y; double zs=input_block.small.z;
    double xe=input_block.large.x; double ye=input_block.large.y; double ze=input_block.large.z;
    double xshift=input_block.small.x;double yshift=input_block.small.y; double zshift=input_block.small.z;
    printf("welcome to use crop_swc_cuboid\n");
    // 1. read input
    XYZ small(xs, ys, zs);
    XYZ large(xe, ye, ze);
    qDebug()<<small.x<<small.y<<small.z;
    qDebug()<<large.x<<large.y<<large.z;
    //NeuronTree nt = readSWC_file(qs_input);
    // 2. Decide which nodes are within the cuboid
    QList <int> inside_nlist;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        if(in_cuboid(node, small, large)){inside_nlist.append(node.n);}
    }
    qDebug()<<inside_nlist.size();
    //3. create a cropped tree
    NeuronTree new_tree;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC node = nt.listNeuron.at(i);
        // If node is outside of the shell, skip to the next node.
        if(inside_nlist.lastIndexOf(node.n)<0){
            continue;
        }
        // If the parent node is to be deleted, put itself as a parent node.
        if(inside_nlist.lastIndexOf(node.pn)<0){
            node.pn = -1;
        }
        new_tree.listNeuron.append(node);
    }
    //if (new_tree.listNeuron.size()<5) return 0;
    nt.deepCopy(my_SortSWC(new_tree, VOID, 0));
    //4. shift if needed
    if((xshift!=0) || (yshift!=0) || (zshift!=0)){
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.listNeuron[i].x -= xshift;
            nt.listNeuron[i].y -= yshift;
            nt.listNeuron[i].z -= zshift;
        }
    }
    croped_swc=nt.listNeuron;
    // 5. save output
    if (export_or_not)  export_list2file(nt.listNeuron, qs_output);
    return 1;
}
void crop_swc(QString input_swc, QString output_swc, block crop_block)
{

    printf("welcome to use crop_swc\n");
    QString cmd = "vaa3d -x preprocess -f crop_swc_cuboid -i " + input_swc + " -o " +output_swc  + " -p "
             + "\""
             + " #a " + QString::number(crop_block.small.x)
             + " #b " + QString::number(crop_block.small.y)
             + " #c " + QString::number(crop_block.small.z)
             + " #d " + QString::number(crop_block.large.x)
             + " #e " + QString::number(crop_block.large.y)
             + " #f " + QString::number(crop_block.large.z)
             + " #r " + QString::number(crop_block.small.x)
             + " #s " + QString::number(crop_block.small.y)
             + " #t " + QString::number(crop_block.small.z)
             + "\"";
    qDebug()<<cmd;
    system(qPrintable(cmd));
    return;
}

NeuronTree revise_radius(NeuronTree inputtree,float radius){

    QList<NeuronSWC> result;
    QList<NeuronSWC> swc_line=inputtree.listNeuron;
    for(int i=0;i<swc_line.size();i++){

        NeuronSWC s;
        s.x=swc_line.at(i).x;
        s.y=swc_line.at(i).y;
        s.z=swc_line.at(i).z;
        s.type=swc_line.at(i).type;
        s.radius=radius;
        s.pn=swc_line.at(i).pn;
        s.n=swc_line.at(i).n;
        result.push_back(s);
    }
    NeuronTree result1;
    QHash <int, int>  hashNeuron;
    for(V3DLONG j=0; j<result.size();j++)
    {
       hashNeuron.insert(result[j].n, j);
    }
    result1.listNeuron=result;
    result1.hashNeuron=hashNeuron;
    return result1;
}

bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix)
{
    if(!ano_dir.endsWith('/')){ano_dir = ano_dir+'/';}
    FILE * fp=0;
    fp = fopen((char *)qPrintable(ano_dir+fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    for(int i=0; i<suffix.size(); i++)
    {
        QString tail = suffix.at(i);
        if(!tail.startsWith('.')){tail = '.'+tail;}
        QString header;
        if(tail.endsWith("SWC") || tail.endsWith("swc")){header = "SWCFILE=";}
        else if(tail.endsWith("APO") || tail.endsWith("apo")){header = "APOFILE=";}
        else{header = "RAWIMG=";}
        fprintf(fp, qPrintable(header+fileNameHeader+tail));
        fprintf(fp, "\n");
    }
    if(fp){fclose(fp);}
    return true;
}

bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    bool eswc_flag=false;
    if(fileSaveName.section('.',-1).toUpper()=="ESWC")
        eswc_flag=true;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin neuron_connector"<<endl;
    if(eswc_flag)
        myfile<<"##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value"<<endl;
    else
        myfile<<"##n,type,x,y,z,radius,parent"<<endl;
    for (V3DLONG i=0;i<lN.size();i++){
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn;
        if(eswc_flag){
            myfile<<" "<<lN.at(i).seg_id<<" "<<lN.at(i).level;
            for(int j=0; j<lN.at(i).fea_val.size(); j++)
                myfile <<" "<< lN.at(i).fea_val.at(j);
        }
        myfile << endl;
    }
    file.close();
    return true;
}

bool export_file2record(string swc_name,vector<int>input_num,double cur_per,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))//append!important!!
        return false;

    QTextStream myfile(&file);
    //myfile<<"# generated by Vaa3D Plugin tip detection"<<endl;
    //myfile<<"1.Name of swc 2.Number of original tips 3.Number of possible fake tips"<<endl;

    myfile << swc_name.c_str() <<" " << input_num.at(0) << " "<< input_num.at(1)<< " "<< QString::number(cur_per,'f',2).toStdString().c_str()<<"%"<<endl;

    file.close();
    return true;
}

void rotation(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString swc_file=infiles[0];
    QString input_image_file=infiles.at(1);
    QString output_2d_dir=outfiles.at(0);
    QString output_image1=outfiles[0];
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    if(!output_image1.endsWith("/")){
        output_image1 = output_image1+"/";
    }

    QString output_down_image=output_image1+"downsample5.nrrd";
    QString output_up_image=output_image1+"upsample5.nrrd";
    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();
    QString output_image=output_image1+flag1+".rotated.nrrd";

    NeuronTree nt=readSWC_file(swc_file);
    if(nt.listNeuron.size()<5) return;
    double *pc=new double[nt.listNeuron.size()];
    double *trans=new double[3];

    NeuronTree result=align_axis(nt,pc,trans);//calling PCA
    QString swc_name = output_2d_dir+flag1+"."+QString ("rotated.swc");
    writeSWC_file(swc_name,result);//save rotated swc
    printf("1. Save rotated swc. \n");

    Matrix3x3 *affinematrix1=new Matrix3x3(float(pc[0]),float(pc[1]),float(pc[2]),float(pc[3]),
            float(pc[4]),float(pc[5]),float(pc[6]),float(pc[7]),float(pc[8]));
    Matrix3x3 affinematrix2=affinematrix1->inverse();

    Matrix affinematrix(3,3);
    affinematrix(1,1)=affinematrix2.data[0];affinematrix(1,2)=affinematrix2.data[1];affinematrix(1,3)=affinematrix2.data[2];
    affinematrix(2,1)=affinematrix2.data[3];affinematrix(2,2)=affinematrix2.data[4];affinematrix(2,3)=affinematrix2.data[5];
    affinematrix(3,1)=affinematrix2.data[6];affinematrix(3,2)=affinematrix2.data[7];affinematrix(3,3)=affinematrix2.data[8];

    //------------------------------------------------------------------------------------------------------------------------------------
    printf("2. Read subject image. \n");
    unsigned char *p_img_sub=0;
    V3DLONG sz_img_sub[4];
    int datatype_sub=0;
    if(!simple_loadimage_wrapper(callback, (char *)qPrintable(input_image_file),p_img_sub,sz_img_sub,datatype_sub))
    {
        printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(input_image_file));
        return;
    }
//****************5 downsample image********************************
//    V3DLONG sz_down[4];
//    sz_down[0]=20;
//    sz_down[1]=20;
//    sz_down[2]=20;
//    sz_down[3]=1;
//    long sz01 = sz_img_sub[0]* sz_img_sub[1];
//    long total_sz = 20*20*sz_img_sub[2];
//    unsigned char* downsample=0;
//    downsample=new unsigned char[total_sz];
//    int x_c=0,count=0;

//    for(int x=2;x<sz_img_sub[0];x+=5)
//    {
//        printf("dowmsample: [%d/%d]\n",sz_img_sub[0],x);
//        int y_c=0;
//        for(int y=2;y<sz_img_sub[1];y+=5)
//        {
//            int z_c=0;
//            for(long z=0;z<sz_img_sub[2];z++)
//            {
//                double signal=0;int nums=0;

//                for(int k=x-2;k<x+2;k++){
//                    for(int j=y-2;j<y+2;j++)
//                       {
//                          signal += p_img_sub[z*sz01+j*sz_img_sub[0]+k];
//                          nums += 1;
//                       }
//                }
//                if(nums>0) signal=signal/nums;
//                downsample[z_c*400+y_c*20+x_c]=signal;
//                count+=1;z_c+=1;
//            }
//            y_c+=1;
//        }
//        x_c+=1;
//    }
//****************5 times resample image in z axis********************************

    double dfactor[3];
    dfactor[0] = 1; dfactor[1] = 1;  dfactor[2] = 5;

    long totalsize_out=sz_img_sub[0]*sz_img_sub[1]*sz_img_sub[2]*dfactor[0]*dfactor[1]*dfactor[2];
    unsigned char * upsample = 0;
    try {upsample = new unsigned char [totalsize_out];}
    catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return;}

    V3DLONG sz_up[4];
    bool option=1;
    if(option)
    {
        sz_up[0] = (V3DLONG)(ceil(dfactor[0]*sz_img_sub[0]));
        sz_up[1] = (V3DLONG)(ceil(dfactor[1]*sz_img_sub[1]));
        sz_up[2] = (V3DLONG)(ceil(dfactor[2]*sz_img_sub[2]));
        sz_up[3] = 1;
        upsample3dvol((unsigned char *)upsample,(unsigned char *)p_img_sub,sz_up,sz_img_sub,dfactor);
    }
//simple_saveimage_wrapper(callback, qPrintable(output_up_image),upsample,sz_up,1);
//****************5 times resample image in z axis********************************
    printf("\t>>read image file [%s] complete.\n",qPrintable(input_image_file));
    printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
    printf("\t\tdatatype: %d\n",datatype_sub);

    if(datatype_sub!=1)
    {
        printf("ERROR: Input image datatype is not UINT8.\n");
        return;
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    printf("3. Affine transform the subject image to target. \n");
    QStringList qsl_outputinfo;
    qsl_outputinfo.push_back("Warped results were saved to:");

    unsigned char *p_img_sub2tar=0;

    //do affine warping
    //if(i_warpmode==0)
    //{
        if(!q_imagewarp_affine(
                upsample, (long * )(sz_up),
                p_img_sub2tar,affinematrix,trans))
        {
            printf("ERROR: q_imagewarp_affine return false!\n");
            if(upsample) 			{delete []upsample;			upsample=0;}
            return;
        }

        //save image
        simple_saveimage_wrapper(callback, qPrintable(output_image),p_img_sub2tar,sz_up,1);
        qsl_outputinfo.push_back(QString("[%1]").arg(output_image));
        if(p_img_sub2tar) 			{delete []p_img_sub2tar;			p_img_sub2tar=0;}
        delete []pc;
        delete []trans;
        delete []affinematrix1;
        delete []upsample;
    //}
}

NeuronTree align_axis(NeuronTree input,double *& PC,double* & trans)
{
    NeuronTree result;
    result.deepCopy(input);
    V3DLONG siz = input.listNeuron.size();
    printf("size=%d\n", siz);

    double * coord = new double[siz * 3];
    for (V3DLONG i=0; i<siz; i++)
    {
        coord[i] = input.listNeuron[i].x;
        coord[siz + i] = input.listNeuron[i].y;
        coord[2*siz + i] = input.listNeuron[i].z;
    }


    double * coord_pca = new double[siz * 3];
    double V[3];
    pca1(coord, 3, siz, coord_pca, PC, V);

    printf("PCA done\n");

    int de[3]={50,50,10};//for getting transform matrix
    for ( int y = 0; y < 3; y++ ){
            double temp=0;
            for (int  k = 0; k < 3; k++ )
                 temp += de[k]*PC[k*3+y];
                 trans[y]=temp;
        }
    for (V3DLONG i=0;i<siz; i++)
    {
        result.listNeuron[i].x = (coord_pca[i]-trans[0]+50);
        result.listNeuron[i].y = (coord_pca[siz + i]-trans[1]+50);
        result.listNeuron[i].z = (coord_pca[2*siz + i]-trans[2]+10)*5;
        result.listNeuron[i].type=input.listNeuron[i].type;
    }

    delete []coord;
    delete []coord_pca;

    return result;
}


bool q_imagewarp_affine(const unsigned char *p_img_sub,const long *sz_img_sub,
        unsigned char *&p_img_affine,Matrix affinematrix,double *trans)
{
    //check parameters
    if(p_img_sub==0 || sz_img_sub==0)
    {
        printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
        return false;
    }
    if(p_img_affine)
    {
        printf("WARNNING: output image pointer is not null, original memeroy it point to will lost!\n");
        p_img_affine=0;
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    //assign output/warp image size
    long sz_img_output[4]={0};
    if(sz_img_output[0]==0)		sz_img_output[0]=sz_img_sub[0];
    if(sz_img_output[1]==0)		sz_img_output[1]=sz_img_sub[1];
    if(sz_img_output[2]==0)		sz_img_output[2]=sz_img_sub[2];
                                sz_img_output[3]=sz_img_sub[3];
    //allocate memory
    p_img_affine=new unsigned char[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]]();
    if(!p_img_affine)
    {
        printf("ERROR: Fail to allocate memory for p_img_sub2tar.\n");
        return false;
    }
    unsigned char ****p_img_sub_4d=0,****p_img_sub2tar_4d=0;
    if(!new4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3],p_img_sub) ||
       !new4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3],p_img_affine))
    {
        printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
        if(p_img_affine) 		{delete []p_img_affine;		p_img_affine=0;}
        if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
        if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}
        return false;
    }
    //affine image warping
    Matrix x_pt_sub2tar_homo(1,3),x_pt_sub_homo(1,3);double aa=0;

    for(long x=0;x<sz_img_output[0];x++)
    {
        printf("affine: [%d/%d]\n",sz_img_output[0],x);
        for(long y=0;y<sz_img_output[1];y++)
            for(long z=0;z<sz_img_output[2];z++)
            {
                //compute the inverse affine projected coordinate in subject image
                x_pt_sub2tar_homo(1,1)=x-50;
                x_pt_sub2tar_homo(1,2)=y-50;
                x_pt_sub2tar_homo(1,3)=z-50;
                //x_pt_sub2tar_homo(4,1)=1.0;
                x_pt_sub_homo=x_pt_sub2tar_homo*affinematrix;
                x_pt_sub_homo(1,1)=x_pt_sub_homo(1,1)+50;
                x_pt_sub_homo(1,2)=x_pt_sub_homo(1,2)+50;
                x_pt_sub_homo(1,3)=x_pt_sub_homo(1,3)+50;

                //linear interpolate
                //coordinate in subject image
                double cur_pos[3];//x,y,z
                cur_pos[0]=x_pt_sub_homo(1,1);
                cur_pos[1]=x_pt_sub_homo(1,2);
                cur_pos[2]=x_pt_sub_homo(1,3);

                //if interpolate pixel is out of subject image region, set to -inf
                if(cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0]-1 ||
                   cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1]-1 ||
                   cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2]-1)
                {
                    p_img_sub2tar_4d[0][z][y][x]=0.0;
                    continue;
                }

                //find 8 neighor pixels boundary
                long x_s,x_b,y_s,y_b,z_s,z_b;
                x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
                y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
                z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

                //compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
                double l_w,r_w,t_w,b_w;
                l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
                t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
                //compute weight for higer slice and lower slice
                double u_w,d_w;
                u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

                //linear interpolate each channel
                for(long c=0;c<sz_img_output[3];c++)
                {
                    //linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
                    double higher_slice;
                    higher_slice=t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s]+r_w*p_img_sub_4d[c][z_s][y_s][x_b])+
                                 b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s]+r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
                    //linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
                    double lower_slice;
                    lower_slice =t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s]+r_w*p_img_sub_4d[c][z_b][y_s][x_b])+
                                 b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s]+r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
                    //linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
                    p_img_sub2tar_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
                    aa=u_w*higher_slice+d_w*lower_slice;
                }
            }
    }
    //------------------------------------------------------------------------------------------------------------------------------------
    printf("6. free memory. \n");
    if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
    if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}

    return true;
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    //1.<get_undertraced_sample>
    cout<<"This plugin for getting deleted tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_undertraced_sample\n";
    cout<<"-i<file name>:\t\t <raw image file> <input .swc > \n";
    cout<<"-p<default=30>:\t\t maximum value for adjusting the range of deleting disdance \n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_undertraced_sample -i <raw image> <swc file> -p <default=30> -o <output sample> dir.\n";

    //2.<get_overtraced_sample>
    cout<<"This plugin for getting overtraced tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_overtraced_sample\n";
    cout<<"-i<file name>:\t\t <raw image file> <input .swc > \n";
    cout<<"-p<default=30>:\t\t maximum value for adjusting the range of deleting disdance \n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_overtraced_sample -i <raw image> <swc file> -p <default=30> -o <output sample> dir.\n";

}
void printHelp1(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    //3.<get_2D3D_block>
    cout<<"This fuction for cropping block swc and image based on tip nodes"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_block\n";
    cout<<"-i<file name>:\t\t input .tif file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"-p<4 parameters>:\t\t \n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_2D3D_block -i <raw image> <swc file> -p <blocksize:xyz> <1(separate axon and dendrite tips) or 2(not separate)> -o <output image(dendrite)> <output image(axon)>dir.\n";

    //4.<get_2D_block>
    cout<<"This fuction for generet 2D images"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_2D_sample\n";
    cout<<"-i<file name>:\t\t input .swc and .tiff file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_2D_block -i <swc file> -p <tiff\nrrd.image> -o <output image> dir.\n";

    //5.<remove_tip_location>
    cout<<"This fuction for removing tip to center of signal(using average signal,and find the maxium one)"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_ML_sample\n";
    cout<<"-i<file name>:\t\t input .swc file\n";
    cout<<"-p<file name>:\t\t input image file(tif,nrrd,v3draw)\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f remove_tip_location -i <original swc> <input tiff\nrrd.image> -p <radius> <0 or 1(relocation)> <1 or 2(separate or not)> -o <output iamge file dir>\n";

    //6.1.prune_tip_APP1
    cout<<"This fuction for pruning nodes in tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t prune_tip_APP1\n";
    cout<<"-i<file name>:\t\t input .swc and image file\n";
    cout<<"-p<2 or 3>:\t\t mode of calculating radius(2D or 3D),in many case 3D is better \n";
    cout<<"-p<1 or 0 >:\t\t 1:chose your input threshold to calculate radius,0:use global average signal to calculate radius \n";
    cout<<"-p<30(default 30)>:\t\t your input threshold(default 30) \n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f prune_tip_APP1 -i <original swc> <input tiff\nrrd.image> -p <2 or 3> <1 or 0 > <input threshold(default 30)> -o <output swc.file dir>\n";

    //6.2.prune_tip_thres
    cout<<"This fuction for pruning nodes in tip using different threshold"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t prune_tip_thres\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f prune_tip_thres -i <swc and nrrd folder> <terafly swc folder> -p <0.99> <20> <2> <10>\n";

    //7.find_fake_tip
    cout<<"This fuction for finding short branches"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t find_fake_tip\n";
    cout<<"-i<swc file name>:\t\t input .swc\n";
    cout<<"-i<apo file name>:\t\t input .apo\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f find_fake_tip -i <.swc file> <.apo file>\n";

    //8.rotation
    cout<<"This fuction for rotating image"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t rotation\n";
    cout<<"-i<swc file name>:\t\t input .swc\n";
    cout<<"-i<image file name>:\t\t input raw image\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f rotation -i <.swc file> <raw image file> -o <directory of output file(folder)>\n";


    //9.<prediction_model>
    cout<<"This fuction using machine learning for fake tip detection"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t prediction_model\n";
    cout<<"-i<image file name>:\t\t input raw image\n";
    cout<<"-i<swc file name>:\t\t input .swc\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f prediction_model -i <raw image file> <.swc file> -p <blocksize:xyz> <1(separate axon and dendrite tips) or 2(not separate)> -o <directory of output file(folder)>\n";


}
