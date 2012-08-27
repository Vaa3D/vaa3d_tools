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

#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "CPlugin.h"

class teramanager::CSettings
{
    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CSettings* uniqueInstance;
        CSettings()
        {
            #ifdef TMP_DEBUG
            printf("--------------------- teramanager plugin [thread unknown] >> CSettings created\n");
            #endif
            loadDefaultSettings();
            readSettings();
        }

        //members
        string volumePathLRU;
        int volMapSizeLimit;
        int VOIdimV;
        int VOIdimH;
        int VOIdimD;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CSettings* instance()
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CSettings();
            return uniqueInstance;
        }
        static void uninstance();
        ~CSettings();

        //GET and SET methods
        string getVolumePathLRU(){return volumePathLRU;}
        int getVolMapSizeLimit(){return volMapSizeLimit;}
        int getVOIdimV(){return VOIdimV;}
        int getVOIdimH(){return VOIdimH;}
        int getVOIdimD(){return VOIdimD;}
        void setVolumePathLRU(string _volumePathLRU){volumePathLRU = _volumePathLRU;}
        void setVolMapSizeLimit(int _volMapSizeLimit){volMapSizeLimit = _volMapSizeLimit;}
        void setVOIdimV(int _VOIdimV){VOIdimV = _VOIdimV;}
        void setVOIdimH(int _VOIdimH){VOIdimH = _VOIdimH;}
        void setVOIdimD(int _VOIdimD){VOIdimD = _VOIdimD;}

        //save and restore application settings
        void writeSettings();
        void readSettings();

        //load default settings
        void loadDefaultSettings();
};

#endif // CSETTINGS_H
