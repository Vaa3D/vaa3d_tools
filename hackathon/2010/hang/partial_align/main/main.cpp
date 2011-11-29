/* main.cpp
 * 2010-02-08: the program is created by Hang Xiao
 */

// 

#include <iostream>
#include <string>

#include "stackutil.h"
#include "basic_types.h"
#include "utils.h"

using namespace std;

void printHelp();

int main(int argc, char* argv[])
{
	if(argc <= 3)
	{
		printHelp();
		return 0;
	}

	if(string(argv[1]) == "-mark-detector")
	{
		if(argc!=4){printHelp(); return 0;}

		unsigned char * data1d = 0;
		V3DLONG *sz = 0;
		int datatype;
		if(!loadImage(argv[2], data1d, sz, datatype)){cerr<<"loadImage error"<<endl; return 0;}

		vector<MarkerType> vecMarker;
		detect_marker(vecMarker, data1d, sz);
		writeMarker_file(argv[3], vecMarker);
		if(data1d){delete [] data1d; data1d = 0;}
	}
	else if(string(argv[1]) == "-calc-feature")
	{
		if(argc!=4 && argc !=5) {printHelp(); return 0;}
		vector<MarkerType> vecMarker;

		unsigned char * data1d = 0;
		V3DLONG *sz = 0;
		int datatype;
		if(!loadImage(argv[2], data1d, sz, datatype)){cerr<<"loadImage error"<<endl; return 0;}

		if(argc==4) {if(!detect_marker(vecMarker, data1d, sz)) {cerr<<"unable to get marker"<<endl; return 0;}}
		else if(argc==5) vecMarker = readMarker_file(argv[3]);


		vector<FeatureType> vecFeature;
		if(!marker_to_feature(vecFeature, vecMarker, data1d, sz)){cerr<<"unable to calc feature"<<endl; return 0;};
		if(!writeFeature_file(string(argv[argc-1]),vecFeature)){cerr<<"unable to write feature"<<endl; return 0;}
		if(argc==4)
		{
			string marker_out = argv[argc-1];
			if(marker_out.find_last_of('.')!=string::npos) marker_out = marker_out.substr(0, marker_out.find_last_of('.')) + ".marker";
			if(!writeMarker_file(marker_out, vecMarker)){cerr<<"unable to write marker"<<endl; return 0;}
		}

		if(data1d){delete [] data1d; data1d = 0;}
	}
	else if(string(argv[1]) == "-uniform-feature")
	{
		if(argc!=4) {printHelp(); return 0;}
		vector<FeatureType> vecFeature1 = readFeature_file(argv[2]);
		vector<FeatureType> vecFeature2 = readFeature_file(argv[3]);
		if(!uniform_features(vecFeature1, vecFeature2)) {cerr<<"unable to uniform feature."<<endl; return 0;}
		string out_file1 = argv[2]; 
		if(out_file1.find_last_of('.')!=string::npos) out_file1 = out_file1.substr(0, out_file1.find_last_of('.')) + "_uniformed.feat";
		string out_file2 = argv[3];
		if(out_file2.find_last_of('.')!=string::npos) out_file2 = out_file2.substr(0, out_file2.find_last_of('.')) + "_uniformed.feat";
		if(!writeFeature_file(out_file1, vecFeature1)){cerr<<"unable to save feature1"<<endl; return 0;}
		if(!writeFeature_file(out_file2, vecFeature2)){cerr<<"unable to save feature2"<<endl; return 0;}
	}
	else if(string(argv[1]) == "-compare-feature")
	{
		if(argc != 6){printHelp(); return 0;}
		vector<FeatureType> vecFeature1 = readFeature_file(argv[2]);
		vector<FeatureType> vecFeature2 = readFeature_file(argv[3]);
		vector<CompareResult> crs;
		if(!compare_features(crs, vecFeature1, vecFeature2)){cerr<<"unable to compare features"<<endl; return 0;}
		cout<<"nindis1 = "<<vecFeature1.size()<<" nindis2 = "<<vecFeature2.size()<<" nresults = "<<crs.size()<<endl;
		int npairs = crs.size();
		for(int i = 0; i < npairs; i++) 
		{
			cout<<crs[i].id1<<" <--> "<<crs[i].id2<<" : "<<crs[i].min_dst<<endl;
		}

		vector<MarkerType> tempMarker;

		tempMarker= feature_to_marker(vecFeature1);
		vector<MarkerType> vecMarker1 = extract_id1_markers(crs, tempMarker);

		tempMarker= feature_to_marker(vecFeature2);
		vector<MarkerType> vecMarker2 = extract_id2_markers(crs, tempMarker);

		if(!writeMarker_file(argv[4], vecMarker1)) {cerr<<"unalbe to save marker 1"<<endl; return false;}
		if(!writeMarker_file(argv[5], vecMarker2)) {cerr<<"unable to save marker 2"<<endl; return false;}

	}
}

void printHelp()
{
	cout<<"Usage : partial_align [option] input_img marker/feature"<<endl;
	cout<<""<<endl;
	cout<<"-mark-detector     in_img out_marker"<<endl;
	cout<<"-calc-feature      in_img [in_marker] output "<<endl;
	cout<<"-uniform-feature   feat1 feat2"<<endl;
	cout<<"-feature-comprare  feat1 feat2 marker1 marker2"<<endl;
	cout<<""<<endl;
}

