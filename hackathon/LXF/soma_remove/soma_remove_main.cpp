<<<<<<< HEAD
#include "soma_remove_main.h"

#include <iostream>
=======
﻿#include "soma_remove_main.h"
#include "v3d_interface.h"
//#include "../../../../v3d_external/v3d_main/plugin_loader/v3d_plugin_loader.h"
#include "openfileDialog.h"
#include <iostream>
#include <set>
>>>>>>> remove_glio version which is operational
#include "string.h"
 using namespace std;
//#define DIS((x,y,z),b) (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z)
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
//#define NTDISs(a,b) (sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z)))
bool FAR = 0;
bool CLOSE = 1;
<<<<<<< HEAD
int dis_thresh = 3;

vector<bool> classify_glio(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method);
bool export_training_data(const QString &fileOpenName,Chart &chart,Each_line &E);
=======
int dis_thresh = 2;
QString model;
bool bo=false;
using namespace boost;
typedef subgraph< adjacency_list<vecS, vecS, directedS,property<vertex_color_t, int>, property<edge_index_t, int> > > Graph;
typedef graph_traits<Graph>::vertex_iterator vertex_iter;

vector<bool> classify_glio(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method);
bool export_training_data(const QString &fileOpenName,Each_line &E1,Each_line &E2,double &logsigma,vector<vector<double> >&R);
void bubblesort(vector<int> &A);
bool simple_saveimage_wrapper_lxf(V3DPluginCallback2 & cb, const char * filename, unsigned char * pdata, V3DLONG sz[4], int datatype);
>>>>>>> remove_glio version which is operational
//bool if_is_connect(Coordinate* &curr,Coordinate* &b,vector<vector<vector<V3DLONG> > > &mark3D);
uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}
uint qHash(const vector<Coordinate> key)
{
    return key[0].x + key[0].y + key[0].z;
}

bool soma_remove_main(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    int thresh = 30;                            //need to be change
    double signal_thresh = 4;                        //need to be change
    double r = 5;                                  //need to be change
    V3DLONG im_cropped_sz[4];
    unsigned char *im_cropped = 0;


    double d = 2*r;
    int y=0;
    im_cropped_sz[0] = in_sz[0];
    im_cropped_sz[1] = in_sz[1];
    im_cropped_sz[2] = in_sz[2];
    im_cropped_sz[3] = in_sz[3];
    V3DLONG pagesz;
    //cout<<im_cropped_sz[0]<<"   "<<im_cropped_sz[1]<<"    "<<im_cropped_sz[2]<<"    "<<im_cropped_sz[3]<<endl;
    pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
     cout<<"pagesz = "<<pagesz<<endl;
    try {im_cropped = new unsigned char [pagesz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

     vector<vector<vector<V3DLONG> > > coodinate3D;
     vector<vector<V3DLONG> > coodinate2D;
     vector<V3DLONG> coodinate1D;

     for(V3DLONG iz = 0; iz < P; iz++)
     {
         V3DLONG offsetk = iz*M*N;
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 V3DLONG tmp = data1d[offsetk + offsetj + ix];
                 if(tmp==0)y++;
                 coodinate1D.push_back(tmp);
             }
             coodinate2D.push_back(coodinate1D);
             coodinate1D.clear();
         }
         coodinate3D.push_back(coodinate2D);
         coodinate2D.clear();
     }
     for(V3DLONG iz = 1; iz < P-1; iz++)
     {
         for(V3DLONG iy = 1; iy < N-1; iy++)
         {
             for(V3DLONG ix = 1; ix < M-1; ix++)
             {
                 int bri = coodinate3D[iz][iy][ix];

                 //cout<<"bri = "<<bri<<endl;
                 if(bri>thresh)
                 {

                     //NI = coodinate3D[iz][iy][ix-1]
                     int SI = coodinate3D[iz][iy][ix+1];
                     int EI = coodinate3D[iz][iy-1][ix];
                     int WI = coodinate3D[iz][iy+1][ix];
                     //DI = coodinate3D[iz-1][iy][ix]
                     //JI = coodinate3D[iz+1][iy][ix]
                     if((SI - bri)*(SI - bri)< signal_thresh)
                     {
                         double count_1=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix+ixi]>thresh)
                                 {
                                     count_1++;
                                 }
                             }
                         }

                         double per_1 = (r*r-count_1)/(r*r);
                         //cout<<"per_1 = "<<per_1<<endl;
                         if(per_1>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
<<<<<<< HEAD
                                     cout<<"33333333333333"<<endl;
=======
>>>>>>> remove_glio version which is operational
                                     if(ix+l>M-1)continue;
                                     if(iy+k>N-1)continue;
                                     coodinate3D[iz][iy+k][ix+l]=0;
                                 }
                             }
                         }


                     }
                     if((WI - bri)*(WI - bri)< signal_thresh)
                     {
                         double count_2=0;
                         for(int ixi=0;ixi<r;ixi++)
                         {
                             for(int iyi=0;iyi<r;iyi++)
                             {
                                 if(ix-r/2+ixi>M-1)continue;
                                 if(iy+iyi>N-1)continue;
                                 if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                                 {
                                     count_2++;
                                 }
                             }
                         }
                         double per_2 = (r*r-count_2)/(r*r);
                         //cout<<"per_2 = "<<per_2<<endl;
                         if(per_2>0.7)
                         {
                             for(int k=0;k<d;k++)
                             {
                                 for(int l=0;l<d;l++)
                                 {
                                     if(ix-r/2+l>M-1)continue;
                                     if(iy+k>N-1)continue;
<<<<<<< HEAD
                                     cout<<"22222222222222"<<endl;
=======

>>>>>>> remove_glio version which is operational
                                     coodinate3D[iz][iy+k][ix-r/2+l]=0;
                                 }
                             }
                         }
                     }
                     //                         if((EI - bri)*(EI -u bri)< signal_thresh)
                     //                         {
                     //                             double count_3=0;
                     //                             for(int ixi=0;ixi<r;ixi++)
                     //                             {
                     //                                 for(int iyi=0;iyi<r;iyi++)
                     //                                 {
                     //                                     if(coodinate3D[iz][iy+iyi][ix-r/2+ixi]>thresh)
                     //                                     {
                     //                                         count_3++;
                     //                                     }
                     //                                 }
                     //                             }
                     //                             double per_3 = (r*r-count_2)/(r*r);
                     //                             if(per_3>0.7)
                     //                             {

                     //                             }
                     //                         }

                 }
             }
         }
     }
<<<<<<< HEAD
     cout<<"hahhahaa"<<endl;
=======
>>>>>>> remove_glio version which is operational
     //int u=0;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         V3DLONG offsetk = iz*M*N;
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             V3DLONG offsetj = iy*N;
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 im_cropped[offsetk + offsetj + ix] = coodinate3D[iz][iy][ix];
                 //u++;
             }
         }
     }
     int u=0;
     for(V3DLONG iz = 0; iz < P; iz++)
     {
         for(V3DLONG iy = 0; iy < N; iy++)
         {
             for(V3DLONG ix = 0; ix < M; ix++)
             {
                 int o = coodinate3D[iz][iy][ix];
                 if(0==0)
                 {
                     u++;
                 }
             }
         }
     }
     cout<<u<<"  "<<y<<endl;
     v3d_msg("check");
         simple_saveimage_wrapper(callback,QString("tmpimg.v3draw").toStdString().c_str(),im_cropped,in_sz,1);




}
bool soma_remove_main_2(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    bool first_floor = false;
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG C = in_sz[3];
    V3DLONG pagesz = M*N*P;
    cout<<"1.make binary image."<<endl;
    unsigned char *im_cropped = 0;
    try {im_cropped = new unsigned char [pagesz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
    V3DLONG signal_all=0;
    for(V3DLONG i=0;i<pagesz;i++)
    {
        signal_all = signal_all + int(data1d[i]);
    }
    V3DLONG signal = signal_all/pagesz;
    cout<<"signal_all = "<<signal_all<<endl;
    cout<<"signal = "<<signal<<endl;
<<<<<<< HEAD
=======
    double lll=0;
    for(V3DLONG i=0;i<pagesz;i++)
    {
        if(int(data1d[i]) < signal+10)
        {
            lll++;
        }
    }
>>>>>>> remove_glio version which is operational
    for(V3DLONG i=0;i<pagesz;i++)
    {
        if(int(data1d[i]) < signal+10)
        {
            im_cropped[i] = 0;
        }
        else
        {
            im_cropped[i] = 255;
        }
    }
    simple_saveimage_wrapper(callback,QString("binary.v3draw").toStdString().c_str(),im_cropped,in_sz,1);
<<<<<<< HEAD
    cout<<"2.make connected area."<<endl;
    vector<vector<vector<V3DLONG> > > coodinate3D,mark3D;
    vector<vector<V3DLONG> > coodinate2D,mark2D;
    vector<V3DLONG> coodinate1D,mark1D;
=======
   // simple_saveimage_wrapper(callback,QString("ori_img.v3draw").toStdString().c_str(),data1d,in_sz,1);
 //   v3d_msg("llllllll");
    cout<<lll/pagesz<<endl;

    cout<<"2.make connected area."<<endl;


    vector<vector<vector<V3DLONG> > > coordinate3D,mark3D,bri3D;
    vector<vector<V3DLONG> > coodinate2D,mark2D,bri2D;
    vector<V3DLONG> coodinate1D,mark1D,bri1D;
>>>>>>> remove_glio version which is operational
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < N; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < M; ix++)
            {
                V3DLONG tmp = im_cropped[offsetk + offsetj + ix];
<<<<<<< HEAD
                ImageMarker coord_curr;
=======
                V3DLONG bri = int(data1d[offsetk + offsetj + ix]);
>>>>>>> remove_glio version which is operational
                if(tmp==255)
                {
                    int one=1;
                    mark1D.push_back(one);
<<<<<<< HEAD

=======
                }
                else
                {
                    int zero=0;
                    mark1D.push_back(zero);
                }
                coodinate1D.push_back(tmp);
                bri1D.push_back(bri);
            }
            coodinate2D.push_back(coodinate1D);
            mark2D.push_back(mark1D);
            bri2D.push_back(bri1D);
            coodinate1D.clear();
            mark1D.clear();
            bri1D.clear();
        }
        coordinate3D.push_back(coodinate2D);
        mark3D.push_back(mark2D);
        bri3D.push_back(bri2D);
        coodinate2D.clear();
        mark2D.clear();
        bri2D.clear();
    }
   cout<<"coordinate3D = "<<coordinate3D.size()<<endl;

   //step2:make connected region
   vector<vector<Coordinate> >connected_region;
   QHash<Coordinate,int> map_index;
   vector<Coordinate> each_region1, each_region2;
   QList<ImageMarker> marker_all;
   int j = 1,u = 0,g = 0;
   int black1 = 0,black2 = 0,white = 0;
   int unknow = 0;
   int all = 0;
   for(int iz = 0; iz < P; iz++)
   {
       for(int iy = 0; iy < N; iy++)
       {
           for(int ix = 0; ix < M; ix++)
           {
               all++;
               Coordinate coord_curr;
               if(coordinate3D[iz][iy][ix]==255)
               {
                   if(ix==1)
                   {
                       coord_curr.x = ix;
                       coord_curr.y = iy;
                       coord_curr.z = iz;
                       coord_curr.bri = bri3D[iz][iy][ix];
                       each_region1.push_back(coord_curr);
                       connected_region.push_back(each_region1);
                       map_index.insert(coord_curr,j);
                       each_region1.clear();
                       j++;
                       g++;
                       first_floor = true;
                       unknow++;
                       continue;
                   }
               }
               else
               {
                   black1++;
                   continue;
               }
               if(ix!=1) //first_floor
               {
                   if(coordinate3D[iz][iy][ix]==255)
                   {
                       white++;
                       int dex;

                       bool ind1 = false;
                       coord_curr.x = ix;
                       coord_curr.y = iy;
                       coord_curr.z = iz;
                       coord_curr.bri = bri3D[iz][iy][ix];

                       ImageMarker m;
                       m.x = coord_curr.x;
                       m.y = coord_curr.y;
                       m.z = coord_curr.z;
                       m.radius = 0.5;
                       m.type = 2;
                       marker_all.push_back(m);

                       for(int l=0;l<connected_region.size();l++)
                       {
                           for(int k=0;k<connected_region[l].size();k++)
                           {
                               if(if_is_connect(coord_curr,connected_region[l][k],mark3D))
                               {
                                   dex = l;
                                   ind1 = true;
                                   break;
                               }
                           }
                           if(ind1)
                           {
                               break;
                           }

                       }
                       if(ind1)
                       {
                           connected_region[dex].push_back(coord_curr);
                           map_index.insert(coord_curr,map_index[connected_region[dex][0]]);
                           ind1 = false;
                           //break;
                       }
                       else
                       {
                           each_region2.clear();
                           each_region2.push_back(coord_curr);
                           u++;
                           connected_region.push_back(each_region2);
                           map_index.insert(coord_curr,j);
                           j++;

                       }
                       each_region2.clear();

                   }
                   else
                   {
                       black2++;
                   }

               }
           }
       }

   }

   cout<<"marker_all = "<<marker_all.size()<<endl;
   cout<<"connected_region.size() = "<<connected_region.size()<<endl;
   //ouput connected region
   for(V3DLONG i = 0;i < connected_region.size();i++)
   {
       QList<ImageMarker> region_marker;
       for(V3DLONG j = 0;j < connected_region[i].size();j++)
       {
           ImageMarker m;
           m.x = connected_region[i][j].x;
           m.y = connected_region[i][j].y;
           m.z = connected_region[i][j].z;
           m.radius = 0;
           m.type = 274;
           region_marker.push_back(m);

       }
       //writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+"_connected_region.marker"),region_marker);  //each region .marker
      // writeTXT_file(QString(p4DImage->getFileName()+QString::number(i)+"_connected_region.txt"),region_marker);
   }
   //v3d_msg("connected_region!");

   //step3:make max connected region   --make distance matrix
   //vector<vector<int> > dist_matrix=vector<vector<int> >(n,vector<int>());
   vector<vector<double> > origin_dist_matrix;
   for(int i=0;i<connected_region.size();i++)
   {
       vector<double> v;
       for(int j=0;j<connected_region.size();j++)
       {
           v.push_back(0);
       }
       origin_dist_matrix.push_back(v);
   }
   double dis,dis_thresh = 1.5;
   for(V3DLONG i = 0;i < connected_region.size();i++)
   {
       for(V3DLONG j = i+1;j< connected_region.size();j++)
       {
           double min_dis = 1000000000;
           for(V3DLONG k = 0;k < connected_region[i].size();k++)
           {
               for(V3DLONG l = 0;l < connected_region[j].size();l++)
               {
                   dis = NTDIS(connected_region[i][k],connected_region[j][l]);
                   if(dis < min_dis)
                        min_dis = dis;
               }
               //cout<<"min_dis = "<<min_dis<<endl;
           }
           origin_dist_matrix[i][j] = min_dis;
       }
   }
//
//   for(int i = 0;i <connected_region.size();i++)
//   {
//       for(int j = 0;j<connected_region.size();j++)
//       {
//           cout<<origin_dist_matrix[i][j]<<"\t";
//       }
//       cout<<endl;
//   }
   //v3d_msg("show origin distance");

   //create origin distance matrix.txt file
//   QString filename = PARA.inimg_file + "_distance.txt";
//   QFile data(filename);

//   if(!data.open(QFile::WriteOnly | QIODevice::Text))
//   {
//       //return -1;
//       QMessageBox::information(0, "", "create .txt file failed");
//       //return;
//   }

//   QTextStream out(&data);
//   for(int i = 0;i <connected_region.size();i++)
//   {
//       for(int j = 0;j<connected_region.size();j++)
//       {
//           out<<origin_dist_matrix[i][j]<<" ";
//       }
//       out<<"\n";
//       cout<<endl;
//   }
//   data.close();

   vector<vector<int> > dist_matrix;
   for(int i=0;i<connected_region.size();i++)
   {
       vector<int> v;
       for(int j=0;j<connected_region.size();j++)
       {
           v.push_back(0);
       }
       dist_matrix.push_back(v);
   }

   for(int i = 0;i < connected_region.size();i++)
   {
       for(int j = 0;j< connected_region.size();j++)
       {
           double min_dis = 1000000000;
           for(V3DLONG k = 0;k < connected_region[i].size();k++)
           {
               for(V3DLONG l = 0;l < connected_region[j].size();l++)
               {
                   dis = NTDIS(connected_region[i][k],connected_region[j][l]);
                   if(dis < min_dis)
                        min_dis = dis;
               }

           }
           if(min_dis <= dis_thresh)
           {
               dist_matrix[i][j] = 1;
           }
       }
   }

   //cout<<dist_matrix[31]
//   cout<<"distance matrix:"<<endl;
//   for(int i = 0;i <connected_region.size();i++)
//   {
//       for(int j = 0;j<connected_region.size();j++)
//       {
//           cout<<dist_matrix[i][j]<<"\t";
//       }
//       cout<<endl;
//   }
   //v3d_msg("show distance matrix");

//    QHash<int,set<int> > vec;
//    int number = 0;
//    for(int i=0;i<connected_region.size();i++)
//    {
//        for(int j=0;j<connected_region.size();j++)
//        {
//            vec[i].insert(dist_matrix[i][j]);
//        }
//        cout<<"vec "<<i<<" = "<<vec[i].size()<<"  ";
//        for(set<int>::iterator it=vec[i].begin();it != vec[i].end();++it)
//        {
//            cout<<*it<<" ";
//        }
//        cout<<endl;
//        if(vec[i].size()==1)
//            number++;
//    }
//    cout<<"number = "<<number<<endl;
   //v3d_msg("number");

   //boost graph libiary
   Graph G;
   for(int i = 0;i <connected_region.size();i++)
   {
       for(int j = 0;j<connected_region.size();j++)
       {
           if(dist_matrix[i][j] == 1)
                add_edge(i,j,G);
       }
   }

   std::vector<int> comp(num_vertices(G));
   int num = connected_components (G, comp.data());
   std::cout << "Total number of components: " << num << std::endl;

   std::cout << std::endl;
   std::vector < int >::iterator i;
   for (i = comp.begin(); i != comp.end(); ++i)
       std::cout << "Vertex " << i - comp.begin()<< " is in component " << *i << std::endl;

   std::vector<Graph* > comps(num);
   for(size_t i=0;i<num;++i)
   {
       comps[i] = & G.create_subgraph();
   }

   for(size_t i=0;i<num_vertices(G);++i)
   {
       cout<<"add vetex "<<i<<"to sub graph "<<comp[i]<<endl;
       add_vertex(i, *comps[comp[i]]);
   }

   pair<vertex_iter, vertex_iter> vip;
   cout << "Vertices in G  = [ ";
   vip = vertices(G);

   for(vertex_iter vi = vip.first; vi != vip.second; ++vi)
   {
        cout << *vi << " ";
   }
   cout << "]" << endl;

   for(size_t i=0;i<num;i++)
   {
       cout << "Vertices (local) in comps[i]' = [ ";
       pair<vertex_iter, vertex_iter> lvip;
       lvip = vertices(*comps[i]);

       for(vertex_iter vi = lvip.first; vi != lvip.second; ++vi)
       {
           cout << (*comps[i]).local_to_global(*vi) << " ";
       }

       cout << "]" << endl;

   }

   vector<vector<Coordinate> >connected_region_final_final;
   QHash<int,set<int> > max_relate;
   for(size_t i=0;i<num;i++)
   {
       pair<vertex_iter, vertex_iter> lvip;
       lvip = vertices(*comps[i]);

       for(vertex_iter vi = lvip.first; vi != lvip.second; ++vi)
       {
           max_relate[i].insert((*comps[i]).local_to_global(*vi));
       }
       cout<<"max_relate "<<i<<" = "<<max_relate[i].size()<<"   ";
       for(set<int>::iterator it = max_relate[i].begin();it != max_relate[i].end();++it)
       {
           cout<<*it<<" ";
       }
       cout<<endl;

   }

   for(QHash<int,set<int> >::iterator it=max_relate.begin();it!=max_relate.end();++it)
   {
       vector<Coordinate> tmp;
       for(set<int>::iterator it2=it.value().begin();it2!=it.value().end();++it2)
       {
           for(int i=0;i<connected_region[*it2].size();i++)
           {
               tmp.push_back(connected_region[*it2][i]);
           }

       }
       connected_region_final_final.push_back(tmp);
   }

   cout<<"connected_region_final_final = "<<connected_region_final_final.size()<<endl;

//   for(V3DLONG i=0;i<max_connected_region.size();i++)
//   {
//       QList<ImageMarker> connected_region_marker;
//       for(V3DLONG j=0;j<max_connected_region[i].size();j++)
//       {
//           ImageMarker m;
//           m.x = max_connected_region[i][j].x;
//           m.y = max_connected_region[i][j].y;
//           m.z = max_connected_region[i][j].z;
//           m.radius = 0.0;
//           m.type = 274;
//           connected_region_marker.push_back(m);

//       }
//       //writeMarker_file(QString(p4DImage->getFileName()+QString::number(i)+"_max_connected_region.marker"),connected_region_marker);
//   }




























 /*
    vector<vector<vector<V3DLONG> > > coodinate3D,mark3D,B;
    vector<vector<V3DLONG> > coodinate2D,mark2D,B_t;
    vector<V3DLONG> coodinate1D,mark1D,b_t;
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < N; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < M; ix++)
            {
                V3DLONG tmp = im_cropped[offsetk + offsetj + ix];
                V3DLONG bri = int(data1d[offsetk + offsetj + ix]);
                ImageMarker coord_curr;
                if(tmp==255)
                {
                    int one=1;
                    mark1D.push_back(one);
>>>>>>> remove_glio version which is operational
                }
                else
                {
                    int zero=0;
                    mark1D.push_back(zero);
                }
                coodinate1D.push_back(tmp);
<<<<<<< HEAD
            }
            coodinate2D.push_back(coodinate1D);
            mark2D.push_back(mark1D);
            coodinate1D.clear();
            mark1D.clear();
        }
        coodinate3D.push_back(coodinate2D);
        mark3D.push_back(mark2D);
        coodinate2D.clear();
        mark2D.clear();
=======
                b_t.push_back(bri);
            }
            coodinate2D.push_back(coodinate1D);
            mark2D.push_back(mark1D);
            B_t.push_back(b_t);
            coodinate1D.clear();
            mark1D.clear();
            b_t.clear();
        }
        coodinate3D.push_back(coodinate2D);
        mark3D.push_back(mark2D);
        B.push_back(B_t);
        coodinate2D.clear();
        mark2D.clear();
        B_t.clear();
>>>>>>> remove_glio version which is operational
    }
    int j = 1;
    int u = 0;
    int g = 0;
    vector<vector<Coordinate> >connected_region;
    QHash<Coordinate,int> map_index;
    vector<Coordinate> each_region1;
    vector<Coordinate> each_region;
    QList<ImageMarker> marker_all3;
    int black = 0;
    int white = 0;
    int unknow = 0;
    int black2 = 0;
    int all = 0;
    for(int iz = 0; iz < P; iz++)
    {
        for(int iy = 0; iy < N; iy++)
        {
            for(int ix = 0; ix < M; ix++)
            {
                all++;
                Coordinate coord_curr;
                if(coodinate3D[iz][iy][ix]==255)
                {

<<<<<<< HEAD
                    if(!first_floor)
=======
                    if(ix==1)
>>>>>>> remove_glio version which is operational
                    {
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
<<<<<<< HEAD
=======
                        coord_curr.bri = B[iz][iy][ix];
>>>>>>> remove_glio version which is operational
                        each_region1.push_back(coord_curr);
                        connected_region.push_back(each_region1);
                        map_index.insert(coord_curr,j);
                        each_region1.clear();
                        j++;
                        g++;
                        first_floor = true;
                        unknow++;
                        continue;
                    }
                }
                else
                {
                    black++;
                    continue;
                }
<<<<<<< HEAD
                if(first_floor)
=======
                if(ix!=1) //first_floor
>>>>>>> remove_glio version which is operational
                {
                    if(coodinate3D[iz][iy][ix]==255)
                    {
                        white++;
                        int dex;

                        bool ind1 = false;
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
<<<<<<< HEAD
=======
                        coord_curr.bri = B[iz][iy][ix];
>>>>>>> remove_glio version which is operational
                        ImageMarker m;
                        m.x = coord_curr.x;
                        m.y = coord_curr.y;
                        m.z = coord_curr.z;
                        m.radius = 0.5;
                        m.type = 2;
                        marker_all3.push_back(m);
                        for(int l=0;l<connected_region.size();l++)
                        {
                            for(int k=0;k<connected_region[l].size();k++)
                            {
                                if(if_is_connect(coord_curr,connected_region[l][k],mark3D))
                                {
                                    dex = l;
                                    ind1 = true;
                                    break;
                                }
                            }
                            if(ind1)
                            {
                                break;
                            }

                        }
                        if(ind1)
                        {
                            connected_region[dex].push_back(coord_curr);
                            map_index.insert(coord_curr,map_index[connected_region[dex][0]]);
                            ind1 = false;
                            //break;
                        }
                        else
                        {
                            each_region.clear();
                            each_region.push_back(coord_curr);
                            u++;
                            connected_region.push_back(each_region);
                            map_index.insert(coord_curr,j);
                            j++;

                        }
                        each_region.clear();

                    }
                    else
                    {
                        black2++;
                    }

                }
            }
        }

    }
    QHash<vector<Coordinate>,vector<Coordinate> > relation;
<<<<<<< HEAD
=======
    QHash<int,vector<int> > rela2;
>>>>>>> remove_glio version which is operational
    int count_ind=1;
    //vector<int> count;
    vector<V3DLONG> index;
    vector<V3DLONG> reminder;
<<<<<<< HEAD
=======




    cout<<"connected_region.size() = "<<connected_region.size()<<endl;
    int c_sum=0;
>>>>>>> remove_glio version which is operational
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder.push_back(zero);
    }
    for(int l=0;l<connected_region.size();l++)
    {
<<<<<<< HEAD
        for(int j=l+1;j<connected_region.size();j++)
        {
            if(reminder[l+1]!=0)continue;
            double min_dis = 1000000000000;
            for(int k=0;k<connected_region[l].size();k++)
            {
=======
       // bool fag=0;
        if(reminder[l]!=0)continue;
        for(int j=l;j<connected_region.size();j++)//for(int j=l+1;j<connected_region.size();j++)
        {
            //if(reminder[l+1]!=0)continue;
            double min_dis = 1000000000000;
            for(int k=0;k<connected_region[l].size();k++)
            {

>>>>>>> remove_glio version which is operational
                for(int h=0;h<connected_region[j].size();h++)
                {

                    double dis = NTDIS(connected_region[l][k],connected_region[j][h]);
                    if(dis<min_dis)
                    {
                        min_dis = dis;
                    }

                }
            }
<<<<<<< HEAD
            if(min_dis<dis_thresh)
            {
                relation.insert(connected_region[l],connected_region[j]);
                reminder[l] = 1;
                reminder[j] = 1;
                index.push_back(l);
                count_ind++;
            }
        }
    }
    int n=2;
    vector<vector<Coordinate> >connected_region_final;
    vector<V3DLONG> reminder2;
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder2.push_back(zero);
    }
    for(int l=0;l<connected_region.size();l++)
    {
        if(reminder2[l]!=0)
        {
            continue;
        }
        if(relation[connected_region[l]].size()==0)
        {
            connected_region_final.push_back(connected_region[l]);
            reminder2[l]=1;
        }
        else
        {
            vector<Coordinate> tmp;
            for(int y=0;y<connected_region[l].size();y++)
            {
                tmp.push_back(connected_region[l][y]);
            }
            reminder2[l]=1;
            find_relation(relation,connected_region[l],tmp,reminder2,map_index);
            connected_region_final.push_back(tmp);
        }
    }

    cout<<"3.train_data"<<endl;
    Chart chart,chart2;
    Each_line E,E2;
    export_training_data(QString("signal.txt"),chart,E);
    export_training_data(QString("glio.txt"),chart2,E2);
=======
            cout<<"min_dis = "<<min_dis<<endl;
            if(min_dis<=dis_thresh)
            {
                cout<<"inside"<<endl;
                rela2[l].push_back(j);
                relation.insert(connected_region[l],connected_region[j]);
                //reminder[l] = 1;
                reminder[j] = 1;
                index.push_back(l);
                count_ind++;
               // fag=1;
                 cout<<"l j = "<<l<<"  "<<j<<endl;

            }
//            if(!fag)
//            {
//                rela2[l].push_back(l);
//            }
        }

    }

 //   cout<<"relation = "<<relation.size()<<endl;
 //  cout<<"count_ind = "<<count_ind<<endl;
    v3d_msg("count_ind");

    vector<vector<Coordinate> >connected_region_final,connected_region_final_final;


    connected_region_final_final = build_rela2(connected_region,rela2);

//    vector<V3DLONG> reminder2;
//    for(int l=0;l<connected_region.size();l++)
//    {
//        int zero = 0;
//        reminder2.push_back(zero);
//    }
//    for(int l=0;l<connected_region.size();l++)
//    {
//        if(reminder2[l]!=0)
//        {
//            continue;
//        }
//        if(relation[connected_region[l]].size()==0)
//        {
//            connected_region_final.push_back(connected_region[l]);
//            reminder2[l]=1;
//        }
//        else
//        {
//            vector<Coordinate> tmp;
//            for(int y=0;y<connected_region[l].size();y++)
//            {
//                tmp.push_back(connected_region[l][y]);
//            }
//            reminder2[l]=1;
//            find_relation(relation,connected_region[l],tmp,reminder2,map_index);
//            connected_region_final.push_back(tmp);
//        }
//    }

//    for(int i=0;i<connected_region_final.size();i++)
//    {
//        connected_region_final_final.push_back(connected_region_final[i]);
//    }
    cout<<"connected_region_final_final = "<<connected_region_final_final.size()<<endl;
    */
  //  v3d_msg("kkk");
    cout<<"3.train_data"<<endl;
    Each_line E1,E2;
    double logsigma;
    vector<vector<double> > R;


    if(!bo)
    {
        OpenfileDialog * openDlg = new OpenfileDialog(0, &callback);
        if (!openDlg->exec())
            return false;

        model = openDlg->file_name;
      //  NeuronTree nt = openDlg->nt;
        //QList<NeuronSWC> neuron = nt.listNeuron;
        bo=true;
    }

    export_training_data(model,E1,E2,logsigma,R);

>>>>>>> remove_glio version which is operational




    cout<<"4.calculate feature."<<endl;


    vector<double> y_n;
    vector<double> overlap_level;
    vector<double> ratio_v;
    vector<double> count_v;
    vector<double> D;
    vector<double> grey;
    vector<double> grey_std;
<<<<<<< HEAD
    vector<inf> inf_v;
    feature_calculate(inf_v,y_n,overlap_level,ratio_v,count_v,D,grey,grey_std,connected_region_final);





    Feature feature_curr;
    Chart chart_curr;
    Each_line E_curr;
=======
    vector<double> overlap_all;
    vector<inf> inf_v;
    QList<ImageMarker> center;
    cout<<connected_region_final_final.size()<<endl;
    feature_calculate(inf_v,y_n,overlap_level,ratio_v,count_v,D,grey,grey_std,center,connected_region_final_final,overlap_all);


    Feature feature_curr;
>>>>>>> remove_glio version which is operational
    feature_curr.y_n = y_n;
    feature_curr.overlap_level = overlap_level;
    feature_curr.ratio_v = ratio_v;
    feature_curr.count_v = count_v;
    feature_curr.D = D;
    feature_curr.grey_mean = grey;
    feature_curr.grey_std = grey_std;
<<<<<<< HEAD
   // Cov_calculate(chart_curr,feature_curr);
    //E_curr = E_calculate(feature_curr);
    bool method=false;
    vector<bool> classify;

    classify = classify_glio(chart,E,chart2,E2,chart_curr,E_curr,feature_curr,method);
    for(int i=0;i<classify.size();i++)
    {
        cout<<"classify = "<<classify[i]<<endl;
    }

    if(im_cropped){delete []im_cropped;im_cropped=0;}
=======
    feature_curr.overlap_all = overlap_all;
// int t = y_n.size();
// cout<<"t = "<<t<<endl;
    vector<double> cl = classify_glio_Y(feature_curr,E1,R,logsigma);//glio
    vector<double> cl2 = classify_glio_Y(feature_curr,E2,R,logsigma);//signal
    LandmarkList glio_list;
    LandmarkList big_area;
    QList<ImageMarker> show_m_txt;
    cout<<"cl.size() = "<<cl.size()<<endl;
        int n=2;
    for(int i=0;i<cl.size();i++)
    {

        if(feature_curr.count_v[i]>1000)
        {
            LocationSimple glio;
            glio.x = center[i].x;
            glio.y = center[i].y;
            glio.z = center[i].z;
            glio.color.a = 0;
            glio.color.b = 0;
            glio.color.g = 0;
            glio.color.r = 0;
            big_area.push_back(glio);
            QList<ImageMarker> big_marker;
            for(int f=0;f<connected_region_final_final[i].size();f++)
            {
                ImageMarker m;
                m.x = connected_region_final_final[i][f].x;
                m.y = connected_region_final_final[i][f].y;
                m.z = connected_region_final_final[i][f].z;
                m.radius = 0.5;
                m.type = n;
                big_marker.push_back(m);
               // bri3D[m.z][m.y][m.x]=0;
            }
           // writeMarker_file(QString("big_area"+QString::number(i)+".marker"),big_marker);
            //cout<<feature_curr.y_n[i]<<"  "<<feature_curr.ratio_v[i]<<"     "<<feature_curr.overlap_level[i]<<"   "<<"    "<<feature_curr.grey_std[i]<<"    "<<feature_curr.count_v[i]<<endl;
        }
        cout<<"cl = "<<cl[i]<<"     "<<cl2[i]<<endl;
        if(feature_curr.y_n[i]==0||feature_curr.count_v[i]<70||feature_curr.ratio_v[i]<0.5||feature_curr.overlap_level[i]<0.2||feature_curr.overlap_all[i]<0.1)continue;
        QList<ImageMarker> marker;
        if(cl[i]>cl2[i])
        {
            LocationSimple glio;
            glio.x = center[i].x;
            glio.y = center[i].y;
            glio.z = center[i].z;
            glio.color.a = 0;
            glio.color.b = 0;
            //m.color.c = 0;
            glio.color.g = 0;
            glio.color.r = 0;
            glio_list.push_back(glio);
            show_m_txt.push_back(center[i]);
            for(int f=0;f<connected_region_final_final[i].size();f++)
            {
                ImageMarker m;
                m.x = connected_region_final_final[i][f].x;
                m.y = connected_region_final_final[i][f].y;
                m.z = connected_region_final_final[i][f].z;
                m.radius = 0.5;
                m.type = n;
                marker.push_back(m);
                bri3D[m.z][m.y][m.x]=0;
                cout<<" MNP = "<<M<<"   "<<N<<"    "<<P<<endl;
                cout<<m.x<<"    "<<m.y<<"   "<<m.z<<endl;
                if(m.x+1>=M)m.x=M-2;
                if(m.y+1>=N)m.y=N-2;
                if(m.z+1>=P)m.z=P-2;
                 cout<<m.x<<"    "<<m.y<<"   "<<m.z<<endl;
                bri3D[m.z][m.y][m.x+1]=0;
                bri3D[m.z][m.y+1][m.x]=0;
                bri3D[m.z+1][m.y][m.x]=0;
                bri3D[m.z][m.y+1][m.x+1]=0;
                bri3D[m.z+1][m.y+1][m.x]=0;
                bri3D[m.z+1][m.y][m.x+1]=0;
                bri3D[m.z+1][m.y+1][m.x+1]=0;
                if(m.x+2>=M)m.x=M-3;
                if(m.y+2>=N)m.y=N-3;
                if(m.z+2>=P)m.z=P-3;
 cout<<m.x<<"    "<<m.y<<"   "<<m.z<<endl;
                bri3D[m.z][m.y][m.x+2]=0;
                bri3D[m.z][m.y+2][m.x]=0;
                bri3D[m.z+2][m.y][m.x]=0;
                bri3D[m.z][m.y+2][m.x+2]=0;
                bri3D[m.z+2][m.y+2][m.x]=0;
                bri3D[m.z+2][m.y][m.x+2]=0;
                bri3D[m.z+2][m.y+2][m.x+2]=0;
                if(m.x+3>=M)m.x=M-4;
                if(m.y+3>=N)m.y=N-4;
                if(m.z+3>=P)m.z=P-4;
 cout<<m.x<<"    "<<m.y<<"   "<<m.z<<endl;

                bri3D[m.z][m.y][m.x+3]=0;
                bri3D[m.z][m.y+3][m.x]=0;
                 cout<<"oooooo"<<endl;
                bri3D[m.z+3][m.y][m.x]=0;
                bri3D[m.z][m.y+3][m.x+3]=0;
                bri3D[m.z+3][m.y+3][m.x]=0;
                bri3D[m.z+3][m.y][m.x+3]=0;
                bri3D[m.z+3][m.y+3][m.x+3]=0;
                cout<<"oooooo"<<endl;
                if(m.x-1<0)m.x=1;
                if(m.y-1<0)m.y=1;
                if(m.z-1<0)m.z=1;
                bri3D[m.z][m.y][m.x-1]=0;
                bri3D[m.z][m.y-1][m.x]=0;
                bri3D[m.z-1][m.y][m.x]=0;
                bri3D[m.z][m.y-1][m.x-1]=0;
                bri3D[m.z-1][m.y-1][m.x]=0;
                bri3D[m.z-1][m.y][m.x-1]=0;
                bri3D[m.z-1][m.y-1][m.x-1]=0;
                if(m.x-2<0)m.x=2;
                if(m.y-2<0)m.y=2;
                if(m.z-2<0)m.z=2;

                bri3D[m.z][m.y][m.x-2]=0;
                bri3D[m.z][m.y-2][m.x]=0;
                bri3D[m.z-2][m.y][m.x]=0;
                bri3D[m.z][m.y-2][m.x-2]=0;
                bri3D[m.z-2][m.y-2][m.x]=0;
                bri3D[m.z-2][m.y][m.x-2]=0;
                bri3D[m.z-2][m.y-2][m.x-2]=0;
                if(m.x-3<0)m.x=3;
                if(m.y-3<0)m.y=3;
                if(m.z-3<0)m.z=3;

                bri3D[m.z][m.y][m.x-3]=0;
                bri3D[m.z][m.y-3][m.x]=0;
                bri3D[m.z-3][m.y][m.x]=0;
                bri3D[m.z][m.y-3][m.x-3]=0;
                bri3D[m.z-3][m.y-3][m.x]=0;
                bri3D[m.z-3][m.y][m.x-3]=0;
                bri3D[m.z-3][m.y-3][m.x-3]=0;
                cout<<"out"<<endl;
                //marker_all.push_back(m);

            }
            writeMarker_file(QString("marker"+QString::number(i)+".marker"),marker);
            cout<<feature_curr.y_n[i]<<"  "<<feature_curr.ratio_v[i]<<"     "<<feature_curr.overlap_level[i]<<"   "<<"    "<<feature_curr.grey_std[i]<<"    "<<feature_curr.count_v[i]<<endl;
        }
       // marker_all_each.push_back(marker);
    }
    writeMarker_file(QString("show_m_txt.marker"),show_m_txt);
    cout<<"size_all = "<<cl.size()<<endl;
    cout<<"sizeeeeeeeeeeee = "<<glio_list.size()<<endl;
     //callback.setLandmarkTeraFly(glio_list);
     simple_saveimage_wrapper(callback,QString("ori_img.v3draw").toStdString().c_str(),data1d,in_sz,1);
//     const Image4DSimple *curr_block = callback.getImageTeraFly();

//     cout<<"check!"<<endl;

//     double ox = curr_block->getOriginX();
//     double oy = curr_block->getOriginY();
//     double oz = curr_block->getOriginZ();
//     double lx = curr_block->getRezX();
//     double ly = curr_block->getRezY();
//     double lz = curr_block->getRezZ();
//     double ratio_x = curr_block->getRezX()/curr_block->getXDim();
//     double ratio_y = curr_block->getRezY()/curr_block->getYDim();
//     double ratio_z = curr_block->getRezZ()/curr_block->getZDim();
//     LandmarkList glia_list_show;
//     for(int i=0;i<show_m_txt.size();i++)
//     {
//         LocationSimple l;
//         l.x = show_m_txt[i].x*ratio_x+ox;
//         l.y = show_m_txt[i].y*ratio_x+oy;
//         l.z = show_m_txt[i].z*ratio_x+oz;
//         l.color.a = 0;
//         l.color.b = 0;
//         //m.color.c = 0;
//         l.color.g = 0;
//         l.color.r = 0;
//         glia_list_show.push_back(l);
//       //  B[l.z][l.y][l.x]=0;
//     }
   //  callback.setLandmarkTeraFly(glia_list_show);

     unsigned char *im_cropped2 = 0;
     try {im_cropped2 = new unsigned char [pagesz];}
     catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
     int jj=0;
     for(int iz = 0; iz < P; iz++)
     {
         for(int iy = 0; iy < N; iy++)
         {
             for(int ix = 0; ix < M; ix++)
             {
                 im_cropped2[jj] = bri3D[iz][iy][ix];
                 jj++;
             }
         }
     }
//     for(int i=0;i<pagesz;i++)
//     {
//         if(im_cropped2[i]!=0)
//             cout<<int(im_cropped2[i])<<endl;
//     }
  //   cout<<"check2!"<<endl;
 //  simple_saveimage_wrapper(callback,QString("binary.tif").toStdString().c_str(),(unsigned char *)im_cropped,in_sz,1);

 // cout<<"iiiiii"<<endl;
     simple_saveimage_wrapper(callback,QString("final_img.v3draw").toStdString().c_str(),(unsigned char *)im_cropped2,in_sz,1);
  //  if(im_cropped){delete []im_cropped;im_cropped=0;}
   //  if(im_cropped2){delete []im_cropped2;im_cropped2=0;}
>>>>>>> remove_glio version which is operational

}
void find_relation(QHash<vector<Coordinate>,vector<Coordinate> > &relation,vector<Coordinate> &curr,vector<Coordinate> &tmp,vector<V3DLONG> &reminder2,QHash<Coordinate,int> &map_index)
{
    if(relation[curr].size()==0)return;
    for(int y = 0;y<relation[curr].size();y++)
    {
        tmp.push_back(relation[curr][y]);
    }
    int g = map_index[relation[curr][0]];
    reminder2[g-1] = 1;
    find_relation(relation,relation[curr],tmp,reminder2,map_index);
    return;

}
bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D)
{

    double dist = NTDIS(curr,b);
<<<<<<< HEAD
    if(dist<3)
=======
    if(dist<2)
>>>>>>> remove_glio version which is operational
    {
        return true;//this
        if(mark3D[curr.z][curr.y][curr.x]==1)
        {
            mark3D[curr.z][curr.y][curr.x]==0;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
<<<<<<< HEAD
bool feature_calculate(vector<inf> &inf_v,vector<double> &y_n,vector<double> &overlap_level,vector<double> &ratio_v,vector<double> &count_v,vector<double> &D,vector<double> &grey,vector<double> &grey_std,vector<vector<Coordinate> >&connected_region_final)
{
    int n=2;
   // cout<<"  <1>.the most big two floor is next to each other or not "<<endl;
    int ind1;
    int ind2;
    vector<Max_level> two_level;
    for(int i=0;i<connected_region_final.size();i++)
    {
        QHash<int,int> hashlinker;
=======
bool feature_calculate(vector<inf> &inf_v,vector<double> &y_n,vector<double> &overlap_level,vector<double> &ratio_v,vector<double> &count_v,vector<double> &D,vector<double> &grey,vector<double> &grey_std,QList<ImageMarker> &center,vector<vector<Coordinate> >&connected_region_final,vector<double> &overlap_all)
{
    int n=2;
    cout<<"  <1>.the most big two floor is next to each other or not "<<endl;
    int ind1;
    int ind2;
    int ind2_rebase;
    int ind2_rebase2;
    vector<Max_level> two_level;
     Max_level max_l;
    set<int> z_s;
    vector<int> z_v;
    for(int i=0;i<connected_region_final.size();i++)
    {
        QHash<int,int> hashlinker;
        z_s.clear();
        z_v.clear();
>>>>>>> remove_glio version which is operational
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            hashlinker[curr.z]++;
<<<<<<< HEAD
        }
=======
            z_s.insert(curr.z);
        }
        for(set<int>::iterator it=z_s.begin();it!=z_s.end();++it)
        {
            z_v.push_back(*it);
        }
        bubblesort(z_v);
>>>>>>> remove_glio version which is operational
        int max1=0;
        int max2=0;
        ind1=0;
        ind2=0;
<<<<<<< HEAD

        for(QHash<int,int>::iterator it=hashlinker.begin();it!=hashlinker.end();it++)
        {
            if(it.value()>max1)
            {
                max1 = it.value();
                ind1 = it.key();
            }
        }
        for(QHash<int,int>::iterator it=hashlinker.begin();it!=hashlinker.end();it++)
        {
            if(it.value()==max1)continue;
            if(it.value()>max2)
            {
                max2 = it.value();
                ind2 = it.key();
            }
        }
        Max_level max_l;
        //cout<<"max = "<<max1<<"  "<<max2<<endl;
        //cout<<"ind = "<<ind1<<"  "<<ind2<<endl;
        max_l.level1=ind1;
        max_l.level2=ind2;
        two_level.push_back(max_l);
        if((ind1-ind2==1)||(ind2-ind1==1))
=======
        ind2_rebase=0;
        ind2_rebase2=0;
        vector<double> S,T;
        for(int j=0;j<z_v.size();j++)//QHash<int,int>::iterator it=hashlinker.begin();it!=hashlinker.end();++it
        {
            if(hashlinker[z_v[j]]>max1)//it.value()>max1
            {
                S.clear();
               // cout<<"coor = "<<z_v[j]<<"  "<<hashlinker[z_v[j]]<<"    "<<max1<<endl;
                max1 = hashlinker[z_v[j]];
                S.push_back(z_v[j]);



            }
            else if(hashlinker[z_v[j]]==max1) //it.value()==max1
            {
                S.push_back(z_v[j]);
            }
            else
            {;}
        }


        if(S.size()>=2)
        {
            max_l.level1=S[0];
            max_l.level2=S[1];
            //ind1 = z_v[j];
        }
        else if(S.size()==1)
        {
            for(int j=0;j<z_v.size();j++)
            {

            //    cout<<"coor = "<<i<<"   "<<z_v[j]<<"  "<<hashlinker[z_v[j]]<<"    "<<max1<<"   "<<max2<<"   "<<connected_region_final[i].size()<<endl;
                if(hashlinker[z_v[j]]==max1)
                {
                    continue;
                }
                if(hashlinker[z_v[j]]>max2)
                {
                    T.clear();
                    max2 = hashlinker[z_v[j]];
                    T.push_back(z_v[j]);
                }

            }
            if(T.size()==0)T.push_back(S[0]);
            max_l.level1 = S[0];
            max_l.level2 = T[0];
        }
        else
        {;}


     //  cout<<max_l.level1<<"   "<<max_l.level2<<endl;



            two_level.push_back(max_l);




        if((max_l.level1 - max_l.level2==1)||(max_l.level2 - max_l.level1==1)||(max_l.level2 == max_l.level1))
>>>>>>> remove_glio version which is operational
        {
            y_n.push_back(true);
        }
        else
        {
<<<<<<< HEAD
            //v3d_msg("check");
            y_n.push_back(false);
        }
    }


  //  cout<<"  <2><3><4>.calculate overlap,ratio and volume of this two level "<<endl;
=======
            y_n.push_back(false);
        }

    }


    cout<<"  <2><3><4>.calculate overlap,ratio and volume of this two level "<<endl;
>>>>>>> remove_glio version which is operational
    double maxx;
    double maxy;
    double minx;
    double miny;
<<<<<<< HEAD
=======
    double maxz;
    double minz;
    double maxx_all;
    double minx_all;
    double maxy_all;
    double miny_all;
    double maxz_all;
    double minz_all;
>>>>>>> remove_glio version which is operational
    double count_level,count_tmp;
    for(int i=0;i<connected_region_final.size();i++)
    {
        count_level=0;
        count_tmp=0;
        maxx=0;
        maxy=0;
<<<<<<< HEAD
        minx=100000000;
        miny=100000000;
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            //cout<<"curr.z = "<<curr.z<<endl;
            //cout<<"two_level[i] = "<<two_level[i].level1<<"  "<<two_level[i].level2<<endl;
            count_level++;
            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2))||two_level[i].level2==0||two_level[i].level1==0)
=======
        maxz=0;

        maxx_all=0;
        maxy_all=0;
        maxz_all=0;

        minx=100000000;
        miny=100000000;
        minz=100000000;

        minx_all=100000000;
        miny_all=100000000;
        minz_all=100000000;
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
            count_level++;
            if(curr.x>maxx_all)
            {
                maxx_all=curr.x;
            }
            if(curr.y>maxy_all)
            {
                maxy_all=curr.y;
            }
            if(curr.z>maxz_all)
            {
                maxz_all=curr.z;
            }

            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2)))
>>>>>>> remove_glio version which is operational
            {
                continue;
            }
            count_tmp++;
            if(curr.x>maxx)
            {
                maxx=curr.x;
            }
            if(curr.y>maxy)
            {
                maxy=curr.y;
            }
<<<<<<< HEAD
=======
            if(curr.z>maxz)
            {
                maxz=curr.z;
            }
>>>>>>> remove_glio version which is operational
        }
        count_v.push_back(count_level);
        for(int j=0;j<connected_region_final[i].size();j++)
        {
            Coordinate curr = connected_region_final[i][j];
<<<<<<< HEAD
            //cout<<"curr.z = "<<curr.z<<endl;
            //cout<<"two_level[i] = "<<two_level[i].level1<<"  "<<two_level[i].level2<<endl;
            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2))||two_level[i].level2==0||two_level[i].level1==0)
            {
                //ratio_v.push_back(-1);
                //overlap_level.push_back(-1);
=======

            if(curr.x<minx_all)
            {
                minx_all=curr.x;
            }
            if(curr.y<miny_all)
            {
                miny_all=curr.y;
            }
            if(curr.z<minz_all)
            {
                minz_all=curr.z;
            }

            if(((curr.z!=two_level[i].level1)&&(curr.z!=two_level[i].level2)))
            {
>>>>>>> remove_glio version which is operational
                continue;
            }
            if(curr.x<minx)
            {
                minx=curr.x;
            }
            if(curr.y<miny)
            {
                miny=curr.y;
            }
<<<<<<< HEAD
        }
        double minus_x=maxx-minx+1;
        double minus_y=maxy-miny+1;
        double overl=count_tmp/(minus_x*minus_y*2);
        cout<<"x/y = "<<minus_x<<"  "<<minus_y<<endl;
=======
            if(curr.z<minz)
            {
                minz=curr.z;
            }
        }
        double minus_x=maxx-minx+1;
        double minus_y=maxy-miny+1;
        double minus_z=maxz-minz+1;
        double overl=count_tmp/(minus_x*minus_y*2);
>>>>>>> remove_glio version which is operational

        double ratio1=minus_x/minus_y;
        double ratio2=minus_y/minus_x;
        double ratio;
<<<<<<< HEAD
//        if(ratio1>ratio2)
//            ratio=ratio1;
//        else
//            ratio=ratio2;
        ratio = ratio1;
        ratio_v.push_back(ratio);
        overlap_level.push_back(overl);
=======
        if(ratio1>ratio2)
            ratio=ratio2;
        else
            ratio=ratio1;
      //  if(z_v.size()==1)//two_level[i].level2==0&&two_level[i].level1==0
   //     {
           // cout<<i<<endl;
         //   overl = overl*2;
          //  v3d_msg("check");
       //     cout<<"z_v.size()==1"<<endl;
       // }
      //  cout<<"count_tmp/(minus_x*minus_y*2) = "<<count_tmp<<"      "<<(minus_x*minus_y*2)<<endl;
            ratio_v.push_back(ratio);
            overlap_level.push_back(overl);


            double minus_x_all=maxx_all-minx_all+1;
            double minus_y_all=maxy_all-miny_all+1;
            double minus_z_all=maxz_all-minz_all+1;

            double each_overlap_all = count_level/(minus_x_all*minus_y_all*minus_z_all);
            overlap_all.push_back(each_overlap_all);

>>>>>>> remove_glio version which is operational

    }


<<<<<<< HEAD
    cout<<"size = "<<overlap_level.size()<<endl;
    cout<<"connected_region_final.size() = "<<connected_region_final.size()<<endl;
=======


   // cout<<"size = "<<overlap_level.size()<<endl;
   // cout<<"connected_region_final.size() = "<<connected_region_final.size()<<endl;
>>>>>>> remove_glio version which is operational









    QList<ImageMarker> marker_all;
    QList<QList<ImageMarker> > marker_all_each;
<<<<<<< HEAD
    QList<ImageMarker> center;
=======
    //QList<ImageMarker> center;
>>>>>>> remove_glio version which is operational
    int size_thresh = 80;

    for(V3DLONG l=0;l<connected_region_final.size();l++)
    {
        QList<ImageMarker> marker;
        for(int f=0;f<connected_region_final[l].size();f++)
        {

            ImageMarker m;
            m.x = connected_region_final[l][f].x;
            m.y = connected_region_final[l][f].y;
            m.z = connected_region_final[l][f].z;
            m.radius = 0.5;
            m.type = n;
            marker.push_back(m);
            marker_all.push_back(m);
        }
        marker_all_each.push_back(marker);
<<<<<<< HEAD
        //writeMarker_file(QString("marker"+QString::number(l)+".marker"),marker);
=======
       // writeMarker_file(QString("marker"+QString::number(l)+".marker"),marker);
>>>>>>> remove_glio version which is operational
    }
    //writeMarker_file(QString("marker_all.marker"),marker_all);
    vector<MyMarker*> inswc;
    vector<Soma> soma_v;


<<<<<<< HEAD
   // cout<<"  <5>.calculate D "<<endl;


  //  cout<<"find_center"<<endl;
=======
    cout<<"  <5>.calculate D "<<endl;


    cout<<"find_center"<<endl;
>>>>>>> remove_glio version which is operational
    for(V3DLONG l=0;l<connected_region_final.size();l++)
    {
        double sumx=0;
        double sumy=0;
        double sumz=0;
        Soma soma;
        soma.x_b = 10000000;
        soma.y_b = 10000000;
        soma.z_b = 10000000;
        soma.x_e = 0;
        soma.y_e = 0;
        soma.z_e = 0;
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            sumx = sumx + connected_region_final[l][f].x;
            sumy = sumy + connected_region_final[l][f].y;
            sumz = sumz + connected_region_final[l][f].z;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].x<soma.x_b)soma.x_b = connected_region_final[l][f].x;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].y<soma.y_b)soma.y_b = connected_region_final[l][f].y;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].z<soma.z_b)soma.z_b = connected_region_final[l][f].z;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].x>soma.x_e)soma.x_e = connected_region_final[l][f].x;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].y>soma.y_e)soma.y_e = connected_region_final[l][f].y;
        }
        for(int f=0;f<connected_region_final[l].size();f++)
        {
            if(connected_region_final[l][f].z>soma.z_e)soma.z_e = connected_region_final[l][f].z;
        }
        ImageMarker m;
        MyMarker* m2 = new MyMarker;
        m.x = sumx/connected_region_final[l].size();
        m.y = sumy/connected_region_final[l].size();
        m.z = sumz/connected_region_final[l].size();
        m.radius = 0.5;
        m.type = n;
        m2->x = m.x;
        m2->y = m.y;
        m2->z = m.z;
        m2->type = n;
        inswc.push_back(m2);
        center.push_back(m);
        soma_v.push_back(soma);
    }
    vector<double> max_dis_v;
    for(int d=0;d<connected_region_final.size();d++)
    {
        double sum_dis=0;
        double sum_dis2=0;
        double mean_dis=0;
        double max_dis = 0;
        for(int f=0;f<connected_region_final[d].size();f++)
        {
            double dis = NTDIS(center[d],connected_region_final[d][f]);
            if(dis>max_dis)
            {
                max_dis = dis;
            }
            sum_dis=sum_dis+dis;
        }
        mean_dis = sum_dis/connected_region_final[d].size();

        for(int f=0;f<connected_region_final[d].size();f++)
        {

            double dis = NTDIS(center[d],connected_region_final[d][f]);

            sum_dis2 = sum_dis2+(dis-mean_dis)*(dis-mean_dis);
        }
        max_dis_v.push_back(max_dis);
<<<<<<< HEAD
        double Dd = sum_dis2/(connected_region_final[d].size()-1);
=======
        double Dd = sum_dis2/(connected_region_final[d].size());
>>>>>>> remove_glio version which is operational
        D.push_back(Dd);

    }


    QList<ImageMarker> center_choose;
    for(V3DLONG d=0;d<connected_region_final.size();d++)
    {
        double x_dis = soma_v[d].x_e - soma_v[d].x_b;
        double y_dis = soma_v[d].y_e - soma_v[d].y_b;
<<<<<<< HEAD
     //   cout<<"d = "<<d<<endl;
 //       cout<<"connected_region_final[d].size() = "<<connected_region_final[d].size()<<endl;
 //       cout<<"max_dis = "<<max_dis_v[d]<<endl;
   //     cout<<"D = "<<D[d]<<endl;
=======
>>>>>>> remove_glio version which is operational
        if(connected_region_final[d].size()<size_thresh)
            continue;
        //if(marker_all_each[d].size()/(max_dis_v[d]*max_dis_v[d]*max_dis_v[d]*8)<0.7)
          //  continue;
        if(y_dis-x_dis>7||x_dis-y_dis>7)
            continue;
        if(max_dis_v[d]>20)
            continue;
        if(D[d]>10)
            continue;

        center_choose.push_back(center[d]);


    }

<<<<<<< HEAD
 //   cout<<"  <6>.calculate mean grey "<<endl;
=======
    cout<<"  <6>.calculate mean grey "<<endl;
>>>>>>> remove_glio version which is operational

    double mean_grey;
    for(int i=0;i<connected_region_final.size();i++)
    {
        double sum_grey=0;
        for(int d=0;d<connected_region_final[i].size();d++)
        {
            sum_grey = sum_grey + connected_region_final[i][d].bri;
        }
        mean_grey = sum_grey/connected_region_final[i].size();
        grey.push_back(mean_grey);
    }
    cout<<"  <7>.calculate std grey "<<endl;
    double std_grey;
    for(int i=0;i<connected_region_final.size();i++)
    {
        double sum_grey=0;
        for(int d=0;d<connected_region_final[i].size();d++)
        {
            sum_grey = (connected_region_final[i][d].bri-grey[i])*(connected_region_final[i][d].bri-grey[i]) + sum_grey;
        }
<<<<<<< HEAD
        std_grey = sum_grey/(connected_region_final[i].size()-1);
      //  cout<<"sum_grey = "<<sum_grey<<"      "<<(connected_region_final[i].size()-1)<<"    "<<i<<endl;

        grey_std.push_back(sqrt(std_grey));
    }
       // v3d_msg("gggggggggggg");
//    for(int i=0;i<D.size();i++)
//    {
//        cout<<"y_n = "<<y_n[i]<<endl;
//        cout<<"D = "<<D[i]<<endl;
//        cout<<"overlap_level = "<<overlap_level[i]<<endl;
//        cout<<"ratio_v = "<<ratio_v[i]<<endl;
//        cout<<"count_v = "<<count_v[i]<<endl;
//        cout<<"grey_mean = "<<grey[i]<<endl;
//        cout<<"grey_std = "<<grey_std[i]<<endl;
//        cout<<"*************************************************"<<endl;
//    }
 //   cout<<"  <7>.get information from folder "<<endl;
=======
        if(connected_region_final[i].size()-1==0)
        {
            std_grey=0;
        }
        else

            std_grey = sum_grey/(connected_region_final[i].size()-1);
//        cout<<"sum_grey = "<<sum_grey<<"      "<<(connected_region_final[i].size())<<"    "<<i<<endl;

        grey_std.push_back(sqrt(std_grey));
    }

    cout<<"  <8>.get information from folder "<<endl;
>>>>>>> remove_glio version which is operational
    for(int i=0;i<connected_region_final.size();i++)
    {
        //vector<inf> inf_tmp_v;
       // for(int d=0;d<connected_region_final[i].size();d++)
        //{
            inf inf_tmp;
            inf_tmp.inf1 =  connected_region_final[i][0].inf1;
            inf_tmp.name = connected_region_final[i][0].name;
<<<<<<< HEAD
=======
            //cout<<"inf_tmp.inf1 = "<<inf_tmp.inf1<<endl;
           // cout<<"inf_tmp.name = "<<inf_tmp.name<<endl;
>>>>>>> remove_glio version which is operational
            inf_v.push_back(inf_tmp);
      //  }
        //inf_v.push_back(inf_tmp_v);
    }



}
<<<<<<< HEAD
bool export_training_data(const QString &fileOpenName,Chart &chart,Each_line &E)
=======
void bubblesort(vector<int> &A)
{
    int n = A.size();
    for(int i=0;i<n-1;i++)
    {
        for(int j=0;j<n-i-1;j++)
        {
            if(A[j]>A[j+1])
            {
                int temp = A[j];
                A[j] = A[j+1];
                A[j+1] = temp;
            }
        }
    }
}
bool export_training_data(const QString &fileOpenName,Each_line &E1,Each_line &E2,double &logsigma,vector<vector<double> >&R)
>>>>>>> remove_glio version which is operational
{


    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }

    V3DLONG k=0,j=1;
    while (! qf.atEnd())
    {
        char curline[2000];

        qf.readLine(curline, sizeof(curline));
        k++;
        {

            QStringList qsl = QString(curline).trimmed().split("   ");
            int qsl_count=qsl.size();
            cout<<"qls.size = "<<qsl_count<<endl;
<<<<<<< HEAD
            if (qsl_count<6)   continue;

            switch(j)
            {
                case 1: E.x1 = qsl[0].toFloat();
                        chart.first_line.x1 = qsl[1].toFloat();
                        chart.first_line.x2 = qsl[2].toFloat();
                        chart.first_line.x3 = qsl[3].toFloat();
                        chart.first_line.x4 = qsl[4].toFloat();
                        chart.first_line.x5 = qsl[5].toFloat();break;
                case 2: E.x2 = qsl[0].toFloat();
                        chart.second_line.x1 = qsl[1].toFloat();
                        chart.second_line.x2 = qsl[2].toFloat();
                        chart.second_line.x3 = qsl[3].toFloat();
                        chart.second_line.x4 = qsl[4].toFloat();
                        chart.second_line.x5 = qsl[5].toFloat();break;
                case 3: E.x3 = qsl[0].toFloat();
                        chart.third_line.x1 = qsl[1].toFloat();
                        chart.third_line.x2 = qsl[2].toFloat();
                        chart.third_line.x3 = qsl[3].toFloat();
                        chart.third_line.x4 = qsl[4].toFloat();
                        chart.third_line.x5 = qsl[5].toFloat();break;
                case 4: E.x4 = qsl[0].toFloat();
                        chart.forth_line.x1 = qsl[1].toFloat();
                        chart.forth_line.x2 = qsl[2].toFloat();
                        chart.forth_line.x3 = qsl[3].toFloat();
                        chart.forth_line.x4 = qsl[4].toFloat();
                        chart.forth_line.x5 = qsl[5].toFloat();break;
                case 5: E.x5 = qsl[0].toFloat();
                        chart.fifth_line.x1 = qsl[1].toFloat();
                        chart.fifth_line.x2 = qsl[2].toFloat();
                        chart.fifth_line.x3 = qsl[3].toFloat();
                        chart.fifth_line.x4 = qsl[4].toFloat();
                        chart.fifth_line.x5 = qsl[5].toFloat();break;
            default:break;
=======
            //if (qsl_count<5)   continue;
            vector<double> R_each;
            switch(j)
            {
                case 1: logsigma = qsl[0].toDouble();break;
                case 2: E1.x1 = qsl[0].toDouble();
                        E1.x2 = qsl[1].toDouble();
                        E1.x3 = qsl[2].toDouble();
                        E1.x4 = qsl[3].toDouble();
                        E1.x5 = qsl[4].toDouble();
                        E1.x6 = qsl[5].toDouble();break;
                case 3: E2.x1 = qsl[0].toDouble();
                        E2.x2 = qsl[1].toDouble();
                        E2.x3 = qsl[2].toDouble();
                        E2.x4 = qsl[3].toDouble();
                        E2.x5 = qsl[4].toDouble();
                        E2.x6 = qsl[5].toDouble();break;
                case 4: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                case 5: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                case 6: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                case 7: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                case 8: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                case 9: R_each.clear();
                        R_each.push_back(qsl[0].toDouble());
                        R_each.push_back(qsl[1].toDouble());
                        R_each.push_back(qsl[2].toDouble());
                        R_each.push_back(qsl[3].toDouble());
                        R_each.push_back(qsl[4].toDouble());
                        R_each.push_back(qsl[5].toDouble());
                        R.push_back(R_each);
                        break;
                default:break;
>>>>>>> remove_glio version which is operational

            }
            j++;

        }
    }
<<<<<<< HEAD
    cout << E.x1 <<"    "<<chart.first_line.x1<<"    "<<chart.first_line.x2<<"    "<<chart.first_line.x3<<"    "<<chart.first_line.x4<<"    "<<chart.first_line.x5<<endl;
    cout << E.x2 <<"    "<<chart.second_line.x1<<"    "<<chart.second_line.x2<<"    "<<chart.second_line.x3<<"    "<<chart.second_line.x4<<"    "<<chart.second_line.x5<<endl;
    cout << E.x3 <<"    "<<chart.third_line.x1<<"    "<<chart.third_line.x2<<"    "<<chart.third_line.x3<<"    "<<chart.third_line.x4<<"    "<<chart.third_line.x5<<endl;
    cout << E.x4 <<"    "<<chart.forth_line.x1<<"    "<<chart.forth_line.x2<<"    "<<chart.forth_line.x3<<"    "<<chart.forth_line.x4<<"    "<<chart.forth_line.x5<<endl;
    cout << E.x5 <<"    "<<chart.fifth_line.x1<<"    "<<chart.fifth_line.x2<<"    "<<chart.fifth_line.x3<<"    "<<chart.fifth_line.x4<<"    "<<chart.fifth_line.x5<<endl;
=======
>>>>>>> remove_glio version which is operational
    return true;
}
vector<bool> classify_glio(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method)
{
    cout<<"oooooooooooooooooooooooo"<<endl;
    vector<bool> classify;
    if(method)
    {
        double dis11 = sqrt( (E1.x1 - E_curr.x1)*(E1.x1 - E_curr.x1) + (chart1.first_line.x1-chart_curr.first_line.x1)*(chart1.first_line.x1-chart_curr.first_line.x1) +
                (chart1.first_line.x2-chart_curr.first_line.x2)*(chart1.first_line.x2-chart_curr.first_line.x2) + (chart1.first_line.x3-chart_curr.first_line.x3 )*(chart1.first_line.x3-chart_curr.first_line.x3)+
                (chart1.first_line.x4-chart_curr.first_line.x4)*(chart1.first_line.x4-chart_curr.first_line.x4) + (chart1.first_line.x5-chart_curr.first_line.x5)*(chart1.first_line.x5-chart_curr.first_line.x5) );

        double dis12 = sqrt( (E1.x2 - E_curr.x2)*(E1.x2 - E_curr.x2) + (chart1.second_line.x1-chart_curr.second_line.x1)*(chart1.second_line.x1-chart_curr.second_line.x1) +
                (chart1.second_line.x2-chart_curr.second_line.x2)*(chart1.second_line.x2-chart_curr.second_line.x2) + (chart1.second_line.x3-chart_curr.second_line.x3 )*(chart1.second_line.x3-chart_curr.second_line.x3)+
                (chart1.second_line.x4-chart_curr.second_line.x4)*(chart1.second_line.x4-chart_curr.second_line.x4) + (chart1.second_line.x5-chart_curr.second_line.x5)*(chart1.second_line.x5-chart_curr.second_line.x5) );

        double dis13 = sqrt( (E1.x3 - E_curr.x3)*(E1.x3 - E_curr.x3) + (chart1.third_line.x1-chart_curr.third_line.x1)*(chart1.third_line.x1-chart_curr.third_line.x1) +
                (chart1.third_line.x2-chart_curr.third_line.x2)*(chart1.third_line.x2-chart_curr.third_line.x2) + (chart1.third_line.x3-chart_curr.third_line.x3 )*(chart1.third_line.x3-chart_curr.third_line.x3)+
                (chart1.third_line.x4-chart_curr.third_line.x4)*(chart1.third_line.x4-chart_curr.third_line.x4) + (chart1.third_line.x5-chart_curr.third_line.x5)*(chart1.third_line.x5-chart_curr.third_line.x5) );

        double dis14 = sqrt( (E1.x4 - E_curr.x4)*(E1.x4 - E_curr.x4) + (chart1.forth_line.x1-chart_curr.forth_line.x1)*(chart1.forth_line.x1-chart_curr.forth_line.x1) +
                (chart1.forth_line.x2-chart_curr.forth_line.x2)*(chart1.forth_line.x2-chart_curr.forth_line.x2) + (chart1.forth_line.x3-chart_curr.forth_line.x3 )*(chart1.forth_line.x3-chart_curr.forth_line.x3)+
                (chart1.forth_line.x4-chart_curr.forth_line.x4)*(chart1.forth_line.x4-chart_curr.forth_line.x4) + (chart1.forth_line.x5-chart_curr.forth_line.x5)*(chart1.forth_line.x5-chart_curr.forth_line.x5) );

        double dis15 = sqrt( (E1.x5 - E_curr.x5)*(E1.x5 - E_curr.x5) + (chart1.fifth_line.x1-chart_curr.fifth_line.x1)*(chart1.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart1.fifth_line.x2-chart_curr.fifth_line.x2)*(chart1.fifth_line.x2-chart_curr.fifth_line.x2) + (chart1.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart1.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart1.fifth_line.x4-chart_curr.fifth_line.x4)*(chart1.fifth_line.x4-chart_curr.fifth_line.x4) + (chart1.fifth_line.x5-chart_curr.fifth_line.x5)*(chart1.fifth_line.x5-chart_curr.fifth_line.x5) );


        double dis21 = sqrt( (E2.x1 - E_curr.x1)*(E2.x1 - E_curr.x1) + (chart2.first_line.x1-chart_curr.first_line.x1)*(chart2.first_line.x1-chart_curr.first_line.x1) +
                (chart2.first_line.x2-chart_curr.first_line.x2)*(chart2.first_line.x2-chart_curr.first_line.x2) + (chart2.first_line.x3-chart_curr.first_line.x3 )*(chart2.first_line.x3-chart_curr.first_line.x3)+
                (chart2.first_line.x4-chart_curr.first_line.x4)*(chart2.first_line.x4-chart_curr.first_line.x4) + (chart2.first_line.x5-chart_curr.first_line.x5)*(chart2.first_line.x5-chart_curr.first_line.x5) );

        double dis22 = sqrt( (E2.x2 - E_curr.x2)*(E2.x2 - E_curr.x2) + (chart2.second_line.x1-chart_curr.second_line.x1)*(chart2.second_line.x1-chart_curr.second_line.x1) +
                (chart2.second_line.x2-chart_curr.second_line.x2)*(chart2.second_line.x2-chart_curr.second_line.x2) + (chart2.second_line.x3-chart_curr.second_line.x3 )*(chart2.second_line.x3-chart_curr.second_line.x3)+
                (chart2.second_line.x4-chart_curr.second_line.x4)*(chart2.second_line.x4-chart_curr.second_line.x4) + (chart2.second_line.x5-chart_curr.second_line.x5)*(chart2.second_line.x5-chart_curr.second_line.x5) );

        double dis23 = sqrt( (E2.x3 - E_curr.x3)*(E2.x3 - E_curr.x3) + (chart2.third_line.x1-chart_curr.third_line.x1)*(chart2.third_line.x1-chart_curr.third_line.x1) +
                (chart2.third_line.x2-chart_curr.third_line.x2)*(chart2.third_line.x2-chart_curr.third_line.x2) + (chart2.third_line.x3-chart_curr.third_line.x3 )*(chart2.third_line.x3-chart_curr.third_line.x3)+
                (chart2.third_line.x4-chart_curr.third_line.x4)*(chart2.third_line.x4-chart_curr.third_line.x4) + (chart2.third_line.x5-chart_curr.third_line.x5)*(chart2.third_line.x5-chart_curr.third_line.x5) );

        double dis24 = sqrt( (E2.x4 - E_curr.x4)*(E2.x4 - E_curr.x4) + (chart2.forth_line.x1-chart_curr.forth_line.x1)*(chart2.forth_line.x1-chart_curr.forth_line.x1) +
                (chart2.forth_line.x2-chart_curr.forth_line.x2)*(chart2.forth_line.x2-chart_curr.forth_line.x2) + (chart2.forth_line.x3-chart_curr.forth_line.x3 )*(chart2.forth_line.x3-chart_curr.forth_line.x3)+
                (chart2.forth_line.x4-chart_curr.forth_line.x4)*(chart2.forth_line.x4-chart_curr.forth_line.x4) + (chart2.forth_line.x5-chart_curr.forth_line.x5)*(chart2.forth_line.x5-chart_curr.forth_line.x5) );

        double dis25 = sqrt( (E2.x5 - E_curr.x5)*(E2.x5 - E_curr.x5) + (chart2.fifth_line.x1-chart_curr.fifth_line.x1)*(chart2.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart2.fifth_line.x2-chart_curr.fifth_line.x2)*(chart2.fifth_line.x2-chart_curr.fifth_line.x2) + (chart2.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart2.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart2.fifth_line.x4-chart_curr.fifth_line.x4)*(chart2.fifth_line.x4-chart_curr.fifth_line.x4) + (chart2.fifth_line.x5-chart_curr.fifth_line.x5)*(chart2.fifth_line.x5-chart_curr.fifth_line.x5) );
        double dis1 = dis11+dis12+dis13+dis14+dis15;
        double dis2 = dis21+dis22+dis23+dis24+dis25;

//        if(dis1>dis2){return true;}
//        else{return false;}

    }
    else
    {
        int size = feature_curr.count_v.size();
        for(int i=0;i<size;i++)
        {
            double dis1 = sqrt( (E1.x1 - feature_curr.y_n[i])*(E1.x1 - feature_curr.y_n[i]) + (E1.x2 - feature_curr.ratio_v[i])*(E1.x2 - feature_curr.ratio_v[i]) + (E1.x3 - feature_curr.overlap_level[i])*(E1.x3 - feature_curr.overlap_level[i])
                                + (E1.x4 - feature_curr.grey_std[i])*(E1.x4 - feature_curr.grey_std[i]) + (E1.x5 - feature_curr.count_v[i])*(E1.x5 - feature_curr.count_v[i]) );

            double dis2 = sqrt( (E2.x1 - feature_curr.y_n[i])*(E2.x1 - feature_curr.y_n[i]) + (E2.x2 - feature_curr.ratio_v[i])*(E2.x2 - feature_curr.ratio_v[i]) + (E2.x3 - feature_curr.overlap_level[i])*(E2.x3 - feature_curr.overlap_level[i])
                                + (E2.x4 - feature_curr.grey_std[i])*(E2.x4 - feature_curr.grey_std[i]) + (E2.x5 - feature_curr.count_v[i])*(E2.x5 - feature_curr.count_v[i]) );



            if(dis1>dis2)
            {
                classify.push_back(1);
            }
            else
            {
                classify.push_back(0);
            }


        }
        return classify;

    }

}
<<<<<<< HEAD
=======
vector<vector<Coordinate> > build_rela2(vector<vector<Coordinate> >&connected_region,QHash<int,vector<int> > &rela2)
{
    vector<V3DLONG> reminder;
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder.push_back(zero);
    }
    vector<vector<Coordinate> > result;
    for(QHash<int,vector<int> >::iterator it=rela2.begin();it!=rela2.end();++it)
    {
        cout<<"check"<<endl;
        if(reminder[it.key()]!=0)
        {
            cout<<"out"<<endl;
            continue;
        }
        vector<Coordinate> tmp;
        for(int i=0;i<connected_region[it.key()].size();i++)
        {
            tmp.push_back(connected_region[it.key()][i]);

        }
        reminder[it.key()]=1;
        for(int i=0;i<rela2[it.key()].size();i++)
        {
            for(int j=0;j<connected_region[rela2[it.key()][i]].size();j++)
            {
                tmp.push_back(connected_region[rela2[it.key()][i]][j]);
            }
            reminder[rela2[it.key()][i]]=1;
        }
        result.push_back(tmp);

    }
    return result;
}
bool simple_saveimage_wrapper_lxf(V3DPluginCallback2 & cb, const char * filename, unsigned char * pdata, V3DLONG sz[4], int datatype)
{
    cout<<"inside"<<endl;
    if (!filename || !sz || !pdata)
    {
        v3d_msg("some of the parameters for simple_saveimage_wrapper() are not valid.", 0);
        return false;
    }

    ImagePixelType dt;
    if (datatype==1)
        dt = V3D_UINT8;
    else if (datatype==2)
        dt = V3D_UINT16;
    else if (datatype==4)
        dt = V3D_FLOAT32;
    else
    {
        v3d_msg(QString("the specified save data type in simple_saveimage_wrapper() is not valid, dt=[%1].").arg(datatype), 0);
        return false;
    }
cout<<"inside1"<<endl;
for(int i=0;i<100000;i++)
{

    if(int(pdata[i])>20)
    {
        cout<<"i = "<<i<<endl;
        cout<<int(pdata[i])<<endl;
    }
}
cout<<"kkk"<<endl;
    Image4DSimple * outimg = new Image4DSimple;
    if (outimg)
    {
        cout<<"iii"<<endl;
        if(!outimg->setData(pdata, sz[0], sz[1], sz[2], sz[3], dt))
        {
            v3d_msg("wrong!");
        }
        cout<<"ooo"<<endl;
    }
    else
    {
        v3d_msg("Fail to new Image4DSimple for outimg.");
        return false;
    }
    cout<<"yes"<<endl;
//    for(int i=0;i<100000;i++)
//    {
//        if(int(outimg->getRawData()[i])>20)
//        {
//            cout<<"j = "<<i<<endl;
//            cout<<int(outimg->getRawData()[i])<<endl;
//        }
//    }
   // cout<<fi
    return cb.saveImage(outimg, (char *)filename);
    //in this case no need to delete "outimg" pointer as it is just a container and will not use too much memory
}
>>>>>>> remove_glio version which is operational
