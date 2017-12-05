/* Neurite_Instructor_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-12-1 : by YourName
 */

#include "v3d_message.h"
#include <vector>
#include "Neurite_Instructor_plugin.h"
#include <qlist.h>
#include <iostream>
#include "classification.h"
#include "mean_shift_fun.h"
#include "ui_Neurite_Instructor.h"
#include "neuriteinstructorui.h"
#include <ctime>


using namespace std;

Q_EXPORT_PLUGIN2(Neurite_Instructor, NeuriteInstructor);
 
QStringList NeuriteInstructor::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList NeuriteInstructor::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void NeuriteInstructor::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        neuriteInstructorUI* inputForm = new neuriteInstructorUI(0, &callback);
        inputForm->exec();

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

        unsigned char* ImgPtr = p4DImage->getRawData();

        V3DLONG imgX = p4DImage->getXDim();
        V3DLONG imgY = p4DImage->getYDim();
        V3DLONG imgZ = p4DImage->getZDim();
        V3DLONG channel = p4DImage->getCDim();


        clock_t startTime, endTime;
        startTime = clock();

        const string deployCString = inputForm->deployName.toStdString();
        const string modelCString = inputForm->modelName.toStdString();
        const string meanCString = inputForm->meanName.toStdString();
        cout << deployCString << " " << modelCString << " " << meanCString << endl;
        //v3d_msg("continue");
        Classifier classifier(deployCString, modelCString, meanCString);

        LandmarkList markerList = inputForm->markerList;
        std::vector<std::vector<float> > outputs_overall;
        NeuronTree newTree;
        for (LandmarkList::iterator markerIt=markerList.begin(); markerIt!=markerList.end(); ++markerIt)
        {
            //qDebug() << markerIt->x << " " << markerIt->y << " " << markerIt->z;
            int markerX = int(floor(markerIt->x)) - 1;
            int markerY = int(floor(markerIt->y)) - 1;
            int z = int(floor(markerIt->z)) - 1;

            NeuronSWC newNode;
            newNode.x = markerIt->x;
            newNode.y = markerIt->y;
            newNode.z = markerIt->z;
            newNode.parent = -1;
            newTree.listNeuron.push_back(newNode);

            V3DLONG VOIxyz[4];
            VOIxyz[0] = 61;
            VOIxyz[1] = 61;
            VOIxyz[2] = imgZ;
            VOIxyz[3] = channel;
            V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
            unsigned char* VOIPtr = new unsigned char[VOIsz];
            int xlb = markerX - 30;
            int xhb = markerX + 30;
            int ylb = markerY - 30;
            int yhb = markerY + 30;
            if (xlb < 1) xlb = 1;
            if (xhb > imgX) xhb = imgX;
            if (ylb < 1) ylb = 1;
            if (yhb > imgY) yhb = imgY;
            this->cropStack(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, 1, imgZ, imgX, imgY, imgZ);

            V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
            unsigned char* blockarea = new unsigned char[ROIsz];
            this->maxIPStack(VOIPtr, blockarea, xlb, xhb, ylb, yhb, 1, imgZ);

            std::vector<std::vector<float> > outputs;
            std::vector<cv::Mat> imgs;
            cv::Mat img(VOIxyz[1], VOIxyz[0], CV_8UC1, blockarea);
            imgs.push_back(img);
            outputs = classifier.Predict(imgs);
            for(V3DLONG d = 0; d<outputs.size();d++)
                outputs_overall.push_back(outputs[d]);
            outputs.clear();

            imgs.clear();
            delete[] VOIPtr;
            delete[] blockarea;
        }

        for (int j = 0; j < outputs_overall.size(); j++)
        {
            std::vector<float> output = outputs_overall[j];
            cout << output.at(0) << " " << output.at(1) << " " << output.at(2) << endl;
            if (output.at(1) > output.at(0)  &&  output.at(1) > output.at(2))
            {
                //newTree.listNeuron[j].type = 2;
                markerList[j].color.r = 255;
                markerList[j].color.g = 0;
                markerList[j].color.b = 0;


            }
            else if (output.at(2) > output.at(0)  &&  output.at(2) > output.at(1))
            {
                markerList[j].color.r = 0;
                markerList[j].color.g = 0;
                markerList[j].color.b = 255;
                //newTree.listNeuron[j].type = 3;

            }
            else
            {
                markerList[j].color.r = 255;
                markerList[j].color.g = 255;
                markerList[j].color.b = 255;
            }
        }

        callback.setLandmark(curwin,markerList);
        callback.pushObjectIn3DWindow(curwin);

//        QString outswc_file = "/local2/MK/DL_work/IVSCC/exampleImages/test.swc";
//        writeSWC_file(outswc_file, newTree);

        endTime = clock();
        double totalTime = double(endTime - startTime) / CLOCKS_PER_SEC;
        cout << "time elapsed: " << totalTime << " secs" << endl;

        inputForm->~neuriteInstructorUI();
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-12-1"));
	}
}

bool NeuriteInstructor::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("neurite_sampler"))
	{
        QString model_file = inparas.at(0);
        QString trained_file = inparas.at(1);
        QString mean_file = inparas.at(2);
        QString outswc_file = outfiles.at(0);

        int Sxy = 4;

        QList<ImageMarker> inputMarkers;
        inputMarkers = readMarker_file(infiles.at(1));

        unsigned char* ImgPtr = 0;
        V3DLONG in_sz[4];
        int datatype;
        if (!simple_loadimage_wrapper(callback, infiles.at(0), ImgPtr, in_sz, datatype))
        {
            cerr << "Error reading image file [" << infiles.at(0) << "]. Exit." << endl;
            return true;
        }
        int imgX = in_sz[0];
        int imgY = in_sz[1];
        int imgZ = in_sz[2];
        int channel = in_sz[3];
        V3DLONG start_z = 0;

        NeuronTree sampledTree;
        int zhb = imgZ;
        for (QList<ImageMarker>::iterator markerIt=inputMarkers.begin(); markerIt!=inputMarkers.end(); ++markerIt)
        {
            //qDebug() << markerIt->x << " " << markerIt->y << " " << markerIt->z;
            int markerX = int(floor(markerIt->x)) - 1;
            int markerY = int(floor(markerIt->y)) - 1;
            int z = int(floor(markerIt->z)) - 1;

            V3DLONG VOIxyz[4];
            VOIxyz[0] = 97;
            VOIxyz[1] = 97;
            VOIxyz[2] = imgZ;
            VOIxyz[3] = channel;
            V3DLONG VOIsz = VOIxyz[0] * VOIxyz[1] * VOIxyz[2];
            unsigned char* VOIPtr = new unsigned char[VOIsz];
            int xlb = markerX - 48;
            int xhb = markerX + 48;
            int ylb = markerY - 48;
            int yhb = markerY + 48;
            this->cropStack(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, 1, zhb, imgX, imgY, imgZ);

            V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
            unsigned char* blockarea = new unsigned char[ROIsz];
            this->maxIPStack(VOIPtr, blockarea, xlb, xhb, ylb, yhb, 1, zhb);

            std::vector<std::vector<float> > detection_results;
            LandmarkList marklist_2D;
            Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
            detection_results = batch_detection(blockarea,classifier,97,97,1,Sxy);
            V3DLONG d = 0;
            for(V3DLONG iiy = 0+Sxy; iiy < 98; iiy = iiy+Sxy)
            {
                for(V3DLONG iix = 0+Sxy; iix < 98; iix = iix+Sxy)
                {
                    std::vector<float> output = detection_results[d];
                    if(output.at(1) > output.at(0)  &&  output.at(1) > output.at(2))
                    {
                        LocationSimple S;
                        S.x = iix;
                        S.y = iiy;
                        S.z = 1;
                        S.category = 2;
                        marklist_2D.push_back(S);
                    }
                    else if (output.at(2) > output.at(0)  &&  output.at(2) > output.at(1))
                    {
                        LocationSimple S;
                        S.x = iix;
                        S.y = iiy;
                        S.z = 1;
                        S.category = 3;
                        marklist_2D.push_back(S);
                    }
                    //cout << output.at(0) << " " << output.at(1) << " " << output.at(2) << " " << endl;
                    d++;
                }
            }

            mean_shift_fun fun_obj;
            LandmarkList marklist_2D_shifted;
            vector<V3DLONG> poss_landmark;
            vector<float> mass_center;
            double windowradius = Sxy+5;

            V3DLONG sz_img[4];
            sz_img[0] = 97; sz_img[1] = 97; sz_img[2] = 1; sz_img[3] = 1;
            fun_obj.pushNewData<unsigned char>((unsigned char*)blockarea, sz_img);
            poss_landmark=landMarkList2poss(marklist_2D, sz_img[0], sz_img[0]*sz_img[1]);

            for (V3DLONG j=0;j<poss_landmark.size();j++)
            {
                mass_center=fun_obj.mean_shift_center_mass(poss_landmark[j],windowradius);
                LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
                tmp.category = marklist_2D[j].category;
                marklist_2D_shifted.append(tmp);
            }

            QList <ImageMarker> marklist_3D;
            ImageMarker S;
            for(V3DLONG i = 0; i < marklist_2D_shifted.size(); i++)
            {
                V3DLONG ix_2D = marklist_2D_shifted.at(i).x;
                V3DLONG iy_2D = marklist_2D_shifted.at(i).y;
                double I_max = 0;
                double I_sum = 0;
                V3DLONG iz_2D;
                for(V3DLONG j = 0; j < imgZ-start_z; j++)
                {
                    I_sum += VOIPtr[j*sz_img[1]*sz_img[0] + iy_2D*sz_img[0] + ix_2D];
                    if(VOIPtr[j*sz_img[1]*sz_img[0] + iy_2D*sz_img[0] + ix_2D] >= I_max)
                    {
                        I_max = VOIPtr[j*sz_img[1]*sz_img[0] + iy_2D*sz_img[0] + ix_2D];
                        iz_2D = j;
                    }

                }
                S.x = ix_2D;
                S.y = iy_2D;
                S.z = iz_2D;
                S.color.r = 255;
                S.color.g = 0;
                S.color.b = 0;
                S.type = marklist_2D_shifted[i].category;
                marklist_3D.append(S);
            }

            QList <ImageMarker> marklist_3D_pruned = batch_deletion(VOIPtr,classifier,marklist_3D,97,97,imgZ-start_z);
            if(marklist_3D_pruned.size()>0)
            {
                for(V3DLONG i = 0; i < marklist_3D_pruned.size(); i++)
                {
                    if(marklist_3D_pruned.at(i).radius > 0.9)  //was 0.995
                    {
                        V3DLONG ix = marklist_3D_pruned.at(i).x + markerX - 48;
                        V3DLONG iy = marklist_3D_pruned.at(i).y + markerY - 48;
                        V3DLONG iz = marklist_3D_pruned.at(i).z;
                        int type = marklist_3D_pruned.at(i).type;

                        NeuronSWC n;
                        n.x = ix-1;
                        n.y = iy-1;
                        n.z = iz-1+start_z;
                        n.n = i+1;
                        n.type = type;
                        //cout << n.type << " ";
                        n.r = marklist_3D_pruned.at(i).radius;
                        n.pn = -1; //so the first one will be root
                        sampledTree.listNeuron << n;
                    }
                }
                //cout << endl;
            }
            if(blockarea) {delete []blockarea; blockarea =0;}
            if(VOIPtr) {delete []VOIPtr; VOIPtr = 0;}
            cout << "complete 1 marker" << endl;
        }
        delete [] ImgPtr;
        writeSWC_file(outswc_file, sampledTree);
        sampledTree.listNeuron.clear();
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

