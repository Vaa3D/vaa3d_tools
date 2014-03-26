#include "z3dsdfont.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QGLWidget>
#include "QsLog.h"

Z3DSDFont::Z3DSDFont(const QString &imageFileName, const QString &txtFileName)
  : m_imageFileName(imageFileName)
  , m_txtFileName(txtFileName)
  , m_isEmpty(false)
  , m_maxFontHeight(0)
  , m_texture(NULL)
{
  loadImage();
  parseFontFile();
}

Z3DSDFont::~Z3DSDFont()
{
  delete m_texture;
}

Z3DSDFont::CharInfo Z3DSDFont::getCharInfo(int id) const
{
  CharInfo space;
  for (int i=0; i<m_charInfos.size(); i++) {
    if (m_charInfos[i].id == id)
      return m_charInfos[i];
    else if (m_charInfos[i].id == 32)
      space = m_charInfos[i];
  }
  return space;
}

Z3DTexture *Z3DSDFont::getTexture()
{
  if (m_isEmpty)
    return NULL;
  if (!m_texture)
    createTexture();
  return m_texture;
}

void Z3DSDFont::loadImage()
{
  if(!m_GLFormattedImage.load(m_imageFileName)) {
    LERROR() << QString("error loading image: %1").arg(m_imageFileName);
    m_isEmpty = true;
    return;
  }
}

void Z3DSDFont::parseFontFile()
{
  if (m_isEmpty)
    return;
  m_charInfos.clear();
  QFile qFile(m_txtFileName);
  if (!qFile.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QTextStream stream(&qFile);
  int numCharFromFile = 0;
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    line = line.trimmed();
    if (line.startsWith("info face=")) {
      m_fontName = line.section('\"', 1, 1);
      continue;
    }
    if (line.startsWith("chars count=")) {
      bool ok;
      numCharFromFile = line.section('=', 1, 1).toInt(&ok);
      if (ok)
        continue;
      else {
        LERROR() << "char count can not be converted to int, wrong file maybe, abort";
        m_isEmpty = true;
        return;
      }
    }
    //char id=32    x=208   y=249   width=4     height=4     xoffset=-1.500    yoffset=1.500     xadvance=15.625      page=0  chnl=0
    if (line.startsWith("char ")) {
      line = line.mid(5).trimmed();
      int	id, x, y, width, height;
      float	xoffset, yoffset, xadvance;
      int	page, chnl;
      QStringList tokens;
      tokens.push_back("id=");
      tokens.push_back("x=");
      tokens.push_back("y=");
      tokens.push_back("width=");
      tokens.push_back("height=");
      tokens.push_back("xoffset=");
      tokens.push_back("yoffset=");
      tokens.push_back("xadvance=");
      tokens.push_back("page=");
      tokens.push_back("chnl=");
      int numTokenFound = 0;
      while (!line.isEmpty()) {
        line = line.trimmed();
        int tokenIndex = -1;
        QString value;
        for (int i=0; i<tokens.size(); i++) {
          if (line.startsWith(tokens[i])) {
            if (line.indexOf(' ') == -1) {
              value = line.mid(tokens[i].size());
              line.clear();
            }
            else {
              value = line.mid(tokens[i].size(), line.indexOf(' ') - tokens[i].size());
              line = line.mid(line.indexOf(' '));
            }
            tokenIndex = i;
            break;
          }
        }
        if (tokenIndex != -1) {
          numTokenFound++;
          bool ok = false;
          if (tokenIndex == 0)
            id = value.toInt(&ok);
          else if (tokenIndex == 1)
            x = value.toInt(&ok);
          else if (tokenIndex == 2)
            y = value.toInt(&ok);
          else if (tokenIndex == 3)
            width = value.toInt(&ok);
          else if (tokenIndex == 4)
            height = value.toInt(&ok);
          else if (tokenIndex == 5)
            xoffset = value.toFloat(&ok);
          else if (tokenIndex == 6)
            yoffset = value.toFloat(&ok);
          else if (tokenIndex == 7)
            xadvance = value.toFloat(&ok);
          else if (tokenIndex == 8)
            page = value.toInt(&ok);
          else if (tokenIndex == 9)
            chnl = value.toInt(&ok);
          if (!ok) {
            LERROR() << "some number convertion error, abort";
            m_charInfos.clear();
            m_isEmpty = true;
            return;
          }
          continue;
        } else {
          LERROR() << "found unknown token, wrong file, abort";
          m_charInfos.clear();
          m_isEmpty = true;
          return;
        }
      }
      if (numTokenFound == tokens.size()) {
        m_charInfos.push_back(CharInfo(id, x, y, width, height,
                                       xoffset, yoffset, xadvance,
                                       page, chnl,
                                       m_GLFormattedImage.width(),
                                       m_GLFormattedImage.height()));
        m_maxFontHeight = std::max(m_maxFontHeight, height);
      } else {
        LERROR() << "some tokens are missing, abort";
        m_charInfos.clear();
        m_isEmpty = true;
        return;
      }
    }
  }
  if (m_charInfos.size() != numCharFromFile) {
    LERROR() << "font char count dont match with txt file, abort";
    m_charInfos.clear();
    m_maxFontHeight = 0;
    m_isEmpty = true;
    return;
  }
}

void Z3DSDFont::createTexture()
{
  if (m_isEmpty || m_texture != 0)
    return;
  m_texture = new Z3DTexture(glm::ivec3(m_GLFormattedImage.width(), m_GLFormattedImage.height(), 1),
                             GL_BGRA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_LINEAR, GL_LINEAR,
                             GL_REPEAT);
  m_texture->setData(m_GLFormattedImage.bits());
  m_texture->uploadTexture();
  CHECK_GL_ERROR;
}
