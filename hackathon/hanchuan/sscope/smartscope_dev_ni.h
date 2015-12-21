// smartscope_dev_ni.h
// the driver functions for NI devices in the smartscope project
// by Hanchuan Peng, 20110516

// Input: Configuration information such as Range (e.g. 0-30 um) ratio (e.g. 10 um/V) dimension (e.g. 1024*1024*256)
// Output: write AO x, AO y, AO z; read AI; read photon counts  

#ifndef __SMARTSCOPE_DEV_NI_H__
#define __SMARTSCOPE_DEV_NI_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

//the real function that is being used now
int DAQ_getImg(	unsigned long *pai, double *pao, unsigned long *pctr,
				long	NSAMPLES,
				double	time_out,
				double	scanning_rate, double duty_cycle,
				int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk, int dev_timing, int dev_ai, int clk_n, int timing_n, int ai_n);

//ai interface
int DAQ_getImg_ai(double *pai, double *pao,
				  double *paixy,
				  long	NSAMPLES,
				  double	time_out,
				  double	scanning_rate, double duty_cycle, 
				  int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, int dev_red, int red_n, int dev_green, int green_n);

//counter interface
int DAQ_getImg_ctr(	unsigned long *pctr0, unsigned long *pctr1, double *pao,
				   double *paixy,
				   long	NSAMPLES,
				   double	time_out,
				   double	scanning_rate, double duty_cycle, 
				   int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, int dev_red, int red_n, int dev_green, int green_n);

// use counter and ai together
//counter_ai interface
int DAQ_getImg_ctr_ai(	unsigned long *pctr0, unsigned long *pctr1, 
				   double *pai0, double *pai1,
				   double *pao, double *paixy,
				   long		NSAMPLES,
				   double	time_out,
				   double	scanning_rate, double duty_cycle, 
				   int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, 
				   int dev_red_ctr, int red_n_ctr, int dev_red_ai, int red_n_ai, int dev_green_ctr, int green_n_ctr, int dev_green_ai, int green_n_ai);

// get ai value of the current position
int DAQ_get_current_ai(double scanning_rate, double *avg_ai, int axis);

//simulation image interface
int DAQ_getImg_sim(	unsigned long *pctr0, unsigned long *pctr1, double *pai0, double *pai1, int ch, long sz_ctr,
					long sx, long sy, long sz, long planesz, long sxsw, long wait_pixels);

// separate AO z from x, y 
int DAQ_UpdateZ(double voltage, int dev_n, int ao_n);

//shutter for starting laser
int DAQ_Shutter(double voltage, int dev_n, int ao_n);

// reset AO x, y, z
int DAQ_Reset(int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z);

// ablation
int DAQ_Stimulating(double *pAO, int dev_ao, int ao_x, int ao_y, int ao_z, int dev_clk, int clk_n, double scanning_rate, double duty_cycle, int NPULSE);

// protect for detector device
int DAQ_Shutter_Detection(char onoff);

// control BF LED
int DAQ_Shutter_BFLED(char onoff);

// control Stimulation
int DAQ_Shutter_Stimulation(char onoff);

// control green laser
int DAQ_Shutter_GreenLaser(char onoff);

// control red laser
int DAQ_Shutter_RedLaser(char onoff);

// control shutter
int DAQ_Shutter_Control(int dev_n, int port_n, int line_n, char onoff);


#endif



