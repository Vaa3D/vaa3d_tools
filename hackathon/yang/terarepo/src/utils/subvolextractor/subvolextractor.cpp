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

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <math.h>
#include <CmdLine.h>
#include "TemplateCLI.h"
#include "S_config.h"
#include "IM_config.h"

//#include "Stack.h"
//#include "StackedVolume.h"
#include "TiledVolume.h"

#define STACK_FOLDER_NAME   "stack"
#define CMDLINE_FILE_NAME   "cmdline.txt"

using namespace std;
using namespace iim;

int main(int argc, char** argv)
{
	char err_msg[3000];
	int height;
	int width;
	int depth;
	int zb, zl, z;
	int z_nblocks, z_lblocks;

	char mdata_filepath[3000];
	uint8 *buffer = (uint8 *) 0;
	int channels;

	try
	{
		//importing command-line arguments from <TeraStitcherCLI> object
		TemplateCLI cli;
		cli.readParams(argc, argv);
		cli.checkParams();
		
		// do what you have to do
		iom::IMOUT_PLUGIN = "tiff2D";

		//trying to unserialize an already existing metadata file, if it doesn't exist the full initialization procedure is performed and metadata is saved
		sprintf(mdata_filepath, "%s/%s", cli.root_dir.c_str(), MDATA_BIN_FILE_NAME.c_str());
		if ( isFile(mdata_filepath) ) {

			string fname = cli.dst_dir + "/" + STACK_FOLDER_NAME;
			if(!makeDir(fname.c_str()))
			{
				char err_msg[S_STATIC_STRINGS_SIZE];
				sprintf(err_msg, "subvolume extractor: unable to create stack directory = \"%s\"\n", fname.c_str());
				throw iom::exception(err_msg);
			}
			//save data on extracted subvolume
			FILE *fsav;
			fname = cli.dst_dir + "/" + CMDLINE_FILE_NAME;
			if ( (fsav = fopen(fname.c_str(),"w")) == 0 ) {
				sprintf(err_msg,"subvolume extractor: cannot be opened file to save command line");
				throw iom::exception(err_msg);
			};
			for ( int i=0; i<argc; i++ )
				fprintf(fsav,"%s ",argv[i]);
			fprintf(fsav,"\n");
			fclose(fsav);

			//StackedVolume volume(cli.root_dir.c_str());
			TiledVolume volume(cli.root_dir.c_str());

			//height, width and depth of the whole volume that covers all stacks
			height = cli.V1 - cli.V0;
			width  = cli.H1 - cli.H0;
			depth  = cli.D1 - cli.D0;

			z_lblocks = (int) floor((cli.mem * 1.0e6) / ((double)(height * width)));
			z_nblocks = depth / z_lblocks;

			for ( zb = 0, z = 0; zb < z_nblocks; zb++ ) {

				buffer = volume.loadSubvolume_to_UINT8(cli.V0,cli.V1,cli.H0,cli.H1,cli.D0+z,cli.D0+z+z_lblocks,&channels);

				if ( channels != 1 ) {
					sprintf(err_msg,
						"subvolume extractor: multi-channel images not supported (#channels: %d)",
						channels
					);
					throw iom::exception(err_msg);
				}

				for ( zl = 0; zl < z_lblocks; zl++, z++ ) {
					stringstream img_path;
					stringstream abs_pos_z;
					abs_pos_z.width(6);
					abs_pos_z.fill('0');
					abs_pos_z << (int)(z * volume.getVXL_D());
					img_path << cli.dst_dir << "/" << STACK_FOLDER_NAME << "/" << abs_pos_z.str(); 
					VirtualVolume::saveImage_from_UINT8(
						img_path.str(), 
						buffer + (height*width*zl),
						(uint8 *) 0,(uint8 *) 0,
						height,width
					);
				}

				delete buffer;
			}

			if ( z < depth ) { // there are slices not extracted

				buffer = volume.loadSubvolume_to_UINT8(cli.V0,cli.V1,cli.H0,cli.H1,cli.D0+z,cli.D0+depth,&channels);

				//if ( channels != 1 ) {
				//	sprintf(err_msg,
				//		"subvolume extractor: multi-channel images not supported (#channels: %d)",
				//		channels
				//	);
				//	throw MyException(err_msg);
				//}

				for ( zl = 0; z < depth; zl++, z++ ) {
					stringstream img_path;
					stringstream abs_pos_z;
					abs_pos_z.width(6);
					abs_pos_z.fill('0');
					abs_pos_z << (int)(z * volume.getVXL_D());
					img_path << cli.dst_dir << "/" << STACK_FOLDER_NAME << "/" << abs_pos_z.str(); 
					VirtualVolume::saveImage_from_UINT8(
						img_path.str(), 
						buffer + (height*width*zl),
						(channels<2) ? (uint8 *) 0 : buffer + (height*width*depth) + (height*width*zl),
						(channels<3) ? (uint8 *) 0 : buffer + 2*(height*width*depth) + (height*width*zl),
						height,width
					);
				}

				delete buffer;
			}

		}
		else {
			sprintf(err_msg,
				"subvolume extractor: mdata.bin missing in directory %s",
				cli.root_dir.c_str()
			);
			throw iom::exception(err_msg);
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
