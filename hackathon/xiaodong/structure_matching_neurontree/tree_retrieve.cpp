//retrieve similar neurons, without pre-specified number
//by Xiaodong Yue
//2017-06-24

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#define FEATSNUM 21

//#include <Qtcore>
#include <QtGui>
#include <v3d_interface.h>
#include <iostream>
#include <QString>

#include <vector>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_dist_func.h"

#include "tree_retrieve.h"
#include "basic_surf_objs.h"
#include "neuron_retrieve.h"
#include "qglobal.h"
#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

#include <QtGlobal>
#include <vector>
#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include <io.h>
#endif
using namespace std;



int tree_retrieval(const NeuronTree & nt_query, const QList<NeuronTree> & nt_list, vector<V3DLONG> & retrieved_id)
{

	printf("neuron tree retrieve \n");
	//
	int num_tree = nt_list.size();
	QList<double*> morph_list, gmi_list;

	for (int i=0;i < num_tree; i++)
	{
		printf("%s, %d of %d\n","neuron tree",i, num_tree);
		NeuronTree nt = nt_list[i];
		double * feats_morph = NULL;
		double * feats_gmi = NULL;
		try 
		{
			feats_morph = new double[FEATSNUM];
			feats_gmi = new double[FEATSNUM];
		}
		catch (...)
		{
			
			if (feats_morph)  {delete []feats_morph; feats_morph=NULL;}
			return false;
			if (feats_gmi)  {delete []feats_gmi; feats_gmi=NULL;}
			return false;
			
		}
		computeFeature(nt, feats_morph);
		computeGMI(nt, feats_gmi);

		morph_list.append(feats_morph);
		gmi_list.append(feats_gmi);
	}//end for

	vector<vector<V3DLONG> > retrieved_all;
	vector<V3DLONG> retrieved_tmp;
	
	//norm type 1,2, feature type 1,2

	vector<int> feature_codes, norm_codes;

	QString q_norm = "1,2,4";
	//QString q_norm = "1,4";

	if (q_norm==NULL)
	{
		feature_codes.push_back(1); feature_codes.push_back(2);
		norm_codes.push_back(1); norm_codes.push_back(2);
	}
	QStringList splitted = q_norm.split(",");
	if (splitted.contains("1")) feature_codes.push_back(1);
	if (splitted.contains("2")) feature_codes.push_back(2);
	if (splitted.contains("3")) norm_codes.push_back(1);
	if (splitted.contains("4")) norm_codes.push_back(2);

	if (feature_codes.empty() || norm_codes.empty())
	{
		fprintf(stderr, "the norm codes you specified is not supported.\n");
		return 1;
	}

	int num_candy = nt_list.size();
	printf("Tag 2: the number of candidates: %d \n", num_candy);
	for  (int i=0;i<feature_codes.size();i++)
	{
		for (int j=0;j<norm_codes.size();j++)
		{
			vector<V3DLONG> retrieved_tmp;
			if (feature_codes[i]==1) 
			{
				if (!neuron_retrieve(nt_query, morph_list, retrieved_tmp,num_candy, feature_codes[i], norm_codes[j]))
				{
					fprintf(stderr,"Error in neuron_retrieval.\n");
					return 1;
				}
			}
			else if (feature_codes[i]==2)
			{
				if (!neuron_retrieve(nt_query, gmi_list, retrieved_tmp, num_candy, feature_codes[i], norm_codes[j]))
				{
					fprintf(stderr,"Error in neuron_retrieval.\n");
					return 1;
				}
			}

			retrieved_all.push_back(retrieved_tmp);
		}//end for j norm_codes

	}//end for i feature_codes


	////for morph features
	//if (!neuron_retrieve(nt_query, morph_list, retrieved_tmp, num_tree, 1, 1))
	//{
	//	fprintf(stderr,"Error in neuron_retrieval.\n");
	//	return 1;
	//}

	//printf("%s %d","retrieved_tmp:",retrieved_tmp.size());
	//for(int j=0; j<retrieved_tmp.size();j++)
	//{
	//	printf("%d %s", retrieved_tmp[j], "\n");
	//}

	////for gmi features
	//if (!neuron_retrieve(nt_query, gmi_list, retrieved_tmp, num_tree, 2, 1))
	//{
	//	fprintf(stderr,"Error in neuron_retrieval.\n");
	//	return 1;
	//}

	//retrieved_all.push_back(retrieved_tmp);


	//vector<V3DLONG>	retrieved_id;
	int rej_thres = 6;//if top 5 candidates of both method have no intersection, consider this query does not have matched neuron
	int retrieved_num = 1;

	if (!compute_intersect(retrieved_all, retrieved_id, retrieved_num, rej_thres))
	{
		printf("No similar neurons exist in the database.\n");
	}
	printf("retrieved_number: %d \n",retrieved_id.size());
	
	for(int k=0; k<retrieved_id.size();k++)
	{
		printf("retrieved_id: %d \n",retrieved_id[k]);
	}

	
	/*
	//output file
	QString outfile;
	outfile = "F:\\mouse brain neuron data\\subtrees\\retrieved.ano";
	//if (dfile_result==NULL)
	//	outfileName = qs_query + QString("_retrieved.ano");

	FILE * fp = fopen(outfile.toLatin1(), "wt");
	if (!fp)
	{
		v3d_msg("Could not open the file to save the neuron.");
		return false;
	}
    
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",outfile);
		return false;
	}
	fprintf(fp,"#database:          %s\n", "");
	fprintf(fp,"#query neuron:      %s\n","");
	fprintf(fp,"#num of candidates: %d\n", retrieved_all.size());
	for (int i=0;i<retrieved_all.size();i++)
	{
		for(int j=0; j<retrieved_tmp.size();j++)
		{
		    fprintf(fp, "%d", retrieved_all[i][j]);
		}
		fprintf(fp,"\n");
	}
	printf("Linker file %s has been successfully generated.\n", outfile);
	fclose(fp);
	*/

	//collect memory of feature vectors
	for(int i=0;i<num_tree;i++)
	{
		if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
		if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
	}
	printf("tag 3: tree retrieve finished! \n");

	return 0;
}



/*
int global_retrieve_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{

	printf("welcome to global_retrieve\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_global_retrieve();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


	if (paralist->size()!=1)
	{
		printf("Please specify all paramters in one text string.\n");
		printHelp_global_retrieve();
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
		printHelp_global_retrieve();


	//read arguments
	char *dfile_database = NULL;
	char *dfile_query = NULL;
	char *dfile_result = NULL;
	char *para_norm = NULL; //normalization method. 1: global feature, 2: GMI, 3: whitening normalization, 4:rankScore
	int retrieved_num = 2;
	double thres = 0.1;

	int c;
	static char optstring[] = "hd:q:o:m:n:t:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring))!=-1)
	{
		switch (c)
		{
			case 'h':
				printHelp_global_retrieve();
				return 0;
				break;
			case 'd':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				dfile_database = optarg;
				break;
			case 'q':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -q.\n");
					return 1;
				}
				dfile_query = optarg;
				break;
			case 'o':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_result = optarg;
				break;
			case 'm':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				para_norm = optarg;
				break;
			case 'n':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -n.\n");
					return 1;
				}
				retrieved_num = atoi(optarg);
				if (retrieved_num<0)
				{
					fprintf(stderr, "Illegal retrieve number. It must>=0.\n");
				}
				break;
			case 't':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				thres = atof(optarg);
				if (thres<=0)
				{
					fprintf(stderr, "Illegal retrieve threshold. It must>0.\n");
				}
				break;
			case '?':
				fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
				return 1;
				break;
		}
	}

	//read database file
	QStringList nameList;
	V3DLONG neuronNum;
	QString qs_database(dfile_database);
	QList<double*> morph_list, gmi_list;

	qs_database = qs_database.simplified();
	if (qs_database.endsWith(".nfb") || qs_database.endsWith(".NFB"))
	{
		if (!loadFeatureFile(QString(dfile_database),morph_list, gmi_list, nameList))
		{
			fprintf(stderr,"Error in reading the linker file.\n");
			return 1;
		}
		neuronNum = nameList.size();
	}
	else {
		fprintf(stderr, "the reference file/list you specified is not supported.\n");
		return 1;
	}

	//read norm parameter
	vector<int> feature_codes, norm_codes;
	QString q_norm(para_norm);
	if (q_norm==NULL)
	{
		feature_codes.push_back(1); feature_codes.push_back(2);
		norm_codes.push_back(1); norm_codes.push_back(2);
	}
	QStringList splitted = q_norm.split(",");
	if (splitted.contains("1")) feature_codes.push_back(1);
	if (splitted.contains("2")) feature_codes.push_back(2);
	if (splitted.contains("3")) norm_codes.push_back(1);
	if (splitted.contains("4")) norm_codes.push_back(2);

	if (feature_codes.empty() || norm_codes.empty())
	{
		fprintf(stderr, "the norm codes you specified is not supported.\n");
		return 1;
	}

	//read query file
	QString qs_query(dfile_query);
	NeuronTree query = readSWC_file(qs_query);

	//read output file
	QString outfileName(dfile_result);
	if (dfile_result==NULL)
		outfileName = qs_query + QString("_retrieved.ano");

	vector<vector<V3DLONG> > retrieved_all;

	for  (int i=0;i<feature_codes.size();i++)
	{
		for (int j=0;j<norm_codes.size();j++)
		{
			vector<V3DLONG> retrieved_tmp;
			if (feature_codes[i]==1) 
			{
				if (!neuron_retrieve(query, morph_list, retrieved_tmp, neuronNum, feature_codes[i], norm_codes[j]))
				{
					fprintf(stderr,"Error in neuron_retrieval.\n");
					return 1;
				}
			}
			else if (feature_codes[i]==2)
			{
				if (!neuron_retrieve(query, gmi_list, retrieved_tmp, neuronNum, feature_codes[i], norm_codes[j]))
				{
					fprintf(stderr,"Error in neuron_retrieval.\n");
					return 1;
				}
			}

			retrieved_all.push_back(retrieved_tmp);
		}

	}

	vector<V3DLONG>	retrieved_id;
	int rej_thres = 6;//if top 5 candidates of both method have no intersection, consider this query does not have matched neuron


	if (!compute_intersect(retrieved_all, retrieved_id, retrieved_num, rej_thres))
	{
		printf("No similar neurons exist in the database.\n");
	}
	if (!print_result(retrieved_id, qPrintable(outfileName), nameList, dfile_database, dfile_query))
	{
		fprintf(stderr, "Error in print_result.\n");
		return 1;
	}

	for (V3DLONG i=0;i<neuronNum;i++)
	{
		if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
		if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
	}

	return 0;
}

//
void printHelp_global_retrieve()
{
	printf("\nBlastNeuron Plugin - Neuron Comparison: given a query neuron, retrieve certain number of candidates from a database. by Yinan Wan.\n\n");
	printf("Usage:\n");
	printf("\t #d <featurebase_file>         input neuron featurebase file (.nfb) which contains neuron swc file names, morphological and invariant moment feature\n");
	printf("\t #q <query_file>               query neuron file (.swc)\n");
	printf("\t #n <candidate_number>         number of candidates you want to retrive.\n");
	printf("\t #o <output_file>              name of the output file, which will be a swc linker file.\n");
	printf("\t                               default result will be generated under the same directory of the query file and has a name of 'queryFileName_retrieved.ano'.\n");
	printf("\t #m <normalization_method>     feature computation and normalization  method\n");
	printf("\t                               m=1: global morphological feature; m=2: moment feature. m=3: whitening normalization; m=4: rankscore normalization\n");
	printf("\t                               use ',' to split if you want to combine: e.g. m=1,2,4 means you use morph & gmi features, together with rankScore norm\n");
	printf("\t                               [default]: m=1,2,3,4\n");
	printf("\t #h                            print this message.\n\n");
	printf("Example: vaa3d -x blastneuron -f global_retrieve -p \"#d myfeaturebase.nfb #q query.swc #n 10 #o result.ano #m 1,2,4\"\n\n");
}

*/