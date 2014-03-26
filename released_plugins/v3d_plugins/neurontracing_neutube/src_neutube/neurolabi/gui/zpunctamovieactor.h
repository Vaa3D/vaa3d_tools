#ifndef ZPUNCTAMOVIEACTOR_H
#define ZPUNCTAMOVIEACTOR_H

#include <vector>
#include "zmovieactor.h"

class ZPunctum;

class ZPunctaMovieActor : public ZMovieActor
{
public:
  ZPunctaMovieActor();
  virtual ~ZPunctaMovieActor();

  void setActor(const std::vector<ZPunctum*> puncta);

  virtual void hide();
  virtual void show();
  virtual void move(double t);
  virtual void reset();

  virtual void pushColor();
  virtual void pushAlpha();
  virtual void pullColor();
  virtual void pullAlpha();

private:
  std::vector<ZPunctum*> m_puncta;

};

#endif // ZPUNCTAMOVIEACTOR_H
