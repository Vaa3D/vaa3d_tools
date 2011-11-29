//lobeseg.cpp
//separated from the main file on Aug 22, 2008
//Last update by BL Aug 26, 2008 separate mask generating function from do_lobeseg_bdbminus(), add gen_lobeseg_mask()
//Last update by BL Aug 28, 2008 add Ting's shortest path function do_shortestpath()
//Last update by BL Aug 18, 2008 add do_shortestpath_xz and do_lobeseg_bdbminus3()
//Last update by BLAug 19, 2008 add do_shortestpath_cm() 
//Last update by Hanchuan Peng 080922, update do_shortestpath_cm() 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
extern "C" {
#include "image_lib.h"
}
#include "../neurolabi/c/tz_stack_utils.h"
#include "../neurolabi/c/tz_stack_graph.h"
#include "../neurolabi/c/tz_stack_lib.h"
#include "../neurolabi/c/tz_image_lib.h"

#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/volimg_proc.h"

#include "lobeseg2.h"

#include "../neurolabi/c/tz_darray.h"
#include "../neurolabi/c/tz_u8array.h"

#include "../worm_straighten_c/bdb_minus.h"

bool do_lobeseg_bdbminus(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl, vector<vector<Coord2D> > &vr)
//note: assume the inimg1d and outimg1d have the same size, and normally out_channel_no should always be 2 (i.e. the third channel)
{
	if (!inimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3])
	{
		printf("Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_Left;
	vector<Coord2D> mCoord_Right;
	
	//initialize control points

	int KK = ceil(double(sz[1])/10.0);

	float a_bottom_left[2];
	float a_bottom_right[2];

	float a_top_left[2];
	float a_top_right[2];

	a_bottom_left[0] = (sz[0] - 1)*2/5;	//x
	a_bottom_left[1] = sz[1] - 1;			//y

	a_bottom_right[0] = (sz[0] - 1)*3/5;
	a_bottom_right[1] = sz[1] - 1;

	a_top_left[0] = (sz[0] - 1)/10;
	a_top_left[1] = 0;

	a_top_right[0] = (sz[0] - 1)*9/10;
	a_top_right[1] = 0;

	Coord2D pt;
	int k, z;

	for (k=0; k <KK; k++)
	{
		pt.x = a_bottom_left[0] + k * (a_top_left[0]-a_bottom_left[0])/KK;
		pt.y = a_bottom_left[1] + k * (a_top_left[1]-a_bottom_left[1])/KK;
		mCoord_Left.push_back(pt);

		pt.x = a_bottom_right[0] + k * (a_top_right[0]-a_bottom_right[0])/KK;
		pt.y = a_bottom_right[1] + k * (a_top_right[1]-a_bottom_right[1])/KK;
		mCoord_Right.push_back(pt);
	}

	pt.x = a_top_left[0];
	pt.y = a_top_left[1];
	mCoord_Left.push_back(pt);

	pt.x = a_top_right[0];
	pt.y = a_top_right[1];
	mCoord_Right.push_back(pt);

	vl.push_back(mCoord_Left);
	vr.push_back(mCoord_Right);
	
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);

		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}

		unsigned char ** tmp2d=0;
		new2dpointer(tmp2d, sz[0], sz[1], tmp1d);

		cout << "start computing..." << endl;
		for (z=0;z<sz[2];z++)
		{
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
					tmp2d[jj][ii] = 255 - inimg_4d[in_channel_no][z][jj][ii];
			}
			cout << z << " ";
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl.at(z), vl.at(z), mypara);
			vl.push_back( vl.at(z));
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr.at(z), vr.at(z), mypara);
			vr.push_back( vr.at(z));
		}
		cout << endl << "done computation." << endl << "Now saving file" <<endl;
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);
	}
		
	return true;
}


//now generate the separating surface
bool gen_lobeseg_mask(const V3DLONG sz[4], unsigned char *outimg1d, int out_channel_no, vector<vector<Coord2D> > &vl, vector<vector<Coord2D> > &vr, int *left_bound1d, int *right_bound1d)
{	
	int **left_bound=0;
	int **right_bound=0;
	Coord2D pt;
	int i, j, k, z;
	new2dpointer(left_bound, sz[1], sz[2], left_bound1d);
	new2dpointer(right_bound, sz[1], sz[2], right_bound1d);
	
	for (z=0; z<sz[2]; z++)
	{
		vector<Coord2D> e_left;
		vector<Coord2D> e_right;

		int original=0;
		if (original)
		{
		e_left = vl[z+1];		
		pt.x = vl[z+1][0].x;
		pt.y = sz[1]-1;
		e_left.insert(e_left.begin(), pt);
		e_left.insert(e_left.end(), vl[z+1].begin(), vl[z+1].end());
		pt.x = vl[z+1].end()->x;
		pt.y = 0;
		e_left.push_back(pt);

		e_right = vr[z+1];		
		pt.x = vr[z+1][0].x;
		pt.y = sz[1]-1;
		e_right.insert(e_right.begin(), pt);
		e_right.insert(e_right.end(), vr[z+1].begin(), vr[z+1].end());
		pt.x = vr[z+1].end()->x;
		pt.y = 0;
		e_right.push_back(pt);
		}
		
		int shortestpath=1;
		if (shortestpath)
		{
		e_left = vl[z];		
		pt.x = vl[z][0].x;
		pt.y = sz[1]-1;
		e_left.insert(e_left.begin(), pt);
		//e_left.insert(e_left.end(), vl[z].begin(), vl[z].end());
		pt.x = vl[z].end()->x;
		pt.y = 0;
		e_left.push_back(pt);

		e_right = vr[z];		
		pt.x = vr[z][0].x;
		pt.y = sz[1]-1;
		e_right.insert(e_right.begin(), pt);
		//e_right.insert(e_right.end(), vr[z].begin(), vr[z].end());
		pt.x = vr[z].end()->x;
		pt.y = 0;
		e_right.push_back(pt);
		}
		
		int kk = e_left.size();
		
		for (k=0; k<kk-1; k++)
		{
			double i1 = e_left[k].x;	// "i" is "j" in the matlab code
			double j1 = e_left[k].y;
			double i2 = e_left[k+1].x;
			double j2 = e_left[k+1].y;
			
			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}				
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}
			}
			
			i1 = e_right[k].x;	// "i" is "j" in the matlab code
			j1 = e_right[k].y;
			i2 = e_right[k+1].x;
			j2 = e_right[k+1].y;

			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
		}
		
		//printf("%d ", z);
	}
	//printf("\n");

	{
		unsigned char **** img_output_4d=0;
		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3], outimg1d);

		for (z=0; z<sz[2]; z++)
		{
			for (j=0; j<sz[1]; j++)
			{
				for (i=0; i<sz[0]; i++)
				{
					if (i<left_bound[z][j])
					{
						img_output_4d[out_channel_no][z][j][i] = 254;
					}
					else if (i>right_bound[z][j])
					{
						img_output_4d[out_channel_no][z][j][i] = 255;
					}
					else
					{
						img_output_4d[out_channel_no][z][j][i] = 0;
					}
				}
			}
		}
		
		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
	}

	// clean all workspace variables

	if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
	if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);

	return true;
}

// find shortest path by ting zhao
bool do_shortestpath(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, int out_channel_no)
{
	Stack stack;
	
	stack.width = sz[0];
	stack.height = sz[1];
	stack.depth = sz[2];
	stack.kind = GREY;
	
	Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
	sgw->conn = 8;
	sgw->wf = Stack_Voxel_Weight;
	int z;
		
	int a_bottom_left[2];
	int a_bottom_right[2];
	
	int a_top_left[2];
	int a_top_right[2];
	
	a_bottom_left[0] = round((sz[0] - 1)*2/5);	//x
	a_bottom_left[1] = sz[1] - 1;				//y
	
	a_bottom_right[0] = round((sz[0] - 1)*3/5);
	a_bottom_right[1] = sz[1] - 1;
	
	a_top_left[0] = round((sz[0] - 1)*1/10);
	a_top_left[1] = 0;
	
	a_top_right[0] = round((sz[0] - 1)*9/10);
	a_top_right[1] = 0;
	
	int start_left[3] = {a_bottom_left[0], a_bottom_left[1], 0};
	int end_left[3] = {a_top_left[0], a_top_left[1], 0};
	
	int start_right[3] = {a_bottom_right[0], a_bottom_right[1], 0};
	int end_right[3] = {a_top_right[0], a_top_right[1], 0};
	
	
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);

		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}

		unsigned char ** tmp2d=0;
		new2dpointer(tmp2d, sz[0], sz[1], tmp1d);		
		
		cout << "start computing..." << endl;
		for (z=0;z<sz[2];z++)
		{	
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
					tmp2d[jj][ii] = inimg_4d[in_channel_no][z][jj][ii];
			}
			cout << z << " ";

			stack.array = tmp1d;
			
			// Left
			Stack_Graph_Workspace_Set_Range(sgw, start_left[0], end_left[0], 
											start_left[1], end_left[1], 0, 0);
			
			Int_Arraylist *offset_path_left = Stack_Route(&stack, start_left, end_left, sgw);
			
			int ptx_left[offset_path_left->length];
			int pty_left[offset_path_left->length];
			int ptz_left[offset_path_left->length];
			int ix;
			for (ix=0; ix<offset_path_left->length; ix++) {
				STACK_UTIL_COORD(offset_path_left->array[ix], sz[0], sz[0]*sz[1], ptx_left[ix], pty_left[ix], ptz_left[ix]);
			}
			
			// Right
			Stack_Graph_Workspace_Set_Range(sgw, start_right[0], end_right[0], 
											start_right[1], end_right[1], 0, 0);
			
			Int_Arraylist *offset_path_right = Stack_Route(&stack, start_right, end_right, sgw);
			
			int ptx_right[offset_path_right->length];
			int pty_right[offset_path_right->length];
			int ptz_right[offset_path_right->length];
			
			for (ix=0; ix<offset_path_right->length; ix++) {
				STACK_UTIL_COORD(offset_path_right->array[ix], sz[0], sz[0]*sz[1], ptx_right[ix], pty_right[ix], ptz_right[ix]);
			}

			// draw mask
			for (int jj=0; jj<offset_path_left->length; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii < ptx_left[jj])
						inimg_4d[out_channel_no][z][pty_left[jj]][ii] = 254;
				}
			}
			for (int jj=0; jj<offset_path_right->length; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii > ptx_right[jj])
						inimg_4d[out_channel_no][z][pty_right[jj]][ii] = 255;
				}
			}						

			Kill_Int_Arraylist(offset_path_left);
			Kill_Int_Arraylist(offset_path_right);
		}
		Kill_Stack_Graph_Workspace(sgw);
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);	
		if (inimg_4d) {delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]); inimg_4d=0;}
	}
	return true;
}

// find shortest path with center of mass
bool do_shortestpath_cm(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, int out_channel_no)
{
	Stack stack;
	
	stack.width = sz[0];
	stack.height = sz[1];
	stack.depth = sz[2];
	stack.kind = GREY;
	
	Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
	sgw->conn = 8;
	sgw->wf = Stack_Voxel_Weight;
	int z;
		
	int a_bottom_left[2];
	int a_bottom_right[2];
	
	int a_top_left[2];
	int a_top_right[2];
	
	a_bottom_left[0] = round((sz[0] - 1)*2/5);	//x
	a_bottom_left[1] = sz[1] - 1;				//y
	
	a_bottom_right[0] = round((sz[0] - 1)*3/5);
	a_bottom_right[1] = sz[1] - 1;
	
	a_top_left[0] = round((sz[0] - 1)*1/10);
	a_top_left[1] = 0;
	
	a_top_right[0] = round((sz[0] - 1)*9/10);
	a_top_right[1] = 0;
	
	int start_left[3] = {a_bottom_left[0], a_bottom_left[1], 0};
	int end_left[3] = {a_top_left[0], a_top_left[1], 0};
	
	int start_right[3] = {a_bottom_right[0], a_bottom_right[1], 0};
	int end_right[3] = {a_top_right[0], a_top_right[1], 0};
	
	//cout << a_top_left[0] << ' ' << a_top_right[0] << endl;
	
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);

		unsigned char * tmp1d=0;
		unsigned char * c_tmp1d=0;
		double * cx_tmp1d=0;
		double * cy_tmp1d=0;
		double * dist_cm1d=0;
		
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
			c_tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
			cx_tmp1d = new double [(V3DLONG)sz[0]*sz[1]];
			cy_tmp1d = new double [(V3DLONG)sz[0]*sz[1]];
			dist_cm1d = new double [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}
		
		//unsigned char ** tmp2d=0;
		//new2dpointer(tmp2d, sz[0], sz[1], tmp1d);	
		unsigned char ** c_tmp2d=0;	
		new2dpointer(c_tmp2d, sz[0], sz[1], c_tmp1d);
		double ** cx_tmp2d=0;	
		new2dpointer(cx_tmp2d, sz[0], sz[1], cx_tmp1d);
		double ** cy_tmp2d=0;	
		new2dpointer(cy_tmp2d, sz[0], sz[1], cy_tmp1d);
		double ** dist_cm=0;	
		new2dpointer(dist_cm, sz[0], sz[1], dist_cm1d);
		
				
		cout << "start computing..." << endl;
		int radius_x = 20;
		int radius_y = 10;
		for (z=0;z<sz[2];z++)
		{	
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					//tmp2d[jj][ii] = inimg_4d[in_channel_no][z][jj][ii];
					int tmp2dv = 255-inimg_4d[in_channel_no][z][jj][ii];
					c_tmp2d[jj][ii] = tmp2dv;
					cx_tmp2d[jj][ii] = double(tmp2dv) * ii;
					cy_tmp2d[jj][ii] = double(tmp2dv) * jj;
				}
			}
			
			
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					V3DLONG x0 = round(ii - radius_x);	x0 = (x0<0)?0:x0;
					V3DLONG x1 = round(ii + radius_x);	x1 = (x1>=(sz[0]-1))?(sz[0]-1):x1;
					V3DLONG y0 = round(jj - radius_y); y0 = (y0<0)?0:y0;
					V3DLONG y1 = round(jj + radius_y);	y1 = (y1>=(sz[1]-1))?(sz[1]-1):y1;
				
					//double cx_tmpval, cy_tmpval, d_tmpval;
					double sum_I=0, sum_Ix=0, sum_Iy=0;
					for (int iy=y0; iy<=y1; iy++)
					{
						for (int ix=x0; ix<=x1; ix++)
						{
							if (ix<0 || ix>=sz[0] || iy<0 || iy>=sz[1])
							{
								sum_Ix += 255*ix;
								sum_Iy += 255*iy;
								sum_I += 255;
							}
							else
							{
								sum_Ix += cx_tmp2d[iy][ix];
								sum_Iy += cy_tmp2d[iy][ix];
								sum_I += c_tmp2d[iy][ix];
							}
						}
					}
					
					if (sum_I != 0.0) 
					{
						double tii = (ii - sum_Ix/sum_I); tii *= tii;
						double tjj = (jj - sum_Iy/sum_I); tjj *= tjj;
						double dtmp = sqrt(tii + tjj);
						//cout << dtmp << " ";
						dist_cm[jj][ii] = (128+255.0-c_tmp2d[jj][ii]) * dtmp; 
					} 
					else 
					{
						dist_cm[jj][ii] = 1000000; //0
					}
				
				}
			}
			
			cout << z << " ";			
	
			//stack.array = tmp1d;
			stack.array = (uint8 *) dist_cm1d;
			stack.kind = FLOAT64;
			// Left

			Stack_Graph_Workspace_Set_Range(sgw, start_left[0], end_left[0], 
											start_left[1], end_left[1], 0, 0);
			
			Int_Arraylist *offset_path_left = Stack_Route(&stack, start_left, end_left, sgw);
			
			int ptx_left[offset_path_left->length];
			int pty_left[offset_path_left->length];
			int ptz_left[offset_path_left->length];
			int ix;
			for (ix=0; ix<offset_path_left->length; ix++) {
				STACK_UTIL_COORD(offset_path_left->array[ix], sz[0], sz[0]*sz[1], ptx_left[ix], pty_left[ix], ptz_left[ix]);
			}
			
			//Clean_Graph_Workspace(sgw->gw);
			// Right
			Stack_Graph_Workspace_Set_Range(sgw, start_right[0], end_right[0], 
											start_right[1], end_right[1], 0, 0);
			
			Int_Arraylist *offset_path_right = Stack_Route(&stack, start_right, end_right, sgw);
			
			int ptx_right[offset_path_right->length];
			int pty_right[offset_path_right->length];
			int ptz_right[offset_path_right->length];
			
			for (ix=0; ix<offset_path_right->length; ix++) {
				STACK_UTIL_COORD(offset_path_right->array[ix], sz[0], sz[0]*sz[1], ptx_right[ix], pty_right[ix], ptz_right[ix]);
			}

			// draw mask
			for (int jj=0; jj<offset_path_left->length; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii < ptx_left[jj])
						inimg_4d[out_channel_no][z][pty_left[jj]][ii] = 254;
				}
			}
			
			for (int jj=0; jj<offset_path_right->length; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii > ptx_right[jj])
						inimg_4d[out_channel_no][z][pty_right[jj]][ii] = 255;
				}
			}						

			Kill_Int_Arraylist(offset_path_left);
			Kill_Int_Arraylist(offset_path_right);
		
			
		}
		Kill_Stack_Graph_Workspace(sgw);
		//if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);	
		if (c_tmp2d) delete2dpointer(c_tmp2d, sz[0], sz[1]);	
		if (cx_tmp2d) delete2dpointer(cx_tmp2d, sz[0], sz[1]);	
		if (cy_tmp2d) delete2dpointer(cy_tmp2d, sz[0], sz[1]);	
		if (dist_cm) delete2dpointer(dist_cm, sz[0], sz[1]);	
		if (inimg_4d) {delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]); inimg_4d=0;}
	}
	return true;
}	

bool do_shortestpath_xz(unsigned char *inimg1d, const V3DLONG sz[4], unsigned char *outimg1d, int in_channel_no, int out_channel_no, vector<int> &pp_left, vector<int> &pp_right, vector<int> &left_zz, vector<int> &right_zz)
{	
	Stack stack;
	
	stack.width = sz[0];
	stack.height = sz[2];
	stack.depth = 1;
	stack.kind = GREY;
	
	Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
	sgw->conn = 8;
	sgw->wf = Stack_Voxel_Weight;
	int z;
	
	int a_bottom_left[2];
	int a_bottom_right[2];
	
	int a_top_left[2];
	int a_top_right[2];
	
	a_bottom_left[0] = round((sz[0] - 1)*3/10);	//x
	a_bottom_left[1] = sz[2] - 1;				//y
	
	a_bottom_right[0] = round((sz[0] - 1)*7/10);
	a_bottom_right[1] = sz[2] - 1;
	
	a_top_left[0] = round((sz[0] - 1)*1.5/10);
	a_top_left[1] = 0;
	
	a_top_right[0] = round((sz[0] - 1)*8.5/10);
	a_top_right[1] = 0;
	
	int start_left[3] = {a_top_left[0], a_top_left[1], 0};
	int end_left[3] = {a_bottom_left[0], a_bottom_left[1], 0};
	
	int start_right[3] = {a_top_right[0], a_top_right[1], 0};
	int end_right[3] = {a_bottom_right[0], a_bottom_right[1], 0};
	
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);
		
		unsigned char * tmp1d_xz=0; 
		try{
			tmp1d_xz = new unsigned char [(V3DLONG)sz[0]*sz[2]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}
		
		unsigned char ** tmp2d_xz=0;
		new2dpointer(tmp2d_xz, sz[0], sz[2], tmp1d_xz);		
		
		cout << "start computing..." << endl;
		
		int mid_slice = round(sz[1]/2);
		//cout << "mid_slice" << mid_slice << endl;
		
		for (z=0; z<sz[2]; z++)
		{	
			for (int ii=0; ii<sz[0]; ii++)
			{
				tmp2d_xz[z][ii] = inimg_4d[in_channel_no][z][mid_slice][ii];
			}
		}
		
		stack.array = tmp1d_xz;
		Stack *dstack = Downsample_Stack_Max(&stack, 1, 0, 0);
		
		// Left
		start_left[0] /= 2;
		end_left[0] /= 2;
		Stack_Graph_Workspace_Set_Range(sgw, start_left[0], end_left[0], 
										start_left[1], end_left[1], 0, 0);
		
		Int_Arraylist *offset_path_left = Stack_Route(dstack, start_left, end_left, sgw);
		
		int ptx_left[offset_path_left->length];
		int pty_left[offset_path_left->length];
		int ptz_left[offset_path_left->length];
		int ix;
		for (ix=0; ix<offset_path_left->length; ix++) {
			STACK_UTIL_COORD(offset_path_left->array[ix], dstack->width, dstack->width*dstack->height, ptx_left[ix], ptz_left[ix], pty_left[ix]);
			ptx_left[ix] *= 2;
		}
		
		// Right
		start_right[0] /= 2;
		end_right[0] /= 2;
		//cout << start_right[0] << ' ' << end_right[0] << ' ' << endl;
		Stack_Graph_Workspace_Set_Range(sgw, start_right[0], end_right[0], 
										start_right[1], end_right[1], 0, 0);
		
		 
		Int_Arraylist *offset_path_right = Stack_Route(dstack, start_right, end_right, sgw);
		
		int ptx_right[offset_path_right->length];
		int pty_right[offset_path_right->length];
		int ptz_right[offset_path_right->length];
		
		for (ix=0; ix<offset_path_right->length; ix++) {
			STACK_UTIL_COORD(offset_path_right->array[ix], dstack->width, dstack->width*dstack->height, ptx_right[ix], ptz_right[ix], pty_right[ix]);
			ptx_right[ix] *= 2;
			//cout << ptx_right[ix] << endl;
		}
		Kill_Stack(dstack);
		// get mask coord
		vector<int> left_xz;
		vector<int> right_xz;
		{
			for (int n=offset_path_left->length-1; n>=0; n--)
			{
				left_xz.push_back(ptx_left[n]);
				left_zz.push_back(ptz_left[n]);
			}
			
			for (int n=offset_path_right->length-1; n>=0; n--)
			{
				right_xz.push_back(ptx_right[n]);
				right_zz.push_back(ptz_right[n]);
			}
		}
		
		// left
		vector<double> p_val_left(sz[2], 0);
		double tmp_p_val;
		
		for (int n=0; n<left_xz.size(); n++)
		{
			if (pp_left.at(left_zz.at(n)) == -1)
			{
				pp_left.at(left_zz.at(n)) = left_xz.at(n);
				p_val_left.at(left_zz.at(n)) = inimg_4d[in_channel_no][left_zz.at(n)][mid_slice][left_xz.at(n)];
			}
			else
			{
				tmp_p_val = inimg_4d[in_channel_no][left_zz.at(n)][mid_slice][left_xz.at(n)];
				if (tmp_p_val < p_val_left.at(left_zz.at(n)))
				{
					pp_left.at(left_zz.at(n)) = left_xz.at(n);
					p_val_left.at(left_zz.at(n)) = tmp_p_val;
				}
			}
		}		
					
		// right
		vector<double> p_val_right(sz[2], 0);
		
		for (int n=0; n<right_xz.size(); n++)
		{
			if (pp_right.at(right_zz.at(n)) == -1)
			{
				pp_right.at(right_zz.at(n)) = right_xz.at(n);
				p_val_right.at(right_zz.at(n)) = inimg_4d[in_channel_no][right_zz.at(n)][mid_slice][right_xz.at(n)];
			}
			else
			{
				tmp_p_val = inimg_4d[in_channel_no][right_zz.at(n)][mid_slice][right_xz.at(n)];
				if (tmp_p_val < p_val_right.at(right_zz.at(n)))
				{
					pp_right.at(right_zz.at(n)) = right_xz.at(n);
					p_val_right.at(right_zz.at(n)) = tmp_p_val;
				}
			}
		}		
		
		
		/*
		// draw mask
		{
			unsigned char **** outimg_4d=0;
			new4dpointer(outimg_4d, sz[0], sz[1], sz[2], sz[3], outimg1d);
			
			for (int z=0; z<offset_path_left->length; z++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii < ptx_left[z])
						outimg_4d[out_channel_no][ptz_left[z]][mid_slice][ii] = 254;
				}
			}
			for (int z=0; z<offset_path_right->length; z++)
			{
				for (int ii=0; ii<sz[0]; ii++)
				{
					if (ii > ptx_right[z])
						outimg_4d[out_channel_no][ptz_right[z]][mid_slice][ii] = 255;
				}
			}	
			if (outimg_4d) {delete4dpointer(outimg_4d, sz[0], sz[1], sz[2], sz[3]); outimg_4d=0;}
		}					
		*/
		Kill_Int_Arraylist(offset_path_left);
		Kill_Int_Arraylist(offset_path_right);
		
		if (tmp2d_xz) {delete2dpointer(tmp2d_xz, sz[0], sz[2]); tmp2d_xz=0;}
		if (inimg_4d) {delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]); inimg_4d=0;}
		
	}
	
	Kill_Stack_Graph_Workspace(sgw);

	return true;
}



bool do_lobeseg_bdbminus2(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl_high,
vector<vector<Coord2D> > &vr_high, vector<vector<Coord2D> > &vl_low, vector<vector<Coord2D> > &vr_low, vector<int> a_middle_left, vector<int> a_middle_right)
//note: assume the inimg1d and outimg1d have the same size, and normally out_channel_no should always be 2 (i.e. the third channel)
{
	if (!inimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3])
	{
		printf("Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_Left_high;
	vector<Coord2D> mCoord_Right_high;
	vector<Coord2D> mCoord_Left_low;
	vector<Coord2D> mCoord_Right_low;
	
	//initialize control points

	int KK = ceil(double(sz[1])/20.0);
	int mid_slice = round(sz[1]/2);

	float a_bottom_left[2];
	float a_bottom_right[2];

	float a_top_left[2];
	float a_top_right[2];

	a_bottom_left[0] = (sz[0] - 1)*2/5;	//x
	a_bottom_left[1] = sz[1] - 1;			//y

	a_bottom_right[0] = (sz[0] - 1)*3/5;
	a_bottom_right[1] = sz[1] - 1;

	a_top_left[0] = (sz[0] - 1)/10;
	a_top_left[1] = 0;

	a_top_right[0] = (sz[0] - 1)*9/10;
	a_top_right[1] = 0;

	Coord2D pt;
	int k, z;

		
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);
		
		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}
		
		unsigned char ** tmp2d=0;
		new2dpointer(tmp2d, sz[0], sz[1], tmp1d);
		
		cout << "start computing..." << endl;
		for (z=0;z<sz[2];z++)
		{
			vector<Coord2D> mCoord_Left_high;
			vector<Coord2D> mCoord_Right_high;
			vector<Coord2D> mCoord_Left_low;
			vector<Coord2D> mCoord_Right_low;
	
			for (k=0; k <KK; k++)
			{
				pt.x = a_bottom_left[0] + k * (a_middle_left.at(z) - a_bottom_left[0])/KK;
				pt.y = a_bottom_left[1] + k * (mid_slice - a_bottom_left[1])/KK;
				mCoord_Left_low.push_back(pt);
			
				pt.x = a_middle_left.at(z) + k * (a_top_left[0] - a_middle_left.at(z))/KK;
				pt.y = mid_slice + k * (a_top_left[1] - mid_slice)/KK;
				mCoord_Left_high.push_back(pt);
				
				pt.x = a_bottom_right[0] + k * (a_middle_right.at(z) - a_bottom_right[0])/KK;
				pt.y = a_bottom_right[1] + k * (mid_slice - a_bottom_right[1])/KK;
				mCoord_Right_low.push_back(pt);
				
				pt.x = a_middle_right.at(z) + k * (a_top_right[0] - a_middle_right.at(z))/KK;
				pt.y = mid_slice + k * (a_top_right[1] - mid_slice)/KK;
				mCoord_Right_high.push_back(pt);
			}
			
			pt.x = a_top_left[0];
			pt.y = a_top_left[1];
			mCoord_Left_high.push_back(pt);
			
			pt.x = a_top_right[0];
			pt.y = a_top_right[1];
			mCoord_Right_high.push_back(pt);
			
			vl_high.push_back(mCoord_Left_high);
			vr_high.push_back(mCoord_Right_high);
			vl_low.push_back(mCoord_Left_low);
			vr_low.push_back(mCoord_Right_low);
			
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
					tmp2d[jj][ii] = 255 - inimg_4d[in_channel_no][z][jj][ii];
			}
			cout << z << " ";
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl_high.at(z), vl_high.at(z), mypara);
						
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr_high.at(z), vr_high.at(z), mypara);
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl_low.at(z), vl_low.at(z), mypara);
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr_low.at(z), vr_low.at(z), mypara);
			
		}
		cout << endl << "done computation." << endl << "Now saving file" <<endl;
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);
	}
	
	return true;
}

bool do_lobeseg_bdbminus3(unsigned char *inimg1d, const V3DLONG sz[4], int in_channel_no, const BDB_Minus_ConfigParameter & mypara, vector<vector<Coord2D> > &vl_low, 
vector<vector<Coord2D> > &vr_low, vector<int> a_middle_left, vector<int> a_middle_right)
//note: assume the inimg1d and outimg1d have the same size, and normally out_channel_no should always be 2 (i.e. the third channel)
{
	if (!inimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3])
	{
		printf("Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_Left_high;
	vector<Coord2D> mCoord_Right_high;
	vector<Coord2D> mCoord_Left_low;
	vector<Coord2D> mCoord_Right_low;
	
	//initialize control points

	int KK = ceil(double(sz[1])/20.0);
	int mid_slice = round(sz[1]/2);

	float a_bottom_left[2];
	float a_bottom_right[2];

	float a_top_left[2];
	float a_top_right[2];

	a_bottom_left[0] = (sz[0] - 1)*2/5;	//x
	a_bottom_left[1] = sz[1] - 1;			//y

	a_bottom_right[0] = (sz[0] - 1)*3/5;
	a_bottom_right[1] = sz[1] - 1;

	a_top_left[0] = (sz[0] - 1)/10;
	a_top_left[1] = 0;

	a_top_right[0] = (sz[0] - 1)*9/10;
	a_top_right[1] = 0;

	Coord2D pt;
	int k, z;

		
	//do the computation
	{
		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);
		
		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...){cerr << "Fail to allocate memory for temp image plane.\n" << endl;}
		
		unsigned char ** tmp2d=0;
		new2dpointer(tmp2d, sz[0], sz[1], tmp1d);
		
		cout << "start computing..." << endl;
		for (z=0;z<sz[2];z++)
		{
			vector<Coord2D> mCoord_Left_high;
			vector<Coord2D> mCoord_Right_high;
			vector<Coord2D> mCoord_Left_low;
			vector<Coord2D> mCoord_Right_low;
	
			for (k=0; k <KK; k++)
			{
				pt.x = a_bottom_left[0] + k * (a_middle_left.at(z) - a_bottom_left[0])/KK;
				pt.y = a_bottom_left[1] + k * (mid_slice - a_bottom_left[1])/KK;
				mCoord_Left_low.push_back(pt);
			
				pt.x = a_middle_left.at(z) + k * (a_top_left[0] - a_middle_left.at(z))/KK;
				pt.y = mid_slice + k * (a_top_left[1] - mid_slice)/KK;
				mCoord_Left_high.push_back(pt);
				
				pt.x = a_bottom_right[0] + k * (a_middle_right.at(z) - a_bottom_right[0])/KK;
				pt.y = a_bottom_right[1] + k * (mid_slice - a_bottom_right[1])/KK;
				mCoord_Right_low.push_back(pt);
				
				pt.x = a_middle_right.at(z) + k * (a_top_right[0] - a_middle_right.at(z))/KK;
				pt.y = mid_slice + k * (a_top_right[1] - mid_slice)/KK;
				mCoord_Right_high.push_back(pt);
			}
			
			pt.x = a_top_left[0];
			pt.y = a_top_left[1];
			mCoord_Left_high.push_back(pt);
			
			//cout << "before" << endl;
			//for (int n=0; n<mCoord_Left_high.size(); n++)
			//	cout << mCoord_Left_high.at(n).x << ' ' << mCoord_Left_high.at(n).y << endl; 
			
			mCoord_Left_low.insert(mCoord_Left_low.end(), mCoord_Left_high.begin(), mCoord_Left_high.end());
			
			//cout << "after" << endl;
			//for (int n=0; n<mCoord_Left_high.size(); n++)
			// 	cout << mCoord_Left_high.at(n).x << ' ' << mCoord_Left_high.at(n).y << endl;
			
			pt.x = a_top_right[0];
			pt.y = a_top_right[1];
			mCoord_Right_high.push_back(pt);
			
			mCoord_Right_low.insert(mCoord_Right_low.end(), mCoord_Right_high.begin(), mCoord_Right_high.end());
			
			vl_low.push_back(mCoord_Left_low);
			vr_low.push_back(mCoord_Right_low);
			
			for (int jj=0; jj<sz[1]; jj++)
			{
				for (int ii=0; ii<sz[0]; ii++)
					tmp2d[jj][ii] = 255 - inimg_4d[in_channel_no][z][jj][ii];
			}
			cout << z << " ";
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl_low.at(z), vl_low.at(z), mypara);
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr_low.at(z), vr_low.at(z), mypara);
			
		}
		cout << endl << "done computation." << endl << "Now saving file" <<endl;
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);
	}
	
	return true;
}


//now generate the separating surface
bool gen_lobeseg_mask2(const V3DLONG sz[4], unsigned char *outimg1d, int out_channel_no, vector<vector<Coord2D> > &vl_high, vector<vector<Coord2D> > &vr_high, 
vector<vector<Coord2D> > &vl_low, vector<vector<Coord2D> > &vr_low, int *left_bound1d, int *right_bound1d)
{	
	int **left_bound=0;
	int **right_bound=0;
	
	int i, j, k, z;
	new2dpointer(left_bound, sz[1], sz[2], left_bound1d);
	new2dpointer(right_bound, sz[1], sz[2], right_bound1d);
	
	for (z=0; z<sz[2]; z++)
	{
		vector<Coord2D> e_left;
		vector<Coord2D> e_right;
		
		e_left = vl_low.at(z);		
		e_left.insert(e_left.end(), vl_high.at(z).begin(),vl_high.at(z).end());
		
		e_right = vr_low.at(z);		
		e_right.insert(e_right.end(), vr_high.at(z).begin(), vr_high.at(z).end());
		
		int kk = e_left.size();
		
		for (k=0; k<kk-1; k++)
		{
			double i1 = e_left[k].x;	// "i" is "j" in the matlab code
			double j1 = e_left[k].y;
			double i2 = e_left[k+1].x;
			double j2 = e_left[k+1].y;
			
			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}				
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					left_bound[z][j] = tmpi;
				}
			}
			
			i1 = e_right[k].x;	// "i" is "j" in the matlab code
			j1 = e_right[k].y;
			i2 = e_right[k+1].x;
			j2 = e_right[k+1].y;

			if (j1>j2)
			{
				for (j=j1; j>j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
			else
			{
				for (j=j1; j<j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					right_bound[z][j] = tmpi;
				}
			}
		}
		
		//printf("%d ", z);
	}
	//printf("\n");

	{
		unsigned char **** img_output_4d=0;
		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3], outimg1d);

		for (z=0; z<sz[2]; z++)
		{
			for (j=0; j<sz[1]; j++)
			{
				for (i=0; i<sz[0]; i++)
				{
					if (i<left_bound[z][j])
					{
						img_output_4d[out_channel_no][z][j][i] = 254;
					}
					else if (i>right_bound[z][j])
					{
						img_output_4d[out_channel_no][z][j][i] = 255;
					}
					else
					{
						img_output_4d[out_channel_no][z][j][i] = 0;
					}
				}
			}
		}
		
		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
	}

	// clean all workspace variables

	if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
	if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);

	return true;
}


