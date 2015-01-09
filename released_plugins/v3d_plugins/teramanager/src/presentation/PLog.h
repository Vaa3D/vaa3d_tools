#ifndef PLOG_H
#define PLOG_H

#include <QDialog>
#include "CPlugin.h"
#include "COperation.h"
#include "PMain.h"

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
        /* ----------- */
        QGroupBox* logPanel;
        QTextEdit *log;
        /* ----------- */
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
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PLog* instance(QWidget *parent)
        {
            /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

            if (uniqueInstance == 0)
                uniqueInstance = new PLog(parent);
            return uniqueInstance;
        }
        static PLog* getInstance(){/**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__); return instance(PMain::getInstance());}
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        PLog(QWidget *parent);

        void append(std::string text);
        void appendOperation(itm::Operation* op);

    signals:

    public slots:

        void reset();

};

#endif // PLOG_H
