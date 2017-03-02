/* mapping3D_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "mapping3D_swc_plugin.h"
#include "openSWCDialog.h"
#include "../../../released_plugins/v3d_plugins/bigneuron_zz_neurontracing_TReMAP/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/bigneuron_zz_neurontracing_TReMAP/smooth_curve.h"
#include "../../../released_plugins/v3d_plugins/bigneuron_zz_neurontracing_TReMAP/fastmarching_linker.h"

#include "../APP2_large_scale/readRawfile_func.h"


using namespace std;
Q_EXPORT_PLUGIN2(mapping3D_swc, mapping3D_swc);
 
QStringList mapping3D_swc::menulist() const
{
	return QStringList() 
		<<tr("mapping")
        <<tr("mapping_markers")
		<<tr("about");
}

QStringList mapping3D_swc::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}
struct Point;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};


typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

bool map3Dfunc(NeuronTree nt,unsigned char * &data1d, V3DLONG N, V3DLONG M,V3DLONG P,vector<MyMarker*> & outswc_final);
bool map3Dfunc_raw(NeuronTree nt,string &image_name,vector<MyMarker*> & outswc_final);


void mapping3D_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("mapping"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        unsigned char* data1d = p4DImage->getRawData();

        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
        QString image_name = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

      //  int mip_plane = 0;

        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        NeuronTree nt = openDlg->nt;
        vector<MyMarker*> outswc_final;
        if(map3Dfunc(nt, data1d, N,M,P,outswc_final))
        {

                QString final_swc = openDlg->file_name + "_3D.swc"; ;
                saveSWC_file(final_swc.toStdString(), outswc_final);
                v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(final_swc.toStdString().c_str()));
         }

    }

//        V3DLONG siz = nt.listNeuron.size();
//        Tree tree;
//        for (V3DLONG i=0;i<siz;i++)
//        {
//            NeuronSWC s = nt.listNeuron[i];
//            Point* pt = new Point;
//            pt->x = s.x;
//            pt->y = s.y;
//            pt->z = s.z;
//            pt->r = s.r;
//            pt ->type = s.type;
//            pt->p = NULL;
//            pt->childNum = 0;
//            tree.push_back(pt);
//        }
//        for (V3DLONG i=0;i<siz;i++)
//        {
//            if (nt.listNeuron[i].pn<0) continue;
//            V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
//            tree[i]->p = tree[pid];
//            tree[pid]->childNum++;
//        }

//        vector<Segment*> seg_list;
//        for (V3DLONG i=0;i<siz;i++)
//        {
//            if (tree[i]->childNum!=1)//tip or branch point
//            {
//                Segment* seg = new Segment;
//                Point* cur = tree[i];
//                do
//                {
//                    seg->push_back(cur);
//                    cur = cur->p;
//                }
//                while(cur && cur->childNum==1);
//                seg_list.push_back(seg);
//            }
//        }

//        vector<MyMarker*> outswc;
//        vector<MyMarker*> outswc_final;

//        for (V3DLONG i=0;i<seg_list.size();i++)
//        {
//            vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
//            nearpos_vec.clear();
//            farpos_vec.clear();
//            if(seg_list[i]->size() > 2)
//            {
//                for (V3DLONG j=0;j<seg_list[i]->size();j++)
//                {
//                    Point* node = seg_list[i]->at(j);
//                    XYZ loc0_t, loc1_t;
//                    loc0_t = XYZ(node->x, node->y,  node->z);
//                    switch (mip_plane)
//                    {
//                        case 0:  loc1_t = XYZ(node->x, node->y,  P-1); break;
//                        case 1:  loc1_t = XYZ(node->x, M-1,  node->z); break;
//                        case 2:  loc1_t = XYZ(N-1, node->y,  node->z); break;
//                        default:
//                            return;
//                    }

//                    XYZ loc0 = loc0_t;
//                    XYZ loc1 = loc1_t;

//                    nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
//                    farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
//                }

//                fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
//                smooth_curve(outswc,5);


//                for(V3DLONG d = 0; d <outswc.size(); d++)
//                {
//                    outswc[d]->radius = 2;
//                    outswc[d]->type = 2;
//                    outswc_final.push_back(outswc[d]);

//                }
//                outswc.clear();

//            }
//            else if(seg_list[i]->size() == 2)
//            {
//                Point* node1 = seg_list[i]->at(0);
//                Point* node2 = seg_list[i]->at(1);

//                for (V3DLONG j=0;j<3;j++)
//                {
//                    XYZ loc0_t, loc1_t;
//                    if(j ==0)
//                    {
//                        loc0_t = XYZ(node1->x, node1->y,  node1->z);
//                        switch (mip_plane)
//                        {
//                            case 0:  loc1_t = XYZ(node1->x, node1->y,  P-1); break;
//                            case 1:  loc1_t = XYZ(node1->x, M-1,  node1->z); break;
//                            case 2:  loc1_t = XYZ(N-1, node1->y,  node1->z); break;
//                            default:
//                                return;
//                        }
//                    }
//                    else if(j ==1)
//                    {
//                        loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
//                        switch (mip_plane)
//                        {
//                            case 0:  loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1); break;
//                            case 1:  loc1_t = XYZ(0.5*(node1->x + node2->x), M-1,   0.5*(node1->z + node2->z)); break;
//                            case 2:  loc1_t = XYZ(N-1,  0.5*(node1->y + node2->y),   0.5*(node1->z + node2->z)); break;
//                            default:
//                                return;
//                        }
//                    }
//                    else
//                    {
//                        loc0_t = XYZ(node2->x, node2->y,  node2->z);
//                        switch (mip_plane)
//                        {
//                            case 0:  loc1_t = XYZ(node2->x, node2->y,  P-1); break;
//                            case 1:  loc1_t = XYZ(node2->x, M-1,  node2->z); break;
//                            case 2:  loc1_t = XYZ(N-1, node2->y,  node2->z); break;
//                            default:
//                                return;
//                        }               }

//                    XYZ loc0 = loc0_t;
//                    XYZ loc1 = loc1_t;

//                    nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
//                    farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
//                 }

//                fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
//                smooth_curve(outswc,5);


//                for(V3DLONG d = 0; d <outswc.size(); d++)
//                {
//                    outswc[d]->radius = 2;
//                    outswc[d]->type = 2;
//                    outswc_final.push_back(outswc[d]);

//                }
//                outswc.clear();
//            }
//        }
    else if (menu_name == tr("mapping_markers"))
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        unsigned char* data1d = p4DImage->getRawData();

        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
        QString image_name = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        LandmarkList listLandmarks = callback.getLandmark(curwin);
        int marknum = listLandmarks.count();
        if(marknum ==0)
        {
            v3d_msg("No markers in the current image, please double check.");
            return;
        }

        QList <ImageMarker> marklist;
        QString markerpath =  image_name + QString("_3D.marker");
        ImageMarker S;

        for(V3DLONG i = 0; i < listLandmarks.size(); i++)
        {
            V3DLONG ix = listLandmarks.at(i).x;
            V3DLONG iy = listLandmarks.at(i).y;
            double I_max = 0;
            V3DLONG iz;
            for(V3DLONG j = 0; j < P; j++)
            {
                if(data1d[j*M*N + iy*N + ix] >= I_max)
                {
                    I_max = data1d[j*M*N + iy*N + ix];
                    iz = j;
                }

            }
            S.x = ix;
            S.y = iy;
            S.z = iz;
            marklist.append(S);
        }
        writeMarker_file(markerpath.toStdString().c_str(),marklist);

    }
    else
	{
        v3d_msg(tr("This is a plugin to map 2D tracing back to 3D locations based on 3D image content..."
			"Developed by Zhi Zhou, 2015-6-25"));
	}
}

bool mapping3D_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("mapping"))
	{
        if(infiles.size() != 2 && infiles.size() != 3)
        {
            cerr<<"Invalid input"<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
        string outswc_file = (infiles.size() == 3) ? infiles[2] : "";
        if(outswc_file == "") outswc_file = inswc_file + "_3D.swc";

        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
        cout<<"outswc_file = "<<outswc_file<<endl;

//        unsigned char * inimg1d = 0;
//        V3DLONG in_sz[4];
//        int datatype;
//        simple_loadimage_wrapper(callback,const_cast<char *>(inimg_file.c_str()), inimg1d, in_sz, datatype);
//        V3DLONG N = in_sz[0];
//        V3DLONG M = in_sz[1];
//        V3DLONG P = in_sz[2];


        NeuronTree nt = readSWC_file(QString(inswc_file.c_str()));
        vector<MyMarker*> outswc_final;
        //if(map3Dfunc(nt, inimg1d, N,M,P,outswc_final))
        if(map3Dfunc_raw(nt, inimg_file,outswc_final))
        {
                saveSWC_file(outswc_file, outswc_final);
                v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outswc_file.c_str()),0);
        }
    //    if(inimg1d) {delete []inimg1d; inimg1d=0;}
        return true;
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool map3Dfunc(NeuronTree nt,unsigned char * &data1d, V3DLONG N, V3DLONG M,V3DLONG P,vector<MyMarker*> & outswc_final)
{
    int mip_plane = 0;
    V3DLONG siz = nt.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }

    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }

    vector<MyMarker*> outswc;
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
        nearpos_vec.clear();
        farpos_vec.clear();
        if(seg_list[i]->size() > 2)
        {
            for (V3DLONG j=0;j<seg_list[i]->size();j++)
            {
                Point* node = seg_list[i]->at(j);
                XYZ loc0_t, loc1_t;
                loc0_t = XYZ(node->x, node->y,  node->z);
                switch (mip_plane)
                {
                    case 0:  loc1_t = XYZ(node->x, node->y,  P-1); break;
                    case 1:  loc1_t = XYZ(node->x, M-1,  node->z); break;
                    case 2:  loc1_t = XYZ(N-1, node->y,  node->z); break;
                    default:
                        return false;
                }

                XYZ loc0 = loc0_t;
                XYZ loc1 = loc1_t;

                nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
                farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
            }

            fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
            smooth_curve(outswc,5);


            for(V3DLONG d = 0; d <outswc.size(); d++)
            {
                outswc[d]->radius = 2;
                outswc[d]->type = 2;
                outswc_final.push_back(outswc[d]);

            }
            outswc.clear();

        }
        else if(seg_list[i]->size() == 2)
        {
            Point* node1 = seg_list[i]->at(0);
            Point* node2 = seg_list[i]->at(1);

            for (V3DLONG j=0;j<3;j++)
            {
                XYZ loc0_t, loc1_t;
                if(j ==0)
                {
                    loc0_t = XYZ(node1->x, node1->y,  node1->z);
                    switch (mip_plane)
                    {
                        case 0:  loc1_t = XYZ(node1->x, node1->y,  P-1); break;
                        case 1:  loc1_t = XYZ(node1->x, M-1,  node1->z); break;
                        case 2:  loc1_t = XYZ(N-1, node1->y,  node1->z); break;
                        default:
                            return false;
                    }
                }
                else if(j ==1)
                {
                    loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
                    switch (mip_plane)
                    {
                        case 0:  loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1); break;
                        case 1:  loc1_t = XYZ(0.5*(node1->x + node2->x), M-1,   0.5*(node1->z + node2->z)); break;
                        case 2:  loc1_t = XYZ(N-1,  0.5*(node1->y + node2->y),   0.5*(node1->z + node2->z)); break;
                        default:
                            return false;
                    }
                }
                else
                {
                    loc0_t = XYZ(node2->x, node2->y,  node2->z);
                    switch (mip_plane)
                    {
                        case 0:  loc1_t = XYZ(node2->x, node2->y,  P-1); break;
                        case 1:  loc1_t = XYZ(node2->x, M-1,  node2->z); break;
                        case 2:  loc1_t = XYZ(N-1, node2->y,  node2->z); break;
                        default:
                            return false;
                    }               }

                XYZ loc0 = loc0_t;
                XYZ loc1 = loc1_t;

                nearpos_vec.push_back(MyMarker(loc0.x, loc0.y, loc0.z));
                farpos_vec.push_back(MyMarker(loc1.x, loc1.y, loc1.z));
             }

            fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, N,M,P, 1, 5);
            smooth_curve(outswc,5);


            for(V3DLONG d = 0; d <outswc.size(); d++)
            {
                outswc[d]->radius = 2;
                outswc[d]->type = 2;
                outswc_final.push_back(outswc[d]);

            }
            outswc.clear();
        }
    }
    return true;
}

bool map3Dfunc_raw(NeuronTree nt,string &image_name,vector<MyMarker*> & outswc_final)
{
    V3DLONG siz = nt.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = nt.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        V3DLONG pid = nt.hashNeuron.value(nt.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }

    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }

    vector<MyMarker*> outswc;

    unsigned char * data1d = 0;
    V3DLONG *im3D_zz = 0;
    V3DLONG *im3D_sz = 0;

    int datatype;
    if (!loadRawRegion(const_cast<char *>(image_name.c_str()), data1d, im3D_zz, im3D_sz,datatype,0,0,0,1,1,1))
    {
        return false;
    }

    if(data1d) {delete []data1d; data1d = 0;}
    V3DLONG N = im3D_zz[0];
    V3DLONG M = im3D_zz[1];
    V3DLONG P = im3D_zz[2];
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        V3DLONG xb = N-1;
        V3DLONG xe = 0;
        V3DLONG yb = M-1;
        V3DLONG ye = 0;

        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            Point* node = seg_list[i]->at(j);
            if(node->x < xb) xb = node->x;
            if(node->x > xe) xe = node->x;
            if(node->y < yb) yb = node->y;
            if(node->y > ye) ye = node->y;
        }

        vector<MyMarker> nearpos_vec, farpos_vec; // for near/far locs testing
        nearpos_vec.clear();
        farpos_vec.clear();
        if(seg_list[i]->size() > 2)
        {
            for (V3DLONG j=0;j<seg_list[i]->size();j++)
            {
                Point* node = seg_list[i]->at(j);
                XYZ loc0_t, loc1_t;
                loc0_t = XYZ(node->x, node->y,  node->z);
                loc1_t = XYZ(node->x, node->y,  P-1);
                XYZ loc0 = loc0_t;
                XYZ loc1 = loc1_t;

                nearpos_vec.push_back(MyMarker(loc0.x - xb, loc0.y - yb, loc0.z));
                farpos_vec.push_back(MyMarker(loc1.x - xb, loc1.y - yb, loc1.z));
            }

            if (!loadRawRegion(const_cast<char *>(image_name.c_str()), data1d, im3D_zz, im3D_sz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
                printf("can not load the region");
                if(data1d) {delete []data1d; data1d = 0;}
                return false;
            }

            fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, xe-xb+1,ye-yb+1,P, 1, 5);
            smooth_curve(outswc,5);

            for(V3DLONG d = 0; d <outswc.size(); d++)
            {
                outswc[d]->radius = 2;
                outswc[d]->type = 2;
                outswc[d]->x = outswc[d]->x + xb;
                outswc[d]->y = outswc[d]->y + yb;
                outswc_final.push_back(outswc[d]);
            }
            if(data1d) {delete []data1d; data1d = 0;}
            outswc.clear();
        }
        else if(seg_list[i]->size() == 2)
        {
            Point* node1 = seg_list[i]->at(0);
            Point* node2 = seg_list[i]->at(1);

            for (V3DLONG j=0;j<3;j++)
            {
                XYZ loc0_t, loc1_t;
                if(j ==0)
                {
                    loc0_t = XYZ(node1->x, node1->y,  node1->z);
                    loc1_t = XYZ(node1->x, node1->y,  P-1);
                }
                else if(j ==1)
                {
                    loc0_t = XYZ(0.5*(node1->x + node2->x), 0.5*(node1->y + node2->y),  0.5*(node1->z + node2->z));
                    loc1_t = XYZ(0.5*(node1->x + node2->x),  0.5*(node1->y + node2->y),  P-1);

                }
                else
                {
                    loc0_t = XYZ(node2->x, node2->y,  node2->z);
                    loc1_t = XYZ(node2->x, node2->y,  P-1);
                }

                XYZ loc0 = loc0_t;
                XYZ loc1 = loc1_t;

                nearpos_vec.push_back(MyMarker(loc0.x - xb, loc0.y - yb, loc0.z));
                farpos_vec.push_back(MyMarker(loc1.x - xb, loc1.y - yb, loc1.z));
             }

            if (!loadRawRegion(const_cast<char *>(image_name.c_str()), data1d, im3D_zz, im3D_sz,datatype,xb,yb,0,xe+1,ye+1,P))
            {
                printf("can not load the region");
                if(data1d) {delete []data1d; data1d = 0;}
                return false;
            }

            fastmarching_drawing_dynamic(nearpos_vec, farpos_vec, (unsigned char*)data1d, outswc, xe-xb+1,ye-yb+1,P, 1, 5);
            smooth_curve(outswc,5);


            for(V3DLONG d = 0; d <outswc.size(); d++)
            {
                outswc[d]->radius = 2;
                outswc[d]->type = 2;
                outswc[d]->x = outswc[d]->x + xb;
                outswc[d]->y = outswc[d]->y + yb;
                outswc_final.push_back(outswc[d]);
            }
            if(data1d) {delete []data1d; data1d = 0;}
            outswc.clear();
        }
    }

    return true;

}
