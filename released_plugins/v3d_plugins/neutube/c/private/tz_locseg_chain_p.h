typedef struct _Path_Test_Workspace{
    double mean_inten_thre;
    double var_inten_thre;
    double gap_thre;
    double mexican_thre;
    double gap_factor;
    double z_res;
} Path_Test_Workspace;

#if 0
static void default_path_test_workspace(Path_Test_Workspace *ws)
{
  ws->mean_inten_thre = 56;
  ws->var_inten_thre = 0;
  ws->gap_thre = 5.0;
  ws->mexican_thre = 0.2;
  ws->gap_factor = 1.5;
  ws->z_res = 1.0;
}

static void Mexin_Hat_Filter1D(double *x, int pn, double sigma, double *y){
    double sigma2 = sigma*sigma;
    float step = 0.5;
    int width = 3*sigma;
    double w = 1/(sqrt(2*3.1416)*sigma)*(1/sigma2);
    int side_len = width*(1.0/step);
    int mf_len = 2*side_len+1;
    //double *mf = new double[mf_len];
    double *mf = darray_malloc(mf_len);
    for(int i=0; i<mf_len; ++i){
        double t = i*step-width;
        mf[i] = w*exp(-t*t/(2*sigma2))*(t*t/sigma2-1);
    }

    int res_len = mf_len+pn;
    //double *res = new double[res_len];
    double *res = darray_malloc(res_len);
    darray_convolve(x,pn,mf,mf_len,0, res);

    int z = 0;
    for(int i=side_len; i<res_len-side_len-1; ++i){
        y[z++] = res[i];
    }

    //delete []mf;
    //delete []res;
    free(mf);
    free(res);
}

static int path_dark_length(double *data, int n, float MF_thre, float inten_thre,
int dark_len){
    int max_len = 0;
    int clen = 0;
    BOOL dark_start = FALSE;
    int cur_dark_start_id = -1;
    int global_dark_start_id = -1;

    //double *x =new double[n];
    double *x = darray_malloc(n);
    double mx = 0.0;
    for(int i=0; i<n; ++i){
        x[i] = data[i]/255.0;
        mx += x[i];
    }
    mx = mx/n;
    for(int i=0; i<n; ++i){
        x[i] = x[i] - mx;
    }

    double sigma = 1.0;
    //double *y = new double[n];
    double *y = darray_malloc(n);
    Mexin_Hat_Filter1D(x, n, sigma, y);

    for(int i=0; i<n; ++i){
        if((y[i] > MF_thre && data[i] < inten_thre) ||
data[i]<inten_thre *0.5){
            if(!dark_start)
                cur_dark_start_id = i;
            dark_start = TRUE;
            clen++;
        }else if(y[i] >= -MF_thre && data[i] < inten_thre){
            if(dark_start)
                clen++;
        }else if((y[i] < -MF_thre ||data[i] > inten_thre) &&
dark_start){
            if(max_len<clen){
                max_len = clen;
                global_dark_start_id = cur_dark_start_id;
            }
            dark_start = FALSE;
            clen = 0;
        }
    }

    dark_len = max_len;
    //delete []x;
    //delete []y;
    free(x);
    free(y);
    return global_dark_start_id;
}

static BOOL is_path_valid(const Stack *stack, int *inds, int path_len,
			  Path_Test_Workspace *ws){
    BOOL is_valid = FALSE;
    if(path_len>1){
      //double *intens = new double[path_len];
      double *intens;
      GUARDED_MALLOC_ARRAY(intens, path_len, double);
      int z;
        for(z = 0; z<path_len; z++){
	  intens[z] = Stack_Array_Value(stack, inds[z]);//stack->array[inds[z]];
        }

        // compute mean intensity
        double sum_inten = 0.0;
        for(z=0; z<path_len; ++z)
            sum_inten += intens[z];
        double mean_inten = sum_inten/path_len;

        // compute variance
	/*
        double ds = 0.0;
        for(z=0; z<path_len; ++z){
            ds += (intens[z]- mean_inten)*(intens[z]- mean_inten);
        }
        double var_inten = sqrt(ds/path_len);
	*/

        // compute gap length
        int dark_len = -1;
        int dark_start_id = path_dark_length(intens, path_len,
					     ws->mexican_thre,
					     ws->gap_factor*ws->mean_inten_thre, dark_len);
        double darklen = 0;

        if(dark_start_id>-1){ // indicating there is a valid dark segment on the path.
            int p1[3], p2[3];
	    int di;
	    int dim[3];
	    dim[0] = stack->width;
	    dim[1] = stack->height;
	    dim[2] = stack->depth;

            for(di = dark_start_id; di<(dark_start_id+dark_len-1); di++){
                ind2sub(dim, 3, inds[di], p1);
                ind2sub(dim, 3, inds[di+1], p2);
                darklen +=
		  sqrt((p1[0]-p2[0])*(p1[0]-p2[0])+(p1[1]-p2[1])*(p1[1]-p2[1])
		       +ws->z_res*(p1[2]-p2[2])*(p1[2]-p2[2]));
            }
        }

        if(mean_inten >= ws->mean_inten_thre && darklen <= ws->gap_thre)
	  is_valid = TRUE;
	
        //delete []intens;
	free(intens);
	
    }

    return is_valid;
}

double Locseg_Chain_Close_Segs(Local_Neuroseg *head_seg1, Local_Neuroseg
*tail_seg1,
                               Local_Neuroseg *head_seg2,
Local_Neuroseg *tail_seg2,
                               Local_Neuroseg
**terminal_seg1, Local_Neuroseg **terminal_seg2){

    double head_pos1[3],
tail_pos1[3];
    Local_Neuroseg_Top(head_seg1, head_pos1);

Local_Neuroseg_Bottom(tail_seg1, tail_pos1);

    double head_pos2[3],
tail_pos2[3];
    Local_Neuroseg_Top(head_seg2, head_pos2);

Local_Neuroseg_Bottom(tail_seg2, tail_pos2);

    double min_dist =
100000.00;
    double ldist = Coordinate_3d_Distance(head_pos1, head_pos2);

if(ldist<min_dist){
        min_dist = ldist;
        *terminal_seg1 =
head_seg1;
        *terminal_seg2 = head_seg2;
    }

    ldist =
Coordinate_3d_Distance(head_pos1, tail_pos2);
    if(ldist<min_dist){

min_dist = ldist;
        *terminal_seg1 = head_seg1;
        *terminal_seg2
= tail_seg2;
    }

    ldist = Coordinate_3d_Distance(tail_pos1,
head_pos2);
    if(ldist<min_dist){
        min_dist = ldist;

*terminal_seg1 = tail_seg1;
        *terminal_seg2 = head_seg2;
    }


ldist = Coordinate_3d_Distance(tail_pos1, tail_pos2);

if(ldist<min_dist){
        min_dist = ldist;
        *terminal_seg1 =
tail_seg1;
        *terminal_seg2 = tail_seg2;
    }

    return
ldist;
}

BOOL Locseg_Chain_Orientation(Locseg_Chain *chain, Local_Neuroseg
*terminal_seg, double orien_seg_len, double *orientation){
    double vec[3];
    double len = 0.0;
    Local_Neuroseg
*lseg;
    BOOL is_head = FALSE;

    Object_3d *ps_obj = Make_Object_3d(40,
0);
    int pn = 0;
    double head_pos[3], tail_pos[3];


Locseg_Chain_Iterator_Start(chain, DL_HEAD);
    Local_Neuroseg *head_seg =
Locseg_Chain_Peek_Seg(chain);
    if(terminal_seg != head_seg){

is_head = FALSE;
        Locseg_Chain_Iterator_Start(chain, DL_TAIL);

lseg = terminal_seg;

        while(len<orien_seg_len && lseg!=NULL){

len += lseg->seg.h;
            Local_Neuroseg_Top(lseg, head_pos);

Local_Neuroseg_Bottom(lseg, tail_pos);
            ps_obj->voxels[pn][0] =
tail_pos[0];
            ps_obj->voxels[pn][1] = tail_pos[1];

ps_obj->voxels[pn][2] = tail_pos[2];
            pn++;

ps_obj->voxels[pn][0] = head_pos[0];
            ps_obj->voxels[pn][1] =
head_pos[1];
            ps_obj->voxels[pn][2] = head_pos[2];

pn++;
            lseg = Locseg_Chain_Prev_Seg(chain);
        }
    }else{

is_head = TRUE;
        Locseg_Chain_Iterator_Start(chain, DL_HEAD);

lseg = terminal_seg;

        while(len<orien_seg_len && lseg!=NULL){

len += lseg->seg.h;
            Local_Neuroseg_Top(lseg, head_pos);

Local_Neuroseg_Bottom(lseg, tail_pos);
            ps_obj->voxels[pn][0] =
head_pos[0];
            ps_obj->voxels[pn][1] = head_pos[1];

ps_obj->voxels[pn][2] = head_pos[2];
            pn++;

ps_obj->voxels[pn][0] = tail_pos[0];
            ps_obj->voxels[pn][1] =
tail_pos[1];
            ps_obj->voxels[pn][2] = tail_pos[2];

pn++;
            lseg = Locseg_Chain_Next_Seg(chain);
        }
    }


ps_obj->size = pn;
    Object_3d_Orientation(ps_obj, vec, MAJOR_AXIS);

orientation[0] = vec[0];
    orientation[1] = vec[1];
    orientation[2] =
vec[2];

    return is_head;
}

BOOL Locseg_Chain_Is_Extension(Locseg_Chain
*chain1, Locseg_Chain *chain2, BOOL *is_head1, BOOL *is_head2){
    BOOL
is_extension = FALSE;
    Locseg_Chain_Iterator_Start(chain1, DL_HEAD);

Local_Neuroseg* head_seg1 = Locseg_Chain_Peek_Seg(chain1);

Locseg_Chain_Iterator_Start(chain1, DL_TAIL);
    Local_Neuroseg* tail_seg1
= Locseg_Chain_Peek_Seg(chain1);

    Locseg_Chain_Iterator_Start(chain2,
DL_HEAD);
    Local_Neuroseg* head_seg2 = Locseg_Chain_Peek_Seg(chain2);

Locseg_Chain_Iterator_Start(chain2, DL_TAIL);
    Local_Neuroseg* tail_seg2
= Locseg_Chain_Peek_Seg(chain2);

    Local_Neuroseg *terminal_seg1,
*terminal_seg2;
    double ldist = Locseg_Chain_Close_Segs(head_seg1,
tail_seg1, head_seg2, tail_seg2, &terminal_seg1, &terminal_seg2);
    double
dist_thre = 20.0; // xiejun
    double orien_seg_len = 20.0;
    double
angle_low_thre = 3.1416/4.0;
    double angle_high_thre = 3.1416*3.0/4.0;


double angle = 0.0;

    if(ldist < dist_thre){
        coordinate_3d_t
orientation1, orientation2;
        *is_head1 =
Locseg_Chain_Orientation(chain1, terminal_seg1, orien_seg_len,
orientation1);
        *is_head2 = Locseg_Chain_Orientation(chain2,
terminal_seg2, orien_seg_len, orientation2);
        angle =
Coordinate_3d_Angle2(orientation1, orientation2);

if(angle<angle_low_thre || angle > angle_high_thre){

is_extension = TRUE;
        }
    }

    return is_extension;
}
#endif
