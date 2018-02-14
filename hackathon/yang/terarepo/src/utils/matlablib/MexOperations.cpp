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

/******************
*    CHANGELOG    *
*******************
* 2016-07-25. Giulio. @CREATED 
*/

#include "MexOperations.h"

#include "VirtualVolume.h"

using namespace iim;

int createObject ( void **objectList, int listLen, const char *initString ) {
	VirtualVolume **vvList = (VirtualVolume **) objectList;
	int i;
	for ( i=0; objectList[i]; i++ );
	if ( i < listLen ) {
		// create the object
		try {
			vvList[i] = VirtualVolume::instance(initString);
			return i;
		}
		catch( iim::IOException& exception){
			return -1;
		}
		catch(char* error){
			return -1;
		}
	}
	else // list full: the object cannot be created
		return -1;
}


void disposeObject ( void *obj ) {
	delete ((VirtualVolume *) obj);
}


void getInfo ( void *obj, info_t *info_ ) {
	VirtualVolume *volume = (VirtualVolume *) obj;
	std::string fmt = volume->getPrintableFormat();

	info_->root_dir   = volume->getROOT_DIR();
	info_->VXL_V      = volume->getVXL_V();
	info_->VXL_H      = volume->getVXL_H();
	info_->VXL_D      = volume->getVXL_D();
	info_->ORG_V      = volume->getORG_V();
	info_->ORG_H      = volume->getORG_H();
	info_->ORG_D      = volume->getORG_D();
	info_->DIM_V      = volume->getDIM_V();
	info_->DIM_H      = volume->getDIM_H();
	info_->DIM_D      = volume->getDIM_D();
	info_->DIM_C      = volume->getDIM_C();
	info_->BYTESxCHAN = volume->getBYTESxCHAN();
	info_->n_active   = volume->getNACtiveChannels();
	info_->active     = volume->getActiveChannels();
	info_->DIM_T      = volume->getDIM_T();
	info_->t0         = volume->getT0();
	info_->t1         = volume->getT1();
}


unsigned char *loadSubvolume ( void *obj, int V0, int V1, int H0, int H1, int D0, int D1 ) {
	VirtualVolume *volume = (VirtualVolume *) obj;
	int channels; // dummy argument
	try {
		return volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,D0,D1,&channels,0); // always require native type
	}
	catch( iim::IOException& exception){
		return (unsigned char *) 0;
	}
	catch(char* error){
		return (unsigned char *) 0;
	}
}

void disposeObjects ( void **objectList, int listLen ) {
	VirtualVolume **vvList = (VirtualVolume **) objectList;
	for ( int i=0; i<listLen; i++ ) {
		if ( vvList[i] ) 
			delete vvList[i];
	}
}
