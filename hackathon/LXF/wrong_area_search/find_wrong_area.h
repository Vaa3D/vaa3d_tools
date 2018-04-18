#ifndef FIND_WRONG_AREA_H
#define FIND_WRONG_AREA_H
#include"sort_swc.h"
#include"basic_surf_objs.h"
#include "my_surf_objs.h"
#include "neuron_tree_align.h"
#include<v3d_interface.h>
#include<vector>
struct Input_para
{
    QString filename1;
    QString filename2;
    QString filename3;
    V3DLONG para1=3;     //resample para
    V3DLONG para2=5;        //lens para
    V3DLONG para3=1;        //step para
    V3DLONG para4=0;        //prune para
    bool model1 = 1;       //1 for.v3dpbd,0 for terafly
    int model2 = 1;        //2 for choose piont_xy ,1 for choose point_xyz ,0 for local alignment
    V3DLONG model3 = 0;      //0 for 1 class.1 for 2 class,2 for 3 class
};

class Image3DSimple
{
protected:
    unsigned char * data1d;
    V3DLONG sz0;
    V3DLONG sz1;
    V3DLONG sz2;
    V3DLONG sz3;
    V3DLONG sz_time;
    TimePackType timepacktype;
    ImagePixelType datatype;
    char imgSrcFile[1024]; //the full file path
    int b_error;
    double rez_x, rez_y, rez_z; //the resolution of a image pixel along the 3 axes
    double origin_x, origin_y, origin_z; //the "true" orgin of an image, in term of the physical units (not pixels) using resolution information

    V3DLONG valid_zslicenum; //indicate how many zslices are usable. This can be used by a plugin program to stream read data
    V3DLONG prevalid_zslicenum; //indicate previous valid slices loaded before update GUI
    void * p_customStruct; //a convenient pointer to pass back and forth some useful parameter information for a plugin

    void setError( int v ) {b_error = v;}

private:

public:
    Image3DSimple()
    {
        data1d = 0;
        sz0 = sz1 = sz2 = sz3 = 0;
        sz_time = 0;
        datatype = V3D_UNKNOWN;
        timepacktype = TIME_PACK_NONE;
        imgSrcFile[0] = '\0';
        b_error = 0;
        rez_x = rez_y = rez_z = 1;

        origin_x = origin_y = origin_z = 0;
        p_customStruct = 0;

        valid_zslicenum = 0;
    }
    virtual ~Image3DSimple()
    {
        cleanExistData();
    }
    void setXDim(V3DLONG v) {sz0=v;}
    void setYDim(V3DLONG v) {sz1=v;}
    void setCDim(V3DLONG v) {sz2=v;}
    void setZDim(V3DLONG v) {sz3=v;}
    void setTDim(V3DLONG v) {sz_time=v;}
    void setDatatype(ImagePixelType v) {datatype=v;}
    void deleteRawDataAndSetPointerToNull() { if (data1d) {delete []data1d; data1d = 0;} }
    bool setNewRawDataPointer(unsigned char *p) {if (!p) return false; if (data1d) delete []data1d; data1d = p; return true;}
    virtual void cleanExistData()
    {
        this->deleteRawDataAndSetPointerToNull();
        sz0 = sz1 = sz2 =sz3= 0;
        sz_time = 0;
        datatype = V3D_UNKNOWN;
        timepacktype = TIME_PACK_NONE;
        imgSrcFile[0] = '\0';
        b_error = 0;
        rez_x = rez_y = rez_z = 1;
        valid_zslicenum = 0;
    }
    bool setData(unsigned char *p, V3DLONG s0, V3DLONG s1, V3DLONG s2, V3DLONG s3,ImagePixelType dt)
    {
        if (p && s0>0 && s1>0 && s2>0 && (dt==V3D_UINT8 || dt==V3D_UINT16 || dt==V3D_FLOAT32))
            if (setNewRawDataPointer(p))
            {
                setXDim(s0);
                setYDim(s1);
                setCDim(s2);
                setZDim(s3);
                setDatatype(dt);
                return true;
            }
        return false;
    }
};


bool export_2dtif(V3DPluginCallback & cb,const char * filename, unsigned char * pdata, V3DLONG sz[3], int datatype);

bool sort_with_standard(QList<NeuronSWC> & neuron1, QList<NeuronSWC>  & neuron2,QList<NeuronSWC> &result);
vector<MyMarker*> nt2mm(QList<NeuronSWC> & inswc, QString fileSaveName);
bool export_neuronList2file(QList<NeuronSWC> & lN, QString fileSaveName);
void SplitString(const string& s, vector<string>& v, const string& c);
bool find_wrong_area(Input_para &PARA,V3DPluginCallback2 &callback,bool bmenu,QWidget *parent);
QList<NeuronSWC> choose_alignment(QList<NeuronSWC> &neuron,QList<NeuronSWC> &gold,double thres1,double thres2);
NeuronTree mm2nt(vector<MyMarker*> & inswc, QString fileSaveName);
double calculate_diameter(NeuronTree nt, vector<V3DLONG> branches);
bool prune_branch(NeuronTree &nt, NeuronTree & result, double prune_size);
bool export_TXT(QVector<QVector<int> > &img_vec,QString fileSaveName);
bool export_1dtxt(unsigned char *im_cropped ,QString fileSaveName);

QList<NeuronSWC> match_point(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2);
NeuronTree match_point_nt(QList<NeuronSWC> &swc1,QList<NeuronSWC> &swc2);



#endif // FIND_WRONG_AREA_H
