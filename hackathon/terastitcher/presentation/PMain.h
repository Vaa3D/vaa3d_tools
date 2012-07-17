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

#ifndef PMAIN_GUI_H
#define PMAIN_GUI_H

#include <QtGui>
#include <v3d_interface.h>
#include "QMyTabWidget.h"
#include "control/CPlugin.h"
#include "PTabImport.h"
#include "PTabDisplComp.h"
#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabPlaceTiles.h"
#include "PTabMergeTiles.h"
#include "StackedVolume.h"

class terastitcher::PMain : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* uniqueInstance;
        PMain(){}
        PMain(V3DPluginCallback *callback, QWidget *parent);

        //members
        V3DPluginCallback* V3D_env;     //handle of V3D environment
        QWidget *parentWidget;          //handle of parent widget

        //widgets
        QMyTabWidget *tabs;             //modified tab widget
        PTabImport* tabImport;          //tab for "Import" step of the stitching process
        PTabDisplComp* tabDisplComp;    //tab for "Pairwise Displacements Computation" step of the stitching process
        PTabDisplProj* tabDisplProj;    //tab for "Displacements Projection" step of the stitching process
        PTabDisplThresh* tabDisplThres; //tab for "Displacements Thresholding" step of the stitching process
        PTabPlaceTiles* tabPlaceTiles;  //tab for "Optimal tiles placement" step of the stitching process
        PTabMergeTiles* tabMergeTiles;  //tab for "Merging tiles" step of the stitching process
        QProgressBar* progressBar;      //progress bar
        QPushButton* startButton;       //start button
        QPushButton* startAllButton;    //start all button
        QPushButton* stopButton;        //stop button
        QStatusBar* statusBar;          //status bar


    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PMain* instance(V3DPluginCallback *callback=0, QWidget *parent=0);
        static void uninstance();
        ~PMain();

        //GET and SET methods
        V3DPluginCallback* getV3D_env(){return V3D_env;}
        QProgressBar* getProgressBar(){return progressBar;}
        QStatusBar* getStatusBar(){return statusBar;}
        void setStartButtonEnabled(bool enabled){startButton->setEnabled(enabled);}
        void setStopButtonEnabled(bool enabled){stopButton->setEnabled(enabled);}

        //resets progress bar, start/stop buttons and tab bar
        void resetGUI();

        //overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        /**********************************************************************************
        * Called by algorithms running from different threads.
        * Emits <sendProgressBarChanged> signal
        ***********************************************************************************/
        void emitProgressBarChanged(int val, int minutes, int seconds, const char* message = 0)
        {emit sendProgressBarChanged(val, minutes, seconds, message);}

    public slots:

        void startButtonClicked();
        void startAllButtonClicked();
        void stopButtonClicked();

        /**********************************************************************************
        * <sendProgressBarChanged> event handler
        ***********************************************************************************/
        void progressBarChanged(int val, int minutes, int seconds, const char* message);

    signals:

        /*********************************************************************************
        * Carries progress bar informations (progress percentage and remaining minutes).
        **********************************************************************************/
        void sendProgressBarChanged(int val, int minutes, int seconds, const char* message);
};

#endif // PMAIN_GUI_H
