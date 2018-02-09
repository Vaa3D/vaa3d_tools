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
* 2017-09-11. Giulio.     @CHANGED interfaces of vcDriver and convetTo to enable passing parameters controlloing the compression algorithm to be used with HDf5 files
* 2017-09-11. Giulio.     @ADDED parameters controlloing the compression algorithm to be used with HDf5 files
* 2017-09-09. Giulio.     @ADDED code to manage compression algorithms to be used in Imaris IMS files generation
* 2017-06-26. Giulio.     @ADDED parameter 'isotropic' and 'mdata_file' to method 'convertTo'
* 2017-06-26. Giulio.     @ADDED timeseries to Imaris format
* 2017-05-25. Giulio.     @ADDED method for enabling lossy compression based on rescaling and implemented rescaling in all output formats
* 2017-04-03. Giulio.     @CHANGED a new configuration of the libtiff library is carried out even if it has already been configured
* 2017-04-20. Giulio      @CHANGED calls to 'IMS_HDF5init' to improve structure initialization
* 2017-04-20. Giulio.     @FIXED a bug in the allocation of 'active_chans' in the SetSrc methods
* 2017-04-17. Giulio.     @ADDED the possibility to generate an IMS file with default metadata
* 2017-04-09. Giulio.     @ADDED the ability to convert a subset of channels
* 2017-04-08. Giulio.     @ADDED support for additional attributes required by the IMS format
* 2017-04-01. Giulio.     @FIXED some error messages
* 2017-04-01. Giulio.     @FIXED save state at the beginning of the main for loop in case an error happens during the first cycle
* 2017-02-01. Giulio.     @FIXED bugs in computing the voxel size along D when isotropic downsizing is set
* 2017-01-23. Giulio.     @FIXED bugs of parallel execution in case 4D formats are specified
* 2017-01-22. Giulio      @CHANGED the setting on z_max_res in tiled formats generation for efficiency reasons
* 2016-10-12. Giulio.     @FIXED when axes are negative this should be propagated to generated image (in all tiled generators)
* 2016-10-09. Giulio.     @ADDED parameter 'ch_dir' to 'generateTilesVaa3DRawMC' interface; the parameter plays a role only if channels are subdirectories (RES_IN_CHANS not defined)
* 2014-06-20. Giulio.     @ADDED conversion to 'simple' representation (series, 2D), including parallel support
* 2014-05-11. Giulio.     @ADDED check that the whole volume is processed in makedir/parallel/metadata modes
* 2014-04-28. Giulio.     @CHANGED output plugin is temporarily substituted to the input plugin to genrate the metadata
* 2014-04-23. Giulio.     WARNING - resolutions directories in channel directory version of generateTilesVaa3DRawMC no longer reliable
* 2016-04-23. Giulio.     @ADDED methods and code to manage parallelization in generateTilesVaa3DRawMC (channels directories in resolutions directories version only) 
* 2016-04-13. Giulio.     @ADDED methods and code to manage parallelization in generateTiles
* 2016-04-13. Giulio.     @ADDED methods and code to manage parallelization in generateTilesVaa3DRaw
* 2016-04-13. Giulio.     @ADDED parallel and isotropic option support
* 2016-04-10. Giulio.     @ADDED the input plugin is saved before callin the TiledVolume constructor in 'generateTilesVaa3D  methods
* 2016-04-09. Giulio.     @FIXED added check in 'generateTiles' to avoid an exception when metadata for the output StackedVolume are created (when the input plugin is 3D)
* 2015-12-26. Giulio.     @FIXED subvolume vertices are set to default values if exceed volume dimensions
* 2015-12-20. Giulio.     @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um 
* 2015-12-19. Giulio.     @ADDED Subvolume conversion (setSubVolume method, changes to mergeTilesVaa3DRaw)
* 2015-06-12. Giulio.     @FIXED the right output reference system is set in all cases at the end of the merge algorithm (the case MC input volume was not properly handled)
* 2015-04-14. Alessandro. @FIXED misleading usage of 'VirtualVolume::instance' w/o format argument in 'setSrcVolume'
* 2015-04-14. Alessandro. @FIXED bug-crash when the volume has not been imported correctly in setSrcVolume.
* 2015-03-03. Giulio.     @ADDED selection of IO plugin if not provided (2D or 3D according to the method).
* 2015-02-12. Giulio.     @ADDED the same optimizations also in multi-channels (MC) methods
* 2015-02-12. Giulio.     #ADDED check on the number of slice in the buffer if multiple resolutions are requested
* 2015-02-10. Giulio.     @ADDED completed optimizations to reduce opend/close in append operations (only in generateTilesVaa3DRaw)
* 2015-01-06. Giulio.     @ADDED optimizations to reduce opend/close in append operations (only in generateTilesVaa3DRaw)
* 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
* 2014-11-10. Giulio.     @CHANGED allowed saving 2dseries with a depth of 16 bit (generateTiles)
*/

#include "VolumeConverter.h"
#include "IM_config.h"
#include "ProgressBar.h"
#include "iomanager.config.h"
#include <math.h>
#include <string>
#include <chrono>
#include <iostream>

#include <stdarg.h>

using namespace std;

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif

/*******************************************************************************************************
* Volume formats supported:
* 
* SimpleVolume:  simple sequence of slices stored as 2D images in the same directory
* StackedVolume: bidimensional matrix of 3D stacks stored in a hierarchical structure of directories
*
*******************************************************************************************************/
#include "BDVVolume.h"
#include "SimpleVolume.h"
#include "SimpleVolumeRaw.h"
#include "RawVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "StackedVolume.h"
#include "TimeSeries.h"
/******************************************************************************************************/

#include "Tiff3DMngr.h"
#include "HDF5Mngr.h"
#include "IMS_HDF5Mngr.h"

#include <limits>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <cstdio>
#include "vcresumer.h"

// 2016--04-09 Giulio.
#include "IOPluginAPI.h" 


using namespace iim;


// code to invoke the converter
void vcDriver (
        iim::VirtualVolume *vPtr,
        std::string src_root_dir,
        std::string dst_root_dir,
        std::string src_format,
        std::string dst_format,
        int         img_depth, // currently non used
        bool       *resolutions,
        std::string chanlist,
        std::string ch_dir,                     // name of the subdirectory where image should be saved (only for a single channel converted to tiled 4D format)
        std::string mdata_fname,                // name of the file containing general metadata to be transferred to destination file (used only by some formats)
        int         slice_depth,
        int         slice_height,
        int         slice_width,
        int         downsamplingFactor,
        int         halving_method,
        int         libtiff_rowsPerStrip,
        bool        libtiff_uncompressed,
        bool        libtiff_bigtiff,
        bool        show_progress_bar,			//enables/disables progress bar with estimated time remaining
        bool        isotropic,                  //generate lowest resolutions with voxels as much isotropic as possible
        int V0, int V1, int H0, int H1, int D0,int D1,
        bool        timeseries,
        bool        makeDirs,                   //creates the directory hiererchy
        bool        metaData,                   //creates the mdata.bin file of the output volume
        bool        parallel,                   //parallel mode: does not perform side-effect operations during merge
        std::string outFmt,                     //additional information about the output format (default: "")
        int         nbits ) throw (iim::IOException, iom::exception) {
    // do what you have to do
    VolumeConverter vc;

    // call the version that forces the configuration even if the library has already been configured
    resetLibTIFFcfg(!libtiff_uncompressed,libtiff_bigtiff,libtiff_rowsPerStrip);

    if ( vPtr )
        vc.setSrcVolume(vPtr,"RGB",timeseries,downsamplingFactor,chanlist);
    else
        vc.setSrcVolume(src_root_dir.c_str(),src_format.c_str(),"RGB",timeseries,downsamplingFactor,chanlist);

    vc.setSubVolume(V0,V1,H0,H1,D0,D1);

    if ( nbits )
        vc.setCompressionAlgorithm(nbits);

    if ( dst_format == iim::SIMPLE_RAW_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchySimple(dst_root_dir.c_str(),resolutions,
                                              slice_height,slice_width,-1,halving_method,isotropic,
                                              show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
            // 				vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
            // 					slice_height,slice_width,-1,halving_method,isotropic,
            // 					show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            //vc.mdataGenerator(dst_root_dir.c_str(),resolutions,
            //	slice_height,slice_width,-1,halving_method,isotropic,
            //	show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTilesSimple(dst_root_dir.c_str(),resolutions,
                                   slice_height,slice_width,halving_method,isotropic,
                                   show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    else if ( dst_format == iim::SIMPLE_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchySimple(dst_root_dir.c_str(),resolutions,
                                              slice_height,slice_width,-1,halving_method,isotropic,
                                              show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
            // 				vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
            // 					slice_height,slice_width,-1,halving_method,isotropic,
            // 					show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            //vc.mdataGenerator(dst_root_dir.c_str(),resolutions,
            //	slice_height,slice_width,-1,halving_method,isotropic,
            //	show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTilesSimple(dst_root_dir.c_str(),resolutions,
                                   slice_height,slice_width,halving_method,isotropic,
                                   show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    else if ( dst_format == iim::STACKED_RAW_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,-1,halving_method,isotropic,
                                        show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,-1,halving_method,isotropic,
                              show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTiles(dst_root_dir.c_str(),resolutions,
                             slice_height,slice_width,halving_method,isotropic,
                             show_progress_bar,"raw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    else if ( dst_format == iim::STACKED_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,-1,halving_method,isotropic,
                                        show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,-1,halving_method,isotropic,
                              show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTiles(dst_root_dir.c_str(),resolutions,
                             slice_height,slice_width,halving_method,isotropic,
                             show_progress_bar,"tif",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    else if ( dst_format == iim::TILED_FORMAT ) {
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,slice_depth,halving_method,isotropic,
                                        show_progress_bar,"Vaa3DRaw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,slice_depth,halving_method,isotropic,
                              show_progress_bar,"Vaa3DRaw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTilesVaa3DRaw(dst_root_dir.c_str(),resolutions,
                                     slice_height,slice_width,slice_depth,halving_method,isotropic,
                                     show_progress_bar,"Vaa3DRaw",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    }
    else if ( dst_format == iim::TILED_TIF3D_FORMAT || dst_format == iim::TIF3D_FORMAT) {

        //cout<<"convert to tiled 3D tiff "<<(timeseries?"timeseries":"not time series")<<" "<<(makeDirs?"makeDirs":"not makeDirs")<<" "<<(metaData?"metaData":"Not metaData")<<endl;

        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,slice_depth,halving_method,isotropic,
                                        show_progress_bar,"Tiff3D",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,slice_depth,halving_method,isotropic,
                              show_progress_bar,"Tiff3D",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
//            vc.generateTilesVaa3DRaw(dst_root_dir.c_str(),resolutions,
//                                     slice_height,slice_width,slice_depth,halving_method,isotropic,
//                                     show_progress_bar,"Tiff3D",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);

            vc.generate3DTiles(dst_root_dir.c_str(),resolutions,
                               slice_height,slice_width,slice_depth,halving_method,isotropic,
                               show_progress_bar,"Tiff3D",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    }
    else if ( dst_format == iim::TILED_MC_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,slice_depth,halving_method,isotropic,
                                        show_progress_bar,"Vaa3DRawMC",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,slice_depth,halving_method,isotropic,
                              show_progress_bar,"Vaa3DRawMC",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTilesVaa3DRawMC(dst_root_dir.c_str(),ch_dir,resolutions,
                                       slice_height,slice_width,slice_depth,halving_method,isotropic,
                                       show_progress_bar,"Vaa3DRaw",8*vc.getVolume()->getBYTESxCHAN(),"",false);
        }
    else if ( dst_format == iim::TILED_MC_TIF3D_FORMAT )
        if ( timeseries ) {
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic);
        }
        else if ( makeDirs ) {
            vc.createDirectoryHierarchy(dst_root_dir.c_str(),ch_dir,resolutions,
                                        slice_height,slice_width,slice_depth,halving_method,isotropic,
                                        show_progress_bar,"Tiff3DMC",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else if ( metaData ) {
            vc.mdataGenerator(dst_root_dir.c_str(),ch_dir,resolutions,
                              slice_height,slice_width,slice_depth,halving_method,isotropic,
                              show_progress_bar,"Tiff3DMC",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
        else {
            vc.generateTilesVaa3DRawMC(dst_root_dir.c_str(),ch_dir,resolutions,
                                       slice_height,slice_width,slice_depth,halving_method,isotropic,
                                       show_progress_bar,"Tiff3D",8*vc.getVolume()->getBYTESxCHAN(),"",parallel);
        }
    else if ( dst_format == iim::BDV_HDF5_FORMAT )
        vc.generateTilesBDV_HDF5(dst_root_dir.c_str(),resolutions,
                                 slice_height,slice_width,slice_depth,halving_method,isotropic,
                                 show_progress_bar,"Fiji_HDF5",8*vc.getVolume()->getBYTESxCHAN());
    else if ( dst_format == iim::IMS_HDF5_FORMAT ) {
        if ( timeseries ) {
            // missing parameters: mdata_fname, isotropic,
            vc.convertTo(dst_root_dir.c_str(),dst_format,8*vc.getVolume()->getBYTESxCHAN(),true,resolutions,
                         slice_height,slice_width,slice_depth,halving_method,isotropic,mdata_fname,outFmt);
        }
        else {
            vc.generateTilesIMS_HDF5(dst_root_dir.c_str(),mdata_fname,resolutions,
                                     slice_height,slice_width,slice_depth,halving_method,isotropic,
                                     show_progress_bar,(outFmt == "" ? "Fiji_HDF5" : outFmt.c_str()),8*vc.getVolume()->getBYTESxCHAN());
        }
    }
    else
        throw iim::IOException(iim::strprintf("in vcDriver(): unsupported format \"%s\"", dst_format.c_str()));
}


VolumeConverter::VolumeConverter( )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    volume = (VirtualVolume *) 0;
    volume_external = false;

    lossy_compression = false;
    nbits = 0;
}


VolumeConverter::~VolumeConverter()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    if(volume && !volume_external)
        delete volume;
}


void VolumeConverter::setSrcVolume(const char* _root_dir, const char* _fmt, const char* _out_fmt, 
                                   bool time_series /* = false */, int downsamplingFactor /* = 1 */,
                                   std::string chanlist /* = ""*/, int _res /* = 0*/, int _timepoint /* = 0*/) throw (IOException, iom::exception)
{

    auto start = std::chrono::high_resolution_clock::now();

    /**/iim::debug(iim::LEV3, strprintf("_root_dir = %s, _fmt = %s, _out_fmt = %s, time_series = %s",
                                        _root_dir, _fmt, _out_fmt, time_series ? "true" : "false").c_str(), __iim__current__function__);

    if(time_series)
        volume = new TimeSeries(_root_dir, _fmt);
    else {
        std::string format = _fmt;
        if ( format.compare((BDVVolume().getPrintableFormat())) == 0 )
            volume = VirtualVolume::instance(_root_dir,_res,(void *)0,_timepoint);
        else
            //volume = VirtualVolume::instance(_root_dir, _fmt, vertical, horizontal, depth, 1.0f, 1.0f, 1.0f);
            //volume = VirtualVolume::instance_format(_root_dir);
            // 2015-04-14. Alessandro. @FIXED misleading usage of 'VirtualVolume::instance' w/o format argument in 'setSrcVolume'
            volume = VirtualVolume::instance_format(_root_dir, _fmt);
    }
    
    // 2015-04-14 Alessandro. @FIXED bug-crash when the volume has not been imported correctly in setSrcVolume.
    if(!volume)
        throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): unable to recognize the volume format of \"%s\"", _root_dir));

    if( downsamplingFactor > 1 ) {
        std::string format = _fmt;
        if ( format.compare((SimpleVolume().getPrintableFormat())) == 0 )
            ((SimpleVolume *) volume)->setDOWNSAMPLINGFACTOR(downsamplingFactor);
        else if ( format.compare((SimpleVolumeRaw().getPrintableFormat())) == 0 )
            ((SimpleVolumeRaw *) volume)->setDOWNSAMPLINGFACTOR(downsamplingFactor);
        else
            throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): source volume (\"%s\") cannot be downsampled", _fmt));
    }

    if ( chanlist == "" )
        channels = volume->getDIM_C();
    else { // a channel list has been specified
        channels = (int) chanlist.size();
        iim::uint32 *active_chans = new iim::uint32[channels];
        for ( int i=0; i<channels; i++) {
            if ( isdigit(chanlist.at(i)) )
                active_chans[i] = chanlist.at(i) - '0';
            else
                throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): the channel list contains a non-digit character (%c)", chanlist.at(i)));
        }
        volume->setActiveChannels(active_chans,channels);
    }

    if ( strcmp(_out_fmt,REAL_REPRESENTATION) == 0 ) {
        if ( channels > 1 ) {
            fprintf(stderr,"*** warning *** more than 1 channel, the internal representation has been changed\n");
            out_fmt = UINT8x3_REPRESENTATION;
            internal_rep = UINT8_INTERNAL_REP;
        }
        else {
            out_fmt = _out_fmt;
            internal_rep = REAL_INTERNAL_REP;
        }
    }
    else if ( strcmp(_out_fmt,UINT8_REPRESENTATION) == 0 ) {
        out_fmt = _out_fmt;
        internal_rep = UINT8_INTERNAL_REP;
    }
    else if ( strcmp(_out_fmt,UINT8x3_REPRESENTATION) == 0 ) {
        out_fmt = _out_fmt;
        internal_rep = UINT8_INTERNAL_REP;
    }
    else {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported output format (%s)",out_fmt);
        throw IOException(err_msg);
    }

    V0 = 0;
    H0 = 0;
    D0 = 0;
    V1 = volume->getDIM_V();
    H1 = volume->getDIM_H();
    D1 = volume->getDIM_D();

    auto end = std::chrono::high_resolution_clock::now();

    //cout<<"setSrcVolume takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<endl;
}

// additional setSrcVolume @ADDED by Alessandro on 2014-04-18: takes an external vm::VirtualVolume in input
void VolumeConverter::setSrcVolume(iim::VirtualVolume * _imported_volume,
                                   const char* _out_fmt, bool time_series,
                                   int downsamplingFactor, std::string chanlist, int _res, int _timepoint) throw (iim::IOException, iom::exception)
{
    volume_external = true;
    volume = _imported_volume;

    // 2015-04-14 Alessandro. @FIXED bug-crash when the volume has not been imported correctly in setSrcVolume.
    if(!volume)
        throw iim::IOException("in VolumeConverter::setSrcVolume(): invalid pre-imported volume provided");

    if( downsamplingFactor > 1 ) {
        if ( dynamic_cast<SimpleVolume*>(volume) )
            dynamic_cast<SimpleVolume*>(volume)->setDOWNSAMPLINGFACTOR(downsamplingFactor);
        else if ( dynamic_cast<SimpleVolumeRaw*>(volume) )
            dynamic_cast<SimpleVolumeRaw*>(volume)->setDOWNSAMPLINGFACTOR(downsamplingFactor);
        else
            throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): source volume (\"%s\") cannot be downsampled", volume->getPrintableFormat().c_str()));
    }

    if ( chanlist == "" )
        channels = volume->getDIM_C();
    else { // a channel list has been specified
        channels = (int) chanlist.size();
        iim::uint32 *active_chans = new iim::uint32[channels];
        for ( int i=0; i<channels; i++) {
            if ( isdigit(chanlist.at(i)) )
                active_chans[i] = chanlist.at(i) - '0';
            else
                throw iim::IOException(iim::strprintf("in VolumeConverter::setSrcVolume(): the channel list contains a non-digit character (%c)", chanlist.at(i)));
        }
        volume->setActiveChannels(active_chans,channels);
    }

    if ( strcmp(_out_fmt,REAL_REPRESENTATION) == 0 ) {
        if ( channels > 1 ) {
            fprintf(stderr,"*** warning *** more than 1 channel, the internal representation has been changed\n");
            out_fmt = UINT8x3_REPRESENTATION;
            internal_rep = UINT8_INTERNAL_REP;
        }
        else {
            out_fmt = _out_fmt;
            internal_rep = REAL_INTERNAL_REP;
        }
    }
    else if ( strcmp(_out_fmt,UINT8_REPRESENTATION) == 0 ) {
        out_fmt = _out_fmt;
        internal_rep = UINT8_INTERNAL_REP;
    }
    else if ( strcmp(_out_fmt,UINT8x3_REPRESENTATION) == 0 ) {
        out_fmt = _out_fmt;
        internal_rep = UINT8_INTERNAL_REP;
    }
    else {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported output format (%s)",out_fmt);
        throw IOException(err_msg);
    }

    V0 = 0;
    H0 = 0;
    D0 = 0;
    V1 = volume->getDIM_V();
    H1 = volume->getDIM_H();
    D1 = volume->getDIM_D();
}


void VolumeConverter::setSubVolume(int _V0, int _V1, int _H0, int _H1, int _D0, int _D1 ) throw (iim::IOException) {
    if ( volume ) {
        V0 = _V0 == -1 ? 0 : std::max<int>(_V0,0);
        H0 = _H0 == -1 ? 0 : std::max<int>(_H0,0);
        D0 = _D0 == -1 ? 0 : std::max<int>(_D0,0);
        V1 = _V1 == -1 ? volume->getDIM_V() : std::min<int>(_V1,volume->getDIM_V());
        H1 = _H1 == -1 ? volume->getDIM_H() : std::min<int>(_H1,volume->getDIM_H());
        D1 = _D1 == -1 ? volume->getDIM_D() : std::min<int>(_D1,volume->getDIM_D());
    }
    else
        throw iim::IOException(iim::strprintf("volume is not set").c_str(),__iim__current__function__);
}


void VolumeConverter::setCompressionAlgorithm(int _nbits ) throw (iim::IOException, iom::exception) {
    if ( _nbits > 0 ) {
        lossy_compression = true;
        nbits = _nbits;
    }
}


/*************************************************************************************************************
* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTiles(std::string output_path, bool* resolutions, 
                                    int slice_height, int slice_width, int method, bool isotropic, bool show_progress_bar,
                                    const char* saved_img_format, int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::generateTiles(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", slice_height = %d, slice_width = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           slice_height, slice_width, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default value: output depth is the same of input depth
        saved_img_depth = (volume->getBYTESxCHAN() * 8);

    // 	if ( saved_img_depth != 8 && volume->getNACtiveChannels() > 1) {
    // 		char err_msg[STATIC_STRINGS_SIZE];
    // 		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d bits per channel of destination is not supported for %d channels",
    // 			saved_img_depth, volume->getNACtiveChannels());
    // 		throw IOException(err_msg);
    // 	}

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) { // saveImage_from and saveImage_from_UINT8 do not support depth conversion yet
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTiles: a mismatch between bits per channel of source (%d) and destination (%d) is not supported",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    int supported_channels;     //channels to be supported (stacks of tiffs 2D only supports 1 or 3 channels)
    // real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT], n_stacks_H[TMITREE_MAX_HEIGHT];             //array of number of tiles along V and H directions respectively at i-th resolution
    int **stacks_height[TMITREE_MAX_HEIGHT], **stacks_width[TMITREE_MAX_HEIGHT];	//array of matrices of tiles dimensions at i-th resolution
    std::stringstream file_path[TMITREE_MAX_HEIGHT];                            //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin;  // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTiles: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff2D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff2D";
    //}

    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    //initializing the progress bar
    char progressBarMsg[200];
    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // test, if any, should be done on V0, V1, ...
    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //	char err_msg[5000];
    //	sprintf(err_msg,"in VolumeConverter::generateTiles(...): block_depth is not set in parallel mode");
    //	throw iom::exception(err_msg);
    //}

    //activating resolutions
    slice_height = (slice_height == -1 ? (int)height : slice_height);
    slice_width  = (slice_width  == -1 ? (int)width  : slice_width);
    if(slice_height < TMITREE_MIN_BLOCK_DIM || slice_width < TMITREE_MIN_BLOCK_DIM)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTiles: The minimum dimension for both slice width and height is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }
    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) slice_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) slice_width  );
        stacks_height[res_i] = new int *[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                stacks_width [res_i][stack_row][stack_col] = ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
            }
        }
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<whole_height/powInt(2,res_i)<<"x"<<whole_width/powInt(2,res_i)<<"x"<<whole_depth/powInt(2,halve_pow2[res_i])<<")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }

                //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
                if ( frame_dir != "" ) {
                    file_path[res_i] << "/" << frame_dir << "/";
                    if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                        if(!check_and_make_dir(file_path[res_i].str().c_str()))
                        {
                            char err_msg[STATIC_STRINGS_SIZE];
                            sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                            throw IOException(err_msg);
                        }
                    }
                }
            }
        }
    }

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,halve_pow2[resolutions_size-1]);
    z_ratio=depth/z_max_res;

    // check the number of channels
    if ( channels > 3 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in generateTiles(...): the volume contains too many channels (%d)", channels);
        throw IOException(err_msg);
    }

    //allocated even if not used
    org_channels = channels; // save for checks
    supported_channels = (channels>1) ? 3 : 1; // only 1 or 3 channels supported if output format is stacks of tiffs 2D
    ubuffer = new iim::uint8 *[supported_channels];
    memset(ubuffer,0,supported_channels*sizeof(iim::uint8 *)); // initializes to null pointers

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,slice_height,slice_width,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,slice_height,slice_width,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),z,z_parts);
        }
        else {
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {

        // save previous group data
        saveVCResumerState(fhandle,z,z_parts);

        // fill one slice block
        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
            // WARNING: next code assumes that channels is 1 or 3, but implementations of loadSubvolume_to_UINT8 do not guarantee this condition
            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            // code has been changed because the load operation can return 1, 2 or 3 channels
            /*
            if ( supported_channels == 3 ) {
                // offsets are to be computed taking into account that buffer size along D may be different
                ubuffer[1] = ubuffer[0] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
                ubuffer[2] = ubuffer[1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
            }
            */
            // elements of ubuffer not set are null pointers
            for ( int c=1; c<channels; c++ )
                ubuffer[c] = ubuffer[c-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
        }

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTiles(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //storing in 'base_path' the absolute path of the directory that will contain all stacks
                std::stringstream base_path;
                if ( par_mode ) // 2016-04-12. Giulio. directory name depends on the depth of the whole volume
                    base_path << output_path << "/RES(" << (int)(whole_height/powInt(2,i)) << "x" <<
                                 (int)(whole_width/powInt(2,i)) << "x" << (int)(whole_depth/powInt(2,halve_pow2[i])) << ")/";
                else
                    base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                                 (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,halve_pow2[i])) << ")/";

                //if frame_dir not empty must create frame directory
                if ( frame_dir != "" ) {
                    base_path << frame_dir << "/";
                    if(!check_and_make_dir(base_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
                    }
                }

                //looping on new stacks
                for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                {
                    //incrementing end_height
                    end_height = start_height + stacks_height[i][stack_row][0]-1;

                    //computing V_DIR_path and creating the directory the first time it is needed
                    std::stringstream V_DIR_path;
                    V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
                    }

                    for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                    {
                        end_width  = start_width  + stacks_width [i][stack_row][stack_column]-1;

                        //computing H_DIR_path and creating the directory the first time it is needed
                        std::stringstream H_DIR_path;
                        H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                        if(z==D0 && !check_and_make_dir(H_DIR_path.str().c_str()))
                        {
                            char err_msg[STATIC_STRINGS_SIZE];
                            sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                            throw IOException(err_msg);
                        }

                        //saving HERE
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++)
                        {
                            std::stringstream img_path;
                            std::stringstream abs_pos_z;
                            abs_pos_z.width(6);
                            abs_pos_z.fill('0');
                            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                            abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                               (powInt(2,halve_pow2[i])*buffer_z + z - D0) * volume->getVXL_D() * 10);
                            img_path << H_DIR_path.str() << "/"
                                     << this->getMultiresABS_V_string(i,start_height) << "_"
                                     << this->getMultiresABS_H_string(i,start_width) << "_"
                                     << abs_pos_z.str();
                            if ( internal_rep == REAL_INTERNAL_REP )
                                VirtualVolume::saveImage(img_path.str(),
                                                         rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                                         (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                         start_height,end_height,start_width,end_width,
                                                         saved_img_format, saved_img_depth);
                            else // internal_rep == UINT8_INTERNAL_REP
                                if ( channels == 1 )
                                    VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // adds the stride
                                            (iim::uint8 *) 0,
                                            (iim::uint8 *) 0,
                                            (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                            start_height,end_height,start_width,end_width,
                                            saved_img_format, saved_img_depth);
                                else if ( channels == 2 )
                                    VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                            ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                            (iim::uint8 *) 0,
                                            (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                            start_height,end_height,start_width,end_width,
                                            saved_img_format, saved_img_depth);
                                else // channels = 3
                                    VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                        ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                            ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                            ubuffer[2] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                            (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                            start_height,end_height,start_width,end_width,
                                            saved_img_format, saved_img_depth);
                        }
                        start_width  += stacks_width [i][stack_row][stack_column];
                    }
                    start_height += stacks_height[i][stack_row][0];
                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets

        // save next group data
        saveVCResumerState(fhandle,z+z_max_res,z_parts+1);
    }

    // save last group data
    saveVCResumerState(fhandle,z+z_max_res,z_parts+1);

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-09. Giulio. @FIXED If input volume is 3D the input plugin cannot be used to generate the meta data file.
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        //try {
        //	// test if it is a 2D plugin
        //	bool temp = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved();
        //}
        //catch(iom::exception & ex){
        //	if ( strstr(ex.what(),"it is not a 2D I/O plugin") ) // it is not a 2D plugin
        //	// reset input plugin so the StackedVolume constructor set it correctly
        //	iom::IMIN_PLUGIN = "empty";
        //}
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for(int res_i=0; res_i< resolutions_size; res_i++) {
            if(resolutions[res_i])
            {
                //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                //system.
                //---- Giulio 2013-08-23 fixed
                StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                       volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));

                //			StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                //							volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
    }
}


/*************************************************************************************************************
* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTilesSimple(std::string output_path, bool* resolutions, 
                                          int slice_height, int slice_width, int method, bool isotropic, bool show_progress_bar,
                                          const char* saved_img_format, int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::generateTilesSimple(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", slice_height = %d, slice_width = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           slice_height, slice_width, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default value: output depth is the same of input depth
        saved_img_depth = (volume->getBYTESxCHAN() * 8);

    // 	if ( saved_img_depth != 8 && volume->getNACtiveChannels() > 1) {
    // 		char err_msg[STATIC_STRINGS_SIZE];
    // 		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d bits per channel of destination is not supported for %d channels",
    // 			saved_img_depth, volume->getNACtiveChannels());
    // 		throw IOException(err_msg);
    // 	}

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) { // saveImage_from and saveImage_from_UINT8 do not support depth conversion yet
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesSimple: a mismatch between bits per channel of source (%d) and destination (%d) is not supported",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    int supported_channels;     //channels to be supported (stacks of tiffs 2D only supports 1 or 3 channels)
    // real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT], n_stacks_H[TMITREE_MAX_HEIGHT];             //array of number of tiles along V and H directions respectively at i-th resolution
    int **stacks_height[TMITREE_MAX_HEIGHT], **stacks_width[TMITREE_MAX_HEIGHT];	//array of matrices of tiles dimensions at i-th resolution
    std::stringstream file_path[TMITREE_MAX_HEIGHT];                            //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesSimple: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff2D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff2D";
    //}

    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    //initializing the progress bar
    char progressBarMsg[200];
    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // test, if any, should be done on V0, V1, ...
    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //	char err_msg[5000];
    //	sprintf(err_msg,"in VolumeConverter::generateTiles(...): block_depth is not set in parallel mode");
    //	throw iom::exception(err_msg);
    //}

    //activating resolutions
    slice_height = (slice_height == -1 ? (int)height : slice_height);
    slice_width  = (slice_width  == -1 ? (int)width  : slice_width);
    if(slice_height < TMITREE_MIN_BLOCK_DIM || slice_width < TMITREE_MIN_BLOCK_DIM)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesSimple: The minimum dimension for both slice width and height is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }
    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = 1; //(int) ceil ( (height/powInt(2,res_i)) / (float) slice_height );
        n_stacks_H[res_i] = 1; //(int) ceil ( (width/powInt(2,res_i))  / (float) slice_width  );
        stacks_height[res_i] = new int *[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                stacks_width [res_i][stack_row][stack_col] = ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
            }
        }
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<whole_height/powInt(2,res_i)<<"x"<<whole_width/powInt(2,res_i)<<"x"<<whole_depth/powInt(2,halve_pow2[res_i])<<")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesSimple(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }

                //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
                if ( frame_dir != "" ) {
                    file_path[res_i] << "/" << frame_dir << "/";
                    if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                        if(!check_and_make_dir(file_path[res_i].str().c_str()))
                        {
                            char err_msg[STATIC_STRINGS_SIZE];
                            sprintf(err_msg, "in generateTilesSimple(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                            throw IOException(err_msg);
                        }
                    }
                }
            }
        }
    }

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,halve_pow2[resolutions_size-1]);
    z_ratio=depth/z_max_res;

    // check the number of channels
    if ( channels > 3 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in generateTilesSimple(...): the volume contains too many channels (%d)", channels);
        throw IOException(err_msg);
    }

    //allocated even if not used
    org_channels = channels; // save for checks
    supported_channels = (channels>1) ? 3 : 1; // only 1 or 3 channels supported if output format is stacks of tiffs 2D
    ubuffer = new iim::uint8 *[supported_channels];
    memset(ubuffer,0,supported_channels*sizeof(iim::uint8 *)); // initializes to null pointers

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesSimple(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,slice_height,slice_width,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,slice_height,slice_width,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),z,z_parts);
        }
        else {
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {
        // save previous group data
        saveVCResumerState(fhandle,z,z_parts);

        // fill one slice block
        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
            // WARNING: next code assumes that channels is 1 or 3, but implementations of loadSubvolume_to_UINT8 do not guarantee this condition
            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            // code has been changed because the load operation can return 1, 2 or 3 channels
            /*
            if ( supported_channels == 3 ) {
                // offsets are to be computed taking into account that buffer size along D may be different
                ubuffer[1] = ubuffer[0] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
                ubuffer[2] = ubuffer[1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
            }
            */
            // elements of ubuffer not set are null pointers
            for ( int c=1; c<channels; c++ )
                ubuffer[c] = ubuffer[c-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
        }

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesSimple(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //storing in 'base_path' the absolute path of the directory that will contain all stacks
                std::stringstream base_path;
                if ( par_mode ) // 2016-04-12. Giulio. directory name depends on the depth of the whole volume
                    base_path << output_path << "/RES(" << (int)(whole_height/powInt(2,i)) << "x" <<
                                 (int)(whole_width/powInt(2,i)) << "x" << (int)(whole_depth/powInt(2,halve_pow2[i])) << ")/";
                else
                    base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                                 (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,halve_pow2[i])) << ")/";

                //if frame_dir not empty must create frame directory
                if ( frame_dir != "" ) {
                    base_path << frame_dir << "/";
                    if(!check_and_make_dir(base_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesSimple(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
                    }
                }

                int start_height = 0;
                int end_height = start_height + stacks_height[i][0][0]-1;
                ////looping on new stacks
                //for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                //{
                //	//incrementing end_height
                //	end_height = start_height + stacks_height[i][stack_row][0]-1;
                //
                //	//computing V_DIR_path and creating the directory the first time it is needed
                //	std::stringstream V_DIR_path;
                //	V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                //                if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                //	{
                //                    char err_msg[STATIC_STRINGS_SIZE];
                //		sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                //                    throw IOException(err_msg);
                //	}

                int start_width=0;
                int end_width  = start_width  + stacks_width [i][0][0]-1;
                //for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                //{
                //	end_width  = start_width  + stacks_width [i][stack_row][stack_column]-1;
                //
                //	//computing H_DIR_path and creating the directory the first time it is needed
                //	std::stringstream H_DIR_path;
                //	H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                //                   if(z==D0 && !check_and_make_dir(H_DIR_path.str().c_str()))
                //	{
                //                       char err_msg[STATIC_STRINGS_SIZE];
                //		sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                //                       throw IOException(err_msg);
                //	}

                //saving HERE
                for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++)
                {
                    std::stringstream img_path;
                    std::stringstream abs_pos_z;
                    abs_pos_z.width(6);
                    abs_pos_z.fill('0');
                    // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                    abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                       (powInt(2,halve_pow2[i])*buffer_z + z - D0) * volume->getVXL_D() * 10);
                    //img_path << H_DIR_path.str() << "/"
                    img_path << base_path.str() << "/"
                             << this->getMultiresABS_V_string(i,start_height) << "_"
                             << this->getMultiresABS_H_string(i,start_width) << "_"
                             << abs_pos_z.str();
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::saveImage(img_path.str(),
                                                 rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                                 (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                 start_height,end_height,start_width,end_width,
                                                 saved_img_format, saved_img_depth);
                    else // internal_rep == UINT8_INTERNAL_REP
                        if ( channels == 1 )
                            VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // adds the stride
                                    (iim::uint8 *) 0,
                                    (iim::uint8 *) 0,
                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                    start_height,end_height,start_width,end_width,
                                    saved_img_format, saved_img_depth);
                        else if ( channels == 2 )
                            VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                    ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                    (iim::uint8 *) 0,
                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                    start_height,end_height,start_width,end_width,
                                    saved_img_format, saved_img_depth);
                        else // channels = 3
                            VirtualVolume::saveImage_from_UINT8(img_path.str(),
                                                                ubuffer[0] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                    ubuffer[1] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                    ubuffer[2] + buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan, // stride to be added for slice buffer_z
                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                    start_height,end_height,start_width,end_width,
                                    saved_img_format, saved_img_depth);
                }
                //start_width  += stacks_width [i][stack_row][stack_column];
                //}
                //start_height += stacks_height[i][stack_row][0];
                //}
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets
    }

    // save last group data
    saveVCResumerState(fhandle,z+z_max_res,z_parts+1);

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        //	// reloads created volumes to generate .bin file descriptors at all resolutions
        //	ref_sys reference(axis(1),axis(2),axis(3));
        //	TiledMCVolume *mcprobe;
        //	TiledVolume   *tprobe;
        //	StackedVolume *sprobe;
        //	sprobe = dynamic_cast<StackedVolume *>(volume);
        //	if ( sprobe ) {
        //		reference.first  = sprobe->getAXS_1();
        //		reference.second = sprobe->getAXS_2();
        //		reference.third  = sprobe->getAXS_3();
        //	}
        //	else {
        //		tprobe = dynamic_cast<TiledVolume *>(volume);
        //		if ( tprobe ) {
        //			reference.first  = tprobe->getAXS_1();
        //			reference.second = tprobe->getAXS_2();
        //			reference.third  = tprobe->getAXS_3();
        //		}
        //		else {
        //			mcprobe = dynamic_cast<TiledMCVolume *>(volume);
        //			if ( mcprobe ) {
        //				reference.first  = mcprobe->getAXS_1();
        //				reference.second = mcprobe->getAXS_2();
        //				reference.third  = mcprobe->getAXS_3();
        //			}
        //		}
        //	}

        //	// 2016-04-09. Giulio. @FIXED If input volume is 3D the input plugin cannot be used to generate the meta data file.
        //	std::string save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        //	//try {
        //	//	// test if it is a 2D plugin
        //	//	bool temp = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved();
        //	//}
        //	//catch(iom::exception & ex){
        //	//	if ( strstr(ex.what(),"it is not a 2D I/O plugin") ) // it is not a 2D plugin
        //	//	// reset input plugin so the StackedVolume constructor set it correctly
        //	//	iom::IMIN_PLUGIN = "empty";
        //	//}
        //	// 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        //	iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        //	for(int res_i=0; res_i< resolutions_size; res_i++) {
        //		if(resolutions[res_i])
        //		{
        //			//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
        //			//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
        //			//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
        //			//system.
        //			//---- Giulio 2013-08-23 fixed
        //			StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
        //							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

        ////			StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
        ////							volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
        //		}
        //	}

        //	// restore input plugin
        //	iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    //// deallocate memory
    //   for(int res_i=0; res_i< resolutions_size; res_i++)
    //{
    //	for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
    //	{
    //		delete []stacks_height[res_i][stack_row];
    //		delete []stacks_width [res_i][stack_row];
    //	}
    //	delete []stacks_height[res_i];
    //	delete []stacks_width[res_i];
    //}
}


/*************************************************************************************************************
* Method to be called for tile generation in Vaa3D raw format. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [block_height]	    : desired dimensions of tiled  blocks after merging.  It is actually an upper-bound of
* [block_width]			  the actual slice dimensions, which will be computed in such a way that all tiles di-
* [block_depth]			  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTilesVaa3DRaw(std::string output_path, bool* resolutions, 
                                            int block_height, int block_width, int block_depth, int method, bool isotropic,
                                            bool show_progress_bar, const char* saved_img_format,
                                            int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    //    printf("in VolumeConverter::generateTilesVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    //    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
    //        printf("%d", resolutions[i]);
    //    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
    //           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    /* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
     * In the following the term 'group' means the groups of slices that are
     * processed together to generate slices of all resolution requested
     */

    /* stack_block[i] is the index of current block along z (it depends on the resolution i)
     * current block is the block in which falls the first slice of the group
     * of slices that is currently being processed, i.e. from z to z+z_max_res-1
     */
    int stack_block[TMITREE_MAX_HEIGHT];

    /* these arrays are the indices of first and last slice of current block at resolution i
     * WARNING: the slice index refers to the index of the slice in the volume at resolution i
     */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

    /* the number of slice of already processed groups at current resolution
     * the index of the slice to be saved at current resolution is:
     *
     *      n_slices_pred + z_buffer
     */
    sint64 n_slices_pred;

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff3D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //initializing the progress bar
    char progressBarMsg[200];

    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    // test, if any, should be done on V0, V1, ...
    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //   char err_msg[5000];
    //    sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): block_depth is not set in parallel mode");
    //    throw iom::exception(err_msg);
    //}

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): the minimum dimension for block height and width is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
    {
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
        {
            if(resolutions[i])
            {
                resolutions_size = std::max(resolutions_size, i+1);
            }
        }
    }

    //cout<<"size ... "<<width<<" "<<height<<" "<<depth<<endl;
    //cout<<"block size ... "<<block_width<<" "<<block_height<<" "<<block_depth<<endl;
    //cout<<"resolution size ... "<<resolutions_size<<endl;

    float w = width;
    float h = height;
    float d = depth;
    long n = 1;
    for(size_t i=0; i<resolutions_size; i++)
    {
        w *= 0.5;
        h *= 0.5;
        d *= 0.5;

        if(w>=1 && h>=1 && d>=1)
        {
            n++;
        }
        else
        {
            break;
        }
    }

    if(n<resolutions_size)
        resolutions_size = n;

    //cout<<"adjusted resolution size ... "<<resolutions_size<<endl;

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //    for ( int i=0; i<resolutions_size; i++ )
    //        cout<<" halve_pow2 "<<halve_pow2[i]<<" at "<<resolutions[i]<<endl;

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,halve_pow2[res_i]))  / (float) block_depth  );

        //cout<<"res "<<res_i<<endl;
        //cout<<"n_stacks "<<n_stacks_V[res_i]<<" "<<n_stacks_H[res_i]<<" "<<n_stacks_D[res_i]<<endl;

        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,halve_pow2[res_i])))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,halve_pow2[res_i])))  % n_stacks_D[res_i] ? 1:0);


                    //                    if(stack_row==0 && stack_col==0 && stack_sli==0)
                    //                    {
                    //                        cout<<"res "<<res_i<<" "<<stack_row<<" "<<stack_col<<" "<<stack_sli<<endl;
                    //                        cout<<"stacks "<<stacks_height[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_width[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_depth[res_i][stack_row][stack_col][stack_sli]<<endl;
                    //                    }
                }
            }
        }

        //cout<<"par_mode "<<(par_mode?"true":"false")<<endl;

        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<whole_height/powInt(2,res_i)<<"x"<<whole_width/powInt(2,res_i)<<"x"<<whole_depth/powInt(2,halve_pow2[res_i])<<")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }
        }
    }

    //cout<<"created top resolution folders \n";

    /* The following check verifies that the numeber of slices in the buffer is not higher than the number of slices in a block file
     * (excluding the last block in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
     * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block
     * file at that resolution as computed by the above code is:
     *
     *                                                B = floor( H / ceil( H/D ) )
     * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
     *
     *                                                  D >= B >= floor(D/2)
     * since it is:
     *
     *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
     * where alpha<1.
     */

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = std::max(std::min(STANDARD_BLOCK_DEPTH,block_depth/2),powInt(2,halve_pow2[resolutions_size-1]));
    if ( (z_max_res > 1) && z_max_res > block_depth/2 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
        throw IOException(err_msg);
    }
    z_ratio=depth/z_max_res;

    cout<<"z_max_res "<<z_max_res<<endl;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8 *));
    org_channels = channels; // save for checks

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
        }
        else {
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {
        // save previous group data
        saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z,z_parts);

        //if ( z > (this->D1/2) ) {
        //	closeResumer(fhandle);
        //	throw IOExcpetion("interruption for test");
        //}

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_START(ConverterLoadBlockOperation)
        #endif

                // fill one slice block
                //cout<<"REAL_INTERNAL_REP "<<(internal_rep == REAL_INTERNAL_REP?"true":"false")<<endl;

                if ( internal_rep == REAL_INTERNAL_REP )
                rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            //ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto start = std::chrono::high_resolution_clock::now();

            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto end = std::chrono::high_resolution_clock::now();

            cout<<"loadSubvolume_to_UINT8 takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;

            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"in generateTilesVaa3DRaw(...): the volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_STOP(ConverterLoadBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: loaded image block x(%d-%d), y(%d-%d), z(%d-%d)",H0, H1, V0, V1, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))))
        #endif

                //updating the progress bar
                if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        auto start = std::chrono::high_resolution_clock::now();

        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            // check if current block is changed
            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
            if ( ((z - this->D0) / powInt(2,halve_pow2[i])) > slice_end[i] ) {
                stack_block[i]++;
                slice_start[i] = slice_end[i] + 1;
                slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
            }

            // find abs_pos_z at resolution i
            std::stringstream abs_pos_z;
            abs_pos_z.width(6);
            abs_pos_z.fill('0');
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
            abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                               (powInt(2,halve_pow2[i])*slice_start[i]) * volume->getVXL_D() * 10);
            //abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
            //					- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
            //                  (powInt(2,i)*slice_start[i]) * volume->getVXL_D());

            //compute the number of slice of previous groups at resolution i
            //note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,halve_pow2[i]);

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //storing in 'base_path' the absolute path of the directory that will contain all stacks
                std::stringstream base_path;
                if ( par_mode ) // 2016-04-12. Giulio. directory name depends on the depth of the whole volume
                    base_path << output_path << "/RES(" << (int)(whole_height/powInt(2,i)) << "x" <<
                                 (int)(whole_width/powInt(2,i)) << "x" << (int)(whole_depth/powInt(2,halve_pow2[i])) << ")/";
                else
                    base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                                 (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,halve_pow2[i])) << ")/";

                //if frame_dir not empty must create frame directory
                if ( frame_dir != "" ) {
                    base_path << frame_dir << "/";
                    if(!check_and_make_dir(base_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
                    }
                }

                //cout<<"base_path "<<base_path.str()<<endl;

                //looping on new stacks
                for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                {
                    //incrementing end_height
                    end_height = start_height + stacks_height[i][stack_row][0][0]-1;

                    //computing V_DIR_path and creating the directory the first time it is needed
                    std::stringstream V_DIR_path;
                    V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);

                    //cout<<"V_DIR_path "<<V_DIR_path.str()<<endl;

                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
                    }

                    for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                    {
                        end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

                        //computing H_DIR_path and creating the directory the first time it is needed
                        std::stringstream H_DIR_path;
                        H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                        //cout<<"H_DIR_path "<<H_DIR_path.str()<<endl;

                        if ( z==D0 ) {
                            if(!check_and_make_dir(H_DIR_path.str().c_str()))
                            {
                                char err_msg[STATIC_STRINGS_SIZE];
                                sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                throw IOException(err_msg);
                            }
                            else { // the directory has been created for the first time
                                // initialize block files
                                V3DLONG *sz = new V3DLONG[4];
                                int datatype;
                                char *err_rawfmt;

                                sz[0] = stacks_width[i][stack_row][stack_column][0];
                                sz[1] = stacks_height[i][stack_row][stack_column][0];
                                sz[3] = channels;

                                if ( internal_rep == REAL_INTERNAL_REP )
                                    datatype = 4;
                                else if ( internal_rep == UINT8_INTERNAL_REP ) {
                                    if ( saved_img_depth == 16 )
                                        datatype = 2;
                                    else if ( saved_img_depth == 8 )
                                        datatype = 1;
                                    else {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
                                        throw IOException(err_msg);
                                    }
                                }
                                else {
                                    char err_msg[STATIC_STRINGS_SIZE];
                                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
                                    throw IOException(err_msg);
                                }

                                int slice_start_temp = 0;
                                for ( int j=0; j < n_stacks_D[i]; j++ ) {
                                    sz[2] = stacks_depth[i][stack_row][stack_column][j];

                                    std::stringstream abs_pos_z_temp;
                                    abs_pos_z_temp.width(6);
                                    abs_pos_z_temp.fill('0');
                                    // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                    abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                            (powInt(2,halve_pow2[i])*(slice_start_temp)) * volume->getVXL_D() * 10);

                                    std::stringstream img_path_temp;
                                    img_path_temp << H_DIR_path.str() << "/"
                                                  << this->getMultiresABS_V_string(i,start_height) << "_"
                                                  << this->getMultiresABS_H_string(i,start_width) << "_"
                                                  << abs_pos_z_temp.str();

                                    //if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
                                    if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
                                           ( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(int)sz[0],(int)sz[1],(int)sz[2],(int)sz[3],datatype)) != 0 ) :
                                           ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
                                        throw IOException(err_msg);
                                    };

                                    slice_start_temp += (int)sz[2];
                                }
                                delete [] sz;
                            }
                        }

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_START(ConverterWriteBlockOperation)
        #endif

                                //saving HERE

                                // 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
                                std::stringstream partial_img_path;
                        partial_img_path << H_DIR_path.str() << "/"
                                         << this->getMultiresABS_V_string(i,start_height) << "_"
                                         << this->getMultiresABS_H_string(i,start_width) << "_";

                        int slice_ind = (int)(n_slices_pred - slice_start[i]);

                        std::stringstream img_path;
                        img_path << partial_img_path.str() << abs_pos_z.str();

                        //cout<<"img_path "<<img_path.str()<<endl;

                        /* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
                         * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations,
                         * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at
                         * end of the file.
                         * The number of pages of block files of interest can be easily computed as:
                         *
                         *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]]
                         *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1]
                         */

                        void *fhandle = 0;
                        int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
                        bool block_changed = false;                                 // true if block is changed executing the next for cycle
                        // fhandle = open file corresponding to current block
                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                            openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle,true);

                        // WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++, slice_ind++)
                        {
                            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                            if ( ((z - this->D0) / powInt(2,halve_pow2[i]) + buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
                                std::stringstream abs_pos_z_next;
                                abs_pos_z_next.width(6);
                                abs_pos_z_next.fill('0');
                                // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                        (powInt(2,halve_pow2[i])*(slice_end[i]+1)) * volume->getVXL_D() * 10);
                                img_path.str("");
                                img_path << partial_img_path.str() << abs_pos_z_next.str();

                                slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

                                // close(fhandle) i.e. file corresponding to current block
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    closeTiff3DFile(fhandle);
                                // fhandle = open file corresponding to next block
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle,true);
                                n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
                                block_changed = true;
                            }

                            if ( internal_rep == REAL_INTERNAL_REP )
                                VirtualVolume::saveImage_to_Vaa3DRaw(
                                            slice_ind,
                                            img_path.str(),
                                            rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                            (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                            start_height,end_height,start_width,end_width,
                                            saved_img_format, saved_img_depth
                                            );
                            else // internal_rep == UINT8_INTERNAL_REP
                                if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
                                    VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
                                                slice_ind,
                                                img_path.str(),
                                                ubuffer,
                                                channels,
                                                buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                start_height,end_height,start_width,end_width,
                                                saved_img_format, saved_img_depth,fhandle,n_pages_block,false);

                                    //(int slice, std::string img_path, uint8** raw_ch, int n_chans, sint64 offset,
                                    // int raw_img_height, int raw_img_width, int start_height, int end_height, int start_width,
                                    // int end_width, const char* img_format, int img_depth, void *fhandle, int n_pages, bool do_open )
                                    //cout<<"saved ... "<<img_path.str()<<" offset "<<buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan<<" "<<slice_ind<<" "
                                    //   <<start_height<<" "<<end_height<<" "<<start_width<<" "<<end_width<<" "<<n_pages_block<<" "<<saved_img_depth<<endl;
                                }
                                else { // can be only Vaa3DRaw
                                    VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
                                                slice_ind,
                                                img_path.str(),
                                                ubuffer,
                                                channels,
                                                buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                start_height,end_height,start_width,end_width,
                                                saved_img_format, saved_img_depth);
                                }
                        }

                        // close(fhandle) i.e. currently opened file
                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                            closeTiff3DFile(fhandle);

                        start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_STOP(ConverterWriteBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: written multiresolution image block x(%d-%d), y(%d-%d), z(%d-%d)",start_width, end_width, start_height, end_height, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))))
        #endif
                    }
                    start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets

        //
        auto end = std::chrono::high_resolution_clock::now();

        cout<<"writing chunk images takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;
    }

    // save last group data
    saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);

    int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for(int res_i=0; res_i< resolutions_size; res_i++)
        {
            if(resolutions[res_i])
            {
                //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                //system.
                try {
                    TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                         volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));
                    //volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
                catch (IOException & ex)
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                    n_err++;
                }
                catch ( ... )
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
                    n_err++;
                }

                //          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                //                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                delete []stacks_height[res_i][stack_row][stack_col];
                delete []stacks_width [res_i][stack_row][stack_col];
                delete []stacks_depth [res_i][stack_row][stack_col];
            }
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
            delete []stacks_depth [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
        delete []stacks_depth[res_i];
    }

    if ( n_err ) { // errors in mdat.bin creation
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
    }
}

// multithreading
void VolumeConverter::generateTilesMT(std::string output_path, bool* resolutions,
                                      int block_height, int block_width, int block_depth, int method, bool isotropic,
                                      bool show_progress_bar, const char* saved_img_format,
                                      int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    //    printf("in VolumeConverter::generateTilesVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    //    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
    //        printf("%d", resolutions[i]);
    //    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
    //           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    /* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
         * In the following the term 'group' means the groups of slices that are
         * processed together to generate slices of all resolution requested
         */

    /* stack_block[i] is the index of current block along z (it depends on the resolution i)
         * current block is the block in which falls the first slice of the group
         * of slices that is currently being processed, i.e. from z to z+z_max_res-1
         */
    int stack_block[TMITREE_MAX_HEIGHT];

    /* these arrays are the indices of first and last slice of current block at resolution i
         * WARNING: the slice index refers to the index of the slice in the volume at resolution i
         */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

    /* the number of slice of already processed groups at current resolution
         * the index of the slice to be saved at current resolution is:
         *
         *      n_slices_pred + z_buffer
         */
    sint64 n_slices_pred;

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff3D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //initializing the progress bar
    char progressBarMsg[200];

    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    // test, if any, should be done on V0, V1, ...
    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //   char err_msg[5000];
    //    sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): block_depth is not set in parallel mode");
    //    throw iom::exception(err_msg);
    //}

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): the minimum dimension for block height and width is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
    {
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
        {
            if(resolutions[i])
            {
                resolutions_size = std::max(resolutions_size, i+1);
            }
        }
    }

    //cout<<"size ... "<<width<<" "<<height<<" "<<depth<<endl;
    //cout<<"block size ... "<<block_width<<" "<<block_height<<" "<<block_depth<<endl;
    //cout<<"resolution size ... "<<resolutions_size<<endl;

    float w = width;
    float h = height;
    float d = depth;
    long n = 1;
    for(size_t i=0; i<resolutions_size; i++)
    {
        w *= 0.5;
        h *= 0.5;
        d *= 0.5;

        if(w>=1 && h>=1 && d>=1)
        {
            n++;
        }
        else
        {
            break;
        }
    }

    if(n<resolutions_size)
        resolutions_size = n;

    //cout<<"adjusted resolution size ... "<<resolutions_size<<endl;

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //    for ( int i=0; i<resolutions_size; i++ )
    //        cout<<" halve_pow2 "<<halve_pow2[i]<<" at "<<resolutions[i]<<endl;

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,halve_pow2[res_i]))  / (float) block_depth  );

        //cout<<"res "<<res_i<<endl;
        //cout<<"n_stacks "<<n_stacks_V[res_i]<<" "<<n_stacks_H[res_i]<<" "<<n_stacks_D[res_i]<<endl;

        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,halve_pow2[res_i])))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,halve_pow2[res_i])))  % n_stacks_D[res_i] ? 1:0);


                    //                    if(stack_row==0 && stack_col==0 && stack_sli==0)
                    //                    {
                    //                        cout<<"res "<<res_i<<" "<<stack_row<<" "<<stack_col<<" "<<stack_sli<<endl;
                    //                        cout<<"stacks "<<stacks_height[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_width[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_depth[res_i][stack_row][stack_col][stack_sli]<<endl;
                    //                    }
                }
            }
        }

        //cout<<"par_mode "<<(par_mode?"true":"false")<<endl;

        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<whole_height/powInt(2,res_i)<<"x"<<whole_width/powInt(2,res_i)<<"x"<<whole_depth/powInt(2,halve_pow2[res_i])<<")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }
        }
    }

    //cout<<"created top resolution folders \n";

    /* The following check verifies that the numeber of slices in the buffer is not higher than the number of slices in a block file
         * (excluding the last block in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
         * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block
         * file at that resolution as computed by the above code is:
         *
         *                                                B = floor( H / ceil( H/D ) )
         * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
         *
         *                                                  D >= B >= floor(D/2)
         * since it is:
         *
         *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
         * where alpha<1.
         */

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = std::max(std::min(STANDARD_BLOCK_DEPTH,block_depth/2),powInt(2,halve_pow2[resolutions_size-1]));
    if ( (z_max_res > 1) && z_max_res > block_depth/2 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
        throw IOException(err_msg);
    }
    z_ratio=depth/z_max_res;

    cout<<"z_max_res "<<z_max_res<<endl;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8 *));
    org_channels = channels; // save for checks

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
        }
        else {
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {
        // save previous group data
        saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z,z_parts);

        //if ( z > (this->D1/2) ) {
        //	closeResumer(fhandle);
        //	throw IOExcpetion("interruption for test");
        //}

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_START(ConverterLoadBlockOperation);
#endif

        // fill one slice block
        //cout<<"REAL_INTERNAL_REP "<<(internal_rep == REAL_INTERNAL_REP?"true":"false")<<endl;

        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            //ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto start = std::chrono::high_resolution_clock::now();

            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto end = std::chrono::high_resolution_clock::now();

            cout<<"loadSubvolume_to_UINT8 takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;

            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"in generateTilesVaa3DRaw(...): the volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_STOP(ConverterLoadBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: loaded image block x(%d-%d), y(%d-%d), z(%d-%d)",H0, H1, V0, V1, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))));
#endif

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        auto start = std::chrono::high_resolution_clock::now();

        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            // check if current block is changed
            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
            if ( ((z - this->D0) / powInt(2,halve_pow2[i])) > slice_end[i] ) {
                stack_block[i]++;
                slice_start[i] = slice_end[i] + 1;
                slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
            }

            // find abs_pos_z at resolution i
            std::stringstream abs_pos_z;
            abs_pos_z.width(6);
            abs_pos_z.fill('0');
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
            abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                               (powInt(2,halve_pow2[i])*slice_start[i]) * volume->getVXL_D() * 10);
            //abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
            //					- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
            //                  (powInt(2,i)*slice_start[i]) * volume->getVXL_D());

            //compute the number of slice of previous groups at resolution i
            //note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,halve_pow2[i]);

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //storing in 'base_path' the absolute path of the directory that will contain all stacks
                std::stringstream base_path;
                if ( par_mode ) // 2016-04-12. Giulio. directory name depends on the depth of the whole volume
                    base_path << output_path << "/RES(" << (int)(whole_height/powInt(2,i)) << "x" <<
                                 (int)(whole_width/powInt(2,i)) << "x" << (int)(whole_depth/powInt(2,halve_pow2[i])) << ")/";
                else
                    base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                                 (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,halve_pow2[i])) << ")/";

                //if frame_dir not empty must create frame directory
                if ( frame_dir != "" ) {
                    base_path << frame_dir << "/";
                    if(!check_and_make_dir(base_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
                    }
                }

                //cout<<"base_path "<<base_path.str()<<endl;

                //looping on new stacks
                for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                {
                    //incrementing end_height
                    end_height = start_height + stacks_height[i][stack_row][0][0]-1;

                    //computing V_DIR_path and creating the directory the first time it is needed
                    std::stringstream V_DIR_path;
                    V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);

                    //cout<<"V_DIR_path "<<V_DIR_path.str()<<endl;

                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
                    }

                    for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                    {
                        end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

                        //computing H_DIR_path and creating the directory the first time it is needed
                        std::stringstream H_DIR_path;
                        H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                        //cout<<"H_DIR_path "<<H_DIR_path.str()<<endl;

                        if ( z==D0 ) {
                            if(!check_and_make_dir(H_DIR_path.str().c_str()))
                            {
                                char err_msg[STATIC_STRINGS_SIZE];
                                sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                throw IOException(err_msg);
                            }
                            else { // the directory has been created for the first time
                                // initialize block files
                                V3DLONG *sz = new V3DLONG[4];
                                int datatype;
                                char *err_rawfmt;

                                sz[0] = stacks_width[i][stack_row][stack_column][0];
                                sz[1] = stacks_height[i][stack_row][stack_column][0];
                                sz[3] = channels;

                                if ( internal_rep == REAL_INTERNAL_REP )
                                    datatype = 4;
                                else if ( internal_rep == UINT8_INTERNAL_REP ) {
                                    if ( saved_img_depth == 16 )
                                        datatype = 2;
                                    else if ( saved_img_depth == 8 )
                                        datatype = 1;
                                    else {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
                                        throw IOException(err_msg);
                                    }
                                }
                                else {
                                    char err_msg[STATIC_STRINGS_SIZE];
                                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
                                    throw IOException(err_msg);
                                }

                                int slice_start_temp = 0;
                                for ( int j=0; j < n_stacks_D[i]; j++ ) {
                                    sz[2] = stacks_depth[i][stack_row][stack_column][j];

                                    std::stringstream abs_pos_z_temp;
                                    abs_pos_z_temp.width(6);
                                    abs_pos_z_temp.fill('0');
                                    // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                    abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                            (powInt(2,halve_pow2[i])*(slice_start_temp)) * volume->getVXL_D() * 10);

                                    std::stringstream img_path_temp;
                                    img_path_temp << H_DIR_path.str() << "/"
                                                  << this->getMultiresABS_V_string(i,start_height) << "_"
                                                  << this->getMultiresABS_H_string(i,start_width) << "_"
                                                  << abs_pos_z_temp.str();

                                    //if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
                                    if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
                                           ( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(int)sz[0],(int)sz[1],(int)sz[2],(int)sz[3],datatype)) != 0 ) :
                                           ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
                                        throw IOException(err_msg);
                                    };

                                    slice_start_temp += (int)sz[2];
                                }
                                delete [] sz;
                            }
                        }

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_START(ConverterWriteBlockOperation);
#endif

                        //saving HERE

                        // 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
                        std::stringstream partial_img_path;
                        partial_img_path << H_DIR_path.str() << "/"
                                         << this->getMultiresABS_V_string(i,start_height) << "_"
                                         << this->getMultiresABS_H_string(i,start_width) << "_";

                        int slice_ind = (int)(n_slices_pred - slice_start[i]);

                        std::stringstream img_path;
                        img_path << partial_img_path.str() << abs_pos_z.str();

                        //cout<<"img_path "<<img_path.str()<<endl;

                        /* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
                             * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations,
                             * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at
                             * end of the file.
                             * The number of pages of block files of interest can be easily computed as:
                             *
                             *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]]
                             *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1]
                             */

                        void *fhandle = 0;
                        int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
                        bool block_changed = false;                                 // true if block is changed executing the next for cycle
                        // fhandle = open file corresponding to current block
                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                            openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle,true);

                        // WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++, slice_ind++)
                        {
                            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                            if ( ((z - this->D0) / powInt(2,halve_pow2[i]) + buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
                                std::stringstream abs_pos_z_next;
                                abs_pos_z_next.width(6);
                                abs_pos_z_next.fill('0');
                                // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                        (powInt(2,halve_pow2[i])*(slice_end[i]+1)) * volume->getVXL_D() * 10);
                                img_path.str("");
                                img_path << partial_img_path.str() << abs_pos_z_next.str();

                                slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

                                // close(fhandle) i.e. file corresponding to current block
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    closeTiff3DFile(fhandle);
                                // fhandle = open file corresponding to next block
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle,true);
                                n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
                                block_changed = true;
                            }

                            if ( internal_rep == REAL_INTERNAL_REP )
                                VirtualVolume::saveImage_to_Vaa3DRaw(
                                            slice_ind,
                                            img_path.str(),
                                            rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                            (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                            start_height,end_height,start_width,end_width,
                                            saved_img_format, saved_img_depth
                                            );
                            else // internal_rep == UINT8_INTERNAL_REP
                                if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
                                    VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
                                                slice_ind,
                                                img_path.str(),
                                                ubuffer,
                                                channels,
                                                buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                start_height,end_height,start_width,end_width,
                                                saved_img_format, saved_img_depth,fhandle,n_pages_block,false);

                                    //(int slice, std::string img_path, uint8** raw_ch, int n_chans, sint64 offset,
                                    // int raw_img_height, int raw_img_width, int start_height, int end_height, int start_width,
                                    // int end_width, const char* img_format, int img_depth, void *fhandle, int n_pages, bool do_open )
                                    //cout<<"saved ... "<<img_path.str()<<" offset "<<buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan<<" "<<slice_ind<<" "
                                    //   <<start_height<<" "<<end_height<<" "<<start_width<<" "<<end_width<<" "<<n_pages_block<<" "<<saved_img_depth<<endl;
                                }
                                else { // can be only Vaa3DRaw
                                    VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
                                                slice_ind,
                                                img_path.str(),
                                                ubuffer,
                                                channels,
                                                buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                start_height,end_height,start_width,end_width,
                                                saved_img_format, saved_img_depth);
                                }
                        }

                        // close(fhandle) i.e. currently opened file
                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                            closeTiff3DFile(fhandle);

                        start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_STOP(ConverterWriteBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: written multiresolution image block x(%d-%d), y(%d-%d), z(%d-%d)",start_width, end_width, start_height, end_height, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))));
#endif
                    }
                    start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets

        //
        auto end = std::chrono::high_resolution_clock::now();

        cout<<"writing chunk images takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;
    }

    // save last group data
    saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);

    int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for(int res_i=0; res_i< resolutions_size; res_i++)
        {
            if(resolutions[res_i])
            {
                //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                //system.
                try {
                    TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                         volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));
                    //volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
                catch (IOException & ex)
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                    n_err++;
                }
                catch ( ... )
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
                    n_err++;
                }

                //          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                //                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                delete []stacks_height[res_i][stack_row][stack_col];
                delete []stacks_width [res_i][stack_row][stack_col];
                delete []stacks_depth [res_i][stack_row][stack_col];
            }
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
            delete []stacks_depth [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
        delete []stacks_depth[res_i];
    }

    if ( n_err ) { // errors in mdat.bin creation
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
    }
}


//
void VolumeConverter::generate3DTiles(std::string output_path, bool* resolutions,
                                      int block_height, int block_width, int block_depth, int method, bool isotropic,
                                      bool show_progress_bar, const char* saved_img_format,
                                      int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    //    printf("in VolumeConverter::generateTilesVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    //    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
    //        printf("%d", resolutions[i]);
    //    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
    //           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    /* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
     * In the following the term 'group' means the groups of slices that are
     * processed together to generate slices of all resolution requested
     */

    /* stack_block[i] is the index of current block along z (it depends on the resolution i)
     * current block is the block in which falls the first slice of the group
     * of slices that is currently being processed, i.e. from z to z+z_max_res-1
     */
    int stack_block[TMITREE_MAX_HEIGHT];

    /* these arrays are the indices of first and last slice of current block at resolution i
     * WARNING: the slice index refers to the index of the slice in the volume at resolution i
     */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

    /* the number of slice of already processed groups at current resolution
     * the index of the slice to be saved at current resolution is:
     *
     *      n_slices_pred + z_buffer
     */
    sint64 n_slices_pred;

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff3D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //initializing the progress bar
    char progressBarMsg[200];

    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    // test, if any, should be done on V0, V1, ...
    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //   char err_msg[5000];
    //    sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): block_depth is not set in parallel mode");
    //    throw iom::exception(err_msg);
    //}

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::generateTilesVaa3DRaw(...): the minimum dimension for block height and width is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //cout<<"size ... "<<width<<" "<<height<<" "<<depth<<endl;
    //cout<<"block size ... "<<block_width<<" "<<block_height<<" "<<block_depth<<endl;
    //cout<<"resolution size ... "<<resolutions_size<<endl;

    float w = width;
    float h = height;
    float d = depth;
    long n = 1;
    for(size_t i=0; i<resolutions_size; i++)
    {
        w *= 0.5;
        h *= 0.5;
        d *= 0.5;

        if(w>=1 && h>=1 && d>=1)
        {
            n++;
        }
        else
        {
            break;
        }
    }

    if(n<resolutions_size)
        resolutions_size = n;

    //cout<<"adjusted resolution size ... "<<resolutions_size<<endl;

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //    for ( int i=0; i<resolutions_size; i++ )
    //        cout<<" halve_pow2 "<<halve_pow2[i]<<" at "<<resolutions[i]<<endl;

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,halve_pow2[res_i]))  / (float) block_depth  );

        //cout<<"res "<<res_i<<endl;
        //cout<<"n_stacks "<<n_stacks_V[res_i]<<" "<<n_stacks_H[res_i]<<" "<<n_stacks_D[res_i]<<endl;

        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,halve_pow2[res_i])))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,halve_pow2[res_i])))  % n_stacks_D[res_i] ? 1:0);


                    //                    if(stack_row==0 && stack_col==0 && stack_sli==0)
                    //                    {
                    //                        cout<<"res "<<res_i<<" "<<stack_row<<" "<<stack_col<<" "<<stack_sli<<endl;
                    //                        cout<<"stacks "<<stacks_height[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_width[res_i][stack_row][stack_col][stack_sli]<<" "<<stacks_depth[res_i][stack_row][stack_col][stack_sli]<<endl;
                    //                    }
                }
            }
        }

        //cout<<"par_mode "<<(par_mode?"true":"false")<<endl;

        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<whole_height/powInt(2,res_i)<<"x"<<whole_width/powInt(2,res_i)<<"x"<<whole_depth/powInt(2,halve_pow2[res_i])<<")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }
        }
    }

    //cout<<"created top resolution folders \n";

    /* The following check verifies that the numeber of slices in the buffer is not higher than the number of slices in a block file
     * (excluding the last block in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
     * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block
     * file at that resolution as computed by the above code is:
     *
     *                                                B = floor( H / ceil( H/D ) )
     * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
     *
     *                                                  D >= B >= floor(D/2)
     * since it is:
     *
     *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
     * where alpha<1.
     */

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = std::max(std::min(STANDARD_BLOCK_DEPTH,block_depth/2),powInt(2,halve_pow2[resolutions_size-1]));
    if ( (z_max_res > 1) && z_max_res > block_depth/2 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
        throw IOException(err_msg);
    }
    z_ratio=depth/z_max_res;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8 *));
    org_channels = channels; // save for checks

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
        }
        else {
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute the file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {
        // save previous group data
        saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z,z_parts);

        //if ( z > (this->D1/2) ) {
        //	closeResumer(fhandle);
        //	throw IOExcpetion("interruption for test");
        //}

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_START(ConverterLoadBlockOperation);
#endif

        // fill one slice block
        //cout<<"REAL_INTERNAL_REP "<<(internal_rep == REAL_INTERNAL_REP?"true":"false")<<endl;

        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            //ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto start = std::chrono::high_resolution_clock::now();

            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);

            auto end = std::chrono::high_resolution_clock::now();

            cout<<"loadSubvolume_to_UINT8 takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;


            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"in generateTilesVaa3DRaw(...): the volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
//        if ( nbits ) {
//            //printf("----> lossy compression nbits = %d\n",nbits);
//            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
//            if ( bytes_chan == 1 ) {
//                iim::uint8 *ptr = ubuffer[0];
//                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
//                    *ptr = *ptr >> nbits << nbits;
//                }
//            }
//            else if ( bytes_chan == 2 ) {
//                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
//                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
//                    *ptr = *ptr >> nbits << nbits;
//                }
//            }
//        }

        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
        TERAFLY_TIME_STOP(ConverterLoadBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: loaded image block x(%d-%d), y(%d-%d), z(%d-%d)",H0, H1, V0, V1, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))));
#endif

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        auto start = std::chrono::high_resolution_clock::now();

        long nSkipped = 0;
        //std::vector<string> emptyDirs;

        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            int datatype;
            int *sz = new int [4];

            // check if current block is changed
            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
            if ( ((z - this->D0) / powInt(2,halve_pow2[i])) > slice_end[i] ) {
                stack_block[i]++;
                slice_start[i] = slice_end[i] + 1;
                slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
            }

            // find abs_pos_z at resolution i
            std::stringstream abs_pos_z;
            abs_pos_z.width(6);
            abs_pos_z.fill('0');
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
            abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                               (powInt(2,halve_pow2[i])*slice_start[i]) * volume->getVXL_D() * 10);
            //abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
            //					- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
            //                  (powInt(2,i)*slice_start[i]) * volume->getVXL_D());

            //compute the number of slice of previous groups at resolution i
            //note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,halve_pow2[i]);

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //storing in 'base_path' the absolute path of the directory that will contain all stacks
                std::stringstream base_path;
                if ( par_mode ) // 2016-04-12. Giulio. directory name depends on the depth of the whole volume
                    base_path << output_path << "/RES(" << (int)(whole_height/powInt(2,i)) << "x" <<
                                 (int)(whole_width/powInt(2,i)) << "x" << (int)(whole_depth/powInt(2,halve_pow2[i])) << ")/";
                else
                    base_path << output_path << "/RES(" << (int)(height/powInt(2,i)) << "x" <<
                                 (int)(width/powInt(2,i)) << "x" << (int)(depth/powInt(2,halve_pow2[i])) << ")/";

                //if frame_dir not empty must create frame directory
                if ( frame_dir != "" ) {
                    base_path << frame_dir << "/";
                    if(!check_and_make_dir(base_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", base_path.str().c_str());
                        throw IOException(err_msg);
                    }
                }

                //cout<<"base_path "<<base_path.str()<<endl;

                //looping on new stacks
                for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                {
                    //incrementing end_height
                    end_height = start_height + stacks_height[i][stack_row][0][0]-1;

                    //computing V_DIR_path and creating the directory the first time it is needed
                    std::stringstream V_DIR_path;
                    V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);

                    //cout<<"V_DIR_path "<<V_DIR_path.str()<<endl;

                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                        throw IOException(err_msg);
                    }

                    for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                    {
                        end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

                        //computing H_DIR_path and creating the directory the first time it is needed
                        std::stringstream H_DIR_path;
                        H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);

                        //cout<<"H_DIR_path "<<H_DIR_path.str()<<endl;

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_START(ConverterWriteBlockOperation);
#endif

                        //
//                        sz[0] = stacks_width[i][stack_row][stack_column][0];
//                        sz[1] = stacks_height[i][stack_row][stack_column][0];
//                        sz[3] = channels;

                        if ( internal_rep == REAL_INTERNAL_REP )
                        {
                            datatype = 4;
                        }
                        else if ( internal_rep == UINT8_INTERNAL_REP )
                        {
                            if ( saved_img_depth == 16 )
                                datatype = 2;
                            else if ( saved_img_depth == 8 )
                                datatype = 1;
                            else {
                                char err_msg[STATIC_STRINGS_SIZE];
                                sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
                                throw IOException(err_msg);
                            }
                        }
                        else
                        {
                            char err_msg[STATIC_STRINGS_SIZE];
                            sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
                            throw IOException(err_msg);
                        }

                        //saving HERE

                        bool folderCreated = false;

                        // 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
                        std::stringstream partial_img_path;
                        partial_img_path << H_DIR_path.str() << "/"
                                         << this->getMultiresABS_V_string(i,start_height) << "_"
                                         << this->getMultiresABS_H_string(i,start_width) << "_";

                        int slice_ind = (int)(n_slices_pred - slice_start[i]);

                        std::stringstream img_path;
                        img_path << partial_img_path.str() << abs_pos_z.str();

                        //cout<<"img_path "<<img_path.str()<<endl;

                        //int saveCount = 0;

                        /* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
                         * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations,
                         * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at
                         * end of the file.
                         * The number of pages of block files of interest can be easily computed as:
                         *
                         *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]]
                         *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1]
                         */

                        int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
                        bool block_changed = false;                                 // true if block is changed executing the next for cycle
                        // fhandle = open file corresponding to current block
                        //if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                        //    openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle,true);


                        sz[0] = end_width - start_width + 1;
                        sz[1] = end_height - start_height + 1;
                        sz[2] = n_pages_block;
                        sz[3] = channels;
                        unsigned char *p = NULL;
                        char filename[2048];
                        sprintf(filename, "%s.%s", img_path.str().c_str(), TIFF3D_SUFFIX);
                        //cout<<"adjusted img_path "<<filename<<endl;

                        long szChunk = sz[0]*sz[1]*sz[2]*sz[3]*datatype;
                        try
                        {
                            p = new unsigned char [szChunk];
                            memset(p, 0, szChunk);
                        }
                        catch(...)
                        {
                            cout<<"fail to alloc memory \n";
                        }

                        //cout<<"size ... "<<sz[0]<<" "<<sz[1]<<" "<<sz[2]<<" "<<sz[3]<<" datatype "<<datatype<<endl;

                        int saveVoxelThresh = 1;
                        int numNonZeros = 0;
                        bool saveChunkImage = false;

                        // WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                        for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++, slice_ind++)
                        {

                            //cout<<"buffer_z "<<buffer_z<<" "<<z_size/(powInt(2,halve_pow2[i]))<<" compare "<<((z - this->D0) / powInt(2,halve_pow2[i]) + buffer_z)<<" > "<<slice_end[i]<<endl;

                            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                            if ( ((z - this->D0) / powInt(2,halve_pow2[i]) + buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
                                std::stringstream abs_pos_z_next;
                                abs_pos_z_next.width(6);
                                abs_pos_z_next.fill('0');
                                // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                        (powInt(2,halve_pow2[i])*(slice_end[i]+1)) * volume->getVXL_D() * 10);
                                img_path.str("");
                                img_path << partial_img_path.str() << abs_pos_z_next.str();

                                sprintf(filename, "%s.%s", img_path.str().c_str(), TIFF3D_SUFFIX);
                                //cout<<"... adjusted img_path ... "<<filename<<endl;

                                slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

                                // close(fhandle) i.e. file corresponding to current block
                                //                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                //                                    closeTiff3DFile(fhandle);
                                //                                // fhandle = open file corresponding to next block
                                //                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                //                                    openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle,true);
                                n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
                                block_changed = true;
                                saveChunkImage = false;

                                sz[2] = n_pages_block;
                                delete [] p;
                                p = new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]*datatype];
                            }

                            sint64 raw_img_width = width/(powInt(2,i));
                            long offset = buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan;

                            uint8** raw_ch = new uint8 *[channels];
                            for (int i=0; i<channels; i++)
                                raw_ch[i] = ubuffer[i] + offset;

                            //
                            if(nbits)
                            {
                                if ( datatype == 1 ) {
                                    for(sint64 i=0; i<sz[1]; i++)
                                    {
                                        uint8* row_data_8bit = p + (i*sz[0] + buffer_z*sz[0]*sz[1])*channels;
                                        for(sint64 j=0; j<sz[0]; j++) {
                                            for ( int c=0; c<channels; c++ ) {
                                                uint8 val = raw_ch[c][(i+start_height)*(raw_img_width) + (j+start_width)];

                                                val = val >> nbits << nbits;

                                                if(saveChunkImage==false)
                                                {
                                                    if(val>0)
                                                        numNonZeros++;

                                                    if(numNonZeros>saveVoxelThresh)
                                                        saveChunkImage = true;
                                                }

                                                row_data_8bit[j*channels + c] = val;
                                            }
                                        }
                                    }
                                }
                                else {
                                    uint16  *imageData16   = (uint16 *) p + buffer_z*sz[0]*sz[1]*channels;
                                    uint16 **raw_ch16 = (uint16 **) raw_ch;

                                    //
                                    if(saveChunkImage==false)
                                    {
                                        for(sint64 i=0; i<sz[1]; i++)
                                        {
                                            for(sint64 j=0; j<sz[0]; j++) {
                                                for ( int c=0; c<channels; c++ ) {
                                                    uint16 val = raw_ch16[c][(i+start_height)*(raw_img_width) + (j+start_width)];

                                                    if(saveChunkImage==false)
                                                    {
                                                        if(val>16)
                                                            numNonZeros++;

                                                        if(numNonZeros>saveVoxelThresh)
                                                            saveChunkImage = true;
                                                    }
                                                    else
                                                    {
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    //
                                    if(saveChunkImage)
                                    {
                                        for(sint64 i=0; i<sz[1]; i++)
                                        {
                                            uint16* row_data_16bit = imageData16 + i*sz[0]*channels;
                                            for(sint64 j=0; j<sz[0]; j++) {
                                                for ( int c=0; c<channels; c++ ) {
//                                                    uint16 val = raw_ch16[c][(i+start_height)*(raw_img_width) + (j+start_width)];

//                                                    val = val >> nbits << nbits;

    //                                                if(saveChunkImage==false)
    //                                                {
    //                                                    if(val>0)
    //                                                        numNonZeros++;

    //                                                    if(numNonZeros>saveVoxelThresh)
    //                                                        saveChunkImage = true;
    //                                                }

                                                    row_data_16bit[j*channels + c] = raw_ch16[c][(i+start_height)*(raw_img_width) + (j+start_width)] >> nbits << nbits;
                                                }
                                            }
                                        }
                                    } // save chunk image

                                }
                            }
                            else
                            {
                                if ( datatype == 1 ) {
                                    for(sint64 i=0; i<sz[1]; i++)
                                    {
                                        uint8* row_data_8bit = p + (i*sz[0] + buffer_z*sz[0]*sz[1])*channels;
                                        for(sint64 j=0; j<sz[0]; j++) {
                                            for ( int c=0; c<channels; c++ ) {
                                                uint8 val = raw_ch[c][(i+start_height)*(raw_img_width) + (j+start_width)];

                                                if(saveChunkImage==false)
                                                {
                                                    if(val>0)
                                                        numNonZeros++;

                                                    if(numNonZeros>saveVoxelThresh)
                                                        saveChunkImage = true;
                                                }

                                                row_data_8bit[j*channels + c] = val;
                                            }
                                        }
                                    }
                                }
                                else {
                                    uint16  *imageData16   = (uint16 *) p + buffer_z*sz[0]*sz[1]*channels;
                                    uint16 **raw_ch16 = (uint16 **) raw_ch;

                                    //
                                    for(sint64 i=0; i<sz[1]; i++)
                                    {
                                        uint16* row_data_16bit = imageData16 + i*sz[0]*channels;
                                        for(sint64 j=0; j<sz[0]; j++) { //
                                            for ( int c=0; c<channels; c++ ) {
                                                uint16 val = raw_ch16[c][(i+start_height)*(raw_img_width) + (j+start_width)];

                                                if(saveChunkImage==false)
                                                {
                                                    if(val>0)
                                                        numNonZeros++;

                                                    if(numNonZeros>saveVoxelThresh)
                                                        saveChunkImage = true;
                                                }

                                                row_data_16bit[j*channels + c] = val;
                                            }
                                        }
                                    }
                                }
                            }

                            //saveChunkImage = true;

                            //
                            if ( ((z - this->D0) / powInt(2,halve_pow2[i]) + buffer_z) == slice_end[i] )
                            {
                                if ( strcmp(saved_img_format,"Tiff3D")==0 ) {

                                    if(saveChunkImage)
                                    {
                                        if(folderCreated==false)
                                        {
                                            if(!check_and_make_dir(H_DIR_path.str().c_str()))
                                            {
                                                char err_msg[STATIC_STRINGS_SIZE];
                                                sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                                throw IOException(err_msg);
                                            }
                                            else
                                            {
                                                folderCreated = true;
                                            }
                                        }

                                        writeTiff3DFile(filename, sz[0], sz[1], sz[2], sz[3], datatype, p);
                                    }
                                    else
                                    {
                                        nSkipped++;

                                        //cout<<"skip zero chunk "<<nSkipped++<<" "<<H_DIR_path.str()<<endl;

                                        //initTiff3DFile(filename, sz[0], sz[1], sz[2], sz[3], datatype);
                                    }
                                }
                                else
                                {
                                    // save to vaa3draw
                                }
                            }

                        }

//                        if(saveCount == 0)
//                        {
//                            emptyDirs.push_back(H_DIR_path.str());
//                        }

                        //
                        delete [] p;

                        start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS

                        // 2015-01-30. Alessandro. @ADDED performance (time) measurement in 'generateTilesVaa3DRaw()' method.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
                        TERAFLY_TIME_STOP(ConverterWriteBlockOperation, tf::ALL_COMPS, terafly::strprintf("converter: written multiresolution image block x(%d-%d), y(%d-%d), z(%d-%d)",start_width, end_width, start_height, end_height, ((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1))));
#endif
                    }
                    start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                }
            }
            delete [] sz;
        }

        //
//        auto start_rmdirs = std::chrono::high_resolution_clock::now();
//        for(size_t dir = 0; dir<emptyDirs.size(); dir++)
//        {
////            char cmd[2048];
////            const char * folder = emptyDirs[dir];

////            cout<<"check folder "<<folder<<endl;

////            int status, exitcode;

////            snprintf(cmd, 2048, "test $(ls -A \"%s\" 2 >/dev/null | wc -l) -ne 0", folder);

////            status = system(cmd);
////            exitcode = WEXITSTATUS(status);

////            if(exitcode == 1)
////            {
////                // empty folder
////                cout<<"rm folder\n";

////                remove_dir(folder);
////            }

//            // rmdir(emptyDirs[dir]);

//            std::stringstream cmd;
//            cmd << "rm -r \""<<emptyDirs[dir].c_str()<<"\"";

//            cout<<"cmd "<<cmd.str()<<endl;

//            system(cmd.str().c_str());
//        }

//        auto end_rmdirs = std::chrono::high_resolution_clock::now();

//        cout<<"rm "<<emptyDirs.size()<<" dirs takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end_rmdirs - start_rmdirs).count()<<" ms."<<endl;

        cout<<"skip "<<nSkipped<<" folders "<<endl;

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets

        //
        auto end = std::chrono::high_resolution_clock::now();

        cout<<"writing chunk images takes "<<std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()<<" ms."<<endl;
    }

    // save last group data
    saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);

    int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for(int res_i=0; res_i< resolutions_size; res_i++)
        {
            if(resolutions[res_i])
            {
                //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                //system.
                try {
                    TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                         volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));
                    //volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
                catch (IOException & ex)
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                    n_err++;
                }
                catch ( ... )
                {
                    printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
                    n_err++;
                }

                //          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                //                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                delete []stacks_height[res_i][stack_row][stack_col];
                delete []stacks_width [res_i][stack_row][stack_col];
                delete []stacks_depth [res_i][stack_row][stack_col];
            }
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
            delete []stacks_depth [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
        delete []stacks_depth[res_i];
    }

    if ( n_err ) { // errors in mdat.bin creation
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
    }
}


/*************************************************************************************************************
* Functions used to obtain absolute coordinates at different resolutions from relative coordinates
**************************************************************************************************************/
int VolumeConverter::getMultiresABS_V(int res, int REL_V)
{
    //if(volume->getVXL_V() > 0)
    return volume->getABS_V( V0 + REL_V*(int)pow(2.0f,res) ) * 10;
    //else
    //	return volume->getABS_V( V0 - 1 + REL_V*pow(2.0f,res))*10 + volume->getVXL_V()*pow(2.0f,res)*10;
}
std::string VolumeConverter::getMultiresABS_V_string(int res, int REL_V)	
{
    std::stringstream multires_merging_x_pos;
    multires_merging_x_pos.width(6);
    multires_merging_x_pos.fill('0');
    multires_merging_x_pos << this->getMultiresABS_V(res, REL_V);
    return multires_merging_x_pos.str();
}
int VolumeConverter::getMultiresABS_H(int res, int REL_H)
{
    //if(volume->getVXL_H() > 0)
    return volume->getABS_H( H0 + REL_H*(int)pow(2.0f,res) )*10;
    //else
    //	return volume->getABS_H( H0 - 1 + REL_H*pow(2.0f,res))*10  + volume->getVXL_H()*pow(2.0f,res)*10;
}
std::string VolumeConverter::getMultiresABS_H_string(int res, int REL_H)	
{
    std::stringstream multires_merging_y_pos;
    multires_merging_y_pos.width(6);
    multires_merging_y_pos.fill('0');
    multires_merging_y_pos << this->getMultiresABS_H(res, REL_H);
    return multires_merging_y_pos.str();
}
int VolumeConverter::getMultiresABS_D(int res)
{
    // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
    //if(volume->getVXL_D() > 0)
    return volume->getABS_D(D0)*10;
    //else
    //	return volume->getABS_D((int)(D0 - 1 + volume->getVXL_D()*pow(2.0f,res)*10.0f));
}



/*************************************************************************************************************
* NEW TILED FORMAT SUPPORTING MULTIPLE CHANNELS
* WARNING: from now on the resolutions directories in channel directories versione is no longer aligned with 
*          channel directories in resolution directories
*          sympol RES_IN_CHANS should not be defined if code is not revised before
**************************************************************************************************************/

# ifdef RES_IN_CHANS // resolutions directories in channels directories (WARNING: from now on do not define this symbol)

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, std::string ch_dir, bool* resolutions, 
                                                int block_height, int block_width, int block_depth, int method,
                                                bool show_progress_bar, const char* saved_img_format,
                                                int saved_img_depth, std::string frame_dir )	throw (IOExcpetion, iom::exception)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOExcpetion(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[S_MAX_MULTIRES];
    int n_stacks_D[S_MAX_MULTIRES];
    int ***stacks_height[S_MAX_MULTIRES];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[S_MAX_MULTIRES];
    int ***stacks_depth[S_MAX_MULTIRES];
    std::stringstream file_path[S_MAX_MULTIRES];                            //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    std::string *chans_dir;
    std::string resolution_dir;

    /* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
     * In the following the term 'group' means the groups of slices that are
     * processed together to generate slices of all resolution requested
     */

    /* stack_block[i] is the index of current block along z (it depends on the resolution i)
     * current block is the block in which falls the first slice of the group
     * of slices that is currently being processed, i.e. from z to z+z_max_res-1
     */
    int stack_block[S_MAX_MULTIRES];

    /* these arrays are the indices of first and last slice of current block at resolution i
     * WARNING: the slice index refers to the index of the slice in the volume at resolution i
     */
    int slice_start[S_MAX_MULTIRES];
    int slice_end[S_MAX_MULTIRES];

    /* the number of slice of already processed groups at current resolution
     * the index of the slice to be saved at current resolution is:
     *
     *      n_slices_pred + z_buffer
     */
    sint64 n_slices_pred;

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
        throw IOExcpetion(err_msg);
    }

    //initializing the progress bar
    char progressBarMsg[200];
    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressInfo(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff3D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < S_MIN_SLICE_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", S_MIN_SLICE_DIM);
        throw IOExcpetion(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //computing tiles dimensions at each resolution
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,res_i))  / (float) block_depth  );
        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
                }
            }
        }
    }

    // computing resolutions directory names
    for(int res_i=0; res_i< resolutions_size; res_i++) {
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true) {
            file_path[res_i] << "/RES("<<height/POW_INT(2,res_i)
                             << "x" << width/POW_INT(2,res_i)
                             << "x" << depth/POW_INT(2,res_i) << ")";
        }
    }

    // computing channel directory names
    chans_dir = new std::string[channels];
    int n_digits = 1;
    int _channels = channels / 10;
    while ( _channels ) {
        n_digits++;
        _channels /= 10;
    }
    for ( int c=0; c<channels; c++ ) {
        std::stringstream dir_name;
        dir_name.width(n_digits);
        dir_name.fill('0');
        dir_name << c;
        chans_dir[c] = output_path + "/" + IM_CHANNEL_PREFIX + dir_name.str();
        //if(make_dir(chans_dir[c].c_str())!=0)
        if(!check_and_make_dir(chans_dir[c].c_str())) { // HP 130914
            {
                char err_msg[S_MAX_MULTIRES];
                sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
                throw IOExcpetion(err_msg);
            }
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                //creating volume directory iff current resolution is selected and test mode is disabled
                if(resolutions[res_i] == true) {
                    //creating directory that will contain image data at current resolution
                    //resolution_dir = chans_dir[c] + file_path[res_i].str();
                    resolution_dir = file_path[res_i].str() + chans_dir[c];
                    //if(make_dir(resolution_dir.c_str())!=0)
                    if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
                    {
                        char err_msg[S_MAX_MULTIRES];
                        sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOExcpetion(err_msg);
                    }
                }
            }
        }

        /* The following check verifies that the numeber of slices in the buffer is not higher than the numbero of slices in a block file
     * (excluding the last blck in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
     * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block
     * file at that resolution as computed by the above code is:
     *
     *                                                B = floor( H / ceil( H/D ) )
     * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
     *
     *                                                  D >= B >= floor(D/2)
     * since it is:
     *
     *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
     * where alpha<1.
     */

        //ALLOCATING  the MEMORY SPACE for image buffer
        z_max_res = std::max(std::min(STANDARD_BLOCK_DEPTH,block_depth/2),powInt(2,halve_pow2[resolutions_size-1]));
        if ( (z_max_res > 1) && z_max_res > block_depth/2 ) {
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
            throw IOException(err_msg);
        }
        z_ratio=depth/z_max_res;

        //allocated even if not used
        ubuffer = new iim::uint8 *[channels];
        memset(ubuffer,0,channels*sizeof(iim::uint8));
        org_channels = channels; // save for checks

        //slice_start and slice_end of current block depend on the resolution
        for(int res_i=0; res_i< resolutions_size; res_i++) {
            stack_block[res_i] = 0;
            //slice_start[res_i] = this->D0;
            slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
            slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
        }

        // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
        for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
        {
            // fill one slice block
            if ( internal_rep == REAL_INTERNAL_REP )
                rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
            else { // internal_rep == UINT8_INTERNAL_REP
                ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
                if ( org_channels != channels ) {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                    throw IOExcpetion(err_msg);
                }

                for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                    // offsets have to be computed taking into account that buffer size along D may be different
                    // WARNING: the offset must be of tipe sint64
                    ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
                }
            }
            // WARNING: should check that buffer has been actually allocated

            // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
            if ( nbits ) {
                //printf("----> lossy compression nbits = %d\n",nbits);
                iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
                if ( bytes_chan == 1 ) {
                    iim::uint8 *ptr = ubuffer[0];
                    for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                        *ptr = *ptr >> nbits << nbits;
                    }
                }
                else if ( bytes_chan == 2 ) {
                    iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                    for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                        *ptr = *ptr >> nbits << nbits;
                    }
                }
            }

            //updating the progress bar
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
                ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            //saving current buffer data at selected resolutions and in multitile format
            for(int i=0; i< resolutions_size; i++)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                // check if current block is changed
                // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                if ( ((z - this->D0) / powInt(2,i)) > slice_end[i] ) {
                    stack_block[i]++;
                    slice_start[i] = slice_end[i] + 1;
                    slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
                }

                // find abs_pos_z at resolution i
                std::stringstream abs_pos_z;
                abs_pos_z.width(6);
                abs_pos_z.fill('0');
                // 2015-12-19. Giulio. @ADDED Subvolume conversion
                // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                   (powInt(2,i)*slice_start[i]) * volume->getVXL_D() * 10);
                //abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                //					- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                //					(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D());

                //compute the number of slice of previous groups at resolution i
                //note that z_parts in the number and not an index (starts from 1)
                n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

                //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
                sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

                //halvesampling resolution if current resolution is not the deepest one
                if(i!=0) {
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);
                }

                //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
                if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
                {
                    if(show_progress_bar)
                    {
                        sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                        ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                        ts::ProgressBar::getInstance()->display();
                    }

                    for ( int c=0; c<channels; c++ ) {

                        //storing in 'base_path' the absolute path of the directory that will contain all stacks
                        std::stringstream base_path;
                        base_path << chans_dir[c] << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

                        //looping on new stacks
                        for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                        {
                            //incrementing end_height
                            end_height = start_height + stacks_height[i][stack_row][0][0]-1;

                            //computing V_DIR_path and creating the directory the first time it is needed
                            std::stringstream V_DIR_path;
                            V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                            if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                            {
                                char err_msg[STATIC_STRINGS_SIZE];
                                sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                                throw IOExcpetion(err_msg);
                            }

                            for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                            {
                                end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

                                //computing H_DIR_path and creating the directory the first time it is needed
                                std::stringstream H_DIR_path;
                                H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
                                if ( z==D0 ) {
                                    if(!check_and_make_dir(H_DIR_path.str().c_str()))
                                    {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                        throw IOExcpetion(err_msg);
                                    }
                                    else { // the directory has been created for the first time
                                        // initialize block files
                                        V3DLONG *sz = new V3DLONG[4];
                                        int datatype;
                                        char *err_rawfmt;

                                        sz[0] = stacks_width[i][stack_row][stack_column][0];
                                        sz[1] = stacks_height[i][stack_row][stack_column][0];
                                        sz[3] = 1; // single channel files

                                        if ( internal_rep == REAL_INTERNAL_REP )
                                            datatype = 4;
                                        else if ( internal_rep == UINT8_INTERNAL_REP ) {
                                            if ( saved_img_depth == 16 )
                                                datatype = 2;
                                            else if ( saved_img_depth == 8 )
                                                datatype = 1;
                                            else {
                                                char err_msg[STATIC_STRINGS_SIZE];
                                                sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unknown image depth (%d)", saved_img_depth);
                                                throw IOExcpetion(err_msg);
                                            }
                                        }
                                        else {
                                            char err_msg[STATIC_STRINGS_SIZE];
                                            sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unknown internal representation (%d)", internal_rep);
                                            throw IOExcpetion(err_msg);
                                        }

                                        int slice_start_temp = 0;
                                        for ( int j=0; j < n_stacks_D[i]; j++ ) {
                                            sz[2] = stacks_depth[i][stack_row][stack_column][j];

                                            std::stringstream abs_pos_z_temp;
                                            abs_pos_z_temp.width(6);
                                            abs_pos_z_temp.fill('0');
                                            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                            abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                                    (POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D() * 10);

                                            std::stringstream img_path_temp;
                                            img_path_temp << H_DIR_path.str() << "/"
                                                          << this->getMultiresABS_V_string(i,start_height) << "_"
                                                          << this->getMultiresABS_H_string(i,start_width) << "_"
                                                          << abs_pos_z_temp.str();

                                            //if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
                                            if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
                                                   ( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),sz[0],sz[1],sz[2],sz[3],datatype)) != 0 ) :
                                                   ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
                                                char err_msg[STATIC_STRINGS_SIZE];
                                                sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
                                                throw IOExcpetion(err_msg);
                                            };

                                            slice_start_temp += (int)sz[2];
                                        }
                                        delete [] sz;
                                    }
                                }

                                //saving HERE

                                // 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
                                std::stringstream partial_img_path;
                                partial_img_path << H_DIR_path.str() << "/"
                                                 << this->getMultiresABS_V_string(i,start_height) << "_"
                                                 << this->getMultiresABS_H_string(i,start_width) << "_";

                                int slice_ind = (int)(n_slices_pred - slice_start[i]);

                                std::stringstream img_path;
                                img_path << partial_img_path.str() << abs_pos_z.str();

                                /* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
                             * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations,
                             * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at
                             * end of the file.
                             * The number of pages of block files of interest can be easily computed as:
                             *
                             *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]]
                             *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1]
                             */

                                void *fhandle = 0;
                                int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
                                bool block_changed = false;                                 // true if block is changed executing the next for cycle
                                // fhandle = open file corresponding to current block
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle,true);

                                // WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                                for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++, slice_ind++)
                                {
                                    // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                                    if ( ((z - this->D0)  /powInt(2,i)+buffer_z) > slice_end[i] && !block_changed) { // start a new block along z
                                        std::stringstream abs_pos_z_next;
                                        abs_pos_z_next.width(6);
                                        abs_pos_z_next.fill('0');
                                        // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                        abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                                (powInt(2,i)*(slice_end[i]+1)) * volume->getVXL_D() * 10);
                                        img_path.str("");
                                        img_path << partial_img_path.str() << abs_pos_z_next.str();

                                        slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

                                        // close(fhandle) i.e. file corresponding to current block
                                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                            closeTiff3DFile(fhandle);
                                        // fhandle = open file corresponding to next block
                                        if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                            openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle,true);
                                        n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
                                        block_changed = true;
                                    }

                                    if ( internal_rep == REAL_INTERNAL_REP )
                                        VirtualVolume::saveImage_to_Vaa3DRaw(
                                                    slice_ind,
                                                    img_path.str(),
                                                    rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
                                                    (int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
                                                    start_height,end_height,start_width,end_width,
                                                    saved_img_format, saved_img_depth
                                                    );
                                    else {// internal_rep == UINT8_INTERNAL_REP
                                        if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
                                            VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
                                                        slice_ind,
                                                        img_path.str(),
                                                        ubuffer + c,
                                                        1,
                                                        buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                        start_height,end_height,start_width,end_width,
                                                        saved_img_format, saved_img_depth,fhandle,n_pages_block,false);
                                        }
                                        else { // can be only Vaa3DRaw
                                            VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
                                                        slice_ind,
                                                        img_path.str(),
                                                        ubuffer + c,
                                                        1,
                                                        buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                        (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                        start_height,end_height,start_width,end_width,
                                                        saved_img_format, saved_img_depth);
                                        }
                                    }
                                }

                                // close(fhandle) i.e. currently opened file
                                if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                    closeTiff3DFile(fhandle);

                                start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                            }
                            start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                        }
                    }
                }
            }

            //releasing allocated memory
            if ( internal_rep == REAL_INTERNAL_REP )
                delete rbuffer;
            else // internal_rep == UINT8_INTERNAL_REP
                delete ubuffer[0]; // other buffer pointers are only offsets
        }

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for ( int c=0; c<channels; c++ ) {
            for(int res_i=0; res_i< resolutions_size; res_i++)
            {
                if(resolutions[res_i])
                {
                    resolution_dir = chans_dir[c] + file_path[res_i].str();

                    //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                    //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                    //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                    //system.
                    TiledVolume temp_vol(resolution_dir.c_str(),reference,
                                         volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

                    //          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                    //                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
                }
            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;

        // restore the output plugin
        iom::IMOUT_PLUGIN = save_imout_plugin;

        // ubuffer allocated anyway
        delete ubuffer;

        // deallocate memory
        for(int res_i=0; res_i< resolutions_size; res_i++)
        {
            for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
            {
                for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
                {
                    delete[] stacks_height[res_i][stack_row][stack_col];
                    delete[] stacks_width [res_i][stack_row][stack_col];
                    delete[] stacks_depth [res_i][stack_row][stack_col];
                }
                delete[] stacks_height[res_i][stack_row];
                delete[] stacks_width [res_i][stack_row];
                delete[] stacks_depth [res_i][stack_row];
            }
            delete[] stacks_height[res_i];
            delete[] stacks_width[res_i];
            delete[] stacks_depth[res_i];
        }
        delete[] chans_dir;
    }

# else // channels directories in resolutions directories (WARNING: the following code is the only one supported: leave symbol RES_IN_CHANS undefined)

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, std::string ch_dir, bool* resolutions, 
                                                int block_height, int block_width, int block_depth, int method, bool isotropic,
                                                bool show_progress_bar, const char* saved_img_format,
                                                int saved_img_depth, std::string frame_dir, bool par_mode )	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    sint64 z_ratio, z_max_res;
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];                            //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change
    std::string save_imout_plugin; // to be used for restoring the output plugin after a change

    sint64 whole_height; // 2016-04-13. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-04-13. Giulio. to be used only if par_mode is set to store the depth of the whole volume
    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    std::string *chans_dir;
    std::string resolution_dir;

    /* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
     * In the following the term 'group' means the groups of slices that are
     * processed together to generate slices of all resolution requested
     */

    /* stack_block[i] is the index of current block along z (it depends on the resolution i)
     * current block is the block in which falls the first slice of the group
     * of slices that is currently being processed, i.e. from z to z+z_max_res-1
     */
    int stack_block[TMITREE_MAX_HEIGHT];

    /* these arrays are the indices of first and last slice of current block at resolution i
     * WARNING: the slice index refers to the index of the slice in the volume at resolution i
     */
    int slice_start[TMITREE_MAX_HEIGHT];
    int slice_end[TMITREE_MAX_HEIGHT];

    /* the number of slice of already processed groups at current resolution
     * the index of the slice to be saved at current resolution is:
     *
     *      n_slices_pred + z_buffer
     */
    sint64 n_slices_pred;

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
        throw IOException(err_msg);
    }

    // 2017-04-24. Giulio. @CHANGED the way the output pluging is set
    save_imout_plugin = iom::IMOUT_PLUGIN;
    iom::IMOUT_PLUGIN = "tiff3D";

    //// 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //initializing the progress bar

    char progressBarMsg[200];
    if ( par_mode ) // in parallel mode never show the progress bar
        show_progress_bar = false;

    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    if ( par_mode ) {
        // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
        whole_height = this->volume->getDIM_V();
        whole_width  = this->volume->getDIM_H();
        whole_depth  = this->volume->getDIM_D();
    }
    else {
        // 2016-04-13. Giulio. whole_depth should not be used
        whole_depth = -1;
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: the minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,halve_pow2[res_i]))  / (float) block_depth  );
        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,halve_pow2[res_i])))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,halve_pow2[res_i])))  % n_stacks_D[res_i] ? 1:0);
                }
            }
        }
    }

    // 2016-10-09. Giulio. @ADDED the passed subdirectory name is used in case one channel image has to be converted
    chans_dir = new std::string[channels];
    if ( ch_dir != "" && channels == 1 ) {
        chans_dir[0] = "/" + ch_dir;
    }
    else { // no subdirectory name has been provided or more than one channel
        // computing channel directory names
        int n_digits = 1;
        int _channels = channels / 10;
        while ( _channels ) {
            n_digits++;
            _channels /= 10;
        }
        for ( int c=0; c<channels; c++ ) {
            std::stringstream dir_name;
            dir_name.width(n_digits);
            dir_name.fill('0');
            dir_name << c;
            chans_dir[c] = "/" + (iim::CHANNEL_PREFIX + dir_name.str());
        }
    }

    // computing resolutions directory names
    for(int res_i=0; res_i< resolutions_size; res_i++) {
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true) {
            if ( par_mode ) { // 2016-04-13. Giulio. uses the depth of the whole volume to generate the directory name
                //creating directory that will contain image data at current resolution
                file_path[res_i] << output_path << "/RES("<<whole_height/powInt(2,res_i)
                                 << "x" << whole_width/powInt(2,res_i)
                                 << "x" << whole_depth/powInt(2,halve_pow2[res_i]) << ")";
            }
            else {
                //creating directory that will contain image data at current resolution
                file_path[res_i] << output_path << "/RES("<<height/powInt(2,res_i)
                                 << "x" << width/powInt(2,res_i)
                                 << "x" << depth/powInt(2,halve_pow2[res_i]) << ")";
                //if(make_dir(file_path[res_i].str().c_str())!=0) {
                if(!check_and_make_dir(file_path[res_i].str().c_str())) { // HP 130914
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if ( !par_mode ) { // 2016-04-13. Giulio. the directory should be created only in non-parallel mode
                    if(!check_and_make_dir(file_path[res_i].str().c_str()))
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                        throw IOException(err_msg);
                    }
                }
            }

            if ( !par_mode ) { // 2016-04-23. Giulio. the channel directories should be created only in non-parallel mode
                for ( int c=0; c<channels; c++ ) {
                    //creating directory that will contain image data at current resolution
                    resolution_dir = file_path[res_i].str() + chans_dir[c];
                    //if(make_dir(resolution_dir.c_str())!=0)
                    if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
                        throw IOException(err_msg);
                    }
                }
            }
        }
    }

    /* The following check verifies that the numeber of slices in the buffer is not higher than the number of slices in a block file
     * (excluding the last blck in a stack). Indeed if D is the maximum number of slices in a block file (i.e. the value of block_depth)
     * and H is the total number of slices at resolution i (i.e. floor(depth/2^i)), the actual minumum number of slices B in a block
     * file at that resolution as computed by the above code is:
     *
     *                                                B = floor( H / ceil( H/D ) )
     * Now, assuming that at resolution i there is more than one block, it is H > D and hence:
     *
     *                                                  D >= B >= floor(D/2)
     * since it is:
     *
     *                               1/ceil(H/D) = 1/(H/D + alpha) = D/(H + alpha * D) > D/(2 * H)
     * where alpha<1.
     */

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = std::max(std::min(STANDARD_BLOCK_DEPTH,block_depth/2),powInt(2,halve_pow2[resolutions_size-1]));
    if ( (z_max_res > 1) && (z_max_res > block_depth/2) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
        throw IOException(err_msg);
    }
    z_ratio=depth/z_max_res;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8 *));
    org_channels = channels; // save for checks

    FILE *fhandle;
    sint64 z;
    sint64 z_parts;

    // WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
    if ( par_mode ) {
        output_path_par << output_path << "/" << "V_" << this->V0 << "-" << this->V1<< "_H_" << this->H0 << "-" << this->H1<< "_D_" << this->D0 << "-" << this->D1;
        if(!check_and_make_dir(output_path_par.str().c_str())) {  // the directory does nor exist or cannot be created
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", output_path_par.str().c_str());
            throw IOException(err_msg);
        }
        if ( initVCResumer(saved_img_format,output_path_par.str().c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) { // halve_pow2 is not saved
            readVCResumerState(fhandle,output_path_par.str().c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts); // halve_pow2 is not saved
        }
        else { // halve_pow2 is not saved: start form the first slice
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
            z = this->D0;
            z_parts = 1;
        }
    }
    else { // not in parallel mode: use output_path to maintain resume status
        if ( initVCResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,method,saved_img_format,saved_img_depth,fhandle) ) {
            readVCResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
        }
        else {
            //slice_start and slice_end of current block depend on the resolution
            for(int res_i=0; res_i< resolutions_size; res_i++) {
                stack_block[res_i] = 0;
                slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning
                slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
            }
            z = this->D0;
            z_parts = 1;
        }
    }

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(/* sint64 z = this->D0, z_parts = 1 */; z < this->D1; z += z_max_res, z_parts++)
    {
        // save previous group data
        saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z,z_parts);

        // fill one slice block
        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)z,(z+z_max_res <= D1) ? (int)(z+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"in generateTilesVaa3DRawMC(...): the volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        for(int i=0; i< resolutions_size; i++)
        {
            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            // check if current block is changed
            // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
            if ( ((z - this->D0) / powInt(2,halve_pow2[i])) > slice_end[i] ) {
                stack_block[i]++;
                slice_start[i] = slice_end[i] + 1;
                slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
            }

            // find abs_pos_z at resolution i
            std::stringstream abs_pos_z;
            abs_pos_z.width(6);
            abs_pos_z.fill('0');
            // 2015-12-19. Giulio. @ADDED Subvolume conversion
            // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
            abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                               (powInt(2,halve_pow2[i])*slice_start[i]) * volume->getVXL_D() * 10);
            //abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
            //					- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
            //					(powInt(2,i)*slice_start[i]) * volume->getVXL_D());

            //compute the number of slice of previous groups at resolution i
            //note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / powInt(2,halve_pow2[i]);

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                for ( int c=0; c<channels; c++ ) {

                    //storing in 'base_path' the absolute path of the directory that will contain all stacks
                    std::stringstream base_path;
                    base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

                    //looping on new stacks
                    for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
                    {
                        //incrementing end_height
                        end_height = start_height + stacks_height[i][stack_row][0][0]-1;

                        //computing V_DIR_path and creating the directory the first time it is needed
                        std::stringstream V_DIR_path;
                        V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                        if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
                        {
                            char err_msg[STATIC_STRINGS_SIZE];
                            sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
                            throw IOException(err_msg);
                        }

                        for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
                        {
                            end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

                            //computing H_DIR_path and creating the directory the first time it is needed
                            std::stringstream H_DIR_path;
                            H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
                            if ( z==D0 ) {
                                if(!check_and_make_dir(H_DIR_path.str().c_str()))
                                {
                                    char err_msg[STATIC_STRINGS_SIZE];
                                    sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
                                    throw IOException(err_msg);
                                }
                                else { // the directory has been created for the first time
                                    // initialize block files
                                    V3DLONG *sz = new V3DLONG[4];
                                    int datatype;
                                    char *err_rawfmt;

                                    sz[0] = stacks_width[i][stack_row][stack_column][0];
                                    sz[1] = stacks_height[i][stack_row][stack_column][0];
                                    sz[3] = 1; // single channel files

                                    if ( internal_rep == REAL_INTERNAL_REP )
                                        datatype = 4;
                                    else if ( internal_rep == UINT8_INTERNAL_REP ) {
                                        if ( saved_img_depth == 16 )
                                            datatype = 2;
                                        else if ( saved_img_depth == 8 )
                                            datatype = 1;
                                        else {
                                            char err_msg[STATIC_STRINGS_SIZE];
                                            sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unknown image depth (%d)", saved_img_depth);
                                            throw IOException(err_msg);
                                        }
                                    }
                                    else {
                                        char err_msg[STATIC_STRINGS_SIZE];
                                        sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unknown internal representation (%d)", internal_rep);
                                        throw IOException(err_msg);
                                    }

                                    int slice_start_temp = 0;
                                    for ( int j=0; j < n_stacks_D[i]; j++ ) {
                                        sz[2] = stacks_depth[i][stack_row][stack_column][j];

                                        std::stringstream abs_pos_z_temp;
                                        abs_pos_z_temp.width(6);
                                        abs_pos_z_temp.fill('0');
                                        // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                        abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                                (powInt(2,halve_pow2[i])*(slice_start_temp)) * volume->getVXL_D() * 10);

                                        std::stringstream img_path_temp;
                                        img_path_temp << H_DIR_path.str() << "/"
                                                      << this->getMultiresABS_V_string(i,start_height) << "_"
                                                      << this->getMultiresABS_H_string(i,start_width) << "_"
                                                      << abs_pos_z_temp.str();

                                        //if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
                                        if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
                                               ( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(int)sz[0],(int)sz[1],(int)sz[2],(int)sz[3],datatype)) != 0 ) :
                                               ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
                                            char err_msg[STATIC_STRINGS_SIZE];
                                            sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
                                            throw IOException(err_msg);
                                        };

                                        slice_start_temp += (int)sz[2];
                                    }
                                    delete [] sz;
                                }
                            }

                            //saving HERE

                            // 2015-02-10. Giulio. @CHANGED changed how img_path is constructed
                            std::stringstream partial_img_path;
                            partial_img_path << H_DIR_path.str() << "/"
                                             << this->getMultiresABS_V_string(i,start_height) << "_"
                                             << this->getMultiresABS_H_string(i,start_width) << "_";

                            int slice_ind = (int)(n_slices_pred - slice_start[i]);

                            std::stringstream img_path;
                            img_path << partial_img_path.str() << abs_pos_z.str();

                            /* 2015-02-06. Giulio. @ADDED optimization to reduce the number of open/close operations in append operations
                             * Since slices of the same block in a group are appended in sequence, to minimize the overhead of append operations,
                             * all slices of a group to be appended to the same block file are appended leaving the file open and positioned at
                             * end of the file.
                             * The number of pages of block files of interest can be easily computed as:
                             *
                             *    number of slice of current block = stacks_depth[i][0][0][stack_block[i]]
                             *    number of slice of next block    = stacks_depth[i][0][0][stack_block[i]+1]
                             */

                            void *fhandle = 0;
                            int  n_pages_block = stacks_depth[i][0][0][stack_block[i]]; // number of pages of current block
                            bool block_changed = false;                                 // true if block is changed executing the next for cycle
                            // fhandle = open file corresponding to current block
                            if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                openTiff3DFile((char *)img_path.str().c_str(),(char *)(slice_ind ? "a" : "w"),fhandle,true);

                            // WARNING: assumes that block size along z is not less that z_size/(powInt(2,i))
                            for(int buffer_z=0; buffer_z<z_size/(powInt(2,halve_pow2[i])); buffer_z++, slice_ind++)
                            {
                                // D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
                                if ( ((z - this->D0)  /powInt(2,halve_pow2[i])+buffer_z) > slice_end[i] && !block_changed ) { // start a new block along z
                                    std::stringstream abs_pos_z_next;
                                    abs_pos_z_next.width(6);
                                    abs_pos_z_next.fill('0');
                                    // 2015-12-20. Giulio. @FIXED file name generation (scale of some absolute coordinates was in 1 um and not in 0.1 um
                                    abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
                                                            (powInt(2,halve_pow2[i])*(slice_end[i]+1)) * volume->getVXL_D() * 10);
                                    img_path.str("");
                                    img_path << partial_img_path.str() << abs_pos_z_next.str();

                                    slice_ind = 0; // 2015-02-10. Giulio. @CHANGED (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;

                                    // close(fhandle) i.e. file corresponding to current block
                                    if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                        closeTiff3DFile(fhandle);
                                    // fhandle = open file corresponding to next block
                                    if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                        openTiff3DFile((char *)img_path.str().c_str(),(char *)"w",fhandle,true);
                                    n_pages_block = stacks_depth[i][0][0][stack_block[i]+1];
                                    block_changed = true;
                                }

                                if ( internal_rep == REAL_INTERNAL_REP )
                                    VirtualVolume::saveImage_to_Vaa3DRaw(
                                                slice_ind,
                                                img_path.str(),
                                                rbuffer + buffer_z*(height/powInt(2,i))*(width/powInt(2,i)), // adds the stride
                                                (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                start_height,end_height,start_width,end_width,
                                                saved_img_format, saved_img_depth
                                                );
                                else {// internal_rep == UINT8_INTERNAL_REP
                                    if ( strcmp(saved_img_format,"Tiff3D")==0 ) {
                                        VirtualVolume::saveImage_from_UINT8_to_Tiff3D(
                                                    slice_ind,
                                                    img_path.str(),
                                                    ubuffer + c,
                                                    1,
                                                    buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                    start_height,end_height,start_width,end_width,
                                                    saved_img_format, saved_img_depth,fhandle,n_pages_block,false);
                                    }
                                    else { // can be only Vaa3DRaw
                                        VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
                                                    slice_ind,
                                                    img_path.str(),
                                                    ubuffer + c,
                                                    1,
                                                    buffer_z*(height/powInt(2,i))*(width/powInt(2,i))*bytes_chan,  // stride to be added for slice buffer_z
                                                    (int)height/(powInt(2,i)),(int)width/(powInt(2,i)),
                                                    start_height,end_height,start_width,end_width,
                                                    saved_img_format, saved_img_depth);
                                    }
                                }
                            }

                            // close(fhandle) i.e. currently opened file
                            if ( strcmp(saved_img_format,"Tiff3D") == 0 )
                                closeTiff3DFile(fhandle);

                            start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                        }
                        start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
                    }
                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets
    }

    // save last group data
    saveVCResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);

    int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

    if ( !par_mode ) {
        // 2016-04-13. Giulio. @ADDED close resume
        closeVCResumer(fhandle,output_path.c_str());

        // reloads created volumes to generate .bin file descriptors at all resolutions
        ref_sys reference(axis(1),axis(2),axis(3));
        TiledMCVolume *mcprobe;
        TiledVolume   *tprobe;
        StackedVolume *sprobe;
        sprobe = dynamic_cast<StackedVolume *>(volume);
        if ( sprobe ) {
            reference.first  = sprobe->getAXS_1();
            reference.second = sprobe->getAXS_2();
            reference.third  = sprobe->getAXS_3();
        }
        else {
            tprobe = dynamic_cast<TiledVolume *>(volume);
            if ( tprobe ) {
                reference.first  = tprobe->getAXS_1();
                reference.second = tprobe->getAXS_2();
                reference.third  = tprobe->getAXS_3();
            }
            else {
                mcprobe = dynamic_cast<TiledMCVolume *>(volume);
                if ( mcprobe ) {
                    reference.first  = mcprobe->getAXS_1();
                    reference.second = mcprobe->getAXS_2();
                    reference.third  = mcprobe->getAXS_3();
                }
            }
        }

        // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
        if ( volume->getAXS_1() < 0 ) {
            if ( volume->getAXS_1() == vertical )
                reference.first = axis(-1);
            else // volume->getAXS_1() == horizontal
                reference.second = axis(-2);
        }
        if ( volume->getAXS_2() < 0 ) {
            if ( volume->getAXS_2() == horizontal )
                reference.second = axis(-2);
            else // volume->getAXS_2() == vertical
                reference.first = axis(-1);
        }

        // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
        save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
        // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
        iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

        for(int res_i=0; res_i< resolutions_size; res_i++) {

            if(resolutions[res_i]) {

                for ( int c=0; c<channels; c++ ) {
                    resolution_dir = file_path[res_i].str() + chans_dir[c];

                    //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                    //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                    //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                    //system.
                    try {
                        TiledVolume temp_vol(resolution_dir.c_str(),reference,
                                             volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));
                    }
                    catch (IOException & ex)
                    {
                        printf("in VolumeConverter::generateTilesVaa3DRawMC: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                        n_err++;
                    }
                    catch ( ... )
                    {
                        printf("in VolumeConverter::generateTilesVaa3DRawMC: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
                        n_err++;
                    }

                    //          	StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
                    //                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
                }

                TiledMCVolume temp_mc_vol(file_path[res_i].str().c_str(),reference,
                                          volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,halve_pow2[res_i]));

            }
        }

        // restore input plugin
        iom::IMIN_PLUGIN = save_imin_plugin;
    }
    else { // par mode
        // 2016-04-13. Giulio. @ADDED close resume in par mode
        closeVCResumer(fhandle,output_path_par.str().c_str());
        // WARNINIG --- the directory should be removed
        bool res = remove_dir(output_path_par.str().c_str());
    }

    // restore the output plugin
    iom::IMOUT_PLUGIN = save_imout_plugin;

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                delete[] stacks_height[res_i][stack_row][stack_col];
                delete[] stacks_width [res_i][stack_row][stack_col];
                delete[] stacks_depth [res_i][stack_row][stack_col];
            }
            delete[] stacks_height[res_i][stack_row];
            delete[] stacks_width [res_i][stack_row];
            delete[] stacks_depth [res_i][stack_row];
        }
        delete[] stacks_height[res_i];
        delete[] stacks_width[res_i];
        delete[] stacks_depth[res_i];
    }

    delete[] chans_dir;

    if ( n_err ) { // errors in mdat.bin creation
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
    }
}

#endif


void VolumeConverter::generateTilesBDV_HDF5 ( std::string output_path, bool* resolutions, 
                                              int block_height, int block_width, int block_depth, int method, bool isotropic,
                                              bool show_progress_bar, const char* saved_img_format,
                                              int saved_img_depth, std::string frame_dir )	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::generateTilesBDV_HDF5(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    sint64 z_ratio, z_max_res;
    int resolutions_size = 0;

    void *file_descr;
    sint64 *hyperslab_descr = new sint64[4*3]; // four 3-valued parameters: [ start(offset), stride count(size), block ]
    memset(hyperslab_descr,0,4*3*sizeof(sint64));
    sint64 *buf_dims = new sint64[3];  // dimensions of the buffer in which the subvolume is stored at a given resolution
    memset(buf_dims,0,3*sizeof(sint64));

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesBDV_HDF5: undefined source volume");
        throw IOException(err_msg);
    }

    // HDF5 crea il file HDF5 se non esiste altrimenti determina i setup e i time point gia' presenti
    BDV_HDF5init(output_path,file_descr,volume->getBYTESxCHAN());

    //initializing the progress bar
    char progressBarMsg[200];
    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    // 2016-04-09. Giulio. @COMMENTED not needed if the output is HDF5.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    //these parameters are used here for chunk dimensions; the default values should be passed without changes to BDV_HDF5 routines
    //block_height = (block_height == -1 ? (int)height : block_height);
    //block_width  = (block_width  == -1 ? (int)width  : block_width);
    //block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    //if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM9 */)
    //{
    //    char err_msg[STATIC_STRINGS_SIZE];
    //    sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
    //    throw IOException(err_msg);
    //}

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    BDV_HDF5addSetups(file_descr,height,width,depth,volume->getVXL_V(),volume->getVXL_H(),volume->getVXL_D(),
                      resolutions,resolutions_size,channels,block_height,block_width,block_depth);

    BDV_HDF5addTimepoint(file_descr);

    //BDV_HDF5close(file_descr);

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1);

    // the following check does not make sense for Fiji_HDF5 format
    //if ( z_max_res > block_depth/2 ) {
    //	char err_msg[STATIC_STRINGS_SIZE];
    //	sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
    //	throw IOException(err_msg);
    //}
    z_ratio=depth/z_max_res;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8));
    org_channels = channels; // save for checks

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
    {
        // fill one slice block
        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        for(int i=0; i< resolutions_size; i++)
        {
            // HDF5 crea i gruppi relativi a questa risoluzione in ciascun setup del time point corrente

            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( internal_rep == REAL_INTERNAL_REP )
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
                else // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
            }

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //std::stringstream  res_name;
                //res_name << i;

                for ( int c=0; c<channels; c++ ) {

                    //storing in 'base_path' the absolute path of the directory that will contain all stacks
                    //std::stringstream base_path;
                    // ELIMINARE? base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

                    // HDF5 scrive il canale corrente nel buffer nel gruppo corrispondente al time point e alla risoluzione correnti
                    if ( internal_rep == REAL_INTERNAL_REP )
                        throw iim::IOException(iim::strprintf("updating already existing files not supported yet").c_str(),__iim__current__function__);
                    else { // internal_rep == UINT8_INTERNAL_REP
                        buf_dims[1] = height/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
                        buf_dims[2] = width/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
                        buf_dims[0] = z_size/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
                        // start
                        hyperslab_descr[0] = 0; // [0][0]
                        hyperslab_descr[1] = 0; // [0][1]
                        hyperslab_descr[2] = 0; // [0][2]
                        // stride
                        hyperslab_descr[3] = 1;  // [1][0]
                        hyperslab_descr[4] = 1;  // [1][1]
                        hyperslab_descr[5] = 1;  // [1][2]
                        // count
                        hyperslab_descr[6] = buf_dims[0]; //z_size/(powInt(2,i)); // [2][0]
                        hyperslab_descr[7] = buf_dims[1]; //height/(powInt(2,i)); // [2][1]
                        hyperslab_descr[8] = buf_dims[2]; //width/(powInt(2,i));  // [2][2]
                        // block
                        hyperslab_descr[9]  = 1; // [3][0]
                        hyperslab_descr[10] = 1; // [3][1]
                        hyperslab_descr[11] = 1; // [3][2]
                        BDV_HDF5writeHyperslab(file_descr,ubuffer[c],buf_dims,hyperslab_descr,i,c);
                    }

                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets
    }

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    //   for(int res_i=0; res_i< resolutions_size; res_i++)
    //{
    //	for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
    //	{
    //		for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
    //		{
    //			delete[] stacks_height[res_i][stack_row][stack_col];
    //			delete[] stacks_width [res_i][stack_row][stack_col];
    //			delete[] stacks_depth [res_i][stack_row][stack_col];
    //		}
    //		delete[] stacks_height[res_i][stack_row];
    //		delete[] stacks_width [res_i][stack_row];
    //		delete[] stacks_depth [res_i][stack_row];
    //	}
    //	delete[] stacks_height[res_i];
    //	delete[] stacks_width[res_i];
    //	delete[] stacks_depth[res_i];
    //}

    //delete[] chans_dir;

    delete hyperslab_descr;
    delete buf_dims;

    BDV_HDF5close(file_descr);

}


void VolumeConverter::generateTilesIMS_HDF5 ( std::string output_path, std::string metadata_file, bool* resolutions, 
                                              int block_height, int block_width, int block_depth, int method, bool isotropic,
                                              bool show_progress_bar, const char* saved_img_format,
                                              int saved_img_depth, std::string frame_dir )	throw (IOException, iom::exception)
{
    //printf("in VolumeConverter::generateTilesBDV_HDF5(path = \"%s\", resolutions = ", output_path.c_str());
    //for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
    //    printf("%d", resolutions[i]);
    //printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
    //       block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    real32* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
    iim::uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
    int bytes_chan = volume->getBYTESxCHAN();
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    sint64 z_ratio, z_max_res;
    int resolutions_size = 0;
    int halve_pow2[TMITREE_MAX_HEIGHT];

    int cur_tp = 0;  // current timepoint
    // determine the time point to be generated
    if ( frame_dir == "" )
        ; // use default time point
    else
        cur_tp = atoi(frame_dir.c_str());

    void *file_descr;
    sint64 *hyperslab_descr = new sint64[4*3]; // four 3-valued parameters: [ start(offset), stride count(size), block ]
    memset(hyperslab_descr,0,4*3*sizeof(sint64));
    sint64 *buf_dims = new sint64[3];  // dimensions of the buffer in which the subvolume is stored at a given resolution
    memset(buf_dims,0,3*sizeof(sint64));

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::generateTilesBDV_HDF5: undefined source volume");
        throw IOException(err_msg);
    }

    //initializing the progress bar
    char progressBarMsg[200];
    if(show_progress_bar)
    {
        ts::ProgressBar::getInstance()->start("Multiresolution tile generation");
        ts::ProgressBar::getInstance()->setProgressValue(0,"Initializing...");
        ts::ProgressBar::getInstance()->display();
    }

    //computing dimensions of volume to be stitched
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    // code for testing
    //iim::uint8 *temp = volume->loadSubvolume_to_UINT8(
    //	10,height-10,10,width-10,10,depth-10,
    //	&channels);

    //these parameters are used here for chunk dimensions; the default values should be passed without changes to IMS_HDF5 routines
    //block_height = (block_height == -1 ? (int)height : block_height);
    //block_width  = (block_width  == -1 ? (int)width  : block_width);
    //block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    //if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM9 */)
    //{
    //    char err_msg[STATIC_STRINGS_SIZE];
    //    sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
    //    throw IOException(err_msg);
    //}

    // 2017-09-09. Giulio. @ADDED code for analyzing the string 'saved_img_format' and extract information to be passed to HDF5 routines
    std::string output_params = saved_img_format;
    if ( output_params.find("ims") != std::string::npos || output_params.find("Fiji_HDF5") != std::string::npos ) {
        output_params = "";
    }
    else {
        ; // WARNINIG: assume that 'saved_img_format' is a string of the form uint[:uint:...:uint] where uint is an unsigner integer between 0 and 65535
    }


    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else {
        for (int i=0; i<TMITREE_MAX_HEIGHT; i++) {
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);
        }
    }

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    // allocate the buffers for the histograms
    int hist_len = 256;
    histogram_t *hist[TMITREE_MAX_HEIGHT];
    for (int i=0; i<TMITREE_MAX_HEIGHT; i++) {
        if(resolutions[i]) {
            hist[i] = new histogram_t[channels];
            for (int j=0; j<channels; j++) {
                hist[i][j].hmin = 0;
                hist[i][j].hmax = 0;
                hist[i][j].hlen = hist_len;
                hist[i][j].hist = new iim::uint64[hist_len];
                memset(hist[i][j].hist,0,hist_len*sizeof(iim::uint64));
            }
        }
        else
            hist[i] = (histogram_t *) 0;
    }

    iim::uint32 thumbnail_size;
    iim::uint8 *thumbnail_buf;

    if ( cur_tp == 0 ) { // thumbnail, metadata, resolutions and other initializations must be processed only for the first timepoint

        // allocate the buffer for the thuumnail
        thumbnail_size = (std::min(width,height) < 512) ? 256 : 512;
        int total_size = thumbnail_size * thumbnail_size * 4;
        thumbnail_buf = new iim::uint8[total_size];
        memset(thumbnail_buf,0,total_size*sizeof(iim::uint8));

        // get metadata to be transferred to image to be generated
        void *olist;
        void *rootalist;
        if ( metadata_file == "default" || metadata_file == "null" ) {
            // default metadata have to be generated
            olist = IMS_HDF5get_olist((void *)0,output_path,(int)height,(int)width,(int)depth,channels,1,volume->getORG_V(),volume->getORG_H()); // assumes 1 timepoint
            rootalist = IMS_HDF5get_rootalist((void *)0);
        }
        else {
            IMS_HDF5init(metadata_file,file_descr,true);
            olist = IMS_HDF5get_olist(file_descr);
            rootalist = IMS_HDF5get_rootalist(file_descr);
            IMS_HDF5close(file_descr);
            // adjust the object/attribute structure to the file to be generated
            olist = IMS_HDF5adjust_olist(olist,output_path,(int)height,(int)width,(int)depth,this->volume->getActiveChannels(),channels,volume->getORG_V(),volume->getORG_H());
        }


        // create output file with acquisition metadata
        IMS_HDF5init(output_path,file_descr,false,volume->getBYTESxCHAN(),olist,rootalist); // set the same voxel size of the file containaing metadata
        olist     = (void *) 0;
        rootalist = (void *) 0;

        // voxel size must be the one corresponding to the height, width and depth used to create resolutions (see call to 'IMS_HDF5addResolution' below)
        // voxel size must be correctly set before creating resolutions
        IMS_HDF5setVxlSize(file_descr,fabs(volume->getVXL_1()),fabs(volume->getVXL_2()),fabs(volume->getVXL_3()));

        // create output file structure and add specific image metadata
        bool is_first = true;
        for ( int i=0; i<resolutions_size; i++ ) {

            if(resolutions[i]) {
                IMS_HDF5addResolution(file_descr,height,width,depth,channels,i,is_first); // istotropic: must pass halve_pow2
                is_first = false;
            }

        }

    }
    else { // cur_tp > 0

        IMS_HDF5init(output_path,file_descr,false,volume->getBYTESxCHAN(),(void *) 0,(void *) 0); // set the same voxel size of the file containaing metadata

    }

    // create timepoint groups (default: timepoint 0)
    IMS_HDF5addTimepoint(file_descr,cur_tp,output_params);

    //ALLOCATING  the MEMORY SPACE for image buffer
    z_max_res = powInt(2,resolutions_size-1); // isotropic: z_max_res = powInt(2,halve_pow2[resolutions_size-1]);

    // the following check does not make sense for Fiji_HDF5 format
    //if ( z_max_res > block_depth/2 ) {
    //	char err_msg[STATIC_STRINGS_SIZE];
    //	sprintf(err_msg, "in generateTilesVaa3DRaw(...): too much resolutions(%d): too much slices (%lld) in the buffer \n", resolutions_size, z_max_res);
    //	throw IOException(err_msg);
    //}
    z_ratio=depth/z_max_res;

    //allocated even if not used
    ubuffer = new iim::uint8 *[channels];
    memset(ubuffer,0,channels*sizeof(iim::uint8));
    org_channels = channels; // save for checks

    //IMS_HDF5init(output_path,file_descr);

    // z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
    for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
    {
        // fill one slice block
        if ( internal_rep == REAL_INTERNAL_REP )
            rbuffer = volume->loadSubvolume_to_real32(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
        else { // internal_rep == UINT8_INTERNAL_REP
            ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,iim::NATIVE_RTYPE);
            if ( org_channels != channels ) {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
                throw IOException(err_msg);
            }

            for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
                // offsets have to be computed taking into account that buffer size along D may be different
                // WARNING: the offset must be of tipe sint64
                ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
            }
        }
        // WARNING: should check that buffer has been actually allocated

        // 2017-05-25. Giulio. Added code for simple lossy compression (suggested by Hanchuan Peng)
        if ( nbits ) {
            //printf("----> lossy compression nbits = %d\n",nbits);
            iim::sint64 tot_size = (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res))) * channels;
            if ( bytes_chan == 1 ) {
                iim::uint8 *ptr = ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
            else if ( bytes_chan == 2 ) {
                iim::uint16 *ptr = (iim::uint16 *) ubuffer[0];
                for ( iim::sint64 i=0; i<tot_size; i++, ptr++ ) {
                    *ptr = *ptr >> nbits << nbits;
                }
            }
        }

        //updating the progress bar
        if(show_progress_bar)
        {
            sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((iim::uint32)(z-D0)),((iim::uint32)(z-D0+z_max_res-1)),(iim::uint32)depth);
            ts::ProgressBar::getInstance()->setProgressValue(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
            ts::ProgressBar::getInstance()->display();
        }

        //saving current buffer data at selected resolutions and in multitile format
        for(int i=0; i< resolutions_size; i++)
        {
            // HDF5 crea i gruppi relativi a questa risoluzione in ciascun setup del time point corrente

            if(show_progress_bar)
            {
                sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,std::max(resolutions_size, resolutions_size));
                ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                ts::ProgressBar::getInstance()->display();
            }

            //buffer size along D is different when the remainder of the subdivision by z_max_res is considered
            sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( internal_rep == REAL_INTERNAL_REP )
                    VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),method);
                else // internal_rep == UINT8_INTERNAL_REP
                    VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,i-1)),channels,method,bytes_chan);
            }

            /* isotropic:
            //halvesampling resolution if current resolution is not the deepest one
            if(i!=0) {
                if ( halve_pow2[i] == (halve_pow2[i-1]+1) ) { // *modified*
                    // also D dimension has to be halvesampled
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else if ( halve_pow2[i] == halve_pow2[i-1] ) {// *modified*
                    if ( internal_rep == REAL_INTERNAL_REP )
                        VirtualVolume::halveSample2D(rbuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),method);
                    else  // internal_rep == UINT8_INTERNAL_REP
                        VirtualVolume::halveSample2D_UINT8(ubuffer,(int)height/(powInt(2,i-1)),(int)width/(powInt(2,i-1)),(int)z_size/(powInt(2,halve_pow2[i-1])),channels,method,bytes_chan);
                }
                else {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): halve sampling level %d not supported at resolution %d\n", halve_pow2[i], i);
                    throw iom::exception(err_msg);
                }
            }
*/

            //saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
            if(resolutions[i] && (z_size/(powInt(2,i))) > 0) // isotropic: if(resolutions[i] && (z_size/(powInt(2,halve_pow2[i]))) > 0)
            {
                if(show_progress_bar)
                {
                    sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                    ts::ProgressBar::getInstance()->setProgressInfo(progressBarMsg);
                    ts::ProgressBar::getInstance()->display();
                }

                //std::stringstream  res_name;
                //res_name << i;

                for ( int c=0; c<channels; c++ ) {

                    //storing in 'base_path' the absolute path of the directory that will contain all stacks
                    //std::stringstream base_path;
                    // ELIMINARE? base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

                    // HDF5 scrive il canale corrente nel buffer nel gruppo corrispondente al time point e alla risoluzione correnti
                    if ( internal_rep == REAL_INTERNAL_REP )
                        throw iim::IOException(iim::strprintf("updating already existing files not supported yet").c_str(),__iim__current__function__);
                    else { // internal_rep == UINT8_INTERNAL_REP
                        buf_dims[1] = height/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
                        buf_dims[2] = width/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1));
                        buf_dims[0] = z_size/(powInt(2,i)); //((i==0) ? powInt(2,i) : powInt(2,i-1)); // isotropic: z_size/(powInt(2,halve_pow2[i]))
                        // start
                        hyperslab_descr[0] = 0; // [0][0]
                        hyperslab_descr[1] = 0; // [0][1]
                        hyperslab_descr[2] = 0; // [0][2]
                        // stride
                        hyperslab_descr[3] = 1;  // [1][0]
                        hyperslab_descr[4] = 1;  // [1][1]
                        hyperslab_descr[5] = 1;  // [1][2]
                        // count
                        hyperslab_descr[6] = buf_dims[0]; //z_size/(powInt(2,i)); // [2][0]
                        hyperslab_descr[7] = buf_dims[1]; //height/(powInt(2,i)); // [2][1]
                        hyperslab_descr[8] = buf_dims[2]; //width/(powInt(2,i));  // [2][2]
                        // block
                        hyperslab_descr[9]  = 1; // [3][0]
                        hyperslab_descr[10] = 1; // [3][1]
                        hyperslab_descr[11] = 1; // [3][2]
                        IMS_HDF5writeHyperslab(file_descr,ubuffer[c],buf_dims,hyperslab_descr,i,c,cur_tp); // fixed timepoint: 0
                    }

                }
            }
        }

        //releasing allocated memory
        if ( internal_rep == REAL_INTERNAL_REP )
            delete rbuffer;
        else // internal_rep == UINT8_INTERNAL_REP
            delete ubuffer[0]; // other buffer pointers are only offsets
    }

    // ubuffer allocated anyway
    delete ubuffer;

    // deallocate memory
    //   for(int res_i=0; res_i< resolutions_size; res_i++)
    //{
    //	for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
    //	{
    //		for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
    //		{
    //			delete[] stacks_height[res_i][stack_row][stack_col];
    //			delete[] stacks_width [res_i][stack_row][stack_col];
    //			delete[] stacks_depth [res_i][stack_row][stack_col];
    //		}
    //		delete[] stacks_height[res_i][stack_row];
    //		delete[] stacks_width [res_i][stack_row];
    //		delete[] stacks_depth [res_i][stack_row];
    //	}
    //	delete[] stacks_height[res_i];
    //	delete[] stacks_width[res_i];
    //	delete[] stacks_depth[res_i];
    //}

    //delete[] chans_dir;

    delete hyperslab_descr;
    delete buf_dims;

    for (int i=0; i<TMITREE_MAX_HEIGHT; i++) {
        if(resolutions[i]) {
            for (int j=0; j<channels; j++) {
                IMS_HDF5set_histogram(file_descr,&hist[i][j],i,j,cur_tp);
                hist[i][j].hist = (iim::uint64 *) 0;
            }
            delete []hist[i];
        }
    }

    if ( cur_tp == 0 )
        IMS_HDF5set_thumbnail(file_descr,thumbnail_buf,thumbnail_size);

    IMS_HDF5close(file_descr);

}


// unified access point for volume conversion (@ADDED by Alessandro on 2014-02-24)
void VolumeConverter::convertTo(
        std::string output_path,                        // path where to save the converted volume
        std::string output_format,                      // format of the converted volume (see IM_config.h)
        int output_bitdepth /*= iim::NUL_IMG_DEPTH*/,   // output image bitdepth
        bool isTimeSeries /*= false*/,                  // whether the volume is a time series
        bool *resolutions /*= 0*/,                      // array of resolutions
        int block_height /*= -1*/,                      // tile's height (for tiled formats)
        int block_width  /*= -1*/,                      // tile's width  (for tiled formats)
        int block_depth  /*= -1*/,                      // tile's depth  (for tiled formats)
        int method /*=HALVE_BY_MEAN*/,                  // downsampling method
        bool isotropic /*=false*/,                      // perform an isotropic conversion
        std::string metadata_file /*= "null"*/,         // last parameter, used only by Imaris file format
        std::string compression_info/*= ""*/            // last parameter, used only by Imaris file format
        ) throw (iim::IOException, iom::exception)
{
    printf("in VolumeConverter::convertTo(output_path = \"%s\", output_format = \"%s\", output_bitdepth = %d, isTimeSeries = %s, resolutions = ",
           output_path.c_str(), output_format.c_str(), output_bitdepth, isTimeSeries ? "true" : "false");
    for(int i=0; i< TMITREE_MAX_HEIGHT && resolutions; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d)\n",
           block_height, block_width, block_depth, method);

    if(isTimeSeries)
    {
        for(int t=0; t<volume->getDIM_T(); t++)
        {
            ts::ProgressBar::instance()->setProgressInfo(strprintf("Converting time frame %d/%d", t+1, volume->getDIM_T()).c_str());
            volume->setActiveFrames(t,t);
            std::string frame_dir = iim::TIME_FRAME_PREFIX + strprintf("%06d", t);
            if(output_format.compare(iim::STACKED_FORMAT) == 0)
                generateTiles(output_path, resolutions, block_height, block_width, method, isotropic, true, "tif", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::STACKED_RAW_FORMAT) == 0)
                generateTiles(output_path, resolutions, block_height, block_width, method, isotropic, true, "raw", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::TILED_FORMAT) == 0)
                generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, isotropic, true, "raw", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::TILED_MC_FORMAT) == 0)
                generateTilesVaa3DRawMC(output_path, "", resolutions, block_height, block_width, block_depth, method, isotropic, true, "raw", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::TILED_TIF3D_FORMAT) == 0)
                generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, isotropic, true, "Tiff3D", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
                generateTilesVaa3DRawMC(output_path, "", resolutions, block_height, block_width, block_depth, method, isotropic, true, "Tiff3D", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::SIMPLE_RAW_FORMAT) == 0)
                generateTilesSimple(output_path, resolutions, block_height, block_width, method, isotropic, true, "raw", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::SIMPLE_FORMAT) == 0)
                generateTilesSimple(output_path, resolutions, block_height, block_width, method, isotropic, true, "tif", output_bitdepth, frame_dir, false);
            else if(output_format.compare(iim::IMS_HDF5_FORMAT) == 0) {
                frame_dir = strprintf("%d", t);
                generateTilesIMS_HDF5(output_path, metadata_file, resolutions, block_height,block_width,block_depth, method, isotropic, true,(compression_info == "" ? "Fiji_HDF5" : compression_info.c_str()),output_bitdepth,frame_dir);
            }
            else
                throw iim::IOException(strprintf("Output format \"%s\" not supported", output_format.c_str()).c_str());
        }
        ts::ProgressBar::instance()->resetMembers();
    }
    else
    {
        if(output_format.compare(iim::STACKED_FORMAT) == 0)
            generateTiles(output_path, resolutions, block_height, block_width, method, isotropic, true, iim::DEF_IMG_FORMAT.c_str(), output_bitdepth, "", false);
        else if(output_format.compare(iim::STACKED_RAW_FORMAT) == 0)
            generateTiles(output_path, resolutions, block_height, block_width, method, isotropic, true, "raw", output_bitdepth, "", false);
        else if(output_format.compare(iim::TILED_FORMAT) == 0)
            generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, isotropic, true, "raw", output_bitdepth, "", false);
        else if(output_format.compare(iim::TILED_MC_FORMAT) == 0)
            generateTilesVaa3DRawMC(output_path, "", resolutions, block_height, block_width, block_depth, method, isotropic, true, "raw", output_bitdepth, "", false);
        else if(output_format.compare(iim::TILED_TIF3D_FORMAT) == 0)
            generateTilesVaa3DRaw(output_path, resolutions, block_height, block_width, block_depth, method, isotropic, true, "Tiff3D", output_bitdepth, "", false);
        else if(output_format.compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
            generateTilesVaa3DRawMC(output_path, "", resolutions, block_height, block_width, block_depth, method, isotropic, true, "Tiff3D", output_bitdepth, "", false);
        else if(output_format.compare(iim::BDV_HDF5_FORMAT) == 0)
            generateTilesBDV_HDF5(output_path,resolutions, block_height,block_width,block_depth, method, isotropic, true,"Fiji_HDF5",output_bitdepth);
        else if(output_format.compare(iim::IMS_HDF5_FORMAT) == 0)
            generateTilesIMS_HDF5(output_path, metadata_file, resolutions, block_height,block_width,block_depth, method, isotropic, true,(compression_info == "" ? "Fiji_HDF5" : compression_info.c_str()),output_bitdepth);
        else if(output_format.compare(iim::SIMPLE_RAW_FORMAT) == 0)
            generateTilesSimple(output_path, resolutions, block_height, block_width, method, isotropic, true, "raw", output_bitdepth, "", false);
        else if(output_format.compare(iim::SIMPLE_FORMAT) == 0)
            generateTilesSimple(output_path, resolutions, block_height, block_width, method, isotropic, true, "tif", output_bitdepth, "", false);
        else
            throw iim::IOException(strprintf("Output format \"%s\" not supported", output_format.c_str()).c_str());
    }
}



void VolumeConverter::createDirectoryHierarchy(std::string output_path, std::string ch_dir, bool* resolutions, 
                                               int block_height, int block_width, int block_depth, int method, bool isotropic,
                                               bool show_progress_bar, const char* saved_img_format,
                                               int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::createDirectoryHierarchyVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::createDirectoryHierarchy: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    int n_stacks_D[TMITREE_MAX_HEIGHT];
    int ***stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int ***stacks_width[TMITREE_MAX_HEIGHT];
    int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    std::string *chans_dir;
    std::string resolution_dir;

    sint64 whole_height; // 2016-05-04. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-05-04. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-05-04. Giulio. to be used only if par_mode is set to store the depth of the whole volume

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::createDirectoryHierarchy: undefined source volume");
        throw IOException(err_msg);
    }

    // 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
    whole_height = this->volume->getDIM_V();
    whole_width  = this->volume->getDIM_H();
    whole_depth  = this->volume->getDIM_D();


    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    height = this->V1-this->V0;
    width  = this->H1-this->H0;
    depth  = this->D1-this->D0;

    if ( (whole_height != height) || (whole_width != width) || (whole_depth != depth) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): currently only the whole voliume can be processed in this mode");
        throw IOException(err_msg);
    }

    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //    char err_msg[5000];
    //    sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): block_depth is not set in parallel mode");
    //    throw iom::exception(err_msg);
    //}

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): the minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    if ( strcmp(saved_img_format,"Tiff3DMC") == 0 || strcmp(saved_img_format,"Vaa3DRawMC") == 0 ) {
        // 2017-01-23. Giulio. @ADDED the passed subdirectory name is used in case one channel image has to be converted
        chans_dir = new std::string[channels];
        if ( ch_dir != "" && channels == 1 ) {
            chans_dir[0] = "/" + ch_dir;
        }
        else { // no subdirectory name has been provided or more than one channel
            // computing channel directory names
            int n_digits = 1;
            int _channels = channels / 10;
            while ( _channels ) {
                n_digits++;
                _channels /= 10;
            }
            for ( int c=0; c<channels; c++ ) {
                std::stringstream dir_name;
                dir_name.width(n_digits);
                dir_name.fill('0');
                dir_name << c;
                chans_dir[c] = "/" + (iim::CHANNEL_PREFIX + dir_name.str());
            }
        }
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = (int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/powInt(2,res_i))  / (float) block_depth  );
        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
                for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
                {
                    stacks_height[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                    stacks_depth[res_i][stack_row][stack_col][stack_sli] =
                            ((int)(depth/powInt(2,halve_pow2[res_i])))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/powInt(2,halve_pow2[res_i])))  % n_stacks_D[res_i] ? 1:0);
                }
            }
        }

        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            //creating directory that will contain image data at current resolution
            file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
            //if(make_dir(file_path[res_i].str().c_str())!=0)
            if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
            {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg, "in createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw IOException(err_msg);
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if(!check_and_make_dir(file_path[res_i].str().c_str()))
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }

            if ( strcmp(saved_img_format,"Tiff3DMC") == 0 || strcmp(saved_img_format,"Vaa3DRawMC") == 0 ) {
                for ( int c=0; c<channels; c++ ) {
                    //creating directory that will contain image data at current resolution
                    resolution_dir = file_path[res_i].str() + chans_dir[c];
                    //if(make_dir(resolution_dir.c_str())!=0)
                    if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
                    {
                        char err_msg[STATIC_STRINGS_SIZE];
                        sprintf(err_msg, "in createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
                        throw IOException(err_msg);
                    }
                }
            }
        }
    }

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                delete []stacks_height[res_i][stack_row][stack_col];
                delete []stacks_width [res_i][stack_row][stack_col];
                delete []stacks_depth [res_i][stack_row][stack_col];
            }
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
            delete []stacks_depth [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
        delete []stacks_depth[res_i];
    }
}



void VolumeConverter::createDirectoryHierarchySimple(std::string output_path, bool* resolutions, 
                                                     int block_height, int block_width, int block_depth, int method, bool isotropic,
                                                     bool show_progress_bar, const char* saved_img_format,
                                                     int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::createDirectoryHierarchyVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::createDirectoryHierarchy: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    int n_stacks_V[TMITREE_MAX_HEIGHT];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[TMITREE_MAX_HEIGHT];
    //int n_stacks_D[TMITREE_MAX_HEIGHT];
    int **stacks_height[TMITREE_MAX_HEIGHT];   //array of matrices of tiles dimensions at i-th resolution
    int **stacks_width[TMITREE_MAX_HEIGHT];
    //int ***stacks_depth[TMITREE_MAX_HEIGHT];
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    //std::string *chans_dir;
    std::string resolution_dir;

    sint64 whole_height; // 2016-05-04. Giulio. to be used only if par_mode is set to store the height of the whole volume
    sint64 whole_width;  // 2016-05-04. Giulio. to be used only if par_mode is set to store the width of the whole volume
    sint64 whole_depth;  // 2016-05-04. Giulio. to be used only if par_mode is set to store the depth of the whole volume

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::createDirectoryHierarchy: undefined source volume");
        throw IOException(err_msg);
    }

    // 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    //if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    //{
    //	iom::IMOUT_PLUGIN = "tiff3D";
    //}

    // 2016-04-13. Giulio. whole_depth is the depth of the whole volume
    whole_height = this->volume->getDIM_V();
    whole_width  = this->volume->getDIM_H();
    whole_depth  = this->volume->getDIM_D();


    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    height = this->V1-this->V0;
    width  = this->H1-this->H0;
    depth  = this->D1-this->D0;

    if ( (whole_height != height) || (whole_width != width) || (whole_depth != depth) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): currently only the whole voliume can be processed in this mode");
        throw IOException(err_msg);
    }

    //if(par_mode && block_depth == -1) // 2016-04-13. Giulio. if conversion is parallelized, option --slicedepth must be used to set block_depth
    //{
    //    char err_msg[5000];
    //    sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): block_depth is not set in parallel mode");
    //    throw iom::exception(err_msg);
    //}

    //activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < TMITREE_MIN_BLOCK_DIM || block_width < TMITREE_MIN_BLOCK_DIM /* 2014-11-10. Giulio. @REMOVED (|| block_depth < TMITREE_MIN_BLOCK_DIM) */)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in VolumeConverter::createDirectoryHierarchy(...): the minimum dimension for block height, width, and depth is %d", TMITREE_MIN_BLOCK_DIM);
        throw IOException(err_msg);
    }

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        n_stacks_V[res_i] = 1; //(int) ceil ( (height/powInt(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = 1; //(int) ceil ( (width/powInt(2,res_i))  / (float) block_width  );
        stacks_height[res_i] = new int *[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = ((int)(height/powInt(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/powInt(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                stacks_width [res_i][stack_row][stack_col] = ((int)(width/powInt(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/powInt(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
            }
        }

        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true)
        {
            //creating directory that will contain image data at current resolution
            file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
            //if(make_dir(file_path[res_i].str().c_str())!=0)
            if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
            {
                char err_msg[STATIC_STRINGS_SIZE];
                sprintf(err_msg, "in createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw IOException(err_msg);
            }

            //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
            if ( frame_dir != "" ) {
                file_path[res_i] << "/" << frame_dir << "/";
                if(!check_and_make_dir(file_path[res_i].str().c_str()))
                {
                    char err_msg[STATIC_STRINGS_SIZE];
                    sprintf(err_msg, "in createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw IOException(err_msg);
                }
            }
        }
    }

    // deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            // 			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            // 			{
            // 				delete []stacks_height[res_i][stack_row][stack_col];
            // 				delete []stacks_width [res_i][stack_row][stack_col];
            // 			}
            delete []stacks_height[res_i][stack_row];
            delete []stacks_width [res_i][stack_row];
        }
        delete []stacks_height[res_i];
        delete []stacks_width[res_i];
    }
}


void VolumeConverter::mdataGenerator(std::string output_path, std::string ch_dir, bool* resolutions, 
                                     int block_height, int block_width, int block_depth, int method, bool isotropic,
                                     bool show_progress_bar, const char* saved_img_format,
                                     int saved_img_depth, std::string frame_dir, bool par_mode)	throw (IOException, iom::exception)
{
    printf("in VolumeConverter::mdataGenerator(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< TMITREE_MAX_HEIGHT; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d, frame_dir = \"%s\")\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth, frame_dir.c_str());

    if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
        saved_img_depth = volume->getBYTESxCHAN() * 8;

    if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::mdataGenerator: mismatch between bits per channel of source (%d) and destination (%d)",
                volume->getBYTESxCHAN() * 8, saved_img_depth);
        throw IOException(err_msg);
    }

    //LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
    int bytes_chan = volume->getBYTESxCHAN();
    //iim::uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    //iim::uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
    int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //real32* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
    std::stringstream file_path[TMITREE_MAX_HEIGHT];  //array of root directory name at i-th resolution
    int resolutions_size = 0;

    std::stringstream output_path_par; // used if parallel option is set
    int halve_pow2[TMITREE_MAX_HEIGHT];

    std::string *chans_dir;
    std::string resolution_dir;

    std::string save_imin_plugin; // to be used for restoring the input plugin after a change

    if ( volume == 0 ) {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::mdataGenerator: undefined source volume");
        throw IOException(err_msg);
    }

    // 2015-03-03. Giulio. @ADDED selection of IO plugin if not provided.
    if(iom::IMOUT_PLUGIN.compare("empty") == 0)
    {
        if ( strcmp(saved_img_format,"tif") == 0 )
            iom::IMOUT_PLUGIN = "tiff2D";
        else if ( strcmp(saved_img_format,"Vaa3DRaw") == 0 )
            ;
        else if ( strcmp(saved_img_format,"Tiff3D") == 0 || strcmp(saved_img_format,"Tiff3DMC") == 0 )
            iom::IMOUT_PLUGIN = "tiff3D";
        else if ( strcmp(saved_img_format,"Vaa3DRaw") == 0 || strcmp(saved_img_format,"Vaa3DRawMC") == 0 )
            ; // 2016-04-23. Giulio. no plugin is used for Vaa3D raw format
        else {
            char err_msg[STATIC_STRINGS_SIZE];
            sprintf(err_msg,"VolumeConverter::mdataGenerator: undefined saved image format (%s)",saved_img_format);
            throw IOException(err_msg);
        }
    }

    //computing dimensions of volume to be stitched
    //this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
    width = this->H1-this->H0;
    height = this->V1-this->V0;
    depth = this->D1-this->D0;

    if(resolutions == NULL)
    {
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
    }
    else
        for(int i=0; i<TMITREE_MAX_HEIGHT; i++)
            if(resolutions[i])
                resolutions_size = std::max(resolutions_size, i+1);

    //2016-04-13. Giulio. set the halving rules
    if ( isotropic ) {
        // an isotropic image must be generated
        float vxlsz_Vx2 = 2*(volume->getVXL_V() > 0 ? volume->getVXL_V() : -volume->getVXL_V());
        float vxlsz_Hx2 = 2*(volume->getVXL_H() > 0 ? volume->getVXL_H() : -volume->getVXL_H());
        float vxlsz_D = volume->getVXL_D();
        halve_pow2[0] = 0;
        for ( int i=1; i<resolutions_size; i++ ) {
            halve_pow2[i] = halve_pow2[i-1];
            if ( vxlsz_D < std::max<float>(vxlsz_Vx2,vxlsz_Hx2) ) {
                halve_pow2[i]++;
                vxlsz_D   *= 2;
            }
            vxlsz_Vx2 *= 2;
            vxlsz_Hx2 *= 2;
        }
    }
    else {
        // halving along D dimension must be always performed
        for ( int i=0; i<resolutions_size; i++ )
            halve_pow2[i] = i;
    }

    if ( strcmp(saved_img_format,"Tiff3DMC") == 0 || strcmp(saved_img_format,"Vaa3DRawMC") == 0 ) {
        // 2017-01-23. Giulio. @ADDED the passed subdirectory name is used in case one channel image has to be converted
        chans_dir = new std::string[channels];
        if ( ch_dir != "" && channels == 1 ) {
            chans_dir[0] = "/" + ch_dir;
        }
        else { // no subdirectory name has been provided or more than one channel
            // computing channel directory names
            int n_digits = 1;
            int _channels = channels / 10;
            while ( _channels ) {
                n_digits++;
                _channels /= 10;
            }
            for ( int c=0; c<channels; c++ ) {
                std::stringstream dir_name;
                dir_name.width(n_digits);
                dir_name.fill('0');
                dir_name << c;
                chans_dir[c] = "/" + (iim::CHANNEL_PREFIX + dir_name.str());
            }
        }
    }

    //computing tiles dimensions at each resolution and initializing volume directories
    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true) {
            //creating directory that will contain image data at current resolution
            file_path[res_i]<<output_path<<"/RES("<<height/powInt(2,res_i)<<"x"<<width/powInt(2,res_i)<<"x"<<depth/powInt(2,halve_pow2[res_i])<<")";
        }

        //if frame_dir not empty must create frame directory (@FIXED by Alessandro on 2014-02-25)
        if ( frame_dir != "" ) {
            file_path[res_i] << "/" << frame_dir << "/";
        }
    }

    int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

    // reloads created volumes to generate .bin file descriptors at all resolutions
    ref_sys reference(axis(1),axis(2),axis(3));
    TiledMCVolume *mcprobe;
    TiledVolume   *tprobe;
    StackedVolume *sprobe;
    sprobe = dynamic_cast<StackedVolume *>(volume);
    if ( sprobe ) {
        reference.first  = sprobe->getAXS_1();
        reference.second = sprobe->getAXS_2();
        reference.third  = sprobe->getAXS_3();
    }
    else {
        tprobe = dynamic_cast<TiledVolume *>(volume);
        if ( tprobe ) {
            reference.first  = tprobe->getAXS_1();
            reference.second = tprobe->getAXS_2();
            reference.third  = tprobe->getAXS_3();
        }
        else {
            mcprobe = dynamic_cast<TiledMCVolume *>(volume);
            if ( mcprobe ) {
                reference.first  = mcprobe->getAXS_1();
                reference.second = mcprobe->getAXS_2();
                reference.third  = mcprobe->getAXS_3();
            }
        }
    }

    // 2016-10-12. Giulio. when axes are negative this should be propagated to generated image
    if ( volume->getAXS_1() < 0 ) {
        if ( volume->getAXS_1() == vertical )
            reference.first = axis(-1);
        else // volume->getAXS_1() == horizontal
            reference.second = axis(-2);
    }
    if ( volume->getAXS_2() < 0 ) {
        if ( volume->getAXS_2() == horizontal )
            reference.second = axis(-2);
        else // volume->getAXS_2() == vertical
            reference.first = axis(-1);
    }

    // 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
    save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
    //if ( strcmp(saved_img_format,"tif") == 0 ) {
    //	try {
    //		// test if it is a 2D plugin
    //		bool temp = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved();
    //	}
    //	catch(iom::exception & ex){
    //		if ( strstr(ex.what(),"it is not a 2D I/O plugin") ) // it is not a 2D plugin
    //		// reset input plugin so the StackedVolume constructor set it correctly
    //		iom::IMIN_PLUGIN = "empty";
    //	}
    //}
    // 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
    iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

    for(int res_i=0; res_i< resolutions_size; res_i++)
    {
        if(resolutions[res_i])
        {
            //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            //system.
            try {
                if ( strcmp(saved_img_format,"tif") == 0 ) {
                    StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                           volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
                else if ( strcmp(saved_img_format,"Vaa3DRaw") == 0 || strcmp(saved_img_format,"Tiff3D") == 0 ) {
                    TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
                                         volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
                else if ( strcmp(saved_img_format,"Vaa3DRawMC") == 0 || strcmp(saved_img_format,"Tiff3DMC") == 0 ) {
                    for ( int c=0; c<channels; c++ ) {
                        resolution_dir = file_path[res_i].str() + chans_dir[c];

                        TiledVolume temp_vol(resolution_dir.c_str(),reference,
                                             volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                    }

                    TiledMCVolume temp_mc_vol(file_path[res_i].str().c_str(),reference,
                                              volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
                }
            }
            catch (IOException & ex)
            {
                printf("in VolumeConverter::mdataGenerator: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
                n_err++;
            }
            catch ( ... )
            {
                printf("in VolumeConverter::mdataGenerator: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
                n_err++;
            }
        }
    }

    // restore input plugin
    iom::IMIN_PLUGIN = save_imin_plugin;

    if ( n_err ) { // errors in mdat.bin creation
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VolumeConverter::mdataGenerator: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
    }
}
