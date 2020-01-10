/* ML_get_sample_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-6 : by OYQ
 */
 
#include "v3d_message.h"
#include <vector>
#include "ML_get_sample_plugin.h"
#include "APP1_pruning.h"
#include "FL_watershed_vs.h"
#include "FL_bwlabel2D3D.h"
#include "volimg_proc.h"
//#include "/home/braincenter5/vaa3d_tools/hackathon/fl_cellseg/src/label_object_dialog.h"
using namespace std;
Q_EXPORT_PLUGIN2(ML_get_sample, ML_sample);
 
void watershed_vs(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
QStringList ML_sample::menulist() const
{
	return QStringList() 
        <<tr("suspicious_tips")
        <<tr("watershed")
		<<tr("about");
}

QStringList ML_sample::funclist() const
{
	return QStringList()
        <<tr("get_undertraced_sample")
        <<tr("get_overtraced_sample")
        <<tr("get_2D3D_block")
        <<tr("get_2D_block")
        <<tr("remove_tip_location")
        <<tr("prune_tip_APP1")
        <<tr("find_fake_tip")
        <<tr("rotation")
        <<tr("mean_shift_center")
        <<tr("prediction_model")
        <<tr("help")
        <<tr("help1");
}

void ML_sample::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("suspicious_tips"))
	{
        find_short_branch_tips_menu(callback,parent);
	}
    else if (menu_name == tr("watershed"))
    {
        watershed_vs(callback, parent, 3);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by OYQ, 2018-12-6"));
	}
}

bool ML_sample::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("get_undertraced_sample"))
	{
        get_undertraced_sample(input,output,callback);
	}
    else if (func_name == tr("get_overtraced_sample"))
    {
        get_overtraced_sample(input,output,callback);
    }
    else if (func_name == tr("get_2D3D_block"))
    {
        get_block(input,output,callback);
    }
    else if (func_name == tr("get_2D_block"))
    {
        get_2d_image(input,output,callback);
    }
    else if (func_name == tr("remove_tip_location"))
    {
        get_tip_image(input,output,callback);
    }
    else if (func_name == tr("prune_tip_APP1"))
    {
        pruning_covered_leaf_single_cover(input,output,callback);
    }
    else if (func_name == tr("prune_tip_thres"))
    {
        prune_terminal_nodes(input,output,callback);
    }
    else if (func_name == tr("find_fake_tip"))
    {
        find_short_branch_tips_func(input,output,callback);
    }
    else if (func_name == tr("rotation"))
    {
        rotation(input,output,callback);
    }
    else if (func_name == tr("prediction_model"))
    {
        prediction(input,output,callback);
    }
    else if (func_name == tr("mean_shift"))
    {
        mean_shift_oyq(input,output,callback);
    }
    else if (func_name == tr("3D_mask"))
    {
        get_3d_mask(input,output,callback);
    }
	else if (func_name == tr("help"))
	{
        printHelp(input,output);
	}
    else if (func_name == tr("help1"))
    {
        printHelp1(input,output);
    }
	else return false;

	return true;
}
const QString title = "Watershed segment image objects";
void watershed_vs(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    if (method_code!=3 && method_code!=2)
    {
        v3d_msg("Invalid watershed method code. You should never see this message. Report this bug to the developer");
        return;
    }

    LabelImgObjectParaDialog dialog(callback, parent);
    if (!dialog.image)
        return;
    if (dialog.exec()!=QDialog::Accepted)
        return;

    V3DLONG ch = dialog.ch;
    V3DLONG th_idx = dialog.th_idx;
    double tt = dialog.thresh;
    V3DLONG volsz_thres = (dialog.b_filtersmallobjs) ? dialog.volsz : 0; //the threshold to filter out small objects

    int start_t = clock(); // record time

    Image4DSimple* subject = callback.getImage(curwin);
    QString m_InputFileName = callback.getImageName(curwin);

    if (!subject)
    {
        QMessageBox::information(0, title, QObject::tr("No image is open."));
        return;
    }
    if (subject->getDatatype()!=V3D_UINT8)
    {
        QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
        return;
    }

    if (th_idx==0 || th_idx==1)
    {
        double mm, vv;
        mean_and_std(subject->getRawDataAtChannel(ch), subject->getTotalUnitNumberPerChannel(), mm, vv);
        tt = (th_idx == 0) ? mm : mm+vv;
        v3d_msg(QString("in fast image object labeling: ch=%1 mean=%2 std=%2").arg(ch).arg(mm).arg(vv), 0);
    }

    Image4DProxy<Image4DSimple> pSub(subject);

    V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
    V3DLONG sz3 = subject->getCDim();

    //----------------------------------------------------------------------------------------------------------------------------------
    V3DLONG channelsz = sz0*sz1*sz2;

    float *pLabel = 0;
    unsigned char *pData = 0;

    try
    {
        //pLabel = new float [channelsz];
        pData = new unsigned char [channelsz];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in Watershed segmentation.");
        if (pLabel) {delete []pLabel; pLabel=0;}
        if (pData) {delete []pData; pData=0;}
        return;
    }

    V3DLONG i;
    unsigned char * pSubtmp = pSub.begin();
    for(i = 0; i < channelsz;  i++)
    {
        pData[i] = (pSubtmp[i]<=tt) ? 0 : 1;
    }

    // dist transform
    V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
    V3DLONG nobjs=0;
    if (method_code==3)
    {
        V3DLONG nh_code=26; //6,18,or 26
        watershed_vs(pData, pLabel, sz_data, 3, nh_code);
    }
    else if (method_code==2)
    {
        V3DLONG nh_code=8; //4 or 8
        watershed_vs(pData, pLabel, sz_data, 2, nh_code);
    }
    else
    {
        v3d_msg("Invalid BWLabelN method code. You should never see this message. Report this bug to the developer");
        return;
    }

    if (pData) {delete []pData; pData=0;}
    if (!pLabel)
    {
        v3d_msg("The label field of the image objects is not valid. Quit.");
        return;
    }

    //seems nobjs has not been set, thus the filtering code should has a problem
    for (i=1, nobjs = pLabel[0]; i<channelsz; i++)
        if (V3DLONG(pLabel[i])>nobjs) {nobjs = pLabel[i];}

    if (nobjs<=0)
    {
        v3d_msg("No valid image object was detected using the p[arameters you specified. Quit.");
        if (pLabel) {delete []pLabel; pLabel=0;}
        return;
    }
    else
    {
        v3d_msg(QString("Watershed labeling finds at most %1 objects").arg(nobjs));
    }

    if (volsz_thres>0  && nobjs>0) //filter out small objects
    {
        try {
            float * hh = new float [nobjs];
            float * mapval = new float [nobjs];

            V3DLONG j;
            for (j=0;j<nobjs;j++) {hh[j]=0; mapval[j]=j;} //of course, 0 also map to 0!
            for (j=0;j<channelsz;j++)
            {
                //pLabel[j]--; //it seems Fuhui's data is 1-based, so subtract 1. Is this correct?
                hh[V3DLONG(pLabel[j])]++;
            }
            V3DLONG k=0;
            for (j=1;j<nobjs;j++) //start from 1 as the value is the background!
            {
                if (hh[j]<volsz_thres)
                {
                    mapval[j]=0; //if less than a thres, then map to 0, which is background
                }
                else
                {
                    printf("Obj [%ld] = [%ld]\n", V3DLONG(j), V3DLONG(hh[j]));
                    k++;
                    mapval[j] = k; //otherwise map to a continous label-value
                }
            }
            for (j=0;j<channelsz;j++)
                pLabel[j] = mapval[V3DLONG(pLabel[j])];

            if (hh) {delete []hh; hh=0;}
            if (mapval) {delete []mapval; mapval=0;}
        }
        catch (...) {
            v3d_msg("Unable to allocate memory to filter small objects. Thus skip it.");
        }
    }

    //----------------------------------------------------------------------------------------------------------------------------------

    int end_t = clock();
    printf("time eclapse %d s for labeling objects!\n", (end_t-start_t)/1000000);

    Image4DSimple p4DImage;
    p4DImage.setData((unsigned char*)pLabel, sz0, sz1, sz2, 1, V3D_FLOAT32);

    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &p4DImage);
    callback.setImageName(newwin, QString("Object-Labeled Image"));
    callback.updateImageWindow(newwin);
}


