#include "get_sample_area.h"
#include <v3d_interface.h>
#include <node.h>
#include <graph.h>
#include "data_training.h"
#include <vector>



#define GET_IND(x,y,z) (x) + (y) * (x) + (z) * (y)*(x)
#define GET_X(ind,sz_x) ind % sz_x
#define GET_Y(ind,sz_x,sz_y) (ind / sz_x) % sz_y
#define GET_Z(ind,sz_xy,sz_z) (ind / sz_xy) % sz_z

struct Center
{
    V3DLONG x;
    V3DLONG y;
    V3DLONG z;
};
bool get_sample_area(V3DPluginCallback2 &callback,QWidget *parent)
{
    v3dhandle curwin;
    vector<double> entrople;
    curwin=callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
        return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);
    if (p4DImage->getDatatype()!=V3D_UINT8)
    {
        v3d_msg("Now we only support 8 bit image.\n");
        return -1;
    }
    int nChannel = p4DImage->getCDim();
    V3DLONG sz[4];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = nChannel;

    unsigned char * inimg1d = p4DImage->getRawDataAtChannel(nChannel);
    get_subimg(inimg1d,sz,callback,entrople);
    cout<<"ent.size = "<<entrople.size()<<endl;
}
bool get_sample_area(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output,QWidget *parent)
{
    vector<char*>* inlist = (vector<char*>*) (input.at(0).p);
    if (inlist->size() != 1)
    {
        cerr<<"You must specify 1 input file!"<<endl;
        return false;
    }
    string infile = inlist->at(0);
    cout<<"input file: "<<infile<<endl;
    unsigned char * inimg1d = 0;
    V3DLONG sz[4];
    vector<double> entrople;
    int datatype;
    if (!simple_loadimage_wrapper(callback, (char *)infile.c_str(), inimg1d, sz, datatype))
    {
        cerr<<"faile to load image"<<endl;
        return false;
    }

    get_subimg(inimg1d,sz,callback,entrople);


    cout<<"ent.size = "<<entrople.size()<<endl;

    
}
bool choose_cube(unsigned char* img1d,double thre,V3DLONG in_sz[4],unsigned char bresh, QMap<V3DLONG,Graph<Node*>*> &nodeMap)
{
    double m=0;
    double n1=0;
    double sz_total = in_sz[0]*in_sz[1]*in_sz[2];
    double per;
    for(V3DLONG i = 0; i < sz_total; i++)
    {
        if(img1d[i] > bresh)
        {
            Graph<Node*>* graphNode = new Graph<Node*>(new Node(GET_X(i,in_sz[0]),GET_Y(i,in_sz[0],in_sz[1]),GET_Z(i,in_sz[0*in_sz[1]],in_sz[2])));
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
bool get_subimg(unsigned char * data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback,vector<double> &entrople)
{
    QMap<V3DLONG,Graph<Node*>*> nodeMap;
    QVector<QVector<int> > hist_vec;
    V3DLONG l;
    V3DLONG im_cropped_sz[4];
    V3DLONG pagesz;
    V3DLONG M = in_sz[0];
    V3DLONG N = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG xe,xb,ye,yb,ze,zb;
    int lens=3;
    double L = (2*lens+1);
    int num_cube = (M*N*P)/((2*lens+1)*(2*lens+1)*(2*lens+1));
    im_cropped_sz[0] = L;
    im_cropped_sz[1] = L;
    im_cropped_sz[2] = L;
    im_cropped_sz[3] = sc;

    double thre = 0.25;
    unsigned char bresh = 30;
    double ratio = 0.333;

    double para = L*ratio;
    cout<<"para = "<<para<<endl;
    int num_x = M/para;
    int num_y = N/para;
    int num_z =  P/para;
    unsigned char *im_cropped;
    Center center;

    pagesz = (2*lens+1)* (2*lens+1)* (2*lens+1)*sc;
    for(V3DLONG i =0;i<num_x;i++)
    {
        for(V3DLONG j =0;j<num_y;j++)
        {
            for(V3DLONG k =0;k<num_z;k++)
            {
                im_cropped = 0;
                center.x = lens+i*para;
                center.y = lens+j*para;
                center.z = lens+k*para;
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
                if(ze>=P-1) ze = P-1;

                try {im_cropped = new unsigned char [pagesz];}
                catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
                l = 0;

                for(V3DLONG iz = zb; iz <= ze; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = yb; iy <= ye; iy++)
                    {
                        V3DLONG offsetj = iy*N;

                        for(V3DLONG ix = xb; ix <= xe; ix++)
                        {
                                im_cropped[l] = data1d[offsetk + offsetj + ix];
                                l++;                                         
                        }
                    }
                }
                if(choose_cube(im_cropped,thre,im_cropped_sz,bresh,nodeMap))
                {
                    get_feature(callback,im_cropped,im_cropped_sz,hist_vec,entrople);
                }

                hist_vec.clear();
            }
        }
    }

    if (im_cropped) {delete []im_cropped,im_cropped=NULL;}
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
        getHistogram(inimg1d, sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
        hist_vec.append(tmp);
    }
    vector<double> sum_entr;
    double sum_temp;
    double p;
    vector<int> count_all;

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {
        sum_temp=0;
        int count=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            sum_temp = hist_vec[i][j]+sum_temp;
            if(hist_vec[i][j]!=0)
            {
                count++;
            }
        }
        sum_entr.push_back(sum_temp);
        count_all.push_back(count);
    }

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {

        sum_temp=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            double tmp_=hist_vec[i][j];
            p=tmp_/sum_entr[i];
            if(p!=0)
            {
                sum_temp = sum_temp + p*log2(p);
            }
        }
        entrople.push_back(-sum_temp);
    }

}
