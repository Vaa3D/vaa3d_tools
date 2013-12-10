#ifndef ZSTACKBLENDER_H
#define ZSTACKBLENDER_H

#include "zstack.hxx"

class ZStackBlender
{
public:
  ZStackBlender();

  ZStack *blend(const ZStack &stack1,
                const ZStack &stack2, double alpha);

};

#endif // ZSTACKBLENDER_H
