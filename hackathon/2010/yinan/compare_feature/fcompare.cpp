#define VSIZE 22

#include "compute.h"
#include "sim_measure.h"

#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

QString parsedir(QString str);

bool compare_feature(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	char * in = (*(vector<char*>*)(input.at(0).p)).at(0);
	char * out = (*(vector<char*>*)(output.at(0).p)).at(0);

	//parse input linker file
	QString infileName = QString(in);
	QFile linker(infileName);
	if (!linker.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cerr<<"Cannot find linker file"<<endl;
		return false;
	}

	QString dir = parsedir(infileName);
	
	QList<QString> nameList = QList<QString>();
	QTextStream files(&linker);
	QString line;
	line = files.readLine();
	while (!line.isNull())
	{
		if (!line.startsWith("SWCFILE=")) {line = files.readLine(); continue;}
		line.remove(0,8);
		nameList.append(line);
		line = files.readLine();
	}
	linker.close();
		
	//parse parameters
	vector<char*>* par = (vector<char*>*)(input.at(1).p);
	if (par->size()!=2)
	{
		cerr<<"Wrong parameter list!"<<endl;
		return false;
	}

	bool ok;
	int queryid = QString(par->at(0)).toInt(&ok, 10);
	if (!ok)
	{
		cerr<<"Error in query id!"<<endl;
		return false;
	}
	cout<<"query id:\t"<<queryid<<endl;

	int sbjnum = QString(par->at(1)).toInt(&ok, 10);
	if (!ok)
	{
		cerr<<"Error in subject number!"<<endl;
		return false;
	}
	cout<<"subject number:\t"<<sbjnum<<endl;

	//parse output file
	QString outfileName = QString(out);
	QFile file(outfileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		v3d_msg(QString("cannot open file %1").arg(outfileName));
	QTextStream myfile(&file);

	int neuronNum = nameList.size();
	QList<double*> featureList = QList<double*>();
	double * qf;
	QString queryName;

	for (int i=0;i<neuronNum;i++)
	{
		QString name = dir+nameList.at(i);
		NeuronTree nt = readSWC_file(name);
		
		cout<<"--------------Neuron #"<<(i+1)<<"----------------\n";
		cout<<nameList.at(i).toStdString()<<endl;
		double * features = new double[VSIZE];
		computeFeature(nt, features);
		for (int jj=0;jj<VSIZE;jj++)
			cout<<features[jj]<<"\t";
		cout<<endl;
		featureList.append(features);
	}

	sbjnum = neuronNum;
	for (int i=0;i<neuronNum;i++)
		myfile<<"\t"<<nameList.at(i);
	myfile<<endl;

	for (queryid = 0;queryid<neuronNum;queryid++)
	{
		qf = featureList.at(queryid);
		queryName = nameList.at(queryid);
		//featureList.removeAt(queryid);
		//nameList.removeAt(queryid);

		//sbjnum = 2;
		int* sbj = new int[sbjnum];
		double * score = new double[sbjnum];
		bool result = unitVar(qf, featureList, sbjnum, sbj, VSIZE, score);

		//generate output files
		if (result)
		{
			//myfile<<"query id:\t"<<queryid<<endl;
			//myfile<<"query name:\t"<<queryName<<endl;
			//myfile<<"pick:\t"<<sbjnum<<endl;
			//myfile<<"rank\tid\tfile name\tscore"<<endl;
			myfile<<queryName<<"\t";
			for (int i=0;i<sbjnum;i++)
			{
				int pp;
				for (pp=0;pp<sbjnum;pp++)
					if (sbj[pp]==i) break;
				myfile<<(-score[pp])<<"\t";
			}
			myfile<<endl;
			/*
			//generate linker files under input directory
			int i = 1;
			//for (int i=0;i<sbjnum;i++)
			{
				QString filename_out = dir+QString("compare_feature_result_%1").arg(queryid+1)+".ano";
				QFile* outlinker=new QFile(filename_out);
				if (!outlinker->open(QIODevice::WriteOnly | QIODevice::Text))
					v3d_msg(QString("cannot open file: %1").arg(filename_out));
				QTextStream outs(outlinker);
				outs<<"SWCFILE="<<queryName<<endl;
				outs<<"SWCFILE="<<nameList.at(sbj[i])<<endl;
				outlinker->close();
				v3d_msg(QString("Save the linker file to: \n\n%1\n\nComplete!").arg(filename_out),0);
			}*/
		}
		delete score; score=NULL;
		delete sbj; sbj=NULL;
	}
	file.close();
	return true;
}

QString parsedir(QString str)
{
	int parser;
	for (parser=str.length()-1;parser>=0;parser--)
		if (str.at(parser)==QChar('/')) break;
	str.chop(str.length()-parser-1);
	return str;
}
