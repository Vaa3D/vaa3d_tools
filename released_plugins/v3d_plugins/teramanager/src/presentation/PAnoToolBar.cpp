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

    button1 = new QToolButton();
    button1->setIcon(QIcon(":/icons/open_volume.png"));
    button1->setCheckable(true);
    toolBar->insertWidget(0, button1);

    button2 = new QToolButton();
    button2->setIcon(QIcon(":/icons/close.png"));
    button2->setCheckable(true);
    toolBar->insertWidget(0, button2);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(toolBar);
    layout->setContentsMargins(0,0,0,0);
    toolBar->setStyleSheet("QToolBar{background:qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                           "stop: 0 rgb(150,179,214), stop: 1 rgb(199,218,241));"
                           "border-style: outset;"
                           "border-width: 2px;"
                           "border-radius: 5px;"
                           "border-color: rgb(56,93,138);}");
    setLayout(layout);

//    setStyleSheet("background:transparent;");
    setAttribute(Qt::WA_TranslucentBackground);

    this->setContentsMargins(0,0,0,0);

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}
