#ifndef ZEDITSWCDIALOG_H
#define ZEDITSWCDIALOG_H

#include <QDialog>
#include <QList>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QToolButton;
class QDoubleSpinBox;
class QSpinBox;
class QPushButton;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QComboBox;
QT_END_NAMESPACE
class ZSwcTree;

class ZEditSwcDialog : public QDialog
{
  Q_OBJECT

#define SVG_VIEW_MAX_X 800
#define SVG_VIEW_MAX_Y 600
public:
  ZEditSwcDialog(QWidget *parent, QList<ZSwcTree*> *swcTreeList = NULL);
  virtual ~ZEditSwcDialog();
signals:

public slots:

private slots:
  void runOperations();
  void setInputSwcSource();
  void selectInputFile();
  void selectOutputFile();
  void selectMultipleSwcFiles(QWidget *sender);
  void selectMultipleApoFiles();
  void selectColorCodeTifFile();
  void selectSignalTifFile();
  void colorChanged(int index);
  void infoChanged(int index);
  void subtractCheckBoxChanged(int state);
  void searchCheckBoxChanged(int state);
  void glueCheckBoxChanged(int state);
  void cutCheckBoxChanged(int state);
  void rootCheckBoxChanged(int state);
  void cleanRootCheckBoxChanged(int state);
  void removeOvershootCheckBoxChanged(int state);
  void tuneForkCheckBoxChanged(int state);
  void typeCheckBoxChanged(int state);
  void rangeCheckBoxChanged(int state);
  void translateCheckBoxChanged(int state);
  void cleanSmallCheckBoxChanged(int state);
  void resizeCheckBoxChanged(int state);
  void mergeCheckBoxChanged(int state);
  void singleCheckBoxChanged(int state);
  void mtCheckBoxChanged(int state);
  void somaCheckBoxChanged(int state);
  void pyramidalCheckBoxChanged(int state);
  void decomposeCheckBoxChanged(int state);
  void apoCheckBoxChanged(int state);
  void lengthCheckBoxChanged(int state);
  void shuffleCheckBoxChanged(int state);
  void pcountCheckBoxChanged(int state);
  void colorCheckBoxChanged(int state);
  void signalCheckBoxChanged(int state);
  void sigmarginCheckBoxChanged(int state);
  void infoCheckBoxChanged(int state);
  void analysisCheckBoxChanged(int state);
  void featFileCheckBoxChanged(int state);

private:
  void createInputGroupBox();
  void createOutputGroupBox();
  void createOperationGroupBox();
  void createLabels();
  QDoubleSpinBox* createDoubleSpinBox();
  QSpinBox* createIntSpinBox();


private:
  QGroupBox *m_inputGroupBox;
  QGroupBox *m_outputGroupBox;
  QGroupBox *m_operationGroupBox;
  QDialogButtonBox *m_buttonBox;

  QLineEdit *m_inputFileEdit;
  QLineEdit *m_outputFileEdit;
  QLineEdit *m_subtractFileEdit;
  QLineEdit *m_searchFileEdit;
  QLineEdit *m_apoFileEdit;
  QLineEdit *m_glueFileEdit;
  QLineEdit *m_cutLineEdit;
  QLineEdit *m_colorCodeTifFileEdit;
  QLineEdit *m_signalTifFileEdit;
  //QCheckBox *m_openOutputFileCheckBox;
  QDoubleSpinBox *m_searchPosXSpinBox;
  QDoubleSpinBox *m_searchPosYSpinBox;
  QDoubleSpinBox *m_searchPosZSpinBox;
  QDoubleSpinBox *m_rootXSpinBox;
  QDoubleSpinBox *m_rootYSpinBox;
  QDoubleSpinBox *m_rootZSpinBox;
  QDoubleSpinBox *m_translateXSpinBox;
  QDoubleSpinBox *m_translateYSpinBox;
  QDoubleSpinBox *m_translateZSpinBox;
  QDoubleSpinBox *m_resizeXScaleSpinBox;
  QDoubleSpinBox *m_resizeYScaleSpinBox;
  QDoubleSpinBox *m_resizeZScaleSpinBox;
  QDoubleSpinBox *m_cleanSmallThresholdSpinBox;
  QDoubleSpinBox *m_mergeThresholdSpinBox;
  QDoubleSpinBox *m_signalMarginSpinBox;
  QSpinBox *m_typeSpinBox;
  QSpinBox *m_rangeBeginSpinBox;
  QSpinBox *m_rangeEndSpinBox;
  QSpinBox *m_mainTrunkTypeSpinBox;
  QSpinBox *m_somaTypeSpinBox;
  QComboBox *m_colorComboBox;
  QComboBox *m_infoComboBox;

  //QString inputFileName;
  //QString outputFileName;
  QStringList m_subtractOtherSwcFileNames;
  QStringList m_searchOtherSwcFileNames;
  QStringList m_glueOtherSwcFileNames;
  QStringList m_apoFileNames;
  QString m_openFilesPath;
  QString m_colorCodeMode;
  QString m_colorCodeTifFileName;
  QString m_signalTifFileName;
  QString m_info;

  QRadioButton *m_useOtherFileButton;
  QRadioButton *m_useCurrentSwcButton;
  QToolButton *m_selectInputFileButton;
  QToolButton *m_selectOutputFileButton;
  QToolButton *m_selectSubtractFileButton;
  QToolButton *m_selectSearchFileButton;
  QToolButton *m_selectGlueFileButton;
  QToolButton *m_selectApoFileButton;
  QToolButton *m_selectColorCodeTifFileButton;
  QToolButton *m_selectSignalTifFileButton;
  QPushButton *m_runButton;
  QPushButton *m_exitButton;

  QCheckBox *m_subtractCheckBox;
  QCheckBox *m_searchCheckBox;
  QCheckBox *m_glueCheckBox;
  QCheckBox *m_cutCheckBox;
  QCheckBox *m_rootCheckBox;
  QCheckBox *m_cleanRootCheckBox;
  QCheckBox *m_removeOvershootCheckBox;
  QCheckBox *m_tuneForkCheckBox;
  QCheckBox *m_typeCheckBox;
  QCheckBox *m_rangeCheckBox;
  QCheckBox *m_translateCheckBox;
  QCheckBox *m_cleanSmallCheckBox;
  QCheckBox *m_resizeCheckBox;
  QCheckBox *m_mergeCheckBox;
  QCheckBox *m_singleCheckBox;
  QCheckBox *m_mtCheckBox;
  QCheckBox *m_somaCheckBox;
  QCheckBox *m_pyramidalCheckBox;
  QCheckBox *m_decomposeCheckBox;
  QCheckBox *m_apoCheckBox;
  QCheckBox *m_lengthCheckBox;
  QCheckBox *m_shuffleCheckBox;
  QCheckBox *m_pcountCheckBox;
  QCheckBox *m_colorCheckBox;
  QCheckBox *m_signalCheckBox;
  QCheckBox *m_sigmarginCheckBox;
  QCheckBox *m_infoCheckBox;
  QCheckBox *m_analysisCheckBox;
  QCheckBox *m_featFileCheckBox;

  QList<ZSwcTree*> *m_swcTreeList;

};

#endif // ZEDITSWCDIALOG_H
