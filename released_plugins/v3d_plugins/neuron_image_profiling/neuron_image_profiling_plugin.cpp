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

		string caseNum = folderPath.substr(folderPath.length() - 9, 9);
		//cout << caseNum << endl;
		string inputImgName = "Y:\\mips\\";
		inputImgName = inputImgName + caseNum + "_ds4.v3draw";
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

			if (!swcFileBaseName.compare(13, 14, "_estimated_seg"))
			{
				cout << swcFileBaseName << endl;
				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				if (nt.listNeuron.at(0).type == 2) segmentListAxonEst.push_back(nt.listNeuron);
				else if (nt.listNeuron.at(0).type == 3) segmentListDendriteEst.push_back(nt.listNeuron);
			}
			else if (!swcFileBaseName.compare(14, 6, "manual"))
			{
				cout << swcFileBaseName << endl;
				NeuronTree nt = readSWC_file(swcList.at(i).absoluteFilePath());
				if (nt.listNeuron.at(0).type == 2) segmentListAxonMan.push_back(nt.listNeuron);
				else if (nt.listNeuron.at(0).type == 3) segmentListDendriteMan.push_back(nt.listNeuron);
			}
		}

		vector<basicSegmentROIStats> axonEstProfiles;
		vector<basicSegmentROIStats> dendriteEstProfiles;
		vector<basicSegmentROIStats> axonManProfiles;
		vector<basicSegmentROIStats> dendriteManProfiles;
		vector<double> axonTubMeans, axonTubStds, axonSegSNRs, axonSegCNRs;
		vector<double> denTubMeans, denTubStds, denSegSNRs, denSegCNRs;
		
		if (segmentListAxonEst.size() > 0)
		{
			axonEstProfiles = compute_metricsSegment(inputImgPtr, &segmentListAxonEst, callback);
			//cout << axonEstProfiles.size() << endl;
			
			for (vector<basicSegmentROIStats>::iterator it = axonEstProfiles.begin(); it != axonEstProfiles.end(); ++it)
			{
				axonTubMeans.push_back(it->tubularityMean);
				axonTubStds.push_back(it->tubularityStd);
				axonSegSNRs.push_back(it->SNR);
				axonSegCNRs.push_back(it->CNR);
			}
		}
		
		if (segmentListDendriteEst.size() > 0)
		{
			dendriteEstProfiles = compute_metricsSegment(inputImgPtr, &segmentListDendriteEst, callback);
			//cout << denEstProfiles.size() << endl;

			for (vector<basicSegmentROIStats>::iterator it = dendriteEstProfiles.begin(); it != dendriteEstProfiles.end(); ++it)
			{
				denTubMeans.push_back(it->tubularityMean);
				denTubStds.push_back(it->tubularityStd);
				denSegSNRs.push_back(it->SNR);
				denSegCNRs.push_back(it->CNR);
			}
		}

		double axonEstTubMeanAll = accumulate(axonTubMeans.begin(), axonTubMeans.end(), 0.0) / axonTubMeans.size();
		double axonEstTubStdAll = 0;
		for (vector<double>::iterator it = axonTubMeans.begin(); it != axonTubMeans.end(); ++it)
			axonEstTubStdAll = axonEstTubStdAll + (*it - axonEstTubMeanAll) * (*it - axonEstTubMeanAll);
		axonEstTubStdAll = axonEstTubStdAll / axonTubMeans.size();

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

		double denEstTubMeanAll = accumulate(denTubMeans.begin(), denTubMeans.end(), 0.0) / denTubMeans.size();
		double denEstTubStdAll = 0;
		for (vector<double>::iterator it = denTubMeans.begin(); it != denTubMeans.end(); ++it)
			denEstTubStdAll = denEstTubStdAll + (*it - denEstTubMeanAll) * (*it - denEstTubMeanAll);
		denEstTubStdAll = denEstTubStdAll / denTubMeans.size();

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

		cout << axonEstTubMeanAll << " " << axonEstTubStdAll << " " << axonEstSNRmean << " " << axonEstSNRstd << " " << axonEstCNRmean << " " << axonEstCNRstd << endl;
		cout << denEstTubMeanAll << " " << denEstTubStdAll << " " << denEstSNRmean << " " << denEstSNRstd << " " << denEstCNRmean << " " << denEstCNRstd << endl;

		string saveRoot = "Y:\\caseSummary\\";
		string saveFilePath = saveRoot + caseNum + ".csv";
		ofstream outputFile(saveFilePath);
		outputFile << axonEstTubMeanAll << "," << axonEstTubStdAll << "," << axonEstSNRmean << "," << axonEstSNRstd << "," << axonEstCNRmean << "," << axonEstCNRstd << endl;
		outputFile << denEstTubMeanAll << "," << denEstTubStdAll << "," << denEstSNRmean << "," << denEstSNRstd << "," << denEstCNRmean << "," << denEstCNRstd << endl;

	}
	else if (func_name == tr("help"))
	{
                printHelp(input, output);
	}
    else
        return false;

	return true;
}

