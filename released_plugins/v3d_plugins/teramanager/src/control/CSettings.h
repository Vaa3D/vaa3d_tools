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
#include <vector>
#include <algorithm>

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
             /**/itm::debug(itm::LEV1, 0, __itm__current__function__);
            loadDefaultSettings();
            readSettings();
        }

        //TeraFly members
        string volumePathLRU;
        std::list<string> volumePathHistory;
        string annotationPathLRU;
        int VOIdimV;
        int VOIdimH;
        int VOIdimD;
        int VOIdimT;
        int traslX;             //traslation percentage with respect to the actual VOI along X axis
        int traslY;             //traslation percentage with respect to the actual VOI along Y axis
        int traslZ;             //traslation percentage with respect to the actual VOI along Z axis
        int traslT;             //traslation percentage with respect to the actual VOI along T axis
        bool annotationSpaceUnlimited;
        int annotationMarkersDeleteROISampling;
        int annotationCurvesDims;
        bool annotationCurvesAspectTube;
        int annotationVirtualMargin;
        int annotationMarkerSize;

        //TeraConverter members
        string volumeConverterInputPathLRU;
        string volumeConverterOutputPathLRU;
        string volumeConverterInputFormatLRU;
        string volumeConverterOutputFormatLRU;
        int volumeConverterStacksWidthLRU;
        int volumeConverterStacksHeightLRU;
        int volumeConverterStacksDepthLRU;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CSettings* instance()
        {
            if (uniqueInstance == 0)
                uniqueInstance = new CSettings();
            return uniqueInstance;
        }
        static void uninstance();
        ~CSettings();

        //GET and SET methods for TeraFly
        string getVolumePathLRU(){return volumePathLRU;}
        std::list<string>& getVolumePathHistory(){return volumePathHistory;}
        string getAnnotationPathLRU(){return annotationPathLRU;}
        int getVOIdimV(){return VOIdimV;}
        int getVOIdimH(){return VOIdimH;}
        int getVOIdimD(){return VOIdimD;}
        int getVOIdimT(){return VOIdimT;}
        int getTraslX(){return traslX;}
        int getTraslY(){return traslY;}
        int getTraslZ(){return traslZ;}
        int getTraslT(){return traslT;}
        bool getAnnotationSpaceUnlimited(){return annotationSpaceUnlimited;}
        int getAnnotationMarkersDeleteROISampling(){return annotationMarkersDeleteROISampling;}
        int getAnnotationCurvesDims(){return annotationCurvesDims;}
        bool getAnnotationCurvesAspectTube(){return annotationCurvesAspectTube;}
        int getAnnotationVirtualMargin(){return annotationVirtualMargin;}
        int getAnnotationMarkerSize(){return annotationMarkerSize;}

        void setVolumePathLRU(string _volumePathLRU){volumePathLRU = _volumePathLRU;}
        void addVolumePathToHistory(string _volumePath){
            if(volumePathHistory.size() > 10)
                volumePathHistory.pop_front();
            volumePathHistory.push_back(_volumePath);
            volumePathHistory.erase(unique(volumePathHistory.begin(), volumePathHistory.end()), volumePathHistory.end());
        }
        void clearVolumePathHistory(){
            volumePathHistory.clear();
            writeSettings();
        }

        void setAnnotationPathLRU(string _annotationPathLRU){annotationPathLRU = _annotationPathLRU;}
        void setVOIdimV(int _VOIdimV){VOIdimV = _VOIdimV;}
        void setVOIdimH(int _VOIdimH){VOIdimH = _VOIdimH;}
        void setVOIdimD(int _VOIdimD){VOIdimD = _VOIdimD;}
        void setVOIdimT(int _VOIdimT){VOIdimT = _VOIdimT;}
        void setTraslX(int _traslX){traslX = _traslX;}
        void setTraslY(int _traslY){traslY = _traslY;}
        void setTraslZ(int _traslZ){traslZ = _traslZ;}
        void setTraslT(int _traslT){traslT = _traslT;}
        void setAnnotationSpaceUnlimited(bool _unl){annotationSpaceUnlimited = _unl;}
        void setAnnotationMarkersDeleteROISampling(int newval){annotationMarkersDeleteROISampling = newval;}
        void setAnnotationCurvesDims(int newval){annotationCurvesDims = newval;}
        void setAnnotationCurvesAspectTube(bool newval){annotationCurvesAspectTube = newval;}
        void setAnnotationVirtualMargin(int newval){annotationVirtualMargin = newval;}
        void setAnnotationMarkerSize(int newval){annotationMarkerSize = newval;}

        //GET and SET methods for TeraConverter
        string getVCInputPath(){return volumeConverterInputPathLRU;}
        string getVCOutputPath(){return volumeConverterOutputPathLRU;}
        string getVCInputFormat(){return volumeConverterInputFormatLRU;}
        string getVCOutputFormat(){return volumeConverterOutputFormatLRU;}
        int getVCStacksWidth(){return volumeConverterStacksWidthLRU;}
        int getVCStacksHeight(){return volumeConverterStacksHeightLRU;}
        int getVCStacksDepth(){return volumeConverterStacksDepthLRU;}
        void setVCInputPath(string newval){volumeConverterInputPathLRU = newval;}
        void setVCOutputPath(string newval){volumeConverterOutputPathLRU = newval;}
        void setVCInputFormat(string newval){volumeConverterInputFormatLRU = newval;}
        void setVCOutputFormat(string newval){volumeConverterOutputFormatLRU = newval;}
        void setVCStacksWidth(int newval){volumeConverterStacksWidthLRU = newval;}
        void setVCStacksHeight(int newval){volumeConverterStacksHeightLRU = newval;}
        void setVCStacksDepth(int newval){volumeConverterStacksDepthLRU = newval;}

        //save and restore application settings
        void writeSettings();
        void readSettings();

        //load default settings
        void loadDefaultSettings();
};

#endif // CSETTINGS_H
