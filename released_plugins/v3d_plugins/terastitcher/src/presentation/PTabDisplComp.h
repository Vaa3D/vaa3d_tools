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

#ifndef PTABDISPLCOMP_H
#define PTABDISPLCOMP_H

#include <QWidget>
#include <QtGui>
#include "src/control/CPlugin.h"
#include "QMyTabWidget.h"

class terastitcher::PTabDisplComp : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PTabDisplComp* uniqueInstance;
        PTabDisplComp(QMyTabWidget* _container, int _tab_index);

        //members
        QMyTabWidget* container;        //tabs container
        int tab_index;                  //tab index

        //basic panel widgets
        QWidget* basic_panel;         //widget containing fields to be used for basing settings
        QLabel* saveproj_label;
        QLineEdit* saveproj_field;
        QPushButton* browse_button;
        QLabel* algorithm_label;
        QComboBox* algorithm_cbox;
        QLabel* subvoldims_label;
        QSpinBox* subvoldims_sbox;
        QLabel* memocc_label;
        QLineEdit* memocc_field;
        QPushButton* showAdvancedButton;
        QComboBox* channel_selection;

        //advanced panel widgets
        QWidget* advanced_panel;
        QLabel* stackrowstbp_label;
        QSpinBox* startrow_sbox;
        QLabel* to_label_1;
        QSpinBox* endrow_sbox;
        QLabel* stackcolstbp_label;
        QSpinBox* startcol_sbox;
        QLabel* to_label_2;
        QSpinBox* endcol_sbox;
        QLabel* searchregion_label;
        QSpinBox* Ysearch_sbox;
        QSpinBox* Xsearch_sbox;
        QSpinBox* Zsearch_sbox;
        QLabel* for_label_1;
        QLabel* for_label_2;
        QLabel* overlap_label;
        QSpinBox* Yoverlap_sbox;
        QSpinBox* Xoverlap_sbox;
        QLabel* for_label_3;
        QLabel* restoreSPIM_label;
        QCheckBox* restoreSPIM_cbox;


        //other widgets
        QMovie *wait_movie;             //animated wait GIF icon
        QLabel *wait_label;             //label containing animated wait GIF icon

    public:

        /**********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        ***********************************************************************************/
        static PTabDisplComp* instance(QMyTabWidget* _container, int _tab_index)
        {
            if (uniqueInstance == NULL)
                uniqueInstance = new PTabDisplComp(_container, _tab_index);
            return uniqueInstance;
        }
        static PTabDisplComp* getInstance(){return uniqueInstance;}
        static void uninstance();
        ~PTabDisplComp();

        /**********************************************************************************
        * Start/Stop methods associated to the current step.
        * They are called by the startButtonClicked/stopButtonClicked methods of <PMain>
        ***********************************************************************************/
        void start();
        void stop();

        /**********************************************************************************
        * Overrides QWidget's setEnabled(bool).
        * If the widget is enabled, its fields are filled with the informations provided by
        * the <StackedVolume> object of <CImport> instance.
        ***********************************************************************************/
        void setEnabled(bool enabled);

        //reset method
        void reset();

        //gives PMain instances public access to this class members
        friend class PMain;

    public slots:

        /**********************************************************************************
        * Updates memory occupancy's estimation field.
        * Called when either "subvoldims_sbox" or stacks intervals fields changed.
        ***********************************************************************************/
        void updateMemoryOccupancy(int);

        /**********************************************************************************
        * Opens file dialog to select the XML file to be saved.
        * Called when user clicks on "browse_button".
        ***********************************************************************************/
        void browse_button_clicked();

        /**********************************************************************************
        * Called by <CDisplComp> when the associated operation has been performed.
        * If an exception has occurred in the <CDisplComp> thread,it is propagated and man-
        * aged in the current thread (ex != 0). Otherwise, the other tabs are updated.
        ***********************************************************************************/
        void displcomp_done(MyException *ex);

        /**********************************************************************************
        * Called when <showAdvancedButton> status changed
        ***********************************************************************************/
        void showAdvancedChanged(bool status);

        /**********************************************************************************
        * Called when "channel_selection" state has changed.
        ***********************************************************************************/
        void channelSelectedChanged(int);

};

#endif // PTABDISPLCOMP_H
