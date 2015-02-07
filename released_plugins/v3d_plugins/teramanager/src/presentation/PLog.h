#ifndef PLOG_H
#define PLOG_H

#include <QDialog>
#include "CPlugin.h"
#include "COperation.h"

class teramanager::PLog : public QDialog
{
        Q_OBJECT

    private:

        //GUI widgets
        QGroupBox* timeComponentsPanel;
        QTextEdit *timeComponents;
        float timeIO, timeGPU, timeCPU, timeSum, timeActual;
        /* ----------- */
        QGroupBox* timeOperationsPanel;
        QTextEdit *timeOperations;
        QCheckBox *enableIoCoreOperationsCheckBox;
        bool enableIoCoreOperations;
        /* ----------- */
        QGroupBox* logPanel;
        QTextEdit *log;
        QCheckBox *appendCheckBox;
        QComboBox *appendOpComboBox;
        QComboBox *appendCompComboBox;
        QLineEdit *appendToFileLineEdit;
        QSpinBox  *appendEverySecondsSpinBox;
        /* ----------- */
        QCheckBox *autoUpdateCheckBox;
        QPushButton* updatePushButton;
        QPushButton* closeButton;
        QPushButton* resetButton;

        // other object members
        std::map< std::string, std::vector<itm::Operation*> > loggedOperations; // grouped by name

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        **********************************************************************************/
        static PLog* uniqueInstance;

        //extracts from the given time field in the corresponding time measure
        float toFloat(QString timeField);

        //update
        void update();

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        **********************************************************************************/
        static PLog* instance(QWidget* parent=0)
        {
            /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

            if (uniqueInstance == 0)
                uniqueInstance = new PLog(parent);
            return uniqueInstance;
        }
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        PLog(QWidget *parent);
        bool isIoCoreOperationsEnabled(){return enableIoCoreOperations;}

        /**********************************************************************************
        * append new operation to log
        ***********************************************************************************/
        void appendOperation(itm::Operation* op, bool update_time_comps = true);
        void appendOperationToFile(itm::Operation* op);
        void append(std::string text);

        /**********************************************************************************
        * Called by algorithms running from different threads.
        * Emits <sendAppend> signal
        ***********************************************************************************/
        void emitSendAppend(void* op);

    signals:

        /*********************************************************************************
        * Carries op informations
        **********************************************************************************/
        void sendAppend(void* op);

    public slots:

        /**********************************************************************************
        * <sendAppend> event handler
        ***********************************************************************************/
        void appendOperationVoid(void* op);

        /**********************************************************************************
        * <enableIoCoreOperationsCheckBox> event handler
        ***********************************************************************************/
        void enableIoCoreOperationsCheckBoxChanged(int s);

        /**********************************************************************************
        * <autoUpdateCheckBox> event handler
        ***********************************************************************************/
        void autoUpdateCheckBoxChanged(int s);

        /**********************************************************************************
        * <appendCheckBox> event handler
        ***********************************************************************************/
        void appendCheckBoxChanged(int s);

        /**********************************************************************************
        * <updatePushButton> event handler
        ***********************************************************************************/
        void updatePushButtonClicked();

        void reset();

};

#endif // PLOG_H
