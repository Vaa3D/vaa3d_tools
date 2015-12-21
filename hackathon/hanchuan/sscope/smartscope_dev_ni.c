// smartscope_dev_ni.c
// Low-level control of NI devices
// 20110515: by Hanchuan Peng

#include <stdlib.h> 
#include <string.h> 

#include "NIDAQmx.h" 
#include "smartscope_dev_ni.h"

#define errChk(fCall) if (error = (fCall), error < 0) {goto Error;} else

#define CLKSURPLUS 10

int DAQ_getImg(	unsigned long *pai, double *pao, unsigned long *pctr, 
				long	NSAMPLES,
				double	time_out,
				double	scanning_rate, double duty_cycle,
				int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk, int dev_timing, int dev_ai, int clk_n, int timing_n, int ai_n)
//*pai: analog input pointer. three segments, the first segment is image data, and the two folowings are real  x,y, locations of the scanner
//*pao: analog output
//time_out: waiting time, longer than this will terminate the program by producing an Error
//scanning_rate: data acquisition speed. now is 1000Hz as of 20100722
//dev_x, y, z are the device id numbers. Now all equal device 3
//ao_x, y, z are the device id numbers. Now ao_x,y,z are 0,1,2
//dev_clk: clock device. Now is device 1's counter 0 (pfi 8)
//dev_timing: device 3's pfi 3
//clk_n: counter_0 (ctr0)
//timing_n: pfi3
//ai_n
{
	// legacy testing program
}

// reserved for ai (green) and ctr (red)
int DAQ_getImg_ai(double *pai, double *pao,
				  double *paixy,
				  long		NSAMPLES,
				  double	time_out,
				  double	scanning_rate, double duty_cycle, 
				  int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, int dev_red, int red_n, int dev_green, int green_n)
{
	// declaration
	TaskHandle  AI_taskHandle=0;
	TaskHandle  AO_taskHandle=0;
	TaskHandle	CLK_taskHandle=0;

	int32       error=0;
	char        errBuff[2048]={'\0'};

	int32       i=0; //iter

	float64		scanning_rate_limit = 2e6; // board limit PCI 6115 ai
	
	float64		thresh = -10.0; // background threshold
	double		polarity = 1; // amplifier

	int32       read_ai, read_aixy; //ai samples

	float64		*paitmp;

	// dev
	char AO_dev[15];

	char ctr0_dev[11];
	char ctr1_dev[11];
	char timing_dev[11];
	char clock_dev[11];
	
	// AO
	AO_dev[0] = 'D'; AO_dev[1] = 'e'; AO_dev[2] = 'v'; AO_dev[3] = dev_x + '0';
	AO_dev[4] = '/'; AO_dev[5] = 'a'; AO_dev[6] = 'o'; AO_dev[7] = ao_x + '0'; 
	AO_dev[8] = ':'; AO_dev[9] = ao_y + '0'; AO_dev[10] = '\0'; // 

	// clock
	clock_dev[0] = '/'; clock_dev[1] = 'D'; clock_dev[2] = 'e'; clock_dev[3] = 'v'; clock_dev[4] = dev_clk + '0'; clock_dev[5] = '/'; 
	clock_dev[6] = 'c'; clock_dev[7] = 't'; clock_dev[8] = 'r'; clock_dev[9] = clk_n + '0'; clock_dev[10] = '\0';

	// timing
	timing_dev[0] = '/'; timing_dev[1] = 'D'; timing_dev[2] = 'e'; timing_dev[3] = 'v'; timing_dev[4] = dev_timing + '0'; timing_dev[5] = '/'; 
	timing_dev[6] = 'P'; timing_dev[7] = 'F'; timing_dev[8] = 'I'; timing_dev[9] = timing_n + '0'; timing_dev[10] = '\0';

	//
	//printf("devs: %s %s %s %s \n", ctr0_dev, ctr1_dev, clock_dev, timing_dev);
	printf("NSAMPLES: %ld; Scan Rate: %lf \n", NSAMPLES, scanning_rate);

	paitmp = malloc(4*NSAMPLES*sizeof(float64));

	if(paitmp==0)
	{
		printf("ERROR: Out of memory\n");
		return -1;
	}
	else
	{
		//memcpy(paitmp, 0, 4*NSAMPLES);
		memset(paitmp, 0, 4*NSAMPLES);
	}

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&AO_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_taskHandle,AO_dev,"",-10.0,10.0,DAQmx_Val_Volts,NULL)); //x, y, z
	DAQmxErrChk (DAQmxCfgSampClkTiming(AO_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,NSAMPLES)); //max samples per second per channel

	DAQmxErrChk (DAQmxCreateTask("",&AI_taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(AI_taskHandle,"Dev1/ai0:3","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); // ai_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(AI_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev
	DAQmxErrChk (DAQmxCfgInputBuffer(AI_taskHandle,NSAMPLES));

	DAQmxErrChk (DAQmxCreateTask("",&CLK_taskHandle));
	DAQmxErrChk (DAQmxCreateCOPulseChanFreq(CLK_taskHandle,clock_dev,"",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,scanning_rate, duty_cycle)); // clk_dev
	DAQmxErrChk (DAQmxCfgImplicitTiming(CLK_taskHandle,DAQmx_Val_FiniteSamps,NSAMPLES+CLKSURPLUS)); // external clock 

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_taskHandle,NSAMPLES,0,time_out,DAQmx_Val_GroupByChannel,pao,NULL,NULL)); //
	
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(AO_taskHandle));
	DAQmxErrChk (DAQmxStartTask(AI_taskHandle)); // ai data
	DAQmxErrChk (DAQmxStartTask(CLK_taskHandle)); // clock always start last

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(AI_taskHandle,NSAMPLES,time_out,DAQmx_Val_GroupByChannel,paitmp,4*NSAMPLES,&read_ai,NULL)); // ai data

	// assign back
	for(i = 0; i< 2*NSAMPLES; i++)
	{
		pai[i] = paitmp[i];
		paixy[i] = paitmp[i+2*NSAMPLES];
	}

	//
	free(paitmp);
	
	//
	printf("Scan finished! \n");

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( AO_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_taskHandle);
		DAQmxClearTask(AO_taskHandle);
	}
	if( CLK_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CLK_taskHandle);
		DAQmxClearTask(CLK_taskHandle);
	}
	if( AI_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AI_taskHandle);
		DAQmxClearTask(AI_taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;

}

// imaging with ctr0 (red) and ctr1 (green)
int DAQ_getImg_ctr(	unsigned long *pctr0, unsigned long *pctr1, double *pao,
				   double *paixy,
				   long		NSAMPLES,
				   double	time_out,
				   double	scanning_rate, double duty_cycle, 
				   int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, int dev_red, int red_n, int dev_green, int green_n)
{
	// declaration
	TaskHandle  CTR0_taskHandle=0;
	TaskHandle  CTR1_taskHandle=0;
	TaskHandle  AO_taskHandle=0;
	TaskHandle	CLK_taskHandle=0;
	TaskHandle  AIXY_taskHandle = 0;

	int32       read_ctr0; //how many samples
	int32       read_ctr1; //

	int32       error=0;
	char        errBuff[2048]={'\0'};

	int32       i=0; //iter

	float64		scanning_rate_limit = 2e6; // board limit PCI 6115 ai
	
	float64		thresh = -10.0; // background threshold
	double		polarity = 1; // amplifier

	int32       read_ai; //ai samples

	// dev
	char AO_dev[15];

	char ctr0_dev[11];
	char ctr1_dev[11];
	char timing_dev[11];
	char clock_dev[11];

	
	// AO
	AO_dev[0] = 'D'; AO_dev[1] = 'e'; AO_dev[2] = 'v'; AO_dev[3] = dev_x + '0';
	AO_dev[4] = '/'; AO_dev[5] = 'a'; AO_dev[6] = 'o'; AO_dev[7] = ao_x + '0'; 
	AO_dev[8] = ':'; AO_dev[9] = ao_y + '0'; AO_dev[10] = '\0'; // 

	// ctr0
	ctr0_dev[0] = '/'; ctr0_dev[1] = 'D'; ctr0_dev[2] = 'e'; ctr0_dev[3] = 'v'; ctr0_dev[4] = dev_red + '0'; ctr0_dev[5] = '/'; 
	ctr0_dev[6] = 'c'; ctr0_dev[7] = 't'; ctr0_dev[8] = 'r'; ctr0_dev[9] = red_n + '0'; ctr0_dev[10] = '\0';

	// ctr1
	ctr1_dev[0] = '/'; ctr1_dev[1] = 'D'; ctr1_dev[2] = 'e'; ctr1_dev[3] = 'v'; ctr1_dev[4] = dev_green + '0'; ctr1_dev[5] = '/'; 
	ctr1_dev[6] = 'c'; ctr1_dev[7] = 't'; ctr1_dev[8] = 'r'; ctr1_dev[9] = green_n + '0'; ctr1_dev[10] = '\0';

	// clock
	clock_dev[0] = '/'; clock_dev[1] = 'D'; clock_dev[2] = 'e'; clock_dev[3] = 'v'; clock_dev[4] = dev_clk + '0'; clock_dev[5] = '/'; 
	clock_dev[6] = 'c'; clock_dev[7] = 't'; clock_dev[8] = 'r'; clock_dev[9] = clk_n + '0'; clock_dev[10] = '\0';

	// timing
	timing_dev[0] = '/'; timing_dev[1] = 'D'; timing_dev[2] = 'e'; timing_dev[3] = 'v'; timing_dev[4] = dev_timing + '0'; timing_dev[5] = '/'; 
	timing_dev[6] = 'P'; timing_dev[7] = 'F'; timing_dev[8] = 'I'; timing_dev[9] = timing_n + '0'; timing_dev[10] = '\0';

	//
	//printf("devs: %s %s %s %s \n", ctr0_dev, ctr1_dev, clock_dev, timing_dev);
	printf("NSAMPLES: %ld; Scan Rate: %lf \n", NSAMPLES, scanning_rate);
	
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&AO_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_taskHandle,AO_dev,"",-10.0,10.0,DAQmx_Val_Volts,NULL)); //x, y, z
	DAQmxErrChk (DAQmxCfgSampClkTiming(AO_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,NSAMPLES)); //max samples per second per channel

	DAQmxErrChk (DAQmxCreateTask("",&AIXY_taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(AIXY_taskHandle,"Dev1/ai0:1","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); // ai_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(AIXY_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev
	DAQmxErrChk (DAQmxCfgInputBuffer(AIXY_taskHandle,NSAMPLES));

	DAQmxErrChk (DAQmxCreateTask("",&CTR0_taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR0_taskHandle,ctr0_dev,"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // photon counter ctr0_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(CTR0_taskHandle,"/Dev3/PFI7",scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev

	DAQmxErrChk (DAQmxCreateTask("",&CTR1_taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR1_taskHandle,ctr1_dev,"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // ctr1_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(CTR1_taskHandle,"/Dev3/PFI7",scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev

	DAQmxErrChk (DAQmxCreateTask("",&CLK_taskHandle));
	DAQmxErrChk (DAQmxCreateCOPulseChanFreq(CLK_taskHandle,clock_dev,"",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,scanning_rate, duty_cycle)); // clk_dev
	DAQmxErrChk (DAQmxCfgImplicitTiming(CLK_taskHandle,DAQmx_Val_FiniteSamps,NSAMPLES+CLKSURPLUS)); // external clock 


	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_taskHandle,NSAMPLES,0,time_out,DAQmx_Val_GroupByChannel,pao,NULL,NULL)); //
	
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(AO_taskHandle));
	DAQmxErrChk (DAQmxStartTask(AIXY_taskHandle)); // testing ai x, y
	DAQmxErrChk (DAQmxStartTask(CTR0_taskHandle));
	DAQmxErrChk (DAQmxStartTask(CTR1_taskHandle));
	DAQmxErrChk (DAQmxStartTask(CLK_taskHandle)); // clock always start last

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadCounterU32(CTR1_taskHandle,NSAMPLES,time_out,pctr1,NSAMPLES,&read_ctr1,NULL)); //
	DAQmxErrChk (DAQmxReadCounterU32(CTR0_taskHandle,NSAMPLES,time_out,pctr0,NSAMPLES,&read_ctr0,NULL)); 

	DAQmxErrChk (DAQmxReadAnalogF64(AIXY_taskHandle,NSAMPLES,time_out,DAQmx_Val_GroupByChannel,paixy,2*NSAMPLES,&read_ai,NULL)); // ai x y

	// rescale counting
	for(i=NSAMPLES-1; i>0; i--)
	{
		pctr0[i] -= pctr0[i-1];
		pctr1[i] -= pctr1[i-1];
	}
	pctr0[0] = pctr0[1];
	pctr1[0] = pctr1[1]; //

	//
	printf("Scan finished! \n");

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( AO_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_taskHandle);
		DAQmxClearTask(AO_taskHandle);
	}
	if( CLK_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CLK_taskHandle);
		DAQmxClearTask(CLK_taskHandle);
	}
	if( CTR0_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CTR0_taskHandle);
		DAQmxClearTask(CTR0_taskHandle);
	}
	if( CTR1_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CTR1_taskHandle);
		DAQmxClearTask(CTR1_taskHandle);
	}
	if( AIXY_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AIXY_taskHandle);
		DAQmxClearTask(AIXY_taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}

// imaging with ctr0 (red) and ctr1 (green) or 
int DAQ_getImg_ctr_ai(unsigned long *pctr0, //counter output
					  unsigned long *pctr1, //counter output 
				   double *pai0,			//green PMT ? output
				   double *pai1,			//red PMT ? output
				   double *pao,				//input voltages
				   double *paixy,			//output vlotages (from scanner)
				   long		NSAMPLES,
				   double	time_out,
				   double	scanning_rate, double duty_cycle, 
				   int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z, int dev_clk,  int clk_n, int dev_timing, int timing_n, 
				   int dev_red_ctr, int red_n_ctr, int dev_red_ai, int red_n_ai, int dev_green_ctr, int green_n_ctr, int dev_green_ai, int green_n_ai)
{
	// declaration
	TaskHandle  CTR0_taskHandle=0;
	TaskHandle  CTR1_taskHandle=0;
	TaskHandle  AO_taskHandle=0;
	TaskHandle	CLK_taskHandle=0;
	TaskHandle  AIXY_taskHandle = 0; //include ai, aixy

	int32       read_ctr0; //how many samples
	int32       read_ctr1; //

	int32       error=0;
	char        errBuff[2048]={'\0'};

	int32       i=0; //iter

	float64		scanning_rate_limit = 2e6; // board limit PCI 6115 ai
	
	float64		thresh = -10.0; // background threshold
	double		polarity = 1; // amplifier

	int32       read_ai; //ai samples
	float64		*paitmp = 0;
	float64     *pai = 0; //temp for pai0, pai1

	// dev
	char AO_dev[15];

	char ctr0_dev[11];
	char ctr1_dev[11];
	char ai_dev[11];
	char timing_dev[11];
	char clock_dev[11];

	//if (!pai0 || !pai1 || !pao || !paixy || !pctr0 || !pctr1 || NSAMPLES<=0)
	//	return -1;

	// AO
	AO_dev[0] = 'D'; AO_dev[1] = 'e'; AO_dev[2] = 'v'; AO_dev[3] = dev_x + '0';
	AO_dev[4] = '/'; AO_dev[5] = 'a'; AO_dev[6] = 'o'; AO_dev[7] = ao_x + '0'; 
	AO_dev[8] = ':'; AO_dev[9] = ao_y + '0'; AO_dev[10] = '\0'; // 

	// ctr0
	ctr0_dev[0] = '/'; ctr0_dev[1] = 'D'; ctr0_dev[2] = 'e'; ctr0_dev[3] = 'v'; ctr0_dev[4] = dev_green_ctr + '0'; ctr0_dev[5] = '/'; 
	ctr0_dev[6] = 'c'; ctr0_dev[7] = 't'; ctr0_dev[8] = 'r'; ctr0_dev[9] = green_n_ctr + '0'; ctr0_dev[10] = '\0';

	// ctr1
	ctr1_dev[0] = '/'; ctr1_dev[1] = 'D'; ctr1_dev[2] = 'e'; ctr1_dev[3] = 'v'; ctr1_dev[4] = dev_red_ctr + '0'; ctr1_dev[5] = '/'; 
	ctr1_dev[6] = 'c'; ctr1_dev[7] = 't'; ctr1_dev[8] = 'r'; ctr1_dev[9] = red_n_ctr + '0'; ctr1_dev[10] = '\0';

	/////////// for ai dev
	ai_dev[0] = 'D'; ai_dev[1] = 'e'; ai_dev[2] = 'v'; ai_dev[3] = dev_red_ai + '0'; ai_dev[4] = '/'; 
	ai_dev[5] = 'a'; ai_dev[6] = 'i'; ai_dev[7] = 0 + '0'; ai_dev[8] = ':';  ai_dev[9] = 3 + '0'; ai_dev[10] = '\0';

	///////////

	// clock
	clock_dev[0] = '/'; clock_dev[1] = 'D'; clock_dev[2] = 'e'; clock_dev[3] = 'v'; clock_dev[4] = dev_clk + '0'; clock_dev[5] = '/'; 
	clock_dev[6] = 'c'; clock_dev[7] = 't'; clock_dev[8] = 'r'; clock_dev[9] = clk_n + '0'; clock_dev[10] = '\0';

	// timing
	timing_dev[0] = '/'; timing_dev[1] = 'D'; timing_dev[2] = 'e'; timing_dev[3] = 'v'; timing_dev[4] = dev_timing + '0'; timing_dev[5] = '/'; 
	timing_dev[6] = 'P'; timing_dev[7] = 'F'; timing_dev[8] = 'I'; timing_dev[9] = timing_n + '0'; timing_dev[10] = '\0';

	//
	//printf("devs: %s %s %s %s \n", ctr0_dev, ctr1_dev, clock_dev, timing_dev);
	printf("NSAMPLES: %ld; Scan Rate: %lf \n", NSAMPLES, scanning_rate);

	paitmp = malloc(4*NSAMPLES*sizeof(float64));

	if(paitmp==0)
	{
		printf("ERROR: Out of memory in DAQ_getImg_ctr_ai!\n");
		return -1;
	}
	else
	{
		memset(paitmp, 0, 4*NSAMPLES);
	}
	
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&AO_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_taskHandle,AO_dev,"",-10.0,10.0,DAQmx_Val_Volts,NULL)); //x, y, z
	DAQmxErrChk (DAQmxCfgSampClkTiming(AO_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,NSAMPLES)); //max samples per second per channel

	DAQmxErrChk (DAQmxCreateTask("",&AIXY_taskHandle));
	//DAQmxErrChk (DAQmxCreateAIVoltageChan(AIXY_taskHandle,"Dev1/ai0:3","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); // ai_dev
	DAQmxErrChk (DAQmxCreateAIVoltageChan(AIXY_taskHandle,ai_dev,"",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); // ai_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(AIXY_taskHandle,timing_dev,scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev
	DAQmxErrChk (DAQmxCfgInputBuffer(AIXY_taskHandle,NSAMPLES));

	DAQmxErrChk (DAQmxCreateTask("",&CTR0_taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR0_taskHandle,ctr0_dev,"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // photon counter ctr0_dev
	//DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR0_taskHandle,"Dev3/ctr0","",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // photon counter ctr0_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(CTR0_taskHandle,"/Dev3/PFI7",scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev

	DAQmxErrChk (DAQmxCreateTask("",&CTR1_taskHandle));
	DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR1_taskHandle,ctr1_dev,"",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // ctr1_dev
	//DAQmxErrChk (DAQmxCreateCICountEdgesChan(CTR1_taskHandle,"Dev3/ctr1","",DAQmx_Val_Rising,0,DAQmx_Val_CountUp)); // ctr1_dev
	DAQmxErrChk (DAQmxCfgSampClkTiming(CTR1_taskHandle,"/Dev3/PFI7",scanning_rate_limit,DAQmx_Val_Falling,DAQmx_Val_FiniteSamps,NSAMPLES)); //timing_dev

	DAQmxErrChk (DAQmxCreateTask("",&CLK_taskHandle));
	DAQmxErrChk (DAQmxCreateCOPulseChanFreq(CLK_taskHandle,clock_dev,"",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,scanning_rate, duty_cycle)); // clk_dev
	DAQmxErrChk (DAQmxCfgImplicitTiming(CLK_taskHandle,DAQmx_Val_FiniteSamps,NSAMPLES+CLKSURPLUS)); // external clock 


	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_taskHandle,NSAMPLES,0,time_out,DAQmx_Val_GroupByChannel,pao,NULL,NULL)); //
	
	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(AO_taskHandle));
	DAQmxErrChk (DAQmxStartTask(AIXY_taskHandle)); // testing ai x, y
	DAQmxErrChk (DAQmxStartTask(CTR0_taskHandle));
	DAQmxErrChk (DAQmxStartTask(CTR1_taskHandle));
	DAQmxErrChk (DAQmxStartTask(CLK_taskHandle)); // clock always start last

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadCounterU32(CTR1_taskHandle,NSAMPLES,time_out,pctr1,NSAMPLES,&read_ctr1,NULL)); //
	DAQmxErrChk (DAQmxReadCounterU32(CTR0_taskHandle,NSAMPLES,time_out,pctr0,NSAMPLES,&read_ctr0,NULL)); 

	DAQmxErrChk (DAQmxReadAnalogF64(AIXY_taskHandle,NSAMPLES,time_out,DAQmx_Val_GroupByChannel,paitmp,4*NSAMPLES,&read_ai,NULL)); // ai x y //from 2 to 4

	pai = malloc(2*NSAMPLES*sizeof(float64));
	if(pai==0)
	{
		printf("ERROR: Out of memory in DAQ_getImg_ctr_ai!\n");
		return -1;
	}else
	{
		memset(pai, 0, 2*NSAMPLES);
	}
	// assign back ai, aixy
	for(i = 0; i< 2*NSAMPLES; i++)
	{
		pai[i] = paitmp[i];
		paixy[i] = paitmp[i+2*NSAMPLES];
	}

	// split pai to pai0 and pai1
	for(i = 0; i<NSAMPLES; i++)
	{
			pai0[i]=pai[i];
			pai1[i]=pai[i+NSAMPLES];
	}

	// rescale counting
	for(i=NSAMPLES-1; i>0; i--)
	{
		pctr0[i] -= pctr0[i-1];
		pctr1[i] -= pctr1[i-1];
	}
	pctr0[0] = pctr0[1];
	pctr1[0] = pctr1[1]; //

	//
	printf("Scan finished! \n");

Error:
	if (paitmp) {free(paitmp); paitmp=0;}
	if (pai) {free(pai); pai=0;}

	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( AO_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_taskHandle);
		DAQmxClearTask(AO_taskHandle);
	}
	if( CLK_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CLK_taskHandle);
		DAQmxClearTask(CLK_taskHandle);
	}
	if( CTR0_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CTR0_taskHandle);
		DAQmxClearTask(CTR0_taskHandle);
	}
	if( CTR1_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(CTR1_taskHandle);
		DAQmxClearTask(CTR1_taskHandle);
	}
	if( AIXY_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AIXY_taskHandle);
		DAQmxClearTask(AIXY_taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}




int DAQ_Stimulating(double *pAO, int dev_ao, int ao_x, int ao_y, int ao_z, int dev_clk, int clk_n, double scanning_rate, double duty_cycle, int NPULSE)
{
	int			error=0;
	char		errBuff[2048]={'\0'};

	TaskHandle	AO_x_taskHandle=0;
	TaskHandle	AO_y_taskHandle=0;
	TaskHandle	AO_z_taskHandle=0;
	TaskHandle  CTR_taskHandle=0;

	TaskHandle  taskHandle_ai_xy=0;

	float64		data_x[1];
	float64		data_y[1];
	float64		data_z[1];

	int32       read_ai_xy;
	float64     data_ai_xy[4];
	int i;
	float64 total_x = 0.0;
	float64 total_y = 0.0;

	char		aox_dev[11];
	char		aoy_dev[11];
	char		aoz_dev[11];
	char		clock_dev[11];
	
	data_x[0] = pAO[0]; data_y[0] = pAO[1]; data_z[0] = pAO[2];
	//printf("pAO of Marker (x,y,z): (%lf, %lf, %lf)\n",data_x[0],data_y[0],data_z[0]);
	
	aox_dev[0] = 'D'; aox_dev[1] = 'e'; aox_dev[2] = 'v'; aox_dev[3] = dev_ao + '0'; aox_dev[4] = '/'; 
	aox_dev[5] = 'a'; aox_dev[6] = 'o'; aox_dev[7] = ao_x + '0'; aox_dev[8] = '\0';

	aoy_dev[0] = 'D'; aoy_dev[1] = 'e'; aoy_dev[2] = 'v'; aoy_dev[3] = dev_ao + '0'; aoy_dev[4] = '/'; 
	aoy_dev[5] = 'a'; aoy_dev[6] = 'o'; aoy_dev[7] = ao_y + '0'; aoy_dev[8] = '\0';

	aoz_dev[0] = 'D'; aoz_dev[1] = 'e'; aoz_dev[2] = 'v'; aoz_dev[3] = dev_ao + '0'; aoz_dev[4] = '/'; 
	aoz_dev[5] = 'a'; aoz_dev[6] = 'o'; aoz_dev[7] = ao_z + '0'; aoz_dev[8] = '\0';

	clock_dev[0] = '/'; clock_dev[1] = 'D'; clock_dev[2] = 'e'; clock_dev[3] = 'v'; clock_dev[4] = dev_clk + '0'; clock_dev[5] = '/'; 
	clock_dev[6] = 'c'; clock_dev[7] = 't'; clock_dev[8] = 'r'; clock_dev[9] = clk_n + '0'; clock_dev[10] = '\0';

	//printf("devs %s %s %s %s\n", aox_dev, aoy_dev, aoz_dev, clock_dev);
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&AO_x_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_x_taskHandle,aox_dev,"",-10.0,10.0,DAQmx_Val_Volts,""));

	DAQmxErrChk (DAQmxCreateTask("",&AO_y_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_y_taskHandle,aoy_dev,"",-10.0,10.0,DAQmx_Val_Volts,""));

	DAQmxErrChk (DAQmxCreateTask("",&AO_z_taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(AO_z_taskHandle,"Dev3/ao2","",0,10.0,DAQmx_Val_Volts,""));

	//calibration
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle_ai_xy));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle_ai_xy,"/Dev1/ai0:1","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle_ai_xy,"",1000.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,2));

	//DAQmxErrChk (DAQmxCreateTask("",&CTR_taskHandle));
	//DAQmxErrChk (DAQmxCreateCOPulseChanFreq(CTR_taskHandle,clock_dev,"",DAQmx_Val_Hz,DAQmx_Val_Low,0.0,scanning_rate, duty_cycle));
	//DAQmxErrChk (DAQmxCfgImplicitTiming(CTR_taskHandle,DAQmx_Val_FiniteSamps,NPULSE)); //NPULSE

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_x_taskHandle,1,1,20.0,DAQmx_Val_GroupByChannel,data_x,NULL,NULL));
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_y_taskHandle,1,1,20.0,DAQmx_Val_GroupByChannel,data_y,NULL,NULL));
	DAQmxErrChk (DAQmxWriteAnalogF64(AO_z_taskHandle,1,1,20.0,DAQmx_Val_GroupByChannel,data_z,NULL,NULL));

	//DAQmxErrChk (DAQmxWriteAnalogF64(AO_taskHandle,NSAMPLES,0,time_out,DAQmx_Val_GroupByChannel,pao,NULL,NULL)); // reference from ctr

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(AO_x_taskHandle));
	DAQmxErrChk (DAQmxStartTask(AO_y_taskHandle));
	DAQmxErrChk (DAQmxStartTask(AO_z_taskHandle));
	//DAQmxErrChk (DAQmxStartTask(CTR_taskHandle)); // laser 3 but now not in use 

	DAQmxErrChk (DAQmxStartTask(taskHandle_ai_xy));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle_ai_xy,2,10.0,DAQmx_Val_GroupByChannel,data_ai_xy,4,&read_ai_xy,NULL));

	// output AIx on this marker
	for(i=0; i<2; i++)
	{
		total_x += data_ai_xy[i];
		total_y += data_ai_xy[2+i];
	}
	//(*pAIx) = total_x/2.0;
	//printf("Real Marker (x,y,z): (%lf, %lf, %lf)\n", (*pAIx), total_y/2.0,data_z[0]);

	
Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( AO_x_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_x_taskHandle);
		DAQmxClearTask(AO_x_taskHandle);
	}
	if( AO_y_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_y_taskHandle);
		DAQmxClearTask(AO_y_taskHandle);
	}
	if( AO_z_taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(AO_z_taskHandle);
		DAQmxClearTask(AO_z_taskHandle);
	}
	if( taskHandle_ai_xy!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle_ai_xy);
		DAQmxClearTask(taskHandle_ai_xy);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}


int DAQ_get_current_ai(double scanning_rate, double *avg_ai, int axis)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	int32       read;
	float64     data[4], total=0.0;
	char        errBuff[2048]={'\0'};
	int i;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	if(axis == 0) //x axis
		DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai2","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); //"", DAQmx_Val_Cfg_Default
	else //y axis
		DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai3","",DAQmx_Val_PseudoDiff,-10.0,10.0,DAQmx_Val_Volts,NULL)); //"", DAQmx_Val_Cfg_Default
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",scanning_rate,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,4)); //1000.0

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,4,10.0,DAQmx_Val_GroupByChannel,data,4,&read,NULL));

	//printf("Acquired %d points\n",read);
	for(i=0;i<4;i++)
	{
		total+= data[i];
	}
	// only average in x direction
	(*avg_ai) = total/4.0;

	//printf("Average = %f \n",(*avg_ai));


Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 )  {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	
	return 0;
}




int DAQ_UpdateZ(double voltage, int dev_n, int ao_n)
{
	int			error=0;
	TaskHandle	taskHandle=0;
	char		errBuff[2048]={'\0'};

	float64		data[1]; 
	char ao_dev[11];
	
	data[0] = voltage;
	
	ao_dev[0] = 'D'; ao_dev[1] = 'e'; ao_dev[2] = 'v'; ao_dev[3] = dev_n + '0'; ao_dev[4] = '/'; 
	ao_dev[5] = 'a'; ao_dev[6] = 'o'; ao_dev[7] = ao_n + '0'; ao_dev[8] = '\0';

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandle,ao_dev,"",0.0,10.0,DAQmx_Val_Volts,"")); // z [0, 10]

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));

	//*********************************************/
	// DAQmx Read Code
	//*********************************************/
	//DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,1,10.0,DAQmx_Val_GroupByChannel,paixy,2,&read_ai,NULL)); // ai z

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}

int DAQ_Shutter(double voltage, int dev_n, int ao_n)
{
	int			error=0;
	TaskHandle	taskHandle=0;
	char		errBuff[2048]={'\0'};

	float64		data[1]; 
	char ao_dev[11];
	
	data[0] = voltage;
	
	ao_dev[0] = 'D'; ao_dev[1] = 'e'; ao_dev[2] = 'v'; ao_dev[3] = dev_n + '0'; ao_dev[4] = '/'; 
	ao_dev[5] = 'a'; ao_dev[6] = 'o'; ao_dev[7] = ao_n + '0'; ao_dev[8] = '\0';

	printf("DAQmx Shutter Trigger.\n");
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandle,ao_dev,"",-10.0,10.0,DAQmx_Val_Volts,""));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data,NULL,NULL));

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}

int DAQ_Reset(int dev_x, int dev_y, int dev_z, int ao_x, int ao_y, int ao_z)
{
	// x,y,z voltage reset to 0.0
	DAQ_Shutter(0.0, dev_x, ao_x);
	DAQ_Shutter(0.0, dev_y, ao_y);
	DAQ_Shutter(3.0, dev_z, ao_z);
}


// shutter controler
int DAQ_Shutter_Control(int dev_n, int port_n, int line_n, char onoff)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	uInt8       data_on[1]={1};
	uInt8       data_off[1]={0};
	char        errBuff[2048]={'\0'};
	char		dev[512];

	// dev
	dev[0] = 'D'; dev[1] = 'e'; dev[2] = 'v';
	dev[3] =  dev_n + '0'; dev[4] = '/'; dev[5] = 'p';
	dev[6] = 'o'; dev[7] = 'r'; dev[8] = 't';
	dev[9] = port_n + '0'; dev[10] = '/'; dev[11] = 'l';
	dev[12] = 'i'; dev[13] = 'n'; dev[14] = 'e';
	dev[15] = line_n + '0'; dev[16] ='\0';

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,dev,"",DAQmx_Val_ChanPerLine));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	if(onoff)
	{
		DAQmxErrChk (DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data_on,NULL,NULL));
	}
	else
	{
		DAQmxErrChk (DAQmxWriteDigitalLines(taskHandle,1,1,10.0,DAQmx_Val_GroupByChannel,data_off,NULL,NULL));
	}
	
Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);

	return 0;
}

// shutter BF
int DAQ_Shutter_BFLED(char onoff)
{
	/*********************************************/
	// DAQmx Shutter Configure Code
	/*********************************************/
	return DAQ_Shutter_Control(2, 0, 2, onoff);
}


int DAQ_Shutter_RedLaser(char onoff)
{
	/*********************************************/
	// DAQmx Shutter Configure Code
	/*********************************************/
	return DAQ_Shutter_Control(2, 0, 1, onoff);
}

int DAQ_Shutter_GreenLaser(char onoff)
{
	/*********************************************/
	// DAQmx Shutter Configure Code
	/*********************************************/
	return DAQ_Shutter_Control(2, 0, 0, onoff);
}

int DAQ_Shutter_Detection(char onoff)
{
	/*********************************************/
	// DAQmx Shutter Configure Code
	/*********************************************/
	return DAQ_Shutter_Control(2, 0, 6, onoff);
}

int DAQ_Shutter_Stimulation(char onoff)
{
	/*********************************************/
	// DAQmx Shutter Configure Code
	/*********************************************/
	return DAQ_Shutter_Control(2, 0, 7, onoff);

}


// This function will create a disk at a random position of the image
int DAQ_getImg_sim(	unsigned long *pctr0, unsigned long *pctr1, double *pai0, double *pai1, int ch, long sz_ctr,
					long sx, long sy, long sz, long planesz, long sxsw, long wait_pixels)
{
     long i, j, k;
	 float radius;
     // initialize all to 0
     for(k=0; k<sz_ctr; k++)
     {
          if(ch==0)
		  {
               pctr1[k]=0; pai1[k]=0;
		  }
          else if(ch==1)
		  {
               pctr0[k]=0; pai0[k]=0;
		  }
          if(ch==2)
          {
               pctr0[k]=0; pai0[k]=0;
               pctr1[k]=0; pai1[k]=0;
          }
     }

	 radius = (sx>=sy)? sy/4.0 : sx/4.0;
     if(radius==0.0) return 1;

     for(k=0; k<sz; k++)
     {
          long offset_k = k*planesz;

		  // create random center
		  int cenx, ceny;// random center
		  int rx=rand()%sx;
		  int ry=rand()%sy;
		  if((sx/2-rx)>=0) cenx=sx/2 - rx/4;
			else cenx=sx/2 + rx/4;
		  if((sy/2-ry)>=0) ceny=sy/2 - ry/4;
			else ceny=sy/2 + ry/4;

          for(j=0; j<sy; j++)
          {
               long offset = j*sxsw + offset_k;

               // for real image part
			   // create a disk (value decreases from center to surroundings)
			   // the center is randomly located
               for(i=wait_pixels; i<sxsw; i++)
               {
                    float dist;
                    dist = sqrt( (float)(i-(wait_pixels+cenx))*(i-(wait_pixels+cenx)) +
                         (float)(j-ceny)*(j-ceny));

                    if(dist<radius)
                    {
                         long idx = offset + i;

                         if(ch==0)
						 {
                              pctr1[idx]=255*(1-dist/radius);
							  pai1[idx]=(double)(255*(1-dist/radius));
						 }
                         else if(ch==1)
						 {
                              pctr0[idx]=255*(1-dist/radius);
							  pai0[idx]=(double)(255*(1-dist/radius));
						 }
                         if(ch==2)
                         {
                              pctr0[idx]=255*(1-dist/radius);
                              pctr1[idx]=255*(1-dist/radius);

							  pai0[idx]=(double)(255*(1-dist/radius));
							  pai1[idx]=(double)(255*(1-dist/radius));
                         }
                    }
               }
          }
     }

	 //printf("Finish creating simulation image");
	 return 0;
}
