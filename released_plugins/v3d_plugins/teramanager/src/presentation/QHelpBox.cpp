#include "QHelpBox.h"

using namespace teramanager;

QHelpBox::QHelpBox(QWidget *_parent) : QWidget(_parent)
{
    parent = _parent;
    QFont tinyFont = QApplication::font();
    #ifndef _USE_NATIVE_FONTS
    tinyFont.setPointSize(9);
    #endif

    backgroundPanel = new QLabel();
    backgroundPanel->setStyleSheet("border: 1px solid; border-color: rgb(71,127,249); background-color: rgb(245,245,245); margin-top:0px; "
                                               "margin-bottom:0px; margin-left: 0px; padding-right: 4px; padding-top:4px; padding-bottom:4px; padding-left:60px;"
                           "");

    helpBox = new QLabel();
    //helpBox->setFont(tinyFont);
    helpBox->setStyleSheet("margin-top:0px; margin-bottom:0px; margin-left: 0px; padding-right: 4px; padding-top:4px; padding-bottom:4px; padding-left:60px; text-align:justify;");
    helpBox->setWordWrap(true);    
    helpBox->setTextFormat(Qt::RichText);
    helpBox->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    helpBox->setOpenExternalLinks(true);
    QPixmap pixmap(":/icons/help2.png");
    helpIcon = new QLabel();
    helpIcon->setPixmap(pixmap.scaled(50,50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    helpIcon->setStyleSheet("margin-left: 5px");

    QStackedLayout *helpLayout = new QStackedLayout();
    helpLayout->addWidget(backgroundPanel);
    helpLayout->addWidget(helpIcon);
    helpLayout->addWidget(helpBox);
    helpLayout->setStackingMode(QStackedLayout::StackAll);
    setLayout(helpLayout);

    this->setFixedHeight(120);
}

void QHelpBox::setText(string text)
{
    helpBox->setText(QString("<html><p style=\"text-align:justify;\">").append(text.c_str()).append("</p></html>"));
}
