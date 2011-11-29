//lobeseg.cpp
//separated from the main file on Aug 22, 2008
//last update: by Hanchuan Peng 090609. set the outputimage's in-chann's lobe rgn to be 0 after the segmentation
//May 16, 2011 : by Hang, cp do_lobeseg_bdbminus -> do_lobeseg_bdbminus_onesideonly
//May 30, 2011 : by Hang, 1. complex boundary detection,  2. any position (x1,y1) (x2,y2)
//June 15, 2011. changed by Hanchuan Peng to allow saving the mask to the last channel
//2011-07-09: add uint16/float32 support

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER) && (_WIN64)
#include "../../../../v3d_main/basic_c_fun/vcdiff.h"
#else
#endif

#include "../../../../v3d_main/basic_c_fun/stackutil.h"
#include "../../../../v3d_main/basic_c_fun/volimg_proc.h"

#include "lobeseg.h"

#include "../../../../v3d_main/worm_straighten_c/bdb_minus.h"


template <class T> bool maskImageUsingSegBoundary(T *outimg1d, V3DLONG sz[4], int in_channel_no, int out_channel_no, int ** left_bound, int ** right_bound)
{
	if (!outimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || !left_bound || !right_bound)
	{
		fprintf(stderr, "Invalid data to maskImageUsingSegBoundry().\n");
		return false;
	}

	int z, i, j;

	//note that for convenience I have not add pointer check in the following part. should add later
	{
		T **** img_output_4d=0;
		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3], outimg1d);

		for (z=0; z<sz[2]; z++)
		{
			for (j=0; j<sz[1]; j++)
			{
				for (i=0; i<sz[0]; i++)
				{

					if (i<left_bound[z][j])
					{
						img_output_4d[in_channel_no][z][j][i] = 0; //added by PHC,090609
					}
					else if (i>right_bound[z][j])
					{
						img_output_4d[in_channel_no][z][j][i] = 0;  //added by PHC,090609
					}
					else
					{
						//do nothing
					}
				}
			}

			if (sz[3]>=out_channel_no) //set up the mask if possible. 090730 //change from sz[3]-1 to sz[3]. to allow save the mask in the last (additional) channel
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

		}

		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
	}

	return true;
}

bool do_lobeseg_bdbminus(unsigned char *inimg1d0, const V3DLONG sz[4], int datatype_input, unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara)
//note: assume the inimg1d and outimg1d have the same size, and normally out_channel_no should always be 2 (i.e. the third channel)
{
	if (!inimg1d0 || !outimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3] || out_channel_no<0)
	{
		printf("Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	if (datatype_input!=1 && datatype_input!=2 && datatype_input!=4)
	{
		fprintf(stderr, "The datatype is NOT UINT8/UINT16/FLOAT. The do_lobeseg_bdbminus() function quits.\n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_Left;
	vector<Coord2D> mCoord_Right;
	vector<vector<Coord2D> > vl;
	vector<vector<Coord2D> > vr;
	
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
	int k, z, i, j;

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
		//first create a buffer of UINT8 type

		V3DLONG totalunits= sz[0]*sz[1]*sz[2]*sz[3];
		unsigned char *inimg1d = 0;
		if (datatype_input==1)
			inimg1d = inimg1d0; //in this case no need to use additional memory
		else if (datatype_input==2 || datatype_input==4) //this conversion part is essential the convert_type2uint8_3dimg_1dpt() function, :-).
		{
			try{
				inimg1d = new unsigned char [totalunits];
			}
			catch (...)
			{
				fprintf(stderr, "Fail to allocate memory for data conversion.\n");
				return false;
			}

			if (datatype_input==2)
			{
				unsigned short int * tmpimg = (unsigned short int *)inimg1d0;
				for (V3DLONG i=0;i<totalunits;i++)
				{
					inimg1d[i] = (unsigned char)(tmpimg[i]>>4); //as I knew it is 12-bit instead of 16 bit
				}
			}
			else
			{
				float * tmpimg = (float *)inimg1d0;
				for (V3DLONG i=0;i<totalunits;i++)
				{
					inimg1d[i] = (unsigned char)(tmpimg[i]*255); //as I knew it is float between 0 and 1
				}
			}
		}

		//now do the real computing

		unsigned char ****inimg_4d = 0;
		new4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3], inimg1d);

		unsigned char * tmp1d=0;
		try{
			tmp1d = new unsigned char [(V3DLONG)sz[0]*sz[1]];
		}
		catch(...)
		{
			fprintf(stderr, "Fail to allocate memory for a temporary buffer of an image plane.\n");
			return false;
		}

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
			fprintf(stdout, "%ld ", z);
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vl.at(z), vl.at(z), mypara);
			vl.push_back( vl.at(z));
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], vr.at(z), vr.at(z), mypara);
			vr.push_back( vr.at(z));
		}
		fprintf(stdout, "\ndone lobeseg computation.\n\n");
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);

		//delete the UINT8 buffer is necessary
		if (datatype_input==1)
		{
			//do nothing. as I have not created any real buffer for this datatype
		}
		else if (datatype_input==2 || datatype_input==4)
		{
			if (inimg1d) {delete []inimg1d; inimg1d = 0;}
		}

	}
	
	//now generate the separating surface
	int **left_bound=0, *left_bound1d=0;
	int **right_bound=0, *right_bound1d=0;
	
	try
	{
		left_bound1d = new int [sz[2]*sz[1]];
		new2dpointer(left_bound, sz[1], sz[2], left_bound1d);
		right_bound1d = new int [sz[2]*sz[1]];
		new2dpointer(right_bound, sz[1], sz[2], right_bound1d);
	}
	catch (...)
	{
		if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
		if (left_bound1d) {delete []left_bound1d; left_bound1d=0;}
		if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);
		if (right_bound1d) {delete []right_bound1d; right_bound1d=0;}
		
		fprintf(stderr, "Fail to allocate momery for surface generation.\n");
		return false;
	}
	

	for (z=0; z<sz[2]; z++)
	{
		vector<Coord2D> e_left;
		vector<Coord2D> e_right;

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

	V3DLONG output_sz[4];
	output_sz[0] = sz[0];
	output_sz[1] = sz[1];
	output_sz[2] = sz[2];
	output_sz[3] = sz[3]+1;

	bool b_maskres = false;
	if (datatype_input==1)
	{
		b_maskres = maskImageUsingSegBoundary(outimg1d, output_sz, in_channel_no, out_channel_no, left_bound, right_bound);
	}
	else if (datatype_input==2)
	{
		b_maskres = maskImageUsingSegBoundary((unsigned short int *)outimg1d, output_sz, in_channel_no, out_channel_no, left_bound, right_bound);
	}
	else if (datatype_input==4)
	{
		b_maskres = maskImageUsingSegBoundary((float *)outimg1d, output_sz, in_channel_no, out_channel_no, left_bound, right_bound);
	}
	if (!b_maskres)
	{
		fprintf(stderr, "Fail to mask the optic lobe regions using the boundary.\n");
	}

	//	{
//		unsigned char **** img_output_4d=0;
//		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]+1, outimg1d); //add +1 to hold the output mask data
//
//		for (z=0; z<sz[2]; z++)
//		{
//			for (j=0; j<sz[1]; j++)
//			{
//				for (i=0; i<sz[0]; i++)
//				{
//
//					if (i<left_bound[z][j])
//					{
//						img_output_4d[in_channel_no][z][j][i] = 0; //added by PHC,090609
//					}
//					else if (i>right_bound[z][j])
//					{
//						img_output_4d[in_channel_no][z][j][i] = 0;  //added by PHC,090609
//					}
//					else
//					{
//						//do nothing
//					}
//				}
//			}
//
//			if (sz[3]>=out_channel_no) //set up the mask if possible. 090730 //change from sz[3]-1 to sz[3]. to allow save the mask in the last (additional) channel
//			{
//				for (j=0; j<sz[1]; j++)
//				{
//					for (i=0; i<sz[0]; i++)
//					{
//						if (i<left_bound[z][j])
//						{
//							img_output_4d[out_channel_no][z][j][i] = 254;
//						}
//						else if (i>right_bound[z][j])
//						{
//							img_output_4d[out_channel_no][z][j][i] = 255;
//						}
//						else
//						{
//							img_output_4d[out_channel_no][z][j][i] = 0;
//						}
//					}
//				}
//			}
//
//		}
//
//		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
//	}

	// clean all workspace variables

	if (left_bound)	delete2dpointer(left_bound, sz[1], sz[2]);
	if (left_bound1d) {delete []left_bound1d; left_bound1d=0;}
	if (right_bound) delete2dpointer(right_bound, sz[1], sz[2]);
	if (right_bound1d) {delete []right_bound1d; right_bound1d=0;}

	return true;
}


bool do_lobeseg_bdbminus_onesideonly(unsigned char *inimg1d, const V3DLONG sz[4], int datatype_input, unsigned char *outimg1d, int in_channel_no, int out_channel_no, const BDB_Minus_ConfigParameter & mypara, int ini_x1, int ini_y1, int ini_x2, int ini_y2, int keep_which, int num_ctrls,bool output_surface)
{
	if (!inimg1d || !outimg1d || !sz || sz[0]<0 || sz[1]<0 || sz[2]<0 || sz[3]<0 || in_channel_no<0 || in_channel_no>=sz[3] || out_channel_no<0)
	{
		fprintf(stderr, "Invalid parameters to the function do_lobeseg_bdbminus(). \n");
		return false;
	}
	if (datatype_input!=1)
	{
		fprintf(stderr, "The datatype is NOT UINT8. The do_lobeseg_bdbminus_onesideonly() function only supports UINT8 data. Thus quit.\n");
		return false;
	}
	
	// Declare some necessary variables.
	vector<Coord2D> mCoord_0;  ///
	vector<vector<Coord2D> > v_0; ///

	//initialize control points

	int KK = num_ctrls; ///ceil(double(sz[1])/10.0);

	///float a_bottom_left[2];
	float a_first[2];

	float a_second[2];
	///float a_top_right[2];

	///a_bottom_left[0] = (sz[0] - 1)*2/5;	//x
	///a_bottom_left[1] = sz[1] - 1;			//y
	
	if(ini_x1 == ini_x2) 
	{
		ini_y1 = 0;
		ini_y2 = 100;
	}
	else if(ini_y1 == ini_y2)
	{
		ini_x1 = 0;
		ini_x2 = 100;
	}
	else
	{
		vector<Coord2D> ini_coords;
		Coord2D pt;
		float k1 = (float)(ini_y2 - ini_y1) / (float)(ini_x2 - ini_x1);
		pt.x = 0.0; pt.y = k1 * (pt.x - ini_x1) + ini_y1; 
		if(pt.y >= 0.0 && pt.y <= 100.0) ini_coords.push_back(pt);
		pt.x = 100.0; pt.y = k1 * (pt.x - ini_x1) + ini_y1; 
		if(pt.y >= 0.0 && pt.y <= 100.0) ini_coords.push_back(pt);

		float k2 = 1.0 / k1;
		pt.y = 0.0; pt.x = k2 * (pt.y - ini_y1) + ini_x1; 
		if(pt.x >= 0.0 && pt.x <= 100.0) ini_coords.push_back(pt);
		pt.y = 100.0; pt.x = k2 * (pt.y - ini_y1) + ini_x1; 
		if(pt.x >= 0.0 && pt.x <= 100.0) ini_coords.push_back(pt);
		if(ini_coords.size() != 2)
		{
			cerr<<"Invalid initial position"<<endl;
			return false;
		}
		else
		{
			if(ini_coords[0].y > ini_coords[1].y)
			{
				ini_x1 = (int)ini_coords[0].x;
				ini_y1 = (int)ini_coords[0].y;
				ini_x2 = (int)ini_coords[1].x;
				ini_y2 = (int)ini_coords[1].y;
			}
			else
			{
				ini_x1 = (int)ini_coords[1].x;
				ini_y1 = (int)ini_coords[1].y;
				ini_x2 = (int)ini_coords[0].x;
				ini_y2 = (int)ini_coords[0].y;
			}
			cout<<ini_x1<<"x"<<ini_y1<<"+"<<ini_x2<<"x"<<ini_y2<<endl;
		}
	}

	a_first[0] = (sz[0] - 1)* ini_x1/100;//(sz[0] - 1)*1/3;///(sz[0] - 1)/10;
	a_first[1] = (sz[1] - 1)* ini_y1/100;///0;

	a_second[0] = (sz[0] - 1)*ini_x2/100;///sz[0] -1;///(sz[0] - 1)*3/5;
	a_second[1] = (sz[1] - 1)*ini_y2/100;///(sz[1] -1)*1/3;///sz[1] - 1;

	///a_top_right[0] = (sz[0] - 1)*9/10;
	///a_top_right[1] = 0;

	Coord2D pt;
	int k, z, i, j;

	for (k=0; k <KK; k++)
	{
		pt.x = a_first[0] + k * (a_second[0]-a_first[0])/KK; ///
		pt.y = a_first[1] + k * (a_second[1]-a_first[1])/KK; ///
		mCoord_0.push_back(pt); ///
	}

///	pt.x = a_top_left[0];
///	pt.y = a_top_left[1];
///	mCoord_Left.push_back(pt);
///
///	pt.x = a_top_right[0];
///	pt.y = a_top_right[1];
///	mCoord_Right.push_back(pt);

	pt.x = a_second[0]; ///
	pt.y = a_second[1]; ///
	mCoord_0.push_back(pt); ///

	///vl.push_back(mCoord_Left);
	///vr.push_back(mCoord_Right);
	v_0.push_back(mCoord_0);///
	
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
			
			point_bdb_minus_2d_localwinmass_bl(tmp2d, sz[0], sz[1], v_0.at(z), v_0.at(z), mypara); ///
			v_0.push_back( v_0.at(z)); ///
		}
		cout << endl << "done computation." << endl << "Now saving file" <<endl;
		
		if (tmp2d) delete2dpointer(tmp2d, sz[0], sz[1]);
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		if (inimg_4d) delete4dpointer(inimg_4d, sz[0], sz[1], sz[2], sz[3]);
	}
	
	//now generate the separating surface
	///int **the_bound=0, *the_bound1d=0; ///
	set<int> **the_bounds=0, * the_bounds1d=0;
	
	try
	{
		///the_bound1d = new int [sz[2]*sz[1]];
		///new2dpointer(the_bound, sz[1], sz[2], the_bound1d);
		the_bounds1d = new set<int> [sz[2]*sz[1]];
		new2dpointer(the_bounds, sz[1],sz[2],the_bounds1d);
	}
	catch (...)
	{
		///if (the_bound) delete2dpointer(the_bound, sz[1], sz[2]);
		///if (the_bound1d) {delete []the_bound1d; the_bound1d=0;}
		if (the_bounds) delete2dpointer(the_bounds, sz[1], sz[2]);
		if (the_bounds1d) {delete []the_bounds1d; the_bounds1d=0;}
		printf("Fail to allocate momery.\n");
		return false;
	}
	

	for (z=0; z<sz[2]; z++)
	{
		vector<Coord2D> e_0;

		e_0 = v_0[z+1];

		pt.x = v_0[z+1][0].x;
		pt.y = sz[1] - 1;
		e_0.insert(e_0.begin(), pt);
		pt.x = v_0[z+1].rbegin()->x;
		pt.y = 0;
		e_0.push_back(pt);
		


		int kk = e_0.size();///e_left.size();
		
		for (k=0; k<kk-1; k++)
		{
			double i1 = e_0[k].x;	// "i" is "j" in the matlab code
			double j1 = e_0[k].y;
			double i2 = e_0[k+1].x;
			double j2 = e_0[k+1].y;
			
			if (j1>j2)
			{
				for (j=j1; j>=j2; j--)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					///the_bound[z][j] = tmpi;
					the_bounds[z][j].insert(tmpi);
				}				
			}
			else if(j1<j2)
			{
				for (j=j1; j<=j2; j++)
				{
					double w1 = j-j1;
					double w2 = j2-j;
					int tmpi = round((w2*i1 + w1*i2)/(w1+w2));
					///the_bound[z][j] = tmpi;
					the_bounds[z][j].insert(tmpi);
				}
			}
			else 
			{
				j = j1;
				for(V3DLONG tmpi = i1; tmpi <= i2; tmpi++) the_bounds[z][j].insert(tmpi);
			}
		}
		//printf("%d ", z);
	}
	//printf("\n");

	{
		unsigned char **** img_output_4d=0;
		new4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]+1, outimg1d);  //add +1 by Hanchuan, 110615

		for (z=0; z<sz[2]; z++)
		{
			for (j=0; j<sz[1]; j++)
			{
				set<int>::iterator it = the_bounds[z][j].begin();

				int keep = 1 - keep_which;   // keep_which : 0 for left , 1 for right
				it = the_bounds[z][j].begin();
				for(i=0; i < sz[0]; i++)
				{
					if(it != the_bounds[z][j].end() && i == *it)
					{
						keep = (keep + 1) % 2;
						int prev_i = *it;
						while(it != the_bounds[z][j].end()) 
						{
							if(output_surface) img_output_4d[sz[3]][z][j][prev_i] = 255;
							it++;
							if(*it - prev_i == 1) // skip interval 1
							{
								prev_i = *it;
								continue;
							}
							else break;
						}
					}
					else 
						if(!keep) img_output_4d[in_channel_no][z][j][i] = 0;
				}
			}
		}
		
		if (img_output_4d) {delete4dpointer(img_output_4d, sz[0], sz[1], sz[2], sz[3]); img_output_4d=0;}
	}

	// clean all workspace variables

	///if (the_bound) delete2dpointer(the_bound,sz[1],sz[2]);  ///
	///if (the_bound1d) {delete []the_bound1d; the_bound1d=0;}  ///
	if (the_bounds) delete2dpointer(the_bounds,sz[1],sz[2]);  ///
	if (the_bounds1d) {delete []the_bounds1d; the_bounds1d=0;}  ///

	return true;
}


