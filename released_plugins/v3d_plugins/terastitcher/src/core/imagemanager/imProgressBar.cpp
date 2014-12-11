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
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include "PConverter.h"
#include "PMain.h"
#endif


/**********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
***********************************************************************************/
iim::imProgressBar* iim::imProgressBar::uniqueInstance = 0;
iim::imProgressBar* iim::imProgressBar::instance()
{
    if (uniqueInstance == 0)
        uniqueInstance = new imProgressBar();
    uniqueInstance = new imProgressBar();
    return uniqueInstance;
}

iim::imProgressBar::imProgressBar()
{
    reset();
}


void iim::imProgressBar::reset()
{
    strcpy(this->message_level_1, "");
    strcpy(this->message_level_2, "");
    strcpy(this->message_level_3, "");
    progress_value=0;
    proctime = 0;
    minutes_remaining = 0;
    seconds_remaining = 0;
}


void iim::imProgressBar::start(const char *new_operation_desc, bool toConverter /* = true */)
{
    strcpy(this->message_level_2, new_operation_desc);
    this->progress_value=0;
    strcpy(this->message_level_3, "");
    proctime = -TIME(0);
    minutes_remaining = 0;
    seconds_remaining = 0;

    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    int progress_value_int = (int) progress_value;
    if(toConverter)
        teramanager::PConverter::instance()->emitProgressBarChanged(progress_value_int, 0, 0, new_operation_desc);
    else
        teramanager::PMain::getInstance()->emitProgressBarChanged(progress_value_int, 0, 0, message_level_1);
    #endif
}

void iim::imProgressBar::update(float new_progress_value, const char* new_progress_info)
{
    progress_value=new_progress_value;
    strcpy(message_level_3,new_progress_info);

    if(new_progress_value!=0)
    {
        minutes_remaining = (int)((proctime + TIME(0))*(100.0-progress_value)/(progress_value*60.0));
        seconds_remaining = (int)((proctime + TIME(0))*(100.0-progress_value)/(progress_value));
    }
}


void iim::imProgressBar::updateInfo(const char* new_progress_info)
{
    strcpy(message_level_3,new_progress_info);
}

void iim::imProgressBar::show(bool toConverter /* = true */)
{
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    int progress_value_int = (int) (progress_value+0.5f);
    if(toConverter)
        teramanager::PConverter::instance()->emitProgressBarChanged(progress_value_int, minutes_remaining, seconds_remaining%60, message_level_1);
    else
        teramanager::PMain::getInstance()->emitProgressBarChanged(progress_value_int, minutes_remaining, seconds_remaining%60, message_level_1);
    #else
    int dummy = system_CLEAR();
    printf("OPERATION:\t%s\n",this->message_level_1); // 140427_IANNELLO
    printf("PHASE:\t\t%s\n",this->message_level_2); // 140427_IANNELLO
    printf("TIME REMAINING:\t%d minutes and %d seconds\n", minutes_remaining, seconds_remaining%60);
    printf("PROGRESS:\t");
    printf("%d%%\t",(int)(progress_value));
    for(int i=1; i<=progress_value; i++)
            printf("*");
    for(int i=(int)progress_value; i<100; i++)
            printf(":");
    printf("\n\n");
    #endif
}


void iim::imProgressBar::setMessage(int level, const char* message)
{
    if(level == 1)
        strcpy(message_level_1,message);
    else if(level == 2)
        strcpy(message_level_2,message);
    else if(level == 3)
        strcpy(message_level_3,message);
}
