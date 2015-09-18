//main function to pre-process the neurons and compute features in batch
//by Yinan Wan
//2012-03-14

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#include "batch_compute_main.h"
#include "basic_surf_objs.h"
#include <unistd.h>
#include "pre_process.h"
#include "compute_morph.h"
#include "compute_gmi.h"
#include <QtGlobal>
#include <vector>
using namespace std;


bool batch_compute_main(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	printf("\nwelcome to batch_compute\n");


	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

	if(input.size() != 2) 
	{
		printf("Please specify parameter set.\n");
		printHelp_batch_compute();
		return false;
	}
	paralist = (vector<char*>*)(input.at(1).p);


    if (paralist->size()!=1)
    {
        printf("Please specify all paramters in one text string.\n");
        printHelp_batch_compute();
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
		printHelp_batch_compute();


	//read arguments
	char *dfile_database = NULL;
	char *dfile_result = NULL;

	double step_size = 2;

	int c;
    static char optstring[] = "h:i:o:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring))!=-1)
	{
		switch (c)
		{
			case 'h':
				printHelp_batch_compute();
				return 0;
				break;
			case 'i':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				dfile_database = optarg;
				break;
			case 'o':
				if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
				{
					fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_result = optarg;
				break;
			case '?':
				fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
				return 1;
				break;
		}
	}

	//read database file
	QStringList nameList, filePathList;
	V3DLONG neuronNum;
	QString qs_database(dfile_database);

	QList<double*> morph_list, gmi_list;
	
	qs_database = qs_database.simplified();
	if (qs_database.endsWith(".ano") || qs_database.endsWith(".ANO"))
	{
		printf("(0). reading a linker file.\n");
		P_ObjectFileType linker_object;
		if (!loadAnoFile(QString(dfile_database),linker_object))
		{
			fprintf(stderr,"Error in reading the linker file.\n");
			return 1;
		}
		nameList = linker_object.swc_file_list;
		neuronNum = nameList.size();
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			NeuronTree tmp = readSWC_file(nameList.at(i));
			printf(qPrintable("calculating features for" + tmp.file +".\n"));
			filePathList.append(tmp.file);
            // it makes more sense to run pre_processing seperately, so that the pre_processed steps can be adjusted (tuning step size), saved and verified.
            //NeuronTree tmp_preprocessed = pre_process(tmp, step_size);
			double * feature_morph = new double[21];
			double * feature_gmi = new double[14];
            computeFeature(tmp, feature_morph);
            computeGMI(tmp, feature_gmi);
			morph_list.append(feature_morph);
			gmi_list.append(feature_gmi);
		}
	}
	else {
		fprintf(stderr, "the reference file/list you specified is not supported.\n");
		return 1;
	}
	

	QString outfile(dfile_result);
	if (!dfile_result)
	{
		outfile = qs_database + "features.nfb";
	}


	//output a neuron featurebase (.nfb) file
	FILE * fp;
	fp = fopen(qPrintable(outfile), "w");
	for (V3DLONG i=0;i<neuronNum;i++)
	{
		fprintf(fp, qPrintable("SWCFILE=" + filePathList[i]));
		fprintf(fp, "\n");
		for (int j=0;j<21;j++)
			fprintf(fp,"%.8f\t", morph_list[i][j]);
		fprintf(fp,"\n");
		for (int j=0;j<14;j++)
			fprintf(fp,"%.8f\t", gmi_list[i][j]);
		fprintf(fp,"\n");
	}
	fclose(fp);

	for (V3DLONG i=0;i<neuronNum;i++)
	{
		if (morph_list[i]) {delete []morph_list[i]; morph_list[i]=NULL;}
		if (gmi_list[i]) {delete []gmi_list[i]; gmi_list[i]=NULL;}
	}

	return true;
}


void printHelp_batch_compute()
{
	printf("\nBlastNeuron Plugin - Batch Compute: compute and record the neuron features (morph & gmi). by Yinan Wan.\n\n");
	printf("Usage:\n");
	printf("\t #i <database_file(s)>         input linker file (.ano) or file name list(\"a.swc b.swc\")\n");
	printf("\t #o <output_file>              name of the output file, which will be neuron featurebase (.nfb) file.\n");
	printf("\t                               default result will be generated under the same directory of the query file and has a name of 'databaseName_features.nfb'.\n");
	printf("\t #h                            print this message.\n\n");
	printf("Example: vaa3d -x blastneuron -f batch_compute -p \"#i mydatabase.ano #o features.nfb\"\n\n");
}
