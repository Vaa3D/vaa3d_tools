//main function to pre-process the neurons and compute features in batch
//by Yinan Wan
//2012-03-14

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#include "basic_surf_objs.h"
#include <unistd.h>
#include "pre_process.h"
#include "compute_morph.h"
#include "compute_gmi.h"
#include <QtGlobal>
#include <vector>
using namespace std;

void printHelp();

extern char *optarg;
extern int optind, opterr;

int main(int argc, char *argv[])
{
	if  (argc <=1)
	{
		printHelp();
		return 0;
	}

	//read arguments
	char *dfile_database = NULL;
	char *dfile_result = NULL;

	double step_size = 2;

	int c;
	static char optstring[] = "hi:o:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring))!=-1)
	{
		switch (c)
		{
			case 'h':
				printHelp();
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
			NeuronTree tmp_preprocessed = pre_process(tmp, step_size);
			double * feature_morph = new double[21];
			double * feature_gmi = new double[14];
			computeFeature(tmp_preprocessed, feature_morph);
			computeGMI(tmp_preprocessed, feature_gmi);
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

	return 0;
}


void printHelp()
{
	printf("\nBatch Compute: compute and record the neuron features (morph & gmi). by Yinan Wan.\n\n");
	printf("Usage:\n");
	printf("\t -i <database_file(s)>         input linker file (.ano) or file name list(\"a.swc b.swc\")\n");
	printf("\t -o <output_file>              name of the output file, which will be neuron featurebase (.nfb) file.\n");
	printf("\t                               default result will be generated under the same directory of the query file and has a name of 'databaseName_features.nfb'.\n");
	printf("\t -h                            print this message.\n\n");
	printf("Example: ./batch_compute -i mydatabase.ano -o features.nfb\n\n");
}
