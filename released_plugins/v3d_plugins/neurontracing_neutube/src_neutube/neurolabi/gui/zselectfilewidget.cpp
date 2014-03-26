#include "zselectfilewidget.h"

#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

ZSelectFileWidget::ZSelectFileWidget(FileMode mode, const QString &guiname, const QString& filter, QBoxLayout::Direction direction,
                                     QWidget *parent)
  : QWidget(parent)
  , m_fileMode(mode)
  , m_guiName(guiname)
  , m_filter(filter)
  , m_destName(NULL)
  , m_destNames(NULL)
  , m_lessThan(NULL)
{
  createWidget(direction);
}

void ZSelectFileWidget::setDestination(QString *name)
{
  if (m_fileMode != OPEN_MULTIPLE_FILES && m_fileMode != OPEN_MULTIPLE_FILES_WITH_FILTER)
    m_destName = name;
}

void ZSelectFileWidget::setDestination(QStringList *namelist)
{
  if (m_fileMode == OPEN_MULTIPLE_FILES || m_fileMode == OPEN_MULTIPLE_FILES_WITH_FILTER)
    m_destNames = namelist;
}

void ZSelectFileWidget::setCompareFunc(bool (*lessThan)(const QString &, const QString &))
{
  m_lessThan = lessThan;
}

QString ZSelectFileWidget::getSelectedOpenFile()
{
  if (m_fileMode == OPEN_SINGLE_FILE)
    return m_lineEdit->text();
  else
    return QString();
}

QString ZSelectFileWidget::getSelectedSaveFile()
{
  if (m_fileMode == SAVE_FILE)
    return m_lineEdit->text();
  else
    return QString();
}

QStringList ZSelectFileWidget::getSelectedMultipleOpenFiles()
{
  if (m_fileMode == OPEN_MULTIPLE_FILES)
    return m_multipleFNames;
  else if (m_fileMode == OPEN_MULTIPLE_FILES_WITH_FILTER) {
    QRegExp regExp(m_filterLineEdit->text(), Qt::CaseInsensitive, QRegExp::Wildcard);
    return m_multipleFNames.filter(regExp);
  }
  else
    return QStringList();
}

QString ZSelectFileWidget::getSelectedDirectory()
{
  if (m_fileMode == DIRECTORY)
    return m_lineEdit->text();
  else
    return QString();
}

void ZSelectFileWidget::setFile(const QString &fn)
{
  if (m_fileMode != OPEN_MULTIPLE_FILES && m_fileMode != OPEN_MULTIPLE_FILES_WITH_FILTER
      && m_lineEdit->text() != fn) {
    m_lineEdit->setText(fn);
    emit changed();
  }
}

void ZSelectFileWidget::setFiles(const QStringList &fl)
{
  if (m_fileMode == OPEN_MULTIPLE_FILES || m_fileMode == OPEN_MULTIPLE_FILES_WITH_FILTER) {
    m_multipleFNames = fl;
    m_textEdit->setText(QString("%1").arg(fl.join("\n")));
    emit changed();
  }
}

void ZSelectFileWidget::createWidget(QBoxLayout::Direction direction)
{
  if (m_fileMode == OPEN_MULTIPLE_FILES) {
    m_layout = new QBoxLayout(direction, this);
    m_layout->setContentsMargins(0,0,0,0);
    m_selectPushButton = new QPushButton(m_guiName, this);
    connect(m_selectPushButton, SIGNAL(clicked()), this, SLOT(selectFile()));
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_layout->addWidget(m_selectPushButton);
    m_layout->addWidget(m_textEdit);
  } else if (m_fileMode == OPEN_MULTIPLE_FILES_WITH_FILTER) {
    QBoxLayout *lo = new QBoxLayout(direction);
    lo->setContentsMargins(0,0,0,0);
    m_layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    m_layout->setContentsMargins(0,0,0,0);
    m_selectPushButton = new QPushButton(m_guiName, this);
    connect(m_selectPushButton, SIGNAL(clicked()), this, SLOT(selectFile()));
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    lo->addWidget(m_selectPushButton);
    lo->addWidget(m_textEdit);
    m_layout->addLayout(lo);
    m_label = new QLabel("Filter:", this);
    m_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_filterLineEdit = new QLineEdit(this);
    m_filterLineEdit->setReadOnly(false);
    m_filterLineEdit->setText("*");
    m_previewButton = new QPushButton("Preview", this);
    connect(m_filterLineEdit, SIGNAL(returnPressed()), this, SLOT(previewFilterResult()));
    connect(m_previewButton, SIGNAL(clicked()), this, SLOT(previewFilterResult()));
    lo = new QBoxLayout(QBoxLayout::LeftToRight);
    lo->setContentsMargins(40, 0, 0, 40);
    lo->addWidget(m_label);
    lo->addWidget(m_filterLineEdit);
    lo->addWidget(m_previewButton);
    m_layout->addLayout(lo);
  } else {
    m_layout = new QBoxLayout(direction, this);
    m_layout->setContentsMargins(0,0,0,0);
    m_label = new QLabel(m_guiName, this);
    m_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setReadOnly(true);
    m_button = new QToolButton(this);
    m_button->setText(tr("..."));
    connect(m_button, SIGNAL(clicked()), this, SLOT(selectFile()));
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_lineEdit);
    m_layout->addWidget(m_button);
  }
}

void ZSelectFileWidget::selectFile()
{
  if (m_fileMode == OPEN_MULTIPLE_FILES || m_fileMode == OPEN_MULTIPLE_FILES_WITH_FILTER) {
    QStringList tmp;
    tmp = QFileDialog::getOpenFileNames(this, m_guiName, m_lastFName, m_filter,
                                        NULL);
    if (!tmp.isEmpty()) {
      m_lastFName = tmp[0];
      m_multipleFNames.clear();
      m_multipleFNames = tmp;
      if (m_lessThan)
        qSort(m_multipleFNames.begin(), m_multipleFNames.end(), m_lessThan);
      else
        qSort(m_multipleFNames.begin(), m_multipleFNames.end());
      m_textEdit->setText(QString("%1").arg(m_multipleFNames.join("\n")));
      if (m_destNames)
        *m_destNames = m_multipleFNames;
      emit changed();
    }
  } else if (m_fileMode == OPEN_SINGLE_FILE) {
    QString fileName = QFileDialog::getOpenFileName(
          this, m_guiName, m_lastFName, m_filter,
          NULL);
    if (!fileName.isEmpty()) {
      m_lastFName = fileName;
      m_lineEdit->setText(fileName);
      if (m_destName)
        *m_destName = fileName;
      emit changed();
    }
  } else if (m_fileMode == SAVE_FILE) {
    QString outputFileName = QFileDialog::getSaveFileName(
          this, m_guiName, m_lastFName, m_filter,
          NULL);
    if (!outputFileName.isEmpty()) {
      m_lastFName = outputFileName;
      m_lineEdit->setText(outputFileName);
      if (m_destName)
        *m_destName = outputFileName;
      emit changed();
    }
  } else if (m_fileMode == DIRECTORY) {
    QString dir = QFileDialog::getExistingDirectory(
          this, m_guiName, m_lastFName);
    if (!dir.isEmpty()) {
      m_lastFName = dir;
      m_lineEdit->setText(dir);
      if (m_destName)
        *m_destName = dir;
      emit changed();
    }
  }
}

void ZSelectFileWidget::previewFilterResult()
{
  setFiles(getSelectedMultipleOpenFiles());
}
