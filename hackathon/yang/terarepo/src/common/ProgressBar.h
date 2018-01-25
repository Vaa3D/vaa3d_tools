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
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#ifndef _IIM_PROGRESS_BAR_H
#define _IIM_PROGRESS_BAR_H

#include <iostream>
#include <string>
#include "config.h"

namespace terastitcher
{
	class ProgressBar
	{
		private:

			// read-write members
			std::string op_info;		// main operation information. An operation consists of one (or more) phases
			std::string phase_info;		// phase information
			float progress_value;		// progress value in [0,1], 0=no progress, 1=completed
			bool toGUI;					// true = display to GUI, false = display to command line (default)

			// read-only (internal) members
			double proctime;
			int minutes_remaining;
			int seconds_remaining;

			// disable default constructor
			ProgressBar();

		public:

			/**********************************************************************************
			* Singleton design pattern: this class can have one instance only,  which must be
			* instantiated by calling static method "instance(...)"
			***********************************************************************************/
			static ProgressBar* instance();
			static ProgressBar* getInstance();
			~ProgressBar(){}


			void start(const std::string & new_operation_desc);
			void setProgressValue(float new_progress_value, std::string new_phase_info);
			void setProgressInfo(std::string new_phase_info);
			void display();
			void resetMembers();
			void setToGUI(bool _toGUI){toGUI = _toGUI;}
	};
}

#endif
