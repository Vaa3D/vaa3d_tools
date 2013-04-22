#include "QHelpBox.h"

using namespace teramanager;

QHelpBox::QHelpBox(QWidget *_parent) : QWidget(_parent)
{
    parent = _parent;
    QFont tinyFont = QApplication::font();
    tinyFont.setPointSize(9);
    helpBox = new QLabel();
    helpBox->setFont(tinyFont);
    helpBox->setStyleSheet("border: 1px solid; border-color: rgb(71,127,249); background-color: rgb(245,245,245); margin-top:0px; "
                                               "margin-bottom:0px; margin-left: 27px; padding-right: 4px; padding-top:4px; padding-bottom:4px; padding-left:23px; text-align:justify;"
                           "");
    helpBox->setWordWrap(true);
    helpIcon = new QLabel("<html><table><tr style=\"vertical-align: middle;\"><td><img src=\":/icons/help.png\"></td></tr></html>");
    helpIcon->setStyleSheet("border: none; background-color: none;");

    QStackedLayout *helpLayout = new QStackedLayout();
    helpLayout->addWidget(helpBox);
    helpLayout->addWidget(helpIcon);
    helpLayout->setStackingMode(QStackedLayout::StackAll);
    setLayout(helpLayout);

    this->setFixedHeight(120);
}

void QHelpBox::setText(string text)
{
    helpBox->setText(QString("<html><p style=\"text-align:justify;\">").append(text.c_str()).append("</p></html>"));

}
