#include "soma_remove_main.h"

#include <iostream>
 using namespace std;
//#define DIS((x,y,z),b) (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z)
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDISs(a,b) (sqrt(((a)->x-(b)->x)*((a)->x-(b)->x)+((a)->y-(b)->y)*((a)->y-(b)->y)+((a)->z-(b)->z)*((a)->z-(b)->z)))
bool FAR = 0;
bool CLOSE = 1;

struct Coordinate
{
    int x;
    int y;
    int z;
    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};
uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}
uint qHash(const vector<Coordinate> key)
{
    return key[0].x + key[0].y + key[0].z;
}


//bool if_is_connect(Coordinate* &curr,Coordinate* &b,vector<vector<vector<V3DLONG> > > &mark3D);
bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D);
void find_relation(QHash<vector<Coordinate>,vector<Coordinate> > &relation,vector<Coordinate> &curr,vector<Coordinate> &tmp,vector<V3DLONG> &reminder2,QHash<Coordinate,int> &map_index);
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
                                     cout<<"33333333333333"<<endl;
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
                                     cout<<"22222222222222"<<endl;
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
     cout<<"hahhahaa"<<endl;
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
    for(V3DLONG i=0;i<pagesz;i++)
    {
        if(int(data1d[i]) < 40)
        {
            im_cropped[i] = 0;
        }
        else
        {
            im_cropped[i] = 255;
        }
    }
    simple_saveimage_wrapper(callback,QString("binary.v3draw").toStdString().c_str(),im_cropped,in_sz,1);
    vector<vector<vector<V3DLONG> > > coodinate3D,mark3D;
    vector<vector<V3DLONG> > coodinate2D,mark2D;
    vector<V3DLONG> coodinate1D,mark1D;


    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < N; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < M; ix++)
            {
                V3DLONG tmp = im_cropped[offsetk + offsetj + ix];
                if(tmp==255)
                {
                   int one=1;
                   mark1D.push_back(one);
                }
                else
                {
                    int zero=0;
                    mark1D.push_back(zero);
                }
                coodinate1D.push_back(tmp);
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
    }
    int j = 1;
    int u = 0;
    int g = 0;
    vector<vector<Coordinate> >connected_region;
    QHash<Coordinate,int> map_index;
    vector<Coordinate> each_region1;
    vector<Coordinate> each_region;
QList<ImageMarker> marker_all3;
    for(V3DLONG iz = 1; iz < P-1; iz++) //P-1
    {
        cout<<"iz = "<<iz<<endl;
        for(V3DLONG iy = 1; iy < N-1; iy++)
        {
            for(V3DLONG ix = 1; ix < M-1; ix++)
            {
                Coordinate coord_curr;
                if(coodinate3D[iz][iy][ix]==255)
                {

                    if(!first_floor)
                    {
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;
                        each_region1.push_back(coord_curr);
                        connected_region.push_back(each_region1);
                        map_index.insert(coord_curr,j);
                        each_region1.clear();
                        j++;
                        g++;
                        first_floor = true;
                        continue;
                    }

                }
                else
                {
                    continue;
                }
                if(first_floor)
                {
                    if(coodinate3D[iz][iy][ix]==255)
                    {
                        cout<<iz<<"  "<<iy<<"  "<<ix<<endl;
                        int dex;

                        bool ind1 = false;
                        coord_curr.x = ix;
                        coord_curr.y = iy;
                        coord_curr.z = iz;




                        int n=2;
                        ImageMarker m;
                        m.x = coord_curr.x;
                        m.y = coord_curr.y;
                        m.z = coord_curr.z;
                        m.radius = 0.5;
                        m.type = n;
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
                            cout<<"connected_region["<<dex<<"].size ++ "<<endl;
                            connected_region[dex].push_back(coord_curr);
                            cout<<"map_index[connected_region[dex][0] = "<<map_index[connected_region[dex][0]]<<endl;
                            map_index.insert(coord_curr,map_index[connected_region[dex][0]]);
                            ind1 = false;
                            break;
                        }
                        else
                        {
                            each_region.clear();
                            each_region.push_back(coord_curr);
                            cout<<"connected_region +++++++++++++++++++++++++++++++++++++++++++++++++++++++ "<<endl;
                            u++;
                            connected_region.push_back(each_region);
                            map_index.insert(coord_curr,j);
                            j++;

                        }
                        each_region.clear();

                    }

                }
            }
        }

    }
    QHash<vector<Coordinate>,vector<Coordinate> > relation;
    cout<<"connected_region = "<<connected_region.size()<<endl;
    int count_ind=1;
    //vector<int> count;
    vector<V3DLONG> index;
    vector<V3DLONG> reminder;
    for(int l=0;l<connected_region.size();l++)
    {
        int zero = 0;
        reminder.push_back(zero);
    }
    for(int l=0;l<connected_region.size();l++)
    {
        for(int j=l+1;j<connected_region.size();j++)
        {
            if(reminder[l+1]!=0)continue;
            double min_dis = 1000000000000;
            for(int k=0;k<connected_region[l].size();k++)
            {
                for(int h=0;h<connected_region[j].size();h++)
                {

                    double dis = NTDIS(connected_region[l][k],connected_region[j][h]);
                    if(dis<min_dis)
                    {
                        min_dis = dis;
                    }

                }
            }
            if(min_dis<2.5)
            {
                relation.insert(connected_region[l],connected_region[j]);
                reminder[l] = 1;
                reminder[j] = 1;
                index.push_back(l);
                count_ind++;
            }
        }
    }
    QList<ImageMarker> marker_all2;
    int n=2;
    for(V3DLONG l=0;l<connected_region.size();l++)
    {
        for(int f=0;f<connected_region[l].size();f++)
        {
            ImageMarker m;
            m.x = connected_region[l][f].x;
            m.y = connected_region[l][f].y;
            m.z = connected_region[l][f].z;
            m.radius = 0.5;
            m.type = n;
            marker_all2.push_back(m);
        }
    }
    writeMarker_file(QString("marker_all_2.marker"),marker_all2);

//    for(QHash<vector<Coordinate>,vector<Coordinate> >::iterator it = relation.begin();it!=relation.end();it++)
//    {

//          cout<<"relation = "<<map_index[it.key()[0]]<<"  "<<map_index[it.value()[0]]<<endl;

//    }
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
        cout<<"map_index = "<<map_index[connected_region[l][0]]<<endl;
    }

    cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^write marker^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
    QList<ImageMarker> marker_all;
    cout<<"connected_region_final.size() = "<<connected_region_final.size()<<endl;
    for(V3DLONG l=0;l<connected_region_final.size();l++)
    {

        QList<ImageMarker> marker;
        //if(connected_region[l].size()<3)continue;
        double mean_x,mean_y,mean_z;
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
        writeMarker_file(QString("marker"+QString::number(l)+".marker"),marker);





        n++;
    }
    writeMarker_file(QString("marker_all.marker"),marker_all);



    cout<<"u = "<<u<<endl;
    cout<<"g = "<<g<<endl;
    cout<<"j = "<<j<<endl;

    writeMarker_file(QString("marker_all_3.marker"),marker_all3);
    cout<<"marker_all.size() = "<<marker_all.size()<<endl;
    cout<<"marker_all2.size() = "<<marker_all2.size()<<endl;
    cout<<"marker_all3.size() = "<<marker_all3.size()<<endl;
    if(im_cropped){delete []im_cropped;im_cropped=0;}
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
    if(dist<2.5)
    {
        return true;//this
//        if(mark3D[curr.z][curr.y][curr.x]==1)
//        {
//            mark3D[curr.z][curr.y][curr.x]==0;
//            return true;
//        }
//        else
//        {
//            return false;
//        }
    }
    else
    {
        return false;
    }
}
