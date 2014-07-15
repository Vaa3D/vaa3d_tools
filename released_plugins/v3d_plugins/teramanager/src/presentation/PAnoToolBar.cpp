#include "PAnoToolBar.h"
#include "PMain.h"
#include "../control/CExplorerWindow.h"

using namespace teramanager;

PAnoToolBar* PAnoToolBar::uniqueInstance = 0;

PAnoToolBar::PAnoToolBar(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    // create toolbar
    toolBar = new QToolBar("ToolBar", this);
    toolBar->setOrientation(Qt::Vertical);
    toolBar->setIconSize(QSize(40,40));

    // add already existing buttons from the main GUI
    toolBar->addAction(PMain::getInstance()->loadAnnotationsAction);
    toolBar->addAction(PMain::getInstance()->saveAnnotationsAction);
    toolBar->addAction(PMain::getInstance()->saveAnnotationsAsAction);
    toolBar->addAction(PMain::getInstance()->clearAnnotationsAction);
    toolBar->addSeparator();


    /**/
    buttonUndo = new QToolButton();
    buttonUndo->setIcon(QIcon(":/icons/undo.png"));
    buttonUndo->setToolTip("Undo (Ctrl+Z)");
    buttonUndo->setEnabled(false);
    buttonUndo->setShortcut(QKeySequence("Ctrl+Z"));
    connect(buttonUndo, SIGNAL(clicked()), this, SLOT(buttonUndoClicked()));
    toolBar->insertWidget(0, buttonUndo);
    buttonRedo = new QToolButton();
    buttonRedo->setIcon(QIcon(":/icons/redo.png"));
    buttonRedo->setToolTip("Redo (Ctrl+Shift+Z)");
    buttonRedo->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    buttonRedo->setEnabled(false);
    connect(buttonRedo, SIGNAL(clicked()), this, SLOT(buttonRedoClicked()));
    toolBar->insertWidget(0, buttonRedo);
    toolBar->addSeparator();

    // add new buttons
    buttonMarkerCreate = new QToolButton();
    buttonMarkerCreate->setIcon(QIcon(":/icons/marker_add.png"));
    buttonMarkerCreate->setCheckable(true);
    buttonMarkerCreate->setToolTip("1-right-click to define a marker");
    connect(buttonMarkerCreate, SIGNAL(toggled(bool)), this, SLOT(buttonMarkerCreateChecked(bool)));
    toolBar->insertWidget(0, buttonMarkerCreate);
    /**/
    buttonMarkerDelete = new QToolButton();
    buttonMarkerDelete->setIcon(QIcon(":/icons/marker_delete.png"));
    buttonMarkerDelete->setCheckable(true);
    buttonMarkerDelete->setToolTip("1-right-click to delete a marker");
    connect(buttonMarkerDelete, SIGNAL(toggled(bool)), this, SLOT(buttonMarkerDeleteChecked(bool)));
    toolBar->insertWidget(0, buttonMarkerDelete);
    /**/
    buttonMarkerRoiDelete = new QToolButton();
    buttonMarkerRoiDelete->setIcon(QIcon(":/icons/marker_delete_roi.png"));
    buttonMarkerRoiDelete->setCheckable(true);
    buttonMarkerRoiDelete->setToolTip("1-right-stroke to delete a group of markers");
    connect(buttonMarkerRoiDelete, SIGNAL(toggled(bool)), this, SLOT(buttonMarkerRoiDeleteChecked(bool)));
    toolBar->insertWidget(0, buttonMarkerRoiDelete);
    /**/
    buttonMarkerRoiView = new QToolButton();
    buttonMarkerRoiView->setIcon(QIcon(":/icons/marker_roi_view.png"));
    buttonMarkerRoiView->setCheckable(true);
    buttonMarkerRoiView->setToolTip("Show/hide markers around the displayed ROI");
    connect(buttonMarkerRoiView, SIGNAL(toggled(bool)), this, SLOT(buttonMarkerRoiViewChecked(bool)));
    toolBar->insertWidget(0, buttonMarkerRoiView);

    /**/
    buttonOptions = new QToolButton();
    buttonOptions->setIcon(QIcon(":/icons/options.png"));
    buttonOptions->setMenu(PMain::getInstance()->threeDMenu);
    buttonOptions->setPopupMode(QToolButton::InstantPopup);
    buttonOptions->setToolTip("Options");
    toolBar->addSeparator();
    toolBar->insertWidget(0, buttonOptions);

    // layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(toolBar);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);


    // make it appear as a true toolbar
    this->setContentsMargins(0,0,0,0);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    toolBar->setStyleSheet("QToolBar{background:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                           "stop: 0 rgb(180,180,180), stop: 1 rgb(220,220,220));"
                           "border-style: outset;"
                           "border-width: 1px;"
                           "border-radius: 5px;"
                           "border-color: rgb(50,50,50);} ");
                           //"QToolBar::separator::horizontal{width: 5px; height: 10px;}");

    // install event filter
    QCoreApplication::instance()->installEventFilter(this);

    // show tooltips also when has no focus on it
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
}


void PAnoToolBar::buttonMarkerCreateChecked(bool checked)
{
     /**/itm::debug(itm::LEV3, strprintf("checked = %s", checked ? "true" : "false").c_str(), __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();

    if(checked)
    {
        // uncheck other buttons but the current one
        if(buttonMarkerDelete->isChecked())
            buttonMarkerDelete->setChecked(false);
        if(buttonMarkerRoiDelete->isChecked())
            buttonMarkerRoiDelete->setChecked(false);


        // change cursor
        QPixmap cur_img(":/icons/cursor_marker_add.png");
        cur_img = cur_img.scaled(32,32,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setCursor(QCursor(cur_img, 0, 0));
        CExplorerWindow::setCursor(QCursor(cur_img, 0, 0), true);

        // switch to Vaa3D's 1-right-click marker create mode
        if(expl)
        {
            expl->view3DWidget->getRenderer()->selectMode = Renderer::smMarkerCreate1;
            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_addthismarker = true;
        }
    }
    else
    {
        // set default cursor
        setCursor(Qt::ArrowCursor);
        CExplorerWindow::setCursor(Qt::ArrowCursor, true);

        // end marker create mode
        if(expl)
            expl->view3DWidget->getRenderer()->endSelectMode();
    }
}


void PAnoToolBar::buttonMarkerDeleteChecked(bool checked)
{
     /**/itm::debug(itm::LEV3, strprintf("checked = %s", checked ? "true" : "false").c_str(), __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();

    if(checked)
    {
        // uncheck other buttons but the current one
        if(buttonMarkerCreate->isChecked())
            buttonMarkerCreate->setChecked(false);
        if(buttonMarkerRoiDelete->isChecked())
            buttonMarkerRoiDelete->setChecked(false);

        if(expl)
        {
            // change cursor
            QPixmap cur_img(":/icons/cursor_marker_delete.png");
            cur_img = cur_img.scaled(32,32,Qt::KeepAspectRatio, Qt::SmoothTransformation);
            setCursor(QCursor(cur_img, 0,0));
            CExplorerWindow::setCursor(QCursor(cur_img, 0, 0), true);

//            if(expl)
//            {
//                expl->view3DWidget->getRenderer()->selectMode = Renderer::smMarkerCreate1;
//                static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_addthismarker = false;
//                static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_imaging = false;
//                static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_grabhighrez = true;
//            }
        }
    }
    else
    {
        // set default cursor
        setCursor(Qt::ArrowCursor);
        CExplorerWindow::setCursor(Qt::ArrowCursor, true);

        // end marker delete mode
//        if(expl)
//        {
//            expl->view3DWidget->getRenderer()->endSelectMode();
//            expl->view3DWidget->getRenderer()->selectMode = Renderer::smObject;
//            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_grabhighrez = false;
//        }
    }
}


void PAnoToolBar::buttonMarkerRoiDeleteChecked(bool checked)
{
     /**/itm::debug(itm::LEV3, strprintf("checked = %s", checked ? "true" : "false").c_str(), __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();

    if(checked)
    {
//        QMessageBox::information(this, "Warning", "Not yet implemented. But stay tuned!");
//        buttonMarkerRoiDelete->setChecked(false);
        // uncheck other buttons but the current one
        if(buttonMarkerCreate->isChecked())
            buttonMarkerCreate->setChecked(false);
        if(buttonMarkerDelete->isChecked())
            buttonMarkerDelete->setChecked(false);

        // change cursor
        QPixmap cur_img(":/icons/cursor_marker_delete_roi.png");
        cur_img = cur_img.scaled(32,32,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setCursor(QCursor(cur_img, 0,0));
        CExplorerWindow::setCursor(QCursor(cur_img, 0, 0), true);

        // switch to Vaa3D's mode "Zoom-in HighRezImage: 1-right stroke ROI"
        if(expl)
        {
            expl->view3DWidget->getRenderer()->selectMode = Renderer::smCurveCreate1;
            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_addthiscurve = false;
            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_imaging = false;
            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_grabhighrez = true;
        }
    }
    else
    {
        // set default cursor
        setCursor(Qt::ArrowCursor);
        CExplorerWindow::setCursor(Qt::ArrowCursor, true);

        // end Vaa3D's mode "Zoom-in HighRezImage: 1-right stroke ROI"
        if(expl)
        {
            expl->view3DWidget->getRenderer()->selectMode = Renderer::smObject;
            static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->b_grabhighrez = false;
        }
    }
}


void PAnoToolBar::buttonMarkerRoiViewChecked(bool checked)
{
    /**/itm::debug(itm::LEV3, strprintf("checked = %s", checked ? "true" : "false").c_str(), __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl)
    {
        // get TRUE markers' handle (others won't work, e.g. LandmarkList)
        QList <ImageMarker>& markers = static_cast<Renderer_gl1*>(expl->view3DWidget->getRenderer())->listMarker;

        // get virtual margin and apply it to each directions separately
        int vmPerc = CSettings::instance()->getAnnotationVirtualMargin();
        int vmX = (expl->volH1-expl->volH0)*(vmPerc/100.0f)/2;
        int vmY = (expl->volV1-expl->volV0)*(vmPerc/100.0f)/2;
        int vmZ = (expl->volD1-expl->volD0)*(vmPerc/100.0f)/2;

        // apply changes to existing markers
        for(int i=0; i<markers.size(); i++)
        {
            // select markers within margin
            if(  (markers[i].x < 0 && markers[i].x >= -vmX) ||
                 (markers[i].y < 0 && markers[i].y >= -vmY) ||
                 (markers[i].z < 0 && markers[i].z >= -vmZ) ||
                 (markers[i].x  >= (expl->volH1-expl->volH0) && markers[i].x <=  (expl->volH1-expl->volH0+vmX)) ||
                 (markers[i].y  >= (expl->volV1-expl->volV0) && markers[i].y <=  (expl->volV1-expl->volV0+vmY)) ||
                 (markers[i].z  >= (expl->volD1-expl->volD0) && markers[i].x <=  (expl->volD1-expl->volD0+vmZ)))
            {
                // activate / deactivate markers
                markers[i].on = checked;

                // markers within margin are assigned gray color so as to be better distinguishable
                markers[i].color.r = markers[i].color.g = markers[i].color.b = 220;
            }

            // always turn off markers outside the visible area
            if(  (markers[i].x < -vmX) || (markers[i].y < -vmY)   || (markers[i].z < -vmZ) ||
                 (markers[i].x  >= (expl->volH1-expl->volH0+vmX)) || (markers[i].y  >= (expl->volV1-expl->volV0+vmY)) || (markers[i].z  >= (expl->volD1-expl->volD0+vmZ)))
                markers[i].on = false;
        }

        expl->view3DWidget->updateTool();
        expl->view3DWidget->update();
    }
}


void PAnoToolBar::buttonUndoClicked()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->undoStack.canUndo())
    {
        expl->undoStack.undo();
        if(!expl->undoStack.canUndo())
            buttonUndo->setEnabled(false);
        buttonRedo->setEnabled(true);
    }
}

void PAnoToolBar::buttonRedoClicked()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    CExplorerWindow* expl = CExplorerWindow::getCurrent();
    if(expl && expl->undoStack.canRedo())
    {
        expl->undoStack.redo();
        if(!expl->undoStack.canRedo())
            buttonRedo->setEnabled(false);
        buttonUndo->setEnabled(true);
    }
}

/**********************************************************************************
* Intercepts global key pressed events
***********************************************************************************/
bool PAnoToolBar::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == 6)
    {
        QKeyEvent *KeyEvent = (QKeyEvent*)event;
        if(KeyEvent->key() == Qt::Key_Escape)
            releaseTools();
    }
    return false;
}

/**********************************************************************************
* Release currently activated tools, if any
***********************************************************************************/
void PAnoToolBar::releaseTools()
{
    /**/itm::debug(itm::LEV3, 0, __itm__current__function__);

    if(buttonMarkerCreate->isChecked())
        buttonMarkerCreate->setChecked(false);
    if(buttonMarkerDelete->isChecked())
        buttonMarkerDelete->setChecked(false);
    if(buttonMarkerRoiDelete->isChecked())
        buttonMarkerRoiDelete->setChecked(false);
}

/**********************************************************************************
* Refresh currently activated tools, if any
***********************************************************************************/
void PAnoToolBar::refreshTools()
{
    /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

    if(buttonMarkerCreate->isChecked())
        buttonMarkerCreateChecked(true);
    if(buttonMarkerDelete->isChecked())
        buttonMarkerDeleteChecked(true);
    if(buttonMarkerRoiDelete->isChecked())
        buttonMarkerRoiDeleteChecked(true);
    update();
}

/**********************************************************************************
* Align the current widget to the left of the given widget
***********************************************************************************/
void PAnoToolBar::alignToLeft(QWidget* widget)
{
    /**/itm::debug(itm::LEV_MAX, 0, __itm__current__function__);

    int new_x = widget->mapToGlobal(QPoint(0,0)).x();
    int new_y = widget->mapToGlobal(QPoint(0,0)).y();
    move(new_x, new_y);
    update();
}


