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

#ifndef CCONVERTER_H
#define CCONVERTER_H

#include <QThread>
#include <string>
#include "CPlugin.h"
#include "../presentation/PConverter.h"
#include "../core/VolumeConverter/VolumeConverter.h"

class teramanager::CConverter : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling the static method "istance(...)"
        **********************************************************************************/
        static CConverter* uniqueInstance;
        CConverter() : QThread(), inVolPath(undefined_str), inVolFormat(undefined_str), fileMode(false), conversionMode(false), resolutions(0),
            resolutionsSize(0), stacksWidth(undefined_int32), stacksHeight(undefined_int32), outVolPath(undefined_str), outVolFormat(undefined_str), vc(0)
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread *] >> CConverter::CConverter()\n");
            printf("--------------------- teramanager plugin [thread *] >> CConverter created\n");
            #endif
        }

        //automatically called when current thread is started
        void run();

        //members
        string inVolPath;           //absolute path of the folder or file containing the volume to be converted
        string inVolFormat;         //the unique ID of the volume's input format
        bool fileMode;              //whether the volume to be imported is stored into a file (fileMode=true) or a folder (fileMode=false)
        bool conversionMode;        //whether the conversion mode is active or not (as it is initially, when the import mode is activated)
        bool *resolutions;          //array of resolutions activation flags
        int resolutionsSize;        //size of <resolutions>
        int stacksWidth;            //width of each stack after conversion
        int stacksHeight;           //height of each stack after conversion
        int stacksDepth;            //depth of each stack after conversion (optional)
        string outVolPath;          //absolute path of the folder where to store the converted volume
        string outVolFormat;        //the unique ID of the volume's output format
        VolumeConverter *vc;        //handle of the <VolumeConverter> object which is responsible of volume conversion from the given format


    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CConverter* instance()
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CConverter();
            return uniqueInstance;
        }
        static void uninstance();
        ~CConverter();

        //GET and SET methods
        void setMembers(PConverter* pConverter) throw (MyException);
        bool isConversionModeEnabled(){return conversionMode;}
        VolumeConverter* getVolumeConverter() throw (MyException)
        {
            if(vc == 0)
                throw MyException("in CConverter::getVolumeConverter(): volume converter object does not exist");
            return vc;
        }


        //reset method
        void reset()
        {
            inVolPath = undefined_str;
            inVolFormat = undefined_str;
            vc = 0;
            conversionMode = false;
            resolutions = 0;
            resolutionsSize = 0;
            stacksWidth  = undefined_int32;
            stacksHeight = undefined_int32;
            stacksDepth = undefined_int32;
            outVolPath = undefined_str;
            outVolFormat = undefined_str;
            fileMode = false;
        }

    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread.
        **********************************************************************************/
        void sendOperationOutcome(MyException* ex);
};

#endif // CCONVERTER_H
