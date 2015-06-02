//jba_main.cpp
//by Hanchuan Peng
//2006-2011

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <queue>
#include <fstream>

#include "jba_mainfunc.h"

#include "stackutil.h"
#include "volimg_proc.h"

#include "jba_match_landmarks.h"

#define WANT_STREAM       
#include <newmatap.h>
#include <newmatio.h>
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif


int b_VERBOSE_PRINT = 0;

///
bool do_jba_core_computation(unsigned char * img_target,
						V3DLONG *sz_target,
						int datatype_target,
						string target_file_name,
						int channelNo_target,
						unsigned char * img_subject,
						V3DLONG *sz_subject,
						int datatype_subject,
						string subject_file_name,
						int channelNo_subject,
						unsigned char * &img_warped,
						V3DLONG *&sz_warped,
						int datatype_warped,
						string warped_file_name,
						string dfile_volumechangemap,
					    string file_displacementfield,
						int warpType,
						const BasicWarpParameter & bwp)
{
	bool b_noerror=true;
	if (warpType!=0 && warpType!=9 && warpType!=10)
	{
		printf("Invalid parameter to do_jba_core_computation(), which only accepts warpType to be 0 (global) or local (9 or 10))!\n");
		b_noerror=false;
		return b_noerror;
	}

	Warp3D * my_warp = 0;
	Vol3DSimple <DisplaceFieldF3D> *cur_df = 0;

	if (!img_target || !sz_target || !img_subject || !sz_subject)
	{
		fprintf(stderr, "The inputs are NULL pointers.\n");
		b_noerror = false;
		return b_noerror;
	}

	if (datatype_warped!=datatype_subject || (datatype_warped!=1 && datatype_warped!=2 && datatype_warped!=4))
	{
		fprintf(stderr, "The data types of the input subject and the output images are not consistent. Please check your program.\n");
		b_noerror = false;
		return b_noerror;
	}

	if (img_warped || sz_warped)
	{
		fprintf(stderr, "Warning: the warped image and its size are not initialized as empty. Will be re-allocated.\n");
		if (img_warped) {delete []img_warped; img_warped=0;}
		if (sz_warped) {delete []sz_warped; sz_warped=0;}
	}

	if (datatype_target==1 && datatype_subject==1)
	{
		printf("********************* target=uint8 subject=uint8\n");
		my_warp = new Warp3D((const unsigned char *)img_target, sz_target, channelNo_target, (const unsigned char *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==1 && datatype_subject==2)
	{
		printf("********************* target=uint8 subject=uint16\n");
		my_warp = new Warp3D((const unsigned char *)img_target, sz_target, channelNo_target, (const unsigned short int *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==1 && datatype_subject==4)
	{
		my_warp = new Warp3D((const unsigned char *)img_target, sz_target, channelNo_target, (const float *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==2 && datatype_subject==1)
	{
		my_warp = new Warp3D((const unsigned short int *)img_target, sz_target, channelNo_target, (const unsigned char *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==2 && datatype_subject==2)
	{
		my_warp = new Warp3D((const unsigned short int *)img_target, sz_target, channelNo_target, (const unsigned short int *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==2 && datatype_subject==4)
	{
		my_warp = new Warp3D((const unsigned short int *)img_target, sz_target, channelNo_target, (const float *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==4 && datatype_subject==1)
	{
		my_warp = new Warp3D((const float *)img_target, sz_target, channelNo_target, (const unsigned char *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==4 && datatype_subject==2)
	{
		my_warp = new Warp3D((const float *)img_target, sz_target, channelNo_target, (const unsigned short int *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else if (datatype_target==4 && datatype_subject==4)
	{
		my_warp = new Warp3D((const float *)img_target, sz_target, channelNo_target, (const float *)img_subject, sz_subject, channelNo_subject, 
								 target_file_name, subject_file_name, warped_file_name);
	}
	else
	{
		fprintf(stderr, "Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
		b_noerror = true;
		goto Label_exit;
	}	

	switch (warpType)
	{
		case 0:
			cur_df = my_warp->do_global_transform(bwp); 
            break;

		case 9: 
            if (bwp.file_landmark_subject.compare("unsetyet")==0)
			{
				cur_df = my_warp->do_global_transform(bwp);

				{
					Vol3DSimple <DisplaceFieldF3D> *df_local2 = 0;

					my_warp->warpSubjectImage(cur_df, bwp.b_nearest_interp);

					df_local2 = my_warp->do_landmark_warping(bwp);
					df_add(cur_df, df_local2, bwp.b_nearest_interp);
					if (df_local2) {delete df_local2; df_local2=0;}
				}
			}
			else
			{
				cur_df = my_warp->do_landmark_in_file_warping(bwp);
			}
			break;

		case 10: 
            if (bwp.file_landmark_subject.compare("unsetyet")==0)
			{
				cur_df = my_warp->do_landmark_warping(bwp);
			}
			else
			{
				cur_df = my_warp->do_landmark_in_file_warping(bwp);
			}
			break;

		default:
			fprintf(stderr, "Undefined warp type found! This sentence should never be printed. Check the warpType parameters and if the source code has error.\n");
			b_noerror = false;
			goto Label_exit;
	}

	switch (datatype_warped)
	{
		case 1:
			my_warp->doWarpUsingDF((unsigned char * &)img_warped, sz_warped, (unsigned char *)img_subject, sz_subject, datatype_warped, cur_df, bwp.b_nearest_interp);
			break;
		case 2:
			my_warp->doWarpUsingDF((unsigned short int * &)img_warped, sz_warped, (unsigned short int *)img_subject, sz_subject, datatype_warped, cur_df, bwp.b_nearest_interp);
			break;
		case 4:
			my_warp->doWarpUsingDF((float * &)img_warped, sz_warped, (float *)img_subject, sz_subject, datatype_warped, cur_df, bwp.b_nearest_interp);
			break;
		default:
			fprintf(stderr, "Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
			b_noerror = true;
			goto Label_exit;
	}

	if (!file_displacementfield.empty()) 
	{
		saveVol3DSimple2RawFloat(cur_df, file_displacementfield, 999, string("DF"));
	}
	
Label_exit:
	if (b_noerror==false)
	{
		if (sz_warped) {delete []sz_warped; sz_warped=0;}
		if (img_warped) {delete []img_warped; img_warped=0;}
	}

	if (my_warp) {delete my_warp; my_warp=0;}
	if (cur_df) {delete cur_df; cur_df=0;}
	return b_noerror;
}

//
void aggregateAffineWarp(WarpParameterAffine3D * res, WarpParameterAffine3D * a, WarpParameterAffine3D * b)
{
    if (!res || !a || !b)
		return;

	Matrix A(4,4);
	A.row(1) << a->mxx << a->mxy << a->mxz << a->sx;
	A.row(2) << a->myx << a->myy << a->myz << a->sy;
	A.row(3) << a->mzx << a->mzy << a->mzz << a->sz;
	A.row(4) <<      0 <<      0 <<      0 <<     1;

	Matrix B(4,4);
	B.row(1) << b->mxx << b->mxy << b->mxz << b->sx;
	B.row(2) << b->myx << b->myy << b->myz << b->sy;
	B.row(3) << b->mzx << b->mzy << b->mzz << b->sz;
	B.row(4) <<      0 <<      0 <<      0 <<     1;

	A = A*B;

	res->mxx = A(1,1); res->mxy = A(1,2); res->mxz = A(1,3); res->sx = A(1,4);
	res->myx = A(2,1); res->myy = A(2,2); res->myz = A(2,3); res->sy = A(2,4);
	res->mzx = A(3,1); res->mzy = A(3,2); res->mzz = A(3,3); res->sz = A(3,4);

    res->si = a->si * b->si;
    res->sb = a->sb + b->sb;

    return;
}


bool eba3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable,
                        Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft,
                        Vol3DSimple<MYFLOAT_JBA> * subject,
                        V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
                        bool b_returnDTable)
{
	if (!wp ||
	    !dfx || !dfx->valid() ||
		!dfy || !dfy->valid() ||
		!dfz || !dfz->valid() ||
		!dft || !dft->valid() ||
		!subject || !subject->valid() )
	{
	    fprintf(stderr, "Invalid internal data pointers [%s][%d].\n", __FILE__, __LINE__);
	    return false;
	}
    
	V3DLONG tsz0 = subject->sz0(), tsz1 = subject->sz1(), tsz2 = subject->sz2();
    
	double xc = (double(xe) + double(xs))/2.0;
	double yc = (double(ye) + double(ys))/2.0;
	double zc = (double(ze) + double(zs))/2.0;
    if (xs>xe ||
		ys>ye ||
		zs>ze ||
		xc < 0 || xc > (tsz0-1) ||
		yc < 0 || yc > (tsz1-1) ||
		zc < 0 || zc > (tsz2-1) )
	{
	    fprintf(stderr, "Invalid cube coordinate parameters [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	if (dTable)
	{
	    fprintf(stderr, "The dTable is not initilized as empty! Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
    
	MYFLOAT_JBA ***	dfx3d = dfx->getData3dHandle();
	MYFLOAT_JBA ***	dfy3d = dfy->getData3dHandle();
	MYFLOAT_JBA ***	dfz3d = dfz->getData3dHandle();
	MYFLOAT_JBA ***	dft3d = dft->getData3dHandle();
	MYFLOAT_JBA ***	subject3d = subject->getData3dHandle();
    
	V3DLONG i,j,k;
    
	V3DLONG is = hardLimit(xs, V3DLONG(0), tsz0-1), ie = hardLimit(xe, V3DLONG(0), tsz0-1);
	V3DLONG js = hardLimit(ys, V3DLONG(0), tsz1-1), je = hardLimit(ye, V3DLONG(0), tsz1-1);
	V3DLONG ks = hardLimit(zs, V3DLONG(0), tsz2-1), ke = hardLimit(ze, V3DLONG(0), tsz2-1);
    
	V3DLONG xlen = ie-is+1, ylen = je-js+1, zlen = ke-ks+1;
	V3DLONG cubelen = (V3DLONG)xlen*ylen*zlen;
    
	const V3DLONG myCUBECOLUMNNUM=15;
    
    Image2DSimple<MYFLOAT_JBA> *curDTable = new Image2DSimple<MYFLOAT_JBA> (cubelen, myCUBECOLUMNNUM);
    if (!curDTable || !curDTable->valid())
    {
        fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
        return false;
    }
    MYFLOAT_JBA **  cubeData2d = curDTable->getData2dHandle();
    
    V3DLONG curind=0;
    for (k=ks;k<=ke;k++)
    {
        double mk = (k-zc);
        for (j=js;j<=je;j++)
        {
#ifndef POSITIVE_Y_COORDINATE
            double mj = -(j-yc);
#else
            double mj = (j-yc);
#endif
            for (i=is;i<=ie;i++)
            {
                double mi = (i-xc);
                
                cubeData2d[0][curind] = dfx3d[k][j][i] * mi;
                cubeData2d[1][curind] = dfx3d[k][j][i] * mj;
                cubeData2d[2][curind] = dfx3d[k][j][i] * mk;
                
                cubeData2d[3][curind] = dfy3d[k][j][i] * mi;
                cubeData2d[4][curind] = dfy3d[k][j][i] * mj;
                cubeData2d[5][curind] = dfy3d[k][j][i] * mk;
                
                cubeData2d[6][curind] = dfz3d[k][j][i] * mi;
                cubeData2d[7][curind] = dfz3d[k][j][i] * mj;
                cubeData2d[8][curind] = dfz3d[k][j][i] * mk;
                
                cubeData2d[9][curind] = dfx3d[k][j][i];
                cubeData2d[10][curind] = dfy3d[k][j][i];
                cubeData2d[11][curind] = dfz3d[k][j][i];
                
                cubeData2d[12][curind] = -subject3d[k][j][i];
                cubeData2d[13][curind] = -1;
                
                cubeData2d[14][curind] = dft3d[k][j][i] + cubeData2d[12][curind] + cubeData2d[0][curind] + cubeData2d[4][curind] + cubeData2d[8][curind];
                
                curind++;
            }
        }
    }
    
    Matrix P_Matrix(14,14);
    Matrix K_Matrix(14,1);
    double tmpp=0.0, tmpk=0.0;
    
    for (k=0;k<14;k++)
    {
        for (j=k;j<14;j++)
        {
            for (i=0, tmpp=0.0;i<cubelen;i++)
                tmpp += cubeData2d[k][i] * cubeData2d[j][i];
            P_Matrix(k+1,j+1) = tmpp;
            P_Matrix(j+1,k+1) = tmpp;
        }
        
        for (i=0, tmpk=0.0;i<cubelen;i++)
        {
            tmpk += cubeData2d[k][i] * (cubeData2d[14][i]);
        }
        K_Matrix(k+1,1) = tmpk;
    }
    
    Matrix Y;
    Try
    {
        Y = P_Matrix.i() * K_Matrix;
        
        wp->mxx = Y(1,1); wp->mxy = Y(2,1); wp->mxz = Y(3,1);
        wp->myx = Y(4,1); wp->myy = Y(5,1); wp->myz = Y(6,1);
        wp->mzx = Y(7,1); wp->mzy = Y(8,1); wp->mzz = Y(9,1);
        wp->sx = Y(10,1); wp->sy = Y(11,1); wp->sz = Y(12,1);
        wp->si = Y(13,1); wp->sb = Y(14,1);
        
        wp->b_transform=1;
    }
    CatchAll
    {
        wp->resetToDefault();
    }
    
//    if (b_VERBOSE_PRINT)
//    {
//        Matrix Y1(4,4);
//        Y1.row(1) << wp->mxx << wp->mxy << wp->mxz << wp->sx;
//        Y1.row(2) << wp->myx << wp->myy << wp->myz << wp->sy;
//        Y1.row(3) << wp->mzx << wp->mzy << wp->mzz << wp->sz;
//        Y1.row(4) << 0 << 0 << 0 << 1;
//        cout << "Transform matrix" << endl;
//        cout << setw(12) << setprecision(3) << Y1 << endl;
//    }
    
    if (b_returnDTable==true)
    {
        dTable = curDTable;
    }
    else
    {
        if (curDTable) delete curDTable; curDTable=0;
    }
	return true;
}


bool getSmoothedWarpParameter(Vol3DSimple<WarpParameterAffine3D> * wp_new, Vol3DSimple<WarpParameterAffine3D> * wp_old, int halfwin)
{
    if (!wp_new || !wp_new->valid()|| !wp_old || !wp_old->valid())
	{
		fprintf(stderr, "Invalid parameter to getSmoothedWarpParameter().\n");
		return false;
	}

	if (isSameSize(wp_new, wp_old)==false)
	{
		fprintf(stderr, "The new and old warp parameter volumes have different size! [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (halfwin<0)
	{
		fprintf(stderr, "Invalid parameter to getSmoothedWarpParameter().\n");
		return false;
	}

	V3DLONG i,j,k;
	V3DLONG ib,ie,jb,je,kb,ke;

	WarpParameterAffine3D tmp_wp;

	WarpParameterAffine3D *** wp3d_old = wp_old->getData3dHandle();
	WarpParameterAffine3D *** wp3d_new = wp_new->getData3dHandle();
	V3DLONG sz0 = wp_old->sz0();
	V3DLONG sz1 = wp_old->sz1();
	V3DLONG sz2 = wp_old->sz2();

	int cnt;
	int tmpi, tmpj;
	for (k=0;k<sz2;k++)
	{
		kb = k-halfwin; kb = (kb<0) ? 0 : kb;
		ke = k+halfwin; ke = (ke>=sz2-1) ? sz2-1 : ke;

		for (j=0;j<sz1;j++)
		{
			jb = j-halfwin; jb = (jb<0) ? 0 : jb;
			je = j+halfwin; je = (je>=sz1-1) ? sz1-1 : je;

			for (i=0;i<sz0;i++)
			{
				ib = i-halfwin; ib = (ib<0) ? 0 : ib;
				ie = i+halfwin; ie = (ie>=sz0-1) ? sz0-1 : ie;

				V3DLONG i1,j1,k1;
				tmp_wp.resetToAllZeros();
				cnt=0;
				for (k1=kb;k1<=ke;k1++)
				{
					for (j1=jb;j1<=je;j1++)
					{
						for (i1=ib;i1<=ie;i1++)
						{
						    if (k1!=k || j1!=j || i1!=i)
							{
							   if (!(wp3d_old[k1][j1][i1].b_transform)) continue; 
                                
							   tmpi = (k1==k || j1==j || i1==i) ? 4 : 1;
							   for (tmpj=0;tmpj<tmpi;tmpj++)
							   {
							     tmp_wp.add(wp3d_old[k1][j1][i1]);
								 cnt++;
							   }
							}
						}
					}
				}

				if (cnt>0) 
				{
					tmp_wp.time(1.0/cnt);
					wp3d_new[k][j][i].copy(tmp_wp);
					wp3d_new[k][j][i].b_transform=1; 
				}

			}
		}
	}

    return true;
}

//

template <class T1, class T2> Warp3D::Warp3D(
								  const T1 * img0_target1d,
								  const V3DLONG *img0_sz_target,
								  const int channelNo0_ref_target,
								  const T2 * img0_subject1d,
								  const V3DLONG *img0_sz_subject,
								  const int channelNo0_ref_subject,
								  string file0_target,
								  string file0_subject,
								  string file0_warped)
{
	initData();
	setInitData(img0_target1d, img0_sz_target, channelNo0_ref_target, img0_subject1d, img0_sz_subject, channelNo0_ref_subject, 
	file0_target, file0_subject, file0_warped);
}

Warp3D::~Warp3D()
{
	deleteData();
}

void Warp3D::initData()
{
	file_target = ""; file_subject = ""; file_warped = "";

	my_len3d = 0;
	for (V3DLONG i=0; i<3;i++) my_sz[i] = 0;

	img_target = NULL;
	img_subject = NULL;
	img_warped = NULL;

	warpPara_local = NULL;
	len_warpPara_local = 0;
}

void Warp3D::deleteData()
{
	len_warpPara_local = 0;
	if (warpPara_local) {delete warpPara_local; warpPara_local=NULL;}

	if (img_warped) {delete img_warped; img_warped=NULL;}
	if (img_subject) {delete img_subject; img_subject=NULL;}
	if (img_target) {delete img_target; img_target=NULL;}

	my_len3d = 0;
	for (V3DLONG i=0; i<3;i++) my_sz[i] = 0;

	file_target = ""; file_subject = ""; file_warped = "";
}

template <class T> float getNormalizer(T *a)
{
	float TNormalizer = 1.0;	if (sizeof(T)==1) TNormalizer=255.0; else if (sizeof(T)==2) TNormalizer=4095.0; 
	return TNormalizer;
}

template <class T1, class T2> bool Warp3D::setInitData(
											const T1 * img0_target1d,
											const V3DLONG *img0_sz_target,
											const int channelNo0_ref_target,
											const T2 * img0_subject1d,
											const V3DLONG *img0_sz_subject,
											const int channelNo0_ref_subject,
											string file0_target,
											string file0_subject,
											string file0_warped)
{
	if (!img0_target1d || !img0_sz_target || channelNo0_ref_target<0 || !img0_subject1d || !img0_sz_subject || channelNo0_ref_subject<0)
	{
		return false;
	}

	deleteData(); //always reset the data first

	file_target = file0_target;
	file_subject = file0_subject;
	file_warped = file0_warped;

        //const V3DLONG MAXPIXELNUMBER = 8*1024*1024*1024;
	V3DLONG i,j,k;
	for (i=0, my_len3d=1;i<3;i++)
	{
		my_sz[i] = (img0_sz_target[i] > img0_sz_subject[i]) ? img0_sz_target[i] : img0_sz_subject[i];
		my_sz[i] = V3DLONG(ceil(double(my_sz[i])/32))*32; 
        
                if (my_sz[i]<=0) // || my_sz[i]>1024)
		{
			fprintf(stderr, "The %ldth size of input images is %ld, which is not correct (either too small, i.e. <0, or too big, i.e. > 1024).\n", i, my_sz[i]);
			return false;
		}
		my_len3d *= my_sz[i];

                //if (my_len3d>MAXPIXELNUMBER)
                //{
                        //fprintf(stderr, "The # of pixels [at least %ld] is already bigger than %ld, which is the limit of what can be handled by this program.\n", my_len3d, MAXPIXELNUMBER);
                        //return false;
                //}

		spos_target[i] = floor((my_sz[i] - img0_sz_target[i])/2);
		spos_subject[i] = floor((my_sz[i] - img0_sz_subject[i])/2);
	}

	img_target = new Vol3DSimple<MYFLOAT_JBA> (my_sz[0], my_sz[1], my_sz[2]);
	img_subject = new Vol3DSimple<MYFLOAT_JBA> (my_sz[0], my_sz[1], my_sz[2]);
	img_warped = new Vol3DSimple<MYFLOAT_JBA> (my_sz[0], my_sz[1], my_sz[2]);

	if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid() || !img_warped || !img_warped->valid() )
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		return false; 
	}

	MYFLOAT_JBA * img_target_ref1d = img_target->getData1dHandle();
	MYFLOAT_JBA * img_subject_ref1d = img_subject->getData1dHandle();
	MYFLOAT_JBA * img_warped_ref1d = img_warped->getData1dHandle();

	for (i=0;i<my_len3d;i++)
		img_warped_ref1d[i] = img_target_ref1d[i] = img_subject_ref1d[i] = 0;

	MYFLOAT_JBA *** img_target_ref3d = img_target->getData3dHandle();
	MYFLOAT_JBA *** img_subject_ref3d = img_subject->getData3dHandle();

	T1 ****img0_target4d = 0;
	float T1Normalizer = getNormalizer(img0_target1d);
	fprintf(stdout, "in setInitData(), the target image data normalizer=%5.3f\n", T1Normalizer);
	new4dpointer(img0_target4d, (V3DLONG)img0_sz_target[0], (V3DLONG)img0_sz_target[1], (V3DLONG)img0_sz_target[2], (V3DLONG)img0_sz_target[3], img0_target1d);

	//saveImage("test1target.raw", (unsigned char *)img0_target1d, img0_sz_target, sizeof(T1));

	if (img0_target4d)
	{
		for (k=0; k<img0_sz_target[2]; k++)
		{
			for (j=0; j<img0_sz_target[1]; j++)
			{
				for (i=0; i<img0_sz_target[0]; i++)
				{
					img_target_ref3d[spos_target[2]+k][spos_target[1]+j][spos_target[0]+i] = (MYFLOAT_JBA)img0_target4d[channelNo0_ref_target][k][j][i]/T1Normalizer;
				}
			}
		}
		delete4dpointer(img0_target4d, img0_sz_target[0], img0_sz_target[1], img0_sz_target[2], img0_sz_target[3]);
	}
	else
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		return false;
	}

	T2 ****img0_subject4d = 0;
	float T2Normalizer = getNormalizer(img0_subject1d);
	fprintf(stdout, "in setInitData(), the subject image data normalizer=%5.3f\n", T2Normalizer);
	new4dpointer(img0_subject4d, (V3DLONG)img0_sz_subject[0], (V3DLONG)img0_sz_subject[1], (V3DLONG)img0_sz_subject[2], (V3DLONG)img0_sz_subject[3], img0_subject1d);
	//saveImage("test1subject.raw", (unsigned char *)img0_subject1d, img0_sz_subject, sizeof(T2));
	
	if (img0_subject4d)
	{
		for (k=0; k<img0_sz_subject[2]; k++)
		{
			for (j=0; j<img0_sz_subject[1]; j++)
			{
				for (i=0; i<img0_sz_subject[0]; i++)
				{
					float a = (MYFLOAT_JBA)img0_subject4d[channelNo0_ref_subject][k][j][i]/T2Normalizer;
					img_subject_ref3d[spos_subject[2]+k][spos_subject[1]+j][spos_subject[0]+i] = a;
				}
			}
		}
		delete4dpointer(img0_subject4d, img0_sz_subject[0], img0_sz_subject[1], img0_sz_subject[2], img0_sz_subject[3]);
	}
	else
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		return false;
	}

	printf("Done: data initilization. \n");
	return true;
}

template <class T> bool Warp3D::applyDFtoChannel(
	const T * img0_subject1d,
	const V3DLONG *img0_sz_subject,
	const int channelNo0_ref, 
	V3DLONG startpos_subject [3],
	Vol3DSimple <DisplaceFieldF3D> *cur_df,
	 bool b_nearest_interp
)
{
	if (!img0_subject1d || !img0_sz_subject || channelNo0_ref>img0_sz_subject[3] || !cur_df || !cur_df->valid())
	{
		fprintf(stderr, "Invalid parameters to applyDFtoChannel(). \n");
		return false;
	}

	V3DLONG i,j,k,c;

	V3DLONG msz0=cur_df->sz0(), msz1=cur_df->sz1(), msz2=cur_df->sz2();
	if (spos_subject[0]+img0_sz_subject[0]>msz0 || spos_subject[1]+img0_sz_subject[1]>msz1 || spos_subject[2]+img0_sz_subject[2]>msz2)
	{
		fprintf(stderr, "The size of the subject image is wrong in applyDFtoChannel(). Check you parameters. \n");
		return false;
	}

	Vol3DSimple <MYFLOAT_JBA> * cur_img_warped = new Vol3DSimple<MYFLOAT_JBA> (msz0, msz1, msz2);
	if (!cur_img_warped || !cur_img_warped->valid() )
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		return false; 
	}
	MYFLOAT_JBA * img_warped_ref1d = cur_img_warped->getData1dHandle();
	MYFLOAT_JBA *** img_warped_ref3d = cur_img_warped->getData3dHandle();

	T ****img0_subject4d = 0;
	float TNormalizer = getNormalizer(img0_subject1d); 
	fprintf(stdout, "in ApplyDFToChannel(), the subject image data normalizer=%5.3f\n", TNormalizer);
	new4dpointer(img0_subject4d, (V3DLONG)img0_sz_subject[0], (V3DLONG)img0_sz_subject[1], (V3DLONG)img0_sz_subject[2], (V3DLONG)img0_sz_subject[3], img0_subject1d);
	if (img0_subject4d)
	{
		for (c=0;c<img0_sz_subject[3]; c++)
		{
			if (c!=channelNo0_ref && channelNo0_ref>=0)
				continue;

			for (i=0;i<cur_img_warped->getTotalElementNumber();i++) img_warped_ref1d[i] = 0;

			for (k=0; k<img0_sz_subject[2]; k++)
			{
				for (j=0; j<img0_sz_subject[1]; j++)
				{
					for (i=0; i<img0_sz_subject[0]; i++)
					{
						img_warped_ref3d[startpos_subject[2]+k][startpos_subject[1]+j][startpos_subject[0]+i] = (MYFLOAT_JBA)img0_subject4d[c][k][j][i]/TNormalizer;
					}
				}
			}

			df_warp(cur_img_warped, cur_df, b_nearest_interp);

			for (k=0; k<img0_sz_subject[2]; k++)
			{
				for (j=0; j<img0_sz_subject[1]; j++)
				{
					for (i=0; i<img0_sz_subject[0]; i++)
					{
						img0_subject4d[c][k][j][i] = (T)(img_warped_ref3d[startpos_subject[2]+k][startpos_subject[1]+j][startpos_subject[0]+i] * TNormalizer);
					}
				}
			}

			printf("Finish warping the %ldth channel.\n", c);
		}

		delete4dpointer(img0_subject4d, img0_sz_subject[0], img0_sz_subject[1], img0_sz_subject[2], img0_sz_subject[3]);
		if (cur_img_warped) {delete cur_img_warped; cur_img_warped=0;}
	}
	else
	{
		fprintf(stderr, "Fail to allocate memory: file=[%s] line=[%d].\n", __FILE__, __LINE__);
		if (cur_img_warped) {delete cur_img_warped; cur_img_warped=0;}
		return false;
	}

	printf("Done: warping. \n");
	return true;
}



bool newGradientData(Vol3DSimple<MYFLOAT_JBA> * & dfx,
                             Vol3DSimple<MYFLOAT_JBA> * & dfy,
							 Vol3DSimple<MYFLOAT_JBA> * & dfz,
							 Vol3DSimple<MYFLOAT_JBA> * & dft,
							 V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2)
{
    if (dfx || dfy || dfz || dft)
	{
		fprintf(stderr, "The input pointers are not empty. [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}

	dfx = new Vol3DSimple<MYFLOAT_JBA>(tsz0, tsz1, tsz2);
	dfy = new Vol3DSimple<MYFLOAT_JBA>(tsz0, tsz1, tsz2);
	dfz = new Vol3DSimple<MYFLOAT_JBA>(tsz0, tsz1, tsz2);
	dft = new Vol3DSimple<MYFLOAT_JBA>(tsz0, tsz1, tsz2);

	if (!dfx || !dfy || !dfz || !dft)
	{
		fprintf(stderr, "Fail to allocate meory. [%s][%d]\n", __FILE__, __LINE__);
		if (dfx) {delete dfx; dfx=0;}
		if (dfy) {delete dfy; dfy=0;}
		if (dfz) {delete dfz; dfz=0;}
		if (dft) {delete dft; dft=0;}
		return false;
	}

    return true;
}

void deleteGradientData(Vol3DSimple<MYFLOAT_JBA> * & dfx,
                                Vol3DSimple<MYFLOAT_JBA> * & dfy,
							    Vol3DSimple<MYFLOAT_JBA> * & dfz,
							    Vol3DSimple<MYFLOAT_JBA> * & dft)
{
	if (dfx) {delete dfx; dfx=0;}
	if (dfy) {delete dfy; dfy=0;}
	if (dfz) {delete dfz; dfz=0;}
	if (dft) {delete dft; dft=0;}
	return;
}

bool Warp3D::downsample3dvol(Vol3DSimple<MYFLOAT_JBA> * &outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor)
{
	if (outimg || !inimg || !inimg->valid())
	{
		fprintf(stderr, "The outimg pointer is not initialized as NULL or get NULL input image pointer [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (int(dfactor)<1 || int(dfactor) >32)
	{
		fprintf(stderr, "The downsampling factor must be >=1 and <= 32 [%s][%d] dfactor=%.2f.\n", __FILE__, __LINE__, dfactor);
		return false;
	}

	V3DLONG cur_sz0 = (V3DLONG)(floor(double(inimg->sz0()) / double(dfactor)));
	V3DLONG cur_sz1 = (V3DLONG)(floor(double(inimg->sz1()) / double(dfactor)));
	V3DLONG cur_sz2 = (V3DLONG)(floor(double(inimg->sz2()) / double(dfactor)));

	if (cur_sz0 <= 0 || cur_sz1 <=0 || cur_sz2<=0)
	{
		fprintf(stderr, "The dfactor is not properly set, -- the downsampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	outimg = new Vol3DSimple<MYFLOAT_JBA> (cur_sz0, cur_sz1, cur_sz2);
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	MYFLOAT_JBA *** out_tmp3d = outimg->getData3dHandle();
	MYFLOAT_JBA *** in_tmp3d = inimg->getData3dHandle();

	for (V3DLONG k=0;k<outimg->sz2();k++)
	{
		V3DLONG k2low=(V3DLONG)(floor(k*dfactor)), k2high=(V3DLONG)(floor((k+1)*dfactor-1));
		if (k2high>inimg->sz2()) k2high = inimg->sz2();
		V3DLONG kw = k2high - k2low + 1;

		for (V3DLONG j=0;j<outimg->sz1();j++)
		{
			V3DLONG j2low=(V3DLONG)(floor(j*dfactor)), j2high=(V3DLONG)(floor((j+1)*dfactor-1));
			if (j2high>inimg->sz1()) j2high = inimg->sz1();
			V3DLONG jw = j2high - j2low + 1;

			for (V3DLONG i=0;i<outimg->sz0();i++)
			{
				V3DLONG i2low=(V3DLONG)(floor(i*dfactor)), i2high=(V3DLONG)(floor((i+1)*dfactor-1));
				if (i2high>inimg->sz0()) i2high = inimg->sz0();
				V3DLONG iw = i2high - i2low + 1;

				double cubevolume = double(kw) * jw * iw;
				//cout<<cubevolume <<" ";

				double s=0.0;
				for (V3DLONG k1=k2low;k1<=k2high;k1++)
				{
					for (V3DLONG j1=j2low;j1<=j2high;j1++)
					{
						for (V3DLONG i1=i2low;i1<=i2high;i1++)
						{
							s += in_tmp3d[k1][j1][i1];
						}
					}
				}

				out_tmp3d[k][j][i] = s/cubevolume;
			}
		}
	}

	return true;
}

bool Warp3D::upsample3dvol(Vol3DSimple<MYFLOAT_JBA> * & outimg, Vol3DSimple<MYFLOAT_JBA> * inimg, double dfactor)
{
    if (outimg || !inimg || !inimg->valid() || dfactor<1)
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}
	MYFLOAT_JBA *** timg3din = inimg->getData3dHandle();
	V3DLONG tsz0 = inimg->sz0();
	V3DLONG tsz1 = inimg->sz1();
	V3DLONG tsz2 = inimg->sz2();

	V3DLONG tsz0o = (V3DLONG)(ceil(dfactor*tsz0)), tsz1o = (V3DLONG)(ceil(dfactor*tsz1)), tsz2o = V3DLONG(ceil(dfactor*tsz2));

    outimg = new Vol3DSimple<MYFLOAT_JBA> (tsz0o, tsz1o, tsz2o);
	if (!outimg || !outimg->valid())
	{
		fprintf(stderr, "Fail to allocate memory in upsample3dvol(). [%d]\n", __LINE__);
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}

	MYFLOAT_JBA *** timg3dout = outimg->getData3dHandle();

	V3DLONG totallen = tsz0o * tsz1o * tsz2o; //8 times because every dim is 2 times bigger
	Coord3D_JBA * c = new Coord3D_JBA [totallen];
	MYFLOAT_JBA * v = new MYFLOAT_JBA [totallen];

	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}

	V3DLONG i,j,k, ind;
    ind=0;
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				c[ind].x = double(i)/dfactor;
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = double(tsz1o-1-j)/dfactor;
#else
				c[ind].y = double(j)/dfactor; //080114
#endif
				c[ind].z = double(k)/dfactor;
				v[ind] = -1; 
				ind++;
			}
		}
	}

	lcl(v, c, totallen, timg3din, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);

    ind=0;
	for (k=0;k<tsz2o;k++)
	{
		for (j=0;j<tsz1o;j++)
		{
			for (i=0;i<tsz0o;i++)
			{
				timg3dout[k][j][i] = v[ind++];
			}
		}
	}

    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}

bool Warp3D::upsampleDisplaceField(Vol3DSimple<DisplaceFieldF3D> * & out_df, Vol3DSimple<DisplaceFieldF3D> * in_df, double dfactor)
{
	if (out_df || !in_df || !in_df->valid() || dfactor < 1)
	{
		fprintf(stderr, "Invalid parameter to upsampleDisplaceField() [%d][%8.7f]!\n", __LINE__, dfactor);
		return false;
	}

	bool b_noerror=true;
	V3DLONG tsz0 = in_df->sz0(), tsz1 = in_df->sz1(), tsz2 = in_df->sz2();
	V3DLONG csz0 = (V3DLONG)(ceil(dfactor*tsz0)), csz1 = (V3DLONG)(ceil(dfactor*tsz1)), csz2 = (V3DLONG)(ceil(dfactor*tsz2));
	V3DLONG i,j,k;

	Vol3DSimple<MYFLOAT_JBA> *tmp_outimg=0, *tmp_inimg=0;
	MYFLOAT_JBA *** tmp_outimg_ref, ***tmp_inimg_ref;
	DisplaceFieldF3D *** out_df_ref, *** in_df_ref;

    out_df = new Vol3DSimple<DisplaceFieldF3D> (csz0, csz1, csz2);
	if (!out_df || !out_df->valid())
	{
		fprintf(stderr, "Fail to allocate memory in upsampleDisplaceField(). [%d]\n", __LINE__);
		b_noerror = false;
		goto Label_exit;
	}

	tmp_inimg = new Vol3DSimple<MYFLOAT_JBA> (tsz0, tsz1, tsz2);
	if (!tmp_inimg || !tmp_inimg->valid())
	{
		fprintf(stderr, "Fail to allocate memory. [%d]\n", __LINE__);
		b_noerror=false;
		goto Label_exit;
	}

	//compute

    out_df_ref = out_df->getData3dHandle();
	in_df_ref = in_df->getData3dHandle();
	tmp_inimg_ref = tmp_inimg->getData3dHandle();

	//for sx
	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_inimg_ref[k][j][i]= in_df_ref[k][j][i].sx * dfactor;}}} //* dfactor, 070315
	if (upsample3dvol( tmp_outimg, tmp_inimg, dfactor)==true)
	{
		tmp_outimg_ref = tmp_outimg->getData3dHandle();
		for (k=0;k<csz2;k++) {for (j=0;j<csz1;j++) { for (i=0;i<csz0;i++) { out_df_ref[k][j][i].sx = tmp_outimg_ref[k][j][i];}}}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	}
	else {b_noerror=false; goto Label_exit;}

	//for sy
	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_inimg_ref[k][j][i]= in_df_ref[k][j][i].sy * dfactor;}}} //* dfactor, 070315
	if (upsample3dvol( tmp_outimg, tmp_inimg, dfactor)==true)
	{
		tmp_outimg_ref = tmp_outimg->getData3dHandle();
		for (k=0;k<csz2;k++) {for (j=0;j<csz1;j++) { for (i=0;i<csz0;i++) { out_df_ref[k][j][i].sy = tmp_outimg_ref[k][j][i];}}}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	}
	else {b_noerror=false; goto Label_exit;}

	//for sz
	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_inimg_ref[k][j][i]= in_df_ref[k][j][i].sz * dfactor;}}} //* dfactor, 070315
	if (upsample3dvol( tmp_outimg, tmp_inimg, dfactor)==true)
	{
		tmp_outimg_ref = tmp_outimg->getData3dHandle();
		for (k=0;k<csz2;k++) {for (j=0;j<csz1;j++) { for (i=0;i<csz0;i++) { out_df_ref[k][j][i].sz = tmp_outimg_ref[k][j][i];}}}
		if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	}
	else {b_noerror=false; goto Label_exit;}

Label_exit:
	if (b_noerror==false)
	{
		if (out_df) {delete out_df; out_df=0;}
	}

	if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	if (tmp_inimg) {delete tmp_inimg; tmp_inimg=0;}

	return b_noerror;
}

void WarpParameterAffine3D::print(string s)
{
	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("%s\n", s.c_str());
	printf("%7.3f\t%7.3f\t%7.3f\t%7.3f\t\n", mxx, mxy, mxz, sx);
	printf("%7.3f\t%7.3f\t%7.3f\t%7.3f\t\n", myx, myy, myz, sy);
	printf("%7.3f\t%7.3f\t%7.3f\t%7.3f\t\n", mzx, mzy, mzz, sz);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

///
Vol3DSimple<DisplaceFieldF3D> *  Warp3D::do_global_transform(const BasicWarpParameter & bwp) 
{
    bool b_noerror=true;

    if (!img_warped || !img_warped->valid() ||
	    !img_target || !img_target->valid() ||
	    !img_subject || !img_subject->valid())
	{
		return 0;
	}

	V3DLONG rx,ry,rz;
	rx = V3DLONG(ceil((double)img_target->sz0() * (sqrt(2.0)-1.0) / 2.0));
	ry = V3DLONG(ceil((double)img_target->sz1() * (sqrt(2.0)-1.0) / 2.0));
	rz = V3DLONG(ceil((double)img_target->sz2() * (sqrt(2.0)-1.0) / 2.0));
	cout << "before padding: old size " << img_subject->sz0() <<","<< img_subject->sz1() << ","<< img_subject->sz2() << endl;
	img_warped->padding(rx, ry, rz);
	img_target->padding(rx, ry, rz);
	img_subject->padding(rx, ry, rz);
	cout << "padding: new size " << img_subject->sz0() << ","<< img_subject->sz1() << ","<< img_subject->sz2() << endl;

	vol3d_assign(img_warped->getData3dHandle(), img_subject->getData3dHandle(), img_subject->sz0(), img_subject->sz1(), img_subject->sz2());

    Vol3DSimple<MYFLOAT_JBA> * targetSmall = 0, *warpedSmall = 0, *subjectSmall = 0;
    Vol3DSimple<MYFLOAT_JBA> * dfx = 0, * dfy =0, * dfz = 0, * dft =0;
	WarpParameterAffine3D wp_current, wp_currotate, wp_best, wp_tmp, wp_local_best, wp_best_slightadjust;
	Image2DSimple<MYFLOAT_JBA> * dTable=0; 
	bool b_returnDTable = false;
	double err_val, err[5], err_val0;
	double err_best; V3DLONG ck_best;
	double scale_tmp; // best_scale;

	bool b_slightadjusting=true;

	Vol3DSimple<MYFLOAT_JBA> *subject_tmp=0;

	const double PI = 3.1415926535897932384626433;
	V3DLONG NCK = 180; double angle_nck = 360.0/NCK;
	V3DLONG ck;

	V3DLONG n;
	V3DLONG sz0, sz1, sz2;

	double cur_dfactor = double(img_warped->sz0())/(64.0);

    if (!downsample3dvol(targetSmall, img_target, cur_dfactor) ||
	    !downsample3dvol(subjectSmall, img_subject, cur_dfactor) ||
		!downsample3dvol(warpedSmall, img_warped, cur_dfactor))
	{
	    b_noerror = false;
	    goto Label_exit;
	}

	sz0 = warpedSmall->sz0();	sz1 = warpedSmall->sz1();	sz2 = warpedSmall->sz2();

	subject_tmp = new Vol3DSimple<MYFLOAT_JBA> (sz0, sz1, sz2);

	if (!subject_tmp || !subject_tmp->valid() ||
	    newGradientData(dfx, dfy, dfz, dft, sz0, sz1, sz2)==false)
	{
		b_noerror=false;
		goto Label_exit;
	}

	for (ck=0;ck<NCK;ck++)
	{
	    printf("Now in shift trans***ck=%ld [rotate=%7.2f degrees].\n", ck, ck*angle_nck);
		wp_currotate.resetToDefault();
#ifndef POSITIVE_Y_COORDINATE
		wp_currotate.mxx =  cos(double(-ck)*angle_nck/180*PI); wp_currotate.mxy = sin(double(-ck)*angle_nck/180*PI); 
		wp_currotate.myx = -sin(double(-ck)*angle_nck/180*PI); wp_currotate.myy = cos(double(-ck)*angle_nck/180*PI); 
#else
		wp_currotate.mxx =  cos(double(ck)*angle_nck/180*PI); wp_currotate.mxy = sin(double(ck)*angle_nck/180*PI); 
		wp_currotate.myx = -sin(double(ck)*angle_nck/180*PI); wp_currotate.myy = cos(double(ck)*angle_nck/180*PI); 
#endif
		vol3d_assign(warpedSmall->getData3dHandle(), subjectSmall->getData3dHandle(), sz0, sz1, sz2);
		affine_warp(warpedSmall, &wp_currotate);
		vol3d_assign(subject_tmp->getData3dHandle(), warpedSmall->getData3dHandle(), sz0, sz1, sz2);

		wp_currotate.resetToDefault();
		for (n=1; n<=bwp.nloop_localest; n++)
		{
			printf("--> loop %ld.", n);

			diffxyz(dfx->getData3dHandle(), dfy->getData3dHandle(), dfz->getData3dHandle(), dft->getData3dHandle(),
			        targetSmall->getData3dHandle(), warpedSmall->getData3dHandle(), sz0, sz1, sz2);

			ebs3d(&wp_current, dTable, dfx, dfy, dfz, dft, warpedSmall, 0, sz0-1, 0, sz1-1, 0, sz2-1, 0, b_returnDTable);

			aggregateAffineWarp(&wp_tmp, &wp_current, &wp_currotate);

			vol3d_assign(warpedSmall->getData3dHandle(), subject_tmp->getData3dHandle(), sz0, sz1, sz2);
			affine_warp(warpedSmall, &wp_tmp);

			vol3d_square_root_diff(err_val, warpedSmall->getData3dHandle(), targetSmall->getData3dHandle(), sz0, sz1, sz2);
			printf("\t error = %7.2f\n", err_val);

			if (n==1) {err[0]=err[1]=err_val0 = err_val; wp_local_best.copy(wp_tmp);}
			else
			{
			  if ((err_val>err[1] && err[1]>err[0]) || (err[1]>err_best+10)) //err_val>err_val0+1
			    break;
			  else
			  {
				err[0]=err[1]; err[1]=err_val;
				if (err_val<err_val0)
				{
				  err_val0 = err_val;
				  wp_local_best.copy(wp_tmp);
				}
				wp_currotate.copy(wp_tmp);
			  }
			}
		}

		if (ck==0)
		{
			err_best = err_val; ck_best=0; wp_best.copy(wp_local_best);
		}
		else
		{
			if (err_val<err_best)
			{
				err_best = err_val; ck_best=ck; wp_best.copy(wp_local_best);
				vol3d_assign(warpedSmall->getData3dHandle(), subject_tmp->getData3dHandle(), sz0, sz1, sz2);
				affine_warp(warpedSmall, &wp_best);
			}
		}
	    printf("***so far the best rotate is [ck*=%ld] [rotate=%7.2f degrees].\n", ck_best, ck_best*angle_nck);
		wp_best.print("wp_best");
	}

	b_slightadjusting=true;
	if (b_slightadjusting)
	{

		wp_currotate.resetToDefault();
#ifndef POSITIVE_Y_COORDINATE
		wp_currotate.mxx =  cos(double(-ck_best)*angle_nck/180*PI); wp_currotate.mxy = sin(double(-ck_best)*angle_nck/180*PI); 
		wp_currotate.myx = -sin(double(-ck_best)*angle_nck/180*PI); wp_currotate.myy = cos(double(-ck_best)*angle_nck/180*PI); 
#else
		wp_currotate.mxx =  cos(double(ck_best)*angle_nck/180*PI); wp_currotate.mxy = sin(double(ck_best)*angle_nck/180*PI); 
		wp_currotate.myx = -sin(double(ck_best)*angle_nck/180*PI); wp_currotate.myy = cos(double(ck_best)*angle_nck/180*PI); 
#endif
		vol3d_assign(warpedSmall->getData3dHandle(), subjectSmall->getData3dHandle(), sz0, sz1, sz2);
		affine_warp(warpedSmall, &wp_currotate);
		affine_warp(warpedSmall, &wp_best);
		vol3d_assign(subject_tmp->getData3dHandle(), warpedSmall->getData3dHandle(), sz0, sz1, sz2);

		wp_currotate.resetToDefault();
		for (scale_tmp=0.75; scale_tmp<=1.3; scale_tmp+=0.025)
		{
			printf("--> scale %5.2f.", scale_tmp);

			wp_tmp.resetToDefault();
			wp_tmp.mxx *= scale_tmp;
			wp_tmp.myy *= scale_tmp;
			wp_tmp.mzz *= scale_tmp;

			vol3d_assign(warpedSmall->getData3dHandle(), subject_tmp->getData3dHandle(), sz0, sz1, sz2);
			affine_warp(warpedSmall, &wp_tmp);

			vol3d_square_root_diff(err_val, warpedSmall->getData3dHandle(), targetSmall->getData3dHandle(), sz0, sz1, sz2);
			printf("\t error = %7.2f previous err_best = %7.2f\n", err_val, err_best);

			if (err_val<err_best)
			{
			  err_best = err_val;
			  wp_local_best.copy(wp_tmp);
			}
		}

		{
			wp_best_slightadjust.copy(wp_local_best);
			vol3d_assign(warpedSmall->getData3dHandle(), subject_tmp->getData3dHandle(), sz0, sz1, sz2);
			affine_warp(warpedSmall, &wp_best_slightadjust);
		}
		wp_best_slightadjust.print("wp_best_slightadjust");
	}

	warpPara_global.copy(wp_best);
	warpPara_global.print("warpPara_global_before_scaling");
	warpPara_global.scale(cur_dfactor);
	cout<<"cur_dafactor="<<cur_dfactor<<endl;
	warpPara_global.print("warpPara_global_after_scaling");

	wp_currotate.resetToDefault();
#ifndef POSITIVE_Y_COORDINATE
	wp_currotate.mxx =  cos(double(-ck_best)*angle_nck/180*PI); wp_currotate.mxy = sin(double(-ck_best)*angle_nck/180*PI); 
	wp_currotate.myx = -sin(double(-ck_best)*angle_nck/180*PI); wp_currotate.myy = cos(double(-ck_best)*angle_nck/180*PI); 
#else
	wp_currotate.mxx =  cos(double(ck_best)*angle_nck/180*PI); wp_currotate.mxy = sin(double(ck_best)*angle_nck/180*PI); 
	wp_currotate.myx = -sin(double(ck_best)*angle_nck/180*PI); wp_currotate.myy = cos(double(ck_best)*angle_nck/180*PI); 
#endif

	aggregateAffineWarp(&wp_tmp, &wp_currotate, &warpPara_global);
	if (b_slightadjusting) 
	{
		aggregateAffineWarp(&warpPara_global, &wp_tmp, &wp_best_slightadjust);
	}
	else
	{
		warpPara_global.copy(wp_tmp);
	}

Label_exit:

	if (dTable) {delete dTable; dTable=0;} 

    if (subject_tmp) {delete subject_tmp; subject_tmp=0;}

	deleteGradientData(dfx, dfy, dfz, dft);

	if (warpedSmall) {delete warpedSmall; warpedSmall=0;}
	if (subjectSmall) {delete subjectSmall; subjectSmall=0;}
	if (targetSmall) {delete targetSmall; targetSmall=0;}

	img_warped->unpadding(rx, ry, rz);
	img_target->unpadding(rx, ry, rz);
	img_subject->unpadding(rx, ry, rz);
	cout << "unpadding: new size " << img_subject->sz0() << ","<< img_subject->sz1() << ","<< img_subject->sz2() << endl;

	if (b_noerror)
	{
		return get_DF_of_affine_warp(img_subject, &warpPara_global); 
	}
	else
	{
		return 0;
	}
}

bool Warp3D::warpSubjectImage(Vol3DSimple <DisplaceFieldF3D> *df, bool bni)
{
	if (!img_subject || !img_subject->valid()) return false;
	if (!df || !df->valid()) return false;

	if (!isSameSize(img_subject, df))
	{
		fprintf(stderr, "Different sizes of the subject image and the df in warpSubjectImage().\n");
		return false;
	}

	df_warp(img_subject, df, bni);
	return true;
}

vector<Coord3D_JBA> readPosFile(string posFile)
{
  vector<Coord3D_JBA> coordPos;
  Coord3D_JBA c3d;

  char curline[2000];
  ifstream file_op;
  file_op.open(posFile.c_str());
  if (!file_op)
  {
    fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
	return coordPos;
  }

  V3DLONG xpos, ypos, zpos;  xpos=ypos=zpos=-1;
  V3DLONG k=0;
  while(!file_op.eof())
  {
	file_op.getline(curline, 2000);
	k++;
	if (k>0) 
	{
	  sscanf(curline, "%ld,%ld,%ld", &xpos, &ypos, &zpos);
	  if (xpos==-1 || ypos==-1 || zpos==-1)
	  {
	    continue;
	  }
	  else
	  {
	    c3d.x = xpos; 
        c3d.y = ypos;
		c3d.z = zpos;
	    coordPos.push_back(c3d);
	  }
	  xpos=ypos=zpos=-1; 
	}
  }
  file_op.close();

  return coordPos;
}

bool readPosFile_3dpair(string posFile, vector<Coord3D_JBA> & pos1, vector<Coord3D_JBA> & pos2)
{
  Coord3D_JBA c3d;

  pos1.erase(pos1.begin(), pos1.end());
  pos2.erase(pos2.begin(), pos2.end());

  char curline[2000];
  ifstream file_op;
  file_op.open(posFile.c_str());
  if (!file_op)
  {
    fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
	return false;
  }

  V3DLONG xpos, ypos, zpos;  xpos=ypos=zpos=-1;
  V3DLONG s_xpos, s_ypos, s_zpos;  s_xpos=s_ypos=s_zpos=-1;
  V3DLONG k=0, KK=0;
  while(!file_op.eof())
  {
	file_op.getline(curline, 2000);
	k++;
	if (k>0) //ignore the first line
	{
	  sscanf(curline, "%ld,%ld,%ld, %ld,%ld,%ld", &xpos, &ypos, &zpos, &s_xpos, &s_ypos, &s_zpos);
	  if (xpos==-1 || ypos==-1 || zpos==-1 || s_xpos==-1 || s_ypos==-1 || s_zpos==-1)
	  {
	    continue;
	  }
	  else
	  {
	    c3d.x = xpos; 
		c3d.y = ypos;
		c3d.z = zpos;
	    pos1.push_back(c3d);

	    c3d.x = s_xpos;
		c3d.y = s_ypos;
		c3d.z = s_zpos;
		pos2.push_back(c3d);

		KK++;
	  }
	  xpos=ypos=zpos=-1; 
	  s_xpos=s_ypos=s_zpos=-1;
	}
  }
  file_op.close();

  return (KK>0)?true:false;
}


bool Warp3D::readBestMatchingCptFile(vector<Coord3D_JBA> & matchTargetPos,
									vector<Coord3D_JBA> & matchSubjectPos,
									Vol3DSimple<MYFLOAT_JBA> * img_target,
								   Vol3DSimple<MYFLOAT_JBA> * img_subject,
								   Coord3D_JBA targetOffset,
								   Coord3D_JBA subjectOffset,
								   Coord3D_JBA targetMultiplyFactor,
								   Coord3D_JBA subjectMultiplyFactor,
								   string priorTargetPosFile,
								   string priorSubjectPosFile,
								   bool b_inonefile = false)
{
    if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid() || !isSameSize(img_subject, img_target) )	{return false;}

	if (b_inonefile || priorSubjectPosFile.compare("unsetyet")==0)
	{
		readPosFile_3dpair(priorTargetPosFile, matchTargetPos, matchSubjectPos);
	}
	else
	{
		matchTargetPos = readPosFile(priorTargetPosFile);
		matchSubjectPos = readPosFile(priorSubjectPosFile);
	}

	if (matchTargetPos.size()>0 && matchSubjectPos.size()>0 && matchTargetPos.size()==matchSubjectPos.size())
	{
		printf("Read %ld predefined control points from the file. \n", matchTargetPos.size());
	}
	else
	{
	    fprintf(stderr, "Error in reading the landmark files. Check it!\n");
		return false;
	}

	unsigned V3DLONG i;
	Coord3D_JBA tmpc;
	for (i=0;i<matchTargetPos.size();i++)
	{
		tmpc = matchTargetPos.at(i);
		matchTargetPos.at(i).x = (tmpc.x * targetMultiplyFactor.x) + targetOffset.x;
		matchTargetPos.at(i).y = (tmpc.y * targetMultiplyFactor.y) + targetOffset.y; 
        matchTargetPos.at(i).z = (tmpc.z * targetMultiplyFactor.z) + targetOffset.z;
	}
	for (i=0;i<matchSubjectPos.size();i++)
	{
		tmpc = matchSubjectPos.at(i);
		matchSubjectPos.at(i).x = (tmpc.x * subjectMultiplyFactor.x) + subjectOffset.x;
		matchSubjectPos.at(i).y = (tmpc.y * subjectMultiplyFactor.y) + subjectOffset.y; 
        matchSubjectPos.at(i).z = (tmpc.z * subjectMultiplyFactor.z) + subjectOffset.z;
	}

	printf("target offset = %5.4f %5.4f %5.4f\n", targetOffset.x, targetOffset.y, targetOffset.z);
	printf("target multifactor = %5.4f %5.4f %5.4f\n", targetMultiplyFactor.x, targetMultiplyFactor.y, targetMultiplyFactor.z);
	printf("subject offset = %5.4f %5.4f %5.4f\n", subjectOffset.x, subjectOffset.y, subjectOffset.z);
	printf("subject multifactor = %5.4f %5.4f %5.4f\n", subjectMultiplyFactor.x, subjectMultiplyFactor.y, subjectMultiplyFactor.z);

	bool b_useBoundaryCond=false;
	if (b_useBoundaryCond)
	{
		Coord3D_JBA corner;
		corner.x = 0; corner.y = 0; corner.z = 0;
		matchTargetPos.push_back(corner);
		corner.x = 0; corner.y = 0; corner.z = 0;
		matchSubjectPos.push_back(corner);

		corner.x = 0; corner.y = 0; corner.z = img_target->sz2()-1;
		matchTargetPos.push_back(corner);
		corner.x = 0; corner.y = 0; corner.z = img_subject->sz2()-1;
		matchSubjectPos.push_back(corner);

		corner.x = 0; corner.y = img_target->sz1()-1; corner.z = 0;
		matchTargetPos.push_back(corner);
		corner.x = 0; corner.y = img_subject->sz1()-1; corner.z = 0;
		matchSubjectPos.push_back(corner);

		corner.x = 0; corner.y = img_target->sz1()-1; corner.z = img_target->sz2()-1;
		matchTargetPos.push_back(corner);
		corner.x = 0; corner.y = img_subject->sz1()-1; corner.z = img_subject->sz2()-1;
		matchSubjectPos.push_back(corner);

		corner.x = img_target->sz0()-1; corner.y = 0; corner.z = 0;
		matchTargetPos.push_back(corner);
		corner.x = img_subject->sz0()-1; corner.y = 0; corner.z = 0;
		matchSubjectPos.push_back(corner);

		corner.x = img_target->sz0()-1; corner.y = 0; corner.z = img_target->sz2()-1;
		matchTargetPos.push_back(corner);
		corner.x = img_subject->sz0()-1; corner.y = 0; corner.z = img_subject->sz2()-1;
		matchSubjectPos.push_back(corner);

		corner.x = img_target->sz0()-1; corner.y = img_target->sz1()-1; corner.z = 0;
		matchTargetPos.push_back(corner);
		corner.x = img_subject->sz0()-1; corner.y = img_subject->sz1()-1; corner.z = 0;
		matchSubjectPos.push_back(corner);

		corner.x = img_target->sz0()-1; corner.y = img_target->sz1()-1; corner.z = img_target->sz2()-1;
		matchTargetPos.push_back(corner);
		corner.x = img_subject->sz0()-1; corner.y = img_subject->sz1()-1; corner.z = img_subject->sz2()-1;
		matchSubjectPos.push_back(corner);
	}

    return true;
}

Vol3DSimple<DisplaceFieldF3D> * Warp3D::do_landmark_warping(const BasicWarpParameter & bwp1)
{
	BasicWarpParameter bwp = bwp1;
    if (!img_warped || !img_warped->valid() || !img_target || !img_target->valid() || !img_subject || !img_subject->valid() ||
	    !isSameSize(img_warped, img_target) || !isSameSize(img_warped, img_subject))
	{return 0;}

	V3DLONG d0=img_subject->sz0(), d1=img_subject->sz1(), d2=img_subject->sz2();

	unsigned V3DLONG n, i, j, k;

	vector<Coord3D_JBA> matchTargetPos, matchSubjectPos;
	PointMatchScore matchScore;
	string priorTargetPosFile = bwp.file_landmark_target;

	vector<Coord3D_JBA> priorTargetPos = readPosFile(priorTargetPosFile);
	if (priorTargetPos.size()>0)
	{
		printf("Read %ld predefined control points from the file. Thus will not invoke the automatic control point detection.\n", priorTargetPos.size());
	}
	else
	{
		fprintf(stderr, "Did not get anything from the pos file. Check it!\n");
		printf("Now automatically define the control points.\n");

		V3DLONG CPT_STEP=16;
		MYFLOAT_JBA *** img_target_ref = img_target->getData3dHandle();
		for (k=1;k<=int(floor(img_target->sz2()/CPT_STEP)-1); k++)
		{
			for (j=1;j<=int(floor(img_target->sz1()/CPT_STEP)-1); j++)
			{
				for (i=1;i<=int(floor(img_target->sz0()/CPT_STEP)-1); i++)
				{
					if (img_target_ref[k*CPT_STEP-1][j*CPT_STEP-1][i*CPT_STEP-1]>0.1) 
					{
						priorTargetPos.push_back(Coord3D_JBA(i*CPT_STEP-1,j*CPT_STEP-1,k*CPT_STEP-1));
						printf("Added control point [%ld][%ld][%ld] pixel intensity=[%5.3f]\n", k*CPT_STEP-1, j*CPT_STEP-1, i*CPT_STEP-1, img_target_ref[k*CPT_STEP-1][j*CPT_STEP-1][i*CPT_STEP-1]);
					}
				}
			}
		}
		if (priorTargetPos.size()<=0)
		{
			fprintf(stderr, "The target image seems wierd, cannot find reasonable control points on the grid. Exit.\n");
			return false;
		}
	}

	Vol3DSimple<MYFLOAT_JBA> * targetSmall = 0, *subjectSmall = 0;
	if (bwp.hierachical_match_level>1)
	{
		if (!downsample3dvol(targetSmall, img_target, bwp.hierachical_match_level) ||
			!downsample3dvol(subjectSmall, img_subject, bwp.hierachical_match_level) )
		{
			if (subjectSmall) {delete subjectSmall; subjectSmall=0;}
			if (targetSmall) {delete targetSmall; targetSmall=0;}
			printf("Fail to downsample images in do_landmark_warping(). \n");
			return 0;
		}

		for (n=0; n<priorTargetPos.size(); n++)
		{
			priorTargetPos.at(n).x = floor(priorTargetPos.at(n).x/bwp.hierachical_match_level);
			priorTargetPos.at(n).y = floor(priorTargetPos.at(n).y/bwp.hierachical_match_level);
			priorTargetPos.at(n).z = floor(priorTargetPos.at(n).z/bwp.hierachical_match_level);
		}
	}
	else
	{
		targetSmall = img_target;
		subjectSmall = img_subject;
	}

	bool b_res = detectBestMatchingCpt_virtual(matchTargetPos, matchSubjectPos, targetSmall, subjectSmall, matchScore, priorTargetPos, bwp);
    
	if (bwp.hierachical_match_level>1)
	{
		if (subjectSmall) {delete subjectSmall; subjectSmall=0;}
		if (targetSmall) {delete targetSmall; targetSmall=0;}
		for (n=0; n<matchTargetPos.size(); n++)
		{
			matchTargetPos.at(n).x *= bwp.hierachical_match_level; 
			matchTargetPos.at(n).y *= bwp.hierachical_match_level;
			matchTargetPos.at(n).z *= bwp.hierachical_match_level;

			matchSubjectPos.at(n).x *= bwp.hierachical_match_level;
			matchSubjectPos.at(n).y *= bwp.hierachical_match_level;
			matchSubjectPos.at(n).z *= bwp.hierachical_match_level;
		}
	}
	else
	{
		subjectSmall=0;	targetSmall=0; 
	}

	if (b_res == false)
	{
		printf("-------------------- Something wrong in detecting the best matching points. check you program and data. -------\n\n");
		return 0;
	}
	else
	{
		printf("-------------------- Finished detecting the best matching points. Now compute displacement field. -------\n\n");
	}

	string tmp_str;
	FILE *tmp_fp=0;

	tmp_str = bwp.tag_output_image_file + "_matching_quality.csv";
	printf("Prepare save the matching quality record file [%s]\n", tmp_str.c_str());
	tmp_fp = fopen(tmp_str.c_str(), "w");
    fprintf(tmp_fp, "method_inconsistency, model_violation\n");
	fprintf(tmp_fp, "%7.4f,%7.4f\n", matchScore.method_inconsistency, matchScore.model_violation);
	printf("method_inconsistency=%7.4f,\tmodel_violation=%7.4f\n", matchScore.method_inconsistency, matchScore.model_violation);
	fclose(tmp_fp);

	tmp_str = bwp.tag_output_image_file + "_subject.csv";
	printf("Now save to matching point file of subject to [%s]\n", tmp_str.c_str());
	tmp_fp = fopen(tmp_str.c_str(), "w");
    fprintf(tmp_fp, "x,y,z\n");
   	for (n=0; n<matchSubjectPos.size(); n++)
	{
		fprintf(tmp_fp, "%ld,%ld,%ld\n", V3DLONG(matchSubjectPos.at(n).x), V3DLONG(matchSubjectPos.at(n).y), V3DLONG(matchSubjectPos.at(n).z));
	}
	fclose(tmp_fp);

	tmp_str = bwp.tag_output_image_file + "_target.csv";
	printf("Now save to matching point file of target to [%s]\n", tmp_str.c_str());
	tmp_fp = fopen(tmp_str.c_str(), "w");
    fprintf(tmp_fp, "x,y,z\n");
   	for (n=0; n<matchTargetPos.size(); n++)
	{
		fprintf(tmp_fp, "%ld,%ld,%ld\n", V3DLONG(matchTargetPos.at(n).x), V3DLONG(matchTargetPos.at(n).y), V3DLONG(matchTargetPos.at(n).z));
	}
	fclose(tmp_fp);

	Vol3DSimple<DisplaceFieldF3D> *df_local = cdum(matchTargetPos, matchSubjectPos, d0, d1, d2, bwp.method_df_compute);
	return df_local;
}


Vol3DSimple<DisplaceFieldF3D> * Warp3D::do_landmark_in_file_warping(const BasicWarpParameter & bwp)
{
    if (!img_warped || !img_warped->valid() || !img_target || !img_target->valid() || !img_subject || !img_subject->valid() ||
	    !isSameSize(img_warped, img_target) || !isSameSize(img_warped, img_subject))
	{return 0;}

	V3DLONG d0=img_subject->sz0(), d1=img_subject->sz1(), d2=img_subject->sz2();

	vector<Coord3D_JBA> matchTargetPos, matchSubjectPos;

	Coord3D_JBA cur_targetOffset, cur_subjectOffset;

	cur_targetOffset.x = spos_target[0];
	cur_targetOffset.y = spos_target[1];
	cur_targetOffset.z = spos_target[2];

	cur_subjectOffset.x = spos_subject[0];
	cur_subjectOffset.y = spos_subject[1];
	cur_subjectOffset.z = spos_subject[2];

	if (readBestMatchingCptFile(matchTargetPos, matchSubjectPos, img_target, img_subject,
	    cur_targetOffset, cur_subjectOffset,
	    bwp.targetMultiplyFactor, bwp.subjectMultiplyFactor,
	    bwp.file_landmark_target, bwp.file_landmark_subject) == false)
	{
	  printf("-------------------- Unable to read the matching points from file. Please check to assure the correctness. -------\n\n");
	  return 0;
	}
	else
	{
	  printf("-------------------- Finished read the best matching points from file. Now compute displacement field. -------\n\n");
	}


	PointMatchScore matchScore;
	analyze_model_matching_smoothness(matchTargetPos, matchSubjectPos, matchScore);
	matchScore.method_inconsistency=-1; 

	string tmp_str;
	FILE *tmp_fp=0;

	tmp_str = bwp.tag_output_image_file + "_matching_quality.csv";
	printf("Prepare to save the matching quality record file [%s]\n", tmp_str.c_str());
	tmp_fp = fopen(tmp_str.c_str(), "w");
    fprintf(tmp_fp, "method_inconsistency, model_violation\n");
	fprintf(tmp_fp, "%7.4f,%7.4f\n", matchScore.method_inconsistency, matchScore.model_violation);
	printf("method_inconsistency=%7.4f,\tmodel_violation=%7.4f\n", matchScore.method_inconsistency, matchScore.model_violation);
	fclose(tmp_fp);

	Vol3DSimple<DisplaceFieldF3D> *df_local = cdum(matchTargetPos, matchSubjectPos, d0, d1, d2, bwp.method_df_compute);
	return df_local;
}

bool Warp3D::computemask(
	Vol3DSimple<unsigned char> * mask,
	Vol3DSimple<MYFLOAT_JBA> * dfx,
	Vol3DSimple<MYFLOAT_JBA> * dfy,
	Vol3DSimple<MYFLOAT_JBA> * dfz,
	Vol3DSimple<MYFLOAT_JBA> * dft,
	Vol3DSimple<MYFLOAT_JBA> * img_target,
	Vol3DSimple<MYFLOAT_JBA> * img_subject,
	int b_verbose_print)
{
    if (!mask || !mask->valid() ||
	    !dfx || !dfx->valid() || !dfy || !dfy->valid() || !dfz || !dfz->valid() || !dft || !dft->valid() ||
		!img_target || !img_target->valid() || !img_subject || !img_subject->valid() ||
        !isSameSize((Vol3DSimple<void> *)mask, (Vol3DSimple<void> *)img_target) ||
        !isSameSize((Vol3DSimple<void> *)mask, (Vol3DSimple<void> *)img_subject) ||
	    !isSameSize(dfx, dfy) || !isSameSize(dfx, dfz) || !isSameSize(dfx, dft) || !isSameSize(dfx, img_subject))
	{
		fprintf(stderr, "Invalid parameters to computemask(). \n");
		return false;
	}

	V3DLONG sz0 = img_subject->sz0(), sz1 = img_subject->sz1(), sz2 = img_subject->sz2();

	unsigned char *** mask_ref = mask->getData3dHandle();
	MYFLOAT_JBA *** dfx_ref = dfx->getData3dHandle();
	MYFLOAT_JBA *** dfy_ref = dfy->getData3dHandle();
	MYFLOAT_JBA *** dfz_ref = dfz->getData3dHandle();

	V3DLONG i,j,k,cnt;
	double t,s;

	cnt=0;
	for (k=0;k<sz2;k++)
	{
	  for (j=0;j<sz1;j++)
	  {
	    for (i=0;i<sz0; i++)
		{
		  s=0.0;
		  t = dfx_ref[k][j][i]; s += t*t;
		  t = dfy_ref[k][j][i]; s += t*t;
		  t = dfz_ref[k][j][i]; s += t*t;
		  s = sqrt(s);
		  if (s > 0.1
		     )
		  {
		     mask_ref[k][j][i] = 255;
		     cnt++;
		  }
		  else
		  {
		     mask_ref[k][j][i] = 0;
		  }
		}
	  }
	}

	return true;
}

bool diffxyz(MYFLOAT_JBA ***dx, MYFLOAT_JBA ***dy, MYFLOAT_JBA ***dz, MYFLOAT_JBA *** dt,
                     MYFLOAT_JBA *** cur_img3d_target, MYFLOAT_JBA *** cur_img3d_subject,
					 V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
    V3DLONG tlen = d0*d1*d2;
	if (tlen<=0 || d0<=0 || d1<=0 || d2<=0)
	{
		fprintf(stderr, "length error.\n");
		return false;
	}

	if (!dx || !dy || !dz || !dt || !cur_img3d_target || !cur_img3d_subject)
	{
		fprintf(stderr, "NULL input vector.\n");
		return false;
    }

	MYFLOAT_JBA * tmp1d = new MYFLOAT_JBA [tlen];
	if (!tmp1d)
	{
		fprintf(stderr, "Fail to allocate memory [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	MYFLOAT_JBA ***tmp3d = 0;
	if (new3dpointer(tmp3d, d0, d1, d2, tmp1d)==false)
	{
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		return false;
	}

#define LEN_KERNEL 5

	MYFLOAT_JBA kernel_p_xyz[LEN_KERNEL]   = {0.036420, 0.248972, 0.429217, 0.248972, 0.036420};
	MYFLOAT_JBA kernel_d_xyz_m[LEN_KERNEL] = {0.108415, 0.280353, 0.0, -0.280353, -0.108415};
#ifndef POSITIVE_Y_COORDINATE
	MYFLOAT_JBA kernel_d_xyz[LEN_KERNEL]   = {-0.108415, -0.280353, 0.0, 0.280353, 0.108415}; 
#else
	MYFLOAT_JBA kernel_d_xyz[LEN_KERNEL]   = {0.108415, 0.280353, 0.0, -0.280353, -0.108415}; 
#endif

	vol3d_plus(tmp3d, cur_img3d_target, cur_img3d_subject, d0, d1, d2);
	vol3d_time_constant(tmp3d, tmp3d, 0.5, d0, d1, d2);

	conv3d_1dvec(dx, tmp3d, d0, d1, d2, kernel_d_xyz_m, LEN_KERNEL, 1);
	conv3d_1dvec(dx, dx,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 2);
	conv3d_1dvec(dx, dx,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 3);

	conv3d_1dvec(dy, tmp3d, d0, d1, d2, kernel_d_xyz, LEN_KERNEL, 2);
	conv3d_1dvec(dy, dy,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 3);
	conv3d_1dvec(dy, dy,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 1);

	conv3d_1dvec(dz, tmp3d, d0, d1, d2, kernel_d_xyz_m, LEN_KERNEL, 3);
	conv3d_1dvec(dz, dz,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 1);
	conv3d_1dvec(dz, dz,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 2);

	vol3d_minus(tmp3d, cur_img3d_subject, cur_img3d_target, d0, d1, d2);

	conv3d_1dvec(dt, tmp3d, d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 1);
	conv3d_1dvec(dt, dt,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 2);
	conv3d_1dvec(dt, dt,    d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 3);

	if (tmp3d) delete3dpointer(tmp3d, d0, d1, d2);
	if (tmp1d) {delete []tmp1d; tmp1d=0;}

	return true;
}

bool eba3d_noshift(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable,
                                Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft,
								Vol3DSimple<MYFLOAT_JBA> * subject,
                                V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
								bool b_returnDTable) 
{
	if (!wp ||
	    !dfx || !dfx->valid() ||
		!dfy || !dfy->valid() ||
		!dfz || !dfz->valid() ||
		!dft || !dft->valid() ||
		!subject || !subject->valid() )
	{
	    fprintf(stderr, "Invalid internal data pointers [%s][%d].\n", __FILE__, __LINE__);
	    return false;
	}

	V3DLONG tsz0 = subject->sz0(), tsz1 = subject->sz1(), tsz2 = subject->sz2();

	double xc = (double(xe) + double(xs))/2.0;
	double yc = (double(ye) + double(ys))/2.0;
	double zc = (double(ze) + double(zs))/2.0;
    if (xs>xe ||
		ys>ye ||
		zs>ze ||
		xc < 0 || xc > (tsz0-1) ||
		yc < 0 || yc > (tsz1-1) ||
		zc < 0 || zc > (tsz2-1) )
	{
	    fprintf(stderr, "Invalid cube coordinate parameters [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (dTable)
	{
	    fprintf(stderr, "The dTable is not initilized as empty! Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	MYFLOAT_JBA ***	dfx3d = dfx->getData3dHandle();
	MYFLOAT_JBA ***	dfy3d = dfy->getData3dHandle();
	MYFLOAT_JBA ***	dfz3d = dfz->getData3dHandle();
	MYFLOAT_JBA ***	dft3d = dft->getData3dHandle();
	MYFLOAT_JBA ***	subject3d = subject->getData3dHandle();

	//prepare data space

	V3DLONG i,j,k;

	V3DLONG is = hardLimit(xs, V3DLONG(0), tsz0-1), ie = hardLimit(xe, V3DLONG(0), tsz0-1);
	V3DLONG js = hardLimit(ys, V3DLONG(0), tsz1-1), je = hardLimit(ye, V3DLONG(0), tsz1-1);
	V3DLONG ks = hardLimit(zs, V3DLONG(0), tsz2-1), ke = hardLimit(ze, V3DLONG(0), tsz2-1);

	V3DLONG xlen = ie-is+1, ylen = je-js+1, zlen = ke-ks+1;
	V3DLONG cubelen = (V3DLONG)xlen*ylen*zlen;

	const V3DLONG myCUBECOLUMNNUM=12;

	Image2DSimple<MYFLOAT_JBA> *curDTable = new Image2DSimple<MYFLOAT_JBA> (cubelen, myCUBECOLUMNNUM);
	if (!curDTable || !curDTable->valid())
	{
	    fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	MYFLOAT_JBA **  cubeData2d = curDTable->getData2dHandle();

	V3DLONG curind=0;
	for (k=ks;k<=ke;k++)
	{
		double mk = (k-zc);
		for (j=js;j<=je;j++)
		{
#ifndef POSITIVE_Y_COORDINATE	
			double mj = -(j-yc);
#else
			double mj = -(j-yc); 
#endif
			for (i=is;i<=ie;i++)
			{
				double mi = (i-xc);

				cubeData2d[0][curind] = dfx3d[k][j][i] * mi;
				cubeData2d[1][curind] = dfx3d[k][j][i] * mj;
				cubeData2d[2][curind] = dfx3d[k][j][i] * mk;

				cubeData2d[3][curind] = dfy3d[k][j][i] * mi;
				cubeData2d[4][curind] = dfy3d[k][j][i] * mj;
				cubeData2d[5][curind] = dfy3d[k][j][i] * mk;

				cubeData2d[6][curind] = dfz3d[k][j][i] * mi;
				cubeData2d[7][curind] = dfz3d[k][j][i] * mj;
				cubeData2d[8][curind] = dfz3d[k][j][i] * mk;

				cubeData2d[9][curind] = -subject3d[k][j][i];
				cubeData2d[10][curind] = -1;

				cubeData2d[11][curind] = dft3d[k][j][i] + cubeData2d[9][curind] + cubeData2d[0][curind] + cubeData2d[4][curind] + cubeData2d[8][curind];

				curind++;
			}
		}
	}

	Matrix P_Matrix(myCUBECOLUMNNUM-1, myCUBECOLUMNNUM-1);
	Matrix K_Matrix(myCUBECOLUMNNUM-1,1);
	double tmpp=0.0, tmpk=0.0;

	for (k=0;k<myCUBECOLUMNNUM-1;k++)
	{
		for (j=k;j<myCUBECOLUMNNUM-1;j++)
		{
			for (i=0, tmpp=0.0;i<cubelen;i++)
				tmpp += cubeData2d[k][i] * cubeData2d[j][i];
			P_Matrix(k+1,j+1) = tmpp;
			P_Matrix(j+1,k+1) = tmpp;
		}

		for (i=0, tmpk=0.0;i<cubelen;i++)
		{
			tmpk += cubeData2d[k][i] * (cubeData2d[11][i]);
		}
		K_Matrix(k+1,1) = tmpk;
	}

	b_VERBOSE_PRINT=true;
//	if (b_VERBOSE_PRINT)
//	{
//		cout << "P_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << P_Matrix << endl <<endl;

//		cout << "K_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << K_Matrix << endl <<endl;
//	}

	Matrix Y;
	Try
	{
		Y = P_Matrix.i() * K_Matrix;

		wp->mxx = Y(1,1); wp->mxy = Y(2,1); wp->mxz = Y(3,1);
		wp->myx = Y(4,1); wp->myy = Y(5,1); wp->myz = Y(6,1);
		wp->mzx = Y(7,1); wp->mzy = Y(8,1); wp->mzz = Y(9,1);
		wp->si = Y(10,1); wp->sb = Y(11,1);

		wp->b_transform=1; 
	}
	CatchAll
	{
		wp->resetToDefault();
	}

//	if (b_VERBOSE_PRINT)
//	{
//		Matrix Y1(4,4);
//		Y1.row(1) << wp->mxx << wp->mxy << wp->mxz << wp->sx;
//		Y1.row(2) << wp->myx << wp->myy << wp->myz << wp->sy;
//		Y1.row(3) << wp->mzx << wp->mzy << wp->mzz << wp->sz;
//		Y1.row(4) << 0 << 0 << 0 << 1;
//		cout << "Transform matrix" << endl;
//		cout << setw(12) << setprecision(3) << Y1 << endl;
//	}

	if (b_returnDTable==true)
	{
	    dTable = curDTable; 
	}
	else
	{
		if (curDTable) delete curDTable; curDTable=0;
	}
	return true;
}


bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<DisplaceFieldF3D> * p, bool bni)
{
    if (!img || !img->valid() || !p || !p->valid())
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}

	if (isSameSize((Vol3DSimple<void> *)img, (Vol3DSimple<void> *)p) == false)
	{
		fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
		cout << "img size: " << img->sz0() << " " << img->sz1() << " " << img->sz2() <<endl;
		cout << "wp size: " << p->sz0() << " " << p->sz1() << " " << p->sz2() <<endl;
		return false;
	}

	MYFLOAT_JBA *** timg3d = img->getData3dHandle();
	V3DLONG tsz0 = img->sz0();
	V3DLONG tsz1 = img->sz1();
	V3DLONG tsz2 = img->sz2();

	DisplaceFieldF3D *** tp3d = p->getData3dHandle();

	V3DLONG totallen = tsz0 * tsz1 * tsz2;
	Coord3D_JBA * c = new Coord3D_JBA [totallen];
	MYFLOAT_JBA * v = new MYFLOAT_JBA [totallen];

	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}

    //computation
	V3DLONG i,j,k, ind;

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				c[ind].x = i         + tp3d[k][j][i].sx;
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = tsz1-1-j  + tp3d[k][j][i].sy; 
#else
				c[ind].y = j  + tp3d[k][j][i].sy; 
#endif
				c[ind].z = k         + tp3d[k][j][i].sz;
				v[ind] = -1; 
				ind++;
			}
		}
	}

	if (bni)
		lcn(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);
	else
		lcl(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				timg3d[k][j][i] = v[ind++];
			}
		}
	}

	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}

bool df_warp(Vol3DSimple<MYFLOAT_JBA> * img, Vol3DSimple<WarpParameterAffine3D> * p, bool bni)
{
    if (!img || !img->valid() || !p || !p->valid())
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}

	if (isSameSize((Vol3DSimple<void> *)img, (Vol3DSimple<void> *)p) == false)
	   {fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__); return false;}

	MYFLOAT_JBA *** timg3d = img->getData3dHandle();
	V3DLONG tsz0 = img->sz0();
	V3DLONG tsz1 = img->sz1();
	V3DLONG tsz2 = img->sz2();

	WarpParameterAffine3D *** tp3d = p->getData3dHandle();

	V3DLONG totallen = tsz0 * tsz1 * tsz2;
	Coord3D_JBA * c = new Coord3D_JBA [totallen];
	MYFLOAT_JBA * v = new MYFLOAT_JBA [totallen];

	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}

	V3DLONG i,j,k, ind;

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				c[ind].x = i         + tp3d[k][j][i].sx;
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = tsz1-1-j  + tp3d[k][j][i].sy;
#else
				c[ind].y = j  + tp3d[k][j][i].sy; 
#endif
				c[ind].z = k         + tp3d[k][j][i].sz;
				v[ind] = -1; 
				ind++;
			}
		}
	}

	if (bni)
		lcn(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);
	else
		lcl(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				timg3d[k][j][i] = v[ind++];
			}
		}
	}

    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}

bool df_add(Vol3DSimple<DisplaceFieldF3D> * addee, Vol3DSimple<DisplaceFieldF3D> * adder, bool b_nearest_interp)
{
    if (!addee || !addee->valid() || !adder || !adder->valid()) {fprintf(stderr, "Invalid para to df_add().\n"); return false;}
	if (isSameSize(addee, adder)==false) {fprintf(stderr, "The two displace fields have different size! \n"); return false;}

	V3DLONG tsz0 = adder->sz0(), tsz1 = adder->sz1(), tsz2 = adder->sz2();
	V3DLONG i,j,k;

	Vol3DSimple<MYFLOAT_JBA> *tmp_outimg = new Vol3DSimple<MYFLOAT_JBA> (tsz0, tsz1, tsz2);
	if (!tmp_outimg || !tmp_outimg->valid()) {fprintf(stderr, "Fail to allocate temprary memory in df_add()! \n"); return false;}

	MYFLOAT_JBA *** tmp_outimg_ref = tmp_outimg->getData3dHandle();
	DisplaceFieldF3D *** addee_ref = addee->getData3dHandle();

	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_outimg_ref[k][j][i]= addee_ref[k][j][i].sx;}}}
	if (df_warp( tmp_outimg, adder, b_nearest_interp)==true)
	{
		for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { addee_ref[k][j][i].sx = tmp_outimg_ref[k][j][i];}}}
	}
	else
	{
	    if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_outimg_ref[k][j][i]= addee_ref[k][j][i].sy;}}}
	if (df_warp( tmp_outimg, adder, b_nearest_interp)==true)
	{
		for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { addee_ref[k][j][i].sy = tmp_outimg_ref[k][j][i];}}}
	}
	else
	{
	    if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

	for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { tmp_outimg_ref[k][j][i]= addee_ref[k][j][i].sz;}}}
	if (df_warp( tmp_outimg, adder, b_nearest_interp)==true)
	{
		for (k=0;k<tsz2;k++) {for (j=0;j<tsz1;j++) { for (i=0;i<tsz0;i++) { addee_ref[k][j][i].sz = tmp_outimg_ref[k][j][i];}}}
	}
	else
	{
	    if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
		return false;
	}

    if (tmp_outimg) {delete tmp_outimg; tmp_outimg=0;}
	return true;
}

bool df_copy(Vol3DSimple<DisplaceFieldF3D> * copyee, Vol3DSimple<DisplaceFieldF3D> * copyer)
{
    if (!copyee || !copyee->valid() || !copyer || !copyer->valid()) {fprintf(stderr, "Invalid para to df_add().\n"); return false;}
	if (isSameSize(copyee, copyer)==false) {fprintf(stderr, "The two displace fields have different size! \n"); return false;}

	V3DLONG tsz0 = copyer->sz0(), tsz1 = copyer->sz1(), tsz2 = copyer->sz2();
	V3DLONG i,j,k;

	DisplaceFieldF3D ***df_out_ref = copyee->getData3dHandle();
	DisplaceFieldF3D ***df_in_ref = copyer->getData3dHandle();

	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				df_out_ref[k][j][i].sx = df_in_ref[k][j][i].sx;
				df_out_ref[k][j][i].sy = df_in_ref[k][j][i].sy;
				df_out_ref[k][j][i].sz = df_in_ref[k][j][i].sz;
			}
		}
	}

	return true;
}




bool shift_warp(Vol3DSimple<MYFLOAT_JBA> * img, DisplaceFieldF3D * p)
{
    if (!img || !img->valid() || !p)
	{
	    fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
	    return false;
	}

	MYFLOAT_JBA *** timg3d = img->getData3dHandle();
	V3DLONG tsz0 = img->sz0();
	V3DLONG tsz1 = img->sz1();
	V3DLONG tsz2 = img->sz2();

	V3DLONG totallen = tsz0 * tsz1 * tsz2;
	Coord3D_JBA * c = new Coord3D_JBA [totallen];
	MYFLOAT_JBA * v = new MYFLOAT_JBA [totallen];

	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}

	V3DLONG i,j,k, ind;
	Coord3D_JBA offset;
	offset.x = (double(tsz0)-1)/2.0;
	offset.y = (double(tsz1)-1)/2.0;
	offset.z = (double(tsz2)-1)/2.0;

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				c[ind].x = i;
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = tsz1-1-j;
#else
				c[ind].y = j; 
#endif
				c[ind].z = k;
				v[ind] = -1; 
				ind++;
			}
		}
	}

	shift_transform(c, totallen, p, &offset);
	lcl(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				timg3d[k][j][i] = v[ind++];
			}
		}
	}

    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}

void shift_transform(Coord3D_JBA * c, V3DLONG numCoord, DisplaceFieldF3D * p, Coord3D_JBA * offset)
{
    double x,y,z, x1, y1, z1;
    for (V3DLONG i=0;i<numCoord;i++)
	{
	    x = c[i].x - offset->x;
		y = c[i].y - offset->y;
		z = c[i].z - offset->z;
	    x1 = x + p->sx;
	    y1 = y + p->sy;
	    z1 = z + p->sz;
		c[i].x = x1 + offset->x;
		c[i].y = y1 + offset->y;
		c[i].z = z1 + offset->z;
	}
}

Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p)
{
    if (!img || !img->valid() || !p)
	{
	    fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
	    return 0;
	}

	V3DLONG tsz0 = img->sz0();
	V3DLONG tsz1 = img->sz1();
	V3DLONG tsz2 = img->sz2();

	return get_DF_of_affine_warp(tsz0, tsz1, tsz2, p);
}

Vol3DSimple<DisplaceFieldF3D> *  get_DF_of_affine_warp(V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, WarpParameterAffine3D * p)
{
    if (tsz0<=0 || tsz1<=0 || tsz2<=0 || !p)
	{
	    fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
	    return 0;
	}

	Vol3DSimple<DisplaceFieldF3D> * cur_df = 0;
	cur_df = new Vol3DSimple<DisplaceFieldF3D> (tsz0, tsz1, tsz2);
	if (!cur_df || !cur_df->valid())
	{
		fprintf(stderr, "Fail to allocate memory in get_DF_of_affine_warp(). [%s][%d]\n", __FILE__, __LINE__);
		if (cur_df) {delete cur_df; cur_df=0;}
		return 0;
	}
	DisplaceFieldF3D *** cur_df_ref = cur_df->getData3dHandle();

    //computation
	V3DLONG i,j,k;
	Coord3D_JBA offset;
	offset.x = (double(tsz0)-1)/2.0;
	offset.y = (double(tsz1)-1)/2.0;
	offset.z = (double(tsz2)-1)/2.0;

	double x0,y0,z0, x,y,z, x1, y1, z1;

	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				x0 = i;
#ifndef POSITIVE_Y_COORDINATE
				y0 = tsz1-1-j; 
#else
				y0 = j; 
#endif
				z0 = k;

				x = x0 - offset.x;
				y = y0 - offset.y;
				z = z0 - offset.z;

				x1 = x * p->mxx + y * p->mxy + z * p->mxz + p->sx;
				y1 = x * p->myx + y * p->myy + z * p->myz + p->sy;
				z1 = x * p->mzx + y * p->mzy + z * p->mzz + p->sz;

				cur_df_ref[k][j][i].sx = (x1 + offset.x) - x0;
				cur_df_ref[k][j][i].sy = (y1 + offset.y) - y0;
				cur_df_ref[k][j][i].sz = (z1 + offset.z) - z0;
			}
		}
	}

	//
	return cur_df;
}


bool affine_warp(Vol3DSimple<MYFLOAT_JBA> * img, WarpParameterAffine3D * p)
{
    if (!img || !img->valid() || !p)
	{
	    fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
	    return false;
	}

	MYFLOAT_JBA *** timg3d = img->getData3dHandle();
	V3DLONG tsz0 = img->sz0();
	V3DLONG tsz1 = img->sz1();
	V3DLONG tsz2 = img->sz2();

	V3DLONG totallen = tsz0 * tsz1 * tsz2;
	Coord3D_JBA * c = new Coord3D_JBA [totallen];
	MYFLOAT_JBA * v = new MYFLOAT_JBA [totallen];

	if (!c || !v)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d]\n", __FILE__, __LINE__);
		if (c) {delete []c; c=0;}
		if (v) {delete []v; v=0;}
		return false;
	}

    //computation
	V3DLONG i,j,k, ind;
	Coord3D_JBA offset;
	offset.x = (double(tsz0)-1)/2.0;
	offset.y = (double(tsz1)-1)/2.0;
	offset.z = (double(tsz2)-1)/2.0;

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				c[ind].x = i;
#ifndef POSITIVE_Y_COORDINATE
				c[ind].y = tsz1-1-j; 
#else
				c[ind].y = j; 
                
#endif
				c[ind].z = k;
				v[ind] = -1; 
				ind++;
			}
		}
	}

	affine_transform(c, totallen, p, &offset);
	lcl(v, c, totallen, timg3d, tsz0, tsz1, tsz2, 0, tsz0-1, 0, tsz1-1, 0, tsz2-1);

    ind=0;
	for (k=0;k<tsz2;k++)
	{
		for (j=0;j<tsz1;j++)
		{
			for (i=0;i<tsz0;i++)
			{
				timg3d[k][j][i] = v[ind++];
			}
		}
	}

    // free temporary memory
	if (c) {delete []c; c=0;}
	if (v) {delete []v; v=0;}
	return true;
}

void affine_transform(Coord3D_JBA * c, V3DLONG numCoord, WarpParameterAffine3D * p, Coord3D_JBA * offset)
{
    double x,y,z, x1, y1, z1;
    for (V3DLONG i=0;i<numCoord;i++)
	{
	    x = c[i].x - offset->x;
		y = c[i].y - offset->y;
		z = c[i].z - offset->z;
	    x1 = x * p->mxx + y * p->mxy + z * p->mxz + p->sx;
	    y1 = x * p->myx + y * p->myy + z * p->myz + p->sy;
	    z1 = x * p->mzx + y * p->mzy + z * p->mzz + p->sz;
		c[i].x = x1 + offset->x;
		c[i].y = y1 + offset->y;
		c[i].z = z1 + offset->z;
	}
}

bool lcn(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord,
					   MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
					   V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
	if (!interpolatedVal || !c || numCoord<=0 ||
		!templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
		tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
		tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
		tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
	{
		fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}

	double curpx, curpy, curpz;
	V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;

	for (V3DLONG ipt=0;ipt<numCoord;ipt++)
	{
		curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
#ifndef POSITIVE_Y_COORDINATE
		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; 
#else
		curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; 
#endif
		curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;

		cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
		cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
		cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

		if (cpz0==cpz1)
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					interpolatedVal[ipt] = (MYFLOAT_JBA)(templateVol3d[cpz0][cpy0][cpx0]);
				}
				else
				{
					double w0x0y0z = (cpx1-curpx);
					double w1x0y0z = (curpx-cpx0);
					interpolatedVal[ipt] = (w0x0y0z>=w1x0y0z) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz0][cpy0][cpx1];
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy);
					double w0x1y0z = (curpy-cpy0);
					interpolatedVal[ipt] = (w0x0y0z>=w0x1y0z) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz0][cpy1][cpx0];
				}
				else
				{
					if (cpx1-curpx>=curpx-cpx0)
					{
						interpolatedVal[ipt] = (cpy1-curpy>=curpy-cpy0) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz0][cpy1][cpx0];
					}
					else
					{
						interpolatedVal[ipt] = (cpy1-curpy>=curpy-cpy0) ? templateVol3d[cpz0][cpy0][cpx1] : templateVol3d[cpz0][cpy1][cpx1];
					}
				}
			}
		}
		else
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpz1-curpz);
					double w0x0y1z = (curpz-cpz0);

					interpolatedVal[ipt] = (w0x0y0z>=w0x0y1z) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz1][cpy0][cpx0];
				}
				else
				{
					if (cpx1-curpx>=curpx-cpx0)
					{
						interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz1][cpy0][cpx0];
					}
					else
					{
						interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy0][cpx1] : templateVol3d[cpz1][cpy0][cpx1];
					}
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					if (cpy1-curpy>=curpy-cpy0)
					{
						interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz1][cpy0][cpx0];
					}
					else
					{
						interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy1][cpx0] : templateVol3d[cpz1][cpy1][cpx0];
					}
				}
				else
				{
					if (cpx1-curpx>=curpx-cpx0)
					{
						if (cpy1-curpy>=curpy-cpy0)
						{
							interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy0][cpx0] : templateVol3d[cpz1][cpy0][cpx0];
						}
						else
						{
							interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy1][cpx0] : templateVol3d[cpz1][cpy1][cpx0];
						}
					}
					else
					{
						if (cpy1-curpy>=curpy-cpy0)
						{
							interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy0][cpx1] : templateVol3d[cpz1][cpy0][cpx1];
						}
						else
						{
							interpolatedVal[ipt] = (cpz1-curpz>=curpz-cpz0) ? templateVol3d[cpz0][cpy1][cpx1] : templateVol3d[cpz1][cpy1][cpx1];
						}
					}
				}
			}
		}

	}

	return true;
}

bool lcl(MYFLOAT_JBA * interpolatedVal, Coord3D_JBA *c, V3DLONG numCoord,
					   MYFLOAT_JBA *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
					   V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
	if (!interpolatedVal || !c || numCoord<=0 ||
		!templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
		tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
		tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
		tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
	{
		fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}

	double curpx, curpy, curpz;
	V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;

	for (V3DLONG ipt=0;ipt<numCoord;ipt++)
	{
		if (c[ipt].x< tlow0 || c[ipt].x> tup0 || c[ipt].y< tlow1 || c[ipt].y> tup1 || c[ipt].z< tlow2 || c[ipt].z> tup2)
		{
			interpolatedVal[ipt] = 0; 
			continue;
		}
		
		curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
#ifndef POSITIVE_Y_COORDINATE
		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; 
#else
		curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; 
#endif
		curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;

		cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
		cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
		cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

		if (cpz0==cpz1)
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					interpolatedVal[ipt] = (MYFLOAT_JBA)(templateVol3d[cpz0][cpy0][cpx0]);
				}
				else
				{
					double w0x0y0z = (cpx1-curpx);
					double w1x0y0z = (curpx-cpx0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy);
					double w0x1y0z = (curpy-cpy0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
					double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);
					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) +
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]));
				}
			}
		}
		else
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpz1-curpz);
					double w0x0y1z = (curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpz1-curpz); 
					double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);

					double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
					double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy)*(cpz1-curpz); 
					double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);

					double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
					double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);

					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
					double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
					double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

					double w1x1y0z = (curpx-cpx0)*(curpy-cpy0)*(cpz1-curpz);
					double w1x1y1z = (curpx-cpx0)*(curpy-cpy0)*(curpz-cpz0);

					interpolatedVal[ipt] = (MYFLOAT_JBA)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) +
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]) +
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]) + w1x1y1z * double(templateVol3d[cpz1][cpy1][cpx1]) );
				}
			}
		}

	}

	return true;
}



template <class T> bool conv3d_1dvec(MYFLOAT_JBA ***outimg3d, T *** inimg3d, V3DLONG d0, V3DLONG d1, V3DLONG d2, MYFLOAT_JBA *vec, V3DLONG lenvec, int dim_of_conv)
{
	if (!outimg3d || !inimg3d || !vec)
	{
		fprintf(stderr, "NULL input vector.\n");
		return false;
	}

	if (lenvec<=0 || V3DLONG(floor(lenvec/2)*2)==lenvec)
	{
		fprintf(stderr, "The length of the kernel-vector should be an odd number and >0. The convolution is not done.\n");
		return false;
	}

	if (dim_of_conv!=1 && dim_of_conv!=2 && dim_of_conv!=3)
	{
		fprintf(stderr, "Find illegal dim_of_conv.");
		return false;
	}

	V3DLONG i,j,k,n;
	V3DLONG r = (lenvec-1)/2;

	//allocate memory
	V3DLONG d0big = d0+2*r;
	V3DLONG d1big = d1+2*r;
	V3DLONG d2big = d2+2*r;
	MYFLOAT_JBA *inimg_big=new MYFLOAT_JBA [d0big*d1big*d2big]; 
	if (!inimg_big)
	{
		fprintf(stderr, "Fail to allocate memory for the temporary data [%s][%d].", __FILE__, __LINE__);
		return false;
	}
	MYFLOAT_JBA ***inimg3d_big = 0;
	if (!new3dpointer(inimg3d_big, d0big, d1big, d2big, inimg_big))
	{
		fprintf(stderr, "Fail to allocate memory for the temporary data [%s][%d].", __FILE__, __LINE__);
		if (inimg_big) {delete inimg_big; inimg_big=0;}
		return false;
	}

	V3DLONG totallen = d0big*d1big*d2big;
	for (k=0;k<totallen;k++)
	{
		inimg_big[k] = 0;
	}
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				inimg3d_big[k+r][j+r][i+r] = (MYFLOAT_JBA)(inimg3d[k][j][i]);
			}
		}
	}

	//computation
	double s=0;
	switch (dim_of_conv)
	{
		case 1:
			for (k=0;k<d2;k++)
			{
				for (j=0;j<d1;j++)
				{
					for (i=0;i<d0;i++)
					{
						s = 0;
						for (n=0;n<lenvec; n++)
						{
							s += inimg3d_big[k+r][j+r][i+n] * vec[n];
						}
						outimg3d[k][j][i] = s;
					}
				}
			}
			break;

		case 2:
			for (k=0;k<d2;k++)
			{
				for (j=0;j<d1;j++)
				{
					for (i=0;i<d0;i++)
					{
						s = 0;
						for (n=0;n<lenvec; n++)
						{
							s += inimg3d_big[k+r][j+n][i+r] * vec[n];
						}
						outimg3d[k][j][i] = s;
					}
				}
			}
			break;

		case 3:
			for (k=0;k<d2;k++)
			{
				for (j=0;j<d1;j++)
				{
					for (i=0;i<d0;i++)
					{
						s = 0;
						for (n=0;n<lenvec; n++)
						{
							s += inimg3d_big[k+n][j+r][i+r] * vec[n];
						}
						outimg3d[k][j][i] = s;
					}
				}
			}
			break;

		default:
			fprintf(stderr, "Unsupported dim_of_conv is found.\n");
			break;
	}

	//free memory

	if (inimg3d_big) delete3dpointer(inimg3d_big, d0big, d1big, d2big);
	if (inimg_big) {delete []inimg_big; inimg_big=0;}
	return false;
}

template <class T> bool Warp3D::doWarpUsingDF(T * &outimg_warped, V3DLONG * &out_sz_warped, T * inimg_subject, V3DLONG * inimg_sz, const int datatype_warped, Vol3DSimple <DisplaceFieldF3D> * cur_df, bool bni)
{
    if (outimg_warped || out_sz_warped)
	{
		fprintf(stderr, "The handle of warped data is not initilized as empty! Exit without copying data!\n");
		return false; //do nothing if the output parameters are associated with data already
	}

    if (!inimg_subject || !inimg_sz)
	{
		fprintf(stderr, "The handle of subject data is not initilized as valid! Exit without copying data!\n");
		return false; //do nothing
	}

	V3DLONG i,totalpix;

	out_sz_warped = new V3DLONG [4];
	for (i=0, totalpix=1;i<4;i++)
	{
		out_sz_warped[i] = inimg_sz[i];
		totalpix *= out_sz_warped[i];
	}

	outimg_warped = new T [totalpix];
	if (!outimg_warped)
	{
		if (out_sz_warped) {delete []out_sz_warped; out_sz_warped=0;}
		return false;
	}

    for (i=0;i<totalpix; i++) outimg_warped[i] = inimg_subject[i];

	if (!applyDFtoChannel(outimg_warped, out_sz_warped, -1,  
		spos_subject, cur_df, bni))
	{
		fprintf(stderr, "Fail to warp using the DF to the specified channel(s).\n");
	}
	else
	{
		printf("Succeed to warp using the DF to the specified channel(s).\n");
	}

	return true;
}


template <class T> bool Warp3D::getWarpedData(T * &outimg_warped, V3DLONG * &out_sz_warped, const int datatype_warped)
{
    if (outimg_warped || out_sz_warped)
	{
		fprintf(stderr, "The handle of warped data is not initilized as empty! Exit without copying data!\n");
		return false; 
	}

	V3DLONG i,j,k;

	out_sz_warped = new V3DLONG [4];
	out_sz_warped[0] = img_warped->sz0();
	out_sz_warped[1] = img_warped->sz1();
	out_sz_warped[2] = img_warped->sz2();
	out_sz_warped[3] = 1; //This is a very important sentence!!

	outimg_warped = new T [img_warped->getTotalElementNumber()];
	if (!outimg_warped)
	{
		if (out_sz_warped) {delete []out_sz_warped; out_sz_warped=0;}
		return false;
	}

	MYFLOAT_JBA * img_warped_ref1d = img_warped->getData1dHandle();


	double tmp;
	switch (datatype_warped)
	{
	    case 1:
			for (i=0;i<img_warped->getTotalElementNumber();i++)
			{
			    tmp = double(img_warped_ref1d[i])*255.0+0.5;
				tmp = (tmp>255)?255:tmp;
				tmp = (tmp<0)?0:tmp;
				outimg_warped[i] = (T)tmp; 
            }
			break;

	    case 2:
			for (i=0;i<img_warped->getTotalElementNumber();i++)
			{
			    tmp = double(img_warped_ref1d[i])*1023.0+0.5;
				tmp = (tmp>1023)?1023:tmp;
				tmp = (tmp<0)?0:tmp;
				outimg_warped[i] = (T)(img_warped_ref1d[i]*1023+0.5); 
			}
			break;

	    default:  //4
			for (i=0;i<img_warped->getTotalElementNumber();i++)
				outimg_warped[i] = (T)(img_warped_ref1d[i]); 
			break;
	}

	return true;
}

bool saveVol3DSimple2RawFloat(Vol3DSimple<unsigned char> *img, string basefilename, int layer, string otherinfo)
{
    if (!img || !img->valid() || basefilename.length()==0)
	{
		fprintf(stderr, "Invalid parameter to saveVol3DSimple2RawFloat(). Do nothing. \n");
		cout << "[" << basefilename << "]" << endl;
		return false;
    }

	V3DLONG sz[4];
	sz[0] = img->sz0(); sz[1] = img->sz1(); sz[2] = img->sz2(); sz[3] = 1;

	unsigned char * outimg = img->getData1dHandle();

	string	outfilename = gen_file_name(basefilename, layer, otherinfo);
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 1)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    return true;
}


bool saveVol3DSimple2RawFloat(Vol3DSimple<MYFLOAT_JBA> *img, string basefilename, int layer, string otherinfo)
{
    if (!img || !img->valid() || basefilename.length()==0)
	{
		fprintf(stderr, "Invalid parameter to saveVol3DSimple2RawFloat(). Do nothing. \n");
		cout << "[" << basefilename << "]" << endl;
		return false;
    }

	V3DLONG sz[4];
	sz[0] = img->sz0(); sz[1] = img->sz1(); sz[2] = img->sz2(); sz[3] = 1;

	MYFLOAT_JBA * outimg = img->getData1dHandle();

	string	outfilename = gen_file_name(basefilename, layer, otherinfo);
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    return true;
}



bool saveVol3DSimple2RawFloat(Vol3DSimple<DisplaceFieldF3D> *p, string basefilename, int layer, string otherinfo)
{
    if (!p || !p->valid() || basefilename.length()==0)
	{
	    fprintf(stderr, "Invalid parameter to saveVol3DSimple2RawFloat(). Do nothing. \n");
		cout << "[" << basefilename << "]" << endl;
		return false;
    }

	V3DLONG sz[4];
	sz[0] = p->sz0(); sz[1] = p->sz1(); sz[2] = p->sz2(); sz[3] = 1;

    MYFLOAT_JBA *outimg = new MYFLOAT_JBA [p->getTotalElementNumber()];
	if (!outimg)
	{
	    fprintf(stderr, "Fail to allocate temp memory in saveVol3DSimple2RawFloat(). \n");
		return false;
	}

	DisplaceFieldF3D * p_ref = p->getData1dHandle();

    V3DLONG i;

    string outfilename;

    //for x

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sx;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sx");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for y

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sy;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sy");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for z

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sz;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sz");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

	if (outimg) {delete outimg; outimg=0;}
	return true;
}

string gen_file_name(string basename, int layer, string otherinfo)
{
    string a = basename + "_";
	char layerstr[64]; for (int i=0;i<64;i++) layerstr[i] = '\0'; sprintf(layerstr, "%d", layer);
	a = a + layerstr + "_" + otherinfo + ".v3draw";
	return a;
}

bool saveVol3DSimple2RawFloat(Vol3DSimple<WarpParameterAffine3D> *p, string basefilename, int layer, string otherinfo)
{
    if (!p || !p->valid() || basefilename.length()==0)
	{
	    fprintf(stderr, "Invalid parameter to saveVol3DSimple2RawFloat(). Do nothing. \n");
		cout << "[" << basefilename << "]" << endl;
		return false;
    }

	V3DLONG sz[4];
	sz[0] = p->sz0(); sz[1] = p->sz1(); sz[2] = p->sz2(); sz[3] = 1;

    MYFLOAT_JBA *outimg = new MYFLOAT_JBA [p->getTotalElementNumber()];
	if (!outimg)
	{
	    fprintf(stderr, "Fail to allocate temp memory in saveVol3DSimple2RawFloat(). \n");
		return false;
	}

	WarpParameterAffine3D * p_ref = p->getData1dHandle();

    V3DLONG i;

    string outfilename;

    //for x

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sx;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sx");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for y

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sy;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sy");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for z

	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sz;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sz");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mxx
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mxx;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mxx");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mxy
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mxy;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mxy");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mxz
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mxz;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mxz");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for myx
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].myx;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_myx");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for myy
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].myy;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_myy");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for myz
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].myz;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_myz");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mzx
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mzx;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mzx");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mzy
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mzy;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mzy");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for mzz
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].mzz;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_mzz");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for si
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].si;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_si");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //for sb
	for (i=0;i<p->getTotalElementNumber();i++) outimg[i] = p_ref[i].sb;
	outfilename = gen_file_name(basefilename, layer, otherinfo + "_sb");
    if (saveImage(outfilename.c_str(), (const unsigned char *)outimg, sz, 4)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

    //exit

	if (outimg) {delete outimg; outimg=0;}
	return true;
}

bool saveVol3DSimple2RawUint8(Vol3DSimple<MYFLOAT_JBA> *img, string basefilename, int layer, string otherinfo, double multiplier=1.0)
{
    if (!img || !img->valid() || basefilename.length()==0)
	{
		fprintf(stderr, "Invalid parameter to saveVol3DSimple2RawFloat(). Do nothing. \n");
		cout << "[" << basefilename << "]" << endl;
		return false;
    }

	V3DLONG sz[4];
	sz[0] = img->sz0(); sz[1] = img->sz1(); sz[2] = img->sz2(); sz[3] = 1;

	MYFLOAT_JBA * outimg = img->getData1dHandle();

	Vol3DSimple<UINT8_JBA> *img_new = new Vol3DSimple<UINT8_JBA> (sz[0], sz[1], sz[2]);
	if (!img_new || !img_new->valid())
	{
		fprintf(stderr, "Fail to allocate memory in saveVol3DSimple2RawUint8(). File NOT save.\n");
		if (img_new) {delete img_new; img_new=0;}
		return false;
	}

    V3DLONG i;
	UINT8_JBA * img_new_ref = img_new->getData1dHandle();
	if (multiplier!=1.0)
	{
		for (i=0;i<img_new->getTotalElementNumber(); i++)
		{
			img_new_ref[i] = (UINT8_JBA)(outimg[i]*multiplier); 
		}
	}
	else
	{
		for (i=0;i<img_new->getTotalElementNumber(); i++)
		{
			img_new_ref[i] = (UINT8_JBA)(outimg[i]); 
		}
	}

	string	outfilename = gen_file_name(basefilename, layer, otherinfo);
    if (saveImage(outfilename.c_str(), (const unsigned char *)img_new_ref, sz, 1)!=true)
	{
	    cout << "Fail to save to file " << outfilename <<endl;
		if (outimg) {delete outimg; outimg=0;}
		return false;
	}else { cout << "Succeed to save to file " << outfilename <<endl;  }

	if (img_new) {delete img_new; img_new=0;}
    return true;
}


template <class T> bool find3DGradient(Vol3DSimple <T> * & g, Vol3DSimple <T> *subject)
{
	if (!subject || !subject->valid())
	{
		fprintf(stderr, "The data pointer is emtpty in findGradient().\n");
		return false;
    }

	V3DLONG d0=subject->sz0(), d1=subject->sz1(), d2=subject->sz2();

	if (g && (!g->valid() || !isSameSize(g, subject)))
	{
	    delete g;
		g = 0;
		g = new Vol3DSimple <T> (d0, d1, d2);
		if (!g || !g->valid())
		{
		   fprintf(stderr, "Fail to allocate memory in findGradient(), line %d.\n", __LINE__);
		   if (g) {delete g; g=0;}
		   return false;
		}
	}

	T * tmp1d = new T [d0*d1*d2];
	if (!tmp1d)
	{
		fprintf(stderr, "Fail to allocate memory [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T ***tmp3d = 0;
	if (new3dpointer(tmp3d, d0, d1, d2, tmp1d)==false)
	{
		if (tmp1d) {delete []tmp1d; tmp1d=0;}
		return false;
	}

#define LEN_KERNEL 5

	MYFLOAT_JBA kernel_p_xyz[LEN_KERNEL]   = {0.036420, 0.248972, 0.429217, 0.248972, 0.036420};
	MYFLOAT_JBA kernel_d_xyz_m[LEN_KERNEL] = {0.108415, 0.280353, 0.0, -0.280353, -0.108415};
#ifndef POSITIVE_Y_COORDINATE
	MYFLOAT_JBA kernel_d_xyz[LEN_KERNEL]   = {-0.108415, -0.280353, 0.0, 0.280353, 0.108415}; 
#else
	MYFLOAT_JBA kernel_d_xyz[LEN_KERNEL]   = {0.108415, 0.280353, 0.0, -0.280353, -0.108415}; 
#endif

	T *** indata3d = subject->getData3dHandle();
	T *** g3d = g->getData3dHandle();
	vol3d_assign(g3d, (T)0, d0, d1, d2);

	conv3d_1dvec(tmp3d, indata3d, d0, d1, d2, kernel_d_xyz_m, LEN_KERNEL, 1);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 2);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 3);
    vol3d_square(tmp3d, tmp3d,  d0, d1, d2);
	vol3d_plus(g3d, g3d, tmp3d, d0, d1, d2);

	conv3d_1dvec(tmp3d, indata3d, d0, d1, d2, kernel_d_xyz, LEN_KERNEL, 2);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 3);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 1);
    vol3d_square(tmp3d, tmp3d,  d0, d1, d2);
	vol3d_plus(g3d, g3d, tmp3d, d0, d1, d2);

	conv3d_1dvec(tmp3d, indata3d, d0, d1, d2, kernel_d_xyz_m, LEN_KERNEL, 3);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 1);
	conv3d_1dvec(tmp3d, tmp3d,  d0, d1, d2, kernel_p_xyz, LEN_KERNEL, 2);
    vol3d_square(tmp3d, tmp3d,  d0, d1, d2);
	vol3d_plus(g3d, g3d, tmp3d, d0, d1, d2);

    vol3d_root(g3d, g3d,  d0, d1, d2);

	if (tmp3d) delete3dpointer(tmp3d, d0, d1, d2);
	if (tmp1d) {delete []tmp1d; tmp1d=0;}

	return true;
}

bool est_best_scale_3d(WarpParameterAffine3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable,
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft,
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable) 
{
	if (!wp ||
	    !dfx || !dfx->valid() ||
		!dfy || !dfy->valid() ||
		!dfz || !dfz->valid() ||
		!dft || !dft->valid() ||
		!subject || !subject->valid() )
	{
	    fprintf(stderr, "Invalid internal data pointers [%s][%d].\n", __FILE__, __LINE__);
	    return false;
	}

	V3DLONG tsz0 = subject->sz0(), tsz1 = subject->sz1(), tsz2 = subject->sz2();

	double xc = (double(xe) + double(xs))/2.0;
	double yc = (double(ye) + double(ys))/2.0;
	double zc = (double(ze) + double(zs))/2.0;
    if (xs>xe ||
		ys>ye ||
		zs>ze ||
		xc < 0 || xc > (tsz0-1) ||
		yc < 0 || yc > (tsz1-1) ||
		zc < 0 || zc > (tsz2-1) )
	{
	    fprintf(stderr, "Invalid cube coordinate parameters [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (dTable)
	{
	    fprintf(stderr, "The dTable is not initilized as empty! Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	MYFLOAT_JBA ***	dfx3d = dfx->getData3dHandle();
	MYFLOAT_JBA ***	dfy3d = dfy->getData3dHandle();
	MYFLOAT_JBA ***	dfz3d = dfz->getData3dHandle();
	MYFLOAT_JBA ***	dft3d = dft->getData3dHandle();

	V3DLONG i,j,k;

	V3DLONG is = hardLimit(xs, V3DLONG(0), tsz0-1), ie = hardLimit(xe, V3DLONG(0), tsz0-1);
	V3DLONG js = hardLimit(ys, V3DLONG(0), tsz1-1), je = hardLimit(ye, V3DLONG(0), tsz1-1);
	V3DLONG ks = hardLimit(zs, V3DLONG(0), tsz2-1), ke = hardLimit(ze, V3DLONG(0), tsz2-1);

	V3DLONG xlen = ie-is+1, ylen = je-js+1, zlen = ke-ks+1;
	V3DLONG cubelen = (V3DLONG)xlen*ylen*zlen;

	const V3DLONG myCUBECOLUMNNUM=4;

	Image2DSimple<MYFLOAT_JBA> *curDTable = new Image2DSimple<MYFLOAT_JBA> (cubelen, myCUBECOLUMNNUM);
	if (!curDTable || !curDTable->valid())
	{
	    fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	MYFLOAT_JBA **  cubeData2d = curDTable->getData2dHandle();

	V3DLONG curind=0;
	for (k=ks;k<=ke;k++)
	{
		double mk = -(k-zc);
		for (j=js;j<=je;j++)
		{
#ifndef POSITIVE_Y_COORDINATE	
			double mj = -(j-yc);
#else
			double mj = (j-yc); 
#endif
			for (i=is;i<=ie;i++)
			{
				double mi = -(i-xc);

				cubeData2d[0][curind] = dfx3d[k][j][i] * mi;
				cubeData2d[1][curind] = dfy3d[k][j][i] * mj;
				cubeData2d[2][curind] = dfz3d[k][j][i] * mk;

				cubeData2d[3][curind] = dft3d[k][j][i] + cubeData2d[0][curind] + cubeData2d[1][curind] + cubeData2d[3][curind];

				curind++;
			}
		}
	}

	Matrix P_Matrix(myCUBECOLUMNNUM-1, myCUBECOLUMNNUM-1);
	Matrix K_Matrix(myCUBECOLUMNNUM-1,1);
	double tmpp=0.0, tmpk=0.0;

	for (k=0;k<myCUBECOLUMNNUM-1;k++)
	{
		for (j=k;j<myCUBECOLUMNNUM-1;j++)
		{
			for (i=0, tmpp=0.0;i<cubelen;i++)
				tmpp += cubeData2d[k][i] * cubeData2d[j][i];
			P_Matrix(k+1,j+1) = tmpp;
			P_Matrix(j+1,k+1) = tmpp;
		}

		for (i=0, tmpk=0.0;i<cubelen;i++)
		{
			tmpk += cubeData2d[k][i] * (cubeData2d[3][i]);
		}
		K_Matrix(k+1,1) = tmpk;
	}

//	if (b_VERBOSE_PRINT)
//	{
//		cout << "P_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << P_Matrix << endl <<endl;

//		cout << "K_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << K_Matrix << endl <<endl;
//	}

	Matrix Y;
	Try
	{
		Y = P_Matrix.i() * K_Matrix;

		wp->mxx = Y(1,1);
		wp->myy = Y(2,1);
		wp->mzz = Y(3,1);

		wp->b_transform=1;
	}
	CatchAll
	{
		wp->resetToDefault();
	}

//	if (b_VERBOSE_PRINT)
//	{
//		Matrix Y1(4,4);
//		Y1.row(1) << wp->mxx << wp->mxy << wp->mxz << wp->sx;
//		Y1.row(2) << wp->myx << wp->myy << wp->myz << wp->sy;
//		Y1.row(3) << wp->mzx << wp->mzy << wp->mzz << wp->sz;
//		Y1.row(4) << 0 << 0 << 0 << 1;
//		cout << "Transform matrix" << endl;
//		cout << setw(12) << setprecision(3) << Y1 << endl;
//	}

	if (b_returnDTable==true)
	{
	    dTable = curDTable; 
	}
	else
	{
		if (curDTable) delete curDTable; curDTable=0;
	}
	return true;
}


bool ebs3d(DisplaceFieldF3D *wp, Image2DSimple<MYFLOAT_JBA> * & dTable,
                       Vol3DSimple<MYFLOAT_JBA> * dfx, Vol3DSimple<MYFLOAT_JBA> * dfy, Vol3DSimple<MYFLOAT_JBA> * dfz, Vol3DSimple<MYFLOAT_JBA> * dft,
	  				   Vol3DSimple<MYFLOAT_JBA> * subject,
                       V3DLONG xs, V3DLONG xe, V3DLONG ys, V3DLONG ye, V3DLONG zs, V3DLONG ze,
	  				   Vol3DSimple<UINT8_JBA> * mask,
					   bool b_returnDTable) 
{
	if (!wp ||
	    !dfx || !dfx->valid() ||
		!dfy || !dfy->valid() ||
		!dfz || !dfz->valid() ||
		!dft || !dft->valid() ||
		!subject || !subject->valid() )
	{
	    fprintf(stderr, "Invalid internal data pointers [%s][%d].\n", __FILE__, __LINE__);
	    return false;
	}

	bool b_usemask;
	if (!mask || !mask->valid()) b_usemask=false; else b_usemask=true; 
    
	V3DLONG tsz0 = subject->sz0(), tsz1 = subject->sz1(), tsz2 = subject->sz2();

	double xc = (double(xe) + double(xs))/2.0;
	double yc = (double(ye) + double(ys))/2.0;
	double zc = (double(ze) + double(zs))/2.0;
    if (xs>xe ||
		ys>ye ||
		zs>ze ||
		xc < 0 || xc > (tsz0-1) ||
		yc < 0 || yc > (tsz1-1) ||
		zc < 0 || zc > (tsz2-1) )
	{
	    fprintf(stderr, "Invalid cube coordinate parameters [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (dTable)
	{
	    fprintf(stderr, "The dTable is not initilized as empty! Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	MYFLOAT_JBA ***	dfx3d = dfx->getData3dHandle();
	MYFLOAT_JBA ***	dfy3d = dfy->getData3dHandle();
	MYFLOAT_JBA ***	dfz3d = dfz->getData3dHandle();
	MYFLOAT_JBA ***	dft3d = dft->getData3dHandle();

	V3DLONG i,j,k;

	V3DLONG is = hardLimit(xs, V3DLONG(0), tsz0-1), ie = hardLimit(xe, V3DLONG(0), tsz0-1);
	V3DLONG js = hardLimit(ys, V3DLONG(0), tsz1-1), je = hardLimit(ye, V3DLONG(0), tsz1-1);
	V3DLONG ks = hardLimit(zs, V3DLONG(0), tsz2-1), ke = hardLimit(ze, V3DLONG(0), tsz2-1);

	V3DLONG xlen = ie-is+1, ylen = je-js+1, zlen = ke-ks+1;
	V3DLONG cubelen = (V3DLONG)xlen*ylen*zlen;

    const V3DLONG myCUBECOLUMNNUM = 4;
	Image2DSimple<MYFLOAT_JBA> *curDTable = new Image2DSimple<MYFLOAT_JBA> (cubelen, myCUBECOLUMNNUM);
	if (!curDTable || !curDTable->valid())
	{
	    fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}
	MYFLOAT_JBA **  cubeData2d = curDTable->getData2dHandle();

	V3DLONG curind=0;
	if (b_usemask==false)
	{
		for (k=ks;k<=ke;k++)
		{
			for (j=js;j<=je;j++)
			{
				for (i=is;i<=ie;i++)
				{
					cubeData2d[0][curind] = dfx3d[k][j][i];
					cubeData2d[1][curind] = dfy3d[k][j][i];
					cubeData2d[2][curind] = dfz3d[k][j][i];

					cubeData2d[3][curind] = dft3d[k][j][i];

					curind++;
				}
			}
		}
	}
	else
	{
		UINT8_JBA *** mask_ref = mask->getData3dHandle();
		for (k=ks;k<=ke;k++)
		{
			for (j=js;j<=je;j++)
			{
				for (i=is;i<=ie;i++)
				{
					if (mask_ref[k][j][i]==0)
						continue;

					cubeData2d[0][curind] = dfx3d[k][j][i];
					cubeData2d[1][curind] = dfy3d[k][j][i];
					cubeData2d[2][curind] = dfz3d[k][j][i];

					cubeData2d[3][curind] = dft3d[k][j][i];

					curind++;
				}
			}
		}
	}

	if (curind<100) 
	{
		wp->sx = wp->sy = wp->sz = 0;
		if (curDTable) delete curDTable; curDTable=0;
		return true;
	}

	Matrix P_Matrix(myCUBECOLUMNNUM-1, myCUBECOLUMNNUM-1);
	Matrix K_Matrix(myCUBECOLUMNNUM-1, 1);
	double tmpp=0.0, tmpk=0.0;

	for (k=0;k<myCUBECOLUMNNUM-1;k++)
	{
		for (j=k;j<myCUBECOLUMNNUM-1;j++)
		{
			for (i=0, tmpp=0.0;i<cubelen;i++)
				tmpp += cubeData2d[k][i] * cubeData2d[j][i];
			P_Matrix(k+1,j+1) = tmpp;
			P_Matrix(j+1,k+1) = tmpp;
		}

		for (i=0, tmpk=0.0;i<cubelen;i++)
		{
			tmpk += cubeData2d[k][i] * (cubeData2d[3][i]);
		}
		K_Matrix(k+1,1) = tmpk;
	}

//	if (b_VERBOSE_PRINT)
//	{
//		cout << "P_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << P_Matrix << endl <<endl;

//		cout << "K_Matrix" << endl;
//		cout << setw(12) << setprecision(3) << K_Matrix << endl <<endl;
//	}

	Matrix Y;
	Try
	{
		Y = P_Matrix.i() * K_Matrix;

		wp->sx = Y(1,1); wp->sy = Y(2,1); wp->sz = Y(3,1);

		wp->b_transform=1;

        if (0)
		{
			cout << "Inverse of P matrix" << endl;
			cout << setw(15) << setprecision(8) << P_Matrix.i() << endl;
		}
	}
	CatchAll
	{
		wp->sx = wp->sy = wp->sz = 0;
	}

	if (b_returnDTable==true)
	{
	    dTable = curDTable; 
	}
	else
	{
		if (curDTable) delete curDTable; curDTable=0;
	}

	return true;
}


Vol3DSimple<DisplaceFieldF3D> * Warp3D::do_local_topdown_blocktps_fast(const BasicWarpParameter & bwp)
{
	bool b_save_intermediate=false;
    bool b_noerror=true;
	int b_useAffine=0;
	int b_useOverlappingGrid=0;
	int b_useDownsampling=0; 
	int b_use_MINBLKSZ=0;
	int b_filterUnnecessaryCPoints=1; 
	bool b_nearest_interp = bwp.b_nearest_interp; 

    if (!img_warped || !img_warped->valid() || !img_target || !img_target->valid() || !img_subject || !img_subject->valid() ||
	    !isSameSize(img_warped, img_target) || !isSameSize(img_warped, img_subject))
	{return 0;}

	Vol3DSimple<MYFLOAT_JBA> * gmap_target = 0, * gmap_subject = 0;

	V3DLONG dsz0=img_subject->sz0(), dsz1=img_subject->sz1(), dsz2=img_subject->sz2();

	gmap_target = new Vol3DSimple <MYFLOAT_JBA> (dsz0, dsz1, dsz2); 
	gmap_subject = new Vol3DSimple <MYFLOAT_JBA> (dsz0, dsz1, dsz2);
	if (!gmap_target || !gmap_target->valid() || !gmap_subject || !gmap_subject->valid())
	{
	  fprintf(stderr, "Fail to allocate memory for in do_local_topdown_blocktps() for the gradient maps. line %d.\n", __LINE__);
	  if (gmap_target) {delete gmap_target; gmap_target=0;}
	  if (gmap_subject) {delete gmap_subject; gmap_subject=0;}
	  return false;
	}

	bool b_set_to_binary=false;

	MYFLOAT_JBA imgMean_target, imgStd_target;
	find3DGradient(gmap_target, img_target);
	mean_and_std(img_target->getData1dHandle(), img_target->getTotalElementNumber(), imgMean_target, imgStd_target); 
	mean_and_std(gmap_target->getData1dHandle(), gmap_target->getTotalElementNumber(), imgMean_target, imgStd_target, imgMean_target+imgStd_target, (MYFLOAT_JBA)1); 
	vol3d_threshold(gmap_target->getData3dHandle(), dsz0, dsz1, dsz2, imgMean_target+2*imgStd_target, b_set_to_binary);

	vol3d_time_constant(gmap_target->getData3dHandle(), gmap_target->getData3dHandle(), 255.0, dsz0, dsz1, dsz2);

	MYFLOAT_JBA imgMean_subject, imgStd_subject;
	find3DGradient(gmap_subject, img_subject);
	mean_and_std(img_subject->getData1dHandle(), img_subject->getTotalElementNumber(), imgMean_subject, imgStd_subject); 
	mean_and_std(gmap_subject->getData1dHandle(), gmap_subject->getTotalElementNumber(), imgMean_subject, imgStd_subject, imgMean_subject+imgStd_subject, (MYFLOAT_JBA)1); 
	vol3d_threshold(gmap_subject->getData3dHandle(), dsz0, dsz1, dsz2, imgMean_subject+2*imgStd_subject, b_set_to_binary);

	vol3d_time_constant(gmap_subject->getData3dHandle(), gmap_subject->getData3dHandle(), 255.0, dsz0, dsz1, dsz2);

	vol3d_assign(img_warped->getData3dHandle(), img_subject->getData3dHandle(), img_subject->sz0(), img_subject->sz1(), img_subject->sz2());

	const V3DLONG MIN_LOCALBLK_XWID = 48, MIN_LOCALBLK_YWID = 48, MIN_LOCALBLK_ZWID = 24;
    
    Vol3DSimple<DisplaceFieldF3D> * df_local = 0;

	WarpParameterAffine3D cur_wp;
	DisplaceFieldF3D cur_df;
	V3DLONG layer, i, j, k;

	int NLayer=3;
	const int N_refinetps = 1; int n_refinetps;

	const double thres_nomove=1e-2; 
    
	for (layer=NLayer-1; layer>=0; layer--)
	{
		double cur_dfactor = (b_useDownsampling) ? pow(2, layer) : 1; 

		Vol3DSimple<MYFLOAT_JBA> * targetSmall = 0, *warpedSmall = 0, *subjectSmall = 0;
		Vol3DSimple<unsigned char> * maskSmall = 0; 

		vector<Coord3D_JBA> matchTargetPos, matchSubjectPos;
		Coord3D_JBA cur_matching_target_pt, cur_matching_subject_pt;
		Coord3D_JBA corner;

		Vol3DSimple <DisplaceFieldF3D> * df_array=0; 
		Vol3DSimple <UINT8_JBA> * df_array_flag=0; 

		V3DLONG KGRID=(V3DLONG)(pow(2,NLayer-layer)), JGRID=(V3DLONG)(2*pow(2,NLayer-layer)), IGRID=(V3DLONG)(2*pow(2,NLayer-layer));

		V3DLONG sz0, sz1, sz2;

		if (!downsample3dvol(targetSmall, img_target, cur_dfactor) ||
			!downsample3dvol(subjectSmall, img_subject, cur_dfactor) ||
			!downsample3dvol(warpedSmall, img_warped, cur_dfactor))
		{
			b_noerror = false;
			goto Label_exit;
		}

		sz0 = warpedSmall->sz0();
		sz1 = warpedSmall->sz1();
		sz2 = warpedSmall->sz2();

		vol3d_assign(warpedSmall->getData3dHandle(), subjectSmall->getData3dHandle(), sz0, sz1, sz2);
		if (layer!=NLayer-1)
		{
			if (df_warp(warpedSmall, df_local, bwp.b_nearest_interp)==false)
			{
				fprintf(stderr, "Fail to warp using the displacement parameter found in the last coarser level.\n");
				b_noerror=false; goto Label_exit;
			}
		}

		V3DLONG KGRID2, JGRID2, IGRID2;
		if (b_useOverlappingGrid) {KGRID2=2*KGRID-1; JGRID2=2*JGRID-1; IGRID2=2*IGRID-1;}
		else {KGRID2=KGRID; JGRID2=JGRID; IGRID2=IGRID;}

		df_array = new Vol3DSimple <DisplaceFieldF3D> (IGRID2, JGRID2, KGRID2); 
		df_array_flag = new Vol3DSimple <UINT8_JBA> (IGRID2, JGRID2, KGRID2); 
		if (!df_array || !df_array->valid() || !df_array_flag || !df_array_flag->valid())
		{
			fprintf(stderr, "Fail to allocate memory  for df_array.\n");
			b_noerror=false; goto Label_exit;
		}


		for (n_refinetps=0; n_refinetps<N_refinetps;n_refinetps++) 
		{
			if (matchTargetPos.size()>0)
				matchTargetPos.erase(matchTargetPos.begin(), matchTargetPos.end()); 
			if (matchSubjectPos.size()>0)
				matchSubjectPos.erase(matchSubjectPos.begin(), matchSubjectPos.end()); 

			for (k=0;k<KGRID2;k++)
			{
				V3DLONG kb, ke;
				if (b_useOverlappingGrid) {kb=V3DLONG(floor(k*double(sz2)/KGRID/2)); ke=V3DLONG(floor((k+2)*double(sz2)/KGRID/2))-1;} 
				else {kb=V3DLONG(floor(k*double(sz2)/KGRID)); ke=V3DLONG(floor((k+1)*double(sz2)/KGRID))-1;}

				double cur_center_z = (double(kb)+ke)/2.0;
				if (b_use_MINBLKSZ)
				{
					if (cur_center_z - MIN_LOCALBLK_ZWID/2 < kb) {kb=V3DLONG(floor(cur_center_z - MIN_LOCALBLK_ZWID/2)); if (kb<0) kb=0;}
					if (cur_center_z + MIN_LOCALBLK_ZWID/2 > ke) {ke=V3DLONG(ceil(cur_center_z + MIN_LOCALBLK_ZWID/2)); if (ke>=sz2) ke=sz2-1;}
				}

				for (j=0;j<JGRID2;j++)
				{
					V3DLONG jb,je;
					if (b_useOverlappingGrid) {jb=V3DLONG(floor(j*double(sz1)/JGRID/2)); je=V3DLONG(floor((j+2)*double(sz1)/JGRID/2))-1;}
					else {jb=V3DLONG(floor(j*double(sz1)/JGRID)); je=V3DLONG(floor((j+1)*double(sz1)/JGRID))-1;}

					double cur_center_y = (double(jb)+je)/2.0;
					if (b_use_MINBLKSZ)
					{
						if (cur_center_y - MIN_LOCALBLK_YWID/2 < jb) {jb=V3DLONG(floor(cur_center_y - MIN_LOCALBLK_YWID/2)); if (jb<0) jb=0;}
						if (cur_center_y + MIN_LOCALBLK_YWID/2 > je) {je=V3DLONG(ceil(cur_center_y + MIN_LOCALBLK_YWID/2)); if (je>=sz1) je=sz1-1;}
					}

					for (i=0;i<IGRID2;i++)
					{
						V3DLONG ib,ie;
						if (b_useOverlappingGrid) {ib=V3DLONG(floor(i*double(sz0)/IGRID/2)); ie=V3DLONG(floor((i+2)*double(sz0)/IGRID/2))-1;}
						else {ib=V3DLONG(floor(i*double(sz0)/IGRID)); ie=V3DLONG(floor((i+1)*double(sz0)/IGRID))-1;}
					
                        double cur_center_x = (double(ib)+ie)/2.0;
						if (b_use_MINBLKSZ)
						{
							if (cur_center_x - MIN_LOCALBLK_XWID/2 < ib) {ib=V3DLONG(floor(cur_center_x - MIN_LOCALBLK_XWID/2)); if (ib<0) ib=0;}
							if (cur_center_x + MIN_LOCALBLK_XWID/2 > ie) {ie=V3DLONG(ceil(cur_center_x + MIN_LOCALBLK_XWID/2)); if (ie>=sz0) ie=sz0-1;}
						}

						Vol3DSimple <MYFLOAT_JBA> curSubjectImgBlock(warpedSmall, ib, ie, jb, je, kb, ke); 
						Vol3DSimple <MYFLOAT_JBA> curTargetImgBlock(targetSmall, ib, ie, jb, je, kb, ke);

						printf("shift est for blk=[%ld,%ld,%ld], krange=[%ld,%ld], jrange=[%ld,%ld], irange=[%ld,%ld]. ", i,j,k, kb, ke, jb, je, ib, ie); fflush(stdout);

						
                        V3DLONG nnz_targetgmap_blk, nnz_subjectgmap_blk;
						if(!vol3d_nnz(nnz_targetgmap_blk, gmap_target->getData3dHandle(), dsz0, dsz1, dsz2, ib, ie, jb, je, kb, ke) ||
						   !vol3d_nnz(nnz_subjectgmap_blk, gmap_subject->getData3dHandle(), dsz0, dsz1, dsz2, ib, ie, jb, je, kb, ke))
						{
							fprintf(stderr,"This should never be called. line %d\n", __LINE__);
						}
						
                        printf("nnz(target_edgemap)=%ld, nnz(subject_edgemap)=%ld. ", nnz_targetgmap_blk, nnz_subjectgmap_blk);

						
                        if ( NLayer-layer>=3 && (nnz_targetgmap_blk<100) )
						{
							cur_df.sx = cur_df.sy = cur_df.sz = 0.0; 
						}
						else
						{
							if (b_useAffine)
							{
								eba3d_interface(cur_wp, &curTargetImgBlock, &curSubjectImgBlock, bwp.nloop_localest);
								cur_df.sx = cur_wp.sx; cur_df.sy = cur_wp.sy; cur_df.sz = cur_wp.sz;
							}
							else
							{
								ebs3d_interface(cur_df, &curTargetImgBlock, &curSubjectImgBlock, bwp.nloop_localest);
							}
						}

						df_array->getData3dHandle()[k][j][i].copy(cur_df);
						if (fabs(cur_df.sx)<thres_nomove && fabs(cur_df.sy)<thres_nomove && fabs(cur_df.sz)<thres_nomove)
						{
							df_array_flag->getData3dHandle()[k][j][i] = 0;
						}
						else
						{
							df_array_flag->getData3dHandle()[k][j][i] = 1;
						}

						cur_matching_target_pt.x = cur_center_x;
						cur_matching_target_pt.y = cur_center_y;
						cur_matching_target_pt.z = cur_center_z;

						cur_matching_subject_pt.x = cur_matching_target_pt.x + cur_df.sx;
						cur_matching_subject_pt.y = cur_matching_target_pt.y + cur_df.sy;
						cur_matching_subject_pt.z = cur_matching_target_pt.z + cur_df.sz;

						printf("target=[%5.3f][%5.3f][%5.3f] -> subject=[%5.3f][%5.3f][%5.3f]\n",
							   cur_matching_target_pt.x, cur_matching_target_pt.y, cur_matching_target_pt.z,
							   cur_matching_subject_pt.x, cur_matching_subject_pt.y, cur_matching_subject_pt.z);
					}
				}
			}
			printf("\n"); fflush(stdout);

			if (b_filterUnnecessaryCPoints)
			{
				DisplaceFieldF3D *** df_array_ref = df_array->getData3dHandle();
				UINT8_JBA *** df_array_flag_ref = df_array_flag->getData3dHandle();
				for (k=0;k<KGRID2;k++)
				{
					V3DLONG kbn, ken;
					kbn=k-1; if (kbn<0) kbn=0;
					ken=k+1; if (ken>=KGRID2) ken=KGRID2-1;

					V3DLONG kb, ke;
					if (b_useOverlappingGrid) {kb=V3DLONG(floor(k*double(sz2)/KGRID/2)); ke=V3DLONG(floor((k+2)*double(sz2)/KGRID/2))-1;} //add the overlapping blocks
					else {kb=V3DLONG(floor(k*double(sz2)/KGRID)); ke=V3DLONG(floor((k+1)*double(sz2)/KGRID))-1;}

					for (j=0;j<JGRID2;j++)
					{
						V3DLONG jbn,jen;
						jbn=j-1; if (jbn<0) jbn=0;
						jen=j+1; if (jen>=JGRID2) jen=JGRID2-1;

						V3DLONG jb,je;
						if (b_useOverlappingGrid) {jb=V3DLONG(floor(j*double(sz1)/JGRID/2)); je=V3DLONG(floor((j+2)*double(sz1)/JGRID/2))-1;}
						else {jb=V3DLONG(floor(j*double(sz1)/JGRID)); je=V3DLONG(floor((j+1)*double(sz1)/JGRID))-1;}

						for (i=0;i<IGRID2;i++)
						{
							V3DLONG ibn,ien;
							ibn=i-1; if (ibn<0) ibn=0;
							ien=i+1; if (ien>=IGRID2) ien=IGRID2-1;

							V3DLONG ib,ie;
							if (b_useOverlappingGrid) {ib=V3DLONG(floor(i*double(sz0)/IGRID/2)); ie=V3DLONG(floor((i+2)*double(sz0)/IGRID/2))-1;}
							else {ib=V3DLONG(floor(i*double(sz0)/IGRID)); ie=V3DLONG(floor((i+1)*double(sz0)/IGRID))-1;}

							bool b_addThisCP=false;
							if (df_array_flag_ref[k][j][i]) 
							{
								b_addThisCP=true;
							}
							else
							{
								b_addThisCP=false;
								V3DLONG tmpi, tmpj, tmpk;
								for (tmpk=kbn;tmpk<=ken; tmpk++)
								{
								  for (tmpj=jbn;tmpj<=jen; tmpj++)
								  {
									for (tmpi=ibn; tmpi<=ien; tmpi++)
									{
										if (df_array_flag_ref[tmpk][tmpj][tmpi])
										{
											b_addThisCP=true; break;
										}
									}
									if (b_addThisCP) break;
								  }
								  if (b_addThisCP) break;
								}
							}

							if (b_addThisCP)
							{
								cur_matching_target_pt.x = (double(ib)+ie)/2.0;
								cur_matching_target_pt.y = (double(jb)+je)/2.0;
								cur_matching_target_pt.z = (double(kb)+ke)/2.0;

								cur_matching_subject_pt.x = (double(ib)+ie)/2.0 + df_array_ref[k][j][i].sx;
								cur_matching_subject_pt.y = (double(jb)+je)/2.0 + df_array_ref[k][j][i].sy;
								cur_matching_subject_pt.z = (double(kb)+ke)/2.0 + df_array_ref[k][j][i].sz;

								matchTargetPos.push_back(cur_matching_target_pt);
								matchSubjectPos.push_back(cur_matching_subject_pt);

								printf("added to TPS control points: \t target=[%5.3f][%5.3f][%5.3f] -> subject=[%5.3f][%5.3f][%5.3f]\n",
									   cur_matching_target_pt.x, cur_matching_target_pt.y, cur_matching_target_pt.z,
									   cur_matching_subject_pt.x, cur_matching_subject_pt.y, cur_matching_subject_pt.z);
							}

						}
					}
				}
				printf("\n"); fflush(stdout);
			}

			corner.x = 0; corner.y = 0; corner.z = 0;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = 0; corner.y = 0; corner.z = sz2-1;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = 0; corner.y = sz1-1; corner.z = 0;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = 0; corner.y = sz1-1; corner.z = sz2-1;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = sz0-1; corner.y = 0; corner.z = 0;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = sz0-1; corner.y = 0; corner.z = sz2-1;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = sz0-1; corner.y = sz1-1; corner.z = 0;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			corner.x = sz0-1; corner.y = sz1-1; corner.z = sz2-1;
			matchTargetPos.push_back(corner); matchSubjectPos.push_back(corner);

			Vol3DSimple <DisplaceFieldF3D>* df_local_curstep = 0;
			if (layer==NLayer-1)
				df_local = cdum(matchTargetPos, matchSubjectPos, sz0, sz1, sz2, bwp.method_df_compute);
			else
				df_local_curstep = cdum(matchTargetPos, matchSubjectPos, sz0, sz1, sz2, bwp.method_df_compute);

			if (df_local_curstep && df_local_curstep->valid())
			{
				df_add(df_local, df_local_curstep, b_nearest_interp); 
			}

			vol3d_assign(warpedSmall->getData3dHandle(), subjectSmall->getData3dHandle(), subjectSmall->sz0(), subjectSmall->sz1(), subjectSmall->sz2());
			df_warp(warpedSmall, df_local, b_nearest_interp);

			if (b_save_intermediate) 
			{
				saveVol3DSimple2RawFloat(df_local, file_warped, (layer+1)*100+n_refinetps, string("whole"));
				if (df_local_curstep && df_local_curstep->valid()) saveVol3DSimple2RawFloat(df_local_curstep, file_warped, (layer+1)*100+n_refinetps, string("step"));

				saveVol3DSimple2RawUint8(warpedSmall, file_warped, (layer+1)*100+n_refinetps, string("wp"), 255.0); //070530
			}

			if (df_local_curstep) {delete df_local_curstep; df_local_curstep=0;}
		}


Label_exit:

		if (df_array) {delete df_array; df_array=0;}
		if (df_array_flag) {delete df_array_flag; df_array_flag=0;}

		if (maskSmall) {delete maskSmall; maskSmall=0;}

		if (warpedSmall) {delete warpedSmall; warpedSmall=0;}
		if (subjectSmall) {delete subjectSmall; subjectSmall=0;}
		if (targetSmall) {delete targetSmall; targetSmall=0;}

		if (b_noerror==false)
		{
			if (df_local) {delete df_local; df_local=0;}
			break; 
		}

	}

	if (b_noerror==true)
	{
	}
	else
	{
		if (df_local) {delete df_local; df_local=0;}
	}

    if (gmap_target) {delete gmap_target; gmap_target=0;}
    if (gmap_subject) {delete gmap_subject; gmap_subject=0;}

	if (b_save_intermediate) 
	{
		saveVol3DSimple2RawFloat(df_local, file_warped, 999, string("DF")); 
	}
	return df_local;
}


bool assign_maskimg_val( Vol3DSimple <UINT8_JBA> * maskImg, Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg)
{
	if (!maskImg || !maskImg->valid() || !targetImg || !targetImg->valid() || !subjectImg || !subjectImg->valid())
	{
		fprintf(stderr, "Invalid parameters in assign_maskimg_val().\n");
		return false;
	}

	if (!isSameSize(maskImg, targetImg) || !isSameSize(maskImg, targetImg))
	{
		fprintf(stderr, "Unmatched sizes in assign_maskimg_val().\n");
		return false;
	}

	V3DLONG i,j,k;
	UINT8_JBA ***mask_ref = maskImg->getData3dHandle();
	MYFLOAT_JBA *** target_ref = targetImg->getData3dHandle();;
	MYFLOAT_JBA *** subject_ref = subjectImg->getData3dHandle();

	V3DLONG csz0=maskImg->sz0(), csz1=maskImg->sz1(), csz2=maskImg->sz2();
	V3DLONG nValidPix=0;
	for (k=0;k<csz2; k++)
	  for (j=0;j<csz1;j++)
	    for (i=0;i<csz0;i++)
		{
		  if (target_ref[k][j][i]>0.1 && subject_ref[k][j][i]>0.1) 
		  {
		    mask_ref[k][j][i] = 1;
			nValidPix++;
		  }
		  else
		  {
		  	mask_ref[k][j][i] = 0;
		  }
		}

	V3DLONG NValidPix=7*7*5;
	if (nValidPix<NValidPix)
	{
		printf("**{nValidPix=%ld. }", nValidPix);
		return false; 
	}
	else
	{
		printf("  {nValidPix=%ld. }", nValidPix);
	}

	bool b_use_std_thres=false; 
	if (b_use_std_thres)
	{
		MYFLOAT_JBA ave_target=0, std_target=0, ave_subject=0, std_subject=0;
		mean_and_std(targetImg->getData1dHandle(), targetImg->getTotalElementNumber(), ave_target, std_target);
		mean_and_std(subjectImg->getData1dHandle(), subjectImg->getTotalElementNumber(), ave_subject, std_subject);
		double STD_Thres=0.05;
		if (std_target<STD_Thres && std_subject<STD_Thres)
		{
			printf("**{std_target=%5.3f std_subject=%5.3f. }", std_target, std_subject);
			return false;
		}
		else
		{
			printf("  {std_target=%5.3f std_subject=%5.3f. }", std_target, std_subject);
		}
	}

	return true;
}


bool ebs3d_interface(DisplaceFieldF3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops)
{
	if (!targetImg || !targetImg->valid() || !subjectImg || !subjectImg->valid() || !isSameSize(targetImg, subjectImg))
	{
		fprintf(stderr, "Invalid input to ebs3d().\n");
		return false;
	}

	V3DLONG csz0=targetImg->sz0(), csz1=targetImg->sz1(), csz2=targetImg->sz2();

	Vol3DSimple<MYFLOAT_JBA> * dfx = 0, * dfy =0, * dfz = 0, * dft =0;
	Image2DSimple<MYFLOAT_JBA> * dTable=0;

	if (newGradientData(dfx, dfy, dfz, dft, csz0, csz1, csz2)==false)
	{
		fprintf(stderr, "Fail to allocate mmeory in ebs3d().\n");
		return false;
	}

	Vol3DSimple <MYFLOAT_JBA> * warpImg = new Vol3DSimple <MYFLOAT_JBA> (csz0, csz1, csz2);
	if (!warpImg || !warpImg->valid())
	{
		fprintf(stderr, "Fail to allocate mmeory in ebs3d().\n");
		deleteGradientData(dfx, dfy, dfz, dft);
		if (warpImg) {delete warpImg; warpImg=0;}
		return false;
	}

	vol3d_assign(warpImg->getData3dHandle(), subjectImg->getData3dHandle(), csz0, csz1, csz2);

	Vol3DSimple <UINT8_JBA> *maskImg = new Vol3DSimple <UINT8_JBA> (csz0, csz1, csz2);
	if (!maskImg || !maskImg->valid())
	{
		fprintf(stderr, "Fail to allocate memory in ebs3d_interface().\n");
		deleteGradientData(dfx, dfy, dfz, dft);
		if (warpImg) {delete warpImg; warpImg=0;}
		if (maskImg) {delete maskImg; maskImg=0;}
		return false;
	}

	double err_val, err_val_n1, err_val_n2; 
	double err_firstdrop=-1;
	const double Ratio_of_stop=0.01; 
	DisplaceFieldF3D wp_tmp; 
	DisplaceFieldF3D wp_tmp_n2, wp_tmp_n1; 

	vector <DisplaceFieldF3D> df_history;
	vector <double> err_history;

	vol3d_square_root_diff(err_val_n1, warpImg->getData3dHandle(), targetImg->getData3dHandle(), csz0, csz1, csz2);
	printf("\n\t initial error[%ld] = %7.2f ", V3DLONG(0), err_val_n1);
	wp_tmp_n1.resetToDefault();

	err_history.push_back(err_val_n1); df_history.push_back(wp_tmp_n1); 

	wp_current.resetToDefault();
	for (V3DLONG n=1; n<=nloops; n++)
	{
		if (n>1)
		{
			err_val_n2 = err_val_n1; wp_tmp_n2.copy(wp_tmp_n1);
			err_val_n1 = err_val; wp_tmp_n1.copy(wp_current);

			err_history.push_back(err_val); df_history.push_back(wp_current); 
		}
		if (n==2)
		{
			err_firstdrop = err_history.at(0) - err_history.at(1); 
		}

		if (!assign_maskimg_val(maskImg, targetImg, warpImg))
		{
			wp_current.resetToDefault();
			printf("Bypass current blk!\n"); fflush(stdout);
			break;
		}
		else
		{
			printf("\n");
		}

		diffxyz(dfx->getData3dHandle(), dfy->getData3dHandle(), dfz->getData3dHandle(), dft->getData3dHandle(),
				targetImg->getData3dHandle(), warpImg->getData3dHandle(), csz0, csz1, csz2);

		if (!ebs3d(&wp_tmp, dTable, dfx, dfy, dfz, dft, warpImg, 0, csz0-1, 0, csz1-1, 0, csz2-1, maskImg, false))
		{
			printf("The function ebs3d() returns false in wp_tmp est, Should not happen. Break in loop %ld. \n", n);
			fflush(stdout);
			wp_tmp.sx = wp_tmp.sy = wp_tmp.sz = 0;
			break;
		}

		if (fabs(wp_current.sx+wp_tmp.sx)>csz0/3 || fabs(wp_current.sy+wp_tmp.sy)>csz1/3 || fabs(wp_current.sz+wp_tmp.sz)>csz2/3 ) 
		{
			printf("Find a single-step movement that would make the overall movement too big: wp_tmp=[%5.3f,%5.3f,%5.3f] . Disallow this change and break in loop %ld. \n", wp_tmp.sx, wp_tmp.sy, wp_tmp.sz, n);
			fflush(stdout);
			wp_tmp.sx = wp_tmp.sy = wp_tmp.sz = 0;
			break;
		}

		wp_current.sx += wp_tmp.sx;
		wp_current.sy += wp_tmp.sy;
		wp_current.sz += wp_tmp.sz;

		vol3d_assign(warpImg->getData3dHandle(), subjectImg->getData3dHandle(), csz0, csz1, csz2);
		shift_warp(warpImg, &wp_current);

		vol3d_square_root_diff(err_val, warpImg->getData3dHandle(), targetImg->getData3dHandle(), csz0, csz1, csz2);
		printf("\t error[%ld] = %7.2f wp(%ld)=[%5.3f,%5.3f,%5.3f]\n", n, err_val, n, wp_current.sx, wp_current.sy, wp_current.sz);

		if (n>1)
		{
			if (err_val>err_val_n1 && err_val_n1>err_val_n2)
			{
				printf("Find the err-val increasing in two continuous steps. Break in loop %ld. wp(t)=[%5.3f,%5.3f,%5.3f], wp(t-2)=[%5.3f,%5.3f,%5.3f] . Err_val(t)=%5.3f, Err_val(t-1)=%5.3f, Err_val(t-2)=%5.3f \n",
				        n, wp_current.sx, wp_current.sy, wp_current.sz, wp_tmp_n2.sx, wp_tmp_n2.sy, wp_tmp_n2.sz, err_val, err_val_n1, err_val_n2);
				fflush(stdout);

				err_history.push_back(err_val); df_history.push_back(wp_current);

				wp_current.copy(wp_tmp_n2); 
				break;
			}

			if (fabs(err_val-err_val_n1)<Ratio_of_stop*err_firstdrop && fabs(err_val_n1-err_val_n2)<Ratio_of_stop*err_firstdrop) 
			{
				printf("Find the almost converged error values in two continuous steps. Break in loop %ld. wp(t)=[%5.3f,%5.3f,%5.3f] . Err_val(t)=%5.3f, Err_val(t-1)=%5.3f, Err_val(t-2)=%5.3f \n",
				        n, wp_current.sx, wp_current.sy, wp_current.sz, err_val, err_val_n1, err_val_n2);
				fflush(stdout);

				err_history.push_back(err_val); df_history.push_back(wp_current); 

				break;
			}

			if (n==nloops) 
			{
				err_history.push_back(err_val); df_history.push_back(wp_current);
			}
		}
		else
		{
			if (err_val>err_val_n1)
			{
				printf("Find the err-val increasing in first step. Break in loop %ld. wp(t)=[%5.3f,%5.3f,%5.3f], wp(t-1)=[%5.3f,%5.3f,%5.3f] . Err_val(t)=%5.3f, Err_val(t-1)=%5.3f  \n",
				        n, wp_current.sx, wp_current.sy, wp_current.sz, wp_tmp_n1.sx, wp_tmp_n1.sy, wp_tmp_n1.sz, err_val, err_val_n1);
				fflush(stdout);

				err_history.push_back(err_val); df_history.push_back(wp_current);

				wp_current.copy(wp_tmp_n1); 

				break;
			}
		}
	}

	unsigned V3DLONG tmpi; double minv=err_history.at(0); unsigned V3DLONG min_i=0;
	for (tmpi=1; tmpi<err_history.size(); tmpi++) {if (minv>err_history.at(tmpi)) {minv=err_history.at(tmpi); min_i=tmpi;}}
	wp_current.copy(df_history.at(min_i));
	printf("Track the wp_current as the best so far: wp(best=%ld)=[%5.3f,%5.3f,%5.3f], err=%5.3f\n", min_i, wp_current.sx, wp_current.sy, wp_current.sz, minv);

	printf("Final shift est: wp=[%5.3f,%5.3f,%5.3f] \n", wp_current.sx, wp_current.sy, wp_current.sz);

	if (maskImg) {delete maskImg; maskImg=0;}
	if (warpImg) {delete warpImg; warpImg=0;}
	deleteGradientData(dfx, dfy, dfz, dft);

	return true;
}


bool eba3d_interface(WarpParameterAffine3D & wp_current,  Vol3DSimple <MYFLOAT_JBA> * targetImg,  Vol3DSimple <MYFLOAT_JBA> * subjectImg, int nloops)
{
	if (!targetImg || !targetImg->valid() || !subjectImg || !subjectImg->valid() || !isSameSize(targetImg, subjectImg))
	{
		fprintf(stderr, "Invalid input to ebs3d().\n");
		return false;
	}

	V3DLONG csz0=targetImg->sz0(), csz1=targetImg->sz1(), csz2=targetImg->sz2();

	Vol3DSimple<MYFLOAT_JBA> * dfx = 0, * dfy =0, * dfz = 0, * dft =0;
	Image2DSimple<MYFLOAT_JBA> * dTable=0;

	if (newGradientData(dfx, dfy, dfz, dft, csz0, csz1, csz2)==false)
	{
		fprintf(stderr, "Fail to allocate mmeory in ebs3d().\n");
		return false;
	}

	Vol3DSimple <MYFLOAT_JBA> * warpImg = new Vol3DSimple <MYFLOAT_JBA> (csz0, csz1, csz2);
	if (!warpImg || !warpImg->valid())
	{
		fprintf(stderr, "Fail to allocate mmeory in ebs3d().\n");
		deleteGradientData(dfx, dfy, dfz, dft);
		if (warpImg) {delete warpImg; warpImg=0;}
		return false;
	}

	vol3d_assign(warpImg->getData3dHandle(), subjectImg->getData3dHandle(), csz0, csz1, csz2);

	Vol3DSimple <UINT8_JBA> *maskImg = new Vol3DSimple <UINT8_JBA> (csz0, csz1, csz2);
	if (!maskImg || !maskImg->valid())
	{
		fprintf(stderr, "Fail to allocate memory in ebs3d_interface().\n");
		deleteGradientData(dfx, dfy, dfz, dft);
		if (warpImg) {delete warpImg; warpImg=0;}
		if (maskImg) {delete maskImg; maskImg=0;}
		return false;
	}

	double err_val, err_val_n1, err_val_n2; 
    double err_firstdrop=-1;
	const double Ratio_of_stop=0.01; 
    WarpParameterAffine3D wp_tmp; 
	WarpParameterAffine3D wp_tmp_n2, wp_tmp_n1; 

	vector <WarpParameterAffine3D> wp_history;
	vector <double> err_history;

	vol3d_square_root_diff(err_val_n1, warpImg->getData3dHandle(), targetImg->getData3dHandle(), csz0, csz1, csz2);
	printf("\n\t initial error[%ld] = %7.2f ", V3DLONG(0), err_val_n1);
	wp_tmp_n1.resetToDefault();

	err_history.push_back(err_val_n1); wp_history.push_back(wp_tmp_n1); 

	wp_current.resetToDefault();
	for (V3DLONG n=1; n<=nloops; n++)
	{
		if (n>1)
		{
			err_val_n2 = err_val_n1; wp_tmp_n2.copy(wp_tmp_n1);
			err_val_n1 = err_val;	wp_tmp_n1.copy(wp_current);

			err_history.push_back(err_val); wp_history.push_back(wp_current); 
		}
		if (n==2)
		{
			err_firstdrop = err_val_n2 - err_val_n1;
		}

		if (!assign_maskimg_val(maskImg, targetImg, warpImg))
		{
			wp_current.resetToDefault();
			printf("Bypass current blk!\n"); fflush(stdout);
			break;
		}
		else
		{
			printf("\n");
		}


		diffxyz(dfx->getData3dHandle(), dfy->getData3dHandle(), dfz->getData3dHandle(), dft->getData3dHandle(),
				targetImg->getData3dHandle(), warpImg->getData3dHandle(), csz0, csz1, csz2);

        eba3d(&wp_tmp, dTable, dfx, dfy, dfz, dft, warpImg, 0, csz0-1, 0, csz1-1, 0, csz2-1, false);

		aggregateAffineWarp(&wp_current,  &wp_current, &wp_tmp); 

		vol3d_assign(warpImg->getData3dHandle(), subjectImg->getData3dHandle(), csz0, csz1, csz2);
		affine_warp(warpImg, &wp_current);

		vol3d_square_root_diff(err_val, warpImg->getData3dHandle(), targetImg->getData3dHandle(), csz0, csz1, csz2);

		if (n>1)
		{
			if (err_val>err_val_n1 && err_val_n1>err_val_n2)
			{
				err_history.push_back(err_val); wp_history.push_back(wp_current); 

				wp_current.copy(wp_tmp_n2); 
				break;
			}

			if (fabs(err_val-err_val_n1)<Ratio_of_stop*err_firstdrop && fabs(err_val_n1-err_val_n2)<Ratio_of_stop*err_firstdrop) 
			{
				err_history.push_back(err_val); wp_history.push_back(wp_current); 

				break;
			}

			if (n==nloops)
			{
				err_history.push_back(err_val); wp_history.push_back(wp_current);
			}
		}
		else
		{
			if (err_val>err_val_n1)
			{
				err_history.push_back(err_val); wp_history.push_back(wp_current); 

				wp_current.copy(wp_tmp_n1); 
				break;
			}
		}
	}

	unsigned V3DLONG tmpi; double minv=err_history.at(0); unsigned V3DLONG min_i=0;
	for (tmpi=1; tmpi<err_history.size(); tmpi++) {if (minv>err_history.at(tmpi)) {minv=err_history.at(tmpi); min_i=tmpi;}}
	wp_current.copy(wp_history.at(min_i));
	printf("Track the wp_current as the best so far: wp(best=%ld)=[%5.3f,%5.3f,%5.3f], err=%5.3f\n", min_i, wp_current.sx, wp_current.sy, wp_current.sz, minv);

	printf("Final affine est for shift: wp=[%5.3f,%5.3f,%5.3f] \n", wp_current.sx, wp_current.sy, wp_current.sz);

	if (maskImg) {delete maskImg; maskImg=0;}
	if (warpImg) {delete warpImg; warpImg=0;}
	deleteGradientData(dfx, dfy, dfz, dft);

	return true;
}


Vol3DSimple<DisplaceFieldF3D> * cdum(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, DFComputeMethodType method)
{
	switch (method)
	{
		case DF_GEN_TPS:
			return compute_df_tps(matchTargetPos, matchSubjectPos, sz0, sz1, sz2);
			break;

		case DF_GEN_TPS_LINEAR_INTERP:
			return compute_df_tps_interp(matchTargetPos, matchSubjectPos, sz0, sz1, sz2, method);
			break;

		default:
			fprintf(stderr, "Invalid DFComputeMethodType was used in cdum(). Do nothing.\n");
			return 0;
			break;
	}
}


Vol3DSimple<DisplaceFieldF3D> * compute_df_tps(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2)
{
	int nCpt = matchTargetPos.size();
	if (nCpt!=matchSubjectPos.size() || nCpt<=0)
	{
		fprintf(stderr, "The input vectors are invalid in compute_tps_df_field().\n");
		return 0;
	}

	Image2DSimple<MYFLOAT_JBA> * cpt_target = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	Image2DSimple<MYFLOAT_JBA> * cpt_subject = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	if (!cpt_target || !cpt_target->valid() || !cpt_subject || !cpt_subject->valid())
	{
	   fprintf(stderr, "Fail to allocate memory.");
	   if (cpt_target) {delete cpt_target; cpt_target=0;}
	   if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
	   return 0;
	}

    V3DLONG n;

    MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
    MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();
	printf("\n---------------------------------\n");
    for (n=0;n<nCpt;n++)
    {
		cpt_target_ref[n][0] = matchTargetPos.at(n).x;
		cpt_target_ref[n][1] = matchTargetPos.at(n).y;
		cpt_target_ref[n][2] = matchTargetPos.at(n).z;

		cpt_subject_ref[n][0] = matchSubjectPos.at(n).x;
		cpt_subject_ref[n][1] = matchSubjectPos.at(n).y;
		cpt_subject_ref[n][2] = matchSubjectPos.at(n).z;

		printf("n=%d \tx=[%5.3f -> %5.3f] y=[%5.3f -> %5.3f] z=[%5.3f -> %5.3f] \n",
		       n, cpt_target_ref[n][0], cpt_subject_ref[n][0], cpt_target_ref[n][1], cpt_subject_ref[n][1], cpt_target_ref[n][2], cpt_subject_ref[n][2]);
    }
	printf("\n#################################\n");

	Matrix wR(nCpt, nCpt);

	double tmp, s;

	V3DLONG i, j, k;
	for (j=0; j<nCpt; j++)
	{
	  for (i=0; i<nCpt;i++)
	  {
		s = 0.0;
		tmp = cpt_target_ref[i][0] - cpt_target_ref[j][0]; s += tmp*tmp;
		tmp = cpt_target_ref[i][1] - cpt_target_ref[j][1]; s += tmp*tmp;
		tmp = cpt_target_ref[i][2] - cpt_target_ref[j][2]; s += tmp*tmp;
	    wR(i+1,j+1) = 2*s*log(s+1e-20);
	  }
	}

	Matrix wP(nCpt, 4);
	for (j=0; j<nCpt; j++)
	{
	   wP(j+1,1) = 1;
	   wP(j+1,2) = cpt_target_ref[j][0];
	   wP(j+1,3) = cpt_target_ref[j][1];
	   wP(j+1,4) = cpt_target_ref[j][2];
	}

	Matrix wL(nCpt+4, nCpt+4);
	wL.submatrix(1, nCpt, 1, nCpt) = wR;
	wL.submatrix(1, nCpt, nCpt+1, nCpt+4) = wP;
	wL.submatrix(nCpt+1, nCpt+4, 1, nCpt) = wP.t();
	wL.submatrix(nCpt+1, nCpt+4, nCpt+1, nCpt+4) = 0;

	Matrix wY(nCpt+4, 3);
	for (j=0; j<nCpt; j++)
	{
	   wY(j+1,1) = cpt_subject_ref[j][0];
	   wY(j+1,2) = cpt_subject_ref[j][1];
	   wY(j+1,3) = cpt_subject_ref[j][2];
	}
	wY.submatrix(nCpt+1, nCpt+4, 1, 3) = 0;

	Matrix wW;

	Try
	{
	   wW = wL.i() * wY;
	}
	CatchAll
	{
	   fprintf(stderr, "Fail to find the inverse of the wL matrix.\n");

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		return 0;
	}

	V3DLONG p;

    Vol3DSimple<DisplaceFieldF3D> *df_local = new Vol3DSimple<DisplaceFieldF3D> (sz0, sz1, sz2);
	if (!df_local || !df_local->valid())
	{
	   fprintf(stderr, "Fail to allocate memory for df_local tps warping [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    V3DLONG ndimpt = 3; 
    double * dist = new double [nCpt+ndimpt+1];
	if (!dist)
	{
	   fprintf(stderr, "Fail to allocate memory dist for tps warping [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    DisplaceFieldF3D *** df_local_ref = df_local->getData3dHandle();

    printf("-------------------- Now compute the distances of pixels to the mapping points. -------\n\n");
	for (k=0;k<sz2;k++)
	{
		for (j=0;j<sz1;j++)
		{
			for (i=0;i<sz0;i++)
			{
				for (n=0;n<nCpt; n++)
				{
					s = 0;
					tmp = i-cpt_target_ref[n][0]; s += tmp*tmp;
					tmp = j-cpt_target_ref[n][1]; s += tmp*tmp;
					tmp = k-cpt_target_ref[n][2]; s += tmp*tmp;
					dist[n] = 2*s*log(s+1e-20);
				}

				dist[nCpt] = 1;
				dist[nCpt+1] = i;
				dist[nCpt+2] = j;
				dist[nCpt+3] = k;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 1);}
				df_local_ref[k][j][i].sx = s - i;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 2);}
				df_local_ref[k][j][i].sy = s - j; 

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 3);}
				df_local_ref[k][j][i].sz = s - k;

			}
		}
		printf("z=%ld ",k); fflush(stdout);
	}
	printf("\n");

	if (dist) {delete []dist; dist=0;} 
	if (cpt_target) {delete cpt_target; cpt_target=0;}
	if (cpt_subject) {delete cpt_subject; cpt_subject=0;}

	return df_local;
}



Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_interp(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
													  DFComputeMethodType interp_method) 
{
	if (interp_method!=DF_GEN_TPS_LINEAR_INTERP)
	{
		fprintf(stderr, "Invalid DFComputeMethodType is input to compute_df_tps_interp(). Do nothing.\n");
		return 0;
	}

	double gfactor_x, gfactor_y, gfactor_z;
	gfactor_x = gfactor_y = gfactor_z =4; 
    Vol3DSimple<DisplaceFieldF3D> * df_subv = compute_df_tps_subsampled_volume(matchTargetPos, matchSubjectPos, sz0, sz1, sz2, gfactor_x, gfactor_y, gfactor_z);
	if (!df_subv)
	{
		printf("Fail to produce the subsampled DF.\n");
		return 0;
	}

	V3DLONG p;
	V3DLONG gsz0 = df_subv->sz0(), gsz1 = df_subv->sz1(), gsz2 = df_subv->sz2();
	Vol3DSimple<MYFLOAT_JBA> *df_grid = new Vol3DSimple<MYFLOAT_JBA> (gsz0+3, gsz1+3, gsz2+3);
	if (!df_grid || !df_grid->valid())
	{
		fprintf(stderr, "Fail to allocate memory for df_grid tps warping [%d].\n", __LINE__);
		if (df_grid) {delete df_grid; df_grid=0;}
		return 0;
	}
	Vol3DSimple<DisplaceFieldF3D> * df_local = new Vol3DSimple<DisplaceFieldF3D> (sz0, sz1, sz2);
	if (!df_local || !df_local->valid())
	{
		if (df_grid) {delete df_grid; df_grid=0;}
		if (df_subv) {delete df_subv; df_subv=0;}
		return 0;
	}

	printf("-------------------- Now interpolate the DF using linear method. -------\n\n");

	DisplaceFieldF3D *** df_subv_3d = df_subv->getData3dHandle();
    MYFLOAT_JBA *** df_grid_3d = df_grid->getData3dHandle();
	DisplaceFieldF3D *** df_local_3d = df_local->getData3dHandle();

	Vol3DSimple <MYFLOAT_JBA> * df_tmp = 0; MYFLOAT_JBA *** df_tmp_3d=0;
	V3DLONG i,j,k;

	for (k=0;k<gsz2;k++) { for (j=0;j<gsz1;j++) { for (i=0;i<gsz0;i++) {df_grid_3d[k][j][i] = df_subv_3d[k][j][i].sx;}}}
	df_tmp = lir3d(sz0, sz1, sz2, df_grid);
	df_tmp_3d = df_tmp->getData3dHandle();
	for (k=0;k<sz2;k++) for (j=0;j<sz1;j++) for (i=0;i<sz0;i++) df_local_3d[k][j][i].sx = df_tmp_3d[k][j][i];
	if (df_tmp) {delete df_tmp; df_tmp=0;}

	for (k=0;k<gsz2;k++) { for (j=0;j<gsz1;j++) { for (i=0;i<gsz0;i++) {df_grid_3d[k][j][i] = df_subv_3d[k][j][i].sy;}}}
	df_tmp = lir3d(sz0, sz1, sz2, df_grid);
	df_tmp_3d = df_tmp->getData3dHandle();
	for (k=0;k<sz2;k++) for (j=0;j<sz1;j++) for (i=0;i<sz0;i++) df_local_3d[k][j][i].sy = df_tmp_3d[k][j][i];
	if (df_tmp) {delete df_tmp; df_tmp=0;}

	for (k=0;k<gsz2;k++) { for (j=0;j<gsz1;j++) { for (i=0;i<gsz0;i++) {df_grid_3d[k][j][i] = df_subv_3d[k][j][i].sz;}}}
	df_tmp = lir3d(sz0, sz1, sz2, df_grid);
	df_tmp_3d = df_tmp->getData3dHandle();
	for (k=0;k<sz2;k++) for (j=0;j<sz1;j++) for (i=0;i<sz0;i++) df_local_3d[k][j][i].sz = df_tmp_3d[k][j][i];
	if (df_tmp) {delete df_tmp; df_tmp=0;}

	if (df_grid) {delete df_grid; df_grid=0;}
	if (df_subv) {delete df_subv; df_subv=0;}

	return df_local;
}

Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z)
{
	int nCpt = matchTargetPos.size();
	if (nCpt!=matchSubjectPos.size() || nCpt<=0)
	{
		fprintf(stderr, "The input vectors are invalid in compute_tps_df_field().\n");
		return 0;
	}

	Image2DSimple<MYFLOAT_JBA> * cpt_target = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	Image2DSimple<MYFLOAT_JBA> * cpt_subject = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	if (!cpt_target || !cpt_target->valid() || !cpt_subject || !cpt_subject->valid())
	{
		fprintf(stderr, "Fail to allocate memory.");
		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		return 0;
	}

    V3DLONG n;

    MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
    MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();
	printf("\n---------------------------------\n");
    for (n=0;n<nCpt;n++)
    {
		cpt_target_ref[n][0] = matchTargetPos.at(n).x;
		cpt_target_ref[n][1] = matchTargetPos.at(n).y;
		cpt_target_ref[n][2] = matchTargetPos.at(n).z;

		cpt_subject_ref[n][0] = matchSubjectPos.at(n).x;
		cpt_subject_ref[n][1] = matchSubjectPos.at(n).y;
		cpt_subject_ref[n][2] = matchSubjectPos.at(n).z;

		printf("n=%d \tx=[%5.3f -> %5.3f] y=[%5.3f -> %5.3f] z=[%5.3f -> %5.3f] \n",
		       n, cpt_target_ref[n][0], cpt_subject_ref[n][0], cpt_target_ref[n][1], cpt_subject_ref[n][1], cpt_target_ref[n][2], cpt_subject_ref[n][2]);
    }
	printf("\n#################################\n");

	Matrix wR(nCpt, nCpt);

	double tmp, s;

	V3DLONG i, j, k;
	for (j=0; j<nCpt; j++)
	{
		for (i=0; i<nCpt;i++)
		{
			s = 0.0;
			tmp = cpt_target_ref[i][0] - cpt_target_ref[j][0]; s += tmp*tmp;
			tmp = cpt_target_ref[i][1] - cpt_target_ref[j][1]; s += tmp*tmp;
			tmp = cpt_target_ref[i][2] - cpt_target_ref[j][2]; s += tmp*tmp;
			wR(i+1,j+1) = 2*s*log(s+1e-20);
		}
	}

	Matrix wP(nCpt, 4);
	for (j=0; j<nCpt; j++)
	{
		wP(j+1,1) = 1;
		wP(j+1,2) = cpt_target_ref[j][0];
		wP(j+1,3) = cpt_target_ref[j][1];
		wP(j+1,4) = cpt_target_ref[j][2];
	}

	Matrix wL(nCpt+4, nCpt+4);
	wL.submatrix(1, nCpt, 1, nCpt) = wR;
	wL.submatrix(1, nCpt, nCpt+1, nCpt+4) = wP;
	wL.submatrix(nCpt+1, nCpt+4, 1, nCpt) = wP.t();
	wL.submatrix(nCpt+1, nCpt+4, nCpt+1, nCpt+4) = 0;

	Matrix wY(nCpt+4, 3);
	for (j=0; j<nCpt; j++)
	{
		wY(j+1,1) = cpt_subject_ref[j][0];
		wY(j+1,2) = cpt_subject_ref[j][1];
		wY(j+1,3) = cpt_subject_ref[j][2];
	}
	wY.submatrix(nCpt+1, nCpt+4, 1, 3) = 0;

	Matrix wW;

	Try
	{
		wW = wL.i() * wY;
	}
	CatchAll
	{
		fprintf(stderr, "Fail to find the inverse of the wL matrix.\n");

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		return 0;
	}

	V3DLONG p;

	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0)/gfactor_x)))+1, gsz1 = (V3DLONG)(ceil((double(sz1)/gfactor_y)))+1, gsz2 = (V3DLONG)(ceil((double(sz2)/gfactor_z)))+1;
	Vol3DSimple<DisplaceFieldF3D> * df_local = new Vol3DSimple<DisplaceFieldF3D> (gsz0, gsz1, gsz2);
	DisplaceFieldF3D *** df_local_3d = df_local->getData3dHandle();

	if (!df_local || !df_local->valid())
	{
		fprintf(stderr, "Fail to allocate memory for the subsampled DF volume memory [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    V3DLONG ndimpt = 3;
    double * dist = new double [nCpt+ndimpt+1];
	if (!dist)
	{
		fprintf(stderr, "Fail to allocate memory dist for tps warping [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    printf("-------------------- Now compute the distances of pixels to the mapping points. -------\n\n");

	DisplaceFieldF3D * df_local_1d = df_local->getData1dHandle();
	for (k=0;k<df_local->getTotalElementNumber();k++)
	{
		df_local_1d[k].sz = df_local_1d[k].sy = df_local_1d[k].sx = 0;
	}
	for (k=0;k<gsz2;k++) 
	{
		for (j=0;j<gsz1;j++)
		{
			for (i=0;i<gsz0;i++)
			{
				for (n=0;n<nCpt; n++)
				{
					s = 0;
					tmp = (i*gfactor_x)-cpt_target_ref[n][0]; s += tmp*tmp;
					tmp = (j*gfactor_y)-cpt_target_ref[n][1]; s += tmp*tmp;
					tmp = (k*gfactor_z)-cpt_target_ref[n][2]; s += tmp*tmp;
					dist[n] = 2*s*log(s+1e-20);
				}

				dist[nCpt] = 1;
				dist[nCpt+1] = i*gfactor_x;
				dist[nCpt+2] = j*gfactor_y;
				dist[nCpt+3] = k*gfactor_z;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 1);}
				df_local_3d[k][j][i].sx = s - i*gfactor_x;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 2);}
				df_local_3d[k][j][i].sy = s - j*gfactor_y;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 3);}
				df_local_3d[k][j][i].sz = s - k*gfactor_z;
			}//i
		}//j
		printf("z=%ld ",k); fflush(stdout);
	}//k
	printf("\n");

	if (dist) {delete []dist; dist=0;}
	if (cpt_target) {delete cpt_target; cpt_target=0;}
	if (cpt_subject) {delete cpt_subject; cpt_subject=0;}

	return df_local;
}



Vol3DSimple <MYFLOAT_JBA> * lir3d(V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, Vol3DSimple <MYFLOAT_JBA> * df_regular_grid)
{
	V3DLONG k, j, i, k1, j1, i1, kk, jj, ii;

	if (!df_regular_grid || !df_regular_grid->valid())
	{
		fprintf(stderr, "The pointer is not correct.\n");
		return 0;
	}
	MYFLOAT_JBA *** df_grid3d = df_regular_grid->getData3dHandle();
	V3DLONG n0 = df_regular_grid->sz0()-3, n1 = df_regular_grid->sz1()-3, n2 = df_regular_grid->sz2()-3;
	if (n0<=0 || n1<=0 || n2<=0)
	{
		fprintf(stderr, "The size  is not correct.\n");
		return 0;
	}
	if (sz0<=0 || sz1<=0 || sz2<=0)
	{
		fprintf(stderr, "The size of the DF to be computed is not correct.\n");
		return 0;
	}

	Vol3DSimple <MYFLOAT_JBA> * df_field  = new Vol3DSimple <MYFLOAT_JBA>(sz0, sz1, sz2);
	if (!df_field ||!df_field->valid())
	{
		fprintf(stderr, "Fail to allocate memory.\n");
		if (df_field) {delete df_field; df_field=0;}
		return 0;
	}

	MYFLOAT_JBA * df_field_ref1d = df_field->getData1dHandle();
	for (i=0;i<df_field->getTotalElementNumber(); i++)
	{
		df_field_ref1d[i] = 0;
	}
	MYFLOAT_JBA *** df_field_ref3d = df_field->getData3dHandle();

	Coord3D_JBA *c = new Coord3D_JBA [df_field->getTotalElementNumber()];
	double nf0=(double)n0/sz0, nf1=(double)n1/sz1, nf2=(double)n2/sz2;
	V3DLONG cnt=0;
	for (k=0;k<sz2;k++)
	{
		double k_tmp=(double)k*nf2;
		for (j=0;j<sz1;j++)
		{
			double j_tmp=(double)j*nf1;
			for (i=0;i<sz0;i++)
			{
				c[cnt].x = i*nf0;
				c[cnt].y = j_tmp;
				c[cnt].z = k_tmp;
				cnt++;
			}
		}
	}

	lcl(df_field_ref1d, c, df_field->getTotalElementNumber(),
								  df_grid3d, df_regular_grid->sz0(), df_regular_grid->sz1(), df_regular_grid->sz2(),
								  0, n0-1+1, 0, n1-1+1, 0, n2-1+1); 

	if (c) {delete []c; c=0;}
	return df_field;
}



