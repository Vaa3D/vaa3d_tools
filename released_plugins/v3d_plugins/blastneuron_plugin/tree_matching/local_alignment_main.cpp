#include "local_alignment_main.h"
#include <cstring>
using namespace std;


#include <unistd.h>

int local_alignment_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	printf("\nwelcome to local_alignment\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_local_alignment();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


	if (paralist->size()!=1)
	{
		printf("Please specify all paramters in one text string.\n");
		printHelp_local_alignment();
		return false;
	}
	
	
	char * paras = paralist->at(0);
	int argc = 1;
	enum {kArgMax = 64};
	char *argv[kArgMax];
	
	//parsing parameters
	if (paras)
	{
		int len = strlen(paras);
		for (int i=0;i<len;i++)
		{
			if (paras[i]=='#')
				paras[i] = '-';
		}

		char* pch = strtok(paras, " ");
		while (pch && argc<kArgMax)
		{
			argv[argc++] = pch;
			pch = strtok(NULL, " ");
		}
	}
	else
		printHelp_local_alignment();

	string name_neuron1, name_neuron2;
	string name_ref;
	string name_result;

	int c;
	static char optstring[] = "ht:s:r:o:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
	{
		switch (c)
		{
			case 'h':
				printHelp_local_alignment();
				return 0;
				break;
			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					cerr<<"Found illegal or NULL parameter for the option -t"<<endl;
					return 1;
				}
				name_neuron1 = optarg;
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					cerr<<"Found illegal or NULL parameter for the option -s"<<endl;
					return 1;
				}
				name_neuron2 = optarg;
				break;
			case 'r':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					cerr<<"Found illegal or NULL parameter for the option -s"<<endl;
					return 1;
				}
				name_ref = optarg;
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					cerr<<"Found illegal or NULL parameter for the option -o"<<endl;
					return 1;
				}
				name_result = optarg;
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
	}

	vector<MyMarker *> neuron1, neuron2;
	neuron1 = readSWC_file(name_neuron1);
	neuron2 = readSWC_file(name_neuron2);
	vector<map<MyMarker*, MyMarker*> > map_result;

	if (name_ref.empty())
	{
		if (!neuron_mapping_dynamic(neuron1, neuron2, map_result))
		{
			cerr<<"error in neuron_mapping"<<endl;
			return 0;
		}
	}
	else
	{
		vector<MyMarker *> neuron_ref = readSWC_file(name_ref);
		if (neuron2.size() != neuron_ref.size())
		{
			cerr<<"wrong reference neuron!"<<endl;
			return 0;
		}
		map<MyMarker*, MyMarker*> map_ref_ori;
		for (int i=0;i<neuron2.size();i++)
			map_ref_ori[neuron_ref[i]] = neuron2[i];
		if (!neuron_mapping_dynamic(neuron1, neuron_ref, map_result))
		{
			cerr<<"error in neuron_mapping"<<endl;
			return 0;
		}
		for (int i=0;i<map_result.size();i++)
			for (map<MyMarker*, MyMarker*>::iterator it=map_result[i].begin();it!=map_result[i].end();it++)
				map_result[i][(*it).first] = map_ref_ori[(*it).second];
	}
	
	vector<MyMarker*> map_swc;
	convert_matchmap_2swc(map_result, map_swc);
	saveSWC_file(name_result, map_swc);

	for (int i=0;i<neuron1.size();i++)
		if (neuron1[i]) {delete(neuron1[i]); neuron1[i]=NULL;}
	for (int i=0;i<neuron2.size();i++)
		if (neuron2[i]) {delete(neuron2[i]); neuron2[i]=NULL;}
	for (int i=0;i<map_swc.size();i++)
		if (map_swc[i]) {delete(map_swc[i]); map_swc[i]=NULL;}

	return 0;
}

void printHelp_local_alignment()
{
	cout<<"\nBlastNeuron Plugin - local alignent. neuron matching by H Xiao and Y Wan"<<endl;
	cout<<"Usage:"<<endl;
	cout<<"\t #t <target_swc> :         target swc name"<<endl;
	cout<<"\t #s <subject_swc>:         subject swc name"<<endl;
	cout<<"\t #o <output_matching_swc>: swc to store matching result"<<endl;
	cout<<"\t [#r] <reference_swc>    : in case of special use, the program match target with the reference, while output its position corresponding to subject_swc. subject_swc and reference_swc should be of same size, with the index matched"<<endl;
	cout<<"\t #h                      : print this message"<<endl;
	cout<<"Example vaa3d -x blastneuron -f local_alignment -p \"#t <target_swc> #s <subject_swc> #o <output_matching_swc> #r <reference_swc>\"\n"<<endl;
}







