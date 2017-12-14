#include <iostream>
#include <vector>
#include <ctime>

#include "v3d_message.h"
#include "building_tester.h"
#include "classification.h"
#include "mean_shift_fun.h"
#include "..\Neurite_Instructor\funcs.h"

using namespace std;

void BuildingTester::test1()
{
	QString model_file = inputStrings[0];
	QString trained_file = inputStrings[1];
	QString mean_file = inputStrings[2];
	QString outswc_file = outputStrings[0];

	int Sxy = 4;

	QList<ImageMarker> inputMarkers;
	inputMarkers = readMarker_file(inputFileStrings[1]);

	unsigned char* ImgPtr = 0;
	V3DLONG in_sz[4];
	int datatype;
	if (!simple_loadimage_wrapper(*theCallbackPtr, inputFileStrings[0].toStdString().c_str(), ImgPtr, in_sz, datatype))
	{
		cerr << "Error reading image file [" << inputFileStrings[0].toStdString() << "]. Exit." << endl;
		return;
	}
	int imgX = in_sz[0];
	int imgY = in_sz[1];
	int imgZ = in_sz[2];
	int channel = in_sz[3];
	V3DLONG start_z = 0;

	NeuronTree sampledTree;
	int zhb = imgZ;
	for (QList<ImageMarker>::iterator markerIt = inputMarkers.begin(); markerIt != inputMarkers.end(); ++markerIt)
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
		cropStack(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, 1, zhb, imgX, imgY, imgZ);

		V3DLONG ROIsz = VOIxyz[0] * VOIxyz[1];
		unsigned char* blockarea = new unsigned char[ROIsz];
		maxIPStack(VOIPtr, blockarea, xlb, xhb, ylb, yhb, 1, zhb);

		std::vector<std::vector<float> > detection_results;
		LandmarkList marklist_2D;
		Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());
		detection_results = batch_detection(blockarea, classifier, 97, 97, 1, Sxy);
		V3DLONG d = 0;
		for (V3DLONG iiy = 0 + Sxy; iiy < 98; iiy = iiy + Sxy)
		{
			for (V3DLONG iix = 0 + Sxy; iix < 98; iix = iix + Sxy)
			{
				std::vector<float> output = detection_results[d];
				if (output.at(1) > output.at(0) && output.at(1) > output.at(2))
				{
					LocationSimple S;
					S.x = iix;
					S.y = iiy;
					S.z = 1;
					S.category = 2;
					marklist_2D.push_back(S);
				}
				else if (output.at(2) > output.at(0) && output.at(2) > output.at(1))
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
		double windowradius = Sxy + 5;

		V3DLONG sz_img[4];
		sz_img[0] = 97; sz_img[1] = 97; sz_img[2] = 1; sz_img[3] = 1;
		fun_obj.pushNewData<unsigned char>((unsigned char*)blockarea, sz_img);
		poss_landmark = landMarkList2poss(marklist_2D, sz_img[0], sz_img[0] * sz_img[1]);

		for (V3DLONG j = 0; j<poss_landmark.size(); j++)
		{
			mass_center = fun_obj.mean_shift_center_mass(poss_landmark[j], windowradius);
			LocationSimple tmp(mass_center[0] + 1, mass_center[1] + 1, mass_center[2] + 1);
			tmp.category = marklist_2D[j].category;
			marklist_2D_shifted.append(tmp);
		}

		QList <ImageMarker> marklist_3D;
		ImageMarker S;
		for (V3DLONG i = 0; i < marklist_2D_shifted.size(); i++)
		{
			V3DLONG ix_2D = marklist_2D_shifted.at(i).x;
			V3DLONG iy_2D = marklist_2D_shifted.at(i).y;
			double I_max = 0;
			double I_sum = 0;
			V3DLONG iz_2D;
			for (V3DLONG j = 0; j < imgZ - start_z; j++)
			{
				I_sum += VOIPtr[j*sz_img[1] * sz_img[0] + iy_2D*sz_img[0] + ix_2D];
				if (VOIPtr[j*sz_img[1] * sz_img[0] + iy_2D*sz_img[0] + ix_2D] >= I_max)
				{
					I_max = VOIPtr[j*sz_img[1] * sz_img[0] + iy_2D*sz_img[0] + ix_2D];
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

		QList <ImageMarker> marklist_3D_pruned = batch_deletion(VOIPtr, classifier, marklist_3D, 97, 97, imgZ - start_z);
		if (marklist_3D_pruned.size()>0)
		{
			for (V3DLONG i = 0; i < marklist_3D_pruned.size(); i++)
			{
				if (marklist_3D_pruned.at(i).radius > 0.9)  //was 0.995
				{
					V3DLONG ix = marklist_3D_pruned.at(i).x + markerX - 48;
					V3DLONG iy = marklist_3D_pruned.at(i).y + markerY - 48;
					V3DLONG iz = marklist_3D_pruned.at(i).z;
					int type = marklist_3D_pruned.at(i).type;

					NeuronSWC n;
					n.x = ix - 1;
					n.y = iy - 1;
					n.z = iz - 1 + start_z;
					n.n = i + 1;
					n.type = type;
					//cout << n.type << " ";
					n.r = marklist_3D_pruned.at(i).radius;
					n.pn = -1; //so the first one will be root
					sampledTree.listNeuron << n;
				}
			}
			//cout << endl;
		}
		if (blockarea) { delete[]blockarea; blockarea = 0; }
		if (VOIPtr) { delete[]VOIPtr; VOIPtr = 0; }
		cout << "complete 1 marker" << endl;
	}
	delete[] ImgPtr;
	writeSWC_file(outswc_file, sampledTree);
	sampledTree.listNeuron.clear();






}