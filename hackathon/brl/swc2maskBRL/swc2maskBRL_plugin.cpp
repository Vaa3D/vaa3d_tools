//last change: by PHC 20130905. Creating this file actually fix the Windows VC++ build problem of creating dynamic library files. (w/o this, the dll was created but not the .lib file and .exp file), and thus the plugin cannot be instantiated.

#include "swc2maskBRL_plugin.h"

#include <iostream>
#include <vector>

#include "src/swc2maskBRL.h"
#include "swc_convert.h"
#include "my_surf_objs.h"
#include "basic_surf_objs.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"


Q_EXPORT_PLUGIN2(swc2maskBRL, SWC2MaskBRLPlugin);
using namespace std;

static string basename(string para)
{
    int pos1 = para.find_last_of("/");
    int pos2 = para.find_last_of(".");
    if(pos1 == string::npos) pos1 = -1;
    if(pos2 == string::npos) pos2 = para.size();
    return para.substr(pos1+1, pos2 - pos1 -1);
}

// file_type("test.tif") == ".tif"
string file_type(string para)
{
    int pos = para.find_last_of(".");
    if(pos == string::npos) return string("unknown");
    else return para.substr(pos, para.size() - pos);
}


QStringList SWC2MaskBRLPlugin::menulist() const
{
    return QStringList()
            << tr("swc2maskBRL")
            << tr("about");
}

void SWC2MaskBRLPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == "swc2maskBRL")
    {
        QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("Open Neuron Tree"), QObject::tr("."), QObject::tr("ESWC Files (*.eswc)"));
        if(fileName == "") {QMessageBox::information(0,"","No swc file open"); return;}

        string inswc_file = fileName.toStdString();
        vector<MyMarkerX*> inswc;
        if(!(readESWC_file(inswc_file, inswc  )))return;
        V3DLONG sz0 = 0, sz1 = 0, sz2 = 0;
        if(1)
        {
            MyMarkerX * marker = inswc[0];
            V3DLONG x = marker->x + 0.5;
            V3DLONG y = marker->y + 0.5;
            V3DLONG z = marker->z + 0.5;
            V3DLONG r = marker->radius;
            V3DLONG mx = x, my = y, mz = z, Mx = x, My = y, Mz = z;
            V3DLONG margin = 0;
            cout<<"marker 0 : x"<< x <<" y "<< y << "z  "<< z << "radius"<< r<<  endl;
            for(int i = 1; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                mx = MIN(x, mx);
                my = MIN(y, my);
                mz = MIN(z, mz);
                Mx = MAX(x, Mx);
                My = MAX(y, My);
                Mz = MAX(z, Mz);
                margin = MAX(margin, (V3DLONG)(marker->radius+0.5));
            }
            mx -= margin;
            my -= margin;
            mz -= margin;
            Mx += margin;
            My += margin;
            Mz += margin;

            sz0 = Mx - mx + 1;
            sz1 = My - my + 1;
            sz2 = Mz - mz + 1;
            for(int i = 0; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                marker->x = x - mx;
                marker->y = y - my;
                marker->z = z - mz;
            }
        }
        cout<<"size : "<<sz0<<"x"<<sz1<<"x"<<sz2<<endl;

        unsigned char * outimg1d = 0;
        if(!swc2maskBRL((unsigned short int* &)outimg1d, inswc, sz0, sz1, sz2))
        {
            QMessageBox::information(0, "", "Failed to do swc2maskBRL !");
            return;
        }
        V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};

        v3dhandle newwin = callback.newImageWindow();
        Image4DSimple * new4DImage = new Image4DSimple();
        new4DImage->setData(outimg1d, sz0, sz1, sz2, 1, V3D_UINT16);
        callback.setImage(newwin, new4DImage);
        callback.updateImageWindow(newwin);
    }
    else if(menu_name == "about")
    {
        QMessageBox::information(0, "SWC2MaskBRL Plugin", \
                                 QObject::tr("This is Brian's version of the swc2mask plugin"));
    }
}

QStringList SWC2MaskBRLPlugin::funclist() const
{
    return QStringList()
            << tr("swc2maskBRL")
            << tr("help");
}

bool SWC2MaskBRLPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(func_name == "swc2maskBRL")
    {
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;

        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(paras.size() != 0 && paras.size() != 3)
        {
            cerr<<"input para error!"<<endl; return false;
        }

        string inimg_file;
        string inswc_file;
       vector<MyMarkerX*> inswc;
        unsigned char * inimg1d = 0; int datatype = 0;
        V3DLONG in_sz[4];

        V3DLONG sz0 = 0, sz1 = 0, sz2 = 0;

        if(infiles.size() == 1)
        {
            inswc_file = infiles[0];
            if(file_type(inswc_file) != ".swc")
            {
                cerr<<"Input is not swc file"<<endl;
                return false;
            }
        }
        else if(infiles.size() == 2)
        {
            if(file_type(infiles[0]) == ".swc") inswc_file = infiles[0];
            else inimg_file = infiles[0];
            if(file_type(infiles[1]) == ".swc") inswc_file = infiles[1];
            else inimg_file = infiles[1];
            if(inswc_file == "" || inimg_file == "")
            {
                cerr<<"Input files error!"<<endl;
                return false;
            }
            if(!simple_loadimage_wrapper(callback, (char*)inimg_file.c_str(), inimg1d, in_sz, datatype))
            {cerr<<"Load image "<<inimg_file<<" error!"<<endl; return false;}
            //if(datatype != V3D_UINT8){cerr<<"Currently only support 8bit image as input"<<endl; return false;}
            sz0 = in_sz[0]; sz1 = in_sz[1]; sz2 = in_sz[2];
            if(in_sz[3] > 1) {cout<<"Use red channel as input."<<endl;}
        }

      if(!readESWC_file(inswc_file, inswc ))
      {
          cerr<<"ESWC read error"<<endl;
          return false;
      }


 //    vector<MyMarker*> inswc = readSWC_file(inswc_file);

        string outimg_file = outfiles.empty() ? basename(inswc_file) + "_out.raw" : outfiles[0];
        if(infiles.size() == 1 && paras.empty())
        {
            MyMarkerX * marker = inswc[0];
            V3DLONG x = marker->x + 0.5;
            V3DLONG y = marker->y + 0.5;
            V3DLONG z = marker->z + 0.5;
            V3DLONG mx = x, my = y, mz = z, Mx = x, My = y, Mz = z;
            V3DLONG margin = 0;
            for(int i = 1; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                mx = MIN(x, mx);
                my = MIN(y, my);
                mz = MIN(z, mz);
                Mx = MAX(x, Mx);
                My = MAX(y, My);
                Mz = MAX(z, Mz);
                margin = MAX(margin, (V3DLONG)(marker->radius+0.5));
            }
            mx -= margin;
            my -= margin;
            mz -= margin;
            Mx += margin;
            My += margin;
            Mz += margin;

            sz0 = Mx - mx + 1;
            sz1 = My - my + 1;
            sz2 = Mz - mz + 1;
            for(int i = 0; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                marker->x = x - mx;
                marker->y = y - my;
                marker->z = z - mz;
            }
        }
        else if(infiles.size() == 1 && paras.size() == 3)
        {
            sz0 = atoi(paras[0]);
            sz1 = atoi(paras[1]);
            sz2 = atoi(paras[2]);
        }
        cout<<"size : "<<sz0<<"x"<<sz1<<"x"<<sz2<<endl;

        unsigned char * outimg1d = 0;
        if(!swc2maskBRL(outimg1d, inswc, sz0, sz1, sz2)) return false;
        if(infiles.size() == 2)
        {
            V3DLONG tol_sz = sz0 * sz1 * sz2;
            if(datatype == V3D_UINT8)
            {
                unsigned char * tmpimg1d = inimg1d;
                for(V3DLONG i = 0; i < tol_sz; i++)
                {
                    tmpimg1d[i] = (outimg1d[i] > 0) ? tmpimg1d[i] : 0;
                }
            }
            else if(datatype == V3D_UINT16)
            {
                unsigned short int * tmpimg1d = (unsigned short int *) inimg1d;
                for(V3DLONG i = 0; i < tol_sz; i++)
                {
                    tmpimg1d[i] = (outimg1d[i] > 0) ? tmpimg1d[i] : 0;
                }
            }
            else if(datatype == V3D_FLOAT32)
            {
                float * tmpimg1d = (float *) inimg1d;
                for(V3DLONG i = 0; i < tol_sz; i++)
                {
                    tmpimg1d[i] = (outimg1d[i] > 0) ? tmpimg1d[i] : 0;
                }
            }
            V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
            if(!simple_saveimage_wrapper(callback, outimg_file.c_str(), inimg1d, out_sz, datatype))
            {
                cerr<<"Unable to save image to file "<<outimg_file<<endl;
                return false;
            }
            if(inimg1d){delete [] inimg1d; inimg1d = 0;}
            //if(in_sz){delete [] in_sz; in_sz = 0;}
        }
        else
        {
            V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
            if(!simple_saveimage_wrapper(callback, outimg_file.c_str(), outimg1d, out_sz, V3D_UINT8))
            {
                cerr<<"Unable to save image to file "<<outimg_file<<endl;
                return false;
            }
        }
        if(outimg1d){delete [] outimg1d; outimg1d = 0;}
        return true;
    }/*
    else if(func_name == "TOOLBOXswc2mask")
    {
        vaa3d_neurontoolbox_paras * toolbox_paras = (vaa3d_neurontoolbox_paras *)input.at(0).p;
        NeuronTree nt = toolbox_paras->nt;
        vector<MyMarkerX*> inswc = swc_convert(nt);

        string outimg_file = basename(nt.file.toStdString())+"_out.raw";
        cout<<"outimg_file = "<<outimg_file<<endl;

        V3DLONG sz0 = 0, sz1 = 0, sz2 = 0;
        v3dhandle curwin = callback.getImageWindowList().empty() ? 0 : callback.currentImageWindow();
        View3DControl * view3d = callback.getView3DControl(curwin);
        if(1)
        {
            MyMarkerX * marker = inswc[0];
            V3DLONG x = marker->x + 0.5;
            V3DLONG y = marker->y + 0.5;
            V3DLONG z = marker->z + 0.5;
            V3DLONG mx = x, my = y, mz = z, Mx = x, My = y, Mz = z;
            V3DLONG margin = 0;
            for(int i = 1; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                mx = MIN(x, mx);
                my = MIN(y, my);
                mz = MIN(z, mz);
                Mx = MAX(x, Mx);
                My = MAX(y, My);
                Mz = MAX(z, Mz);
                margin = MAX(margin, (V3DLONG)(marker->radius+0.5));
            }
            mx -= margin;
            my -= margin;
            mz -= margin;
            Mx += margin;
            My += margin;
            Mz += margin;

            sz0 = Mx - mx + 1;
            sz1 = My - my + 1;
            sz2 = Mz - mz + 1;
            for(int i = 0; i < inswc.size(); i++)
            {
                marker = inswc[i];
                x = marker->x + 0.5;
                y = marker->y + 0.5;
                z = marker->z + 0.5;
                marker->x = x - mx;
                marker->y = y - my;
                marker->z = z - mz;
            }
        }
        else
        {
            Image4DSimple * p4dImage = callback.getImage(curwin);
            sz0 = p4dImage->getXDim();
            sz1 = p4dImage->getYDim();
            sz2 = p4dImage->getZDim();
        }
        cout<<"size : "<<sz0<<"x"<<sz1<<"x"<<sz2<<endl;

        unsigned char * outimg1d = 0;
        if(!swc2maskBRL(outimg1d, inswc, sz0, sz1, sz2)) return false;

        v3dhandle newwin = callback.newImageWindow();
        Image4DSimple * new4dImage = new Image4DSimple();
        //NeuronTree new_nt = swc_convert(inswc);
        new4dImage->setData(outimg1d, sz0, sz1, sz2, 1, V3D_UINT8);
        callback.setImage(newwin, new4dImage);
        callback.updateImageWindow(newwin);
        callback.open3DWindow(newwin);
        //callback.setSWC(newwin, new_nt);
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
        return true;
    }*/
    else if(func_name == "help")
    {
        cout<<"Usage : v3d -x dll_name -f swc2maskBRL -i <swc_file> [-p <sz0> <sz1> <sz2>] [-o <outimg_file>]"<<endl;
        cout<<endl;
        cout<<"Produce mask image from swc structure. The outimg size will be the bounding box of swc structure or provided size."<<endl;
    }
    return false;
}



