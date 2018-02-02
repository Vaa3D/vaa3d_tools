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


using namespace std;

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


        QString model_file = "/local4/DeepNeuron/2_labels/2D_CNN_5_layers/deploy.prototxt";
        QString trained_file = "/local4/DeepNeuron/2_labels/2D_CNN_5_layers/caffenet_train_iter_270000.caffemodel";
        QString mean_file = "/local4/DeepNeuron/2_labels/2D_CNN_5_layers/imagenet_mean.binaryproto";

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

