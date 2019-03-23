
#include "tip_main.h"
#include <stdio.h>
#include <iostream>
#include "qlist.h"
#include <algorithm>
#include <iterator>
#include <cmath>
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#include "../../../../vaa3d_tools/hackathon/PengXie/preprocess/sort_swc_redefined.cpp"
#include "../../../../vaa3d_tools/hackathon/LXF/blastneuron_bjut/my_surf_objs.cpp"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define MIN_DIST 2
#define VOID 1000000000

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
using namespace std;

void get_undertraced_sample(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_apo;

    double maximum = (inparas.size() >=1) ? atoi(inparas[0]) : 30;
    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree nt1 = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }   
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
    for(int i=0; i<tip_list.size(); i++){
        int tipnum=i;
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);
        // crop swc
        QString num_cnt=QString("%1").arg(i);
        QString output_swc = output_dir+flag1+"_"+num_cnt+".eswc";
        crop_swc_cuboid(nt, output_swc, crop_block);
        // crop image
        QString output_image = flag1+"_"+num_cnt;
        crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
        //my_saveANO(output_dir, crop_block.name, output_suffix);
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
    QString output_dir=outfiles.at(0);
    QString output_apo;

    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }

    double maximum = (inparas.size() >=1) ? atoi(inparas[0]) : 30;
    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");
    QString flag1=list1.first();


    XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree nt1 = readSWC_file(swc_file);

    QList<NeuronSWC> sort_swc;
    SortSWC(nt1.listNeuron, sort_swc ,VOID, 0);
    NeuronTree sorted_tree;
    QHash <int, int> hash_nt;

    for(V3DLONG j=0; j<sort_swc.size();j++){
        hash_nt.insert(sort_swc[j].n, j);
    }
    sorted_tree.listNeuron=sort_swc;
    sorted_tree.hashNeuron=hash_nt;

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
    for(int i=0; i<tip_list.size(); i++){
        int tipnum=i;
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);
        // crop swc
        QString num_cnt=QString("%1").arg(i);
        QString output_swc = output_dir+flag1+"_"+num_cnt+".eswc";
        crop_swc_cuboid(nt, output_swc, crop_block);
        // crop image
        QString output_image = flag1+"_"+num_cnt;
        crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,true);
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
    QString output_apo;

    QStringList list=swc_file.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();

    XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree nt = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }
    // Find tips
    QList<int> tip_list = get_tips(nt, false);
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
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);
        // crop swc
        QString num_cnt=QString("%1").arg(i);
        QString output_swc = output_dir+flag1+"_"+num_cnt+".eswc";
        crop_swc(swc_file, output_swc, crop_block);
        // crop image
        QString output_image = flag1+"_"+num_cnt;
        crop_img(image_file, crop_block, output_dir, callback, output_image, output_swc,tipnum,tip,false);
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
        QPair<MyMarker,MyMarker> two_marker;two_marker.first=tip;two_marker.second=tip_pn;
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
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2],mark_other_nodes,mark_other_nodes_or_not, margin);
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
   QString mipoutpuut = output_format + "_mip.tif";
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
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], mark_others,false,margin);
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
void get_tip_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=infiles.at(1);
    //QString input_radius=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    int radius=5;
    if (inparas.size()==1)
    {
        int tmp=atoi(inparas.at(0));
        if (tmp>1 && tmp<=20)
        {
            radius=tmp;
            qDebug()<<"mean shift search window radius is set to: "<<tmp;
        }
        else
            v3d_msg("The parameter of window radius is not valid, the program will use default value of 5",0);
    }

    QStringList list=input_swc.split("/");
    QString flag1=(list[6].split("."))[0];//the id in list should not be added 1 to the exact string
//    QString flag=input_swc.right(input_swc.length()-43);
//    QString flag1=flag.left(flag.length()-4);
    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_swc;
    qDebug("name:%s",qPrintable(flag1));
    //1.read croped swc.file
    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
    QList<NeuronSWC> nt_crop_swc=nt_crop_sorted.listNeuron;
    //2.read croped tif.file
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();
    long mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    //printf("+++++++++++:%p\n",p4dImage);
    // 3.Find tip node: and make tip node to the center of tip image signal
    int tip_id = find_tip(nt_crop_sorted, mysz[0], mysz[1], mysz[2]);
    MyMarker tip,center;
    cout<<"tip id in input swc:"<<tip_id<<endl;
    vector<MyMarker> all_cube_markers;
    double ave_signal;
    tip.x=ceil(nt_crop_sorted.listNeuron.at(tip_id).x);
    tip.y=ceil(nt_crop_sorted.listNeuron.at(tip_id).y);
    tip.z=ceil(nt_crop_sorted.listNeuron.at(tip_id).z);
    cout<<"tip location x:"<<tip.x<<" y:"<<tip.y<<" z:"<<tip.z<<endl;
    //3.1 delete fake tip with weak signal(sample num. 002_10 swc. file)
    //to be implemented
    //3.2 return average signal back to nodes in cube
    //int radius=5;
    all_cube_markers=get_in_circle_nodes(tip,radius);//R=5
    cout<<"cube size(number of pixels):"<<all_cube_markers.size()<<endl;
    ave_signal=get_circle_signal(all_cube_markers,data1d_crop,mysz[0],mysz[1],mysz[2]);
    cout<<"average signal in cube:"<<ave_signal<<endl;
//    unsigned char *result;
//    long total_sz = mysz[0] * mysz[1] * mysz[2];
//    result = new unsigned char [total_sz];
//    if (!return_signal_in_circle_nodes(radius,ave_signal,mysz,data1d_crop,tip)) return;
      node_and_id max_info;
      max_info=return_signal_in_circle_nodes(radius,ave_signal,mysz,data1d_crop,tip);
      center=node_to_center(max_info,nt_crop_swc,mysz,data1d_crop);
      QList<NeuronSWC> output_swc;
      NeuronTree result_tree;
      output_swc=change_tip_xyz(nt_crop_swc,tip_id,center);
      result_tree=neuronlist_2_neurontree(output_swc);
      QString swc_name = output_2d_dir+flag1+"."+QString ("tip_reset.swc");
      writeSWC_file(swc_name,result_tree);
      qDebug()<<swc_name;
//    QString mipoutput = output_2d_dir +flag1+"."+"average.tif";
//    qDebug("number:%s",qPrintable(mipoutput));
//    simple_saveimage_wrapper(callback,mipoutput.toStdString().c_str(),data1d_crop,mysz,1);
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
    }
    printf("No tip found!\n");
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
    unsigned char * result=0;
    double length=2*circle_radius;
    double cube_bx=center_marker.x-circle_radius;
    double cube_by=center_marker.y-circle_radius;
    double cube_bz=center_marker.z-circle_radius;
    long sz01 = mysz[0] * mysz[1];
    long total_sz = mysz[0] * mysz[1] * mysz[2];
    cout<<"input block size:"<<total_sz<<endl;
    result = new unsigned char [total_sz];
    int num=0;
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
                            if(data1d[id]<=ave_signal) result[id]=min;
                            if(data1d[id]>ave_signal) {result[id]=max;num+=1;
                                all_roi_nodes.all_nodes.push_back(node);
                                all_roi_nodes.all_id.push_back(id);
                            }//get the foreground node's id,it is wroten for generating an 0-1 image firstly
                            //result[id] = (data1d[id]<=ave_signal) ? min:max;
                            //cout<<"max_id size:"<<max_list.size()<<endl;
                            }
                       }}
               }
          }
         }
    cout<<"total max num :"<<num<<endl;
    return all_roi_nodes;
}


MyMarker node_to_center(node_and_id all_max_nodes,QList<NeuronSWC> input_swc,long mysz[4],unsigned char * data1d){

    vector<MyMarker> nodes=all_max_nodes.all_nodes;
    vector<int> ids=all_max_nodes.all_id;
    node_and_id info;
//    for(int i=0;i<ids.size();i++){

//       cout<<"id:"<<ids.at(i)<<endl;
//    }
    sort(nodes.begin(),nodes.end());
    vector<int> id_numofneibs,total_signal;
    for(int i=0;i<nodes.size();i++){
        vector<int> result;
        info=get_26_neib_id(nodes.at(i),mysz,data1d);
        //cout<<"27 pixels' id(size):"<<neibs.size()<<endl;
        sort(info.all_id.begin(),info.all_id.end());
        set_intersection(ids.begin(),ids.end(),info.all_id.begin(),info.all_id.end(),back_inserter(result));
        int num_inters=result.size();
        total_signal.push_back(info.total_signal);
        //cout<<"total signal size:"<<info.total_signal<<endl;
        id_numofneibs.push_back(num_inters);
        result.clear();
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
    MyMarker center=nodes.at(pos);
    cout<<"original id position:"<<pos<<endl<<"max total 27 pixel signal of all cube nodes:"<<total_signal.at(pos)<<endl;
    cout<<"x:"<<center.x<<endl<<"y:"<<center.y<<endl<<"z:"<<center.z<<endl;
    return center;
}


node_and_id get_26_neib_id(MyMarker center_marker,long mysz[4],unsigned char * data1d){

    node_and_id info_27;
    int length=3;//27 cube's length
    double cube_bx=center_marker.x-1;
    double cube_by=center_marker.y-1;
    double cube_bz=center_marker.z-1;
    double signal=0;
    long sz01 = mysz[0] * mysz[1];
    long total_sz=mysz[0] * mysz[1] * mysz[2];
    for (int i=0;i<length;i++){
        MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
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
    large.x = ceil(large.x)+1;
    large.y = ceil(large.y)+1;
    large.z = ceil(large.z)+1;
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
    QString saveName = outputdir_img + output_format + ".nrrd";
    qDebug("--------------------------nrrd name:%s",qPrintable(saveName));
    //const char* fileName = saveName.toAscii();
    simple_saveimage_wrapper(callback, saveName.toStdString().c_str(), cropped_image, in_sz, 1);
    QString save2d = outputdir_img + output_format;
    NeuronTree nt_crop_sorted;
    nt_crop_sorted=readSWC_file(input_swc);
    QList<int> mark_others_nodes;
    if (mark_others_or_not) mark_others_nodes=find_tip_and_itspn(nt_crop_sorted,in_sz[0],in_sz[1],in_sz[2]);
    else mark_others_nodes.clear();
    get2d_label_image(nt_crop_sorted,in_sz,cropped_image,callback,save2d,tipnum,tip,mark_others_nodes,mark_others_or_not);

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
bool crop_swc_cuboid(NeuronTree nt, QString qs_output,block input_block)
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
    export_list2file(nt.listNeuron, qs_output);
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


void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    //1.1<get_undertraced_sample>
    cout<<"This plugin for getting deleted tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_undertraced_sample\n";
    cout<<"-i<file name>:\t\t <raw image file> <input .swc > \n";
    cout<<"-p<default=30>:\t\t maximum value for adjusting the range of deleting disdance \n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_undertraced_sample --i <raw image> <swc file> -p <default=30> -o <output sample> dir.\n";

    //1.2<get_overtraced_sample>
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
    //2.<get_2D&3D_block>
    cout<<"This fuction for cropping block swc and image based on tip nodes"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_block\n";
    cout<<"-i<file name>:\t\t input .tif file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_2D&3D_block -i <raw image> <swc file> -o <output image> dir.\n";

    //3.<get_2D_block>
    cout<<"This fuction for generet 2D images"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_2D_sample\n";
    cout<<"-i<file name>:\t\t input .swc and .tiff file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_2D_block -i <swc file> -p <tiff\nrrd.image> -o <output image> dir.\n";

    //4.<remove_tip_location>
    cout<<"This fuction for removing tip to center of signal(using average signal,and find the maxium one)"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_ML_sample\n";
    cout<<"-i<file name>:\t\t input .swc file\n";
    cout<<"-p<file name>:\t\t input image file(tif,nrrd,v3draw)\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_tip_sample -i <original swc> <input tiff\nrrd.image> -p <radius> -o <output swc.file dir>\n";

    //5.prune_tip_APP1
    cout<<"This fuction for pruning nodes in tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t prune_tip_APP1\n";
    cout<<"-i<file name>:\t\t input .swc and image file\n";
    cout<<"-p<2 or 3>:\t\t mode of calculating radius(2D or 3D),in many case 3D is better \n";
    cout<<"-p<1 or 0 >:\t\t 1:chose your input threshold to calculate radius,0:use global average signal to calculate radius \n";
    cout<<"-p<30(default 30)>:\t\t your input threshold(default 30) \n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f prune_tip_APP1 -i <original swc> <input tiff\nrrd.image> -p <2 or 3> <1 or 0 > <input threshold(default 30)> -o <output swc.file dir>\n";

    //6.find_fake_tip
    cout<<"This fuction for finding short branches"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t find_fake_tip\n";
    cout<<"-i<swc file name>:\t\t input .swc\n";
    cout<<"-i<apo file name>:\t\t input .apo\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f find_fake_tip -i <.swc file> <.apo file>\n";



}
