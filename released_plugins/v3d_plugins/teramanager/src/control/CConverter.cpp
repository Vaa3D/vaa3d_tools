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
#include "../core/ImageManager/RawVolume.h"
#include "../core/ImageManager/SimpleVolume.h"
#include "../core/ImageManager/SimpleVolumeRaw.h"
#include "../core/ImageManager/TiledVolume.h"
#include "../core/ImageManager/TiledMCVolume.h"
#include "../core/VolumeConverter/VolumeConverter.h"

using namespace teramanager;

CConverter* CConverter::uniqueInstance = NULL;

void CConverter::uninstance()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
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

void CConverter::setMembers(PConverter* pConverter) throw (MyException)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    char errMsg[1024];

    conversionMode = pConverter->conversion_panel->isEnabled();

    if(!conversionMode)
    {
        inVolPath = pConverter->inPathField->text().toStdString();
        inVolFormat = pConverter->inFormatCBox->currentText().toStdString();
        if(inVolFormat.compare("Image series (tiled)") == 0)
        {
            inVolFormat = STACKED_FORMAT;
            fileMode = false;
        }
        else if(inVolFormat.compare("Image series (nontiled)") == 0)
        {
            inVolFormat = SIMPLE_FORMAT;
            fileMode = false;
        }
        else if(inVolFormat.compare("Vaa3D raw (tiled, RGB)") == 0)
        {
            inVolFormat = TILED_FORMAT;
            fileMode = false;
        }
        else if(inVolFormat.compare("Vaa3D raw (tiled, 4D)") == 0)
        {
            inVolFormat = TILED_MC_FORMAT;
            fileMode = false;
        }
        else if(inVolFormat.compare("Vaa3D raw") == 0)
        {
            inVolFormat = RAW_FORMAT;
            fileMode = true;
        }
        else if(inVolFormat.compare("Vaa3D raw (series)") == 0)
        {
            inVolFormat = SIMPLE_RAW_FORMAT;
            fileMode = false;
        }
        else
        {
            sprintf(errMsg, "Input format \"%s\" not yet supported", inVolFormat.c_str());
            throw MyException(errMsg);
        }
    }
    else
    {
        outVolPath = pConverter->outPathField->text().toStdString();
        outVolFormat = pConverter->outFormatCBox->currentText().toStdString();
        if(outVolFormat.compare("Image series (tiled)") == 0)
        {
            outVolFormat = STACKED_FORMAT;
            fileMode = false;
        }
        else if(outVolFormat.compare("Vaa3D raw (tiled, RGB)") == 0)
        {
            outVolFormat = TILED_FORMAT;
            fileMode = true;
        }
        else if(outVolFormat.compare("Vaa3D raw (tiled, 4D)") == 0)
        {
            outVolFormat = TILED_MC_FORMAT;
            fileMode = true;
        }
        else
        {
            sprintf(errMsg, "Output format \"%s\" not yet supported", outVolFormat.c_str());
            throw MyException(errMsg);
        }
        resolutionsSize = pConverter->resolutionsNumber;
        if(resolutionsSize <= 0)
            throw MyException("No resolutions selected");
        if(resolutionsSize > S_MAX_MULTIRES)
        {
            sprintf(errMsg, "Exceeded the maximum number (%d) of resolutions that can be produced", S_MAX_MULTIRES);
            throw MyException(errMsg);
        }
        resolutions = new bool[S_MAX_MULTIRES];
        for(int i=0; i<S_MAX_MULTIRES; i++)
        {
            if(i < resolutionsSize)
                resolutions[i] = pConverter->resolutionsCboxs[i]->isChecked();
            else
                resolutions[i] = false;
        }
        stacksWidth = pConverter->stacksWidthField->value();
        stacksHeight = pConverter->stacksHeightField->value();
        stacksDepth = pConverter->stacksDepthField->value();
    }
}

//automatically called when current thread is started
void CConverter::run()
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    try
    {
        if(!conversionMode)
        {
            //first checking that the given filepath or folder exists
            if(!fileMode && !QDir(inVolPath.c_str()).exists())
                throw MyException(QString("Unable to find the directory \"").append(inVolPath.c_str()).append("\"").toStdString().c_str());
            if(fileMode && !QFile(inVolPath.c_str()).exists())
                throw MyException(QString("Unable to find the file \"").append(inVolPath.c_str()).append("\"").toStdString().c_str());

            vc = new VolumeConverter();
            vc->setSrcVolume(inVolPath.c_str(), inVolFormat.c_str(), "RGB");
        }
        else
        {
            //first checking that the given folder exists
            if(!fileMode && !QDir(outVolPath.c_str()).exists())
                throw MyException(QString("Unable to find the directory \"").append(outVolPath.c_str()).append("\"").toStdString().c_str());

            if(outVolFormat.compare(STACKED_FORMAT) == 0)
                vc->generateTiles(outVolPath, resolutions, stacksHeight, stacksWidth);
            else if(outVolFormat.compare(TILED_FORMAT) == 0)
                vc->generateTilesVaa3DRaw(outVolPath, resolutions, stacksHeight, stacksWidth, stacksDepth);
            else if(outVolFormat.compare(TILED_MC_FORMAT) == 0)
                vc->generateTilesVaa3DRawMC(outVolPath, resolutions, stacksHeight, stacksWidth, stacksDepth);
            else
            {
                char errMsg[1024];
                sprintf(errMsg, "Output format \"%s\" not yet supported", outVolFormat.c_str());
                throw MyException(errMsg);
            }
        }

        //everything went OK
        emit sendOperationOutcome(0);

        /**/itm::debug(itm::LEV1, "EOF", __itm__current__function__);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(new MyException(exception.what()));}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error));}
    catch(...)                      {emit sendOperationOutcome(new MyException("Unknown error occurred"));}
}
