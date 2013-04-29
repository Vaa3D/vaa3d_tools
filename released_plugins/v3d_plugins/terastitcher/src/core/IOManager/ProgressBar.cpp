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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

#include "ProgressBar.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <cstdlib>

using namespace std;

#ifdef _VAA3D_PLUGIN_MODE
#include "src/presentation/PMain.h"
#endif


/**********************************************************************************
* Singleton design pattern: this class can have one instance only,  which must be
* instantiated by calling static method "istance(...)"
***********************************************************************************/
ProgressBar* ProgressBar::uniqueInstance = NULL;
ProgressBar* ProgressBar::instance()
{
    if (uniqueInstance == NULL)
        uniqueInstance = new ProgressBar();
    uniqueInstance = new ProgressBar();
    return uniqueInstance;
}

ProgressBar::ProgressBar()
{
    strcpy(this->operation_desc, "none");
    progress_value=0;
    strcpy(this->progress_info, "");
    proctime = 0;
    minutes_remaining = 0;
    seconds_remaining = 0;
}


void ProgressBar::start(const char *new_operation_desc)
{
    strcpy(this->operation_desc, new_operation_desc);
    this->progress_value=0;
    strcpy(this->progress_info, "");
    proctime = -TIME(0);
    minutes_remaining = 0;
    seconds_remaining = 0;

    #ifdef _VAA3D_PLUGIN_MODE
    int progress_value_int = (int) progress_value;
    terastitcher::PMain::instance()->emitProgressBarChanged(progress_value_int, 0, 0, new_operation_desc);
    #endif
}

void ProgressBar::update(float new_progress_value, const char* new_progress_info)
{
        progress_value=new_progress_value;
	strcpy(progress_info,new_progress_info);

        if(new_progress_value!=0)
	{
		minutes_remaining = (proctime + TIME(0))*(100.0-progress_value)/(progress_value*60.0);
		seconds_remaining = (proctime + TIME(0))*(100.0-progress_value)/(progress_value);
        }
}


void ProgressBar::updateInfo(const char* new_progress_info)
{
	strcpy(progress_info,new_progress_info);
}

void ProgressBar::show()
{

#ifdef _VAA3D_PLUGIN_MODE
    int progress_value_int = (int) progress_value;
    terastitcher::PMain::instance()->emitProgressBarChanged(progress_value_int, minutes_remaining, seconds_remaining%60);
#else
    system_CLEAR();
    printf("OPERATION:\t%s\n",this->operation_desc);
    printf("PHASE:\t\t%s\n",this->progress_info);
    printf("TIME REMAINING:\t%d minutes and %d seconds\n", minutes_remaining, seconds_remaining%60);
    printf("PROGRESS:\t");
    printf("%d%%\t",(int)(progress_value));
    for(int i=1; i<=progress_value; i++)
            printf("Û");
    for(int i=progress_value; i<100; i++)
            printf(":");
    printf("\n\n");
#endif
}
