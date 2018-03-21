/* main_v3dneuron_tracing.cpp
 * V3DNeuron Tracing
 * by Hanchuan Peng
 * 2010-05-24
 * Last change: Nov 30, 2010
 * Last change: Sept 7, 2011
 * Last change: April 2, 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FREAL float

#include "stackutil.h"
#include "basic_memory.h"
#include "volimg_proc.h"
#include "basic_surf_objs.h"
#include "autoseed.h"

#include "v3dneuron_gd_tracing.h"

#define __USE_HIERARCHICAL_PRUNING__

#ifdef __USE_HIERARCHICAL_PRUNING__
#include "fastmarching_dt.h"
#endif

void printHelp ();
void printHelp()
{
        //V3DLONG i; int j; short int k; float tf; double td; unsigned char *p;
        //printf("V3DLONG=%d, int=%d, short=%d, float=%d, double=%d, pointer=%d\n", sizeof(i), sizeof(j), sizeof(k), sizeof(tf), sizeof(td), sizeof(p)); 

	printf("\nNeuronTracer (originally called V3D-Neuron/Vaa3D-Neuron Tracing) (version 1.66 09/17/2011; 1.67 03/07/2012. 1.70 04/3/2012, 1.71 02/10/2013. )\n");
	printf("\nUsage: neurontracer -s <rawImgFile_subject> -c <channel> -o <outfile> -d <downsampling rate> -S <src_marker_file> -D <dst_marker_file> -w <anisotropy factor> -a -m -z -p -g <th_global> -q <transform_method>\n");
	printf("Trace a neuron (or other 3D path) from a 3D image using the V3DNeuron Tracing method. \n");
	printf("\t [-s]                               subject image.\n");
	printf("\t [-c]                               channel number (starting from 0, separated using comma ','; at most 32-byte string.). Use multiple channels for brainbow type of data. Single channel for conventional/non-brainbow data.\n");
	printf("\t [-o]                               output file (a swc file).\n");
	printf("\t [-S]                               source (SRC) marker file, each row is a root node. If unset, then use the first marker in the DST list as the src marker.\n");
	printf("\t [-D]                               destination (DST) marker file, each row is a leaf node. If unset, then for every marker in the SRC list, trace to the entire image.\n");
	printf("\t                                    ** Note: when no -S or -D is specified, then invoke the automatic seed (soma) detection program.\n");
	printf("\t [-d]                               must be a >=1 integer. if specified, use the customary downsampling step (default: 2).\n");
	printf("\t [-a]                               if specified, then use the entire image (but not just the foreground) (default: use only foreground).\n");
	printf("\t [-m]                               if specified, then merge closeby branches (default: no merge).\n");
	printf("\t [-z]                               if specified, then use shortest path ONLY and do NOT use deformable model (default: use GD, i.e. shortest path followed by the deformable model).\n");
	printf("\t [-w]                               the ratio of voxel size along Z and XY plane, used as a weighting factor for anisotropy tracing.\n");
	printf("\t [-p]                               if specified, produce projection patterns of both the original image and the reconstructed neuron for quick proof-reading.\n");
	printf("\t [-y]                               if specified as 0, do NOT produce post-pruned neuron tracing for 1-to-entire image. (default: 1, i.e. use covered-leaf pruning and inter-node pruning). The dark-leaf pruning is alway used.\n");
	printf("\t [-r]                               if specified, this defines the median filter radius (not including the current pixel), i.e. 1 means a 3x3x3 win. If not specified or <=0, then not using median filtering.\n");
	printf("\t [-g]                               if specified, this defines a global threshold of intensity (< this value will be set to 0 before tracing). Default is 0.\n");
	printf("\t [-q]                               if specified, then first distance transform the image (after global thresholding and median filter, if applicable) and then trace on the transformed image.\n");
	printf("\t [-A]                               if specified as 0, then do NOT detect all horizontal straight lines and prune them (if dark) as they should be artifacts. Default set to be 1.\n");
	printf("\t                                        options for transformation: 0 (default. do NOT trace on DT image, -- of course use the input image in this case); 1 (use DT image to trace); 2 (use DT image * original image).\n");
	printf("\t [-h]                               print this message.\n");
	return;
}

bool parseChannelString(char *optarg, V3DLONG * &chan, V3DLONG &nchan)
{
	if (!optarg) return false;
	//printf("The channel string is [%s]\n", optarg);
	
	if (chan) {delete []chan; chan=0;}
	nchan=0;
	
	V3DLONG i,j;
	int MAXLEN=32;
	char curstr[128];
	for (i=0;i<MAXLEN;i++)
	{
		if (optarg[i]!='\0'){ curstr[i] = optarg[i];}
		else { curstr[i] = '\0';break; }
	}
	curstr[i]=',';
	curstr[i+1]='\0';
	V3DLONG slen = i+1;
	//printf("The working copy of channel string is [%s]\n", curstr);
	
	vector <V3DLONG> mychan;
	vector <V3DLONG> pos; 
	for (i=0;i<slen;i++)
		if (curstr[i]==',') pos.push_back(i); 
	int nfield = pos.size();
	
	nchan=0;
	for (i=0;i<nfield;i++)
	{
		curstr[pos[i]]='\0';
		V3DLONG pos1;
		if (i==0) 
		{
			if (pos[i]>0) //so that the string dos not start with comma
			{
				pos1=0;
				mychan.push_back(atoi(curstr+pos1));
			}
		}
		else 
		{
			pos1=pos[i-1]+1;
			if (pos[i]-pos[i-1]>1)
				mychan.push_back(atoi(curstr+pos1));
		}
	}
	
	if (mychan.size()<=0) return false;
	
	//return only the unique channels
	
	vector <V3DLONG>::iterator it;
	it = unique(mychan.begin(), mychan.end());
	
	mychan.resize(it - mychan.begin());
	
	nchan = mychan.size();
	chan = new V3DLONG [nchan];
	for (i=0;i<nchan;i++)
	{
		chan[i] = mychan[i];
	}

	for (i=0;i<nchan;i++)
		printf("You specified channel [%ld] to use.\n", chan[i]);
	return true;
}


#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

int main (int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	// Read arguments

	char *dfile_subject = NULL;
	char *dfile_output = NULL;
	char *src_markers_file = NULL;
	char *dst_markers_file = NULL;
	V3DLONG channelNo_subject=0;
	V3DLONG *channelsToUse=0;
	V3DLONG nChannelsToUse=0;
	double weight_xy_z=1.0;
	bool b_useEntireImg = false;
	bool b_mergeCloseBranches = false;
	bool b_usedshortestpathonly = false;
	bool b_postTrim = true;
    bool b_pruneArtifactBranches = true;
	int ds_step = 2;
	int medianf_rr = 0;
	double th_global = 0;
	int traceOnDTImg_method = 0; //default do NOT use DT image for tracing
	
	bool b_produceProjectionPatterns = false;

	int c;
	static char optstring[] = "hamzpq:y:s:c:d:o:S:D:w:r:g:A:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;

			case 'a':
				b_useEntireImg = true;
				break;

			case 'm':
				b_mergeCloseBranches = true;
				break;
				
			case 'z':
				b_usedshortestpathonly = true;
				break;
				
			case 'p':
				b_produceProjectionPatterns = true;
				break;
				
			case 'q':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -q.\n");
					return 1;
				}
				traceOnDTImg_method = atoi (optarg);
				if (traceOnDTImg_method!=0 && traceOnDTImg_method!=1 && traceOnDTImg_method!=2 && traceOnDTImg_method!=3) // 3 for gsdt
				{
					fprintf (stderr, "Found illegal value for the option -q (which is %d).\n", traceOnDTImg_method);
					fprintf (stderr, "Reset this option value to its default value (which is 0).\n");
					traceOnDTImg_method = 0;
				}
				break;
				
			case 'y':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -y.\n");
					return 1;
				}
				b_postTrim = (atoi (optarg)) ? true : false;
				break;
				
			case 'A':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -A.\n");
					return 1;
				}
				b_pruneArtifactBranches = (atoi (optarg)) ? true : false;
				break;
				
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				dfile_subject = optarg;
				break;

			case 'c':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				if (!parseChannelString(optarg, channelsToUse, nChannelsToUse))
				//channelNo_subject = atoi (optarg);
				//if (channelNo_subject < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! All channel number must be >=0.\n");
					return 1;
				}
				break;

			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_output = optarg;
				break;

			case 'S': //source 3D location file, should be .marker file
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -S.\n");
					return 1;
				}
				src_markers_file = optarg;
				break;
				
				
			case 'D':  //destination 3D location file, should be .marker file
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -D.\n");
					return 1;
				}
				dst_markers_file = optarg;
				break;
				
			case 'w': //the weighting factor of XY and Z resolutions (pixel size)
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					return 1;
				}
				weight_xy_z = atof (optarg);
				if (weight_xy_z<=0)
				{
					fprintf (stderr, "xy rez of an image must be bigger than 0.\n");
					return 1;
				}
				break;
				
			case 'd': //the downsampling rate 
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				ds_step = atoi (optarg);
				if (ds_step<1)
				{
					fprintf (stderr, "the downsampling rate must >=1. Reset it to the default (=2)\n");
					ds_step=2;
				}
				break;
				
			case 'r': //median filter radius
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -r.\n");
					return 1;
				}
				medianf_rr = atoi (optarg);
				if (medianf_rr<1)
				{
					fprintf (stderr, "you input a median filter value <1, which means no median filter will be used.\n");
					medianf_rr=0;
				}
				break;
				
			case 'g': //global threshold
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -g.\n");
					return 1;
				}
				th_global = atof (optarg);
				if (th_global<=0)
				{
					fprintf (stderr, "you input a global threshold which is <= 0, thus reset to 0.\n");
					th_global=0;
				}
				break;
				
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 0;

			default: 
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 0;
				//abort();
        }
    }

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);
	
	
	if (!channelsToUse) //if not specified the channels, then set the default to be the first channel
	{
		nChannelsToUse=1;
		channelsToUse = new V3DLONG[nChannelsToUse];
		channelsToUse[0] = 0;
	}
	
	for (int i=0;i<nChannelsToUse;i++)
		printf("You really specified channel [%ld] to use.\n", channelsToUse[i]);

	unsigned char * img_subject = 0; // must be initialized as NULL
	V3DLONG * sz_subject = 0; // must be initialized as NULL
	int datatype_subject = 0;
	V3DLONG channel_bytes = 0;
	V3DLONG total_units=0;
	V3DLONG total_bytes=0;
	unsigned char *img_new = 0;
	
	//read file
	{
		//load marker files
		QList <ImageMarker> marker_src; if (src_markers_file) marker_src = readMarker_file(QString(src_markers_file));
		QList <ImageMarker> marker_dst;	if (dst_markers_file) marker_dst = readMarker_file(QString(dst_markers_file));

		if (marker_src.size()<=0 && marker_dst.size()<=0)
		{
			fprintf (stdout, "Both marker files are empty. Will invoke the automatic seed/soma detection... \n");
		}
		else if (marker_src.size()<=0) //when there is no src, there use the first of dst list as src
		{
			fprintf (stderr, "The src marker files [%s] is empty. Use the first marker in the dst list as the src marker, and use the remaing markers in dst list as dst locations. \n", src_markers_file);
			marker_src.append(marker_dst.at(0));
			marker_dst.removeAt(0);
		}
		else if (marker_dst.size()<=0) //if there is no dst, then assume need to trace from multiple src to the enture image
		{
			fprintf (stdout, "Dst marker file [%s] is empty. Thus for every marker in the src list, V3D-Neuron is going to trace to the entire image.  \n", dst_markers_file);
		}
		
		//finally load image
		
		if (loadImage(dfile_subject, img_subject, sz_subject, datatype_subject)!=true)
		{
			fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", dfile_subject);
			goto Label_exit;
		}
		
		if (datatype_subject!=1 && datatype_subject!=2 && datatype_subject!=4)
		{
			fprintf (stderr, "The data type is not 1, 2, or 4. Thus invalid. Exit. \n");
			goto Label_exit;
		}
		
		if (nChannelsToUse>0)
		{
			for (V3DLONG k=0;k<nChannelsToUse;k++)
			{
				if (channelsToUse[k]<0 || channelsToUse[k]>=sz_subject[3])
				{
					fprintf (stderr, "The channel id for tracing is out of range. Exit. \n");
					goto Label_exit;
				}
			}
		}
		else 
		{
			fprintf (stderr, "No channel has been specified. Exit. \n");
			goto Label_exit;
		}
		
		
		if (datatype_subject==1 && nChannelsToUse==1 && channelsToUse[0]==0)
		{
			img_new = img_subject;
		}
		else	
		{
			//fprintf (stderr, "Now only support 8bit data [%s]. Exit. \n", dfile_subject);
			
			total_units = (V3DLONG)sz_subject[0]*(V3DLONG)sz_subject[1]*(V3DLONG)sz_subject[2];
			channel_bytes = total_units * datatype_subject; 
			total_bytes = nChannelsToUse * total_units;
			
			if (img_new) {delete []img_new; img_new=0;}
			try
			{
				img_new = new unsigned char [total_bytes];
				short int *tmp_si = 0;
				float * tmp_float = 0;
				if (datatype_subject==1)
				{
					for (V3DLONG j=0;j<nChannelsToUse;j++)
					{
						unsigned char *datahead = img_subject + (V3DLONG)channelsToUse[j]*channel_bytes;
						unsigned char *targethead = img_new + j*total_units;
						memcpy(targethead, datahead, total_units);
					}
				}
				else if (datatype_subject==2)
				{
					for (V3DLONG j=0;j<nChannelsToUse;j++)
					{
						short int *datahead = (short int *)(img_subject + (V3DLONG)channelsToUse[j]*channel_bytes);
						unsigned char *targethead = img_new + j*total_units;
						short int minv, maxv; V3DLONG pos_min, pos_max; 
						minMaxInVector(datahead, total_units, pos_min, minv, pos_max, maxv);
						if (minv==maxv)
						{
							fprintf (stderr, "The data has the min value equals its max value. Thus the neuron tracing won't make sense. Exit. \n");
							goto Label_exit;
						}
						else
						{
							double dd=double(255.0)/(maxv-minv);
							
							for (V3DLONG i=0;i<total_units;i++)
								targethead[i] = (datahead[i]-minv)*dd;
						}
					}
				}
				else // (datatype_subject==4)
				{
					for (V3DLONG j=0;j<nChannelsToUse;j++)
					{
						float *datahead = (float *)(img_subject + (V3DLONG)channelsToUse[j]*channel_bytes);
						unsigned char *targethead = img_new + j*total_units;
						float minv, maxv; V3DLONG pos_min, pos_max; 
						minMaxInVector(datahead, total_units, pos_min, minv, pos_max, maxv);
						if (minv==maxv)
						{
							fprintf (stderr, "The data has the min value equals its max value. Thus the neuron tracing won't make sense. Exit. \n");
							goto Label_exit;
						}
						else
						{
							double dd=double(255.0)/(maxv-minv);
							
							for (V3DLONG i=0;i<total_units;i++)
								targethead[i] = (datahead[i]-minv)*dd;
						}
					}
				}
			}
			catch(...)
			{
				fprintf (stderr, "The data conversion encounters a problem. Exit. \n");
				goto Label_exit;
			}
		}
		
		unsigned char ****p4d = 0;
		V3DLONG sz_tracing[4]; sz_tracing[0] = sz_subject[0]; sz_tracing[1] = sz_subject[1]; sz_tracing[2] = sz_subject[2]; sz_tracing[3] = nChannelsToUse; 
		if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], img_new))
		{
			fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
			goto Label_exit;
		}
		
		//do computation
		
		//first print a log info
		printf("\n** ============================================================================ **\n");
		printf("  subject file = [%s]\n", dfile_subject);
		printf("  subject file sz = [%ld, %ld, %ld, %ld]\n", sz_subject[0], sz_subject[1], sz_subject[2], sz_subject[3]);
		printf("  subject file channel to trace = [%ld]\n", channelNo_subject);
		printf("  number of src marker(s) = [%d]\n", marker_src.size());
		printf("  number of dst marker(s) = [%d]\n", marker_dst.size());
		if (marker_src.size()<=0 && marker_dst.size()<=0)
			printf("  Will invoke auto-seed/soma detection.\n");
			
		
		if (img_subject && sz_subject 
		    // && marker_src.size()>0
		   )
		{
            CurveTracePara trace_para;
            trace_para.channo = 0; 
            trace_para.sp_graph_resolution_step = ds_step;
            trace_para.b_deformcurve = b_usedshortestpathonly; 
            trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
            trace_para.b_3dcurve_width_from_xyonly = true;
            trace_para.b_post_trimming = b_postTrim;
            trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

			if (nChannelsToUse==1)
			{
				trace_para.sp_graph_background = (b_useEntireImg==true) ?  0 : 1; //0 use the entire image
				if (trace_para.sp_graph_background)
				{
					//double imgMax = getImageMaxValue(p4d[0], sz_subject[0], sz_subject[1], sz_subject[2]);
					double imgAve = getImageAveValue(p4d[0], sz_subject[0], sz_subject[1], sz_subject[2]);
					//double imgStd = getImageStdValue(p4d[0], sz_subject[0], sz_subject[1], sz_subject[2]);
					trace_para.imgTH = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
				}
				else
				{
					trace_para.imgTH = 0;
				}
				
				//set the global threshold if it is higher than the default
				if (trace_para.imgTH < th_global)
					trace_para.imgTH = th_global;
				
				printf("  -----------------------------------------\n");
				printf("  tracing parameters (single channel tracing): \n");
				printf("    downsampling step = [%d]\n", int(trace_para.sp_graph_resolution_step));
				printf("    median filter radius (not including the current pixel) = [%d]\n", int(medianf_rr));
				printf("    if trace using only the foreground (1) or the entire image (0) = [%d]\n", int(trace_para.sp_graph_background));
				printf("    tracing for any voxel/pixel values > [%5.4f]\n", trace_para.imgTH);
				printf("    if use shortest path only (1) or GD (0) = [%d]\n", int(trace_para.b_deformcurve));
				printf("    if post merge closeby branches (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_postMergeClosebyBranches));
				printf("    if estimate radius only using XY plane info (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_3dcurve_width_from_xyonly));
				printf("    if produce projection pattern for proofreading (1 - yes, 0 - no) = [%d]\n", int(b_produceProjectionPatterns));
				printf("    if post trim 1-to-entire img result (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_post_trimming));
				printf("    if prune horizontal straight dark segments (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_pruneArtifactBranches));
				printf("** ============================================================================ **\n");				
				
				printf("\n Run thresholding (th=%5.3f), then median filter (when filter radius >=1), and distance transform (if specified -q option)...\n", trace_para.imgTH);
				{
					for (V3DLONG myii=0;myii<sz_tracing[0]*sz_tracing[1]*sz_tracing[2];myii++)
						if (img_new[myii]<=trace_para.imgTH) img_new[myii]=0; 
					if (medianf_rr>=1)
						if (!median_filter_3d(p4d[0], sz_subject, medianf_rr))
							goto Label_exit; 
					if (traceOnDTImg_method==1 || traceOnDTImg_method==2 || traceOnDTImg_method == 3)
					{
						bool b_to_background = true;
						bool b_3d_dt = true; 
						V3DLONG totallen=sz_subject[0]*sz_subject[1]*sz_subject[2];
						float *dd = new float [totallen];
						
						unsigned char *p_curdata = p4d[trace_para.channo][0][0];
						if(traceOnDTImg_method == 3) 
							fastmarching_dt(p_curdata, dd, sz_subject[0], sz_subject[1], sz_subject[2], 2, trace_para.imgTH);
						else
							dt_v3dneuron(p_curdata, sz_subject, trace_para.imgTH, b_to_background, b_3d_dt, dd);
						
						V3DLONG n;
						
						double maxval=-1, minval=0;
						for (n=0;n<totallen;n++)
							maxval = (maxval<dd[n])?dd[n]:maxval;
						
						if (maxval!=0)
							maxval = 1.0/maxval;
						
						if (traceOnDTImg_method==1 || traceOnDTImg_method == 3)
						{
							for (n=0;n<totallen;n++) //rescale between 0 and 255
								p_curdata[n] = (dd[n] * 255.0 * maxval);
						}
						else {
							for (n=0;n<totallen;n++) //normalize the original intensity using DT values
								p_curdata[n] = (double(p_curdata[n]) * dd[n] * maxval);
						}
						
					}
				}
				
				//auto-seed/soma detection
				
				if (marker_src.size() <= 0)
				{
					marker_src = autoSeed(p4d[0], sz_tracing, weight_xy_z); //to add 120403
					if (marker_src.size()>0) 
                                            writeMarker_file(QString(dfile_subject)+"_autoseed.marker", marker_src);
				}
				
				//tracing
				
				LocationSimple p0;
				vector<LocationSimple> pp; 
				NeuronTree nt;
				
				for (V3DLONG isrc=0; isrc<marker_src.size(); isrc++)
				{
					p0.x = marker_src.at(isrc).x-1; p0.y = marker_src.at(isrc).y-1; p0.z = marker_src.at(isrc).z-1;  //-1 because of the marker save format is 1-based
					
					pp.clear();
					LocationSimple tmpp;
					for (V3DLONG idst=0; idst<marker_dst.size(); idst++)
					{
						tmpp.x = marker_dst.at(idst).x-1; tmpp.y = marker_dst.at(idst).y-1; tmpp.z = marker_dst.at(idst).z-1; //-1 because of the marker save format is 1-based 
						pp.push_back(tmpp);
					}
					
					nt = v3dneuron_GD_tracing(p4d, sz_tracing, 
											  p0, pp, 
											  trace_para, weight_xy_z);
					
					//save a tmp copy for debug purpose
					V3DLONG localsz[4]; localsz[0]=sz_subject[0]; localsz[1]=sz_subject[1]; localsz[2]=sz_subject[2]; localsz[3]=1; 
					saveImage("test_mask.tif", img_new, localsz, 1);
					
					QString outfilename = dfile_output; 
					if (marker_src.size()>0)
						outfilename = QString("%1_%2.swc").arg(outfilename).arg(isrc);
					if(!writeSWC_file(outfilename, nt))
						fprintf(stderr, "Fail to produce the output file %s.\n", qPrintable(outfilename));
					else
					{
						printf("Succeed in tracing the path in the image file [%s] and save to the neuron file [%s].\n", dfile_subject, qPrintable(outfilename));
						
						//save the quick proof-reading projections
						if (b_produceProjectionPatterns)
						{
							NeuronTree nt_proj = nt;
							nt_proj.projection(3); //projection so that z=0
							writeSWC_file(outfilename+"_projz.swc", nt_proj);
							
							unsigned char *p2d_proj = new unsigned char [sz_subject[0]*sz_subject[1]];
							for (V3DLONG tmpk=0; tmpk<sz_subject[2]; tmpk++)
							{
								if (tmpk==0)
								{	
									V3DLONG ind=0;
									for (V3DLONG tmpj=0;tmpj<sz_subject[1]; tmpj++)
										for (V3DLONG tmpi=0;tmpi<sz_subject[0];tmpi++)
											p2d_proj[ind++] = p4d[0][0][tmpj][tmpi];
								}
								else
								{
									V3DLONG ind=0;
									for (V3DLONG tmpj=0;tmpj<sz_subject[1]; tmpj++)
										for (V3DLONG tmpi=0;tmpi<sz_subject[0];tmpi++)
										{
											if (p2d_proj[ind] < p4d[0][tmpk][tmpj][tmpi])
												p2d_proj[ind] = p4d[0][tmpk][tmpj][tmpi];
											ind++;
										}
								}
							}
							V3DLONG sz_img_proj[4]; sz_img_proj[0] = sz_subject[0]; sz_img_proj[1] = sz_subject[1]; sz_img_proj[2] = sz_img_proj[3] = 1; 
							saveImage(qPrintable(outfilename+"_projz.tif"), p2d_proj, sz_img_proj, 1);
							
							FILE *fp=0;
							fp = fopen(qPrintable(outfilename+"_projz.ano"), "wt");
							QFileInfo fi(outfilename);
							fprintf(fp, "GRAYIMG=%s\n", qPrintable(fi.fileName()+"_projz.tif"));
							fprintf(fp, "SWCFILE=%s\n", qPrintable(fi.fileName()+"_projz.swc"));
							fclose(fp);
						}
					}
				}
			}
			else //nChannels>1
			{
				trace_para.sp_graph_background = (b_useEntireImg==true) ?  0 : 1; //0 use the entire image
				if (trace_para.sp_graph_background)
				{
					V3DLONG * chanUse = new V3DLONG [sz_tracing[3]];
					for (V3DLONG tmpi=0;tmpi<sz_tracing[3]; tmpi++) chanUse[tmpi]=tmpi;
					VPoint * v = getImageAveValueVPoint(p4d, sz_tracing, chanUse, sz_tracing[3], 1); //the last parameter means use the maximal of all channels
					double imgAve = v->abs();
					trace_para.imgTH = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
					if (v) {delete v; v=0;}
					if (chanUse) {delete []chanUse; chanUse=0;}
				}
				else
				{
					trace_para.imgTH = 0;
				}
				
				//set the global threshold if it is higher than the default
				if (trace_para.imgTH < th_global)
					trace_para.imgTH = th_global;
				
				printf("  -----------------------------------------\n");
				printf("  tracing parameters (multichannel tracing): \n");
				printf("    downsampling step = [%d]\n", int(trace_para.sp_graph_resolution_step));
				printf("    median filter radius (not including the ciurrent pixel) = [%d]\n", int(medianf_rr));
				printf("    if trace using only the foreground (1) or the entire image (0) = [%d]\n", int(trace_para.sp_graph_background));
				printf("    tracing for any voxel/pixel values > [%5.4f]\n", trace_para.imgTH);
				printf("    if use shortest path only (1) or GD (0) = [%d]\n", int(trace_para.b_deformcurve));
				printf("    if post merge closeby branches (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_postMergeClosebyBranches));
				printf("    if estimate radius only using XY plane info (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_3dcurve_width_from_xyonly));
				printf("    if produce projection pattern for proofreading (1 - yes, 0 - no) = [%d]\n", int(b_produceProjectionPatterns));
				printf("    if post trim 1-to-entire img result (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_post_trimming));
				printf("    if prune horizontal straight dark segments (1 - yes, 0 - no) = [%d]\n", int(trace_para.b_pruneArtifactBranches));
				printf("** ============================================================================ **\n");
				
				printf("\n Run thresholding (th=%5.3f), then median filter (when filter radius >=1), and distance transform (if specified -q option)...\n", trace_para.imgTH);
				{
					for (V3DLONG myii=0;myii<sz_tracing[0]*sz_tracing[1]*sz_tracing[2]*sz_tracing[3];myii++)
						if (img_new[myii]<=trace_para.imgTH) img_new[myii]=0; 
					if (medianf_rr>=1)
					{
						for (V3DLONG i=0;i<sz_tracing[3];i++)
							if (!median_filter_3d(p4d[i], sz_subject, medianf_rr))
								goto Label_exit; 
					}
					if (traceOnDTImg_method==1 || traceOnDTImg_method==2)
					{
						printf("At this moment does not support DT based multichannel tracing.\n");
						goto Label_exit; 
//						bool b_to_background = true;
//						bool b_3d_dt = true; 
//						V3DLONG totallen=sz_subject[0]*sz_subject[1]*sz_subject[2];
//						float *dd = new float [totallen];
//						
//						unsigned char *p_curdata = p4d[trace_para.channo][0][0];
//						dt_v3dneuron(p_curdata, sz_subject, trace_para.imgTH, b_to_background, b_3d_dt, dd);
//						
//						V3DLONG n;
//						
//						double maxval=-1, minval=0;
//						for (n=0;n<totallen;n++)
//							maxval = (maxval<dd[n])?dd[n]:maxval;
//						
//						if (maxval!=0)
//							maxval = 1.0/maxval;
//						
//						if (traceOnDTImg_method==1)
//						{
//							for (n=0;n<totallen;n++) //rescale between 0 and 255
//								p_curdata[n] = (dd[n] * 255.0 * maxval);
//						}
//						else {
//							for (n=0;n<totallen;n++) //normalize the original intensity using DT values
//								p_curdata[n] = (double(p_curdata[n]) * dd[n] * maxval);
//						}
//						
					}
				}
				
				LocationSimple p0;
				vector<LocationSimple> pp; 
				NeuronTree nt;
				
				for (V3DLONG isrc=0; isrc<marker_src.size(); isrc++)
				{
					p0.x = marker_src.at(isrc).x-1; p0.y = marker_src.at(isrc).y-1; p0.z = marker_src.at(isrc).z-1;  //-1 because of the marker save format is 1-based
					
					pp.clear();
					LocationSimple tmpp;
					for (V3DLONG idst=0; idst<marker_dst.size(); idst++)
					{
						tmpp.x = marker_dst.at(idst).x-1; tmpp.y = marker_dst.at(idst).y-1; tmpp.z = marker_dst.at(idst).z-1; //-1 because of the marker save format is 1-based 
						pp.push_back(tmpp);
					}
					
					nt = v3dneuron_GD_tracing(p4d, sz_tracing, 
											  p0, pp, 
											  trace_para, weight_xy_z);
					
					//save a tmp copy for debug purpose
					saveImage("test_mask.tif", img_new, sz_tracing, 1);
					
					QString outfilename = dfile_output; 
					if (marker_src.size()>0)
						outfilename = QString("%1_%2.swc").arg(outfilename).arg(isrc);
					if(!writeSWC_file(outfilename, nt))
						fprintf(stderr, "Fail to produce the output file %s.\n", qPrintable(outfilename));
					else
					{
						printf("Succeed in tracing the path in the image file [%s] and save to the neuron file [%s].\n", dfile_subject, qPrintable(outfilename));
						
						//save the quick proof-reading projections
						if (b_produceProjectionPatterns)
						{
							NeuronTree nt_proj = nt;
							nt_proj.projection(3); //projection so that z=0
							writeSWC_file(outfilename+"_projz.swc", nt_proj);
							
							unsigned char *p2d_proj = new unsigned char [sz_subject[0]*sz_subject[1]];
							for (V3DLONG tmpk=0; tmpk<sz_subject[2]; tmpk++)
							{
								if (tmpk==0)
								{	
									V3DLONG ind=0;
									for (V3DLONG tmpj=0;tmpj<sz_subject[1]; tmpj++)
										for (V3DLONG tmpi=0;tmpi<sz_subject[0];tmpi++)
											p2d_proj[ind++] = p4d[0][0][tmpj][tmpi];
								}
								else
								{
									V3DLONG ind=0;
									for (V3DLONG tmpj=0;tmpj<sz_subject[1]; tmpj++)
										for (V3DLONG tmpi=0;tmpi<sz_subject[0];tmpi++)
										{
											if (p2d_proj[ind] < p4d[0][tmpk][tmpj][tmpi])
												p2d_proj[ind] = p4d[0][tmpk][tmpj][tmpi];
											ind++;
										}
								}
							}
							V3DLONG sz_img_proj[4]; sz_img_proj[0] = sz_subject[0]; sz_img_proj[1] = sz_subject[1]; sz_img_proj[2] = sz_img_proj[3] = 1; 
							saveImage(qPrintable(outfilename+"_projz.tif"), p2d_proj, sz_img_proj, 1);
							
							FILE *fp=0;
							fp = fopen(qPrintable(outfilename+"_projz.ano"), "wt");
							QFileInfo fi(outfilename);
							fprintf(fp, "GRAYIMG=%s\n", qPrintable(fi.fileName()+"_projz.tif"));
							fprintf(fp, "SWCFILE=%s\n", qPrintable(fi.fileName()+"_projz.swc"));
							fclose(fp);
						}
					}
				}
			}

		}
		else
		{
			printf("  !! Unable to run the tracing! Check your files!\n");
			printf("** ============================================================================ **\n");
		}
		
		//free memory
		if (p4d) delete4dpointer(p4d, sz_subject[0], sz_subject[1], sz_subject[2], 1);
	}

	// clean all workspace variables

Label_exit:

	if (img_new && img_new!=img_subject) {delete [] img_new; img_new=0;}
	if (img_subject) {delete [] img_subject; img_subject=0;}
	if (sz_subject) {delete [] sz_subject; sz_subject=0;}
	
	if (channelsToUse) {delete []channelsToUse; channelsToUse=0;}

	return 0;
}



