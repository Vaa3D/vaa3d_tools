#include "QPixmapToolTip.h"
#include "../control/CImageUtils.h"

using namespace teramanager;

QPixmapToolTip* QPixmapToolTip::uniqueInstance = 0;

QPixmapToolTip::QPixmapToolTip(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(6);

    rawData = 0;
    imageLabel = new QLabel();
    imageLabel->setScaledContents(true);
    imageLabel->setStyleSheet("border: 2px solid rgb(0,0,0);");
    textLabel = new QLabel();
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
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}
