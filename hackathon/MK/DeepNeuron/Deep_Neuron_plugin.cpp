/* Deep_Neuron_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-12-12 : by MK
 */
 
#include <vector>
#include <iostream>

#include "v3d_message.h"
#include "Deep_Neuron_plugin.h"
#include "DeepNeuronUI.h"
#include "DLOperator.h"
#include "imgOperator.h"
#include "tester.h"
#include "../../zhi/deep_learning/prediction/classification.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../xiaoxiaol/consensus_skeleton_2/mst_boost_prim.h"
#include "../../zhi/AllenNeuron_postprocessing/sort_swc_IVSCC.h"


using namespace std;
using namespace boost;

double vectorDistance(std::vector<float> v1, std::vector<float> v2)
{
  double ret = 0.0;
  for(int i = 0; i< v1.size();i++)
  {
      double dist = (v1.at(i) - v2.at(i));
      ret += dist * dist;
  }
  return ret > 0.0 ? sqrt(ret) : 0.0;
}


Q_EXPORT_PLUGIN2(Deep_Neuron, DeepNeuron_plugin);
 
QStringList DeepNeuron_plugin::menulist() const
{
    return QStringList()
            <<tr("Neurite Signal Detection")
           <<tr("Neurite Connection")
          <<tr("Smart Pruning")
         <<tr("Reconstruction Evaluation")
        <<tr("Classification of Dendrites and Axons")
       <<tr("about");
}

QStringList DeepNeuron_plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void DeepNeuron_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Neurite Signal Detection"))
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

        DNInputDialog * openDlg = new DNInputDialog(&callback,parent);
        openDlg->m_pLineEdit_swcfile->setEnabled(false);
        openDlg->pPushButton_openFileDlg_swcfile->setEnabled(false);

        if (!openDlg->exec())
            return;

        QString model_file = openDlg->model_file;
        QString trained_file = openDlg->trained_file;
        QString mean_file = openDlg->mean_file;

        int Sxy = 10;
        int Ws = 512;

        unsigned char* data1d = p4DImage->getRawData();
        QString imagename = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        unsigned char *data1d_mip=0;
        V3DLONG pagesz_mip = in_sz[0]*in_sz[1];
        try {data1d_mip = new unsigned char [pagesz_mip];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}
        for(V3DLONG iy = 0; iy < M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                int max_mip = 0;
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    if(data1d[offsetk + offsetj + ix] >= max_mip)
                    {
                        data1d_mip[iy*N + ix] = data1d[offsetk + offsetj + ix];
                        max_mip = data1d[offsetk + offsetj + ix];
                    }
                }
            }
        }

        std::vector<std::vector<float> > detection_results;
        LandmarkList marklist_2D;
        for(V3DLONG iy = 0; iy < M; iy = iy+Ws)
        {

            V3DLONG yb = iy;
            V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;
            for(V3DLONG ix = 0; ix < N; ix = ix+Ws)
            {
                V3DLONG xb = ix;
                V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;
                unsigned char *blockarea=0;
                V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*1;

                blockarea = new unsigned char [blockpagesz];
                V3DLONG i = 0;
                for(V3DLONG iiy = yb; iiy < ye+1; iiy++)
                {
                    V3DLONG offsetj = iiy*N;
                    for(V3DLONG iix = xb; iix < xe+1; iix++)
                    {

                        blockarea[i] = data1d_mip[offsetj + iix];
                        i++;
                    }
                }
                Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
                detection_results = batch_detection(blockarea,classifier,xe-xb+1,ye-yb+1,1,Sxy);

                V3DLONG d = 0;
                for(V3DLONG iiy = yb+Sxy; iiy < ye+1; iiy = iiy+Sxy)
                {
                    for(V3DLONG iix = xb+Sxy; iix < xe+1; iix = iix+Sxy)
                    {
                        std::vector<float> output = detection_results[d];
                        if(output.at(1) > output.at(0))
                        {
                            LocationSimple S;
                            S.x = iix;
                            S.y = iiy;
                            S.z = 1;
                            marklist_2D.push_back(S);
                        }
                        d++;
                    }
                }
                if(blockarea) {delete []blockarea; blockarea =0;}

            }
        }

        cerr<<"mean shifting ..."<<endl;

        //mean shift
        mean_shift_fun fun_obj;
        LandmarkList marklist_2D_shifted;
        vector<V3DLONG> poss_landmark;
        vector<float> mass_center;
        double windowradius = Sxy+5;

        V3DLONG sz_img[4];
        sz_img[0] = N; sz_img[1] = M; sz_img[2] = 1; sz_img[3] = 1;
        fun_obj.pushNewData<unsigned char>((unsigned char*)data1d_mip, sz_img);
        poss_landmark=landMarkList2poss(marklist_2D, sz_img[0], sz_img[0]*sz_img[1]);

        for (V3DLONG j=0;j<poss_landmark.size();j++)
        {
            mass_center=fun_obj.mean_shift_center_mass(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            marklist_2D_shifted.append(tmp);

        }

        QList <ImageMarker> marklist_3D;
        ImageMarker S;
        NeuronTree nt;
        QList <NeuronSWC> & listNeuron = nt.listNeuron;

        for(V3DLONG i = 0; i < marklist_2D_shifted.size(); i++)
        {
            V3DLONG ix = marklist_2D_shifted.at(i).x;
            V3DLONG iy = marklist_2D_shifted.at(i).y;
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
            S.color.r = 255;
            S.color.g = 0;
            S.color.b = 0;
            marklist_3D.append(S);
        }

        Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
        QList <ImageMarker> marklist_3D_pruned = batch_deletion(data1d,classifier,marklist_3D,N,M,P);
        cerr<<"Deleting false detection ..."<<endl;

        for(V3DLONG i = 0; i < marklist_3D_pruned.size(); i++)
        {
            V3DLONG ix = marklist_3D_pruned.at(i).x;
            V3DLONG iy = marklist_3D_pruned.at(i).y;
            V3DLONG iz = marklist_3D_pruned.at(i).z;

            NeuronSWC n;
            n.x = ix-1;
            n.y = iy-1;
            n.z = iz-1;
            n.n = i+1;
            n.type = 2;
            n.r = 1;
            n.pn = -1; //so the first one will be root
            listNeuron << n;
        }

        QString  swc_processed = imagename + "_detection.swc";
        writeSWC_file(swc_processed,nt);
        v3d_msg(QString("Neurite signal detection result is saved in %1").arg(swc_processed));
	}
    else if (menu_name == tr("Neurite Connection"))
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
        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        DNInputDialog * openDlg = new DNInputDialog(&callback,parent);
        openDlg->m_pLineEdit_meanfile->setEnabled(false);
        openDlg->pPushButton_openFileDlg_meanfile->setEnabled(false);

        if (!openDlg->exec())
            return;

        QString model_file = openDlg->model_file;
        QString trained_file = openDlg->trained_file;
        QString SWCfileName = openDlg->swc_file;

        NeuronTree nt = readSWC_file(SWCfileName);
        Classifier classifier(model_file.toStdString(), trained_file.toStdString(),"");
        std::vector<cv::Mat> imgs;

        int Wx = 60, Wy = 60, Wz = 5;
        V3DLONG num_patches = 0;
        std::vector<std::vector<float> > outputs_overall;
        std::vector<std::vector<float> > outputs;

        for (V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            V3DLONG tmpx = nt.listNeuron.at(i).x;
            V3DLONG tmpy = nt.listNeuron.at(i).y;
            V3DLONG tmpz = nt.listNeuron.at(i).z;

            V3DLONG xb = tmpx-1-Wx; if(xb<0) xb = 0;if(xb>=N-1) xb = N-1;
            V3DLONG xe = tmpx-1+Wx; if(xe>=N-1) xe = N-1;
            V3DLONG yb = tmpy-1-Wy; if(yb<0) yb = 0;if(yb>=M-1) yb = M-1;
            V3DLONG ye = tmpy-1+Wy; if(ye>=M-1) ye = M-1;
            V3DLONG zb = tmpz-1-Wz; if(zb<0) zb = 0;if(zb>=P-1) zb = P-1;
            V3DLONG ze = tmpz-1+Wz; if(ze>=P-1) ze = P-1;

            V3DLONG im_cropped_sz[4];
            im_cropped_sz[0] = xe - xb + 1;
            im_cropped_sz[1] = ye - yb + 1;
            im_cropped_sz[2] = 1;
            im_cropped_sz[3] = sc;

            unsigned char *im_cropped = 0;

            V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for im_cropped."); return;}
            memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

            for(V3DLONG iz = zb; iz <= ze; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                V3DLONG j = 0;
                for(V3DLONG iy = yb; iy <= ye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix <= xe; ix++)
                    {
                        if(data1d[offsetk + offsetj + ix] >= im_cropped[j])
                            im_cropped[j] = data1d[offsetk + offsetj + ix];
                        j++;
                    }
                }
            }

            cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
            imgs.push_back(img);

            if(num_patches >=5000)
            {
                outputs = classifier.extractFeature_siamese(imgs);
                for(V3DLONG d = 0; d<outputs.size();d++)
                    outputs_overall.push_back(outputs[d]);
                outputs.clear();
                imgs.clear();
                num_patches = 0;
            }else
                num_patches++;
        }
        if(imgs.size()>0)
        {
            outputs = classifier.extractFeature_siamese(imgs);
            for(V3DLONG d = 0; d<outputs.size();d++)
                outputs_overall.push_back(outputs[d]);
        }

        imgs.clear();

        cerr<<"MST generating ..."<<endl;

        UndirectedGraph g(nt.listNeuron.size());
        for (int i=0;i<nt.listNeuron.size()-1;i++)
        {
            for (int j=i+1;j<nt.listNeuron.size();j++)
            {
                V3DLONG x1 = nt.listNeuron.at(i).x;
                V3DLONG y1 = nt.listNeuron.at(i).y;
                V3DLONG z1 = nt.listNeuron.at(i).z;
                V3DLONG x2 = nt.listNeuron.at(j).x;
                V3DLONG y2 = nt.listNeuron.at(j).y;
                V3DLONG z2 = nt.listNeuron.at(j).z;
                double dis = sqrt(pow2(x1-x2) + pow2(y1-y2) + pow2(z1-z2));

                EdgeQuery edgeq = edge(i, j, *&g);
                if (!edgeq.second && i!=j && dis <=1000)
                {
                    double Vedge;
                    std::vector<float> v1 = outputs_overall[i];
                    std::vector<float> v2 = outputs_overall[j];
                    Vedge = vectorDistance(v1, v2)*dis;
                    add_edge(i, j, LastVoted(i, Weight(Vedge)), *&g);
                }
            }
        }

        vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(*&g));
        prim_minimum_spanning_tree(*&g, &p[0]);

        NeuronTree marker_MST;
        QList <NeuronSWC> listNeuron;
        QHash <int, int>  hashNeuron;
        listNeuron.clear();
        hashNeuron.clear();

        for (std::size_t i = 0; i != p.size(); ++i)
        {
            NeuronSWC S;
            int pn;
            if(p[i] == i)
                pn = -1;
            else
                pn = p[i] + 1;

            S.n 	= i+1;
            S.type 	= 7;
            S.x 	= nt.listNeuron.at(i).x;
            S.y 	= nt.listNeuron.at(i).y;
            S.z 	= nt.listNeuron.at(i).z;;
            S.r 	= 1;
            S.pn 	= pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }

        marker_MST.n = -1;
        marker_MST.on = true;
        marker_MST.listNeuron = listNeuron;
        marker_MST.hashNeuron = hashNeuron;


        for (int i=1;i<marker_MST.listNeuron.size()-1;i++)
        {
            if(marker_MST.listNeuron.at(i).parent>0)
            {
                V3DLONG x1 = marker_MST.listNeuron.at(i).x;
                V3DLONG y1 = marker_MST.listNeuron.at(i).y;
                V3DLONG z1 = marker_MST.listNeuron.at(i).z;
                V3DLONG x2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent-1).x;
                V3DLONG y2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent-1).y;
                V3DLONG z2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent-1).z;
                double dis = sqrt(pow2(x1-x2) + pow2(y1-y2) + 4*pow2(z1-z2));
                if(dis>80)
                    marker_MST.listNeuron[i].parent = -1;
            }
        }

        QString outfilename = SWCfileName + "_connection.swc";
        writeSWC_file(outfilename,marker_MST);
        v3d_msg(QString("The output file is [%1]").arg(outfilename));

    }
    else if (menu_name == tr("Smart Pruning"))
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


        DNInputDialog * openDlg = new DNInputDialog(&callback,parent);
        if (!openDlg->exec())
            return;

        QString model_file = openDlg->model_file;
        QString trained_file = openDlg->trained_file;
        QString mean_file = openDlg->mean_file;
        QString SWCfileName = openDlg->swc_file;

        unsigned char* data1d = p4DImage->getRawData();

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        NeuronTree nt = readSWC_file(SWCfileName);
        unsigned int Wx=15, Wy=15, Wz=15;
        Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
        std::vector<cv::Mat> imgs;
        V3DLONG num_patches = 0;
        std::vector<std::vector<float> > outputs_overall;
        std::vector<std::vector<float> > outputs;
        for (V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            V3DLONG tmpx = nt.listNeuron.at(i).x;
            V3DLONG tmpy = nt.listNeuron.at(i).y;
            V3DLONG tmpz = nt.listNeuron.at(i).z;

            V3DLONG xb = tmpx-1-Wx; if(xb<0) xb = 0;if(xb>=N-1) xb = N-1;
            V3DLONG xe = tmpx-1+Wx; if(xe>=N-1) xe = N-1;
            V3DLONG yb = tmpy-1-Wy; if(yb<0) yb = 0;if(yb>=M-1) yb = M-1;
            V3DLONG ye = tmpy-1+Wy; if(ye>=M-1) ye = M-1;
            V3DLONG zb = tmpz-1-Wz; if(zb<0) zb = 0;if(zb>=P-1) zb = P-1;
            V3DLONG ze = tmpz-1+Wz; if(ze>=P-1) ze = P-1;

            V3DLONG im_cropped_sz[4];
            im_cropped_sz[0] = xe - xb + 1;
            im_cropped_sz[1] = ye - yb + 1;
            im_cropped_sz[2] = 1;
            im_cropped_sz[3] = sc;

            unsigned char *im_cropped = 0;

            V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for im_cropped."); return;}
            memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

            for(V3DLONG iz = zb; iz <= ze; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                V3DLONG j = 0;
                for(V3DLONG iy = yb; iy <= ye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix <= xe; ix++)
                    {
                        if(data1d[offsetk + offsetj + ix] >= im_cropped[j])
                            im_cropped[j] = data1d[offsetk + offsetj + ix];
                        j++;
                    }
                }
            }

            cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
            imgs.push_back(img);

            if(num_patches >=10000)
            {
                outputs = classifier.Predict(imgs);
                for(V3DLONG d = 0; d<outputs.size();d++)
                    outputs_overall.push_back(outputs[d]);
                outputs.clear();
                imgs.clear();
                num_patches = 0;
            }else
                num_patches++;
        }

        if(imgs.size()>0)
        {
            outputs = classifier.Predict(imgs);
            for(V3DLONG d = 0; d<outputs.size();d++)
                outputs_overall.push_back(outputs[d]);
        }

        double p_num = 0;
        double n_num = 0;
        QList <ImageMarker> marklist;
        for (V3DLONG j=0;j<nt.listNeuron.size();j++)
        {
            std::vector<float> output = outputs_overall[j];
            if(output.at(0) > output.at(1))
            {
                ImageMarker S;
                S.x = nt.listNeuron.at(j).x;
                S.y = nt.listNeuron.at(j).y;
                S.z = nt.listNeuron.at(j).z;
                marklist.append(S);
                n_num++;
            }
            else
                p_num++;
        }

        NeuronTree nt_DL = DL_eliminate_swc(nt,marklist);
        NeuronTree nt_DL_sort = SortSWC_pipeline(nt_DL.listNeuron,VOID, 10);
        NeuronTree nt_DL_sort_pruned = remove_swc(nt_DL_sort,50);

        QString outswc_file = SWCfileName + "_pruned.swc";
        writeSWC_file(outswc_file, nt_DL_sort_pruned);
        outputs_overall.clear();
        imgs.clear();
        v3d_msg(QString("Smart pruning result is saved in %1").arg(outswc_file));
    }
    else if (menu_name == tr("Reconstruction Evaluation"))
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

        DNInputDialog * openDlg = new DNInputDialog(&callback,parent);
        if (!openDlg->exec())
            return;

        QString model_file = openDlg->model_file;
        QString trained_file = openDlg->trained_file;
        QString mean_file = openDlg->mean_file;
        QString SWCfileName = openDlg->swc_file;

        unsigned char* data1d = p4DImage->getRawData();

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        NeuronTree nt = readSWC_file(SWCfileName);
        unsigned int Wx=30, Wy=30, Wz=15;
        Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
        std::vector<cv::Mat> imgs;

        for (V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            V3DLONG tmpx = nt.listNeuron.at(i).x;
            V3DLONG tmpy = nt.listNeuron.at(i).y;
            V3DLONG tmpz = nt.listNeuron.at(i).z;

            V3DLONG xb = tmpx-1-Wx; if(xb<0) xb = 0;if(xb>=N-1) xb = N-1;
            V3DLONG xe = tmpx-1+Wx; if(xe>=N-1) xe = N-1;
            V3DLONG yb = tmpy-1-Wy; if(yb<0) yb = 0;if(yb>=M-1) yb = M-1;
            V3DLONG ye = tmpy-1+Wy; if(ye>=M-1) ye = M-1;
            V3DLONG zb = tmpz-1-Wz; if(zb<0) zb = 0;if(zb>=P-1) zb = P-1;
            V3DLONG ze = tmpz-1+Wz; if(ze>=P-1) ze = P-1;

            unsigned char *im_cropped = 0;
            V3DLONG im_cropped_sz[4];
            im_cropped_sz[0] = xe - xb + 1;
            im_cropped_sz[1] = ye - yb + 1;
            im_cropped_sz[2] = 1;
            V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2];
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for im_cropped."); return;}
            memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

            for(V3DLONG iz = zb; iz <= ze; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                V3DLONG j = 0;
                for(V3DLONG iy = yb; iy <= ye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix <= xe; ix++)
                    {
                        if(data1d[offsetk + offsetj + ix] >= im_cropped[j])
                            im_cropped[j] = data1d[offsetk + offsetj + ix];
                        j++;
                    }
                }
            }


            cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
            imgs.push_back(img);
        }

        std::vector<std::vector<float> > outputs = classifier.Predict(imgs);
        double p_num = 0;
        double n_num = 0;
        QList <ImageMarker> marklist;
        for (V3DLONG j=0;j<nt.listNeuron.size();j++)
        {
            std::vector<float> output = outputs[j];
            if(output.at(0) > output.at(1))
            {
                ImageMarker S;
                S.x = nt.listNeuron.at(j).x;
                S.y = nt.listNeuron.at(j).y;
                S.z = nt.listNeuron.at(j).z;
                marklist.append(S);
                n_num++;
            }
            else
                p_num++;

        }
        v3d_msg(QString("Evaluation score is %1").arg(p_num/outputs.size()));
        imgs.clear();
        return;
    }
    else if (menu_name == tr("Classification of Dendrites and Axons"))
    {
        DeepNeuronUI* inputForm = new DeepNeuronUI(parent, &callback);
        inputForm->setWindowModality(Qt::NonModal);
        inputForm->setWindowFlags(Qt::WindowStaysOnTopHint);
        inputForm->show();
    }
	else
	{
        v3d_msg(tr("This is a DeepNeuron plugin including five modules "
            "Developed by ZZ and MK, 2017-12-12"));
	}
}

bool DeepNeuron_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		Tester tester1;
		tester1.inputFileStrings.push_back(infiles[0]); // image
		tester1.inputFileStrings.push_back(infiles[1]); // marker
		tester1.inputStrings.push_back(inparas[0]);     // deploy
		tester1.inputStrings.push_back(inparas[1]);     // model
		tester1.inputStrings.push_back(inparas[2]);     // mean
		tester1.outputStrings.push_back(outfiles[0]);   // output swc

		tester1.theCallbackPtr = &callback;
		tester1.test1();
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

