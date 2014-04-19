#ifndef PANOTOOLBAR_H
#define PANOTOOLBAR_H

#include <QDialog>
#include "../control/CPlugin.h"

class teramanager::PAnoToolBar : public QWidget
{
    Q_OBJECT

    private:

        QToolBar* toolBar;                      //tool bar with buttons
//        QButtonGroup *buttonGroup;              //exclusive button group
        QToolButton *buttonMarkerCreate;        //button1
        QToolButton *buttonMarkerDelete;        //button2
        QToolButton *buttonMarkerRoiDelete;     //button3
        QToolButton *buttonMarkerRoiView;       //button4
//        QToolButton *buttonInvisible;           //invisible button
        QPoint dragPosition;

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "istance(...)"
        **********************************************************************************/
        static PAnoToolBar* uniqueInstance;

    public:

        /*********************************************************************************
        * Singleton design pattern: this class can have one instance only,  which must be
        * instantiated by calling static method "instance(...)"
        **********************************************************************************/
        static PAnoToolBar* instance(QWidget *parent=0)
        {
            /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

            if (uniqueInstance == 0)
                uniqueInstance = new PAnoToolBar(parent);
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
        PAnoToolBar(QWidget *parent = 0);

    protected:

        void mousePressEvent(QMouseEvent *event)
        {
            if (event->button() == Qt::LeftButton) {
                dragPosition = event->globalPos() - frameGeometry().topLeft();
                event->accept();
            }
        }

        void mouseMoveEvent(QMouseEvent *event)
        {
            if (event->buttons() & Qt::LeftButton) {
                move(event->globalPos() - dragPosition);
                event->accept();
            }
        }
    
    signals:
    
    public slots:

        void buttonChecked(bool checked);
    
};

#endif // PANOTOOLBAR_H
