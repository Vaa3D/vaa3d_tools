#include "PAnoToolBar.h"

using namespace teramanager;

PAnoToolBar* PAnoToolBar::uniqueInstance = 0;

PAnoToolBar::PAnoToolBar(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    //toolbar
    toolBar = new QToolBar("ToolBar", this);
    toolBar->setOrientation(Qt::Vertical);
    toolBar->setIconSize(QSize(60,60));

//    buttonGroup = new QButtonGroup();
//    buttonGroup->setExclusive(true);

    buttonMarkerCreate = new QToolButton();
    buttonMarkerCreate->setIcon(QIcon(":/icons/marker_add.png"));
    buttonMarkerCreate->setCheckable(true);
    buttonMarkerCreate->setToolTip("1-right-click to define a marker");
    connect(buttonMarkerCreate, SIGNAL(toggled(bool)), this, SLOT(buttonChecked(bool)));
//    buttonGroup->addButton(buttonMarkerCreate);
    toolBar->insertWidget(0, buttonMarkerCreate);

    buttonMarkerDelete = new QToolButton();
    buttonMarkerDelete->setIcon(QIcon(":/icons/marker_delete.png"));
    buttonMarkerDelete->setCheckable(true);
    buttonMarkerDelete->setToolTip("1-left-click to delete a marker");
    connect(buttonMarkerDelete, SIGNAL(toggled(bool)), this, SLOT(buttonChecked(bool)));
//    buttonGroup->addButton(buttonMarkerDelete);
    toolBar->insertWidget(0, buttonMarkerDelete);

    buttonMarkerRoiDelete = new QToolButton();
    buttonMarkerRoiDelete->setIcon(QIcon(":/icons/marker_delete_roi.png"));
    buttonMarkerRoiDelete->setCheckable(true);
    buttonMarkerRoiDelete->setToolTip("1-right-stroke to delete a group of markers");
    connect(buttonMarkerRoiDelete, SIGNAL(toggled(bool)), this, SLOT(buttonChecked(bool)));
//    buttonGroup->addButton(buttonMarkerRoiDelete);
    toolBar->insertWidget(0, buttonMarkerRoiDelete);

    buttonMarkerRoiView = new QToolButton();
    buttonMarkerRoiView->setIcon(QIcon(":/icons/marker_roi_view.png"));
    buttonMarkerRoiView->setCheckable(true);
    buttonMarkerRoiView->setToolTip("Show/hide markers around the displayed ROI");
    connect(buttonMarkerRoiView, SIGNAL(toggled(bool)), this, SLOT(buttonChecked(bool)));
//    buttonGroup->addButton(buttonMarkerRoiView);
    toolBar->insertWidget(0, buttonMarkerRoiView);

//    buttonInvisible = new QToolButton();
//    buttonInvisible->setCheckable(true);
//    buttonGroup->addButton(buttonInvisible);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(toolBar);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);


    // make it appear as a true toolbar
    this->setContentsMargins(0,0,0,0);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    toolBar->setStyleSheet("QToolBar{background:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
//                           "stop: 0 rgb(150,179,214), stop: 1 rgb(199,218,241));"
                           "stop: 0 rgb(180,180,180), stop: 1 rgb(220,220,220));"
                           "border-style: outset;"
                           "border-width: 1px;"
                           "border-radius: 5px;"
//                           "border-color: rgb(56,93,138);}");
                            "border-color: rgb(50,50,50);}");
}

void PAnoToolBar::buttonChecked(bool checked)
{
    /**/itm::debug(itm::LEV3, strprintf("checked = %s", checked ? "true" : "false").c_str(), __itm__current__function__);

    QToolButton* sender = dynamic_cast<QToolButton*>(QObject::sender());
    if(sender && checked)
    {
        // uncheck other buttons but the current one
        if(sender != buttonMarkerCreate && buttonMarkerCreate->isChecked())
            buttonMarkerCreate->setChecked(false);
        if(sender != buttonMarkerDelete && buttonMarkerDelete->isChecked())
            buttonMarkerDelete->setChecked(false);
        if(sender != buttonMarkerRoiDelete && buttonMarkerRoiDelete->isChecked())
            buttonMarkerRoiDelete->setChecked(false);
        if(sender != buttonMarkerRoiView && buttonMarkerRoiView->isChecked())
            buttonMarkerRoiView->setChecked(false);
    }
}
