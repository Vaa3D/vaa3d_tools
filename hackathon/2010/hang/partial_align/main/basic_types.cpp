#include <iostream>
#include <fstream>

#include "basic_types.h"

using namespace std;

static int split(const char *paras, char ** &args, char delim)
{
	int argc = 1;
	int len = strlen(paras);
	int posb[200];
	char * myparas = new char[len];
	strcpy(myparas, paras);

	for(int i = 1; i < len; i++){if(myparas[i] == delim) argc++;}

	args = new char*[argc];
	int index = 0;
	args[index++] = myparas;

	for(int i = 0; i < len; i++)
	{
		if(myparas[i]==delim){myparas[i]='\0';args[index++] = myparas + (i+1);}
	}
	return argc;
}

vector<MarkerType> readMarker_file(const string filename)
{
	vector <MarkerType> tmp_vec;

	ifstream ifs(filename.c_str());
	if(ifs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return tmp_vec;
	}

	V3DLONG k=0;
	while (ifs.good())
	{
		char curline[2000];
		ifs.getline(curline, sizeof(curline));
		k++;
		{
			if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

			char ** args;
			int argc = split(curline,args,',');
			if (argc<3){for(int i = 0; i < argc; i++) delete [] args[i];   continue;}

			MarkerType S;

			S.x = atoi(args[0]);
			S.y = atoi(args[1]);
			S.z = atoi(args[2]);
			//S.value = (argc >=4) ? atoi(args[3]) : -1;

			tmp_vec.push_back(S);
			//for(int i = 0; i < argc; i++) delete [] args[i];
		}
	}

	ifs.close();
	return tmp_vec;
}

bool writeMarker_file(const string filename, const vector <MarkerType> & vecMarker)
{
	ofstream ofs(filename.c_str());
	if(ofs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return false;
	}
	ofs<<"#x, y, z, radius, shape"<<endl;
	vector<MarkerType>::const_iterator it = vecMarker.begin();
	while(it != vecMarker.end())
	{
		MarkerType S = *it;
		ofs<<S.x<<","<<S.y<<","<<S.z;
		//if(S.value >= 0) ofs<<","<<S.value<<endl;
		//else 
			ofs<<endl;
		it++;
	}
	ofs.close();
	return true;
}

vector<FeatureType> readFeature_file(const string filename)
{
	vector<FeatureType> tmp_vec;

	ifstream ifs(filename.c_str());
	if(ifs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return tmp_vec;
	}

	//V3DLONG k=0;
	while (ifs.good())
	{
		char curline[2000];
		ifs.getline(curline, sizeof(curline));
		//k++;
		{
			if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

			char ** args;
			int argc = split(curline,args,',');
			if (argc<3){for(int i = 0; i < argc; i++) delete [] args[i];   continue;}

			FeatureType S;

			S.x = atof(args[0]);
			S.y = atof(args[1]);
			S.z = atof(args[2]);
			for(int i = 3; i < argc; i++)S.descriptor.push_back(atof(args[i]));

			tmp_vec.push_back(S);
			//for(int i = 0; i < argc; i++) delete [] args[i];
		}
	}

	ifs.close();
	return tmp_vec;
}

bool writeFeature_file(const string filename, const vector <FeatureType> & vecFeature)
{
	ofstream ofs(filename.c_str());
	if(ofs.fail())
	{
		cerr<<"open file ["<<filename<<"] failed!"<<endl;
		return false;
	}
	ofs<<"#x, y, z, descriptor"<<endl;
	vector<FeatureType>::const_iterator it = vecFeature.begin();
	while(it != vecFeature.end())
	{
		FeatureType S = *it;
		ofs<<S.x<<","<<S.y<<","<<S.z;
		vector<double>::iterator itr = S.descriptor.begin();
		while(itr != S.descriptor.end()) { ofs<<","<<*itr; itr++; }
		ofs<<endl;
		it++;
	}
	ofs.close();
	return true;
}
