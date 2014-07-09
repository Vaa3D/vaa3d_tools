#include "QPixmapToolTip.h"

using namespace teramanager;

QPixmapToolTip* QPixmapToolTip::uniqueInstance = 0;

QPixmapToolTip::QPixmapToolTip(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    rawData = 0;

    imageLabel = new QLabel();
    imageLabel->setScaledContents(true);
    imageLabel->setStyleSheet("border: 2px solid rgb(0,0,0);");
    //imageLabel->setAttribute(Qt::WA_TranslucentBackground);

    textLabel = new QLabel();
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(6);
    textLabel->setFont(tinyFont);
    QPalette pal = textLabel->palette();
    pal.setColor(QPalette::WindowText, QColor(Qt::white));
    textLabel->setPalette(pal);
    textLabel->setAlignment(Qt::AlignTop);
    textLabel->setContentsMargins(10,5,10,5);

    QStackedLayout* layout = new QStackedLayout();
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(imageLabel);
    layout->addWidget(textLabel);
    this->setLayout(layout);

    // remove frame
    //setContentsMargins(2,2,2,2);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}
