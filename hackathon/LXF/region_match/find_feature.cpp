#include "find_feature.h"
#include <vector>
#include "math.h"
#include "sim_measure.h"
#include <QList>
#include <QHash>
#include "match_swc.h"
#include "define.h"

using namespace std;


//#define VOID 1000000000
//#define PI 3.14159265359
//#define min(a,b) (a)<(b)?(a):(b)
//#define max(a,b) (a)>(b)?(a):(b)
//#define mean(a,b) (a+b)/2.0
//#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
//#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
//#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(dist(a,b)*dist(a,c)))*180.0/PI)




double Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
int N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0, Max_Order=0;
double Pd_ratio=0, Contraction=0, Max_Eux=0, Max_Path=0, BifA_local=0, BifA_remote=0, Soma_surface=0, Fragmentation=0;
int rootidx=0;

#define D(A,B) sqrt((A-B)*(A-B))

QVector<QVector<V3DLONG> > childs;



bool get_feature(vector<NeuronTree> &v_nt,NeuronTree &nt,vector<int> &num_sortd)
{
    cout<<"************enter into get_feature*******************************"<<endl;

    double * m_feature_morph = new double[21];

    double * m_feature_gmi = new double[14];
    QList<double*> v_morph_list;
    QList<double* > v_gmi_list;

    double max_a;
    double min_a;


    computeFeature(nt,m_feature_morph);
    computeGMI(nt, m_feature_gmi);


    V3DLONG v_nt_size = v_nt.size();
//    for(int i=0;i<1000000;i++)
//    {
//    cout<<"v_nt_size="<<v_nt_size<<endl;


//    }



//    for(V3DLONG i=0;i<v_nt_size;i++)
//    {
//        for(V3DLONG i = 0;i < 21;i++)
//        {
//            cout<<"m_feature["<<i<<"]= "<<m_feature_morph[i]<<endl;
//        }
//    }


    cout<<"*************compute_v_feature*********************************"<<endl;




    cout<<"v_nt_size ="<<v_nt_size<<endl;
    for(V3DLONG i = 0;i < v_nt_size;i++)
    {
        cout<<"i="<<i<<endl;
        double * v_feature_morph = new double[21];
        double * v_feature_gmi = new double[14];

        computeFeature(v_nt[i],v_feature_morph);



//        for(V3DLONG i = 0;i < 21;i++)
//        {
//            cout<<"v_feature["<<i<<"]= "<<v_feature_morph[i]<<endl;
//        }

        computeGMI(v_nt[i], v_feature_gmi);


        v_morph_list.append(v_feature_morph);//v_feature_morph

        v_gmi_list.append(v_feature_gmi);//v_feature_gmi
    }


    double sum_morph_inner;
    double sum_gmi_inner;

    double sorted_all[35];


   for(V3DLONG i = 0; i < v_nt_size; i++)
   {

       sum_morph_inner = 0;
       sum_gmi_inner = 0;
/***********************************morph begin********************************/
      for(V3DLONG j = 0; j < 21; j++)
       {

      //     cout<<"v_morph_list[i][j] = "<<v_morph_list[i][j]<<endl;
       //    cout<<"model_feature_morph = "<<m_feature_morph[j]<<endl;

           sum_morph_inner = sum_morph_inner + D(v_morph_list[i][j],m_feature_morph[j]);


       }
         //cout<<" sum_morph_inner="<< sum_morph_inner<<endl;

//*********************************morph done gmi begin****************************/
//      for(V3DLONG j = 0; j < 14; j++)
//      {
//          sum_gmi_inner = sum_gmi_inner + D(v_gmi_list[i][j],m_feature_gmi[j]);
//      }
//         cout<<" sum_gmi_inner="<< sum_gmi_inner<<endl;
//   // seq_g[i] = sum_morph_inner;


    sorted_all[i]=sum_morph_inner+sum_gmi_inner;
  //  cout<<"sorted_all[i] = "<<sorted_all[i]<<endl;

  }



   double* arr=sorted_all;






/***************************************regular*****************************************/

    max_a=find_biggest_f(v_nt_size,arr);
    min_a=find_shortest_f(v_nt_size,arr);
    cout<<"max_a="<<max_a<<endl;

    cout<<"min_a = "<<min_a<<endl;



    map<int,double> all_my;
  //

    for(V3DLONG i=0;i<v_nt_size;i++)
    {
     //   cout<<"i =      "<<i<<endl;
     //   cout<<"map.size="<<all_my.size();
        sorted_all[i]=sorted_all[i]/(max_a-min_a);
          //  cout<<"sorted_all[i] = "<<sorted_all[i]<<endl;
        all_my.insert(map<int,double>::value_type(i,sorted_all[i]));
        //cout<<"map.size="<<all_my.size()<<endl;
    }

    double sorted_all_final[v_nt_size];

   // if()
    seq_sorted(v_nt_size,arr);





    for(V3DLONG i=0;i<v_nt_size;i++,arr++)
    {
       // cout<<"arr ="<<*arr<<endl;


        sorted_all_final[i]=*arr;
    //    cout<<"sorted_all_final[]"<<sorted_all_final[i]<<endl;
    }
    for(V3DLONG i=0;i<10;i++,arr++)
    {
        cout<<"arr ="<<*arr<<endl;


       // sorted_all_final[i]=*arr;
        cout<<"sorted_all_final[]"<<sorted_all_final[i]<<endl;
    }



//    double c[10]={8.88888,6.688888,3.388888,5.588888,7.788888,4.455555555,2.25555555,0,9.9555555,1.1555555555};
//    double b[10];
//    double *p=c;
//    seq_sorted(10,p);
//    for(int i=0;i<10;i++,p++)
//    {
//        b[i]=*p;
//        cout<<"hahhaha="<<b[i]<<endl;
//    }





    for(V3DLONG i=0;i<v_nt_size;i++,arr++)
    {
        cout<<"i_in="<<i<<endl;
        for(map<int,double>::iterator iter = all_my.begin();iter!=all_my.end();iter++)
        {
             cout << iter->first << "=" << iter->second << endl;
          //  cout<<"++++++++++++++++++"<<endl;
            if(*arr = iter->second)
            {
                cout<<"*arr = "<<*arr<<endl;
                cout<<"iter-> second = "<<iter->second<<endl;
                num_sortd.push_back(iter->first);
              //  cout << iter->first << "=" << iter->second << endl;
                cout<<"num_sorted.size = "<<num_sortd.size()<<endl;
             //   cout<<"iter->second= "<<iter->second<<endl;
            }
        }
    }


cout<<"%%%%%%%%%%%%%%%%%%%%5"<<endl;





/*
    for(V3DLONG i=0;i<v_nt_size;i++,arr++)
    {
        for(V3DLONG j=0;j<v_nt_size;j++)
        {
            //if(sorted_all_final[i]=all_my.all[j])
            if(*arr=all_my.mapped_type)
            {
                num_sortd.push_back(all_my.n[j]);
                cout<<"all_my.n = "<<all_my.n[j]<<endl;
            }
        }
    }

*/

    return 0;
}




double seq_sorted(V3DLONG v_nt_size,double *seq)
{
    V3DLONG i,j;
    double temp;


    for (i = 0; i < v_nt_size-1; i++)
    {
            if(seq[i]=NULL)
            {
                seq[i]=0;
            }
         for (j = 0; j < v_nt_size-1-i; j++)
          if (seq[j] > seq[j+1])
          {
                temp = seq[j];
                seq[j] = seq[j+1];
                seq[j+1] = temp;
          }

    }
    //return seq;
}



double find_biggest_f(V3DLONG v_nt_size,double* seq)
{
    int max_b= 0;
    for(int i=0;i<v_nt_size;i++)
    {
        if(seq[i]>max_b)
        {
            max_b = seq[i];
        }
    }
    return max_b;
}

double find_shortest_f(V3DLONG v_nt_size,double* seq)
{
    int min_s= 0;
    for(int i=0;i<v_nt_size;i++)
    {
        if(seq[i]<min_s)
        {
            min_s = seq[i];
        }
    }
    return min_s;
}

























bool match_little_pattern(vector<NeuronTree> v_nt,NeuronTree nt,vector<V3DLONG> &num_out,QList<double*> m_morph_list, QList<double*> m_gmi_list,QList<double*> v_morph_list,QList<double*> v_gmi_list)
{
    cout<<"************enter into match_little_pattern********************************"<<endl;

    cout<<"*************compute_model_feature*********************************"<<endl;
   // QList<double*> m_morph_list, m_gmi_list;
    double * m_feature_morph = new double[21];

    double * m_feature_gmi = new double[14];


    m_feature_morph = new double[21];
    m_feature_gmi = new double[14];


    computeFeature(nt,m_feature_morph);




    computeGMI(nt, m_feature_gmi);
    cout<<"m_morph_list append : step two"<<endl;
    m_morph_list.append(m_feature_morph);//model_feature_morph
    m_gmi_list.append(m_feature_gmi);//model_feature_gmi





    for(V3DLONG i=0;i<m_morph_list.size();i++)
    {
        for(V3DLONG i = 0;i < 21;i++)
        {
            cout<<"m_feature["<<i<<"]= "<<m_feature_morph[i]<<endl;
        }
    }



/*
  //  QList<double*> v_morph_list, v_gmi_list;
    vector<int> feature_codes, norm_codes;
    QStringList nameList;
    char *dfile_database = NULL;

    char *dfile_query = NULL;
    char *dfile_result = NULL;
    //char *para_norm = NULL; //normalization method. 1: global feature, 2: GMI, 3: whitening normalization, 4:rankScore
    int retrieved_num = 2;
    double thres = 0.1;
    QString outfileName(dfile_result);
    QString qs_query(dfile_query);
     //   QString qs_query(nameList);
    if (dfile_result==NULL)
        outfileName = qs_query + QString("_retrieved.ano");
    //    outfileName = qs_query +QString("_retrieved.swc");
   // V3DLONG neuronNum;



    char *para_norm = "1,2,4";
    QString q_norm(para_norm);
    cout<<"*************do qstring*********************************"<<endl;

    QStringList splitted = q_norm.split(",");
    if (splitted.contains("1")) feature_codes.push_back(1);
    if (splitted.contains("2")) feature_codes.push_back(2);
    if (splitted.contains("3")) norm_codes.push_back(1);
    if (splitted.contains("4")) norm_codes.push_back(2);

    if (feature_codes.empty() || norm_codes.empty())
    {
        fprintf(stderr, "the norm codes you specified is not supported.\n");
        return 1;
    }


*/


    cout<<"*************compute_v_feature*********************************"<<endl;
    V3DLONG v_nt_size = v_nt.size();
    double * v_feature_morph = new double[21];
    double * v_feature_gmi = new double[14];

    cout<<"v_nt_size ="<<v_nt_size<<endl;
    for(V3DLONG i = 0;i < v_nt_size;i++)
    {
       // cout<<"i="<<i<<endl;

        cout<<"*************compute_feature*********************************"<<endl;
        cout<<"v_nt.size="<<v_nt.size();

        computeFeature(v_nt[i],v_feature_morph);

        for(V3DLONG i = 0;i < 21;i++)
        {
            cout<<"v_feature_morph["<<i<<"]= "<<v_feature_morph[i]<<endl;
        }


        computeGMI(v_nt[i], v_feature_gmi);
        for(V3DLONG i = 0;i < 14;i++)
        {
           cout<<"v_feature_gmi["<<i<<"]= "<<v_feature_gmi[i]<<endl;
        }


        cout<<"append : v_morph_list step one"<<endl;
        v_morph_list.append(v_feature_morph);//v_feature_morph
        v_gmi_list.append(v_feature_gmi);//mv_feature_gmi
    }

    V3DLONG forest_morph_size = v_morph_list.size();
    V3DLONG forest_gmi_size = v_gmi_list.size();
    V3DLONG sum_morph_inner = 0;
    V3DLONG sum_morph_outer = 0;
    V3DLONG sum_gmi_inner = 0;
    V3DLONG sum_gmi_outer = 0;

    for(V3DLONG i = 0; i < forest_morph_size; i++)
    {
        for(V3DLONG j = 0; j < 21; j++)
        {
           cout<<"v_morph_list[i][j] = "<<v_morph_list[0][j]<<endl;
           cout<<"m_morph_list[0][j] = "<<m_morph_list[0][j]<<endl;
            cout<<"i = "<<i<<endl;
            cout<<"j = "<<j<<endl;
            sum_morph_inner = sum_morph_inner + D(v_morph_list[i][j],m_morph_list[0][j]);
            cout<<"sum_morph_inner = "<<sum_morph_inner<<endl;
            cout<<"D1 ="<<D(v_morph_list[i][j],m_morph_list[0][j])<<endl;
        }
        sum_morph_outer = sum_morph_outer +sum_morph_inner;
        cout<<"sum_morph_outer = "<<sum_morph_outer<<endl;
    }

    for(V3DLONG i = 0; i < forest_gmi_size; i++)
    {
        for(V3DLONG j = 0; j < 14; j++)
        {
            sum_gmi_inner = sum_gmi_inner + D(v_gmi_list[i][j],m_gmi_list[0][j]);
            cout<<"D2 = "<<D(v_gmi_list[i][j],m_gmi_list[0][j])<<endl;
            cout<<"sum_gmi_inner = "<<sum_gmi_inner<<endl;


        }
        sum_gmi_outer = sum_gmi_outer +sum_gmi_inner;
        cout<<"sum_gmi_outer = "<<sum_gmi_outer<<endl;
    }




































/*
    vector<vector<V3DLONG> > retrieved_all;

    cout<<"feature_codes,size = "<<feature_codes.size()<<endl;
    cout<<"norm_codes,size = "<<norm_codes.size()<<endl;




    for  (int i=0;i<feature_codes.size();i++)
    {
        cout<<"i i = "<<i<<endl;
        for (int j=0;j<norm_codes.size();j++)
        {
            cout<<"j j ="<<j<<endl;
            vector<V3DLONG> retrieved_tmp;
            if (feature_codes[i]==1)
            {
                if (!neuron_retrieve(nt, v_morph_list, retrieved_tmp, v_nt_size, feature_codes[i], norm_codes[j]))
                {
                    fprintf(stderr,"Error in neuron_retrieval.\n");
                    return 1;
                }
            }
            else if (feature_codes[i]==2)
            {
                if (!neuron_retrieve(nt, v_gmi_list, retrieved_tmp, v_nt_size, feature_codes[i], norm_codes[j]))
                {
                    fprintf(stderr,"Error in neuron_retrieval.\n");
                    return 1;
                }
            }
            cout<<"i = "<<i<<endl;
    cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&  my  if  &&&&&&&&&&&&&&&&&&&&&&&&&7"<<endl;
            if (feature_codes[i]==1)
            {
              for(V3DLONG i = 0;i < 21;i++)
              {
                  cout<<"feature["<<i<<"]= "<<v_morph_list[i]<<endl;
              }
            }
            else if (feature_codes[i]==2)
            {
                for(V3DLONG i = 0;i < 14;i++)
                {
                    cout<<"feature["<<i<<"]= "<<v_gmi_list[i]<<endl;
                }
            }


            if (feature_codes[i]==1)
            {
              for(V3DLONG i = 0;i < v_nt_size;i++)
              {
                  cout<<"retrive_tmp_1["<<i<<"]["<<j<<"]"<<retrieved_tmp[i]<<endl;
              }
            }
            else if (feature_codes[i]==2)
            {
                for(V3DLONG i = 0;i < v_nt_size;i++)
                {
                   cout<<"retrive_tmp_2["<<i<<"]["<<j<<"]"<<retrieved_tmp[i]<<endl;
                }
            }

            retrieved_all.push_back(retrieved_tmp);

            cout<<"sizeof_retrieved_all = "<<retrieved_all.size()<<endl;

        }


    }
    vector<V3DLONG>	retrieved_id;
    int rej_thres = 6;//if top 5 candidates of both method have no intersection, consider this query does not have matched neuron
  //  cout<<i<<endl;

    if (!compute_intersect(retrieved_all, retrieved_id, retrieved_num, rej_thres))
    {
        printf("No similar neurons exist in the database.\n");
    }

    cout<<"sizeof retrieved = "<<retrieved_id.size()<<endl;
   vector<NeuronTree> result_out;

    if (!print_result(retrieved_id,dfile_database, dfile_query,result_out,num_out))
    {
        fprintf(stderr, "Error in print_result.\n");
        return 1;
    }

    cout<<"neuronnum="<<v_nt_size<<endl;
    for (V3DLONG i=0;i<v_nt_size;i++)
    {
        if (v_morph_list[i]) {delete []v_morph_list[i]; v_morph_list[i]=NULL;}
        if (v_gmi_list[i]) {delete []v_gmi_list[i]; v_gmi_list[i]=NULL;}
    }
*/

    return 0;
}


QVector<V3DLONG> getRemoteChild(int t)
{
    QVector<V3DLONG> rchildlist;
    rchildlist.clear();
    int tmp;
    for (int i=0;i<childs[t].size();i++)
    {
        tmp = childs[t].at(i);
        while (childs[tmp].size()==1)
            tmp = childs[tmp].at(0);
        rchildlist.append(tmp);
    }
    return rchildlist;
}

void computeFeature(const NeuronTree & nt, double * features)
{
    Width=0, Height=0, Depth=0, Diameter=0, Length=0, Volume=0, Surface=0, Hausdorff=0;
    N_node=0, N_stem=0, N_bifs=0, N_branch=0, N_tips=0, Max_Order=0;
    Pd_ratio=0, Contraction=0, Max_Eux=0, Max_Path=0, BifA_local=0, BifA_remote=0, Soma_surface=0, Fragmentation=0;
    rootidx=0;
 //   cout<<"*************into 1********************************"<<endl;

    V3DLONG neuronNum = nt.listNeuron.size();

//    cout<<"num="<<neuronNum<<endl;

//    cout<<"*************into 2********************************"<<endl;
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
  //  childs.clear();
    for (V3DLONG i=0;i<neuronNum;i++)
    {
       // cout<<"i="<<i<<endl;
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;

//            cout<<"i = "<<i<<"  ";

        childs[nt.hashNeuron.value(par)].push_back(i);
     //   cout<<"child = "<<childs[0][i]<<"   ";
    }


    //find the root
    rootidx = VOID;
    QList<NeuronSWC> list = nt.listNeuron;
    cout<<"list = "<<list.size()<<endl;
    for (int i=0;i<list.size();i++)
    {
        if (list.at(i).pn==-1){
            //compute the first tree in the forest
            rootidx = i;
  //          cout<<"rootidx = "<<rootidx<<endl;
            break;
        }
    }
 //   cout<<"*************into 3********************************"<<endl;
    if (rootidx==VOID){
        cerr<<"the input neuron tree does not have a root, please check your data"<<endl;
        return;
    }


    N_node = list.size();
    N_stem = childs[rootidx].size();
  //  cout<<"n_stem = "<<childs[rootidx].size()<<endl;
    Soma_surface = 4*PI*(list.at(rootidx).r)*(list.at(rootidx).r);
 //   cout<<"*************into 4*******************************"<<endl;
    computeLinear(nt);
 //   cout<<"*************into 5********************************"<<endl;
    computeTree(nt);
 //   cout<<"*************into 6********************************"<<endl;
//	Hausdorff = computeHausdorff(list,LUT);

    //feature # 0: Number of Nodes
    features[0] = N_node;
    //feature #1: Soma Surface
    features[1] = Soma_surface;
    //feature # 2: Number of Stems
    features[2] = N_stem;
    //feature # 3: Number of Bifurcations
    features[3] = N_bifs;
    //feature # 4: Number of Branches
    features[4] = N_branch;
    //feature # 5: Number of Tips
    features[5] = N_tips;
    //feature # 6: Overall Width
    features[6] = Width;
    //feature # 7: Overall Height
    features[7] = Height;
    //feature # 8: Overall Depth
    features[8] = Depth;
    //feature # 9: Average Diameter
    features[9] = Diameter;
    //feature # 10: Total Length
    features[10] = Length;
    //feature # 11: Total Surface
    features[11] = Surface;
    //feature # 12: Total Volume
    features[12] = Volume;
    //feature # 13: Max Euclidean Distance
    features[13] = Max_Eux;
    //feature # 14: Max Path Distance
    features[14] = Max_Path;
    //feature # 15: Max Branch Order
    features[15] = Max_Order;
    //feature # 16: Average Contraction
    features[16] = Contraction;
    //feature # 17: Average Fragmentation
    features[17] = Fragmentation;
    //feature # 18: Average Parent-daughter Ratio
    features[18] = Pd_ratio;
    //feature # 19: Average Bifurcation Angle Local
    features[19] = BifA_local;
    //feature # 20: Average Bifurcation Angle Remote
    features[20] = BifA_remote;
//	//feature # 21: Hausdorr Dimension
//	features[21] = Hausdorff;		//Hausdorff program crash when running on complex neuron data, we don't use it
}

void computeLinear(const NeuronTree & nt)
{
    double xmin,ymin,zmin;
    xmin = ymin = zmin = VOID;
    double xmax,ymax,zmax;
    xmax = ymax = zmax = 0;
    QList<NeuronSWC> list = nt.listNeuron;
    NeuronSWC soma = list.at(rootidx);

    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        xmin = min(xmin,curr.x); ymin = min(ymin,curr.y); zmin = min(zmin,curr.z);
        xmax = max(xmax,curr.x); ymax = max(ymax,curr.y); zmax = max(zmax,curr.z);
        if (childs[i].size()==0)
            N_tips++;
        else if (childs[i].size()>1)
            N_bifs++;
        int parent = getParent(i,nt);
        if (parent==VOID) continue;
        double l = dist(curr,list.at(parent));
        Diameter += 2*curr.r;
        Length += l;
        Surface += 2*PI*curr.r*l;
        Volume += PI*curr.r*curr.r*l;
        double lsoma = dist(curr,soma);
        Max_Eux = max(Max_Eux,lsoma);
    }
    Width = xmax-xmin;
    Height = ymax-ymin;
    Depth = zmax-zmin;
    Diameter /= list.size();
}

void computeTree(const NeuronTree & nt)
{
    QList<NeuronSWC> list = nt.listNeuron;
    NeuronSWC soma = nt.listNeuron.at(rootidx);

    double * pathTotal = new double[list.size()];
    int * depth = new int[list.size()];
    for (int i=0;i<list.size();i++)
    {
        pathTotal[i] = 0;
        depth[i] = 0;
    }

    QStack<int> stack = QStack<int>();
    stack.push(rootidx);
    double pathlength,eudist,max_local_ang,max_remote_ang;
    V3DLONG N_ratio = 0, N_Contraction = 0;

    if (childs[rootidx].size()>1)
    {
        double local_ang,remote_ang;
        max_local_ang = 0;
        max_remote_ang = 0;
        int ch_local1 = childs[rootidx][0];
        int ch_local2 = childs[rootidx][1];
        local_ang = angle(list.at(rootidx),list.at(ch_local1),list.at(ch_local2));

        int ch_remote1 = getRemoteChild(rootidx).at(0);
        int ch_remote2 = getRemoteChild(rootidx).at(1);
        remote_ang = angle(list.at(rootidx),list.at(ch_remote1),list.at(ch_remote2));
        if (local_ang==local_ang)
            max_local_ang = max(max_local_ang,local_ang);
        if (remote_ang==remote_ang)
            max_remote_ang = max(max_remote_ang,remote_ang);

        BifA_local += max_local_ang;
        BifA_remote += max_remote_ang;
    }

    int t,tmp,fragment;
    while (!stack.isEmpty())
    {
        t = stack.pop();
        QVector<V3DLONG> child = childs[t];
        for (int i=0;i<child.size();i++)
        {
            N_branch++;
            tmp = child[i];
            if (list[t].r > 0)
            {
                N_ratio ++;
                Pd_ratio += list.at(tmp).r/list.at(t).r;
            }
            pathlength = dist(list.at(tmp),list.at(t));

            fragment = 0;
            while (childs[tmp].size()==1)
            {
                int ch = childs[tmp].at(0);
                pathlength += dist(list.at(ch),list.at(tmp));
                fragment++;
                tmp = ch;
            }
            eudist = dist(list.at(tmp),list.at(t));
            Fragmentation += fragment;
            if (pathlength>0)
            {
                Contraction += eudist/pathlength;
                N_Contraction++;
            }

            //we are reaching a tip point or another branch point, computation for this branch is over
            int chsz = childs[tmp].size();
            if (chsz>1)  //another branch
            {
                stack.push(tmp);

                //compute local bif angle and remote bif angle
                double local_ang,remote_ang;
                max_local_ang = 0;
                max_remote_ang = 0;
                int ch_local1 = childs[tmp][0];
                int ch_local2 = childs[tmp][1];
                local_ang = angle(list.at(tmp),list.at(ch_local1),list.at(ch_local2));

                int ch_remote1 = getRemoteChild(tmp).at(0);
                int ch_remote2 = getRemoteChild(tmp).at(1);
                remote_ang = angle(list.at(tmp),list.at(ch_remote1),list.at(ch_remote2));
                if (local_ang==local_ang)
                    max_local_ang = max(max_local_ang,local_ang);
                if (remote_ang==remote_ang)
                    max_remote_ang = max(max_remote_ang,remote_ang);

                BifA_local += max_local_ang;
                BifA_remote += max_remote_ang;
            }
            pathTotal[tmp] = pathTotal[t] + pathlength;
            depth[tmp] = depth[t] + 1;
        }
    }

    Pd_ratio /= N_ratio;
    Fragmentation /= N_branch;
    Contraction /= N_Contraction;

    if (N_bifs==0)
    {
        BifA_local = 0;
        BifA_remote = 0;
    }
    else
    {
        BifA_local /= N_bifs;
        BifA_remote /= N_bifs;
    }

    for (int i=0;i<list.size();i++)
    {
        Max_Path = max(Max_Path,pathTotal[i]);
        Max_Order = max(Max_Order,depth[i]);
    }
    delete pathTotal; pathTotal = NULL;
    delete depth; depth = NULL;
}



void computeGMI(const NeuronTree & nt, double * gmi)
{
   // cout<<"************************computeGMI********************"<<endl;
    QList<NeuronSWC> list = nt.listNeuron;
    QHash<int, int> LUT = QHash<int, int>();
    for (int i=0;i<list.size();i++)
        LUT.insert(list.at(i).n,i);
    double centerpos[3] = {0,0,0};

    int siz = list.size();
    double** b = NULL;
    try {
        b = new double*[siz];
    }
    catch (...)
    {
        fprintf(stderr,"fail to allocate memory");
        if (b) { delete []b; b=NULL;}
        return;
    }
    double avgR = 0;
    for (int i=0;i<siz;i++)
    {
        //here I only kept x,y,z & pn info because others are not used
        b[i] = NULL;
        try
        {
            b[i] = new double[4];
        }
        catch (...)
        {
            fprintf(stderr,"fail to allocate memory");
            if (b[i]) { delete b[i]; b[i]=NULL;}
            return;
        }
        b[i][0] = list.at(i).x;
        b[i][1] = list.at(i).y;
        b[i][2] = list.at(i).z;
        avgR += list.at(i).r;
        if (list.at(i).pn<0) {
            b[i][3] = -1;
        }
        else
            b[i][3] = LUT.value(list.at(i).pn);
        //b[i][4] = list.at(i).r;
    }
    avgR /= siz;
    gmi[13] = avgR;

    double m000 = compute_moments_neuron(b,siz,0,0,0,VOID);
    centerpos[0] = compute_moments_neuron(b,siz,1,0,0,VOID);
    centerpos[1] = compute_moments_neuron(b,siz,0,1,0,VOID);
    centerpos[2] = compute_moments_neuron(b,siz,0,0,1,VOID);

    for (int j=0;j<3;j++)
        centerpos[j] /= m000;


    compute_neuron_GMI(b,siz,centerpos,VOID,gmi);

    for (int i=0;i<siz;i++)
    {
        if (b[i])
        {
            delete b[i];
            b[i] = NULL;
        }
    }
    if (b) {delete b;
        b = NULL; }

        return;
}

double compute_moments_neuron(double ** a, int siz, double p, double q, double r, double radius_thres)
{
    double m = 0;
    double step0=0.1;
    double b1[4],b2[4];

    for (int i=0;i<siz;i++)
    {
        if (a[i][3]<0) continue;
        double sum = 0;
        for (int j=0;j<3;j++)
        {
            b1[j] = a[i][j];
            b2[j] = a[int(a[i][3])][j];
            sum += (b1[j]-b2[j])*(b1[j]-b2[j]);
        }

        double len = sqrt(sum);
        int K = floor(len/step0)+1;

        double xstep,ystep,zstep;//,rstep;
        xstep = (b2[0]-b1[0])/K;
        ystep = (b2[1]-b1[1])/K;
        zstep = (b2[2]-b1[2])/K;
        //rstep = (b2[4]-b1[4])/K;

        double x,y,z,d;//,radius;
        for (int k=1;k<=K;k++)
        {
            x = b1[0]+k*xstep;
            y = b1[1]+k*ystep;
            z = b1[2]+k*zstep;
            //radius = b1[4]+k*rstep;
            d = sqrt(x*x+y*y+z*z);
            if (d>radius_thres) { cout<<"invalid VOID!!"<<endl; break;}

            m += pow(x,p) * pow(y,q) * pow(z,r);
        }

    }
    return m;
}

void compute_neuron_GMI(double **b, int siz,  double* centerpos, double radius_thres, double * gmi)
{
    if (centerpos[0]!=0 || centerpos[1]!=0 || centerpos[2]!=0)
        for (int i=0;i<siz;i++)
            for (int j=0;j<3;j++)
                b[i][j] -= centerpos[j];

    double c000 = compute_moments_neuron(b,siz,0,0,0, radius_thres);

    double c200 = compute_moments_neuron(b,siz, 2,0,0, radius_thres);
    double c020 = compute_moments_neuron(b,siz, 0,2,0, radius_thres);
    double c002 = compute_moments_neuron(b,siz, 0,0,2, radius_thres);
    double c110 = compute_moments_neuron(b,siz, 1,1,0, radius_thres);
    double c101 = compute_moments_neuron(b,siz, 1,0,1, radius_thres);
    double c011 = compute_moments_neuron(b,siz, 0,1,1, radius_thres);

    double c300 = compute_moments_neuron(b,siz, 3,0,0, radius_thres);
    double c030 = compute_moments_neuron(b,siz, 0,3,0, radius_thres);
    double c003 = compute_moments_neuron(b,siz, 0,0,3, radius_thres);
    double c120 = compute_moments_neuron(b,siz, 1,2,0, radius_thres);
    double c102 = compute_moments_neuron(b,siz, 1,0,2, radius_thres);
    double c210 = compute_moments_neuron(b,siz, 2,1,0, radius_thres);
    double c201 = compute_moments_neuron(b,siz, 2,0,1, radius_thres);
    double c012 = compute_moments_neuron(b,siz, 0,1,2, radius_thres);
    double c021 = compute_moments_neuron(b,siz, 0,2,1, radius_thres);
    double c111 = compute_moments_neuron(b,siz, 1,1,1, radius_thres);
    /*
       gmi[0] = (c200+c020+c002)/c000;
       gmi[1] = (c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110)/(c000*c000);
       gmi[2] = (c200*c020*c002+2*c110*c101*c011-c200*c011*c011-c020*c101*c101-c002*c110*c110)/(c000*c000*c000);
       gmi[3] = (c003*c003+6*c012*c012+6*c021*c021+c030*c030+6*c102*c102+15*c111*c111-3*c102*c120+6*c120*c120-3*c021*c201+6*c102*c102-3*c003*(c021+c201)-3*c030*c210+6*c210*c210-3*c012*(c030+c210)-3*c102*c300-3*c120*c300+c300*c300)/(c000*c000);
     */
    //cout<<"c000:"<<c000<<"\tc200:"<<c200<<"\tc020:"<<c020<<"\tc002:"<<c002<<"\tc110:"<<c110<<"\tc101:"<<c101<<"\tc011:"<<c011<<endl;
    //cout<<"c300:"<<c300<<"\tc030:"<<c030<<"\tc003:"<<c003<<"\tc120:"<<c120<<"\tc102:"<<c102<<"\tc210:"<<c210<<"\tc201:"<<c201<<"\tc012:"<<c012<<"\tc021:"<<c021<<"\tc111:"<<c111<<endl;
    //feaVec(:,1) = size(b,1);

    gmi[0] = c000;
    gmi[1] = c200+c020+c002;
    gmi[2] = c200*c020+c020*c002+c002*c200-c101*c101-c011*c011-c110*c110;
    gmi[3] = c200*c020*c002-c002*c110*c110+2*c110*c101*c011-c020*c101*c101-c200*c011*c011;

    double spi = sqrt(PI);

    complex<double> v_0_0 ((2*spi/3)*(c200+c020+c002),0);

    complex<double> v_2_2 (c200-c020,2*c110);
    v_2_2 *= spi*sqrt(2.0/15);
    complex<double> v_2_1 (-2*c101,-2*c011);
    v_2_1 *= spi*sqrt(2.0/15);
    complex<double> v_2_0 (2*c002-c200-c020,0);
    v_2_0 *= spi*sqrt(4.0/45);
    complex<double> v_2_m1 (2*c101,-2*c011);
    v_2_m1 *= spi*sqrt(2.0/15);
    complex<double> v_2_m2 (c200-c020,-2*c110);
    v_2_m2 *= spi*sqrt(2.0/15);

    complex<double> v_3_3 ((-c300+3*c120) , (c030-3*c210));
    v_3_3 *= spi*sqrt(1.0/35);
    complex<double> v_3_2 ((c201-c021), 2*c111);
    v_3_2 *= spi*sqrt(6.0/35);
    complex<double> v_3_1 ((c300+c120-4*c102), (c030+c210-4*c012));
    v_3_1 *= spi*sqrt(3.0/175);
    complex<double> v_3_0 (2*c003 - 3*c201 - 3*c021,0);
    v_3_0 *= spi*sqrt(4.0/175);
    complex<double> v_3_m1 ((-c300-c120+4*c102) , (c030+c210-4*c012));
    v_3_m1 *=  spi*sqrt(3.0/175);
    complex<double> v_3_m2 ((c201-c021) , -2*c111);
    v_3_m2 *=  spi*sqrt(6.0/35);
    complex<double> v_3_m3 ((c300-3*c120) , (c030-3*c210));
    v_3_m3 *= spi*sqrt(1.0/35);

    complex<double> v_1_1 ((-c300-c120-c102), -(c030+c210+c012));
    v_1_1 *= spi*sqrt(6.0/25);
    complex<double> v_1_0 (c003+c201+c021,0);
    v_1_0 *= spi*sqrt(12.0/25);
    complex<double> v_1_m1 ((c300+c120+c102) , -(c030+c210+c012));
    v_1_m1 *= spi*sqrt(6.0/25);

    complex<double> v_g33_2_2 = sqrt(10.0/21)*v_3_3*v_3_m1 - sqrt(20.0/21)*v_3_2*v_3_0 + sqrt(2.0/7)*v_3_1*v_3_1;
    complex<double> v_g33_2_1 = sqrt(25.0/21)*v_3_3*v_3_m2 - sqrt(5.0/7)*v_3_2*v_3_m1 + sqrt(2.0/21)*v_3_1*v_3_0;
    complex<double> v_g33_2_0 = sqrt(25.0/21)*v_3_3*v_3_m3 - sqrt(3.0/7)*v_3_1*v_3_m1 + sqrt(4.0/21)*v_0_0*v_0_0;
    complex<double> v_g33_2_m1 = sqrt(25.0/21)*v_3_m3*v_3_2 - sqrt(5.0/7)*v_3_m2*v_3_1 + sqrt(2.0/21)*v_3_m1*v_3_0;
    complex<double> v_g33_2_m2 = sqrt(10.0/21)*v_3_m3*v_3_1 - sqrt(20.0/21)*v_3_m2*v_3_0 + sqrt(2.0/7)*v_3_m1*v_3_m1;

    complex<double> v_g31_2_2 = -sqrt(1.0/105)*v_3_2*v_1_0 + sqrt(1.0/35)*v_3_3*v_1_m1 + sqrt(1.0/525)*v_3_1*v_1_1;
    complex<double> v_g31_2_1 = sqrt(2.0/105)*v_3_2*v_1_m1 + sqrt(1.0/175)*v_3_0*v_1_1 - sqrt(4.0/525)*v_3_1*v_1_0;
    complex<double> v_g31_2_0 = -sqrt(3.0/175)*v_3_0*v_1_0 + sqrt(2.0/175)*v_3_1*v_1_m1 + sqrt(2.0/175)*v_3_m1*v_1_1;
    complex<double> v_g31_2_m1 = sqrt(2.0/105)*v_3_m2*v_1_1 + sqrt(1.0/175)*v_3_0*v_1_m1 -sqrt(4.0/525)*v_3_m1*v_1_0;
    complex<double> v_g31_2_m2 = -sqrt(1.0/105)*v_3_m2*v_1_0 + sqrt(1.0/35)*v_3_m3*v_1_1 + sqrt(1.0/525)*v_3_m1*v_1_m1;

    complex<double> v_g11_2_2 = 0.2*v_1_1*v_1_1;
    complex<double> v_g11_2_1 = sqrt(2.0/25)*v_1_0*v_1_1;
    complex<double> v_g11_2_0 = sqrt(2.0/75)*(v_1_0*v_1_0 + v_1_1*v_1_m1);
    complex<double> v_g11_2_m1 = sqrt(2.0/25)*v_1_0*v_1_m1;
    complex<double> v_g11_2_m2 = 0.2*v_1_m1*v_1_m1;

    complex<double> tmp;
    tmp = pow(v_0_0,(12.0/5));

    gmi[4] = real((1/sqrt(7.0)) * (v_3_3*v_3_m3*2.0 - v_3_2*v_3_m2*2.0 + v_3_1*v_3_m1*2.0 - v_3_0*v_3_0) / tmp);
    gmi[5] = real((1/sqrt(3.0))* (v_1_1*v_1_m1*2.0 - v_1_0*v_1_0) / tmp);

    tmp = pow(v_0_0,(24.0/5));
    gmi[6] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_g33_2_2*2.0 - v_g33_2_m1*v_g33_2_1*2.0 + v_g33_2_0*v_g33_2_0) / tmp);
    gmi[7] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_g31_2_2*2.0 - v_g31_2_m1*v_g31_2_1*2.0 + v_g31_2_0*v_g31_2_0) / tmp);
    gmi[8] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_g31_2_2 - v_g33_2_m1*v_g31_2_1 + v_g33_2_0*v_g31_2_0 - v_g33_2_1*v_g31_2_m1 + v_g33_2_2*v_g31_2_m2) / tmp);
    gmi[9] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_g11_2_2 - v_g31_2_m1*v_g11_2_1 + v_g31_2_0*v_g11_2_0 - v_g31_2_1*v_g11_2_m1 + v_g31_2_1*v_g11_2_m2) / tmp);

    tmp = pow(v_0_0,(17.0/5));
    gmi[10] = real((1/sqrt(5.0)) * (v_g33_2_m2*v_2_2 - v_g33_2_m2*v_2_1 + v_g33_2_0*v_2_0 - v_g33_2_1*v_2_m1 + v_g33_2_2*v_2_m2) / tmp);
    gmi[11] = real((1/sqrt(5.0)) * (v_g31_2_m2*v_2_2 - v_g31_2_m2*v_2_1 + v_g31_2_0*v_2_0 - v_g31_2_1*v_2_m1 + v_g31_2_2*v_2_m2) / tmp);
    gmi[12] = real((1/sqrt(5.0)) * (v_g11_2_m2*v_2_2 - v_g11_2_m2*v_2_1 + v_g11_2_0*v_2_0 - v_g11_2_1*v_2_m1 + v_g11_2_2*v_2_m2) / tmp);

}

bool neuron_retrieve(NeuronTree query, QList<double*> & feature_list, vector<V3DLONG>  & result, V3DLONG cand, int method_code, int norm_code)
{
    cout<<"************************************into neuron retrieve****************************"<<endl;
    double step_size = 2;
    V3DLONG VSIZE = 0;
    switch (method_code)
    {
        case 1:
            VSIZE = 21;
            break;
        case 2:
            VSIZE = 14;
            break;
        default:
            printf("ERROR: feature extraction method code unsupported.\n");
            return false;
    }

    V3DLONG neuronNum = feature_list.size();

    //pre-process the query neuron
    double prune_size = 2.0;
   // NeuronTree query_preprocessed = pre_process(query, step_size,prune_size);

    double *qf;
    int *sbj;
    double* score;
    try
    {
        qf = new double[VSIZE];
        sbj = new int[cand];
        score = new double[cand];
    }
    catch (...)
    {
        fprintf(stderr,"fail to allocate memory");
        if (qf) {delete []qf; qf=NULL;}
        if (sbj) {delete []sbj; sbj=NULL;}
        if (score) {delete []score; score=NULL;}
        return false;
    }

    if (method_code==1)
    {
        computeFeature(query, qf);
        cout<<"*************cout_feature_m********************************"<<endl;
        for(V3DLONG i = 0;i < 21;i++)
        {
            cout<<"m_feature["<<i<<"]= "<<qf[i]<<endl;
        }
    }
    else if (method_code==2)
    {
        computeGMI(query, qf);
    }

    printf("(2) normalization.\n");
    switch (norm_code)
    {
        case 1:
            if (!retrieve_dist(qf, feature_list, cand, sbj, VSIZE, score))
            {
                printf("Error in retrieve_dist.\n");
                if (qf) {delete []qf; qf=NULL;}
                for (V3DLONG i=0;i<neuronNum;i++)
                    if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
                if (sbj) {delete []sbj; sbj=NULL;}
                if (score) {delete[]score; score=NULL;}
                return false;
            }
            break;
        case 2:
            if (!rankScore(qf, feature_list, cand, sbj, VSIZE, score))
            {
                printf("Error in rankscore.\n");
                if (qf) {delete []qf; qf=NULL;}
                for (V3DLONG i=0;i<neuronNum;i++)
                    if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
                if (sbj) {delete []sbj; sbj=NULL;}
                if (score) {delete []score; score=NULL;}
                return false;
            }
            break;
        default:
            printf("ERROR: normalization code not supported.\n");
            if (qf) {delete []qf; qf=NULL;}
            for (V3DLONG i=0;i<neuronNum;i++)
                if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
            if (sbj) {delete []sbj; sbj=NULL;}
            if (score) {delete[]score; score=NULL;}
            return false;
    }

    printf("(3) store candidates.\n");
    for (V3DLONG i=0;i<cand;i++)
    {
        result.push_back(sbj[i]);
 //  cout<<"result = "<<result[0]<<"...."<<endl;
    cout<<i<<endl;
    }


    if (qf) {delete []qf; qf=NULL;}
    if (sbj) {delete []sbj; sbj=NULL;}
    if (score) {delete[]score; score=NULL;}


    cout<<"^^^^^^^^^^^^^^^^^^"<<endl;


    return true;


}





bool compute_intersect(vector< vector<V3DLONG> > & list, vector<V3DLONG> & result, int number, int rej_thres)
{

    cout<<"**************************************into compute intersect  1*********************8"<<endl;
    result.clear();
    V3DLONG stratNum = list.size();
    V3DLONG neuronNum = list[0].size();

        cout<<"numberNUM  list[0]___1= "<<neuronNum<<endl;
    double * rank_sum = new double[neuronNum];
    for (V3DLONG i=0;i<neuronNum;i++)
        rank_sum[i] = 0;
    for (V3DLONG i=0;i<neuronNum;i++)
        for (V3DLONG j=0;j<stratNum;j++)
            rank_sum[list[j][i]] += i;

    int * idx = new int[number];
    double * score = new double[number];
    for (V3DLONG i=0;i<neuronNum;i++)
        rank_sum[i] = -rank_sum[i];

    pick_max_n(idx, rank_sum, number, neuronNum, score);


    cout<<"score = "<<score<<endl;
/*	for (V3DLONG siz=1;siz<=l1.size();siz++)
    {
        result.clear();
        for (V3DLONG i=0;i<siz;i++)
        {
            for (V3DLONG j=0;j<siz;j++)
            {
                if (l1[i]==l2[j])
                {
                    result.push_back(l1[i]);
                    break;
                }
                if (result.size()>=number) return true;
            }
        }
    }*/



    cout<<"number = "<<number<<endl;

    for (int i=0;i<number;i++)
        result.push_back(idx[i]);
    if (rank_sum) {delete []rank_sum; rank_sum=NULL;}
    if (idx) {delete []idx; idx=NULL;}
    if (score) {delete []score; score=NULL;}

    return true;
}
bool compute_intersect(vector<V3DLONG> l1, vector<V3DLONG> l2, vector<V3DLONG> & result, double thres)
{

      cout<<"**************************************into compute intersect  2*********************8"<<endl;

    if (l1.size()!=l2.size()) return false;
    int siz = (int) (l1.size() * thres);
    result.clear();
    for (V3DLONG i=0;i<siz;i++)
    {
        for (V3DLONG j=0;j<siz;j++)
        {
            if (l1[i]==l2[j])
            {
                result.push_back(l1[i]);
                break;
            }
        }
    }
    return true;
}
bool print_result(vector<V3DLONG> result,const char* database_name, const char* query_name,vector<NeuronTree> &result_out,vector<V3DLONG> &num_out)
{
       cout<<"**************************************into print result*********************8"<<endl;


    cout<<"sizeofresult = "<<result.size()<<endl;
    for (int i=0;i<result.size();i++)
    {
        cout<<"result["<<i<<"] = "<<i<<endl;

        num_out.push_back(result[i]);

    }
           cout<<"**************************************eeeeeeeeeeeeeeeeeeeeeee*********************8"<<endl;

    return true;
}




/*
bool print_result(vector<V3DLONG> result, const char* file_name, QStringList name_list, const char* database_name, const char* query_name)
{



    FILE * fp;
    fp = fopen(file_name, "w");
    if (fp==NULL)
    {
        fprintf(stderr,"ERROR: %s: failed to open file to write!\n",file_name);
        return false;
    }
    fprintf(fp,"#database:          %s\n",database_name);
    fprintf(fp,"#query neuron:      %s\n",query_name);
    fprintf(fp,"#num of candidates: %d\n",result.size());

    cout<<"result,size="<<result.size()<<endl;

    //NeuronTree resultSWC;
    for (int i=0;i<result.size();i++)
    {
        cout<<"result[1]"<<result[1]<<endl;

        cout<<"^$$$$$$$$$$$$$$$$$$$$4"<<endl;

        fprintf(fp,qPrintable(name_list[result[i]]));
    //    NeuronTree resultSWC;
      //  resultSWC = readSWC_file(name_list[result[i]]);
       // R.push_back(resultSWC);
        fprintf(fp,"\n");
    }
    printf("Linker file %s has been successfully generated.\n", file_name);
    fclose(fp);

    cout<<"^$$$$$$$$$$$$$$$$$$$$4"<<endl;

    return true;
}
*/

