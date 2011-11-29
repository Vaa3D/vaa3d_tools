#include "createdialog.h"
#include "ui_createdialog.h"
#include "../../CT3D/cell_track.h"
#include "../extends/cell_track_ex.h"
#include <QtGui>
#include <QFileDialog>

bool isInt(QString str)
{
    if(str.length() == 0) return false;
    for(int i = 0; i < str.length(); i++)
    {
        if(str.at(i) > QChar('9') || str.at(i) < QChar('0')) return false;
    }
    return true;
}

QString getFileName(QString str)
{
	if(str.contains(QChar('\\'))) return str.right(str.size() - str.lastIndexOf(QChar('\\')) - 1);
	else if(str.contains(QChar('/'))) return str.right(str.size() - str.lastIndexOf(QChar('/')) - 1);
	else return str;
}

CreateCellTrackDialog::CreateCellTrackDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateCellTrackDialog)
{
    ui->setupUi(this);
	ui->progressBar->setHidden(true);
	ui->progressLabel->setHidden(true);
	ui->exportButton->setEnabled(false);
	ui->exportButton->setHidden(true);
	connect(this,SIGNAL(setProgressValue(int)), this, SLOT(onSetProgressValue(int)));
	//m_celltrack = new CellTrack();
	m_celltrack = NULL;
}

CreateCellTrackDialog::~CreateCellTrackDialog()
{
    delete ui;
}
void CreateCellTrackDialog::setCellTrack(CellTrack* celltrack)
{
	m_celltrack = celltrack;
	connect((CellTrackEX*)celltrack, SIGNAL(setProgressValue(int)), this, SLOT(onSetProgressValue(int)));
}

CellTrack* CreateCellTrackDialog::getCellTrack()
{
	return m_celltrack;
}
int CreateCellTrackDialog::getMinThresh()
{
    return atoi((char*) ui->minEditor->text().toStdString().c_str());
}

int CreateCellTrackDialog::getMaxThresh()
{
    return atoi((char*) ui->maxEditor->text().toStdString().c_str());

}

int CreateCellTrackDialog::getSingleThresh()
{
    return atoi((char*) ui->singleEditor->text().toStdString().c_str());

}

QStringList CreateCellTrackDialog::getFileList()
{
    return m_filelist;
}

bool CreateCellTrackDialog::checkValid()
{
   if(ui->fromImagesButton->isChecked())
    {
        QString minStr = ui->minEditor->text();
        QString maxStr = ui->maxEditor->text();
        QString singleStr = ui->singleEditor->text();
        return isInt(minStr) && isInt(maxStr) && isInt(singleStr) && (!m_filelist.empty());
    }
    return !m_filelist.empty();
}

bool CreateCellTrackDialog::isFromTrees()
{
    return ui->fromTreesButton->isChecked();
}

void CreateCellTrackDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CreateCellTrackDialog::on_fromImagesButton_clicked(bool checked)
{
        ui->parasGroupBox->setVisible(checked);
        ui->openFilesButton->setText(tr("Open Images"));
}

void CreateCellTrackDialog::on_fromTreesButton_clicked(bool checked)
{
    ui->parasGroupBox->setHidden(checked);
    ui->openFilesButton->setText(tr("Open Trees"));
}

void CreateCellTrackDialog::setExportButton()
{
	ui->exportButton->setVisible(true);
}
void CreateCellTrackDialog::onSetProgressValue(int value)
{
	ui->progressBar->setValue(value+1);
	if(value < m_filelist.size())
	{
		QString str = m_filelist.at(value);
		str = str.left(str.lastIndexOf("."));
		str = str + tr(".bin.tree");
		str = getFileName(str);
		ui->progressLabel->setText(tr("create component tree '%1'").arg(str));
	}
	else
	{
		QString str1 = m_filelist.at(value - m_filelist.size());
		str1 = str1.left(str1.lastIndexOf("."));
		str1 = str1 + tr(".bin.tree");
		str1 = getFileName(str1);
		QString str2 = m_filelist.at(value + 1 - m_filelist.size());
		str2 = str2.left(str2.lastIndexOf("."));
		str2 = str2 + tr(".bin.tree");
		str2 = getFileName(str2);
		ui->progressLabel->setText(tr("tree assignment between '%1' and '%2'").arg(str1).arg(str2));
	}
}


void CreateCellTrackDialog::on_openFilesButton_clicked()
{

    //QStringList filelist ;
    if(ui->fromImagesButton->isChecked())
    {
         m_filelist = QFileDialog::getOpenFileNames(
                                this,
                                "Select one or more files to open",
                                "",
                                "Images (*.tif *.tiff)");
        /*
           filelist =  QFileDialog::getOpenFileNames(
            this,
            "Choose image files",
            "",
            "Images (*.tif *.tiff)");
            */
    }
    else
    {
         m_filelist = QFileDialog::getOpenFileNames(
                                this,
                                "Select one or more files to open",
                                "",
                                "Images (*.tree)");
        /*
        filelist = QFileDialog::getOpenFileNames(
                this,
                "Choose tree files",
                "",
                "Images (*.tree)");
                */
    }
    QString allfiles = "";
    QStringList::iterator it = m_filelist.begin();
    while(it != m_filelist.end())
    {
        allfiles += tr("%1\n").arg(*it);
        it++;
    }
    ui->filesListEditor->setText(allfiles);
}


void CreateCellTrackDialog::accept()
{
	if(checkValid())
	{ 
		if(m_celltrack == NULL)
		{
			QMessageBox::question(this, "","Please click \"Start Tracking\" button first!");
			return;
		}
		else return QDialog::accept();
	}
	else 
		ui->startTrackingButton->click();
}

void CreateCellTrackDialog::reject()
{
	if(checkValid() && m_celltrack != NULL)
	{
		int rt = QMessageBox::information(this, "Cancel","Are you sure to cancel? \n The tracking result will be discarded.", QMessageBox::Yes|QMessageBox::No);
		if(rt == QMessageBox::Yes) return QDialog::reject();
		else return;
	}
	else return QDialog::reject();
}


void CreateCellTrackDialog::on_exportButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     "",
                                                     QFileDialog::ShowDirsOnly);
    if(dir != tr("")) m_celltrack->exportImages((char*)"",(char*)dir.toStdString().c_str());
}

void CreateCellTrackDialog::on_startTrackingButton_clicked()
{
    if(checkValid())
    {
            ui->progressBar->setMinimum(0);
            ui->progressBar->setMaximum(2*m_filelist.size()-1);
            ui->progressBar->show();
            ui->progressLabel->show();
        if(m_celltrack == NULL) m_celltrack = new CellTrack();
        else
        {
            m_celltrack->releaseFrames();
            m_celltrack->releaseAllCells();
            m_celltrack->releaseTracks();
        }

        vector<string> filelist;
        QStringList::iterator it = m_filelist.begin();
        while(it != m_filelist.end())
        {
            QString file = *it;
            filelist.push_back(file.toStdString().c_str());
            it++;
        }
        if(this->isFromTrees())
        {
            ui->progressBar->setMinimum(m_filelist.size());
            vector<string> & tree_files = filelist;
            if(!m_celltrack->createFromTrees(tree_files))
            {
                QMessageBox::information(this,tr("Create Error"),tr("Unable to create from tree files!"));
            }
            else ui->exportButton->setEnabled(true);
        }
        else
        {
            int min = this->getMinThresh();
            int max = this->getMaxThresh();
            int single = this->getSingleThresh();

            vector<string> & image_files = filelist;
            ComponentTree * tree = new ComponentTree();
            vector<string> tree_files;
            for(int i = 0; i < image_files.size(); i++)
            {
                emit setProgressValue(i);
                tree->clear();
                tree->create((char*) image_files[i].c_str(), min, max, single);
                //===============================================
                string tree_file = image_files[i];
                tree_file = tree_file.substr(0, tree_file.rfind("."));
                tree_file.append(".bin.tree");
                //===============================================
                tree->save((const char*)tree_file.c_str());
                tree_files.push_back(tree_file);
            }

            if(! m_celltrack->createFromTrees(tree_files))
            {
                QMessageBox::information(this,tr("Create Error"),tr("Unable to create from image files!"));
            }
            else ui->exportButton->setEnabled(true);
        }
		ui->progressBar->setValue(2*m_filelist.size() - 1);
		//QMessageBox::information(this,tr("Create Finished"),tr("Create Successfully!"));
        return ;// QDialog::accept();
    }
    else
    {
        QMessageBox::information(this, tr("Invalid Information"),tr("Please Check Your Input!"));
        if(ui->fromImagesButton->isChecked())
        {
            QString minStr = ui->minEditor->text();
            QString maxStr = ui->maxEditor->text();
            QString singleStr = ui->singleEditor->text();
            if(!isInt(minStr)) ui->minEditor->setText(tr(""));
            if(!isInt(maxStr)) ui->maxEditor->setText(tr(""));
            if(!isInt(singleStr)) ui->singleEditor->setText(tr(""));
        }
    }
}
