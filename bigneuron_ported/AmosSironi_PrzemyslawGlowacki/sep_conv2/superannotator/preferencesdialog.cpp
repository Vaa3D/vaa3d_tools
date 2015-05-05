#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    connect( ui->butBrowseFijiPath, SIGNAL(clicked()), this, SLOT(browseFijiPathClicked()) );

    connect(  ui->spinMaxVox, SIGNAL(valueChanged(int)), this, SLOT( spinMaxVoxValueChanged(int) ) );
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

QString PreferencesDialog::getFijiExePath()
{
    return ui->lineEdit->text();
}

void PreferencesDialog::setFijiExePath( const QString &str )
{
    ui->lineEdit->setText( str );
}

unsigned PreferencesDialog::getMaxVoxelsForSV() const
{
    return ((unsigned int)ui->spinMaxVox->value()) * 1000000U;
}

void PreferencesDialog::setMaxVoxelsForSV( unsigned val )
{
    ui->spinMaxVox->setValue( val / 1000000U );
}

unsigned PreferencesDialog::getSliceJump() const
{
    return ((unsigned int)ui->spinSliceJump->value());
}

void PreferencesDialog::setSliceJump( unsigned val )
{
    ui->spinMaxVox->setValue( val );
}

void PreferencesDialog::on_spinSliceJump_valueChanged(int val)
{
    setSliceJump((unsigned)val);
}

void PreferencesDialog::browseFijiPathClicked()
{
    QString fName = QFileDialog::getOpenFileName( this, "Browse for Fiji executable" );

    if ( fName.isEmpty() )
        return;

    ui->lineEdit->setText(fName);
}

void PreferencesDialog::spinMaxVoxValueChanged(int newVal)
{
    const unsigned Mult = 16;    // empirically found :(

    const unsigned maxRAM = ((unsigned) newVal) * Mult;
    ui->labelMaxVox->setText( QString("(approx. max %1 MB of RAM needed)").arg( maxRAM ) );
}
