//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include "iomanager.config.h"
#include "IOPluginAPI.h"

/******************
*    CHANGELOG    *
*******************
* 2017-04-01. Giulio. @ADDED global variable CHANS_no to deal with the case of more than three channels
* 2014-11-25. Giulio. @CHANGED default plugins are set to "empty" because they have to be set by the application
*/

// initialize namespace parameters
namespace iomanager
{
    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    std::string VERSION = "1.1.1";          // version of current module
    int DEBUG = NO_DEBUG;					// debug level
    bool TIME_CALC = true;					// whether to enable time measurements
    channel CHANS = ALL;					// channel to be loaded (default is ALL)
    int CHANS_no = -1;					    // alternative way to specify a channel to be loaded (must be invalid if CHANS <> NONE, invalid value = -1)
	std::string IMIN_PLUGIN  = "empty";		// plugin to manage input image format 
	std::string IMIN_PLUGIN_PARAMS="";		// additional parameters <param1=val,param2=val,...> to the plugin for image input 
	std::string IMOUT_PLUGIN = "empty";	// plugin to manage output image format (WARNING: must be a 2D pluging to output test image until 3D plugins do not have a write operation)
	std::string IMOUT_PLUGIN_PARAMS="";		// additional parameters <param1=val,param2=val,...> to the plugin for image output 
    /*-------------------------------------------------------------------------------------------------------------------------*/
}
