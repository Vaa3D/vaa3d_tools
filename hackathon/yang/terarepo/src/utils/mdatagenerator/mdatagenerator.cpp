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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
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

/******************
*    CHANGELOG    *
*******************
* 2014-11-10. Giulio.     @FIXED writes mdata.bin if it does not exist OR it must be overwritten (WARNING: by default it should not overwritten and there is not an overwrite option 
*/

#include <iostream>
#include <stdio.h>
#include <CmdLine.h>
#include "TemplateCLI.h"

#include "IM_config.h"
#include "iomanager.config.h"
#include "Stack.h"
#include "StackedVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"

using namespace std;
using namespace iim;
using namespace iom;

int main(int argc, char** argv)
{
	try
	{
		//importing command-line arguments from <TeraStitcherCLI> object
		TemplateCLI cli;
		cli.readParams(argc, argv);
		cli.checkParams();
		
		// do what you have to do

		char mdata_filepath[1000];
		if ( cli.src_format.compare(iim::TILED_MC_FORMAT) == 0 || cli.src_format.compare(iim::TILED_MC_TIF3D_FORMAT) == 0 ) {
			sprintf(mdata_filepath, "%s/%s", cli.root_dir.c_str(), MC_MDATA_BIN_FILE_NAME.c_str());
			if ( !isFile(mdata_filepath) || cli.overwrite_mdata ) { // if metadata file does not exist or must be overw written the full initialization procedure is performaed and metadata saved
				TiledMCVolume volume(cli.root_dir.c_str(),ref_sys(cli.axis_V,cli.axis_H,cli.axis_D),cli.vxlsz_V,cli.vxlsz_H,cli.vxlsz_D,cli.overwrite_mdata);
			}
			else {
				TiledMCVolume volume(cli.root_dir.c_str()); // load available data
				if ( cli.update_mdata ) {
					// regenerate metadata
					TiledMCVolume new_volume(cli.root_dir.c_str(),
						volume.getVolumes()[0]->getREF_SYS(),volume.getVolumes()[0]->getVXL_1(),volume.getVolumes()[0]->getVXL_2(),volume.getVolumes()[0]->getVXL_3(),true);
				}
				else {
					volume.print();
				}
			}
		}
		else { // Stacked or Tiled formats
			sprintf(mdata_filepath, "%s/%s", cli.root_dir.c_str(), MDATA_BIN_FILE_NAME.c_str());
			if ( !isFile(mdata_filepath) || cli.overwrite_mdata ) { // if metadata file does not exist or must be overw written the full initialization procedure is performaed and metadata saved
				if(cli.src_format.compare(iim::STACKED_FORMAT) == 0)
					StackedVolume volume(cli.root_dir.c_str(),ref_sys(cli.axis_V,cli.axis_H,cli.axis_D),cli.vxlsz_V,cli.vxlsz_H,cli.vxlsz_D,cli.overwrite_mdata);
				else if(cli.src_format.compare(iim::TILED_FORMAT) == 0 || cli.src_format.compare(iim::TILED_TIF3D_FORMAT) == 0)
					TiledVolume volume(cli.root_dir.c_str(),ref_sys(cli.axis_V,cli.axis_H,cli.axis_D),cli.vxlsz_V,cli.vxlsz_H,cli.vxlsz_D,cli.overwrite_mdata);
			}
			else {  // perform normal initialization and prints metadata if a text file
			   if(cli.src_format.compare(iim::STACKED_FORMAT) == 0) {
					StackedVolume volume(cli.root_dir.c_str());
					volume.print();
			   }
				else if(cli.src_format.compare(iim::TILED_FORMAT) == 0 || cli.src_format.compare(iim::TILED_TIF3D_FORMAT) == 0) {
					TiledVolume volume(cli.root_dir.c_str());
					volume.print();
				}
			}
		}
	}
	catch( iom::exception& exception){
		cout<<"ERROR: "<<exception.what()<<endl<<endl;
	}
	catch(bad_exception& be){
		cout<<"GENERIC ERROR: "<<be.what()<<endl<<endl;
	}
	catch(char* error){
		cout<<"GENERIC ERROR: "<<error<<endl<<endl;
	}
	//system("PAUSE");
	return EXIT_SUCCESS;
}
