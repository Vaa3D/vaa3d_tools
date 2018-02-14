#include "QHelpBox.h"

using namespace terastitcher;

QHelpBox::QHelpBox(QWidget *_parent) : QWidget(_parent)
{
    defaultMsg = "";
    parent = _parent;
    helpBox = new QLabel();
    helpBox->setStyleSheet("border: 1px solid; border-color: rgb(71,127,249); background-color: rgb(245,245,245); margin-top:0px; "
                                               "margin-bottom:0px; margin-left: 0px; padding-right: 4px; padding-top:2px; padding-bottom:2px; padding-left:60px; text-align:justify;"
                           "");
    helpBox->setWordWrap(true);

  
    QFont tinyFont = QApplication::font();
	#if defined Q_OS_WIN
    tinyFont.setPointSize(7);
	#endif
    helpBox->setFont(tinyFont);

    QPixmap pixmap(":/icons/help.png");
    helpIcon = new QLabel();
    helpIcon->setPixmap(pixmap.scaled(35,35, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    helpIcon->setStyleSheet("margin-left: 10px");

    QStackedLayout *helpLayout = new QStackedLayout();
    helpLayout->addWidget(helpBox);
    helpLayout->addWidget(helpIcon);
    helpLayout->setStackingMode(QStackedLayout::StackAll);
    setLayout(helpLayout);
}

void QHelpBox::setText(std::string text)
{
    helpBox->setText(QString("<html><p style=\"text-align:justify;\">").append(text.c_str()).append("</p></html>"));
}
