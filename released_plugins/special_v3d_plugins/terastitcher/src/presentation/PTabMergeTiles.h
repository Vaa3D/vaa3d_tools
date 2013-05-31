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

#ifndef PTABMERGETILES_H
#define PTABMERGETILES_H

#include <QWidget>
#include <QtGui>
#include "src/control/CPlugin.h"
#include "QMyTabWidget.h"
#include "S_config.h"

class terastitcher::PTabMergeTiles : public QWidget
{
    Q_OBJECT

    /*********************************************************************************
    * Singleton design pattern: this class can have one instance only,  which must be
    * instantiated by calling static method "istance(...)"
    **********************************************************************************/
    static PTabMergeTiles* uniqueInstance;
    PTabMergeTiles(QMyTabWidget* _container, int _tab_index);

    //members
    QMyTabWidget* container;        //tabs container
    int tab_index;                  //tab index

    //basic settings panel widgets
    QWidget* basic_panel;
    QLabel* savedir_label;
    QLineEdit* savedir_field;
    QPushButton* browse_button;
    QLabel* resolutions_label;
    QLabel* resolutions_size_label;
    QLabel* resolutions_save_label;
    QLabel* resolutions_view_label;
    QLabel* outputs_label;
    QLabel* resolutions_fields[S_MAX_MULTIRES];
    QLabel* resolutions_sizes[S_MAX_MULTIRES];
    QButtonGroup* resolutions_save_selection;
    QCheckBox* resolutions_save_cboxs[S_MAX_MULTIRES];
    QCheckBox* resolutions_view_cboxs[S_MAX_MULTIRES];
    QButtonGroup* volumeformat_selection;
    QLabel* volumeformat_label;
    QCheckBox* multistack_cbox;
    QCheckBox* singlestack_cbox;
    QSpinBox* stackheight_field;
    QSpinBox* stackwidth_field;
    QLabel* byLabel;
    QLabel* memocc_label;
    QLineEdit* memocc_field;
    QPushButton* showAdvancedButton;

    //advanced settings panel widgets
    QWidget* advanced_panel;
    QLabel* volumeportion_label;
    QSpinBox *row0_field, *row1_field, *col0_field, *col1_field;
    QSpinBox *slice0_field, *slice1_field;
    QCheckBox* excludenonstitchables_cbox;
    QLabel* blendingalgo_label;
    QComboBox* blendingalbo_cbox;
    QLabel* restoreSPIM_label;
    QComboBox* restoreSPIM_cbox;
    QLabel* imgformat_label;
    QComboBox* imgformat_cbox;
    QLabel* imgdepth_label;
    QComboBox* imgdepth_cbox;


    //other widgets
    QMovie *wait_movie;             //animated wait GIF icon
    QLabel *wait_label;             //label containing animated wait GIF icon

public:

    /**********************************************************************************
    * Singleton design pattern: this class can have one instance only,  which must be
    * instantiated by calling static method "istance(...)"
    ***********************************************************************************/
    static PTabMergeTiles* instance(QMyTabWidget* _container, int _tab_index)
    {
        if (uniqueInstance == NULL)
            uniqueInstance = new PTabMergeTiles(_container, _tab_index);
        return uniqueInstance;
    }
    static PTabMergeTiles* getInstance(){return uniqueInstance;}
    static void uninstance();
    ~PTabMergeTiles();

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

    //gives PMain and CMergeTiles instances public access to this class members
    friend class PMain;
    friend class terastitcher::CMergeTiles;

public slots:

    /**********************************************************************************
    * Opens the dialog to select the directory where the stitched volume has to be saved.
    * Called when user clicks on "browse_button".
    ***********************************************************************************/
    void browse_button_clicked();

    /**********************************************************************************
    * Called when <excludenonstitchables_cbox> combobox state changed.
    * Inferior and superior limits of spinboxes are recomputed.
    ***********************************************************************************/
    void excludenonstitchables_changed();

    /**********************************************************************************
    * Called when <row0_field>, <row1_field>, <col0_field> or <col1_field> changed.
    * Inferior and superior limits of <slice_[]_cbox> spinboxes are recomputed.
    ***********************************************************************************/
    void stacksinterval_changed();

    /**********************************************************************************
    * Called when <multistack_cbox> or <multistack_cbox> state changed.
    ***********************************************************************************/
    void volumeformat_changed();

    /**********************************************************************************
    * Called when <resolutions_view_cboxs[i]> changed
    ***********************************************************************************/
    void viewinVaa3D_changed(int checked);

    /**********************************************************************************
    * Called when <resolutions_save_cboxs[i]> changed
    ***********************************************************************************/
    void save_changed(int checked);

    /**********************************************************************************
    * Updates widgets contents
    ***********************************************************************************/
    void updateContent();

    /**********************************************************************************
    * Called when <showAdvancedButton> status changed
    ***********************************************************************************/
    void showAdvancedChanged(bool status);

    /**********************************************************************************
    * Called when the corresponding spinboxes changed.
    * New maximum/minimum values are set according to the status of spinboxes.
    ***********************************************************************************/
    void row0_field_changed(int val);
    void row1_field_changed(int val);
    void col0_field_changed(int val);
    void col1_field_changed(int val);
    void slice0_field_changed(int val);
    void slice1_field_changed(int val);

    /**********************************************************************************
    * Called by <CMergeTiles> when the associated operation has been performed.
    * If an exception has occurred in the <CMergeTiles> thread,it is propagated and man-
    * aged in the current thread (ex != 0). Otherwise, if a valid  3D image  is passed,
    * it is shown in Vaa3D.
    ***********************************************************************************/
    void merging_done(MyException *ex, Image4DSimple* img);

};

#endif // PTABMERGETILES_H
