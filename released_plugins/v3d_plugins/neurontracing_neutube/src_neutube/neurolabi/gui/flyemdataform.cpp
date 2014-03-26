#include "flyemdataform.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QTextBrowser>
#include <QListView>
#include <QInputDialog>
#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>

#include "neutubeconfig.h"
#include "tz_error.h"
#include "zswctree.h"
#include "zstackdoc.h"
#include "ui_flyemdataform.h"
#include "flyem/zflyemdataframe.h"
#include "flyem/zflyemstackframe.h"

FlyEmDataForm::FlyEmDataForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmDataForm),
  m_statusBar(NULL),
  m_neuronContextMenu(NULL),
  m_showSelectedModelAction(NULL),
  m_changeClassAction(NULL),
  m_neighborSearchAction(NULL)
{
  ui->setupUi(this);
  ui->progressBar->hide();
  m_neuronList = new ZFlyEmNeuronListModel(this);
  ui->queryView->setModel(m_neuronList);
  ui->queryView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(assignClass(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateStatusBar(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateInfoWindow(QModelIndex)));

  //customize
  ui->testPushButton->hide();
  ui->generalPushButton->hide();

  createAction();
  createContextMenu();
  ui->queryView->setContextMenu(m_neuronContextMenu);

  m_specialProgressReporter.setProgressBar(ui->progressBar);
  setProgressReporter(&m_specialProgressReporter);
}

FlyEmDataForm::~FlyEmDataForm()
{
  delete ui;
}

QSize FlyEmDataForm::sizeHint() const
{
#ifdef _DEBUG_2
  std::cout << geometry().x() << " " << geometry().y()
            << geometry().width() << " " << geometry().height() << std::endl;
#endif

  return geometry().size();
}

void FlyEmDataForm::on_pushButton_clicked()
{
  emit showSummaryTriggered();
}

void FlyEmDataForm::on_processPushButton_clicked()
{
  emit processTriggered();
}

void FlyEmDataForm::on_queryPushButton_clicked()
{
  emit queryTriggered();
}

void FlyEmDataForm::appendOutput(const QString &text)
{
  ui->outputTextEdit->append(text);
}

void FlyEmDataForm::setQueryOutput(const ZFlyEmNeuron *neuron)
{
  m_neuronList->clear();
  appendQueryOutput(neuron);
}

void FlyEmDataForm::appendQueryOutput(const ZFlyEmNeuron *neuron)
{
  //ui->queryTextWidget->append(neuron.c_str());
  if (neuron != NULL) {
    m_neuronList->append(neuron);
  }
}

void FlyEmDataForm::on_testPushButton_clicked()
{
  emit testTriggered();
}

QProgressBar* FlyEmDataForm::getProgressBar()
{
  return ui->progressBar;
}

void FlyEmDataForm::on_generalPushButton_clicked()
{
  emit generalTriggered();
}

void FlyEmDataForm::on_optionPushButton_clicked()
{
  emit optionTriggered();
}

void FlyEmDataForm::on_addDataPushButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load FlyEM Database"), "", tr("Json files (*.json)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    ZFlyEmDataFrame *frame = dynamic_cast<ZFlyEmDataFrame*>(this->parentWidget());
    if (frame != NULL) {
      frame->load(fileName.toStdString(), true);
    }
  }
}

void FlyEmDataForm::on_savePushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Query Output"), "", tr("Text files (*.csv)"),
        NULL);
  if (!fileName.isEmpty()) {
    m_neuronList->exportCsv(fileName);
  }
}

void FlyEmDataForm::updateStatusBar(const QModelIndex &index)
{
  if (m_statusBar != NULL) {
    m_statusBar->showMessage(
          m_neuronList->data(index, Qt::StatusTipRole).toString());
  }
}

void FlyEmDataForm::updateInfoWindow(const QModelIndex &index)
{
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

  ui->infoWindow->clear();
  foreach (const ZFlyEmNeuron* neuron, neuronArray) {
    std::ostringstream stream;
    neuron->print(stream);
    ui->infoWindow->append(stream.str().c_str());
  }
}

void FlyEmDataForm::assignClass(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in assignClass");
    return;
  }

  if (index.column() == 0) {
    ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
    if (neuron != NULL) {
      const std::vector<const ZFlyEmNeuron*> &topMatch = neuron->getTopMatch();
      QString className = "";
      if (!topMatch.empty()) {
        className = topMatch[0]->getClass().c_str();
      }

      //Popup dialog
      bool ok;
      className = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                        tr("Class name:"), QLineEdit::Normal,
                                        className, &ok);
      if (ok) {
        neuron->setClass(className.toStdString());
        m_neuronList->notifyRowDataChanged(index.row());
      }
    }
  }
}

void FlyEmDataForm::viewModel(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in viewSwc");
    return;
  }

  ui->progressBar->setValue(50);
  ui->progressBar->show();
  QApplication::processEvents();

  //const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

#ifdef _DEBUG_2
  std::cout << m_neuronList->headerData(index.column(), Qt::Horizontal).toString().toStdString()
             << std::endl;

#endif

  if (!neuronArray.isEmpty()) {
    ZStackFrame *frame = new ZStackFrame;

    foreach (const ZFlyEmNeuron* neuron, neuronArray) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZSwcTree *model = neuron->getModel();
      ZStackDoc *doc = frame->document().get();
      doc->blockSignals(true);
      if (model != NULL) {
        doc->addSwcTree(model->clone(), true);
      }

      std::vector<ZPunctum*> puncta = neuron->getSynapse();
      for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        doc->addPunctum(*iter);
      }
      doc->blockSignals(false);
      doc->updateModelData(ZStackDoc::SWC_DATA);
      doc->updateModelData(ZStackDoc::PUNCTA_DATA);
    }

    frame->open3DWindow(this->parentWidget());
    delete frame;
  } else if (m_neuronList->headerData(index.column(), Qt::Horizontal) ==
             "Volume Path") {
    if (!m_neuronList->data(index).toString().isEmpty()) {
      emit volumeTriggered(m_neuronList->data(index).toString());
    }
  }

  ui->progressBar->hide();
}

void FlyEmDataForm::showSelectedModel()
{
  ui->progressBar->setValue(50);
  ui->progressBar->show();
  QApplication::processEvents();

  QItemSelectionModel *sel = ui->queryView->selectionModel();

#ifdef _DEBUG_2
  appendOutput(QString("%1 rows selected").arg(sel->selectedIndexes().size()).toStdString());
#endif

  ZStackFrame *frame = new ZStackFrame;

  m_neuronList->retrieveModel(sel->selectedIndexes(), frame->document().get());
  ui->progressBar->setValue(75);
  QApplication::processEvents();

  frame->open3DWindow(this->parentWidget());
  delete frame;

  ui->progressBar->hide();
}

void FlyEmDataForm::on_showModelPushButton_clicked()
{
  showSelectedModel();
}

void FlyEmDataForm::setPresenter(ZFlyEmNeuronPresenter *presenter)
{
  m_neuronList->setPresenter(presenter);
}

void FlyEmDataForm::on_saveSwcPushButton_clicked()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Export SWC"),
    "", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    m_neuronList->exportSwc(dirpath);
  }
}

void FlyEmDataForm::on_exportPushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save bundle"), "", tr("JSON files (*.json)"),
        NULL);
  if (!fileName.isEmpty()) {
    emit saveBundleTriggered(0, fileName);
  }
}

void FlyEmDataForm::changeNeuronClass()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();
  int neuronCount = 0;

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      ++neuronCount;
    }
  }

  if (neuronCount > 0) {
    bool ok;

    QString className = QInputDialog::getText(
          this, QString("%1 neurons: Change Class").arg(neuronCount),
          tr("Class name:"), QLineEdit::Normal,
          "", &ok);
    if (ok) {
      foreach (QModelIndex index, indexList) {
        if (m_neuronList->isNeuronKey(index)) {
          ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
          if (neuron != NULL) {
            neuron->setClass(className.toStdString());
            m_neuronList->notifyRowDataChanged(index.row());
          }
        }
      }
    }
  } else {
    QMessageBox::warning(this,
          "No Neuron Selected",
          "At least one neuron key should be selected to change class");
  }
}

void FlyEmDataForm::createContextMenu()
{
  if (m_neuronContextMenu == NULL) {
    m_neuronContextMenu = new QMenu(this);
    m_neuronContextMenu->addAction(m_showSelectedModelAction);
    m_neuronContextMenu->addAction(m_changeClassAction);
    m_neuronContextMenu->addAction(m_neighborSearchAction);
  }
}

void FlyEmDataForm::createAction()
{
  if (m_showSelectedModelAction == NULL) {
    m_showSelectedModelAction = new QAction("Show Model", this);
    connect(m_showSelectedModelAction, SIGNAL(triggered()),
            this, SLOT(showSelectedModel()));
  }

  if (m_changeClassAction == NULL) {
    m_changeClassAction = new QAction("Change Class", this);
    connect(m_changeClassAction, SIGNAL(triggered()),
            this, SLOT(changeNeuronClass()));
  }

  if (m_neighborSearchAction == NULL) {
    m_neighborSearchAction = new QAction("Search neighbor", this);
    connect(m_neighborSearchAction, SIGNAL(triggered()),
            this, SLOT(showNearbyNeuron()));
  }
}

void FlyEmDataForm::showNearbyNeuron()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      emit showNearbyNeuronTriggered(m_neuronList->getNeuron(index));
      break;
    }
  }
}
