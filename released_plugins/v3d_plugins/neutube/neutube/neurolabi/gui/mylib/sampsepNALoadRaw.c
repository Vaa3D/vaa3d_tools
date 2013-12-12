//  Command line:
//                 [-c<double(5.0)>] [-e<double(3.5)>] [-s<int(300)>]
//                 [-gp] [-pj] [-nr] [-fx]
//                 <folder:FOLDER>   <core:STRING>   <inputs:FILE> ...
//
//  All results are placed in 'folder' which is created if not present.
//    All file names place there begin with the previx 'core', the remainder
//    further describes the file.  The 'input' is assumed to be 1 or more 16-bit
//    .lsm stacks of 2 or more channels wherein exactly one of these is of the
//    neuropil.  This channel is identified and discarded, channels that remain
//    are numbered consecutively as the channels of the input stacks accumulate.
//
//  Each channel is initially segmented as follows:  Find all 8-connected
//    regions of '-s' pixels or more all of which are '-c' or more standard deviations
//    above the mean.  Add to these regions any pixels 8-connected to them that
//    are '-e' standard deviations above the mean.
//
//  The program always produces the output <core>.neu whose format is described
//    below.  It can produce other outputs including diagnostic or progress messages
//    depending on the setting of the options and the defined variables PROGRESS,
//    VERBOSE, and DEBUG.  The options are as follows:
//
//   -gp  create cluster stacks and projections (<core>.clust<#>.tif & <core.PR.clust<#>.tif)
//   -nr  create neuron stack and projections (<core>.neuron<#>.tif & <core>.PR.neuron<#>.tif)
//   -pj  show projections only, skip making stacks (for both -nr and -gp)
//   -fx  create faux color stacks of cluster color segmentation (<core>.cluster<#>.tif)
//
//  The file <core>.neu is a binary file organized as follows:
//
//     <version #: STRING >
//     <# of input files: INT32 >
//     <file name 1: STRING >
//     <# of eliminated channel in file 1: INT32 >
//     ...
//     <# of regions/neurons: INT32 >
//     <region 1: REGION >
//     ...
//
//  where STRING is a '\n' terminated sequence of bytes, and REGION is the data written
//  by a call to 'Write_Region' in mylib (it can be read with a call to 'Read_Region').

#define PROGRESS
#define VERBOSE
#undef  DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "mylib.h"
#include "array.h"
#include "filters.h"
#include "image.h"
#include "utilities.h"
#include "region.h"
#include "connectivity.h"
#include "draw.h"
#include "histogram.h"

int loadRaw2Stack(char * filename, unsigned char ** img, long ** sz, int datatype); //this is the function of 4-byte raw format.

#define ISCON2N  0
#define ESCON2N  1

#define MIN_OVERLAP 100
#define COL_DRIFT   .25

//  UNIVERSAL GLOBALS

static char   *RezFolder;      //  Path at which to put results
static char   *CoreName;       //  Core name for creating files (sans extension and path prefix)

static int     NumChans;       //  Number of channels
static Array **Images;         //  The series of channel images
static char    NameBuf[5000];  //  A buffer to build file names in as outputs are required

static char   *Letter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//  DATA STRUCTURES

typedef struct
  { int      base;     //  Sum of # of segs in previous channels
    Array   *label;    //  Label field for segmented channel (segs[i] pixels labeled i+1)
    int      nsegs;    //  Number of segments in the partition of this channel
    Region **segs;     //  segs[i] is the i'th region for i in [0,nsegs)
    double   mean;     //  mean signal = estimate of background
    int      ethresh;
    int      cthresh;
  } Segmentation;

void Kill_Segmentation(Segmentation *seg)
{ int i;

  for (i = 0; i < seg->nsegs; i++)
    Kill_Region(seg->segs[i]);
  free(seg->segs);
  Kill_Array(seg->label);
}

typedef struct
  { int    totsegs;   //  Total segments in all channels
    int   *chans;     //  Channel of segment i;
    int   *alist;     //  Segment i overlaps segments head[j] for j in [alist[i],alist[i+1])
    int   *heads;     //  If v and w intersect and v < w, then only v->w is in graph
  } Overlaps;



/////////// From V3D for RAW file utilities
  char checkMachineEndian()
  {
    char e='N'; //for unknown endianness

    long int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
      e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
      e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
      e = 'M';
    else
      e = 'N';

    //printf("[%c] \n", e);
    return e;
  }

  void swap2bytes(void *targetp)
  {
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
  }

  void swap4bytes(void *targetp)
  {
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
  }

char * getSuffix(char *filename)
{
  if (!filename)
    return 0;

  long i=0,k=-1;
  while (filename[i]!='\0')
    {
      if (filename[i]=='.')
	k=i;
      i++;
    }

  if (k==-1 || k==i)
    return 0;

  return (filename+k+1);
}


////////////////// end V3D


void Kill_Overlaps(Overlaps *ovl)
{ free(ovl->chans);
  free(ovl->heads);
  free(ovl->alist);
  free(ovl);
}

typedef struct
  { int    inum;    //   Number of clusters
    int   *ilist;   //   Cluster c is item[j] for j in [ilist[c],ilist[c+1]]
    int   *item;
  } Clusters;

void Kill_Clusters(Clusters *clu)
{ free(clu->item);
  free(clu);
}

typedef struct
  { int    numneu;
    Region **neuron;
    int    **channels;
  } Neurons;

void Kill_Neurons(Neurons *neu)
{ int i;

  for (i = 0; i < neu->numneu; i++)
    Kill_Region(neu->neuron[i]);
  free(neu->neuron);
  free(neu->channels[0]);
  free(neu->channels);
}

//  VARIOUS STACK OUTPUT ROUTINES

void Z_Projection(Array *stack, Array *proj)  // Z-project stack onto proj
{ Indx_Type k, p, q;
  Size_Type area;
  Dimn_Type i, j, depth;
  uint8    *val, *max;

  val   = AUINT8(stack);
  max   = AUINT8(proj);
  depth = stack->dims[2];
  area  = proj->size/3;

  for (q = 0; q < proj->size; q++)
    max[q] = 0;
  p = q = 0;
  for (i = 0; i < 3; i++)
    { for (j = 0; j < depth; j++)
         { for (k = 0; k < area; k++)
             { if (val[p] > max[q])
                 max[q] = val[p];
               p += 1;
               q += 1;
             }
           q -= area;
         }
      q += area;
    }
}

void Output_Neurons(int numneur, Region **neurons, int do_brighten)
{ Array  *stack, *proj;
  int     i, k;

#ifdef PROGRESS
  printf("\nGenerating %d individual neuron stacks\n",numneur);
  fflush(stdout);
#endif

  stack = Make_Array(RGB_KIND,UINT8_TYPE,3,Images[0]->dims);
  proj  = Make_Array(RGB_KIND,UINT8_TYPE,2,Images[0]->dims);

  for (i = 0; i < numneur; i++)
    { Array_Op_Scalar(stack,SET_OP,UVAL,VALU(0));

      for (k = 0; k < NumChans; k++)
        { Array_Bundle plane = *stack;
          Draw_Region_Image(Get_Array_Plane(&plane,k%3),Images[k],neurons[i]);
        }

      if (do_brighten)
        Scale_Array_To_Range(stack,VALU(0),VALU(255));

      if (!Is_Arg_Matched("-pj"))
        { sprintf(NameBuf,"%s/%s.neuron%d.tif",RezFolder,CoreName,i);
          Write_Image(NameBuf,stack,LZW_PRESS);
        }

#ifdef PROGRESS
      printf("*"); fflush(stdout);
#endif

      Z_Projection(stack,proj);
      sprintf(NameBuf,"%s/%s.PR.neuron%d.tif",RezFolder,CoreName,i);
      Write_Image(NameBuf,proj,LZW_PRESS);
    }

  // Kill_Array(proj); these are causing a glib.c memory error in Linux
  // Kill_Array(stack);

#ifdef PROGRESS
  printf("\n"); fflush(stdout);
#endif
}


void Output_Consolidated_Mask(int numneur, Region **neurons, int do_brighten)
{ 

  printf("Starting Output_Consolidated_Mask\n");
  fflush(stdout);

  Array  *mask,*stack;
  int     i, k;
  Indx_Type mp,sp;
  uint8 *m,*s;

  mask = Make_Array(PLAIN_KIND,UINT8_TYPE,3,Images[0]->dims);
  stack = Make_Array(RGB_KIND,UINT8_TYPE,3,Images[0]->dims);
  
  m=AUINT8(mask);
  s=AUINT8(stack);

  printf("ConsolidatedMask mask size=%d\n", (int)mask->size); // plain C has no %z
  printf("ConsolidatedMask stack size=%d\n", (int)stack->size);
  fflush(stdout);

  for (i = 0; i < numneur; i++) {

    if (i<256) {

      for (k = 0; k < NumChans; k++) {
	Array_Bundle plane = *stack;
	Draw_Region_Image(Get_Array_Plane(&plane,k%3),Images[k],neurons[i]);
      }

      int count=0;

      for (mp=0;mp<mask->size;mp++) {
	if (s[mp]>0 ||
	    s[mp+mask->size]>0 ||
	    s[mp+(mask->size * 2)]>0) {
	  count++;
	  m[mp]=i+1;
	}
      }

      printf("For neuron %d, added %d label points\n", i, count);
      fflush(stdout);

      // Clear stack
      for (sp=0;sp<stack->size;sp++) {
	s[sp]=0;
      }

    } else {
      printf("Can only handle 256 neurons with 8-bit label - skipping neuron %d\n", i);
      fflush(stdout);
    }

  }

  sprintf(NameBuf,"%s/ConsolidatedLabel.tif", RezFolder);
  Write_Image(NameBuf,mask,LZW_PRESS);

  Kill_Array(mask);
  Kill_Array(stack);
  
  printf("Finished Output_Consolidated_Mask\n");
  fflush(stdout);
}


void Output_False_Color_Stack(int nsegs, Region **regs, char *name, int which)
{ Array       *stack;
  int          i;
  Color_Bundle color;

#ifdef PROGRESS
  printf("  Generating faux color stack of %s.%s%d.tif\n",CoreName,name,which); fflush(stdout);
  fflush(stdout);
#endif
  
  stack = Make_Array(RGB_KIND,UINT8_TYPE,3,Images[0]->dims);
  Array_Op_Scalar(stack,SET_OP,UVAL,VALU(0));

  color.op = SET_PIX;
  for (i = 0; i < nsegs; i++)
    { color.red   = VALU(rand()%256); 
      color.green = VALU(rand()%256); 
      color.blue  = VALU(rand()%256); 
      Draw_Region(stack,&color,regs[i]);
    }

  sprintf(NameBuf,"%s/%s.%s%d.tif",RezFolder,CoreName,name,which);
  Write_Image(NameBuf,stack,LZW_PRESS);

  Free_Array(stack);
}

void Output_Clusters(Segmentation *segs, Overlaps *ovl, Clusters *clust)
{ Array       *stack, *proj;
  int          i, j, k;

#ifdef PROGRESS
  printf("\nGenerating %d color cluster stacks\n",clust->inum);
  fflush(stdout);
#endif

  stack = Make_Array(RGB_KIND,UINT8_TYPE,3,Images[0]->dims);
  proj  = Make_Array(RGB_KIND,UINT8_TYPE,2,Images[0]->dims);

  for (i = 0; i < clust->inum; i++)
    { Array_Op_Scalar(stack,SET_OP,UVAL,VALU(0));

      for (j = clust->ilist[i]; j < clust->ilist[i+1]; j++)
        { Array_Bundle plane;
          int item = clust->item[j];
          int chan = ovl->chans[item];
          int seg  = item - segs[chan].base;
          for (k = 0; k < NumChans; k++)
            { plane = *stack;
              Draw_Region_Image(Get_Array_Plane(&plane,k%3),
                                Images[k],segs[chan].segs[seg]);
            }
        }

      if (!Is_Arg_Matched("-pj"))
        { sprintf(NameBuf,"%s/%s.clust%d.tif",RezFolder,CoreName,i);
          Write_Image(NameBuf,stack,LZW_PRESS);
        }

#ifdef PROGRESS
      printf("*"); fflush(stdout);
#endif

      Z_Projection(stack,proj);
      sprintf(NameBuf,"%s/%s.PR.clust%d.tif",RezFolder,CoreName,i);
      Write_Image(NameBuf,proj,LZW_PRESS);
    }

  Free_Array(proj);
  Free_Array(stack);

#ifdef PROGRESS
  printf("\n"); fflush(stdout);
#endif
}

//  BI_LEVEL SEGMENTATION OF EACH CHANNEL

static int       SEG_threshc;
static int       SEG_threshe;
static Size_Type SEG_sizemin;
static uint8    *SEG_values;
static uint8    *SEG_labels;
static Size_Type SEG_count;
static boolean   SEG_coretouch;
static int       SEG_id;

boolean InCore(Indx_Type p, void *arg)
{ return (SEG_values[p] >= SEG_threshc); }

void CountCore(Indx_Type p, void *arg)
{ SEG_count += 1; }

boolean GoodCore(Size_Type p, void *arg)
{ boolean good = (SEG_count >= SEG_sizemin);
  SEG_count = 0;
  return (good);
}

void MarkAsIn(Indx_Type p, void *arg)
{ SEG_labels[p]  = 1; }

boolean InExtend(Indx_Type p, void *arg)
{ return (SEG_values[p] >= SEG_threshe); }

void TouchCore(Indx_Type p, void *arg)
{ if (SEG_labels[p] > 0)
    SEG_coretouch = 1;
}

boolean GoodExtend(Size_Type p, void *arg)
{ if (SEG_coretouch)
    { SEG_coretouch = 0;
      SEG_id += 1;
      return (1);
    }
  SEG_coretouch = 0;
  return (0);
}

void SetLabel(Indx_Type p, void *arg)
{ SEG_labels[p] = SEG_id; }

void Segment_Channel(Array *input, Segmentation *seg)
{ double  mean, sdev;
  int     threshc, threshe, sizemin;
  Array  *labels;

  Histogram *hist = Histogram_Array(input,0x100,VALU(1),VALU(0));
  mean = Histogram_Mean(hist);
  sdev = Histogram_Sigma(hist);

  threshc = mean + Get_Double_Arg("-c")*sdev;
  threshe = mean + Get_Double_Arg("-e")*sdev;
  sizemin = Get_Int_Arg("-s");

#ifdef PROGRESS
  printf("\nChannel Segmentation:\n");
  printf("  Mean = %.2f  Std.Dev = %.2f\n",mean,sdev);
  printf("  Thresh-c = %d   Thresh-e = %d  Size-s = %d\n",threshc,threshe,sizemin);
#ifdef DEBUG
  Print_Histogram(hist,stdout,4,BIN_COUNT|CUMULATIVE_COUNT|CLIP_HGRAM,0);
#endif
  fflush(stdout);
#endif

  Free_Histogram(hist);

  labels = Make_Array(PLAIN_KIND,UINT8_TYPE,3,input->dims);
  Array_Op_Scalar(labels,SET_OP,UVAL,VALU(0));

  SEG_threshc = threshc;
  SEG_threshe = threshe;
  SEG_sizemin = sizemin;

  SEG_values    = AUINT8(input);
  SEG_labels    = AUINT8(labels);
  SEG_count     = 0;
  SEG_coretouch = 0;
  SEG_id        = 0;

  // Mark connected-components of pixels >= threshc that have not less than sizemin pixels

  Flood_All(input,0,ISCON2N,NULL,InCore,NULL,CountCore,NULL,GoodCore,NULL,MarkAsIn);

  // Mark all connected components of pixels >= threshe that contain a good core as above

  Flood_All(input,0,ISCON2N,NULL,InExtend,NULL,TouchCore,NULL,GoodExtend,NULL,SetLabel);

  // Capture each labeled region in "labels" with a Region

  { int       i, nsegs;
    Indx_Type p;
    uint8    *val;
    Region  **segs;

    seg->label   = labels;
    seg->nsegs   = nsegs = SEG_id;
    seg->segs    = segs = (Region **) Guarded_Malloc(sizeof(Region *)*nsegs,Program_Name());
    seg->mean    = mean;
    seg->ethresh = threshe;
    seg->cthresh = threshc;

    for (i = 0; i < nsegs; i++)
      segs[i] = NULL;

    val = AUINT8(labels);
    for (p = 0; p < labels->size; p++)
      { i = val[p];
        if (i > 0 && segs[i-1] == NULL)
          segs[i-1] = Record_Basic(labels,0,ISCON2N,p,1,EQ_COMP,VALU(i));
      }
  }
}

//  FIND AND ALL INTERSECTION CHANNEL SEGMENT PAIRS

Overlaps *Find_Overlaps(Segmentation *segs)
{ Overlaps  *ovl;
  int        totsegs, *alist, *heads, *chans;
  int        ecount, emax;
  uint8    **vals;
  Size_Type  k;
  Indx_Type  v, w, p;
  Region    *rci;
  int        c, d;
  int        i, j;

  totsegs = segs[NumChans-1].base + segs[NumChans-1].nsegs;
  alist   = (int *) Guarded_Malloc(sizeof(int)*(totsegs+1),Program_Name());
  chans   = (int *) Guarded_Malloc(sizeof(int)*totsegs,Program_Name());
  emax    = totsegs*NumChans;
  heads   = (int *) Guarded_Malloc(sizeof(int)*emax,Program_Name());
  for (k = 0; k <= totsegs; k++)
    alist[k] = 0;

  vals   = (uint8 **) Guarded_Malloc(sizeof(uint8 *)*NumChans,Program_Name());
  for (k = 0; k < NumChans; k++)
    vals[k] = AUINT8(segs[k].label);

  ecount = 0;
  for (c = 0; c < NumChans-1; c++) {
    printf("channel=%d numSegs=%d\n", c, segs[c].nsegs);
    for (i = 0; i < segs[c].nsegs; i++)
      { rci = segs[c].segs[i];
        alist[segs[c].base+i] = ecount;
        chans[segs[c].base+i] = c;
        for (k = 0; k < rci->rastlen; k += 2)
          { v = rci->raster[k];
            w = rci->raster[k+1];
            for (p = v; p <= w; p++)
              for (d = c+1; d < NumChans; d++) {
		//		printf("vals d=%d p=%d =%d\n", d, p, vals[d][p]);
                if (vals[d][p] > 0)
                  { j = segs[d].base+(vals[d][p]-1);
                    alist[j] += 1;
                    if (alist[j] == MIN_OVERLAP)
                      { if (ecount >= emax)
                          { emax = 1.2*emax + 100;
                            heads = (int *) Guarded_Realloc(heads,sizeof(int)*emax,Program_Name());
                          }
                        heads[ecount++] = j;
                      }
		  }
	      }
	  }
        for (j = segs[c+1].base; j < totsegs; j++)
          alist[j] = 0;
      }
  }
  for (i = 0; i <= segs[c].nsegs; i++)
    { alist[segs[c].base+i] = ecount;
      chans[segs[c].base+i] = c;
    }

  free(vals);

  ovl = (Overlaps *) Guarded_Malloc(sizeof(Overlaps),Program_Name());
  ovl->totsegs = totsegs;
  ovl->alist   = alist;
  ovl->heads   = heads;
  ovl->chans   = chans;

#ifdef VERBOSE
  { int   k, h, c, i, d;

    printf("\nOverlaps:\n");
    for (k = 0; k < ovl->totsegs; k++)
      { c = chans[k];
        d = (k-segs[c].base)+1;
        if (d < 10)
          printf("  ");
        else if (d < 100)
          printf(" ");
        printf("  %c%d:",Letter[c],(k-segs[c].base)+1);
        for (h = alist[k]; h < alist[k+1]; h++)
          { i = heads[h];
            c = chans[i];
            printf("  %c%d",Letter[c],(i-segs[c].base)+1);
          }
        printf("\n");
        fflush(stdout);
      }
    fflush(stdout);
  }
#endif

  return (ovl);
}


//  MERGE OVERLAPPING SEGMENTS INTO CLUSTERS

int FIND(int *fathers, int a)
{ int r, b;

  r = a;
  while (fathers[r] >= 0)
    r = fathers[r];
  while (fathers[a] >= 0)
    { b = fathers[a];
      fathers[a] = r;
      a = b;
    }
  return (r);
}

void UNION(int *fathers, int a, int b)
{ int r1, r2, n;

  r1 = FIND(fathers,a);
  r2 = FIND(fathers,b);
  if (r1 != r2)
    { n = fathers[r1]+fathers[r2];
      if (fathers[r1] < fathers[r2])
        { fathers[r2] = r1;
          fathers[r1] = n;
        }
      else
        { fathers[r1] = r2;
          fathers[r2] = n;
        }
    }
}

Clusters *Merge_Segments(Segmentation *segs, Overlaps *ovl)
{ Clusters *clust;
  int      *fathers, *sets, *which;
  int       nsegs, nsets;
  int       j, k;

  nsegs = ovl->totsegs;

  which   = (int *) Guarded_Malloc(sizeof(int)*(2*nsegs+1),Program_Name());
  sets    = which + nsegs;
  fathers = (int *) Guarded_Malloc(sizeof(int)*nsegs,Program_Name());

  for (k = 0; k < nsegs; k++)
    fathers[k] = -1;

  for (k = 0; k < nsegs; k++)
    for (j = ovl->alist[k]; j < ovl->alist[k+1]; j++)
      UNION(fathers,k,ovl->heads[j]);

  nsets = 0;
  for (k = 0; k < nsegs; k++)
    if (fathers[k] < 0)
      { nsets += 1;
        fathers[k] = -nsets;
      }
    else
      fathers[k] = FIND(fathers,k);

  for (k = 0; k < nsegs; k++)
    if (fathers[k] > 0)
      fathers[k] = fathers[fathers[k]];

  for (k = 0; k < nsegs; k++)
    fathers[k] = -(fathers[k]+1);

  for (k = 0; k <= nsets; k++)
    sets[k] = 0;
  for (k = 0; k < nsegs; k++)
    sets[fathers[k]+1] += 1;
  for (k = 2; k <= nsets; k++)
    sets[k] += sets[k-1];
  for (k = 0; k < nsegs; k++)
    { which[sets[fathers[k]]] = k;
      sets[fathers[k]] += 1;
    }
  for (k = nsegs; k > 0; k--)
    sets[k] = sets[k-1];
  sets[0] = 0;

  free(fathers);

  clust = (Clusters *) Guarded_Malloc(sizeof(Clusters),Program_Name());
  clust->inum   = nsets;
  clust->ilist  = sets;
  clust->item   = which;

#ifdef VERBOSE
  printf("\nClusters:\n");
  for (k = 0; k < nsets; k++)
    { printf("  Set %3d:",k);
      for (j = sets[k]; j < sets[k+1]; j++)
        { int c = ovl->chans[which[j]];
          printf(" %c%d",Letter[c],(which[j]-segs[c].base)+1);
        }
      printf("\n");
      fflush(stdout);
    }
  fflush(stdout);
#endif

  return (clust);
}

//  COLOR SEPARATE CLUSTERS

static uint8     **COL_cval;
static uint32     *COL_mval;

static Size_Type   COL_cnt, COL_bits;
static double      COL_sum;
static Size_Type  *COL_csum;
static Size_Type  *COL_colr;

boolean ColCore(Indx_Type p, void *arg)
{ double sum;
  int    c;

  if (COL_mval[p] != 1)
    return (0);
  sum = 0;
  for (c = 0; c < NumChans; c++)
    { COL_colr[c] = COL_cval[c][p];
      sum += COL_colr[c];
    }
  if (COL_cnt != 0)
    { for (c = 0; c < NumChans; c++)
        if (fabs(COL_colr[c]/sum - COL_csum[c]/COL_sum) >= COL_DRIFT)
          return (0);
    }
  COL_cnt += 1;
  COL_sum += sum;
  for (c = 0; c < NumChans; c++)
    COL_csum[c] += COL_colr[c];
  return (1);
}

boolean GoodColor(Size_Type cnt, void *arg)
{ int x;

  COL_bits += 1;

  COL_cnt = 0;
  COL_sum = 0.;
  for (x = 0; x < NumChans; x++)
    COL_csum[x] = 0;
  return (1);
}

void ColAction(Indx_Type p, void *arg)
{ COL_mval[p] = COL_bits; }

Region **Segment_Clusters(Segmentation *segs, Overlaps *ovl, Clusters *clu, int *preg)
{ Array     *mark;
  uint32    *mval;

  Grid_Id    grid;
  Size_Type  gsize;
  Offs_Type *gngh;

  int        c;
  uint8    **cval;
  Size_Type *csum;

  int         hmax  = 0;
  int         rmax  = 0;
  Region    **regs  = NULL;
  int        *rdone = NULL;
  int        *rlist;
  int        *rhead = NULL;
  int         rcnt;
  Size_Type  *volm = NULL;
  Size_Type  *satr;
  Size_Type **fact;

  int      nmax = 1000;
  int      nreg = 0;
  Region **neurons = (Region **) Guarded_Malloc(sizeof(Region *)*nmax,Program_Name());

  mark = Make_Array(PLAIN_KIND,UINT32_TYPE,3,Images[0]->dims);
  mval = AUINT32(mark);

  cval = (uint8 **) Guarded_Malloc(sizeof(uint8 *)*NumChans,Program_Name());
  csum = (Size_Type *) Guarded_Malloc(sizeof(Size_Type)*2*NumChans,Program_Name());
  fact = (Size_Type **) Guarded_Malloc(sizeof(Size_Type *)*NumChans,Program_Name());

  for (c = 0; c < NumChans; c++)
    cval[c] = AUINT8(Images[c]);

  grid  = Setup_Grid(mark,Program_Name());
  gsize = Grid_Size(grid,ISCON2N);
  gngh  = Grid_Neighbors(grid,ISCON2N);

  COL_cval = cval;
  COL_mval = mval;
  COL_csum = csum;
  COL_colr = csum + NumChans;

  for (c = 0; c < clu->inum; c++)

    if (clu->ilist[c+1] - clu->ilist[c] > 1)

      { Array_Op_Scalar(mark,SET_OP,UVAL,VALU(0));

        { Plain_Bundle one = { SET_OP, VALU(1) };
          int          k;

#ifdef PROGRESS
          printf("\nSegmenting Cluster %d:",c);
#endif
          for (k = clu->ilist[c]; k < clu->ilist[c+1]; k++)
            { int item = clu->item[k];
              int chan = ovl->chans[item];
              int seg  = item - segs[chan].base;
              Region *r = segs[chan].segs[seg];
#ifdef PROGRESS
              printf(" %c%d",Letter[chan],seg+1);
#endif
              Draw_Region(mark,&one,r);
            }
#ifdef PROGRESS
          printf("\n");
#endif
        }
 
        { Indx_Type p, k;

          COL_cnt  = 0;
          COL_sum  = 0.;
          for (k = 0; k < NumChans; k++)
            COL_csum[k] = 0;
          COL_bits = 1;
          Flood_All(mark,0,ISCON2N,NULL,ColCore,NULL,NULL,NULL,GoodColor,NULL,ColAction);

          rcnt = COL_bits-1;
          if (rcnt > rmax)
            { rmax = 1.2*rcnt + 500;
              regs  = (Region **) Guarded_Realloc(regs,sizeof(Region *)*rmax,Program_Name());
              rdone = (int *) Guarded_Realloc(rdone,sizeof(int)*(2*rmax+1),Program_Name());
              rlist = rdone + rmax;
              volm  = (Size_Type *)
                          Guarded_Realloc(volm,sizeof(Size_Type)*(NumChans+2)*rmax,Program_Name()); 
              satr  = volm + rmax;

              fact[0] = satr + rmax;
              for (k = 1; k < NumChans; k++)
                fact[k] = fact[k-1] + rmax;
            } else {
	  }
          if (2*rcnt > hmax)
            { hmax = 2.4*rcnt + 1000;
              rhead = (int *) Guarded_Realloc(rhead,sizeof(int)*hmax,Program_Name());
            }

          for (k = 0; k < rcnt; k++)
            regs[k] = NULL;

          for (p = 0; p < mark->size; p++)
            { k = mval[p];
              if (k > 1 && regs[k-2] == NULL) {
                regs[k-2] = Record_Basic(mark,0,ISCON2N,p,1,EQ_COMP,VALU(k));
	      }
            }
        }

        if (Is_Arg_Matched("-fx"))
          Output_False_Color_Stack(rcnt,regs,"colseg",c);

        { int       m, r, s, x, htop;
          Size_Type k;
          Indx_Type p, v, w;
          boolean  *bnd;
          double    sum;
          uint64    sat, bgt, vol, max;

          htop = 0;
          for (r = 0; r < rcnt; r++)
            { rlist[r] = htop;
              for (x = 0; x < NumChans; x++)
                csum[x] = 0;
              sat  = bgt = vol = 0;
              for (k = 0; k < regs[r]->rastlen; k += 2)
                { v = regs[r]->raster[k];
                  w = regs[r]->raster[k+1];
                  for (p = v; p <= w; p++)
                    { bnd = Boundary_Pixels_3d(grid,p,ISCON2N);
                      for (m = 0; m < gsize; m++)
                        if (bnd[m])
                          { s = mval[p+gngh[m]]-2;
                            if (s >= 0 && s != r)
                              { if (rdone[s] == 0)
                                  { if (htop > hmax)
                                      { hmax  = 1.2*htop + 1000;
                                        rhead = (int *)
                                          Guarded_Realloc(rhead,sizeof(int)*hmax,Program_Name());
                                      }
                                    rhead[htop++] = s;
                                  }
                                rdone[s] = 1;
                              }
                          }
                      max = 0;
                      for (x = 0; x < NumChans; x++)
                        { int co = cval[x][p];
                          csum[x] += co;
                          if (co > max) max = co;
                        }
                      if (max == 255)
                        sat += 1;
                      bgt += max;
                      vol += 1;
                    }
                }
              for (m = rlist[r]; m < htop; m++)
                rdone[rhead[m]] = 0;
              for (x = 0; x < NumChans; x++)
                fact[x][r] = csum[x];
              satr[r] = sat;
              volm[r] = vol;
            }
          rlist[rcnt] = htop;
        }

        { int r, x;

          for (r = 0; r < rcnt; r++)
            if (Region_Volume(regs[r]) >= 500)
              { for (x = 0; x < NumChans; x++)
                  if (fact[x][r]/volm[r] >= segs[x].cthresh)
                    break;
                if (x < NumChans)
                  { if (nreg >= nmax)
                      { nmax = 1.2*nreg + 500;
                        neurons = (Region **)
                             Guarded_Realloc(neurons,sizeof(Region *)*nmax,Program_Name());
                      }
                    neurons[nreg++] = Copy_Region(regs[r]);
                  }
#ifdef DEBUG
                printf("  Neuron %d: %d %d %d\n",nreg-1,r,Region_Volume(regs[r]),x < NumChans);
                fflush(stdout);
#endif
              }
        }

#ifdef VERBOSE
        { int    r, e, n;
          double dsum;

          printf("  Fragments %lld\n",rcnt);
          for (r = 0; r < rcnt; r++)
            if (rlist[r+1] - rlist[r] > 1 || Region_Volume(regs[r]) > 10)
              { Indx_Type first = regs[r]->raster[0];
                dsum = 0.;
                for (e = 0; e < NumChans; e++)
                  dsum += fact[e][r];
                printf("  %d(%d) (%.2f(%d)",r,Region_Volume(regs[r]),
                                            fact[0][r]/dsum,fact[0][r]/volm[r]);
                for (e = 1; e < NumChans; e++)
                  printf(",%.2f(%d)",fact[e][r]/dsum,fact[e][r]/volm[r]);
                printf(") (");
                Print_Coord(stdout,Idx2CoordA(mark,first));
                printf(") sat = %3.1f:",(100.*satr[r])/volm[r]);

                if (rlist[r+1] - rlist[r] > 5)
                  printf("\n                ");
                n = 0;
                for (e = rlist[r]; e < rlist[r+1]; e++)
                  { if (n++ == 10)
                      { n = 0;
                        printf("\n                ");
                      }
                    printf(" ->%d",rhead[e]);
                  }
                printf("\n");
                fflush(stdout);
              }
          fflush(stdout);
        }
#endif

        { int r;

          for (r = 0; r < rcnt; r++)
            Free_Region(regs[r]);
        }
      }

  free(regs);
  free(rdone);
  free(volm);
  free(rhead);

  free(fact);
  free(csum);
  free(cval);
  Free_Array(mark);

  *preg = nreg;
  return (neurons);
}

int Read_All_Channels(string name, int maxchans)
{ Tiff *tif;
  int   depth, height, width, chans;
  int   i;

  if(strcmp(getSuffix(name), "tif")==0 || strcmp(getSuffix(name), "lsm")==0) {

    tif = Open_Tiff(name,"r");
    if (tif == NULL)
      { fprintf(stderr,"Cannot open file %s for reading\n",name);
	exit (1);
      }

    Get_IFD_Shape(tif,&width,&height,&chans);

    printf("width=%d height=%d chans=%d\n", width, height, chans);

    if (chans < 2)
      { fprintf(stderr,"LSM %s does not at least 2 channels\n",name);
	exit (1);
      }

    depth = 0;
    while ( ! Tiff_EOF(tif))
      { int w, h, c;
	Get_IFD_Shape(tif,&w,&h,&c);
	if (w == width && h == height && c == chans)
	  { for (i = 0; i < chans; i++)
	      if (Get_IFD_Channel_Type(tif,i) != UINT16_TYPE)
		{ fprintf(stderr,"Channel %d of %s is not 16-bit)\n",i,name);
		  exit (1);
		}
	    depth += 1;
	  }
	Advance_Tiff(tif);
      }

    Rewind_Tiff(tif);
    
    if (NumChans + chans > maxchans)
      { maxchans = 1.2*(NumChans+chans) + 20;
	Images   = (Array **) Guarded_Realloc(Images,sizeof(Array *)*maxchans,Program_Name());
      }
     
    for (i = NumChans; i < NumChans + chans; i++)
      Images[i] = Make_Array_With_Shape(PLAIN_KIND,UINT16_TYPE,Coord3(depth,height,width));

    depth = 0;
    while ( ! Tiff_EOF(tif))
      { int w, h, c;
	Get_IFD_Shape(tif,&w,&h,&c);
	if (w == width && h == height && c == chans)
	  { for (i = 0; i < chans; i++)
	      { Array_Bundle plane = *(Images[NumChans+i]);
		Get_IFD_Channel(tif,i,Get_Array_Plane(&plane,depth));
	      }
	    depth += 1;
	  }
	Advance_Tiff(tif);
      }

    Close_Tiff(tif);

    NumChans += chans;
    return (maxchans);

  } else if (strcmp(getSuffix(name), "raw")==0 || strcmp(getSuffix(name), "v3draw")==0) {

    unsigned char* img;
    long* sz;
    img=0;
    sz=0;

    int rawLoadResult=loadRaw2Stack(name, &img, &sz, 2);
    if (rawLoadResult!=0) {
      fprintf(stderr, "Load of raw file failed\n");
      exit(1);
    }

    width=sz[0];
    height=sz[1];
    depth=sz[2];
    chans=sz[3];

    printf("width=%d height=%d depth=%d chans=%d\n", width, height, depth, chans);

    if (chans < 2)
      { fprintf(stderr,"File %s does not contain at least 2 channels\n",name);
	exit (1);
      }

    if (NumChans + chans > maxchans)
      { maxchans = 1.2*(NumChans+chans) + 20;
	Images   = (Array **) Guarded_Realloc(Images,sizeof(Array *)*maxchans,Program_Name());
      }
     
    for (i = NumChans; i < NumChans + chans; i++) {
      Images[i] = Make_Array_With_Shape(PLAIN_KIND,UINT16_TYPE,Coord3(depth,height,width));
      {
	int rx,ry,rz;
	uint16 *v;
	Indx_Type p;

	v = AUINT16(Images[i]);
	p = 0;
	long oc=(i-NumChans)*depth*height*width;
	for (rz=0;rz<depth;rz++) {
	  long oz = rz*height*width;
	  for (ry=0;ry<height;ry++) {
	    long oy = ry*width;
	    for (rx=0;rx<width;rx++) {
	      long offset=oc + oz + oy + rx;
	      unsigned char* imgLocation=img + offset*2; // for 16-bit
	      uint16 iv = *((uint16*)imgLocation);
	      v[p++] = iv;
	    }
	  }
	}
      }
    }

    NumChans += chans;
    return (maxchans);

  } else {
    fprintf(stderr, "Do not recognize file suffix %s\n", getSuffix(name));
    exit(1);
  }
}

char *Spec[] = { " [-c<double(5.0)>] [-e<double(3.5)>] [-s<int(300)>] ",
                 " [-gp] [-pj] [-nr] [-fx]",
                 "  <folder:FOLDER>   <core:STRING>   <inputs:FILE> ...",
                 NULL
               };

int main(int argc, char *argv[])
{ FILE *output;

  Process_Arguments(argc,argv,Spec,0);

#ifdef PROGRESS
  printf("\nParameters: c=%g e=%g s=%d\n",
         Get_Double_Arg("-c"),Get_Double_Arg("-e"),Get_Int_Arg("-s"));
  printf("SubFolder:  %s\n",Get_String_Arg("folder"));
  printf("CoreName:   %s\n",Get_String_Arg("core"));
  fflush(stdout);
#endif

  RezFolder = strdup(Get_String_Arg("folder"));
  if (RezFolder[strlen(RezFolder)-1] == '/')
    RezFolder[strlen(RezFolder)-1] = '\0';

  if (mkdir(RezFolder,S_IRWXU|S_IRWXG|S_IRWXO))
    { if (errno != EEXIST)
        { fprintf(stderr,"Error trying to create directory %s: %s\n",RezFolder,strerror(errno)); 
          exit (1);
        }
    }

  CoreName = strdup(Get_String_Arg("core"));

  sprintf(NameBuf,"%s.neu",CoreName);
  output = fopen(NameBuf,"w");
  fprintf(output,"NEUSEP: Version 0.9\n");

  { Histogram *hist;
    int        curchan;
    int        maxchans;
    int        i, n;

    n = Get_Repeat_Count("inputs");
    fwrite(&n,sizeof(int),1,output);

    hist = Make_Histogram(UVAL,0x10000,VALU(1),VALU(0));

    maxchans = 0;
    for (i = 0; i < n; i++)
      { 
	curchan  = NumChans;
        maxchans = Read_All_Channels(Get_String_Arg("inputs",i),maxchans);
	int channelsInCurrentFile=NumChans-curchan;


        { Size_Type sum, max;
          Indx_Type p;
          int       j, wch;
          uint16   *val;

          max = -1;
          for (j = curchan; j < NumChans; j++)
            { val = AUINT16(Images[j]);
              sum = 0;
              for (p = 0; p < Images[j]->size; p++)
                sum += val[p];
              if (sum > max)
                { max = sum;
                  wch = j;
                }
            }

          fprintf(output,"%s\n",Get_String_Arg("inputs",i));
          j = wch-curchan;
          fwrite(&j,sizeof(int),1,output);

#ifdef PROGRESS
          printf("\n  Eliminating channel %d from %s\n",j+1,Get_String_Arg("inputs",i));
          fflush(stdout);
#endif

	  {
	    // Section to write out the reference channel
	    printf("\n Considering reference channel output, channelsInCurrentFile=%d\n", channelsInCurrentFile);
	    fflush(stdout);
	    if (channelsInCurrentFile>2) { // should work with both lsm pair with channels=3, or raw file with channels=4
	      sprintf(NameBuf,"%s/Reference.tif",RezFolder,CoreName,i);
	      Write_Image(NameBuf,Images[wch],LZW_PRESS);
	    }

	  }

          Free_Array(Images[wch]);
          NumChans -= 1;
          for (j = wch; j < NumChans; j++)
            Images[j] = Images[j+1];
        }

        { int        j, ceil;
          Indx_Type  p;
          uint16    *val;

          for (j = curchan; j < NumChans; j++)
            {
              Histagain_Array(hist,Images[j],0);

              ceil = Percentile2Bin(hist,1e-5);

	      if (ceil==0) {
		fprintf(stderr, "Channel must have non-zero values for this program to function\n");
                exit(1);
	      } 

#ifdef PROGRESS
              printf("  Clipping channel %d at ceil = %d\n",j,ceil); fflush(stdout);
              fflush(stdout);
#endif
    
              val  = AUINT16(Images[j]);
              for (p = 0; p < Images[j]->size; p++)
                { if (val[p] > ceil)
                    val[p] = ceil;
                  val[p] = (val[p]*255)/ceil;
                }
    
              Convert_Array_Inplace(Images[j],PLAIN_KIND,UINT8_TYPE,8,0);
            }
    
        }
      }

    Free_Histogram(hist);

    printf("Starting ConsolidatedSignal.tif section\n");
    fflush(stdout);

    // NA addition: write tif with re-scaled intensities to serve as basis for mask file
    {
      Array *signalStack;
      signalStack = Make_Array(RGB_KIND,UINT8_TYPE,3,Images[0]->dims);
      uint8 *sp=AUINT8(signalStack);
      int m;
      Indx_Type signalIndex;
      signalIndex=0;
      for (m=0;m<NumChans;m++) {
	uint8 *ip=AUINT8(Images[m]);
	Indx_Type  channelIndex;
	for (channelIndex=0;channelIndex<Images[m]->size;channelIndex++) {
	  sp[signalIndex++]=ip[channelIndex];
	}
      }
      sprintf(NameBuf,"%s/ConsolidatedSignal.tif", RezFolder);
      Write_Image(NameBuf,signalStack,LZW_PRESS);
      //Free_Array(signalStack); - this is causing a bug
    }

    printf("Finished ConsolidatedSignal.tif section\n");
    fflush(stdout);

  }

  { int           i;
    Segmentation *segs;
    Overlaps     *ovl;
    Clusters     *clust;
    int           numneur;
    Region      **neurons;

    segs = (Segmentation *) Guarded_Malloc(sizeof(Segmentation)*NumChans,Program_Name());

    for (i = 0; i < NumChans; i++)
      { Segment_Channel(Images[i],segs+i);
        if (i == 0)
          segs[i].base = 0;
        else
          segs[i].base = segs[i-1].base + segs[i-1].nsegs;
	printf("channel=%d segmentBase=%d\n", i, segs[i].base);
      }

    ovl     = Find_Overlaps(segs);
    clust   = Merge_Segments(segs,ovl);
    neurons = Segment_Clusters(segs,ovl,clust,&numneur);

    if (Is_Arg_Matched("-gp"))
      Output_Clusters(segs,ovl,clust);
    if (Is_Arg_Matched("-nr"))
      Output_Neurons(numneur,neurons,1);

    // Added for NA
    Output_Consolidated_Mask(numneur,neurons,1);

    fwrite(&numneur,sizeof(int),1,output);
    for (i = 0; i < numneur; i++)
      Write_Region(neurons[i],output);

#ifdef PROGRESS
    printf("\nProduced %d neurons/fragments in %s.neu\n",numneur,CoreName);
    fflush(stdout);
#endif

    printf("DEBUG: starting cleanup\n");
    fflush(stdout);

    for (i = 0; i < numneur; i++) {
      printf("DEBUG: calling Kill_Region on neuron=%d\n", i);
      fflush(stdout);
      Kill_Region(neurons[i]);
    }
    printf("DEBUG: calling Kill_Clusters\n");
    fflush(stdout);
    Kill_Clusters(clust);
    printf("DEBUG: calling Kill_Overlaps\n");
    fflush(stdout);
    //Kill_Overlaps(ovl); - causing a bug
    printf("DEBUG: starting Kill_Segmentation loop\n");
    fflush(stdout);
    for (i = 0; i < NumChans; i++) {
      printf("DEBUG: Kill_Segmentation on index=%d\n", i);
      fflush(stdout);
      Kill_Segmentation(segs+i);
    }
    printf("DEBUG: calling free() on segs\n");
    fflush(stdout);
    free(segs);
  }

  printf("DEBUG: starting filestream cleanup\n");
  fflush(stdout);

  { int i;

    fclose(output);
    free(CoreName);
    free(RezFolder);
    for (i = 0; i < NumChans; i++)
      Kill_Array(Images[i]);
    free(Images);
  }

#ifdef VERBOSE
  printf("\nDid I free all arrays?:\n"); 
  Print_Inuse_List(stdout,4);
#endif

  exit (0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is adapted from the V3D function for reading Hanchuan's RAW format in stackutil.cpp
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* The following is the core function for image stack reading */

int loadRaw2Stack(char * filename, unsigned char ** img, long ** sz, int datatype) //this is the function of 4-byte raw format.
{
  /* This function reads 2-4D image stack from raw data generated by the program "saveStack2Raw.m". */
  /* The input parameters img, sz, and datatype should be empty, especially the pointers "img" and "sz". */

  int berror = 0;

  FILE * fid = fopen(filename, "rb");
  if (!fid)
    {
      printf("Fail to open file for reading.\n");
      berror = 1;
      return berror;
    }

  fseek (fid, 0, SEEK_END);
  long fileSize = ftell(fid);
  rewind(fid);

  /* Read header */

  char formatkey[] = "raw_image_stack_by_hpeng";
  long lenkey = strlen(formatkey);

  if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte. 
    {
      printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
      printf("The fseek-ftell produces a file size = %ld.", fileSize);
      berror = 1;
      return berror;
    }

  char * keyread = (char*)Guarded_Malloc((lenkey+1)*sizeof(char), Program_Name());
  if (!keyread)
    {
      printf("Fail to allocate memory.\n");
      berror = 1;
      return berror;
    }
  long nread = fread(keyread, 1, lenkey, fid);
  if (nread!=lenkey)
    {
      printf("File unrecognized or corrupted file.\n");
      berror = 1;
      return berror;
    }
  keyread[lenkey] = '\0';

  long i;
  if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
    {
      printf("Unrecognized file format.\n");
      if (keyread) {free(keyread); keyread=0;}
      berror = 1;
      return berror;
    }

  char endianCodeData;
  fread(&endianCodeData, 1, 1, fid);
  if (endianCodeData!='B' && endianCodeData!='L')
    {
      printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
      berror = 1;
      if (keyread) {free(keyread); keyread=0;}
      return berror;
    }

  char endianCodeMachine;
  endianCodeMachine = checkMachineEndian();
  if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
      printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
      berror = 1;
      if (keyread) {free(keyread); keyread=0;}
      return berror;
    }

  int b_swap = (endianCodeMachine==endianCodeData)?0:1;
  printf("machine endian=[%c] data endian=[%c] b_swap=%d\n", endianCodeMachine, endianCodeData, b_swap);


  short int dcode = 0;
  fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
  if (b_swap)
    swap2bytes((void *)&dcode);

  switch (dcode)
    {
    case 1:
      datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
      break;

    case 2:
      datatype = 2;
      break;

    case 4:
      datatype = 4;
      break;

    default:
      printf("Unrecognized data type code [%d]. The file type is incorrect or this code is not supported in this version.\n", dcode);
      if (keyread) {free(keyread); keyread=0;}
      berror = 1;
      return berror;
    }

  long unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. 

  unsigned int mysz[4];
  mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
  int tmpn=fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. 
  if (tmpn!=4)
    {
      printf("This program only reads [%d] units.\n", tmpn);
      berror=1;
      return berror;
    }
  if (b_swap)
    {
      for (i=0;i<4;i++)
	{
	  //swap2bytes((void *)(mysz+i));
	  printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
	  swap4bytes((void *)(mysz+i));
	  printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
	}
    }

  if (*sz) {free(*sz); *sz=0;}
  *sz=(long*)Guarded_Malloc(4*sizeof(long), Program_Name());
  if (!(*sz))
    {
      printf("Fail to allocate memory.\n");
      if (keyread) {free(keyread); keyread=0;}
      berror = 1;
      return berror;
    }

  long totalUnit = 1;
  for (i=0;i<4;i++)
    {
      (*sz)[i] = (long)mysz[i];
      totalUnit *= (*sz)[i];
    }

  //mexPrintf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey);
  //mexPrintf("The read sizes are: %ld %ld %ld %ld\n", sz[0], sz[1], sz[2], sz[3]);

  if ((totalUnit*unitSize+4*4+2+1+lenkey) != fileSize)
    {
      printf("The input file has a size [%ld bytes], different from what specified in the header [%ld bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*4+2+1+lenkey);
      printf("The read sizes are: %ld %ld %ld %ld\n", (*sz)[0], (*sz)[1], (*sz)[2], (*sz)[3]);
      printf("The read sizes are: %d %d %d %d\n", mysz[0], mysz[1], mysz[2], mysz[3]);
      if (keyread) {free(keyread); keyread=0;}
      if (*sz) {free(*sz); *sz=0;}
      berror = 1;
      return berror;
    }

  if (*img) {free(*img); *img=0;}
  long totalBytes = unitSize*totalUnit;
  *img = (unsigned char *)Guarded_Malloc(totalBytes*sizeof(unsigned char), Program_Name());
  if (*img==0 || *img==NULL)
    {
      fprintf(stderr, "Fail to allocate memory in loadRaw2Stack().\n");
      if (keyread) {free(keyread); keyread=0;}
      if (*sz) {free(*sz); *sz=0;}
      berror = 1;
      return berror;
    }

  long remainingBytes = totalBytes;
  long nBytes2G = 1024L*1024L*1024L*2L;
  long cntBuf = 0;
  while (remainingBytes>0)
    {
      long curReadBytes = (remainingBytes<nBytes2G) ? remainingBytes : nBytes2G;
      long curReadUnits = curReadBytes/unitSize;
      nread = fread(*img+cntBuf*nBytes2G, unitSize, curReadUnits, fid);
      if (nread!=curReadUnits)
	{
	  printf("Something wrong in file reading. The program reads [%ld data points] but the file says there should be [%ld data points].\n", nread, totalUnit);
	  if (keyread) {free(keyread); keyread=0;}
	  if (*sz) {free(*sz); *sz=0;}
	  if (*img) {free(*img); *img=0;}
	  berror = 1;
	  return berror;
	}
      
      remainingBytes -= nBytes2G;
      cntBuf++;
    }

  // swap the data bytes if necessary 

  if (b_swap==1)
    {
      if (unitSize==2)
	{
	  for (i=0;i<totalUnit; i++)
	    {
	      swap2bytes((void *)(*img+i*unitSize));
	    }
	}
      else if (unitSize==4)
	{
	  for (i=0;i<totalUnit; i++)
	    {
	      swap4bytes((void *)(*img+i*unitSize));
	    }
	}
    }


  // clean and return 

  if (keyread) {free(keyread); keyread = 0;}
  fclose(fid); //bug fix on 060412

  //a debug check of the min max value
  double minvv=10000, maxvv=-1;
  long myii=0;
  for (; myii<(*sz)[0]*(*sz)[1]*(*sz)[2];myii++)
    {
      if (minvv>(*img)[myii]) minvv=(*img)[myii];
      else if (maxvv<(*img)[myii]) maxvv=(*img)[myii];
    }
  printf("*** for loadRaw2Stack() all readin bytes: minvv=%5.3f maxvv=%5.3f\n", minvv, maxvv);
       
  return berror;
}


