#include "ztraceproject.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include "tz_xml_utils.h"
#include "tz_utilities.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "zstack.hxx"
#include "tz_string.h"
#include "zfiletype.h"

ZTraceProject::ZTraceProject(ZStackFrame *parent)
{
  m_parent = parent;
}

ZTraceProject::~ZTraceProject()
{

}

int ZTraceProject::load(const QString path)
{
  return m_parent->loadTraceProject((path
                                     + m_parent->defaultTraceProjectFile())
                                    .toLocal8Bit().constData());
}

void ZTraceProject::save()
{
  QString filePath = m_projFilePath;

  QDir workDir(rootPath());

  if (!workDir.exists()) {
    workDir.mkpath(rootPath());
  }

  QDir tubeDir(m_workspacePath);

  if (tubeDir.exists()) {
    QDir bkTubeDir(rootPath() + "/" + "~" + tubeDir.dirName());
    if (bkTubeDir.exists()) {
      rmpath(bkTubeDir.absolutePath().toLocal8Bit().constData());
    }
    tubeDir.rename(tubeDir.absolutePath(), bkTubeDir.absolutePath());
  }

  workDir.mkdir(tubeFolder());

  qDebug() << filePath;

  QFile file(filePath);
  file.open(QIODevice::WriteOnly);

  QTextStream out(&file);

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<trace version = \"1.0\">" << endl;
  out << "<data>" << endl;
  out << "<image type=";
  ZStackFile *stackSource = m_parent->document()->stack()->source();

  //Stack_Document *stackSource = m_parent->document()->stack()->source();

  switch (stackSource->type()) {
  case ZStackFile::SINGLE_FILE:
  {
    switch (ZFileType::fileType(stackSource->firstUrl())) {
    case ZFileType::TIFF_FILE:
      out << "\"tif\">" << endl;
      out << "<url>" << stackSource->firstUrl().c_str() << "</url>" << endl;
      break;
    case ZFileType::LSM_FILE:
    //STACK_DOC_LSM_FILE:
      out << "\"lsm\">" << endl;
      out << "<url>" << stackSource->firstUrl().c_str() << "</url>" << endl;;
      break;
    case ZFileType::V3D_RAW_FILE:
      out << "\"raw\">" << endl;
      out << "<url>" << stackSource->firstUrl().c_str() << "</url>" << endl;;
      break;
    default:
      out << "unknown>" << stackSource->firstUrl().c_str() << endl;
    }
  }
    break;
  case ZStackFile::FILE_BUNDLE:
  {
    out << "\"bundle\">" << endl;
    //File_Bundle_S *fb = (File_Bundle_S*) stackSource->ci;
    out << "<prefix>" << stackSource->prefix().c_str() << "</prefix>" << endl;
    out << "<suffix>" << stackSource->suffix().c_str() << "</suffix" << endl;
    out << "<num_width>" << stackSource->numWidth() << "</num_width>" << endl;
    out << "<first_num>" << stackSource->firstNum() << "</first_num>" << endl;
  }
    break;
  default:
    out << "unknown>" << stackSource->firstUrl().c_str() << endl;
  }

  out << "</image>" << endl;

  ZResolution resolution = m_parent->document()->stack()->resolution();
  const double *voxelSize = resolution.voxelSize();
  out << "<resolution>" << "<x>" << voxelSize[0] << "</x>"
      << "<y>" << voxelSize[1] << "</y>"
      << "<z>" << voxelSize[2] << "</z>"
      << "</resolution>" << endl;
  out << "<unit>" << resolution.unit() << "</unit>" << endl;
  out << "<channel>" << stackSource->channel() << "</channel>" << endl;

  out << "</data>" << endl;

  out << "<output>" << endl;
  out << "<workdir>" << m_workspacePath << "</workdir>" << endl;
  if (m_parent->document()->tubePrefix() != NULL) {
    out << "<tube>" << m_parent->document()->tubePrefix()
        << "</tube>" << endl;
  }
  out << "</output>" << endl;

  if (!m_decorationPath.isEmpty()) {
    out << "<object>" << endl;
    for (int i = 0; i < m_decorationPath.length(); i++) {
      out << "<" << m_decorationTag.at(i) << ">";
      out << m_decorationPath.at(i);
      out << "</" << m_decorationTag.at(i) << ">" << endl;
    }
    out << "</object>" << endl;
  }

  out << "</trace>" << endl;

  file.close();

  m_parent->document()
      ->exportBinary((m_workspacePath + "/"
                      + m_parent->document()->tubePrefix()).toLocal8Bit().constData());
}

void ZTraceProject::saveAs(QString workspacePath)
{
  m_workspacePath = workspacePath + "/traced";
  m_projFilePath = workspacePath + "/project.xml";
  save();
}

void ZTraceProject::setWorkDir(const char *workDir)
{
  m_workspacePath = QDir(workDir).canonicalPath();
}

void ZTraceProject::setProjFilePath(const char *path)
{
  m_projFilePath = path;
}

void ZTraceProject::addDecoration(const QString &path, const QString &tag)
{
  m_decorationPath.append(path);
  m_decorationTag.append(tag);
}

QString ZTraceProject::rootPath() const
{
  return QDir(m_workspacePath + "/../").absolutePath();
}

QString ZTraceProject::tubeFolder() const
{
  return QDir(m_workspacePath).dirName();
}
