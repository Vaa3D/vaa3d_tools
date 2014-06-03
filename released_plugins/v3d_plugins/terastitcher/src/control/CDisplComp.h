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

#ifndef CDISPLCOMP_H
#define CDISPLCOMP_H

#include <QThread>
#include <string>
#include "vmStackedVolume.h"
#include "StackStitcher.h"
#include "CPlugin.h"

class terastitcher::CDisplComp : public QThread
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CDisplComp* uniqueInstance;
        CDisplComp() : QThread()
        {
            #ifdef TSP_DEBUG
            printf("TeraStitcher plugin [thread %d] >> CDisplComp created\n", this->thread()->currentThreadId());
            #endif

            reset();
        }

        //automatically called when current thread is started
        void run();

        //members
        std::string saveproj_path;
        int algo, subvol_dim, row0, row1, col0, col1, Vrad, Hrad, Drad, Voverlap, Hoverlap;
        bool restoreSPIM;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static CDisplComp* instance()
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new CDisplComp();
            return uniqueInstance;
        }
        static void uninstance();
        ~CDisplComp();

        //GET and SET methods
        void setProjPath(string new_path){saveproj_path = new_path;}
        void setAlgorithm(int _algo){algo = _algo;}
        void setSubvolDim(int new_subvoldim){subvol_dim = new_subvoldim;}
        void setStacksIntervals(int _row0, int _row1, int _col0, int _col1){row0=_row0; row1=_row1; col0=_col0; col1=_col1;}
        void setSearchRadius(int _Vrad, int _Hrad, int _Drad){Vrad = _Vrad; Hrad = _Hrad; Drad = _Drad;}
        void setOverlap(int _Voverlap, int _Hoverlap){Voverlap = _Voverlap; Hoverlap = _Hoverlap;}
        void setRestoreSPIM(bool _restoreSPIM){restoreSPIM = _restoreSPIM;}

        //reset method
        void reset()
        {
            algo = -1;
            subvol_dim = S_SUBVOL_DIM_D_DEFAULT;
            row0 = row1 = col0 = col1 = -1;
            Vrad = Hrad = Drad = S_DISPL_SEARCH_RADIUS_DEF;
            Voverlap = Hoverlap = -1;
            restoreSPIM = false;
        }

    signals:

        /*********************************************************************************
        * Carries the outcome of the operation associated to this thread.
        **********************************************************************************/
        void sendOperationOutcome(MyException* ex);

};

#endif // CDISPLCOMP_H
