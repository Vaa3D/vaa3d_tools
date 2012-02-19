//consensus_skeleton.cpp
//main function for merging two neurons in space
//by Yinan Wan
//2011-10-14

#ifdef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#include "basic_surf_objs.h"
#include <unistd.h>
#include <QtGlobal>
#include <vector>
using namespace std;
#include "eswc_core.h"

void printHelp();
bool export_eswc(NeuronTree & input, vector<V3DLONG> seg_id, vector<V3DLONG> seg_layer, const char* infile_name, const char* outfile_name);

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
	char *dfile_linker = NULL;
	char *dfile_result = NULL;

	int c;
	static char optstring[] = "hi:o:n:m:";
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
					fprintf(stderr, "Found illegal or NULL parameter for the option -l.\n");
					return 1;
				}
				dfile_linker = optarg;
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

	printf("(0). reading a swc file.\n");
	QString infileName(dfile_linker);
	NeuronTree input = readSWC_file(infileName);
	QString outfileName(dfile_result);
	if (!dfile_result)
	{
		outfileName = QString(dfile_linker)+".eswc";
	}

	vector<V3DLONG> segment_id, segment_layer;

	if (!swc2eswc(input,segment_id,segment_layer))
	{
		fprintf(stderr,"Error in swc2eswc.\n");
		return 1;
	}
	if (check_eswc(input, segment_id, segment_layer))
		v3d_msg("check passed!");
	else v3d_msg("check not passed!");
	export_eswc(input,segment_id,segment_layer, qPrintable(infileName),qPrintable(outfileName));
	printf("\t %s has been generated successfully, size:%d.\n",outfileName.toStdString().c_str(), input.listNeuron.size());

	return 0;
}

bool export_eswc(NeuronTree & input, vector<V3DLONG> seg_id, vector<V3DLONG> seg_layer, const char* infile_name, const char* outfile_name)
{
	V3DLONG pntNum = input.listNeuron.size();
	if (seg_id.size()!=pntNum || seg_layer.size()!=pntNum)
	{
		fprintf(stderr, "Cannot export eswc!.\n");
		return false;
	}
	FILE * fp;
	fp = fopen(outfile_name,"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",outfile_name);
		return false;
	}
	fprintf(fp, "#ESWC(extended swc) adapted from %s\n",infile_name);
	fprintf(fp, "##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value\n");
	for (int i=0;i<pntNum;i++)
	{
		NeuronSWC curr = input.listNeuron[i];
		fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d %d %d 0\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn,seg_id[i],seg_layer[i]);
	//	fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d\n",curr.n,seg_layer[i],curr.x,curr.y,curr.z,curr.r,curr.pn);
	}
	fclose(fp);
	return true;
}

void printHelp()
{
	printf("\nConsensus Skeleton: compute consensus skeleton for the reference neuron candidates. by Yinan Wan.\n\n");
	printf("Usage:\n");
	printf("\t -r <reference_file(s)>        input linker file (.ano) or file name list(\"a.swc b.swc\")\n");
	printf("\t -n <node_number>              set sampling node number, final output tree is of this size.\n");
	printf("\t                               if not specified, it will be the average size of the candidate neurons.\n");
	printf("\t -o <output_file>              output file name or directory. If -r is followd by a linkerfile name, this para can be omitted.\n");
	printf("\t                               default result will be generated under the same directory of the ref linkerfile and has a name of 'linkerFileName_consensus.swc'.\n");
	printf("\t -m <method_code>              the method to construct minimum spanning tree.\n");
	printf("\t                               m=0: Dijkstra's shortest path tree [default]; m=1: prim's mst algorithm, note this is very slow on large dataset.\n");
	printf("\t -h                            print this message.\n\n");
	printf("Example: ./consensus_skeleton -r mylinker.ano -n 200 -o consensus_skeleton.swc -m 0\n\n");
}
