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

#ifndef CLOADSUBVOLUME_H
#define CLOADSUBVOLUME_H

#include <QThread>
#include <string>
#include "CPlugin.h"
#include "CImport.h"

class teramanager::CVolume : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CVolume* uniqueInstance;
        CVolume() : QThread()
        {
            #ifdef TMP_DEBUG
            printf("teramanager plugin [thread %d] >> CLoadSubvolume created\n", this->thread()->currentThreadId());
            #endif

            voi_data = 0;
            V0 = V1 = H0 = H1 = D0 = D1 = -1;
            show_in_a_new_window = true;
        }

        //automatically called when current thread is started
        void run();

        //members
        int V0,V1,H0,H1,D0,D1;      //Volume Of Interest coordinates
        uint8* voi_data;            //Volume Of Interest data
        bool show_in_a_new_window;  //tells PMain where the loaded data should be shown

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CVolume* instance()
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CVolume();
            return uniqueInstance;
        }
        static void uninstance();
        ~CVolume();

        //GET and SET methods
        uint8* getVOI_Data(){return voi_data;}
        void resetVOI_Data(){voi_data = 0;}
        int getV0(){return V0;}
        int getV1(){return V1;}
        int getH0(){return H0;}
        int getH1(){return H1;}
        int getD0(){return D0;}
        int getD1(){return D1;}
        bool getShowInNewWindow(){return show_in_a_new_window;}
        void setVOI(int _V0, int _V1, int _H0, int _H1, int _D0, int _D1)
        {
            StackedVolume* volume = CImport::instance()->getVolume();
            V0 = MAX(_V0, 0);
            V1 = MIN(_V1, volume->getDIM_V());
            H0 = MAX(_H0, 0);
            H1 = MIN(_H1, volume->getDIM_H());
            D0 = MAX(_D0, 0);
            D1 = MIN(_D1, volume->getDIM_D());
            printf("VOI set at [%d - %d], [%d - %d], [%d - %d]\n", V0, V1, H0, H1, D0, D1);
        }
        void setShowInNewWindow(bool show){show_in_a_new_window = show;}

        //reset method
        void reset()
        {
            if(voi_data)
                delete[] voi_data;
            voi_data = 0;
            V0 = V1 = H0 = H1 = D0 = D1 = -1;
        }

    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread as well as image
        * data in <Image4DSimple> object.
        **********************************************************************************/
        void sendOperationOutcome(MyException* ex);
};

#endif // CLOADSUBVOLUME_H
