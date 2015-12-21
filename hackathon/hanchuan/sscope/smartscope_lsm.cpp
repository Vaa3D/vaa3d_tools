// smartscope_lsm.cpp
// the Laser scanning code for the smartscope project
// by Hanchuan Peng
// 20110516
// 2011-Oct-06: Add acquire_single_direction() to acquire the image in single direction. By ZJL
// 2011-Oct-17: Add createSimImg() to create a simulation image for testing. By ZJL
// 2011-Oct-17: Add do_image_acc() for imaging with single direction, no zigzag. By ZJL
// 2011-Oct-17: Add parameter "scanMethod" to control scanning method: single, zigzag_acc. By ZJL
// 2011-Oct-21: Add bool "bUseSimData" to control whether using simulation data for testing. By ZJL

#include "smartscope_lsm.h"
#include "smartscope_gui.h"
#include "smartscope_util.h"
#include "stackutil.h"
#include "smartscope_ip.h"

#include <cstdio>

// roi imaging
#include "customary_structs/v3d_imaging_para.h"

#define FIND_DEBUG 1 // macro for recording debug info

#define USE_PRIOR_CORRECTION 1 // macro for deciding whether using correction before scanning

#define PRINT_POS_LUT(filename){ \
     FILE *fp=fopen(filename, "wt"); \
     for(V3DLONG i=0; i<sx; i++) \
     { \
         fprintf(fp, "%d to %d\n", exp_to_real_pos_LUT[i], i); \
     } \
     fclose(fp); \
}


const QString ssConfigFolder = "C:/smartscope_conf/";

void LaserScanImaging::initializeData()
{
	b_reset_PHC = false;

	pImFloat = NULL;
	xa = ya = za = NULL;
	xacc_forward = xacc_backward = NULL;

	pAO = NULL;
	pCTR0 = NULL; pCTR1 = NULL;
	pAI0 = NULL; pAI1 = NULL;
	pAOcx = NULL; pAOcy = NULL;

	pCTR_Out0 = NULL; pCTR_Out1 = NULL;

	flag = NULL;
	fl_count = 0; fl_col = 0;

	pAIx = NULL; pAIy = NULL;
	pAI = NULL; pAIxy = NULL;

	b_scanner_correction = false; //this should be moved to parameters in the future

	scanning_offset_x = 0; // for interactive ROI

    scanMethod = 0; //single direction with acc
	b_useSimImg = 0; //use simulation data
	b_scan_z_reverse = 0; //z from 0 to max


    real_to_exp_pos_LUT = NULL;
    exp_to_real_pos_LUT = NULL;

	//some stupid initialization
    old_sz = -1;
    old_min_fovx = old_max_fovx = old_min_fovy = old_max_fovy = old_fov_step_x = old_fov_step_y = 0;
    cal_offset_left = cal_offset_right = offset_left = offset_right = 0;
	real_bb_start_x = real_bb_end_x = real_sx =0;
	real_min_fovx =real_max_fovx=0;
}

void LaserScanImaging::cleanData(bool b_cleanLUT)
{
	if(xa) {delete []xa; xa=0;}
	if(ya) {delete []ya; ya=0;}
	if(za) {delete []za; za=0;}

	if(xacc_forward) {delete []xacc_forward; xacc_forward=0;}
	if(xacc_backward) {delete []xacc_backward; xacc_backward=0;}

	if(pAO) {delete []pAO; pAO=0;}
	if(pCTR0) {delete []pCTR0; pCTR0=0;}
	if(pCTR1) {delete []pCTR1; pCTR1=0;}

	if(pCTR_Out0) {delete []pCTR_Out0; pCTR_Out0=0;}
	if(pCTR_Out1) {delete []pCTR_Out1; pCTR_Out1=0;}

	if(flag) {delete []flag; flag=0;}

	if(pAI0) {delete []pAI0; pAI0=0;}
	if(pAI1) {delete []pAI1; pAI1=0;}
	if(pAIxy) {delete []pAIxy; pAIxy=0;}

	if(pAIx) {delete []pAIx; pAIx=0;}
	if(pAIy) {delete []pAIy; pAIy=0;}
	if(pAOcx) {delete []pAOcx; pAOcx=0;}
	if(pAOcy) {delete []pAOcy; pAOcy=0;}

	// temprarily kept for non-single scan functions
	if(pAI) {delete []pAI; pAI=0;}

	if(b_cleanLUT==true)
	{
		if(real_to_exp_pos_LUT) {delete []real_to_exp_pos_LUT; real_to_exp_pos_LUT=0;}
		if(exp_to_real_pos_LUT) {delete []exp_to_real_pos_LUT; exp_to_real_pos_LUT=0;}
	}
}

LaserScanImaging::LaserScanImaging(V3DPluginCallback &callback)
{
	initializeData(); //set all pointers to 0 and setup other variables
	cb = &callback; //this is the place for callback, could be useful later for 3D direct imaging
}

void LaserScanImaging::printParameters()
{
	FILE *fp=fopen("c://test_h2.txt", "wt");
	fprintf(fp, "dev_x = %d\n", dev_x);
	fprintf(fp, "dev_y = %d\n", dev_y);
	fprintf(fp, "dev_z = %d\n", dev_z);
	fprintf(fp, "ao_x = %d\n", ao_x);
	fprintf(fp, "ao_y = %d\n", ao_y);
	fprintf(fp, "ao_z = %d\n", ao_z);
	fprintf(fp, "dev_red = %d\n", dev_red);
	fprintf(fp, "red_n_ai = %d\n", red_n_ai);
	fprintf(fp, "red_n_ctr = %d\n", red_n_ctr);
	fprintf(fp, "dev_green = %d\n", dev_green);
	fprintf(fp, "green_n_ai = %d\n", green_n_ai);
	fprintf(fp, "green_n_ctr = %d\n", green_n_ctr);
	fprintf(fp, "dev_clk = %d\n", dev_clk);
	fprintf(fp, "clk_n = %d\n", clk_n);
	fprintf(fp, "dev_timing = %d\n", dev_timing);
	fprintf(fp, "timing_n = %d\n", timing_n);

	fprintf(fp, "time_out = %d\n", time_out);
	fprintf(fp, "scanning_rate = %d\n", scanning_rate);

	fprintf(fp, "min_fovx = %7.3f\n", min_fovx);
	fprintf(fp, "max_fovx = %7.3f\n", max_fovx);
	fprintf(fp, "min_fovy = %7.3f\n", min_fovy);
	fprintf(fp, "max_fovy = %7.3f\n", max_fovy);
	fprintf(fp, "min_fovz = %7.3f\n", min_fovz);
	fprintf(fp, "max_fovz = %7.3f\n", max_fovz);
	fprintf(fp, "ratio_x = %7.3f\n", ratio_x);
	fprintf(fp, "ratio_y = %7.3f\n", ratio_y);
	fprintf(fp, "ratio_z = %7.3f\n", ratio_z);

	fprintf(fp, "sx = %ld\n", sx);
	fprintf(fp, "sy = %ld\n", sy);
	fprintf(fp, "sz = %ld\n", sz);

	fprintf(fp, "fov_step_x = %7.3f\n", fov_step_y);
	fprintf(fp, "fov_step_y = %7.3f\n", fov_step_y);
	fprintf(fp, "fov_step_z = %7.3f\n", fov_step_z);

	fprintf(fp, "start_x = %ld\n", start_x);
	fprintf(fp, "start_y = %ld\n", start_y);
	fprintf(fp, "end_x = %ld\n", end_x);
	fprintf(fp, "end_y = %ld\n", end_y);

	fprintf(fp, "a_lsmbf = %7.3f\n", a_lsmbf);
	fprintf(fp, "d_lsmbf = %7.3f\n", d_lsmbf);
	fprintf(fp, "e_lsmbf = %7.3f\n", e_lsmbf);
	fprintf(fp, "f_lsmbf = %7.3f\n", f_lsmbf);

	fprintf(fp, "a_bflsm = %7.3f\n", a_bflsm);
	fprintf(fp, "d_bflsm = %7.3f\n", d_bflsm);
	fprintf(fp, "e_bflsm = %7.3f\n", e_bflsm);
	fprintf(fp, "f_bflsm = %7.3f\n", f_bflsm);

	fprintf(fp, "aictr_red = %d\n", aictr_red);
	fprintf(fp, "aictr_green = %d\n", aictr_green);
	fprintf(fp, "ch = %d\n", ch);
	fprintf(fp, "marker_sorted = %d\n", marker_sorted);

	fprintf(fp, "sc = %d\n", sc);
	fprintf(fp, "duty_cycle = %d\n", duty_cycle);

	fprintf(fp, "scanMethod = %d\n", scanMethod);
	fprintf(fp, "b_useSimImg = %d\n", b_useSimImg);

	fprintf(fp, "b_scan_z_reverse = %d\n", b_scan_z_reverse);

	fclose(fp);
}

bool LaserScanImaging::setup_parameters(QString para_file)
{
	Parameters_LSM para_lsm;

	QString m_FileName;
	if (para_file.isEmpty() || para_file.trimmed().size()<=0)
		m_FileName = "c://smartscope_conf//mi_configuration.txt";
	else
		m_FileName = para_file;

	if (!QFile(m_FileName).exists())
	{
		v3d_msg(QString("Not found the configuration file [%1]. Do nothing.\n").arg(m_FileName));
		return false;
	}
	if (!para_lsm.load_ParameterFile(m_FileName))
	{
		v3d_msg(QString("Fail to load the configuration file [%1]. Do nothing.\n").arg(m_FileName));
		return false;
	}

	dev_x = para_lsm.DEVICE_X;
	dev_y = para_lsm.DEVICE_Y;
	dev_z = para_lsm.DEVICE_Z;
	ao_x = para_lsm.AO_X;
	ao_y = para_lsm.AO_Y;
	ao_z = para_lsm.AO_Z;
	aictr_red = para_lsm.AI_COUNTER_RED;
	aictr_green = para_lsm.AI_COUNTER_GREEN;
	dev_red_ai = para_lsm.DEVICE_RED_AI;
	dev_red_ctr = para_lsm.DEVICE_RED_CTR;
	red_n_ai = para_lsm.PORT_RED_AI;
	red_n_ctr = para_lsm.PORT_RED_CTR;

	dev_green_ai = para_lsm.DEVICE_GREEN_AI;
	dev_green_ctr = para_lsm.DEVICE_GREEN_CTR;
	green_n_ai = para_lsm.PORT_GREEN_AI;
	green_n_ctr = para_lsm.PORT_GREEN_CTR;
	dev_clk = para_lsm.DEVICE_CLOCK;
	clk_n = para_lsm.PORT_CLOCK;
	dev_timing = para_lsm.DEVICE_TIMING;
	timing_n = para_lsm.PORT_TIMING;

	time_out = para_lsm.TIMEOUT;
	scanning_rate = para_lsm.SCANNING_RATE;
	min_fovx = para_lsm.FOVX_MIN;
	max_fovx = para_lsm.FOVX_MAX;
	min_fovy = para_lsm.FOVY_MIN;
	max_fovy = para_lsm.FOVY_MAX;
	min_fovz = para_lsm.FOVZ_MIN;
	max_fovz = para_lsm.FOVZ_MAX;
	ratio_x = para_lsm.RATIO_X;
	ratio_y = para_lsm.RATIO_Y;
	ratio_z = para_lsm.RATIO_Z;
	sx = para_lsm.SX;
	sy = para_lsm.SY;
	sz = para_lsm.SZ;

	start_x = para_lsm.STARTX;
	start_y = para_lsm.STARTY;
	end_x = para_lsm.ENDX;
	end_y = para_lsm.ENDY;

	a_lsmbf = para_lsm.LSM_TO_BF_A;
	d_lsmbf = para_lsm.LSM_TO_BF_D;
	e_lsmbf = para_lsm.LSM_TO_BF_E;
	f_lsmbf = para_lsm.LSM_TO_BF_F;

	a_bflsm = para_lsm.BF_TO_LSM_A;
	d_bflsm = para_lsm.BF_TO_LSM_D;
	e_bflsm = para_lsm.BF_TO_LSM_E;
	f_bflsm = para_lsm.BF_TO_LSM_F;

	ch = para_lsm.CHANNEL;
	marker_sorted = para_lsm.MARKER_SORTED;

	fov_step_x = para_lsm.FOVX_STEP;
	fov_step_y = para_lsm.FOVY_STEP;
	fov_step_z = para_lsm.FOVZ_STEP;

	sc = (ch<2)?1:2;
	duty_cycle = para_lsm.DUTY_CYCLE;

	scanMethod = para_lsm.SCAN_METHOD;
	b_useSimImg = para_lsm.USE_SIMIMG;
	b_scan_z_reverse = para_lsm.SCAN_Z_REVERSE;

	//printParameters();

	return true;
}


void LaserScanImaging::acquire() //by PHC, 20110523
{
	//do_init(); //commented on 20110529

	if (!setup_parameters()) //as of now will cause crash, 20110529
		return;

	set_voltages();

	if(b_useSimImg == 0)
		shutter_open();

	qDebug()<<"WAIT_PIXELS.......:"<<WAIT_PIXELS;

	if(imaging_op==0)
	{
		set_mi_zigzag_acc();

		// imaging
		int start_t = clock();

		for(k=0; k<sz; k++)
		{
			long offset_k = k*planesz;

			qDebug()<<"..."<<k+1<<"/"<<sz; // showing current

			for(j=0; j<sy; j++)
			{
				V3DLONG offset = j*sxsw; // saw-tooth

				V3DLONG offset_x = offset;
				V3DLONG offset_y = planesz + offset;

				if(j%2==0)
				{
					for(i=0; i<WAIT_PIXELS; i++)
					{
						pAO[offset_x + i] = xacc_forward[i];
						pAO[offset_y + i] = ya[j];
					}

					V3DLONG xend_2nd = WAIT_PIXELS + sx;

					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						pAO[offset_x + i] = xa[i-WAIT_PIXELS];
						pAO[offset_y + i] = ya[j];
					}

				}
				else
				{
					for(i=0; i<WAIT_PIXELS; i++)
					{
						pAO[offset_x + i] = xacc_backward[WAIT_PIXELS - 1 - i];
						pAO[offset_y + i] = ya[j];
					}

					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						pAO[offset_x + i] = xa[sx-1 -(i-WAIT_PIXELS)];
						pAO[offset_y + i] = ya[j];
					}

				}

			}

			//imaging
			try
			{
				if(b_useSimImg == 0)
					DAQ_UpdateZ(za[k], dev_z, ao_z); // z

				int success;
				if(b_useSimImg ==0)
				{
					if(aictr==0)
						success = DAQ_getImg_ai(pAI, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
					else if(aictr!=0)
						success = DAQ_getImg_ctr(pCTR0+offset_k, pCTR1+offset_k, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
				}else if(b_useSimImg ==1)
				{
					if(aictr==0)
					{
						v3d_msg("Simulation imaging is only available for aictr=1.\n", 0);
						return;
					}
					else if(aictr!=0)// using simulation image
						success = DAQ_getImg_sim(pCTR0, pCTR1, pAI0, pAI1, ch, sz_ctr, sx, sy, sz, planesz, sxsw, WAIT_PIXELS);
				}

				if(success)
				{
					v3d_msg("Failed in microscope imaging.\n", 0);
					return;
				}

				wait(WAITING_PERSLICE); // every k slice
			}
			catch (...)
			{
				v3d_msg("Invoking DAQ_getImg failed.\n", 0);
				return;
			}

			V3DLONG offsets_k = k*sx*sy;

			// ai x, y
			if(b_scanner_correction)
			{
				for(j=0; j<sy; j++)
				{
					V3DLONG offset = j*sxsw;
					V3DLONG offsets = j*sx + offsets_k;
					if(j%2==0) // forward
					{
						for(i=WAIT_PIXELS; i<sxsw; i++)
						{
							//assign
							V3DLONG idx = offset + i;
							V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

							// ai x y
							pAIx[idx_ori] = pAIxy[idx];
							pAIy[idx_ori] = pAIxy[idx + planesz];

							pAOcx[idx_ori] = pAO[idx];
							pAOcy[idx_ori] = pAO[idx + planesz];
						}
					}
					else // backward
					{
						for(i=WAIT_PIXELS; i<sxsw; i++)
						{
							//assign
							V3DLONG idx = offset + i;
							V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

							// ai x y
							pAIx[idx_ori] = pAIxy[idx];
							pAIy[idx_ori] = pAIxy[idx + planesz];

							pAOcx[idx_ori] = pAO[idx];
							pAOcy[idx_ori] = pAO[idx + planesz];
						}
					}
				}
			}

			// ai data
			if(aictr==0)
			{
				if(ch==0)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 0
								pImFloat[idx_ori] = pAI[idx];
							}
						}
					}
				}
				else if(ch==1)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else if(ch==2)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 0
								pImFloat[idx_ori] = pAI[idx];

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else
				{
					cout<<"wrong channel number when closing."<<endl;
					return;
				}
			}

		} // for k

		size_t end_t = clock();
		printf("time elapse ... %d \n", end_t-start_t);



		// assign back ctr
		if(aictr!=0)
		{
			if(ch==0)
			{
				double max_v=0, min_v=INF;

				// green ctr1
				// readjust input orders
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR1[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
						else
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								float tmp = pCTR1[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==1)
			{
				double max_v=0, min_v=INF;

				// red ctr0
				// readjust input orders
				V3DLONG i,j,k;
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
						else
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==2)
			{
				double max_v_red=0, min_v_red=INF;
				double max_v_green=0, min_v_green=INF;

				// readjust input orders
				// photon counting - red ctr0 & green ctr1
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw + offset_k;
						V3DLONG offsets = j*sx + offsets_k;
						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ctr0
								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v_red<tmp) max_v_red = tmp;
								if(min_v_red>tmp) min_v_red = tmp;

								// ctr1
								float tmp1 = pCTR1[idx];

								pImFloat[idx_ori+offset_image] = tmp1;

								if(max_v_green<tmp1) max_v_green = tmp1;
								if(min_v_green>tmp1) min_v_green = tmp1;

							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ctr0
								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v_red<tmp) max_v_red = tmp;
								if(min_v_red>tmp) min_v_red = tmp;

								// ctr1
								float tmp1 = pCTR1[idx];

								pImFloat[idx_ori+offset_image] = tmp1;

								if(max_v_green<tmp1) max_v_green = tmp1;
								if(min_v_green>tmp1) min_v_green = tmp1;

							}
						}
					}
				}
				qDebug()<<"[max_v_red, min_v_red] ... " << "["<<max_v_red<<","<<min_v_red<<"]";
				qDebug()<<"[max_v_green, min_v_green] ... " << "["<<max_v_green<<","<<min_v_green<<"]";

			}
			else
			{
				cout<<"wrong channel number when closing."<<endl;
				return;
			}
		}

	}
	else if(imaging_op==1)
	{

	}
	else if(imaging_op==2)
	{

	}
	else if(imaging_op==3)
	{

	}
	else if(imaging_op==4)
	{

	}
	else
	{
		cout<<"Undefined imaging operation."<<endl;
		return;
	}

	if(b_useSimImg == 0)
		shutter_close_daq_reset();

	cleanData();
}

bool LaserScanImaging::acquire_single_direction(int op, bool b_from3DViewer) //by ZJL, 20111006, based on acquire()
{
	//do_init(); //commented on 20110529
	if(op==0)    // get full image
	{
		if (!setup_parameters()) //as of now will cause crash, 20110529
			return false;

		// set paras for getting correction offset
          if(USE_PRIOR_CORRECTION == 1)
          {
               make_a_copy_PHC(true);
               set_real_scanning_range_with_real_offset();
          }

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();

		bool succ=do_LSM_single();

          if(USE_PRIOR_CORRECTION == 1)
          {
               make_a_copy_PHC(false);
          }

		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();
		return true;
	}
	else if(op==1) // imaging with ROI_i //this one is still probblematic, so I suggest using 7 now. by PHC, 120506
	{
		bool succ=do_image_withROI_i();
		if(!succ) return false;

		return true;
    }
	else if(op==2) // imaging one slice
	{
		if (!setup_parameters())
			return false;

		min_fovz=30; max_fovz=35;
		sz = 1; // one slice

		// set paras for getting correction offset
          if(USE_PRIOR_CORRECTION == 1)
          {
               make_a_copy_PHC(true);
               set_real_scanning_range_with_real_offset();
          }

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();

		bool succ=do_LSM_single();

          if(USE_PRIOR_CORRECTION == 1)
          {
               make_a_copy_PHC(false);
          }

		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();
		return true;
	}
	else if(op==3) //Focus Controls
	{
          if(USE_PRIOR_CORRECTION == 1)
          {
			if (b_reset_PHC)
			{
				// set paras for getting correction offset
				make_a_copy_PHC(true);
				// laser open and close are performed in SSDriver
				set_real_scanning_range_with_real_offset(); //by PHC, 20120420
				b_reset_PHC=false;
			}
          }
		// laser open and close are performed in SSDriver
		set_voltages();

		if(!set_scanning_buffers())
			return false;

		bool succ=do_LSM_single();
		if(!succ)
			return false;

		return true;
	}
	else if(op==4)// ROI_IMAGING: imaging roi which is chosen from 3d view
	{
		QString para_file = get_roi_scanning_parameters(true, false, 0); // paras from 3d view, not marker
		if (para_file.isEmpty())
			return false;

		setup_parameters(para_file);
		real_min_fovx = min_fovx;
		real_max_fovx = max_fovx;

		min_fovx = -150; //note that if in the future for full-image scan, the -150/150 range change, then this should also change
		max_fovx =  150; //ideally these values should be adjusted based on the image where the ROI was produced

		// set paras for getting correction offset
		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(true);
			set_real_scanning_range_with_real_offset();
		}

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();
		bool succ=do_LSM_single();

		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(false);
		}

		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();

		return true;
	}
	else if(op==5) // AO/AI calibration
	{
		if (!setup_parameters())
			return false;

		if(scanning_rate>2000)
		{
			v3d_msg("Must use slow speed (e.g. <=2000) for calibration!");
			return false;
		}

		sy=4; sz=1; sc =1;// only scan 4 lines, 1 slice

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();

		bool succ=do_LSM_single();
		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();

		return true;
	}
	if(op==6)
	{
		bool succ=do_image_adaptiveROI();
		if(!succ) return false;

		return true;
	}
	else if(op==7) // imaging with ROI_i_x
	{
		QString para_file = get_roi_scanning_parameters(false, false, 0); //from 2d view, not marker
		if (para_file.isEmpty())
			return false;

		setup_parameters(para_file);
		real_min_fovx = min_fovx;
		real_max_fovx = max_fovx;

		min_fovx = -150; //note that if in the future for full-image scan, the -150/150 range change, then this should also change
		max_fovx =  150; //ideally these values should be adjusted based on the image where the ROI was produced

		// set paras for getting correction offset
		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(true);
			set_real_scanning_range_with_real_offset();
		}

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();
		bool succ=do_LSM_single();

		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(false);
		}

		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();
		return true;

		//add post-trim based on real x range

    }
	else if(op==8) // landmarker imaging from 3d viewer
	{
		QString para_file = get_roi_scanning_parameters(true, true, 5); //from 3d view, marker
		if (para_file.isEmpty())
			return false;

		setup_parameters(para_file);
		real_min_fovx = min_fovx;
		real_max_fovx = max_fovx;

		min_fovx = -150; //note that if in the future for full-image scan, the -150/150 range change, then this should also change
		max_fovx =  150; //ideally these values should be adjusted based on the image where the ROI was produced


		// set paras for getting correction offset
		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(true);
			set_real_scanning_range_with_real_offset();
		}

		set_voltages();

		if(!set_scanning_buffers())
			return false;

		// open laser
		if(b_useSimImg == 0) shutter_open();
		bool succ=do_LSM_single();

		if(USE_PRIOR_CORRECTION == 1)
		{
			make_a_copy_PHC(false);
		}

		if(!succ)
			return false;

		if(b_useSimImg == 0) shutter_close_daq_reset();
		return true;

		//post-trim based on real x range in smartscope_control.cpp
    }
	else if(op==9) // ablation by scanning a whole line without feedback
	{
		// load ablation paras from log file
		QString m_FileName_ma = ssConfigFolder + "markerablation_configuration.log";
		if(!QFile(m_FileName_ma).exists())
		{
			m_FileName_ma = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
													  "/Documents", QObject::tr("Image Configuration (*.log)"));
			if(m_FileName_ma.isEmpty())
				return false;
		}

		//int NPULSE;
		//int region_radius;
		int stimulate_waiting;

		StimulatingParas para_abl;
		para_abl.load_ParaFile(m_FileName_ma);

		//NPULSE = para_abl.NPULSE;
		//region_radius = para_abl.REGION_RADIUS;
		scanning_rate = para_abl.SCANNING_RATE; //by PHC, there is also a potential problem and needs real-time calibration, but maybe fine now as the scan rate is just 1000Hz.
		stimulate_waiting = para_abl.WAITING;

		// scanning parameters: this calling sets scanning parameters from the original image
		// and return landmarklist on this image
		LandmarkList marker_list = get_ablation_marker_parameters( b_from3DViewer );

		if(marker_list.size()<1)
		{
			v3d_msg("No marker available. Need at least one marker. Do nothing!", 0);
			return false;
		}

		float originx = min_fovx;
		float originy = min_fovy;
		float originz = min_fovz;
		V3DLONG origin_sx = sx;

		for (int ii=0;ii<marker_list.size(); ii++)
		{
			// first set full x line to get real_to_exp_pos_LUT[]
			max_fovx = originx + origin_sx * fov_step_x;
			min_fovx = -170; //-150-boundary_estimate, so we do not need to compute offset_left...
			min_fovy = originy + (marker_list.at(ii).y - 1) * fov_step_y;
			max_fovy = min_fovy + fov_step_y;
			min_fovz = originz + (marker_list.at(ii).z - 1) * fov_step_z;
			max_fovz = min_fovz + fov_step_z;

            // set new max_fovx and sx using real_to_exp_pos_LUT[] to get expected position
			double marker_x = originx + (V3DLONG)(marker_list.at(ii).x - 1)*fov_step_x; //physical pos of marker_x
			//max_fovx = marker_x;
			sx = (max_fovx - min_fovx)/fov_step_x;
			sy = 1;
			sz = 1;

			double old_min_fovx = min_fovx;
			get_current_scanning_mirror_offset(); // get real_to_exp_pos_LUT[]

			// x index of marker on updated sx
			V3DLONG index_x = (V3DLONG)((marker_x - min_fovx)/fov_step_x);
			V3DLONG real_pos = exp_to_real_pos_LUT[index_x]; // get position number of current marker in the calibrated form
			printf("marker's index_x: %d, real_pos from LUT: %d .\n");
			max_fovx = min_fovx + real_pos * fov_step_x;
			//min_fovx = old_min_fovx - offset_left * fov_step_x;
			sx = (max_fovx - min_fovx)/fov_step_x;

			//v3d_msg(QString(" Marker (x, y, z)= (%1, %2, %3)\n Origin (x, y, z)= (%4, %5, %6)\n Physical pos (x, y, z) = (%7, %8, %9)\n Physical pos after calib (x, y, z) = (%10, %11, %12)")
			//				.arg(marker_list.at(ii).x).arg(marker_list.at(ii).y).arg(marker_list.at(ii).z)
			//				.arg(originx).arg(originy).arg(originz)
			//				.arg(marker_x).arg(max_fovy).arg(max_fovz)
			//				.arg(max_fovx).arg(max_fovy).arg(max_fovz));

			// for real scanning
			set_voltages();
			if(!set_scanning_buffers()) //a waste of resource!
				return false;

			// close shutters for normal laser scanning
			shutter_close_daq_reset(true); //only close shutter, no reset operation

            // scanner run to the ablation position and stimulation shutter is turned on
			bool succ = do_LSM_single(true);

			// print ai_x value before waiting for testing. It is in pAIx. so can be printed here.
			FILE *fp_b=fopen("ai_x_before_ablation.txt", "wt");
			for(V3DLONG i=0; i<sx; i++)
			{
				fprintf(fp_b, "%.6f  \t", pAIx[i]);
			}
			fprintf(fp_b, "\n");
			fclose(fp_b);

			// turn shutter on for ablation
			DAQ_Shutter_Stimulation(true);

            // stimulating waiting time
            wait_milliseconds(stimulate_waiting);

			// get ai_x value at the end of stimulation for testing
			double avg_ai;
			DAQ_get_current_ai(scanning_rate, &avg_ai,0);

			// stimulation shutter off
			DAQ_Shutter_Stimulation(false);

			cleanData();
			if(!succ)
				return false;

			// close and reset scanning shutter
			shutter_close_daq_reset();
		}
	}
	else
	{
		cout <<"Undefined imaging operation." << endl;
		return false;
	}

	return false;
}


void LaserScanImaging::do_init() // parsing parameters
{
	QString m_FileName_ma = ssConfigFolder + "mi_configuration.txt";
	if(!QFile(m_FileName_ma).exists())
	{
		m_FileName_ma = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
												  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(m_FileName_ma.isEmpty())
		{
			v3d_msg("No device configuration file exist. Do nothing!");
			return;
		}
	}

	Parameters_LSM pl;
	pl.load_ParameterFile(m_FileName_ma);
	cout << "logfile name:"<<m_FileName_ma.toStdString()<<endl;

	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;
	min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP; //(max_fovx-min_fovx+1)/sx;
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP; //(max_fovy-min_fovy+1)/sy;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP; //(max_fovz-min_fovz+1)/sz;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse =pl.SCAN_Z_REVERSE;
	sc = (ch<2)?1:2;
	start_x = pl.STARTX; start_y = pl.STARTY; end_x = pl.ENDX; end_y = pl.ENDY;
	duty_cycle = pl.DUTY_CYCLE;
}

V3DLONG LaserScanImaging::waiting_pxls(double scanning_rate) 	/// waiting pixels per line
{
	return ((V3DLONG)( (double)WAIT_TIME_PERLINE*scanning_rate/1000.0) ); // change from 500 to 1000, 2012-04-23
}

void LaserScanImaging::wait( int milliseconds )
{
	clock_t t1=clock();
	while(clock()-t1 < milliseconds){}
}

void LaserScanImaging::set_voltages()
{
	// make sure voltage > 0 along z
	min_z = 0.0;
	min_x = 0.0;
	min_y = 0.0;

	max_x = (max_fovx - min_fovx)/ratio_x;
	max_y = (max_fovy - min_fovy)/ratio_y;
	max_z = (max_fovz - min_fovz)/ratio_z;

	// prevent voltage from overflow
	if(max_x > VOLTAGE_MAX_XY)
		max_x = VOLTAGE_MAX_XY;
	if(max_y > VOLTAGE_MAX_XY)
		max_y = VOLTAGE_MAX_XY;
	if(max_z > VOLTAGE_MAX)
		max_z = VOLTAGE_MAX;

	if(sx>1)
		step_x = max_x/(sx-1);
	else
		step_x = 0;

	if(sy>1)
		step_y = max_y/(sy-1);
	else
		step_y = 0;

	if(sz>1)
		step_z = max_z/(sz-1);
	else
		step_z = 0;

	max_x = (sx-1)*step_x; max_y = (sy-1)*step_y; max_z = (sz-1)*step_z;

	try
	{
		if (!xa) {delete []xa; xa=0;}
		if (!ya) {delete []ya; ya=0;}
		if (!za) {delete []za; za=0;}

		xa = new double [sx];
		ya = new double [sy];
		za = new double [sz];

		double val=min_fovx/ratio_x;

		if(val<-VOLTAGE_MAX)
			val = -VOLTAGE_MAX;

		for(V3DLONG i=0; i<sx; i++)
		{
			xa[i] = val;
			val += step_x;
		}

		val=min_fovy/ratio_y;

		if(val<-VOLTAGE_MAX)
			val = -VOLTAGE_MAX;

		for(V3DLONG i=0; i<sy; i++)
		{
			ya[i] = val;
			val += step_y;
		}

		val=min_fovz/ratio_z;
		if(val<0) val = 0; // make sure volt of z is greater than 0
		for(V3DLONG i=0; i<sz; i++)
		{
			za[i] = val;
			val += step_z;
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for voltage arrays.\n");
		return;
	}
}

void LaserScanImaging::set_mi()
{
	WAIT_PIXELS = waiting_pxls(scanning_rate);
	sxsw = sx + WAIT_PIXELS; // only consider scanning along x direction
	planesz = sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 2*planesz;	// x and y
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		pAO = new double [sz_ao]; // x,y

		pAIx = new float [pagesz]; // feedback x,y
		pAIy = new float [pagesz];

		if(aictr==0) // ai
		{
			pAI = new double [2*sz_ctr];
		}
		else // ctr
		{
			pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
			pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

void LaserScanImaging::set_mi_roi_acc()
{
	WAIT_PIXELS = waiting_pxls(scanning_rate);

	sxsw = sx + WAIT_PIXELS; // only consider scanning along x direction
	//planesz = fl_col*WAIT_PIXELS + fl_count;//old
	planesz = sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		// also use acc same as in full scan //by ZJL 20111031
		xacc_forward = new double [WAIT_PIXELS];

		double vstep = xa[1] - xa[0];
		V3DLONG midWP = WAIT_PIXELS/2;
		xacc_forward[WAIT_PIXELS-1] = xa[0];
		for(i=WAIT_PIXELS-2; i>=midWP; i--)
		{
			xacc_forward[i] =  xacc_forward[i+1] - vstep;
		}
		double vacc = xacc_forward[midWP];
		for(i=1; i<=midWP; i++)
		{
			xacc_forward[i-1] = vacc - 1.0/(double)(i);
		}

		for(i=0; i<WAIT_PIXELS; i++)
		{
			if(xacc_forward[i]<0) xacc_forward[i] = 0;
		}
		// end 20111031


		pAO = new double [sz_ao]; // x,y

		pAIx = new float [pagesz]; // feedback x,y
		pAIy = new float [pagesz];

		pAOcx = new float [pagesz];
		pAOcy = new float [pagesz];

		pAIxy = new double [sz_ao]; // x,y feedback

		if(aictr==0) // ai
		{
			pAI = new double [2*sz_ctr];
		}
		else // ctr
		{
			pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
			pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
		}

		pCTR_Out0 = new unsigned V3DLONG [pagesz];
		pCTR_Out1 = new unsigned V3DLONG [pagesz];

		memset(pCTR_Out0, 0, sizeof(unsigned V3DLONG)*pagesz);
		memset(pCTR_Out1, 0, sizeof(unsigned V3DLONG)*pagesz);
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

void LaserScanImaging::set_mi_focuscontrol()
{
	// 2D image
	sx = 256; sy = 256; sz = 1;
	ch = 2; // two color

	WAIT_PIXELS = waiting_pxls(scanning_rate);
	sxsw = sx + WAIT_PIXELS; // only consider scanning along x direction
	planesz = sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 3*planesz;
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		pAO = new double [sz_ao]; // x,y,z

		pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
		pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

void LaserScanImaging::set_mi_zigzag()
{
	WAIT_PIXELS = waiting_pxls(scanning_rate);
	sxsw = sx + 2*WAIT_PIXELS; // only consider scanning along x direction
	planesz = 2*sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 3*planesz;
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		pAO = new double [sz_ao]; // x,y,z

		pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
		pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

template <class T> void check_and_free(T* & p)
{
	if (!p) {delete []p;p=0;}
	return;
}

template <class T> bool new_after_check_and_free(T* & p, V3DLONG nunits)
{
	try
	{
		if (!p) {delete []p;p=0;}
		p = new T [nunits];
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool LaserScanImaging::set_scanning_buffers()
{
	// set parameters
	WAIT_PIXELS = waiting_pxls(scanning_rate);//200; // [0, 100] for accelerate and [100, 200] for cache
	if(WAIT_PIXELS<1) WAIT_PIXELS=2;

	sxsw = sx + WAIT_PIXELS; // only consider scanning along x direction
	planesz = sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 2*planesz; // x, y
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			new_after_check_and_free(pImFloat, pagesz);
               sc = 1;
		}
		else if(ch==2)
		{
			new_after_check_and_free(pImFloat, V3DLONG(2)*pagesz);
			sc=2;
		}
		else
			return false;

		new_after_check_and_free(pAO, sz_ao);  // x,y
		new_after_check_and_free(pAIxy, sz_ao); // x,y feedback

		// only save the first line on the first slice but not the whole image?
		new_after_check_and_free(pAIx, sx*sy); // feedback x,
		//new_after_check_and_free(pAIy, sx*sy); // feedback y,
		new_after_check_and_free(pAOcx, sx*sy);
		
		new_after_check_and_free(pAI0, planesz);  // green
		new_after_check_and_free(pAI1, planesz);  // red

		new_after_check_and_free(pCTR0, planesz);	// green //no need to use?? by PHc 20120421
		new_after_check_and_free(pCTR1, planesz);  // red

        // real pos lut
        new_after_check_and_free(real_to_exp_pos_LUT, sx);
        new_after_check_and_free(exp_to_real_pos_LUT, sx);

	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers pAO, pAIxy, ....\n");
		return false;
	}
	return true;
}


void LaserScanImaging::set_mi_zigzag_acc()
{
	// for calibration AO and AI
	if(b_scanner_correction)
	{
		sz=1;
		sc=1;
	}

	// set parameters
	WAIT_PIXELS = 0;//200; // [0, 100] for accelerate and [100, 200] for cache
	sxsw = sx + WAIT_PIXELS; // only consider scanning along x direction
	planesz = sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 2*planesz; // x, y
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		xacc_forward = new double [WAIT_PIXELS];
		xacc_backward = new double [WAIT_PIXELS];

		double vstep = xa[1] - xa[0];
		V3DLONG midWP = WAIT_PIXELS/2;
		xacc_forward[WAIT_PIXELS-1] = xa[0];
		xacc_backward[0] = xa[sx-1];
		for(i=WAIT_PIXELS-2; i>=midWP; i--)
		{
			xacc_forward[i] =  xacc_forward[i+1] - vstep;
		}
		double vacc = xacc_forward[midWP];
		for(i=1; i<=midWP; i++)
		{
			xacc_forward[i-1] = vacc - 1.0/(double)(i);
			xacc_backward[i] = xacc_backward[i-1] + vstep;
		}
		vacc = xacc_backward[midWP];
		for(i=WAIT_PIXELS-1; i>midWP; i--)
		{
			xacc_backward[i] = vacc + 1.0/(double)(WAIT_PIXELS - i);
		}

		for(i=0; i<WAIT_PIXELS; i++)
		{
			if(xacc_forward[i]<0) xacc_forward[i] = 0;
		}

		qDebug()<<"min max voltage ..."<<xacc_forward[0]<<xacc_forward[WAIT_PIXELS-1]<<xacc_backward[0]<<xacc_backward[WAIT_PIXELS-1];

		pAO = new double [sz_ao]; // x,y

		pAIx = new float [pagesz]; // feedback x,y
		pAIy = new float [pagesz];

		pAOcx = new float [pagesz];
		pAOcy = new float [pagesz];

		pAIxy = new double [sz_ao]; // x,y feedback

		if(aictr==0) // ai
		{
			pAI = new double [2*sz_ctr];
		}
		else // ctr
		{
			pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
			pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

void LaserScanImaging::set_mi_zigzag_acc_dd() // double direction
{
	// for calibration AO and AI
	if(b_scanner_correction)
	{
		sz=1;
		sc=1;
	}

	// set parameters
	WAIT_PIXELS = 200; // [0, 100] for accelerate and [100, 200] for cache
	sxsw = sx + 2*WAIT_PIXELS; // only consider scanning along x direction
	planesz = 2*sxsw*sy; // for AO
	pagesz = sx*sy*sz;
	offset_image = pagesz;

	sz_ao = 2*planesz; // x, y
	sz_ctr = planesz*sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat = new float [pagesz];
		}
		else if(ch==2)
		{
			pImFloat = new float [2*pagesz];
			sc=2;
		}
		else
			return;

		xacc_forward = new double [WAIT_PIXELS];
		xacc_backward = new double [WAIT_PIXELS];

		double vstep = xa[1] - xa[0];

		//

		V3DLONG midWP = WAIT_PIXELS/2;
		xacc_forward[WAIT_PIXELS-1] = xa[0];
		xacc_backward[0] = xa[sx-1];
		for(i=WAIT_PIXELS-2; i>=midWP; i--)
		{
			xacc_forward[i] =  xa[0]; //xacc_forward[i+1] - vstep;
		}
		double vacc = xacc_forward[midWP];
		for(i=1; i<=midWP; i++)
		{
			xacc_forward[i-1] = xa[0]; //vacc - 1.0/(double)(i);
			xacc_backward[i] = xa[sx-1]; //xacc_backward[i-1] + vstep;
		}
		vacc = xacc_backward[midWP];
		for(i=WAIT_PIXELS-1; i>midWP; i--)
		{
			xacc_backward[i] = xa[sx-1]; //vacc + 1.0/(double)(WAIT_PIXELS - i);
		}

		qDebug()<<"min max voltage ..."<<xacc_forward[0]<<xacc_forward[WAIT_PIXELS-1]<<xacc_backward[0]<<xacc_backward[WAIT_PIXELS-1];

		pAO = new double [sz_ao]; // x,y

		pAIx = new float [pagesz]; // feedback x,y
		pAIy = new float [pagesz];

		pAOcx = new float [pagesz];
		pAOcy = new float [pagesz];

		pAIxy = new double [sz_ao]; // x,y feedback

		if(aictr==0) // ai
		{
			pAI = new double [2*sz_ctr];
		}
		else // ctr
		{
			pCTR0 = new unsigned V3DLONG [sz_ctr];	// red
			pCTR1 = new unsigned V3DLONG [sz_ctr];  // green
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for data pointers.\n");
		return;
	}
}

void LaserScanImaging::shutter_open() /// open shutters for detection
{
	// for detection signals
	DAQ_Shutter_Detection(true);

	// initialize both channels shutter off
	DAQ_Shutter_GreenLaser(false); // green
	DAQ_Shutter_RedLaser(false); // red

	//
	wait(100);

	// open laser shutter
	if(ch==0)
	{
		//green
		DAQ_Shutter_GreenLaser(true);
	}
	else if(ch==1)
	{
		//red
		DAQ_Shutter_RedLaser(true);
	}
	else if(ch==2)
	{
		//red & green
		DAQ_Shutter_GreenLaser(true); // green
		DAQ_Shutter_RedLaser(true); // red
	}
	else
	{
		cout<<"wrong channel number when opening."<<endl;
		return;
	}
}

void LaserScanImaging::shutter_close_daq_reset(bool b_onlycloseshutter) /// close shutters for detection
{
	// close laser
	if(ch==0)
	{
		//green
		DAQ_Shutter_GreenLaser(false);
	}
	else if(ch==1)
	{
		//red
		DAQ_Shutter_RedLaser(false);
	}
	else if(ch==2)
	{
		//red & green
		DAQ_Shutter_GreenLaser(false); // green
		DAQ_Shutter_RedLaser(false); // red
	}
	else
	{
		cout<<"wrong channel number when closing."<<endl;
		return;
	}

	// close
	DAQ_Shutter_Detection(false);

	//reset
	if (b_onlycloseshutter==false)
		DAQ_Reset(dev_x, dev_y, dev_z, ao_x, ao_y, ao_z);
}

void LaserScanImaging::choose_roi()
{
	v3dhandleList win_list = ((V3DPluginCallback *)cb)->getImageWindowList();
	if(win_list.size()<1)
	{
		QMessageBox::information(0, "micrscope imaging", QObject::tr("No image is open."));
		return;
	}

	v3dhandle i1 = ((V3DPluginCallback *)cb)->currentImageWindow();
	Image4DSimple* pBFI = ((V3DPluginCallback *)cb)->getImage(i1); //win_list[i1]
	if (! pBFI) return;

	QString img_wn = ((V3DPluginCallback *)cb)->getImageName(i1);
	bool flag_bffl = img_wn.contains("BF", Qt::CaseSensitive); // if it is BFI, then return "true"

	//unsigned char* data1d = pBFI->getRawData();
	//V3DLONG pagesz_BFI = pBFI->getTotalUnitNumberPerChannel();

	V3DLONG ssx = pBFI->getXDim();
	V3DLONG ssy = pBFI->getYDim();
	V3DLONG ssz = pBFI->getZDim();
	V3DLONG ssc = pBFI->getCDim();

	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views

	ROIList pRoiList=((V3DPluginCallback *)cb)->getROI(i1); //win_list[i1]

	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();

	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;

	//
	try
	{
		flag = new bool [sx*sy];
	}
	catch(...)
	{
		printf("Cannot allocate memory.\n");
		return;
	}

	QImage xy_image(ssx, ssy, QImage::Format_RGB32);
	if(vxy)
	{
		//
		for(V3DLONG j=0; j<ssy; j++)
		{
			for(V3DLONG i=0; i<ssx; i++)
			{
				xy_image.setPixel(i,j,qRgb(0, 0, 0));
			}
		}
		QPainter paintROI(&xy_image);

		paintROI.setPen( Qt::white );
		paintROI.setBrush( Qt::white );
		paintROI.drawConvexPolygon(pRoiList.at(0));

		//
		if(flag_bffl)
		{
			ssx = end_x - start_x + 1;
			ssy = end_y - start_y + 1;

			double scale_x = (double)ssx / (double)sx, scale_y = (double)ssy / (double)sy;

			for(V3DLONG j=0; j<sy; j++)
			{
				V3DLONG offset_j = j*sx;
				V3DLONG fl_count_pre = fl_count;

				for (V3DLONG i=0; i<sx; i++)
				{
					V3DLONG idx = offset_j + i;

					int ii = floor( (double)i*scale_x + start_x + 0.5);
					int jj = floor( (double)j*scale_y + start_y + 0.5);

					if(qGray(xy_image.pixel(ii,jj)))
					{
						flag[idx] = 1;
						fl_count++;
					}
					else
					{
						flag[idx] = 0;
					}

				}

				if(fl_count>fl_count_pre)
					fl_col++;

			}

		}
		else
		{
			double scale_x = (double)ssx / (double)sx, scale_y = (double)ssy / (double)sy;

			for(V3DLONG j=0; j<sy; j++)
			{
				V3DLONG offset_j = j*sx;
				V3DLONG fl_count_pre = fl_count;

				for (V3DLONG i=0; i<sx; i++)
				{
					V3DLONG idx = offset_j + i;

					int ii = floor( (double)i*scale_x + 0.5);
					int jj = floor( (double)j*scale_y + 0.5);

					if(qGray(xy_image.pixel(ii,jj)))
					{
						flag[idx] = 1;
						fl_count++;
					}
					else
					{
						flag[idx] = 0;
					}

				}

				if(fl_count>fl_count_pre)
					fl_col++;

			}
		}

		cout<< "painting... "<<fl_col<<" "<<fl_count<<endl;

	}
	else
	{
		fl_col = sy; fl_count = sx*sy;

		for(V3DLONG i=0; i<fl_count; i++)
		{
			flag[i] = 1;
		}
	}
}



void LaserScanImaging::do_image_zigzag_acc() /// imaging
{
	do_init();

	set_voltages();

	if(b_useSimImg == 0)
		shutter_open();

	if(imaging_op==0)
	{
		set_mi_zigzag_acc();

		// imaging
		int start_t = clock();

		for(k=0; k<sz; k++)
		{
			long offset_k = k*planesz;

			qDebug()<<"..."<<k+1<<"/"<<sz; // showing current

			for(j=0; j<sy; j++)
			{
				V3DLONG offset = j*sxsw; // saw-tooth

				V3DLONG offset_x = offset;
				V3DLONG offset_y = planesz + offset;

				if(j%2==0)
				{
					for(i=0; i<WAIT_PIXELS; i++)
					{
						pAO[offset_x + i] = xacc_forward[i];
						pAO[offset_y + i] = ya[j];
					}

					V3DLONG xend_2nd = WAIT_PIXELS + sx;

					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						pAO[offset_x + i] = xa[i-WAIT_PIXELS];
						pAO[offset_y + i] = ya[j];
					}

				}
				else
				{
					for(i=0; i<WAIT_PIXELS; i++)
					{
						pAO[offset_x + i] = xacc_backward[WAIT_PIXELS - 1 - i];
						pAO[offset_y + i] = ya[j];
					}

					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						pAO[offset_x + i] = xa[sx-1 -(i-WAIT_PIXELS)];
						pAO[offset_y + i] = ya[j];
					}

				}

			}

			//imaging
			try
			{
				if(b_useSimImg == 0)
					DAQ_UpdateZ(za[k], dev_z, ao_z); // z

				int success;

				if(b_useSimImg ==0)
				{
					if(aictr==0)
						success = DAQ_getImg_ai(pAI, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
					else
						success = DAQ_getImg_ctr(pCTR0+offset_k, pCTR1+offset_k, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
				}else if(b_useSimImg ==1)
				{
					if(aictr==0)
					{
						v3d_msg("Simulation imaging is only available for aictr=1.\n", 0);
						return;
					}
					else if(aictr!=0)// using simulation image
						success = DAQ_getImg_sim(pCTR0, pCTR1, pAI0, pAI1, ch, sz_ctr, sx, sy, sz, planesz, sxsw, WAIT_PIXELS);
				}

				if(success)
				{
					printf("Failed in microscope imaging.\n");
					return;
				}

				wait(WAITING_PERSLICE); // every k slice
			}
			catch (...)
			{
				printf("Invoking DAQ_getImg failed.\n");
				return;
			}

			V3DLONG offsets_k = k*sx*sy;

			// ai x, y
			if(b_scanner_correction)
			{

				for(j=0; j<sy; j++)
				{
					V3DLONG offset = j*sxsw;
					V3DLONG offsets = j*sx + offsets_k;

					if(j%2==0) // forward
					{
						for(i=WAIT_PIXELS; i<sxsw; i++)
						{
							//assign
							V3DLONG idx = offset + i;
							V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

							// ai x y
							pAIx[idx_ori] = pAIxy[idx];
							pAIy[idx_ori] = pAIxy[idx + planesz];

							pAOcx[idx_ori] = pAO[idx];
							pAOcy[idx_ori] = pAO[idx + planesz];
						}
					}
					else // backward
					{
						for(i=WAIT_PIXELS; i<sxsw; i++)
						{
							//assign
							V3DLONG idx = offset + i;
							V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

							// ai x y
							pAIx[idx_ori] = pAIxy[idx];
							pAIy[idx_ori] = pAIxy[idx + planesz];

							pAOcx[idx_ori] = pAO[idx];
							pAOcy[idx_ori] = pAO[idx + planesz];
						}
					}
				}
			}

			// ai data
			if(aictr==0)
			{
				if(ch==0)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 0
								pImFloat[idx_ori] = pAI[idx];
							}
						}
					}
				}
				else if(ch==1)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else if(ch==2)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ai 0
								pImFloat[idx_ori] = pAI[idx];

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else
				{
					cout<<"wrong channel number when closing."<<endl;
					return;
				}
			}

		} // for k

		size_t end_t = clock();
		printf("time elapse ... %d \n", end_t-start_t);

		// assign back ctr
		if(aictr!=0)
		{
			if(ch==0)
			{
				double max_v=0, min_v=INF;

				// green ctr1
				// readjust input orders
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR1[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
						else
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								float tmp = pCTR1[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==1)
			{
				double max_v=0, min_v=INF;

				// red ctr0
				// readjust input orders
				V3DLONG i,j,k;
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
						else
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = j*sx + offsets_k;

							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==2)
			{
				double max_v_red=0, min_v_red=INF;
				double max_v_green=0, min_v_green=INF;

				// readjust input orders
				// photon counting - red ctr0 & green ctr1
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw + offset_k;
						V3DLONG offsets = j*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ctr0
								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v_red<tmp) max_v_red = tmp;
								if(min_v_red>tmp) min_v_red = tmp;

								// ctr1
								float tmp1 = pCTR1[idx];

								pImFloat[idx_ori+offset_image] = tmp1;

								if(max_v_green<tmp1) max_v_green = tmp1;
								if(min_v_green>tmp1) min_v_green = tmp1;

							}
						}
						else // backward
						{
							for(i=WAIT_PIXELS; i<sxsw; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ sx - 1 - (i-WAIT_PIXELS);

								// ctr0
								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v_red<tmp) max_v_red = tmp;
								if(min_v_red>tmp) min_v_red = tmp;

								// ctr1
								float tmp1 = pCTR1[idx];

								pImFloat[idx_ori+offset_image] = tmp1;

								if(max_v_green<tmp1) max_v_green = tmp1;
								if(min_v_green>tmp1) min_v_green = tmp1;

							}
						}
					}
				}
				qDebug()<<"[max_v_red, min_v_red] ... " << "["<<max_v_red<<","<<min_v_red<<"]";
				qDebug()<<"[max_v_green, min_v_green] ... " << "["<<max_v_green<<","<<min_v_green<<"]";

			}
			else
			{
				cout<<"wrong channel number when closing."<<endl;
				return;
			}
		}

	}
	else if(imaging_op==1)
	{

	}
	else if(imaging_op==2)
	{

	}
	else if(imaging_op==3)
	{

	}
	else if(imaging_op==4)
	{

	}
	else
	{
		cout<<"Undefined imaging operation."<<endl;
		return;
	}

	if(b_useSimImg == 0)
		shutter_close_daq_reset();

	cleanData();
}




void LaserScanImaging::do_image_zigzag_acc_dd() /// imaging double direction
{
	do_init();

	set_voltages();

	if(b_useSimImg == 0)
		shutter_open();

	if(imaging_op==0)
	{
		set_mi_zigzag_acc_dd();

		// imaging
		int start_t = clock();

		// set AO
		V3DLONG xend_2nd = WAIT_PIXELS + sx;
		V3DLONG yend_2nd = 2*sy;
		for(j=0; j<yend_2nd; j++)
		{
			V3DLONG offset = j*sxsw; // saw-tooth

			V3DLONG offset_x = offset;
			V3DLONG offset_y = planesz + offset;

			if(j%2==0)
			{
				for(i=0; i<WAIT_PIXELS; i++)
				{
					pAO[offset_x + i] = xacc_forward[i];
					pAO[offset_y + i] = ya[j/2];
				}

				for(i=WAIT_PIXELS; i<xend_2nd; i++)
				{
					pAO[offset_x + i] = xa[i-WAIT_PIXELS];
					pAO[offset_y + i] = ya[j/2];
				}

				for(i=xend_2nd; i<sxsw; i++)
				{
					pAO[offset_x + i] = xacc_backward[i-xend_2nd];
					pAO[offset_y + i] = ya[j/2];
				}

			}
			else
			{
				for(i=0; i<WAIT_PIXELS; i++)
				{
					pAO[offset_x + i] = xacc_backward[WAIT_PIXELS - 1 - i];
					pAO[offset_y + i] = ya[j/2];
				}

				for(i=WAIT_PIXELS; i<xend_2nd; i++)
				{
					pAO[offset_x + i] = xa[sx-1 -(i-WAIT_PIXELS)];
					pAO[offset_y + i] = ya[j/2];
				}

				for(i=xend_2nd; i<sxsw; i++)
				{
					pAO[offset_x + i] = xacc_forward[WAIT_PIXELS - 1 + i-xend_2nd];
					pAO[offset_y + i] = ya[j/2];
				}

			}

		}

		qDebug()<<"test ... sx "<<sx<<" sxsw ..."<<sxsw;
		for(int i=0; i<sz_ao; i++)
		{
			//qDebug()<<pAO[i];
			//pAO[i] = 1;
		}

		// imaging
		for(k=0; k<sz; k++)
		{
			long offset_k = k*planesz;

			qDebug()<<"..."<<k+1<<"/"<<sz; // showing current

			//imaging
			try
			{
				if(b_useSimImg == 0)
					DAQ_UpdateZ(za[k], dev_z, ao_z); // z

				int success;

				if(b_useSimImg ==0)
				{
					if(aictr==0)
						success = DAQ_getImg_ai(pAI, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
					else
						success = DAQ_getImg_ctr(pCTR0+offset_k, pCTR1+offset_k, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
				}else if(b_useSimImg ==1)
				{
					if(aictr==0)
					{
						v3d_msg("Simulation imaging is only available for aictr=1.\n", 0);
						return;
					}else if(aictr!=0)// using simulation image
						success = DAQ_getImg_sim(pCTR0, pCTR1, pAI0, pAI1, ch, sz_ctr, sx, sy, sz, planesz, sxsw, WAIT_PIXELS);
				}

				if(success)
				{
					printf("Failed in microscope imaging.\n");
					return;
				}

				wait(WAITING_PERSLICE); // every k slice
			}
			catch (...)
			{
				printf("Invoking DAQ_getImg failed.\n");
				return;
			}

			V3DLONG offsets_k = k*sx*sy;

			// ai x, y
			if(b_scanner_correction)
			{
				for(j=0; j<yend_2nd; j++)
				{
					V3DLONG offset = j*sxsw;
					V3DLONG offsets = (j/2)*sx + offsets_k;

					if(j%2==0) // forward only
					{
						for(i=WAIT_PIXELS; i<xend_2nd; i++)
						{
							//assign
							V3DLONG idx = offset + i;
							V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

							// ai x y
							pAIx[idx_ori] = pAIxy[idx];
							pAIy[idx_ori] = pAIxy[idx + planesz];

							pAOcx[idx_ori] = pAO[idx];
							pAOcy[idx_ori] = pAO[idx + planesz];
						}
					}

				}
			}

			// ai data
			if(aictr==0)
			{
				if(ch==0)
				{
					for(j=0; j<yend_2nd; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = (j/2)*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];
							}
						}
					}
				}
				else if(ch==1)
				{
					for(j=0; j<yend_2nd; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = (j/2)*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else if(ch==2)
				{
					for(j=0; j<yend_2nd; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = (j/2)*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ai 0
								pImFloat[idx_ori] = pAI[idx];

								// ai 1
								pImFloat[idx_ori] = pAI[idx + planesz];
							}
						}
					}
				}
				else
				{
					cout<<"wrong channel number when closing."<<endl;
					return;
				}
			}

		} // for k

		size_t end_t = clock();
		printf("time elapse ... %d \n", end_t-start_t);

		// assign back ctr
		if(aictr!=0)
		{
			if(ch==0)
			{
				double max_v=0, min_v=INF;

				// green ctr1
				// readjust input orders
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<yend_2nd; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = (j/2)*sx + offsets_k;

							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR1[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==1)
			{
				double max_v=0, min_v=INF;

				// red ctr0
				// readjust input orders
				V3DLONG i,j,k;
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<yend_2nd; j++)
					{
						if(j%2==0)
						{
							V3DLONG offset = j*sxsw + offset_k;
							V3DLONG offsets = (j/2)*sx + offsets_k;

							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v<tmp) max_v = tmp;
								if(min_v>tmp) min_v = tmp;

							}
						}
					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==2)
			{
				double max_v_red=0, min_v_red=INF;
				double max_v_green=0, min_v_green=INF;

				// readjust input orders
				// photon counting - red ctr0 & green ctr1
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<yend_2nd; j++)
					{
						V3DLONG offset = j*sxsw + offset_k;
						V3DLONG offsets = (j/2)*sx + offsets_k;

						if(j%2==0) // forward
						{
							for(i=WAIT_PIXELS; i<xend_2nd; i++)
							{
								//assign
								V3DLONG idx = offset + i;
								V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

								// ctr0
								float tmp = pCTR0[idx];

								pImFloat[idx_ori] = tmp;

								if(max_v_red<tmp) max_v_red = tmp;
								if(min_v_red>tmp) min_v_red = tmp;

								// ctr1
								float tmp1 = pCTR1[idx];

								pImFloat[idx_ori+offset_image] = tmp1;

								if(max_v_green<tmp1) max_v_green = tmp1;
								if(min_v_green>tmp1) min_v_green = tmp1;

							}
						}
					}
				}
				qDebug()<<"[max_v_red, min_v_red] ... " << "["<<max_v_red<<","<<min_v_red<<"]";
				qDebug()<<"[max_v_green, min_v_green] ... " << "["<<max_v_green<<","<<min_v_green<<"]";

			}
			else
			{
				cout<<"wrong channel number when closing."<<endl;
				return;
			}
		}

	}
	else if(imaging_op==1)
	{

	}
	else if(imaging_op==2)
	{

	}
	else if(imaging_op==3)
	{

	}
	else if(imaging_op==4)
	{

	}
	else
	{
		cout<<"Undefined imaging operation."<<endl;
		return;
	}

	if(b_useSimImg == 0)
		shutter_close_daq_reset();

	cleanData();
}


void LaserScanImaging::do_image_acc() /// imaging with single direction, no zigzag
{
	do_init();

	set_voltages();

	if(b_useSimImg == 0)
		shutter_open();

	if(imaging_op==0)
	{
		set_mi_zigzag_acc(); // can use this method, but do not use xacc_backward

		// imaging
		int start_t = clock();

		for(k=0; k<sz; k++)
		{
			long offset_k = k*planesz;

			qDebug()<<"..."<<k+1<<"/"<<sz; // showing current

			for(j=0; j<sy; j++)
			{
				V3DLONG offset = j*sxsw; // saw-tooth

				V3DLONG offset_x = offset;
				V3DLONG offset_y = planesz + offset;

                    for(i=0; i<WAIT_PIXELS; i++)
                    {
                         pAO[offset_x + i] = xacc_forward[i];
                         pAO[offset_y + i] = ya[j];
                    }

                    for(i=WAIT_PIXELS; i<sxsw; i++)
                    {
                         pAO[offset_x + i] = xa[i-WAIT_PIXELS];
                         pAO[offset_y + i] = ya[j];
                    }
			}

			//imaging
			try
			{
				if(b_useSimImg == 0)
					DAQ_UpdateZ(za[k], dev_z, ao_z); // z

				int success;

				if(b_useSimImg ==0)
				{
					if(aictr==0)
						success = DAQ_getImg_ai(pAI, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
					else
						success = DAQ_getImg_ctr(pCTR0+offset_k, pCTR1+offset_k, pAO, pAIxy, planesz, time_out, scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n, dev_red, red_n, dev_green, green_n);
				}else if(b_useSimImg ==1)
				{
					if(aictr==0)
					{
						v3d_msg("Simulation imaging is only available for aictr=1.\n", 0);
						return;
					}
					else if(aictr!=0)// using simulation image
						success = DAQ_getImg_sim(pCTR0, pCTR1, pAI0, pAI1, ch, sz_ctr, sx, sy, sz, planesz, sxsw, WAIT_PIXELS);
				}

				if(success)
				{
					printf("Failed in microscope imaging.\n");
					return;
				}

				wait(WAITING_PERSLICE); // every k slice
			}
			catch (...)
			{
				printf("Invoking DAQ_getImg failed.\n");
				return;
			}

			V3DLONG offsets_k = k*sx*sy;

			// ai x, y
			if(b_scanner_correction)
			{

				for(j=0; j<sy; j++)
				{
					V3DLONG offset = j*sxsw;
					V3DLONG offsets = j*sx + offsets_k;

                         for(i=WAIT_PIXELS; i<sxsw; i++)
                         {
                              //assign
                              V3DLONG idx = offset + i;
                              V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

                              // ai x y
                              pAIx[idx_ori] = pAIxy[idx];
                              pAIy[idx_ori] = pAIxy[idx + planesz];

                              pAOcx[idx_ori] = pAO[idx];
                              pAOcy[idx_ori] = pAO[idx + planesz];
                         }
                    }
			}

			// ai data
			if(aictr==0)
			{
				if(ch==0)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

                                   // ai 0
                                   pImFloat[idx_ori] = pAI[idx];
                              }
                         }
				}
				else if(ch==1)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

                                   // ai 1
                                   pImFloat[idx_ori] = pAI[idx + planesz];
                              }

					}
				}
				else if(ch==2)
				{
					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw;
						V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

                                   // ai 0
                                   pImFloat[idx_ori] = pAI[idx];

                                   // ai 1
                                   pImFloat[idx_ori] = pAI[idx + planesz];
                              }

					}
				}
				else
				{
					cout<<"wrong channel number when closing."<<endl;
					return;
				}
			}

		} // for k

		size_t end_t = clock();
		printf("time elapse ... %d \n", end_t-start_t);

		// assign back ctr
		if(aictr!=0)
		{
			if(ch==0)
			{
				double max_v=0, min_v=INF;

				// green ctr1
				// readjust input orders
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{

                              V3DLONG offset = j*sxsw + offset_k;
                              V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

                                   float tmp = pCTR1[idx];

                                   pImFloat[idx_ori] = tmp;

                                   if(max_v<tmp) max_v = tmp;
                                   if(min_v>tmp) min_v = tmp;

                              }

					}
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==1)
			{
				double max_v=0, min_v=INF;

				// red ctr0
				// readjust input orders
				V3DLONG i,j,k;
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
                              V3DLONG offset = j*sxsw + offset_k;
                              V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+i-WAIT_PIXELS;

                                   float tmp = pCTR0[idx];

                                   pImFloat[idx_ori] = tmp;

                                   if(max_v<tmp) max_v = tmp;
                                   if(min_v>tmp) min_v = tmp;

                              }
                         }
				}
				qDebug()<<"[max_v, min_v] ... " << "["<<max_v<<","<<min_v<<"]";

			}
			else if(ch==2)
			{
				double max_v_red=0, min_v_red=INF;
				double max_v_green=0, min_v_green=INF;

				// readjust input orders
				// photon counting - red ctr0 & green ctr1
				for(k=0; k<sz; k++)
				{
					V3DLONG offset_k = k*planesz;
					V3DLONG offsets_k = k*sx*sy;

					for(j=0; j<sy; j++)
					{
						V3DLONG offset = j*sxsw + offset_k;
						V3DLONG offsets = j*sx + offsets_k;

                              for(i=WAIT_PIXELS; i<sxsw; i++)
                              {
                                   //assign
                                   V3DLONG idx = offset + i;
                                   V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

                                   // ctr0
                                   float tmp = pCTR0[idx];

                                   pImFloat[idx_ori] = tmp;

                                   if(max_v_red<tmp) max_v_red = tmp;
                                   if(min_v_red>tmp) min_v_red = tmp;

                                   // ctr1
                                   float tmp1 = pCTR1[idx];

                                   pImFloat[idx_ori+offset_image] = tmp1;

                                   if(max_v_green<tmp1) max_v_green = tmp1;
                                   if(min_v_green>tmp1) min_v_green = tmp1;

                              }
					}
				}
				qDebug()<<"[max_v_red, min_v_red] ... " << "["<<max_v_red<<","<<min_v_red<<"]";
				qDebug()<<"[max_v_green, min_v_green] ... " << "["<<max_v_green<<","<<min_v_green<<"]";

			}
			else
			{
				cout<<"wrong channel number when closing."<<endl;
				return;
			}
		}

	}
	else if(imaging_op==1)
	{

	}
	else if(imaging_op==2)
	{

	}
	else if(imaging_op==3)
	{

	}
	else if(imaging_op==4)
	{

	}
	else
	{
		cout<<"Undefined imaging operation."<<endl;
		return;
	}

	if(b_useSimImg == 0)
		shutter_close_daq_reset();

	cleanData();
}


// confocal imaging with interactive selecting ROI
bool LaserScanImaging::do_image_withROI_i()
{
	qDebug()<<"............. Now running LSM imaging for ROI ..............."; //return;

	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;
	double vz_x, vz_y, vz_z;

	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=((V3DPluginCallback *)cb)->currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return false;
	}

	//finding the bounding box of ROI
	ROIList pRoiList=((V3DPluginCallback *)cb)->getROI(curwin);
	if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
	{
		v3d_msg("No ROI specified ... Do nothing.");
		return false;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	Parameters_LSM pl;
	bool b_roi_FC = false;
	bool b_roi_LSMFC =false;

	// decide whether this is selected from Focus_Control_BF/LSM window
	if((logname.compare("Focus_Control_BF")==0) || (logname.contains("BF_", Qt::CaseSensitive)) || (logname.compare("Focus_Control_LSM")==0))
	{
		// get Parameters_LSM from LSM configure file
		QString lsm_FileName = ssConfigFolder + "mi_configuration.txt";
		if(!QFile(lsm_FileName).exists())
		{
			lsm_FileName = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
													  "/Documents", QObject::tr("Image Configuration (*.log)"));
			if(lsm_FileName.isEmpty())
			{
				v3d_msg("No device configuration file exist. Do nothing!");
				return false;
			}
		}
		// load lsm paras
		pl.load_ParameterFile(lsm_FileName);
		if(logname.compare("Focus_Control_LSM")==0)
			b_roi_LSMFC = true;

		b_roi_FC = true;
	}else
	{
		// load from current full image log file
		logname.chop(4);
		logname.append(".txt");

		pl.load_ParameterFile(logname);
		cout << "logfile name:"<<logname.toStdString()<<endl;

		b_roi_FC = false;
	}

	// these paras are from log file for current full image
	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	fov_step_x = pl.FOVX_STEP; fov_step_y = pl.FOVY_STEP; fov_step_z = pl.FOVZ_STEP;
	min_fovz = pl.FOVZ_MIN; max_fovz = pl.FOVZ_MAX;

	// always get BF_TO_LSM_A... from "mi_configuration.txt"
	// because this is the latest infor.
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return false;
	}

	a_bflsm = mypara.BF_TO_LSM_A; d_bflsm = mypara.BF_TO_LSM_D;
	e_bflsm = mypara.BF_TO_LSM_E; f_bflsm = mypara.BF_TO_LSM_F;

	//===========preparing parameters for ROI parameter dialog=====
	// get image in current window
	pImgIn=((V3DPluginCallback *)cb)->getImage(curwin);
	if(!pImgIn) return false;

	double o_origin_x, o_origin_y, o_origin_z;
	double o_rez_x, o_rez_y, o_rez_z;
	V3DLONG o_sx, o_sy, o_sz, o_sc;
	V3DLONG n_sx, n_sy, n_sz;
	double n_origin_x, n_origin_y, n_origin_z;
	double n_min_fovx, n_min_fovy, n_min_fovz;
	double n_max_fovx, n_max_fovy, n_max_fovz;

	o_sx = pImgIn->getXDim();
	o_sy = pImgIn->getYDim();
	o_sz = pImgIn->getZDim();
	o_sc = pImgIn->getCDim();

	// ROI list
	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();

	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views
	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;

	bb_start_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), o_sx-1);
	bb_start_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), o_sy-1);
	bb_start_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), o_sz-1);

	bb_end_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.right(), b_zx.right())), o_sx-1); //use qMax but not qMin
	bb_end_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.bottom(), b_yz.bottom())), o_sy-1);
	bb_end_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.right(), b_zx.bottom())), o_sz-1);

	if( vxy && !vyz && !vzx )  // only has rect on xy view, roi_BF only has xy view
	{
		bb_start_z = 0;
		if(b_roi_FC)
			bb_end_z = 0;
		else
			bb_end_z = o_sz-1;
	}
	else if( !vxy && vyz && !vzx )// only has rect on yz view
	{
		bb_start_x = 0;
		bb_end_x = o_sx-1;
	}
	else if( !vxy && !vyz && vzx )// only has rect on zx view
	{
		bb_start_y = 0;
		bb_end_y = o_sy-1;
	}

	qDebug()<<"bb_start... (x, y, z)"<< bb_start_x << bb_start_y << bb_start_z;
	qDebug()<<"bb_end... (x, y, z)"<< bb_end_x << bb_end_y << bb_end_z;


	// test codes
	if (0)
	{
		V3DLONG out_sz[4];
		out_sz[0] = bb_end_x - bb_start_x + 1;
		out_sz[1] = bb_end_y - bb_start_y + 1;
		out_sz[2] = bb_end_z - bb_start_z + 1;
		out_sz[3] = o_sc;
		V3DLONG out_sz01 = out_sz[0] * out_sz[1];
		V3DLONG tol_sz = out_sz01 * out_sz[2];
		float * outimg1d = new float[tol_sz*o_sc];
		int datatype = 4;
		float * inimg1d = (float*)(pImgIn->getRawData());
		V3DLONG in_sz[4] = {o_sx, o_sy, o_sz, o_sc};
		V3DLONG in_sz01 = in_sz[0] * in_sz[1];
		for(int c = 0; c < o_sc; c++)
		{
			float * inimg1d_channel = inimg1d + c * in_sz[0] * in_sz[1] * in_sz[2];
			float * outimg1d_channel = outimg1d + c * out_sz[0] * out_sz[1] * out_sz[2];
			for(V3DLONG k = bb_start_z; k <= bb_end_z; k++)
			{
				for(V3DLONG j = bb_start_y; j <= bb_end_y; j++)
				{
					for(V3DLONG i = bb_start_x; i <= bb_end_x; i++)
					{
						V3DLONG ind1 = k * in_sz01 + j * in_sz[0] + i;
						V3DLONG ind2 = (k - bb_start_z) * out_sz01 + (j - bb_start_y) * out_sz[0] + (i - bb_start_x);
						outimg1d_channel[ind2] = inimg1d_channel[ind1];
					}
				}
			}
		}
		saveImage("test.raw", (unsigned char*)outimg1d, out_sz, datatype);
		if(outimg1d){delete [] outimg1d; outimg1d = 0;}
	}



	if (bb_start_x>bb_end_x || bb_start_y>bb_end_y || bb_start_z>bb_end_z)
	{
		v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
		return false;
	}

	if(!b_roi_FC)
	{
		// make sure to get correct value. These values can always get from log file
		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_origin_z = pl.FOVZ_MIN;

		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
		o_rez_z = pl.FOVZ_STEP;

		vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

		// print info message
		// roi bounding box, voxel size: vz_x vz_y vz_z
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		//v3d_msg(QString("%1 %2 %3 %4 %5 %6").arg(o_origin_x).arg(o_origin_y).arg(bb_start_x).arg(bb_start_y).arg(bb_end_x).arg(bb_end_y));

		n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
        n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
		n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

		n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
		n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
		n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;
		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------old info---------------";
		qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
		qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
		qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
		qDebug()<<"-----------new info---------------";
		qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
		qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
		n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
		n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
		n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;

		//v3d_msg(QString("%1 %2 %3 %4").arg(n_min_fovx).arg(n_min_fovy).arg(n_max_fovx).arg(n_max_fovy));

	//the above code has a lot of redundancy! can be simplified
	}
	else // ROI from focus control window
	{
		if(b_roi_LSMFC) //ROI from LSM focus control window
		{
			o_origin_x = pl.FOVX_MIN;
			o_origin_y = pl.FOVY_MIN;
			o_origin_z = pl.FOVZ_MIN;

			o_rez_x = pl.FOVX_STEP;
			o_rez_y = pl.FOVY_STEP;
			o_rez_z = pl.FOVZ_STEP;

			vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

			// roi bounding box, voxel size: vz_x vz_y vz_z
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------passing para---------------";
			qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
			n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
			n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

			n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
			n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
			n_sz = 1; // only one slice
			// print info message
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------old info---------------";
			qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
			qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
			qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
			qDebug()<<"-----------new info---------------";
			qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
			qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
			n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
			n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
			n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		}
		else //ROI from BF focus control window
		{
			v3d_msg("Please make sure that BF/LSM calibration has been done before this operation!\n",0);
			// transform BF coords to LSM coords: the LSM coords are physical pos
			n_min_fovx = a_bflsm * bb_start_x + e_bflsm;
			n_min_fovy = d_bflsm * bb_start_y + f_bflsm;

			n_max_fovx = a_bflsm * bb_end_x + e_bflsm;
			n_max_fovy = d_bflsm * bb_end_y + f_bflsm;

			n_min_fovz = 30.0; n_max_fovz = 35.0;  // default

			n_sx = (V3DLONG)((n_max_fovx-n_min_fovx)/fov_step_x);
			n_sy = (V3DLONG)((n_max_fovy-n_min_fovy)/fov_step_y);
			n_sz = 1;

			vz_x=fov_step_x; vz_y=fov_step_y; vz_z=fov_step_z;
		}
	}

	if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096) //
	{
		printf("The target image size is too large.\n");
		return false;
	}

	// symbol link for scanning.
	sx = n_sx; sy = n_sy; sz = n_sz;
	min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
	max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
	fov_step_x = vz_x; //(max_fovx-min_fovx)/sx;
	fov_step_y = vz_y; //(max_fovy-min_fovy)/sy;
	fov_step_z = (max_fovz-min_fovz)/sz;

	// these are put back to lsm configure file and used as initial paras in ROI parameter dialog
	pl.FOVX_MIN = min_fovx;	pl.FOVX_MAX = max_fovx;	pl.SX = sx;	pl.FOVX_STEP = fov_step_x;
	pl.FOVY_MIN = min_fovy;	pl.FOVY_MAX = max_fovy;	pl.SY = sy;	pl.FOVY_STEP = fov_step_y;
	pl.FOVZ_MIN = min_fovz;	pl.FOVZ_MAX = max_fovz;	pl.SZ = sz;	pl.FOVZ_STEP = fov_step_z;

	// user can now define parameters by ROI_Setup_Dialog
	LSM_ROI_Setup_Dialog roi_dialog(&pl, false);
	if (roi_dialog.exec()!=QDialog::Accepted)
		return false;

	roi_dialog.copyDataOut(&pl);

	// get following paras from ROI para dialog
	min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP;
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	sc = (ch<2)?1:2;

	//v3d_msg(QString("Real use: %1 %2 %3 %4").arg(min_fovx).arg(min_fovy).arg(max_fovx).arg(max_fovy));

	//// set paras for getting correction offset
     if(USE_PRIOR_CORRECTION == 1)
     {
          make_a_copy_PHC(true);
          old_fov_step_x = o_rez_x;
          old_fov_step_y = o_rez_y;

          // now correct the scanning ranges based on scanning speed, etc , by PHC 201020420
          set_real_scanning_range_with_real_offset();
     }

	set_voltages();
	if(!set_scanning_buffers())
		return false;
	// min_fovx change depending on scanning
	// open laser
	if(b_useSimImg == 0) shutter_open();
	if(!do_LSM_single())
		return false;

	// copy back to original paras set from dialog
     if(USE_PRIOR_CORRECTION == 1)
     {
          make_a_copy_PHC(false);
          fov_step_x = pl.FOVX_STEP;
          fov_step_y = pl.FOVY_STEP;
     }

	if(b_useSimImg == 0) shutter_close_daq_reset();

	return true;
}

void LaserScanImaging::make_a_copy_PHC(bool b_forward)
{
    if (b_forward)
    {
        old_sz = sz;
        old_min_fovx = min_fovx; old_max_fovx = max_fovx;
        old_min_fovy = min_fovy; old_max_fovy = max_fovy;
        old_fov_step_x = fov_step_x;
        old_fov_step_y = fov_step_y;
    }
    else
    {
        sz = old_sz;
        min_fovx = old_min_fovx; max_fovx = old_max_fovx;
        min_fovy = old_min_fovy; max_fovy = old_max_fovy;
        fov_step_x = old_fov_step_x;
        fov_step_y = old_fov_step_y;
    }
}

void LaserScanImaging::get_current_scanning_mirror_offset()
{
	V3DLONG sx_thresh = 40;
	if(sx < sx_thresh)
	{
		cal_offset_left = sx_thresh;
		cal_offset_right = sx_thresh;
		v3d_msg(QString("sx is too small. The boundary offset is set to sx_thresh: %1").arg(sx_thresh), 0);
	}
	else
	{
		sz = 1;
		sy = 1; //just scan two lines, 1 line is actually enough

		set_voltages();

		if(!set_scanning_buffers())
			return;

		// min_fovx change depending on scanning
		// open laser
		if(b_useSimImg == 0)
			shutter_open();

		if(!do_LSM_single())
			return;

		if(b_useSimImg == 0)
			shutter_close_daq_reset();
	}

    V3DLONG boundary = 40;
    offset_left = cal_offset_left + boundary;   // offset_left, cal_offset_left are global vars
    offset_right = cal_offset_right + boundary; // offset_right, cal_offset_right are global vars

    cleanData(false); //clean all pointers except real pos LUT

    return;
}

void LaserScanImaging::set_real_scanning_range_with_real_offset(bool b_addrightmargin)
{
    get_current_scanning_mirror_offset(); //this function will reset scanning_offset_x, by PHC, 20120420

	sz = old_sz;

	min_fovx = old_min_fovx - offset_left * old_fov_step_x;  // scanning_offset_left/right is in pixel value, need to convert to um
	max_fovx = old_max_fovx;
	if (b_addrightmargin==true)
		max_fovx += offset_right * old_fov_step_x;

    sx = (max_fovx-min_fovx)/fov_step_x;

	//min_fovy = old_min_fovy - scanning_offset_x * old_fov_step_y;// temporarily set as this , PHC 120420 //- scanning_offset_x * o_rez_y;  // also calib y value
	//max_fovy = old_max_fovy - scanning_offset_x * old_fov_step_y;// - scanning_offset_x * o_rez_y;
    sy = (max_fovy-min_fovy)/fov_step_y; //this sentence should NOT be removed
}


// imaging roi which chosen from 3d view, this function is no use at the moment
bool LaserScanImaging::do_image_3DViewROI()
{
	qDebug()<<".................... Now running roi imaging program ..................."; //return;

	// current window
	v3dhandle wincurr = ((V3DPluginCallback *)cb)->curHiddenSelectedWindow(); //by PHC, 101009. currentImageWindow();
	if (!wincurr)
	{
		v3d_msg("The pointer to the current image window is invalid. Do nothing.");
		return false;
	}
	Image4DSimple* pImgIn = ((V3DPluginCallback *)cb)->getImage(wincurr); //not using the image class pointer in the parameter-struct, - is there correct in the V3DLONG run? noted by PHC
    if (! pImgIn)
	{
		v3d_msg("The pointer to the current image window is invalid. Do nothing.");
		return false;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(wincurr);
	logname.chop(4); logname.append(".txt");

	Parameters_LSM pl;
	pl.load_ParameterFile(logname);
	cout << "logfile name:"<<logname.toStdString()<<endl;

	// these paras are from the current full image config file
	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	// parameters passed from 3d view
	v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
	if(!p) //a bug fixed by PHC, 101008
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.");
		return false;
	}
	if (!(p->imgp))
	{
		v3d_msg("The image pointer inside the parameter-struct is invalid. Do nothing.");
		return false;
	}

	// print info message
	qDebug()<<"-----------------------------------------------------------------------";
	qDebug()<<"-----------passing para through interface---------------";
	qDebug()<<"bb ..."<< p->xs << p->xe << p->ys << p->ye << p->zs << p->ze;
	qDebug()<<"resolution ..."<< p->xrez << p->yrez << p->zrez;
	qDebug()<<"-----------------------------------------------------------------------";

	v3d_msg(QString("ROI bounding box: xs = %1, xe = %2, ys = %3, ye = %4, zs = %5, ze = %6").arg(p->xs).arg(p->xe).arg(p->ys).arg(p->ye).arg(p->zs).arg(p->ze) );

	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;
	double vz_x, vz_y, vz_z;
	bb_start_x=p->xs; bb_end_x=p->xe; bb_start_y=p->ys; bb_end_y=p->ye; bb_start_z=p->zs; bb_end_z=p->ze;
	vz_x=p->xrez; vz_y=p->yrez; vz_z=p->zrez;

	// absolute physical position of the current image: pImgIn

	V3DLONG o_sx = pImgIn->getXDim();
	V3DLONG o_sy = pImgIn->getYDim();
	V3DLONG o_sz = pImgIn->getZDim();
	V3DLONG o_sc = pImgIn->getCDim();

	// make sure to get correct value. These values can always get from log file
	double o_origin_x = pl.FOVX_MIN;
	double o_origin_y = pl.FOVY_MIN;
	double o_origin_z = pl.FOVZ_MIN;

	double o_rez_x = pl.FOVX_STEP;
	double o_rez_y = pl.FOVY_STEP;
	double o_rez_z = pl.FOVZ_STEP;

	// imaging
	// ------------------------------------------------------
	double n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
	double n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
	double n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

	V3DLONG n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
	V3DLONG n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
	V3DLONG n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;

	double n_min_fovx = n_origin_x, n_min_fovy = n_origin_y, n_min_fovz = n_origin_z;
	double n_max_fovx = n_origin_x + ((double)n_sx)*vz_x, n_max_fovy = n_origin_y + ((double)n_sy)*vz_y, n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;

	if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096) //
	{
		v3d_msg("The target image size is too large.\n", 0);
		return false;
	}

	// print info message
	qDebug()<<"-----------------------------------------------------------------------";
	qDebug()<<"-----------old info---------------";
	qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
	qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
	qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
	qDebug()<<"-----------new info---------------";
	qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
	qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
	qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
	qDebug()<<"-----------------------------------------------------------------------";

	// symbol link
	sx = n_sx; sy = n_sy; sz = n_sz;
	min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
	max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
	fov_step_x = (max_fovx-min_fovx)/sx;
	fov_step_y = (max_fovy-min_fovy)/sy;
	fov_step_z = (max_fovz-min_fovz)/sz;

	// these are put back lsm configure file and used as initial paras in ROI parameter dialog
	pl.FOVX_MIN = min_fovx;	pl.FOVX_MAX = max_fovx;	pl.SX = sx;	pl.FOVX_STEP = fov_step_x;
	pl.FOVY_MIN = min_fovy;	pl.FOVY_MAX = max_fovy;	pl.SY = sy;	pl.FOVY_STEP = fov_step_y;
	pl.FOVZ_MIN = min_fovz;	pl.FOVZ_MAX = max_fovz;	pl.SZ = sz;	pl.FOVZ_STEP = fov_step_z;

	// user can now define parameters
	LSM_ROI_Setup_Dialog roi_dialog(&pl,false);
	if (roi_dialog.exec()!=QDialog::Accepted)
		return false;

	roi_dialog.copyDataOut(&pl);

	// use full x to scan firstly
	fov_step_x = pl.FOVX_STEP;
	min_fovx = o_origin_x;	max_fovx = o_origin_x + o_sx*o_rez_x;	sx = (max_fovx-min_fovx)/fov_step_x;

	// save real x for recovering image
	real_bb_start_x = (V3DLONG)((pl.FOVX_MIN-o_origin_x)/fov_step_x + 0.5);
	real_bb_end_x   = (V3DLONG)((pl.FOVX_MAX-o_origin_x)/fov_step_x + 0.5);
	real_sx = pl.SX;
	old_min_fovx = pl.FOVX_MIN; old_max_fovx = pl.FOVX_MAX;

	// get following paras from ROI para dialog
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP; //(max_fovy-min_fovy+1)/sy;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP; //(max_fovz-min_fovz+1)/sz;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	sc = (ch<2)?1:2;


	set_voltages();
	if(!set_scanning_buffers())
		return false;
	if(!do_LSM_single())
		return false;


	return true;
}



bool LaserScanImaging::do_LSM_single(bool b_nopostprocessing) //laser scanning with single direction
{
	b_scanner_correction = true;

#ifdef FIND_DEBUG
	QString filename = ssConfigFolder + "Current_Debug_Info.txt";
	FILE *fp=fopen(filename.toStdString().c_str(), "wt");
	if (!fp)
	{
		v3d_msg(QString("Fail to open file %1 to write.").arg(filename));
		return false;
	}
#endif

	// imaging
	size_t start_t = clock();

	float alltime, lefttime;
	float usedtime = 0.0;
	size_t head_time;

	MicroscopeFocusControls dialog;
	// stop dialog
	if(imaging_op!=3) // 3 is for fucus control and we do not need this stop here
	{
		dialog.setText( QString("Laser Scanning...") );
		dialog.show();
	}

	// max value for display to user
	double max_val_red=0.0, max_val_green=0.0;
	double min_val_red=INF, min_val_green=INF;

	// initialize image to 0
	memset(pImFloat, 0.0, sx*sy*sz*sc);

	for(V3DLONG kn=0; kn<sz; kn++)
	{
		k = (b_scan_z_reverse==0)?kn:sz-kn-1;
		QCoreApplication::processEvents();

		bool flag_stop = dialog.flag_stop;
		if(flag_stop)
			return false;

		long offset_k = k*planesz;

		QString curslide;
		if(b_scan_z_reverse == 1)
		{
			qDebug()<<"..."<<sz-k<<"/"<<sz; // showing current
			curslide=QString("%1/%2").arg(sz-k-1).arg(sz);
		}else
		{
			qDebug()<<"..."<<k+1<<"/"<<sz; // showing current
			curslide=QString("%1/%2").arg(k+2).arg(sz);
		}

		size_t start_at = clock();

		for(j=0; j<sy; j++)
		{
			V3DLONG offset = j*sxsw;
			V3DLONG offset_x = offset;
			V3DLONG offset_y = planesz + offset;
			// for acc part for one line
			for(i=0; i<WAIT_PIXELS; i++)
			{
				//pAO[offset_x + i] = xacc_forward[i];
				pAO[offset_x + i] = xa[0];
				pAO[offset_y + i] = ya[j];
			}

			// for real image part
			for(i=0; i<sx; i++)
			{
				pAO[offset_x + WAIT_PIXELS+i] = xa[i];
				pAO[offset_y + WAIT_PIXELS+i] = ya[j];
			}
		}

		//imaging
		try
		{
			if(b_useSimImg == 0)
				DAQ_UpdateZ(za[k], dev_z, ao_z); // z

			int success;
			if(b_useSimImg ==0)
			{
				success = DAQ_getImg_ctr_ai(pCTR0, pCTR1, pAI0, pAI1, pAO, pAIxy, planesz, time_out,
											scanning_rate, duty_cycle, dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_clk, clk_n, dev_timing, timing_n,
											dev_red_ctr, red_n_ctr, dev_red_ai, red_n_ai, dev_green_ctr, green_n_ctr, dev_green_ai, green_n_ai);
			}
			else if(b_useSimImg ==1)
			{
				success = DAQ_getImg_sim(pCTR0, pCTR1, pAI0, pAI1, ch, sz_ctr, sx, sy, sz, planesz, sxsw, WAIT_PIXELS);
			}

			if(success)
			{
				v3d_msg("Failed in microscope imaging.\n", 0);
				return false;
			}

			wait(WAITING_PERSLICE); // every slice
		}
		catch (...)
		{
			v3d_msg("Invoking DAQ_getImg failed.\n", 0);
			return false;
		}

		if (b_nopostprocessing) //by PHC 20120523
			return true; //to process next kth slice ZJL

		V3DLONG offsets_k = k*sx*sy;

		// for calibration part
		if(b_scanner_correction)
		{
			// this version save (pAIx, pAOcx) of the entire 3d image.
			if(k==0)
			{
				for(j=0; j<sy; j++)
				{
					V3DLONG offset = j*sxsw;
					V3DLONG offsets = j*sx;

					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						//assign
						V3DLONG idx = offset + i;
						V3DLONG idx_ori = offsets+ i-WAIT_PIXELS;

						// ai x
						pAIx[idx_ori] = pAIxy[idx];
						// ao x
						pAOcx[idx_ori] = pAO[idx];
						// aiy
						//pAIy[idx_ori] = pAIxy[idx + planesz];
					}
				}
				// crate real pos lut
                if(!create_real_pos_LUT(pAIx, pAOcx))
                    return false;
				// copy back pAIx based on LUT
				for(j=0; j<sy; j++)
				{					
					for(i=WAIT_PIXELS; i<sxsw; i++)
					{
						bool b_zero = false;
						V3DLONG real_x_pos_i = real_to_exp_pos_LUT[i-WAIT_PIXELS]; // get  real pos from LUT
						
						if(real_x_pos_i < 0 || real_x_pos_i>=sx) // real_x_pos_i is out effective range
						{
							real_x_pos_i = i-WAIT_PIXELS; 
							b_zero = true;
						}
						
						V3DLONG sbuffer_idx = j*sxsw + i; 
						V3DLONG oimg_idx = j*sx + real_x_pos_i; 
						
						if(!b_zero)
						{
						   pAIx[oimg_idx] = pAIxy[sbuffer_idx];
						}
					}
				}
			}

			// only save (pAIx, pAOcx) at the first line on the first slice
			//if(k==0)
			//{	// j =0
			//	for(i=WAIT_PIXELS; i<sxsw; i++)
			//	{
			//		//assign
			//		V3DLONG idx = i;
			//		V3DLONG idx_ori = i-WAIT_PIXELS;

			//		// ai x y
			//		pAIx[idx_ori] = pAIxy[idx];
			//		//pAIy[idx_ori] = pAIxy[idx + planesz]; // pAIy, pAOcy are not used at the moment

			//		pAOcx[idx_ori] = pAO[idx];
			//		//pAOcy[idx_ori] = pAO[idx + planesz];
			//	}

                //// crate real pos lut
                //if(!create_real_pos_LUT(pAIx, pAOcx))
                //    return false;
			//}
		}

		// assign data back and check max value for display
		for(j=0; j<sy; j++)
		{
			V3DLONG scanbuffer_curline_startpos = j*sxsw; //offset
			V3DLONG outimg_curline_startpos = j*sx + offsets_k; //offsets

			for(i=WAIT_PIXELS; i<sxsw; i++)
			{
				float tmp;

				int n_ch = (ch>=2) ? 2 : 1; //n_ch is the number of channels
				for (V3DLONG curch=0;curch<n_ch;curch++)
				{
					//assign
					bool b_set_zero = false;
					V3DLONG real_x_pos = real_to_exp_pos_LUT[i-WAIT_PIXELS]; // get  real pos from LUT
					//printf("real_x_pos: %d", real_x_pos);
					if(real_x_pos < 0 || real_x_pos>=sx) // real_x_pos is out effective range
					{
						real_x_pos = i-WAIT_PIXELS; // Not used actually
						b_set_zero = true;
					}

					V3DLONG scanbuffer_idx = scanbuffer_curline_startpos + i; //idx
					V3DLONG outimg_idx = outimg_curline_startpos + real_x_pos;  //(i-WAIT_PIXELS); //idx_ori

					//note the channel id should need some optimization, now is confusing!
					if (n_ch==1)
					{
						if(ch==0) // green channel
						{
							if(aictr_green==0) //ai
								tmp=(float)pAI0[scanbuffer_idx];
							else               //ctr
								tmp=pCTR0[scanbuffer_idx];
						}
						else if(ch==1) //red channel
						{
							if(aictr_red==0) //ai
								tmp=(float)pAI1[scanbuffer_idx];
							else             //ctr
								tmp=pCTR1[scanbuffer_idx];
						}
					}
					else//n_ch==2
					{
						if(curch==1) // green channel
						{
							if(aictr_green==0) //ai
								tmp=(float)pAI0[scanbuffer_idx];
							else               //ctr
								tmp=pCTR0[scanbuffer_idx];
						}
						else if(curch==0) //red channel
						{
							if(aictr_red==0) //ai
								tmp=(float)pAI1[scanbuffer_idx];
							else             //ctr
								tmp=pCTR1[scanbuffer_idx];
						}
					}

					// consider offset_xp in idx_ori
					if(!b_useSimImg)
					{
						if(!b_set_zero)
						{
						   pImFloat[curch*offset_image + outimg_idx] = tmp;
						}
					}
					else
					{
						pImFloat[curch*offset_image + outimg_idx] = tmp;
					}
				}
			}// end for i
		} // end for j

		// for information display
		size_t end_at = clock();
		head_time=end_at - start_at;
		if(b_scan_z_reverse==1)
		{
			//estimate time left
			lefttime=(float)(k+1)*head_time/CLOCKS_PER_SEC;
			alltime = (float)sz*head_time/CLOCKS_PER_SEC;
			usedtime = usedtime + (float)head_time/CLOCKS_PER_SEC;

			qDebug()<<"Current max red value:"<<max_val_red<< ","<<"max green value:"<<max_val_green;
			qDebug()<<"Already Spent Time:"<<usedtime<< "s";
			qDebug()<<"Estimated Remaining Time:"<<lefttime<<"/"<<alltime << "s";
			qDebug()<<"Z Scanning reversed!\n";
		}else
		{
			//estimate time left
			lefttime=(float)(sz-k-1)*head_time/CLOCKS_PER_SEC;
			alltime = (float)sz*head_time/CLOCKS_PER_SEC;
			usedtime = usedtime + (float)head_time/CLOCKS_PER_SEC;

			qDebug()<<"Current max red value:"<<max_val_red<< ","<<"max green value:"<<max_val_green;
			qDebug()<<"Already Spent Time:"<<usedtime<< "s";
			qDebug()<<"Estimated Remaining Time:"<<lefttime<<"/"<<alltime << "s";
		}
		if(imaging_op!=3)
			dialog.setText(
				QString("Already spent time: %1 s, Remaining Time: %2 s,\n Current scanning: %3\n").arg(usedtime).arg(lefttime).arg(curslide) );

			//dialog.setText(
			//	QString("Current max red value: %1, max green value: %2.\n Already spent time: %3 s, Remaining Time: %4 s,\n Current scanning: %5\n").arg(max_val_red).arg(max_val_green).arg(usedtime).arg(lefttime).arg(curslide) );

		//now process events
		QCoreApplication::processEvents();
	} // for k

	size_t end_t = clock();
	printf("time elapse ... %d \n", end_t-start_t);

#ifdef FIND_DEBUG
	if(fp) fclose(fp);
#endif

	return true;
}


// pAI pos to pAO pos
bool LaserScanImaging::create_real_pos_LUT(float* pAIx_in, float* pAOx_in)
{
	if(!pAIx_in || !pAOx_in)
	{
		v3d_msg("Parameters to create_real_pos_LUT() are not valid! Do nothing");
		return false;
	}

	// pAIx is pAIxy on one line [0 - sx]
	// pAOcx is pAO on one line [0 - sx]
	// LUT: size is sx
	if(real_to_exp_pos_LUT) {delete []real_to_exp_pos_LUT; real_to_exp_pos_LUT=0;}
	real_to_exp_pos_LUT = new V3DLONG [sx];
	memset(real_to_exp_pos_LUT, -1, sx); // initialize to -1

    if(exp_to_real_pos_LUT) {delete []exp_to_real_pos_LUT; exp_to_real_pos_LUT=0;}
	exp_to_real_pos_LUT = new V3DLONG [sx];
	memset(exp_to_real_pos_LUT, -1, sx); // initialize to -1

	// compute line equ
	// this boundary is based on the fact that high speed AI is always shifted to right
	// at most about 20 pixels (200k Hz) from low speed AI
	V3DLONG boundary = 25;
	V3DLONG start_i = boundary, end_i = sx - boundary/2;

	if(start_i>sx || end_i<0)
	{
		v3d_msg("sx is too small for the calculation of LUT.");
		return false;
	}

	V3DLONG num = end_i - start_i + 1;
	if(num < 5)
	{
		v3d_msg("sx is too small for calculation of LUT.");
		return false;
	}

	float *pX_eq  = new float[num];   // for line equation
	float *pAI_eq = new float[num];   // for line equation
	float *pX1_eq = new float[num];   // for line equation
	float *pAO_eq = new float[num];   // for line equation

	// only use the first line of pAIx, pAOcx (0~sx-1)
	for(V3DLONG i=start_i; i<=end_i; i++)
	{
		pAI_eq[i - start_i] = pAIx_in[i]; //pAIx
		pAO_eq[i - start_i] = pAOx_in[i]; //pAOcx
		pX_eq[i-start_i]  = i;
		pX1_eq[i-start_i] = i;
	}

	double intercept_real, slope_real;

	getLineEqu(intercept_real, slope_real, pX_eq, pAI_eq, num);
	printf("Real line intercept and slope: %.4f, %.4f \n", intercept_real, slope_real);

	double intercept_exp, slope_exp;
	getLineEqu(intercept_exp, slope_exp, pX1_eq, pAO_eq, num);
	printf("Expected line intercept and slope: %.4f, %.4f \n", intercept_exp, slope_exp);
	//v3d_msg(QString("Expected line intercept and slope: %1, %2").arg(intercept_exp).arg(slope_exp));

	if(!pX_eq) {delete []pX_eq;  pX_eq =0;}
	if(!pAI_eq){delete []pAI_eq; pAI_eq=0;}
	if(!pX1_eq){delete []pX1_eq; pX1_eq=0;}
	if(!pAO_eq){delete []pAO_eq; pAO_eq=0;}

	// if the scope is off, pAIx is close to zero (random value). How to judge this case?
	if(abs(slope_real) < 1e-5) // this threshold should be set from experiments, e.g. the slope_real is around 0.0039 for a scanning at 200k Hz
	{
		// if the scope is off, the line of pAIx is close to X axis, we set pAIx and pAO lines
		// are same in order to make program run. But this has no meaning.
		// Create lut
		for(V3DLONG i=0; i<sx; i++)
		{
			real_to_exp_pos_LUT[i] = i;
            exp_to_real_pos_LUT[i] = i;
		}
		cal_offset_left = 0;
		cal_offset_right = 0;
	}
	else
	{
		// Create lut
		for(V3DLONG i=0; i<sx; i++)
		 {
            // real_to_expected LUT
			float real_val = i * slope_real + intercept_real;
			V3DLONG index_exp = (V3DLONG)((real_val - intercept_exp)/slope_exp + 0.5);
			real_to_exp_pos_LUT[i] = index_exp;

			// now i is expected pos, get its real out pos
			float exp_val = i * slope_exp + intercept_exp;
			V3DLONG index_real = (V3DLONG)((exp_val - intercept_real)/slope_real + 0.5);
			exp_to_real_pos_LUT[i] = index_real;
		 }

		 // get left and right correction offset without boundary
		 float left_val_exp = intercept_exp; // 0*slope_exp+intercept_exp
		 V3DLONG index_left_real = (V3DLONG)((left_val_exp - intercept_real)/slope_real + 0.5);
		 cal_offset_left = abs(index_left_real);

		 float right_val_exp = sx*slope_exp + intercept_exp; // from line equ
		 V3DLONG index_right_real = (V3DLONG)((right_val_exp - intercept_real)/slope_real + 0.5);
		 cal_offset_right = abs(index_right_real - sx);
	}

    return true;
}



// This function computes scanner calibration: get offset value in x direction
void LaserScanImaging::getCalibOffset(FILE *fp, int &offset_xp)
{
	if(!b_useSimImg)
	{
		// read saved AIx, AOx
		QString filename=ssConfigFolder + "calibratingAO2AI_SR_AIx_AOx.txt";
		float *pAIx_in;
		float *pAOx_in;
		try{
			pAIx_in = new float [256];
			pAOx_in = new float [256];
		}
		catch (...)
		{
			v3d_msg("Failed to allocate memory for pAIx_in, pAOx_in.", 0);
			return ;
		}

		V3DLONG size=256;
		// size will be updated after this calling
		readAIAOFile(filename, pAIx_in, pAOx_in, size);

		// we only need to take middle of the line for calib
		V3DLONG ps = (V3DLONG)size/4;
		V3DLONG pe = (V3DLONG)size*3/4;

		V3DLONG nPoint = pe - ps + 1;
		float *pAIx_eq;
		float *pX;
		try{
			pAIx_eq= new float[nPoint];
			pX = new float[nPoint];
		}
		catch (...)
		{
			v3d_msg("Failed to allocate memory for pAIx_eq, pX.", 0);
			return ;
		}

		for(V3DLONG i=ps; i<=pe; i++)
		{
			pAIx_eq[i - ps] = pAIx_in[i];
			pX[i-ps]=pAOx_in[i]; //i; for not using pAO
		}

		// get offset and slope of the line
		double offset_in, slope_in;
		getLineEqu(offset_in, slope_in, pX, pAIx_eq, nPoint);
		printf("Offset and slope for slow speed: %.4f, %.4f \n", offset_in, slope_in);

#ifdef FIND_DEBUG
		fprintf(fp, "Offset and slope for slow speed: %.4f, %.4f \n", offset_in, slope_in);
#endif
		// get the cross point of line and x-axis: y=ax+b (y=0), so x=-b/a
		double interx_in;
		interx_in = -offset_in/slope_in;

		// get live cross point of current (AOx-AIx) line and x-axis
		//-------------------------------------------------
		// for testing
		//QString filenamelive=ssConfigFolder + "LSM_20111202_43_200000Hz_sx_301_x_xrange_0-300_um_AIx_AOx.txt";
		//float *pAIx_lin = new float [256];
		//float *pAOx_lin = new float [256];
		//V3DLONG lsize=256;
		//// lsize will be updated after this calling
		//readAIAOFile(filenamelive, pAIx_lin, pAOx_lin, lsize);

		//V3DLONG lps = (V3DLONG) lsize/4;
		//V3DLONG lpe = (V3DLONG) lsize*3/4;
		//-------------------------------------------------

		// we only need to take middle of the line for calib
		V3DLONG lps = (V3DLONG) sx/4;   // this is for live scanning
		V3DLONG lpe = (V3DLONG) sx*3/4;

		if(sx<4)
		{
			lps = sx-2;
			lpe = sx-1;
		}

		V3DLONG nLPoint = lpe - lps+1;
		float *pAIx_leq = new float[nLPoint];
		float *pXL = new float[nLPoint];

		for(V3DLONG i=lps; i<=lpe; i++)
		{
			//pAIx_leq[i - lps] = pAIx_lin[i]; // for testing
			//pXL[i-lps] = pAOx_lin[i];
			pAIx_leq[i - lps] = pAIx[i]; // this is from live pAIx, first line on first slice
			pXL[i-lps] = pAOcx[i]; //i;
		}

		double offset_cur, slope_cur;
		getLineEqu(offset_cur, slope_cur, pXL, pAIx_leq, nLPoint);
		printf("Current offset and slope: %.4f, %.4f \n", offset_cur, slope_cur);

#ifdef FIND_DEBUG
		fprintf(fp, "Current offset and slope: %.4f, %.4f \n", offset_cur, slope_cur);
#endif

		// de-alloc
		if (pAIx_in) {delete []pAIx_in; pAIx_in=0;}
		if (pAOx_in) {delete []pAOx_in; pAOx_in=0;}

		//if (pAIx_lin) {delete []pAIx_lin; pAIx_lin=0;}
		//if (pAOx_lin) {delete []pAOx_lin; pAOx_lin=0;}

		if (pAIx_eq) {delete []pAIx_eq; pAIx_eq=0;}
		if (pX) {delete []pX; pX=0;}

		if (pAIx_leq) {delete []pAIx_leq; pAIx_leq=0;}
		if (pXL) {delete []pXL; pXL=0;}

		// get the cross point of cur_line and x-axis: y=ax+b (y=0), so x=-b/a
		// check
		double interx_cur;
		if(slope_cur==0.0)
		{
			interx_cur = 0;
			v3d_msg("Current offset value is not normal. Please check scanning rate!\n",0);
		}else
		{
			interx_cur = -offset_cur/slope_cur;
		}

		// get the absolute offset of image
		double offset_xv = interx_cur - interx_in;
		// offset_xv is in volt, need to be transformed to value in pixel
		offset_xp = (int) (offset_xv * ratio_x / fov_step_x);

		printf("Current offset value in X direction in volt is: %.3f.\n", offset_xv);
		printf("Current offset value in X direction in pixel is: %d.\n", offset_xp);

#ifdef FIND_DEBUG
		fprintf(fp, "Current offset value in X direction in volt is: %.3f.\n", offset_xv);
		fprintf(fp, "Current offset value in X direction is: %d.\n", offset_xp);
#endif

		// check
		if((offset_xp >= sx)||(offset_xp<0))
		{
			offset_xp = 0;
			v3d_msg("Current offset value is not normal. Please check scanning rate! The current result image has no offset.\n",0);
		}
	}
}




// ablation: stimulating with strong laser: the version not using WAIT_PIXELS
void LaserScanImaging::do_image_ablation(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks)
{
	qDebug()<<"............. Now running ABLATION ...............";


	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	logname.chop(4);
	logname.append(".txt");

	Parameters_LSM pl;
	pl.load_ParameterFile(logname);
	cout << "logfile name:"<<logname.toStdString()<<endl;

	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;
	aictr_green = pl.AI_COUNTER_GREEN; aictr_red = pl.AI_COUNTER_RED;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	bool b_trans=false;
	if (b_trans) //by PHC, 20120507
	{
		min_fovx = -150;	max_fovx = 150;	sx = 600;	fov_step_x = 0.5;
		min_fovy = -150;	max_fovy = 150;	sy = 600;	fov_step_y = 0.5;
		min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;
	}
	else
	{
		min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP;
		min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
		min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;
	}

	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	sc = (ch<2)?1:2;


	// load ablation paras from log file
	QString m_FileName_ma = ssConfigFolder + "markerablation_configuration.log";

	if(!QFile(m_FileName_ma).exists())
	{
		m_FileName_ma = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
												  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(m_FileName_ma.isEmpty())
			return;
	}

	int NPULSE, stimulate_waiting;
	int region_radius;

	StimulatingParas para_abl;
	para_abl.load_ParaFile(m_FileName_ma);

	NPULSE = para_abl.NPULSE;
	stimulate_waiting = para_abl.WAITING;
	region_radius = para_abl.REGION_RADIUS;
	scanning_rate = para_abl.SCANNING_RATE; //by PHC, there is also a potential problem and needs real-time calibration, but maybe fine now as the scan rate is just 1000Hz.

	// avoid mismatch image with configuration file
	V3DLONG sx_image = pImgIn->getXDim();
	V3DLONG sy_image = pImgIn->getYDim();
	V3DLONG sz_image = pImgIn->getZDim();

	//======= initialization for ablation======================
	int dev_ao = 1;
	sc = 1;
	ao_x=0, ao_y=1, ao_z=2;
	dev_clk = 1; clk_n = 1;
	duty_cycle = 0.5;
	//=========================================================

    size_t start_t = clock();

	set_voltages();

	// get parameters for ablation
	V3DLONG pagesz = list_landmarks.size(); // the number of markers
	V3DLONG *pSortAO=NULL;
	V3DLONG *flag_markers = NULL; // record order info of landmarkers
	V3DLONG offset_y = pagesz, offset_z = 2*pagesz;

	// sorting markers
	try
	{
		pSortAO = new V3DLONG [3*pagesz]; // x,y,z
		flag_markers = new V3DLONG [pagesz];

		for(V3DLONG i_marker=0; i_marker<pagesz; i_marker++)
		{
			//do a transformation, by PHC, 120507
			V3DLONG cx, cy, cz;
			if (b_trans)
			{
				cx = (list_landmarks.at(i_marker).x * pl.FOVX_STEP + pl.FOVX_MIN - min_fovx)/fov_step_x;
				cy = (list_landmarks.at(i_marker).y * pl.FOVY_STEP + pl.FOVY_MIN - min_fovy)/fov_step_y;
				cz = (list_landmarks.at(i_marker).z);
				//V3DLONG cz = (list_landmarks.at(i_marker).z * pl.FOVZ_STEP + pl.FOVZ_MIN - min_fovz)/fov_step_z;
				v3d_msg(QString("%1 %2").arg(list_landmarks.at(i_marker).x).arg(cx));
			}
			else
			{
				cx = (list_landmarks.at(i_marker).x);
				cy = (list_landmarks.at(i_marker).y);
				cz = (list_landmarks.at(i_marker).z);
			}
			// init by original order
			pSortAO[i_marker] = cx-1;
			pSortAO[offset_y + i_marker] = cy-1;
			pSortAO[offset_z + i_marker] = cz-1;

			// init order flag info
			flag_markers[i_marker] = i_marker;
		}

		if(marker_sorted)
		{
			// sorting z
			quickSort(pSortAO, 0L, pagesz-1, pagesz, flag_markers, 0L); // along z

			// sorting y
			V3DLONG start_markers_sort=0;
			V3DLONG count_markers_sort=0;
			for(V3DLONG y_markers=0; y_markers<pagesz; y_markers++)
			{
				count_markers_sort++;
				if(y_markers>=1)
				{
					if(pSortAO[y_markers + offset_z] != pSortAO[y_markers + offset_z - 1] || (y_markers==pagesz-1) )
					{
						if(y_markers==pagesz-1 && pSortAO[y_markers + offset_z] == pSortAO[y_markers + offset_z - 1])
						{
							count_markers_sort++;
						}

						if(start_markers_sort==0)
						{
							count_markers_sort--;
						}

						if(count_markers_sort>1)
							quickSort(pSortAO, start_markers_sort, start_markers_sort+count_markers_sort-1, pagesz, flag_markers, 1L); // along y

						count_markers_sort=0;
						start_markers_sort = y_markers;
					}
				}
			}

			// sorting x
			start_markers_sort=0;
			count_markers_sort=0;
			for(V3DLONG x_markers=0; x_markers<pagesz; x_markers++)
			{
				count_markers_sort++;
				if(x_markers>=1)
				{
					if(pSortAO[x_markers + offset_y] != pSortAO[x_markers + offset_y - 1] || (x_markers == pagesz-1) )
					{
						if(x_markers==pagesz-1 && pSortAO[x_markers + offset_y] == pSortAO[x_markers + offset_y - 1])
						{
							count_markers_sort++;
						}

						if(start_markers_sort==0)
						{
							count_markers_sort--;
						}

						if(count_markers_sort>1)
							quickSort(pSortAO, start_markers_sort, start_markers_sort+count_markers_sort-1, pagesz, flag_markers, 2L); // along x

						count_markers_sort=0;
						start_markers_sort = x_markers;
					}
				}
			}
		} // sorting markers according z y x locations
	}
	catch (...)
	{
		printf("Fail to allocate memory for pSortAO.\n");
		return;
	}

    double *pAO = 0;

	try
	{
		size_t point_t = clock();

		float alltime, lefttime;
		float usedtime = 0.0;
		size_t head_time;

		WAIT_PIXELS = waiting_pxls(scanning_rate);//200; // [0, 100] for accelerate and [100, 200] for cache
		if(WAIT_PIXELS<1) WAIT_PIXELS=2;


		pAO = new double [3]; // x,y,z

		MicroscopeFocusControls dialog;
		// stop dialog
		dialog.setText( QString("Laser stimulation...") );
		dialog.show();

		// stimulating
		for(V3DLONG i_marker=0; i_marker<pagesz; i_marker++)
		{
			// use region_radius in ablation
			V3DLONG bb_start_x, bb_start_y, bb_start_z, bb_end_x, bb_end_y, bb_end_z;
			V3DLONG mx = pSortAO[i_marker];
			V3DLONG my = pSortAO[i_marker+offset_y];
			V3DLONG mz = pSortAO[i_marker+offset_z];

			qDebug()<< "mx, my, mz:"<<mx<<" "<<my<<" "<<mz;

			// create a ROI with marker's position and region_radius
			// ROI is a cube with the size of 2*region_radius+1
			bb_start_x = mx-region_radius; bb_end_x = mx+region_radius;
			bb_start_y = my-region_radius; bb_end_y = my+region_radius;
			bb_start_z = mz-region_radius; bb_end_z = mz+region_radius;

			// make ROI inside the volume
			bb_start_x = qBound(V3DLONG(0), V3DLONG(bb_start_x), sx-1);
			bb_start_y = qBound(V3DLONG(0), V3DLONG(bb_start_y), sy-1);
			bb_start_z = qBound(V3DLONG(0), V3DLONG(bb_start_z), sz-1);

			bb_end_x = qBound(V3DLONG(0), V3DLONG(bb_end_x), sx-1);
			bb_end_y = qBound(V3DLONG(0), V3DLONG(bb_end_y), sy-1);
			bb_end_z = qBound(V3DLONG(0), V3DLONG(bb_end_z), sz-1);

			V3DLONG bx = bb_end_x-bb_start_x+1, by = bb_end_y-bb_start_y+1, bz = bb_end_z-bb_start_z+1;
			V3DLONG region_size = bx*by*bz;

			qDebug()<< "bx, by, bz:"<<bx<<" "<<by<<" "<<bz;

			// loop all points for ablation
			size_t start_DAQ=clock();
			for(V3DLONG k=bb_start_z; k<=bb_end_z; k++)
			{
				for(V3DLONG j=bb_start_y; j<=bb_end_y; j++)
				{
					for(V3DLONG i=bb_start_x; i<=bb_end_x; i++)
					{
						// for stop dialog
						QCoreApplication::processEvents();
						bool flag_stop = dialog.flag_stop;
						if(flag_stop)
							return;

						// set pAO
                        // calibrated x_pos
                        //V3DLONG calib_x_pos = real_to_exp_pos_LUT[i]; // or use exp_to_real_pos_LUT[i]
						//pAO[0] = xa[calib_x_pos];
                        pAO[0] = xa[i];
						pAO[1] = ya[j];
						pAO[2] = za[k];

						// stimulate
						size_t start_at = clock();
						try
						{
							double aix_feedback;
							// ablation
							DAQ_Stimulating(pAO, dev_ao, ao_x, ao_y, ao_z, dev_clk, clk_n, scanning_rate, duty_cycle, NPULSE);
							// shutter on
							DAQ_Shutter_Stimulation(true);
							// hold on
							wait_milliseconds(stimulate_waiting);
							// shutter off
							DAQ_Shutter_Stimulation(false);
						}
						catch (...)
						{
							printf("Function of DAQ_Stimulating fail.\n");
							return;
						}

						// time display
						size_t end_at = clock();
						head_time=end_at - start_at;
						V3DLONG index_point = bx*by*(k-bb_start_z) + bx*(j-bb_start_y)+ i-bb_start_x;
						//estimate time left
						lefttime=(float)((pagesz-i_marker) * region_size - index_point - 1)*head_time/CLOCKS_PER_SEC;
						alltime = (float)pagesz * region_size * head_time/CLOCKS_PER_SEC;
						usedtime = usedtime + (float)head_time/CLOCKS_PER_SEC;

						qDebug()<<"Already Spent Time:"<<usedtime<< "s";
						qDebug()<<"Estimated Remaining Time:"<<lefttime<<"/"<<alltime << "s";
						dialog.setText( QString("Already spent time: %1 s, Remaining Time: %2 s.\n").arg(usedtime).arg(lefttime) );

						//now process events
						QCoreApplication::processEvents();
					}
				}
			}

			size_t end_DAQ=clock();
			qDebug()<<"time elapse for stimulating one marker ..."<< end_DAQ-start_DAQ;
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for AI and AO.\n");
		return;
	}

	//de-alloc
	if(pAO) {delete []pAO; pAO=0;}
	if(pSortAO) {delete []pSortAO; pSortAO=0;}
	if(flag_markers) {delete []flag_markers; flag_markers=0;}

	//reset
	DAQ_Reset(dev_x, dev_y, dev_z, ao_x, ao_y, ao_z);

	int end_t = clock();
	printf("time elapse ... %d \n", end_t-start_t);

}


// ablation: stimulating with strong laser: the version not using WAIT_PIXELS
void LaserScanImaging::do_image_ablation_with_feedback(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks)
{
	qDebug()<<"............. Now running ABLATION ...............";

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	logname.chop(4);
	logname.append(".txt");

	Parameters_LSM pl;
	pl.load_ParameterFile(logname);
	cout << "logfile name:"<<logname.toStdString()<<endl;

	// get file name of AIx image of the current 3d image
	QString aix_name = ((V3DPluginCallback *)cb)->getImageName(curwin);
	aix_name.chop(4);
	aix_name.append("_AIx.raw");
	cout << "AIx image name: "<<aix_name.toStdString()<<endl;

	// load AIx image to the buffer. This is not used at the moment. 
	unsigned char * pAIx_orig = 0;
	V3DLONG * in_sz = 0;
	int datatype;
	if(!loadImage((char*)qPrintable(aix_name), pAIx_orig, in_sz, datatype))
	{
		cerr<<"load image "<<qPrintable(aix_name)<<" error!"<<endl;
		return;
	}
	// get file name of AIx txt of the curren 3d image
	QString aix_txt_name = ((V3DPluginCallback *)cb)->getImageName(curwin);
	aix_txt_name.chop(4);
	aix_txt_name.append("_AIx.txt");
	cout << "AIx image name: "<<aix_txt_name.toStdString()<<endl;


	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;
	aictr_green = pl.AI_COUNTER_GREEN; aictr_red = pl.AI_COUNTER_RED;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP;
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;

	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	sc = (ch<2)?1:2;

	// load ablation paras from log file
	QString m_FileName_ma = ssConfigFolder + "markerablation_configuration.log";

	if(!QFile(m_FileName_ma).exists())
	{
		m_FileName_ma = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
												  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(m_FileName_ma.isEmpty())
			return;
	}

	int NPULSE, stimulate_waiting;
	int region_radius;

	StimulatingParas para_abl;
	para_abl.load_ParaFile(m_FileName_ma);

	NPULSE = para_abl.NPULSE;
	stimulate_waiting = para_abl.WAITING;
	region_radius = para_abl.REGION_RADIUS;
	scanning_rate = para_abl.SCANNING_RATE; 

	// avoid mismatch image with configuration file
	V3DLONG sx_image = pImgIn->getXDim();
	V3DLONG sy_image = pImgIn->getYDim();
	V3DLONG sz_image = pImgIn->getZDim();

	if(!(sx==sx_image && sy==sy_image && sz==sz_image ))
	{
		qDebug()<<"Image and configuration are not match!";
		return;
	}

	//======= initialization for ablation======================
	int dev_ao = 1;
	sc = 1;
	ao_x=0, ao_y=1, ao_z=2;
	dev_clk = 1; clk_n = 1;
	duty_cycle = 0.5;
	//=========================================================

    size_t start_t = clock();

	set_voltages();

	// get parameters for ablation
	V3DLONG pagesz = list_landmarks.size(); // the number of markers
	V3DLONG *pSortAO=NULL;
	V3DLONG *flag_markers = NULL; // record order info of landmarkers
	V3DLONG offset_y = pagesz, offset_z = 2*pagesz;

	// sorting markers
	try
	{
		pSortAO = new V3DLONG [3*pagesz]; // x,y,z
		flag_markers = new V3DLONG [pagesz];

		for(V3DLONG i_marker=0; i_marker<pagesz; i_marker++)
		{
			//do a transformation, by PHC, 120507
			V3DLONG cx, cy, cz;
			cx = (list_landmarks.at(i_marker).x);
			cy = (list_landmarks.at(i_marker).y);
			cz = (list_landmarks.at(i_marker).z);
		
			// init by original order
			pSortAO[i_marker] = cx-1;
			pSortAO[offset_y + i_marker] = cy-1;
			pSortAO[offset_z + i_marker] = cz-1;

			// init order flag info
			flag_markers[i_marker] = i_marker;
		}

		if(marker_sorted)
		{
			// sorting z
			quickSort(pSortAO, 0L, pagesz-1, pagesz, flag_markers, 0L); // along z

			// sorting y
			V3DLONG start_markers_sort=0;
			V3DLONG count_markers_sort=0;
			for(V3DLONG y_markers=0; y_markers<pagesz; y_markers++)
			{
				count_markers_sort++;
				if(y_markers>=1)
				{
					if(pSortAO[y_markers + offset_z] != pSortAO[y_markers + offset_z - 1] || (y_markers==pagesz-1) )
					{
						if(y_markers==pagesz-1 && pSortAO[y_markers + offset_z] == pSortAO[y_markers + offset_z - 1])
						{
							count_markers_sort++;
						}

						if(start_markers_sort==0)
						{
							count_markers_sort--;
						}

						if(count_markers_sort>1)
							quickSort(pSortAO, start_markers_sort, start_markers_sort+count_markers_sort-1, pagesz, flag_markers, 1L); // along y

						count_markers_sort=0;
						start_markers_sort = y_markers;
					}
				}
			}

			// sorting x
			start_markers_sort=0;
			count_markers_sort=0;
			for(V3DLONG x_markers=0; x_markers<pagesz; x_markers++)
			{
				count_markers_sort++;
				if(x_markers>=1)
				{
					if(pSortAO[x_markers + offset_y] != pSortAO[x_markers + offset_y - 1] || (x_markers == pagesz-1) )
					{
						if(x_markers==pagesz-1 && pSortAO[x_markers + offset_y] == pSortAO[x_markers + offset_y - 1])
						{
							count_markers_sort++;
						}

						if(start_markers_sort==0)
						{
							count_markers_sort--;
						}

						if(count_markers_sort>1)
							quickSort(pSortAO, start_markers_sort, start_markers_sort+count_markers_sort-1, pagesz, flag_markers, 2L); // along x

						count_markers_sort=0;
						start_markers_sort = x_markers;
					}
				}
			}
		} // sorting markers according z y x locations
	}
	catch (...)
	{
		printf("Fail to allocate memory for pSortAO.\n");
		return;
	}

    double *pAO = 0;

	try
	{
		size_t point_t = clock();

		float alltime, lefttime;
		float usedtime = 0.0;
		size_t head_time;

		WAIT_PIXELS = waiting_pxls(scanning_rate);//200; 
		if(WAIT_PIXELS<1) WAIT_PIXELS=2;

		pAO = new double [3]; // x,y,z

		MicroscopeFocusControls dialog;
		// stop dialog
		dialog.setText( QString("Laser stimulation...") );
		dialog.show();

		// get line equation of aix_orig on one line
		float *pLineX  = new float[sx];   // for line equation
		float *pLineAIx = new float[sx];   // for line equation

		// open aix_txt_name and load data
		ifstream file_AIx_txt(aix_txt_name.toStdString().c_str());

		if(file_AIx_txt.is_open())
		{
			string line;
			getline(file_AIx_txt, line);
			istringstream buffer(line);

			for(V3DLONG i=0; i<sx; i++)
				buffer >> pLineAIx[i];
		}
		file_AIx_txt.close();

		// only use the first line of pAIx_orig (j=0)
		for(V3DLONG i=0; i<sx; i++)
		{
			//pLineAIx[i] = (float)pAIx_orig[i]; //pAIx
			pLineX[i]  = (float)i;
		}

		double line_offset, line_slope;
		V3DLONG ncount = sx;
		// get line equation: y = line_slope * x + line_offset
		getLineEqu(line_offset, line_slope, pLineX, pLineAIx, ncount);
		//printf("Line offset: %lf, line slope: %lf", line_offset, line_slope);


		// stimulating
		for(V3DLONG i_marker=0; i_marker<pagesz; i_marker++)
		{
			size_t start_DAQ=clock();

			// use region_radius in ablation
			V3DLONG bb_start_x, bb_start_y, bb_start_z, bb_end_x, bb_end_y, bb_end_z;
			V3DLONG mx = pSortAO[i_marker];
			V3DLONG my = pSortAO[i_marker+offset_y];
			V3DLONG mz = pSortAO[i_marker+offset_z];

			//qDebug()<< "mx, my, mz:"<<mx<<" "<<my<<" "<<mz;

			// create a ROI with marker's position and region_radius
			// ROI is a cube with the size of 2*region_radius+1
			bb_start_x = mx-region_radius; bb_end_x = mx+region_radius;
			bb_start_y = my-region_radius; bb_end_y = my+region_radius;
			bb_start_z = mz-region_radius; bb_end_z = mz+region_radius;

			// make ROI inside the volume
			bb_start_x = qBound(V3DLONG(0), V3DLONG(bb_start_x), sx-1);
			bb_start_y = qBound(V3DLONG(0), V3DLONG(bb_start_y), sy-1);
			bb_start_z = qBound(V3DLONG(0), V3DLONG(bb_start_z), sz-1);

			bb_end_x = qBound(V3DLONG(0), V3DLONG(bb_end_x), sx-1);
			bb_end_y = qBound(V3DLONG(0), V3DLONG(bb_end_y), sy-1);
			bb_end_z = qBound(V3DLONG(0), V3DLONG(bb_end_z), sz-1);

			V3DLONG bx = bb_end_x-bb_start_x+1, by = bb_end_y-bb_start_y+1, bz = bb_end_z-bb_start_z+1;
			V3DLONG region_size = bx*by*bz;

			//qDebug()<< "bx, by, bz:"<<bx<<" "<<by<<" "<<bz;

			// loop all points for ablation
			for(V3DLONG k=bb_start_z; k<=bb_end_z; k++)
			{
				for(V3DLONG j=bb_start_y; j<=bb_end_y; j++)
				{
					for(V3DLONG i=bb_start_x; i<=bb_end_x; i++)
					{
						// for stop dialog
						QCoreApplication::processEvents();
						bool flag_stop = dialog.flag_stop;
						if(flag_stop)
							return;

						// set pAO
                        pAO[0] = xa[i];
						pAO[1] = ya[j];
						pAO[2] = za[k];
						//printf("pAOx, pAOy, pAOz: %lf, %lf, %lf \n", pAO[0], pAO[1], pAO[2]);

						// stimulate
						double aix_feedback;
						double aix_threshold = 0.01; // threshold
						double ao_step = 0.005; // ao change on each step
						double aix_diff = 99999.0; // initialize to a large number
						double aix_orig = line_slope * mx + line_offset; // ai_x from image scanning: (float)pAIx_orig(my*sx + mx)
						//printf("aix in original AIx image: %lf \n", aix_orig);

						size_t start_at = clock();
						try
						{
							// ablation with feedback
							printf("Approaching the stimulation point ...");
							do 
							{
								DAQ_Stimulating(pAO, dev_ao, ao_x, ao_y, ao_z, dev_clk, clk_n, scanning_rate, duty_cycle, NPULSE);
								DAQ_get_current_ai(scanning_rate, &aix_feedback, 0);

								aix_diff = aix_feedback-aix_orig;
							
								if(aix_diff>0) // aix_feedback>aix_orig
								{
									pAO[0] = pAO[0] - ao_step;
								}
								else           // aix_feedback<aix_orig
								{
									pAO[0] = pAO[0] + ao_step;
								}
								printf("..");
								//printf("current aix during stimulation: %lf \n", aix_feedback);
							}while( abs(aix_diff) > aix_threshold );

							printf("\n Current aix during stimulation: %lf \n", aix_feedback);

							//double aiy_feedback;
							//DAQ_get_current_ai(scanning_rate, &aiy_feedback, 1);
							//printf("current aiy during stimulation: %lf \n", aiy_feedback);

							// shutter on
							DAQ_Shutter_Stimulation(true);
							// hold on
							wait_milliseconds(stimulate_waiting);
							// shutter off
							DAQ_Shutter_Stimulation(false);
						}
						catch (...)
						{
							printf("Function of DAQ_Stimulating fail.\n");
							return;
						}

						// time display
						size_t end_at = clock();
						head_time=end_at - start_at;
						V3DLONG index_point = bx*by*(k-bb_start_z) + bx*(j-bb_start_y)+ i-bb_start_x;
						//estimate time left
						lefttime=(float)((pagesz-i_marker) * region_size - index_point - 1)*head_time/CLOCKS_PER_SEC;
						alltime = (float)pagesz * region_size * head_time/CLOCKS_PER_SEC;
						usedtime = usedtime + (float)head_time/CLOCKS_PER_SEC;

						qDebug()<<"Already Spent Time:"<<usedtime<< "s";
						qDebug()<<"Estimated Remaining Time:"<<lefttime<<"/"<<alltime << "s";
						dialog.setText( QString("Already spent time: %1 s, Remaining Time: %2 s.\n Current stimulation: %3/%4").arg(usedtime).arg(lefttime).arg(i_marker+1).arg(pagesz) );

						//now process events
						QCoreApplication::processEvents();
					}
				}
			}

			size_t end_DAQ=clock();
			qDebug()<<"time elapse for stimulating one marker ..."<< (float)(end_DAQ-start_DAQ)/CLOCKS_PER_SEC;
		}
	}
	catch (...)
	{
		printf("Fail to allocate memory for AI and AO.\n");
		return;
	}

	//de-alloc
	if(pAO) {delete []pAO; pAO=0;}
	if(pSortAO) {delete []pSortAO; pSortAO=0;}
	if(flag_markers) {delete []flag_markers; flag_markers=0;}

	//reset
	DAQ_Reset(dev_x, dev_y, dev_z, ao_x, ao_y, ao_z);

	if(pAIx_orig) {delete []pAIx_orig; pAIx_orig=0;}
	if(in_sz)  {delete []in_sz; in_sz=0;}

	int end_t = clock();
	printf("time elapse ... %d \n", end_t-start_t);

}



// imaging on landermarkers
void LaserScanImaging::do_image_landmarkers(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks)
{
	qDebug()<<"............. Now running LSM imaging for Landmarkers ...............";

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	logname.chop(4);
	logname.append(".txt");

	Parameters_LSM pl;
	pl.load_ParameterFile(logname);
	cout << "logfile name:"<<logname.toStdString()<<endl;

	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;
	min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP;
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP; //(max_fovz-min_fovz)/sz;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;
	sc = (ch<2)?1:2;

	// avoid mismatch image with configuration file
	pImgIn=((V3DPluginCallback *)cb)->getImage(curwin);
	if(!pImgIn) return;

	V3DLONG o_sx = pImgIn->getXDim();
	V3DLONG o_sy = pImgIn->getYDim();
	V3DLONG o_sz = pImgIn->getZDim();
	V3DLONG o_sc = pImgIn->getCDim();

	if(!(sx==o_sx && sy==o_sy && sz==o_sz ))
	{
		qDebug()<<"Image and configuration are not match!";
		return;
	}

	double o_origin_x, o_origin_y, o_origin_z;
	double o_rez_x, o_rez_y, o_rez_z;
	// begin imaging code
	// make sure to get correct value. These values can always get from log file
	o_origin_x = pl.FOVX_MIN;
	o_origin_y = pl.FOVY_MIN;
	o_origin_z = pl.FOVZ_MIN;

	o_rez_x = pl.FOVX_STEP;
	o_rez_y = pl.FOVY_STEP;
	o_rez_z = pl.FOVZ_STEP;

	// set ROI imaging resolution
	double vz_x, vz_y, vz_z;
	vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z/2.0;

	//===========================================================
	// for final image saving initialization
	V3DLONG f_sx = o_sx*o_rez_x / vz_x;
	V3DLONG f_sy = o_sy*o_rez_y / vz_y;
	V3DLONG f_sz = o_sz*o_rez_z / vz_z;
	V3DLONG f_pagesz = f_sx*f_sy*f_sz;

	try
	{
		if(ch==0 || ch==1)
		{
			pImFloat_f = new float [f_pagesz];
		}
		else if(ch==2)
		{
			pImFloat_f = new float [2*f_pagesz];
			sc=2;
		}
		else
			return;
	}
	catch (...)
	{
		printf("Fail to allocate memory for final landmarker imaging saving.\n");
		return;
	}

	// initialize all to 0.0
	if(ch==0 || ch==1)
	{
		for(V3DLONG i=0; i<f_pagesz; i++)
			pImFloat_f[i]=0.0;
	}
	else if(ch==2)
	{
		for(V3DLONG i=0; i<2*f_pagesz; i++)
			pImFloat_f[i]=0.0;
	}

	//
	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;

	// open laser
	if(b_useSimImg == 0) shutter_open();

	// the number of markers
	V3DLONG pagesz = list_landmarks.size();

	// imaging for each marker
	for(V3DLONG i_marker=0; i_marker<pagesz; i_marker++)
	{
		LocationSimple marker=list_landmarks.at(i_marker);
		float radius = marker.radius;

		// create a ROI with marker's position and radius
		// ROI is a cube with the size of 2*radius
		bb_start_x = marker.x-radius; bb_end_x = marker.x+radius;
		bb_start_y = marker.y-radius; bb_end_y = marker.y+radius;
		bb_start_z = marker.z-radius; bb_end_z = marker.z+radius;

		// make ROI inside the volume
		bb_start_x = qBound(V3DLONG(0), V3DLONG(bb_start_x), o_sx-1);
		bb_start_y = qBound(V3DLONG(0), V3DLONG(bb_start_y), o_sy-1);
		bb_start_z = qBound(V3DLONG(0), V3DLONG(bb_start_z), o_sz-1);

		bb_end_x = qBound(V3DLONG(0), V3DLONG(bb_end_x), o_sx-1);
		bb_end_y = qBound(V3DLONG(0), V3DLONG(bb_end_y), o_sy-1);
		bb_end_z = qBound(V3DLONG(0), V3DLONG(bb_end_z), o_sz-1);

		// print info message on roi bounding box, voxel size: vz_x vz_y vz_z
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		// imaging
		// ------------------------------------------------------
		double n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
		double n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
		double n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

		V3DLONG n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
		V3DLONG n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
		V3DLONG n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;

		double n_min_fovx = n_origin_x, n_min_fovy = n_origin_y, n_min_fovz = n_origin_z;
		double n_max_fovx = n_origin_x + ((double)n_sx)*vz_x, n_max_fovy = n_origin_y + ((double)n_sy)*vz_y, n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;

		if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096)
		{
			printf("The target image size is too large.\n");
			return;
		}

		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------old info---------------";
		qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
		qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
		qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
		qDebug()<<"-----------new info---------------";
		qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
		qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		// symbol link
		sx = n_sx; sy = n_sy; sz = n_sz;
		min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
		max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
		fov_step_x = (max_fovx-min_fovx)/sx;
		fov_step_y = (max_fovy-min_fovy)/sy;
		fov_step_z = (max_fovz-min_fovz)/sz;

		set_voltages();

		if(!set_scanning_buffers()) return;

		do_LSM_single();

		// save pImFloat to pImFloat_f
		V3DLONG bb_s_x, bb_e_x, bb_s_y, bb_e_y, bb_s_z, bb_e_z;//pos of bb in final full image
		bb_s_x = bb_start_x * o_rez_x/vz_x;   bb_e_x = bb_end_x * o_rez_x/vz_x;
		bb_s_y = bb_start_y * o_rez_y/vz_y;   bb_e_y = bb_end_y * o_rez_y/vz_y;
		bb_s_z = bb_start_z * o_rez_z/vz_z;   bb_e_z = bb_end_z * o_rez_z/vz_z;

		for(V3DLONG k=bb_s_z;k<bb_e_z;k++)
		{
			for(V3DLONG j=bb_s_y;j<bb_e_y;j++)
			{
				for(V3DLONG i=bb_s_x;i<bb_e_x;i++)
				{
					V3DLONG f_idx = k*f_sy*f_sx + j*f_sx + i;
					V3DLONG b_idx = (k-bb_s_z)*n_sy*n_sx + (j-bb_s_y)*n_sx + (i-bb_s_x);
					pImFloat_f[f_idx] = pImFloat[b_idx];
				}
			}
		} // end saving to full image

		// display each marker imaging as a small image
		V3DPluginCallback* cbb=(V3DPluginCallback*)cb;
		QString filename_img_saved;
		if (!passImagingData2V3D((*cbb),
			pImFloat,   //this function will do parameter-check
			"LSM",      //LSM image
			false,      //do NOT convert to 8bit directly
			sx, sy, sz, sc,
			fov_step_x, fov_step_y, fov_step_z,
			min_fovx,  min_fovy,  min_fovz,
			max_fovx,  max_fovy,  max_fovz,
			ratio_x, ratio_y, ratio_z,
			time_out, scanning_rate, ch,
			scanMethod, b_useSimImg, b_scan_z_reverse,
			filename_img_saved,false))
		{
			v3d_msg("Fail to pass the LSM data back to V3D.",0);
			return;
		}
	} // end for i_marker

	// close laser
	if(b_useSimImg == 0) shutter_close_daq_reset();

	// display full image for all marker imaging
	V3DPluginCallback* cbb=(V3DPluginCallback*)cb;
	QString filename_img_saved;
	if (!passImagingData2V3D((*cbb),
		pImFloat_f, //this function will do parameter-check
		"LSM",      //LSM image
		false,      //do NOT convert to 8bit directly
		f_sx, f_sy, f_sz, sc,
		fov_step_x, fov_step_y, fov_step_z,
		min_fovx,  min_fovy,  min_fovz,
		max_fovx,  max_fovy,  max_fovz,
		ratio_x, ratio_y, ratio_z,
		time_out, scanning_rate, ch,
		scanMethod, b_useSimImg, b_scan_z_reverse,
		filename_img_saved,false)) // fov values should be different from markers
	{
		v3d_msg("Fail to pass the LSM data back to V3D.",0);
		return;
	}

}




//=============================================================================
// Function: Using Least square method to get a line function
// y= a*x + b: a=xyTopY, b=xyTopX
// xyTopX -- offset (Jie Ju)
// xyTopY -- slope (Xie Lv)
// nCount -- point number
//=============================================================================
void LaserScanImaging::getLineEqu(double &offset, double &slope, float  *X, float *Y, V3DLONG &nCount)
{
    int      i;
    double   SumX, SumY, SumXY, SumX2;

    SumX = 0;
    SumX2 = 0;
    for( i=0; i<nCount; i++)
    {
        SumX += X[i];
        SumX2 += (X[i]*X[i]);
    }

    SumY = 0;
    for( i=0; i<nCount; i++)
    {
        SumY += Y[i];
    }
    SumXY = 0;
    for( i=0; i<nCount; i++)
    {
        SumXY += ( X[i]* Y[i]);
    }
    offset = ( (SumX2*SumY - SumX*SumXY) / (nCount*SumX2 - SumX * SumX));
    slope = ( (nCount*SumXY - SumX*SumY) / (nCount*SumX2 - SumX * SumX));
}



// read AIx from file for calibration
void LaserScanImaging::readAIAOFile(QString& filename, float *pAIx, float*pAOx, V3DLONG &size)
{
	if(!QFile(filename).exists())
	{
		filename = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
												  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(filename.isEmpty())
		{
			v3d_msg(QString("File [%1] is empty!").arg(filename));
			return;
		}
	}

	QFile file(filename);
	if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		v3d_msg(QString("Open file [%1] failed!").arg(filename));
		return;
	}

	QTextStream in(&file);

	V3DLONG u_size;

	for(int j=0; j<2; j++)
    {
		QString line = in.readLine();
		QStringList qsl = line.split(" ",QString::SkipEmptyParts);

		V3DLONG lsize = qsl.size();
		//if(lsize<=0) continue;

		if(lsize < size)
			u_size=lsize;
		else
			u_size=size;

        for (V3DLONG i=0; i<u_size; i++)
        {
			if(j==0) // first line is pAIx
				pAIx[i] = qsl[i].toFloat();
			if(j==1) // second line is pAOx
				pAOx[i] = qsl[i].toFloat();
        }
	}

	file.close();

	size = u_size;
}



// adaptive ROI
bool LaserScanImaging::do_image_adaptiveROI()
{
	qDebug()<<"............. Now running LSM imaging for adaptive ROI ..............."; //return;

	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;
	double vz_x, vz_y, vz_z;

	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=((V3DPluginCallback *)cb)->currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return false;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	Parameters_LSM pl;
	bool b_roi_FC = false;
	bool b_roi_LSMFC =false;

	// decide whether this is selected from Focus_Control_BF/LSM window
	if((logname.compare("Focus_Control_BF")==0) || (logname.contains("BF_", Qt::CaseSensitive)) || (logname.compare("Focus_Control_LSM")==0))
	{
		// get Parameters_LSM from LSM configure file
		QString lsm_FileName = ssConfigFolder + "mi_configuration.txt";
		if(!QFile(lsm_FileName).exists())
		{
			lsm_FileName = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
													  "/Documents", QObject::tr("Image Configuration (*.log)"));
			if(lsm_FileName.isEmpty())
			{
				v3d_msg("No device configuration file exist. Do nothing!");
				return false;
			}
		}
		// load lsm paras
		pl.load_ParameterFile(lsm_FileName);
		if(logname.compare("Focus_Control_LSM")==0)
			b_roi_LSMFC = true;

		b_roi_FC = true;
	}else
	{
		// load from current full image log file
		logname.chop(4);
		logname.append(".txt");

		pl.load_ParameterFile(logname);
		cout << "logfile name:"<<logname.toStdString()<<endl;

		b_roi_FC = false;
	}

	// these paras are from log file for current full image
	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;
	scanning_rate = pl.SCANNING_RATE;

	fov_step_x = pl.FOVX_STEP; fov_step_y = pl.FOVY_STEP; fov_step_z = pl.FOVZ_STEP;
	min_fovz = pl.FOVZ_MIN; max_fovz = pl.FOVZ_MAX;

	// always get BF_TO_LSM_A... from "mi_configuration.txt"
	// because this is the latest infor.
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return false;
	}

	a_bflsm = mypara.BF_TO_LSM_A; d_bflsm = mypara.BF_TO_LSM_D;
	e_bflsm = mypara.BF_TO_LSM_E; f_bflsm = mypara.BF_TO_LSM_F;

	//-------------preparing parameters for ROI parameter dialog----
	// get image in current window
	pImgIn=((V3DPluginCallback *)cb)->getImage(curwin);
	if(!pImgIn) return false;

	double o_origin_x, o_origin_y, o_origin_z;
	double o_rez_x, o_rez_y, o_rez_z;
	V3DLONG o_sx, o_sy, o_sz, o_sc;
	V3DLONG n_sx, n_sy, n_sz;
	double n_origin_x, n_origin_y, n_origin_z;
	double n_min_fovx, n_min_fovy, n_min_fovz;
	double n_max_fovx, n_max_fovy, n_max_fovz;

	o_sx = pImgIn->getXDim();
	o_sy = pImgIn->getYDim();
	o_sz = pImgIn->getZDim();
	o_sc = pImgIn->getCDim();

	//-----------------------------------------------
	// Get ROI bounding box using region growing
	RegionGrowing* pRgnGrow = new RegionGrowing();
	if(!pRgnGrow)
	{
		v3d_msg("Errors in getting region growing result.\n", 0);
		return false;
	}

	bool bRGrow = pRgnGrow->doRegionGrow((V3DPluginCallback *)cb, 0);
	if(!bRGrow)
		return false;

	// feedback boundingbox
	bb_start_x=pRgnGrow->bbox_minx; bb_start_y=pRgnGrow->bbox_miny;
	bb_end_x=pRgnGrow->bbox_maxx;   bb_end_y=pRgnGrow->bbox_maxy;
	//-----------------------------------------------

	// check
	if(((bb_start_x==bb_end_x) && (bb_start_y==bb_end_y))||
		((bb_start_x==o_sx)&&(bb_start_y==o_sy)&&(bb_end_x==0)&&(bb_end_y==0)))
	{
		v3d_msg("No ROI detected. Do nothing!");
		return false;
	}

	// add edge pixels around the adaptive ROI
	int edge_n = 20;
	bb_start_x = bb_start_x-edge_n; bb_start_y = bb_start_y-edge_n;
	bb_end_x = bb_end_x+edge_n; bb_end_y = bb_end_y+edge_n;

	bb_start_x = qBound(V3DLONG(0), bb_start_x, o_sx-1);
	bb_start_y = qBound(V3DLONG(0), bb_start_y, o_sy-1);
	bb_end_x = qBound(V3DLONG(0), bb_end_x, o_sx-1);
	bb_end_y = qBound(V3DLONG(0), bb_end_y, o_sy-1);

	// z is only set as 0
	bb_start_z = bb_end_z = 0;

	if(!b_roi_FC)
	{
		// make sure to get correct value. These values can always get from log file
		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_origin_z = pl.FOVZ_MIN;

		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
		o_rez_z = pl.FOVZ_STEP;

		vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

		// print info message
		// roi bounding box, voxel size: vz_x vz_y vz_z
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
		n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
		n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

		n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
		n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
		n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;
		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------old info---------------";
		qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
		qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
		qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
		qDebug()<<"-----------new info---------------";
		qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
		qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
		n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
		n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
		n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
	}
	else // ROI from focus control window
	{
		if(b_roi_LSMFC) //ROI from LSM focus control window
		{
			o_origin_x = pl.FOVX_MIN;
			o_origin_y = pl.FOVY_MIN;
			o_origin_z = pl.FOVZ_MIN;

			o_rez_x = pl.FOVX_STEP;
			o_rez_y = pl.FOVY_STEP;
			o_rez_z = pl.FOVZ_STEP;

			vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

			// roi bounding box, voxel size: vz_x vz_y vz_z
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------passing para---------------";
			qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
			n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
			n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

			n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
			n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
			n_sz = 1; // only one slice
			// print info message
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------old info---------------";
			qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
			qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
			qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
			qDebug()<<"-----------new info---------------";
			qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
			qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
			n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
			n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
			n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		}
		else //ROI from BF focus control window
		{
			v3d_msg("Please make sure that BF/LSM calibration has been done before this operation!\n",0);
			// transform BF coords to LSM coords: the LSM coords are physical pos
			n_min_fovx = a_bflsm * bb_start_x + e_bflsm;
			n_min_fovy = d_bflsm * bb_start_y + f_bflsm;

			n_max_fovx = a_bflsm * bb_end_x + e_bflsm;
			n_max_fovy = d_bflsm * bb_end_y + f_bflsm;

			n_min_fovz = 30.0; n_max_fovz = 35.0;  // default

			n_sx = (V3DLONG)((n_max_fovx-n_min_fovx)/fov_step_x);
			n_sy = (V3DLONG)((n_max_fovy-n_min_fovy)/fov_step_y);
			n_sz = 1;

			vz_x=fov_step_x; vz_y=fov_step_y; vz_z=fov_step_z;
		}
	}

	if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096) //
	{
		printf("The target image size is too large.\n");
		return false;
	}

	// symbol link for scanning.
	sx = n_sx; sy = n_sy; sz = n_sz;
	min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
	max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
	fov_step_x = vz_x; //(max_fovx-min_fovx)/sx;
	fov_step_y = vz_y; //(max_fovy-min_fovy)/sy;
	fov_step_z = (max_fovz-min_fovz)/sz;

	// these are put back to lsm configure file and used as initial paras in ROI parameter dialog
	pl.FOVX_MIN = min_fovx;	pl.FOVX_MAX = max_fovx;	pl.SX = sx;	pl.FOVX_STEP = fov_step_x;
	pl.FOVY_MIN = min_fovy;	pl.FOVY_MAX = max_fovy;	pl.SY = sy;	pl.FOVY_STEP = fov_step_y;
	pl.FOVZ_MIN = min_fovz;	pl.FOVZ_MAX = max_fovz;	pl.SZ = sz;	pl.FOVZ_STEP = fov_step_z;

	// user can now define parameters by ROI_Setup_Dialog
	LSM_ROI_Setup_Dialog roi_dialog(&pl, false);
	if (roi_dialog.exec()!=QDialog::Accepted)
		return false;

	roi_dialog.copyDataOut(&pl);

	// get following paras from ROI para dialog
	min_fovx = pl.FOVX_MIN;	max_fovx = pl.FOVX_MAX;	sx = pl.SX;	fov_step_x = pl.FOVX_STEP;
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	sc = (ch<2)?1:2;

	set_voltages();

	if(!set_scanning_buffers())
		return false;

	// open laser
	if(b_useSimImg == 0) shutter_open();

	if(!do_LSM_single())
		return false;

	if(b_useSimImg == 0) shutter_close_daq_reset();

	return true;

}


// confocal imaging with interactive selecting ROI
bool LaserScanImaging::do_image_withROI_i_x()
{
	qDebug()<<"............. Now running LSM imaging for ROI with full X scan ..............."; //return;

	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;
	double vz_x, vz_y, vz_z;

	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=((V3DPluginCallback *)cb)->currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return false;
	}

	//finding the bounding box of ROI
	ROIList pRoiList=((V3DPluginCallback *)cb)->getROI(curwin);
	if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
	{
		v3d_msg("No ROI specified ... Do nothing.");
		return false;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	Parameters_LSM pl;
	bool b_roi_FC = false;
	bool b_roi_LSMFC =false;

	// decide whether this is selected from Focus_Control_BF/LSM window
	if((logname.compare("Focus_Control_BF")==0) || (logname.contains("BF_", Qt::CaseSensitive)) || (logname.compare("Focus_Control_LSM")==0))
	{
		// get Parameters_LSM from LSM configure file
		QString lsm_FileName = ssConfigFolder + "mi_configuration.txt";
		if(!QFile(lsm_FileName).exists())
		{
			lsm_FileName = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
													  "/Documents", QObject::tr("Image Configuration (*.log)"));
			if(lsm_FileName.isEmpty())
			{
				v3d_msg("No device configuration file exist. Do nothing!");
				return false;
			}
		}
		// load lsm paras
		pl.load_ParameterFile(lsm_FileName);
		if(logname.compare("Focus_Control_LSM")==0)
			b_roi_LSMFC = true;

		b_roi_FC = true;
	}else
	{
		// load from current full image log file
		logname.chop(4);
		logname.append(".txt");

		pl.load_ParameterFile(logname);
		cout << "logfile name:"<<logname.toStdString()<<endl;

		b_roi_FC = false;
	}

	// these paras are from log file for current full image
	dev_x = pl.DEVICE_X; dev_y = pl.DEVICE_Y; dev_z = pl.DEVICE_Z;
	ao_x = pl.AO_X;	ao_y = pl.AO_Y;	ao_z = pl.AO_Z;	aictr_red = pl.AI_COUNTER_RED; aictr_green = pl.AI_COUNTER_GREEN;
	dev_red_ai = pl.DEVICE_RED_AI; dev_red_ctr = pl.DEVICE_RED_CTR; red_n_ai = pl.PORT_RED_AI; red_n_ctr = pl.PORT_RED_CTR;
	dev_green_ai = pl.DEVICE_GREEN_AI; dev_green_ctr = pl.DEVICE_GREEN_CTR; green_n_ai = pl.PORT_GREEN_AI; green_n_ctr = pl.PORT_GREEN_CTR;
	dev_timing = pl.DEVICE_TIMING; timing_n = pl.PORT_TIMING; dev_clk = pl.DEVICE_CLOCK; clk_n = pl.PORT_CLOCK;

	fov_step_x = pl.FOVX_STEP; fov_step_y = pl.FOVY_STEP; fov_step_z = pl.FOVZ_STEP;
	min_fovz = pl.FOVZ_MIN; max_fovz = pl.FOVZ_MAX;

	// always get BF_TO_LSM_A... from "mi_configuration.txt"
	// because this is the latest infor.
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return false;
	}

	a_bflsm = mypara.BF_TO_LSM_A; d_bflsm = mypara.BF_TO_LSM_D;
	e_bflsm = mypara.BF_TO_LSM_E; f_bflsm = mypara.BF_TO_LSM_F;

	//===========preparing parameters for ROI parameter dialog=====
	// get image in current window
	pImgIn=((V3DPluginCallback *)cb)->getImage(curwin);
	if(!pImgIn) return false;

	double o_origin_x, o_origin_y, o_origin_z;
	double o_rez_x, o_rez_y, o_rez_z;
	V3DLONG o_sx, o_sy, o_sz, o_sc;
	V3DLONG n_sx, n_sy, n_sz;
	double n_origin_x, n_origin_y, n_origin_z;
	double n_min_fovx, n_min_fovy, n_min_fovz;
	double n_max_fovx, n_max_fovy, n_max_fovz;

	o_sx = pImgIn->getXDim();
	o_sy = pImgIn->getYDim();
	o_sz = pImgIn->getZDim();
	o_sc = pImgIn->getCDim();

	// ROI list
	QRect b_xy = pRoiList.at(0).boundingRect();
	QRect b_yz = pRoiList.at(1).boundingRect();
	QRect b_zx = pRoiList.at(2).boundingRect();

	//finding the bounding box of ROI
	bool vxy=true,vyz=true,vzx=true; // 3 2d-views
	if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
		vxy=false;
	if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
		vyz=false;
	if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
		vzx=false;

	bb_start_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), o_sx-1);
	bb_start_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), o_sy-1);
	bb_start_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), o_sz-1);

	bb_end_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.right(), b_zx.right())), o_sx-1); //use qMax but not qMin
	bb_end_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.bottom(), b_yz.bottom())), o_sy-1);
	bb_end_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.right(), b_zx.bottom())), o_sz-1);

	if( vxy && !vyz && !vzx )  // only has rect on xy view, roi_BF only has xy view
	{
		bb_start_z = 0;
		if(b_roi_FC)
			bb_end_z = 0;
		else
			bb_end_z = o_sz-1;
	}
	else if( !vxy && vyz && !vzx )// only has rect on yz view
	{
		bb_start_x = 0;
		bb_end_x = o_sx-1;
	}
	else if( !vxy && !vyz && vzx )// only has rect on zx view
	{
		bb_start_y = 0;
		bb_end_y = o_sy-1;
	}

	qDebug()<<"bb_start... (x, y, z)"<< bb_start_x << bb_start_y << bb_start_z;
	qDebug()<<"bb_end... (x, y, z)"<< bb_end_x << bb_end_y << bb_end_z;


	if (bb_start_x>bb_end_x || bb_start_y>bb_end_y || bb_start_z>bb_end_z)
	{
		v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
		return false;
	}

	if(!b_roi_FC)
	{
		// make sure to get correct value. These values can always get from log file
		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_origin_z = pl.FOVZ_MIN;

		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
		o_rez_z = pl.FOVZ_STEP;

		vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

		// print info message
		// roi bounding box, voxel size: vz_x vz_y vz_z
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		//v3d_msg(QString("%1 %2 %3 %4 %5 %6").arg(o_origin_x).arg(o_origin_y).arg(bb_start_x).arg(bb_start_y).arg(bb_end_x).arg(bb_end_y));

		n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
          n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
		n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

		n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
		n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
		n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;
		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------old info---------------";
		qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
		qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
		qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
		qDebug()<<"-----------new info---------------";
		qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
		qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
		n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
		n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
		n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		//the above code has a lot of redundancy! can be simplified
	}
	else // ROI from focus control window
	{
		if(b_roi_LSMFC) //ROI from LSM focus control window
		{
			o_origin_x = pl.FOVX_MIN;
			o_origin_y = pl.FOVY_MIN;
			o_origin_z = pl.FOVZ_MIN;

			o_rez_x = pl.FOVX_STEP;
			o_rez_y = pl.FOVY_STEP;
			o_rez_z = pl.FOVZ_STEP;

			vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

			// roi bounding box, voxel size: vz_x vz_y vz_z
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------passing para---------------";
			qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
			n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
			n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

			n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
			n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
			n_sz = 1; // only one slice
			// print info message
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------old info---------------";
			qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
			qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
			qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
			qDebug()<<"-----------new info---------------";
			qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
			qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
			n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
			n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
			n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		}
		else //ROI from BF focus control window
		{
			v3d_msg("Please make sure that BF/LSM calibration has been done before this operation!\n",0);
			// transform BF coords to LSM coords: the LSM coords are physical pos
			n_min_fovx = a_bflsm * bb_start_x + e_bflsm;
			n_min_fovy = d_bflsm * bb_start_y + f_bflsm;

			n_max_fovx = a_bflsm * bb_end_x + e_bflsm;
			n_max_fovy = d_bflsm * bb_end_y + f_bflsm;

			n_min_fovz = 30.0; n_max_fovz = 35.0;  // default

			n_sx = (V3DLONG)((n_max_fovx-n_min_fovx)/fov_step_x);
			n_sy = (V3DLONG)((n_max_fovy-n_min_fovy)/fov_step_y);
			n_sz = 1;

			vz_x=fov_step_x; vz_y=fov_step_y; vz_z=fov_step_z;
		}
	}

	if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096) //
	{
		printf("The target image size is too large.\n");
		return false;
	}

	// symbol link for scanning.
	sx = n_sx; sy = n_sy; sz = n_sz;
	min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
	max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
	fov_step_x = vz_x; //(max_fovx-min_fovx)/sx;
	fov_step_y = vz_y; //(max_fovy-min_fovy)/sy;
	fov_step_z = (max_fovz-min_fovz)/sz;

	// these are put back to lsm configure file and used as initial paras in ROI parameter dialog
	pl.FOVX_MIN = min_fovx;	pl.FOVX_MAX = max_fovx;	pl.SX = sx;	pl.FOVX_STEP = fov_step_x;
	pl.FOVY_MIN = min_fovy;	pl.FOVY_MAX = max_fovy;	pl.SY = sy;	pl.FOVY_STEP = fov_step_y;
	pl.FOVZ_MIN = min_fovz;	pl.FOVZ_MAX = max_fovz;	pl.SZ = sz;	pl.FOVZ_STEP = fov_step_z;

	// user can now define parameters by ROI_Setup_Dialog
	LSM_ROI_Setup_Dialog roi_dialog(&pl, false);
	if (roi_dialog.exec()!=QDialog::Accepted)
		return false;

	roi_dialog.copyDataOut(&pl);

	// use full x to scan firstly
	fov_step_x = pl.FOVX_STEP;
	min_fovx = o_origin_x;	max_fovx = o_origin_x + o_sx*o_rez_x;	sx = (max_fovx-min_fovx)/fov_step_x;

	// save real x for recovering image after scanning
	real_bb_start_x = (V3DLONG)((pl.FOVX_MIN-o_origin_x)/fov_step_x + 0.5);
	real_bb_end_x   = (V3DLONG)((pl.FOVX_MAX-o_origin_x)/fov_step_x + 0.5);
	real_sx = pl.SX;

	// get following paras from ROI para dialog
	min_fovy = pl.FOVY_MIN;	max_fovy = pl.FOVY_MAX;	sy = pl.SY;	fov_step_y = pl.FOVY_STEP;
	min_fovz = pl.FOVZ_MIN;	max_fovz = pl.FOVZ_MAX;	sz = pl.SZ;	fov_step_z = pl.FOVZ_STEP;
	ratio_x = pl.RATIO_X; ratio_y = pl.RATIO_Y;	ratio_z = pl.RATIO_Z; time_out = pl.TIMEOUT;
	scanning_rate = pl.SCANNING_RATE; ch = pl.CHANNEL; marker_sorted = pl.MARKER_SORTED;
	scanMethod = pl.SCAN_METHOD; b_useSimImg = pl.USE_SIMIMG; duty_cycle = pl.DUTY_CYCLE;
	b_scan_z_reverse=pl.SCAN_Z_REVERSE;

	qDebug()<<"-----------full x scanning new info---------------";
	qDebug()<<"dims ..."<< sx << sy << sz;
	qDebug()<<"origin ..."<< min_fovx << min_fovy << min_fovz;
	qDebug()<<"real_bb_start_x, real_bb_end_x.."<< real_bb_start_x << real_bb_end_x;

	sc = (ch<2)?1:2;

	//----------------------------------------->>>
	// set paras for getting correction offset
	if(USE_PRIOR_CORRECTION == 1)
	{
	   make_a_copy_PHC(true);

	   old_fov_step_x = o_rez_x;
       old_fov_step_y = o_rez_y;

	   set_real_scanning_range_with_real_offset();
	}
	//-----------------------------------------<<<

	set_voltages();
	if(!set_scanning_buffers())
		return false;
	// min_fovx change depending on scanning
	// open laser
	if(b_useSimImg == 0) shutter_open();
	if(!do_LSM_single())
		return false;

	//------------------------------------------->>>
	// copy back to original paras set from dialog
	if(USE_PRIOR_CORRECTION == 1)
	{
		make_a_copy_PHC(false);

		fov_step_x = pl.FOVX_STEP;
		fov_step_y = pl.FOVY_STEP;
	}
	//------------------------------------------<<<

	if(b_useSimImg == 0) shutter_close_daq_reset();

	return true;
}



LandmarkList LaserScanImaging::get_ablation_marker_parameters(bool b_from3DViewer)
{
	Image4DSimple *pImgIn;
	v3dhandle curwin;

	LandmarkList list_landmarks;

	if(b_from3DViewer)
		curwin = ((V3DPluginCallback *)cb)->curHiddenSelectedWindow(); //by PHC, 101009. currentImageWindow();
	else
		curwin=((V3DPluginCallback *)cb)->currentImageWindow();

	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return list_landmarks;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	// load from current full image log file
	logname.chop(4);
	logname.append(".txt");

	// setup scanning parameters
	setup_parameters(logname);

	if(b_from3DViewer)
	{
		pImgIn = ((V3DPluginCallback *)cb)->getImage(curwin);
		if (! pImgIn)
		{
			v3d_msg("The pointer to the current image window is invalid. Do nothing.");
			return list_landmarks;
		}
		v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
		if(!p)
		{
			v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.");
			return list_landmarks;
		}
		if (!(p->imgp))
		{
			v3d_msg("The image pointer inside the parameter-struct is invalid. Do nothing.");
			return list_landmarks;
		}

		if(p->list_landmarks.size() < 1)
		{
			v3d_msg("Need at least one marker. Do nothing.");
			return list_landmarks;
		}

		list_landmarks = p->list_landmarks;
	}
	else
	{
		list_landmarks=((V3DPluginCallback *)cb)->getLandmark(curwin);
	}

	return list_landmarks;
}



// add marker imaging parameter?
QString LaserScanImaging::get_roi_scanning_parameters(bool b_from3DViewer, bool b_landmarkerImging, int markerRadius)
{
	QString logname_roi;

	qDebug()<<"............. Now get ROI parameters ..............."; //return;

	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;
	double vz_x, vz_y, vz_z;

	Image4DSimple *pImgIn;
	v3dhandle curwin;

	if(b_from3DViewer)
		curwin = ((V3DPluginCallback *)cb)->curHiddenSelectedWindow(); //by PHC, 101009. currentImageWindow();
	else
		curwin=((V3DPluginCallback *)cb)->currentImageWindow();


	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return logname_roi;
	}

	// Acquire parameters from the log file of the current image window
	QString logname = ((V3DPluginCallback *)cb)->getImageName(curwin);
	Parameters_LSM pl;
	bool b_roi_FC = false;
	bool b_roi_LSMFC =false;

	// decide whether this is selected from Focus_Control_BF/LSM window
	if((logname.compare("Focus_Control_BF")==0) || (logname.contains("BF_", Qt::CaseSensitive)) || (logname.compare("Focus_Control_LSM")==0))
	{
		// get Parameters_LSM from LSM configure file
		QString lsm_FileName = ssConfigFolder + "mi_configuration.txt";
		if(!QFile(lsm_FileName).exists())
		{
			lsm_FileName = QFileDialog::getOpenFileName(NULL, QObject::tr("Find Configuration file"),
													  "/Documents", QObject::tr("Image Configuration (*.log)"));
			if(lsm_FileName.isEmpty())
			{
				v3d_msg("No device configuration file exist. Do nothing!");
				return logname_roi;
			}
		}
		// load lsm paras
		pl.load_ParameterFile(lsm_FileName);
		if(logname.compare("Focus_Control_LSM")==0)
			b_roi_LSMFC = true;

		b_roi_FC = true;
	}
	else
	{
		// load from current full image log file
		logname.chop(4);
		logname.append(".txt");

		pl.load_ParameterFile(logname);
		cout << "logfile name:"<<logname.toStdString()<<endl;

		b_roi_FC = false;
	}

	// always get BF_TO_LSM_A... from "mi_configuration.txt"
	// because this is the latest infor.
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return logname_roi;
	}

	a_bflsm = mypara.BF_TO_LSM_A; d_bflsm = mypara.BF_TO_LSM_D;
	e_bflsm = mypara.BF_TO_LSM_E; f_bflsm = mypara.BF_TO_LSM_F;

	//===========preparing parameters for ROI parameter dialog=====
	// get image in current window
	pImgIn=((V3DPluginCallback *)cb)->getImage(curwin);
	if(!pImgIn)
		return logname_roi;

	double o_origin_x, o_origin_y, o_origin_z;
	double o_rez_x, o_rez_y, o_rez_z;
	V3DLONG o_sx, o_sy, o_sz, o_sc;
	V3DLONG n_sx, n_sy, n_sz;
	double n_origin_x, n_origin_y, n_origin_z;
	double n_min_fovx, n_min_fovy, n_min_fovz;
	double n_max_fovx, n_max_fovy, n_max_fovz;

	o_sx = pImgIn->getXDim();
	o_sy = pImgIn->getYDim();
	o_sz = pImgIn->getZDim();
	o_sc = pImgIn->getCDim();

	// parameters passed from 3d view
	if(b_from3DViewer)
	{
		v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
		if(!p) //a bug fixed by PHC, 101008
		{
			v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.");
			return logname_roi;
		}
		if (!(p->imgp))
		{
			v3d_msg("The image pointer inside the parameter-struct is invalid. Do nothing.");
			return logname_roi;
		}

		// using landmarker to get roi
		if(b_landmarkerImging)
		{
			if(p->list_landmarks.size() < 1)
			{
				v3d_msg("Need at least one marker. Do nothing.");
				return logname_roi;
			}
			LocationSimple marker=p->list_landmarks.at(0); // always use index 0 for one marker imaging
			float radius = markerRadius;

			// create a ROI with marker's position and radius
			// ROI is a cube with the size of 2*radius
			bb_start_x = marker.x-radius; bb_end_x = marker.x+radius;
			bb_start_y = marker.y-radius; bb_end_y = marker.y+radius;
			bb_start_z = marker.z-radius; bb_end_z = marker.z+radius;

			// make ROI inside the volume
			bb_start_x = qBound(V3DLONG(0), V3DLONG(bb_start_x), o_sx-1);
			bb_start_y = qBound(V3DLONG(0), V3DLONG(bb_start_y), o_sy-1);
			bb_start_z = qBound(V3DLONG(0), V3DLONG(bb_start_z), o_sz-1);

			bb_end_x = qBound(V3DLONG(0), V3DLONG(bb_end_x), o_sx-1);
			bb_end_y = qBound(V3DLONG(0), V3DLONG(bb_end_y), o_sy-1);
			bb_end_z = qBound(V3DLONG(0), V3DLONG(bb_end_z), o_sz-1);
		}
		else
		{
			bb_start_x=p->xs; bb_end_x=p->xe; bb_start_y=p->ys; bb_end_y=p->ye; bb_start_z=p->zs; bb_end_z=p->ze;
			vz_x=p->xrez; vz_y=p->yrez; vz_z=p->zrez;
		}

		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para through interface---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"-----------------------------------------------------------------------";
	}
	else // paras from 2d view
	{
		// get landmarker roi
		if(b_landmarkerImging)
		{
			LandmarkList list_landmarks=((V3DPluginCallback *)cb)->getLandmark(curwin);
			if(list_landmarks.size()<1)
			{
				v3d_msg("Need at least 1 markers in subject image.");
				return logname_roi;
			}
			LocationSimple marker=list_landmarks.at(0); // always use index 0 for one marker imaging
			float radius = markerRadius;

			// create a ROI with marker's position and radius
			// ROI is a cube with the size of 2*radius
			bb_start_x = marker.x-radius; bb_end_x = marker.x+radius;
			bb_start_y = marker.y-radius; bb_end_y = marker.y+radius;
			bb_start_z = marker.z-radius; bb_end_z = marker.z+radius;

			// make ROI inside the volume
			bb_start_x = qBound(V3DLONG(0), V3DLONG(bb_start_x), o_sx-1);
			bb_start_y = qBound(V3DLONG(0), V3DLONG(bb_start_y), o_sy-1);
			bb_start_z = qBound(V3DLONG(0), V3DLONG(bb_start_z), o_sz-1);

			bb_end_x = qBound(V3DLONG(0), V3DLONG(bb_end_x), o_sx-1);
			bb_end_y = qBound(V3DLONG(0), V3DLONG(bb_end_y), o_sy-1);
			bb_end_z = qBound(V3DLONG(0), V3DLONG(bb_end_z), o_sz-1);
		}
		else
		{
			//finding the bounding box of ROI
			ROIList pRoiList=((V3DPluginCallback *)cb)->getROI(curwin);
			if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
			{
				v3d_msg("No ROI specified ... Do nothing.");
				return logname_roi;
			}

			// ROI list
			QRect b_xy = pRoiList.at(0).boundingRect();
			QRect b_yz = pRoiList.at(1).boundingRect();
			QRect b_zx = pRoiList.at(2).boundingRect();

			//finding the bounding box of ROI
			bool vxy=true,vyz=true,vzx=true; // 3 2d-views
			if(b_xy.left()==-1 || b_xy.top()==-1 || b_xy.right()==-1 || b_xy.bottom()==-1)
				vxy=false;
			if(b_yz.left()==-1 || b_yz.top()==-1 || b_yz.right()==-1 || b_yz.bottom()==-1)
				vyz=false;
			if(b_zx.left()==-1 || b_zx.top()==-1 || b_zx.right()==-1 || b_zx.bottom()==-1)
				vzx=false;

			bb_start_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.left(), b_zx.left())), o_sx-1);
			bb_start_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.top(),  b_yz.top())), o_sy-1);
			bb_start_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.left(), b_zx.top())), o_sz-1);

			bb_end_x = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.right(), b_zx.right())), o_sx-1); //use qMax but not qMin
			bb_end_y = qBound(V3DLONG(0), V3DLONG(qMax(b_xy.bottom(), b_yz.bottom())), o_sy-1);
			bb_end_z = qBound(V3DLONG(0), V3DLONG(qMax(b_yz.right(), b_zx.bottom())), o_sz-1);

			if( vxy && !vyz && !vzx )  // only has rect on xy view, roi_BF only has xy view
			{
				bb_start_z = 0;
				if(b_roi_FC)
					bb_end_z = 0;
				else
					bb_end_z = o_sz-1;
			}
			else if( !vxy && vyz && !vzx )// only has rect on yz view
			{
				bb_start_x = 0;
				bb_end_x = o_sx-1;
			}
			else if( !vxy && !vyz && vzx )// only has rect on zx view
			{
				bb_start_y = 0;
				bb_end_y = o_sy-1;
			}
		}
	} // end getting bb_start_x,y,z, bb_end_x,y,z

	if (bb_start_x>bb_end_x || bb_start_y>bb_end_y || bb_start_z>bb_end_z)
	{
		v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
		return logname_roi;
	}

	if(!b_roi_FC)
	{
		// make sure to get correct value. These values can always get from log file
		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_origin_z = pl.FOVZ_MIN;

		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
		o_rez_z = pl.FOVZ_STEP;

		vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

		// print info message
		// roi bounding box, voxel size: vz_x vz_y vz_z
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------passing para---------------";
		qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
        n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
		n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

		n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
		n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
		n_sz = ((double)(bb_end_z - bb_start_z + 1))*o_rez_z / vz_z;
		// print info message
		qDebug()<<"-----------------------------------------------------------------------";
		qDebug()<<"-----------old info---------------";
		qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
		qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
		qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
		qDebug()<<"-----------new info---------------";
		qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
		qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
		qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
		qDebug()<<"-----------------------------------------------------------------------";

		n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
		n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
		n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
		n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		//the above code has a lot of redundancy! can be simplified
	}
	else // ROI from focus control window
	{
		if(b_roi_LSMFC) //ROI from LSM focus control window
		{
			o_origin_x = pl.FOVX_MIN;
			o_origin_y = pl.FOVY_MIN;
			o_origin_z = pl.FOVZ_MIN;

			o_rez_x = pl.FOVX_STEP;
			o_rez_y = pl.FOVY_STEP;
			o_rez_z = pl.FOVZ_STEP;

			vz_x = o_rez_x/2.0; vz_y = o_rez_y/2.0; vz_z = o_rez_z; //use original resolution and be changed in the dialog

			// roi bounding box, voxel size: vz_x vz_y vz_z
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------passing para---------------";
			qDebug()<<"bb ..."<< bb_start_x << bb_end_x << bb_start_y << bb_end_y << bb_start_z << bb_end_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_origin_x = o_origin_x + (o_rez_x * (double)bb_start_x);
			n_origin_y = o_origin_y + (o_rez_y * (double)bb_start_y);
			n_origin_z = o_origin_z + (o_rez_z * (double)bb_start_z);

			n_sx = ((double)(bb_end_x - bb_start_x + 1))*o_rez_x / vz_x;
			n_sy = ((double)(bb_end_y - bb_start_y + 1))*o_rez_y / vz_y;
			n_sz = 1; // only one slice
			// print info message
			qDebug()<<"-----------------------------------------------------------------------";
			qDebug()<<"-----------old info---------------";
			qDebug()<<"dims ..."<< o_sx << o_sy << o_sz;
			qDebug()<<"origin ..."<< o_origin_x << o_origin_y << o_origin_z;
			qDebug()<<"resolution ..."<< o_rez_x << o_rez_y << o_rez_z;
			qDebug()<<"-----------new info---------------";
			qDebug()<<"dims ..."<< n_sx << n_sy << n_sz;
			qDebug()<<"origin ..."<< n_origin_x << n_origin_y << n_origin_z;
			qDebug()<<"resolution ..."<< vz_x << vz_y << vz_z;
			qDebug()<<"-----------------------------------------------------------------------";

			n_min_fovx = n_origin_x; n_min_fovy = n_origin_y; n_min_fovz = n_origin_z;
			n_max_fovx = n_origin_x + ((double)n_sx)*vz_x;
			n_max_fovy = n_origin_y + ((double)n_sy)*vz_y;
			n_max_fovz = n_origin_z + ((double)n_sz)*vz_z;
		}
		else //ROI from BF focus control window
		{
			v3d_msg("Please make sure that BF/LSM calibration has been done before this operation!\n",0);
			// transform BF coords to LSM coords: the LSM coords are physical pos
			n_min_fovx = a_bflsm * bb_start_x + e_bflsm;
			n_min_fovy = d_bflsm * bb_start_y + f_bflsm;

			n_max_fovx = a_bflsm * bb_end_x + e_bflsm;
			n_max_fovy = d_bflsm * bb_end_y + f_bflsm;

			n_min_fovz = 30.0; n_max_fovz = 35.0;  // default

			n_sx = (V3DLONG)((n_max_fovx-n_min_fovx)/fov_step_x);
			n_sy = (V3DLONG)((n_max_fovy-n_min_fovy)/fov_step_y);
			n_sz = 1;

			vz_x=fov_step_x; vz_y=fov_step_y; vz_z=fov_step_z;
		}
	}

	if(n_sx > 4096 || n_sy > 4096 || n_sz > 4096) //
	{
		printf("The target image size is too large.\n");
		return logname_roi;
	}

	// symbol link for scanning.
	//sx = n_sx; sy = n_sy; sz = n_sz;
	//min_fovx = n_min_fovx; min_fovy = n_min_fovy; min_fovz = n_min_fovz;
	//max_fovx = n_max_fovx; max_fovy = n_max_fovy; max_fovz = n_max_fovz;
	//fov_step_x = vz_x; //(max_fovx-min_fovx)/sx;
	//fov_step_y = vz_y; //(max_fovy-min_fovy)/sy;
	//fov_step_z = (max_fovz-min_fovz)/sz;

	// these are put back to lsm configure file and used as initial paras in ROI parameter dialog
	pl.FOVX_MIN = n_min_fovx;	pl.FOVX_MAX = n_max_fovx;	pl.SX = n_sx;	pl.FOVX_STEP = vz_x;
	pl.FOVY_MIN = n_min_fovy;	pl.FOVY_MAX = n_max_fovy;	pl.SY = n_sy;	pl.FOVY_STEP = vz_y;
	pl.FOVZ_MIN = n_min_fovz;	pl.FOVZ_MAX = n_max_fovz;	pl.SZ = n_sz;	pl.FOVZ_STEP = (n_max_fovz-n_min_fovz)/n_sz;


	// use previous ROI parameters
	Parameters_LSM pre_roi_para;
	if (pre_roi_para.load_ParameterFile(ssConfigFolder + "mi_configuration_last_roi.txt"))
	{
		// only use fov_step_x, fov_step_y, fov_step_z fovz_min, fovz_max of previous ROI
		pl.FOVX_STEP = pre_roi_para.FOVX_STEP;  pl.FOVY_STEP = pre_roi_para.FOVY_STEP;  pl.FOVZ_STEP = pre_roi_para.FOVZ_STEP;

		pl.FOVZ_MIN = pre_roi_para.FOVZ_MIN;	pl.FOVZ_MAX = pre_roi_para.FOVZ_MAX;

		pl.SX = (pl.FOVX_MAX - pl.FOVX_MIN)/pl.FOVX_STEP;	
		pl.SY = (pl.FOVY_MAX - pl.FOVY_MIN)/pl.FOVY_STEP;;    
		pl.SZ = (pl.FOVZ_MAX - pl.FOVZ_MIN)/pl.FOVZ_STEP;;	
	}
	else
	{
		v3d_msg("Use roi parameters of this image!", 0);
	}
	
	// user can now define parameters by ROI_Setup_Dialog
	LSM_ROI_Setup_Dialog roi_dialog(&pl, false);
	if (roi_dialog.exec()!=QDialog::Accepted)
		return logname_roi;

	roi_dialog.copyDataOut(&pl);

	logname_roi =logname;
	logname_roi.chop(4);
	logname_roi.append("_roi.txt");

	//save config info
	if (!pl.save_ParameterFile(logname_roi)) //always save to the same file, do NOT use the manually selected file
	{
		v3d_msg(QString("Fail to save to the SmartScope Configuration file [%1]").arg(logname_roi), 0);
		QString wrongstr;
		return wrongstr;
	}

	// save roi config for future use in ROI configuration 
	QString roi_FileName = ssConfigFolder + "mi_configuration_last_roi.txt";
	if (!pl.save_ParameterFile(roi_FileName)) //always save to the same file, do NOT use the manually selected file
	{
		v3d_msg(QString("Fail to save to the SmartScope Configuration file [%1]").arg(logname_roi), 0);
		QString wrongstr;
		return wrongstr;
	}

	return logname_roi;
}


