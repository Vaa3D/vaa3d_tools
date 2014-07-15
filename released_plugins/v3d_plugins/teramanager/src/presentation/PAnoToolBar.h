#ifndef PANOTOOLBAR_H
#define PANOTOOLBAR_H

#include <QDialog>
#include "../control/CPlugin.h"

class teramanager::PAnoToolBar : public QWidget
{
    Q_OBJECT

    private:

        QToolBar* toolBar;                      //tool bar with buttons
        QToolButton *buttonMarkerCreate;        //button1
        QToolButton *buttonMarkerDelete;        //button2
        QToolButton *buttonMarkerRoiDelete;     //button3
        QToolButton *buttonMarkerRoiView;       //button4

        QToolButton *buttonUndo;                //undo button
        QToolButton *buttonRedo;                //redo button

        QToolButton* buttonOptions;             //options button

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

        /**********************************************************************************
        * Intercepts global key pressed events
        ***********************************************************************************/
        bool eventFilter(QObject *object, QEvent *event);

        /**********************************************************************************
        * Release currently activated tools, if any
        ***********************************************************************************/
        void releaseTools();

        /**********************************************************************************
        * Refresh currently activated tools, if any
        ***********************************************************************************/
        void refreshTools();

        /**********************************************************************************
        * Align the current widget to the left of the given widget
        ***********************************************************************************/
        void alignToLeft(QWidget* widget);

        friend class PMain;
        friend class CExplorerWindow;
        friend class QUndoMarkerCreate;
        friend class QUndoMarkerDelete;
        friend class QUndoMarkerDeleteROI;

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

        void buttonMarkerCreateChecked(bool checked);
        void buttonMarkerDeleteChecked(bool checked);
        void buttonMarkerRoiDeleteChecked(bool checked);
        void buttonMarkerRoiViewChecked(bool checked);
        void buttonUndoClicked();
        void buttonRedoClicked();
    
};

#endif // PANOTOOLBAR_H
