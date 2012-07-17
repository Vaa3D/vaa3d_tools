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
#include "presentation/PMain.h"

class teramanager::CLoadSubvolume : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CLoadSubvolume* uniqueInstance;
        CLoadSubvolume(PMain* handle) : QThread()
        {
            #ifdef TSP_DEBUG
            printf("teramanager plugin [thread %d] >> CLoadSubvolume created\n", this->thread()->currentThreadId());
            #endif

            pMainHandle = handle;
        }

        //automatically called when current thread is started
        void run();

        //members
        PMain* pMainHandle;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CLoadSubvolume* instance(PMain* handle)
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CLoadSubvolume(handle);
            return uniqueInstance;
        }
        static void uninstance();
        ~CLoadSubvolume();

        //GET and SET methods
        void setPMainHandle(PMain* handle){pMainHandle = handle;}

    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread as well as image
        * data in <Image4DSimple> object.
        **********************************************************************************/
        void sendOperationOutcome(MyException* ex, Image4DSimple* img);
};

#endif // CLOADSUBVOLUME_H
