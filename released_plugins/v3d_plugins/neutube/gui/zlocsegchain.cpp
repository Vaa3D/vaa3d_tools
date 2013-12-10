#include <iostream>
#include "zlocsegchain.h"
#include "zlocalneuroseg.h"
#include "tz_trace_utils.h"
#include "tz_geo3d_utils.h"
#include "tz_workspace.h"
#include "tz_stack_attribute.h"

ZLocsegChain::ZLocsegChain(Locseg_Chain *chain)
{
  init(chain);
}

ZLocsegChain::ZLocsegChain(Local_Neuroseg *locseg)
{
  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  init(chain);
}

ZLocsegChain::ZLocsegChain(const ZLocsegChain &zlocseg)
  : ZInterface(), ZDocumentable(), ZStackDrawable(), ZSwcExportable(), ZVrmlExportable()
{
  m_chain = NULL;
  copyData(zlocseg.m_chain);
  updateBufferChain();

  m_zscale = zlocseg.m_zscale;
  m_id = zlocseg.m_id;
  m_heldNode = zlocseg.m_heldNode;
  m_ignorable = zlocseg.m_ignorable;
  m_source = zlocseg.m_source;

  m_oddColor = zlocseg.m_oddColor;
  m_evenColor = zlocseg.m_evenColor;
  m_endColor = zlocseg.m_endColor;
}

void ZLocsegChain::init(Locseg_Chain *chain)
{
  m_chain = chain;
  updateBufferChain();

  m_zscale = 1.0;
  m_id = -1;
  m_heldNode = -1;
  m_ignorable = false;

  m_oddColor.setRgb(255, 0, 0, 255);
  m_evenColor.setRgb(0, 255, 0, 255);
  m_endColor.setRgb(0, 0, 255, 255);

  m_source = "traced";
}

ZLocsegChain::~ZLocsegChain()
{
  m_bufferChain.clear();
  if (m_chain != NULL) {
    Kill_Locseg_Chain(m_chain);
  }
}

Locseg_Chain* ZLocsegChain::detachData()
{
   Locseg_Chain *chain = m_chain;
   m_chain = NULL;

   return chain;
}

void ZLocsegChain::copyData(Locseg_Chain *chain)
{
  if (m_chain != NULL) {
    Kill_Locseg_Chain(m_chain);
  }
  m_chain = New_Locseg_Chain();

  Locseg_Node_Dlist *srchead = Locseg_Node_Dlist_Head(chain->list);
  if (srchead != NULL) {
    Locseg_Node_Dlist *head = Locseg_Node_Dlist_New();
    head->data = Copy_Locseg_Node(srchead->data);
    //head->prev = NULL;
    //head->next = NULL;
    m_chain->list = head;
    srchead = srchead->next;
    Locseg_Node_Dlist *prev = head;
    while (srchead != NULL) {
      Locseg_Node_Dlist* current = Locseg_Node_Dlist_New();
      current->data = Copy_Locseg_Node(srchead->data);
      current->prev = prev;
      current->prev->next = current;
      //current->next = NULL;
      prev = current;
      srchead = srchead->next;
    }
  }
}

void ZLocsegChain::updateBufferChain()
{
  m_bufferChain.clear();
  if (isEmpty() == false) {
    Locseg_Chain_Iterator_Start(m_chain, DL_HEAD);
    Locseg_Node *node = NULL;
    while ((node = Locseg_Chain_Next(m_chain)) != NULL) {
      ZLocalNeuroseg zseg(node->locseg, false);
      m_bufferChain.append(zseg);
      m_bufferChain[m_bufferChain.size()-1].asyncGenerateFilterStack();
    }
  }
}

void ZLocsegChain::display(QPainter &painter, int z, Display_Style option) const
{
  if (!isVisible())
    return;

  if (!this->isEmpty()) {
    Locseg_Chain_Iterator_Start(m_chain, DL_HEAD);
    Locseg_Node *node = NULL;

    int index = 0;

    QList<ZLocalNeuroseg>::const_iterator zsegIterator = m_bufferChain.begin();

    Locseg_Chain_Next(m_chain);
    index++;
    ++zsegIterator;

    QColor color;

    while ((node = Locseg_Chain_Next(m_chain)) != NULL) {
      if (Locseg_Chain_Peek(m_chain) != NULL) {
        if (index % 2 == 0) {
          color = m_evenColor;
        } else {
          color = m_oddColor;
        }

        if (m_selected == true) {
          if (m_heldNode == index) {
            color = selectingColor(color);
          } else {
            color = highlightingColor(color);
          }
        }

        (*zsegIterator).display(painter, z, option, color);

        index++;
        ++zsegIterator;
      } else {
        break;
      }
    }

    Locseg_Chain_Iterator_Start(m_chain, DL_HEAD);

    if ((node = Locseg_Chain_Next(m_chain)) != NULL) {
      color = m_endColor;
      if (m_selected == true) {
        if (m_heldNode == 0) {
          color = selectingColor(color);
        } else {
          color = highlightingColor(color);
        }
      }

      (*m_bufferChain.begin()).display(painter, z, option, color);
    }

    if (index > 0) {
      Locseg_Chain_Iterator_Start(m_chain, DL_TAIL);
      if ((node = Locseg_Chain_Prev(m_chain)) != NULL) {
        color = m_endColor;
        if (m_selected == true) {
          if (m_heldNode == index) {
            color = selectingColor(color);
          } else {
            color = highlightingColor(color);
          }
        }

        (*(--m_bufferChain.end())).display(painter, z, option, color);
      }
    }
  }
}

int ZLocsegChain::swcFprint(FILE *fp, int start_id, int parent_id,
                            double z_scale)
{
  return Locseg_Chain_Swc_Fprint_Z(fp, m_chain, 2, start_id, parent_id,
                                   DL_FORWARD, z_scale / m_zscale);
}

void ZLocsegChain::swcExport(const char *filePath)
{
  FILE *fp = fopen(filePath, "w");
  if (fp != NULL) {
    swcFprint(fp);
    fclose(fp);
  }
}

void ZLocsegChain::vrmlFprint(FILE *fp, const Vrml_Material *material,
			      int indent, double z_scale)
{
  Locseg_Chain_Vrml_Fprint_Z(fp, m_chain, z_scale, material, indent);
}

void ZLocsegChain::save(const char *filePath)
{
  Write_Locseg_Chain(filePath, m_chain);
  m_source = QString(filePath);
}

void ZLocsegChain::load(const char *filePath)
{
  if (m_chain != NULL) {
    Kill_Locseg_Chain(m_chain);
  }

  m_chain = Read_Locseg_Chain(filePath);
  updateBufferChain();
  m_source = QString(filePath);
}

void ZLocsegChain::labelTraceMask(Stack *mask, int overwrite)
{
  if (mask != NULL) {
    Locseg_Chain_Label_G(m_chain, mask, m_zscale, 0,
                         Locseg_Chain_Length(m_chain),
                         1.0, 0.0, overwrite, m_id + 1);
  }
}

void ZLocsegChain::eraseTraceMask(Stack *mask) const
{
  if (mask != NULL) {
    Locseg_Chain_Label_G(m_chain, mask, m_zscale, 0,
			 Locseg_Chain_Length(m_chain),
			 1.0, 0.0, m_id + 1, 0);
  }
}

double ZLocsegChain::holdClosestSeg(double x, double y, double z)
{
  double pos[3];

  Locseg_Chain_Iterator_Start(m_chain, DL_HEAD);
  Local_Neuroseg *seg;
  int index = 0;
  double min_dist = 0.0;

  m_heldNode = 0;

  double zpos = z;

  while ((seg = Locseg_Chain_Next_Seg(m_chain)) != NULL) {
    Local_Neuroseg_Center(seg, pos);
    double dist = 0.0;

    if (z < 0) {
      dist = Geo3d_Dist_Sqr(x, y, 0, pos[0], pos[1], 0);
    } else {
      dist = Geo3d_Dist_Sqr(x, y, z, pos[0], pos[1], pos[2]);
    }
    if ((dist < min_dist) || (index == 0)) {
      min_dist = dist;
      m_heldNode = index;
      zpos = pos[2];
    }
    index++;
  }

  return zpos;
}

void ZLocsegChain::setSelected(bool selected)
{
  ZStackDrawable::setSelected(selected);
}

Local_Neuroseg* ZLocsegChain::heldNeuroseg()
{
  if (m_heldNode >= 0) {
    return Locseg_Chain_Peek_Seg_At(m_chain, m_heldNode);
  }

  return NULL;
}

ZLocsegChain* ZLocsegChain::pushHeldEnd(const Stack *stack, const Stack *mask)
{
  Locseg_Chain *chain = NULL;

  if (m_heldNode == 0) {
    Local_Neuroseg *locseg2 =
      Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(m_chain));
    Flip_Local_Neuroseg(locseg2);
    chain = Local_Neuroseg_Push(locseg2, stack, 1.0, mask, 1, NULL, NULL);
    Delete_Local_Neuroseg(locseg2);
  } else if (m_heldNode == Locseg_Chain_Length(m_chain) - 1) {
    Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(m_chain);
    chain = Local_Neuroseg_Push(locseg, stack, 1.0, mask, 1, NULL, NULL);
  }

  if (chain != NULL) {
    return new ZLocsegChain(chain);
  }

  return NULL;
}

ZLocsegChain* ZLocsegChain::pushHeldNode(const Stack *stack, const Stack *mask)
{
  Locseg_Chain *chain = NULL;

  Local_Neuroseg *locseg2 =
    Copy_Local_Neuroseg(Locseg_Chain_Peek_Seg_At(m_chain, m_heldNode));

  if ((m_heldNode + 1) * 2 <= Locseg_Chain_Length(m_chain)) {
    Flip_Local_Neuroseg(locseg2);
  }

  chain = Local_Neuroseg_Push(locseg2, stack, 1.0, mask, 1, NULL, NULL);
  Delete_Local_Neuroseg(locseg2);

  if (chain != NULL) {
    return new ZLocsegChain(chain);
  }

  return NULL;
}

void ZLocsegChain::fixTerminal(const Stack *stack, const Trace_Workspace *tw)
{
#ifdef _ADVANCED_
  Trace_Workspace tmptw;
  memcpy(&tmptw, tw, sizeof(Trace_Workspace));
  tmptw.trace_status[0] = TRACE_NORMAL;
  tmptw.trace_status[1] = TRACE_NORMAL;
  tmptw.refit = FALSE;

  if (Locseg_Chain_Length(m_chain) > 1) {
    Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(m_chain);
    Flip_Local_Neuroseg(locseg);
    if (Local_Neuroseg_Top_Sample(locseg, tw->trace_mask, 1.0) > 0) {
        tmptw.trace_status[0] = TRACE_NOT_ASSIGNED;
    }
    Local_Neuroseg_Height_Search_P(locseg, stack, 1.0);
    Flip_Local_Neuroseg(locseg);

    locseg = Locseg_Chain_Tail_Seg(m_chain);
    if (Local_Neuroseg_Top_Sample(locseg, tw->trace_mask, 1.0) > 0) {
        tmptw.trace_status[1] = TRACE_NOT_ASSIGNED;
    }
    Local_Neuroseg_Height_Search_P(locseg, stack, 1.0);
  }

  Trace_Locseg(stack, 1.0, m_chain, &tmptw);
  updateBufferChain();
#else
  UNUSED_PARAMETER(stack)
  UNUSED_PARAMETER(tw)
#endif
}

int ZLocsegChain::length()
{
  return Locseg_Chain_Length(m_chain);
}

double ZLocsegChain::geoLength()
{
  return Locseg_Chain_Geolen(m_chain);
}

QStringList ZLocsegChain::toStringList(const Stack *stack)
{
  QStringList list;
  list.append(QString("Chain %1: %2 segments").arg(m_id).arg(length()));
  if (m_source.isEmpty() == false) {
    list.append(m_source);
  }

  if (stack != NULL) {
    if (Stack_Channel_Number(stack) == 1) {
      double score = Locseg_Chain_Average_Score(m_chain, stack, m_zscale,
                                                STACK_FIT_CORRCOEF);
      list.append(QString("Average score: %1").arg(score));
    }

    Local_Neuroseg *locseg = heldNeuroseg();
    if (locseg != NULL) {
      list.append(QString("Held seg (%1) properties:").arg(m_heldNode));
      if (Stack_Channel_Number(stack) == 1) {
        Stack_Fit_Score fs;
        fs.n = 1;
        fs.options[0] = STACK_FIT_CORRCOEF;
        double score = Local_Neuroseg_Score(locseg, stack, m_zscale, &fs);
        list.append(QString("score %1;").arg(score));
      }
      list.append(QString("(%1, %2, %3) bottom r %4, top r %5, scale %6, "
                          "h %7, (%8, %9)")
                  .arg(locseg->pos[0]).arg(locseg->pos[1])
                  .arg(locseg->pos[2]).arg(NEUROSEG_R1(&(locseg->seg)))
                  .arg(NEUROSEG_R2(&(locseg->seg))).arg(locseg->seg.scale)
                  .arg(locseg->seg.h).arg(locseg->seg.theta).arg(locseg->seg.psi));
#ifdef _ADVANCED_
      list.append(QString("%1, %2, %3, %4, %5, 0.0, 0.0, %6, %7, %8, %9")
                  .arg(locseg->seg.r1).arg(locseg->seg.c)
                  .arg(locseg->seg.h).arg(locseg->seg.theta)
                  .arg(locseg->seg.psi).arg(locseg->seg.scale)
                  .arg(locseg->pos[0]).arg(locseg->pos[1])
                  .arg(locseg->pos[2]));
      double pos[3];
      Local_Neuroseg_Bottom(locseg, pos);
      double bottom_z = pos[2];
      Local_Neuroseg_Top(locseg, pos);
      double top_z = pos[2];
      list.append(QString("bottom z: %1, top z: %2, score: %3").arg(bottom_z)
                  .arg(top_z).arg(fabs(bottom_z - top_z) / locseg->seg.h));
#endif
    }
  }

  list.append(QString("Average thickness: %1")
              .arg(Locseg_Chain_Average_Bottom_Radius(m_chain)));

  return list;
}

Local_Neuroseg* ZLocsegChain::heldEndNeuroseg()
{
  if (m_heldNode > 0) {
    if (m_heldNode >= length() / 2) {
      return Locseg_Chain_Tail_Seg(m_chain);
    }
  }

  return Locseg_Chain_Head_Seg(m_chain);
}

ZLocsegChain* ZLocsegChain::bridge(ZLocsegChain *chain, bool link)
{
  const Local_Neuroseg *start_locseg = heldEndNeuroseg();
  const Local_Neuroseg *end_locseg = NULL;

  if (link == true) {
    end_locseg = chain->heldEndNeuroseg();
  } else {
    end_locseg = chain->heldNeuroseg();
  }

  Locseg_Chain *chain2 = Locseg_Chain_Bridge(start_locseg, end_locseg);

  return new ZLocsegChain(chain2);
}

ZLocsegChain* ZLocsegChain::spBridge(ZLocsegChain *chain, Stack *signal,
                                     Locseg_Fit_Workspace *fw)
{
  const Local_Neuroseg *start_locseg = heldEndNeuroseg();

  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  Locseg_Chain_Update_Stack_Graph_Workspace(start_locseg, chain->data(), signal,
                                            m_zscale, sgw);
  Locseg_Chain *chain2 = Locseg_Chain_Bridge_Sp(start_locseg, chain->data(),
                                                signal, m_zscale, sgw, fw);
  Kill_Stack_Graph_Workspace(sgw);

  if (chain2 == NULL) {
    /* require modification to be more efficient */
    Stack *mask = Make_Stack(GREY, Stack_Width(signal), Stack_Height(signal),
                             Stack_Depth(signal));
    Zero_Stack(mask);
    Sp_Grow_Workspace *tmpsgw = New_Sp_Grow_Workspace();
    tmpsgw->size = Stack_Voxel_Number(signal);

    Sp_Grow_Workspace_Set_Mask(tmpsgw, mask->array);
    tmpsgw->wf = Stack_Voxel_Weight_S;
    Stack_Sp_Grow_Infer_Parameter(tmpsgw, signal);

    ZLocsegChain source(Copy_Local_Neuroseg(start_locseg));
    chain2 = Locseg_Chain_Bridge_Sp_G(source.data(), chain->data(),
                                      signal, 1.0, tmpsgw);
    Kill_Sp_Grow_Workspace(tmpsgw);
    Kill_Stack(mask);
  }

  if (chain2 == NULL) {
    return NULL;
  }

  return new ZLocsegChain(chain2);
}

void ZLocsegChain::merge(ZLocsegChain *chain, bool bufferUpdate)
{
  Local_Neuroseg *locseg1 = Locseg_Chain_Head_Seg(this->m_chain);
  Local_Neuroseg *locseg2 = Locseg_Chain_Tail_Seg(this->m_chain);

  Local_Neuroseg *locseg3 = Locseg_Chain_Head_Seg(chain->m_chain);
  Local_Neuroseg *locseg4 = Locseg_Chain_Tail_Seg(chain->m_chain);

  Dlist_End_e ends[2];
  double c1[3], c2[3], c3[3], c4[3];

  Local_Neuroseg_Center(locseg1, c1);
  Local_Neuroseg_Center(locseg2, c2);
  Local_Neuroseg_Center(locseg3, c3);
  Local_Neuroseg_Center(locseg4, c4);

  double mindist = Geo3d_Dist_Sqr(c1[0], c1[1],c1[2], c3[0], c3[1],c3[2]);
  ends[0] = DL_HEAD;
  ends[1] = DL_HEAD;

  double d = Geo3d_Dist_Sqr(c1[0], c1[1],c1[2], c4[0], c4[1],c4[2]);
  if (mindist > d) {
    mindist = d;
    ends[1] = DL_TAIL;
  }

  d = Geo3d_Dist_Sqr(c2[0], c2[1],c2[2], c3[0], c3[1],c3[2]);
  if (mindist > d) {
    mindist = d;
    ends[0] = DL_TAIL;
    ends[1] = DL_HEAD;
  }

  d = Geo3d_Dist_Sqr(c2[0], c2[1],c2[2], c4[0], c4[1],c4[2]);
  if (mindist > d) {
    mindist = d;
    ends[0] = DL_TAIL;
    ends[1] = DL_TAIL;
  }

  Locseg_Chain_Merge(this->m_chain, chain->m_chain, ends[0], ends[1]);

  this->setIgnorable(false);

  if (bufferUpdate) {
    updateBufferChain();
  }
}

ZLocsegChain* ZLocsegChain::cutHeldNode(bool bufferUpdate)
{
  ZLocsegChain *new_chain = NULL;

  if (m_heldNode >= 0) {
    m_chain->list = Locseg_Node_Dlist_Head(m_chain->list);

    int n = Locseg_Chain_Length(m_chain);
    Locseg_Chain_Iterator_Start(m_chain, DL_HEAD);
    for (int i = 0; i < n; i++) {
      if (i == m_heldNode) {
        setSelected(false);
        m_heldNode = -1;

        Locseg_Chain *chain2 = New_Locseg_Chain();
        chain2->list = m_chain->iterator->next;
        if (chain2->list != NULL) {
          m_chain->iterator->next->prev = NULL;
          m_chain->iterator->next = NULL;
        }

        Locseg_Node_Dlist *orphan = m_chain->iterator;
        if (i == 0) {
          m_chain->list = NULL;
        } else {
          orphan->prev->next = NULL;
          orphan->prev = NULL;
          Locseg_Node_Dlist_Remove_All(orphan);
        }

        if (chain2->list == NULL) {
          Delete_Locseg_Chain(chain2);
        } else {
          new_chain = new ZLocsegChain(chain2);
        }

        if (bufferUpdate) {
          updateBufferChain();
        }

        break;
      } else {
        Locseg_Chain_Next(m_chain);
      }
    }
  }

  return new_chain;
}

void ZLocsegChain::refineEnd(Dlist_End_e end, Stack *signal,
                             Trace_Workspace *tw)
{
  if (isEmpty()) {
    return;
  }

  double oldTraceStep = tw->trace_step;
  int oldRefit = tw->refit;

  tw->trace_step = 0.1;
  tw->refit = FALSE;

  eraseTraceMask(tw->trace_mask);

  if (end == DL_HEAD) {
    tw->trace_status[0] = TRACE_NORMAL;
    tw->trace_status[1] = TRACE_NOT_ASSIGNED;
    Locseg_Node *node = Locseg_Chain_Head(m_chain);
    if (Trace_Record_Direction(node->tr) != DL_BACKWARD) {
      Locseg_Node_Set_Direction(node, DL_BOTHDIR);
    }
  } else {
    tw->trace_status[1] = TRACE_NORMAL;
    tw->trace_status[0] = TRACE_NOT_ASSIGNED;
    Locseg_Node *node = Locseg_Chain_Tail(m_chain);
    if (Trace_Record_Direction(node->tr) != DL_FORWARD) {
      Locseg_Node_Set_Direction(node, DL_BOTHDIR);
    }
  }
  Locseg_Chain_Adjust_Seed(m_chain);
  Trace_Locseg(signal, 1.0, m_chain, tw);

  Locseg_Chain_Down_Sample(m_chain);

  Locseg_Chain_Tune_End(m_chain, signal, 1.0, tw->trace_mask, end);

  if (Locseg_Chain_Length(m_chain) > 0) {
    labelTraceMask(tw->trace_mask);
  }

  tw->trace_step = oldTraceStep;
  tw->refit = oldRefit;

  updateBufferChain();
}

Dlist_End_e ZLocsegChain::heldEnd()
{
  if (m_heldNode < 0) {
    return DL_UNKNOWN_END;
  }

  if (m_heldNode * 2 < length()) {
    return DL_HEAD;
  } else {
    return DL_TAIL;
  }
}

bool ZLocsegChain::refineHeldEnd(Stack *stack, Trace_Workspace *tw)
{
  if (m_heldNode < 0) {
    return false;
  }

  Dlist_End_e end;

  if (m_heldNode * 2 < length()) {
    end = DL_HEAD;
  } else {
    end = DL_TAIL;
  }

  refineEnd(end, stack, tw);

  if (end == DL_HEAD) {
    m_heldNode = 0;
  } else {
    m_heldNode = length() - 1;
  }

  return true;
}

void ZLocsegChain::extendHeldEnd(double x, double y, double z)
{
  Dlist_End_e end = heldEnd();
  if (end != DL_UNKNOWN_END) {
    double pos[3];
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    Locseg_Chain_Extend(m_chain, end, pos);
    updateBufferChain();
  }
}

Local_Neuroseg* ZLocsegChain::headNeuroseg()
{
  return Locseg_Chain_Head_Seg(m_chain);
}

Local_Neuroseg* ZLocsegChain::tailNeuroseg()
{
  return Locseg_Chain_Tail_Seg(m_chain);
}

Local_Neuroseg* ZLocsegChain::neurosegAt(int index)
{
  return Locseg_Chain_Peek_Seg_At(m_chain, index);
}

double ZLocsegChain::confidence(Stack *stack, double z_scale)
{
  /* parameters learned from logistic regression */
  double c[3] = {1.1072, 5.3103, -2.0465};

  /* anisotropic parameters */
  if (z_scale != 1.0) {
    c[0] = 7.0544;
    c[1] = 5.2073;
    c[2] = -12.8934;
  }

  double x1;
  if ((z_scale == 1.0) && (m_zscale == 1.0)) {
    x1 = Locseg_Chain_Geolen(m_chain);
  } else {
    x1 = Locseg_Chain_Geolen_Z(m_chain, z_scale / m_zscale);
  }
  double x2 = Locseg_Chain_Average_Score(m_chain, stack, m_zscale,
                                         STACK_FIT_CORRCOEF);

  return 1.0 / (1.0 + exp((x1 * c[0] + x2 * c[1] + x1 * x2 * c[2])));
}

QColor ZLocsegChain::highlightingColor(const QColor &color) const
{
  QColor highlight;

  highlight.setRed(imin2(255, color.red() + 96));
  highlight.setGreen(imin2(255, color.green() + 96));
  highlight.setBlue(imin2(255, color.blue() + 96));

  return highlight;
}

QColor ZLocsegChain::selectingColor(const QColor &color) const
{
  QColor select;

  select.setHsv((color.hue() + 60) % 360,
                /*color.saturation()*/255, 255/*color.value()*/);

  return select;
}

ZLocsegChain* ZLocsegChain::breakBetween(int index1, int index2)
{
  ZLocsegChain *newChain =
      new ZLocsegChain(Locseg_Chain_Break_Between(m_chain, index1, index2));

  updateBufferChain();

  return newChain;
}

void ZLocsegChain::headPosition(double pos[]) const
{
  m_bufferChain[0].topPosition(pos);
}

void ZLocsegChain::tailPosition(double pos[]) const
{
  if (!m_bufferChain.empty())
    m_bufferChain[m_bufferChain.size()-1].bottomPosition(pos);
}

ZINTERFACE_DEFINE_CLASS_NAME(ZLocsegChain)
