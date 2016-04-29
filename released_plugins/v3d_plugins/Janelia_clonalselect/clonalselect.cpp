/* clonalselect.cpp
 * 2013-01-13: create this program by Yang Yu
 */


//
//#ifndef __CLONALSELECT_SRC_CPP__
//#define __CLONALSELECT_SRC_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "clonalselect.h"
#include "clonalselect_gui.h"
#include "clonalselect_core.h"

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

//plugin interface
const QString title = "Clonal Selecting";
Q_EXPORT_PLUGIN2(clonalSelect, ClonalSelectPlugin);

// func declare
int clonalselecting(V3DPluginCallback2 &callback, QWidget *parent);

// gui interface
QStringList ClonalSelectPlugin::menulist() const
{
    return QStringList() << tr("Clonal Selecting")
                         << tr("About");
}

void ClonalSelectPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Clonal Selecting"))
    {
        if(clonalselecting(callback, parent))
        {
            cout<<"Error occurred in clonal selecting!"<<endl;
            return;
        }
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("ClonalSelecting Plugin %1 (March 1, 2010) developed by Yang Yu. Janelia Research Farm Campus, HHMI").arg(getPluginVersion()).append(""));
        return;
    }
}

// cmd interface
QStringList ClonalSelectPlugin::funclist() const
{
    return QStringList() << tr("i2p")
                         << tr("help");
}



// image stitch plugin dofunc
bool ClonalSelectPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("help"))
    {
        //print Help info
        printf("\nUsage: \tvaa3d -x clonalSelect -h \n\n");


        return true;
    }
    else if (func_name == tr("i2p"))
    {
        // input  : 3d mask image
        // output : point cloud

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nNo Inputs!\n");
            printf("\nUsage: \tvaa3d -x clonalSelect -f i2p -i <input_image> -o <output_pointcloud> \n\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".pcd";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".pcd";
        }

        ///// test ////
//        PointClouds pctest;
//        if(pctest.read(qs_filename_output.toStdString()))
//        {
//            cout<<"";
//            return false;
//        }
//        for(long i=0; i<pctest.pcdheadinfo.len; i++)
//        {
//            qDebug()<<pctest.points.at(i).x<<pctest.points.at(i).y<<pctest.points.at(i).z<<pctest.points.at(i).v;
//        }
//        return true;
        //// test ////


        /// load images
        V3DLONG sz_img[4];
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFile::exists(qs_filename_img_input))
        {
            if(!simple_loadimage_wrapper(callback,const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                if(p1dImg) {delete []p1dImg; p1dImg = 0;}
                return false;
            }
        }
        else
        {
            cout<<"Input does not exist"<<endl;
            return false;
        }

        // convert image to point cloud
        PointClouds pc;
        pc.convert(p1dImg, sz_img[0], sz_img[1], sz_img[2]);
        pc.write(qs_filename_output.toStdString());

        // de-alloc
        y_del<unsigned char>(p1dImg);

        return true;
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
}

// func
int clonalselecting(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();

    if(win_list.size()<1)
    {
        QMessageBox::information(0, title, QObject::tr("No image is open."));
        return -1;
    }

    ColonalSelectWidget *csw = new ColonalSelectWidget(callback, parent);

    if(csw)
    {
        csw->show();
    }

    //
    return 0;
}






//#endif // __CLONALSELECT_SRC_CPP__

