/*
 *  swc_to_maskimage .cpp
 *  swc_to_maskimage
 *
 *  Created by Yang, Jinzhu, on 11/27/10.
 *  Last change: by Hanchuan peng, 2010-Dec-8
 */

#include <QtGlobal>

#include "swc_to_maskimage.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "stackutil.h"
#include <vector>
using namespace std;
static filter_dialog *dialog=0;



//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(swc_to_maskimage, SWC_TO_MASKIMAGElugin);


//plugin funcs
const QString title = "swc_to_maskimage";
QStringList SWC_TO_MASKIMAGElugin::menulist() const
{
    return QStringList()
    <<tr("swc_to_maskimage")
    <<tr("swc_filter")
    <<tr("About");

}
QStringList SWC_TO_MASKIMAGElugin::funclist() const
{
	return QStringList()
	<<tr("swc_to_maskimage")
    <<tr("swc_filter")
    <<tr("Help");
}
bool SWC_TO_MASKIMAGElugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name==tr("swc_to_maskimage"))
    {
        swc_to_maskimage(callback, input, output);
    }
    else if (func_name==tr("Help"))
    {
        printHelp();
    }
//    else if (func_name==tr("TOOLBOXswc_to_maskimage"))
//    {
//        swc_to_maskimage_toolbox(input, callback, parent);
//        return true;
//    }
    return false;
}

void SWC_TO_MASKIMAGElugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("swc_to_maskimage"))
	{
        swc_to_maskimage(callback, parent);
    }

    else if (menu_name == tr("swc_filter"))
    {
        dialog=new filter_dialog(&callback);
        dialog->setWindowTitle("swc_filter");
        dialog->show();
    }
    else if (menu_name == tr("About"))
	{
        v3d_msg("(version 1.0)<p> <b>'swc_to_maskimage'</b> converts a swc file to a mask image where the masked areas are 255 while the others are 0.<br>"
                "<b>'swc_filter'</b> directly uses a swc file to mask an image. The swc masked area in the image are non-zeros while other areas are set to 0.<p>"
                "The tool is developed by ** and reimplemented by Yujie Li. Further question please contact yujie.jade@gmail.com");
    }

}

void swc_to_maskimage(V3DPluginCallback2 &callback, QWidget *parent)
{
	NeuronTree neuron;
	double x_min,x_max,y_min,y_max,z_min,z_max;
	x_min=x_max=y_min=y_max=z_min=z_max=0;
	V3DLONG sx,sy,sz;
    unsigned char *pImMask = 0;
	QString filename;
	QStringList filenames;
    V3DLONG nx,ny,nz;

    filenames = QFileDialog::getOpenFileNames(0, 0,"","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);

    if(filenames.isEmpty())
    {
        v3d_msg("You don't have any SWC file open in the main window.");
        return;
    }
    NeuronSWC *p_cur=0;

    for (V3DLONG i = 0; i < filenames.size();i++)
    {
        filename = filenames[i];
        if (filename.isEmpty())
        {
            v3d_msg("You don't have any SWC file open in the main window.");
            return;
        }
        neuron = readSWC_file(filename);
        for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
        {
            p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
            if (p_cur->r<=0)
            {
                v3d_msg("You have illeagal radius values. Check your data.");
                return;
            }
         }
        BoundNeuronCoordinates(neuron,x_min,x_max,y_min,y_max,z_min,z_max);
        sx=x_max;
        sy=y_max;
        sz=z_max;
        V3DLONG stacksz = sx*sy*sz;
        pImMask = new unsigned char [stacksz];
        memset(pImMask,0,stacksz*sizeof(unsigned char));
        ComputemaskImage(neuron, pImMask, sx, sy, sz);
	}

	// compute coordinate region
        nx=sx;ny=sy;nz=sz;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want set the image size?"), QMessageBox::Yes, QMessageBox::No))
	{
		SetsizeDialog dialog(callback, parent);
		dialog.coord_x->setValue(sx);
		dialog.coord_y->setValue(sy);
		dialog.coord_z->setValue(sz);
        if (dialog.exec()==QDialog::Accepted){
			nx = dialog.coord_x->text().toLong();
			ny = dialog.coord_y->text().toLong();
			nz = dialog.coord_z->text().toLong();
		}
    }
    unsigned char *pData = new unsigned char[nx*ny*nz];
    memset(pData,0,nx*ny*nz*sizeof(unsigned char));

    if(nx>=sx && ny>=sy && nz>=sz)
     {
         for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
                for(V3DLONG i1 = 0; i1 < sx; i1++){
                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];
                }
            }
         }
     }
     else
     {
        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
                " get part of the swc mask");
        for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
               for(V3DLONG i1 = 0; i1 < sx; i1++){
                   if ((i1>nx-1)||(j1>ny-1)||(k1>nz-1)) continue;
                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];
               }
           }
        }
     }
    Image4DSimple tmp;
    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &tmp);
    callback.setImageName(newwin, QString("Output_swc_mask"));
    callback.updateImageWindow(newwin);
    callback.open3DWindow(newwin);

}

bool swc_to_maskimage(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;

    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    if(infiles.size() != 1) return false;
    if(paras.size() != 0 && paras.size() != 3) return false;

    QString qs_input(infiles[0]);
    NeuronTree neuron = readSWC_file(qs_input);
    QString qs_output = outfiles.empty() ? qs_input + "_out.raw" : QString(outfiles[0]);
    NeuronSWC *p_cur=0;
    double x_min,x_max,y_min,y_max,z_min,z_max;
    x_min=x_max=y_min=y_max=z_min=z_max=0;
    V3DLONG sx,sy,sz;
    unsigned char* pImMask = 0;
    V3DLONG nx,ny,nz;

    for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
    {
        p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
        if (p_cur->r<=0)
        {
            v3d_msg("You have illeagal radius values. Check your data.");
            return false;
        }
     }
    BoundNeuronCoordinates(neuron,x_min,x_max,y_min,y_max,z_min,z_max);
    sx=x_max;
    sy=y_max;
    sz=z_max;
    V3DLONG stacksz = sx*sy*sz;
    pImMask = new unsigned char [stacksz];
    memset(pImMask,0,stacksz*sizeof(unsigned char));
    ComputemaskImage(neuron, pImMask, sx, sy, sz);

    // compute coordinate region
    if (paras.empty()) {nx = sx; ny = sy; nz = sz;}
    else {
        nx = atoi(paras[0]);
        ny = atoi(paras[1]);
        nz = atoi(paras[2]);
    }

    unsigned char *pData = new unsigned char[nx*ny*nz];
    memset(pData,0,nx*ny*nz*sizeof(unsigned char));

    if(nx>=sx && ny>=sy && nz>=sz)
     {
         for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
                for(V3DLONG i1 = 0; i1 < sx; i1++){
                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];
                }
            }
         }
     }
     else
     {
        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
                " get part of the swc mask");
        for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
               for(V3DLONG i1 = 0; i1 < sx; i1++){
                   if ((i1>=nx)||(j1>=ny)||(k1>=nz)) continue;
                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];
               }
           }
        }
     }
    V3DLONG siz[4];
    siz[0] = nx; siz[1] = ny; siz[2] = nz; siz[3] = 1;
    simple_saveimage_wrapper(callback, qPrintable(qs_output), pData, siz, V3D_UINT8);

    if (pData) {delete []pData; pData=0;}
    return true;
}

//void swc_to_maskimage_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback, QWidget * parent)
//{
//	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *) (input.at(0).p);
//	NeuronTree neuron = paras->nt;

//	double x_min,x_max,y_min,y_max,z_min,z_max;
//	x_min=x_max=y_min=y_max=z_min=z_max=0;
//	V3DLONG sx,sy,sz;
//	unsigned char* pImMask = 0;
//	unsigned char* ImMark = 0;

//	BoundNeuronCoordinates(neuron,
//			x_min,
//			x_max,
//			y_min,
//			y_max,
//			z_min,
//			z_max);

//    sx=x_max;
//    sy=y_max;
//    sz=z_max;

//	V3DLONG stacksz = sx*sy*sz;
//    pImMask = new unsigned char [stacksz];
//    ImMark = new unsigned char [stacksz];
//	for (V3DLONG i=0; i<stacksz; i++)
//		pImMask[i] = ImMark[i] = 0;

//    ComputemaskImage(neuron, pImMask, sx, sy, sz);


//	// compute coordinate region
//	Image4DSimple tmp;
//	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want set the image size?"), QMessageBox::Yes, QMessageBox::No))
//    {
//		SetsizeDialog dialog(callback, parent);
//		dialog.coord_x->setValue(sx);
//		dialog.coord_y->setValue(sy);
//		dialog.coord_z->setValue(sz);
//        if (dialog.exec()!=QDialog::Accepted){
//            nx=sx;ny=sy;nz=sz;
//            return;
//        }
//		else
//		{
//			nx = dialog.coord_x->text().toLong();
//			ny = dialog.coord_y->text().toLong();
//			nz = dialog.coord_z->text().toLong();
//			//printf("nx=%d ny=%d nz=%d\n ",nx,ny,nz);
//		}
//     }
//     else{
//        nx=sx;ny=sy;nz=sz;
//     }

//    unsigned char *pData = new unsigned char[nx*ny*nz];
//    for (V3DLONG ii=0; ii<nx*ny*nz; ii++)
//        pData[ii] = 0;

//    if(nx>=sx && ny>=sy && nz>=sz)
//     {
//         for (V3DLONG k1 = 0; k1 < sz; k1++)
//         {
//             for(V3DLONG j1 = 0; j1 < sy; j1++)
//             {
//                for(V3DLONG i1 = 0; i1 < sx; i1++)
//                {
//                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

//                }
//            }
//        }

//     }

//     else
//     {
//        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
//                " get part of the swc mask");
//        //tmp.setData(pImMask, nx, ny, nz, 1, V3D_UINT8);
//        for (V3DLONG k1 = 0; k1 < sz; k1++){
//            for(V3DLONG j1 = 0; j1 < sy; j1++){
//               for(V3DLONG i1 = 0; i1 < sx; i1++){
//                   if ((i1>nx)||(j1>ny)||(k1>nz)) continue;
//                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

//               }
//           }
//       }

//     }
//    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
//	v3dhandle newwin = callback.newImageWindow();
//	callback.setImage(newwin, &tmp);
//	callback.setImageName(newwin, QString("Neuron_Mask_%1.tif").arg(neuron.file));
//	callback.updateImageWindow(newwin);
//}

void printHelp()
{
	printf("\nswc to mask image using sphere unit\n");
	printf("Usage v3d -x swc_to_maskimage_sphere_unit -f swc_to_maskimage -i <intput.swc> [-p <sz0> <sz1> <sz2>] [-o <output_image.raw>]\n");
}


