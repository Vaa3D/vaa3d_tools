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

#include "CMergeTiles.h"
#include "CImportUnstitched.h"
#include <new>
#include <iostream>
#include "StackStitcher.h"
#include "ProgressBar.h"
#include "IM_config.h"
#include "vmBlockVolume.h"

#include "Tiff3DMngr.h"
#include "VolumeConverter.h"

using namespace terastitcher;

CMergeTiles* CMergeTiles::uniqueInstance = NULL;

void CMergeTiles::uninstance()
{
    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CMergeTiles::~CMergeTiles()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles destroyed\n", this->thread()->currentThreadId());
    #endif
}

UnstitchedVolume* CMergeTiles::unstitchedVolume() throw (iim::IOException)
{
	try
	{
		if(!_unstitchedVolume)
			_unstitchedVolume = new UnstitchedVolume(CImportUnstitched::instance()->getVolume());
		return _unstitchedVolume;
	}
	catch (iim::IOException & e)
	{
		throw e;
	}
	catch(...)
	{
		throw iim::IOException("Unhandled exception in CMergeTiles::unstitchedVolume()");
	}
}

//automatically called when current thread is started
void CMergeTiles::run()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles::run() launched\n", this->thread()->currentThreadId());
    #endif

    try
    {
        // check preconditions
        unstitchedVolume();
		if(!pMergeTiles)
			throw iim::IOException("in CMergeTiles::run(): invalid reference to GUI");

		// get other info from GUI
		int slice_height =  pMergeTiles->block_height_field->value();
		int slice_width = pMergeTiles->block_width_field->value();
		int slice_depth = pMergeTiles->block_depth_field->value();
		int x0 = pMergeTiles->x0_field->value();
		int x1 = pMergeTiles->x1_field->value()+1;
		int y0 = pMergeTiles->y0_field->value();
		int y1 = pMergeTiles->y1_field->value()+1;
		int z0 = pMergeTiles->z0_field->value();
		int z1 = pMergeTiles->z1_field->value()+1;
		bool restoreSPIM = pMergeTiles->restoreSPIM_cbox->currentIndex() != 0;
		std::string dst_root_dir = pMergeTiles->savedir_field->text().toStdString();
		int restore_direction = pMergeTiles->restoreSPIM_cbox->currentIndex();
		int blending_algo = pMergeTiles->blendingalbo_cbox->currentIndex();
		//int img_depth = pMergeTiles->imgdepth_cbox->currentText().section(" ", 0, 0).toInt();
		std::string dst_format = pMergeTiles->vol_format_cbox->currentText().toStdString();
		bool parallel = false;
		bool isotropic = pMergeTiles->isotropic_checkbox->isChecked();
		bool show_progress_bar = true;
		//bool timeseries = false;
		//bool makeDirs = false;
		//bool metaData = false;
		bool halving_method = HALVE_BY_MEAN;
		// std::string chanlist = "";      // already defined as object member
		std::string ch_dir = "";           // currently cannot be changed, the default is passed
		std::string mdata_fname = pMergeTiles->mdata_line->text().toStdString();
		if(mdata_fname == PTabMergeTiles::metadata_path_null || mdata_fname.empty())
			mdata_fname = "null";
		std::string external_compressor_options = pMergeTiles->external_compressor_line->text().toStdString();
		if(external_compressor_options == PTabMergeTiles::external_compressor_line_null || external_compressor_options.empty())
			external_compressor_options = ""; // use default compressor with default options (GZIP level 3)
		int downsamplingFactor = 1;        // currently cannot be changed, the default is passed
		//std::string outFmt = "RGB";      // no more used



		// set blending algorithm
		_unstitchedVolume->setBLENDING_ALGO(blending_algo);

		// call volume conversion driver
		vcDriver(
			_unstitchedVolume, 
			"",                 // the volume has been already loaded this parameter is not used
			dst_root_dir, 
			"",                 // the volume has been already loaded this parameter is not used
			dst_format, 
			0,                  // currently not used: imout depth is set internally
			resolutions, 
			chanlist, 
			ch_dir, 
			mdata_fname,        
			slice_depth, 
			slice_height, 
			slice_width, 
			downsamplingFactor,
			halving_method, 
			1,                  // libtiff_rowsPerStrip: currently cannot be changed, the default is passed
			pMergeTiles->libtiff_uncompressed_checkbox->isChecked(), 
			pMergeTiles->libtiff_bigtiff_checkbox->isChecked(), 
			show_progress_bar, 
			isotropic, 
			y0, y1, x0, x1, z0, z1,
			false,
			false,
			false,
			false,
			external_compressor_options
			// the other parameters are for command line version only, the default values are passed
		);

        //everything went OK
		emit sendOperationOutcome(0);
    }
    catch( iim::IOException& exception)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch( iom::exception& exception)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", exception.what()).c_str());
        emit sendOperationOutcome(new iom::exception(exception.what()));
    }
    catch(const char* error)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", error).c_str());
        emit sendOperationOutcome(new iom::exception(error));
    }
    catch(std::bad_alloc& ba)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", ba.what()).c_str());
        emit sendOperationOutcome(new iom::exception(ba.what()));
    }
    catch(...)
    {
        /**/ts::warning(strprintf("exception thrown in CMergeTiles::run(): \"%s\"", "Generic error").c_str());
        emit sendOperationOutcome(new iom::exception("Unable to determine error's type"));
    }
}

//reset method
void CMergeTiles::reset()
{
    #ifdef TSP_DEBUG
    printf("TeraStitcher plugin [thread %d] >> CMergeTiles::reset()\n", this->thread()->currentThreadId());
    #endif

    for(int i=0; i<S_MAX_MULTIRES; i++)
        resolutions[i] = i==0;
    pMergeTiles = 0;
	if(_unstitchedVolume)
	{
		delete _unstitchedVolume;
		_unstitchedVolume = 0;
	}
	chanlist = "";
}