#include <QtConcurrentRun>

template<class T> void ZImage::setBinaryData(const T *data, T bg,
                                             int threshold)
{
  if (threshold < 0) {
    int i, j;
    for (j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (i = 0; i < width(); i++) {
        uint8 value = '\0';
        if (*data++ != bg) {
          value = '\xff';
        }

        *line++ = value;
        *line++ = value;
        *line++ = value;
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
          *line++ = '\xff';
          ++data;
        } else {
          uint8 value = '\0';
          if (*data++ != bg) {
            value = '\xff';
          }
          
          *line++ = value;
          *line++ = value;
          *line++ = value;
          *line++ = '\xff';
        }
      }
    }
  }
}

template<class T>
void ZImage::setData(const T *data, double scale, double offset,
                     int threshold)
{
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
    return;
  }


    if (threshold < 0) {
        int i, j;
        for (j = 0; j < height(); j++) {
            uchar *line = scanLine(j);
            for (i = 0; i < width(); i++) {
                double value = scale * (*data++) + offset;
                uint8 v;
                if (value <= 0.0) {
                    v = '\0';
                } else if (value >= 255.0) {
                    v = '\xff';
                } else {
                    v = (uint8) value;
                }

                memset(line, v, 3);
                line += 4;
                /*
        *line++ = v;
        *line++ = v;
        *line++ = v;
        *line++ = '\xff';
    */
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
                    double value = scale * (*data++) + offset;
                    uint8 v;
                    if (value <= 0.0) {
                        v = '\0';
                    } else if (value >= 255.0) {
                        v = '\xff';
                    } else {
                        v = (uint8) value;
                    }
                    *line++ = v;
                    *line++ = v;
                    *line++ = v;
                }

                *line++ = '\xff';
            }
        }
    }
}

template<class T>
void ZImage::set3ChannelData(const T *data0, double scale0, double offset0,
                             const T *data1, double scale1, double offset1,
                             const T *data2, double scale2, double offset2,
                             uint8_t alpha)
{
  if (scale0 == 1.0 && offset0 == 0.0 && scale1 == 1.0 && offset1 == 0.0 &&
          scale2 == 1.0 && offset2 == 0.0) {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        *line++ = (uint8)(*data2++);
        *line++ = (uint8)(*data1++);
        *line++ = (uint8)(*data0++);
        *line++ = alpha;
        //line++;
      }
    }
  } else {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        double value = scale2 * (*data2++) + offset2;
        uint8 v;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        value = scale1 * (*data1++) + offset1;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        value = scale0 * (*data0++) + offset0;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        //*line++ = '\xff';
        line++;
      }
    }
  }
}

template<class T>
void ZImage::set2ChannelData(const T *data0, double scale0, double offset0,
                             const T *data1, double scale1, double offset1,
                             uint8_t alpha)
{
  if (scale0 == 1.0 && offset0 == 0.0 && scale1 == 1.0 && offset1 == 0.0) {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        *line++ = '\0';
        *line++ = (uint8)(*data1++);
        *line++ = (uint8)(*data0++);
        *line++ = alpha;
      }
    }
  } else {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        uint8 v = '\0';
        *line++ = v;
        double value = scale1 * (*data1++) + offset1;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        value = scale0 * (*data0++) + offset0;
        if (value <= 0.0) {
          v = '\0';
        } else if (value >= 255.0) {
          v = '\xff';
        } else {
          v = (uint8) value;
        }
        *line++ = v;
        *line++ = alpha;
        //line++;
      }
    }
  }
}


template<typename T>
void ZImage::setData(const ZImage::DataSource<T> &source, int threshold, bool useMultithread)
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
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlock<T>, source, startLine,
                                 endLine, threshold);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  const T* data = source.data;
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  if (scale == 1.f && offset == 0.f) {
    if (threshold < 0) {
      for (int j = 0; j < height(); j++) {
        uchar *line = scanLine(j);
        for (int i = 0; i < width(); i++) {
          glm::col3 col3 = glm::col3(color * (float)(*data++));

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
            glm::col3 col3 = glm::col3(color * (float)(*data++));

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
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));

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
          glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    }
  }
}

template<typename T>
void ZImage::setData(const std::vector<ZImage::DataSource<T> > &sources, uint8_t alpha,
                     bool useMultithread)
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
      res[i] = QtConcurrent::run(this, &ZImage::setDataBlockMS<T>, sources, startLine,
                                 endLine, alpha);
    }
    for (int i=0; i<numBlock; ++i)
      res[i].waitForFinished();
    return;
  }

  std::vector<ZImage::DataSource<T> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }

  if (!needScaleAndClamp) {
    for (int j = 0; j < height(); j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = glm::col3(allSources[0].color *
            (float)(*(allSources[0].data)++));
        for (size_t ch=1; ch<allSources.size(); ++ch)
          col3 = glm::max(col3, glm::col3(allSources[ch].color *
                          (float)(*(allSources[ch].data)++)));

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
    return;
  }

  for (int j = 0; j < height(); j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      glm::col3 col3 = glm::col3(glm::clamp(allSources[0].color *
                                 (allSources[0].scale * (*(allSources[0].data)++) + allSources[0].offset),
          glm::vec3(0.f), glm::vec3(255.f)));
      for (size_t ch=1; ch<allSources.size(); ++ch)
        col3 = glm::max(col3, glm::col3(glm::clamp(allSources[ch].color *
                                        (allSources[ch].scale * (*(allSources[ch].data)++) + allSources[ch].offset),
            glm::vec3(0.f), glm::vec3(255.f))));

      *line++ = col3[0];
      *line++ = col3[1];
      *line++ = col3[2];
      *line++ = alpha;
    }
  }
}

template<typename T>
void ZImage::setDataBlock(const ZImage::DataSource<T> &source, int startLine,
                          int endLine, int threshold)
{
  const T* data = source.data + startLine * width();
  float scale = source.scale;
  float offset = source.offset;
  glm::vec3 color = glm::vec3(source.color.b, source.color.g, source.color.r);

  if (scale == 1.f && offset == 0.f) {
    if (threshold < 0) {
      int i, j;
      for (j = startLine; j < endLine; j++) {
        uchar *line = scanLine(j);
        for (i = 0; i < width(); i++) {
          glm::col3 col3 = glm::col3(color * (float)(*data++));

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
            glm::col3 col3 = glm::col3(color * (float)(*data++));

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
        glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                              glm::vec3(0.f), glm::vec3(255.f)));

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
          glm::col3 col3 = glm::col3(glm::clamp(color * (scale * (*data++) + offset),
                                                glm::vec3(0.f), glm::vec3(255.f)));

          *line++ = col3[0];
          *line++ = col3[1];
          *line++ = col3[2];
          *line++ = '\xff';
        }
      }
    }
  }
}


template<typename T>
void ZImage::setDataBlockMS(const std::vector<ZImage::DataSource<T> > &sources, int startLine,
                    int endLine, uint8_t alpha)
{
  std::vector<ZImage::DataSource<T> > allSources = sources;
  bool needScaleAndClamp = false;
  for (size_t i=0; i<allSources.size(); ++i) {
    allSources[i].data += startLine * width();
    std::swap(allSources[i].color.r, allSources[i].color.b);
    if (allSources[i].scale != 1.f || allSources[i].offset != 0.f) {
      needScaleAndClamp = true;
    }
  }

  if (!needScaleAndClamp) {
    for (int j = startLine; j < endLine; j++) {
      uchar *line = scanLine(j);
      for (int i = 0; i < width(); i++) {
        glm::col3 col3 = glm::col3(allSources[0].color *
            (float)(*(allSources[0].data)++));
        for (size_t ch=1; ch<allSources.size(); ++ch)
          col3 = glm::max(col3, glm::col3(allSources[ch].color *
                          (float)(*(allSources[ch].data)++)));

        *line++ = col3[0];
        *line++ = col3[1];
        *line++ = col3[2];
        *line++ = alpha;
      }
    }
    return;
  }

  for (int j = startLine; j < endLine; j++) {
    uchar *line = scanLine(j);
    for (int i = 0; i < width(); i++) {
      glm::col3 col3 = glm::col3(glm::clamp(allSources[0].color *
                                 (allSources[0].scale * (*(allSources[0].data)++) + allSources[0].offset),
          glm::vec3(0.f), glm::vec3(255.f)));
      for (size_t ch=1; ch<allSources.size(); ++ch)
        col3 = glm::max(col3, glm::col3(glm::clamp(allSources[ch].color *
                                        (allSources[ch].scale * (*(allSources[ch].data)++) + allSources[ch].offset),
            glm::vec3(0.f), glm::vec3(255.f))));

      *line++ = col3[0];
      *line++ = col3[1];
      *line++ = col3[2];
      *line++ = alpha;
    }
  }
}

