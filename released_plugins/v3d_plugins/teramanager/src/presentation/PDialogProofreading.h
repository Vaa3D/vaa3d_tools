#ifndef PDIALOGPROOFREADING_H
#define PDIALOGPROOFREADING_H

#include <vector>
#include <QWidget>
#include "../control/CPlugin.h"
#include "PMain.h"

class teramanager::PDialogProofreading : public QWidget
{
    Q_OBJECT

    private:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PDialogProofreading* uniqueInstance;
        PDialogProofreading(itm::PMain *_parent);

        itm::PMain  *parent;
        QComboBox*   resolution_cbox;   // combobox "Resolution"
        QComboBox*   scan_method_cbox;  // combobox "Scan pattern"
        QSpinBox*    overlap_sbox;      // spinbox "Blocks overlap"
        QSpinBox*    perblock_time_sbox;// spinbox "Per-block time"
        QLineEdit*   VOI_field;         // field "Volume of interest"
        QLineEdit*   blocks_size_field; // field "Block size"
        QLineEdit*   coverage_field;    // field "Volume coverage"
        QLineEdit*   blocks_field;      // field "Number of blocks"
        QLineEdit*   est_time_field;    // field "Estimated time"
        QPushButton* show_blocks;       // show blocks button
        QPushButton* start_button;      // start button
        QPushButton* cancel_button;     // cancel button
        QTextEdit*   blocks_text;       // list of blocks

        PDialogProofreading();          // default constructor unavailable

    public:

        /**********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        ***********************************************************************************/
        static PDialogProofreading* instance()
        {
            if (uniqueInstance == 0)
                uniqueInstance = new PDialogProofreading(itm::PMain::getInstance());
            return uniqueInstance;
        }
        static void uninstance();
        ~PDialogProofreading(){}

        // overrides closeEvent method of QWidget
        void closeEvent(QCloseEvent *evt);

        static std::vector<itm::block_t> blocks;   // permanently stores the array of blocks
        static int blocks_res;                     // permanently stores blocks resolution

    signals:

    public slots:

        /**********************************************************************************
        * Called when <start_button> emits <click()> signal.
        ***********************************************************************************/
        void startButtonClicked();

        /**********************************************************************************
        * Called when <show_blocks_button> emits <click()> signal.
        ***********************************************************************************/
        void showBlocksButtonClicked();

        /**********************************************************************************
        * Re-computes blocks and updates GUI
        ***********************************************************************************/
        void updateBlocks(int);

        /**********************************************************************************
        * For mouse-enter tooltips
        ***********************************************************************************/
        bool eventFilter(QObject *obj, QEvent *evt);
    
};

#endif // PDIALOGPROOFREADING_H
