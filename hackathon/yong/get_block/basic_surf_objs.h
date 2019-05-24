/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//basic_surf_objs.h
//defining the basic types of surface objects
//by Hanchuan Peng. 2009-03-06. This is the first step to unify all basic surface object data types in different modules
//
//090605: merge with p_objectfile.h
//090706: add neuronswc and marker write functions here
//130103: add a few constructor functions for ImageMarker

#ifndef __BASIC_SURF_OBJS_H__
#define __BASIC_SURF_OBJS_H__

#include "v3d_basicdatatype.h"
#include "color_xyz.h"
#include "v3d_message.h"

#include <QtGui> // this is for QList, QString etc types
#include <QFileDialog>

// .ano linker files

struct P_ObjectFileType
{
    QStringList raw_image_file_list;
    QStringList labelfield_image_file_list;
    QStringList annotation_file_list;
    QStringList swc_file_list;
    QStringList pointcloud_file_list;
    QStringList surface_file_list;
};

bool importKeywordString2FileType(QString ss, QString vv, QString basedir, P_ObjectFileType & cc);
bool loadAnoFile(QString openFileNameLabel, P_ObjectFileType & cc);
bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc, const QStringList & commentStrList);
bool saveAnoFile(QString openFileNameLabel, const P_ObjectFileType & cc); // a convenient overloading function for case there is no comments

struct BasicSurfObj
{
    V3DLONG n;				// index
    RGBA8 color;
    bool on;
    bool selected;
    QString name;
    QString comment;
    BasicSurfObj() {n=0; color.r=color.g=color.b=color.a=255; on=true;selected=false; name=comment="";}
};

// .marker marker files
//##########################################################################################
// 090617 RZC : image marker position is 1-based to consist with LocationSimple
// ATTENTION: it is easy to be chaos in 0/1-based coordinates!!!
//##########################################################################################
struct ImageMarker : public BasicSurfObj
{
    int type;			// 0-pxUnknown, 1-pxLocaNotUseful, 2-pxLocaUseful, 3-pxLocaUnsure, 4-pxTemp
    int shape;			// 0-pxUnset, 1-pxSphere, 2-pxCube, 3-pxCircleX, 4-pxCircleY, 5-pxCircleZ,
    // 6-pxSquareX, 7-pxSquareY, 8-pxSquareZ, 9-pxLineX, 10-pxLineY, 11-pxLineZ,
    // 12-pxTriangle, 13-pxDot;
    float x, y, z;		// point coordinates
    float radius;

    operator XYZ() const { return XYZ(x, y, z); }
    ImageMarker() {type=shape=0; radius=x=y=z=0;}
    ImageMarker(float x0, float y0, float z0) {type=shape=0; x=x0; y=y0; z=z0; radius=0;}
    ImageMarker(int t0, int s0, float x0, float y0, float z0, float r0) {type=t0; shape=s0; x=x0; y=y0; z=z0; radius=r0;}
};

QList <ImageMarker> readMarker_file(const QString & filename);
bool writeMarker_file(const QString & filename, const QList <ImageMarker> & listMarker);


// .apo pointcloud files

struct CellAPO  : public BasicSurfObj
{
    float x, y, z;		// point coordinates
    float intensity;
    float sdev, pixmax, mass;
    float volsize;		// volume size
    QString orderinfo;

    operator XYZ() const { return XYZ(x, y, z); }
    CellAPO() {x=y=z=intensity=volsize=sdev=pixmax=mass=0; orderinfo="";}
};

QList <CellAPO> readAPO_file(const QString& filename);
bool writeAPO_file(const QString& filename, const QList <CellAPO> & listCell);

// .swc neurons and other graph-describing files

struct NeuronSWC : public BasicSurfObj
{
    int type;			// 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite, 4-Apical_dendrite, 5-Fork_point, 6-End_point, 7-Custom
    float x, y, z;		// point coordinates

    union{
    float r;			// radius
    float radius;
    };

    union{
    V3DLONG pn;				// previous point index (-1 for the first point)
    V3DLONG parent;				// previous point index (-1 for the first point)
    };

    V3DLONG level; //20120217, by PHC. for ESWC format
    QList<float> fea_val; //20120217, by PHC. for ESWC format

    V3DLONG seg_id; //this is reused for ESWC format, 20120217, by PHC
    V3DLONG nodeinseg_id; //090925, 091027: for segment editing

    operator XYZ() const { return XYZ(x, y, z); }
    NeuronSWC () {n=type=pn=0; x=y=z=r=0; seg_id=-1; nodeinseg_id=0; fea_val=QList<float>(); level=-1;}
};

// .v3ds label surfaces

struct LabelSurf : public BasicSurfObj
{
    int label;			// label
    int label2;			// label2 (range from label to label2)

    operator int() const { return label; }
    LabelSurf() {label=label2=0;}
};

// .neuron trees

struct NeuronTree : public BasicSurfObj
{
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    QString file;
    bool editable;
    int linemode; //local control if a neuron will displayed as line or tube mode(s). by PHC 20130926

    NeuronTree()
    {
        listNeuron.clear(); hashNeuron.clear(); file=""; editable=false;
        linemode=-1; //-1 is no defined. 0 is NO, and 1 is yes
    }

    void deepCopy(const NeuronTree p)
    {
        n=p.n; color=p.color; on=p.on;
        selected=p.selected; name=p.name; comment=p.comment;

        file     = p.file;
        editable = p.editable;
        linemode = p.linemode;
        listNeuron.clear();
        hashNeuron.clear();

        for (int i =0; i< p.listNeuron.size();i++){
            NeuronSWC S;
            S.n = p.listNeuron[i].n;
            S.type = p.listNeuron[i].type;
            S.x = p.listNeuron[i].x;
            S.y= p.listNeuron[i].y;
            S.z = p.listNeuron[i].z;
            S.r = p.listNeuron[i].r;
            S.pn = p.listNeuron[i].pn;
            S.seg_id = p.listNeuron[i].seg_id;
            S.fea_val = p.listNeuron[i].fea_val;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }





    void copy(const NeuronTree & p)
    {
        n=p.n; color=p.color; on=p.on; selected=p.selected; name=p.name; comment=p.comment;
        listNeuron = p.listNeuron;
        hashNeuron = p.hashNeuron;
        file     = p.file;
        editable = p.editable;
        linemode = p.linemode;
    }
    void copyGeometry(const NeuronTree & p)
    {
        if (p.listNeuron.size()!=listNeuron.size()) return;

        NeuronSWC *p_tmp;
        for (V3DLONG i=0;i<listNeuron.size();i++)
        {
            p_tmp = (NeuronSWC *)(&(listNeuron.at(i)));
            //qDebug()<<"before:"<<p_tmp->x<<p_tmp->y<<p_tmp->z<<p_tmp->r;
            p_tmp->x = p.listNeuron.at(i).x;
            p_tmp->y = p.listNeuron.at(i).y;
            p_tmp->z = p.listNeuron.at(i).z;
            p_tmp->r = p.listNeuron.at(i).r;
            //qDebug()<<"src:"<<p.listNeuron.at(i).x<<p.listNeuron.at(i).y<<p.listNeuron.at(i).z<<p.listNeuron.at(i).r;
            //qDebug()<<"after:"<<p_tmp->x<<p_tmp->y<<p_tmp->z<<p_tmp->r;
        }
    }
    bool projection(int axiscode=3) //axiscode, 1 -- x, 2 -- y, 3 -- z, 4 -- r
    {
        if (axiscode!=1 && axiscode!=2 && axiscode!=3 && axiscode!=4) return false;
        NeuronSWC *p_tmp;
        for (V3DLONG i=0;i<listNeuron.size();i++)
        {
            p_tmp = (NeuronSWC *)(&(listNeuron.at(i)));
            //qDebug()<<"before:"<<p_tmp->x<<p_tmp->y<<p_tmp->z<<p_tmp->r;
            if (axiscode==1) p_tmp->x = 0;
            else if (axiscode==2) p_tmp->y = 0;
            else if (axiscode==3) p_tmp->z = 0;
            else if (axiscode==4) p_tmp->r = 0.5;
        }
        return true;
    }
};

NeuronTree readSWC_file(const QString& filename);
//bool writeSWC_file(const QString& filename, const NeuronTree& nt);
bool writeSWC_file(const QString& filename, const NeuronTree& nt, const QStringList *infostring=0);
bool writeESWC_file(const QString& filename, const NeuronTree& nt);

//general operators

inline bool operator==(ImageMarker& a, ImageMarker& b)
{
    return XYZ(a)==XYZ(b);
}

inline bool operator==(CellAPO& a, CellAPO& b)
{
    return XYZ(a)==XYZ(b);
}

inline bool operator==(NeuronSWC& a, NeuronSWC& b)
{
    return XYZ(a)==XYZ(b);
}

inline bool operator==(LabelSurf& a, LabelSurf& b) // for test of contains
{
    return int(a)==int(b);
}
inline bool operator<(LabelSurf& a, LabelSurf& b)  // for test of sort
{
    return int(a)<int(b);
}
inline bool operator>(LabelSurf& a, LabelSurf& b)  // for test of sort
{
    return int(a)>int(b);
}

inline bool operator==(NeuronTree& a, NeuronTree& b)
{
    return QString::compare(a.file, b.file, Qt::CaseSensitive)==0;
}


#endif

