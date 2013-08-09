#ifndef PABOUT_H
#define PABOUT_H

#include <QDialog>
#include "../control/CPlugin.h"

class teramanager::PAbout : public QDialog
{
        Q_OBJECT

    private:

        //object members
        QLabel *desc;
        QLabel *logo;
        QTextEdit *changelog;
        QPushButton* closeButton;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PAbout* uniqueInstance;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PAbout* instance(QWidget *parent)
        {
            if (uniqueInstance == 0)
                uniqueInstance = new PAbout(parent);
            return uniqueInstance;
        }
        static PAbout* getInstance(){return uniqueInstance;}
        static void uninstance()
        {
            if(uniqueInstance)
            {
                delete uniqueInstance;
                uniqueInstance = 0;
            }
        }
        PAbout(QWidget *parent);

    signals:

    public slots:
    
};

#endif // PABOUT_H
