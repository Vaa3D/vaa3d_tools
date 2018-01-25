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

#include "ProgressBar.h"
#include "QProgressSender.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>
#include "IM_config.h"

ts::ProgressBar::ProgressBar() { resetMembers(); }

/**********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "instance(...)"
***********************************************************************************/
ts::ProgressBar* ts::ProgressBar::instance()
{
	static ProgressBar* uniqueInstance = new ProgressBar();
	return uniqueInstance;
}
ts::ProgressBar* ts::ProgressBar::getInstance()
{
	return instance();
}

void ts::ProgressBar::resetMembers()
{
    if(!op_info.empty())
		op_info.clear();
	if(!phase_info.empty())
		phase_info.clear();
    progress_value=0;
    proctime = 0;
    minutes_remaining = 0;
    seconds_remaining = 0;
	toGUI = false;
}


void ts::ProgressBar::start(const std::string & new_operation_desc)
{
    op_info = new_operation_desc;
    progress_value=0;
    proctime = -TIME(0);
    minutes_remaining = 0;
    seconds_remaining = 0;

    #ifdef WITH_QT
	if(toGUI)
	{
		int progress_value_int = (int) progress_value;
		QProgressSender::instance()->sendProgressBarChanged(progress_value_int, 0, 0, op_info);
	}
    #endif
}

void ts::ProgressBar::setProgressValue(float new_progress_value, std::string new_phase_info)
{
    progress_value=new_progress_value;
    phase_info = new_phase_info;

    if(new_progress_value!=0)
    {
        minutes_remaining = (int)((proctime + TIME(0))*(100.0-progress_value)/(progress_value*60.0));
        seconds_remaining = (int)((proctime + TIME(0))*(100.0-progress_value)/(progress_value));
    }
}


void ts::ProgressBar::setProgressInfo(std::string new_phase_info)
{
    phase_info = new_phase_info;
}

void ts::ProgressBar::display()
{   
	if(toGUI)
	{
#ifdef WITH_QT
		int progress_value_int = (int) (progress_value);
		if(op_info.empty())
			QProgressSender::instance()->sendProgressBarChanged(progress_value_int, minutes_remaining, seconds_remaining%60, phase_info);   
		else
			QProgressSender::instance()->sendProgressBarChanged(progress_value_int, minutes_remaining, seconds_remaining%60, (op_info + ": " + phase_info)); 
#endif
	}
	if(!toGUI)
	{
		int dummy = system_CLEAR();
		printf("OPERATION:\t%s\n",op_info.c_str()); // 140427_IANNELLO
		printf("PHASE:\t\t%s\n",phase_info.c_str()); // 140427_IANNELLO
		printf("TIME REMAINING:\t%d minutes and %d seconds\n", minutes_remaining, seconds_remaining%60);
		printf("PROGRESS:\t");
		printf("%d%%\t",(int)(progress_value));
		for(int i=1; i<=progress_value; i++)
				printf("*");
		for(int i=(int)progress_value; i<100; i++)
				printf(":");
		printf("\n\n");
	}
}
