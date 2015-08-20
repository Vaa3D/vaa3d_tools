
#ifndef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#include "inverse_projection_main.h"


#include <unistd.h>

int inverse_projection_main(const V3DPluginArgList &input, V3DPluginArgList & output)
{
	printf("\nwelcome to inverse_projection\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_invp();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


	if (paralist->size()!=1)
	{
		printf("Please specify all paramters in one text string.\n");
		printHelp_invp();
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
		printHelp_invp();

	//read arguments
	char *dfile_target	= NULL;
	char *dfile_subject = NULL;
	char *dfile_swc 	= NULL;

	int c;
	static char optstring[] = "ht:s:o:N:n:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp_invp();
				return 0;
				break;
			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				dfile_target = optarg;
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				dfile_subject = optarg;
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_swc = optarg;
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
	}

	QString qs_filename_1(dfile_target);
	QString qs_filename_2(dfile_subject);

	int n_dismethod = 0; //initial basic: euclidean 
	int arr_voterweight[100]={0}, n_bin=10, K=14;												//initial adv
	int n_refinemethod = 1; //affine refine															//refine basic
	int n_sampling = 2000, n_affineweight = 10;														//refine adv - affine
	bool arr_constrainter[100]={0};
	int n_iternum = 50,n_neighbor = 15, n_topcandidatenum = 20;			//refine adv - manifold

	arr_voterweight[0] = 1;
	arr_voterweight[1] = 1;
	arr_voterweight[2] = 1;

	arr_constrainter[0] = 1;
	arr_constrainter[1] = 1;
	arr_constrainter[2] = 1;

	NeuronTree nt_tar,nt_sub;
	if(qs_filename_1.endsWith(".swc") && qs_filename_2.endsWith(".swc"))
    {
		nt_tar=readSWC_file(qs_filename_1);
		nt_sub=readSWC_file(qs_filename_2);
    	printf("\t>>read %d points from [%s]\n",nt_tar.listNeuron.size(),qPrintable(qs_filename_1));
    	printf("\t>>read %d points from [%s]\n",nt_sub.listNeuron.size(),qPrintable(qs_filename_2));
    }
    else
    {
    	printf("ERROR: at least one swc file is invalid.\n");
    	return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//reorganize the markers to the format needed by point_cloud_registration.app
	vector<Coord3D_PCM> vec_1,vec_2;
	Coord3D_PCM temp;
	for(int i=0;i<nt_tar.listNeuron.size();i++)
	{
		temp.x=nt_tar.listNeuron.at(i).x;
		temp.y=nt_tar.listNeuron.at(i).y;
		temp.z=nt_tar.listNeuron.at(i).z;
		vec_1.push_back(temp);
	}
	for(int i=0;i<nt_sub.listNeuron.size();i++)
	{
//		temp.x=nt_sub.listNeuron.at(i).x;
//		temp.y=nt_sub.listNeuron.at(i).y;
//		temp.z=nt_sub.listNeuron.at(i).z;
		temp.x=nt_sub.listNeuron.at(i).x;
		temp.y=nt_sub.listNeuron.at(i).y;
		temp.z=nt_sub.listNeuron.at(i).z;
		vec_2.push_back(temp);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//run point clouds matching
	vector<int> vec_1to2index;
	vector<Coord3D_PCM> vec_2_invp;
	if(!q_pointcloud_match(vec_1,vec_2,
			n_dismethod,												//initial basic
			arr_voterweight,n_bin,K,									//initial adv
			n_refinemethod,												//refine basic
			n_sampling,n_affineweight,									//refine adv - affine
			arr_constrainter,n_iternum,n_neighbor,n_topcandidatenum,	//refine adv - manifold
			vec_1to2index,												//output
			vec_2_invp))												//output - refine adv - affine
	{
		fprintf(stderr,"ERROR: q_pointcloud_match() return false! \n");
		return 0;
	}


	NeuronTree nt_invp = export_invp_2swc(vec_2_invp, nt_sub);
	
	QString qs_swc(dfile_swc);
	writeSWC_file(qs_swc, nt_invp);

	return 1;
}

NeuronTree export_invp_2swc(vector<Coord3D_PCM> & vec_2_invp, NeuronTree & nt_sub)
{
	NeuronTree nt_invp = nt_sub;
	nt_invp.file = nt_sub.file + "_invp.swc";
	for (int i=0;i<nt_sub.listNeuron.size();i++)
	{
		nt_invp.listNeuron[i].x = vec_2_invp[i].x;
		nt_invp.listNeuron[i].y = vec_2_invp[i].y;
		nt_invp.listNeuron[i].z = vec_2_invp[i].z;
	}
	return nt_invp;
}

void printHelp_invp()
{
	cout<<"\nBlastNeuron Plugin - inverse projection: compute optimal affine transform from swc1 to swc2, output the inverse projection of swc2"<<endl;
	cout<<"Usage:"<<endl;
	cout<<"\t #t <target_swc>:        target swc name"<<endl;
	cout<<"\t #s <subject_swc>:       subject swc name"<<endl;
	cout<<"\t #o <output_swc>:        output swc name. default: subject_invp.swc"<<endl;
	cout<<"Example: vaa3d -x  blastneuron -f inverse_projection -p \"#t target.swc #s subject.swc #o subject_invp.swc\"\n"<<endl;
}
