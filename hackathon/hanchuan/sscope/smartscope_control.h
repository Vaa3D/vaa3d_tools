/* smartscope_control.h
 * 2011-May-13: The header for the SmartScope controller, by Hanchuan Peng
 */


#ifndef __SMARTSCOPECONTROLLER_H__
#define __SMARTSCOPECONTROLLER_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "v3d_interface.h"
#include "smartscope_lsm.h"

class SSDriver
{
public:
	void setup_lsm_parameters();
	void setpara_markerablation(V3DPluginCallback &callback, QWidget *parent); // setting paras for marker ablation

	void laser_scanning_imaging_entire_fov(V3DPluginCallback &callback, QWidget *parent); // simultaneous imaging with both color
	void bright_field_imaging(V3DPluginCallback &callback, QWidget *parent); // object finding bright field imaging

	void stimulation_laser_alignment(V3DPluginCallback &callback, QWidget *parent); // stimulation laser alignment

	void laser_scanning_imaging_focuscontrol(V3DPluginCallback &callback, QWidget *parent); // focus control of lsm
	void bright_field_focuscontrols(V3DPluginCallback &callback, QWidget *parent); // BF fc

	void calibrate_BF_LSM(V3DPluginCallback &callback, QWidget *parent); // Bright field and fluoresence imaging calibration
	void reset_BF_field(V3DPluginCallback &callback, QWidget *parent); // reset BF field to its original value
	void calibrate_AO_AI(V3DPluginCallback &callback, bool flag_ao2ai); // AO and AI calibration

	// ablation
	void landmarker_ablating(V3DPluginCallback &callback, QWidget *parent); 
	void tdview_landmarker_ablating(V3DPluginCallback &callback, QWidget *parent); // markers ablation from 3D view window
	void landmarker_ablating_nocalib(V3DPluginCallback &callback, QWidget *parent); // ablation without calibration
	void tdview_landmarker_ablating_nocalib(V3DPluginCallback &callback, QWidget *parent); // markers ablation from 3D view window without calib
	void landmarker_ablating_with_feedback(V3DPluginCallback &callback, QWidget *parent);
	void tdview_landmarker_ablating_with_feedback(V3DPluginCallback &callback, QWidget *parent);

	void landmarker_imaging(V3DPluginCallback &callback, QWidget *parent); // imaging on landmarkers
	void mi_imaging_withROI_i(V3DPluginCallback &callback, QWidget *parent); //interactive
	void mi_imaging_withROI_i_x(V3DPluginCallback &callback, QWidget *parent); //interactive ROI from full X
	void mi_imaging_withROI_adaptive(V3DPluginCallback &callback, QWidget *parent); //adaptive ROI
	void tdviewroi_imaging(V3DPluginCallback &callback, QWidget *parent);   // ROI Imaging selected from 3d view
	void tdview_landmarker_imaging(V3DPluginCallback &callback, QWidget *parent); // markers imaging from 3D view window

	void loading_imconfig(V3DPluginCallback &callback, QWidget *parent);  // loading config info for current image
	void display_imconfig(V3DPluginCallback &callback, QWidget *parent); // display config info for current image

	void printImg2Txt(QString filename, V3DLONG sx, V3DLONG sy, float *pAIx, float *pAOx); // print img data to txt file for analysis, used in AO2AI calibration
	void flip_BF_Y_Axis(V3DLONG dimx, V3DLONG dimy, unsigned char *pImage);    // flip BF image in Y direction for calibration purpose

	void shutterAllOnOff(char onoff);
	void mi_imaging_oneslice_z30(V3DPluginCallback &callback, QWidget *parent); //one slice at z=30um

	void mapROI_BF_TO_LSM(V3DPluginCallback &callback, QWidget *parent); // map ROI in BF to LSM


	//void mi_imaging_withROI_a(V3DPluginCallback &callback, QWidget *parent); // automated
	void microscopeimaging2step(V3DPluginCallback &callback, QWidget *parent);
    void saveCorrectedImage (V3DPluginCallback &callback, QString &filename_img_saved, LaserScanImaging * myimg);
	void saveCorrectedImage_roi (V3DPluginCallback &callback, QString &filename_img_saved, LaserScanImaging * myimg);
	void saveAIx2Txt(QString filename, V3DLONG sx, V3DLONG sy, V3DLONG sz, float *pAIx);

private: //
	float *pCalibrateAO2AI; // global LUT AO2AI
	float *pCalibrateAI2AO; // global LUT AI2AO

	V3DLONG dimx, dimy;
	bool bFlip_BF; // flip BF image in Y direction

public:
	void cleanData()
	{
		if (pCalibrateAO2AI) {delete []pCalibrateAO2AI; pCalibrateAO2AI=0;}
		if (pCalibrateAI2AO) {delete []pCalibrateAI2AO; pCalibrateAI2AO=0;}
	}
	SSDriver() {pCalibrateAO2AI = pCalibrateAI2AO = 0; bFlip_BF=false;}
	~SSDriver() {cleanData();}
};

class SSDriverPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

private:
	SSDriver ssd;
};

#endif

