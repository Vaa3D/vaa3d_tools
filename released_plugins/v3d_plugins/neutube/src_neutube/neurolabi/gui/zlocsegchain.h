/**@file zlocsegchain.h
 * @brief Locseg chain class
 * @author Ting Zhao
 */
#ifndef _ZLOCSEGCHAIN_H_
#define _ZLOCSEGCHAIN_H_

#include <QString>
#include <QStringList>
#include <QList>
#include <QColor>

#include "zstackdrawable.h"
#include "zdocumentable.h"
#include "zswcexportable.h"
#include "zvrmlexportable.h"
#include "tz_locseg_chain.h"
#include "zlocalneuroseg.h"

/*
ZlocsegChain defines the class of locseg chains.
  Sample usage:
    ...
    ZLocsegChain *chain = new ZLocsegChain();
    chain->load("test.tb");
    chain->display(image);
    ...
*/

class ZLocsegChain : public ZDocumentable, public ZStackDrawable,
		     public ZSwcExportable, public ZVrmlExportable {
public:
  ZLocsegChain(Locseg_Chain *chain = NULL);
  ZLocsegChain(Local_Neuroseg *locseg);
  ZLocsegChain(const ZLocsegChain &zlocseg);   // deep copy
  virtual ~ZLocsegChain();

  virtual const std::string& className() const;

public:
  virtual void display(QPainter &painter, int z = 0, Display_Style option = NORMAL)
  const;

public: // I/O functions
  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  virtual int swcFprint(FILE *fp, int start_id = 0, int parent_id = -1,
			double z_scale = 1.0);
  virtual void vrmlFprint(FILE *fp, const Vrml_Material *material = NULL,
			  int indent = 0, double z_scale = 1.0);
  virtual void swcExport(const char *filePath);

public:
  //Detach data from the object
  Locseg_Chain* detachData();
  inline Locseg_Chain *data() { return m_chain; }
  void setId(int id) { m_id = id; }
  inline int id() const { return m_id; }
  inline const QString source() const { return m_source; }
  inline int heldNode() const { return m_heldNode; }
  inline double zScale() const { return m_zscale; }
  inline void setZScale(double zScale) { m_zscale = zScale; }
  inline bool isIgnorable() { return m_ignorable; }
  inline bool isEmpty() const { return (m_chain == NULL) || (m_chain->list == NULL); }
  inline void setIgnorable(bool ignorable) { m_ignorable = ignorable; }
  void copyData(Locseg_Chain* locseg);

  void labelTraceMask(Stack *mask, int overwrite = 0);
  void eraseTraceMask(Stack *mask) const;

  virtual void setSelected(bool selected);
  double holdClosestSeg(double x, double y, double z);

  ZLocsegChain* pushHeldEnd(const Stack *stack, const Stack *mask = NULL);
  ZLocsegChain* pushHeldNode(const Stack *stack, const Stack *mask = NULL);

  //Cut at the held node
  ZLocsegChain* cutHeldNode(bool bufferUpdate = true);

  void fixTerminal(const Stack *stack, const Trace_Workspace *tw);
  void refineEnd(Dlist_End_e end, Stack *stack, Trace_Workspace *tw);
  Dlist_End_e heldEnd();
  bool refineHeldEnd(Stack *stack, Trace_Workspace *tw);
  void extendHeldEnd(double x, double y, double z);

  ZLocsegChain* bridge(ZLocsegChain *chain, bool link = true);
  ZLocsegChain* spBridge(ZLocsegChain *chain, Stack *signal,
                         Locseg_Fit_Workspace *fw);

  void merge(ZLocsegChain *chain, bool bufferUpdate = true);

  Local_Neuroseg* heldNeuroseg();
  Local_Neuroseg* heldEndNeuroseg();
  int length();
  double geoLength();

  Local_Neuroseg* headNeuroseg();
  Local_Neuroseg* tailNeuroseg();
  Local_Neuroseg* neurosegAt(int index);

  QStringList toStringList(const Stack *stack = NULL);

  double confidence(Stack *stack, double z_scale = 1.0);
  void updateBufferChain();

  ZLocsegChain* breakBetween(int index1, int index2);

  void headPosition(double pos[3]) const;
  void tailPosition(double pos[3]) const;

private:
  void init(Locseg_Chain *chain);
  QColor highlightingColor(const QColor &color) const;
  QColor selectingColor(const QColor &color) const;

private:
  Locseg_Chain *m_chain;
  QList<ZLocalNeuroseg> m_bufferChain;
  double m_zscale;
  int m_id;
  int m_heldNode;
  QString m_source;
  bool m_ignorable;

  QColor m_oddColor;
  QColor m_evenColor;
  QColor m_endColor;
};

#endif
