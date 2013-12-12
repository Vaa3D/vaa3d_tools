#include <QtGui>
#include <iostream>
#include "tz_error.h"
#include "zimage.h"
#include "tz_stack_neighborhood.h"

ZImage::ZImage() : QImage()
{
}

ZImage::ZImage(int width, int height, QImage::Format format) :
  QImage(width, height, format)
{
  memset(scanLine(0), 255, this->bytesPerLine() * height);
  /*
  int i, j;
  for (j = 0; j < height; j++) {
    char *line = (char*) scanLine(j);
    for (i = 0; i < width; i++) {
      line[3] = '\xff';
      line += 4;
    }
  }*/

}

void ZImage::setData(const uint8 *data, int threshold)
{
  if (threshold < 0) {
    int i, j;
    int w = width();
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < w; i++) {
        memset(line, *data++, 3);
        line += 3;
        *line++ = '\xff';
      }
    }
  } else {
    int i, j;
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        if (*data > threshold) {
          *line++ = '\0';
          *line++ = '\0';
          *line++ = '\xff';
          ++data;
        } else {
          *line++ = *data;
          *line++ = *data;
          *line++ = *data++;
        }
        *line++ = '\xff';
      }
    }
  }
}

void ZImage::setData(const color_t *data, int alpha)
{
  int i, j;
  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      *line++ = (*data)[2];
      *line++ = (*data)[1];
      *line++ = (*data)[0];
      data++;
      //*line++ = '\xff';
      *line++ = alpha;
    }
  }
}

void ZImage::setCData(const color_t *data, double scale, double offset)
{
  int i, j;
  if (scale == 1.0 && offset == 0.0) {
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        *line++ = (*data)[2];
        *line++ = (*data)[1];
        *line++ = (*data)[0];
        data++;
        //*line++ = '\xff';
        line++;
      }
    }
  } else {
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        double value = scale * (*data)[2] + offset;
        uint8 v;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        value = scale * (*data)[1] + offset;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        value = scale * (*data)[0] + offset;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        data++;
        //*line++ = '\xff';
        line++;
      }
    }
  }
}

void ZImage::setData(const uint8 *data, double scale, double offset,
                     int threshold)
{
#ifdef _DEBUG_2
  tic();
#endif
  if (scale == 1.0 && offset == 0.0) {
    if (threshold < 0) {
      int i, j;
      int w = width();
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < w; i++) {
          memset(line, *data++, 3);
          line += 3;
          *line++ = '\xff';
        }
      }
    } else {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            ++data;
          } else {
            *line++ = *data;
            *line++ = *data;
            *line++ = *data++;
          }
          *line++ = '\xff';
        }
      }
    }
  } else {
    uint8 valueMap[256];
    for (int i = 0; i < 256; ++i) {
      double value = scale * i + offset;
      if (value <= 0.0) {
        valueMap[i] = '\0';
      } else if (value >= 255.0) {
        valueMap[i] = '\xff';
      } else {
        valueMap[i] = (uint8) value;
      }
    }

    if (threshold < 0) {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          //memset(line, valueMap[*data++], 3);
          uint8 v = valueMap[*data++];
          *line++ = v;
          *line++ = v;
          *line++ = v;
          line++;
        }
      }
    } else {
      int i, j;
      for (j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            ++data;
          } else {
            uint8 v = valueMap[*data++];
            *line++ = v;
            *line++ = v;
            *line++ = v;
          }

          *line++ = '\xff';
        }
      }
    }
  }
#ifdef _DEBUG_2
  std::cout << toc() << std::endl;
#endif
}


void ZImage::setCData(const uint16_t *data, uint8_t alpha)
{
  int i, j;

  //uint8_t *data8 = (uint8_t*) data;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      int v = *(data++) * 127;
      *line++ = v % 255;
      v /= 255;
      *line++ = v % 255;
      *line++ = v / 255;
      *line++ = alpha;
    }
  }
}

void ZImage::setCData(const uint8_t *data, uint8_t alpha)
{
  int i, j;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      *line++ = *data;
      *line++ = *data;
      *line++ = *data++;
      *line++ = alpha;
    }
  }
}

void ZImage::drawRaster(const void *data, int kind, double scale,
			double offset, int threshold)
{
  Image_Array ima;
  ima.array = (uint8*) data;

  switch (kind) {
  case GREY:
    setData(ima.array8, threshold);
    break;
  case GREY16:
    setData(ima.array16, scale, offset, threshold);
    break;
  case FLOAT32:
    setData(ima.array32, scale, offset);
    break;
  case FLOAT64:
    setData(ima.array64, scale, offset);
    break;
  case COLOR:
    setData(ima.arrayc);
    break;
  default:
    PRINT_EXCEPTION("Unknown data type", "The kind of data is not recognized");
    throw std::exception();
  }
}

void ZImage::setBackground()
{
  int w = this->bytesPerLine();
  int h = height();
  memset_pattern4(scanLine(0), "\xef\xef\xef\xff", w * h);
}

ZImage* ZImage::createMask()
{
  ZImage *mask = new ZImage(width(), height(),
                            QImage::Format_ARGB32_Premultiplied);
  mask->fill(0);

  return mask;
}

bool ZImage::hasSameColor(uchar *pt1, uchar *pt2)
{
  return (pt1[0] == pt2[0]) && (pt1[1] == pt2[1]) && (pt1[2] == pt2[2]);
}

static void stack_neighbor_offset(int n_nbr, int width, int height,
                                  int neighbor[])
{
  switch (n_nbr) {
  case 4:
    neighbor[0] = -4;
    neighbor[1] = -neighbor[0];
    neighbor[2] = -width;
    neighbor[3] = -neighbor[2];
    break;
  case 8:
    stack_neighbor_offset(4, width, height, neighbor);
    neighbor[4] = -width - 4;
    neighbor[5] = -neighbor[4];
    neighbor[6] = -width + 4;
    neighbor[7] = -neighbor[6];
  default:
    break;
  }
}

void ZImage::enhanceEdge()
{
  int i, j;
  int neighborOffset[26];
  int isInBound[26];
  int nnbr = 8;
  stack_neighbor_offset(nnbr, bytesPerLine(), height(), neighborOffset);

  uchar *edge = new uchar[width() * height()];
  uchar *edgeIterator = edge;

  int cwidth = width() - 1;
  int cheight = height() - 1;

  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      int n = Stack_Neighbor_Bound_Test_S(nnbr, cwidth, cheight, 1, i, j, 0,
                                          isInBound);
      *edgeIterator = 0;
      if (n != nnbr) {
        for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
          if (isInBound[neighborIndex]) {
            if (!hasSameColor(line, line + neighborOffset[neighborIndex])) {
              *edgeIterator = 1;
              break;
            }
          }
        }
      } else {
        for (int neighborIndex = 0; neighborIndex < nnbr; ++neighborIndex) {
          if (!hasSameColor(line, line + neighborOffset[neighborIndex])) {
            *edgeIterator = 1;
            break;
          }
        }
      }
      line += 4;
      edgeIterator++;
    }
  }

  edgeIterator = edge;
  for (j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (i = 0; i < width(); i++) {
      if (*edgeIterator++) {
        *line++ = 255;
        *line++ = 0;
        *line++ = 0;
        *line++ = 127;
      } else {
        *line++ = 0;
        *line++ = 0;
        *line++ = 0;
        *line++ = 0;
      }
    }
  }

  delete []edge;
}

void ZImage::setData8(const ZImage::DataSource<uint8_t> &source,
                     int threshold, bool useMultithread)
{
  if (source.color == glm::vec3(1.f,1.f,1.f)) {
    setData(source.data, source.scale, source.offset, threshold);
    return;
  }

  if (useMultithread) {
    int numBlock = std::min(height(), 16);
    int blockHeight = height() / numBlock;
    std::vector<QFuture<void> > res(numBlock);
    for (int i=0; i<numBlock; ++i) {
      int startLine = i * blockHeight;
      int endLine = (i+1) * blockHeight;
      if (i == numBlock - 1)
        endLine = height();
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlock<uint8_t>, source, startLine,
                                 endLine, threshold);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  const uint8_t* data = source.data;
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  glm::col3 colorMap[256];
  for (int i = 0; i < 256; ++i) {
      if (scale == 1.f && offset == 0.f) {
          colorMap[i] = glm::col3(color * (float) i);
      } else {
          colorMap[i] = glm::col3(glm::clamp(color * (scale * i + offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
      }
  }

  if (scale == 1.f && offset == 0.f) {
    if (threshold < 0) {
      for (int j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (int i = 0; i < width(); i++) {
          //glm::col3 col3 = glm::col3(color * (float)(*data++));
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    } else {
      for (int j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (int i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            *line++ = '\xff';
            ++data;
          } else {
            //glm::col3 col3 = glm::col3(color * (float)(*data++));
            glm::col3 col3 = colorMap[*data++];

            *line++ = col3[0];
            *line++ = col3[1];
            *line++ = col3[2];
            *line++ = '\xff';
          }
        }
      }
    }
    return;
  }

  if (threshold < 0) {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
          /*
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));
                                              */
        glm::col3 col3 = colorMap[*data++];

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = '\xff';
      }
    }
  } else {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        if (*data > threshold) {
          *line++ = '\0';
          *line++ = '\0';
          *line++ = '\xff';
          *line++ = '\xff';
          ++data;
        } else {
          glm::col3 col3 = colorMap[*data++];

          /*glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));
                                                */

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    }
  }
}

void ZImage::setDataBlock(const ZImage::DataSource<uint8_t> &source, int startLine,
                          int endLine, int threshold)
{
  const uint8_t* data = source.data + startLine * width();
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  glm::col3 colorMap[256];
  for (int i = 0; i < 256; ++i) {
      if (scale == 1.f && offset == 0.f) {
          colorMap[i] = glm::col3(color * (float) i);
      } else {
          colorMap[i] = glm::col3(glm::clamp(color * (scale * i + offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
      }
  }

  if (scale == 1.f && offset == 0.f) {
    if (threshold < 0) {
      int i, j;
      for (j = startLine; j < endLine; j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          //glm::col3 col3 = glm::col3(color * (float)(*data++));
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    } else {
      int i, j;
      for (j = startLine; j < endLine; j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          if (*data > threshold) {
            *line++ = '\0';
            *line++ = '\0';
            *line++ = '\xff';
            *line++ = '\xff';
            ++data;
          } else {
            //glm::col3 col3 = glm::col3(color * (float)(*data++));
            glm::col3 col3 = colorMap[*data++];

            *line++ = col3[0];
            *line++ = col3[1];
            *line++ = col3[2];
            *line++ = '\xff';
          }
        }
      }
    }
    return;
  }

  if (threshold < 0) {
    int i, j;
    for (j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        /*
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));
                                              */

        glm::col3 col3 = colorMap[*data++];

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = '\xff';
      }
    }
  } else {
    int i, j;
    for (j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        if (*data > threshold) {
          *line++ = '\0';
          *line++ = '\0';
          *line++ = '\xff';
          *line++ = '\xff';
          ++data;
        } else {
          /*
          glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));
                                                */
          glm::col3 col3 = colorMap[*data++];

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    }
  }
}

void ZImage::setData(const std::vector<ZImage::DataSource<uint8_t> > &sources,
                     uint8_t alpha, bool useMultithread)
{
  if (useMultithread) {
    int numBlock = std::min(height(), 16);
    int blockHeight = height() / numBlock;
    std::vector<QFuture<void> > res(numBlock);
    for (int i=0; i<numBlock; ++i) {
      int startLine = i * blockHeight;
      int endLine = (i+1) * blockHeight;
      if (i == numBlock - 1)
        endLine = height();
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlockMS8,
                                 sources, startLine, endLine, alpha);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }


  std::vector<std::vector<glm::col3> > colorMap(allSources.size());
  for (size_t i = 0; i < colorMap.size(); ++i) {
    colorMap[i].resize(256);
  }


  if (!needScaleAndClamp) {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(allSources[0].color *
          (float) i);
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[1][i] = glm::col3(allSources[ch].color * (float) i);
      }
    }

    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
  } else {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(
            glm::clamp(allSources[0].color *
            (allSources[0].scale * i + allSources[0].offset),
          glm::vec3(0.f), glm::vec3(255.f)));
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[ch][i] = glm::col3(glm::clamp(allSources[ch].color *
                                               (allSources[ch].scale * i + allSources[ch].offset),
                                               glm::vec3(0.f), glm::vec3(255.f)));
      }
    }

    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
  }
}

void ZImage::setDataBlockMS8(
    const std::vector<ZImage::DataSource<uint8_t> > &sources,
    int startLine, int endLine, uint8_t alpha)
{
  std::vector<ZImage::DataSource<uint8_t> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    allSources[i].data += startLine * width();
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }

  std::vector<std::vector<glm::col3> > colorMap(allSources.size());
  for (size_t i = 0; i < colorMap.size(); ++i) {
    colorMap[i].resize(256);
  }

  if (!needScaleAndClamp) {
    for (int i = 0; i < 256; ++i) {
      colorMap[0][i] = glm::col3(allSources[0].color *
          (float) i);
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        colorMap[ch][i] = glm::col3(allSources[ch].color * (float) i);
      }
    }

    for (int j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
        for (size_t ch=1; ch<allSources.size(); ++ch) {
          col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
        }

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
    return;
  }

  for (int i = 0; i < 256; ++i) {
    colorMap[0][i] = glm::col3(
          glm::clamp(allSources[0].color *
          (allSources[0].scale * i + allSources[0].offset),
        glm::vec3(0.f), glm::vec3(255.f)));
    for (size_t ch=1; ch<allSources.size(); ++ch) {
      colorMap[ch][i] = glm::col3(glm::clamp(allSources[ch].color *
                                             (allSources[ch].scale * i + allSources[ch].offset),
                                             glm::vec3(0.f), glm::vec3(255.f)));
    }
  }

  for (int j = startLine; j < endLine; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      glm::col3 col3 = colorMap[0][*(allSources[0].data)++];
      for (size_t ch=1; ch<allSources.size(); ++ch) {
        col3 = glm::max(col3, colorMap[ch][*(allSources[ch].data)++]);
      }

      *line++ = col3[0];
      *line++ = col3[1];
      *line++ = col3[2];
      *line++ = alpha;
    }
  }
}
