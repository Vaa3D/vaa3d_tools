/* ImageProcessing_plugin.cpp
 * This is a image processing plugin, you can use it as a demo.
 * 2018-10-27 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include "iostream"
#include "basic_surf_objs.h"
#include "v3d_interface.h"
#include "threshold_select.h"
#include "GMM_Algorithm.h"
#include "binary_gsdt.h"
#include "binary_connected.h"
#include "get_sub_block.h"
#include "branch_angle.h"
#include "expand_swc.h"
#include "ImageGrayValue.h"
#include "image_label.h"
#include "v3drawTOtif.h"

#include "ImageProcessing_plugin.h"
Q_EXPORT_PLUGIN2(ImageProcessing, ImageProcessingPlugin);

//using namespace std;

bool binary_gsdt(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool binary_gsdt(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
bool binary_connected(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
bool binary_connected(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);

QStringList ImageProcessingPlugin::menulist() const
{
    return QStringList()
            <<tr("threshold_select")
            <<tr("gmmAlgorithm")
            <<tr("binary_gsdt")
            <<tr("binary_connected")
            <<tr("get_sub_block")
            <<tr("branch_angle")
            <<tr("expand_swc")
            <<tr("ImageGrayValue")
            <<tr("image_label")
            <<tr("v3drawTOtif")
            <<tr("help");
}

QStringList ImageProcessingPlugin::funclist() const
{
    return QStringList()
            <<tr("threshold_select")
            <<tr("gmmAlgorithm")
            <<tr("binary_gsdt")
            <<tr("binary_connected")
            <<tr("get_sub_block")
            <<tr("branch_angle")
            <<tr("expand_swc")
            <<tr("ImageGrayValue")
            <<tr("image_label")
            <<tr("v3drawTOtif")
            <<tr("help");
}

void ImageProcessingPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("threshold_select"))
    {
        input_PARA PARA;
        threshold_select(callback,parent,PARA);
        v3d_msg("threshold_select done!");

    }

    else if (menu_name == tr("gmmAlgorithm"))
    {
        input_PARA PARA;
        gmm(callback,parent,PARA);
        v3d_msg("gmmAlgorithm done!");

    }

//    else if (menu_name == tr("binary_gsdt"))
//    {
//        input_PARA PARA;
//        binary_gsdt(callback,parent,PARA);
//        v3d_msg("binary_gsdt done!");

//    }

    else if (menu_name == tr("binary_connected"))
    {
        input_PARA PARA;
        binary_connected(callback,parent,PARA);
        v3d_msg("binary_connected done!");
    }

    else if (menu_name == tr("get_sub_block"))
    {
//        v3dhandle win = callback.currentImageWindow();
//        Image4DSimple * img = callback.getImage(win);
//        V3DLONG in_sz[4];
//        in_sz[0] = img->getXDim();
//        in_sz[1] = img->getYDim();
//        in_sz[2] = img->getZDim();
//        in_sz[3] = img->getCDim();
//        get_sub_block(img->getRawData(),in_sz,callback);

        get_sub_block(callback,parent);
        v3d_msg("get_sub_block done!");
    }

    else if (menu_name == tr("branch_angle"))
    {
        input_PARA PARA;
        //branch_angle1(callback,parent,PARA);  //remove by distance and angle
        branch_angle2(callback,parent,PARA);  //remove by parentpoints and angle
        //calculate_subtree(callback,parent,PARA); //calculate subtree angle
        v3d_msg("branch_angle done!");
    }
    else if (menu_name == tr("expand_swc"))
    {
        input_PARA PARA;
        expand_swc(callback,parent,PARA);
        v3d_msg("expand_swc done!");
    }
    else if (menu_name == tr("ImageGrayValue"))
    {
        input_PARA PARA;
        ImageGrayValue(callback,parent,PARA);
        v3d_msg("ImageGrayValue done!");
    }

    else if (menu_name == tr("image_label"))
    {
        input_PARA PARA;
        image_label(callback,parent,PARA);
        v3d_msg("image_label done!");
    }

    else if (menu_name == tr("v3drawTOtif"))
    {
        input_PARA PARA;
        v3drawTOtif(callback,parent,PARA);
        v3d_msg("v3drawTOtif done!");
    }
    else
    {
        v3d_msg(tr("This is a image processing plugin, you can use it as a demo.. "
            "Developed by Yongzhang, 2018-10-27"));
    }
}

bool ImageProcessingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    input_PARA PARA;
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("threshold_select"))
    {
        threshold_select(callback,input,output,parent,PARA);
        v3d_msg("threshold_select done!");
    }

    else if (func_name == tr("gmmAlgorithm"))
    {
        gmm(callback,input,output,parent,PARA);
        v3d_msg("gmmAlgorithm done!");
    }

//    else if (func_name == tr("binary_gsdt"))
//    {
//        input_PARA PARA;
//        binary_gsdt(callback,input,output,parent,PARA);
//        v3d_msg("binary_gsdt done!");
//    }

    else if (func_name == tr("binary_connected"))
    { 
        input_PARA PARA;
        binary_connected(callback,input,output,parent,PARA);
        cout<<"binary_connected done!"<<endl;
    }

    else if (func_name == tr("get_sub_block"))
    {
//        if(infiles.empty())
//        {
//            cerr<<"Need input swc file"<<endl;
//            return false;
//        }


//        QString  inImg_file =  infiles[0];
//        QString  outImg_file;
//        if(!outfiles.empty())
//            outImg_file = outfiles[0];
//        else
//            outImg_file = inImg_file + ".v3draw";



//        unsigned char * total1dData = 0;
//        V3DLONG in_sz[4];
//        int dataType;
//        if(!simple_loadimage_wrapper(callback, inImg_file.toStdString().c_str(), total1dData, in_sz, dataType))
//        {
//            cerr<<"load image "<<inImg_file.toStdString()<<" error!"<<endl;
//            return false;
//        }


//        Image4DSimple* total4DImage = new Image4DSimple;
//        total4DImage->setData((unsigned char*)total1dData, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);

//        unsigned char * out1dData = 0;
//        get_sub_block(total1dData,in_sz,callback,out1dData);

        get_sub_block(callback,input,output,parent);
        v3d_msg("get_sub_block done!");
    }

    else if (func_name == tr("branch_angle"))
    {
        input_PARA PARA;
        branch_angle(callback,input,output,parent,PARA);
        cout<<"branch_angle done!"<<endl;
    }

    else if (func_name == tr("expand_swc"))
    {
        input_PARA PARA;
        expand_swc(input,output);
        cout<<"expand_swc done!"<<endl;
    }
    else if (func_name == tr("ImageGrayValue"))
    {
        input_PARA PARA;
        ImageGrayValue(callback,input,output,parent,PARA);
        cout<<"ImageGrayValue done!"<<endl;
    }

    else if (func_name == tr("image_label"))
    {
        input_PARA PARA;
        image_label(callback,input,output,parent,PARA);
        v3d_msg("image_label done!");
    }
    else if (func_name == tr("v3drawTOtif"))
    {
        input_PARA PARA;
        v3drawTOtif(callback,input,output,parent,PARA);
        //v3d_msg("v3drawTOtif done!");
    }

    else if (func_name == tr("help"))
    {

       printf("Usage : vaa3d -x ImageProcessing -f threshold_select -i <inimg_file> -p <ch> ");
       printf("Usage : vaa3d -x ImageProcessing -f gmmAlgorithm -i <inimg_file> -p <ch> ");
       printf("Usage : vaa3d -x ImageProcessing -f binary_gsdt -i <inimg_file> -p <ch> ");
       printf("Usage : vaa3d -x ImageProcessing -f binary_connected -i <inimg_file> -p <ch> ");
       printf("Usage : vaa3d -x ImageProcessing -f branch_angle -i <inimg_file> -p <ch> ");

       printf("inimg_file       The input image\n");
       printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

    }
    else return false;

    return true;
}
