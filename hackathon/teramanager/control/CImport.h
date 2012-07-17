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

#ifndef CIMPORT_H
#define CIMPORT_H

#include <QThread>
#include <string>
#include "StackedVolume.h"
#include "CPlugin.h"

class teramanager::CImport : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CImport* uniqueInstance;
        CImport() : QThread(), path(""), volume(0), AXS_1(axis(0)), AXS_2(axis(0)), AXS_3(axis(0)), VXL_1(0), VXL_2(0), VXL_3(0), reimport(false)
        {
            #ifdef TSP_DEBUG
            printf("teramanager plugin [thread %d] >> CImport created\n", this->thread()->currentThreadId());
            #endif
        }

        //automatically called when current thread is started
        void run();

        //members
        std::string path;
        axis AXS_1, AXS_2, AXS_3;
        float VXL_1, VXL_2, VXL_3;
        bool reimport, genmap;
        StackedVolume *volume;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CImport* instance()
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CImport();
            return uniqueInstance;
        }
        static void uninstance();
        ~CImport();

        //GET and SET methods
        StackedVolume* getVolume(){return volume;}
        void setPath(string new_path){path = new_path;}
        void setAxes(string axs1, string axs2, string axs3);
        void setVoxels(std::string vxl1, std::string vxl2, std::string vxl3);
        void setReimport(bool _reimport){reimport = _reimport;}
        void setGenerateMap(bool _genmap){genmap = _genmap;}

        //reset method
        void reset(){path=""; AXS_1=AXS_2=AXS_3=axis_invalid; VXL_1=VXL_2=VXL_3=0; reimport=false;}

    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread.
        **********************************************************************************/
        void sendOperationOutcome(MyException* ex, Image4DSimple* vmap_image = 0);
};

#endif // CIMPORT_H
