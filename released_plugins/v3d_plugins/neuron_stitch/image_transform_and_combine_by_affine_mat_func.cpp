// by Hanbo Chen
//2015-03-18
#include <stdio.h>
#include <math.h>
#include <QtGui>
#include <fstream>
#include "neuron_stitch_func.h"

#define MAX(a,b) (a)>(b)?(a):(b)
#define MIN(a,b) (a)<(b)?(a):(b)

using namespace std;
//amat ind after reshape:
//[0 1 2 3
//4 5 6 7
//8 9 10 11
//12 13 14 15]
template <class A, class B>
void transformCoord(const A coord_in[3], B coord_out[3], double amat[16])
{
    coord_out[0]=(B)(amat[0]*(double)coord_in[0]+amat[1]*(double)coord_in[1]+amat[2]*(double)coord_in[2]+amat[3]);
    coord_out[1]=(B)(amat[4]*(double)coord_in[0]+amat[5]*(double)coord_in[1]+amat[6]*(double)coord_in[2]+amat[7]);
    coord_out[2]=(B)(amat[8]*(double)coord_in[0]+amat[9]*(double)coord_in[1]+amat[10]*(double)coord_in[2]+amat[11]);
}

//affine transform image based on given affine matrix
template <class T>
bool affineTransformImage(T * p_img_in,const V3DLONG sz_in[4], T * p_img_out,const V3DLONG sz_out[4], const double amat[16], const V3DLONG globalShift[3], const int i_interpmethod_img)
{
    //memset(p_img_out,0,sz_out[0]*sz_out[1]*sz_out[2]*sz_in[3]*sizeof(T));

    //calculate inverse affine mat
    double amat_inv[16];
    inverseAmat(amat, amat_inv);

    printf("Performing Affine Transform\n");

    //calculate affine
    for(V3DLONG z=0;z<sz_out[2];z++){
        for(V3DLONG y=0;y<sz_out[1];y++){
            printf("\r%2.3f%%",(float)z/(float)sz_out[2]*100);
            for(V3DLONG x=0;x<sz_out[0];x++)
            {
                V3DLONG pos_warp[3];
                pos_warp[0]=x-globalShift[0];
                pos_warp[1]=y-globalShift[1];
                pos_warp[2]=z-globalShift[2];

                double pos_sub[3];
                transformCoord(pos_warp, pos_sub, amat_inv);
                if(pos_sub[0]<0 || pos_sub[0]>sz_in[0]-1 ||
                   pos_sub[1]<0 || pos_sub[1]>sz_in[1]-1 ||
                   pos_sub[2]<0 || pos_sub[2]>sz_in[2]-1)
                {
                    continue;
                }


                //nearest neighbor interpolate
                if(i_interpmethod_img==1)
                {
                    V3DLONG pos_sub_nn[3];
                    for(int i=0;i<3;i++)
                    {
                        pos_sub_nn[i]=pos_sub[i]+0.5;
                        pos_sub_nn[i]=pos_sub_nn[i]<sz_in[i]?pos_sub_nn[i]:sz_in[i]-1;
                    }
                    for(V3DLONG c=0;c<sz_in[3];c++){
                        V3DLONG ind=x+y*sz_out[0]+z*sz_out[1]*sz_out[0]+c*sz_out[0]*sz_out[1]*sz_out[2];
                        V3DLONG ind_sub=pos_sub_nn[0]+pos_sub_nn[1]*sz_in[0]+
                                pos_sub_nn[2]*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        p_img_out[ind]=p_img_in[ind_sub];
                    }
                }
                //linear interpolate
                else if(i_interpmethod_img==0)
                {
                    //find 8 neighor pixels boundary
                    V3DLONG x_s,x_b,y_s,y_b,z_s,z_b;
                    x_s=floor(pos_sub[0]);		x_b=ceil(pos_sub[0]);
                    y_s=floor(pos_sub[1]);		y_b=ceil(pos_sub[1]);
                    z_s=floor(pos_sub[2]);		z_b=ceil(pos_sub[2]);

                    //compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
                    double l_w,r_w,t_w,b_w;
                    l_w=1.0-(pos_sub[0]-x_s);	r_w=1.0-l_w;
                    t_w=1.0-(pos_sub[1]-y_s);	b_w=1.0-t_w;
                    //compute weight for higer slice and lower slice
                    double u_w,d_w;
                    u_w=1.0-(pos_sub[2]-z_s);	d_w=1.0-u_w;

                    //linear interpolate each channel
                    for(V3DLONG c=0;c<sz_in[3];c++)
                    {
                        //linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
                        double higher_slice;
                        V3DLONG ind_1=x_s+y_s*sz_in[0]+z_s*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        V3DLONG ind_2=x_b+y_s*sz_in[0]+z_s*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        V3DLONG ind_3=x_s+y_b*sz_in[0]+z_s*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        V3DLONG ind_4=x_b+y_b*sz_in[0]+z_s*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        higher_slice=t_w*(l_w*p_img_in[ind_1]+r_w*p_img_in[ind_2])+
                                     b_w*(l_w*p_img_in[ind_3]+r_w*p_img_in[ind_4]);
                        //linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
                        double lower_slice;
                        ind_1=x_s+y_s*sz_in[0]+z_b*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        ind_2=x_b+y_s*sz_in[0]+z_b*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        ind_3=x_s+y_b*sz_in[0]+z_b*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        ind_4=x_b+y_b*sz_in[0]+z_b*sz_in[1]*sz_in[0]+c*sz_in[0]*sz_in[1]*sz_in[2];
                        lower_slice =t_w*(l_w*p_img_in[ind_1]+r_w*p_img_in[ind_2])+
                                     b_w*(l_w*p_img_in[ind_3]+r_w*p_img_in[ind_4]);
                        //linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
                        T intval=(T)(u_w*higher_slice+d_w*lower_slice+0.5);
                        V3DLONG ind=x+y*sz_out[0]+z*sz_out[1]*sz_out[0]+c*sz_out[0]*sz_out[1]*sz_out[2];
                        p_img_out[ind]=intval;
                    }
                }

            }
        }
    }

    printf("\rAffine Transform Done!\n");

    return true;
}


//affine transform image based on given affine matrix and combine the image
//p_img_out will be initialized in the program
template <class T>
bool affineCombineImage(T * p_img_tar,const V3DLONG sz_tar[4],T * p_img_sub,const V3DLONG sz_sub[4], T * &p_img_out, V3DLONG sz_out[4], V3DLONG globalShift[3], const double amat[16], const int i_interpmethod_img, const bool b_channelSeperate, const bool b_negativeShift)
{
    //estimate output image size and initialize memory
    if(p_img_out!=0){
        delete[] p_img_out;
    }
    sz_out[0]=sz_tar[0];
    sz_out[1]=sz_tar[1];
    sz_out[2]=sz_tar[2];
    if(b_channelSeperate){
        sz_out[3]=sz_tar[3]+sz_sub[3];
    }else{
        sz_out[3]=MAX(sz_tar[3],sz_sub[3]);
    }
    QList<V3DLONG> box_x, box_y, box_z;
    box_x.append(0); box_x.append(sz_sub[0]);
    box_y.append(1); box_y.append(sz_sub[1]);
    box_z.append(2); box_z.append(sz_sub[2]);
    V3DLONG negativeShift[3]={0,0,0};
    for(V3DLONG ix=0; ix<2; ix++)
        for(V3DLONG iy=0; iy<2; iy++)
            for(V3DLONG iz=0; iz<2; iz++){
                V3DLONG newx=amat[0]*box_x[ix]+amat[1]*box_y[iy]+amat[2]*box_z[iz]+amat[3];
                V3DLONG newy=amat[4]*box_x[ix]+amat[5]*box_y[iy]+amat[6]*box_z[iz]+amat[7];
                V3DLONG newz=amat[8]*box_x[ix]+amat[9]*box_y[iy]+amat[10]*box_z[iz]+amat[11];
                negativeShift[0]=MAX(-newx,negativeShift[0]);
                negativeShift[1]=MAX(-newy,negativeShift[1]);
                negativeShift[2]=MAX(-newz,negativeShift[2]);
                sz_out[0]=MAX(newx,sz_out[0]);
                sz_out[1]=MAX(newy,sz_out[1]);
                sz_out[2]=MAX(newz,sz_out[2]);
            }

    qDebug()<<"Target image size: "<<sz_tar[0]<<":"<<sz_tar[1]<<":"<<sz_tar[2]<<":"<<sz_tar[3];
    qDebug()<<"Subject image size: "<<sz_sub[0]<<":"<<sz_sub[1]<<":"<<sz_sub[2]<<":"<<sz_sub[3];
    qDebug()<<"Estimated global shifting: "<<negativeShift[0]<<":"<<negativeShift[1]<<":"<<negativeShift[2];
    if(!b_negativeShift){
        if(negativeShift[0]>0 || negativeShift[1]>0 || negativeShift[2]>0)
            qDebug()<<"warning: part of the subject image will have negative index after affine. The negative part will be removed.";
        globalShift[0]=globalShift[1]=globalShift[2]=0;
    }else{
        sz_out[0]+=negativeShift[0];
        sz_out[1]+=negativeShift[1];
        sz_out[2]+=negativeShift[2];
        globalShift[0]=negativeShift[0];
        globalShift[1]=negativeShift[1];
        globalShift[2]=negativeShift[2];
    }
    qDebug()<<"Estimated new image size: "<<sz_out[0]<<":"<<sz_out[1]<<":"<<sz_out[2]<<":"<<sz_out[3];
    qDebug()<<"Allocating and initializing memory:";
    p_img_out = new T[sz_out[0]*sz_out[1]*sz_out[2]*sz_out[3]];
    memset(p_img_out, 0, sz_out[0]*sz_out[1]*sz_out[2]*sz_out[3]*sizeof(T));

    //affine transform subject image
    if(b_channelSeperate){
        if(!affineTransformImage(p_img_sub,sz_sub,p_img_out+sz_out[0]*sz_out[1]*sz_out[2]*sz_tar[3],sz_out,amat,globalShift,i_interpmethod_img) ){

        }
    }else{
        if(!affineTransformImage(p_img_sub,sz_sub,p_img_out,sz_out,amat,globalShift,i_interpmethod_img) ){

        }
    }

    //copy the target image to output
    qDebug()<<"copying target image";
    for(V3DLONG x=0; x<sz_tar[0]; x++){
        printf("\r%2.3f%%",(float)x/(float)sz_tar[0]*100);
        for(V3DLONG y=0; y<sz_tar[1]; y++){
            for(V3DLONG z=0; z<sz_tar[2]; z++){
                V3DLONG xwarp=x+globalShift[0];
                V3DLONG ywarp=y+globalShift[1];
                V3DLONG zwarp=z+globalShift[2];
                if(xwarp < 0 || xwarp >= sz_out[0] ||
                        ywarp < 0 || ywarp >= sz_out[1] ||
                        zwarp < 0 || zwarp >= sz_out[2]){
                    continue;
                }
                for(V3DLONG c=0; c<sz_tar[3]; c++){
                    V3DLONG ind_tar=x+y*sz_tar[0]+z*sz_tar[1]*sz_tar[0]+c*sz_tar[0]*sz_tar[1]*sz_tar[2];
                    V3DLONG ind_warp=xwarp+ywarp*sz_out[0]+zwarp*sz_out[1]*sz_out[0]+c*sz_out[0]*sz_out[1]*sz_out[2];
                    if(b_channelSeperate)
                        p_img_out[ind_warp]=p_img_tar[ind_tar];
                    else
                        p_img_out[ind_warp]=MAX(p_img_tar[ind_tar],p_img_out[ind_warp]);
                }
            }
        }
    }

    qDebug()<<"\r100% done!";
    return true;
}
