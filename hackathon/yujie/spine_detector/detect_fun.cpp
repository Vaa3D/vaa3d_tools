#include "detect_fun.h"


detect_fun::detect_fun()
{
    data1Dc_float=0;
    mask1D=0;
    sz_image[0]=sz_image[1]=sz_image[2]=sz_image[3]=0;
    page_size=0;
    bg_thr=80;
}

detect_fun::~detect_fun()
{
    if(data1Dc_float!=0)
        memory_free_float1D(data1Dc_float);
    if(mask1D!=0)
        memory_free_uchar1D(mask1D);
}

vector<V3DLONG> detect_fun::spine_grow(float* bound_box,unsigned short *label,V3DLONG ind,
                        int max_spine_width,int spine_id,int max_pixel,int min_pixel)
{
    //qDebug()<<"in spine_grow";
    QString filename=QString::number(spine_id-1)+".txt";
    FILE *fp=fopen(filename.toAscii(),"wt");

    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> cluster,tmp_cluster;
    vector<V3DLONG> tmp_curr_layer;
    vector<V3DLONG> temp_i,temp_j;
    vector<float> spread_width_array;
    temp_i.push_back(ind);
    //tmp_curr_layer.push_back(ind);
    float temp_floor=bound_box[ind];
    //qDebug()<<"temp floor:"<<bound_box[ind];
    fprintf(fp,"temp floor: %.2f\n",temp_floor);
    map<V3DLONG,bool> temp_label;
    //temp_label[ind]=true;

   //label=0 not visited; label>=2 indicates cluster_id
    float floor=-1;
    float cluster_spread_width=-1;
    cluster_spread_width=-1;
    float spread_width=0;
    float spread_ratio;
    bool over_max_pixel=false;
    float nsum=0;
    vector<V3DLONG> neighbor(6,0);
    while (1)
    {
        //qDebug()<<"in loop";
        tmp_curr_layer.clear();
        floor=-1;
        spread_ratio=0;

        while (temp_i.size()>0)
        {
           temp_j.clear();
            //look at 26 neighbors of ind
           for (int i=0;i<temp_i.size();i++)
           {
               V3DLONG pos=temp_i[i];
               vector<V3DLONG> coord;
               coord=pos2xyz(pos,y_offset,z_offset);
               V3DLONG x=coord[0]; V3DLONG y=coord[1]; V3DLONG z=coord[2];
               for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_image[0]-1,x+1); dx++){
                   for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_image[1]-1,y+1); dy++){
                       for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_image[2]-1,z+1); dz++){
                           V3DLONG pos1=xyz2pos(dx,dy,dz,y_offset,z_offset);
                           if (temp_label[pos1]==1) continue;
                           if (bound_box[pos1]<0) continue;
                           if (bound_box[pos1]>=floor)
                           {
                               //check whether over 4 background nbs. If >4,set that pixel to bg pixel

                               if(pos1-1>=0&&pos1+1<page_size&&pos1-sz_image[0]>=0&&pos1+sz_image[0]<page_size
                                       &&pos1-z_offset>=0&&pos1+z_offset<page_size)
                               {
                                   neighbor[0]=pos1-1;
                                   neighbor[1]=pos1+1;
                                   neighbor[2]=pos1-sz_image[0];
                                   neighbor[3]=pos1+sz_image[0];
                                   neighbor[4]=pos1-z_offset;
                                   neighbor[5]=pos1+z_offset;
                                   nsum=0;
                                   for (int j=0;j<neighbor.size();j++)
                                   {
                                       if(bound_box[neighbor[j]]>0) nsum++;
                                   }
                                   if (nsum<=2) continue;
                                   //qDebug()<<"nsum:"<<nsum;
                               }
                               temp_j.push_back(pos1);
                               temp_label[pos1]=1;
                               if (floor<0&&bound_box[pos1]<temp_floor)
                                   temp_floor=bound_box[pos1];
                           }

                       }
                   }
               }
           }
           //qDebug()<<"temp_j size:"<<temp_j.size();
           fprintf(fp,"temp_curr_layer size: %d\n",temp_j.size());
           if (floor<0) {floor=temp_floor;
               fprintf(fp,"floor: %.2f\n",floor);}
           //qDebug()<<"floor:"<<floor;
           temp_i.clear();
           //if for the first round
           for (int k=0;k<temp_j.size();k++)
           {
               tmp_curr_layer.push_back(temp_j[k]);
           }
           if (tmp_curr_layer.size()>max_pixel)
           {
               over_max_pixel=true;
               qDebug()<<"over_max_pixel break:"<<tmp_curr_layer.size()<<":"<<max_pixel;
               fprintf(fp,"over_max_pixel_break: %d ,%d\n",tmp_curr_layer.size(),max_pixel);
               break;
           }
           //qDebug()<<"tmp_curr_layer size:"<<tmp_curr_layer.size();
           temp_i=temp_j;

        }
        //qDebug()<<"2loop:tmp_curr_layer size:"<<tmp_curr_layer.size();
        if (tmp_curr_layer.size()<=0)
        {
            qDebug()<<"seeds run out";
            fprintf(fp,"seeds run out\n");
            break;
        }
        if (over_max_pixel) {break;}
        //current layer analysis: spread width
        spread_width=calc_spread_width(tmp_curr_layer);
        fprintf(fp,"spread_width of current layer: %.2f\n",spread_width);
        if (spread_width>max_spine_width)
        {
            qDebug()<<"width breakout"<<spread_width;
            fprintf(fp,"width breakout: %.2f %d\n",spread_width,max_spine_width);
            break;
        }

        for (int k=0;k<tmp_curr_layer.size();k++)
        {
            tmp_cluster.push_back(tmp_curr_layer[k]);
        }

        //check the cluster spread ratio after adding the curr layer
        if (cluster_spread_width==-1){ //first time to calc cluster_spread_width
            cluster_spread_width=spread_width;
            spread_ratio=1;
            spread_width_array.push_back(spread_width);
            fprintf(fp,"first layer\n");
        }
        else
        {
            cluster_spread_width=calc_spread_width(tmp_cluster);
            fprintf(fp,"2+ layer cluster spread width: %.2f\n",cluster_spread_width);
            spread_width_array.push_back(cluster_spread_width);
            float sum=0.;
            for (int kk=0;kk<spread_width_array.size();kk++)
            {
                sum+=spread_width_array[kk];
            }
            if (sum==0) {qDebug()<<"sum==0,will crash"; break;}
            spread_ratio=cluster_spread_width*spread_width_array.size()/sum;
            fprintf(fp,"spread ratio: %.2f\n",spread_ratio);
        }

        if (spread_ratio>=1.5 && spread_width>15)
        {
            qDebug()<<"ratio break"<<spread_ratio<<"cluster_width:"
                   <<cluster_spread_width<<" current spread_width:"<<spread_width;
            fprintf(fp,"ratio break: %.2f\n",spread_ratio);
            break; //tmp_curr layer cannot be added
        }
        fprintf(fp,"push to cluster\n");
        for (int k=0;k<tmp_curr_layer.size();k++)
        {
            cluster.push_back(tmp_curr_layer[k]);
        }

        temp_i.clear();
        temp_i=tmp_curr_layer;
    }
    fprintf(fp,"cluster build finished,check aspect ratio and size\n");
    //compute aspect ratio
    if (!spread_width_array.empty())
    {
        float aspect_ratio=bound_box[ind]/spread_width_array.back();
        fprintf(fp,"aspect ratio: %.2f\n",aspect_ratio);
        if (aspect_ratio<0.25)
        {
            qDebug()<<"aspect ratio breakout"<<aspect_ratio;
            fprintf(fp,"aspect ratio breakout, reject cluster\n");
            cluster.clear();
            fprintf(fp,"cluster size: %d\n",cluster.size());
            fclose(fp);
            return cluster;
        }
    }
    if (cluster.size()>=min_pixel)
    {
        fprintf(fp,"cluster size: %d\n",cluster.size());
        for (int j=0;j<cluster.size();j++)
        {
            if(label[cluster[j]]<=1)
                label[cluster[j]]=spine_id;
        }
    }
    else {cluster.clear(); fprintf(fp,"cluster too small, reject cluster\n");}
    fclose(fp);
    return cluster;
}

float detect_fun::calc_spread_width(vector<V3DLONG> array)
{
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    V3DLONG x_min,x_max,y_min,y_max,z_min,z_max;
    x_min=sz_image[0];
    y_min=sz_image[1];
    z_min=sz_image[2];
    x_max=y_max=z_max=0;
    for (int i=0;i<array.size();i++)
    {
        vector<V3DLONG> coord=pos2xyz(array[i],y_offset,z_offset);
        V3DLONG x=coord[0];
        V3DLONG y=coord[1];
        V3DLONG z=coord[2];
        if (x>x_max) x_max=x;
        if (x<x_min) x_min=x;
        if (y>y_max) y_max=y;
        if (y<y_min) y_min=y;
        if (z>z_max) z_max=z;
        if (z<z_min) z_min=z;
    }
    float tmp=(x_max-x_min)*(x_max-x_min)+(y_max-y_min)*(y_max-y_min)+
            (z_max-z_min)*(z_max-z_min);
    float dis_tmp=sqrt(tmp);
    qDebug()<<"spread width:"<<x_max<<":"<<x_min<<":"<<y_max<<":"<<y_min<<":"
           <<z_max<<":"<<z_min<<":"<<dis_tmp;
    return dis_tmp;
}

vector<float> detect_fun::ray_shoot_feature(NeuronSWC p, NeuronSWC p0,LandmarkList &tmplist)
{
    vector<vector <float> > dir_vec;
    dir_vec=get_dir_vec(p,p0);
    vector <float> emptyvector;
    if(dir_vec.empty())
    {
        qDebug()<<"error happen.";
        return emptyvector;
    }
    int dir_vec_size=32;
    float v_prev,v_new,x_prev,y_prev,z_prev,x_new,y_new,z_new;
    tmplist.clear();
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    float x=p0.x;
    float y=p0.y;
    float z=p0.z;
    V3DLONG ind=xyz2pos(x,y,z,y_offset,z_offset);
    vector<float> ray(3,0);
    vector<float> dis;
    v_prev=data1Dc_float[ind];


    for (int i=0;i<dir_vec_size;i++)
    {
        float dx=dir_vec[i][0];
        float dy=dir_vec[i][1];
        float dz=dir_vec[i][2];
        //qDebug()<<"dx,dy,dz:"<<dx<<":"<<dy<<":"<<dz<<"v_prev:"<<v_prev;
        x_prev=x;
        y_prev=y;
        z_prev=z;
        v_new=v_prev;

        while (v_new>bg_thr)
        {
            x_new=x_prev+dx;
            if (x_new<0 || x_new>sz_image[0]-1)
            {
                y_new=y_prev+dy;
                z_new=z_prev+dz;
                //qDebug()<<"x touches edge";
                break;
            }
            y_new=y_prev+dy;
            if (y_new<0 || y_new>sz_image[1]-1)
            {
                z_new=z_prev+dz;
                //qDebug()<<"y touches edge";
                break;
            }
            z_new=z_prev+dz;
            if (z_new<0 || z_new>sz_image[2]-1)
            {
                //qDebug()<<"z touches edge";
                break;
            }
            V3DLONG pos=xyz2pos(x_new+0.5,y_new+0.5,z_new+0.5,y_offset,z_offset); //float to int
            v_new=data1Dc_float[pos];
            x_prev=x_new;
            y_prev=y_new;
            z_prev=z_new;

            //qDebug()<<"dir i:"<<i<<" x,y,z:"<<x_prev<<":"<<y_prev<<":"<<":"<<z_prev<<":"<<"intensity:"<<v_new;
        }

        ray[0]=x_new-dx;
        ray[1]=y_new-dy;
        ray[2]=z_new-dz;
        LocationSimple tmp;
        tmp.x=ray[0]; tmp.y=ray[1]; tmp.z=ray[2];
        //qDebug()<<"after shooting rays:"<<ray[0]<<":"<<ray[1]<<":"<<ray[2];
        float dis_tmp=(ray[0]-x)*(ray[0]-x)+(ray[1]-y)*(ray[1]-y)+(ray[2]-z)*(ray[2]-z);
        dis_tmp=sqrt(dis_tmp);
        //qDebug()<<"dis_tmp:"<<dis_tmp;
        dis.push_back(dis_tmp);
        tmplist.append(tmp);
    }

    QString filename="ray_coord.swc";
    FILE* fp=fopen(filename.toAscii(),"wt");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    for (int j=0;j<tmplist.size();j++)
    {
        fprintf(fp, "%d %d %.lf %.lf %.lf %.1f %d\n",j+1,1,tmplist.at(j).x,tmplist.at(j).y,tmplist.at(j).z,1.0,j+1);
    }
    fclose(fp);
    return dis;
}

void writer_swc(vector<vector<float> > new_dir_vec,QString filename, NeuronSWC p0)
{
    FILE *fp=fopen(filename.toAscii(),"a");
    for (int i=0;i<new_dir_vec.size();i++)
    {
        fprintf(fp, "%d %d %.2f %.2f %.2f %.2f %d\n",i+3,3,
                p0.x+new_dir_vec[i][0]*10,p0.y+new_dir_vec[i][1]*10,p0.z+new_dir_vec[i][2]*10,1,2);
        //qDebug()<<"new vec:"<<new_dir_vec[i][0]<<":"<<new_dir_vec[i][1]<<":"<<new_dir_vec[i][2];
    }
    fclose(fp);
}

vector<float> normalize_vec(vector<float> m)
{
    float sum_m=0;
    for (int i=0;i<m.size();i++)
    {
        sum_m=m[i]*m[i]+sum_m;
    }
    for (int i=0;i<m.size();i++)
    {
        m[i]=m[i]/sqrt(sum_m);
    }
    return m;
}

vector<vector<float> > get_dir_vec(NeuronSWC p, NeuronSWC p0)
{
    vector<float> n0(3,0);
    vector<float> v0(3,0);
    vector<float> v90(3,0);
    n0[0]=p.x-p0.x;
    n0[1]=p.y-p0.y;
    n0[2]=p.z-p0.z;
//    qDebug()<<"p:"<<p.x<<":"<<p.y<<":"<<p.z;
//    qDebug()<<"p0:"<<p0.x<<":"<<p0.y<<":"<<p0.z;
//    qDebug()<<"n0:"<<n0[0]<<":"<<n0[1]<<":"<<n0[2];
    if (n0[0]==0&&n0[1]==0&&n0[2]==0)
    {
       qDebug()<<"error, two points are exactly same";
       vector <vector <float> > emptyvector;
       return emptyvector;
    }

    QString filename="dir_vec.swc";
    FILE *fp=fopen(filename.toAscii(),"wt");
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    fprintf(fp, "%d %d %.lf %.lf %.lf %.1f %d\n",1,3,p.x,p.y,p.z,1,1);
    fprintf(fp, "%d %d %.lf %.lf %.lf %.1f %d\n",2,3,p0.x,p0.y,p0.z,1,1);
    fclose(fp);
    if (n0[0]==0&&n0[1]==0)
    {
        qDebug()<<"n0[0]=n0[1]=0";
        vector<vector<float> > dir_vec;
        for (int i=0;i<32;i++)
        {
            vector<float> new_dir(3,0);
            new_dir[0]=cos(i*2*pi/32); //divide into 32 pieces
            new_dir[1]=sin(i*2*pi/32);
            dir_vec.push_back(new_dir);
        }
        writer_swc(dir_vec,filename,p0);
        return dir_vec;
    }

    else if (n0[0]!=0&&n0[1]==0)  //xz plane
    {
       //qDebug()<<"xz plane";
       v0[1]=1;
       v90[0]=-n0[2]/n0[0];
       v90[2]=1;
       v90=normalize_vec(v90);
       vector<float> v180(3,0);
       vector<float> v270(3,0);
       v270[0]=-1*v90[0]; v270[1]=-1*v90[1]; v270[2]=-1*v90[2];
       v180[0]=-1*v0[0]; v180[1]=-1*v0[1];v180[2]=-1*v0[2];
       //push v0,v90,v180,v270 into a vector
       vector<vector<float> >dir_vec,new_dir_vec;
       vector<float> new_dir(3,0);
       dir_vec.push_back(v0);
       dir_vec.push_back(v90);
       dir_vec.push_back(v180);
       dir_vec.push_back(v270);
       //qDebug()<<"dir_vec size:"<<dir_vec.size();
       for (int j=0;j<3;j++)
       {
           new_dir_vec.clear();
           for (int i=0;i<dir_vec.size()-1;i++)
           {
               new_dir[0]=(dir_vec[i][0]+dir_vec[i+1][0])/2;
               new_dir[1]=(dir_vec[i][1]+dir_vec[i+1][1])/2;
               new_dir[2]=(dir_vec[i][2]+dir_vec[i+1][2])/2;
               new_dir=normalize_vec(new_dir);
               new_dir_vec.push_back(dir_vec[i]);
               new_dir_vec.push_back(new_dir);
           }
           //add the last one
           new_dir_vec.push_back(dir_vec.back());
           //add the one between last and first
           new_dir[0]=(dir_vec.back()[0]+dir_vec.front()[0])/2;
           new_dir[1]=(dir_vec.back()[1]+dir_vec.front()[1])/2;
           new_dir[2]=(dir_vec.back()[2]+dir_vec.front()[2])/2;
           new_dir=normalize_vec(new_dir);
           new_dir_vec.push_back(new_dir);

           //qDebug()<<"new_dir_vec size:"<<new_dir_vec.size();
           //copy the new_dir_vec into dir_vec and clean new_dir_vec
           dir_vec=new_dir_vec;
       }
       writer_swc(dir_vec,filename,p0);
       //qDebug()<<"finish dir_vec";
       return new_dir_vec;

    }
    else if (n0[0]==0&&n0[1]!=0)  //yz plane
    {
        //qDebug()<<"yz plane";
        v0[0]=1;
        v90[1]=-n0[2]/n0[1];
        v90[2]=1;
        v90=normalize_vec(v90);
        vector<float> v180(3,0);
        vector<float> v270(3,0);
        v270[0]=-1*v90[0]; v270[1]=-1*v90[1]; v270[2]=-1*v90[2];
        v180[0]=-1*v0[0]; v180[1]=-1*v0[1];v180[2]=-1*v0[2];
        //push v0,v90,v180,v270 into a vector
        vector<vector<float> >dir_vec,new_dir_vec;
        vector<float> new_dir(3,0);
        dir_vec.push_back(v0);
        dir_vec.push_back(v90);
        dir_vec.push_back(v180);
        dir_vec.push_back(v270);
        //qDebug()<<"dir_vec size:"<<dir_vec.size();

        for (int j=0;j<3;j++)
        {
            new_dir_vec.clear();
            for (int i=0;i<dir_vec.size()-1;i++)
            {
                new_dir[0]=(dir_vec[i][0]+dir_vec[i+1][0])/2;
                new_dir[1]=(dir_vec[i][1]+dir_vec[i+1][1])/2;
                new_dir[2]=(dir_vec[i][2]+dir_vec[i+1][2])/2;
                new_dir=normalize_vec(new_dir);
                new_dir_vec.push_back(dir_vec[i]);
                new_dir_vec.push_back(new_dir);
            }
            //add the last one
            new_dir_vec.push_back(dir_vec.back());
            //add the one between last and first
            new_dir[0]=(dir_vec.back()[0]+dir_vec.front()[0])/2;
            new_dir[1]=(dir_vec.back()[1]+dir_vec.front()[1])/2;
            new_dir[2]=(dir_vec.back()[2]+dir_vec.front()[2])/2;
            new_dir=normalize_vec(new_dir);
            new_dir_vec.push_back(new_dir);

            //qDebug()<<"new_dir_vec size:"<<new_dir_vec.size();
            //copy the new_dir_vec into dir_vec and clean new_dir_vec
            dir_vec=new_dir_vec;
        }
        writer_swc(dir_vec,filename,p0);
        //qDebug()<<"finish dir_vec";
        return new_dir_vec;
    }

    else if (n0[0]!=0&&n0[1]!=0)
    {
        //qDebug()<<"n0[0]!=0&&n0[1]!=0";

            //0,180 vector (v0_x,v0_y,0) (-v0_x,-v0_y,0) z set to 0
            v0[0]=n0[1]/sqrt(n0[0]*n0[0]+n0[1]*n0[1]);
            v0[1]=-n0[0]/sqrt(n0[0]*n0[0]+n0[1]*n0[1]);
            v0[2]=0;
            qDebug()<<"v0:"<<v0[0]<<":"<<v0[1]<<":"<<v0[2];

            //90,270 vector (v90,-v90) |z| set to 1
           float ratio_v=v0[1]/v0[0];
           float ratio_n=n0[1]/n0[0];
           //qDebug()<<"ratio_v:"<<ratio_v<<" ratio_n:"<<ratio_n;
           if(ratio_v!=ratio_n)
           {
                v90[1]=n0[2]/(n0[0]*(ratio_v-ratio_n));
                v90[0]=-ratio_v*v90[1];
                v90[2]=1;
           }

            //normalize v90
            v90=normalize_vec(v90);
            //qDebug()<<"v90:"<<v90[0]<<":"<<v90[1]<<":"<<v90[2];
            vector<float> v180(3,0);
            vector<float> v270(3,0);
            v270[0]=-1*v90[0]; v270[1]=-1*v90[1]; v270[2]=-1*v90[2];
            v180[0]=-1*v0[0]; v180[1]=-1*v0[1];v180[2]=-1*v0[2];
            //qDebug()<<"v180:"<<v180[0]<<":"<<v180[1]<<":"<<v180[2];

            //push v0,v90,v180,v270 into a vector
            vector<vector<float> >dir_vec,new_dir_vec;
            vector<float> new_dir(3,0);
            dir_vec.push_back(v0);
            dir_vec.push_back(v90);
            dir_vec.push_back(v180);
            dir_vec.push_back(v270);
            //qDebug()<<"dir_vec size:"<<dir_vec.size();

            for (int j=0;j<3;j++)
            {
                new_dir_vec.clear();
                for (int i=0;i<dir_vec.size()-1;i++)
                {
                    new_dir[0]=(dir_vec[i][0]+dir_vec[i+1][0])/2;
                    new_dir[1]=(dir_vec[i][1]+dir_vec[i+1][1])/2;
                    new_dir[2]=(dir_vec[i][2]+dir_vec[i+1][2])/2;
                    new_dir=normalize_vec(new_dir);
                    new_dir_vec.push_back(dir_vec[i]);
                    new_dir_vec.push_back(new_dir);
                }
                //add the last one
                new_dir_vec.push_back(dir_vec.back());
                //add the one between last and first
                new_dir[0]=(dir_vec.back()[0]+dir_vec.front()[0])/2;
                new_dir[1]=(dir_vec.back()[1]+dir_vec.front()[1])/2;
                new_dir[2]=(dir_vec.back()[2]+dir_vec.front()[2])/2;
                new_dir=normalize_vec(new_dir);
                new_dir_vec.push_back(new_dir);

                //qDebug()<<"new_dir_vec size:"<<new_dir_vec.size();
                //copy the new_dir_vec into dir_vec and clean new_dir_vec
                dir_vec=new_dir_vec;
            }
            writer_swc(dir_vec,filename,p0);
            //qDebug()<<"finish dir_vec";
            return new_dir_vec;

   }

}

vector<V3DLONG> detect_fun::distance_to_skel(vector<MyMarker> bubbles,NeuronTree &neuron,vector<int>&neuron_idx,
                                                  LandmarkList &far_points)
{
    //get the foreground points
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    qDebug()<<"sz_image in dtect_fun:"<<sz_image[3];
    vector<V3DLONG> neighbor(6,0);
    vector<V3DLONG> boundary;
//    for (V3DLONG i=0;i<page_size;i++)
//    {
//        if (data1Dc_float[i]>=bg_thr)
//        {
//            mask1D[i]=1;
//        }
//    }
    //calculate neuron radius
//    for (int i=0;i<neuron.listNeuron.size();i++)
//    {
//        float s_x=neuron.listNeuron.at(i).x;
//        float s_y=neuron.listNeuron.at(i).y;
//        float s_z=neuron.listNeuron.at(i).z;
//        V3DLONG pos=xyz2pos(s_x,s_y,s_z,y_offset,z_offset);
//        float radius=skel_node_radius(pos);
//        neuron.listNeuron[i].radius=radius;
//        qDebug()<<"radius"<<radius;
//    }
//    //get the boundary points
//    for (V3DLONG i=0;i<page_size;i++)
//    {
//        int sum_tmp=0; int count=0;
//        if (i-1>=0) {neighbor[0]=i-1; sum_tmp+=mask1D[neighbor[0]]; count++;}
//        if (i+1<page_size) {neighbor[1]=i+1; sum_tmp+=mask1D[neighbor[1]]; count++;}
//        if (i-sz_image[0]>=0) {neighbor[2]=i-sz_image[0];sum_tmp+=mask1D[neighbor[2]]; count++;}
//        if (i+sz_image[0]<page_size) {neighbor[3]=i+sz_image[0]; sum_tmp+=mask1D[neighbor[3]]; count++;}
//        if (i-z_offset>=0) {neighbor[4]=i-z_offset; sum_tmp+=mask1D[neighbor[4]]; count++;}
//        if (i+z_offset<page_size) {neighbor[5]=i+z_offset; sum_tmp+=mask1D[neighbor[5]]; count++;}
//        if (count>=5&&sum_tmp<=1) continue;
//        if (sum_tmp<count&&mask1D[i]>0)
//        {
//           boundary.push_back(i);
//        }
//    }
    for (V3DLONG j=0;j<bubbles.size();j++)
    {
        V3DLONG pos=xyz2pos(bubbles[j].x,bubbles[j].y,bubbles[j].z,y_offset,z_offset);
        boundary.push_back(pos);
    }
    vector<float> distance_min;
    vector<V3DLONG> skel_ind; //store the neuron number closest to the boundary point
    vector<V3DLONG> boundary_order;
    while (!boundary.empty())
    {
        //calculate the distance and get the smallest one
        V3DLONG ind=boundary.back();
        vector<V3DLONG> coord;
        coord=pos2xyz(ind,y_offset,z_offset);
        float x=coord[0];
        float y=coord[1];
        float z=coord[2];
        float min_dis=10000;
        V3DLONG tmp_neuron_id=0;
        for (int i=0;i<neuron.listNeuron.size();i++)
        {
            float s_x=neuron.listNeuron.at(i).x;
            float s_y=neuron.listNeuron.at(i).y;
            float s_z=neuron.listNeuron.at(i).z;
            float tmp_dis=(s_x-x)*(s_x-x)+(s_y-y)*(s_y-y)+(s_z-z)*(s_z-z);
            if (tmp_dis<min_dis*min_dis)
            {
                min_dis=sqrt(tmp_dis);
                tmp_neuron_id=i;
            }
        }

        boundary_order.push_back(ind);
        distance_min.push_back(min_dis);
        skel_ind.push_back(tmp_neuron_id);
        boundary.pop_back();
        //qDebug()<<"ind:"<<ind<<":"<<min_dis<<":"<<min_dis<<":"<<"neuron id:"<<tmp_neuron_id;
    }
    qDebug()<<"boundary number:"<<boundary_order.size();

    // pick out points that are far from skeleton. min_dis>3*skel node radius
    vector<V3DLONG> far_point_ind(boundary_order.size(),0);//if 1 far point, if 0 near point
    for (V3DLONG i=0;i<boundary_order.size();i++)
    {
        if (distance_min[i]>4*neuron.listNeuron.at(skel_ind[i]).r)
        {
            far_point_ind[i]=1;
            LocationSimple tmp;
            vector<V3DLONG> tmp_coord=pos2xyz(boundary_order[i],y_offset,z_offset);
            tmp.x=tmp_coord[0]; tmp.y=tmp_coord[1]; tmp.z=tmp_coord[2];
            far_points.push_back(tmp);
        }
    }
    qDebug()<<"far_points size:"<<far_points.size();
//    vector<V3DLONG> tmp_coord;
//    LocationSimple tmp;
//    vector<LandmarkList> LList_stack;
//    LandmarkList LList;
//    for (int j=0;j<neuron.listNeuron.size();j++)
//    {
//        LList.clear();
//        tmp.x=neuron.listNeuron.at(j).x;
//        tmp.y=neuron.listNeuron.at(j).y;
//        tmp.z=neuron.listNeuron.at(j).z;
//        LList.append(tmp);
//        int count=0; float sum_dis=0;
//        for (int i=0;i<boundary_order.size();i++)
//        {
//            if (1) //far_point_ind[i]==0)
//            {
//                if (skel_ind[i]==j)
//                {
//                    tmp_coord=pos2xyz(boundary_order[i],y_offset,z_offset);
//                    tmp.x=tmp_coord[0]; tmp.y=tmp_coord[1]; tmp.z=tmp_coord[2];
//                    tmp.radius=distance_min[i]; //store the minimum distance
//                    count++;
//                    sum_dis+=distance_min[i];
//                    LList.append(tmp);
//                }
//            }
//        }
//        //if only one member in the group, ignore the group
//        if (LList.size()==0)
//        {
//            qDebug()<<"No member in the group,ignore";
//            continue;
//        }

////        //normalize each group
////        float sum_dev=0;
////        float std;
////        float ave_dis=sum_dis/count;
////        //qDebug()<<"ave_dis:"<<ave_dis;
////        for (int i=0;i<LList.size();i++)
////        {
////            sum_dev+=(LList.at(i).radius-ave_dis)*(LList.at(i).radius-ave_dis);
////        }

////        std=sqrt(sum_dev/count);
////        qDebug()<<"j:"<<j<<" std:"<<std;
////        for (int i=0;i<LList.size();i++)
////        {
////            LList[i].radius=(LList[i].radius-ave_dis)/std;
////        }
//        //qDebug()<<"neuron "<<j<<" :"<<count;
//        LList_stack.push_back(LList);
//        neuron_idx.push_back(j);
//    }
    //qDebug()<<"dis to skel finished"<<LList_stack.size();
    //return LList_stack;
    return far_point_ind;
}


vector<V3DLONG> detect_fun::bubbles_no_gsdt(unsigned char *seperate)
{
    qDebug()<<"In bubbles_no_gsdt";
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    memset(mask1D,0,page_size);
    vector<V3DLONG> fg_map;
    vector<float> gsdt;

    unsigned char *gsdt_image=new unsigned char[page_size];
    memset(gsdt_image,0,page_size);

    for (V3DLONG i=0;i<page_size;i++)
    {
        if (data1Dc_float[i]>=bg_thr && seperate[i]<=0)
        {
            gsdt_image[i]=data1Dc_float[i];
        }
    }
    //generate gsdt
    float *phi=0;
    fastmarching_dt(gsdt_image,phi,sz_image[0],sz_image[1],sz_image[2],3,bg_thr);
    float min_val = phi[0], max_val = phi[0];
    float *outimg1d_tmp = new float[page_size*sizeof(float)];
    for(V3DLONG i = 0; i < page_size; i++)
    {
        if(phi[i] == INF) continue;
        min_val = MIN(min_val, phi[i]);
        max_val = MAX(max_val, phi[i]);
    }
    cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
    max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;
    for(V3DLONG i = 0; i < page_size; i++)
    {
        if(phi[i] == INF) outimg1d_tmp[i] = 0;
        else if(phi[i] ==0) outimg1d_tmp[i] = 0;
        else
        {
            outimg1d_tmp[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
            outimg1d_tmp[i] = MAX(outimg1d_tmp[i], 1);
        }
    }
    if (phi!=0) {delete phi; phi=0;}

    //get the gsdt intensity for each of the foreground points
    for (V3DLONG i=0;i<page_size;i++)
    {
        if (gsdt_image[i]>0)
        {
            mask1D[i]=1;
            fg_map.push_back(i);
            gsdt.push_back(outimg1d_tmp[i]);
        }
    }
    qDebug()<<"foreground num:"<<fg_map.size();

    multimap<float, V3DLONG> order_map;
    for(V3DLONG i = 0; i < fg_map.size(); i++)
        order_map.insert(pair<float, V3DLONG> (gsdt[i],fg_map[i]));

    qDebug()<<"ordermapsize:"<<order_map.size();

     multimap<float, V3DLONG>::reverse_iterator it = order_map.rbegin();

     vector<V3DLONG> bubble_ind;
     //bubbles.clear();
     vector<MyMarker> bubbles;
     int count1,count10; count1=count10=0;
     while(it!= order_map.rend())
     {
         //qDebug()<<"intensity:"<<it->first<<" order:"<<it->second;
         V3DLONG ind=it->second;
         vector<V3DLONG> coord=pos2xyz(ind,y_offset,z_offset);
         if(mask1D[ind]==0)
         {
             it++;
             continue;
         }
         //vector<V3DLONG> coord=pos2xyz(ind,y_offset,z_offset);
         double radius=markerRadius_nooverlap(ind);
         if (radius<=1)
         {
             count1++;
             it++;
             continue;
         }
        else
         {
             MyMarker tmp;
             tmp.x=coord[0]; tmp.y=coord[1]; tmp.z=coord[2]; tmp.radius=radius;
             bubbles.push_back(tmp);
             bubble_ind.push_back(ind);
         }
         it++;
     }
     qDebug()<<"count1,10:"<<count1;
     write_csv(bubbles,"attached_bubbles.marker");
     return bubble_ind;
}

vector<MyMarker> detect_fun::bubbles()
{
    vector<MyMarker> bubbles;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> fg_map;
    vector<float> gsdt;
    //to record the order of points in gstd based on intensity
    qDebug()<<"sz:"<<sz_image[3]<<":"<<sz_image[2]<<"total size:"<<page_size;

    //generate gsdt
    float *phi=0;
    fastmarching_dt(data1Dc_float,phi,sz_image[0],sz_image[1],sz_image[2],3,bg_thr);

    float min_val = phi[0], max_val = phi[0];

    float *outimg1d_tmp = new float[page_size*sizeof(float)];

    for(V3DLONG i = 0; i < page_size; i++)
    {
        if(phi[i] == INF) continue;
        min_val = MIN(min_val, phi[i]);
        max_val = MAX(max_val, phi[i]);
    }
    cout<<"min_val = "<<min_val<<" max_val = "<<max_val<<endl;
    max_val -= min_val; if(max_val == 0.0) max_val = 0.00001;

    for(V3DLONG i = 0; i < page_size; i++)
    {
        if(phi[i] == INF) outimg1d_tmp[i] = 0;
        else if(phi[i] ==0) outimg1d_tmp[i] = 0;
        else
        {
            outimg1d_tmp[i] = (phi[i] - min_val)/max_val * 255 + 0.5;
            outimg1d_tmp[i] = MAX(outimg1d_tmp[i], 1);
        }
    }

    if (phi!=0) {delete phi; phi=0;}

    //get the gsdt intensity for each of the foreground points
    for (V3DLONG i=0;i<page_size;i++)
    {
        if (data1Dc_float[i]>=bg_thr)
        {
            mask1D[i]=1;
            fg_map.push_back(i);
            gsdt.push_back(outimg1d_tmp[i]);
        }
    }
    qDebug()<<"foreground num:"<<fg_map.size();

    //sort the foreground points in gsdt
    multimap<double, V3DLONG> order_map;
    for(V3DLONG i = 0; i < fg_map.size(); i++)
    {
        order_map.insert(pair<double, V3DLONG> (gsdt[i],fg_map[i]));
    }


    qDebug()<<"ordermapsize:"<<order_map.size();

     multimap<double, V3DLONG>::reverse_iterator it = order_map.rbegin();

     bubbles.clear();
     int count1,count10; count1=count10=0;
     while(it!= order_map.rend())
     {
         //qDebug()<<"intensity:"<<it->first<<" order:"<<it->second;
         V3DLONG ind=it->second;
         if (mask1D[ind]==0)
         {
             it++;
             continue;
         }
         vector<V3DLONG> coord=pos2xyz(ind,y_offset,z_offset);
         V3DLONG x=coord[0];
         V3DLONG y=coord[1];
         V3DLONG z=coord[2];
         double radius=markerRadius(ind);

         for (V3DLONG dx=MAX(0,x-radius);dx<=MIN(x+radius,sz_image[0]-1);dx++){
             for (V3DLONG dy=MAX(0,y-radius);dy<=MIN(y+radius,sz_image[1]-1);dy++){
                 for (V3DLONG dz=MAX(0,z-radius);dz<=MIN(z+radius,sz_image[2]-1);dz++)
                 {
                     double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
                     if (tmp>radius*radius) continue;
                     V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                     mask1D[pos]=0;
                 }
             }
         }

         if (radius<=1.5)
         {
             count1++;
             it++;
             continue;
         }
         if (radius>7)
         {
             count10++;
             it++;
             continue;
         }

        else
         {
             MyMarker p;
             p.x=x; p.y=y; p.z=z; p.radius=radius;
             bubbles.push_back(p);
         }

         it++;
     }
    write_swc(bubbles,"first_time.swc");
    qDebug()<<"count1:"<<count1<<" count10+:"<<count10;
    qDebug()<<"first time filter:"<<bubbles.size()<<" left";

    //-----second time: build map one more time and grow one more time
    multimap<double, V3DLONG> order_sec_map;
    for(V3DLONG i = 0; i < bubbles.size(); i++)
    {
        V3DLONG pos=xyz2pos(bubbles[i].x,bubbles[i].y,bubbles[i].z,y_offset,z_offset);
        order_sec_map.insert(pair<double, V3DLONG> (outimg1d_tmp[pos],pos));
    }
    qDebug()<<" order_sec_map:"<<order_sec_map.size();
    memset(mask1D,0,page_size*sizeof(unsigned char));

    for (V3DLONG i=0;i<page_size;i++)
    {
        if (data1Dc_float[i]>=bg_thr) mask1D[i]=1;
    }

    multimap<double, V3DLONG>::reverse_iterator it_s = order_sec_map.rbegin();
    bubbles.clear(); count10=count1=0;
    while(it_s!= order_sec_map.rend())
    {
        //qDebug()<<"intensity:"<<it_s->first<<" order:"<<it_s->second;
        V3DLONG ind=it_s->second;
        if (mask1D[ind]==0)
        {
            it_s++;
            continue;
        }
        vector<V3DLONG> coord=pos2xyz(ind,y_offset,z_offset);
        V3DLONG x=coord[0];
        V3DLONG y=coord[1];
        V3DLONG z=coord[2];
        double radius=markerRadius_nooverlap(ind);

        if (radius<=1.5)
        {
            count1++;
            it_s++;
            continue;
        }
        if (radius>=7)
        {
            count10++;
            it_s++;
            continue;
        }
       else
        {
            MyMarker p;
            p.x=x; p.y=y; p.z=z; p.radius=radius;
            bubbles.push_back(p);
        }

        it_s++;
    }
    qDebug()<<"count1:"<<count1<<" count10:"<<count10;
    qDebug()<<"bubbles size second:"<<bubbles.size();
    //write_swc(bubbles,"second_time.swc");
//    //Perform mean_shift and calculate the mean shift distance
//    vector<MyMarker> dis_small,dis_mid,dis_large;
//    QString filename1="mean_shift_dis.txt";
//    FILE *fp1=fopen(filename1.toAscii(),"w");
//    //run a mean_shift_on all obtained bubbles and record their moving distance
//    for (int j=0;j<bubbles.size();j++)
//    {
//        qDebug()<<"_______j:____________"<<j;
//        V3DLONG pos=xyz2pos(bubbles[j].x,bubbles[j].y,bubbles[j].z,y_offset,z_offset);
//        double distance=calc_mean_shift_center(pos,15,outimg1d_tmp,sz_image,2);
//        fprintf(fp1,"%.2f\n",distance);
//        if (distance>0&&distance<=2)
//            dis_small.push_back(bubbles[j]);
//        else if(distance>=8)
//            dis_large.push_back(bubbles[j]);
//        else dis_mid.push_back(bubbles[j]);
//    }
//    fclose(fp1);

//gather the rays for each node
    vector<MyMarker> bubbles_ray_filter;
    bool filter_flag;
    vector<float> dis(48,0);
    QString filename="nodes_ray.csv";
    FILE *fp=fopen(filename.toAscii(),"w");
    for (int i=0;i<bubbles.size();i++)
    {
        V3DLONG ind=xyz2pos(bubbles[i].x,bubbles[i].y,bubbles[i].z,y_offset,z_offset);
        dis=ray_shoot_vector(ind);
        for (int j=0;j<48;j++)
        {
          if (j==47) fprintf(fp,"%.2f\n",dis[j]);
          else fprintf(fp,"%.2f,",dis[j]);
        }
//        for (int j=0;j<48;j++)
//        {
//            filter_flag=false;
//            if (dis[j]>60)
//            {
//               filter_flag=true;
//               break;
//            }
//        }
//        if (filter_flag) bubbles_ray_filter.push_back(bubbles[i]);
        //node_dir.push_back(bound);
    }
    fclose(fp);
    //qDebug()<<"after ray_filter:"<<bubbles_ray_filter.size();


    write_csv(bubbles,"nodes.marker");
    //write_swc(bubbles_ray_filter,"bubbles_ray_filter.swc");
//    write_swc(dis_large,"bubbles_large.swc");
    qDebug()<<"the end....";
    return bubbles;
}

double detect_fun::calc_mean_shift_center(V3DLONG ind, int windowradius,float *data1Dc_float,
                                     V3DLONG sz_image[],int methodcode)
{
    //qDebug()<<"methodcode:"<<methodcode;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    V3DLONG page_size=sz_image[0]*sz_image[1]*sz_image[2];

    V3DLONG pos;
    vector<V3DLONG> coord;

    float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
    float center_dis=1;
    vector<float> center_float(3,0);

    coord=pos2xyz(ind, y_offset, z_offset);
    x=(float)coord[0];y=(float)coord[1];z=(float)coord[2];
    //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;

    //find out the channel with the maximum intensity for the marker
    v_prev=data1Dc_float[ind];
    int channel=0;
    for (int j=1;j<sz_image[3];j++)
    {
        if (data1Dc_float[ind+page_size*j]>v_prev)
        {
            v_prev=data1Dc_float[ind+page_size*j];
            channel=j;
        }
    }
    //qDebug()<<"v_Prev:"<<v_prev;
    int testCount=0;
    int testCount1=0;

    while (center_dis>=0.5)
    {
        total_x=total_y=total_z=sum_v=0;

        testCount=testCount1=0;

        for(V3DLONG dx=MAX(x+0.5-windowradius,0); dx<=MIN(sz_image[0]-1,x+0.5+windowradius); dx++){
            for(V3DLONG dy=MAX(y+0.5-windowradius,0); dy<=MIN(sz_image[1]-1,y+0.5+windowradius); dy++){
                for(V3DLONG dz=MAX(z+0.5-windowradius,0); dz<=MIN(sz_image[2]-1,z+0.5+windowradius); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=data1Dc_float[pos+page_size*channel];

                    total_x=v_color*(float)dx+total_x;
                    total_y=v_color*(float)dy+total_y;
                    total_z=v_color*(float)dz+total_z;
                    sum_v=sum_v+v_color;

                    testCount++;
                    if(v_color>100)
                        testCount1++;
                 }
             }
         }
        //qDebug()<<"windowradius:"<<windowradius;
        //qDebug()<<"total xyz:"<<total_x<<":"<<total_y<<":"<<total_z<<":"<<sum_v<<":"<<sum_v/testCount<<":"<<testCount<<":"<<testCount1;

        center_float[0]=total_x/sum_v;
        center_float[1]=total_y/sum_v;
        center_float[2]=total_z/sum_v;

        if (total_x<1e-5||total_y<1e-5||total_z<1e-5) //a very dark marker.
        {

            v3d_msg("Sphere surrounding the marker is zero. Mean-shift cannot happen. Marker location will not move",0);
            center_float[0]=x;
            center_float[1]=y;
            center_float[2]=z;
            return 0;
        }

        V3DLONG prev_ind=xyz2pos((int)(x+0.5),(int)(y+0.5),(int)(z+0.5),y_offset,z_offset);
        V3DLONG tmp_ind=xyz2pos((int)(center_float[0]+0.5),(int)(center_float[1]+0.5),(int)(center_float[2]+0.5),
                y_offset,z_offset);

        if (methodcode==2)
        {
            if (data1Dc_float[tmp_ind+channel*page_size]<data1Dc_float[prev_ind+channel*page_size]&& windowradius>=2) // && windowradius>2)
            {
            //qDebug()<<methodcode<<" window too large"<<windowradius;
            windowradius--;
            center_dis=1;
            continue;
            }
         }

        float tmp_1=(center_float[0]-x)*(center_float[0]-x)+(center_float[1]-y)*(center_float[1]-y)
                    +(center_float[2]-z)*(center_float[2]-z);
        center_dis=sqrt(tmp_1);

        //qDebug()<<"new_center:"<<center_float[0]<<":"<<center_float[1]<<":"<<center_float[2]<<" intensity:"<<data1Dc_float[tmp_ind+channel*page_size];
        //qDebug()<<"center distance:"<<center_dis;
        x=center_float[0]; y=center_float[1]; z=center_float[2];
    }
    double dis_tmp=(x-coord[0])*(x-coord[0])+(y-coord[1])*(y-coord[1])+(z-coord[2])*(z-coord[2]);
    double dis=sqrt(dis_tmp);
    return dis;
}
vector<float> detect_fun::ray_shoot_vector(V3DLONG ind)
{
    float dir_vec[48][3]={{0.6667,0.6667,0.3333},{0.2852,0.6886,0.6667},{0.6886,0.2852,0.6667},
                        {0.2826,0.2826,0.9167},{-0.6667,0.6667,0.3333},{-0.6886,0.2852,0.6667},
                        {-0.2852,0.6886,0.6667},{-0.2826,0.2826,0.9167},{-0.6667,-0.6667,0.3333},
                        {-0.2852,-0.6886,0.6667},{-0.6886,-0.2852,0.6667},{-0.2826,-0.2826,0.9167},
                        {0.6667,-0.6667,0.3333},{0.6886,-0.2852,0.6667},{0.2852,-0.6886,0.6667},
                        {0.2826,-0.2826,0.9167},{0.9428,0.0000,-0.3333},{0.9239,0.3827,0.0000},
                        {0.9239,-0.3827,0.0000},{0.9428,0.0000,0.3333},{0.0000,0.9428,-0.3333},
                        {-0.3827,0.9239,0.0000},{0.3827,0.9239,0.0000},{0.0000,0.9428,0.3333},
                        {-0.9428,0.0000,-0.3333},{-0.9239,-0.3827,0.0000},{-0.9239,0.3827,0.0000},
                        {-0.9428,0.0000,0.3333},{0.0000,-0.9428,-0.3333},{0.3827,-0.9239,0.0000},
                        {-0.3827,-0.9239,0.0000},{0.0000,-0.9428,0.3333},{0.2826,0.2826,-0.9167},
                        {0.2852,0.6886,-0.6667},{0.6886,0.2852,-0.6667},{0.6667,0.6667,-0.3333},
                        {-0.2826,0.2826,-0.9167},{-0.6886,0.2852,-0.6667},{-0.2852,0.6886,-0.6667},
                        {-0.6667,0.6667,-0.3333},{-0.2826,-0.2826,-0.9167},{-0.2852,-0.6886,-0.6667},
                        {-0.6886,-0.2852,-0.6667},{-0.6667,-0.6667,-0.3333},{0.2826,-0.2826,-0.9167},
                        {0.6886,-0.2852,-0.6667},{0.2852,-0.6886,-0.6667},{0.6667,-0.6667,-0.3333}};

    int dir_vec_size=48;
    float v_prev,v_new,x_prev,y_prev,z_prev,x_new,y_new,z_new;

    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord=pos2xyz(ind, y_offset, z_offset);
    V3DLONG x,y,z;
    x=coord[0];y=coord[1];z=coord[2];
    vector<float> ray(3,0);
    vector<float> dis;
    vector<vector<float> > bound;
    v_prev=data1Dc_float[ind];

    for (int i=0;i<dir_vec_size;i++)
    {
        float dx=dir_vec[i][0];
        float dy=dir_vec[i][1];
        float dz=dir_vec[i][2];
        //qDebug()<<"dx,dy,dz:"<<dx<<":"<<dy<<":"<<dz;
        x_prev=x;
        y_prev=y;
        z_prev=z;
        v_new=v_prev;

        while (v_new>bg_thr)
        {
            x_new=x_prev+dx;
            if (x_new<0 || x_new>sz_image[0]-1)
            {
                y_new=y_prev+dy;
                z_new=z_prev+dz;
                //qDebug()<<"x touches edge";
                break;
            }
            y_new=y_prev+dy;
            if (y_new<0 || y_new>sz_image[1]-1)
            {
                z_new=z_prev+dz;
                //qDebug()<<"y touches edge";
                break;
            }
            z_new=z_prev+dz;
            if (z_new<0 || z_new>sz_image[2]-1)
            {
                //qDebug()<<"z touches edge";
                break;
            }
            V3DLONG pos=xyz2pos(x_new+0.5,y_new+0.5,z_new+0.5,y_offset,z_offset); //float to int
            v_new=data1Dc_float[pos];
            x_prev=x_new;
            y_prev=y_new;
            z_prev=z_new;

            //qDebug()<<"dir i:"<<i<<" x,y,z:"<<x_prev<<":"<<y_prev<<":"<<":"<<z_prev<<":"<<"intensity:"<<v_new;
        }

        ray[0]=x_new-dx;
        ray[1]=y_new-dy;
        ray[2]=z_new-dz;
        float dis_tmp=(ray[0]-x)*(ray[0]-x)+(ray[1]-y)*(ray[1]-y)+(ray[2]-z)*(ray[2]-z);
        dis_tmp=sqrt(dis_tmp);
        //qDebug()<<"dis_tmp:"<<dis_tmp;
        dis.push_back(dis_tmp);
        bound.push_back(ray);
    }

    //build a map and sort bound based on length of rays
    //qDebug()<<"boudnd size:"<<bound.size();
//    multimap<double, vector<float> > ray_map;
//    for(V3DLONG i = 0; i < bound.size(); i++)
//    {
//        ray_map.insert(pair<double, vector<float> > (dis[i],bound[i]));
//    }
//    //put the ordered one in bound
//    multimap<double, vector<float>>::reverse_iterator it = ray_map.rbegin();
//    bound.clear();
//    while(it!= ray_map.rend())
//    {
//        bound.push_back(it->second);
//        //qDebug()<<"dis:"<<it->first<<" xyz:"<<it->second[0]<<":"<<it->second[1]<<":"<<it->second[2];
//        it++;
//    }

    return dis;
}

double detect_fun::markerRadius_nooverlap(V3DLONG ind)
{
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord=pos2xyz(ind, y_offset, z_offset);
    V3DLONG x,y,z;
    x=coord[0];y=coord[1];z=coord[2];
    int total_num,fg_num;
    double r_grow=1.;

    while(fg_num/total_num>=0.3)
    {
        for(V3DLONG dx=MAX(x-r_grow,0); dx<=MIN(sz_image[0],x+r_grow); dx++){
            for(V3DLONG dy=MAX(y-r_grow,0); dy<=MIN(sz_image[1],y+r_grow); dy++){
                for(V3DLONG dz=MAX(z-r_grow,0); dz<=MIN(sz_image[2]-1,z+r_grow); dz++){
                    V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    if (tmp>r_grow*r_grow) continue;
                    total_num++;
                    if (mask1D[pos]==0) continue;
                    fg_num++;
                }
            }
        }
        r_grow=r_grow+0.5;
    }
    r_grow=r_grow-0.5;

    //mask the growed areas
    for(V3DLONG dx=MAX(x-r_grow,0); dx<=MIN(sz_image[0],x+r_grow); dx++){
        for(V3DLONG dy=MAX(y-r_grow,0); dy<=MIN(sz_image[1],y+r_grow); dy++){
            for(V3DLONG dz=MAX(z-r_grow,0); dz<=MIN(sz_image[2]-1,z+r_grow); dz++){

                V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                if (mask1D[pos]==0) continue;
                double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
                if (tmp>r_grow*r_grow) continue;
                mask1D[pos]=0;
            }
        }
    }
    return r_grow;
}

double detect_fun::skel_node_radius(V3DLONG ind)
{

    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    vector<V3DLONG> coord=pos2xyz(ind, y_offset, z_offset);
    float x,y,z,center_x,center_y,center_z;
    vector<float> x_all,y_all,z_all;
    vector<float> new_mass_center(3,0);
    x=(float)coord[0];y=(float)coord[1];z=(float)coord[2];
    x_all.push_back(x); y_all.push_back(y); z_all.push_back(z);
    int total_num,fg_num;
    double r_grow=1.;

    while(fg_num/total_num>=0.6)
    {
        x_all.clear();y_all.clear();z_all.clear();
        total_num=fg_num=0;
        for(V3DLONG dx=MAX(x-r_grow,0); dx<=MIN(sz_image[0],x+r_grow); dx++){
            for(V3DLONG dy=MAX(y-r_grow,0); dy<=MIN(sz_image[1],y+r_grow); dy++){
                for(V3DLONG dz=MAX(z-r_grow,0); dz<=MIN(sz_image[2]-1,z+r_grow); dz++){
                    V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    if (tmp>r_grow*r_grow) continue;
                    total_num++;
                    if (data1Dc_float[pos]<bg_thr) continue;
                    x_all.push_back(x);
                    y_all.push_back(y);
                    z_all.push_back(z);
                    fg_num++;
                }
            }
        }
        r_grow=r_grow+0.5;
//        new_mass_center=get_mass_center(x_all,y_all,z_all);
//        x=new_mass_center[0];
//        y=new_mass_center[1];
//        z=new_mass_center[2];
    }
    r_grow=r_grow-0.5;

    return r_grow;
}

V3DLONG detect_fun::extract(vector<V3DLONG>& x_all, vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                            V3DLONG seed_ind,float r_start,float stop_p)
{
    //qDebug()<<"sz_img:"<<sz_image[0]<<":"<<sz_image[1]<<":"<<sz_image[2]<<":"<<sz_image[3];
    x_all.clear();
    y_all.clear();
    z_all.clear();
    //qDebug()<<"starting radius:"<<r_start;
    vector<V3DLONG> seeds,seeds_next,seeds_pre,seeds_all;
    V3DLONG x,y,z,pos,center_x,center_y,center_z;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    memset(mask1D, 0, page_size*sizeof(unsigned char));
    vector<V3DLONG> coord;
    vector<V3DLONG> new_mass_center;
    coord=pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];y=coord[1];z=coord[2];

    //set the foreground voxels within radius start_growth as seeds
    //mask the growed areas

    for(V3DLONG dx=MAX(x-r_start,0); dx<=MIN(sz_image[0],x+r_start); dx++){
        for(V3DLONG dy=MAX(y-r_start,0); dy<=MIN(sz_image[1],y+r_start); dy++){
            for(V3DLONG dz=MAX(z-r_start,0); dz<=MIN(sz_image[2]-1,z+r_start); dz++){
                V3DLONG pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)+(dz-z)*(dz-z);
                if (tmp>r_start*r_start) continue;
                mask1D[pos]=1;
                if (data1Dc_float[pos]>bg_thr) //(skel[pos]>0)
                {
                    seeds_pre.push_back(pos);
                    seeds_all.push_back(pos);
                    seeds.push_back(pos);
                    x_all.push_back(dx);
                    y_all.push_back(dy);
                    z_all.push_back(dz);
                }
            }
        }
    }


    //qDebug()<<"seeds.size:"<<seeds_pre.size();

    float total_r_grow=1;
//    seeds.push_back(seed_ind);
//    seeds_pre.push_back(seed_ind);
//    seeds_all.push_back(seed_ind);
//    if (label[seed_ind]>0)
//    {
//        //qDebug()<<"this seed is used";
//        return 0;
//    }
//    label[seed_ind]=marker;
//    x_all.push_back(x);
//    y_all.push_back(y);
//    z_all.push_back(z);
    float r_grow=r_start+1;
    center_x=x;
    center_y=y;
    center_z=z;
    //1)Start from a sphere with radius 1, region grow several rounds inside the sphere till
    //all eligible seeds are found and stored in the seed_pre;
    //2)Three stop criteria: a) seeds_pre is empty.(No more seeds can be found) b)Among all the newly grown seeds
    // the ones on the foreground is lower than 20%. A stricter rule trying to seperate cells are closely located.
    // c) the r_grow is over 30;
    //3)if criteria satisfied,the radius of the sphere increased. repeat 1).
    //qDebug()<<"seeds_pre/total_r_grow:"<<(float)seeds_pre.size()/total_r_grow;
    while(seeds_pre.size()>0&&(float)seeds_pre.size()/total_r_grow>=stop_p)
    {
        seeds_pre.clear();
        total_r_grow=0;
        //qDebug()<<"New while break.";
        do
        {
            seeds_next.clear();
            V3DLONG sid=0;

            while(sid<seeds.size())
            {
                coord=pos2xyz(seeds[sid], y_offset, z_offset);
                x=coord[0];y=coord[1];z=coord[2];
                for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_image[0]-1,x+1); dx++){
                    for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_image[1]-1,y+1); dy++){
                        for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_image[2]-1,z+1); dz++){
                            pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                            if (mask1D[pos]>0) continue; //mask>1 visited
                            double tmp=(dx-center_x)*(dx-center_x)+(dy-center_y)*(dy-center_y)
                                 +(dz-center_z)*(dz-center_z);
                            if (tmp>r_grow*r_grow) continue;
                            total_r_grow++;
                            if (data1Dc_float[pos]<=bg_thr) //(skel[pos]<=0)
                            {
                                mask1D[pos]=1;
                                continue;
                            }
                            x_all.push_back(dx);
                            y_all.push_back(dy);
                            z_all.push_back(dz);
                            seeds_next.push_back(pos);
                            seeds_pre.push_back(pos);
                            seeds_all.push_back(pos);
                            mask1D[pos]=1;

                        }
                    }
                }
                sid++;
             }
            //qDebug()<<"seeds_next size:"<<seeds_next.size();
            seeds=seeds_next;

        }while(!seeds_next.empty());

        seeds=seeds_all; //after this round of r_grow finished, sets all seeds as the seeds for next r_grow

        if (r_grow>20)
            break;
        else    r_grow=r_grow+1;

        //Calculate the new mass center
        new_mass_center=get_mass_center(x_all,y_all,z_all);
        center_x=new_mass_center[0];
        center_y=new_mass_center[1];
        center_z=new_mass_center[2];
//        qDebug()<<"total_r_grow:"<<total_r_grow<<"sum_count"<<seeds_pre.size()
//               <<":"<<(float)seeds_pre.size()/total_r_grow;
//        qDebug()<<"r_grow"<<r_grow;
    }
    return x_all.size();
}

V3DLONG detect_fun::extract_nonoverlap(unsigned char *skel_mask,long *label,
                       vector<V3DLONG>& x_all,vector<V3DLONG>& y_all,vector<V3DLONG>& z_all,
                                       V3DLONG seed_ind,float stop_p,int marker)
{
    //qDebug()<<"sz_img:"<<sz_image[0]<<":"<<sz_image[1]<<":"<<sz_image[2]<<":"<<sz_image[3];
    x_all.clear();
    y_all.clear();
    z_all.clear();
    //qDebug()<<"starting radius:"<<r_start;
    vector<V3DLONG> seeds,seeds_next,seeds_pre,seeds_all;
    V3DLONG x,y,z,pos,center_x,center_y,center_z;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];
    //memset(mask1D, 0, page_size*sizeof(unsigned char));
    vector<V3DLONG> coord;
    vector<V3DLONG> new_mass_center;
    coord=pos2xyz(seed_ind, y_offset, z_offset);
    x=coord[0];y=coord[1];z=coord[2];

    float total_r_grow=1;
    seeds.push_back(seed_ind);
    seeds_pre.push_back(seed_ind);
    seeds_all.push_back(seed_ind);
    if (label[seed_ind]>0)
    {
        //qDebug()<<"this seed is used";
        return 0;
    }
    label[seed_ind]=marker;
    x_all.push_back(x);
    y_all.push_back(y);
    z_all.push_back(z);
    float r_grow=1;
    center_x=x;
    center_y=y;
    center_z=z;

    while(seeds_pre.size()>0&&(float)seeds_pre.size()/total_r_grow>=stop_p)
    {
        seeds_pre.clear();
        total_r_grow=0;
        //qDebug()<<"New while break.";
        do
        {
            seeds_next.clear();
            V3DLONG sid=0;

            while(sid<seeds.size())
            {
                coord=pos2xyz(seeds[sid], y_offset, z_offset);
                x=coord[0];y=coord[1];z=coord[2];
                for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_image[0]-1,x+1); dx++){
                    for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_image[1]-1,y+1); dy++){
                        for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_image[2]-1,z+1); dz++){
                            pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                            if (label[pos]>=0) continue; //label=-1 non-visited label=0 bg
                            double tmp=(dx-center_x)*(dx-center_x)+(dy-center_y)*(dy-center_y)
                                 +(dz-center_z)*(dz-center_z);
                            if (tmp>r_grow*r_grow) continue;
                            total_r_grow++;
                            if (skel_mask[pos]>0) {label[pos]=0; continue;}
                            if (data1Dc_float[pos]<=bg_thr)
                            {
                                label[pos]=0;
                                continue;
                            }
                            x_all.push_back(dx);
                            y_all.push_back(dy);
                            z_all.push_back(dz);
                            seeds_next.push_back(pos);
                            seeds_pre.push_back(pos);
                            seeds_all.push_back(pos);
                            label[pos]=marker;
                        }
                    }
                }
                sid++;
             }
            //qDebug()<<"seeds_next size:"<<seeds_next.size();
            seeds=seeds_next;

        }while(!seeds_next.empty());

        seeds=seeds_all; //after this round of r_grow finished, sets all seeds as the seeds for next r_grow

        if (r_grow>20)
            break;
        else    r_grow=r_grow+0.5;

        //Calculate the new mass center
        new_mass_center=get_mass_center(x_all,y_all,z_all);
        center_x=new_mass_center[0];
        center_y=new_mass_center[1];
        center_z=new_mass_center[2];
//        qDebug()<<"total_r_grow:"<<total_r_grow<<"sum_count"<<seeds_pre.size()
//               <<":"<<(float)seeds_pre.size()/total_r_grow;
//        qDebug()<<"r_grow"<<r_grow;
    }
    return x_all.size();
}

V3DLONG detect_fun::extract_nonsphere(unsigned char * all)

{
    qDebug()<<"In extract_nonshpere";
    memset(mask1D,0,page_size*sizeof(unsigned char));
    //qDebug()<<"sz_img:"<<sz_image[0]<<":"<<sz_image[1]<<":"<<sz_image[2]<<":"<<sz_image[3];

    //int neighbor_size=2;
    //1) Collect user specified point info
    vector<V3DLONG> seeds,seeds_next,seeds_pre,seeds_all;
    V3DLONG x,y,z,pos,center_x,center_y,center_z;
    V3DLONG y_offset=sz_image[0];
    V3DLONG z_offset=sz_image[0]*sz_image[1];

    vector<V3DLONG> coord;
    x=464.4; y=204.4; z=35.8;
    V3DLONG seed_ind=xyz2pos(x,y,z,y_offset,z_offset);
    qDebug()<<"first point value:"<<data1Dc_float[seed_ind];
    seeds.push_back(seed_ind);
    all[seed_ind]=1;

    int nsum;
    V3DLONG sid=0;
    while(sid<seeds.size())
    {
        coord=pos2xyz(seeds[sid], y_offset, z_offset);
        x=coord[0];y=coord[1];z=coord[2];
        for(V3DLONG dx=MAX(x-1,0); dx<=MIN(sz_image[0]-1,x+1); dx++){
            for(V3DLONG dy=MAX(y-1,0); dy<=MIN(sz_image[1]-1,y+1); dy++){
                for(V3DLONG dz=MAX(z-1,0); dz<=MIN(sz_image[2]-1,z+1); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    if (mask1D[pos]>0)  continue; //mask1D==1 visited mask1D==0 not visited
                    if (data1Dc_float[pos]<bg_thr) {mask1D[pos]=1; continue;}
                    all[pos]=1;
                    seeds.push_back(pos);
                    mask1D[pos]=1;

                }
            }
        }
        sid++;
    }

    return seeds.size();
}
vector<V3DLONG> get_mass_center(vector<V3DLONG> x_all, vector<V3DLONG> y_all,
                                             vector<V3DLONG> z_all)
{
        float total_x,total_y,total_z,sum_v;
        vector<V3DLONG> center(3,0);
        total_x=total_y=total_z=sum_v=0;

        for (int i=0;i<x_all.size();i++)
        {
            total_x=x_all[i]+total_x;
            total_y=y_all[i]+total_y;
            total_z=z_all[i]+total_z;

        }
            sum_v=x_all.size();
        if (x_all.size()>0) {
            center[0]=total_x/sum_v;
            center[1]=total_y/sum_v;
            center[2]=total_z/sum_v;
        }
        //qDebug()<<"center:"<<center[0]<<","<<center[1]<<","<<center[2]<<","<<center[3];
        return center;
}

void write_swc(vector<MyMarker> vp,QString filename)
{
    qDebug()<<"write marker";

    FILE * fp_1 = fopen(filename.toAscii(), "w");
    if (!fp_1)
    {
        qDebug()<<"cannot open the file to save the landmark points.\n";
        return;
    }
//    fprintf(fp_1,"#parameters: \n");
//    fprintf(fp_1,"#mean shift search window radius: %d\n",windowradius);
   fprintf(fp_1, "##n,type,x,y,z,radius,parent\n");

    for (int i=0;i<vp.size(); i++)
    {
        fprintf(fp_1, "%d %d %.lf %.lf %.lf %.1f %d\n",i+1,3,vp[i].x,vp[i].y,vp[i].z,vp[i].radius,i+1);
        //fprintf(fp_1, "%d,%d,%.lf,%.lf,%.lf,%.1f,%d\n",i+1,3,vp[i].x,vp[i].y,vp[i].z,vp[i].radius,i+1);
    }
    fclose(fp_1);
    qDebug()<<"writing finished";
}

void write_csv(vector<MyMarker> vp,QString filename)
{
    qDebug()<<"write marker";

    FILE * fp_1 = fopen(filename.toAscii(), "w");
    if (!fp_1)
    {
        qDebug()<<"cannot open the file to save the landmark points.\n";
        return;
    }
//    fprintf(fp_1,"#parameters: \n");
//    fprintf(fp_1,"#mean shift search window radius: %d\n",windowradius);
   fprintf(fp_1, "##x,y,z,radius,shape,name,comment\n");

    for (int i=0;i<vp.size(); i++)
    {
        fprintf(fp_1, "%.lf,%.lf,%.lf,%.1f,%d," "," "\n",vp[i].x,vp[i].y,vp[i].z,vp[i].radius,1);

    }
    fclose(fp_1);
    qDebug()<<"writing finished";
}

double detect_fun:: markerRadius(V3DLONG ind)
{
    long y_offset = sz_image[0];
    long z_offset = sz_image[0] * sz_image[1];
    double max_r = sz_image[0]/2;
    if (max_r > sz_image[1]/2) max_r = sz_image[1]/2;
    vector<V3DLONG> coord=pos2xyz(ind,y_offset,z_offset);
    int x=coord[0];
    int y=coord[1];
    int z=coord[2];

    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir=ir+0.5)
    {
        total_num = background_num = 0;

        double dz, dy, dx;
        double zlower = 0, zupper = 0;
        for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; ++dy)
                for (dx= -ir; dx <= +ir; ++dx)
                {
                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                    if (r>ir-1 && r<=ir)
                    {

                        V3DLONG i = x+dx;   if (i<0 || i>=sz_image[0]) goto end1;
                        V3DLONG j = y+dy;   if (j<0 || j>=sz_image[1]) goto end1;
                        V3DLONG k = z+dz;   if (k<0 || k>=sz_image[2]) goto end1;
                        total_num++;
                        if (data1Dc_float[k * z_offset + j * y_offset + i] <= bg_thr)
                        {
                            background_num++;

                            if ((background_num/total_num) > 0.7) goto end1; //change 0.01 to 0.001 on 100104
                        }
                    }
                }
    }
end1:
    return ir;

}

unsigned char * memory_allocate_uchar1D(const V3DLONG i_size)
{
    unsigned char *ptr_result;
    ptr_result=(unsigned char *) calloc(i_size, sizeof(unsigned char));
    return(ptr_result);
}

void memory_free_uchar1D(unsigned char *ptr_input)
{
    free(ptr_input);
}

float * memory_allocate_float1D(const V3DLONG i_size)
{
    float *ptr_result;
    ptr_result=(float *) calloc(i_size, sizeof(float));
    return(ptr_result);
}

void memory_free_float1D(float *ptr_input)
{
    free(ptr_input);
}

vector<V3DLONG> pos2xyz(const V3DLONG _pos_input, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    vector<V3DLONG> pos3_result (3, -1);
    pos3_result[2]=floor(_pos_input/(double)_offset_Z);
    pos3_result[1]=floor((_pos_input-pos3_result[2]*_offset_Z)/(double)_offset_Y);
    pos3_result[0]=_pos_input-pos3_result[2]*_offset_Z-pos3_result[1]*_offset_Y;
    return pos3_result;
}

V3DLONG xyz2pos(const V3DLONG _x, const V3DLONG _y, const V3DLONG _z, const V3DLONG _offset_Y, const V3DLONG _offset_Z)
{
    return _z*_offset_Z+_y*_offset_Y+_x;
}

vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
{
    vector<V3DLONG> poss_result;
    V3DLONG count_landmark=LandmarkList_input.count();
    for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
    {
        poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
    }
    return poss_result;
}

V3DLONG landMark2pos(LocationSimple Landmark_input, V3DLONG _offset_Y, V3DLONG _offset_Z)
{
    float x=0;
    float y=0;
    float z=0;
    Landmark_input.getCoord(x, y, z);
    x=MAX(x,0);
    y=MAX(y,0);
    z=MAX(z,0);
    return (xyz2pos(x-1, y-1, z-1, _offset_Y, _offset_Z));
}

void GetColorRGB(int* rgb, int idx)
{
    idx=idx+1;
    if(idx>=0){
    idx = idx > 0 ? idx % 128 : 128 - abs(idx % 128);
    int colorLib[128][3] = {
            {39,16,2}, //{55, 173, 188},
            {3, 91, 61},
            {237, 195, 97},
            {175, 178, 151},
            {245, 199, 137},
            {24, 226, 66},
            {118, 84, 193},
            {205, 205, 224},
            {22, 170, 128},
            {86, 150, 90},
            {53, 67, 25},
            {38, 82, 152},
            {55,173,188}, //{39, 16, 2},
            {197, 174, 31},
            {183, 41, 31},
            {174, 37, 15},
            {34, 4, 48},
            {18, 10, 79},
            {132, 100, 254},
            {112, 39, 198},
            {189, 22, 120},
            {75, 104, 110},
            {222, 180, 4},
            {6, 60, 153},
            {236, 85, 113},
            {25, 182, 184},
            {200, 240, 12},
            {204, 119, 71},
            {11, 201, 229},
            {198, 214, 88},
            {49, 97, 65},
            {5, 75, 61},
            {52, 89, 137},
            {32, 49, 125},
            {104, 187, 76},
            {206, 233, 195},
            {85, 183, 9},
            {157, 29, 13},
            {14, 7, 244},
            {134, 193, 150},
            {188, 81, 216},
            {126, 29, 207},
            {170, 97, 183},
            {107, 14, 149},
            {76, 104, 149},
            {80, 38, 253},
            {27, 85, 10},
            {235, 95, 252},
            {139, 144, 48},
            {55, 124, 56},
            {239, 71, 243},
            {208, 89, 6},
            {87, 98, 24},
            {247, 48, 42},
            {129, 130, 13},
            {94, 149, 254},
            {111, 177, 4},
            {39, 229, 12},
            {7, 146, 87},
            {56, 231, 174},
            {95, 102, 52},
            {61, 226, 235},
            {199, 62, 149},
            {51, 32, 175},
            {232, 191, 210},
            {57, 99, 107},
            {239, 27, 135},
            {158, 71, 50},
            {104, 92, 41},
            {228, 112, 171},
            {54, 120, 13},
            {126, 69, 174},
            {191, 100, 143},
            {187, 156, 148},
            {18, 95, 9},
            {104, 168, 147},
            {249, 113, 198},
            {145, 5, 131},
            {104, 56, 59},
            {112, 235, 81},
            {73, 93, 127},
            {207, 60, 6},
            {77, 76, 211},
            {35, 208, 220},
            {141, 5, 129},
            {182, 178, 228},
            {179, 239, 3},
            {209, 9, 132},
            {167, 192, 71},
            {154, 227, 164},
            {200, 125, 103},
            {224, 181, 211},
            {3, 24, 139},
            {218, 67, 198},
            {163, 185, 228},
            {196, 53, 11},
            {39, 183, 33},
            {56, 116, 119},
            {23, 129, 20},
            {42, 191, 85},
            {78, 209, 250},
            {247, 147, 60},
            {74, 172, 146},
            {51, 244, 86},
            {38, 84, 29},
            {197, 202, 150},
            {20, 49, 198},
            {90, 214, 34},
            {178, 49, 101},
            {86, 235, 214},
            {161, 221, 55},
            {17, 173, 136},
            {132, 65, 217},
            {210, 4, 121},
            {241, 117, 217},
            {137, 111, 6},
            {129, 224, 232},
            {73, 34, 0},
            {81, 135, 211},
            {172, 233, 193},
            {43, 246, 89},
            {153, 187, 222},
            {85, 118, 43},
            {119, 116, 33},
            {163, 229, 109},
            {45, 75, 15},
            {15, 7, 140},
            {144, 78, 192}
    };
    for(int k = 0; k < 3; k++)
        rgb[k] = colorLib[idx][k];
    }
    else{
        rgb[0]=0;
        rgb[1]=0;
        rgb[2]=0;
    }
}
