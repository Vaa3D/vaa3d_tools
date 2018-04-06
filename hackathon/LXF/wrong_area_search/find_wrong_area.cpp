

/* resample_swc_func.cpp
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 */
#include"wrong_area_search_plugin.h"
#include "find_wrong_area.h"
#include <iostream>
#include<string>
//#include"openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include"sort_swc.h"
#include"resampling.h"
#include"local_aligment.h"
#include"v3d_interface.h"
#include<algorithm>
//#include<QSet>
//#include"basic_string.h"

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define MHDIS(a,b) ( (fabs((a).x-(b).x)) + (fabs((a).y-(b).y)) + (fabs((a).z-(b).z)) )
struct Coodinate
{
    V3DLONG x;
    V3DLONG y;
    V3DLONG z;
    V3DLONG lens;
};
struct FourType
{
    QList<NeuronSWC> little;
    QList<NeuronSWC> middle;
    QList<NeuronSWC> point_rec;
    QList<NeuronSWC> point_gold;
};
QList<NeuronSWC>choose_point(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,int thre1,int thre2);
void processImage(V3DPluginCallback2 &callback,vector<Coodinate> &fourcood_each);
bool get_subarea(QList<NeuronSWC> &nt,vector<Coodinate> &fourcood,vector<QList<NeuronSWC> >&subarea);
bool get_subarea_in_nt(vector<FourType> &fourtype,V3DLONG length,QList<NeuronSWC> &little,QList<NeuronSWC> &middle,QList<NeuronSWC> &point_rec,QList<NeuronSWC> &point_gold);
//bool get_mean_xyz(vector<QList<NeuronSWC> > &subarea,vector<Coodinate>  &mean_xyz_all);
bool make_coodinate(vector<vector<Coodinate> > &four_cood,vector<FourType> &four_type,V3DLONG lens);
bool get_subimg(QString raw_img,QString name,vector<Coodinate> &mean,unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
//bool get_subimg_terafly(QString inimg_file,QString name,vector<Coodinate> &mean,unsigned char * data1d_crop,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
bool get_subimg_terafly(QString inimg_file,QString name,vector<Coodinate> &mean,V3DPluginCallback2 &callback);
//void processImage_terafly(V3DPluginCallback2 &callback,vector<Coodinate> &fourcood_each);
bool sort_with_standard(QList<NeuronSWC>  & neuron1, QList<NeuronSWC> & neuron2,QList<NeuronSWC>  &result)
{
    V3DLONG siz = neuron1.size();
    V3DLONG root_id = 1;
    double dist;
    if (siz==0) return false;
    double min_dist = sqrt((neuron1[0].x-neuron2[0].x)*(neuron1[0].x-neuron2[0].x)
         +(neuron1[0].y-neuron2[0].y)*(neuron1[0].y-neuron2[0].y)
         +(neuron1[0].z-neuron2[0].z)*(neuron1[0].z-neuron2[0].z));
    for(V3DLONG i=0; i<siz; i++)
    {
         dist = sqrt((neuron1[i].x-neuron2[0].x)*(neuron1[i].x-neuron2[0].x)
                +(neuron1[i].y-neuron2[0].y)*(neuron1[i].y-neuron2[0].y)
                +(neuron1[i].z-neuron2[0].z)*(neuron1[i].z-neuron2[0].z));
         if(min_dist > dist) {min_dist = dist; root_id = i+1;}
    }
    cout<<"min_dist = "<< min_dist <<endl;
    cout<<"root_id = " << root_id <<endl;

    //sort_swc process
    double thres = 10000;
    if(!SortSWC(neuron1,result,root_id,thres))
    {
        cout<<"Error in sorting swc"<<endl;
        return false;
    }
    return true;
}

bool find_wrong_area(Input_para &PARA,V3DPluginCallback2 &callback,bool bmenu,QWidget *parent)
{
    bool type=PARA.model1;  //1 for.v3dpbd,0 for terafly
    if(bmenu)     //0 for dofunc,1 for domenu
    {

        if(!type)
        {
            //PARA.filename1 = "/home/penglab/Data/xuefeng/023_x_7522.77_y_13532.6_y_2019.19.swc";
                        //PARA.filename2 = "/home/penglab/Data/xuefeng/023 final.ano.swc";

                        //PARA.filename3 = "/run/media/penglab/WS5/mouseID_321237-17302/RES(54600x34412x9847)";

            PARA.filename1 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/029_x_8969.08_y_9020.6_y_1273.9.swc";
            PARA.filename2 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/029.ano.swc";
            PARA.filename3 = "/media/lxf/zhang/mouseID_321237-17302/RES(54600x34412x9847)";
        }
        else
        {
//            PARA.filename1 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/reconstruction.swc";
//            PARA.filename2 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/gold.swc";
//            PARA.filename3 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/img.v3dpbd";
            PARA.filename1 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/exp1.swc";
            PARA.filename2 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/exp2.swc";
            PARA.filename3 = "/media/lxf/8213-B4FE/3.19/Data/xuefeng/test/test1156.v3dpbd";
        }
    }
    else
    {
        printf("In to dofunc.\n");
    }



       /*****************************resample************************************************/
    cout<<"this is resample "<<endl;
    NeuronTree nt_p,nt,temp_neuron_p,temp_neuron;
    double prune_size = PARA.para4;
    if (PARA.filename1.toUpper().endsWith(".SWC") || PARA.filename1.toUpper().endsWith(".ESWC"))
        nt_p = readSWC_file(PARA.filename1);

    prune_branch(nt_p,nt,prune_size);
    NeuronTree resample_result = resample(nt,PARA.para1);


    //  find the nearest point as new root node calculated between input_1's all node  and input_2's root node
    cout<<"find_roodID begin.";
    QList<NeuronSWC> neuron1,neuron2;

    neuron1 = resample_result.listNeuron;

    if (PARA.filename2.toUpper().endsWith(".SWC") || PARA.filename2.toUpper().endsWith(".ESWC"))
    {

        temp_neuron_p = readSWC_file(PARA.filename2);
        neuron2 = readSWC_file(PARA.filename2).listNeuron;
    }
    prune_branch(temp_neuron_p,temp_neuron,prune_size);
    NeuronTree resample_gold = resample(temp_neuron,PARA.para1);

    QList<NeuronSWC> result,result_2;
        QString fileSaveName;
        QString fileSaveName_gold;
        fileSaveName = PARA.filename1 + "_sort.swc";
        fileSaveName_gold = PARA.filename2 + "_sort.swc";

    /******************************sort******************************************************/
    cout<<"this is sort "<<endl;
    sort_with_standard(neuron1,neuron2,result);
    cout<<"first sort done "<<endl;
    sort_with_standard(resample_gold.listNeuron,neuron2,result_2);
    cout<<"result.size = "<<result.size()<<endl;
    cout<<"result_2.size = "<<result_2.size()<<endl;
    if (!export_list2file(result, fileSaveName, PARA.filename1))
    {
        printf("fail to write the output swc file.\n");
        return false;
    }
    if (!export_list2file(result_2, fileSaveName_gold, PARA.filename2))
    {
        printf("fail to write the output swc file.\n");
        return false;
    }


    vector<MyMarker *> neuron_m;
    vector<MyMarker *> neuron_gold;
    neuron_m = nt2mm(result,fileSaveName);

    cout<<"size_nt = "<<result.size()<<endl;
    cout<<"size_marker = "<<neuron_m.size()<<endl;
    neuron_gold = nt2mm(result_2,PARA.filename2);
    string name1 = PARA.filename1.toStdString();
    string name2 = PARA.filename2.toStdString();
    cout<<"name1 = "<<name1<<endl;
    cout<<"name2 = "<<name2<<endl;

    cout<<"neuron_m_size = "<<neuron_m.size()<<endl;
    cout<<"neuron_gold_size = "<<neuron_gold.size()<<endl;

    vector<FourType> fourtype;
    vector<vector<Coodinate> > fourcood;

    bool model= PARA.model2;  //1 for choose point ,0 for local alignment
    if(model)
    {
        /********************************choose point***********************************/


        QList<NeuronSWC> choose_rec1,choose_rec2,choose_gold1,choose_gold2,output_result;
        choose_rec1 = choose_point(result,result_2,0,4);
        choose_rec2 = choose_point(result,result_2,4,10000);
        choose_gold1 = choose_point(result_2,result,0,4);
        choose_gold2 = choose_point(result_2,result,4,10000);

        QString fileSaveName2 = "temp1.swc";
        QString fileSaveName3 = "temp2.swc";
        QString fileSaveName4 = "temp3.swc";
     //   cout<<""
        //    for(V3DLONG i=0;i<choose_rec1.size();i++)


        if (!export_list2file(choose_rec1,fileSaveName2, fileSaveName2))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(choose_rec2, fileSaveName3, fileSaveName2))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(choose_gold2, fileSaveName4, fileSaveName2))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }


        V3DLONG lens=PARA.para2;
        V3DLONG length=PARA.para3;                             //step_for_get_sub_img
        get_subarea_in_nt(fourtype,length,choose_rec1,choose_rec2,choose_gold1,choose_gold2);
        cout<<"four_type_size = "<<fourtype.size()<<endl;
        make_coodinate(fourcood,fourtype,lens);


    }
    else
    {

        /*******************************local_alignment*********************************/
        cout<<"local alignment"<<endl;
        vector<map<MyMarker*, MyMarker*> > map_result;
        if (!neuron_mapping_dynamic(neuron_m, neuron_gold, map_result))
        {
            printf("error in neuron_mapping\n");
            return false;
        }
        string name_result =  "aligned_result.swc";
        QString name_aligned = "aligned_result.swc";
        vector<MyMarker*> map_swc;
        convert_matchmap_2swc(map_result, map_swc);
        saveSWC_file(name_result, map_swc);
        NeuronTree nt_for_choose;
        QList<NeuronSWC> aligned_choosen,little_aligned,middle_aligned;
        nt_for_choose = mm2nt(map_swc,name_aligned);
        cout<<"size nt_for_choose = "<<nt_for_choose.listNeuron.size()<<endl;
        for(V3DLONG i=0;i<nt_for_choose.listNeuron.size();i++)
        {
            aligned_choosen.push_back(nt_for_choose.listNeuron[i]);

        }
        cout<<"hahahahahahahhaha"<<endl;
        QList<NeuronSWC> gold,not_in_gold,gold_little,gold_middle,little_all,middle_all;
        little_aligned = choose_alignment(aligned_choosen,gold_little,0,20);
        middle_aligned = choose_alignment(aligned_choosen,gold_middle,20,20000);
        for(V3DLONG i=0;i<gold_little.size();i++)
        {
            gold.push_back(gold_little[i]);
        }
        for(V3DLONG i=0;i<gold_middle.size();i++)
        {
            gold.push_back(gold_middle[i]);
        }



        for(V3DLONG i=0;i<little_aligned.size();i++)
        {
            little_all.push_back(little_aligned[i]);
        }
        for(V3DLONG i=0;i<gold_little.size();i++)
        {
            little_all.push_back(gold_little[i]);
        }
        for(V3DLONG i=0;i<middle_aligned.size();i++)
        {
            middle_all.push_back(middle_aligned[i]);
        }
        for(V3DLONG i=0;i<gold_middle.size();i++)
        {
            middle_all.push_back(gold_middle[i]);
        }



        QList<NeuronSWC> point_in_re,point_not_in_re;
        for(V3DLONG i=0;i<little_aligned.size();i++)
        {
            point_in_re.push_back(little_aligned[i]);
        }
        for(V3DLONG i=0;i<middle_aligned.size();i++)
        {
            point_in_re.push_back(middle_aligned[i]);
        }
        cout<<"point_in_rec = "<<point_in_re.size()<<endl;
        cout<<"point_in_gold = "<<gold.size()<<endl;


        bool ind,ind2;
        for(V3DLONG i=0;i<result.size();i++)
        {
            ind=true;
            for(V3DLONG j=0;j<point_in_re.size();j++)
            {
                if(NTDIS(result[i],point_in_re[j])<0.1)
                {
                    ind=false;
                }
            }
            if(ind==true)
            {
                point_not_in_re.push_back(result[i]);
            }
        }
        for(V3DLONG i=0;i<result_2.size();i++)
        {
            ind2=true;
            for(V3DLONG j=0;j<gold.size();j++)
            {
                if(NTDIS(result_2[i],gold[j])<0.1)
                {
                    ind2=false;
                }
            }
            if(ind2==true)
            {
                not_in_gold.push_back(result_2[i]);
            }

        }


        cout<<"little_size = "<<little_aligned.size()<<endl;
        cout<<"middle_size = "<<middle_aligned.size()<<endl;
        QString little_gold = "little_gold.swc";
        QString middle_gold = "middle_gold.swc";
        QString little = "little_all.swc";
        QString middle = "middle_all.swc";
        QString point_not_in_recon = "point_not_in_reconstruction.swc";
        QString point_not_in_gold = "point_not_in_gold.swc";
        for(V3DLONG i=0;i<gold_little.size();i++)
        {
            gold_little[i].pn = -1;
        }
        for(V3DLONG i=0;i<gold_middle.size();i++)
        {
            gold_middle[i].pn = -1;
        }
        QList<NeuronSWC> gold_little_final,gold_middle_final,little_final,middle_final,all,gold_middle_temp;
        gold_little_final = match_point(gold_little,gold_little);
        gold_middle_temp = match_point(gold_middle,gold_middle);
        little_final = match_point(little_aligned,little_aligned);
        middle_final = match_point(middle_aligned,middle_aligned);
        gold_middle_final = match_point(gold_middle_temp,gold_little_final);


        cout<<"                                          gold_little_final = "<<gold_little_final.size()<<endl;
        cout<<"                                          gold_middle_final = "<<gold_middle_final.size()<<endl;

        if (!export_list2file(gold_little_final,little_gold, little_gold))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(gold_middle_final,middle_gold, middle_gold))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }



        if (!export_list2file(little_all, little, little))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(middle_all, middle, middle))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(point_not_in_re, point_not_in_recon,point_not_in_recon))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        if (!export_list2file(not_in_gold, point_not_in_gold,point_not_in_gold))
        {
            printf("fail to write the output swc file.\n");
            return false;
        }
        cout<<"                      not in gold size = "<<not_in_gold.size()<<endl;




        vector<vector<QList<NeuronSWC> > >subarea(4);
        QList<NeuronSWC> subarea_little_final;
        //vector<QList<NeuronSWC> > subarea_little_final_vec;
        QList<NeuronSWC> subarea_middle_final;
        QList<NeuronSWC> subarea_rec_final;
        QList<NeuronSWC> subarea_gold_final;
        QString temp_little;
        QString temp_middle;
        QString temp_point_rec;
        QString temp_point_gold;
        V3DLONG length=1;                             //step_for_get_sub_img
        V3DLONG lens=2;                                //lens_for_cube
        get_subarea_in_nt(fourtype,length,gold_little_final,middle_aligned,point_not_in_re,not_in_gold);

        make_coodinate(fourcood,fourtype,lens);

        get_subarea(result_2,fourcood[0],subarea[0]);
        get_subarea(result_2,fourcood[1],subarea[1]);
        get_subarea(result,fourcood[2],subarea[2]);
        get_subarea(result_2,fourcood[3],subarea[3]);
        for(V3DLONG i=0;i<subarea[0].size();i++)
        {
            subarea_little_final = match_point(subarea[0][i],subarea[0][i]);
        }
        subarea[0].clear();
        for(V3DLONG i=0;i<subarea[0].size();i++)
        {
            subarea[0].push_back(subarea_little_final);
        }
        for(V3DLONG i=0;i<subarea[1].size();i++)
        {
            subarea_middle_final = match_point(subarea[1][i],subarea[1][i]);
        }
        subarea[1].clear();
        for(V3DLONG i=0;i<subarea[1].size();i++)
        {
            subarea[1].push_back(subarea_middle_final);
        }
        for(V3DLONG i=0;i<subarea[2].size();i++)
        {
            subarea_rec_final = match_point(subarea[2][i],subarea[2][i]);
        }
        subarea[2].clear();
        for(V3DLONG i=0;i<subarea[2].size();i++)
        {
            subarea[2].push_back(subarea_rec_final);
        }
        for(V3DLONG i=0;i<subarea[3].size();i++)
        {
            subarea_gold_final = match_point(subarea[3][i],subarea[3][i]);
        }
        subarea[3].clear();
        for(V3DLONG i=0;i<subarea[3].size();i++)
        {
            subarea[3].push_back(subarea_gold_final);
        }

        //     subarea_middle_final = match_point(subarea[1],subarea[1]);
        //     subarea_rec_final = match_point(subarea[2],subarea[2]);
        //      subarea_gold_final = match_point(subarea[3],subarea[3]);
        for(V3DLONG i=0;i<subarea[0].size();i++)
        {
            temp_little = "subarea_little"+QString::number(i+1)+".swc";
            export_list2file(subarea[0][i], temp_little,temp_little);
        }
        for(V3DLONG i=0;i<subarea[1].size();i++)
        {
            temp_middle = "subarea_middle"+QString::number(i+1)+".swc";
            export_list2file(subarea[1][i], temp_middle,temp_middle);
        }
        for(V3DLONG i=0;i<subarea[2].size();i++)
        {
            temp_point_rec = "subarea_point_rec"+QString::number(i+1)+".swc";
            export_list2file(subarea[2][i], temp_point_rec,temp_point_rec);
        }
        for(V3DLONG i=0;i<subarea[3].size();i++)
        {
            temp_point_gold = "subarea_point_gold"+QString::number(i+1)+".swc";
            export_list2file(subarea[3][i], temp_point_gold,temp_point_gold);
        }

        //     export_list2file(subarea_middle_final, temp_middle,temp_middle);
        //     export_list2file(subarea_rec_final, temp_point_rec,temp_point_rec);
        //     export_list2file(subarea_gold_final, temp_point_gold,temp_point_gold);
        for (int i=0;i<map_swc.size();i++)
            if (map_swc[i]) {delete(map_swc[i]); map_swc[i]=NULL;}
}
     cout<<"**************************img part********************"<<endl;

     vector<string> v,v_rec;
     string img = PARA.filename3.toStdString();
     string name_temp_ = PARA.filename1.toStdString();
     SplitString(name_temp_,v_rec,"/");
     int size = v_rec.size();
     cout<<"size = "<<v_rec.size()<<endl;
     cout<<"size2 = "<<v.size()<<endl;
     SplitString(img,v,"/");

     QString name_temp= QString::fromStdString(v_rec[size-1]);
     QString name_little = "_"+name_temp+"_little_1_";
     QString name_middle = "_"+name_temp+"_middle_2_";
     QString name_rec = "_"+name_temp+"_point_not_rec_3_";
     QString name_gold = "_"+name_temp+"_point_not_gold_4_";
     V3DLONG in_sz[4];
     unsigned char * data1d = 0;
     int datatype;
     //QString temp_name;
     //QString a=PARA.filename1.split("/");




     if(type)
     {
         QString image =QString::fromStdString(v[6]);
         cout<<"v3dpbd"<<endl;
         if(!simple_loadimage_wrapper(callback,(char *)img.c_str(), data1d, in_sz, datatype))  {cout<<"load img wrong"<<endl;  return false;}


         get_subimg(image,name_little,fourcood[0],data1d,in_sz,callback);
         //get_subimg(image,name_middle,fourcood[1],data1d,in_sz,callback);
         //get_subimg(image,name_rec,fourcood[2],data1d,in_sz,callback);
         //get_subimg(image,name_gold,fourcood[3],data1d,in_sz,callback);
         processImage(callback,fourcood[0]);

     }
     else
     {
         cout<<"terafly"<<endl;
         if(PARA.model3 == 0)
         {
             cout<<"***************model 1*************************"<<endl;
             get_subimg_terafly(PARA.filename3,name_little,fourcood[0],callback);
             processImage(callback,fourcood[0]);
         }
         else if(PARA.model3 == 1)
         {
             cout<<"***************model 2*************************"<<endl;
             get_subimg_terafly(PARA.filename3,name_middle,fourcood[1],callback);
             processImage(callback,fourcood[1]);
         }
         else if(PARA.model3 == 2)
         {
             cout<<"***************model 3*************************"<<endl;
             get_subimg_terafly(PARA.filename3,name_gold,fourcood[3],callback);
             processImage(callback,fourcood[3]);
         }
         else
         {
             v3d_msg("please input 0,1,2 as correct input");
             return false;
         }
     }




     //
     //processImage(callback,fourcood[2]);
     //


     cout<<"out"<<endl;


     return true;
}



bool prune_branch(NeuronTree &nt, NeuronTree & result, double prune_size)
{

    V3DLONG siz = nt.listNeuron.size();
    vector<V3DLONG> branches(siz,0); //number of branches on the pnt: 0-tip, 1-internal, >=2-branch
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        branches[pid]++;
    }

    double diameter = calculate_diameter(nt, branches);
    printf("diameter=%.3f\n",diameter);

    if (prune_size  == -1 ){
        double thres = 0.05;
        prune_size = diameter * thres;
    }
    //calculate the shortest edge starting from each tip point
    vector<bool> to_prune(siz, false);
    for (V3DLONG i=0;i<siz;i++)
    {
        if (branches[i]!=0) continue;
        //only consider tip points
        vector<V3DLONG> segment;
        double edge_length = 0;
        V3DLONG cur = i;
        V3DLONG pid;
        do
        {
            NeuronSWC s = nt.listNeuron[cur];
            segment.push_back(cur);
            pid = nt.hashNeuron.value(s.pn);
            edge_length += NTDIS(s, nt.listNeuron[pid]);
            cur = pid;
        }
        while (branches[pid]==1 && pid>0);
        if (pid<0)
        {
            printf("The input tree has only 1 root point. Please check.\n");
            return false;
        }
        if (edge_length < prune_size)
        {
            for (int j=0;j<segment.size();j++)
                to_prune[segment[j]] = true;
        }
    }



    //prune branches
    result.listNeuron.clear();
    result.hashNeuron.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        if (!to_prune[i])
        {
            NeuronSWC s = nt.listNeuron[i];
            result.listNeuron.append(s);
            result.hashNeuron.insert(nt.listNeuron[i].n, result.listNeuron.size()-1);
        }
    }

    return true;
}
QList<NeuronSWC>choose_point(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,int thre1,int thre2)
{
    //vector<V3DLONG> mark;
    V3DLONG ind;
    QList<NeuronSWC> choose;
    choose.clear();
    double min_dis;
    double dis;
    cout<<"neuron1.size = "<<neuron1.size()<<endl;
    cout<<"neuron2.size = "<<neuron2.size()<<endl;
    for(V3DLONG i=0;i<neuron1.size();i++)
    {
        min_dis = 10000000;
        for(V3DLONG j=0;j<neuron2.size();j++)
        {

            dis = MHDIS(neuron1[i],neuron2[j]);
            if(dis<min_dis)
            {
                min_dis = dis;

            }

        }

        if((thre1<=min_dis)&&(min_dis<thre2))
        {
            //mark.push_back(i);
            choose.push_back(neuron1[i]);
        }

    }
    return choose;
}
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches)
{
    V3DLONG siz = nt.listNeuron.size();
    vector<vector<double> > longest_path(siz, vector<double>(2,0));//the 1st and 2nd longest path to each node in a rooted tree
    vector<V3DLONG> chd(siz, -1);//immediate child of the current longest path
    for (V3DLONG i=0;i<siz;i++)
    {
        if (branches[i]!=0) continue;
        V3DLONG cur = i;
        V3DLONG pid;
        do
        {
            NeuronSWC s = nt.listNeuron[cur];
            pid = nt.hashNeuron.value(s.pn);
            double dist = NTDIS(s, nt.listNeuron[pid]) + longest_path[cur][0];
            if (dist>longest_path[pid][0])
            {
                chd[pid] = cur;
                longest_path[pid][0] = dist;
            }
            else if (dist>longest_path[pid][1] && chd[pid]!=cur)
                longest_path[pid][1] = dist;
            cur = pid;
        }
        while (branches[cur]!=0 && pid>0);
    }

    double diam = -1;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (longest_path[i][0] + longest_path[i][1]>diam)
            diam = longest_path[i][0]+longest_path[i][1];
    }
    return diam;
}
void SplitString(const string& s, vector<string>& v, const string& c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}
bool make_coodinate(vector<vector<Coodinate> > &four_cood,vector<FourType> &four_type,V3DLONG lens)
{
    Coodinate cood;
    vector<Coodinate> vec_cood;
    for(V3DLONG i=0;i<four_type[0].little.size();i++)
    {
        cood.x = four_type[0].little[i].x;
        cood.y = four_type[0].little[i].y;
        cood.z = four_type[0].little[i].z;
        cood.lens = lens;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
    for(V3DLONG i=0;i<four_type[0].middle.size();i++)
    {
        cood.x = four_type[0].middle[i].x;
        cood.y = four_type[0].middle[i].y;
        cood.z = four_type[0].middle[i].z;
        cood.lens = lens;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
    for(V3DLONG i=0;i<four_type[0].point_rec.size();i++)
    {
        cood.x = four_type[0].point_rec[i].x;
        cood.y = four_type[0].point_rec[i].y;
        cood.z = four_type[0].point_rec[i].z;
        cood.lens = lens;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
    for(V3DLONG i=0;i<four_type[0].point_gold.size();i++)
    {
        cood.x = four_type[0].point_gold[i].x;
        cood.y = four_type[0].point_gold[i].y;
        cood.z = four_type[0].point_gold[i].z;
        cood.lens = lens;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
}
QList<NeuronSWC> match_point(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2)
{
    int ind1;
    QList<NeuronSWC> swc3;
    for(V3DLONG i=0;i<swc1.size();i++)
    {
        ind1=0;
        for(V3DLONG j=i+1;j<swc2.size();j++)
        {
            if(NTDIS(swc1[i],swc2[j])<0.00001)
            { ind1=1;
            }
            continue;
        }
        if(ind1==0)
        {
            swc3.push_back(swc1[i]);
        }
    }
    return swc3;
}
bool get_subarea_in_nt(vector<FourType> &fourtype,V3DLONG length,QList<NeuronSWC> &little,QList<NeuronSWC> &middle,QList<NeuronSWC> &point_rec,QList<NeuronSWC> &point_gold)
{
    V3DLONG i=0;
    FourType type;
    while(i<little.size())
    {
        type.little.push_back(little[i]);
        i=i+length;
    }
    i=0;
    while(i<middle.size())
    {
        type.middle.push_back(middle[i]);
        i=i+length;
    }
    i=0;
    while(i<point_rec.size())
    {
        type.point_rec.push_back(point_rec[i]);
        i=i+length;
    }
    i=0;
    while(i<point_gold.size())
    {
        type.point_gold.push_back(point_gold[i]);
        i=i+length;
    }
    fourtype.push_back(type);
}
bool get_subimg(QString raw_img,QString name,vector<Coodinate> &mean,unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{

    cout<<"enter into get subimg"<<endl;
     V3DLONG im_cropped_sz[4];
     V3DLONG pagesz;
     V3DLONG M = in_sz[0];
     V3DLONG N = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG xe,xb,ye,yb,ze,zb;
     unsigned char *im_cropped = 0;
     cout<<"img_size = "<<mean.size()<<endl;
    for(V3DLONG i =0;i<mean.size();i++)
    {        

        cout<<mean[i].lens<<endl;
        xe=mean[i].x+mean[i].lens;
        xb=mean[i].x-mean[i].lens;
        ye=mean[i].y+mean[i].lens;
        yb=mean[i].y-mean[i].lens;
        ze=mean[i].z+mean[i].lens;
        zb=mean[i].z-mean[i].lens;


        if(xb<0) xb = 0;
        if(xe>=N-1) xe = N-1;
        if(yb<0) yb = 0;
        if(ye>=M-1) ye = M-1;
        if(zb<0) zb = 0;
        if(ze>=N-1) ze = P-1;

//        im_cropped_sz[0] = xe - xb + 1;
//        im_cropped_sz[1] = ye - yb + 1;
//        im_cropped_sz[2] = ze - zb + 1;
        im_cropped_sz[0] = 11;
        im_cropped_sz[1] = 11;
        im_cropped_sz[2] = 11;
        im_cropped_sz[3] = sc;
        cout<<im_cropped_sz[0]<<"   "<<im_cropped_sz[1]<<"    "<<im_cropped_sz[2]<<"    "<<im_cropped_sz[3]<<endl;
        pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
         cout<<"pagesz = "<<pagesz<<endl;
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
        V3DLONG j = 0;
        for(V3DLONG iz = zb; iz <= ze; iz++)
        {
            V3DLONG offsetk = iz*M*N;

            for(V3DLONG iy = yb; iy <= ye; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = xb; ix <= xe; ix++)
                {

                     im_cropped[j] = data1d[offsetk + offsetj + ix];
                     j++;
                }
            }
        }
        QString outimg_file;
        outimg_file = name+QString::number(i)+".tif";
        //outimg_file = raw_img +name+QString::number(i)+".tif";



        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
         if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
    return true;

}
bool get_subimg_terafly(QString inimg_file,QString name,vector<Coodinate> &mean,V3DPluginCallback2 &callback)
{
    cout<<"enter into get subimg for terafly"<<endl;
    V3DLONG im_cropped_sz[4];
    V3DLONG im_cropped_sz2[4];
    V3DLONG pagesz;
    //V3DLONG M = in_sz[0];
    //V3DLONG N = in_sz[1];
    //V3DLONG P = in_sz[2];
    //V3DLONG sc = in_sz[3];
    V3DLONG sc = 1;
    V3DLONG xe,xb,ye,yb,ze,zb;
    V3DLONG xee,xbb,yee,ybb,zee,zbb;
    unsigned char *im_cropped = 0;
    unsigned char *im_cropped2 = 0;
    cout<<"img_size = "<<mean.size()<<endl;
    for(V3DLONG i =0;i<mean.size();i++)
    {
        xe=mean[i].x+mean[i].lens;
        xb=mean[i].x-mean[i].lens;
        ye=mean[i].y+mean[i].lens;
        yb=mean[i].y-mean[i].lens;
        ze=mean[i].z+mean[i].lens;
        zb=mean[i].z-mean[i].lens;
//        cout<<"M = "<<M<<endl;
//        if(xb<0) xb = 0;
//        if(xe>=N-1) xe = N-1;
//        if(yb<0) yb = 0;
//        if(ye>=M-1) ye = M-1;
//        if(zb<0) zb = 0;
//        if(ze>=N-1) ze = P-1;


        xee=mean[i].x+100;
        xbb=mean[i].x-100;
        yee=mean[i].y+100;
        ybb=mean[i].y-100;
        zee=mean[i].z+100;
        zbb=mean[i].z-100;


        im_cropped_sz[0] = xe - xb + 1;
        im_cropped_sz[1] = ye - yb + 1;
        im_cropped_sz[2] = ze - zb + 1;
        im_cropped_sz[3] = sc;


        im_cropped_sz2[0] = xee - xbb + 1;
        im_cropped_sz2[1] = yee - ybb + 1;
        im_cropped_sz2[2] = zee - zbb + 1;
        im_cropped_sz2[3] = sc;

        pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2];

        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

        try {im_cropped2 = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}


        //V3DLONG mysz[4];
        //    mysz[0] = end_x - start_x +1;
        //    mysz[1] = end_y - start_y +1;
        //    mysz[2] = end_z - start_z +1;
        //    mysz[3] = 1;
        //    unsigned char* data1d_crop = 0;
        //V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];

        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);

        //im_cropped2 = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xbb,xee+1,ybb,yee+1,zbb,zee+1);
        QString outimg_file;
        QString outimg_file2;
        outimg_file = name+QString::number(i)+".tif";
        outimg_file2 = "big_"+QString::number(i)+".tif";

       simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
     //  simple_saveimage_wrapper(callback, outimg_file2.toStdString().c_str(),(unsigned char *)im_cropped2,im_cropped_sz2,1);
       if(im_cropped) {delete []im_cropped; im_cropped = 0;}

    }
    return true;
}
bool get_subarea(QList<NeuronSWC> &nt,vector<Coodinate> &fourcood,vector<QList<NeuronSWC> > &subarea)
{
   double min_x;
   double min_y;
   double min_z;
   double max_x;
   double max_y;
   double max_z;
  // QList<NeuronSWC> swc;
    for(V3DLONG i=0; i<nt.size(); i++)
    {
        NeuronSWC curr = nt[i];
        QList<NeuronSWC> sub;

        for(int j=0; j<fourcood.size(); j++)
        {
            if(min_x<0) min_x = 0;
           // if(max_x>=N-1) max_x = N-1;
            if(min_y<0) min_y = 0;
           // if(max_y>=M-1) max_y= M-1;
            if(min_z<0) min_z = 0;
          //  if(max_z>=N-1) max_z = P-1;

            min_x = fourcood[i].x-fourcood[i].lens;
            max_x = fourcood[i].x+fourcood[i].lens;
            min_y = fourcood[i].y-fourcood[i].lens;
            max_y = fourcood[i].y+fourcood[i].lens;
            min_z = fourcood[i].z-fourcood[i].lens;
            max_z = fourcood[i].z+fourcood[i].lens;
            if(curr.x>min_x&& curr.y>min_y && curr.z>min_z &&curr.x<max_x && curr.y<max_y&&curr.z<max_z)
            {
                sub.push_back(curr);
            }
        }
        subarea.push_back(sub);
    }

    return true;
}

QList<NeuronSWC> choose_alignment(QList<NeuronSWC> &neuron,QList<NeuronSWC> &gold,double thres1,double thres2)
{
    QList<NeuronSWC> result;
    V3DLONG siz = neuron.size();
    double dist;
    for(V3DLONG i=0; i<siz-1;i=i+2)
    {
        dist = sqrt((neuron[i].x-neuron[i+1].x)*(neuron[i].x-neuron[i+1].x)
                +(neuron[i].y-neuron[i+1].y)*(neuron[i].y-neuron[i+1].y)
                +(neuron[i].z-neuron[i+1].z)*(neuron[i].z-neuron[i+1].z));
        cout<<"dist = "<<dist<<endl;
        if(dist >= thres1 && dist < thres2)
        {
            result.push_back(neuron[i]);
            gold.push_back(neuron[i+1]);
        }
    }
    return result;
}
NeuronTree mm2nt(vector<MyMarker*> & inswc, QString fileSaveName)
{
    QString tempSaveName = fileSaveName + "temp.swc";
    saveSWC_file(tempSaveName.toStdString(), inswc);
    NeuronTree nt_out = readSWC_file(tempSaveName);
    //const char * tempRemoveName = tempSaveName.toLatin1().data();
    //const char * tempRemoveName = tempSaveName.toStdString().data();
    //if(remove(tempRemoveName))
    QFile f;
    if(!f.remove(tempSaveName))
    {
        cout << "nt_temp file didn't remove."<< endl;
        perror("remove");
    }
    return nt_out;
}

vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName)
{
    QString tempSaveName = fileSaveName + "temp.swc";
    export_neuronList2file(inswc, tempSaveName);
    vector<MyMarker*> mm_out = readSWC_file(tempSaveName.toStdString());
    //const char * tempRemoveName = tempSaveName.toLatin1().data();
    QFile f;
   // if(remove(tempRemoveName))
    if( !f.remove(tempSaveName))
    {
        cout << "mm_temp file didn't remove."<< endl;
        perror("remove");
    }
    return mm_out;
}
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    // myfile<<"# generated by Vaa3D Plugin overlap_gold"<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}
//void processImage_terafly(V3DPluginCallback2 &callback,vector<Coodinate> &fourcood_each)
//{
//    cout<<"processimage"<<endl;
//    v3dhandle curwin = callback.currentImageWindow();
//    if (!curwin)
//    {
//        QMessageBox::information(0, "", "You don't have any image open in the main window.");
//        return;
//    }

//    Image4DSimple* p4DImage = callback.getImage(curwin);

//    if (!p4DImage)
//    {
//        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
//        return;
//    }



//    unsigned char* data1d;

//    QString imgname = callback.getImageName(curwin);
//    //V3DLONG totalpxls = p4DImage->getTotalBytes();

//    data1d = callback.getSubVolumeTeraFly(imgname.toStdString(),xb,xe+1,
//                                               yb,ye+1,zb,ze+1);
//    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

//    V3DLONG N = p4DImage->getXDim();
//    V3DLONG M = p4DImage->getYDim();
//    V3DLONG P = p4DImage->getZDim();
//    V3DLONG sc = p4DImage->getCDim();

//    V3DLONG in_sz[4];
//    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

//   cout<<"qqqqqqqqqqqqqqqqqqqq"<<endl;
//    int tmpx,tmpy,tmpz,x1,y1,z1;
//    LandmarkList listLandmarks = callback.getLandmark(curwin);
//    //LocationSimple tmpLocation(0,0,0);
//    int marknum = fourcood_each.size();
//    if(marknum ==0)
//    {
//        v3d_msg("No markers in the current image, please double check.");
//        return;
//    }

//    NeuronTree marker_windows;
//    QList <NeuronSWC> listNeuron;
//    QHash <int, int>  hashNeuron;
//    listNeuron.clear();
//    hashNeuron.clear();
//    int index = 1;
//    cout<<"fourcood_each.size() = "<<fourcood_each.size()<<endl;
//    for (int i=0;i<fourcood_each.size();i++)
//    {
//       tmpx= fourcood_each[i].x;
//       tmpy=fourcood_each[i].y;
//       tmpz=fourcood_each[i].z;
//        //tmpLocation = listLandmarks.at(i);
//        //tmpLocation.getCoord(tmpx,tmpy,tmpz);
//        V3DLONG ix = tmpx-1;
//        V3DLONG iy = tmpy-1;
//        V3DLONG iz = tmpz-1;
//        V3DLONG offsetk = iz*M*N;
//        V3DLONG offsetj = iy*N;
//        V3DLONG PixelValue = data1d[offsetk + offsetj + ix];
//       // int Ws = 2*(int)round((log(PixelValue)/log(2)));
//        int Ws = fourcood_each[0].lens;
//        //int Ws = 2*PixelValue;
//        printf("window size is %d %d (%d %d %d)\n", Ws,PixelValue,ix,iy,iz);
//        NeuronSWC S;
//        if(Ws>=0)
//        {

//            S.n     = index;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz-Ws;
//            S.r 	= 1;
//            S.pn 	= -1;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+1;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz-Ws;
//            S.r 	= 1;
//            S.pn 	= index;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+2;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz-Ws;
//            S.r 	= 1;
//            S.pn 	= index+1;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+3;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz-Ws;
//            S.r 	= 1;
//            S.pn 	= index+2;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+4;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+3;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+5;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+4;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+6;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+5;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+7;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+6;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+8;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+1;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+9;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+2;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+10;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+3;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);


//            S.n     = index+11;
//            S.type 	= 7;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz-Ws;
//            S.r 	= 1;
//            S.pn 	= index;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);

//            S.n     = index+12;
//            S.type 	= 7;
//            S.x 	= tmpx+Ws;
//            S.y 	= tmpy-Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index+4;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);


//            S.n     = index+13;
//            S.x 	= tmpx-Ws;
//            S.y 	= tmpy+Ws;
//            S.z 	= tmpz+Ws;
//            S.r 	= 1;
//            S.pn 	= index;
//            listNeuron.append(S);
//            hashNeuron.insert(S.n, listNeuron.size()-1);


//             index += 14;

//       }
//        marker_windows.n = -1;
//        marker_windows.on = true;
//        marker_windows.listNeuron = listNeuron;
//        marker_windows.hashNeuron = hashNeuron;

//    }


//    QString outfilename = imgname + "_marker.swc";
//    if (outfilename.startsWith("http", Qt::CaseInsensitive))
//    {
//        QFileInfo ii(outfilename);
//        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
//    }
//    //v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
//    writeSWC_file(outfilename,marker_windows);
//    NeuronTree nt = readSWC_file(outfilename);
//    callback.setSWC(curwin, nt);
//    callback.open3DWindow(curwin);
//    callback.getView3DControl(curwin)->updateWithTriView();
//    v3d_msg(QString("You have totally [%1] markers for the file [%2] and the computed MST has been saved to the file [%3]").arg(marknum).arg(imgname).arg(outfilename));

//    return;
//}


void processImage(V3DPluginCallback2 &callback,vector<Coodinate> &fourcood_each)
{
    cout<<"processimage"<<endl;
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    QString imgname = callback.getImageName(curwin);
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    cout<<"pagesz = "<<pagesz<<endl;

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    int tmpx,tmpy,tmpz,x1,y1,z1;
    LandmarkList listLandmarks = callback.getLandmark(curwin);
    //LocationSimple tmpLocation(0,0,0);
    int marknum = fourcood_each.size();
    if(marknum ==0)
    {
        v3d_msg("No markers in the current image, please double check.");
        return;
    }

    NeuronTree marker_windows;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    int index = 1;
    cout<<"fourcood_each.size() = "<<fourcood_each.size()<<endl;
    for (int i=0;i<fourcood_each.size();i++)
    {
       tmpx= fourcood_each[i].x;
       cout<<"tmpx = "<<tmpx<<endl;
       tmpy=fourcood_each[i].y;
       cout<<"tmpy = "<<tmpy<<endl;
       tmpz=fourcood_each[i].z;
       cout<<"tmpz = "<<tmpz<<endl;
        //tmpLocation = listLandmarks.at(i);
        //tmpLocation.getCoord(tmpx,tmpy,tmpz);
        V3DLONG ix = tmpx-1;
        V3DLONG iy = tmpy-1;
        V3DLONG iz = tmpz-1;
        V3DLONG offsetk = iz*M*N;
        V3DLONG offsetj = iy*N;
        V3DLONG PixelValue;


       PixelValue = data1d[offsetk + offsetj + ix];
       cout<<"PixelValue = "<<PixelValue<<endl;

       // int Ws = 2*(int)round((log(PixelValue)/log(2)));
        int Ws = fourcood_each[0].lens;
        cout<<"ws = "<<Ws<<endl;
        //int Ws = 2*PixelValue;
        //printf("window size is %d %d (%d %d %d)\n", Ws,PixelValue,ix,iy,iz);
        NeuronSWC S;
        if(Ws>=0)
        {

            S.n     = index;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= -1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+1;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+2;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index+1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+3;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index+2;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+4;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+3;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+5;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+4;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+6;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+5;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+7;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+6;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+8;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+9;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+2;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+10;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+3;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


            S.n     = index+11;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+12;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+4;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


            S.n     = index+13;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


             index += 14;

       }
        marker_windows.n = -1;
        marker_windows.on = true;
        marker_windows.listNeuron = listNeuron;
        marker_windows.hashNeuron = hashNeuron;

    }


    QString outfilename = imgname + "_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    //v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
    writeSWC_file(outfilename,marker_windows);
    NeuronTree nt = readSWC_file(outfilename);
    callback.setSWC(curwin, nt);
    callback.open3DWindow(curwin);
    callback.getView3DControl(curwin)->updateWithTriView();
    v3d_msg(QString("You have totally [%1] markers for the file [%2] and the computed MST has been saved to the file [%3]").arg(marknum).arg(imgname).arg(outfilename));

    return;
}
