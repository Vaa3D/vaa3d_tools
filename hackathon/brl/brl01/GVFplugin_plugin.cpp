/* GVFplugin_plugin.cpp
 * Implementation of Gradient Vector Flow cell body segmentation
 * 2014.01.06 : by BRL
 */
 
#include "v3d_message.h"
#include <vector>
#include "dialog_watershed_para.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "GVFplugin_plugin.h"
#include "FL_gvfCellSeg.h"

#define USHORTINT16 unsigned short int

using namespace std;
Q_EXPORT_PLUGIN2(GVFplugin, GVFplugin);
 



QStringList GVFplugin::menulist() const
{
	return QStringList() 
		<<tr("GVF_Segmentation")
        <<tr("About");
}

QStringList GVFplugin::funclist() const
{
	return QStringList()
        <<tr("gvf_segmentation_brl")
        <<tr("help");
}

void GVFplugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("GVF_Segmentation"))
    {


        // the GVF function wants a pointer to a Vol3DSimple, which I haven't seen before.
        // this code below generates it  (take from plugin_FL_cellseg)


        // check what's up with the current window: is there a valid image pointer?
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            v3d_msg("Please open an image.");
            return;
        }



        Image4DSimple* subject = callback.getImage(curwin);
        QString m_InputFileName = callback.getImageName(curwin);

        if (!subject)
        {
            QMessageBox::information(0, "", QObject::tr("No image is open."));
            return;
        }
        if (subject->getDatatype()!=V3D_UINT8)
        {
            QMessageBox::information(0, "", QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
            return;
        }

        V3DLONG sz0 = subject->getXDim();
        V3DLONG sz1 = subject->getYDim();
        V3DLONG sz2 = subject->getZDim();
        V3DLONG sz3 = subject->getCDim();

        Image4DProxy<Image4DSimple> pSub(subject);





        V3DLONG channelsz = sz0*sz1*sz2;

        float *pLabel = 0;
        unsigned char *pData = 0;


        //the original code got the segmentation parameters using a special dialog window. but
        //  The processing function gvfCellSeg wants a gvfsegPara, and the dialog generates a segParameters...

   /*     gvfsegPara segpara;
        dialog_watershed_para *p_mydlg=0;
        if (!p_mydlg) p_mydlg = new dialog_watershed_para(&segpara, subject);
        int res = p_mydlg->exec();
        if (res!=QDialog::Accepted)
            return;
        else
            p_mydlg->fetchData(&segpara);
        if (p_mydlg) {delete p_mydlg; p_mydlg=0;}
*/
        gvfsegPara segpara;  // I'm just going to set these manually for now:

        segpara.diffusionIteration=  5;
        segpara.fusionThreshold = 10;
        segpara.minRegion = 10;
        segpara.sigma = 3;  // doesn't seem to be used in the actual function?


        //input parameters
        bool ok1;
        int c=1;

        if (sz3>1) //only need to ask if more than one channel
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Choose channel # (1):",
                                             1, 1, sz3, 1, &ok1);
            if (!ok1)
                return;
        }

        // clunky way to read in parameters

        segpara.diffusionIteration = QInputDialog::getInteger(parent, "Diffusion Iterations",
                                         "Choose Number of Diffusion Iterations:",
                                         1, 1, 10, 1, &ok1);
        if (!ok1)
            return;


        segpara.fusionThreshold = QInputDialog::getInteger(parent, "Fusion Threshold",
                                         "Choose Fusion Threshold :",
                                         1, 1, 10, 1, &ok1);
        if (!ok1)
            return;


        segpara.minRegion= QInputDialog::getInteger(parent, "Minimum Region",
                                         "Choose Minimum Region Size :",
                                         1, 1, 1000, 1, &ok1);
        if (!ok1)
            return;


        // allocate memory for the images

        Vol3DSimple <unsigned char> * tmp_inimg = 0;
        Vol3DSimple <USHORTINT16> * tmp_outimg = 0;
        try
        {
            tmp_inimg = new Vol3DSimple <unsigned char> (sz0, sz1, sz2);
            tmp_outimg = new Vol3DSimple <USHORTINT16> (sz0, sz1, sz2);
        }
        catch (...)
        {
            v3d_msg("Unable to allocate memory for processing.");
            if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}
            if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
            return;
        }

        //copy image data into our new memory
        memcpy((void *)tmp_inimg->getData1dHandle(), (void *)subject->getRawDataAtChannel(c), sz0*sz1*sz2);

        //now do computation

        //bool b_res = gvfCellSeg(img3d, outimg3d, segpara);
        bool b_res = gvfCellSeg(tmp_inimg, tmp_outimg, segpara);

        // clear out temporary space
        if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}


        if (!b_res)
        {
            v3d_msg("image segmentation  using gvfCellSeg()  failed \n");
        }
        else
        {   // now display the results

       // parameters for the new image data
            V3DLONG new_sz0 = tmp_outimg->sz0();
            V3DLONG new_sz1 = tmp_outimg->sz1();
            V3DLONG new_sz2 = tmp_outimg->sz2();
            V3DLONG new_sz3 = 1;
            V3DLONG tunits = new_sz0*new_sz1*new_sz2*new_sz3;


            //
            USHORTINT16 * outvol1d = new USHORTINT16 [tunits];

         //   USHORTINT16 * tmpImg_d1d = (USHORTINT16 *)(tmp_outimg->getData1dHandle());

            memcpy((void *)outvol1d, (void *)tmp_outimg->getData1dHandle(), tunits*sizeof(USHORTINT16));
            if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;} //free the space immediately for better use of memory

            Image4DSimple p4DImage;
            p4DImage.setData((unsigned char*)outvol1d, sz0, sz1, sz2, 1, V3D_UINT16);

            v3dhandle newwin = callback.newImageWindow();
            callback.setImage(newwin, &p4DImage);
            callback.setImageName(newwin, QString("Segmented Image"));
            callback.updateImageWindow(newwin);
        }

        return;
       // remove("temp.v3draw");
       // remove("gfImage.v3draw");
    }
	else
	{
		v3d_msg(tr("Implementation of Gradient Vector Flow cell body segmentation. "
			"Developed by BRL, 2014.01.06"));
	}
}

bool GVFplugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("gvf_segmentation_brl"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

