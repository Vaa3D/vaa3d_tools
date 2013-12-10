/**@file zlocsegchainconn.h
 * @brief Connection between two locseg chains
 * @author Ting Zhao
 * @date 22-AUG-2009
 */

#ifndef ZLOCSEGCHAINCONN_H_
#define ZLOCSEGCHAINCONN_H_

#include <QXmlStreamWriter>
#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "tz_neurocomp_conn.h"

class QImage;
class XmlStreamReader;
class ZLocsegChain;

/*
ZlocsegChainConn defines the class of locseg chain connections.
  Sample usage:
    ...
    ZLocsegChainConn *conn = new ZLocsegChainConn(hook, loop, mode);
    conn->translateMode();
    conn->print();
    ...
*/

class ZLocsegChainConn : public ZDocumentable, public ZStackDrawable {
public:
  ZLocsegChainConn();
  ZLocsegChainConn(int hook, int link, int hookSpot, int linkSpot,
                   int mode = NEUROCOMP_CONN_HL);
  ZLocsegChainConn(ZLocsegChain *hook, ZLocsegChain *loop,
                   int hookSpot, int loopSpot, int mode = NEUROCOMP_CONN_HL);
  virtual ~ZLocsegChainConn();

  virtual const std::string& className() const;

  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL)
  const;

  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  bool has(const ZLocsegChain *hook, const ZLocsegChain *loop) const;
  bool isHook(const ZLocsegChain *chain) const;
  bool isLoop(const ZLocsegChain *chain) const;

  inline ZLocsegChain *loopChain() { return m_loopChain; }
  inline ZLocsegChain *hookChain() { return m_hookChain; }

  //add for 3d widget
  inline int hookSpot() {return m_hookSpot;}
  inline int loopSpot() {return m_loopSpot;}
  inline int mode() {return m_mode;}

  void translateMode();

  void writeXml(QXmlStreamWriter &xml);

  void print();
private:
  int m_hook;
  int m_loop;
  int m_hookSpot;
  int m_loopSpot;
  int m_mode;
  ZLocsegChain *m_hookChain;
  ZLocsegChain *m_loopChain;
};

#endif /* ZLOCSEGCHAINCONN_H_ */
