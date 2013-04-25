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
#include "VolumeConverter.h"

using namespace teramanager;

CConverter* CConverter::uniqueInstance = NULL;

void CConverter::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CConverter::uninstance()\n");
    #endif

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CConverter::~CConverter()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CConverter::~CConverter()\n");
    printf("--------------------- teramanager plugin [thread *] >> CConverter destroyed\n");
    #endif
    if(resolutions)
        delete[] resolutions;
}

void CConverter::setMembers(PConverter* pConverter) throw (MyException)
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CConverter::setMembers()\n");
    #endif

    conversionMode = pConverter->conversion_panel->isEnabled();

    if(!conversionMode)
    {
        volPath = pConverter->volpathField->text().toStdString();
        volFormat = pConverter->volformatCombobox->currentText().toStdString();
        if(volFormat.compare("TeraStitcher") == 0)
        {
            volFormat = STACKED_FORMAT;
            fileMode = false;
        }
        else if(volFormat.compare("Image series") == 0)
        {
            volFormat = SIMPLE_FORMAT;
            fileMode = false;
        }
        else if(volFormat.compare("Vaa3D raw") == 0)
        {
            volFormat = RAW_FORMAT;
            fileMode = true;
        }
        else
            throw MyException("in CConverter::setMembers(): unable to recognize the selected volume format");
    }
    else
    {
        volOutPath = pConverter->voloutpathField->text().toStdString();
        resolutionsSize = pConverter->resolutionsNumber;
        if(resolutionsSize <= 0)
            throw MyException("in CConverter::setMembers(): no selected resolutions found");
        if(resolutionsSize > S_MAX_MULTIRES)
        {
            char errMsg[IM_STATIC_STRINGS_SIZE];
            sprintf(errMsg, "in CConverter::setMembers(): exceeded the maximum number (%d) of resolutions that can be produced", S_MAX_MULTIRES);
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
    }
}

//automatically called when current thread is started
void CConverter::run()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread *] >> CConverter::run()\n");
    #endif

    try
    {
        if(!conversionMode)
        {
            //first checking that the given filepath or folder exists
            if(!fileMode && !QDir(volPath.c_str()).exists())
                throw MyException(QString("Unable to find the directory \"").append(volPath.c_str()).append("\"").toStdString().c_str());
            if(fileMode && !QFile(volPath.c_str()).exists())
                throw MyException(QString("Unable to find the file \"").append(volPath.c_str()).append("\"").toStdString().c_str());

            vc = new VolumeConverter();
            vc->setSrcVolume(volPath.c_str(), volFormat.c_str(), "RGB");
        }
        else
        {
            //first checking that the given folder exists
            if(!fileMode && !QDir(volOutPath.c_str()).exists())
                throw MyException(QString("Unable to find the directory \"").append(volOutPath.c_str()).append("\"").toStdString().c_str());

            vc->generateTiles(volOutPath, resolutions, stacksHeight, stacksWidth);
        }

        //everything went OK
        emit sendOperationOutcome(0);
    }
    catch( MyException& exception)  {emit sendOperationOutcome(new MyException(exception.what()));}
    catch(const char* error)        {emit sendOperationOutcome(new MyException(error));}
    catch(...)                      {emit sendOperationOutcome(new MyException("Unknown error occurred"));}
}
