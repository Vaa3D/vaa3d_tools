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

#ifndef RESUMER_H
#define RESUMER_H

//#include "../ImageManager/IM_config.h"
#include "IM_config.h"

#include <stdio.h>

bool initResumer ( const char *out_fmt, const char *output_path, int resolution_size,bool* resolutions, 
				   int block_height, int block_width, int block_depth, int method, 
                   const char* saved_img_format, int saved_img_depth, FILE *&fhandle ) throw (iim::IOException);

void readResumerState ( FILE *&fhandle, const char *output_path, int &resolution_size, int *stack_block, int *slice_start, int *slice_end, 
                 iim::sint64 &z, iim::sint64 &z_parts ) throw (iim::IOException);

void saveResumerState ( FILE *fhandle, int resolution_size, int *stack_block, int *slice_start, int *slice_end, 
                 iim::sint64 z, iim::sint64 z_parts ) throw (iim::IOException);

void closeResumer ( FILE *fhandle, const char *output_path = 0 ) throw (iim::IOException);

#endif

