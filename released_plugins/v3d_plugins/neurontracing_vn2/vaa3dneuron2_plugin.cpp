//last change: by Hanchuan Peng. 2012-09-21
//last change: by Hanchuan Peng. 2012-11-24
//last change: by Hanchuan Peng. 2012-11-27. add the better radius estimation
//last change: by Hanchuan Peng. 2012-12-17. Separate the img preprocessing functions
//last change: by Hanchuan Peng. 2012-12-17. make the APP2 code much modulized!
//last change: by Hanchuan Peng. 2012-12-30. make the code better modulized and also modulize the do_func part!
//last change: by Hanchuan Peng. 2013-01-02. change the help info and the default paras for the do_func of APP2 module, to make it consistent with the do_menu

#include <QtGui>
#include <v3d_interface.h>

#include "vn_app1.h"
#include "vn_app2.h"

#include "vaa3dneuron2_plugin.h"


Q_EXPORT_PLUGIN2(vn2, NTApp2Plugin);

QString versionStr = "v2.620";

QStringList NTApp2Plugin::menulist() const
{
    return QStringList()
    << tr("Vaa3D-Neuron2-APP2") //tr("APP2 - all path pruning v2")
    << tr("Vaa3D-Neuron2-APP1") //tr("APP1 - all path pruning v1")
    << tr("about and help");
}

void NTApp2Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == "about and help")
    {
        v3d_msg(versionStr.prepend("Vaa3D-Neuron2 ").append(". Developed by Hanchuan Peng Lab. 2012 @ Janelia/Allen\n\n** How to use **\n\nJust open your neuron image, then click the menu items. You can also use the Vaa3D do-func command line interface to script this plugin for batched neuron reconstruction."));
    }
    else if(menu_name == "Vaa3D-Neuron2-APP2" || 
            menu_name == "Vaa3D-Neuron2-APP1" )
    {
        if(callback.getImageWindowList().empty())
        {
            v3d_msg("Please open an image first");
            return;
        }
        
        if (menu_name == "Vaa3D-Neuron2-APP2")
        {
            PARA_APP2 p;
            if(!p.initialize(callback)) //here will initialize the image pointer, bounding box, etc.
                return;
            
            // fetch parameters from dialog
            if (!p.app2_dialog())
                return;
            
            if (!proc_app2(callback, p, versionStr))
                return;
        }
        else //if (menu_name == "Vaa3D-Neuron2-APP1")
        {
            //v3d_msg("Vaa3D-Neuron2-APP1 func to add soon.");
            PARA_APP1 p;
            if(!p.initialize(callback)) //here will initialize the image pointer, bounding box, etc.
                return;
            
            // fetch parameters from dialog
            if (!p.app1_dialog())
                return;
            
            if (!proc_app1(callback, p, versionStr))
                return;
        }
    }
    else
    {
        v3d_msg("Invalid menu string!");
    }
}

QStringList NTApp2Plugin::funclist() const
{
    return QStringList()<<"app2"<<"app1"<<"help";
}

bool NTApp2Plugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(func_name == "app2")
    {
        PARA_APP2 p;

        if (!p.fetch_para_commandline(input, output, callback, parent))
            return false;
        
        if (!proc_app2(callback, p, versionStr))
            return false;
    }
    else if(func_name == "app1")
    {
        PARA_APP1 p;
        
        if (!p.fetch_para_commandline(input, output, callback, parent))
            return false;
        
        if (!proc_app1(callback, p, versionStr))
            return false;
    }
    else //if (func_name == "help")
    {
        v3d_msg(versionStr.prepend("\nVaa3D-Neuron2 APP2/APP1  "), 0);
        printf("\n**** Usage of APP2 ****\n");
        printf("vaa3d -x plugin_name -f app2 -i <inimg_file> -o <outswc_file> -p [<inmarker_file> [<channel> [<bkg_thresh> [<b_256cube> [<b_RadiusFrom2D> [<is_gsdt> [<is_gap> [<length_thresh>]]]]]]]]\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("inmarker_file    If no input marker file, please set this para to NULL and it will detect soma automatically. \n"
               "                 When the file is set, then the first marker is used as root/soma.\n");
        printf("channel          Data channel for tracing. Start from 0 (default 0).\n");
        printf("bkg_thresh       Default 10 (is specified as -1 then auto-thresolding)\n");

        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 1.)\n");
        printf("b_RadiusFrom2D   If estimate the radius of each reconstruction node from 2D plane only (1 for yes as many times the data is anisotropic, and 0 for no. Default 1 which which uses 3D estimation.)\n");
        printf("is_gsdt          If use gray-scale distance transform (1 for yes and 0 for no. Default 0.)\n");
        printf("is_gap           If allow gap (1 for yes and 0 for no. Default 0.)\n");
        
        printf("length_thresh    Default 5\n");
        //printf("SR_ratio         signal/reduntancy threshold, default 1/3\n");
        printf("outswc_file      If not be specified, will be named automatically based on the input image file name.\n");
        
        printf("\n**** Usage of APP1 ****\n");
        printf("vaa3d -x plugin_name -f app1 -i <inimg_file> -p [<inmarker_file> [<channel> [<bkg_thresh> [<b_256cube> ]]]]\n");
        printf("inimg_file       Should be 8/16/32bit image\n");
        printf("inmarker_file    If no input marker file, please set this para to NULL and it will detect soma automatically. \n"
               "                 When the file is set, then the first marker is used as root/soma.\n");
        printf("channel          Data channel for tracing. Start from 0 (default 0).\n");
        printf("bkg_thresh       Default -1 (-1 is for auto-thresholding, otherwise the threshold specified by a user will be used.\n");
        
        printf("b_256cube        If trace in a auto-downsampled volume (1 for yes, and 0 for no. Default 1.)\n");
        printf("outswc_file      If not be specified, will be named automatically based on the input image file name.\n");

    }
    return true;
}


