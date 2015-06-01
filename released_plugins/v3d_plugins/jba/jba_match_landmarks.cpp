//jba_match_landmarks.cpp
// by Hanchuan Peng
// 2006-2011

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <queue>
#include <fstream>


#include "jba_match_landmarks.h"
#include "volimg_proc.h"

#include "remove_nonaffine_points.h" 

#define WANT_STREAM
#include "newmatap.h"
#include "newmatio.h"
#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const KernelSet * KS) 
{
	if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid())
		return -1; 
	
	V3DLONG TSZ2=img_target->sz2(), TSZ1=img_target->sz1(), TSZ0=img_target->sz0();
	V3DLONG SSZ2=img_subject->sz2(), SSZ1=img_subject->sz1(), SSZ0=img_subject->sz0();
	
	if (txpos<0 || txpos>=TSZ0 || typos<0 || typos>=TSZ1 || tzpos<0 || tzpos>=TSZ2 ||
		sxpos<0 || sxpos>=SSZ0 || sypos<0 || sypos>=SSZ1 || szpos<0 || szpos>=SSZ2)
		return -1;
	
	T *** img_target_ref = img_target->getData3dHandle();
	T *** img_subject_ref = img_subject->getData3dHandle();
	
	if (img_subject_ref[szpos][sypos][sxpos]==0) 
		return -1;
	
	double s=0;
	V3DLONG i,j,k, kt,jt,it, ks,js,is;
	V3DLONG cnt=0;
	
	{
		printf("Matching intensity!\n");
		double s1=0, s2=0;
		s=0;
		cnt=0;
		for (k=-R_compare; k<=R_compare; k++)
		{
			kt=tzpos+k; if (kt<0 || kt>=TSZ2) continue;
			ks=szpos+k; if (ks<0 || ks>=SSZ2) continue;
			for (j=-R_compare; j<=R_compare; j++)
			{
				jt=typos+j; if (jt<0 || jt>=TSZ1) continue;
				js=sypos+j; if (js<0 || js>=SSZ1) continue;
				for (i=-R_compare; i<=R_compare; i++)
				{
					it=txpos+i; if (it<0 || it>=TSZ0) continue;
					is=sxpos+i; if (is<0 || is>=SSZ0) continue;
					
					s += fabs(img_target_ref[kt][jt][it] - img_subject_ref[ks][js][is]);
					s1 += img_target_ref[kt][jt][it];
					s2 += img_subject_ref[ks][js][is];
					cnt++;
				}
			}
		}
		
		if (cnt<=0)
		{
			s=-1; 
		}
		else
		{
			s = exp(-s/cnt - fabs(s1-s2)/cnt);
		}
	}
	
	return s;
}


template <class T> double cmpFeaturesonTwoImages(Vol3DSimple<T>* img_target, V3DLONG txpos, V3DLONG typos, V3DLONG tzpos,
												 Vol3DSimple<T>* img_subject, V3DLONG sxpos, V3DLONG sypos, V3DLONG szpos,
												 V3DLONG R_compare, const PointMatchMethodType mMethod, Vector1DSimple<double> * matchingScore) 
{
#define USE_SPHERE_WIN
	
	if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid())
		return -1; 
	
	V3DLONG TSZ2=img_target->sz2(), TSZ1=img_target->sz1(), TSZ0=img_target->sz0();
	V3DLONG SSZ2=img_subject->sz2(), SSZ1=img_subject->sz1(), SSZ0=img_subject->sz0();
	
	if (txpos<0 || txpos>=TSZ0 || typos<0 || typos>=TSZ1 || tzpos<0 || tzpos>=TSZ2 ||
		sxpos<0 || sxpos>=SSZ0 || sypos<0 || sypos>=SSZ1 || szpos<0 || szpos>=SSZ2)
		return -1;
	
	if (!matchingScore || !matchingScore->valid())
	{
		fprintf(stderr, "Invalid matchingScore data structure (invalid pointer) passed to cmpFeaturesonTwoImages().\n");
		return -1;
	}
	
	if (mMethod==MATCH_MULTIPLE_MI_INT_CORR && matchingScore->sz0()!=2)
	{
		fprintf(stderr, "Invalid matchingScore data structure (sz unmatched) passed to cmpFeaturesonTwoImages().\n");
		return -1;
	}
	
	T *** img_target_ref = img_target->getData3dHandle();
	T *** img_subject_ref = img_subject->getData3dHandle();
	
	double s=0, tdiswei;
	V3DLONG i,j,k, kt,jt,it, ks,js,is;
	double cnt=0;
	
    if (mMethod==MATCH_MEANCIRCLEINTENSITY) 
	{
		if (R_compare>15) R_compare=15; 
		
#ifdef USE_SPHERE_WIN
		double RS2=R_compare*R_compare, kS2, jS2, iS2, tdis2, tdis3;
#endif
		
		double * cir_hist_target = new double [R_compare+1];
		double * cir_hist_subject = new double [R_compare+1];
		V3DLONG * cir_cnt = new V3DLONG [R_compare+1];
		for (i=0;i<=R_compare;i++) {cir_hist_target[i] = cir_hist_subject[i] = 0; cir_cnt[i]=0;}
		
		cnt=0;
		for (k=-R_compare; k<=R_compare; k+=1)
		{
			kt=tzpos+k; if (kt<0 || kt>=TSZ2) continue;
			ks=szpos+k; if (ks<0 || ks>=SSZ2) continue;
			
#ifdef USE_SPHERE_WIN
			kS2=k*k;
#endif
			
			for (j=-R_compare; j<=R_compare; j+=1) 
			{
				jt=typos+j; if (jt<0 || jt>=TSZ1) continue;
				js=sypos+j; if (js<0 || js>=SSZ1) continue;
				
#ifdef USE_SPHERE_WIN
				jS2=j*j;
				if ((tdis2=kS2+jS2)>RS2) continue;
#endif
				
				for (i=-R_compare; i<=R_compare; i+=1)
				{
					it=txpos+i; if (it<0 || it>=TSZ0) continue;
					is=sxpos+i; if (is<0 || is>=SSZ0) continue;
					
#ifdef USE_SPHERE_WIN
					iS2=i*i;
					if ((tdis3=tdis2+iS2)>RS2) continue; 
#endif
					
#ifdef USE_SPHERE_WIN
					tdiswei = 1;
#else
					tdiswei = 1;
#endif
					
					double cur_t = img_target_ref[kt][jt][it]; 
                    double cur_s = img_subject_ref[ks][js][is];
					double tmp_s1 = tdiswei* cur_t, tmp_s2 = tdiswei* cur_s;
					
					V3DLONG tmp_r=V3DLONG(floor(sqrt(tdis3)));
					cir_hist_target[tmp_r] += tmp_s1;
					cir_hist_subject[tmp_r] += tmp_s2;
					cir_cnt[tmp_r]++;
					
					cnt+=tdiswei;
				}
			}
		}
		
		
		s=0;
		for (i=0;i<=R_compare;i++)
		{
			s += fabs(cir_hist_target[i] - cir_hist_subject[i])/cir_cnt[i];
		}
		s = exp(-s/(R_compare+1));
		
		if (cir_hist_target) {delete []cir_hist_target; cir_hist_target=0;}
		if (cir_hist_subject) {delete []cir_hist_subject; cir_hist_subject=0;}
		if (cir_cnt) {delete []cir_cnt; cir_cnt=0;}
		
		matchingScore->getData1dHandle()[0] = s;
	}
	else 
	{
#ifdef USE_SPHERE_WIN
		double RS2=R_compare*R_compare, kS2, jS2, iS2, tdis2, tdis3;
#endif
		
#define ILEVEL 16
		double P_ST[ILEVEL][ILEVEL], P_S[ILEVEL], P_T[ILEVEL];  for (j=0;j<ILEVEL;j++) {P_S[j]=P_T[j]=0; for (i=0;i<ILEVEL; i++) P_ST[j][i]=0; } 
        V3DLONG ind_s, ind_t; 
        double s_mi, s_nmi;
		
		double * cir_hist_target = new double [R_compare+1];
		double * cir_hist_subject = new double [R_compare+1];
		for (i=0;i<=R_compare;i++) {cir_hist_target[i] = cir_hist_subject[i] = 0;}
		
		double s1=0, s2=0;
		s=0;
		cnt=0;
		for (k=-R_compare; k<=R_compare; k+=4)
		{
			kt=tzpos+k; if (kt<0 || kt>=TSZ2) continue;
			ks=szpos+k; if (ks<0 || ks>=SSZ2) continue;
			
#ifdef USE_SPHERE_WIN
			kS2=k*k;
#endif
			
			for (j=-R_compare; j<=R_compare; j+=4) 
			{
				jt=typos+j; if (jt<0 || jt>=TSZ1) continue;
				js=sypos+j; if (js<0 || js>=SSZ1) continue;
				
#ifdef USE_SPHERE_WIN
				jS2=j*j;
				if ((tdis2=kS2+jS2)>RS2) continue;
#endif
				
				for (i=-R_compare; i<=R_compare; i+=4)
				{
					it=txpos+i; if (it<0 || it>=TSZ0) continue;
					is=sxpos+i; if (is<0 || is>=SSZ0) continue;
					
#ifdef USE_SPHERE_WIN
					iS2=i*i;
					if ((tdis3=tdis2+iS2)>RS2) continue; 
#endif
					
#ifdef USE_SPHERE_WIN
					tdiswei = exp(-tdis3/RS2*3);
#else
					tdiswei = 1;
#endif
					
					double cur_t = img_target_ref[kt][jt][it]; 
                    double cur_s = img_subject_ref[ks][js][is];
					double tmp_s1 = tdiswei* cur_t, tmp_s2 = tdiswei* cur_s;
					
					s += fabs(tmp_s1-tmp_s2);
					s1 += tmp_s1;
					s2 += tmp_s2;
					
					ind_t = V3DLONG(floor(cur_t*ILEVEL)); if (ind_t<0) ind_t=0; if (ind_t>=ILEVEL) ind_t=ILEVEL-1;
					ind_s = V3DLONG(floor(cur_s*ILEVEL)); if (ind_s<0) ind_s=0; if (ind_s>=ILEVEL) ind_s=ILEVEL-1;
					P_ST[ind_t][ind_s]+=tdiswei;
					P_S[ind_s]+=tdiswei;
					P_T[ind_t]+=tdiswei;
					
					cnt+=tdiswei;
				}
			}
		}
		
		if (cnt<=0)
		{
			s=-1; 
		}
		else
		{
			s = exp(-s/cnt);
		}
		matchingScore->getData1dHandle()[0] = s;
		
		if (mMethod==MATCH_MI || mMethod==MATCH_MULTIPLE_MI_INT_CORR)
		{
			s_mi=0; double h_T=0, h_S=0; 
			for (j=0;j<ILEVEL;j++)
			{
				P_T[j] /= cnt;
				P_S[j] /= cnt;
				
				for (i=0;i<ILEVEL; i++)
				{
					P_ST[j][i] /= cnt;
				}
			}
			for (j=0;j<ILEVEL;j++)
			{
				if (P_T[j]) h_T -= P_T[j]*log(P_T[j]);
				if (P_S[j]) h_S -= P_S[j]*log(P_S[j]);
				
				if (P_T[j]<=0) continue;
				for (i=0;i<ILEVEL; i++)
				{
					if (P_S[i]<=0) continue;
					if (P_ST[j][i]<=0) continue;
					s_mi += double(P_ST[j][i])*log(double(P_ST[j][i])/(double(P_T[j])*P_S[i]));
				}
			}
			
			s_nmi = s_mi*2.0/(h_T + h_S);
			s = s_nmi;
			
			if (mMethod==MATCH_MULTIPLE_MI_INT_CORR)
				matchingScore->getData1dHandle()[1] = s;
			else
				matchingScore->getData1dHandle()[0] = s;
		}
		
		if (mMethod==MATCH_CORRCOEF ) 
		{
			if (cnt>0)
			{
				double m1=s1/cnt;
				double m2=s2/cnt;
				double tmp_s=0, tmp_s1=0, tmp_s2=0, tmp1, tmp2;
				
				for (k=-R_compare; k<=R_compare; k+=2)
				{
					kt=tzpos+k; if (kt<0 || kt>=TSZ2) continue;
					ks=szpos+k; if (ks<0 || ks>=SSZ2) continue;
#ifdef USE_SPHERE_WIN
					kS2=k*k;
#endif
					for (j=-R_compare; j<=R_compare; j+=2) 
					{
						jt=typos+j; if (jt<0 || jt>=TSZ1) continue;
						js=sypos+j; if (js<0 || js>=SSZ1) continue;
						
#ifdef USE_SPHERE_WIN
						jS2=j*j;
						if ((tdis2=kS2+jS2)>RS2) continue;
#endif
						
						for (i=-R_compare; i<=R_compare; i+=2)
						{
							it=txpos+i; if (it<0 || it>=TSZ0) continue;
							is=sxpos+i; if (is<0 || is>=SSZ0) continue;
							
#ifdef USE_SPHERE_WIN
							iS2=i*i;
							if ((tdis3=tdis2+iS2)>RS2) continue; 
#endif
							
#ifdef USE_SPHERE_WIN
							tdiswei = exp(-tdis3/RS2*3);
#else
							tdiswei = 1;
#endif
							
							tmp1 = (img_target_ref[kt][jt][it]-m1);
							tmp2 = (img_subject_ref[ks][js][is]-m2);
							tmp_s += tdiswei* tmp1*tmp2;
							tmp_s1 += tdiswei* tmp1*tmp1;
							tmp_s2 += tdiswei* tmp2*tmp2;
						}
					}
				}
				
				tmp_s = (tmp_s / sqrt(tmp_s1) / sqrt(tmp_s2) + 1 )/2; 
                s = tmp_s;
			}
			
			if (mMethod==MATCH_MULTIPLE_MI_INT_CORR)
				matchingScore->getData1dHandle()[2] = s;
			else
				matchingScore->getData1dHandle()[0] = s;
		}
		
	}
	
	return s;
}

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<MYFLOAT_JBA> * img_target,
								   Vol3DSimple<MYFLOAT_JBA> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
								   const BasicWarpParameter & bwp
								   )
{
	return detectBestMatchingCpt(matchTargetPos,
								 matchSubjectPos,
								 img_target,
								 img_subject,
								 matchScore,
								 priorTargetPos,
								 0,
								 0,
								 bwp
								 );
}

bool detectBestMatchingCpt_virtual(vector<Coord3D_JBA> & matchTargetPos,
								   vector<Coord3D_JBA> & matchSubjectPos,
								   Vol3DSimple<unsigned char> * img_target,
								   Vol3DSimple<unsigned char> * img_subject,
								   PointMatchScore & matchScore,
								   const vector<Coord3D_JBA> & priorTargetPos,
								   const BasicWarpParameter & bwp
								   )
{
	return detectBestMatchingCpt(matchTargetPos,
								 matchSubjectPos,
								 img_target,
								 img_subject,
								 matchScore,
								 priorTargetPos,
								 0,
								 0,
								 bwp
								 );
}


template <class T> bool detectBestMatchingCpt(vector<Coord3D_JBA> & matchTargetPos,
											  vector<Coord3D_JBA> & matchSubjectPos,
											  Vol3DSimple<T> * img_target,
											  Vol3DSimple<T> * img_subject,
											  PointMatchScore & matchScore,
											  const vector<Coord3D_JBA> & priorTargetPos0,
											  Vol3DSimple<unsigned short int> *img_target_matchrange,				  
											  Vol3DSimple<unsigned short int> *img_subject_matchrange,				  
											  const BasicWarpParameter & bwp
											  )
{
	bool b_noerror=true;
	V3DLONG i,j,k, kk;
	
	if (!img_target || !img_target->valid() || !img_subject || !img_subject->valid()  || priorTargetPos0.size()<=0) 
	{
		printf("Your data are not valid\n");
		return false;
	}
	if (!isSameSize(img_subject, img_target))
	{
		printf("Warning: your images have different sizes, - be sure they are correct.\n");
	}
	if (img_target_matchrange && img_target_matchrange->valid() && img_subject_matchrange && img_subject_matchrange->valid())
	{
		if (!isSameSize(img_target_matchrange, img_subject_matchrange))
		{
			printf("You specify two valid matching range pointers but they have different sizes. Thus overall your data is invalid. Do nothing.\n");
			return false;
		}
	}
	bool b_use_target_mrange=false, b_use_subject_mrange = false;
	if (img_target_matchrange && img_target_matchrange->valid())
	{
		if (img_target_matchrange->sz0()!=2 || img_target_matchrange->sz1()!=img_target->sz1() || img_target_matchrange->sz2()!=img_target->sz2())
		{
			printf("The size of the matching range of the target image does not match the size of the target volume image. Do nothing.\n");
			return false;
		}
		b_use_target_mrange = true;
	}
	if (img_subject_matchrange && img_subject_matchrange->valid())
	{
		if (img_subject_matchrange->sz0()!=2 || img_subject_matchrange->sz1()!=img_subject->sz1() || img_subject_matchrange->sz2()!=img_subject->sz2())
		{
			printf("The size of the matching range of the subject image does not match the size of the subject volume image. Do nothing.\n");
			return false;
		}
		b_use_subject_mrange = true;
	}
	
	PointMatchMethodType mMethod = bwp.method_match_landmarks;
	
	V3DLONG R_search = V3DLONG(floor(img_target->sz0()/16));
	V3DLONG R_compare = 48*(double(img_target->sz0())/512);
	
	vector<Coord3D_JBA> matchSubjectPos_searchCenter;
	if (bwp.b_search_around_preset_subject_pos==true) 
	{
		if (priorTargetPos0.size()<=0 || priorTargetPos0.size()!=matchSubjectPos.size())
		{
			printf("The size of the prior target pos list [len=%d] and the prior subject pos list [len=%d] do not match. Do nothing. \n", priorTargetPos0.size(), matchSubjectPos.size());
			return false;
		}
		matchSubjectPos_searchCenter = matchSubjectPos;
		R_search = bwp.radius_search_around_preset_subject_pos;
	}
	else
	{
		matchSubjectPos_searchCenter = priorTargetPos0;
	}
	const V3DLONG Step_Search_coarse = (V3DLONG)(floor(R_search/2)), Step_Search_fine = 1;
	
	matchTargetPos.clear();
	matchSubjectPos.clear();
	
	unsigned V3DLONG n;
	
	unsigned short int *** trange3d = 0, *** srange3d = 0;
	if (b_use_target_mrange) {trange3d = img_target_matchrange->getData3dHandle();}
	if (b_use_subject_mrange) {srange3d = img_subject_matchrange->getData3dHandle();}
	
	vector<Coord3D_JBA> priorTargetPos = priorTargetPos0;
	if (b_use_target_mrange)
	{
		for (n=priorTargetPos.size()-1;n>=0;n--)
		{
			V3DLONG px = V3DLONG(priorTargetPos.at(n).x);
			V3DLONG py = V3DLONG(priorTargetPos.at(n).y);
			V3DLONG pz = V3DLONG(priorTargetPos.at(n).z);
			
			if (px<0 || px>=img_target->sz0() || 
				py<0 || py>=img_target->sz1() || 
				pz<0 || pz>=img_target->sz2()) 
			{
				priorTargetPos.erase(priorTargetPos.begin()+n);
				continue;
			}
			
			if (px<trange3d[pz][py][0] || px>trange3d[pz][py][1]) 
			{
				priorTargetPos.erase(priorTargetPos.begin()+n);
				continue;
			}
		}
	}
	
	double *distMove = 0;
	if (priorTargetPos.size()>0)
		distMove = new double [priorTargetPos.size()];
	
	double cur_score, cur_image_force_score; 
	
	int N_USE_METHOD=1;
	Vector1DSimple<double> bestScoreCoarse, bestScoreFine; 
    Vector1DSimple<Coord3D_JBA> bestCoordCoarse, bestCoordFine; 
    
	Vector1DSimple<double> cur_image_force_score_matching;
	N_USE_METHOD = (mMethod==MATCH_MULTIPLE_MI_INT_CORR) ? 2 : 1;
	
	cur_image_force_score_matching.resize(N_USE_METHOD);
	bestScoreCoarse.resize(N_USE_METHOD);
	bestScoreFine.resize(N_USE_METHOD);
	bestCoordCoarse.resize(N_USE_METHOD);
	bestCoordFine.resize(N_USE_METHOD); 
    
#ifdef MATCH_MODEL_PRIORS
	
	Image2DSimple<double> dis_prior_model(priorTargetPos.size(), priorTargetPos.size());
	double ** dis_prior = dis_prior_model.getData2dHandle();
	for (n=0; n<priorTargetPos.size(); n++)
	{
		dis_prior[n][n]=0;
		V3DLONG txpos_1 = (V3DLONG)(priorTargetPos.at(n).x), typos_1 = (V3DLONG)(priorTargetPos.at(n).y), tzpos_1 = (V3DLONG)(priorTargetPos.at(n).z);
		for (m=n+1; m<priorTargetPos.size(); m++)
		{
			V3DLONG txpos_2 = (V3DLONG)(priorTargetPos.at(m).x), typos_2 = (V3DLONG)(priorTargetPos.at(m).y), tzpos_2 = (V3DLONG)(priorTargetPos.at(m).z);
			dis_prior[m][n] = dis_prior[n][m] = sqrt((txpos_1 - txpos_2)*(txpos_1 - txpos_2) + (typos_1 - typos_2)*(typos_1 - typos_2) + (tzpos_1 - tzpos_2)*(tzpos_1 - tzpos_2));
			printf("dis_prior[%d][%d]=%5.2f\t", n,m,dis_prior[n][m]);
		}
		printf("\n");
	}
	printf("\n");
	
#endif
	
	V3DLONG n_added=0; 
    for (n=0; n<priorTargetPos.size(); n++)
	{
		V3DLONG txpos_target = (V3DLONG)(priorTargetPos.at(n).x), typos_target = (V3DLONG)(priorTargetPos.at(n).y), tzpos_target = (V3DLONG)(priorTargetPos.at(n).z);
		V3DLONG txpos_subject = (V3DLONG)(matchSubjectPos_searchCenter.at(n).x), typos_subject = (V3DLONG)(matchSubjectPos_searchCenter.at(n).y), tzpos_subject = (V3DLONG)(matchSubjectPos_searchCenter.at(n).z);
		printf("target coord[%ld]=[%ld,\t%ld,\t%ld] around subject [%ld,\t%ld,\t%ld]: \t", n, txpos_target, typos_target, tzpos_target, txpos_subject, typos_subject, tzpos_subject);
		
		for (kk=0; kk<N_USE_METHOD; kk++)
		{
			bestScoreCoarse.getData1dHandle()[kk] = -1;
			bestCoordCoarse.getData1dHandle()[kk].x = txpos_subject; bestCoordCoarse.getData1dHandle()[kk].y = typos_subject; bestCoordCoarse.getData1dHandle()[kk].z = tzpos_subject;
			
			bestScoreFine.getData1dHandle()[kk] = -1;
			bestCoordFine.getData1dHandle()[kk].x = txpos_subject; bestCoordFine.getData1dHandle()[kk].y = typos_subject; bestCoordFine.getData1dHandle()[kk].z = tzpos_subject;
		}
		
		for (k=tzpos_subject-8; k<=tzpos_subject+8; k+=2)
		{
			if (k<0 || k>=img_subject->sz2()) continue; 
			for (j=typos_subject-R_search; j<=typos_subject+R_search; j+=4)
			{
				if (j<0 || j>=img_subject->sz1()) continue; 
				for (i=txpos_subject-R_search; i<=txpos_subject+R_search; i+=4)
				{
					if (i<0 || i>=img_subject->sz0()) continue; 
                    
					if (b_use_subject_mrange) 
					{
						if (i<srange3d[k][j][0] || i>srange3d[k][j][1])
							continue;
					}
					
#ifdef MATCH_MODEL_PRIORS
					cur_model_force_score = 0;
					if (priorTargetPos.size()>1)
					{
						Coord3D_JBA tmpLMP; double tmp_kk, tmp_jj, tmp_ii, tmp_wei, tmp_normalizer=0;
						for (V3DLONG itmp=0;itmp<priorTargetPos.size(); itmp++) 
						{
							if (itmp==n) continue;
							tmpLMP = priorTargetPos.at(itmp);
							tmp_kk=(tmpLMP.z-k); tmp_kk*=tmp_kk;
							tmp_jj=(tmpLMP.y-j); tmp_jj*=tmp_jj;
							tmp_ii=(tmpLMP.x-i); tmp_ii*=tmp_ii;
							tmp_wei=exp(-dis_prior[n][itmp]/100);
							cur_model_force_score += tmp_wei * fabs((sqrt(tmp_ii + tmp_jj + tmp_kk) - dis_prior[n][itmp]))/dis_prior[n][itmp];
							tmp_normalizer += tmp_wei;
						}
						cur_model_force_score = exp(-cur_model_force_score/tmp_normalizer);
					}
#endif
					
					cmpFeaturesonTwoImages(img_target, txpos_target, typos_target, tzpos_target, img_subject, i, j, k, R_compare, mMethod, &cur_image_force_score_matching); 					
					for (kk=0;kk<N_USE_METHOD;kk++)
					{
						cur_image_force_score = cur_image_force_score_matching.getData1dHandle()[kk];
						
#ifdef MATCH_MODEL_PRIORS
						cur_score = cur_image_force_score * cur_model_force_score;
#else
						cur_score = cur_image_force_score;
#endif
						
						if (cur_score > bestScoreCoarse.getData1dHandle()[kk])
						{
							bestScoreCoarse.getData1dHandle()[kk] = cur_score;
							bestCoordCoarse.getData1dHandle()[kk].x = i; bestCoordCoarse.getData1dHandle()[kk].y = j; bestCoordCoarse.getData1dHandle()[kk].z = k;
						}
					}
				}
			}
		}
		
		for (kk=0;kk<N_USE_METHOD; kk++)
		{
			bestCoordFine.getData1dHandle()[kk].x = bestCoordCoarse.getData1dHandle()[kk].x;
			bestCoordFine.getData1dHandle()[kk].y = bestCoordCoarse.getData1dHandle()[kk].y;
			bestCoordFine.getData1dHandle()[kk].z = bestCoordCoarse.getData1dHandle()[kk].z;
			bestScoreFine.getData1dHandle()[kk] = bestScoreCoarse.getData1dHandle()[kk];
		}
		
		int xy_fine_radius=6;
		
		for (kk=0; kk<N_USE_METHOD; kk++)
		{
			PointMatchMethodType fineSearch_mMethod;
			if (N_USE_METHOD==1) fineSearch_mMethod = mMethod;
			else
			{
				if (kk==0) fineSearch_mMethod = MATCH_INTENSITY;
				else if (kk==1) fineSearch_mMethod = MATCH_MI;
				else if (kk==2) fineSearch_mMethod = MATCH_CORRCOEF;
				else fineSearch_mMethod = MATCH_INTENSITY;
			}
			
			Vector1DSimple<double> fineSearch_image_score_matching(1);
			
			for (k=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].z-3; k<=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].z+3; k+=Step_Search_fine)
			{
				if (k<0 || k>=img_subject->sz2()) continue; 
				for (j=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].y-xy_fine_radius; j<=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].y+xy_fine_radius; j+=Step_Search_fine)
				{
					if (j<0 || j>=img_subject->sz1()) continue; 
					for (i=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].x-xy_fine_radius; i<=(V3DLONG)bestCoordCoarse.getData1dHandle()[kk].x+xy_fine_radius; i+=Step_Search_fine)
					{
						if (i<0 || i>=img_subject->sz0()) continue; 
						if (b_use_subject_mrange) 
						{
							if (i<srange3d[k][j][0] || i>srange3d[k][j][1])
								continue;
						}
						
#ifdef MATCH_MODEL_PRIORS
						cur_model_force_score = 0;
						if (priorTargetPos.size()>1)
						{
							Coord3D_JBA tmpLMP; double tmp_kk, tmp_jj, tmp_ii, tmp_wei, tmp_normalizer=0;
							for (V3DLONG itmp=0;itmp<priorTargetPos.size(); itmp++) 
                            {
								if (itmp==n) continue;
								tmpLMP = priorTargetPos.at(itmp);
								tmp_kk=(tmpLMP.z-k); tmp_kk*=tmp_kk;
								tmp_jj=(tmpLMP.y-j); tmp_jj*=tmp_jj;
								tmp_ii=(tmpLMP.x-i); tmp_ii*=tmp_ii;
								tmp_wei=exp(-dis_prior[n][itmp]/100);
								cur_model_force_score += tmp_wei * fabs((sqrt(tmp_ii + tmp_jj + tmp_kk) - dis_prior[n][itmp]))/dis_prior[n][itmp];
								tmp_normalizer += tmp_wei;
							}
							cur_model_force_score = exp(-cur_model_force_score/tmp_normalizer);
						}
#endif
						
						cur_image_force_score = cmpFeaturesonTwoImages(img_target, txpos_target, typos_target, tzpos_target, img_subject, i, j, k, R_compare, fineSearch_mMethod, &fineSearch_image_score_matching); 
						
#ifdef MATCH_MODEL_PRIORS
						cur_score = cur_image_force_score * cur_model_force_score;
#else
						cur_score = cur_image_force_score;
#endif
						
						if (cur_score > bestScoreFine.getData1dHandle()[kk])
						{
							bestScoreFine.getData1dHandle()[kk] = cur_score;
							bestCoordFine.getData1dHandle()[kk].x = i; bestCoordFine.getData1dHandle()[kk].y = j; bestCoordFine.getData1dHandle()[kk].z = k;
						}
					}
				}
			}
		}
		
		bool b_add_to_list=true;
		Coord3D_JBA optimal_subject_point;
		
		double c_radius = 5.0/512*img_target->maxdimsz();
		
		if (N_USE_METHOD==1)
		{
			optimal_subject_point = bestCoordFine.getData1dHandle()[0];
		}
		else
		{
			int kk1,kk2;
			b_add_to_list=true;
			for (kk=0;kk<N_USE_METHOD; kk++)
			{
				for (kk1=kk+1;kk1<N_USE_METHOD;kk1++)
				{
					double tmpdx = bestCoordFine.getData1dHandle()[kk].x - bestCoordFine.getData1dHandle()[kk1].x;
					double tmpdy = bestCoordFine.getData1dHandle()[kk].y - bestCoordFine.getData1dHandle()[kk1].y;
					double tmpdz = bestCoordFine.getData1dHandle()[kk].z - bestCoordFine.getData1dHandle()[kk1].z;
					
					if (sqrt(tmpdx*tmpdx + tmpdy*tmpdy + tmpdz*tmpdz) > c_radius)
					{
						b_add_to_list=false;
						break;
					}
				}
				if (b_add_to_list==false)
					break;
			}
			
			if (b_add_to_list==true)
			{
				kk1=0; double tmp_best_score=-1, tmp_score; V3DLONG tmp_ind;
				for (kk=kk1;kk<N_USE_METHOD;kk++)
				{
					cmpFeaturesonTwoImages(img_target, txpos_target, typos_target, tzpos_target,
										   img_subject, V3DLONG(bestCoordFine.getData1dHandle()[kk].x), V3DLONG(bestCoordFine.getData1dHandle()[kk].y), V3DLONG(bestCoordFine.getData1dHandle()[kk].z),
										   R_compare, mMethod, &cur_image_force_score_matching);
					tmp_score = 1;
					for (kk2=0;kk2<N_USE_METHOD; kk2++)
						tmp_score *= cur_image_force_score_matching.getData1dHandle()[kk2];
					
					if (tmp_score>tmp_best_score)
					{
						tmp_ind = kk; tmp_best_score = tmp_score;
					}
				}
				optimal_subject_point = bestCoordFine.getData1dHandle()[tmp_ind];
			}
		}
		
		if (b_add_to_list==true)
		{
			printf("target [%ld][%ld][%ld] -> subject[%ld][%ld][%ld]\n",
				   tzpos_target, typos_target, txpos_target,
				   V3DLONG(optimal_subject_point.z), V3DLONG(optimal_subject_point.y), V3DLONG(optimal_subject_point.x));
			
			distMove[n_added] = sqrt((optimal_subject_point.x - txpos_subject)*(optimal_subject_point.x - txpos_subject) +
									 (optimal_subject_point.y - typos_subject)*(optimal_subject_point.y - typos_subject) +
									 (optimal_subject_point.z - tzpos_subject)*(optimal_subject_point.z - tzpos_subject));
			
			matchTargetPos.push_back(priorTargetPos.at(n));
			matchSubjectPos.push_back(optimal_subject_point);
			
			n_added++;
		}
		else
		{
			printf("...... not added due to inconsistency of various matching scores.\n");
		}
	}
	
	Matrix x4x4_affinematrix_2to1;
	vector<int> arr1d_1to2index;
	vector<Coord3D_PCR> arr_2_invp, arr_1_afterkill, arr_2_afterkill, arr_1, arr_2;
	
	for (i=0;i<matchTargetPos.size(); i++)
	{
		Coord3D_PCR tt(matchTargetPos.at(i).x, matchTargetPos.at(i).y, matchTargetPos.at(i).z), 
		            ss(matchSubjectPos.at(i).x, matchSubjectPos.at(i).y, matchSubjectPos.at(i).z);
		
		arr_1.push_back(tt);
		arr_2.push_back(ss);
	}

	int n_sampling		=1000;
	int n_pairs_per_sampling=5;	
	float f_kill_factor=2.0;
	
	if(!q_killwrongmatch(arr_1,arr_2,n_sampling,n_pairs_per_sampling,f_kill_factor,
						 x4x4_affinematrix_2to1,arr1d_1to2index,arr_2_invp,
						 arr_1_afterkill,arr_2_afterkill))
	{
		fprintf(stderr,"ERROR: q_killwrongmatch return false! \n");
		return false;
	}
	fprintf(stdout, "\tkill %d pairs, %d pairs left!\n",arr_1.size()-arr_1_afterkill.size(),arr_1_afterkill.size());

	matchTargetPos.clear();
	matchSubjectPos.clear();
	for (i=0;i<arr_1_afterkill.size(); i++)
	{
		Coord3D_JBA tt, ss;
		tt.x = arr_1_afterkill.at(i).x; tt.y = arr_1_afterkill.at(i).y; tt.z = arr_1_afterkill.at(i).z;
		ss.x = arr_2_afterkill.at(i).x; ss.y = arr_2_afterkill.at(i).y; ss.z = arr_2_afterkill.at(i).z;
		matchTargetPos.push_back(tt);
		matchSubjectPos.push_back(ss);
	}	
	
	analyze_model_matching_smoothness(matchTargetPos, matchSubjectPos, matchScore);
	matchScore.method_inconsistency = 1.0-double(matchTargetPos.size())/priorTargetPos.size(); 
	
	bool b_use_dist_filtering=false; 
	
	if (b_use_dist_filtering==true)
	{
		double distMean, distStd;
		if (mean_and_std(distMove, n_added, distMean, distStd)==false)
		{
			fprintf(stderr, "Sth wrong in the mean_and_std() function.\n"); 
		}
		else
		{
			printf("** Remove probable wrong matching points: mean=%5.3f \t std=%5.3f\n", distMean, distStd);
			double t_thres=2;
			for (n=n_added-1; n>=0; n--)
			{
				if (distMove[n] > distMean + t_thres*distStd)
				{
					printf("Remove target [%ld][%ld][%ld] -> subject[%ld][%ld][%ld] dist[%ld]=%5.3f > %5.3f\n",
						   V3DLONG(matchTargetPos.at(n).z), V3DLONG(matchTargetPos.at(n).y), V3DLONG(matchTargetPos.at(n).x),
						   V3DLONG(matchSubjectPos.at(n).z), V3DLONG(matchSubjectPos.at(n).y), V3DLONG(matchSubjectPos.at(n).x),
						   n, distMove[n], double(Step_Search_coarse) 
						   );
					
					matchTargetPos.erase(matchTargetPos.begin()+n);
					matchSubjectPos.erase(matchSubjectPos.begin()+n);
				}
				else
				{
					printf("\tKeep dist[%ld]=%5.3f < %5.3f\n",  n, distMove[n], distMean+t_thres*distStd  );
				}
			}
		}
	}
	if (distMove) {delete []distMove; distMove=0;}
	
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
	
	return b_noerror;
}

bool analyze_model_matching_smoothness(vector<Coord3D_JBA> & matchTargetPos,
									   vector<Coord3D_JBA> & matchSubjectPos, PointMatchScore & matchScore)
{
	if (matchTargetPos.size()<=0 || matchTargetPos.size()!=matchSubjectPos.size())
	{
		fprintf(stderr, "Illeagal size of the matchTargetPos or matchSubjectPos lists in function analyze_model_matching_smoothness().\n");
		return false;
	}
	
	unsigned V3DLONG i,j,k,n;
	
	double *cc = new double [matchTargetPos.size()];
	if (!cc) {fprintf(stderr, "Fail to allocate memory in analyze_model_matching_smoothness().\n"); return false;}
	
	for (k=0;k<matchTargetPos.size(); k++) cc[k]=0;
	
	const double TH_TRIANGLE_EAGE_LEN=50;
	
	double b_dist_target, b_dist_subject; 
    for (k=0;k<matchTargetPos.size(); k++)
	{
		Coord3D_JBA pt_t_1 = matchTargetPos.at(k);
		Coord3D_JBA pt_s_1 = matchSubjectPos.at(k);
		
		for (j=k+1;j<matchTargetPos.size(); j++)
		{
			Coord3D_JBA pt_t_2 = matchTargetPos.at(j);
			Coord3D_JBA pt_s_2 = matchSubjectPos.at(j);
			
			double dist12 = compute_dist_two_pts(pt_t_1, pt_t_2);
			if (dist12>TH_TRIANGLE_EAGE_LEN)
				continue;
			
			for (i=j+1;i<matchTargetPos.size(); i++)
			{
				Coord3D_JBA pt_t_3 = matchTargetPos.at(i);
				Coord3D_JBA pt_s_3 = matchSubjectPos.at(i);
				
				double dist13 = compute_dist_two_pts(pt_t_3, pt_t_1);
				if (dist13>TH_TRIANGLE_EAGE_LEN)
					continue;
				
				double dist23 = compute_dist_two_pts(pt_t_3, pt_t_2);
				if (dist23>TH_TRIANGLE_EAGE_LEN)
					continue;
				
				double angle12 = compute_angle_triangle_edge(dist12, dist13, dist23);
				if (angle12<20 || angle12>145)
				{
					printf("angle12=%5.3f [tedge len=%5.3f e1=%5.3f, e2=%5.3f]  - drop this plane\n", angle12, dist12, dist23, dist13);
					continue;
				}
				
				double angle23 = compute_angle_triangle_edge(dist23, dist13, dist12);
				if (angle23<20 || angle23>145)
				{
					printf("angle23=%5.3f [tedge len=%5.3f e1=%5.3f, e2=%5.3f]  - drop this plane\n", angle23, dist23, dist13, dist12);
					continue;
				}
				
				double angle13 = compute_angle_triangle_edge(dist13, dist23, dist12);
				if (angle13<20 || angle13>145)
				{
					printf("angle13=%5.3f [tedge len=%5.3f e1=%5.3f, e2=%5.3f]  - drop this plane\n", angle13, dist13, dist12, dist23);
					continue;
				}
				
				for (n=0;n<matchTargetPos.size();n++)
				{
					if (n==i || n==j || n==k) continue;
					
					Coord3D_JBA pt_t_n = matchTargetPos.at(n);
					Coord3D_JBA pt_s_n = matchSubjectPos.at(n);
					
					try{ 
						b_dist_target = compute_dist_pt_plane(pt_t_1, pt_t_2, pt_t_3, pt_t_n);
						b_dist_subject = compute_dist_pt_plane(pt_s_1, pt_s_2, pt_s_3, pt_s_n);
					}
					catch (...)
					{
						printf("Detect an exception in computing the dist_pt_plane. Ignore this point.\n");
						continue;
					}
				
					double Dth=0;
					if ((b_dist_target>Dth && b_dist_subject<-Dth) || (b_dist_target<-Dth && b_dist_subject>Dth))
					{
						if (cc[n] < fabs(b_dist_target-b_dist_subject))
							cc[n] = fabs(b_dist_target-b_dist_subject); 
					}
				}
				
			}
		}
	}
	printf("=============================\n");
	
	double ccMean, ccStd;
	mean_and_std(cc, matchTargetPos.size(), ccMean, ccStd);
	for (k=0;k<matchTargetPos.size(); k++)
	{
		printf("cc[%ld]=%5.3f\t target=[%ld,%ld,%ld] subject=[%ld,%ld,%ld]\n", k+1, cc[k],
			   V3DLONG(matchTargetPos.at(k).x), V3DLONG(matchTargetPos.at(k).y), V3DLONG(matchTargetPos.at(k).z),
			   V3DLONG(matchSubjectPos.at(k).x), V3DLONG(matchSubjectPos.at(k).y), V3DLONG(matchSubjectPos.at(k).z));
	}
	printf("Overall mean=%5.3f \t std=%5.3f\n", ccMean, ccStd);
	
	matchScore.model_violation = ccMean; 
    if (cc) {delete []cc; cc=0;}
	
	return true;
}

double compute_dist_two_pts(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2)
{
	double x=pt1.x-pt2.x, y=pt1.y-pt2.y, z=pt1.z-pt2.z;
	return sqrt(x*x+y*y+z*z);
}

double compute_angle_triangle_edge(double e_target, double e1, double e2)
{
	return acos((e1*e1+e2*e2-e_target*e_target)/(2*e1*e2))/3.141592653589793*180.0;
}

double compute_dist_pt_plane(const Coord3D_JBA & pt1, const Coord3D_JBA &  pt2, const Coord3D_JBA &  pt3, const Coord3D_JBA &  pt_n)
{
	Matrix AA(3,3); ColumnVector b(3);
	AA(1,1) = pt1.x; AA(1,2) = pt1.y; AA(1,3) = pt1.z;
	AA(2,1) = pt2.x; AA(2,2) = pt2.y; AA(2,3) = pt2.z;
	AA(3,1) = pt3.x; AA(3,2) = pt3.y; AA(3,3) = pt3.z;
	
	Real D = AA.determinant();
	
	b=-D;
    
	ColumnVector y = AA.i() * b;
	
	double A=y(1), B=y(2), C=y(3);
	return (A*pt_n.x + B*pt_n.y + C*pt_n.z + D) / sqrt(A*A+B*B+C*C);
}

