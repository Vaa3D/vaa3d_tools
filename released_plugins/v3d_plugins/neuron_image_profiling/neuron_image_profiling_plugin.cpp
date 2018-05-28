/* neuron_image_profiling_plugin.cpp
 * This is a plugin for profiling image qualities for neuron tracing, via SNR and tubuliarty measures.
 * Given an input swc with labled neurite types (soma, dendrite and axon), generate dynamic range and SNR
 * 2015-7-6 : by Xiaoxiao Liu and Zhi Zhou
 */

#include "v3d_message.h"
#include <vector>
#include "neuron_image_profiling_plugin.h"
#include "profile_swc.h"
#include <iostream>
#include <sstream>

#include <fstream>
#include <string>
#include <numeric>

#include <qdir.h>

//#include "basic_4dimage.h"

using namespace std;

Q_EXPORT_PLUGIN2(neuron_image_profiling, image_profiling);
 
QStringList image_profiling::menulist() const
{
	return QStringList() 
		<<tr("profile_swc")
		<<tr("about");
}

QStringList image_profiling::funclist() const
{
	return QStringList()
		<<tr("profile_swc")
		<< tr("seg_separate")
		<< tr("segmentBasedProfile")
		<<tr("help");
}

void image_profiling::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("profile_swc"))
	{
		if (!	profile_swc_menu(callback, parent))
		{
                     std::cout << "error in profile_swc_menu() " << std::endl;
		}  
	}
	else 
	{
		printHelp(callback, parent);
	}
}

bool image_profiling::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("profile_swc"))
	{	
                    profile_swc_func(callback,input, output);
	}
	else if (func_name == tr("seg_separate"))
	{
		vector<char*> infiles, inparas, outfiles;
		if (input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
		if (input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
		if (output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

		QString folderPathQ = infiles.at(0);
		string folderPath = folderPathQ.toStdString();
		QDir directory;
		directory.setPath(folderPathQ);
		QFileInfoList swcList = directory.entryInfoList();

		string saveRoot;
		if (outfiles.size() > 0) saveRoot = outfiles.at(0);

		string caseNum = folderPath.substr(folderPath.length() - 9, 9);
		//cout << caseNum << endl;
		string imageRoot = "Y:\\mips\\";
		string inputImgName = imageRoot + caseNum + "_ds4.v3draw";
		const char* inputImgNameC = inputImgName.c_str();
		char* inputImgNameChar = new char[inputImgName.length()];
		strcpy(inputImgNameChar, inputImgNameC);
		Image4DSimple* inputImgPtr = callback.loadImage(inputImgNameChar);
		if (!inputImgPtr->convert_to_UINT8())
		{
			cout << "Error in converting data into  UINT8 type." << endl;
			return false;
		}

		for (int i = 0; i < swcList.size(); ++i)
		{
			string swcFileBaseName = swcList.at(i).baseName().toStdString();
			string swcFileSuffix = swcList.at(i).suffix().toStdString();

			if (!swcFileBaseName.compare("") || !swcFileBaseName.compare(".") || !swcFileBaseName.compare("..")) continue;
			if (!swcFileBaseName.compare(swcFileBaseName.length() - 3, 3, "ds4") || !swcFileSuffix.compare("v3draw")) continue;

			if (!swcFileBaseName.compare(10, 8, "axon_bad"))
			{
				string saveRoot = "Y:\\forComputeScores\\axon_bad";
				string saveRootEst = saveRoot + "\\" + caseNum + ".csv";
				ofstream outputFileEst(saveRootEst);

				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				vector<QList<NeuronSWC>> segmentListEst;	 IMAGE_METRICS segEstMetrics;
				segmentListEst.push_back(nt.listNeuron);

				vector<basicSegmentROIStats> segProfiles;
				vector<double> tubMeans, tubStds, tubSegMeans, segSNRs, segCNRs;
				segProfiles = compute_metricsSegment(inputImgPtr, &segmentListEst, callback);
				//cout << axonEstProfiles.size() << endl;

				outputFileEst << segProfiles.at(0).segTub << "," << segProfiles.at(0).SNR << "," << segProfiles.at(0).CNR << endl;
			}
			else if (!swcFileBaseName.compare(10, 9, "axon_good"))
			{
				string saveRoot = "Y:\\forComputeScores\\axon_good";
				string saveRootEst = saveRoot + "\\" + caseNum + ".csv";
				ofstream outputFileEst(saveRootEst);

				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				vector<QList<NeuronSWC>> segmentListEst;	 IMAGE_METRICS segEstMetrics;
				segmentListEst.push_back(nt.listNeuron);

				vector<basicSegmentROIStats> segProfiles;
				vector<double> tubMeans, tubStds, tubSegMeans, segSNRs, segCNRs;
				segProfiles = compute_metricsSegment(inputImgPtr, &segmentListEst, callback);
				//cout << axonEstProfiles.size() << endl;

				outputFileEst << segProfiles.at(0).segTub << "," << segProfiles.at(0).SNR << "," << segProfiles.at(0).CNR << endl;
			}
			else if (!swcFileBaseName.compare(10, 13, "dendrite_good"))
			{
				string saveRoot = "Y:\\forComputeScores\\dendrite_good";
				string saveRootEst = saveRoot + "\\" + caseNum + ".csv";
				ofstream outputFileEst(saveRootEst);

				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				vector<QList<NeuronSWC>> segmentListEst;	 IMAGE_METRICS segEstMetrics;
				segmentListEst.push_back(nt.listNeuron);

				vector<basicSegmentROIStats> segProfiles;
				vector<double> tubMeans, tubStds, tubSegMeans, segSNRs, segCNRs;
				segProfiles = compute_metricsSegment(inputImgPtr, &segmentListEst, callback);
				//cout << axonEstProfiles.size() << endl;

				outputFileEst << segProfiles.at(0).segTub << "," << segProfiles.at(0).SNR << "," << segProfiles.at(0).CNR << endl;
			}
			else if (!swcFileBaseName.compare(10, 12, "dendrite_bad"))
			{
				string saveRoot = "Y:\\forComputeScores\\dendrite_bad";
				string saveRootEst = saveRoot + "\\" + caseNum + ".csv";
				ofstream outputFileEst(saveRootEst);

				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				vector<QList<NeuronSWC>> segmentListEst;	 IMAGE_METRICS segEstMetrics;
				segmentListEst.push_back(nt.listNeuron);

				vector<basicSegmentROIStats> segProfiles;
				vector<double> tubMeans, tubStds, tubSegMeans, segSNRs, segCNRs;
				segProfiles = compute_metricsSegment(inputImgPtr, &segmentListEst, callback);
				//cout << axonEstProfiles.size() << endl;

				outputFileEst << segProfiles.at(0).segTub << "," << segProfiles.at(0).SNR << "," << segProfiles.at(0).CNR << endl;
			}
		}
	}
	else if (func_name == tr("segmentBasedProfile"))
	{
		vector<char*> infiles, inparas, outfiles;
		if (input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
		if (input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
		if (output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

		QString folderPathQ = infiles.at(0);
		string folderPath = folderPathQ.toStdString();
		QDir directory;
		directory.setPath(folderPathQ);
		QFileInfoList swcList = directory.entryInfoList();

		string saveRoot;
		if (outfiles.size() > 0) saveRoot = outfiles.at(0);

		string caseNum = folderPath.substr(folderPath.length() - 9, 9);
		//cout << caseNum << endl;
		string inputImgName = folderPath + "\\" + caseNum + "_ds4.v3draw";
		const char* inputImgNameC = inputImgName.c_str();
		char* inputImgNameChar = new char[inputImgName.length()];
		strcpy(inputImgNameChar, inputImgNameC);
		Image4DSimple* inputImgPtr = callback.loadImage(inputImgNameChar);
		if (!inputImgPtr->convert_to_UINT8())
		{
			cout << "Error in converting data into  UINT8 type." << endl;
			return false;
		}

		vector<QList<NeuronSWC>> segmentListAxonEst;	 IMAGE_METRICS axonEstMetrics;
		vector<QList<NeuronSWC>> segmentListDendriteEst; IMAGE_METRICS dendriteEstMetrics;
		vector<QList<NeuronSWC>> segmentListAxonMan;     IMAGE_METRICS axonManMetrics;
		vector<QList<NeuronSWC>> segmentListDendriteMan; IMAGE_METRICS dendriteManMetrics;

		segmentListAxonEst.clear();
		segmentListDendriteEst.clear();
		segmentListAxonMan.clear();
		segmentListDendriteMan.clear();
		for (int i = 0; i < swcList.size(); ++i)
		{
			string swcFileBaseName = swcList.at(i).baseName().toStdString();
			string swcFileSuffix = swcList.at(i).suffix().toStdString();

			if (!swcFileBaseName.compare("") || !swcFileBaseName.compare(".") || !swcFileBaseName.compare("..")) continue;
			if (!swcFileBaseName.compare(swcFileBaseName.length() - 3, 3, "ds4") || !swcFileSuffix.compare("v3draw")) continue;

			if (!swcFileBaseName.compare(10, 14, "_estimated_seg"))
			{
				stringstream numStrStream;
				int chari = 27;
				while (chari <= swcFileBaseName.length() - 1)
				{	
					numStrStream << swcFileBaseName[chari];
					//cout << swcFileBaseName[chari] << endl;
					++chari;
				}
				string segNum;
				numStrStream >> segNum;
				//cout << segNum << endl;

				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				string manualFileName = swcList.at(i).absolutePath().toStdString();
				manualFileName = manualFileName + "\\" + caseNum + "_ds4_resampled_manualtraced" + segNum + ".swc";
				QString manualFileNameQ = QString::fromStdString(manualFileName);
				NeuronTree typeCheckNt = readSWC_file(manualFileNameQ);

				if (typeCheckNt.listNeuron.at(0).type == 2) segmentListAxonEst.push_back(nt.listNeuron);
				else if (typeCheckNt.listNeuron.at(0).type == 3) segmentListDendriteEst.push_back(nt.listNeuron);
			}
			else if (!swcFileBaseName.compare(14, 9, "resampled"))
			{
				//cout << swcFileBaseName << endl;
				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				for (QList<NeuronSWC>::iterator it = nt.listNeuron.begin(); it != nt.listNeuron.end(); ++it) it->radius = 2;

				if (nt.listNeuron.at(0).type == 2) segmentListAxonMan.push_back(nt.listNeuron);
				else if (nt.listNeuron.at(0).type == 3) segmentListDendriteMan.push_back(nt.listNeuron);
			}
		}

		vector<basicSegmentROIStats> axonEstProfiles;
		vector<basicSegmentROIStats> dendriteEstProfiles;
		vector<basicSegmentROIStats> axonManProfiles;
		vector<basicSegmentROIStats> dendriteManProfiles;

		vector<double> axonTubMeans, axonTubStds, axonTubSegMeans, axonSegSNRs, axonSegCNRs;
		vector<double> denTubMeans, denTubStds, denTubSegMeans, denSegSNRs, denSegCNRs;
		string saveRootEst = saveRoot + "\\caseSummary_est\\";
		string saveFilePathEst = saveRootEst + caseNum + ".csv";
		ofstream outputFileEst(saveFilePathEst);
		string saveRootMan = saveRoot + "\\caseSummary_manual\\";
		string saveFilePathMan = saveRootMan + caseNum + ".csv";
		ofstream outputFileMan(saveFilePathMan);
		if (segmentListAxonEst.size() > 0)
		{
			axonEstProfiles = compute_metricsSegment(inputImgPtr, &segmentListAxonEst, callback);
			//cout << axonEstProfiles.size() << endl;
			
			axonTubMeans.clear();
			axonTubStds.clear();
			axonTubSegMeans.clear();
			axonSegSNRs.clear();
			axonSegCNRs.clear();
			for (vector<basicSegmentROIStats>::iterator it = axonEstProfiles.begin(); it != axonEstProfiles.end(); ++it)
			{
				axonTubMeans.push_back(it->tubularityMean);
				axonTubStds.push_back(it->tubularityStd);
				axonTubSegMeans.push_back(it->segTub);
				axonSegSNRs.push_back(it->SNR);
				axonSegCNRs.push_back(it->CNR);
			}

			double axonEstTubMeanAll = accumulate(axonTubMeans.begin(), axonTubMeans.end(), 0.0) / axonTubMeans.size();
			double axonEstTubStdAll = 0;
			for (vector<double>::iterator it = axonTubMeans.begin(); it != axonTubMeans.end(); ++it)
				axonEstTubStdAll = axonEstTubStdAll + (*it - axonEstTubMeanAll) * (*it - axonEstTubMeanAll);
			axonEstTubStdAll = axonEstTubStdAll / axonTubMeans.size();

			double axonEstTubMeanSegs = accumulate(axonTubSegMeans.begin(), axonTubSegMeans.end(), 0.0) / axonTubSegMeans.size();
			double axonEstTubStdSegs = 0;
			for (vector<double>::iterator it = axonTubSegMeans.begin(); it != axonTubSegMeans.end(); ++it)
				axonEstTubStdSegs = axonEstTubStdSegs + (*it - axonEstTubMeanSegs) * (*it - axonEstTubMeanSegs);
			axonEstTubStdSegs = axonEstTubStdSegs / axonTubSegMeans.size();

			double axonEstSNRmean = accumulate(axonSegSNRs.begin(), axonSegSNRs.end(), 0.0) / axonSegSNRs.size();
			double axonEstSNRstd = 0;
			for (vector<double>::iterator it = axonSegSNRs.begin(); it != axonSegSNRs.end(); ++it)
				axonEstSNRstd = axonEstSNRstd + (*it - axonEstSNRmean) * (*it - axonEstSNRmean);
			axonEstSNRstd = axonEstSNRstd / axonSegSNRs.size();

			double axonEstCNRmean = accumulate(axonSegCNRs.begin(), axonSegCNRs.end(), 0.0) / axonSegCNRs.size();
			double axonEstCNRstd = 0;
			for (vector<double>::iterator it = axonSegCNRs.begin(); it != axonSegCNRs.end(); ++it)
				axonEstCNRstd = axonEstCNRstd + (*it - axonEstCNRmean) * (*it - axonEstCNRmean);
			axonEstCNRstd = axonEstCNRstd / axonSegCNRs.size();

			outputFileEst << axonEstTubMeanAll << "," << axonEstTubStdAll << "," << axonEstTubMeanSegs << "," << axonEstTubStdSegs << ","
				<< axonEstSNRmean << "," << axonEstSNRstd << "," << axonEstCNRmean << "," << axonEstCNRstd << "," << ",";
		}
		
		if (segmentListDendriteEst.size() > 0)
		{
			dendriteEstProfiles = compute_metricsSegment(inputImgPtr, &segmentListDendriteEst, callback);
			//cout << denEstProfiles.size() << endl;

			denTubMeans.clear();
			denTubStds.clear();
			denTubSegMeans.clear();
			denSegSNRs.clear();
			denSegCNRs.clear();
			for (vector<basicSegmentROIStats>::iterator it = dendriteEstProfiles.begin(); it != dendriteEstProfiles.end(); ++it)
			{
				denTubMeans.push_back(it->tubularityMean);
				denTubStds.push_back(it->tubularityStd);
				denTubSegMeans.push_back(it->segTub);
				denSegSNRs.push_back(it->SNR);
				denSegCNRs.push_back(it->CNR);
			}

			double denEstTubMeanAll = accumulate(denTubMeans.begin(), denTubMeans.end(), 0.0) / denTubMeans.size();
			double denEstTubStdAll = 0;
			for (vector<double>::iterator it = denTubMeans.begin(); it != denTubMeans.end(); ++it)
				denEstTubStdAll = denEstTubStdAll + (*it - denEstTubMeanAll) * (*it - denEstTubMeanAll);
			denEstTubStdAll = denEstTubStdAll / denTubMeans.size();

			double denEstTubMeanSegs = accumulate(denTubSegMeans.begin(), denTubSegMeans.end(), 0.0) / denTubSegMeans.size();
			double denEstTubStdSegs = 0;
			for (vector<double>::iterator it = denTubSegMeans.begin(); it != denTubSegMeans.end(); ++it)
				denEstTubStdSegs = denEstTubStdSegs + (*it - denEstTubMeanSegs) * (*it - denEstTubMeanSegs);
			denEstTubStdSegs = denEstTubStdSegs / denTubSegMeans.size();

			double denEstSNRmean = accumulate(axonSegSNRs.begin(), denSegSNRs.end(), 0.0) / denSegSNRs.size();
			double denEstSNRstd = 0;
			for (vector<double>::iterator it = denSegSNRs.begin(); it != denSegSNRs.end(); ++it)
				denEstSNRstd = denEstSNRstd + (*it - denEstSNRmean) * (*it - denEstSNRmean);
			denEstSNRstd = denEstSNRstd / denSegSNRs.size();

			double denEstCNRmean = accumulate(denSegCNRs.begin(), denSegCNRs.end(), 0.0) / denSegCNRs.size();
			double denEstCNRstd = 0;
			for (vector<double>::iterator it = denSegCNRs.begin(); it != denSegCNRs.end(); ++it)
				denEstCNRstd = denEstCNRstd + (*it - denEstCNRmean) * (*it - denEstCNRmean);
			denEstCNRstd = denEstCNRstd / denSegCNRs.size();

			outputFileEst << denEstTubMeanAll << "," << denEstTubStdAll << "," << denEstTubMeanSegs << "," << denEstTubStdSegs << ","
				<< denEstSNRmean << "," << denEstSNRstd << "," << denEstCNRmean << "," << denEstCNRstd << endl;
		}

		if (segmentListAxonMan.size() > 0)
		{
			axonManProfiles = compute_metricsSegment(inputImgPtr, &segmentListAxonMan, callback);
			//cout << axonManProfiles.size() << endl;

			axonTubMeans.clear();
			axonTubStds.clear();
			axonTubSegMeans.clear();
			axonSegSNRs.clear();
			axonSegCNRs.clear();

			double tubeMeanSum = 0;
			double tubeMax = 0;
			int segNum = 1;
			for (vector<basicSegmentROIStats>::iterator it = axonManProfiles.begin(); it != axonManProfiles.end(); ++it)
			{
				cout << "seg #:" << segNum << " ";

				axonTubMeans.push_back(it->tubularityMean);
				axonTubStds.push_back(it->tubularityStd);
				axonTubSegMeans.push_back(it->segTub);
				axonSegSNRs.push_back(it->SNR);
				axonSegCNRs.push_back(it->CNR);

				tubeMeanSum += it->tubularityMean;

				if (it->tubularityMean > tubeMax) tubeMax = it->tubularityMean;

				cout << it->tubularityMean << endl;
				++segNum;
			}
			cout << "max tubularity: " << tubeMax << endl;
			cout << "tube sum: " << tubeMeanSum << endl;
			cout << "number of tube means: " << axonManProfiles.size() << endl;

			double axonManTubMeanAll = accumulate(axonTubMeans.begin(), axonTubMeans.end(), 0.0) / axonTubMeans.size();
			double axonManTubStdAll = 0;
			for (vector<double>::iterator it = axonTubMeans.begin(); it != axonTubMeans.end(); ++it)
				axonManTubStdAll = axonManTubStdAll + (*it - axonManTubMeanAll) * (*it - axonManTubMeanAll);
			axonManTubStdAll = axonManTubStdAll / axonTubMeans.size();

			double axonManTubMeanSegs = accumulate(axonTubSegMeans.begin(), axonTubSegMeans.end(), 0.0) / axonTubSegMeans.size();
			double axonManTubStdSegs = 0;
			for (vector<double>::iterator it = axonTubSegMeans.begin(); it != axonTubSegMeans.end(); ++it)
				axonManTubStdSegs = axonManTubStdSegs + (*it - axonManTubMeanSegs) * (*it - axonManTubMeanSegs);
			axonManTubStdSegs = axonManTubStdSegs / axonTubSegMeans.size();

			double axonManSNRmean = accumulate(axonSegSNRs.begin(), axonSegSNRs.end(), 0.0) / axonSegSNRs.size();
			double axonManSNRstd = 0;
			for (vector<double>::iterator it = axonSegSNRs.begin(); it != axonSegSNRs.end(); ++it)
				axonManSNRstd = axonManSNRstd + (*it - axonManSNRmean) * (*it - axonManSNRmean);
			axonManSNRstd = axonManSNRstd / axonSegSNRs.size();

			double axonManCNRmean = accumulate(axonSegCNRs.begin(), axonSegCNRs.end(), 0.0) / axonSegCNRs.size();
			double axonManCNRstd = 0;
			for (vector<double>::iterator it = axonSegCNRs.begin(); it != axonSegCNRs.end(); ++it)
				axonManCNRstd = axonManCNRstd + (*it - axonManCNRmean) * (*it - axonManCNRmean);
			axonManCNRstd = axonManCNRstd / axonSegCNRs.size();

			outputFileMan << axonManTubMeanAll << "," << axonManTubStdAll << "," << axonManTubMeanSegs << "," << axonManTubStdSegs << ","
				<< axonManSNRmean << "," << axonManSNRstd << "," << axonManCNRmean << "," << axonManCNRstd << "," << ",";
		}

		if (segmentListDendriteMan.size() > 0)
		{
			dendriteManProfiles = compute_metricsSegment(inputImgPtr, &segmentListDendriteMan, callback);
			//cout << dendriteManProfiles.size() << endl;

			denTubMeans.clear();
			denTubStds.clear();
			denTubSegMeans.clear();
			denSegSNRs.clear();
			denSegCNRs.clear();
			for (vector<basicSegmentROIStats>::iterator it = dendriteManProfiles.begin(); it != dendriteManProfiles.end(); ++it)
			{
				denTubMeans.push_back(it->tubularityMean);
				denTubStds.push_back(it->tubularityStd);
				denTubSegMeans.push_back(it->segTub);
				denSegSNRs.push_back(it->SNR);
				denSegCNRs.push_back(it->CNR);
			}

			double denManTubMeanAll = accumulate(denTubMeans.begin(), denTubMeans.end(), 0.0) / denTubMeans.size();
			double denManTubStdAll = 0;
			for (vector<double>::iterator it = denTubMeans.begin(); it != denTubMeans.end(); ++it)
				denManTubStdAll = denManTubStdAll + (*it - denManTubMeanAll) * (*it - denManTubMeanAll);
			denManTubStdAll = denManTubStdAll / denTubMeans.size();

			double denManTubMeanSegs = accumulate(denTubSegMeans.begin(), denTubSegMeans.end(), 0.0) / denTubSegMeans.size();
			double denManTubStdSegs = 0;
			for (vector<double>::iterator it = denTubSegMeans.begin(); it != denTubSegMeans.end(); ++it)
				denManTubStdSegs = denManTubStdSegs + (*it - denManTubMeanSegs) * (*it - denManTubMeanSegs);
			denManTubStdSegs = denManTubStdSegs / denTubSegMeans.size();

			double denManSNRmean = accumulate(denSegSNRs.begin(), denSegSNRs.end(), 0.0) / denSegSNRs.size();
			double denManSNRstd = 0;
			for (vector<double>::iterator it = denSegSNRs.begin(); it != denSegSNRs.end(); ++it)
				denManSNRstd = denManSNRstd + (*it - denManSNRmean) * (*it - denManSNRmean);
			denManSNRstd = denManSNRstd / denSegSNRs.size();

			double denManCNRmean = accumulate(denSegCNRs.begin(), denSegCNRs.end(), 0.0) / denSegCNRs.size();
			double denManCNRstd = 0;
			for (vector<double>::iterator it = denSegCNRs.begin(); it != denSegCNRs.end(); ++it)
				denManCNRstd = denManCNRstd + (*it - denManCNRmean) * (*it - denManCNRmean);
			denManCNRstd = denManCNRstd / denSegCNRs.size();

			outputFileMan << denManTubMeanAll << "," << denManTubStdAll << "," << denManTubMeanSegs << "," << denManTubStdSegs << ","
				<< denManSNRmean << "," << denManSNRstd << "," << denManCNRmean << "," << denManCNRstd << endl;
		}
	}
	else if (func_name == tr("help"))
	{
                printHelp(input, output);
	}
    else
        return false;

	return true;
}

