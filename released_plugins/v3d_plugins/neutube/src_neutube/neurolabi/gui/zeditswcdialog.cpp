#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "zeditswcdialog.h"

#include <stdio.h>
#include "tz_stack_lib.h"
#include "tz_swc_tree.h"
#include "tz_utilities.h"
#include "tz_error.h"
#include "tz_math.h"
#include "tz_darray.h"
#include "tz_u8array.h"
#include "tz_iarray.h"
#include "tz_xml_utils.h"
#include "tz_random.h"
#include "tz_geo3d_utils.h"

#include "zswctree.h"


ZEditSwcDialog::ZEditSwcDialog(QWidget *parent, QList<ZSwcTree*> *swcTreeList) :
    QDialog(parent), m_swcTreeList(swcTreeList)
{
  m_inputFileEdit = new QLineEdit;
  m_inputFileEdit->setReadOnly(true);
  m_selectInputFileButton = new QToolButton(this);
  m_selectInputFileButton->setText(tr("..."));
  m_selectInputFileButton->setToolTip(tr("Select Input File"));
  connect(m_selectInputFileButton, SIGNAL(clicked()), this, SLOT(selectInputFile()));

  m_outputFileEdit = new QLineEdit;
  m_selectOutputFileButton = new QToolButton(this);
  m_selectOutputFileButton->setText(tr("..."));
  m_selectOutputFileButton->setToolTip(tr("Select Output File"));
  connect(m_selectOutputFileButton, SIGNAL(clicked()), this, SLOT(selectOutputFile()));

  QSignalMapper *signalMapper = new QSignalMapper(this);
  connect(signalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(selectMultipleSwcFiles(QWidget*)));

  m_subtractFileEdit = new QLineEdit;
  m_subtractFileEdit->setReadOnly(true);
  m_selectSubtractFileButton = new QToolButton(this);
  m_selectSubtractFileButton->setText(tr("..."));
  signalMapper->setMapping(m_selectSubtractFileButton, m_selectSubtractFileButton);
  connect(m_selectSubtractFileButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
  m_searchFileEdit = new QLineEdit;
  m_searchFileEdit->setReadOnly(true);
  m_selectSearchFileButton = new QToolButton(this);
  m_selectSearchFileButton->setText(tr("..."));
  signalMapper->setMapping(m_selectSearchFileButton, m_selectSearchFileButton);
  connect(m_selectSearchFileButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
  m_glueFileEdit = new QLineEdit;
  m_glueFileEdit->setReadOnly(true);
  m_selectGlueFileButton = new QToolButton(this);
  m_selectGlueFileButton->setText(tr("..."));
  signalMapper->setMapping(m_selectGlueFileButton, m_selectGlueFileButton);
  connect(m_selectGlueFileButton, SIGNAL(clicked()), signalMapper, SLOT(map()));


  createInputGroupBox();
  createOutputGroupBox();
  createOperationGroupBox();

  m_runButton = new QPushButton(tr("Run"), this);
  m_exitButton = new QPushButton(tr("Exit"), this);
  m_buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_buttonBox->addButton(m_exitButton, QDialogButtonBox::RejectRole);
  m_buttonBox->addButton(m_runButton, QDialogButtonBox::ActionRole);
  connect(m_exitButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_runButton, SIGNAL(clicked()), this, SLOT(runOperations()));

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_inputGroupBox);
  mainLayout->addWidget(m_outputGroupBox);
  mainLayout->addWidget(m_operationGroupBox);
  mainLayout->addWidget(m_buttonBox);
  setLayout(mainLayout);

  setWindowTitle(tr("Edit Swc"));
}

ZEditSwcDialog::~ZEditSwcDialog()
{

}

void ZEditSwcDialog::createInputGroupBox()
{
  m_inputGroupBox = new QGroupBox(tr("Input"), this);
  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *hlayout = new QHBoxLayout;
  if (m_swcTreeList == NULL) {
    hlayout->addWidget(new QLabel(tr("Input File:"), this));
    hlayout->addWidget(m_inputFileEdit);
    hlayout->addWidget(m_selectInputFileButton);
    m_inputGroupBox->setLayout(hlayout);
  } else {
    m_useCurrentSwcButton = new QRadioButton("Use Current Opened Swc", this);
    m_useOtherFileButton = new QRadioButton("Use Other File", this);
    m_useCurrentSwcButton->setChecked(true);
    m_inputFileEdit->setEnabled(false);
    m_selectInputFileButton->setEnabled(false);
    connect(m_useCurrentSwcButton, SIGNAL(clicked()), this, SLOT(setInputSwcSource()));
    connect(m_useOtherFileButton, SIGNAL(clicked()), this, SLOT(setInputSwcSource()));
    layout->addWidget(m_useCurrentSwcButton);
    hlayout->addWidget(m_useOtherFileButton);
    hlayout->addWidget(m_inputFileEdit);
    hlayout->addWidget(m_selectInputFileButton);
    layout->addLayout(hlayout);
    m_inputGroupBox->setLayout(layout);
  }
}

void ZEditSwcDialog::createOutputGroupBox()
{
  m_outputGroupBox = new QGroupBox(tr("Output"), this);
  //m_openOutputFileCheckBox = new QCheckBox(tr("Open Output File after process"), this);
  QVBoxLayout *layout = new QVBoxLayout;
  QHBoxLayout *hlayout = new QHBoxLayout;
  hlayout->addWidget(new QLabel(tr("Output File:"), this));
  hlayout->addWidget(m_outputFileEdit);
  hlayout->addWidget(m_selectOutputFileButton);
  layout->addLayout(hlayout);
  //layout->addWidget(m_openOutputFileCheckBox);
  m_outputGroupBox->setLayout(layout);
}

void ZEditSwcDialog::createOperationGroupBox()
{
  m_operationGroupBox = new QGroupBox(tr("Operations"), this);
  QGridLayout *layout = new QGridLayout;
  int row = 0;

  m_subtractCheckBox = new QCheckBox(tr("subtract"), this);
  m_subtractCheckBox->setToolTip(tr("subtract trees from input tree."));
  connect(m_subtractCheckBox, SIGNAL(stateChanged(int)), this, SLOT(subtractCheckBoxChanged(int)));
  m_subtractFileEdit->setEnabled(false);
  m_selectSubtractFileButton->setEnabled(false);
  layout->addWidget(m_subtractCheckBox, row, 0);
  layout->addWidget(m_subtractFileEdit, row, 1, 1, 6);
  layout->addWidget(m_selectSubtractFileButton, row, 7);
  row++;

  m_searchCheckBox = new QCheckBox(tr("search"), this);
  m_searchCheckBox->setToolTip(tr("search trees at a given position."));
  connect(m_searchCheckBox, SIGNAL(stateChanged(int)), this, SLOT(searchCheckBoxChanged(int)));
  layout->addWidget(m_searchCheckBox, row, 0);
  m_searchPosXSpinBox = createDoubleSpinBox();
  m_searchPosYSpinBox = createDoubleSpinBox();
  m_searchPosZSpinBox = createDoubleSpinBox();
  QLabel *pl = new QLabel(tr("x:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_searchPosXSpinBox, row, 2);
  pl = new QLabel(tr("y:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 3);
  layout->addWidget(m_searchPosYSpinBox, row, 4);
  pl = new QLabel(tr("z:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_searchPosZSpinBox, row, 6);
  row++;
  pl = new QLabel(tr("in:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_searchFileEdit, row, 2, 1, 5);
  layout->addWidget(m_selectSearchFileButton, row, 7);
  m_searchFileEdit->setEnabled(false);
  m_selectSearchFileButton->setEnabled(false);
  m_searchPosXSpinBox->setEnabled(false);
  m_searchPosYSpinBox->setEnabled(false);
  m_searchPosZSpinBox->setEnabled(false);
  row++;

  m_glueCheckBox = new QCheckBox(tr("glue"), this);
  m_glueCheckBox->setToolTip(tr("glue trees."));
  connect(m_glueCheckBox, SIGNAL(stateChanged(int)), this, SLOT(glueCheckBoxChanged(int)));
  layout->addWidget(m_glueCheckBox, row, 0);
  layout->addWidget(m_glueFileEdit, row, 1, 1, 6);
  layout->addWidget(m_selectGlueFileButton, row, 7);
  m_glueFileEdit->setEnabled(false);
  m_selectGlueFileButton->setEnabled(false);
  row++;

  m_cutCheckBox = new QCheckBox(tr("cut"), this);
  m_cutCheckBox->setToolTip(tr("cut nodes with specified IDs."));
  connect(m_cutCheckBox, SIGNAL(stateChanged(int)), this, SLOT(cutCheckBoxChanged(int)));
  m_cutLineEdit = new QLineEdit;
  layout->addWidget(m_cutCheckBox, row, 0);
  layout->addWidget(m_cutLineEdit, row, 1, 1, 6);
  m_cutLineEdit->setEnabled(false);
  row++;

  m_rootCheckBox = new QCheckBox(tr("set root"), this);
  connect(m_rootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(rootCheckBoxChanged(int)));
  layout->addWidget(m_rootCheckBox, row, 0);
  m_rootXSpinBox = createDoubleSpinBox();
  m_rootYSpinBox = createDoubleSpinBox();
  m_rootZSpinBox = createDoubleSpinBox();
  pl = new QLabel(tr("x:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_rootXSpinBox, row, 2);
  pl = new QLabel(tr("y:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 3);
  layout->addWidget(m_rootYSpinBox, row, 4);
  pl = new QLabel(tr("z:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_rootZSpinBox, row, 6);
  m_rootXSpinBox->setEnabled(false);
  m_rootYSpinBox->setEnabled(false);
  m_rootZSpinBox->setEnabled(false);
  row++;

  m_cleanRootCheckBox = new QCheckBox(tr("clean root"), this);
  m_cleanRootCheckBox->setToolTip(tr("cut off branches of the root to make sure it has only one child."));
  connect(m_cleanRootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(cleanRootCheckBoxChanged(int)));
  layout->addWidget(m_cleanRootCheckBox, row, 0);
  m_removeOvershootCheckBox = new QCheckBox(tr("remove overshoot"), this);
  m_removeOvershootCheckBox->setToolTip(tr("find and remove overshoots."));
  connect(m_removeOvershootCheckBox, SIGNAL(stateChanged(int)), this, SLOT(removeOvershootCheckBoxChanged(int)));
  layout->addWidget(m_removeOvershootCheckBox, row, 2);
  m_tuneForkCheckBox = new QCheckBox(tr("tune fork"), this);
  m_tuneForkCheckBox->setToolTip(tr("tune branch points to make it more reasonable (not guaranteed to work)."));
  connect(m_tuneForkCheckBox, SIGNAL(stateChanged(int)), this, SLOT(tuneForkCheckBoxChanged(int)));
  layout->addWidget(m_tuneForkCheckBox, row, 4);
  m_singleCheckBox = new QCheckBox(tr("single"), this);
  m_singleCheckBox->setToolTip(tr("extract the biggest tree."));
  connect(m_singleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(singleCheckBoxChanged(int)));
  layout->addWidget(m_singleCheckBox, row, 6);
  row++;

  m_typeCheckBox = new QCheckBox(tr("set type"), this);
  m_typeCheckBox->setToolTip(tr("change the type of the tree."));
  connect(m_typeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(typeCheckBoxChanged(int)));
  m_rangeCheckBox = new QCheckBox(tr("select range"), this);
  m_rangeCheckBox->setToolTip(tr("select range for change type."));
  connect(m_rangeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(rangeCheckBoxChanged(int)));
  m_typeSpinBox = createIntSpinBox();
  m_rangeBeginSpinBox = createIntSpinBox();
  m_rangeEndSpinBox = createIntSpinBox();
  layout->addWidget(m_typeCheckBox, row, 0);
  layout->addWidget(m_typeSpinBox, row, 2);
  row++;
  layout->addWidget(m_rangeCheckBox, row, 2);
  pl = new QLabel(tr("start:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 3);
  layout->addWidget(m_rangeBeginSpinBox, row, 4);
  pl = new QLabel(tr("end:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_rangeEndSpinBox, row, 6);
  m_typeSpinBox->setEnabled(false);
  m_rangeCheckBox->setEnabled(false);
  m_rangeBeginSpinBox->setEnabled(false);
  m_rangeEndSpinBox->setEnabled(false);
  row++;

  m_translateCheckBox = new QCheckBox(tr("translate"), this);
  m_translateCheckBox->setToolTip(tr("translate the input tree. It is always done before resizing."));
  connect(m_translateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(translateCheckBoxChanged(int)));
  layout->addWidget(m_translateCheckBox, row, 0);
  m_translateXSpinBox = createDoubleSpinBox();
  m_translateYSpinBox = createDoubleSpinBox();
  m_translateZSpinBox = createDoubleSpinBox();
  pl = new QLabel(tr("x:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_translateXSpinBox, row, 2);
  pl = new QLabel(tr("y:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 3);
  layout->addWidget(m_translateYSpinBox, row, 4);
  pl = new QLabel(tr("z:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_translateZSpinBox, row, 6);
  m_translateXSpinBox->setEnabled(false);
  m_translateYSpinBox->setEnabled(false);
  m_translateZSpinBox->setEnabled(false);
  row++;

  m_cleanSmallCheckBox = new QCheckBox(tr("clean small"), this);
  connect(m_cleanSmallCheckBox, SIGNAL(stateChanged(int)), this, SLOT(cleanSmallCheckBoxChanged(int)));
  layout->addWidget(m_cleanSmallCheckBox, row, 0);
  m_cleanSmallThresholdSpinBox = createDoubleSpinBox();
  layout->addWidget(new QLabel(tr("threshold:"), this), row, 1);
  layout->addWidget(m_cleanSmallThresholdSpinBox, row, 2);
  m_cleanSmallThresholdSpinBox->setEnabled(false);
  m_mergeCheckBox = new QCheckBox(tr("merge"), this);
  m_mergeCheckBox->setToolTip(tr("reduce a tree by merging close nodes."));
  connect(m_mergeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(mergeCheckBoxChanged(int)));
  layout->addWidget(m_mergeCheckBox, row, 4);
  m_mergeThresholdSpinBox = createDoubleSpinBox();
  layout->addWidget(new QLabel(tr("threshold:"), this), row, 5);
  layout->addWidget(m_mergeThresholdSpinBox, row, 6);
  m_mergeThresholdSpinBox->setEnabled(false);
  row++;

  m_resizeCheckBox = new QCheckBox(tr("resize"), this);
  m_resizeCheckBox->setToolTip(tr("rescale the input tree."));
  connect(m_resizeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(resizeCheckBoxChanged(int)));
  m_resizeXScaleSpinBox = createDoubleSpinBox();
  m_resizeYScaleSpinBox = createDoubleSpinBox();
  m_resizeZScaleSpinBox = createDoubleSpinBox();
  layout->addWidget(m_resizeCheckBox, row, 0);
  pl = new QLabel(tr("x scale:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_resizeXScaleSpinBox, row, 2);
  pl = new QLabel(tr("y scale:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 3);
  layout->addWidget(m_resizeYScaleSpinBox, row, 4);
  pl = new QLabel(tr("z scale:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_resizeZScaleSpinBox, row, 6);
  m_resizeXScaleSpinBox->setEnabled(false);
  m_resizeYScaleSpinBox->setEnabled(false);
  m_resizeZScaleSpinBox->setEnabled(false);
  row++;

  m_mtCheckBox = new QCheckBox(tr("label main trunk"), this);
  connect(m_mtCheckBox, SIGNAL(stateChanged(int)), this, SLOT(mtCheckBoxChanged(int)));
  layout->addWidget(m_mtCheckBox, row, 0);
  m_mainTrunkTypeSpinBox = createIntSpinBox();
  pl = new QLabel(tr("with type:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 1);
  layout->addWidget(m_mainTrunkTypeSpinBox, row, 2);
  m_mainTrunkTypeSpinBox->setEnabled(false);

  m_somaCheckBox = new QCheckBox(tr("label soma"), this);
  connect(m_somaCheckBox, SIGNAL(stateChanged(int)), this, SLOT(somaCheckBoxChanged(int)));
  layout->addWidget(m_somaCheckBox, row, 4);
  m_somaTypeSpinBox = createIntSpinBox();
  pl = new QLabel(tr("with type:"), this);
  pl->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(pl, row, 5);
  layout->addWidget(m_somaTypeSpinBox, row, 6);
  m_somaTypeSpinBox->setEnabled(false);
  row++;

  m_pyramidalCheckBox = new QCheckBox(tr("build pyramidal"), this);
  m_pyramidalCheckBox->setToolTip(tr("process the input swc tree as a pyramidal cell."));
  connect(m_pyramidalCheckBox, SIGNAL(stateChanged(int)), this, SLOT(pyramidalCheckBoxChanged(int)));
  layout->addWidget(m_pyramidalCheckBox, row, 0);
  m_decomposeCheckBox = new QCheckBox(tr("decompose"), this);
  m_decomposeCheckBox->setToolTip(tr("decompose the tree into individual branches and each branch is saved as a separate SWC file. When this argument is specified, the output file will be taken as the prefix of the output files."));
  connect(m_decomposeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(decomposeCheckBoxChanged(int)));
  layout->addWidget(m_decomposeCheckBox, row, 2);
  row++;

  m_lengthCheckBox = new QCheckBox(tr("show length"), this);
  m_lengthCheckBox->setToolTip(tr("show length in svg."));
  connect(m_lengthCheckBox, SIGNAL(stateChanged(int)), this, SLOT(lengthCheckBoxChanged(int)));
  layout->addWidget(m_lengthCheckBox, row, 0);
  m_shuffleCheckBox = new QCheckBox(tr("shuffling"), this);
  m_shuffleCheckBox->setToolTip(tr("shuffle the punctas."));
  connect(m_shuffleCheckBox, SIGNAL(stateChanged(int)), this, SLOT(shuffleCheckBoxChanged(int)));
  layout->addWidget(m_shuffleCheckBox, row, 2);
  m_pcountCheckBox = new QCheckBox(tr("show count"), this);
  m_pcountCheckBox->setToolTip(tr("show counting in svg file."));
  connect(m_pcountCheckBox, SIGNAL(stateChanged(int)), this, SLOT(pcountCheckBoxChanged(int)));
  layout->addWidget(m_pcountCheckBox, row, 4);
  row++;

  m_apoCheckBox = new QCheckBox(tr("apo"), this);
  m_apoCheckBox->setToolTip(tr("supply apo files while producing dendrogram."));
  connect(m_apoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(apoCheckBoxChanged(int)));
  m_apoFileEdit = new QLineEdit;
  m_apoFileEdit->setReadOnly(true);
  m_selectApoFileButton = new QToolButton(this);
  m_selectApoFileButton->setText(tr("..."));
  m_selectApoFileButton->setToolTip(tr("Select Apo Files"));
  connect(m_selectApoFileButton, SIGNAL(clicked()), this, SLOT(selectMultipleApoFiles()));
  layout->addWidget(m_apoCheckBox, row, 0);
  layout->addWidget(m_apoFileEdit, row, 1, 1, 6);
  layout->addWidget(m_selectApoFileButton, row, 7);
  m_apoFileEdit->setEnabled(false);
  m_selectApoFileButton->setEnabled(false);
  row++;

  m_colorCheckBox = new QCheckBox(tr("color"), this);
  connect(m_colorCheckBox, SIGNAL(stateChanged(int)), this, SLOT(colorCheckBoxChanged(int)));
  layout->addWidget(m_colorCheckBox, row, 0);
  m_colorComboBox = new QComboBox;
  m_colorComboBox->addItem(tr("depth"));
  m_colorComboBox->addItem(tr("surface"));
  m_colorComboBox->addItem(tr("intensity from tif file:"));
  m_colorCodeMode = "depth";
  connect(m_colorComboBox, SIGNAL(activated(int)), this, SLOT(colorChanged(int)));
  m_colorCodeTifFileEdit = new QLineEdit;
  m_colorCodeTifFileEdit->setReadOnly(true);
  m_selectColorCodeTifFileButton = new QToolButton(this);
  m_selectColorCodeTifFileButton->setText(tr("..."));
  connect(m_selectColorCodeTifFileButton, SIGNAL(clicked()), this, SLOT(selectColorCodeTifFile()));
  layout->addWidget(m_colorComboBox, row, 2);
  layout->addWidget(m_colorCodeTifFileEdit, row, 3, 1, 4);
  layout->addWidget(m_selectColorCodeTifFileButton, row, 7);
  m_colorComboBox->setEnabled(false);
  m_colorCodeTifFileEdit->setEnabled(false);
  m_selectColorCodeTifFileButton->setEnabled(false);
  row++;

  m_signalCheckBox = new QCheckBox(tr("signal"), this);
  connect(m_signalCheckBox, SIGNAL(stateChanged(int)), this, SLOT(signalCheckBoxChanged(int)));
  layout->addWidget(m_signalCheckBox, row, 0);
  m_signalTifFileEdit = new QLineEdit;
  m_signalTifFileEdit->setReadOnly(true);
  m_selectSignalTifFileButton = new QToolButton(this);
  m_selectSignalTifFileButton->setText(tr("..."));
  connect(m_selectSignalTifFileButton, SIGNAL(clicked()), this, SLOT(selectSignalTifFile()));
  layout->addWidget(m_signalTifFileEdit, row, 1, 1, 3);
  layout->addWidget(m_selectSignalTifFileButton, row, 4);
  m_sigmarginCheckBox = new QCheckBox(tr("margin"), this);
  connect(m_sigmarginCheckBox, SIGNAL(stateChanged(int)), this, SLOT(sigmarginCheckBoxChanged(int)));
  m_signalMarginSpinBox = createDoubleSpinBox();
  layout->addWidget(m_sigmarginCheckBox, row, 5);
  layout->addWidget(m_signalMarginSpinBox, row, 6);
  m_signalTifFileEdit->setEnabled(false);
  m_selectSignalTifFileButton->setEnabled(false);
  m_sigmarginCheckBox->setEnabled(false);
  m_signalMarginSpinBox->setEnabled(false);
  row++;

  m_infoCheckBox = new QCheckBox(tr("show info"), this);
  m_infoCheckBox->setToolTip(tr("print information of a tree."));
  connect(m_infoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(infoCheckBoxChanged(int)));
  layout->addWidget(m_infoCheckBox, row, 0);
  m_infoComboBox = new QComboBox;
  m_infoComboBox->addItem(tr("all"));
  m_infoComboBox->addItem(tr("length"));
  m_infoComboBox->addItem(tr("leaf"));
  m_infoComboBox->addItem(tr("branch number"));
  m_infoComboBox->addItem(tr("surface"));
  m_info = "all";
  connect(m_infoComboBox, SIGNAL(activated(int)), this, SLOT(infoChanged(int)));
  layout->addWidget(m_infoComboBox, row, 2);
  m_infoComboBox->setEnabled(false);

  m_analysisCheckBox = new QCheckBox(tr("get analysis file"), this);
  m_analysisCheckBox->setToolTip(tr("turn an swc file and an apo file to files for analysis."));
  connect(m_analysisCheckBox, SIGNAL(stateChanged(int)), this, SLOT(analysisCheckBoxChanged(int)));
  layout->addWidget(m_analysisCheckBox, row, 4);
  m_featFileCheckBox = new QCheckBox(tr("get feature file"), this);
  connect(m_featFileCheckBox, SIGNAL(stateChanged(int)), this, SLOT(featFileCheckBoxChanged(int)));
  layout->addWidget(m_featFileCheckBox, row, 6);

  m_operationGroupBox->setLayout(layout);
}

void ZEditSwcDialog::setInputSwcSource()
{
  if (m_useCurrentSwcButton->isChecked()) {
    m_inputFileEdit->setEnabled(false);
    m_selectInputFileButton->setEnabled(false);
  } else {
    m_inputFileEdit->setEnabled(true);
    m_selectInputFileButton->setEnabled(true);
  }
}

void ZEditSwcDialog::selectInputFile()
{
  QString inputFileName = QFileDialog::getOpenFileName(
        this, tr("select input file for swc edit"),
        m_openFilesPath,
        tr("Swc Files (*.swc *.apo)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!inputFileName.isEmpty()) {
    m_openFilesPath = inputFileName;
    m_inputFileEdit->setText(inputFileName);
  }
}

void ZEditSwcDialog::selectOutputFile()
{
  QString outputFileName = QFileDialog::getSaveFileName(this,
                                                  tr("specify output file (or prefix)"),
                                                  m_openFilesPath,
                                                  tr("All Files (*)"));
  if (!outputFileName.isEmpty()) {
    m_openFilesPath = outputFileName;
    m_outputFileEdit->setText(outputFileName);
  }
}

void ZEditSwcDialog::selectMultipleSwcFiles(QWidget *sender)
{
  QStringList tmp;
  tmp = QFileDialog::getOpenFileNames(
        this, tr("select multiple swcs for subtract, glue, or search"),
        m_openFilesPath,
        tr("SWC Files (*.swc)"));
  if (tmp.count()) {
    m_openFilesPath = tmp[0];
    if (sender == m_selectSubtractFileButton) {
      m_subtractOtherSwcFileNames = tmp;
      m_subtractFileEdit->setText(QString("[%1]").arg(tmp.join(", ")));
    } else if (sender == m_selectSearchFileButton) {
      m_searchOtherSwcFileNames = tmp;
      m_searchFileEdit->setText(QString("[%1]").arg(tmp.join(", ")));
    } else if (sender == m_selectGlueFileButton) {
      m_glueOtherSwcFileNames = tmp;
      m_glueFileEdit->setText(QString("[%1]").arg(tmp.join(", ")));
    }
  }
}

void ZEditSwcDialog::selectMultipleApoFiles()
{
  m_apoFileNames = QFileDialog::getOpenFileNames(
        this, tr("select apo files"),
        m_openFilesPath,
        tr("APO Files (*.apo)"), NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (m_apoFileNames.count()) {
    m_openFilesPath = m_apoFileNames[0];
    m_apoFileEdit->setText(QString("[%1]").arg(m_apoFileNames.join(", ")));
  }
}

void ZEditSwcDialog::selectColorCodeTifFile()
{
  m_colorCodeTifFileName = QFileDialog::getOpenFileName(
        this, tr("select tif file for intensity color code"),
        m_openFilesPath,
        tr("TIF File (*.tif)"), NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!m_colorCodeTifFileName.isEmpty()) {
    m_openFilesPath = m_colorCodeTifFileName;
    m_colorCodeTifFileEdit->setText(m_colorCodeTifFileName);
  }
}

void ZEditSwcDialog::selectSignalTifFile()
{
  m_signalTifFileName = QFileDialog::getOpenFileName(
        this, tr("select tif file for intensity color code"),
        m_openFilesPath,
        tr("TIF File (*.tif)"), NULL/*, QFileDialog::DontUseNativeDialog*/);
  if (!m_signalTifFileName.isEmpty()) {
    m_openFilesPath = m_signalTifFileName;
    m_signalTifFileEdit->setText(m_signalTifFileName);
  }
}

QDoubleSpinBox* ZEditSwcDialog::createDoubleSpinBox()
{
  QDoubleSpinBox *pSpinBox = new QDoubleSpinBox(this);
  pSpinBox->setRange(-1E10, 1E10);
  pSpinBox->setDecimals(4);
  return pSpinBox;
}

QSpinBox* ZEditSwcDialog::createIntSpinBox()
{
  QSpinBox *pSpinBox = new QSpinBox(this);
  pSpinBox->setRange(-1000, 1E9);
  return pSpinBox;
}

void ZEditSwcDialog::colorChanged(int index)
{
  switch (index) {
  case 0:
    m_colorCodeMode = "depth";
    m_colorCodeTifFileEdit->setEnabled(false);
    m_selectColorCodeTifFileButton->setEnabled(false);
    break;
  case 1:
    m_colorCodeMode = "surface";
    m_colorCodeTifFileEdit->setEnabled(false);
    m_selectColorCodeTifFileButton->setEnabled(false);
    break;
  case 2:
    m_colorCodeMode = "intensity";
    m_colorCodeTifFileEdit->setEnabled(true);
    m_selectColorCodeTifFileButton->setEnabled(true);
  }
}

void ZEditSwcDialog::infoChanged(int index)
{
  switch (index) {
  case 0:
    m_info = "all";
    break;
  case 1:
    m_info = "length";
    break;
  case 2:
    m_info = "leaf";
    break;
  case 3:
    m_info = "branch_number";
    break;
  case 4:
    m_info = "surface";
  }
}

void ZEditSwcDialog::subtractCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_subtractFileEdit->setEnabled(true);
    m_selectSubtractFileButton->setEnabled(true);
  } else {
    m_subtractFileEdit->setEnabled(false);
    m_selectSubtractFileButton->setEnabled(false);
  }

}

void ZEditSwcDialog::searchCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_searchFileEdit->setEnabled(true);
    m_selectSearchFileButton->setEnabled(true);
    m_searchPosXSpinBox->setEnabled(true);
    m_searchPosYSpinBox->setEnabled(true);
    m_searchPosZSpinBox->setEnabled(true);
  } else {
    m_searchFileEdit->setEnabled(false);
    m_selectSearchFileButton->setEnabled(false);
    m_searchPosXSpinBox->setEnabled(false);
    m_searchPosYSpinBox->setEnabled(false);
    m_searchPosZSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::glueCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_glueFileEdit->setEnabled(true);
    m_selectGlueFileButton->setEnabled(true);
  } else {
    m_glueFileEdit->setEnabled(false);
    m_selectGlueFileButton->setEnabled(false);
  }
}

void ZEditSwcDialog::cutCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_cutLineEdit->setEnabled(true);
  } else {
    m_cutLineEdit->setEnabled(false);
  }
}

void ZEditSwcDialog::rootCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_rootXSpinBox->setEnabled(true);
    m_rootYSpinBox->setEnabled(true);
    m_rootZSpinBox->setEnabled(true);
  } else {
    m_rootXSpinBox->setEnabled(false);
    m_rootYSpinBox->setEnabled(false);
    m_rootZSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::cleanRootCheckBoxChanged(int)
{

}

void ZEditSwcDialog::removeOvershootCheckBoxChanged(int)
{

}

void ZEditSwcDialog::tuneForkCheckBoxChanged(int)
{

}

void ZEditSwcDialog::typeCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_typeSpinBox->setEnabled(true);
    m_rangeCheckBox->setEnabled(true);
  } else {
    m_typeSpinBox->setEnabled(false);
    m_rangeCheckBox->setChecked(false);
    m_rangeCheckBox->setEnabled(false);
  }
}

void ZEditSwcDialog::rangeCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_rangeBeginSpinBox->setEnabled(true);
    m_rangeEndSpinBox->setEnabled(true);
  } else {
    m_rangeBeginSpinBox->setEnabled(false);
    m_rangeEndSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::translateCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_translateXSpinBox->setEnabled(true);
    m_translateYSpinBox->setEnabled(true);
    m_translateZSpinBox->setEnabled(true);
  } else {
    m_translateXSpinBox->setEnabled(false);
    m_translateYSpinBox->setEnabled(false);
    m_translateZSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::cleanSmallCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_cleanSmallThresholdSpinBox->setEnabled(true);
  } else {
    m_cleanSmallThresholdSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::resizeCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_resizeXScaleSpinBox->setEnabled(true);
    m_resizeYScaleSpinBox->setEnabled(true);
    m_resizeZScaleSpinBox->setEnabled(true);
  } else {
    m_resizeXScaleSpinBox->setEnabled(false);
    m_resizeYScaleSpinBox->setEnabled(false);
    m_resizeZScaleSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::mergeCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_mergeThresholdSpinBox->setEnabled(true);
  } else {
    m_mergeThresholdSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::singleCheckBoxChanged(int)
{

}

void ZEditSwcDialog::mtCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_mainTrunkTypeSpinBox->setEnabled(true);
  } else {
    m_mainTrunkTypeSpinBox->setEnabled(false);
  }
}

void ZEditSwcDialog::somaCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_somaTypeSpinBox->setEnabled(true);
  } else {
    m_somaTypeSpinBox->setEnabled(false);
  }
}
void ZEditSwcDialog::pyramidalCheckBoxChanged(int)
{

}
void ZEditSwcDialog::decomposeCheckBoxChanged(int)
{

}
void ZEditSwcDialog::apoCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_apoFileEdit->setEnabled(true);
    m_selectApoFileButton->setEnabled(true);
  } else {
    m_apoFileEdit->setEnabled(false);
    m_selectApoFileButton->setEnabled(false);
  }
}
void ZEditSwcDialog::lengthCheckBoxChanged(int)
{

}
void ZEditSwcDialog::shuffleCheckBoxChanged(int)
{

}
void ZEditSwcDialog::pcountCheckBoxChanged(int)
{

}
void ZEditSwcDialog::colorCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_colorComboBox->setEnabled(true);
    if (m_colorComboBox->currentIndex() == 2) {
      m_colorCodeTifFileEdit->setEnabled(true);
      m_selectColorCodeTifFileButton->setEnabled(true);
    }
  } else {
    m_colorComboBox->setEnabled(false);
    m_colorCodeTifFileEdit->setEnabled(false);
    m_selectColorCodeTifFileButton->setEnabled(false);
  }
}
void ZEditSwcDialog::signalCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_signalTifFileEdit->setEnabled(true);
    m_selectSignalTifFileButton->setEnabled(true);
    m_sigmarginCheckBox->setEnabled(true);
  } else {
    m_signalTifFileEdit->setEnabled(false);
    m_selectSignalTifFileButton->setEnabled(false);
    m_sigmarginCheckBox->setChecked(false);
    m_sigmarginCheckBox->setEnabled(false);
  }
}
void ZEditSwcDialog::sigmarginCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_signalMarginSpinBox->setEnabled(true);
  } else {
    m_signalMarginSpinBox->setEnabled(false);
  }
}
void ZEditSwcDialog::infoCheckBoxChanged(int state)
{
  if (state == Qt::Checked) {
    m_infoComboBox->setEnabled(true);
  } else {
    m_infoComboBox->setEnabled(false);
  }
}
void ZEditSwcDialog::analysisCheckBoxChanged(int)
{

}
void ZEditSwcDialog::featFileCheckBoxChanged(int)
{

}

void ZEditSwcDialog::runOperations()
{
  QString inputFileName = m_inputFileEdit->text();
  QString outputFileName = m_outputFileEdit->text();
  if (m_swcTreeList != NULL) {
    if (m_useOtherFileButton->isChecked() && inputFileName.isEmpty()) {
      return;
    }
  } else if (inputFileName.isEmpty()) {
    return;
  }
  QByteArray bafilepath = inputFileName.toLocal8Bit();
  const char *filepath = bafilepath.data();
  QByteArray baoutfilepath = outputFileName.toLocal8Bit();
  const char *outfilepath = baoutfilepath.data();

  /* convert apo file to swc file */
  if (inputFileName.endsWith(".apo") && !outputFileName.isEmpty()) {
    Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Import_Apo_E(filepath, 4);
    if (field != NULL) {
      Swc_Node tmp_node;
      FILE *fp = fopen(outfilepath, "w");
      for (int i = 0; i < field->size; i++) {
        tmp_node.x = field->points[i][0];
        tmp_node.y = field->points[i][1];
        tmp_node.z = field->points[i][2];
        tmp_node.id = i + 1;
        tmp_node.parent_id = -1;
        tmp_node.d = Cube_Root(0.75 / TZ_PI * field->values[i]);
        Swc_Node_Fprint(fp, &tmp_node);
      }
      fclose(fp);
    } else {
      return;
    }
    return;
  }

  Swc_Tree *tree;
  if (m_swcTreeList != NULL && m_useCurrentSwcButton->isChecked()) {
    tree = m_swcTreeList->value(m_swcTreeList->size()-1)->cloneData();   //take last one as input
  } else {
    tree = Read_Swc_Tree(filepath);
  }

  if (m_subtractCheckBox->isChecked()) {
    if (m_subtractOtherSwcFileNames.size() > 0) {
      QByteArray tmpba = m_subtractOtherSwcFileNames[0].toLocal8Bit();
      Swc_Tree *sub = Read_Swc_Tree(tmpba.data());
      for (int i = 1; i < m_subtractOtherSwcFileNames.size(); i++) {
        tmpba = m_subtractOtherSwcFileNames[i].toLocal8Bit();
        Swc_Tree *tmp = Read_Swc_Tree(tmpba.data());
        Swc_Tree_Merge(sub, tmp);
        Kill_Swc_Tree(tmp);
      }
      Swc_Tree_Subtract(tree, sub);
    }
    if (!outputFileName.isEmpty()) {
      Write_Swc_Tree(outfilepath, tree);
    }
    return;
  }

  double search_pos[3];
  double mindist;
  int minindex = -1;
  if (m_searchCheckBox->isChecked()) {
    search_pos[0] = m_searchPosXSpinBox->value();
    search_pos[1] = m_searchPosYSpinBox->value();
    search_pos[2] = m_searchPosZSpinBox->value();
    mindist = Swc_Tree_Point_Dist(tree, search_pos[0], search_pos[1],
                                  search_pos[2], NULL, NULL);
  }

  bool node_changed = false;

  for (int i = 0; i < m_subtractOtherSwcFileNames.size(); i++) {
    QByteArray tmpba = m_subtractOtherSwcFileNames[i].toLocal8Bit();
    Swc_Tree *tree2 = Read_Swc_Tree(tmpba.data());

    if (m_searchCheckBox->isChecked()) {
      double dist = Swc_Tree_Point_Dist(tree2, search_pos[0], search_pos[1],
                                        search_pos[2], NULL, NULL);
      if (dist < mindist) {
        mindist = dist;
        minindex = i;
      }
    } else {
      if (m_glueCheckBox->isChecked()) {
        Swc_Tree_Glue(tree, Swc_Tree_Regular_Root(tree2));
        Swc_Tree_Merge_Close_Node(tree, 0.01);
      } else {
        Swc_Tree_Merge(tree, tree2);
      }
    }

    Kill_Swc_Tree(tree2);
    node_changed = TRUE;
  }

  if (m_searchCheckBox->isChecked()) {
    if (minindex == -1) {
      //printf("Search result: %s\n", qstringToChar(m_inputFileName));
      QMessageBox::information(this, tr("search result"), QString("Search result: %1").arg(inputFileName));
    } else {
      //printf("Search result: %s\n", qstringToChar(m_subtractOtherSwcFileNames[minindex]));
      QMessageBox::information(this, tr("search result"), QString("Search result: %1").arg(m_subtractOtherSwcFileNames[minindex]));
    }
  }

  if (m_cutCheckBox->isChecked() && !m_cutLineEdit->text().isEmpty()) {
    QString alltext = m_cutLineEdit->text();
    QStringList septextList = alltext.split(",", QString::SkipEmptyParts);
    QList<int> cutList;
    for (int i=0; i<septextList.size(); i++) {
      bool ok;
      int tmp = septextList[i].toInt(&ok);
      if (ok)
        cutList.push_back(tmp);
    }
    int ncut = cutList.size();

    for (int i = 0; i < ncut; i++) {
      Swc_Tree_Iterator_Start(tree, 2, TRUE);
      Swc_Tree_Node *tn = NULL;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        if (Swc_Tree_Node_Id(tn) == cutList[i]) {
          Swc_Tree_Node_Detach_Parent(tn);
          Swc_Tree_Node *next_tn = Swc_Tree_Node_Next(tn);
          while (tn != NULL) {
            Kill_Swc_Tree_Node(tn);
            tn = next_tn;
            next_tn = Swc_Tree_Node_Next(tn);
            if (next_tn != NULL) {
              if ((tn->next_sibling == next_tn) ||
                  (tn->index > next_tn->index)) {
                next_tn = NULL;
              }
            }
          }
          break;
        }
      }
    }
  }

  if (m_rootCheckBox->isChecked()) {
    double root[3];
    root[0] = m_rootXSpinBox->value();
    root[1] = m_rootYSpinBox->value();
    root[2] = m_rootZSpinBox->value();
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, root);
    Swc_Tree_Node_Set_Root(tn);
    node_changed = TRUE;
  }

  if (m_cleanRootCheckBox->isChecked()) {
    Swc_Tree_Clean_Root(tree);
    node_changed = TRUE;
  }

  if (m_removeOvershootCheckBox->isChecked()) {
    Swc_Tree_Remove_Overshoot(tree);
  }

  if (m_tuneForkCheckBox->isChecked()) {
    Swc_Tree_Tune_Fork(tree);
    node_changed = TRUE;
  }

  if (m_typeCheckBox->isChecked()) {
    int type = m_typeSpinBox->value();

    Swc_Tree_Iterator_Start(tree, 1, FALSE);

    if (m_rangeCheckBox->isChecked()) {
      Swc_Tree_Node *begin = Swc_Tree_Query_Node(
            tree, m_rangeBeginSpinBox->value(), SWC_TREE_ITERATOR_NO_UPDATE);
      Swc_Tree_Node *end = Swc_Tree_Query_Node(
            tree, m_rangeEndSpinBox->value(), SWC_TREE_ITERATOR_NO_UPDATE);

      if ((begin == NULL) || (end == NULL)) {
        //fprintf(stderr, "Invalid selection ID.\n");
        QMessageBox::warning(this, tr("Invalid selection"), tr("Invalid selection ID.\n"));
        return;
      }

      Swc_Tree_Iterator_Path(tree, begin, end);
    }

    Swc_Tree_Node *tn;
    if (type >= 0) {
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        Swc_Tree_Node_Data(tn)->type = type;
      }
    } else if (type == -1) {
      type = 0;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        Swc_Tree_Node_Data(tn)->type = type + 2;
        if (!(Swc_Tree_Node_Is_Continuation(tn) ||
              Swc_Tree_Node_Is_Root(tn))) {
          type++;
          type = type % 11;
        }
      }
    }
  }

  if (m_translateCheckBox->isChecked()) {
    double x = m_translateXSpinBox->value();
    double y = m_translateYSpinBox->value();
    double z = m_translateZSpinBox->value();
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn = tree->root;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        Swc_Tree_Node_Data(tn)->x += x;
        Swc_Tree_Node_Data(tn)->y += y;
        Swc_Tree_Node_Data(tn)->z += z;
      }
    }
  }

  if (m_cleanSmallCheckBox->isChecked()) {
    double threshold = m_cleanSmallThresholdSpinBox->value();

    Swc_Tree_Node *tn = tree->root;
    int n = 0;
    int total = 0;
    if (Swc_Tree_Node_Is_Virtual(tn)) {
      Swc_Tree_Node *child = tn->first_child;
      TZ_ASSERT(Swc_Tree_Node_Is_Regular(child), "virtual");
      tn = child;
      while (tn != NULL) {
        total++;
        Swc_Tree tmp_tree;
        Swc_Tree_Node *tmp_tn1 = tn->next_sibling;
        Swc_Tree_Node *tmp_tn2 = tn->parent;
        tn->next_sibling = NULL;
        tn->parent = NULL;
        tmp_tree.root = tn;
        double length = Swc_Tree_Overall_Length(&tmp_tree);
        tn->next_sibling = tmp_tn1;
        tn->parent = tmp_tn2;
        child = tn->next_sibling;
        //printf("%g\n", length);
        if (length < threshold) {
          n++;
          Swc_Tree_Node_Detach_Parent(tn);
          Swc_Tree_Node_Kill_Subtree(tn);
        }
        tn = child;
      }
    }
    //printf("%d trees removed from %d trees.\n", n, total);
    //printf("%g\n", Swc_Tree_Overall_Length(tree));
    QMessageBox::information(this, tr("result"),
                             QString("%1 trees removed from %2 trees. \n %3\n").arg(n).arg(total).arg(Swc_Tree_Overall_Length(tree)));
    node_changed = TRUE;
  }

  if (m_resizeCheckBox->isChecked()) {
    double x_scale = m_resizeXScaleSpinBox->value();
    double y_scale = m_resizeYScaleSpinBox->value();
    double z_scale = m_resizeZScaleSpinBox->value();
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn = tree->root;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        Swc_Tree_Node_Data(tn)->x *= x_scale;
        Swc_Tree_Node_Data(tn)->y *= y_scale;
        Swc_Tree_Node_Data(tn)->z *= z_scale;
        Swc_Tree_Node_Data(tn)->d *= sqrt(x_scale * y_scale);
      }
    }
  }

  if (m_mergeCheckBox->isChecked()) {
    Swc_Tree_Merge_Close_Node(tree, m_mergeThresholdSpinBox->value());
    node_changed = TRUE;
  }

  if (m_singleCheckBox->isChecked()) {
    if (Swc_Tree_Number(tree) > 1) {
      Swc_Tree_Node *new_root = tree->root->first_child;
      int max_size = Swc_Tree_Node_Fsize(new_root);
      Swc_Tree_Node *tmp_tn = new_root->next_sibling;
      while (tmp_tn != NULL) {
        int tmp_size = Swc_Tree_Node_Fsize(tmp_tn);
        if (tmp_size > max_size) {
          max_size = tmp_size;
          new_root = tmp_tn;
        }
        tmp_tn = tmp_tn->next_sibling;
      }
      Swc_Tree_Node_Detach_Parent(new_root);
      Clean_Swc_Tree(tree);
      tree->root = new_root;
    }
  }

  if (m_mtCheckBox->isChecked()) {
    Swc_Tree_Label_Main_Trunk_L(tree, 5, 300.0, 1600.0);
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Data(tn)->label == 5) {
        Swc_Tree_Node_Data(tn)->type = m_mainTrunkTypeSpinBox->value();
      }
    }
  }

  if (m_somaCheckBox->isChecked()) {
    /* under development */
    //Swc_Tree_Label_Soma(tree, 6, 300.0, 1600.0);
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Node *tn;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Data(tn)->label == 6) {
        Swc_Tree_Node_Data(tn)->type = m_somaTypeSpinBox->value();
      }
    }
  }

  Swc_Tree *soma_tree = NULL;
  Swc_Tree *out_tree = NULL;
  if (m_pyramidalCheckBox->isChecked()) {
    Swc_Tree_Grow_Soma(tree, 1);
    soma_tree = Copy_Swc_Tree(tree);
    out_tree = Copy_Swc_Tree(tree);
    Swc_Tree_Resort_Pyramidal(tree, FALSE, TRUE);
    Swc_Tree_Resort_Pyramidal(out_tree, FALSE, FALSE);
    Swc_Tree_Set_Type_As_Label(tree);
    Swc_Tree_Set_Type_As_Label(out_tree);
    Swc_Tree_Resort_Id(tree);
    Swc_Tree_Resort_Id(out_tree);
  } else {
    soma_tree = Copy_Swc_Tree(tree);
  }



  if (node_changed == TRUE) {
    Swc_Tree_Resort_Id(tree);
  }

  if (!outputFileName.isEmpty()) {
    if (m_decomposeCheckBox->isChecked()) {
      Swc_Tree_Iterator_Start(tree, 1, FALSE);
      Swc_Tree_Node *tn = tree->root;
      int index = 0;
      char filepath1[500];
      BOOL start = TRUE;
      int length = 0;
      FILE *fp = NULL;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
        if (Swc_Tree_Node_Is_Regular(tn)) {
          if (start) {
            if (fp != NULL) {
              fclose(fp);
            }
            sprintf(filepath1, "%s%0*d.swc", outfilepath,  4, index);
            fp = fopen(filepath1, "w");
            start = FALSE;
            if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
              tn->parent->node.parent_id = -1;
              Swc_Node_Fprint(fp, &(tn->parent->node));
            } else {
              tn->node.parent_id = -1;
            }
            length = 0;
          }

          Swc_Node_Fprint(fp, &(tn->node));
          length++;

          if (Swc_Tree_Node_Is_Leaf(tn)) {
            start = TRUE;
            index++;
          } else {
            if (length > 1) {
              if (Swc_Tree_Node_Is_Branch_Point(tn)) {
                start = TRUE;
                index++;
              }
            }
          }
        }
      }
    } else {
      if (outputFileName.endsWith(".dot")) {
        Swc_Tree_To_Dot_File(tree, outfilepath);
      } else if (outputFileName.endsWith(".svg")) {
        Swc_Tree_Svg_Workspace *ws = New_Swc_Tree_Svg_Workspace();
        ws->max_vx = SVG_VIEW_MAX_X;
        ws->max_vy = SVG_VIEW_MAX_Y;
        if (m_lengthCheckBox->isChecked()) {
          ws->showing_length = TRUE;
        }
        int napo = m_apoFileNames.size();
        int *puncta_number = iarray_malloc(napo);
        if (napo > 0) {
          QByteArray tmpba = m_apoFileNames[0].toLocal8Bit();
          ws->puncta = Geo3d_Scalar_Field_Import_Apo(tmpba.data());
          puncta_number[0] = ws->puncta->size;
          for (int i = 1; i < napo; i++) {
            tmpba = m_apoFileNames[i].toLocal8Bit();
            Geo3d_Scalar_Field *puncta =
                Geo3d_Scalar_Field_Import_Apo(tmpba.data());
            puncta_number[i] = puncta_number[i-1] + puncta->size;
            Geo3d_Scalar_Field_Merge(ws->puncta, puncta, ws->puncta);
            Kill_Geo3d_Scalar_Field(puncta);
          }

          ws->puncta_type = iarray_calloc(ws->puncta->size);

          int cur_type = 0;
          for (int i = 0; i < ws->puncta->size; i++) {
            if (i >= puncta_number[cur_type]) {
              cur_type++;
            }
            ws->puncta_type[i] = cur_type;
          }

          if (m_shuffleCheckBox->isChecked()) {
            ws->shuffling = TRUE;
          }

          if (m_pcountCheckBox->isChecked()) {
            ws->showing_count = TRUE;
          }

          if ((soma_tree != NULL) && (ws->puncta != NULL)) {
            if (ws->puncta->size > 0) {
              GUARDED_CALLOC_ARRAY(ws->on_root, ws->puncta->size, BOOL);
              Swc_Tree_Identify_Puncta(soma_tree, ws->puncta, 1, ws->on_root);
            }
          }
        }

        if (m_colorCheckBox->isChecked()) {
          if (m_colorCodeMode == "depth") {
            ws->color_code = SWC_SVG_COLOR_CODE_Z;
            Swc_Tree_Z_Feature(tree);
          } else if (m_colorCodeMode == "surface") {
            ws->color_code = SWC_SVG_COLOR_CODE_SURFAREA;
            Swc_Tree_Perimeter_Feature(tree);
          } else if (m_colorCodeMode == "intensity") {
            ws->color_code = SWC_SVG_COLOR_CODE_INTENSITY;
            QByteArray tmpba = m_colorCodeTifFileName.toLocal8Bit();
            Stack *swc_signal = Read_Stack(tmpba.data());
            Swc_Tree_Intensity_Feature(tree, swc_signal, NULL);
            Kill_Stack(swc_signal);
          } else {
            //fprintf(stderr, "Invalid color option: %s. "
            //        "Color coding is ignored.\n", qstringToChar(m_colorCodeMode));
            QMessageBox::warning(this, tr("Invalid color option"),
                                 QString("Invalid color option: %1. Color coding is ignored.\n").arg(m_colorCodeMode));
          }
        }
        Swc_Tree_To_Svg_File_W(tree, outfilepath, ws);
      } else {
        int napo = m_apoFileNames.size();
        if (napo > 0) {
          Geo3d_Scalar_Field *puncta = New_Geo3d_Scalar_Field();
          QByteArray tmpba = m_apoFileNames[0].toLocal8Bit();
          puncta = Geo3d_Scalar_Field_Import_Apo(tmpba.data());
          Swc_Tree_Puncta_Feature(tree, puncta);
          Kill_Geo3d_Scalar_Field(puncta);
          Swc_Tree_Iterator_Start(tree, 2, FALSE);
          int color_map[] = { 1, 3, 5, 7, 6, 8, 2 };
          Swc_Tree_Node *tn = NULL;
          while ((tn = Swc_Tree_Next(tree)) != NULL) {
            int new_type = iround(tn->feature);
            if (new_type > 6) {
              new_type = 6;
            }
            Swc_Tree_Node_Data(tn)->type = color_map[new_type];
          }
        }
        if (m_signalCheckBox->isChecked()) {
          QByteArray tmpba = m_signalTifFileName.toLocal8Bit();
          Stack *signal = Read_Stack(tmpba.data());
          double margin = 0.0;
          if (m_sigmarginCheckBox->isChecked()) {
            margin = m_signalMarginSpinBox->value();
          }
          Swc_Tree_Intensity_Feature_E(tree, signal, NULL, margin);
          double fmin, fmax;
          Swc_Tree_Feature_Range(tree, &fmin, &fmax);
          Swc_Tree_Iterator_Start(tree, 2, FALSE);
          int color_map[] = { 1, 3, 5, 7, 6, 8, 2 };
          Swc_Tree_Node *tn = NULL;
          while ((tn = Swc_Tree_Next(tree)) != NULL) {
            int new_type = iround((tn->feature - fmin) / (fmax - fmin) * 6.0);
            if (new_type > 6) {
              new_type = 6;
            }
            Swc_Tree_Node_Data(tn)->type = color_map[new_type];
            Swc_Tree_Node_Data(tn)->d += 5.0;
          }

          Kill_Stack(signal);
        }

        if (out_tree != NULL) {
          Write_Swc_Tree(outfilepath, out_tree);
        } else {
          Write_Swc_Tree(outfilepath, tree);
        }
      }
    }
  }

  if (m_infoCheckBox->isChecked()) {
    QString infostr;
    if ((m_info == "length") || (m_info == "all")) {
      //printf("Overall length: %g\n", Swc_Tree_Overall_Length(tree));
      infostr.append(QString("Overall length: %1\n").arg(Swc_Tree_Overall_Length(tree)));
    }

    if ((m_info == "leaf") || (m_info == "all")) {
      //printf("Number of tips: %d\n", Swc_Tree_Leaf_Number(tree));
      infostr.append(QString("Number of tips: %1\n").arg(Swc_Tree_Leaf_Number(tree)));
    }

    if ((m_info == "branch_number") || (m_info == "all")) {
      //printf("Number of branches: %d\n", Swc_Tree_Branch_Number(tree));
      infostr.append(QString("Number of branches: %1\n").arg(Swc_Tree_Branch_Number(tree)));
    }

    if ((m_info == "surface") || (m_info == "all")) {
      //printf("Surface area: %g\n", Swc_Tree_Surface_Area(tree));
      infostr.append(QString("Surface area: %1\n").arg(Swc_Tree_Surface_Area(tree)));
    }
    QMessageBox::information(this, tr("info"), infostr);
  }

  if (m_analysisCheckBox->isChecked()) {
    QByteArray tmpba = m_apoFileNames[0].toLocal8Bit();
    Geo3d_Scalar_Field *puncta =
        Geo3d_Scalar_Field_Import_Apo(tmpba.data());
    //char branch_file[500];
    //char puncta_file[500];
    //sprintf(branch_file, "%s_branch.txt", filepath);
    //sprintf(puncta_file, "%s_puncta.txt", qstringToChar(m_apoFileNames[0]));
    QByteArray babranch_file = QString("%1_branch.txt").arg(inputFileName).toLocal8Bit();
    QByteArray bapuncta_file = QString("%1_puncta.txt").arg(m_apoFileNames[0]).toLocal8Bit();
    Swc_Tree_To_Analysis_File(tree, puncta, babranch_file.data(), bapuncta_file.data());
  }

  if (m_featFileCheckBox->isChecked()) {
//    char *feat_option = Get_String_Arg("-feat_file");
//    if (strcmp(feat_option, "branch")) {
//      Swc_Tree_To_Branch_Feature_File(tree, outfilepath);
//    }
    if (!outputFileName.isEmpty()) {
      Swc_Tree_To_Branch_Feature_File(tree, outfilepath);
    }

  }
  return;

}
