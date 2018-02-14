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

#ifndef PTERASTITCHER_H
#define PTERASTITCHER_H

#include <QtGui>
#ifdef VAA3D_TERASTITCHER
#include <v3d_interface.h>
#endif
#include "QMyTabWidget.h"
#include "CTeraStitcher.h"
#include "PTabImport.h"
#include "PTabDisplComp.h"
#include "PTabDisplProj.h"
#include "PTabDisplThresh.h"
#include "PTabPlaceTiles.h"
#include "PTabMergeTiles.h"
#include "QHelpBox.h"
#include <QMainWindow>

class terastitcher::PTeraStitcher : public QMainWindow
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PTeraStitcher* uniqueInstance;
        PTeraStitcher(){}
#ifdef VAA3D_TERASTITCHER
        PTeraStitcher(V3DPluginCallback *callback, QWidget *parent);
#else
		PTeraStitcher(QWidget *parent);
#endif

        //members
#ifdef VAA3D_TERASTITCHER
        V3DPluginCallback* V3D_env;     //handle of V3D environment
#endif
        QWidget *parentWidget;          //handle of parent widget

        //menu widgets
        QMenu* fileMenu;                //"File" menu
        QAction* closeVolumeAction;     //"Close volume" menu action
        QAction* exitAction;            //"Exit" menu action
        QMenu* optionsMenu;             //"Options" menu
        QMenu* modeOptionsMenu;         //"Import" menu
        QAction* modeBasicAction;        //"Basic mode" menu action
        QAction* modeAdvancedAction;     //"Advanced mode" menu action
        QMenu* helpMenu;                //"Help" menu
        QAction* aboutAction;           //"About" menu action

        //widgets
        QMyTabWidget *tabs;             //modified tab widget
        PTabImport* tabImport;          //tab for "Import" step of the stitching process
        PTabDisplComp* tabDisplComp;    //tab for "Pairwise Displacements Computation" step of the stitching process
        PTabDisplProj* tabDisplProj;    //tab for "Displacements Projection" step of the stitching process
        PTabDisplThresh* tabDisplThres; //tab for "Displacements Thresholding" step of the stitching process
        PTabPlaceTiles* tabPlaceTiles;  //tab for "Optimal tiles placement" step of the stitching process
        PTabMergeTiles* tabMergeTiles;  //tab for "Merging tiles" step of the stitching process
        //QHelpBox* helpBox;              //helpbox
        QProgressBar* progressBar;      //progress bar
        QPushButton* startButton;       //start button
        QPushButton* startAllButton;    //start all button
        QPushButton* stopButton;        //stop button


    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
#ifdef VAA3D_TERASTITCHER
        static PTeraStitcher* instance(V3DPluginCallback *callback=0, QWidget *parent=0);
#else
		static PTeraStitcher* instance(QWidget *parent=0);
#endif
        static void uninstance();
        ~PTeraStitcher();

        //GET and SET methods
#ifdef VAA3D_TERASTITCHER
        V3DPluginCallback* getV3D_env(){return V3D_env;}
#endif
        QProgressBar* getProgressBar(){return progressBar;}
        QStatusBar* getStatusBar(){return this->statusBar();}
        void setStartButtonEnabled(bool enabled){startButton->setEnabled(enabled);}
        void setStopButtonEnabled(bool enabled){stopButton->setEnabled(enabled);}

        //resets progress bar, start/stop buttons and tab bar
        void setToReady();

        //resets all GUI elements
        void reset();

        //overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        //overrides eventFilter method of QWidget
        //bool eventFilter(QObject *object, QEvent *event);

        //very useful (not included in Qt): disables the given item of the given combobox
        static void setEnabledComboBoxItem(QComboBox* cbox, int _index, bool enabled);

        //friend declarations: allowing tabs to access PTeraStitcher members
        friend class PTabImport;
        friend class PTabDisplComp;
        friend class PTabDisplThresh;
        friend class PTabDisplProj;
        friend class PTabPlaceTiles;
        friend class PTabMergeTiles;

    public slots:

        void startButtonClicked();
        void startAllButtonClicked();
        void stopButtonClicked();

        /**********************************************************************************
        * <sendProgressBarChanged> event handler
        ***********************************************************************************/
        void progressBarChanged(int val, int minutes, int seconds, std::string message);

        /**********************************************************************************
        * Called when "Close volume" menu action is triggered.
        * All the memory allocated is released and GUI is reset".
        ***********************************************************************************/
        void closeVolumeTriggered();

        /**********************************************************************************
        * Called when "Exit" menu action is triggered or TeraFly window is closed.
        ***********************************************************************************/
        void exit();

        /**********************************************************************************
        * Called when "Help->About" menu action is triggered
        ***********************************************************************************/
        void about();

        /**********************************************************************************
        * Called when "Options->Mode" menu action has changed
        ***********************************************************************************/
        void modeChanged();
};

#endif // PTERASTITCHER_H
