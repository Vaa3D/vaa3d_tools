 #include "neurontreepruneorgraft.h"
#include<QHash>
#include"app1/v3dneuron_gd_tracing.h"
#include"app1/marker_radius.h"
#include"app1/sort_swc.h"
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#ifndef VOID
#define VOID 1000000000
#endif
#define PI 3.1415926
#include"mydialog.h"
#include"new_ray-shooting.h"

void neurontreepruneorgraft::Hackthon_process_onetip(NeuronTree nt,V3DLONG i)
{

}


void neurontreepruneorgraft::Hackthon_process_tip(NeuronTree nt)
{
    cout<<"Hackthon_process_tip "<<endl;
    if(nt.listNeuron.isEmpty())
    {
        cout<<"deal_not_match_points:input a wrong  nt"<<endl;
        v3d_msg("deal_not_match_points:input a wrong  nt");
        return ;
    }
//    tp.getImgData(*cb);
    this->setSwcImg(nt);
    //    setSegmentImg or binary
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
//    int trans_count=0;
    for(V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
//        V3DLONG img_size=sz[0]*sz[1]*sz[2];
    unsigned radius_tip=5;
    QVector<NeuronSWC > tip_trouble;
    QVector<V3DLONG > tip_trouble_num;
    QVector<NeuronSWC > tip_ok;
    LandmarkList real_tip;
    cout<<"begin to  creat ray"<<endl;
    vector<vector<float> > x_dis(16*32,vector<float>(100)), y_dis(16*32,vector<float>(100)),z_dis(16*32,vector<float>(100));
    for(int len = 1; len <based_distance; len++)
    {
        int mm = 0;
        for(int n = 1; n <= 16; n++ )
        {
            for(int k = 0; k <=31; k++ )
            {
             x_dis[mm][len-1] = len * sin(PI * n/16) * cos(PI * k / 16);
             y_dis[mm][len-1] = len * sin(PI * n/16) * sin(PI * k / 16);
             z_dis[mm][len-1] = len * cos(PI * n/16);
             mm++;
            }
        }
    }
    cout<<"success creat ray"<<endl;

    for(V3DLONG neu_i=0;neu_i<neuronNum;neu_i++)
    {
        if (childs[neu_i].size()==0)
        {
            //tip
            LocationSimple p;
            p.x=nt.listNeuron.at(neu_i).x;
            p.y=nt.listNeuron.at(neu_i).y;
            p.z=nt.listNeuron.at(neu_i).z;

            LocationSimple true_tip;

            cout<<"begin to get "<<endl;
            bool find_tip=false;
            for(int  i =0;i<16*32;i++)
            {
//                 cout<<"i  "<<i<<endl;
                int count_bcak=0;
                for(int j=0; j < based_distance; j++)
                {
//                     cout<<"in ray  "<<endl;
                    if(p.x+x_dis[i][j]<0||p.x+x_dis[i][j]>=sz[0]
                     ||p.y+y_dis[i][j]<0||p.y+y_dis[i][j]>=sz[1]
                     ||p.z+z_dis[i][j]<0||p.z+z_dis[i][j]>=sz[2])
                    {
                        //out of bound
                       continue;
                    }

                    unsigned char pixe;
                    pixe=p4DImage_bimg->getValueUINT8(p.x+x_dis[i][j],p.y+y_dis[i][j],p.z+z_dis[i][j],0);
//                    pixe=255;
//                    double swc_type=p4DImage_swc_img->getValueUINT8(p.x+x_dis[i][j],p.y+y_dis[i][j],p.z+z_dis[i][j],0);
                    if(pixe>0)
                    {
                        p.x=nt.listNeuron.at(neu_i).x+x_dis[i][j];
                        p.y=nt.listNeuron.at(neu_i).y+y_dis[i][j];
                        p.z=nt.listNeuron.at(neu_i).z+z_dis[i][j];
//                        cout<<"begin bool is_tip=tp.TipDetect_onePoint(p);"<<endl;
                        bool is_tip=tp.refer_TipDetect_onePoint_(p);
//                        cout<<" bool is_tip=tp.TipDetect_onePoint(p);"<<endl;
                        if(is_tip)
                        {//break
                            cout<<"is_tip"<<neu_i<<endl;
                            find_tip=true;
                            true_tip=p;
                            real_tip.push_back(true_tip);
                            break;
                        }
                    } else if(pixe==0)//background
                    {
//                        count_bcak++;
//                        if(count_bcak>background_stop_num)
//                        {
//                            //stop
//                            //back
//                            break;
//                        }

                        p.x=nt.listNeuron.at(neu_i).x+x_dis[i][j];
                        p.y=nt.listNeuron.at(neu_i).y+y_dis[i][j];
                        p.z=nt.listNeuron.at(neu_i).z+z_dis[i][j];
//                        cout<<"begin bool is_tip=tp.TipDetect_onePoint(p);"<<endl;
                        bool is_tip=tp.TipDetect_onePoint(p);
//                        cout<<" bool is_tip=tp.TipDetect_onePoint(p);"<<endl;
                        if(is_tip)
                        {//break
                            cout<<"is_tip"<<neu_i<<endl;
                            find_tip=true;
                            true_tip=p;
                            real_tip.push_back(true_tip);
                            break;
                        }
                    }
                    if(find_tip)
                    {
                        break;
                    }
                }
                if(find_tip)
                {
                    break;
                }
            }
            if(!find_tip)
            {
                tip_trouble.push_back(nt.listNeuron.at(neu_i));
                tip_trouble_num.push_back(neu_i);
            }
            else
            {
                tip_ok.push_back(nt.listNeuron.at(neu_i));
            }
        }
//         if (childs[neu_i].size()==0)
//         {
//             bool isTip;
//             LocationSimple candi;
//             LocationSimple leaf_node;
//             leaf_node.x=nt.listNeuron.at(neu_i).x;
//             leaf_node.y=nt.listNeuron.at(neu_i).y;
//             leaf_node.z=nt.listNeuron.at(neu_i).z;
//             isTip=tp.TipDetect_onePoint_neighbor(leaf_node,candi,based_distance);
//             if(!isTip)
//             {
//                 tip_trouble.push_back(nt.listNeuron.at(neu_i));
////                 cout<<"not 27neighbor tip "<<i<<"th x"<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
//     //                tip_point.at(i).color = blue;
//             }
//             else
//             {
//                 tip_ok.push_back(nt.listNeuron.at(neu_i));
//                 real_tip.push_back(candi);
////                 cout<<"27neighbor tip "<<neu_i<<"th x"<<curlist.at(i).x<<" y "<<curlist.at(i).y<<" z "<<curlist.at(i).z<<endl;
////                 cout<<"27neighbor tip and near "<<i<<"th x"<<candi.x<<" y "<<candi.y<<" z "<<candi.z<<endl;
////                 realtip_list<<candi;
//             }

//         }
    }
    cout<<"success find unmatch"<<endl;
    LandmarkList not_match;
    RGBA8 red,blue,green;
    red.r=255;red.g=0;red.b=0;
    blue.r=0;blue.g=0;blue.b=255;
    green.r=0;green.g=255;green.b=0;
    for (V3DLONG i=0;i<tip_trouble.size();i++)
    {
        //every node can't find
        LocationSimple not_match_p;
        not_match_p.x=tip_trouble.at(i).x;
        not_match_p.y=tip_trouble.at(i).y;
        not_match_p.z=tip_trouble.at(i).z;
        not_match_p.color=red;
        need_to_show.push_back(not_match_p);
    }
    for (V3DLONG i=0;i<tip_ok.size();i++)
    {
        //配对上的叶子节点
        LocationSimple not_match_p;
        not_match_p.x=tip_ok.at(i).x;
        not_match_p.y=tip_ok.at(i).y;
        not_match_p.z=tip_ok.at(i).z;
        not_match_p.color=blue;
        need_to_show.push_back(not_match_p);
    }

    for (V3DLONG i=0;i<real_tip.size();i++)
    {
        //配对上的末梢点
        LocationSimple not_match_p;
        not_match_p.x=real_tip.at(i).x;
        not_match_p.y=real_tip.at(i).y;
        not_match_p.z=real_tip.at(i).z;
        not_match_p.color=green;
        need_to_show.push_back(not_match_p);
    }



}

NeuronTree neurontreepruneorgraft::pruningSwc_back_ground(NeuronTree nt)
{
    //must after binary
    if(!bimg_datald){cout<<"pruningSwc_back_ground but no binary"<<endl;return nt;}//
    cout<<"before pruningSwc_back_ground nt.listNeuron.size()"<<nt.listNeuron.size()<<endl;
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    int trans_count=0;

    for(V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    V3DLONG img_size=sz[0]*sz[1]*sz[2];

    for(V3DLONG i=0;i<neuronNum;i++)
    {
        if (childs[i].size()==0)
        {
            bool save_not=false;
            NeuronSWC S1,S2;
            V3DLONG aaa;
            int current_tip=i;
            int parent_tip = getParent(current_tip,nt);
            if(parent_tip == 1000000000)
                continue;
            while(childs[parent_tip].size()<2)
            {
            //    cout<<"input tip_point_num"<<tip_point_num<<endl;
                S1=nt.listNeuron.at(current_tip);
                if(S1.parent==-1){/*cout<<"tip_point_num>nt.listNeuron.size()";*/ return nt;}
                aaa=sz[0]*sz[1]*S1.z+sz[0]*S1.y+S1.x;
                if(aaa<img_size&&aaa>=0)
                {
                    if(bimg_datald[aaa]==255)
                    {break;}
                }
                S2=nt.listNeuron.at(nt.hashNeuron.value(S1.parent));

            //    cout<<"begin get branch"<<endl;
                vector<NeuronSWC> needtoshow;
                needtoshow=pointsOfTwoPoint(S1,S2);

            //    cout<<"init is over"<<endl;

                for(int ii=0;ii<needtoshow.size();ii++)
                {
                    NeuronSWC current=needtoshow.at(ii);
                    NeuronSWC p1;
                    p1.x=V3DLONG(current.x+0.5);
                    p1.y=V3DLONG(current.y+0.5);
                    p1.z=V3DLONG(current.z+0.5);
                    aaa=sz[0]*sz[1]*p1.z+sz[0]*p1.y+p1.x;
                    if(aaa>=img_size||aaa<0) {cout<<"aaa>=img_size||aaa<0"; continue;}
            //        cout<<" point1 x:"<<p1.x<<" y:"<<p1.y<<" z:"<<p1.z<<" bimg:"<<int(bimg_datald[aaa])<<endl;
                    if(bimg_datald[aaa]==255)
                    {
                        //stop
                        nt.listNeuron[current_tip]=current;
                        save_not=true;
                        trans_count++;
                        break;
                    }
                }
                if(save_not)
                {
                    break;
                }
                flag[current_tip]=0;
                current_tip=parent_tip;
                parent_tip = getParent(current_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }
        }
    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 )
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }
   }
    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"transfer background  trans_count:"<<trans_count<<endl;
    cout<<"after pruningSwc_back_ground neuronNum is "<<final_tree.listNeuron.size()<<endl;

    QString temp_trasfer_name="pruningSwc_back_ground.swc";
    writeSWC_file(temp_trasfer_name,final_tree);
    final_tree=readSWC_file(temp_trasfer_name);

    return final_tree;



}
NeuronTree neurontreepruneorgraft::TreeSegment(NeuronTree nt)
{
    //修改type
    NeuronTree nt1=nt;
    for(V3DLONG i=0;i<nt1.listNeuron.size();i++)
    {
        //init all to 1
        nt1.listNeuron[i].type=1;
    }
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt1.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
//    QVector<QVector<V3DLONG> >childs_p = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );


    for(V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt1.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt1.hashNeuron.value(par)].push_back(i);
    }
    int typecount=0;
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs[i].size()==0||childs[i].size()>=2)
       {
           nt1.listNeuron[i].type=(typecount%254)+2;
           int parent_tip = getParent(i,nt1);
           if(parent_tip == 1000000000)
               continue;
           while(childs[parent_tip].size()<2)
           {
               nt1.listNeuron[parent_tip].type=typecount;
               parent_tip = getParent(parent_tip,nt1);
               if(parent_tip == 1000000000)
                   break;
           }
           typecount++;
           if(parent_tip == 1000000000)
               continue;
       }
    }
    return nt1;
}

bool neurontreepruneorgraft::InputTreeCheck(NeuronTree nt)
{
    int count_neg_1=0;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        if(nt.listNeuron.at(i).parent==-1)
        {
           count_neg_1++;
        }
    }
    if(count_neg_1>1||count_neg_1==0)
    {
        cout<<"Attention:NeuronTree has "<<count_neg_1<<" -1 point"<<endl<<endl;
        return false;
    }
    return true;
}
void neurontreepruneorgraft::Binarization(int threshold)
{
    //
    int img_size=sz[0]*sz[1]*sz[2];
    try{bimg_datald=new unsigned char [img_size];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return ;}

    for(V3DLONG ii=0;ii<img_size;ii++)
    {
        if(datald[ii]>=threshold)
        {
            bimg_datald[ii]=255;
        }
        else
        {
            bimg_datald[ii]=0;
        }
    }
    p4DImage_bimg=new Image4DSimple;
    p4DImage_bimg->setData(bimg_datald,p4DImage);

}
void neurontreepruneorgraft::setSegmentImg(unsigned char * img )
{
    p4DImage_bimg=new Image4DSimple;
    p4DImage_bimg->setData(img,p4DImage);
}

neurontreepruneorgraft::neurontreepruneorgraft()
{
    dist_traced=5;
    radius_tip_checked=2;
    dist_need_trace=20;
    radius_leaf_detect=3;
    small_length=10;
    pruing_all_leaf_fix_length=10;

    pruingSwc_one_leafnode_stop_length=10;
    isProcessTip=false;
    isInputSwc=false;

    //init
    this->set_function_list();

    cb=NULL;
    datald=NULL;
    bimg_datald=NULL;
    swcimg_datald=NULL;
    p4DImage=NULL;
    p4DImage_bimg=NULL;
    p4DImage_swc_img=NULL;
    TMI_show_match_result=false;

    //para for not match
    based_distance=15;
    online_confidece_min=0.3;
    Max_number_other_swc=6;
    binary_threshold=8;
    background_stop_num=8;
    pruning_branch_count=0;
    angle_swc=120;

}
neurontreepruneorgraft::~neurontreepruneorgraft()
{
    if(bimg_datald) {delete []bimg_datald; bimg_datald = 0;}
    if(swcimg_datald) {delete []swcimg_datald; swcimg_datald = 0;}

}

void neurontreepruneorgraft::do_function_dialog(V3DPluginCallback2 &callback, QWidget *parent)
{
    MyDialog dialog;

    dialog.setRightParaList(func_list);
    QStringList func_list_needto_do;
    if(dialog.exec()!= QDialog::Accepted)
    {
        cout<<"cancle to input para list"<<endl;
        return ;

    }//return funlist
    func_list_needto_do=dialog.return_left_para();
    if(func_list_needto_do.isEmpty())
    {
        cout<<"input empty para list"<<endl;
        return ;
    }




    cout<<"do_function_dialog get para over"<<endl;
    if(!getImgData_not_process_tip(callback, parent))
    {
        //no img is open
        return;
    }
    cout<<"do_function_dialog getImgData_not_process_tip "<<endl;
    NeuronTree nt;
    nt=this->return_neurontree();
    for(int temp=0;temp<func_list_needto_do.size();temp++)
    {
        cout<<"begin to  func:"<<func_list_needto_do.at(temp).toUtf8().data()<<endl;
        this->run_function_list(func_list_needto_do.at(temp),nt,callback);
    }

    cout<<"do_function_dialog is over"<<endl;
    QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\331.swc";
    writeSWC_file(filename,nt);

    v3dhandle curwin = callback.currentImageWindow();
    callback.setSWC(curwin,nt);


}
void neurontreepruneorgraft::run_function_list(QString func_name,NeuronTree &nt,V3DPluginCallback2 &callback)
{
    int func_ind;
    for(int i=0;i<func_list.size();i++)
    {
        if(func_name==func_list.at(i))
        {
            //i is needfunc_ind
            func_ind=i;

        }
    }
    cout<<"func_ind:"<<func_ind<<endl;
    if((!isProcessTip)&&need_tip[func_ind])
    {
        //get tip data
        if(!tp.GUI_input_argu())
        {
            //didn't input argu
            //program should stop
            cout<<"argu input cancel"<<endl;
            return ;
        }
        tp.whole_img(callback);
        tip_list=tp.OutputTipPoint();
        isProcessTip=true;
    }
    LandmarkList point_need_pruning;
    LandmarkList point_tip_list;
    switch (func_ind) {
    case 0:
        cout<<"now goingto do "<<"Pruning_leaf_stop_at_tip"<<endl;
        nt=this->tipBasedPruning_stop_before_tip(nt);
        break;
    case 1:
        cout<<"now goingto do "<<"Pruning_leaf_not_tip"<<endl;
        nt=this->tipBasedPruning(nt);
        break;
    case 2:
        cout<<"now goingto do "<<"graft_to_border"<<endl;
        nt=this->graftToBorder(nt);
        break;
    case 3:
        cout<<"now goingto do "<<"MergeSwc_two_leafnode"<<endl;
        nt=this->mergeSwc_two_leafnode(nt);
        break;
    case 4:
        cout<<"now goingto do "<<"sort_by_SOMA"<<endl;
        nt=this->sort_neuron(nt);
        break;

    case 5:
        cout<<"now goingto do "<<"Reestimate_radius"<<endl;
        nt=this->reestimate_radius(nt);
        break;
    case 6:
        cout<<"now goingto do "<<"pruningSwc_two_leafnode"<<endl;
        nt=this->pruningSwc_two_leafnode(nt);
        break;
    case 7:
        cout<<"now goingto do "<<"pruningSwc_small_length"<<endl;
//        nt=this->sort_neuron(nt);
        this->set_small_length(10);
        nt=this->pruningSwc_small_length(nt);
        break;
    case 8:
        cout<<"now goingto do "<<"pruning_single_leaf_in_small_length"<<endl;
        this->set_leaf_fix_length(10);
        nt=this->pruning_single_leaf_in_small_length(nt);
        break;
    case 9:
        cout<<"now goingto do "<<"find_leaf_need_prun"<<endl;

        point_need_pruning=this->find_leaf_need_prun(nt);
        need_to_show.append(point_need_pruning);
//        point_need_pruning.append();
        break;
    case 10:
        cout<<"now goingto do "<<"Pruning_leaf_stop_at_tip"<<endl;
        nt=this->tipBasedPruning_stop_before_tip_no_z(nt);
        break;
    case 11:
        cout<<"now goingto do "<<"return_tip_list"<<endl;
        point_tip_list=tip_list;
        RGBA8 white;
        white.r=0;
        white.g=255;
        white.b=255;
        for(int i=0;i<point_tip_list.size();i++)
        {
            point_tip_list[i].name="true_tip";
            point_tip_list[i].color=white;
        }
        need_to_show.append(point_tip_list);
        break;
    case 12:
        cout<<"now goingto do "<<"pruning_leaf_node_in_background"<<endl;
        nt=this->Pruning_leaf_node_in_background(nt);
        break;

    default:
        cout<<"can't recognize func_ind :"<<func_ind<<endl;
        return;
    }

}

QStringList neurontreepruneorgraft::set_function_list()
{
    //neuron tree must be used
    func_list.clear();
    func_list<<"Pruning_leaf_stop_at_tip"  //0
             <<"Pruning_leaf_not_tip"
             <<"graft_to_border"        //2
             <<"MergeSwc_two_leafnode"
             <<"sort_by_SOMA"      //4
             <<"Reestimate_radius"
             <<"pruningSwc_two_leafnode" //6
             <<"pruningSwc_small_length"
             <<"pruning_single_leaf_in_small_length" //8
             <<"find_leaf_need_prun"
             <<"Pruning_leaf_stop_at_tip_no_z"
             <<"return_tip_list"
             <<"pruning_leaf_node_in_background"
              ;
    need_tip[0]=1;//need tip
    need_tip[1]=1;
    need_tip[2]=1;
    need_tip[3]=1;
    need_tip[4]=0; // sort_by_SOMA   //not need tip
    need_tip[5]=0; // Reestimate_radius
    need_tip[6]=1;
    need_tip[7]=0;//pruningSwc_small_length
    need_tip[8]=0;//pruning_single_leaf_in_small_length
    need_tip[9]=1;//find_leaf_need_prun
    need_tip[10]=1;//
    need_tip[11]=1;//
    need_tip[12]=1;//
    return func_list;
}
NeuronTree neurontreepruneorgraft::pruning_single_leaf_in_small_length( NeuronTree nt)
{
    cout<<"before pruning_single_leaf_in_small_length neuronNum is "<<nt.listNeuron.size()<<endl;

    QList<NeuronSWC> list = nt.listNeuron;
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = list.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    for(V3DLONG i=0;i<neuronNum;i++)
    {
        if (childs[i].size()==0)
        {
            //pruning only leaf
            V3DLONG prun_leaf_number=i;
            flag[prun_leaf_number] = -1;
            V3DLONG parent_tip = getParent(prun_leaf_number,nt);
            if(parent_tip == 1000000000)
            {
                cout<<"something may be error ,the -1 point is a swc leaf point"<<endl;
                cout<<"before the function you need sort"<<endl;
                // need to change  it maybe not child but
                continue;
            }

//            cout<<"before loop ,parent is "<<parent_tip<<endl;
            MyMarker curr_node;
            curr_node.x=list.at(prun_leaf_number).x;
            curr_node.y=list.at(prun_leaf_number).y;
            curr_node.z=list.at(prun_leaf_number).z;
            MyMarker parent_node;
            parent_node.x=list.at(parent_tip).x;
            parent_node.y=list.at(parent_tip).y;
            parent_node.z=list.at(parent_tip).z;
            double index_tip = dist(curr_node,parent_node);

            while(childs[parent_tip].size()<2&&index_tip<pruing_all_leaf_fix_length)
            {
                flag[parent_tip] = -1;
                MyMarker curr_node;
                curr_node.x=list.at(parent_tip).x;
                curr_node.y=list.at(parent_tip).y;
                curr_node.z=list.at(parent_tip).z;
                MyMarker parent_node;
                parent_node.x=list.at(getParent(parent_tip,nt)).x;
                parent_node.y=list.at(getParent(parent_tip,nt)).y;
                parent_node.z=list.at(getParent(parent_tip,nt)).z;
                index_tip+=dist(curr_node,parent_node);

                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;

            }

//            int parent_index = parent_tip;
//            if(index_tip < pruing_all_leaf_fix_length)
//            {
//                flag[prun_leaf_number] = -1;
//                int parent_tip = getParent(prun_leaf_number,nt);
//                while(parent_tip!=parent_index)
//                {
//                    flag[parent_tip] = -1;
//                    parent_tip = getParent(parent_tip,nt);
//                    if(parent_tip == 1000000000)
//                        break;
//                }
//            }
    //             cout<<"finish select of need save of  "<<temp_i.at(i)<<" point that need prun"<<endl;

                cout<<"index_tip : "<<QString::number(index_tip).toUtf8().data()<<" less than"<<QString::number(pruing_all_leaf_fix_length).toUtf8().data()<<endl;
        }
    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 )
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }
   }
    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"after pruning_single_leaf_in_small_length neuronNum is "<<final_tree.listNeuron.size()<<endl;
    return final_tree;

}

NeuronTree neurontreepruneorgraft::pruning_single_leaf_in_small_length(LocationSimple lead_need_prun, NeuronTree nt, int length)
{
    cout<<"begin to pruingSwc_one_leafnode_stop_before_tip,x:"<<lead_need_prun.x
       <<" y:"<<lead_need_prun.y<<" z:"<<lead_need_prun.z<<endl;
    //init

    //find i
    QList<NeuronSWC> list = nt.listNeuron;
    vector<V3DLONG > temp_i;temp_i.clear();
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }


    for (V3DLONG i=0;i<list.size();i++)
    {
        if(list.at(i).x==lead_need_prun.x&&
           list.at(i).y==lead_need_prun.y&&
           list.at(i).z==lead_need_prun.z)
        {
            temp_i.push_back(i);
        }
    }
    if(temp_i.size()==0)
    {
        //no match
//        cout<< "there is no match,so no pruing,lead_need_prun x: "<<lead_need_prun.x
//            <<" y "<<lead_need_prun.y<<" z "<<lead_need_prun.z<<endl;
//        cout<<"nt.listNeuron.size() "<<nt.listNeuron.size()<<endl;
        if(flag) {delete[] flag; flag = 0;}
        return nt;
    }
    if(temp_i.size()>=2)
    {
        cout<< "there is extra match,so should pruing twice or more "<<endl;
    }

    for(V3DLONG i=0;i<temp_i.size();i++)
    {
        if (childs[temp_i.at(i)].size()==0)
        {
            //pruning only leaf
            V3DLONG prun_leaf_number=temp_i.at(i);
    //        flag[prun_leaf_number] = -1;
            V3DLONG parent_tip = getParent(prun_leaf_number,nt);
                if(parent_tip == 1000000000)
                {
                    cout<<"something may be error ,the -1 point is a swc leaf point"<<endl;
                    cout<<"before the function you need sort"<<endl;
                    // need to change  it maybe not child but
                    continue;
                }

    //            cout<<"before loop ,parent is "<<parent_tip<<endl;
                MyMarker curr_node;
                curr_node.x=list.at(prun_leaf_number).x;
                curr_node.y=list.at(prun_leaf_number).y;
                curr_node.z=list.at(prun_leaf_number).z;
                MyMarker parent_node;
                parent_node.x=list.at(parent_tip).x;
                parent_node.y=list.at(parent_tip).y;
                parent_node.z=list.at(parent_tip).z;

                double index_tip = dist(curr_node,parent_node);

                while(childs[parent_tip].size()<2&&index_tip<length)
                {
                    MyMarker curr_node;
                    curr_node.x=list.at(parent_tip).x;
                    curr_node.y=list.at(parent_tip).y;
                    curr_node.z=list.at(parent_tip).z;
                    MyMarker parent_node;
                    parent_node.x=list.at(getParent(parent_tip,nt)).x;
                    parent_node.y=list.at(getParent(parent_tip,nt)).y;
                    parent_node.z=list.at(getParent(parent_tip,nt)).z;
                    index_tip+=dist(curr_node,parent_node);

                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
                }

                int parent_index = parent_tip;
                if(index_tip < length)
                {
                    flag[prun_leaf_number] = -1;
                    int parent_tip = getParent(prun_leaf_number,nt);
                    while(parent_tip!=parent_index)
                    {
                        flag[parent_tip] = -1;
                        parent_tip = getParent(parent_tip,nt);
                        if(parent_tip == 1000000000)
                            break;
                    }
                }


    //             cout<<"finish select of need save of  "<<temp_i.at(i)<<" point that need prun"<<endl;

                cout<<"index_tip : "<<QString::number(index_tip).toUtf8().data()<<" less than"<<QString::number(length).toUtf8().data()<<endl;


        }

    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 )
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }

   }

    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"after pruning_single_leaf_in_small_length neuronNum is "<<final_tree.listNeuron.size()<<endl;
    return final_tree;
}
bool neurontreepruneorgraft::TMI_simple_argu_input()
{
     //set update the dialog
    QDialog * dialog = new QDialog();

    QGridLayout * layout = new QGridLayout();

    QSpinBox * based_distance_spinbox = new QSpinBox();
    based_distance_spinbox->setRange(1,100);
    based_distance_spinbox->setValue(based_distance);

    QDoubleSpinBox * online_confidece_min_spinbox = new QDoubleSpinBox();
    online_confidece_min_spinbox->setRange(0.2, 1.0);
    online_confidece_min_spinbox->setValue(online_confidece_min);

    QSpinBox * Max_number_other_swc_spinbox = new QSpinBox();
    Max_number_other_swc_spinbox->setRange(0,100);
    Max_number_other_swc_spinbox->setValue(Max_number_other_swc);

    QSpinBox * binary_threshold_spinbox = new QSpinBox();
    binary_threshold_spinbox->setRange(0,255);
    binary_threshold_spinbox->setValue(binary_threshold);

    QSpinBox * background_stop_num_spinbox = new QSpinBox();
    background_stop_num_spinbox->setRange(0,99);
    background_stop_num_spinbox->setValue(background_stop_num);

    QDoubleSpinBox * angle_swc_spinbox = new QDoubleSpinBox();
    angle_swc_spinbox->setRange(90.00, 180.00);
    angle_swc_spinbox->setValue(angle_swc);



    layout->addWidget(new QLabel("based_distance "),0,0);
    layout->addWidget(based_distance_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("Minimum_online_confidece"),1,0);
    layout->addWidget(online_confidece_min_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("Max_number_other_swc"),2,0);
    layout->addWidget(Max_number_other_swc_spinbox, 2,1,1,5);

    layout->addWidget(new QLabel("background threshold"),3,0);
    layout->addWidget(binary_threshold_spinbox, 3,1,1,5);

    layout->addWidget(new QLabel("background_stop_num"),4,0);
    layout->addWidget(background_stop_num_spinbox, 4,1,1,5);

//    layout->addWidget(new QLabel("angle_permit_other_swc"),5,0);
//    layout->addWidget(angle_swc_spinbox, 5,1,1,5);


    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,8,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //run the dialog
    if(dialog->exec() != QDialog::Accepted)
    {
        if (dialog)
        {
            delete dialog;
            dialog=0;
            cout<<"delete dialog"<<endl;
        }
        return false;
    }

    based_distance=based_distance_spinbox->value();
    online_confidece_min=online_confidece_min_spinbox->value();
    Max_number_other_swc=Max_number_other_swc_spinbox->value();
    binary_threshold=binary_threshold_spinbox->value();
    background_stop_num=background_stop_num_spinbox->value();
    angle_swc=angle_swc_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
    return true;

}
bool neurontreepruneorgraft::setSwcImg(NeuronTree nt)
{
    if(swcimg_datald!=NULL)
    {
        if(swcimg_datald) {delete []swcimg_datald; swcimg_datald = 0;}
        if(p4DImage_swc_img) {delete []p4DImage_swc_img; p4DImage_swc_img = 0;}//need it ?
    }

    swcimg_datald=NeuronTree2img_special(nt,sz);
    if(swcimg_datald==NULL)
    {
        cout<<"swcimg_datald==NULL"<<endl;
        return false;
    }
//    cout<<"begin to set date"<<endl;
//    cout<<"p4DImage.xdim"<<p4DImage->getXDim()<<endl;
    p4DImage_swc_img=new Image4DSimple;
    p4DImage_swc_img->setData(swcimg_datald,p4DImage);
//    cout<<"set date success"<<endl;
    if(swcimg_datald==NULL)
    {
        cout<<"swcimg_datald==NULL"<<endl;
        return false;
    }
    return true;
}
NeuronTree neurontreepruneorgraft::deal_not_match(NeuronTree nt,LandmarkList not_match)
{
    //init
    if(not_match.isEmpty()||nt.listNeuron.isEmpty())
    {
        cout<<"deal_not_match:input a wrong not_match or nt"<<endl;
        return nt;
    }
    cout<<"deal_not_match input not_match.size:"<<not_match.size()<<endl;
//    int img_size=sz[0]*sz[1]*sz[2];
    //begin to deal not match
    //create swc figure
    //input not_match  nt

//    swcimg_datald=NeuronTree2img_special(nt,sz);
////    cout<<"begin to set date"<<endl;
////    cout<<"p4DImage.xdim"<<p4DImage->getXDim()<<endl;
//    p4DImage_swc_img=new Image4DSimple;
//    p4DImage_swc_img->setData(swcimg_datald,p4DImage);
////    cout<<"set date success"<<endl;
//    if(swcimg_datald==NULL)
//    {
//        cout<<"img_swc==NULL"<<endl;
//        return nt;
//    }
    setSwcImg(nt);
//    cout<<"swcimg_datald is not NULL"<<endl;

    if(1)
    {
        //test for swc_img
        simple_saveimage_wrapper(*cb,"1234.tif",(unsigned char *)swcimg_datald,sz,1);
        cout<<"save SWC img to file"<<endl;
// //        stop here
//        return nt;
    }

//GUI input
//    cout<<"tp.GUI_input_argu();"<<endl;
//    if(!tp.GUI_input_argu())
//    {return nt;}
    //    cout<<"begin to Binarization background_threshold:"<<tp.background_threshold<<endl;

//    return nt;


    //input two img  1.  bimg_datald  2.swc_img
    if(1)
    {
        for(int i=0;i<not_match.size();i++)
        {
            cout<<endl<<endl;
            nt=deal_not_match_points(nt,not_match.at(i));
        }
    }
    else
    {//test
        cout<<"deal_not_match_points once "<<endl;
        int control=1;
        nt=deal_not_match_points(nt,not_match.at(control));
    }
    cout<<"deal_not_match over "<<endl;
    cb->setLandmark(cb->currentImageWindow(),temp_for_debug);

    return nt;

}
NeuronTree neurontreepruneorgraft::deal_not_match_points(NeuronTree nt,LocationSimple not_match_p)
{
    //init
    if(nt.listNeuron.isEmpty())
    {
        cout<<"deal_not_match_points:input a wrong  nt"<<endl;
        v3d_msg("deal_not_match_points:input a wrong  nt");
        return nt;
    }
    cout<<"deal_not_match_points not_match_p x:"<<not_match_p.x<<"  not_match_p.y:"<<not_match_p.y<<" not_match_p.z:"<<not_match_p.z<<endl;

    cout<<" nt.listNeuron.size() "<<nt.listNeuron.size()<<endl;
    cout<<" nt.hashNeuron.size() "<<nt.hashNeuron.size()<<endl;
    QVector<QVector<V3DLONG> > childs;
    childs = QVector< QVector<V3DLONG> >(nt.listNeuron.size(), QVector<V3DLONG>() );

    for (V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    //input two img  1.  bimg_datald  2.swc_img
    V3DLONG temp=-1;
    int current_type=-1;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        if(nt.listNeuron.at(i).x==not_match_p.x&&nt.listNeuron.at(i).y==not_match_p.y&&nt.listNeuron.at(i).z==not_match_p.z&&childs[i].isEmpty())
        {
            temp=i;
            break;
        }
    }
    current_type=nt.listNeuron.at(temp).type;
    if(temp<0||current_type<0)
    {
        cout<<"deal_not_match_points temp :"<<temp<<endl;
        cout<<"deal_not_match_points current_type== "<<current_type<<endl;
        return nt;
    }
//    cout<<"deal_not_match_points not_match_p x:"<<not_match_p.x<<"not_match_p.y"<<not_match_p.y<<"not_match_p.z"<<not_match_p.z<<endl;
//    cout<<"It's "<<temp<<"th point"<<endl;
    cout<<" temp_type== "<<current_type<<endl;


    // ray-burst
    //prepare
//    based_distance=15;
    vector<vector<float> > x_dis(16*32,vector<float>(100)), y_dis(16*32,vector<float>(100)),z_dis(16*32,vector<float>(100));
    for(int len = 1; len < based_distance; len++)
    {
        int mm = 0;
        for(int n = 1; n <= 16; n++ )
        {
            for(int k = 0; k <=31; k++ )
            {
             x_dis[mm][len-1] = len * sin(PI * n/16) * cos(PI * k / 16);
             y_dis[mm][len-1] = len * sin(PI * n/16) * sin(PI * k / 16);
             z_dis[mm][len-1] = len * cos(PI * n/16);
             mm++;
            }
        }
    }
//    cout<<"create 3D_ray success"<<endl;

    vector<float> x_loc; x_loc.clear();
    vector<float> y_loc; y_loc.clear();
    vector<float> z_loc; z_loc.clear();
    vector<float> type_loc; type_loc.clear();//0 for  back;   1  fore;   2  swc;  3 transfer from 2 to 1
    int type_count_back=0;
    int type_count_swc=0;
//    cout<<"begin to get "<<endl;
    for(int  i =0;i<16*32;i++)
    {
        int count_bcak=0;
//        cout<<"i "<<i<<endl;
        for(int j=0; j < based_distance; j+=1)
        {
            if(not_match_p.x+x_dis[i][j]<0||not_match_p.x+x_dis[i][j]>=sz[0]
                ||not_match_p.y+y_dis[i][j]<0||not_match_p.y+y_dis[i][j]>=sz[1]
                ||not_match_p.z+z_dis[i][j]<0||not_match_p.z+z_dis[i][j]>=sz[2])
            {
                //out of bound
               continue;
            }

            double pixe=p4DImage_bimg->getValueUINT8(not_match_p.x+x_dis[i][j],not_match_p.y+y_dis[i][j],not_match_p.z+z_dis[i][j],0);
            double swc_type=p4DImage_swc_img->getValueUINT8(not_match_p.x+x_dis[i][j],not_match_p.y+y_dis[i][j],not_match_p.z+z_dis[i][j],0);
            if(pixe>0)
            {
//                x_loc.push_back(not_match_p.x+x_dis[i][j]);
//                y_loc.push_back(not_match_p.y+y_dis[i][j]);
//                z_loc.push_back(not_match_p.z+z_dis[i][j]);
//                type_loc.push_back(1);
                // cout<<"the x location is "<<x_location-1+ray_x[i][j]<<" "<<"the y location is "<<y_location-1+ray_y[i][j]<<endl;

                if(swc_type!=current_type&&swc_type!=1&&swc_type!=0)
                {//前景且非本身
                    //stop
                    x_loc.push_back(not_match_p.x+x_dis[i][j]);
                    y_loc.push_back(not_match_p.y+y_dis[i][j]);
                    z_loc.push_back(not_match_p.z+z_dis[i][j]);
                    type_loc.push_back(2);
//                    cout<<"swc_type!=current_type find!!! swc_type:"<<swc_type<<" current_type :"<<current_type<<endl;
                    type_count_swc++;
                    break;
                }
            }
            else if(pixe==0)//background
            {
                count_bcak++;
                if(count_bcak>background_stop_num/*||j>background_stop_num*/)
                {
                    //stop
                    //back
                    x_loc.push_back(not_match_p.x+x_dis[i][j]);
                    y_loc.push_back(not_match_p.y+y_dis[i][j]);
                    z_loc.push_back(not_match_p.z+z_dis[i][j]);
                    type_loc.push_back(0);
                    type_count_back++;
                    break;
                }
                else
                {
                    if(swc_type!=current_type&&swc_type!=1&&swc_type!=0)
                    {//背景且非本身
                        //stop
                        x_loc.push_back(not_match_p.x+x_dis[i][j]);
                        y_loc.push_back(not_match_p.y+y_dis[i][j]);
                        z_loc.push_back(not_match_p.z+z_dis[i][j]);
                        type_loc.push_back(2);
    //                    cout<<"swc_type!=current_type find!!! swc_type:"<<swc_type<<" current_type :"<<current_type<<endl;
                        type_count_swc++;
                        break;
                    }
                }
            }
        }
    }
    cout<<"now to calculate confidence or pruning. number of other swc:"<<type_count_swc<<endl;
    if(type_count_swc==0)
    {
        //need to save do nothing
        //cal oc
//        cout<<"now to count confidence "<<endl;
        float confidence=online_confidece_one_branch(temp,nt,this->bimg_datald,sz);
        cout<<"no other and confice score is "<<confidence<<" "<<endl;
        if(confidence<online_confidece_min)
        {
            cout<<" confidence less than "<<online_confidece_min<<"， so need pruning  "<<endl;
            nt=this->pruingSwc_one_leafnode_stop_before_tip(not_match_p,nt);
        }
        else
        {//do nothing
//            cout<<" confidence "<<confidence<<endl;
        }
    }
    else if(type_count_swc>0)
    {
        //need to pruning  maybe
//        int temp_type_count_swc=type_count_swc;s
//         cout<<"now to count cal  pruning"<<endl;
        for(V3DLONG i = 0; i < x_loc.size(); i++)
        {
            if(1&&type_loc.at(i)==2)
            {
                //与其parent 成钝角  或120°以上？  或许
                XYZ point,parent;
                parent.x=nt.listNeuron.at(nt.hashNeuron.value(temp)).x-nt.listNeuron.at(temp).x;
                parent.y=nt.listNeuron.at(nt.hashNeuron.value(temp)).y-nt.listNeuron.at(temp).y;
                parent.z=nt.listNeuron.at(nt.hashNeuron.value(temp)).z-nt.listNeuron.at(temp).z;
                point.x=x_loc.at(i)-not_match_p.x;
                point.y=y_loc.at(i)-not_match_p.y;
                point.z=z_loc.at(i)-not_match_p.z;
                float cos_vector=dot(parent,point)/norm(parent) / (norm(point));

                if(cos_vector<-0.5)//-120°   //typical  -0.5   120°   -0.866   150°
                {
                    //钝角
//                    x_loc[i]=x_loc.back();
//                    y_loc[i]=y_loc.back();
//                    z_loc[i]=z_loc.back();
//                    type_loc[i]=type_loc.back();
//                    i--;
//                    cout<<"change one point to 3"<<endl;
                    type_loc[i]=3;

                    type_count_swc--;
                }
            }
        }
        if(type_count_swc>Max_number_other_swc)
        {
            //need pruing
            cout<<"still need pruning type_count_swc:"<<type_count_swc<<endl;
            nt=this->pruingSwc_one_leafnode_stop_before_tip(not_match_p,nt);

        }else
        {
            cout<<"not need pruning now type_count_swc:"<<type_count_swc<<endl;
            float confidence=online_confidece_one_branch(temp,nt,this->bimg_datald,sz);
            if(confidence<online_confidece_min)
            {
                cout<<"no other and confice score is "<<confidence<<endl;
                nt=this->pruingSwc_one_leafnode_stop_before_tip(not_match_p,nt);
            }
        }

    }
    if(1)
    {
        RGBA8 red; red.r=255; red.g=0; red.b=0;
        RGBA8 green; green.r=0; green.g=255; green.b=0;
        RGBA8 blue; blue.r=0; blue.g=0; blue.b=255;
        RGBA8 yellow;yellow.r=255; yellow.g=255; yellow.b=0;
        LandmarkList toshow;
        LocationSimple s;
        for(V3DLONG i = 0; i < x_loc.size(); i++)
        {
            s.x=  x_loc[i];
            s.y = y_loc[i];
            s.z = z_loc[i];
            s.radius = 1;
            if(1&&type_loc.at(i)==0)//back
            {s.color = red;
            toshow<<s;}
            else if(1&&type_loc.at(i)==2)//swc
            {
                s.color = blue;
                toshow<<s;
            }else if(0&&type_loc.at(i)==1) //fore
            {
                s.color = green;
                toshow<<s;
            }
            else if(1&&type_loc.at(i)==3) //fore
            {
                s.color = yellow;
                toshow<<s;
            }
        }
        temp_for_debug +=toshow;
//        cb->setLandmark(cb->currentImageWindow(),toshow);
    }



    return nt;

}


LandmarkList neurontreepruneorgraft::find_leaf_need_prun(NeuronTree nt,LandmarkList tip_list,float threshold,float threshold2,bool ignore_z)
{
    LandmarkList need_prun_tip;
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return need_prun_tip;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }

//    NeuronTree temp_tree=this->sort_neuron(nt);
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(nt);
//    for(V3DLONG num=0;num<swc_tip_list.size();num++)
//    {
//        cout<<num+1<<"th point swc_tip_list x:"<<swc_tip_list.at(num).x<<" y "<<swc_tip_list.at(num).y<<" z "<<swc_tip_list.at(num).z<<endl;
//    }
    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return need_prun_tip;
    }
//    else
//    {
//        cout<<"there is swc_tip_Marker :"<<swc_tip_list.size()<<endl;
//    }
    QVector<QVector<V3DLONG> > Nearest;
    Nearest = QVector< QVector<V3DLONG> >(swc_tip_list.size(), QVector<V3DLONG>(2) );
    //match have the same size as tip_list
    // it save  the nearest and Second near tip
    QVector<QVector<float> > Nearest_dist;
    Nearest_dist = QVector< QVector<float> >(swc_tip_list.size(), QVector<float>(2) );
    //match have the same size as tip_list
    // it save  the nearest and Second near tip

    QVector<QVector<V3DLONG> > Match;
    Match = QVector< QVector<V3DLONG> >(tip_list.size(), QVector<V3DLONG>() );
    //match have the same size as tip_list
    // it save  which match the tip

    RGBA8 blue,pink,green;
    blue.r=0;
    blue.g=0;
    blue.b=255;

    pink.r=255;
    pink.g=100;
    pink.b=0;

    green.r=0;
    green.g=255;
    green.b=150;
    cout<<"threshold:"<<threshold<<endl;
//    LandmarkList need_not_need_prun_tip;
    V3DLONG count=1;
    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {

        LocationSimple b;
        LocationSimple a;
        a.x=swc_tip_list.at(num).x;
        a.y=swc_tip_list.at(num).y;
        a.z=swc_tip_list.at(num).z;
//        a.name="leaf_need_prun";
//        a.color=pink;
        float mindist=10000;
        float mindist_2=20000;
        V3DLONG minIndex=-1;
        V3DLONG minIndex_2=-1;
        for(V3DLONG num2=0;num2<tip_list.size();num2++)
        {
            b.x=tip_list.at(num2).x;
            b.y=tip_list.at(num2).y;
            b.z=tip_list.at(num2).z;
            float dist;
            if(ignore_z)
            {
                dist=sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) );
            }
            else
            {
                dist=sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
            }

            if(dist<mindist)
            {
                mindist_2=mindist;
                minIndex_2=minIndex;
                // save value

                mindist=dist;
                minIndex=num2;

            }
            else if(dist<mindist_2)
            {
                mindist_2=mindist;
                minIndex_2=minIndex;
            }

        }

        Nearest[num][0]=minIndex;
        Nearest[num][1]=minIndex_2;
        Nearest_dist[num][0]=mindist;
        Nearest_dist[num][1]=mindist_2;

//        cout<<endl;
//        cout<<"num loop is ok "<<num<<endl;
    }
    // deal is above
    cout<<"deal is over"<<endl;
    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {
        LocationSimple a;
        a.x=swc_tip_list.at(num).x;
        a.y=swc_tip_list.at(num).y;
        a.z=swc_tip_list.at(num).z;
        if(Nearest_dist[num][0]>=threshold2)
        {
//            cout<<count<<"th point need_prun_tip x:"<<a.x<<" y "<<a.y<<" z "<<a.z<<endl;
            count++;
//            cout<<"nearest tip point x "<<b.x<<" y "<<b.y<<" z "<<b.z<<" with mindist "<<mindist<<endl;
//            Match[minIndex].push_back(num);

            a.name="leaf_need_prun";
            a.color=pink;
//            need_prun_tip.push_back(a);
        }
        else
        {
            //
            if(Nearest_dist[num][0]<threshold2)
            {//
                Match[Nearest[num][0]].push_back(num);

            }
//            //it means it match some
//            //
////            cout<<num<<"th point need_prun_tip x:"<<a.x<<" y "<<a.y<<" z "<<a.z<<endl;
////            cout<<"nearest tip point x "<<b.x<<" y "<<b.y<<" z "<<b.z<<" with mindist "<<mindist<<endl;

            if(0)
            {
                //debug
                //            a.color=blue;
                //            need_not_need_prun_tip.push_back(a);
            }

        }
//        cout<<endl;

    }
    cout<<"deal over"<<endl;

    QVector<V3DLONG> need_change_to_unmatch;
    for(V3DLONG i=0;i<tip_list.size();i++)
    {
        if(Match[i].size()>1)
        {//多重匹配
            //一个末梢点被多次匹配
            //one tip is matched twice or more
            //only nearest can be match or smaller than threshold(this is a stric condition)

            V3DLONG minIndex=Match[i][0];//
            float mindist=Nearest_dist[minIndex][0];
            for(V3DLONG j=1;j<Match[i].size();j++)
            {
                //find the nearest to the tip
                if(mindist>Nearest_dist[Match[i][j]][0])
                {
                    mindist=Nearest_dist[minIndex][0];
                    minIndex=Match[j][0];
                }
            }
            for(V3DLONG j=0;j<Match[i].size();j++)
            {// other will match other tip or
                if(Match[i][j]!=minIndex&&Nearest_dist[Match[i][j]][0]>threshold)
                {
                    V3DLONG index=Match[i][j];
                    need_change_to_unmatch.push_back(index);
                    Match[i].remove(j);
                    j--;
                    //need  change to second

                    if(Nearest_dist[index][1]>threshold2)
                    {
                        //no match
//                        need_change_to_unmatch.push_back(index);
//                        Match[i].remove(j);
//                        j--;
                        continue;
                    }
                    else if(Nearest_dist[index][1]<threshold2)
                    {//Nearest[Match[i][j]][1]
                        //index
                        V3DLONG index_second_near=Nearest[index][1];
                        if(Match[index_second_near].size()>0)
                        {
                            if(Match[index_second_near].size()==1)
                            {
                                float dist_index,dist_index_already_match;
                                dist_index=Nearest_dist[index][1];
                                dist_index_already_match=Nearest_dist[Match[index_second_near][0]][0];
                                if(dist_index<dist_index_already_match)
                                {
                                    //match to
                                    Match[index_second_near].remove(0);// remove the first
                                    Match[index_second_near].push_back(index);
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            else
                            { //it means the second_near has been matched twice or more
                                cout<<"index_second_near is already been matched twice or more,leaf index:"<<index<<" index_second_near: "<<index_second_near<<endl;

                                continue;
                            }

                        }
                        else
                        {
                            Match[index_second_near].push_back(index);
                        }



                    }
                }
            }
        }

    }
    //out
    char * flag;
    flag=new char[swc_tip_list.size()];
    for(int i=0;i<swc_tip_list.size();i++)
    {
        flag[i]=0;
        // need pruning
    }

    for(V3DLONG j=0;j<Match.size();j++)
    {
//        cout<<"the "<<j<<"th point match "<<Match[j].size()<<" point"<<endl;
        for(V3DLONG k=0;k<Match[j].size();k++)
        {
//            cout<<"/tmatch "<<k<<"th point at:"<<Match[j][k];
            char color_flag=1;
            if(Match[j].size()>=2)
            {
                color_flag=2;
            }
            else
            {
                color_flag=1;
            }
            if(Match[j][k]>=0&&Match[j][k]<swc_tip_list.size())
            {
                flag[Match[j][k]]=color_flag;
            }
            else
            {
                 cout<<"wrong putput in Match[j][k] j:"<<j<<",k:"<<k<<",Match[j][k]"<<Match[j][k]<<endl;
            }

        }
    }
    V3DLONG num1=0,num2=0,num0=0;

    LandmarkList not_match;
    LandmarkList double_match;
    LandmarkList match;
    for(int i=0;i<swc_tip_list.size();i++)
    {
        LocationSimple a;
        a.x=swc_tip_list.at(i).x;
        a.y=swc_tip_list.at(i).y;
        a.z=swc_tip_list.at(i).z;
        if(flag[i]==0)
        {
            a.name="leaf_need_prun";
            a.color=pink;
            need_prun_tip.push_back(a);
            not_match.push_back(a);
            num0++;
        }
        else if(flag[i]==1)
        {
            a.name="leaf_matched";
            a.color=green;
            if(TMI_show_match_result)
            {
                need_prun_tip.push_back(a);
            }
            match.push_back(a);
            num1++;
        }
        else
        {
            a.name="leaf_double_matched";
            a.color=blue;
            if(TMI_show_match_result)
            {
                need_prun_tip.push_back(a);
            }
            double_match.push_back(a);
            num2++;

        }

    }


    cout<<"there is need_prun_tip:"<<need_prun_tip.size()<<endl;
    cout<<"there is leaf_need_prun: flag0:"<<num0<<endl;
    cout<<"there is leaf_matched flag1:"<<num1<<endl;
    cout<<"there is leaf_double_matched flag2:"<<num2<<endl;
//    need_prun_tip.append(need_not_need_prun_tip);
    return need_prun_tip;

}
LandmarkList neurontreepruneorgraft::find_leaf_need_prun(NeuronTree nt)
{
    //not ok
    LandmarkList need_prun_tip;
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return need_prun_tip;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }
    return find_leaf_need_prun(nt,tip_list);

////    NeuronTree temp_tree=this->sort_neuron(nt);
//    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(nt);
//    if(swc_tip_list.size()<1)
//    {
//        cout<<"There is no tip in swc"<<endl;
//        return need_prun_tip;
//    }
//    else
//    {
//        cout<<"there is swc_tip_Marker :"<<swc_tip_list.size()<<endl;
//    }

//    RGBA8 red,pink;
//    red.r=255;
//    red.g=0;
//    red.b=0;
//    pink.r=255;
//    pink.g=100;
//    pink.b=0;

//    for(V3DLONG num=0;num<swc_tip_list.size();num++)
//    {

//        LocationSimple true_tip;
//        LocationSimple tippoint;
//        tippoint.x=swc_tip_list.at(num).x;
//        tippoint.y=swc_tip_list.at(num).y;
//        tippoint.z=swc_tip_list.at(num).z;
//        tippoint.name="leaf_need_prun";
//        if(!tp.TipDetect_onePoint_neighbor_no_z(tippoint,true_tip,5))
//        {
////            after_prun_tree=pruingSwc_one_leafnode(tippoint,after_prun_tree);
//            //

//            if(tp.isForeground(tippoint))
//            {
//                tippoint.color=red;
//            }
//            else
//            {
//                tippoint.color=pink;
//            }

////            tippoint.name=;
//            need_prun_tip.push_back(tippoint);
//        }
//    }

//    cout<<"there is need_prun_tip:"<<need_prun_tip.size()<<endl;
//    return need_prun_tip;

}
NeuronTree neurontreepruneorgraft::sort_neuron( NeuronTree nt)
{
    V3DLONG neuronNum = nt.listNeuron.size();
    if(neuronNum<=0)
    {
        cout<<"nt.listNeuron.size()<=0 sort_neuron is over "<<endl;
        return nt;
    }
    else
    {
        cout<<"nt.listNeuron.size() "<<neuronNum<<" sort_neuron is begin "<<endl;
    }

   QVector<QVector<V3DLONG> > childs;
   childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

   for (V3DLONG i=0;i<neuronNum;i++)
   {
       V3DLONG par = nt.listNeuron[i].pn;
       if (par<0) continue;
       childs[nt.hashNeuron.value(par)].push_back(i);
   }
   V3DLONG temp_rootid=-1;
   V3DLONG max_size=-1;
   V3DLONG rootid;
   for(V3DLONG i=0;i<neuronNum;i++)
   {
      if(max_size<childs[i].size())
      {
          max_size=childs[i].size();
          temp_rootid=nt.listNeuron[i].n;
      }
   }
//   cout<<"max_size "<<max_size<<endl;
   if(temp_rootid==-1)
   {
       rootid=VOID;
   }
   else
   {
       rootid=temp_rootid;
   }

   NeuronTree resulttree;
   cout<<" sort swc rootid is"<<rootid<<endl;
   if(SortSWC(nt.listNeuron,resulttree.listNeuron,rootid,1000000000))
   {
       //success..

        cout<<"after sort resulttree size "<<resulttree.listNeuron.size()<<" sort_neuron is end "<<endl;
        QString temp_trasfer_name="sort_neuron.swc";
        writeSWC_file(temp_trasfer_name,resulttree);
        resulttree=readSWC_file(temp_trasfer_name);

       return resulttree;
   }
   else
   {
       //fail
       cout<<"fail to sort swc"<<endl;
       return nt;
   }
}

NeuronTree neurontreepruneorgraft::mergeSwc_two_leafnode( NeuronTree nt)
{
    //did not test
    cout<<"begin the function of mergeSwc_two_leafnode"<<endl;
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    QVector<QVector<V3DLONG> >childs_p = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
//        childs_p[i].push_back(-1);
    }
    //get every leaf node of every branch
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs[i].size()==0)
       {
           int parent_tip = getParent(i,nt);
           if(parent_tip == 1000000000)
               continue;

           while(childs[parent_tip].size()<2)
           {
               parent_tip = getParent(parent_tip,nt);
               if(parent_tip == 1000000000)
                   break;
           }
           if(parent_tip == 1000000000)
               continue;
           childs_p[parent_tip].push_back(i);
       }
    }

//
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs[i].size()>=2)
       {
           //it means
           for(int branch_1=0;branch_1<childs_p[i].size();branch_1++)
           {
               for(int branch_2=branch_1+1;branch_2<childs_p[i].size();branch_2++)
               {
                   //childs_p[i][branch_1]    childs_p[i][branch_2]
                   V3DLONG leaf_a=childs_p[i][branch_1];
                   V3DLONG leaf_b=childs_p[i][branch_2];
                   NeuronSWC leaf_a_point=nt.listNeuron.at(leaf_a);
                   NeuronSWC leaf_b_point=nt.listNeuron.at(leaf_b);
                   LocationSimple leaf_a_p,leaf_b_p;
                   leaf_a_p.x=leaf_a_point.x;leaf_b_p.x=leaf_b_point.x;
                   leaf_a_p.y=leaf_a_point.y;leaf_b_p.y=leaf_b_point.y;
                   leaf_a_p.z=leaf_a_point.z;leaf_b_p.z=leaf_b_point.z;
                   LocationSimple nearest_tip_a,nearest_tip_b;
                   nearest_tip_a=tp.Nearest_tip(leaf_a_p);
                   nearest_tip_b=tp.Nearest_tip(leaf_b_p);
                   if(nearest_tip_a.x==nearest_tip_b.x
                           &&nearest_tip_a.y==nearest_tip_b.y
                           &&nearest_tip_a.z==nearest_tip_b.z)
                   {
                       //it means the two point need delete
                       flag[leaf_a] = -1;
                       int parent_tip = getParent(leaf_a,nt);
                       if(parent_tip == 1000000000)
                           break;
                       while(childs[parent_tip].size()<2)
                       {
                           flag[parent_tip] = -1;
                           parent_tip = getParent(parent_tip,nt);
                           if(parent_tip == 1000000000)
                               break;
                       }

                       flag[leaf_b] = -1;
                       parent_tip = getParent(leaf_b,nt);
                       if(parent_tip == 1000000000)
                           break;
                       while(childs[parent_tip].size()<2)
                       {
                           flag[parent_tip] = -1;
                           parent_tip = getParent(parent_tip,nt);
                           if(parent_tip == 1000000000)
                               break;
                       }
                   }
                   else
                   {
                       continue;
                   }
                   //pruning is over
                   //begin to merge
                   NeuronTree nt1;
                   nt1.listNeuron.clear();nt1.hashNeuron.clear();
                   LocationSimple Bifurcate_point;
                   Bifurcate_point.x=nt.listNeuron.at(i).x;
                   Bifurcate_point.y=nt.listNeuron.at(i).y;
                   Bifurcate_point.z=nt.listNeuron.at(i).z;
                   trace_one_pt_to_one_points_shortestdist(Bifurcate_point,nearest_tip_a,nt1);
                   nt=merge_two_neuron(nt,nt1);
               }
           }
       }
    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int j=0;j<neuronNum;j++)
    {
        if(flag[j] == 1)
        {
            after_prunning_swc.push_back(before_prunning_swc[j]);
        }
   }
    for(int j=neuronNum;j<before_prunning_swc.size();j++)
    {
        after_prunning_swc.push_back(before_prunning_swc[j]);
    }
    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    return final_tree;
}
//NeuronTree neurontreepruneorgraft::pruningSwc_low_confidence(NeuronTree nt)
//{
//    //input NeuronTree  ,small length
//    //
//    cout<<"pruningSwc_small_length before neuronNum is "<<nt.listNeuron.size()<<endl;

//    QVector<QVector<V3DLONG> > childs;
//    V3DLONG neuronNum = nt.listNeuron.size();
//    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
//    V3DLONG *flag = new V3DLONG[neuronNum];
//    double *segment_length = new double[neuronNum];
//    V3DLONG *parent_id = new V3DLONG[neuronNum];

//    for (V3DLONG i=0;i<neuronNum;i++)
//    {
//        flag[i] = 1;
//        segment_length[i] = 100000.00;
//        parent_id[i] = -1;
//        V3DLONG par = nt.listNeuron[i].pn;
//        if (par<0) continue;
//        childs[nt.hashNeuron.value(par)].push_back(i);
//    }


//    QList<NeuronSWC> list = nt.listNeuron;
//    for (int i=0;i<list.size();i++)
//    {
//        float confidence_score=0;//online confidence
//        if (childs[i].size()==0)
//        {
//            int parent_tip = getParent(i,nt);
//            MyMarker curr_node, parent_node;
//            curr_node.x = list.at(i).x;
//            curr_node.y = list.at(i).y;
//            curr_node.z = list.at(i).z;
//            if(parent_tip == 1000000000)
//                continue;


//            parent_node.x = list.at(parent_tip).x;
//            parent_node.y = list.at(parent_tip).y;
//            parent_node.z = list.at(parent_tip).z;
//            double index_tip = dist(curr_node,parent_node);


//            while(childs[parent_tip].size()<2)
//            {
//                MyMarker curr_node, parent_node;

//                curr_node.x = list.at(parent_tip).x;
//                curr_node.y = list.at(parent_tip).y;
//                curr_node.z = list.at(parent_tip).z;

//                parent_node.x = list.at(getParent(parent_tip,nt)).x;
//                parent_node.y = list.at(getParent(parent_tip,nt)).y;
//                parent_node.z = list.at(getParent(parent_tip,nt)).z;

//                index_tip += dist(curr_node,parent_node);

//                parent_tip = getParent(parent_tip,nt);

//                if(parent_tip == 1000000000)
//                    break;

//             }

//            int parent_index = parent_tip;

//            if(index_tip < small_length)
//            {
//                flag[i] = -1;
//                segment_length[i] = index_tip;
//                parent_id[i] = parent_index;
//                int parent_tip = getParent(i,nt);
//                while(childs[parent_tip].size()<2)
//                {
//                    flag[parent_tip] = -1;
//                    segment_length[parent_tip] = index_tip;
//                    parent_id[parent_tip] = parent_index;
//                    parent_tip = getParent(parent_tip,nt);
//                    if(parent_tip == 1000000000)
//                        break;
//                }
//                if(segment_length[parent_tip] > index_tip)
//                    segment_length[parent_tip]  = index_tip;
//            }
//        }
//    }
//    cout<<"test2 "<<endl;

//    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
//    vector<MyMarker*> after_prunning_swc;
//    for (int i=0;i<before_prunning_swc.size();i++)
//    {
//        if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
//        {
//            after_prunning_swc.push_back(before_prunning_swc[i]);
//        }

//   }

//    if(flag) {delete[] flag; flag = 0;}
//    if(segment_length) {delete[] segment_length; segment_length = 0;}
//    if(parent_id) {delete[] parent_id; parent_id = 0;}

//    // output as  after_prunning_swc
//    NeuronTree final_tree;
//    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
//    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
//    {
//        delete before_prunning_swc.at(i);
//    }
//    cout<<"after neuronNum is "<<final_tree.listNeuron.size()<<endl;
//    return final_tree;

//    // output as  after_prunning_swc

//}

NeuronTree neurontreepruneorgraft::pruningSwc_small_length(NeuronTree nt)
{
    //input NeuronTree  ,small length
    //
    cout<<"pruningSwc_small_length before neuronNum is "<<nt.listNeuron.size()<<endl;

    QVector<QVector<V3DLONG> > childs;


    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    double *segment_length = new double[neuronNum];
    V3DLONG *parent_id = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        segment_length[i] = 100000.00;
        parent_id[i] = -1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }


    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            int parent_tip = getParent(i,nt);
            MyMarker curr_node, parent_node;
            curr_node.x = list.at(i).x;
            curr_node.y = list.at(i).y;
            curr_node.z = list.at(i).z;
            if(parent_tip == 1000000000)
                continue;


            parent_node.x = list.at(parent_tip).x;
            parent_node.y = list.at(parent_tip).y;
            parent_node.z = list.at(parent_tip).z;
            double index_tip = dist(curr_node,parent_node);


            while(childs[parent_tip].size()<2)
            {
                MyMarker curr_node, parent_node;

                curr_node.x = list.at(parent_tip).x;
                curr_node.y = list.at(parent_tip).y;
                curr_node.z = list.at(parent_tip).z;

                parent_node.x = list.at(getParent(parent_tip,nt)).x;
                parent_node.y = list.at(getParent(parent_tip,nt)).y;
                parent_node.z = list.at(getParent(parent_tip,nt)).z;

                index_tip += dist(curr_node,parent_node);

                parent_tip = getParent(parent_tip,nt);

                if(parent_tip == 1000000000)
                    break;

             }

            int parent_index = parent_tip;

            if(index_tip < small_length)
            {
                flag[i] = -1;
                segment_length[i] = index_tip;
                parent_id[i] = parent_index;
                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    segment_length[parent_tip] = index_tip;
                    parent_id[parent_tip] = parent_index;
                    parent_tip = getParent(parent_tip,nt);
                    if(parent_tip == 1000000000)
                        break;
                }
                if(segment_length[parent_tip] > index_tip)
                    segment_length[parent_tip]  = index_tip;
            }
        }
    }
    cout<<"test2 "<<endl;

    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 || (flag[i] != 1 && (segment_length[i] > segment_length[parent_id[i]])))
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }

   }

    if(flag) {delete[] flag; flag = 0;}
    if(segment_length) {delete[] segment_length; segment_length = 0;}
    if(parent_id) {delete[] parent_id; parent_id = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"after neuronNum is "<<final_tree.listNeuron.size()<<endl;
    return final_tree;

    // output as  after_prunning_swc

}
NeuronTree neurontreepruneorgraft::pruningSwc_two_leafnode(NeuronTree nt)
{
    cout<<"begin the function of mergeSwc_two_leafnode"<<endl;
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    QVector<QVector<V3DLONG> >childs_p = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    cout<<"before neuronNum is "<<neuronNum<<endl;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
//        childs_p[i].push_back(-1);
    }
    //get every leaf node of every branch
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs[i].size()==0)
       {
           int parent_tip = getParent(i,nt);
           if(parent_tip == 1000000000)
               continue;

           while(childs[parent_tip].size()<2)
           {
               parent_tip = getParent(parent_tip,nt);
               if(parent_tip == 1000000000)
                   break;
           }
           if(parent_tip == 1000000000)
               continue;
           childs_p[parent_tip].push_back(i);
       }
    }

    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs[i].size()>=2)
       {
           //it means
           for(int branch_1=0;branch_1<childs_p[i].size();branch_1++)
           {
               for(int branch_2=branch_1+1;branch_2<childs_p[i].size();branch_2++)
               {
                   //childs_p[i][branch_1]    childs_p[i][branch_2]
                   V3DLONG leaf_a=childs_p[i][branch_1];
                   V3DLONG leaf_b=childs_p[i][branch_2];
                   NeuronSWC leaf_a_point=nt.listNeuron.at(leaf_a);
                   NeuronSWC leaf_b_point=nt.listNeuron.at(leaf_b);
//                   NeuronSWC leaf_b_point=nt.listNeuron.at(leaf_b);
                   LocationSimple leaf_a_p,leaf_b_p;
                   leaf_a_p.x=leaf_a_point.x;leaf_b_p.x=leaf_b_point.x;
                   leaf_a_p.y=leaf_a_point.y;leaf_b_p.y=leaf_b_point.y;
                   leaf_a_p.z=leaf_a_point.z;leaf_b_p.z=leaf_b_point.z;
                   LocationSimple nearest_tip_a,nearest_tip_b;
                   nearest_tip_a=tp.Nearest_tip(leaf_a_p);
                   nearest_tip_b=tp.Nearest_tip(leaf_b_p);
                   if(nearest_tip_a.x==nearest_tip_b.x&&nearest_tip_a.y==nearest_tip_b.y&&nearest_tip_a.z==nearest_tip_b.z)
                   {
                       //it means the two point need delete
                       flag[leaf_a] = -1;
                       int parent_tip = getParent(leaf_a,nt);
                       if(parent_tip == 1000000000)
                           break;
//                           cout<<"before loop ,parent is "<<parent_tip<<" childs[parent_tip] is "<<childs[parent_tip].size()<<endl;

                       while(childs[parent_tip].size()<2)
                       {
                           flag[parent_tip] = -1;
                           parent_tip = getParent(parent_tip,nt);
                           if(parent_tip == 1000000000)
                               break;
                       }

                       flag[leaf_b] = -1;
                       parent_tip = getParent(leaf_b,nt);
                       if(parent_tip == 1000000000)
                           break;
                       while(childs[parent_tip].size()<2)
                       {
                           flag[parent_tip] = -1;
                           parent_tip = getParent(parent_tip,nt);
                           if(parent_tip == 1000000000)
                               break;
                       }
                   }
               }
           }
       }
    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int j=0;j<before_prunning_swc.size();j++)
    {
        if(flag[j] == 1)
        {
            after_prunning_swc.push_back(before_prunning_swc[j]);
        }
   }
    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"after neuronNum is "<<final_tree.listNeuron.size()<<endl;
    return final_tree;
}
NeuronTree neurontreepruneorgraft::pruingSwc_one_leafnode_stop_before_tip_no_arg_input(LocationSimple lead_need_prun,   //叶子节点
                            NeuronTree nt,LandmarkList true_tip_list)
{
    //input:NeuronTree
    //output:  NeuronTree or/and vector<MyMarker*>
//    cout<<endl<<"begin to pruingSwc_one_leafnode_stop_before_tip,x:"<<lead_need_prun.x
//       <<" y:"<<lead_need_prun.y<<" z:"<<lead_need_prun.z<<endl;
//    cout<<"before prun nt.listNeuron.size"<<nt.listNeuron.size()<<endl;
//    //init
    QVector<QVector<V3DLONG> > childs;
//    NeuronTree before_tree=this->sort_neuron(nt);
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    vector<V3DLONG > temp_i;temp_i.clear();


    for (V3DLONG i=0;i<list.size();i++)
    {
        NeuronSWC temp_point;
        temp_point.x=list.at(i).x;
        temp_point.y=list.at(i).y;
        temp_point.z=list.at(i).z;

        if( sqrt((temp_point.x - lead_need_prun.x)*(temp_point.x - lead_need_prun.x) + (temp_point.y - lead_need_prun.y)*(temp_point.y - lead_need_prun.y) + (temp_point.z - lead_need_prun.z)*(temp_point.z - lead_need_prun.z))<0.01)
        {
            temp_i.push_back(i);
        }
    }
    if(temp_i.size()==0)
    {
        static int no_match_count=0;
        cout<<"times no match:"<<no_match_count<<",there is no match,so no pruing,lead_need_prun x: "<<lead_need_prun.x
                    <<" y "<<lead_need_prun.y<<" z "<<lead_need_prun.z<<endl;
        no_match_count++;
        //        cout<<"nt.listNeuron.size() "<<nt.listNeuron.size()<<endl;

        if(flag) {delete[] flag; flag = 0;}
        return nt;
    }
    if(temp_i.size()>=2)
    {
        cout<< "there is extra match,so should pruing twice or more "<<endl;
    }


    for(V3DLONG i=0;i<temp_i.size();i++)
    {
        V3DLONG prun_leaf_number=temp_i.at(i);
        flag[prun_leaf_number] = -1;
        V3DLONG parent_tip = getParent(prun_leaf_number,nt);
            if(parent_tip == 1000000000)
            {
                cout<<"something may be error ,the -1 point is a swc leaf point"<<endl;
                // need to change  it maybe not child but
                continue;
            }

//            cout<<"before loop ,parent is "<<parent_tip<<endl;

            double small_dist=1000000;
            V3DLONG small_dist_number=-1;
            while(childs[parent_tip].size()<2)
            {
                NeuronSWC parent_tip_swc=nt.listNeuron.at(parent_tip);
                LocationSimple parent_tip_location;
                parent_tip_location.x=parent_tip_swc.x;
                parent_tip_location.y=parent_tip_swc.y;
                parent_tip_location.z=parent_tip_swc.z;
                LocationSimple near_tip=Nearest_Match(parent_tip_location,true_tip_list);
                double curent_dist=sqrt((near_tip.x - parent_tip_location.x)*(near_tip.x - parent_tip_location.x)
                                          + (near_tip.y - parent_tip_location.y)*(near_tip.y - parent_tip_location.y)
                                          + (near_tip.z - parent_tip_location.z)*(near_tip.z - parent_tip_location.z));
                if(curent_dist<=small_dist&&curent_dist<pruingSwc_one_leafnode_stop_length)
                {
                    small_dist=curent_dist;
                    small_dist_number=parent_tip;
                }
                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }
            parent_tip = getParent(prun_leaf_number,nt);
            while((childs[parent_tip].size()<2)&&(parent_tip!=small_dist_number))
            {
                flag[parent_tip] = -1;
                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }


//             cout<<"finish select of need save of  "<<temp_i.at(i)<<" point that need prun"<<endl;

//            cout<<"end the  "<<temp_i.at(i)<<" point that need prun"<<endl;

    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int j=0;j<before_prunning_swc.size();j++)
    {
        if(flag[j] == 1)
        {
            after_prunning_swc.push_back(before_prunning_swc[j]);
        }
    }

    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc

    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    if(lead_need_prun.z==23)
    {
        cout<<"final_tree.listNeuron.size() "<<final_tree.listNeuron.size()<<endl;
        QString filename3="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\63.swc";
    //    saveSWC_file(filename3.toStdString(),radius_tree);
        writeSWC_file(filename3,final_tree);
    }
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
//    cout<<"after prun nt.listNeuron.size"<<final_tree.listNeuron.size()<<endl;
    return final_tree;

}


NeuronTree neurontreepruneorgraft::pruingSwc_one_leafnode_stop_before_tip(LocationSimple lead_need_prun,   //叶子节点
                            NeuronTree nt)
{
    //input:NeuronTree
    //output:  NeuronTree or/and vector<MyMarker*>
    cout<<"begin to pruingSwc_one_leafnode_stop_before_tip,x:"<<lead_need_prun.x
       <<" y:"<<lead_need_prun.y<<" z:"<<lead_need_prun.z<<endl;
    cout<<"before prun nt.listNeuron.size :"<<nt.listNeuron.size()<<endl;
//    //init
    QVector<QVector<V3DLONG> > childs;
//    NeuronTree before_tree=this->sort_neuron(nt);
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    vector<V3DLONG > temp_i;temp_i.clear();

    for (V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        if(nt.listNeuron.at(i).x==lead_need_prun.x&&
           nt.listNeuron.at(i).y==lead_need_prun.y&&
           nt.listNeuron.at(i).z==lead_need_prun.z)
        {
            temp_i.push_back(i);
        }
    }
    if(temp_i.size()==0)
    {
        static int no_match_count=0;
        cout<<"times no match:"<<no_match_count<<",there is no match,so no pruing,lead_need_prun x: "<<lead_need_prun.x
                    <<" y "<<lead_need_prun.y<<" z "<<lead_need_prun.z<<endl;
        no_match_count++;
        //        cout<<"nt.listNeuron.size() "<<nt.listNeuron.size()<<endl;

        if(flag) {delete[] flag; flag = 0;}
        return nt;
    }
    if(temp_i.size()>=2)
    {
        cout<< "there is extra match,so should pruing twice or more "<<endl;
    }


    for(V3DLONG i=0;i<temp_i.size();i++)
    {
        V3DLONG prun_leaf_number=temp_i.at(i);
        flag[prun_leaf_number] = -1;
        V3DLONG parent_tip = getParent(prun_leaf_number,nt);
            if(parent_tip == 1000000000)
            {
                cout<<"something may be error ,the -1 point is a swc leaf point"<<endl;
                // need to change  it maybe not child but
                continue;
            }

//            cout<<"before loop ,parent is "<<parent_tip<<endl;

            double small_dist=1000000;
            V3DLONG small_dist_number=-1;
            while(childs[parent_tip].size()<2)
            {
                NeuronSWC parent_tip_swc=nt.listNeuron.at(parent_tip);
                LocationSimple parent_tip_location;
                parent_tip_location.x=parent_tip_swc.x;
                parent_tip_location.y=parent_tip_swc.y;
                parent_tip_location.z=parent_tip_swc.z;
                LocationSimple near_tip=tp.Nearest_tip(parent_tip_location);
                double curent_dist=sqrt((near_tip.x - parent_tip_location.x)*(near_tip.x - parent_tip_location.x)
                                          + (near_tip.y - parent_tip_location.y)*(near_tip.y - parent_tip_location.y)
                                          + (near_tip.z - parent_tip_location.z)*(near_tip.z - parent_tip_location.z));
                if(curent_dist<=small_dist&&curent_dist<pruingSwc_one_leafnode_stop_length)
                {
                    float ocs=online_confidece(near_tip,parent_tip_location,this->bimg_datald,sz);
                    if(ocs>0.3)
                    {
                        small_dist=curent_dist;
                        small_dist_number=parent_tip;
                    }
                }
                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }
            parent_tip = getParent(prun_leaf_number,nt);
            while((childs[parent_tip].size()<2)&&(parent_tip!=small_dist_number))
            {
                flag[parent_tip] = -1;
                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }


//             cout<<"finish select of need save of  "<<temp_i.at(i)<<" point that need prun"<<endl;

//            cout<<"end the  "<<temp_i.at(i)<<" point that need prun"<<endl;

    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int j=0;j<before_prunning_swc.size();j++)
    {
        if(flag[j] == 1)
        {
            after_prunning_swc.push_back(before_prunning_swc[j]);
        }
    }

    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc

    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
////    if(lead_need_prun.z==23)
////    {
//        cout<<"final_tree.listNeuron.size() "<<final_tree.listNeuron.size()<<endl;
//        static int i=64;
//        i++;
//        QString filename3="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\";
//        filename3.append(QString::number(i));
//        filename3.append(".swc");
//    //    saveSWC_file(filename3.toStdString(),radius_tree);
//        writeSWC_file(filename3,final_tree);
////    }
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
//    cout<<endl;
    pruning_branch_count++;
    cout<<"after prun nt.listNeuron.size :"<<final_tree.listNeuron.size()<<endl;

    return final_tree;

}

NeuronTree neurontreepruneorgraft::pruingSwc_one_leafnode(LocationSimple lead_need_prun,   //叶子节点
                            NeuronTree nt)
{
    //input:NeuronTree
    //output:  NeuronTree or/and vector<MyMarker*>

    //init
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }
    //find i
    QList<NeuronSWC> list = nt.listNeuron;
    vector<V3DLONG > temp_i;
    for (V3DLONG i=0;i<list.size();i++)
    {
        if(list.at(i).x==lead_need_prun.x&&
           list.at(i).y==lead_need_prun.y&&
           list.at(i).z==lead_need_prun.z)
        {
            temp_i.push_back(i);

        }
    }
    if(temp_i.size()==0)
    {
        //no match
        cout<< "there is no match,so no pruing "<<endl;

        if(flag) {delete[] flag; flag = 0;}
        return nt;
    }
    if(temp_i.size()>=2)
    {
        cout<< "there is extra match,so should pruing twice "<<endl;
    }
    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;

    for(V3DLONG i=0;i<temp_i.size();i++)
    {
//        cout<<"begin the  "<<temp_i.at(i)<<" point that need prun"<<endl;
        V3DLONG prun_leaf_number=temp_i.at(i);
        flag[prun_leaf_number] = -1;
        int parent_tip = getParent(prun_leaf_number,nt);
            if(parent_tip == 1000000000)
                break;
//            cout<<"before loop ,parent is "<<parent_tip<<endl;

            while(childs[parent_tip].size()<2)
            {
                flag[parent_tip] = -1;
                parent_tip = getParent(parent_tip,nt);
                if(parent_tip == 1000000000)
                    break;
            }
//             cout<<"finish select of need save of  "<<temp_i.at(i)<<" point that need prun"<<endl;
            for (int j=0;j<before_prunning_swc.size();j++)
            {
                if(flag[j] == 1)
                {
                    after_prunning_swc.push_back(before_prunning_swc[j]);
                }
           }
//            cout<<"end the  "<<temp_i.at(i)<<" point that need prun"<<endl;
    }

    if(flag) {delete[] flag; flag = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    return final_tree;

}
NeuronTree neurontreepruneorgraft::Pruning_leaf_node_in_background(NeuronTree nt)
{
// no ok!
    nt=this->sort_neuron(nt);
    cout<<"nt=this->sort_neuron(nt) over"<<endl;

    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];
    V3DLONG *parent_id = new V3DLONG[neuronNum];
    cout<<"before pruning neuronNum "<<neuronNum<<endl;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;
        parent_id[i] = -1;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {

        if (childs[i].size()==0)
        {

            int parent_tip = getParent(i,nt);
            LocationSimple curr_node;
            curr_node.x = list.at(i).x;
            curr_node.y = list.at(i).y;
            curr_node.z = list.at(i).z;
            if(tp.isForeground(curr_node))
            {
                //
                continue;
            }
            else
            {
                flag[i] = -1;
            }
            if(parent_tip == 1000000000)
                continue;

            while(childs[parent_tip].size()<2)
            {

                curr_node.x = list.at(parent_tip).x;
                curr_node.y = list.at(parent_tip).y;
                curr_node.z = list.at(parent_tip).z;
                if(tp.isForeground(curr_node))
                {
                    //
                    break;
                }
                else
                {
                    flag[parent_tip] = -1;
                }

                parent_tip = getParent(parent_tip,nt);

                if(parent_tip == 1000000000)
                    break;

             }

//            int parent_index = parent_tip;

//            if(index_tip < small_length)
//            {
//                flag[i] = -1;
//                segment_length[i] = index_tip;
//                parent_id[i] = parent_index;
//                int parent_tip = getParent(i,nt);
//                while(childs[parent_tip].size()<2)
//                {
//                    flag[parent_tip] = -1;
//                    segment_length[parent_tip] = index_tip;
//                    parent_id[parent_tip] = parent_index;
//                    parent_tip = getParent(parent_tip,nt);
//                    if(parent_tip == 1000000000)
//                        break;
//                }
//                if(segment_length[parent_tip] > index_tip)
//                    segment_length[parent_tip]  = index_tip;
//            }
        }
    }

    vector<MyMarker*> before_prunning_swc = NeuronTree2vectorofMyMarker(nt);
    vector<MyMarker*> after_prunning_swc;
    for (int i=0;i<before_prunning_swc.size();i++)
    {
        if(flag[i] == 1 )
        {
            after_prunning_swc.push_back(before_prunning_swc[i]);
        }

   }

    if(flag) {delete[] flag; flag = 0;}
    if(parent_id) {delete[] parent_id; parent_id = 0;}

    // output as  after_prunning_swc
    NeuronTree final_tree;
    final_tree=vectorofMyMarker2NeuronTree(after_prunning_swc);
    for(V3DLONG i=0;i<before_prunning_swc.size();i++)
    {
        delete before_prunning_swc.at(i);
    }
    cout<<"after neuronNum is "<<final_tree.listNeuron.size()<<endl;
    return final_tree;

}

NeuronTree neurontreepruneorgraft::tipBasedPruning_stop_before_tip_no_z(NeuronTree nt)
{
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return nt;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }
    NeuronTree before_tree=this->sort_neuron(nt);
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(before_tree);
    for(int temp=0;temp<swc_tip_list.size();temp++)
    {
        if(swc_tip_list.at(temp).z==23)
        {
           cout<<" swc_tip_list.at(temp).z==23 x:"<<swc_tip_list.at(temp).x<<" y "<<swc_tip_list.at(temp).y<<" z "<<swc_tip_list.at(temp).z<<endl;

        }
    }
    for(int temp=0;temp<before_tree.listNeuron.size();temp++)
    {
        if(before_tree.listNeuron.at(temp).z==23)
        {
           cout<<"before_tree.listNeuron.at(temp).z==23 x:"<<before_tree.listNeuron.at(temp).x
              <<" y "<<before_tree.listNeuron.at(temp).y<<" z "<<before_tree.listNeuron.at(temp).z<<endl;

        }
    }


    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return before_tree;
    }
    else
    {
        cout<<"there is swc_tip_list size: "<<swc_tip_list.size()<<endl;
    }
//    NeuronTree after_prun_tree=vectorofMyMarker2NeuronTree(swc);
//    NeuronTree after_prun_tree=mergeSwc_two_leafnode(vectorofMyMarker2NeuronTree(swc));
    NeuronTree after_tree=before_tree;
    for(int temp=0;temp<after_tree.listNeuron.size();temp++)
    {
        if(after_tree.listNeuron.at(temp).z==23)
        {
            cout<<"before_tree..z==23 x:"<<before_tree.listNeuron.at(temp).x
               <<endl;
           cout<<"after_tree..z==23 x:"<<after_tree.listNeuron.at(temp).x
              <<endl;

        }
    }
    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {
        LocationSimple true_tip;
        LocationSimple tippoint;
        tippoint.x=swc_tip_list.at(num).x;
        tippoint.y=swc_tip_list.at(num).y;
        tippoint.z=swc_tip_list.at(num).z;
//        if(tippoint.z==23)
//        {
//           cout<<"tippoint.z==23 x:"<<tippoint.x<<" y "<<tippoint.y<<" z "<<tippoint.z<<endl;
//cout<<"tippoint.z==23 x:"<<swc_tip_list.at(num).x<<" y "<<swc_tip_list.at(num).y<<" z "<<swc_tip_list.at(num).z<<endl;
//        }
        if(!tp.TipDetect_onePoint_neighbor_no_z(tippoint,true_tip,5))
        {
            after_tree=pruingSwc_one_leafnode_stop_before_tip(tippoint,after_tree);
        }
    }
    QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\71.swc";
    writeSWC_file(filename,after_tree);

    return after_tree;
}
NeuronTree neurontreepruneorgraft::tipBasedPruning_stop_before_tip_no_arg_input(LandmarkList need_pruning_leaf
                                                                                ,NeuronTree nt,LandmarkList realtip)
{
    if(realtip.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return nt;
    }else
    {
        cout<<"there is true_tip_Marker "<<realtip.size()<<endl;
    }
    if(need_pruning_leaf.size()<1)
    {
        cout<<"There is no need_pruning_leaf"<<endl;
        return nt;
    }else
    {
        cout<<"there is need_pruning_leaf "<<need_pruning_leaf.size()<<endl;
    }

    NeuronTree before_tree=this->sort_neuron(nt);
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(before_tree);
    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return before_tree;
    }
    else
    {
        cout<<"there is swc_tip_list size: "<<swc_tip_list.size()<<endl;
    }
//    NeuronTree after_prun_tree=vectorofMyMarker2NeuronTree(swc);
//    NeuronTree after_prun_tree=mergeSwc_two_leafnode(vectorofMyMarker2NeuronTree(swc));
    NeuronTree after_tree=before_tree;
    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {
        LocationSimple true_tip;
        LocationSimple tippoint;
        tippoint.x=swc_tip_list.at(num).x;
        tippoint.y=swc_tip_list.at(num).y;
        tippoint.z=swc_tip_list.at(num).z;
        for(V3DLONG num1=0;num1<need_pruning_leaf.size();num1++)
        {
            true_tip.x=need_pruning_leaf.at(num1).x;
            true_tip.y=need_pruning_leaf.at(num1).y;
            true_tip.z=need_pruning_leaf.at(num1).z;
            if(true_tip.x==tippoint.x&&true_tip.y==tippoint.y&&true_tip.z==tippoint.z)
            {

                after_tree=pruingSwc_one_leafnode_stop_before_tip_no_arg_input(tippoint,after_tree,realtip);

            }
        }
    }
    QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\1131.swc";
    writeSWC_file(filename,after_tree);
    cout<<"after pruning listneuronsize is "<<nt.listNeuron.size()<<endl;

    return after_tree;
}

NeuronTree neurontreepruneorgraft::tipBasedPruning_stop_before_tip(NeuronTree nt)
{
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return nt;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }
    NeuronTree before_tree=this->sort_neuron(nt);
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(before_tree);
    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return before_tree;
    }
    else
    {
        cout<<"there is swc_tip_list size: "<<swc_tip_list.size()<<endl;
    }
    NeuronTree after_tree=before_tree;
    QString filename1="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\3212.swc";
    writeSWC_file(filename1,after_tree);
    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {
        LocationSimple true_tip;
        LocationSimple tippoint;
        tippoint.x=swc_tip_list.at(num).x;
        tippoint.y=swc_tip_list.at(num).y;
        tippoint.z=swc_tip_list.at(num).z;
        if(!tp.TipDetect_onePoint_neighbor(tippoint,true_tip,5))
        {
            after_tree=pruingSwc_one_leafnode_stop_before_tip(tippoint,after_tree);
        }
    }
    QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\31.swc";
    writeSWC_file(filename,after_tree);

    return after_tree;
}

NeuronTree neurontreepruneorgraft::reestimate_radius(NeuronTree nt)
{
    if(1)
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            double radius_after=1;
            nt.listNeuron[i].r=radius_after ;
        }
        cout<<" reestimate the markerRadius is over";
        return nt;
    }
    else
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            //printf(" node %ld of %ld.\n", i, outswc.size());
            double real_thres = 40;
            double radius_after=markerRadius_hanchuan_XY(datald,sz,nt.listNeuron[i], real_thres);
            nt.listNeuron[i].r=radius_after ;
        }
        cout<<" reestimate the markerRadius is over";
        return nt;
    }

}

NeuronTree neurontreepruneorgraft::tipBasedPruning(NeuronTree nt)
{
    //not ok
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return nt;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }

    NeuronTree temp_tree=this->sort_neuron(nt);
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(temp_tree);
    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return temp_tree;
    }
    else
    {
        cout<<"there is swc_tip_Marker "<<swc_tip_list.size()<<endl;
    }
    NeuronTree after_prun_tree=temp_tree;

    for(V3DLONG num=0;num<swc_tip_list.size();num++)
    {
        LocationSimple true_tip;
        LocationSimple tippoint;
        tippoint.x=swc_tip_list.at(num).x;
        tippoint.y=swc_tip_list.at(num).y;
        tippoint.z=swc_tip_list.at(num).z;
        if(!tp.TipDetect_onePoint_neighbor(tippoint,true_tip,5))
        {
            after_prun_tree=pruingSwc_one_leafnode(tippoint,after_prun_tree);
        }
    }

    QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\21.swc";
    writeSWC_file(filename,after_prun_tree);
    return after_prun_tree;
}

bool neurontreepruneorgraft::getImgData_not_process_tip(V3DPluginCallback2 &callback2, QWidget *parent)
{
    //get img date
    v3dhandle curwin = callback2.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return false;
    }
    p4DImage = callback2.getImage(curwin);
    datald = p4DImage->getRawData();
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
//    tp.getImgData(callback2);


//    this->getNeuronTree(callback2,parent);
    if(!this->getNeuronTree(callback2,parent))
    {
        return false;
    }
    return true;
}


bool neurontreepruneorgraft::getImgData(V3DPluginCallback2 &callback2, QWidget *parent)
{
    //get img date
    v3dhandle curwin = callback2.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return false;
    }
    p4DImage = callback2.getImage(curwin);
    datald = p4DImage->getRawData();
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    // get tip point
    tp.getImgData(callback2);
    if(!tp.GUI_input_argu())
    {
        //didn't input argu
        //program should stop
        cout<<"argu input cancel"<<endl;
        return false;
    }
    tp.whole_img(callback2);
    tip_list=tp.OutputTipPoint();
    //get branch point
    //to be writed
    if(!this->getNeuronTree(callback2,parent))
    {
        return false;
    }
    isProcessTip=true;
    return true;
}
LandmarkList neurontreepruneorgraft::FromNeuronTreeGetLeaf_branch(NeuronTree nt)
{
    LandmarkList point_list;
    LocationSimple s;
    RGBA8 red,green;
    red.r=255;
    red.g=0;
    red.b=0;
    green.r=0;
    green.g=255;
    green.b=0;
    V3DLONG neuronNum = nt.listNeuron.size();
    V3DLONG *childs_num;
    childs_num=new V3DLONG[neuronNum];
    for(V3DLONG i=0;i<neuronNum;i++)
    {
        childs_num[i]=0;
    }

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) {
            childs_num[i]--;
            continue;
        }
        childs_num[nt.hashNeuron.value(par)]++;
    }
    //get every leaf node of every branch
    V3DLONG tipcount=0;
    V3DLONG branchcount=0;
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs_num[i]<=0)
       {
           s.x=nt.listNeuron.at(i).x+1;
           s.y=nt.listNeuron.at(i).y+1;
           s.z=nt.listNeuron.at(i).z+1;
           s.radius=nt.listNeuron.at(i).r;
           s.color = red;
           point_list<<s;
           tipcount++;
           //tip
       }
       else if(childs_num[i]>=2)
       {
           s.x=nt.listNeuron.at(i).x+1;
           s.y=nt.listNeuron.at(i).y+1;
           s.z=nt.listNeuron.at(i).z+1;
           s.radius=nt.listNeuron.at(i).r;
           s.color = green;
           point_list<<s;
           branchcount++;
       }
    }
    cout<<"tipcount number:"<<tipcount<<endl;
    cout<<"branchcount number:"<<branchcount<<endl;
    return point_list;
}
LandmarkList neurontreepruneorgraft::FromNeuronTreeGetLeaf(NeuronTree nt)
{
    LandmarkList point_list;
    LocationSimple s;
    RGBA8 red,green;
    red.r=255;
    red.g=0;
    red.b=0;
    green.r=0;
    green.g=255;
    green.b=0;
    V3DLONG neuronNum = nt.listNeuron.size();
    V3DLONG *childs_num;
    childs_num=new V3DLONG[neuronNum];
    for(V3DLONG i=0;i<neuronNum;i++)
    {
        childs_num[i]=0;
    }
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) {
            childs_num[i]--;
            continue;
        }
        childs_num[nt.hashNeuron.value(par)]++;
    }
    //get every leaf node of every branch
    V3DLONG tipcount=0;
    for(V3DLONG i=0;i<neuronNum;i++)
    {
       if(childs_num[i]==0)
       {
           s.x=nt.listNeuron.at(i).x;
           s.y=nt.listNeuron.at(i).y;
           s.z=nt.listNeuron.at(i).z;
           s.radius=nt.listNeuron.at(i).r;
           s.color = red;
           point_list<<s;
           tipcount++;
           //tip
       }
//       else if(childs_num[i]>=2)
//       {
//           s.x=nt.listNeuron.at(i).x;
//           s.y=nt.listNeuron.at(i).y;
//           s.z=nt.listNeuron.at(i).z;
//           s.radius=nt.listNeuron.at(i).r;
//           s.color = green;
//           point_list<<s;
//       }
    }
    cout<<"tipcount number:"<<tipcount<<endl;
    return point_list;

}

bool neurontreepruneorgraft::getNeuronTree(V3DPluginCallback2 &callback2, QWidget *parent)
{
    //get leaf node or branch node
    tp.GetNeuronTreeFrom3dview(callback2);
//    this->FromNeuronTreeGetTipBranch(callback2);

//    tp.FromNeuronTreeGetTipBranch(callback2);
//    swc_tip_list=tp.returnSwcTipPoints();
//    swc_branch_list=tp.returnSwcBranchPoints();
    QList <V3dR_MainWindow *> list_3dviewer = callback2.getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
        v3d_msg("Please open  a SWC file from the main menu first! list_3dviewer");
        return false;
    }
    V3dR_MainWindow *surface_win = list_3dviewer[0];
    if (!surface_win){
        v3d_msg("Please open up a SWC file from the main menu first!");
        return false;
    }
//    cout<<"read SWC from 3d Viewer  "<<endl;
    QList<NeuronTree> * mTreeList = callback2.getHandleNeuronTrees_Any3DViewer(surface_win);
    if(mTreeList->size()<=0)
    {
        return false;
    }
    init_neurontree=mTreeList->at(mTreeList->size()-1);
    init_neurontree=vectorofMyMarker2NeuronTree(NeuronTree2vectorofMyMarker(init_neurontree));
//    temp_neurontree=mTreeList->at(0);
    return true;
}
bool neurontreepruneorgraft::trace_one_pt_to_one_points_shortestdist(LocationSimple start_nodes,LocationSimple end_nodes,NeuronTree &nt)
{
    //作用是从一个点追踪到其他点
    //gd.cpp  find_shortest_path_graphimg directly call it
    // trace_one_pt_to_N_points_shortestdist
    //v3dneuron_GD_tracing
    printf("start of find_shortest_path_graphimg \n");
    if(!datald|| !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0)
    {
        cout<<"Invalid image or sz for trace_one_pt_to_one_points_shortestdist()."<<endl;
        return false;
    }
    if(start_nodes.x<0||start_nodes.x>=sz[0]||
            start_nodes.y<0||start_nodes.y>=sz[1]||
            start_nodes.z<0||start_nodes.z>=sz[2])
    {
        cout<<"Invalid start_nodes."<<endl;
        return false;
    }
    if(end_nodes.x<0||end_nodes.x>=sz[0]||
            end_nodes.y<0||end_nodes.y>=sz[1]||
            end_nodes.z<0||end_nodes.z>=sz[2])
    {
        cout<<"Invalid end_nodes."<<endl;
        return false;
    }

    // one to one
    unsigned char ****p4d = 0;
    CurveTracePara trace_para;
    trace_para.channo=0;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = -1;
    }
    else
    {
        trace_para.imgTH = 0;
    }
    trace_para.visible_thresh =30;
    if(sz[3]==1)
    {
        //call find_shortest_path_graphimg
//        NeuronTree nt;
        try {
            new4dpointer(p4d, sz[0], sz[1], sz[2], sz[3], datald);
        } catch (...) {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            return false;
        }
        vector<LocationSimple> pp;
        pp.push_back(end_nodes);
        nt = v3dneuron_GD_tracing(p4d, sz,
                                  start_nodes,pp ,
                                  trace_para, 1);
        QTime t;
        QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\";
        filename.append(t.currentTime().toString("hh_mm_ss")).append("_2.swc");
        writeSWC_file(filename,nt);
    }
    else
    {
        printf("it is not ready for 3 chanel ");
        return false;
    }
    // call mergeback_mmunits_to_neuron_path
    printf("trace is over ");
    return false;
}

bool neurontreepruneorgraft::trace_one_pt_to_one_points_shortestdist(MyMarker* start_nodes,MyMarker* end_nodes,NeuronTree &nt)
{
    //作用是从一个点追踪到其他点
    //gd.cpp  find_shortest_path_graphimg directly call it
    // trace_one_pt_to_N_points_shortestdist
    //v3dneuron_GD_tracing
    printf("start of find_shortest_path_graphimg \n");
    if(!datald|| !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0)
    {
        cout<<"Invalid image or sz for trace_one_pt_to_one_points_shortestdist()."<<endl;
        return false;
    }
    if(start_nodes->x<0||start_nodes->x>=sz[0]||
            start_nodes->y<0||end_nodes->y>=sz[1]||
            start_nodes->z<0||end_nodes->z>=sz[2])
    {
        cout<<"Invalid start_nodes."<<endl;
        return false;
    }
    if(end_nodes->x<0||end_nodes->x>=sz[0]||
            end_nodes->y<0||start_nodes->y>=sz[1]||
            end_nodes->z<0||start_nodes->z>=sz[2])
    {
        cout<<"Invalid end_nodes."<<endl;
        return false;
    }

    // one to one
    char* s_error = 0;
    unsigned char ****p4d = 0;
    CurveTracePara trace_para;
    trace_para.channo=0;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = -1;
    }
    else
    {
        trace_para.imgTH = 0;
    }
    trace_para.visible_thresh =30;
    if(sz[3]==1)
    {
        //call find_shortest_path_graphimg
//        NeuronTree nt;
        try {
            new4dpointer(p4d, sz[0], sz[1], sz[2], sz[3], datald);
        } catch (...) {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            return false;
        }
        vector<LocationSimple> pp;
        pp.push_back(MyMarker2LocationSimple(end_nodes));
        nt = v3dneuron_GD_tracing(p4d, sz,
                                  MyMarker2LocationSimple(start_nodes),pp ,
                                  trace_para, 1);
        QString filename="C:\\Users\\204\\Desktop\\demo\\fruitfly5\\2.swc";
        writeSWC_file(filename,nt);
    }
    else
    {
        printf("it is not ready for 3 chanel ");
        return false;
    }
    // call mergeback_mmunits_to_neuron_path

    return false;

}

NeuronTree neurontreepruneorgraft::graftToBorder(NeuronTree nt)
{
    //for tip to
    //  TipDetect_onePoint_27neighbor
    if(tip_list.size()<1)
    {
        cout<<"There is no tip in img"<<endl;
        return nt;
    }else
    {
        cout<<"there is true_tip_Marker "<<tip_list.size()<<endl;
    }
    LandmarkList swc_tip_list=this->FromNeuronTreeGetLeaf(nt);
    if(swc_tip_list.size()<1)
    {
        cout<<"There is no tip in swc"<<endl;
        return nt;
    }
    else
    {
        cout<<"there is swc_tip_Marker "<<swc_tip_list.size()<<endl;
    }

    //backup
    LandmarkList cluster_tip_list=tip_list;
    LandmarkList swc_leaf_list=swc_tip_list;
    //from leaf of swc to img_tip
    // or from

    //init if a tip point is traced ,it must b removed
    for(int j=0;j<cluster_tip_list.size();j++)
    {
        for(int swc_num=0;swc_num<nt.listNeuron.size();swc_num++)
        {
            double dx=nt.listNeuron.at(swc_num).x-cluster_tip_list.at(j).x;
            double dy=nt.listNeuron.at(swc_num).y-cluster_tip_list.at(j).y;
            double dz=nt.listNeuron.at(swc_num).z-cluster_tip_list.at(j).z;
            double dist=sqrt(pow(dx,2)+pow(dy,2)+pow(dz,2));
            if(dist<dist_traced)
            {
                cluster_tip_list.removeAt(j);
                j=j-1;
                break;
            }
        }
    }
    if(cluster_tip_list.size()<0)
    {
        cout<<"all tip points are traced by swc"<<endl;
        return nt;
    }

    cout<<"after compare there is true_tip_Marker: "<<cluster_tip_list.size()<<endl;

    for(int j=0;j<swc_leaf_list.size();j++)
    {
        LocationSimple trueLeaf;
        if(tp.TipDetect_onePoint_neighbor(swc_leaf_list.at(j),trueLeaf,radius_tip_checked))
        {
            swc_leaf_list[j]=swc_leaf_list.back();
            swc_leaf_list.pop_back();
            j=j-1;
            break;
        }

    }
    if(swc_leaf_list.size()<0)
    {
        cout<<"all leaf node in swc are true tip points"<<endl;
        return nt;
    }
//    cout<<"after compare there is swc_tip_Marker: "<<swc_leaf_list.size()<<endl;

    QMap<int,int> map_need_trace;
    for(int i=0;i<swc_leaf_list.size();i++)
    {
        double min_dist=10000;
        for(int j=0;j<cluster_tip_list.size();j++)
        {
            double dx=swc_leaf_list.at(i).x-cluster_tip_list.at(j).x;
            double dy=swc_leaf_list.at(i).y-cluster_tip_list.at(j).y;
            double dz=swc_leaf_list.at(i).z-cluster_tip_list.at(j).z;

            double dist=sqrt(pow(dx,2)+pow(dy,2)/*+pow(dz,2)*/);
            if(dist<dist_need_trace&&dist<min_dist)
            {
                // need to traced
                cout<<"the graft point true tip x:"<<cluster_tip_list.at(j).x<<" y:"<<cluster_tip_list.at(j).y<<" z:"<<cluster_tip_list.at(j).z<<endl;
                cout<<"the graft point swc leaf x:"<<swc_leaf_list.at(i).x<<" y:"<<swc_leaf_list.at(i).y<<" z:"<<swc_leaf_list.at(i).z<<endl;

                map_need_trace.insert(i,j);
//                cout<<"need to graft "<<endl;
            }
        }
    }
    QMap<int,int>::const_iterator cim;
    for(cim=map_need_trace.begin();cim!=map_need_trace.end();cim++)
    {
//        cout<<"begin to get one "<<endl;
        int key=cim.key();
        int value=cim.value();
//        swc_leaf_marker.at(key);
        LocationSimple tip= cluster_tip_list.at(value);
        cout<<"the graft point true tip x:"<<cluster_tip_list.at(value).x<<" y:"<<cluster_tip_list.at(value).y<<" z:"<<cluster_tip_list.at(value).z<<endl;
        cout<<"the graft point swc leaf x:"<<swc_leaf_list.at(key).x<<" y:"<<swc_leaf_list.at(key).y<<" z:"<<swc_leaf_list.at(key).z<<endl;
        //找到对应的MyMarker
//        swc.push_back(new_marker);
        NeuronTree nt1;
        trace_one_pt_to_one_points_shortestdist(swc_leaf_list.at(key),cluster_tip_list.at(value),nt1);
        nt=merge_two_neuron(nt,nt1);
    }
//    cout<<"ok";
    return nt;

}
