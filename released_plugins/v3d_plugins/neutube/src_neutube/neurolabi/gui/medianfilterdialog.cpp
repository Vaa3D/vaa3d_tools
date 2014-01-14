#include "medianfilterdialog.h"
#include "ui_medianfilterdialog.h"

MedianFilterDialog::MedianFilterDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MedianFilterDialog)
{
    m_ui->setupUi(this);
}

MedianFilterDialog::~MedianFilterDialog()
{
    delete m_ui;
}

void MedianFilterDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

int MedianFilterDialog::radius()
{
  return m_ui->spinBox->value();
}
