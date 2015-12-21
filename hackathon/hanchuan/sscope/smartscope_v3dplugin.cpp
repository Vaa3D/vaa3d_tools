/* smartscope_v3dplugin.cpp
 * 2011-May-13: the V3D plugin menu for the controller program of the SmartScope. By Hanchuan Peng
 */

#include "smartscope_control.h"
 
#include "v3d_message.h"
 
using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(smartscope_controller, SSDriverPlugin);


//=================================================================================
//
// V3D menu for SmartScope controller
//
//=================================================================================

QString sscopetitle = "v0.132 2012-08-09";

QStringList SSDriverPlugin::menulist() const
{
    return QStringList() 
	<< tr("Setup parameters: laser scanning imaging")
	<< tr("Focus Control: Bright field")
	<< tr("Focus Control: LSM")		
	<< tr("Bright Field Acquisition: full image")		// bright field imaging
	<< tr("LSM Acquisition: full image")	
	<< tr("----------------------------------------")
	<< tr("FOV (BF/LSM) Calibration")
	<< tr("Map ROI in BF to LSM Display")
//	<< tr("Reset BF field")
	<< tr("----------------------------------------")
	<< tr("Acquisition: Interactive ROI")					// ROI imaging selected from 2d view
	<< tr("Acquisition: Interactive ROI from full X")		// ROI imaging selected from 2d view from full X
	<< tr("Acquisition: ROI from 3D Viewer")				// ROI imaging called from 3dviewer
	<< tr("Acquisition: full 2D image at z=30um") //
	<< tr("----------------------------------------")
	<< tr("Setup parameters: marker ablation")				//stimulating with landmark 
	<< tr("Marker Ablation")								// Stimulating with Landmarks
	<< tr("Marker Ablation from 3D Viewer")					// POINT_ABLATING called from 3dviewer
	<< tr("Marker Ablation (no Calibration)")				// Stimulating with Landmarks in 2d view without calibration
	<< tr("Marker Ablation from 3D Viewer (no Calibration)")// POINT_ABLATING called from 3dviewer without calibration
	<< tr("Marker Ablation (with feedback)")                // POINT_ABLATING called from 3dviewer without calibration  
	<< tr("Marker Ablation from 3D Viewer (with feedback)")  
	<< tr("Stimulation Laser Alignment")					// Stimulating laser alignment
	<< tr("----------------------------------------")
	<< tr("All Shutter On")								    // shutter control
	<< tr("All Shutter Off")
	<< tr("----------------------------------------")
	<< tr("Copy Current Image Config Info to Clipboard") // load config info
	<< tr("Display Current Image Config Info") // load config info
	<< tr("About")
	<< tr("----------------------------------------")
	//<< tr ("**** Un-used ****")
	//<< tr("----------------------------------------")
	//<< tr("Acquisition: Series of Markers")					// imaging with landmarkers
	//<< tr("Acquisition: Markers from 3D Viewer")            // landmarker imaging called from 3dviewer
	//<< tr("Acquisition: Adaptive ROI")					    // Adaptive ROI imaging from 2d view
	//<< tr("AO2AI Calibration")								// AO AI calibration
	//<< tr("----------------------------------------")
	//<< tr("AI2AO Calibration")
	//<< tr("Acquisition: full image: sequential")			// two-step
	////<< tr("Acquisition: full image with ROI (automated)")	// 
	//<< tr("----------------------------------------")
	//<< tr("Setup parameters: marker scan")					//imaging with landmark dialog 
	//<< tr("----------------------------------------")
	;
//	<< tr("----------------------------------------")
//	<< tr("test crash")
	
}

void SSDriverPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Focus Control: Bright field"))
	{
		ssd.bright_field_focuscontrols(callback, parent);
	} 
	if (menu_name == tr("Focus Control: LSM"))
	{
		ssd.laser_scanning_imaging_focuscontrol(callback, parent);
	} 
	else if (menu_name == tr("Bright Field Acquisition: full image"))
	{
		ssd.bright_field_imaging(callback, parent);
	}
	else if (menu_name == tr("Setup parameters: laser scanning imaging"))
	{
		ssd.setup_lsm_parameters();
	}
	else if (menu_name == tr("LSM Acquisition: full image"))
	{
		ssd.laser_scanning_imaging_entire_fov(callback, parent);
	}
	else if (menu_name == tr("FOV (BF/LSM) Calibration")) 
	{
		ssd.calibrate_BF_LSM(callback, parent);
	}
	else if (menu_name == tr("Map ROI in BF to LSM Display")) 
	{
		ssd.mapROI_BF_TO_LSM(callback, parent);
	}
	else if (menu_name == tr("Reset BF field")) 
	{
		ssd.reset_BF_field(callback, parent);
	}
	else if (menu_name == tr("Setup parameters: marker ablation"))
	{
		ssd.setpara_markerablation(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation"))
	{
		ssd.landmarker_ablating(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation from 3D Viewer"))
	{
		ssd.tdview_landmarker_ablating(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation (no Calibration)"))
	{
		ssd.landmarker_ablating_nocalib(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation from 3D Viewer (no Calibration)"))
	{
		ssd.tdview_landmarker_ablating_nocalib(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation (with feedback)")) //ablation with feedback
	{
		ssd.landmarker_ablating_with_feedback(callback, parent);
	}
	else if (menu_name == tr("Marker Ablation from 3D Viewer (with feedback)"))
	{
		ssd.tdview_landmarker_ablating_with_feedback(callback, parent);
	}
	else if (menu_name == tr("Stimulation Laser Alignment"))
	{
		ssd.stimulation_laser_alignment(callback, parent);
	}
	else if (menu_name == tr("AO2AI Calibration"))
	{
		ssd.calibrate_AO_AI(callback, true);
	}
	else if (menu_name == tr("AI2AO Calibration"))
	{
		v3d_msg("Pull back for new development. Not ready!"); return;
		ssd.calibrate_AO_AI(callback, false);
	}
	else if (menu_name == tr("Acquisition: Interactive ROI")) 
	{
		ssd.mi_imaging_withROI_i(callback, parent);
	}
	else if (menu_name == tr("Acquisition: Interactive ROI from full X"))
	{
		ssd.mi_imaging_withROI_i_x(callback, parent);
	}
	else if (menu_name == tr("Acquisition: Adaptive ROI"))
	{
		ssd.mi_imaging_withROI_adaptive(callback, parent);
	}
	else if (menu_name == tr("Acquisition: Series of Markers"))
	{
		ssd.landmarker_imaging(callback, parent);
	}
	else if (menu_name == tr("Acquisition: Markers from 3D Viewer"))
	{
		ssd.tdview_landmarker_imaging(callback, parent);
	}
	else if (menu_name == tr("Acquisition: ROI from 3D Viewer"))
	{
		ssd.tdviewroi_imaging(callback, parent);
	}
	else if (menu_name == tr("Copy Current Image Config Info to Clipboard"))
	{
		ssd.loading_imconfig(callback, parent);
	}
	else if (menu_name == tr("Display Current Image Config Info"))
	{
		ssd.display_imconfig(callback, parent);
	}
	else if (menu_name == tr("All Shutter On"))
	{
		ssd.shutterAllOnOff(true);
	}
	else if (menu_name == tr("All Shutter Off"))
	{
		ssd.shutterAllOnOff(false);
	}
	else if (menu_name == tr("Acquisition: full 2D image at z=30um"))
	{
		ssd.mi_imaging_oneslice_z30(callback, parent);
	}
	else if (menu_name == tr("About"))
	{
		v3d_msg((sscopetitle + ", SmartScope Controller (2010-2012) [build time = %1] developed by Hanchuan Peng Lab, Janelia Farm Research Campus").arg(current_time_stamp()));
	}
	//=============================================================================================================
	else if (menu_name == tr("Acquisition: full image: sequential"))
	{
		ssd.microscopeimaging2step(callback, parent);
	}
	else if (menu_name == tr("Setup parameters: marker scan"))
	{
		v3d_msg("Pull back for new development. Not ready!"); return;
		//
	}
	else if (menu_name == tr("test crash"))
	{
		for (V3DLONG  i=0; i<10000; i++)
		{
			ssd.bright_field_imaging(callback, parent);
			ssd.laser_scanning_imaging_entire_fov(callback, parent);
			printf("now at test loop %ld\n", i); 
		}
	}
}
