/* autoCropping_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-6-15 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "autoCropping_plugin.h"

#include "basic_surf_objs.h"
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "neuron_sim_scores.h"

using namespace std;
Q_EXPORT_PLUGIN2(autoCropping, autoCropping);
 
QStringList autoCropping::menulist() const
{
	return QStringList() 
        <<tr("Crop")
        <<tr("TraingSetGeneration");
}

QStringList autoCropping::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

NeuronTree cropSWCfile(NeuronTree nt, int xb, int xe, int yb, int ye);


void autoCropping::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Crop"))
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
        QString imgname = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        int tmpx,tmpy,tmpz;
        LandmarkList listLandmarks = callback.getLandmark(curwin);
        LocationSimple tmpLocation(0,0,0);
        int marknum = listLandmarks.count();
        if(marknum ==0)
        {
            v3d_msg("No markers in the current image, please double check.");
            return;
        }
        bool ok;
        int winSize = QInputDialog::getInteger(parent, "Please specify the block size","size:",50,0,256,1,&ok);
        if (!ok)
            return;

        QString APP2fileName;
        APP2fileName = QFileDialog::getOpenFileName(0, QObject::tr("Open APP2 SWC File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(APP2fileName.isEmpty())
            return;

        QString NeutubefileName;
        NeutubefileName = QFileDialog::getOpenFileName(0, QObject::tr("Open Neutube SWC File"),
                APP2fileName,
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(NeutubefileName.isEmpty())
            return;

        QString GSfileName;
        GSfileName = QFileDialog::getOpenFileName(0, QObject::tr("Open Gold Standard SWC File"),
                APP2fileName,
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(GSfileName.isEmpty())
            return;

        NeuronTree nt_app2, nt_neutube, nt_gs;
        nt_app2 = readSWC_file(APP2fileName);
        nt_neutube = readSWC_file(NeutubefileName);
        nt_gs = readSWC_file(GSfileName);

        for (int i=0;i<marknum;i++)
        {
            tmpLocation = listLandmarks.at(i);
            tmpLocation.getCoord(tmpx,tmpy,tmpz);

            V3DLONG xb = tmpx-1-winSize; if(xb<0) xb = 0;
            V3DLONG xe = tmpx-1+winSize; if(xe>=N-1) xe = N-1;
            V3DLONG yb = tmpy-1-winSize; if(yb<0) yb = 0;
            V3DLONG ye = tmpy-1+winSize; if(ye>=M-1) ye = M-1;


            QString outimg_file_app2 = imgname + QString("_x%1_x%2_y%3_y%4_app2.swc").arg(xb).arg(xe).arg(yb).arg(ye);
            QString outimg_file_neutube = imgname + QString("_x%1_x%2_y%3_y%4_neutube.swc").arg(xb).arg(xe).arg(yb).arg(ye);
            QString outimg_file_gs = imgname + QString("_x%1_x%2_y%3_y%4_gs.swc").arg(xb).arg(xe).arg(yb).arg(ye);


            NeuronTree nt_app2_cropped =  cropSWCfile(nt_app2,xb,xe,yb,ye);
            NeuronTree nt_app2_sort;
            if(nt_app2_cropped.listNeuron.size()>0)
            {
                nt_app2_sort = SortSWC_pipeline(nt_app2_cropped.listNeuron,nt_app2_cropped.listNeuron.at(0).n, 0);
            }
            else
                nt_app2_sort = nt_app2_cropped;

            NeuronTree nt_neutube_cropped =  cropSWCfile(nt_neutube,xb,xe,yb,ye);
            NeuronTree nt_neutube_sort;
            if(nt_neutube_cropped.listNeuron.size()>0)
            {
                nt_neutube_sort = SortSWC_pipeline(nt_neutube_cropped.listNeuron,nt_neutube_cropped.listNeuron.at(0).n, 0);
            }else
                nt_neutube_sort = nt_neutube_cropped;

            NeuronTree nt_gs_cropped =  cropSWCfile(nt_gs,xb,xe,yb,ye);
            NeuronTree nt_gs_sort;
            if(nt_gs_cropped.listNeuron.size()>0)
            {
                nt_gs_sort = SortSWC_pipeline(nt_gs_cropped.listNeuron,nt_gs_cropped.listNeuron.at(0).n, 0);
            }else
                nt_gs_sort = nt_gs_cropped;

            writeSWC_file(outimg_file_app2,nt_app2_sort);
            writeSWC_file(outimg_file_neutube,nt_neutube_sort);
            writeSWC_file(outimg_file_gs,nt_gs_sort);


            NeuronDistSimple score_app2 = neuron_score_rounding_nearest_neighbor(&nt_gs_sort, &nt_app2_sort,false);
            NeuronDistSimple score_neutube = neuron_score_rounding_nearest_neighbor(&nt_gs_sort, &nt_neutube_sort,false);


            V3DLONG im_cropped_sz[4];
            im_cropped_sz[0] = xe - xb + 1;
            im_cropped_sz[1] = ye - yb + 1;
            im_cropped_sz[2] = P;
            im_cropped_sz[3] = sc;

            unsigned char *im_cropped = 0;
            V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}
            V3DLONG j = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = yb; iy <= ye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix <= xe; ix++)
                    {
                         im_cropped[j] = data1d[offsetk + offsetj + ix];
                         j++;
                    }
                }
            }
            QString outimg_file;
            if(score_app2.dist_allnodes < score_neutube.dist_allnodes)
                outimg_file = imgname + QString("_x%1_x%2_y%3_y%4_APP2.tif").arg(xb).arg(xe).arg(yb).arg(ye);
            else
                outimg_file = imgname + QString("_x%1_x%2_y%3_y%4_NEUTUBE.tif").arg(xb).arg(xe).arg(yb).arg(ye);

            simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
            if(im_cropped) {delete []im_cropped; im_cropped = 0;}
        }
        v3d_msg("Done!");
	}
    else if (menu_name == tr("TraingSetGeneration"))
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
        QString imgname = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        bool ok;
        int winSize = QInputDialog::getInteger(parent, "Please specify the block size","size:",50,0,256,1,&ok);
        if (!ok)
            return;

        QString SWCfileName;
        SWCfileName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(SWCfileName.isEmpty())
            return;

        NeuronTree nt = readSWC_file(SWCfileName);
        for (int i=0;i<nt.listNeuron.size();i++)
        {
            if(nt.listNeuron.at(i).type == 2)
            {
                double tmpx = nt.listNeuron.at(i).x;
                double tmpy = nt.listNeuron.at(i).y;

                V3DLONG xb = tmpx-1-winSize; if(xb<0) xb = 0;
                V3DLONG xe = tmpx-1+winSize; if(xe>=N-1) xe = N-1;
                V3DLONG yb = tmpy-1-winSize; if(yb<0) yb = 0;
                V3DLONG ye = tmpy-1+winSize; if(ye>=M-1) ye = M-1;


                QString outimg_file_swc = imgname + QString("_x%1_x%2_y%3_y%4.swc").arg(xb).arg(xe).arg(yb).arg(ye);

                NeuronTree nt_cropped =  cropSWCfile(nt,xb,xe,yb,ye);
                NeuronTree nt_sort;
                if(nt_cropped.listNeuron.size()>0)
                {
                    nt_sort = SortSWC_pipeline(nt_cropped.listNeuron,nt_cropped.listNeuron.at(0).n, 0);
                }
                else
                    nt_sort = nt_cropped;

                writeSWC_file(outimg_file_swc,nt_sort);


                V3DLONG im_cropped_sz[4];
                im_cropped_sz[0] = xe - xb + 1;
                im_cropped_sz[1] = ye - yb + 1;
                im_cropped_sz[2] = P;
                im_cropped_sz[3] = sc;

                unsigned char *im_cropped = 0;
                V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
                try {im_cropped = new unsigned char [pagesz];}
                catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}
                V3DLONG j = 0;
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = yb; iy <= ye; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        for(V3DLONG ix = xb; ix <= xe; ix++)
                        {
                             im_cropped[j] = data1d[offsetk + offsetj + ix];
                             j++;
                        }
                    }
                }
                QString outimg_file = imgname + QString("_x%1_x%2_y%3_y%4.tif").arg(xb).arg(xe).arg(yb).arg(ye);

                simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
                if(im_cropped) {delete []im_cropped; im_cropped = 0;}
            }
        }

    }
    else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2016-6-15"));
	}
}

NeuronTree cropSWCfile(NeuronTree nt, int xb, int xe, int yb, int ye)
{
    //NeutronTree structure
    NeuronTree nt_prunned;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node

    QList<NeuronSWC> list = nt.listNeuron;
    NeuronSWC S;
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        if(curr.x <= xe && curr.x >=xb && curr.y <= ye && curr.y >=yb)
        {
            S.x 	= curr.x-xb;
            S.y 	= curr.y-yb;
            S.n 	= curr.n;
            S.type = curr.type;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
   }
   nt_prunned.n = -1;
   nt_prunned.on = true;
   nt_prunned.listNeuron = listNeuron;
   nt_prunned.hashNeuron = hashNeuron;

   return nt_prunned;
}

bool autoCropping::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
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

