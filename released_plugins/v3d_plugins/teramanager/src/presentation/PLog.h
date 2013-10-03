#ifndef PLOG_H
#define PLOG_H

#include <QDialog>
#include "../control/CPlugin.h"
#include "PMain.h"

class teramanager::PLog : public QDialog
{
        Q_OBJECT

    private:

        //object members
        QTextEdit *log;
        QPushButton* closeButton;
        QPushButton* resetButton;
        QLineEdit *timeIO, *timeGPU, *timeCPU, *timeSum, *timeActual, *perfGain;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PLog* uniqueInstance;

        //extracts from the given time field in the corresponding time measure
        float toFloat(QString timeField);

        //updates time percentages
        void updatePercentages();

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PLog* instance(QWidget *parent)
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

            if (uniqueInstance == 0)
                uniqueInstance = new PLog(parent);
            return uniqueInstance;
        }
        static PLog* getInstance(){return instance(PMain::getInstance());}
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        PLog(QWidget *parent);

        void append(string text);
        void appendIO(int milliseconds, string message);
        void appendGPU(int milliseconds, string message);
        void appendCPU(int milliseconds, string message);
        void appendActual(int milliseconds, string message);

    signals:

    public slots:

        void reset();

};

#endif // PLOG_H
