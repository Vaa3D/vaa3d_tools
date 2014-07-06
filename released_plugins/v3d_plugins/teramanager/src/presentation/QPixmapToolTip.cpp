#include "QPixmapToolTip.h"

using namespace teramanager;

QPixmapToolTip* QPixmapToolTip::uniqueInstance = 0;

QPixmapToolTip::QPixmapToolTip(QWidget *parent) : QWidget(parent)
{
    /**/itm::debug(itm::LEV1, 0, __itm__current__function__);

    imageLabel = new QLabel();
    imageLabel->setScaledContents(true);

    textLabel = new QLabel();
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(7);
    textLabel->setFont(tinyFont);
    QPalette pal = textLabel->palette();
    pal.setColor(QPalette::WindowText, QColor(Qt::white));
    textLabel->setPalette(pal);
    textLabel->setAlignment(Qt::AlignBottom);
    textLabel->setTextFormat(Qt::RichText);
    textLabel->setContentsMargins(10,10,10,5);

    QStackedLayout* layout = new QStackedLayout();
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(imageLabel);
    layout->addWidget(textLabel);
    this->setLayout(layout);

    // remove frame
    setContentsMargins(2,2,2,2);
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}
