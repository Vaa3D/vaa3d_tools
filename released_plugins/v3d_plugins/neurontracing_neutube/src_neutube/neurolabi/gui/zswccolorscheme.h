#ifndef ZSWCCOLORSCHEME_H
#define ZSWCCOLORSCHEME_H

#include <QColor>
#include <QVector>

class ZSwcColorScheme
{
public:
  ZSwcColorScheme();

  enum EColorScheme {
    VAA3D_TYPE_COLOR, BIOCYTIN_TYPE_COLOR, JIN_TYPE_COLOR, UNIQUE_COLOR
  };

  QColor getColor(int index) const;
  int getColorNumber() const { return m_colorTable.size(); }

  void setColorScheme(EColorScheme scheme);

private:
  void buildVaa3dColorTable();
  void buildBiocytinColorTable();
  void buildUniqueColorTable();
  void buildJinTypeColorTable();

private:
  EColorScheme m_colorScheme;
  QVector<QColor> m_colorTable;
};

#endif // ZSWCCOLORSCHEME_H
