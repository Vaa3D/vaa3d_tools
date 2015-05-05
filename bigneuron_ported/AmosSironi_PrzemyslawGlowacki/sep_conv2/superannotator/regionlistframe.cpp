#include "regionlistframe.h"
#include "ui_regionlistframe.h"

#include <QKeyEvent>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QListWidgetItem>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>

#include "annotatorwnd.h"

RegionListFrame::RegionListFrame(QWidget *parent, AnnotatorWnd *annWnd) :
    QFrame(parent),
    ui(new Ui::RegionListFrame),
    mAnnotatorWnd(annWnd)
{
    ui->setupUi(this);

    connect( ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
             this, SLOT(listCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    connect( ui->butSave, SIGNAL(clicked()), this, SLOT(saveAsClicked()) );
    connect( ui->listWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(listClickedSignal(QModelIndex)) );
    connect( ui->butLabelRegion, SIGNAL(clicked()), this, SLOT(butLabelRegionClicked()) );
}

void RegionListFrame::keyReleaseEvent(QKeyEvent *evt)
{
    int key = evt->key();
    if ((key >= '0') && (key < '3'))
    {
        emit labelRegion( ui->listWidget->currentRow() , key - '0' );
    }
}

void RegionListFrame::closeEvent(QCloseEvent *event)
{
      emit widgetClosed();
      event->accept();
}

void RegionListFrame::butLabelRegionClicked()
{
    QMenu menu("Assign to", this);

    QStringList sList;
    mAnnotatorWnd->getLabelClassList( sList );

    foreach( const QString &s, sList )
    {
        menu.addAction( s );
    }

    QAction *res = menu.exec( ui->butLabelRegion->mapToGlobal( QPoint(0,0) ) );
    if (res == 0)
        return;

    unsigned int i=0;
    foreach( const QString &s, sList )
    {
        if ( res->text() == s )
        {
            emit labelRegion( ui->listWidget->currentRow() , i );

            break;
        }
        i++;
    }
}

void RegionListFrame::listClickedSignal(QModelIndex mIdx)
{
    static int lastCurrentItem = -1;

    int newItem = ui->listWidget->currentRow();

    if ((lastCurrentItem == -1) || (newItem != lastCurrentItem)) {
        lastCurrentItem = newItem;
        return;
    }

    Qt::CheckState newState;
    switch( ui->listWidget->item(newItem)->checkState() )
    {
        case Qt::Unchecked:
            newState = Qt::PartiallyChecked;
            break;
        case Qt::PartiallyChecked:
            newState = Qt::Checked;
            break;
        case Qt::Checked:
            newState = Qt::Unchecked;
            break;
    }

    ui->listWidget->item(newItem)->setCheckState( newState );

    lastCurrentItem = newItem;

    //qDebug("Clicked");
}

void RegionListFrame::setRegionData( const std::vector< ShapeStatistics<> > &info )
{
    mRegionDescriptions = info;

    // update
    ui->listWidget->clear();
    for (unsigned int i=0; i < info.size(); i++)
    {
        QString name = QString("Region %1").arg(i+1);
        QListWidgetItem *item = new QListWidgetItem( ui->listWidget );
        item->setText( name );
        item->setFlags( item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsTristate );

        Qt::CheckState state = Qt::Unchecked;
        if (mRegionDescriptions[i].annotationLabel() == 0)
            state = Qt::PartiallyChecked;
        else if(mRegionDescriptions[i].annotationLabel() == 1)
            state = Qt::Checked;

        item->setCheckState( state );
        ui->listWidget->addItem( item );
    }

    ui->listWidget->setCurrentRow(0);
}

/*** Saves a txt file: "<LABEL_ID>\t<NUM_PIXS>\t<ANNOTATION_LABEL>"
  *  where ANNOTATION_LABEL is 0 or 1 according to the checkbox
  *  NUM_PIXS is saved to provide a safe key if those labels are used later on
  */
void RegionListFrame::saveAsClicked()
{
    QString fName = QFileDialog::getSaveFileName( this, "Save region annotation", ".", "*.txt" );
    if (fName.isEmpty())
        return;

    QFile file(fName);
    file.open( QIODevice::WriteOnly );

    QString strFmt = "%1\t%2\t%3\n";

    // add descr
    file.write( ("%%" + strFmt.arg("LABEL_ID").arg("NUM_PIXS").arg("ANNOTATION_LABEL")).toLatin1() );

    for (unsigned int i=0; i < mRegionDescriptions.size(); i++)
    {
        unsigned int checked = ui->listWidget->item( i )->checkState() == Qt::Checked;
        file.write( strFmt.arg( mRegionDescriptions[i].labelIdx() ).arg( mRegionDescriptions[i].numVoxels() ).arg( checked ).toLatin1() );
    }

    file.close();
}

void RegionListFrame::listCurrentItemChanged( QListWidgetItem *, QListWidgetItem * )
{
    if (ui->listWidget->selectedItems().count() == 0)
        return;

    const int curItem = ui->listWidget->currentRow();

    // show descr
    QString s = QString("<b>Region Label = %1</b><br>").arg( mRegionDescriptions.at(curItem).labelIdx() );
    s += mRegionDescriptions.at(curItem).toString();
    ui->textBrowser->setHtml( s );

    emit currentRegionChanged(curItem);
}

void RegionListFrame::moveToBottomLeftCorner()
{
    const QRect scrnRect = QApplication::desktop()->rect();

    QRect wndRect;

    wndRect.setX( scrnRect.width() - width() );
    wndRect.setY( scrnRect.height() - height() );

    wndRect.setWidth( this->width() );
    wndRect.setHeight( this->height() );

    this->setGeometry( wndRect );
}

RegionListFrame::~RegionListFrame()
{
    delete ui;
}
