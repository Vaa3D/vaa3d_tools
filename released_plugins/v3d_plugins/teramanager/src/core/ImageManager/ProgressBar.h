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
#include <string.h>
#include "IM_config.h"

class iim::imProgressBar
{
	private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static imProgressBar* uniqueInstance;
        imProgressBar();

        char message_level_1[1000];     // main operation
        char message_level_2[1000];     // sub-operation
        char message_level_3[1000];     // sub-sub-operation
        float progress_value;
        double proctime;
        int minutes_remaining;
        int seconds_remaining;

	public:

        /**********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        ***********************************************************************************/
        static imProgressBar* instance();
        static imProgressBar* getInstance()
        {
            if(uniqueInstance)
                return uniqueInstance;
            else
                return instance();
        }
        ~imProgressBar(){}


        void start(const char* new_operation_desc, bool toConverter = true);
		void update(float new_progress_value, const char* new_progress_info);
		void updateInfo(const char* new_progress_info);
        void setMessage(int level, const char* message);
        void show(bool toConverter = true);
        void reset();
};

#endif
