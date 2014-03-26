#include "zswccolorscheme.h"
#include <QDebug>

ZSwcColorScheme::ZSwcColorScheme()
{
}

QColor ZSwcColorScheme::getColor(int index) const
{
  QColor color;

  if (m_colorTable.isEmpty()) {
    color = QColor(0, 0, 0);
  } else {
    color = m_colorTable[index % m_colorTable.size()];
  }

  //qDebug() << color;

  return color;
}

void ZSwcColorScheme::buildVaa3dColorTable()
{
  m_colorTable.resize(19);
  m_colorTable[0].setRgb(255, 255, 255);
  m_colorTable[1].setRgb(20, 20, 20);
  m_colorTable[2].setRgb(200, 20, 0); //red
  m_colorTable[3].setRgb(0, 20, 200); //blue
  m_colorTable[4].setRgb(200, 0, 200); //purple
  m_colorTable[5].setRgb(0, 200, 200); //cyan
  m_colorTable[6].setRgb(220, 200, 0); //yellow
  m_colorTable[7].setRgb(0, 200, 20); //green
  m_colorTable[8].setRgb(188, 94, 37); //coffee
  m_colorTable[9].setRgb(180, 200, 120); //asparagus
  m_colorTable[10].setRgb(250, 100, 120); //salmon
  m_colorTable[11].setRgb(120, 200, 200); //ice
  m_colorTable[12].setRgb(100, 120, 200); //orchid
  m_colorTable[13].setRgb(255, 128, 168);
  m_colorTable[14].setRgb(128, 255, 168);
  m_colorTable[16].setRgb(128, 168, 255);
  m_colorTable[17].setRgb(128, 255, 168);
  m_colorTable[18].setRgb(255, 168, 128);
  m_colorTable[19].setRgb(168, 128, 255);
  m_colorTable[20].setRgb(0xcc, 0xcc, 0xcc);
}

void ZSwcColorScheme::buildBiocytinColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(QColor(200, 20, 0)); //red
  m_colorTable.push_back(QColor(0, 20, 200)); //blue
  m_colorTable.push_back(QColor(200, 0, 200)); //purple
  m_colorTable.push_back(QColor(0, 200, 200)); //cyan
  m_colorTable.push_back(QColor(220, 200, 0)); //yellow
  m_colorTable.push_back(QColor(0, 200, 20)); //green
  m_colorTable.push_back(QColor(188, 94, 37)); //coffee
  //m_colorTable.push_back(QColor(180, 200, 120)); //asparagus
  //m_colorTable.push_back(QColor(250, 100, 120)); //salmon
  //m_colorTable.push_back(QColor(120, 200, 200)); //ice
  //m_colorTable.push_back(QColor(100, 120, 200)); //orchid
}

void ZSwcColorScheme::buildUniqueColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(QColor(Qt::red));
  m_colorTable.push_back(QColor(Qt::green));
  m_colorTable.push_back(QColor(Qt::blue));
  m_colorTable.push_back(QColor(Qt::cyan));
  m_colorTable.push_back(QColor(Qt::magenta));
  m_colorTable.push_back(QColor(Qt::darkRed));
  m_colorTable.push_back(QColor(Qt::darkGreen));
  m_colorTable.push_back(QColor(Qt::darkBlue));
  m_colorTable.push_back(QColor(Qt::darkCyan));
  m_colorTable.push_back(QColor(Qt::darkMagenta));
  m_colorTable.push_back(QColor(Qt::darkYellow));
  m_colorTable.push_back(QColor(Qt::white));
  m_colorTable.push_back(QColor(Qt::black));
}

void ZSwcColorScheme::setColorScheme(EColorScheme scheme)
{
  if (scheme != m_colorScheme) {
    m_colorScheme = scheme;
    switch (scheme) {
    case VAA3D_TYPE_COLOR:
      buildVaa3dColorTable();
      break;
    case BIOCYTIN_TYPE_COLOR:
      buildBiocytinColorTable();
      break;
    case UNIQUE_COLOR:
      buildUniqueColorTable();
      break;
    case JIN_TYPE_COLOR:
      buildJinTypeColorTable();
      break;
    }
  }
}

void ZSwcColorScheme::buildJinTypeColorTable()
{
  m_colorTable.clear();
  m_colorTable.push_back(QColor(Qt::white));
  m_colorTable.push_back(QColor(Qt::blue));
  m_colorTable.push_back(QColor(Qt::black));
  m_colorTable.push_back(QColor(Qt::green));
  m_colorTable.push_back(QColor(Qt::red));
  m_colorTable.push_back(QColor(Qt::gray));
}
