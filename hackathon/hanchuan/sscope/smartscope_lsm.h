
// smartscope_lsm.h
// the Laser scanning code for the smartscope project
// by Hanchuan Peng
// 20110516

#ifndef __SMARTSCOPE_LSM_H__
#define __SMARTSCOPE_LSM_H__

#include "v3d_interface.h"

#include "configfileio_lsm.h"

extern "C" {
#include "smartscope_dev_ni.h"  // Get declaration for int DAQ_getImg;
}

class LaserScanImaging
{
public:
	bool b_reset_PHC;
	void make_a_copy_PHC(bool b_forward);

	LaserScanImaging(V3DPluginCallback &callback);
	~LaserScanImaging(){cleanData();}

	void initializeData(); //initialize all data pointers to 0, etc.
	void cleanData(bool b_cleanLUT=true); //free all spaces
	void freeDataBuffer() {if (pImFloat) {delete []pImFloat; pImFloat=0;}
							if (pImFloat_f) {delete []pImFloat_f; pImFloat_f=0;}}

	bool setup_parameters(QString para_file=""); //set up parameters
	void printParameters();

	void acquire(); //acquire images in zigzag direction
	bool acquire_single_direction(int op, bool b_from3DViewer=false); //acquire images in single direction
	bool do_image_withROI_i();   // imaging with ROI_i
	bool do_image_withROI_i_x(); // imaging with ROI_i from full X
	bool do_LSM_single(bool b_nopostprocessing=false);        // do laser scanning with single direction
	bool do_image_3DViewROI();   // imaging roi which chosen from 3d view
	bool do_image_adaptiveROI();   // imaging with adaptive ROI (ROI is from a 2d slice)
	void do_image_ablation(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks);    // ablation: stimulating with strong laser
	void do_image_ablation_with_feedback(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks);

	void do_image_landmarkers(v3dhandle curwin, Image4DSimple *pImgIn, LandmarkList list_landmarks); // imaging on landermarkers

	QString get_roi_scanning_parameters(bool b_from3DViewer, bool b_landmarkerImging, int markerRadius); //by PHC 120521
	
	LandmarkList LaserScanImaging::get_ablation_marker_parameters(bool b_from3DViewer);

	void getLineEqu(double &offset, double &slope, float *X, float *Y, V3DLONG &nCount); // using least square method to get line equation
	void readAIAOFile(QString& filename, float *pAIx, float*pAOx, V3DLONG &size);
	void getCalibOffset(FILE *fp, int &offset_xp);// compute scanner calibration: get offset value in x direction

	void get_current_scanning_mirror_offset(); //by PHC, 20120420
    void set_real_scanning_range_with_real_offset(bool b_addrightmargin=true); //by PHC, 20120420

	//void getAdaptiveROI(V3DPluginCallback *callback, V3DLONG n_sx, V3DLONG n_sy, V3DLONG n_sz,
	//					V3DLONG &minx, V3DLONG &miny, V3DLONG &maxx, V3DLONG &maxy); // get ROI based on slice content in one slice

	void do_init();// parsing parameters
	V3DLONG waiting_pxls(double scanning_rate); 	// waiting pixels per line for saw-tooth
	void wait( int milliseconds );
	void set_voltages();

	bool set_scanning_buffers(); // set paras for LSM single direction scanning
	void shutter_open(); // open shutters for detection
	void shutter_close_daq_reset(bool b_onlycloseshutter=false); // close shutters for detection

    bool create_real_pos_LUT(float* pAIx_in, float* pAOx_in);

	void choose_roi();
	void set_mi();
	void set_mi_roi_acc();
	void set_mi_focuscontrol();
	void set_mi_zigzag();
	void set_mi_zigzag_acc();
	void set_mi_zigzag_acc_dd();

	void do_image_zigzag_acc(); // imaging
	void do_image_zigzag_acc_dd(); // imaging double direction
    void do_image_acc(); // imaging with single direction, no zigzagw

public:
	float *pImFloat;
	void *cb;

	int imaging_op; // 0: Acquisition: full image: simultaneous;
					// 1: Acquisition: full image with ROI (interactive)
					// 2: Acquisition: full 2D image at z=30um (one slice)
					// 3: Focus Controls (FL)
					// 4: ROI_IMAGING
					// 5: AO/AI Calibration
					// 6: Adaptive ROI imaging
					// 7: interactive ROI from full X
		            // 8: one landmarker imaging
					// 9: landmarker ablation

	V3DLONG i,j,k;
	double x,y,z;

	// para
	V3DLONG WAIT_PIXELS;
	V3DLONG sxsw; // MI imaging
	V3DLONG planesz, pagesz, offset_image, sz_ao, sz_ctr;

	// temp ptrs
	double *pAO;
	unsigned V3DLONG *pCTR0; // uInt32	red
	unsigned V3DLONG *pCTR1; //			green

	unsigned V3DLONG *pCTR_Out0, *pCTR_Out1; // ROI

	float *pAIx, *pAIy; // ai 0,1 : data; ai x,y : feedback of real physical positions
	double *pAIxy; // ai feedback
	double *pAI0, *pAI1; // ai hardware data

	float *pAOcx, *pAOcy; // for calibration
	bool b_scanner_correction;

	// compute FOV by volts
	double	min_x, max_x, min_y, max_y, min_z, max_z;
	double	step_x, step_y, step_z;

	double *xa, *ya, *za;
	double *xacc_forward, *xacc_backward;

	// configuration parameters
	int dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_red_ai, dev_red_ctr, red_n_ai, red_n_ctr, dev_green_ai, dev_green_ctr, green_n_ai, green_n_ctr,
		dev_clk, clk_n, dev_timing, timing_n, time_out, scanning_rate;
	double min_fovx, max_fovx, min_fovy, max_fovy, min_fovz, max_fovz;
	double ratio_x, ratio_y, ratio_z;
	V3DLONG sx, sy, sz;
	V3DLONG cr_x, cr_y, cr_z; //not used??
	V3DLONG start_x, start_y, end_x, end_y;
	double a_lsmbf, d_lsmbf, e_lsmbf, f_lsmbf; // LSM->BF factors
	double a_bflsm, d_bflsm, e_bflsm, f_bflsm; // BF->LSM factors
	int ch;
	int aictr_red;
	int aictr_green;
	int marker_sorted;
	double fov_step_x, fov_step_y, fov_step_z;
	V3DLONG sc;

	int scanning_offset_x; // the offset value in x direction during scanning, used in interactive ROI

	// reserve for non do_LSM_single() functions
	int aictr;
	int red_n, green_n, dev_red, dev_green;
	double *pAI;

	double duty_cycle;

    int scanMethod; // 0:single direction with acc, 1:zigzag_acc ZJL
	int b_useSimImg; // create simulation data for testing
	int b_scan_z_reverse; //0:z from 0 to max; 1:z from max to 0
	float *pImFloat_f; // for saving full image in landmarker imaging


	bool *flag; // ROI
	V3DLONG fl_count, fl_col;

    V3DLONG *real_to_exp_pos_LUT;
    V3DLONG *exp_to_real_pos_LUT;

	// paras for getting correction offset
    V3DLONG old_sz;
    double old_min_fovx, old_max_fovx, old_min_fovy, old_max_fovy, old_fov_step_x, old_fov_step_y;
	double real_min_fovx, real_max_fovx;
    V3DLONG cal_offset_left, cal_offset_right, offset_left, offset_right;
	V3DLONG real_bb_start_x, real_bb_end_x, real_sx; // for interactive ROI_i_x to recovering image


};


#endif
