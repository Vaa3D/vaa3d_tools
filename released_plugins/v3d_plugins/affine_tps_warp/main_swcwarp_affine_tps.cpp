//main_affineTPS.cpp
//affine+TPS transform pointset(swc) based on a pair of given control pointsets
//
//example:
//./main_warp_affine_tps.app/Contents/MacOS/main_warp_affine_tps -t ./data/Theoretical.marker -s ./data/I3-1_Fiduciary.marker -m ./data/fidu2theor.match -S ./data/CHA_STOP-9A.swc -a sub2tar_affine.swc -o sub2tar_affine_tps.swc
//
// by Lei Qu
//2010-03-18

#include <stdio.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

#include "../../basic_c_fun/basic_surf_objs.h"
#include "q_warp_affine_tps.h"


void printHelp();


int main(int argc, char *argv[])
{
	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
//	QString qs_filename_marker_tar	="/Users/qul/work/v3d_2.0/plugin_demo/affine_TPS_trans/data/Theoretical.marker";
//	QString qs_filename_marker_sub	="/Users/qul/work/v3d_2.0/plugin_demo/affine_TPS_trans/data/I3-1 Fiduciary.marker";
//	QString qs_filename_matchpairind="/Users/qul/work/v3d_2.0/plugin_demo/affine_TPS_trans/data/fidu2theor.match";
//	QString qs_filename_swc_sub		="/Users/qul/work/v3d_2.0/plugin_demo/affine_TPS_trans/data/CHA STOP-9A.swc";
//	QString qs_filename_swc_sub2tar ="/Users/qul/work/v3d_2.0/plugin_demo/affine_TPS_trans/data/CHA STOP-9A_warped.swc";
	//input
	QString qs_filename_marker_tar			=NULL;
	QString qs_filename_marker_sub			=NULL;
	QString qs_filename_matchpairind		=NULL;
	QString qs_filename_sub					=NULL;
	//output
	QString qs_filename_sub2tar_affine 		=NULL;
	QString qs_filename_sub2tar_affine_tps 	=NULL;

	int c;
	static char optstring[]="ht:s:m:S:a:o:f:";
	opterr=0;
	while((c=getopt(argc,argv,optstring))!=-1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				qs_filename_marker_tar.append(optarg);
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				qs_filename_marker_sub.append(optarg);
				break;
			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				qs_filename_matchpairind.append(optarg);
				break;
			case 'S':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -S.\n");
					return 1;
				}
				qs_filename_sub.append(optarg);
				break;
			case 'a':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				qs_filename_sub2tar_affine.append(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_sub2tar_affine_tps.append(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Image affine transformation:\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target  marker:          %s\n",qPrintable(qs_filename_marker_tar));
	printf(">>  input subject marker:          %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  input matched pair index:      %s\n",qPrintable(qs_filename_matchpairind));
	printf(">>  input subject data:            %s\n",qPrintable(qs_filename_sub));
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar_affine data:       %s\n",qPrintable(qs_filename_sub2tar_affine));
	printf(">>  output sub2tar_affine_tps data:   %s\n",qPrintable(qs_filename_sub2tar_affine_tps));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>Target: read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>Subject:read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read matched-pair index file and extract matched pairs. \n");
	//read matched-pair index file
	vector< vector<long> > vec2D_sub2tar_matchind;	//n*2; [*][0]: sub_index; [*][1]: tar_index
	if(!q_readMatchInd_file(qs_filename_matchpairind,vec2D_sub2tar_matchind))
	{
		printf("ERROR: q_readMatchInd_file() return false.\n");
		return false;
	}

	//print matched-pair index
    for(long i=0;i<vec2D_sub2tar_matchind.size();i++)
    {
    	printf("\tsub[%ld]-->tar[%ld]\n",vec2D_sub2tar_matchind[i][0],vec2D_sub2tar_matchind[i][1]);
    }

    //extract matched pairs
	vector<Coord3D_PCM> vec_tar,vec_sub;
	for(long i=0;i<vec2D_sub2tar_matchind.size();i++)
	{
		long tar_ind=vec2D_sub2tar_matchind[i][1]-1;
		long sub_ind=vec2D_sub2tar_matchind[i][0]-1;
		vec_tar.push_back(Coord3D_PCM(ql_marker_tar[tar_ind].x,ql_marker_tar[tar_ind].y,ql_marker_tar[tar_ind].z));
		vec_sub.push_back(Coord3D_PCM(ql_marker_sub[sub_ind].x,ql_marker_sub[sub_ind].y,ql_marker_sub[sub_ind].z));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. estimate the affine matrix. \n");
	Matrix x4x4_affinematrix;
	if(!q_affine_compute_affinmatrix_3D(vec_tar,vec_sub,x4x4_affinematrix))	//B=T*A
	{
		printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
		return false;
	}

	//print affine matrix
	for(long row=1;row<=x4x4_affinematrix.nrows();row++)
	{
		printf("\t");
		for(long col=1;col<=x4x4_affinematrix.ncols();col++)
			printf("%.3f\t",x4x4_affinematrix(row,col));
		printf("\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. read subject swc file. \n");
	NeuronTree nt_sub;
	if(qs_filename_sub.endsWith(".swc"))
    {
		nt_sub=readSWC_file(qs_filename_sub);
    	printf("\t>>read %d nodes from swc file [%s]\n",nt_sub.listNeuron.size(),qPrintable(qs_filename_sub));
    }

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. affine warp subject swc file. \n");
	NeuronTree nt_sub2tar_affine(nt_sub);
	{
		long n_swcnodes=nt_sub.listNeuron.size();
		Matrix x_vec_sub(4,n_swcnodes),x_sub2tar_affine(4,n_swcnodes);
		for(long i=0;i<n_swcnodes;i++)
		{
			x_vec_sub(1,i+1)=nt_sub.listNeuron[i].x;
			x_vec_sub(2,i+1)=nt_sub.listNeuron[i].y;
			x_vec_sub(3,i+1)=nt_sub.listNeuron[i].z;
			x_vec_sub(4,i+1)=1.0;
		}
		x_sub2tar_affine=x4x4_affinematrix.i()*x_vec_sub;
		for(long i=0;i<n_swcnodes;i++)
		{
			nt_sub2tar_affine.listNeuron[i].x=x_sub2tar_affine(1,i+1)/x_sub2tar_affine(4,i+1);
			nt_sub2tar_affine.listNeuron[i].y=x_sub2tar_affine(2,i+1)/x_sub2tar_affine(4,i+1);
			nt_sub2tar_affine.listNeuron[i].z=x_sub2tar_affine(3,i+1)/x_sub2tar_affine(4,i+1);
		}
	}
    QStringList infostr;
	if(!qs_filename_sub2tar_affine.isEmpty())
		writeSWC_file(qPrintable(qs_filename_sub2tar_affine),nt_sub2tar_affine, &infostr);

	//affine warp subject markers to target
	vector<Coord3D_PCM> vec_sub2tar_affine(vec_sub);
	{
		long n_sub_marker=ql_marker_sub.size();
		Matrix x_vec_sub(4,n_sub_marker),x_sub2tar_affine(4,n_sub_marker);
		for(long i=0;i<n_sub_marker;i++)
		{
			x_vec_sub(1,i+1)=ql_marker_sub[i].x;
			x_vec_sub(2,i+1)=ql_marker_sub[i].y;
			x_vec_sub(3,i+1)=ql_marker_sub[i].z;
			x_vec_sub(4,i+1)=1.0;
		}
		x_sub2tar_affine=x4x4_affinematrix.i()*x_vec_sub;
		QList<ImageMarker> ql_marker_sub2tar_affine(ql_marker_sub);
		for(long i=0;i<n_sub_marker;i++)
		{
			ql_marker_sub2tar_affine[i].x=x_sub2tar_affine(1,i+1);
			ql_marker_sub2tar_affine[i].y=x_sub2tar_affine(2,i+1);
			ql_marker_sub2tar_affine[i].z=x_sub2tar_affine(3,i+1);
			vec_sub2tar_affine[i].x=x_sub2tar_affine(1,i+1);
			vec_sub2tar_affine[i].y=x_sub2tar_affine(2,i+1);
			vec_sub2tar_affine[i].z=x_sub2tar_affine(3,i+1);
		}
//		wirteMarker_file("sub2tar_affine.marker",ql_marker_sub2tar_affine);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. TPS warp affine-warped swc file to target. \n");

	//compute sub2tar tps warp parameters
	//tps_para_wa=[w;a], where w is a n_marker*1 vector and a is a 3*1 vector
	Matrix x_tpspara_wa_sub2tar;
	if(!q_compute_tps_paras_3D(vec_sub2tar_affine,vec_tar,x_tpspara_wa_sub2tar))
	{
		printf("ERROR: q_compute_tps_paras_3D() return false.\n");
		return false;
	}

	//TPS warp subject swc to target
	NeuronTree nt_sub2tar_affine_tps(nt_sub2tar_affine);
	{
		Coord3D_PCM pt_sub,pt_sub_tps;
		for(long i=0;i<nt_sub2tar_affine_tps.listNeuron.size();i++)
		{
			pt_sub.x=nt_sub2tar_affine_tps.listNeuron[i].x;
			pt_sub.y=nt_sub2tar_affine_tps.listNeuron[i].y;
			pt_sub.z=nt_sub2tar_affine_tps.listNeuron[i].z;

			if(!q_compute_ptwarped_from_tpspara_3D(pt_sub,vec_sub2tar_affine,x_tpspara_wa_sub2tar,pt_sub_tps))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return false;
			}

			nt_sub2tar_affine_tps.listNeuron[i].x=pt_sub_tps.x;
			nt_sub2tar_affine_tps.listNeuron[i].y=pt_sub_tps.y;
			nt_sub2tar_affine_tps.listNeuron[i].z=pt_sub_tps.z;
		}
	}
    QStringList infostr;
	if(!qs_filename_sub2tar_affine_tps.isEmpty())
		writeSWC_file(qPrintable(qs_filename_sub2tar_affine_tps),nt_sub2tar_affine_tps, &infostr);

	//TPS warp subject markers to target
	vector<Coord3D_PCM> vec_sub2tar_affineTPS;
	{
		Coord3D_PCM pt_sub_tps;
		for(long i=0;i<vec_sub2tar_affine.size();i++)
		{
			if(!q_compute_ptwarped_from_tpspara_3D(vec_sub2tar_affine[i],vec_sub2tar_affine,x_tpspara_wa_sub2tar,pt_sub_tps))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return false;
			}
			vec_sub2tar_affineTPS.push_back(pt_sub_tps);
		}

		QList<ImageMarker> ql_marker_sub2tar_affineTPS(ql_marker_sub);
		for(long i=0;i<ql_marker_sub.size();i++)
		{
			ql_marker_sub2tar_affineTPS[i].x=vec_sub2tar_affineTPS[i].x;
			ql_marker_sub2tar_affineTPS[i].y=vec_sub2tar_affineTPS[i].y;
			ql_marker_sub2tar_affineTPS[i].z=vec_sub2tar_affineTPS[i].z;
		}
//		wirteMarker_file("sub2tar_affine_tps.marker",ql_marker_sub2tar_affineTPS);
	}

	//save affine and TPS warped markers to apo file
	{
		char *filename_apo="sub2tar_affine_tps.apo";

		//save to apo file
		FILE *fp;
		fp=fopen(filename_apo,"w");
		if(fp==NULL)
		{
			fprintf(stderr,"ERROR: Failed to open file to write! \n");
			return false;
		}
		int n=0;
		double volsize=10;	//size of ball
		for(int i=0;i<ql_marker_tar.size();i++)	//tar-red
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,255,0,0\n",n,n,ql_marker_tar[i].z,ql_marker_tar[i].x,ql_marker_tar[i].y,volsize);
			n++;
		}
		for(int i=0;i<ql_marker_sub.size();i++)	//sub_affine-green
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,255,0\n",n,n,vec_sub2tar_affine[i].z,vec_sub2tar_affine[i].x,vec_sub2tar_affine[i].y,volsize);
			n++;
		}
		for(int i=0;i<ql_marker_sub.size();i++)	//sub_affine-blue
		{
			fprintf(fp,"%d,%d,,,%.2f,%.2f,%.2f,,,,%.2f,,,,,0,0,255\n",n,n,vec_sub2tar_affineTPS[i].z,vec_sub2tar_affineTPS[i].x,vec_sub2tar_affineTPS[i].y,volsize);
			n++;
		}
		fclose(fp);

		printf("done with saving file: %s\n",filename_apo);
	}


	printf("Program exit success.\n");
	return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_warp_affine_tps\n");
	printf("Input paras:\n");
	printf("\t  -t   <markerFilename_target>	input target marker file full name.\n");
	printf("\t  -s   <markerFilename_subject>	input subject marker file full name.\n");
	printf("\t [-m]  <matchFilename_sub2tar>	input matched-pair index file full name; if NULL, all input markers are deemed to be matched and have same length\n");
	printf("\t  -S   <XFilename_sub>	        input subject (swc) file full name.\n");
	printf("Output paras:\n");
	printf("\t [-a]  <XFilename_sub2tar_affine> output sub2tar affine warped (image,marker,swc,apo) file full name.\n");
	printf("\t [-o]  <XFilename_sub2tar_affine_tps>output sub2tar affine+tps warped (image,marker,swc,apo) file full name.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

