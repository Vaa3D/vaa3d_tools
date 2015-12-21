//fileio for the LSM configuration parameters
// by Hanchuan Peng
// 20110516

#ifndef __CONFIGFILEIO_LSM_H__
#define __CONFIGFILEIO_LSM_H__

#include <QtGui>
#include <QString>
#include <stdio.h>

#include "v3d_interface.h"
#include "v3d_message.h"

#define SS_DEVICE_X 1               //NI PCI 6115
#define SS_DEVICE_Y 1               //NI PCI 6115
#define SS_DEVICE_Z 3               //NI USB 6229
#define SS_AO_X 0                   //DEVICE_X'S AO0
#define SS_AO_Y 1                   //DEVICE_Y's AO1
#define	SS_AO_Z 2                   //DEVICE_Z's AO2

#define SS_AI_OR_COUNTER 0          //1 for AI and 0 for counter
//AI ports of NI PCI 6115 are reserved for future analog PMTs
//COUNTER channels of NI PCI 6115 are used right now for counter PMTs

#define SS_DEVICE_RED_SHUTTER 2     //for red (561nm) laser shutter
#define SS_PORTLINE_RED_SHUTTER 0   //

#define SS_DEVICE_GREEN_SHUTTER 2   //for green (488nm) laser shutter
#define SS_PORTLINE_GREEN_SHUTTER 1 //

#define SS_DEVICE_LSMDETECTION_SHUTTER   2
#define SS_PORTLINE_LSMDETECTION_SHUTTER 6

#define SS_DEVICE_STIMULATION_SHUTTER 2
#define SS_PORTLINE_STIMULATION_SHUTTER 2

#define SS_DEVICE_BRIGHTFIELD_SHUTTER 2
#define SS_PORTLINE_BRIGHTFIELD_SHUTTER 7

#define SS_DEVICE_LSM_TIMING 1   //NI PCI 6115 
#define SS_CHANNEL_LSM_TIMING 7  //PFI (programmable ports)??

#define SS_DEVICE_CLOCK  3         //
#define SS_COUNTERCHANNEL_CLOCK  0 //


class Parameters_LSM
{
public:
	int DEVICE_X, //device num
		DEVICE_Y,
		DEVICE_Z,
		AO_X, //port
		AO_Y,
		AO_Z,
		AI_COUNTER_RED, //bool to determine use ai or counter for red channel
		AI_COUNTER_GREEN, //bool to determine use ai or counter for green channel
		DEVICE_RED_AI,
		DEVICE_RED_CTR,
		PORT_RED_AI,
		PORT_RED_CTR,
		DEVICE_GREEN_AI,
		DEVICE_GREEN_CTR,
		PORT_GREEN_AI,
		PORT_GREEN_CTR,
		DEVICE_TIMING,
		PORT_TIMING,
		DEVICE_CLOCK,  
		PORT_CLOCK,
		SCAN_Z_REVERSE, // 1: Reverse z direction
		SCAN_METHOD,
		USE_SIMIMG; //bool to determine use sim or not

	double FOVX_MIN, //field of view in physical coord
		FOVX_MAX,
		FOVX_STEP,
		FOVY_MIN,
		FOVY_MAX,
		FOVY_STEP,
		FOVZ_MIN,
		FOVZ_MAX,
		FOVZ_STEP,
		RATIO_X,
		RATIO_Y,
		RATIO_Z,
		TIMEOUT,
		SCANNING_RATE;

	V3DLONG SX, //# of pxiels of each dim
		SY,
		SZ,
		STARTX, //blue frame
		STARTY,
		ENDX,
		ENDY;
	double BF_TO_LSM_A,
		BF_TO_LSM_D,
		BF_TO_LSM_E,
		BF_TO_LSM_F,
		LSM_TO_BF_A,
		LSM_TO_BF_D,
		LSM_TO_BF_E,
		LSM_TO_BF_F;

	int	CHANNEL, //red, green or both
		MARKER_SORTED;

	double DUTY_CYCLE; 

public:
	Parameters_LSM()
	{
		DEVICE_X = 
		DEVICE_Y = 
		DEVICE_Z = 
		AO_X = 
		AO_Y = 
		AO_Z =
		AI_COUNTER_RED =
		AI_COUNTER_GREEN =
		DEVICE_RED_AI = 
		DEVICE_RED_CTR = 
		PORT_RED_AI = 
		PORT_RED_CTR =
		DEVICE_GREEN_AI = 
		DEVICE_GREEN_CTR = 
		PORT_GREEN_AI = 
		PORT_GREEN_CTR = 
		DEVICE_TIMING = 
		PORT_TIMING = 
		DEVICE_CLOCK =  
		PORT_CLOCK = 
		SCAN_METHOD =
		USE_SIMIMG = 
		SCAN_Z_REVERSE = -1;

		FOVX_MIN = 
		FOVX_MAX = 
		FOVX_STEP = 
		FOVY_MIN = 
		FOVY_MAX =
		FOVY_STEP =
		FOVZ_MIN =
		FOVZ_MAX =
		FOVZ_STEP =
		RATIO_X = 
		RATIO_Y = 
		RATIO_Z = 
		TIMEOUT =
		SCANNING_RATE = -9999;
	
		SX = 
		SY = 
		SZ = 
		STARTX = 
		STARTY = 
		ENDX = 
		ENDY = 
		CHANNEL = 
		MARKER_SORTED = -1;
		BF_TO_LSM_A = 
		BF_TO_LSM_D =
		BF_TO_LSM_E =
		BF_TO_LSM_F =
		LSM_TO_BF_A =
		LSM_TO_BF_D =
		LSM_TO_BF_E =
		LSM_TO_BF_F = 0;

		DUTY_CYCLE = 0.5;
	}
	bool isValid()
	{
		return (DEVICE_X == -1 || 
			DEVICE_Y == -1 ||
			DEVICE_Z == -1 ||
			AO_X == -1 || 
			AO_Y == -1 ||
			AO_Z == -1 ||
			AI_COUNTER_RED == -1 ||
			AI_COUNTER_GREEN == -1 ||
			DEVICE_RED_AI == -1 || 
			DEVICE_RED_CTR == -1 || 
			PORT_RED_AI == -1 || 
			PORT_RED_CTR == -1 || 
			DEVICE_GREEN_AI == -1 || 
			DEVICE_GREEN_CTR == -1 || 
			PORT_GREEN_AI == -1 ||
			PORT_GREEN_CTR == -1 ||
			DEVICE_TIMING == -1 || 
			PORT_TIMING == -1 ||
			DEVICE_CLOCK == -1 || 
			PORT_CLOCK == -1 ||
			FOVX_MIN == -9999 ||
			FOVX_MAX == -9999 || 
			FOVX_STEP == -9999 || 
			FOVY_MIN == -9999 || 
			FOVY_MAX == -9999 ||
			FOVY_STEP == -9999 ||
			FOVZ_MIN == -9999 ||
			FOVZ_MAX == -9999 ||
			FOVZ_STEP == -9999 ||
			RATIO_X == -9999 || 
			RATIO_Y == -9999 || 
			RATIO_Z == -9999 ||
			TIMEOUT == -9999 ||
			SCANNING_RATE == -9999 || 
			SX == (V3DLONG)-1 || 
			SY == (V3DLONG)-1 ||  
			SZ == (V3DLONG)-1 || 
			//STARTX == (V3DLONG)-1 || 
			//STARTY == (V3DLONG)-1 ||  
			//ENDX == (V3DLONG)-1 || 
			//ENDY == (V3DLONG)-1 ||  
			//BF_TO_LSM_A == -9999 || 
			//BF_TO_LSM_D == -9999 ||
			//BF_TO_LSM_E == -9999 ||
			//BF_TO_LSM_F == -9999 ||
			//LSM_TO_BF_A == -9999 ||
			//LSM_TO_BF_D == -9999 ||
			//LSM_TO_BF_E == -9999 ||
			//LSM_TO_BF_F == -9999 || 
			CHANNEL == (V3DLONG)-1 || 
			MARKER_SORTED == (V3DLONG)-1 ||
			SCAN_METHOD == -1 ||
			USE_SIMIMG == -1 ||
			SCAN_Z_REVERSE == -1) ? false : true;
	}

private:
	bool importKeywordString2ParameterValues(QString ss, QString vv);

public:
	bool load_ParameterFile(QString openFileNameLabel);
	bool save_ParameterFile(QString openFileNameLabel);
};


//===========================================================================
// for Parameters_ROI_LSM
//===========================================================================
class Parameters_ROI_LSM
{
public:
	int DEVICE_X, //device num
		DEVICE_Y,
		DEVICE_Z,
		AO_X, //port
		AO_Y,
		AO_Z,
		AI_COUNTER_RED, //bool to determine use ai or counter for red channel
		AI_COUNTER_GREEN, //bool to determine use ai or counter for green channel
		DEVICE_RED_AI,
		DEVICE_RED_CTR,
		PORT_RED_AI,
		PORT_RED_CTR,
		DEVICE_GREEN_AI,
		DEVICE_GREEN_CTR,
		PORT_GREEN_AI,
		PORT_GREEN_CTR,
		DEVICE_TIMING,
		PORT_TIMING,
		DEVICE_CLOCK,  
		PORT_CLOCK,
		SCAN_Z_REVERSE,
		SCAN_METHOD,
		USE_SIMIMG; //bool to determine use sim or not
//		USE_X_FULLSCAN;

	double FOVX_MIN, //field of view in physical coord
		FOVX_MAX,
		FOVX_STEP,
		FOVY_MIN,
		FOVY_MAX,
		FOVY_STEP,
		FOVZ_MIN,
		FOVZ_MAX,
		FOVZ_STEP,
		RATIO_X,
		RATIO_Y,
		RATIO_Z,
		TIMEOUT,
		SCANNING_RATE,
		REZ_X,
		REZ_Y,
		REZ_Z,
		ORIGIN_X,
		ORIGIN_Y,
		ORIGIN_Z;

	V3DLONG SX, //# of pxiels of each dim
		SY,
		SZ,
		STARTX, //blue frame
		STARTY,
		ENDX,
		ENDY;
	double BF_TO_LSM_A,
		BF_TO_LSM_D,
		BF_TO_LSM_E,
		BF_TO_LSM_F,
		LSM_TO_BF_A,
		LSM_TO_BF_D,
		LSM_TO_BF_E,
		LSM_TO_BF_F;

	int	CHANNEL, //red, green or both
		MARKER_SORTED;

	double DUTY_CYCLE; 

public:
	Parameters_ROI_LSM()
	{
		DEVICE_X = 
		DEVICE_Y = 
		DEVICE_Z = 
		AO_X = 
		AO_Y = 
		AO_Z =
		AI_COUNTER_RED =
		AI_COUNTER_GREEN =
		DEVICE_RED_AI = 
		DEVICE_RED_CTR = 
		PORT_RED_AI = 
		PORT_RED_CTR = 
		DEVICE_GREEN_AI = 
		DEVICE_GREEN_CTR = 
		PORT_GREEN_AI = 
		PORT_GREEN_CTR = 
		DEVICE_TIMING = 
		PORT_TIMING = 
		DEVICE_CLOCK =  
		PORT_CLOCK = 
		SCAN_METHOD =
		USE_SIMIMG = 
		SCAN_Z_REVERSE = -1;
//		USE_X_FULLSCAN = -1;

		FOVX_MIN = 
		FOVX_MAX = 
		FOVX_STEP = 
		FOVY_MIN = 
		FOVY_MAX =
		FOVY_STEP =
		FOVZ_MIN =
		FOVZ_MAX =
		FOVZ_STEP =
		RATIO_X = 
		RATIO_Y = 
		RATIO_Z = 
		TIMEOUT =
		SCANNING_RATE =
		REZ_X =
		REZ_Y =
		REZ_Z =
		ORIGIN_X =
		ORIGIN_Y =
		ORIGIN_Z = -9999;
	
		SX = 
		SY = 
		SZ = 
		STARTX = 
		STARTY = 
		ENDX = 
		ENDY = 
		CHANNEL = 
		MARKER_SORTED = -1;
		BF_TO_LSM_A = 
		BF_TO_LSM_D =
		BF_TO_LSM_E =
		BF_TO_LSM_F =
		LSM_TO_BF_A =
		LSM_TO_BF_D =
		LSM_TO_BF_E =
		LSM_TO_BF_F = -9999;

		DUTY_CYCLE = 0.5;
	}
	bool isValid()
	{
		return (DEVICE_X == -1 || 
			DEVICE_Y == -1 ||
			DEVICE_Z == -1 ||
			AO_X == -1 || 
			AO_Y == -1 ||
			AO_Z == -1 ||
			AI_COUNTER_RED == -1 ||
			AI_COUNTER_GREEN == -1 ||
			DEVICE_RED_AI == -1 || 
			DEVICE_RED_CTR == -1 || 
			PORT_RED_AI == -1 || 
			PORT_RED_CTR == -1 || 
			DEVICE_GREEN_AI == -1 || 
			DEVICE_GREEN_CTR == -1 || 
			PORT_GREEN_AI == -1 ||
			PORT_GREEN_CTR == -1 ||
			DEVICE_TIMING == -1 || 
			PORT_TIMING == -1 ||
			DEVICE_CLOCK == -1 || 
			PORT_CLOCK == -1 ||
			FOVX_MIN == -9999 ||
			FOVX_MAX == -9999 || 
			FOVX_STEP == -9999 || 
			FOVY_MIN == -9999 || 
			FOVY_MAX == -9999 ||
			FOVY_STEP == -9999 ||
			FOVZ_MIN == -9999 ||
			FOVZ_MAX == -9999 ||
			FOVZ_STEP == -9999 ||
			RATIO_X == -9999 || 
			RATIO_Y == -9999 || 
			RATIO_Z == -9999 ||
			TIMEOUT == -9999 ||
			SCANNING_RATE == -9999 || 
			REZ_X == -9999 || 
			REZ_Y == -9999 || 
			REZ_Z == -9999 || 
			ORIGIN_X == -9999 || 
			ORIGIN_Y == -9999 || 
			ORIGIN_Z == -9999 || 
			SX == (V3DLONG)-1 || 
			SY == (V3DLONG)-1 ||  
			SZ == (V3DLONG)-1 || 
			STARTX == (V3DLONG)-1 || 
			STARTY == (V3DLONG)-1 ||  
			ENDX == (V3DLONG)-1 || 
			ENDY == (V3DLONG)-1 ||  
			BF_TO_LSM_A == -9999 || 
			BF_TO_LSM_D == -9999 ||
			BF_TO_LSM_E == -9999 ||
			BF_TO_LSM_F == -9999 ||
			LSM_TO_BF_A == -9999 ||
			LSM_TO_BF_D == -9999 ||
			LSM_TO_BF_E == -9999 ||
			LSM_TO_BF_F == -9999 || 
			CHANNEL == (V3DLONG)-1 || 
			MARKER_SORTED == (V3DLONG)-1 ||
			SCAN_METHOD == -1 ||
			USE_SIMIMG == -1 ||
//			USE_X_FULLSCAN == -1 ||
			SCAN_Z_REVERSE == -1) ? false : true;
	}

private:
	bool importKeywordString2ParameterValues(QString ss, QString vv);

public:
	bool load_ParameterFile(QString openFileNameLabel);
	bool save_ParameterFile(QString openFileNameLabel);
};

//===========================================================================

class LSM_Setup_Dialog : public QDialog
{
	Q_OBJECT

public:
	int dev_x, dev_y, dev_z, 
		ao_x,  ao_y,  ao_z, 
		dev_red_ai, dev_red_ctr, red_n_ai,red_n_ctr, //red_n 
		dev_green_ai, dev_green_ctr, green_n_ai, green_n_ctr, //green_n 
		dev_clk, clk_n, 
		dev_timing, timing_n, 
		time_out, scanning_rate;
	double ratio_x, ratio_y, ratio_z;
	double fov_min_x, fov_max_x, 
		fov_min_y, fov_max_y, 
		fov_min_z, fov_max_z;
	V3DLONG dims_x, dims_y, dims_z;
	int ch; // 0 green; 1 red; 2 both

	//int aictr; // 0 ai; 1 ctr
	
	int aictr_green;// 0 ai; 1 ctr
	int aictr_red;  // 0 ai; 1 ctr

	int marker_sorted; // 0 without sorting; 1 sorting 

	double fov_step_x, fov_step_y, fov_step_z;

	int scanMethod; // 0:single direction with acc, 1:zigzag_acc ZJL
	int b_useSimImg; // create simulation data for testing: 0 not use; 1 use 
	int b_scan_z_reverse; // 0: z from 0 to max; 1: z from max to 0

	bool b_resetInProgress; // used to control reset display

	QString dir;
	QString qstr_aictr; //TOBE_DELETED

	QString qstr_aictr_red;
	QString qstr_aictr_green;

	QGridLayout *gridLayout;

	// setting device configurations
	QLabel *label_dev_conf;
	QLabel *label_img_conf;

	QLabel *label_xdev;
	QLabel *label_ydev;
	QLabel *label_zdev;

	QLabel *label_xao;
	QLabel *label_yao;
	QLabel *label_zao;

	QSpinBox *qsb_dev_x;
	QSpinBox *qsb_dev_y;
	QSpinBox *qsb_dev_z;

	QSpinBox *qsb_ao_x;
	QSpinBox *qsb_ao_y;
	QSpinBox *qsb_ao_z;

	QLabel *label_reddev;
	QLabel *label_greendev;
	QLabel *label_timingdev;
	QLabel *label_clkdev;

	QLabel *label_red;
	QLabel *label_green;
	QLabel *label_timing;
	QLabel *label_clk;

	QSpinBox *qsb_dev_red;
	QSpinBox *qsb_dev_green;
	QSpinBox *qsb_dev_timing;
	QSpinBox *qsb_dev_clk;

	QSpinBox *qsb_red;
	QSpinBox *qsb_green;
	QSpinBox *qsb_timing;
	QSpinBox *qsb_clk;

	// setting imaging parameters
	QLabel *label_fov_min_x;
	QLabel *label_fov_max_x;
	QLabel *label_fov_min_y;
	QLabel *label_fov_max_y;
	QLabel *label_fov_min_z;
	QLabel *label_fov_max_z;

	QLabel *label_dim_x;
	QLabel *label_dim_y;
	QLabel *label_dim_z;

	QDoubleSpinBox *qsb_fov_min_x;
	QDoubleSpinBox *qsb_fov_max_x;
	QDoubleSpinBox *qsb_fov_min_y;
	QDoubleSpinBox *qsb_fov_max_y;
	QDoubleSpinBox *qsb_fov_min_z;
	QDoubleSpinBox *qsb_fov_max_z;

	QLabel *label_fov_step_x;
	QLabel *label_fov_step_y;
	QLabel *label_fov_step_z;

	QDoubleSpinBox *qsb_fov_step_x;
	QDoubleSpinBox *qsb_fov_step_y;
	QDoubleSpinBox *qsb_fov_step_z;

	QLabel *label_ratio_x;
	QLabel *label_ratio_y;
	QLabel *label_ratio_z;
	QDoubleSpinBox *qsb_ratio_x;
	QDoubleSpinBox *qsb_ratio_y;
	QDoubleSpinBox *qsb_ratio_z;

	QLabel *label_timeout;
	QDoubleSpinBox *qsb_timeout;
	QLabel *label_scanrate;
	QDoubleSpinBox *qsb_scanrate;

	QSpinBox *qsb_dim_x;
	QSpinBox *qsb_dim_y;
	QSpinBox *qsb_dim_z;

	QLabel *label_log_dir;
	QLineEdit *edit_log_dir;
	QPushButton *pb_browse;

	QLabel *label_image_size;
	QLabel *label_total_time_num;

	// whether use simulation data ZJL
	QLabel *label_use_sim_img;
	QCheckBox *check_use_sim_img;
	// scanning method ZJL
	QLabel *label_scan_method;
	QComboBox *combo_scan_method;

	//TOBE_DELETED
	QLabel *label_aictr;
	QComboBox* combo_aictr;

	// for ai_ctr selection 
	QLabel *label_aictr_green;
	QComboBox* combo_aictr_green;

	QLabel *label_aictr_red;
	QComboBox* combo_aictr_red;

	QLabel *label_ch;
	QComboBox* combo_channel;

	QLabel *label_sort;
	QComboBox* combo_sort;

	QCheckBox* check_stepsize_or_dims; // balancing the setting dims and steps

	QCheckBox *check_scan_z_reverse; // reverse scan z direction

	QPushButton* reset; // reset image dimension, resolution, etc to default.
	QPushButton* paste; // paste image configuration info

public:
	LSM_Setup_Dialog(Parameters_LSM *pl, bool b_initnull);
	~LSM_Setup_Dialog();
	void copyDataOut(Parameters_LSM *pl);

public slots:
	void update();
	void update_dev_aictr();
	void update_step_x();
	void update_step_y();
	void update_step_z();
	void update_dims_x();
	void update_dims_y();
	void update_dims_z();
	void get_dir();
	void resetFOVDIM();
	void pasteConfigInfo();
};



class LSM_ROI_Setup_Dialog : public QDialog
{
	Q_OBJECT

public:
	int time_out, scanning_rate;
	double ratio_x, ratio_y, ratio_z;
	double fov_min_x, fov_max_x, 
		fov_min_y, fov_max_y, 
		fov_min_z, fov_max_z;
	V3DLONG dims_x, dims_y, dims_z;
	int ch; // 0 green; 1 red; 2 both
	int marker_sorted; // 0 without sorting; 1 sorting 

	double fov_step_x, fov_step_y, fov_step_z;

	int scanMethod; // 0:single direction with acc, 1:zigzag_acc ZJL
	int b_useSimImg; // create simulation data for testing: 0 not use; 1 use 

	int b_scan_z_reverse;

	bool b_resetInProgress; // used to control reset display

	QGridLayout *gridLayout;

	// setting imaging parameters
	QLabel *label_fov_min_x;
	QLabel *label_fov_max_x;
	QLabel *label_fov_min_y;
	QLabel *label_fov_max_y;
	QLabel *label_fov_min_z;
	QLabel *label_fov_max_z;

	QLabel *label_dim_x;
	QLabel *label_dim_y;
	QLabel *label_dim_z;

	QDoubleSpinBox *qsb_fov_min_x;
	QDoubleSpinBox *qsb_fov_max_x;
	QDoubleSpinBox *qsb_fov_min_y;
	QDoubleSpinBox *qsb_fov_max_y;
	QDoubleSpinBox *qsb_fov_min_z;
	QDoubleSpinBox *qsb_fov_max_z;

	QLabel *label_fov_step_x;
	QLabel *label_fov_step_y;
	QLabel *label_fov_step_z;

	QDoubleSpinBox *qsb_fov_step_x;
	QDoubleSpinBox *qsb_fov_step_y;
	QDoubleSpinBox *qsb_fov_step_z;

	QLabel *label_ratio_x;
	QLabel *label_ratio_y;
	QLabel *label_ratio_z;
	QDoubleSpinBox *qsb_ratio_x;
	QDoubleSpinBox *qsb_ratio_y;
	QDoubleSpinBox *qsb_ratio_z;

	QLabel *label_timeout;
	QDoubleSpinBox *qsb_timeout;
	QLabel *label_scanrate;
	QDoubleSpinBox *qsb_scanrate;

	QSpinBox *qsb_dim_x;
	QSpinBox *qsb_dim_y;
	QSpinBox *qsb_dim_z;

	QLabel *label_image_size;
	QLabel *label_total_time_num;

	// whether use simulation data ZJL
	QLabel *label_use_sim_img;
	QCheckBox *check_use_sim_img;
	// scanning method ZJL
	QLabel *label_scan_method;
	QComboBox *combo_scan_method;

	QLabel *label_ch;
	QComboBox* combo_channel;

	QCheckBox* check_stepsize_or_dims; // balancing the setting dims and steps

	QCheckBox *check_scan_z_reverse; // reverse scan z direction

	QCheckBox *check_use_x_fullscan; // use x full scan in roi

	QPushButton* reset; // reset image dimension, resolution, etc to default.
	QPushButton* paste; // paste image configuration info

	QPushButton* preset_paras; // preset paras for z, rezx,rezy,rezz
	QMenu menu_preset_paras;

public:
	LSM_ROI_Setup_Dialog(Parameters_LSM *pl, bool b_initnull);
	~LSM_ROI_Setup_Dialog();
	void copyDataOut(Parameters_LSM *pl);
	void createMenuOfPresetParas();
public slots:
	void update();
	void update_step_x();
	void update_step_y();
	void update_step_z();
	void update_dims_x();
	void update_dims_y();
	void update_dims_z();
	void pasteConfigInfo();
	void doMenuOfPrestParas();
	void preset_paras_0(); // scan one slice setting
	void preset_paras_1();
	void preset_paras_2();
	void preset_paras_3();
	void preset_paras_4();
	void preset_paras_5();
	void preset_paras_6(); // use the setting of the last scan
};

#endif
