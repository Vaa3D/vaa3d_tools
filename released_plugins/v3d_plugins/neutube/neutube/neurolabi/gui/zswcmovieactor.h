#ifndef ZSWCMOVIEACTOR_H
#define ZSWCMOVIEACTOR_H

#include "zmovieactor.h"

class ZSwcTree;

class ZSwcMovieActor : public ZMovieActor
{
public:
  ZSwcMovieActor();
  virtual ~ZSwcMovieActor();

  void setActor(ZSwcTree *tree);

  virtual void hide();
  virtual void show();
  virtual void move(double t);

  virtual void pushColor();
  virtual void pushAlpha();
  virtual void pullColor();
  virtual void pullAlpha();

  virtual void reset();

private:
  ZSwcTree *m_tree;
};

#endif // ZSWCMOVIEACTOR_H
