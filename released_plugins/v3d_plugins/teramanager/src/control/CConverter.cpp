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

#include "CConverter.h"
#include "CPlugin.h"
#include "RawVolume.h"
#include "SimpleVolume.h"
#include "SimpleVolumeRaw.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "VolumeConverter.h"
#include "iomanager.config.h"

using namespace teramanager;
using namespace iim;

CConverter* CConverter::uniqueInstance = 0;

void CConverter::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = 0;
    }
}

CConverter::~CConverter()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(resolutions)
        delete[] resolutions;
    resolutions = 0;

    /**/itm::debug(itm::LEV1, "object successfully DESTROYED", __itm__current__function__);
}

void CConverter::setMembers(PConverter* pConverter) throw (RuntimeException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    char errMsg[1024];

    conversionMode = pConverter->conversion_panel->isEnabled();

    if(!conversionMode)
    {
        inVolPath = pConverter->inPathField->text().toStdString();
        inVolFormat = pConverter->inFormatCBox->currentText().toStdString();
        try
        {
            fileMode = !VirtualVolume::isHierarchical(inVolFormat) && !pConverter->timeSeriesCheckBox->isChecked();
        }
        catch( iim::IOException& exception)  {throw RuntimeException(exception.what());}
        time_series = pConverter->timeSeriesCheckBox->isChecked();
    }
    else
    {
        outVolPath = pConverter->outPathField->text().toStdString();
        outVolFormat = pConverter->outFormatCBox->currentText().toStdString();
        if(outVolFormat.compare(iim::STACKED_FORMAT)        != 0 &&
           outVolFormat.compare(iim::TILED_FORMAT)          != 0 &&
           outVolFormat.compare(iim::TILED_MC_FORMAT)       != 0 &&
           outVolFormat.compare(iim::TILED_TIF3D_FORMAT)    != 0 &&
           outVolFormat.compare(iim::TILED_MC_TIF3D_FORMAT) != 0)
        {
            sprintf(errMsg, "Output format \"%s\" not yet supported", outVolFormat.c_str());
            throw RuntimeException(errMsg);
        }
        resolutionsSize = pConverter->resolutionsCboxs.size();
        if(resolutionsSize <= 0)
            throw RuntimeException("No resolutions selected");
        if(resolutionsSize > iim::TMITREE_MAX_HEIGHT)
        {
            sprintf(errMsg, "Exceeded the maximum number (%d) of resolutions that can be produced", iim::TMITREE_MAX_HEIGHT);
            throw RuntimeException(errMsg);
        }
        resolutions = new bool[iim::TMITREE_MAX_HEIGHT];
        for(int i=0; i<iim::TMITREE_MAX_HEIGHT; i++)
        {
            if(i < resolutionsSize)
                resolutions[i] = pConverter->resolutionsCboxs[i]->isChecked();
            else
                resolutions[i] = false;
        }
        stacksWidth = pConverter->blockWidthField->value();
        stacksHeight = pConverter->blockHeightField->value();
        stacksDepth = pConverter->blockDepthField->value();
        downsamplingMethod = pConverter->downsamplingCbox->currentIndex();
        time_series = pConverter->timeSeriesCheckBox->isChecked();
        if(downsamplingMethod == 0)
            downsamplingMethod = HALVE_BY_MEAN;
        else if(downsamplingMethod == 1)
            downsamplingMethod = HALVE_BY_MAX;
        else
            throw RuntimeException(strprintf("Unsupported downsampling method").c_str());
    }
}

//automatically called when current thread is started
void CConverter::run()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        // TO BE REMOVED !!!
        iom::IMIN_PLUGIN = "tiff3D";
        iom::IMOUT_PLUGIN = "tiff3D";

        if(!conversionMode)
        {
            vc = new VolumeConverter();
            vc->setSrcVolume(inVolPath.c_str(), inVolFormat.c_str(), "RGB", time_series);
        }
        else
        {
            //first checking that the given folder exists
            if(!fileMode && !QDir(outVolPath.c_str()).exists())
                throw RuntimeException(QString("Unable to find the directory \"").append(outVolPath.c_str()).append("\"").toStdString().c_str());

            vc->convertTo(outVolPath, outVolFormat, iim::NUL_IMG_DEPTH, time_series, resolutions, stacksHeight, stacksWidth, stacksDepth, downsamplingMethod);
        }

        //everything went OK
        emit sendOperationOutcome(0);

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( iim::IOException& exception)  {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( iom::exception& exception)    {reset(); emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch( RuntimeException& exception)  {emit sendOperationOutcome(new RuntimeException(exception.what()));}
    catch(const char* error)        {emit sendOperationOutcome(new RuntimeException(error));}
    catch(...)                      {emit sendOperationOutcome(new RuntimeException("Unknown error occurred"));}
}
