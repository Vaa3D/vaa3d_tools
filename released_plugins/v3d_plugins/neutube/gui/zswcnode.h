#ifndef ZSWCNODE_H
#define ZSWCNODE_H

#include "zball.h"

class ZSwcNode : public ZBall
{
public:
  enum EType{
      UNKNOWN = 0, SOMA = 1, AXON, BASAL_DENDRITE, APICAL_DENDRITE, DENDRITE,
      NEURITE
  };

public:
  ZSwcNode();
  ZSwcNode(int id, EType type, double x, double y, double z, double r);

public:
  inline int id() const { return m_id; }
  inline EType type() const { return m_type; }
  inline void setId(int id) { m_id = id; }
  inline void setType(EType type) { m_type = type; }

private:
  int m_id;
  EType m_type;
};

#endif // ZSWCNODE_H
