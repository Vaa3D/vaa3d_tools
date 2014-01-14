#ifndef ZSTACKMOVIEACTOR_H
#define ZSTACKMOVIEACTOR_H

#include "zmovieactor.h"

class ZStack;

class ZStackMovieActor : public ZMovieActor
{
public:
  ZStackMovieActor();
  ~ZStackMovieActor();

  void setActor(ZStack *stack);

  virtual void hide();
  virtual void show();

  virtual void reset();

  virtual void move(double t);
  virtual void pushAlpha();
  virtual void pushColor();
  virtual void pullAlpha();
  virtual void pullColor();

private:
  ZStack *m_stack;
};

#endif // ZSTACKMOVIEACTOR_H
