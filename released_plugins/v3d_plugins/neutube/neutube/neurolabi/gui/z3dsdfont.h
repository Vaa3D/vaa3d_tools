#ifndef Z3DSDFONT_H
#define Z3DSDFONT_H

#include <QString>
#include <QImage>
#include "z3dtexture.h"

class Z3DSDFont
{
public:
  struct CharInfo
  {
    CharInfo(int id = 0, int x = 0, int y = 0, int width = 0, int height = 0, float xoffset = 0.f, float yoffset = 0.f,
             float xadvance = 0.f, int page = 0, int chnl = 0, int texWidth = 1, int texHeight = 1) :
      id(id), x(x), y(y), width(width), height(height), xoffset(xoffset),
      yoffset(yoffset), xadvance(xadvance), page(page), chnl(chnl)
    {
      sMin = static_cast<float>(x)/static_cast<float>(texWidth);
      tMin = static_cast<float>(y+height)/static_cast<float>(texHeight);

      sMax = static_cast<float>(x+width)/static_cast<float>(texWidth);
      tMax = static_cast<float>(y)/static_cast<float>(texHeight);
    }

    int id;
    int x;
    int y;
    int width;
    int height;
    float xoffset;
    float yoffset;
    float xadvance;
    int page;
    int chnl;

    float sMin;
    float sMax;
    float tMin;
    float tMax;
  };

  Z3DSDFont(const QString &imageFileName, const QString &txtFileName);
  ~Z3DSDFont();

  inline QString getFontName() const { return m_fontName; }
  inline int getMaxFontHeight() const { return m_maxFontHeight; }
  bool isEmpty() const { return m_isEmpty; }

  CharInfo getCharInfo(int id) const;
  Z3DTexture* getTexture();

protected:

  void loadImage();
  void parseFontFile();

  void createTexture();

private:
  QString m_imageFileName;
  QImage m_GLFormattedImage;
  QString m_txtFileName;

  QString m_fontName;
  bool m_isEmpty;   //if load image or txt failed, the font is empty
  QList<CharInfo> m_charInfos;
  int m_maxFontHeight;

  Z3DTexture* m_texture;
};

#endif // Z3DSDFONT_H
