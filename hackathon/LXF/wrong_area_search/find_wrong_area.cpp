

/* resample_swc_func.cpp
 * This is a plugin to resample neuron swc subject to a fixed step length.
 * 2012-03-02 : by Yinan Wan
 */
#include"wrong_area_search_plugin.h"
#include "find_wrong_area.h"
#include <iostream>
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
//struct classify
//{
//    QList<NeuronSWC> swc;
//    V3DLONG label=0;
//};
bool get_subarea_in_nt(vector<FourType> &fourtype,V3DLONG length,QList<NeuronSWC> &little,QList<NeuronSWC> &middle,QList<NeuronSWC> &point_rec,QList<NeuronSWC> &point_gold);
//bool get_mean_xyz(vector<QList<NeuronSWC> > &subarea,vector<Coodinate>  &mean_xyz_all);
bool make_coodinate(vector<vector<Coodinate> > &four_cood,vector<FourType> &four_type,V3DLONG length);
bool get_subimg(QString raw_img,QString name,vector<Coodinate> &mean,unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
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
    {\
        cout<<"Error in sorting swc"<<endl;
        return false;
    }
    return true;
}

bool find_wrong_area(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 &callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    if(input.size() != 2)
    {
        printf("Please specify both input file and step length parameter.\n");
        return false;
    }
    paralist = (vector<char*>*)(input.at(1).p);
    if(paralist->size()!=2)
    {
        printf("Please specify two parameter -  the resampling step length and ...");
        return false;
    }
    double step = atof(paralist->at(0));
    double pa = atof(paralist->at(1));
    QString fileOpenName = QString(inlist->at(0));
    QString fileSaveName;
    if (output.size()==0)
    {
        printf("No outputfile specified.\n");
        fileSaveName = fileOpenName + "_sort.swc";
    }
    else if (output.size()==1)
    {
        outlist = (vector<char*>*)(output.at(0).p);
        fileSaveName = QString(outlist->at(0));
    }
    else
    {
        printf("You have specified more than 1 output file.\n");
        return false;
    }

    // resample input_1
    NeuronTree nt,temp_neuron;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        nt = readSWC_file(fileOpenName);
    NeuronTree resample_result = resample(nt,step);


    //  find the nearest point as new root node calculated between input_1's all node  and input_2's root node
    cout<<"find_roodID begin.";
    QList<NeuronSWC> neuron1,neuron2;

    neuron1 = resample_result.listNeuron;
    QString fileOpenName2 = QString(inlist->at(1));
    if (fileOpenName2.toUpper().endsWith(".SWC") || fileOpenName2.toUpper().endsWith(".ESWC"))
    {

        temp_neuron = readSWC_file(fileOpenName2);
        neuron2 = readSWC_file(fileOpenName2).listNeuron;
    }
   /*****************************resample************************************************/
    NeuronTree resample_gold = resample(temp_neuron,step);

    QList<NeuronSWC> result,result_2;
    /******************************sort******************************************************/
    sort_with_standard(neuron1,neuron2,result);
    sort_with_standard(resample_gold.listNeuron,neuron2,result_2);
    if (!export_list2file(result, fileSaveName, fileOpenName))
    {
        printf("fail to write the output swc file.\n");
        return false;
    }
    vector<MyMarker *> neuron_m;
    vector<MyMarker *> neuron_gold;
    neuron_m = nt2mm(result,fileSaveName);

    cout<<"size_nt = "<<result.size()<<endl;
    cout<<"size_marker = "<<neuron_m.size()<<endl;
    neuron_gold = nt2mm(result_2,fileOpenName2);
    string name1 = fileSaveName.toStdString();
    string name2 = fileOpenName2.toStdString();
    cout<<"name1 = "<<name1<<endl;
    cout<<"name2 = "<<name2<<endl;

    cout<<"neuron_m_size = "<<neuron_m.size()<<endl;
    cout<<"neuron_gold_size = "<<neuron_gold.size()<<endl;
    //cout<<"hello"<<endl;
    /*******************************local_alignment*********************************/
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
        //cout<<"i= "<<i<<endl;
        aligned_choosen.push_back(nt_for_choose.listNeuron[i]);

    }
    cout<<"size aligned_choosen = "<<aligned_choosen.size()<<endl;

    /********************************choose_alignment*************************************************/
    QList<NeuronSWC> gold,not_in_gold,gold_little,gold_middle,little_all,middle_all;
    little_aligned = choose_alignment(aligned_choosen,gold_little,0,5);
    middle_aligned = choose_alignment(aligned_choosen,gold_middle,5,20000);
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
    //cout<<"                      little gold size = "<<gold_little.size()<<endl;
    for(V3DLONG i=0;i<gold_middle.size();i++)
    {
        gold_middle[i].pn = -1;
    }
    cout<<"                      middle gold size = "<<gold_middle.size()<<endl;


  QList<NeuronSWC> gold_little_final,gold_middle_final,little_final,middle_final,all,gold_middle_temp;
  gold_little_final = match_point(gold_little,gold_little);
  gold_middle_temp = match_point(gold_middle,gold_middle);
  little_final = match_point(little_aligned,little_aligned);
  middle_final = match_point(middle_aligned,middle_aligned);
  gold_middle_final = match_point(gold_middle_temp,gold_little_final);

  //for(V3DLONG i=0;i<mid)


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
    //sort(gold_little.begin(),gold_little.end());
   // gold_little.erase(unique(gold_little.begin(),gold_little.end()),gold_little.end());



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



    vector<FourType> fourtype;
    vector<vector<Coodinate> > fourcood;
    V3DLONG length=9;
    get_subarea_in_nt(fourtype,length,gold_little_final,middle_aligned,point_not_in_re,not_in_gold);
    cout<<"four_type_size = "<<fourtype.size()<<endl;
    make_coodinate(fourcood,fourtype,length);
    //cout<<"fourcood1 = "<<fourcood[0][0].x<<endl;



    QString raw_img = QString(inlist->at(2));
    string img = raw_img.toStdString();
    unsigned char * data1d = 0;
     V3DLONG in_sz[4];
    int datatype;
    QString name_little = "_little_1";
    QString name_middle = "_middle_2";
    QString name_rec = "_point_not_rec_3";
    QString name_gold = "_point_not_gold_1";
    if(!simple_loadimage_wrapper(callback,(char *)img.c_str(), data1d, in_sz, datatype))  {cout<<"load img wrong"<<endl;  return false;}
    cout<<"in_sz2 = "<<in_sz[2]<<endl;


    get_subimg(raw_img,name_little,fourcood[0],data1d,in_sz,callback);
   // get_subimg(raw_img,name_middle,fourcood[1],data1d,in_sz,callback);
   // get_subimg(raw_img,name_rec,fourcood[2],data1d,in_sz,callback);
    get_subimg(raw_img,name_gold,fourcood[3],data1d,in_sz,callback);




    for (int i=0;i<neuron_m.size();i++)
        if (neuron_m[i]) {delete(neuron_m[i]); neuron_m[i]=NULL;}
    for (int i=0;i<neuron2.size();i++)
        if (neuron_gold[i]) {delete(neuron_gold[i]); neuron_gold[i]=NULL;}
    for (int i=0;i<map_swc.size();i++)
        if (map_swc[i]) {delete(map_swc[i]); map_swc[i]=NULL;}



    return true;


}
bool make_coodinate(vector<vector<Coodinate> > &four_cood,vector<FourType> &four_type,V3DLONG length)
{
    //cout<<four_type[0].little.size()<<endl;
    Coodinate cood;
    vector<Coodinate> vec_cood;
    for(V3DLONG i=0;i<four_type[0].little.size();i++)
    {
        cood.x = four_type[0].little[i].x;
        cood.y = four_type[0].little[i].y;
        cood.z = four_type[0].little[i].z;
        cood.lens = 2*length/3;
        vec_cood.push_back(cood);
        //cout<<"cood.x = "<<cood.x<<endl;

    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
    for(V3DLONG i=0;i<four_type[0].middle.size();i++)
    {
        cood.x = four_type[0].middle[i].x;
        cood.y = four_type[0].middle[i].y;
        cood.z = four_type[0].middle[i].z;
        cood.lens = 2*length/3;
        vec_cood.push_back(cood);
       // cout<<"cood.x = "<<cood.x<<endl;

    }
    for(V3DLONG i=0;i<four_type[0].point_rec.size();i++)
    {
        cood.x = four_type[0].point_rec[i].x;
        cood.y = four_type[0].point_rec[i].y;
        cood.z = four_type[0].point_rec[i].z;
        cood.lens = 2*length/3;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();
    for(V3DLONG i=0;i<four_type[0].point_gold.size();i++)
    {
        cood.x = four_type[0].point_gold[i].x;
        cood.y = four_type[0].point_gold[i].y;
        cood.z = four_type[0].point_gold[i].z;
        cood.lens = 2*length/3;
        vec_cood.push_back(cood);
    }
    four_cood.push_back(vec_cood);
    vec_cood.clear();



}
//QList<NeuronSWC> match_point(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2)
//{
//    int ind1;
//    QList<NeuronSWC> swc3,swc4;
//    for(V3DLONG i=0;i<swc1.size();i++)
//    {
//        ind1=0;
//        for(V3DLONG j=0;j<swc2.size();j++)
//        {
//            if(NTDIS(swc1[i],swc2[j])<0.1)
//            {
//                ind1++;
//            }

//        }
//        cout<<"ind1 = "<<ind1<<endl;
//        if(ind1==1)
//        {
//            swc3.push_back(swc1[i]);
//        }


//    }
//    return swc3;
//}

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
    for(V3DLONG i =0;i<mean.size();i++)
    {
        cout<<"img_size = "<<mean.size()<<endl;
        xe=mean[i].x+mean[i].lens;
        xb=mean[i].x -mean[i].lens;
        ye=mean[i].y+mean[i].lens;
        yb=mean[i].y-mean[i].lens;
        ze=mean[i].z+mean[i].lens;
        zb=mean[i].z-mean[i].lens;
        im_cropped_sz[0] = xe - xb + 1;
        im_cropped_sz[1] = ye - yb + 1;
        im_cropped_sz[2] = ze - zb + 1;
        im_cropped_sz[3] = sc;

        pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
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
        cout<<"kkkkkk"<<endl;
        QString outimg_file;
        outimg_file = raw_img + name+QString::number(i+1)+".tif";


        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
         if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
    return true;

}
//bool get_mean_xyz(vector<QList<NeuronSWC> > &subarea,vector<Coodinate>  &mean_xyz_all)
//{
//    //vector<double>  mean_xyz;
//    Coodinate mean_xyz;
//    for(V3DLONG i=0;i<subarea.size();i++)
//    {
//        double sum_x=0;
//        double sum_y=0;
//        double sum_z=0;
//        double mean_x=0;
//        double mean_y=0;
//        double mean_z=0;
//        double max_x=0;
//        double max_y=0;
//        double max_z=0;
//        double temp_x=0;
//        double temp_y=0;
//        double temp_z=0;
//        for(V3DLONG j=0;j<subarea[i].size();j++)
//        {
//            sum_x = sum_x + subarea[i][j].x;
//            sum_y = sum_y + subarea[i][j].y;
//            sum_z = sum_z + subarea[i][j].z;
//        }
//        mean_x=sum_x/subarea[i].size();
//        mean_y=sum_y/subarea[i].size();
//        mean_z=sum_z/subarea[i].size();
//        mean_xyz.x=mean_x;
//        mean_xyz.y=mean_y;
//        mean_xyz.z=mean_z;
//        for(V3DLONG j=0;j<subarea[i].size();j++)
//        {
//           temp_x  = (mean_x-subarea[i][j].x)*(mean_x-subarea[i][j].x);
//           if(temp_x>max_x)
//           {
//                max_x = temp_x;
//           }
//           temp_y  = (mean_y-subarea[i][j].y)*(mean_y-subarea[i][j].y);
//           if(temp_y>max_y)
//           {
//                max_y = temp_y;
//           }
//           temp_z  = (mean_z-subarea[i][j].z)*(mean_z-subarea[i][j].z);
//           if(temp_z>max_z)
//           {
//                max_z = temp_z;
//           }
//        }
//        mean_xyz.lens_x=sqrt(max_x);
//        mean_xyz.lens_y=sqrt(max_y);
//        mean_xyz.lens_z=sqrt(max_z);
//        mean_xyz_all.push_back(mean_xyz);
//    }
//    return true;
//}
bool get_subarea(QList<NeuronSWC> &aligned,vector<QList<NeuronSWC> > &subarea)
{
    cout<<"this is get subarea"<<endl;
    double dis,dis_temp;
    QList<NeuronSWC> temp_S;
     dis_temp=sqrt((aligned[0].x-aligned[1].x)*(aligned[0].x-aligned[1].x)+(aligned[0].y-aligned[1].y)*(aligned[0].y-aligned[1].y)+(aligned[0].z-aligned[1].z)*(aligned[0].z-aligned[1].z));
    cout<<"dis_temp = "<<dis_temp<<endl;
     for(V3DLONG i=0;i<aligned.size()-1;i++)
    {
         temp_S.push_back(aligned[i]);
        dis=sqrt((aligned[i].x-aligned[i+1].x)*(aligned[i].x-aligned[i+1].x)+(aligned[i].y-aligned[i+1].y)*(aligned[i].y-aligned[i+1].y)+(aligned[i].z-aligned[i+1].z)*(aligned[i].z-aligned[i+1].z));
        if(dis>dis_temp+20)                  //change parameter
        {
            cout<<"hahaha"<<endl;
            subarea.push_back(temp_S);
            cout<<"subarea_size = "<<subarea.size()<<endl;
            temp_S.clear();
            cout<<"mmmmmmm"<<endl;
        }

            //cout<<"--------------------------"<<endl;
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



//bool make_label(QList<classify> &cl,QList<NeuronSWC> &rec,QList<NeuronSWC> &gold,QList<NeuronSWC> &little,QList<NeuronSWC> &middle,QList<NeuronSWC> &point1,QList<NeuronSWC> &point2)
//{
//    classify point;
//    bool ind1,ind2,ind3,ind4;
//    for(V3DLONG i=0;i<rec.size();i++)
//    {
//        ind1=true;ind2=true;ind3=true;ind4=true;
//        for(V3DLONG j=0;j<little.size();j++)
//        {
//            if(NTDIS(rec[i],little[j])<0.1)
//            {
//                ind1=false;
//            }
//        }
//        if(ind1=true)
//        {
//            point.swc.push_back(rec[i]);
//            point.label = 1;
//        }
//        if(point.label != 0)
//        {

//        }

//    }

//}


/*
 vector<QList<NeuronSWC> > subarea_little,subarea_middle,subarea_not_in_re,subarea_not_in_gold;
  get_subarea(little_aligned,subarea_little);
  get_subarea(middle_aligned,subarea_middle);
  get_subarea(point_in_re,subarea_not_in_re);
 cout<<"subarea_little_size = "<<subarea_little.size()<<endl;


 QString sub_little;
 QString sub_middle;
 QString sub_long;
 for(V3DLONG i=0;i<subarea_little.size();i++)
 {
     sub_little = "sub_little"+ QString::number(i+1)+".swc";
     export_list2file(subarea_little[i], sub_little, sub_little);
 }
 cout<<"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<endl;
 for(V3DLONG i=0;i<subarea_middle.size();i++)
 {
     sub_middle = "sub_middle"+ QString::number(i+1)+".swc";
     export_list2file(subarea_middle[i], sub_middle, sub_middle);
 }
     cout<<"kkkkkkkkkkkkkkkkkkkkkkkkk"<<endl;
 for(V3DLONG i=0;i<subarea_long.size();i++)
 {
     sub_long = "sub_long"+ QString::number(i+1)+".swc";
     export_list2file(subarea_long[i], sub_long, sub_long);
 }
   cout<<"uuuuuuuuuuuuuuuuuuuuuuui"<<endl;
   vector<Coodinate> mean_little,mean_middle,mean_long;
  get_mean_xyz(subarea_little,mean_little);
  get_mean_xyz(subarea_middle,mean_middle);
  get_mean_xyz(subarea_long,mean_long);
  cout<<"size---mean---little = "<<mean_little.size()<<endl;
  cout<<"mean_little[1] = "<<mean_little[0].x<<"    "<<mean_little[0].y<<"    "<<mean_little[0].z<<"    "<<mean_little[0].lens_x<<"    "<<mean_little[0].lens_y<<"    "<<mean_little[0].lens_z<<endl;


   QString raw_img = QString(inlist->at(2));
   string img = raw_img.toStdString();
   unsigned char * data1d = 0;
    V3DLONG in_sz[4];
   int datatype;
   if(!simple_loadimage_wrapper(callback,(char *)img.c_str(), data1d, in_sz, datatype))  {cout<<"load img wrong"<<endl;  return false;}
   cout<<"in_sz2 = "<<in_sz[2]<<endl;
     QString name_little = "little";
     QString name_middle = "middle";
     QString name_long = "long";
    get_subimg(raw_img,name_little, mean_little,data1d,in_sz,callback);
    get_subimg(raw_img,name_middle,mean_middle,data1d,in_sz,callback);
    get_subimg(raw_img,name_long,mean_long,data1d,in_sz,callback);



*/

//    QList<NeuronSWC>::Iterator it;
//    for(V3DLONG i=0;i<result.size();i++)
//    {
//        it=find_first_not_of(point_in_re.begin(),point_in_re.end(),result[i]);//
//        if(it != point_in_re.exnd())
//        {
//            continue;
//        }
//            cout<<"*it = "<<(*it).x<<endl;
//            point_not_in_re.push_back(*it);
//            cout<<"point_not_in_re size = "<<point_not_in_re.size()<<endl;
//     }
 //   cout<<"llllllllll"<<endl;
//    QList<NeuronSWC>::Iterator it_gold;
//    for(V3DLONG i=0;i<result_2.size();i++)
//    {
//        it_gold=find(gold.begin(),gold.end(),result_2[i]);
//        cout<<"kkkkkkkkkkk"<<endl;
//        if(it_gold == gold.end())
//        {
//             cout<<"*it_gold = "<<(*it_gold).x<<endl;
//            not_in_gold.push_back(*it_gold);
//            cout<<"point_not_in_gold size = "<<not_in_gold.size()<<endl;
//        }
//     }

