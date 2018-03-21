#include "get_sample_area.h"
#include <v3d_interface.h>
#include <node.h>
#include <graph.h>
#include "data_training.h"
#include <vector>
struct Center
{
    V3DLONG x;
    V3DLONG y;
    V3DLONG z;
};
bool get_sample_area(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output,QWidget *parent)
{
    vector<char*>* inlist = (vector<char*>*) (input.at(0).p);
    if (inlist->size() != 1)
    {
        cerr<<"You must specify 1 input file!"<<endl;
        return false;
    }
    char * infile = inlist->at(0);
    cout<<"input file: "<<infile<<endl;
    unsigned char * inimg1d = NULL;
    V3DLONG sz[4];
    int datatype;
    if (!simple_loadimage_wrapper(callback, infile, inimg1d, sz, datatype))
    {
        cerr<<"faile to load image"<<endl;
        return false;
    }

    
}
bool choose_cube(unsigned char* img1d,double thre,V3DLONG in_sz[4],unsigned char bresh, QMap<V3DLONG,Graph<Node*>*> &nodeMap,double m=0,double n1=0)
{
    double sz_total = in_sz[0]*in_sz[1]*in_sz[2];
    double per;
    for(V3DLONG i = 0; i < sz_total; i++)//把所有点选出来
    {
        if(img1d[i] > bresh)
        {
            Graph<Node*>* graphNode = new Graph<Node*>(new Node(GET_X(i),GET_Y(i),GET_Z(i)));
            nodeMap[i] = graphNode;
            n1=n1+1;
            m=m+img1d[i];
        }
    }
    per = n1/sz_total;
    if(per>= thre)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool get_subimg(unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback)
{
    QMap<V3DLONG,Graph<Node*>*> &nodeMap;
//    double m;
//    double n;
    V3DLONG im_cropped_sz[4];
    V3DLONG pagesz;
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG xe,xb,ye,yb,ze,zb;
    int lens=3;
    int num_cube = (M*N*P)/((2*lens+1)*(2*lens+1)*(2*lens+1));
    int num_x = M/(2*lens+1);
    int num_y = N/(2*lens+1);
    int num_z =  P/(2*lens+1);

    double thre = 0.25;
    unsigned char bresh =30;


    //unsigned char *im_cropped = 0;
    cout<<"img_size = "<<num_cube<<endl;
    Center center;
    for(V3DLONG i =0;i<num_z;i++)
    {
        for(V3DLONG j =0;j<num_y;j++)
        {
            for(V3DLONG k =0;k<num_x;k++)
            {

                unsigned char *im_cropped = 0;
                center.x = lens+k*(2*lens+1);
                center.y = lens+j*(2*lens+1);
                center.z = lens+i*(2*lens+1);
                xe=center.x+lens;
                xb=center.x-lens;
                ye=center.y+lens;
                yb=center.y-lens;
                ze=center.z+lens;
                zb=center.z-lens;


                if(xb<0) xb = 0;
                if(xe>=N-1) xe = N-1;
                if(yb<0) yb = 0;
                if(ye>=M-1) ye = M-1;
                if(zb<0) zb = 0;
                if(ze>=N-1) ze = P-1;

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
                            if(choose_cube(im_cropped,thre,im_cropped_sz,bresh,nodeMap))
                            {
                                get_feature();
                            }
                            j++;
                        }
                    }
                }
            }
        }
    }
}
bool get_feature(V3DPluginCallback2 & callback,unsigned char * inimg1d,V3DLONG sz[4],QVector<QVector<int> > &hist_vec,vector<double> &entrople)
{
    QVector<int> tmp;
    double max_value = 256;
    V3DLONG histscale = 256;


    int nChannel = sz[3];
    tmp.clear();
    for (int c=0;c<nChannel;c++)
    {

        getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
        hist_vec.append(tmp);
    }

    //output histogram to csv file
    //    FILE *fp;
    //    fp = fopen(outfile, "w");
    //    cout<<"hang = "<<hist_vec.size()<<endl;
    //    cout<<"lie = "<<hist_vec[0].size()<<endl;
    //    for (int i=0;i<hist_vec.size();i++)
    //    {

    //        for (int j=0;j<hist_vec[i].size();j++)
    //            fprintf(fp, "%d,", hist_vec[i][j]);
    //        fprintf(fp,"\n");
    //    }
    //    fclose(fp);


    cout<<"******************************calculate entropie*****************************"<<endl;

    vector<double> sum_entr;
    double sum_temp;
    //vector<vector<double> > p_sum;
    //vector<double> p_each;
    double p;
    //double entro_each;
    vector<int> count_all;

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {
        sum_temp=0;
        int  count=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            sum_temp = hist_vec[i][j]+sum_temp;
            if(hist_vec[i][j]!=0)
            {
                count++;
            }
        }
        //cout<<"sum_temp = "<<sum_temp<<endl;
        sum_entr.push_back(sum_temp);
        count_all.push_back(count);
    }

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {
        //cout<<"count_all = "<<count_all[i]<<endl;
        sum_temp=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            double tmp_=hist_vec[i][j];
            //cout<<"count_all = "<<count_all[i]<<endl;
            p=tmp_/count_all[i];

            if(p!=0)
            {
                //cout<<"p = "<<p<<endl;
                sum_temp = sum_temp - p*log2(p);
            }
        }
        entrople.push_back(sum_temp);
    }


    if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
}
