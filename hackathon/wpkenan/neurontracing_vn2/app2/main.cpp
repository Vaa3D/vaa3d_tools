#include <iostream>
#include <vector>
#include <string>

#include "fastmarching_tree.h"
#include "fastmarching_dt.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
using namespace std;

string basename(string para)
{
	int pos1 = para.find_last_of("/");
	int pos2 = para.find_last_of(".");
	if(pos1 == string::npos) pos1 = -1;
	if(pos2 == string::npos) pos2 = para.size();
	return para.substr(pos1+1, pos2 - pos1 -1);
}
void printHelp()
{
	cout<<"Usage : ./hp <inimg_file> -inmarker <marker_file> [-outswc <swc_file>] [-length-thresh <double>] [-bkg-thresh <int>] [-cnn-type <int>] [-with-break] [-channel <int>]"<<endl;
	cout<<endl;
	cout<<"-inmarker           [-im] input marker file, the first marker is source marker, the rest are target markers"<<endl;
	cout<<"-outswc             [-os] output tracing result, default is <imagename>_tracing.swc"<<endl;
	cout<<"-length-thresh      [-lt] default 1.0, the length threshold value for hierarchy pruning(hp)"<<endl;
	cout<<"-sr-ratio           [-sr] default 1/9, signal/redundancy ratio threshold"<<endl;
	cout<<"-bkg-thresh         [-bt] default 30, background threshold value used in GSDT and tree construction when no target marker"<<endl;
	//cout<<"-gsdt               [-gs] perform GSDT for original image"<<endl;
	//cout<<"-coverage           [-co] use hierarchy coverage pruning"<<endl;
	cout<<"-cnn-type           [-ct] default 2. connection type 1 for 6 neighbors, 2 for 18 neighbors, 3 for 26 neighbors"<<endl;
	cout<<"-with-break         [-wb] accept one background point between foreground during tree construction when only one marker"<<endl;
	cout<<"-channel            [-ch] set image channel"<<endl;
	cout<<endl;
	cout<<"Hierarchical pruning method by Hang Xiao"<<endl;
	cout<<endl;
}

int main(int argc, char * argv[])
{
	if(argc == 1) {printHelp(); return 0;}

	vector<string> filelist;
	string inimg_file;
	string inmarker_file;
	string outswc_file;
	bool is_gsdt = true;
	//bool is_coverage_prune = true;//false;
	bool is_break_accept = false;
	bool is_leaf_prune = true;
	bool is_smooth = false;
	int bkg_thresh = 30;
	double length_thresh = 1.0;
	int cnn_type = 2; // default connection type 2
	int channel = 0;
	double SR_ratio = 3.0/9.0;

	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i],"-inmarker") == 0 || strcmp(argv[i],"-im") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			inmarker_file = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i],"-outswc") == 0 || strcmp(argv[i],"-os") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			outswc_file = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i],"-length-thresh") == 0 || strcmp(argv[i],"-lt") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			length_thresh = atoi(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i],"-sr-ratio") == 0 || strcmp(argv[i],"-sr") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			SR_ratio = atof(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i], "-bkg-thresh") == 0 || strcmp(argv[i],"-bt") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			bkg_thresh = atoi(argv[i+1]);
			i++;
		}
//		else if(strcmp(argv[i], "-coverage") == 0 || strcmp(argv[i],"-co") == 0)
//		{
//			is_coverage_prune = true;
//		}
		else if(strcmp(argv[i], "-gsdt") == 0 || strcmp(argv[i],"-gs") == 0)
		{
			is_gsdt = true;
		}
		else if(strcmp(argv[i], "-leaf-prune") == 0)
		{
			is_leaf_prune = true;
		}
		else if(strcmp(argv[i], "-smooth") == 0)
		{
			is_smooth = true;
		}
		else if(strcmp(argv[i], "-with-break") == 0 || strcmp(argv[i],"-wb") == 0)
		{
			is_break_accept = true;
		}
		else if(strcmp(argv[i], "-cnn-type") == 0 || strcmp(argv[i],"-ct") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			cnn_type = atoi(argv[i+1]);
			i++;
		}
		else if(strcmp(argv[i], "-channel") == 0 || strcmp(argv[i],"-ch") == 0)
		{
			if(i+1 >= argc || argv[i+1][0] == '-'){cerr<<"need parameter for "<<argv[i]<<endl; printHelp(); return 0;}
			channel = atoi(argv[i+1]);
			if(channel < 0 || channel >= 3) {cerr<<"chanel should between 0~2"<<endl; printHelp(); return 0;}
			i++;
		}
		else 
		{
			if(argv[i][0] == '-') {cerr<<"unknown option "<<argv[i]<<endl; printHelp(); return 0;}
			else filelist.push_back(argv[i]);
		}
	}
	//if(inmarker_file == ""){cerr<<"Need input marker file"<<endl; printHelp(); return 0;}
	if(filelist.size() != 1){cerr<<"Need one input image only!"<<endl; printHelp(); return 0;}
	inimg_file = filelist[0];
	if(outswc_file == "") outswc_file = basename(inimg_file) + "_hp.swc";

	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"inmarker_file = "<<inmarker_file<<endl;
	cout<<"outswc_file = "<<outswc_file<<endl;
	cout<<"bkg_thresh = "<<bkg_thresh<<endl;
	cout<<"length_thresh = "<<length_thresh<<endl;
	cout<<"SR_ratio = "<<SR_ratio<<endl;
	cout<<"is_gsdt = "<<is_gsdt<<endl;
	cout<<"is_smooth = "<<is_smooth<<endl;
	//cout<<"is_coverage_prune = "<<is_coverage_prune<<endl;
	cout<<"is_with_break = "<<is_break_accept<<endl;
	cout<<"cnn_type = "<<cnn_type<<endl;
	cout<<"channel = "<<channel<<endl;

	unsigned char * indata1d = 0; V3DLONG * in_sz = 0; int datatype = 0;
	if(!loadImage((char*)inimg_file.c_str(), indata1d, in_sz, datatype, channel))
	{
		cerr<<"Load image error!"<<endl; 
		printHelp(); 
		return 0;
	}
	if(datatype != V3D_UINT8)// && datatype != V3D_UINT16) 
	{
		cerr<<"Current only support 8bit image!"<<endl;
		printHelp();
		return 0;
	}

	vector<MyMarker> inmarkers; 
	vector<MyMarker *> outtree;

	float * phi = 0;
	if(inmarker_file != "") inmarkers = readMarker_file(inmarker_file);
	else
	{
		cout<<"Start detecting cellbody"<<endl;
        switch(datatype)
        {
            case V3D_UINT8:
                fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],cnn_type, bkg_thresh);
                break;
            case V3D_UINT16:
                fastmarching_dt_XY((short int*)indata1d, phi, in_sz[0], in_sz[1], in_sz[2],cnn_type, bkg_thresh);
                break;
        }

        V3DLONG sz0 = in_sz[0];
        V3DLONG sz1 = in_sz[1];
        V3DLONG sz2 = in_sz[2];
        V3DLONG sz01 = sz0 * sz1;
        V3DLONG tol_sz = sz01 * sz2;

        V3DLONG max_loc = 0;
        double max_val = phi[0];
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            if(phi[i] > max_val)
            {
                max_val = phi[i];
                max_loc = i;
            }
        }
        MyMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);
        inmarkers.push_back(max_marker);
	}

	cout<<"======================================="<<endl;
	cout<<"Construct neuron tree"<<endl;
	if(inmarkers.empty()) 
	{
		cerr<<"need at least one markers"<<endl; return false;
	}
	else if(inmarkers.size() == 1)
	{
		cout<<"only one input marekr"<<endl;
		if(is_gsdt) 
		{
			if(phi == 0)
			{
				cout<<"processing fastmarching distance transformation ..."<<endl;
				switch(datatype)
				{
					case V3D_UINT8:
						fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
						break;
					case V3D_UINT16:
						fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
						break;
				}
			}

			cout<<endl<<"constructing fastmarching tree ..."<<endl;
			fastmarching_tree(inmarkers[0], phi, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, is_break_accept);
		}
		else 
		{
			switch(datatype)
			{
				case V3D_UINT8:
					fastmarching_tree(inmarkers[0], indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, is_break_accept);
					break;
				case V3D_UINT16:
					fastmarching_tree(inmarkers[0], (short int*)indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh, is_break_accept);
					break;
			}
		}
	}
	else
	{
		vector<MyMarker> target; target.insert(target.end(), inmarkers.begin()+1, inmarkers.end());
		if(is_gsdt)
		{
			if(phi == 0)
			{
				cout<<"processing fastmarching distance transformation ..."<<endl;
				switch(datatype)
				{
					case V3D_UINT8:
						fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
						break;
					case V3D_UINT16:
						fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], cnn_type, bkg_thresh);
						break;
				}
			}
			cout<<endl<<"constructing fastmarching tree ..."<<endl;
			fastmarching_tree(inmarkers[0], target, phi, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type);
		}
		else 
		{
			switch(datatype)
			{
				case V3D_UINT8:
					fastmarching_tree(inmarkers[0], target, indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type);
					break;
				case V3D_UINT16:
					fastmarching_tree(inmarkers[0], target, (short int*) indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], cnn_type);
					break;
			}
		}
	}
	cout<<endl<<"Pruning neuron tree"<<endl;
	vector<MyMarker*> & inswc = outtree;
	vector<MyMarker*> outswc;
	if(true)//is_coverage_prune)
	{
		happ(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], bkg_thresh, length_thresh, SR_ratio, is_leaf_prune, is_smooth);
	}
	else
	{
		hierarchy_prune(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], length_thresh);
		if(1) //get radius
		{
			for(int i = 0; i < outswc.size(); i++)
			{
				outswc[i]->radius = markerRadius(indata1d, in_sz, *(outswc[i]), bkg_thresh);
			}
		}
	}

	saveSWC_file(outswc_file, outswc);

	if(indata1d){delete [] indata1d; indata1d = 0;}
	if(in_sz){delete [] in_sz; in_sz = 0;}
	if(phi){delete [] phi; phi = 0;}
	for(int i = 0; i < outtree.size(); i++) delete outtree[i];
	return 0;
}
