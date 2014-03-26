#ifndef ZMOVIEPHOTOGRAPHER_H
#define ZMOVIEPHOTOGRAPHER_H

#include <string>

class ZStackDoc;
class ZMovieStage;

class ZMoviePhotographer
{
public:
  ZMoviePhotographer(ZMovieStage *stage = NULL);

  inline void setStage(ZMovieStage *stage) { m_stage = stage; }
  void render();
  void takePicture(const std::string &filePath, int width, int height);

private:
  ZMovieStage *m_stage;
};

#endif // ZMOVIEPHOTOGRAPHER_H
