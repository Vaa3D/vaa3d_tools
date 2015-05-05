#include "textinfodialog.h"
#include "ui_textinfodialog.h"

#include <QTextDocument>

TextInfoDialog::TextInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextInfoDialog)
{
    ui->setupUi(this);
}

void TextInfoDialog::setText(const QString &s)
{
    ui->textBrowser->setHtml( s );
}

TextInfoDialog::~TextInfoDialog()
{
    delete ui;
}
