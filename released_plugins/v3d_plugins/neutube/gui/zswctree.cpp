#if defined(_QT_GUI_USED_)
#include <QtGui>
#include <QPointF>
#endif

#include <algorithm>
#include <sstream>
#include <queue>
#include <limits.h>
#include "tz_error.h"
#include "zswctree.h"
#include "tz_voxel_graphics.h"
#include "tz_math.h"
#include "tz_apo.h"
#include "tz_geo3d_utils.h"
#include "tz_locseg_chain_com.h"
#include "tz_trace_utils.h"
#include "zcircle.h"
#include "zswcforest.h"
#include "zswcbranch.h"
#include "zfilelist.h"
#include "tz_random.h"
#include "tz_geoangle_utils.h"
#include "zrandomgenerator.h"
#include "swctreenode.h"
#include "zswctrunkanalyzer.h"
#include "zswcdisttrunkanalyzer.h"
#include "zstring.h"
#include "zfiletype.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "zerror.h"

using namespace std;

ZSwcTree::ZSwcTree(const ZSwcTree &src)
  : ZInterface(), ZDocumentable(), ZStackDrawable(), ZSwcExportable()
{
  m_tree = src.cloneData();
  //m_source = "new tree";
  m_iteratorReady = false;
}

ZSwcTree::ZSwcTree()
{
  m_tree = NULL;
  //m_source = "new tree";
  m_iteratorReady = false;
}

ZSwcTree::~ZSwcTree()
{
#ifdef _DEBUG_2
  std::cout << "Deleting ...";
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    std::cout << tn << ": ";
    Print_Swc_Tree_Node(tn);
  }
#endif

  if (m_tree != NULL) {
    Kill_Swc_Tree(m_tree);
  }
  m_tree = NULL;
}

void swap(ZSwcTree &first, ZSwcTree &second)
{
  std::swap(first.m_tree, second.m_tree);
}

void ZSwcTree::setData(Swc_Tree *tree, ESetDataOption option)
{
  switch (option) {
  case CLEAN_ALL:
    if (m_tree != NULL) {
      Kill_Swc_Tree(m_tree);
    }
    break;
  case FREE_WRAPPER:
    if (m_tree != NULL) {
      free(m_tree);
    }
    break;
  default:
    break;
  }

  m_tree = tree;
  deprecate(ALL_COMPONENT);
}

bool ZSwcTree::hasRegularNode()
{
  if (m_tree != NULL) {
    if (SwcTreeNode::isRegular(m_tree->root)) {
      return true;
    } else {
      updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);
      for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
        if (SwcTreeNode::isRegular(tn)) {
          return true;
        }
      }
    }
  }

  return false;
}

void ZSwcTree::setDataFromNode(Swc_Tree_Node *node, ESetDataOption option)
{
  if (node->parent != NULL) {
    Swc_Tree_Node_Detach_Parent(node);
  }

  switch (option) {
  case CLEAN_ALL:
    if (m_tree != NULL) {
      Kill_Swc_Tree(m_tree);
      m_tree = NULL;
    }
    break;
  case FREE_WRAPPER:
    if (m_tree != NULL) {
      free(m_tree);
      m_tree = NULL;
    }
  default:
    break;
  }

  if (m_tree == NULL) {
    m_tree = New_Swc_Tree();
  }

  m_tree->root = node;
  deprecate(ALL_COMPONENT);
}

void ZSwcTree::setDataFromNodeRoot(Swc_Tree_Node *node, ESetDataOption option)
{
  while (node->parent != NULL) {
    node = node->parent;
  }

  setDataFromNode(node, option);
}

void ZSwcTree::save(const char *filePath)
{
#ifdef _DEBUG_
  cout << "Saving " << filePath << endl;
#endif

  if (!isEmpty()) {
    Write_Swc_Tree_E(filePath, m_tree);
  } else {
    RECORD_WARNING_UNCOND("Empty tree. No file saved.");
  }
}

void ZSwcTree::load(const string &filePath)
{
  load(filePath.c_str());
}

void ZSwcTree::save(const string &filePath)
{
  save(filePath.c_str());
}

void ZSwcTree::load(const char *filePath)
{
  if (m_tree != NULL) {
    Kill_Swc_Tree(m_tree);
  }

  if (!fexist(filePath)) {
    RECORD_WARNING_UNCOND("Swc file does not exist.");
    return;
  }

  m_tree = Read_Swc_Tree_E(filePath);
  if (m_tree) {
    m_source = filePath;

#ifdef _DEBUG_2
    m_source = "test";
#endif
  }

  if (m_tree != NULL) {
    //Read meta information
    FILE *fp = fopen(filePath, "r");
    ZString line;
    ZString styleFilePath;
    if (line.readLine(fp)) {
      if (line.startsWith("#@")) {
        styleFilePath = line.substr(2);
        styleFilePath.trim();
      }
    }

    if (ZFileType::fileType(styleFilePath) == ZFileType::JSON_FILE) {
      if (!styleFilePath.isAbsolutePath()) {
        styleFilePath = styleFilePath.absolutePath(ZString::dirPath(m_source));
      }
      if (fexist(styleFilePath.c_str())) {
        ZJsonObject jsonObj;
        jsonObj.load(styleFilePath.c_str());
        json_t *value;
        value = jsonObj["color"];
        if (value != NULL) {
          int alpha = 255;
          if (ZJsonParser::arraySize(value) == 4) {
            alpha = iround(ZJsonParser::numberValue(value, 3) * 255.0);
          }

          setColor(iround(ZJsonParser::numberValue(value, 0) * 255.0),
                   iround(ZJsonParser::numberValue(value, 1) * 255.0),
                   iround(ZJsonParser::numberValue(value, 2) * 255.0),
                   alpha);
        }

        value = jsonObj["resolution"];
        if (value != NULL) {
          double res[3] = {1, 1, 1};
          if (ZJsonParser::arraySize(value) == 3) {
            for (int i = 0; i < 3; ++i) {
              res[i] = ZJsonParser::numberValue(value, i);
            }

            rescale(res[0], res[1], res[2]);
          }
        }

        value = jsonObj["transform"];
        if (value != NULL) {
          bool scaleFirst = true;
          double scaleFactor[3] = {1, 1, 1};
          double offset[3] = {1, 1, 1};

          ZJsonObject transformObj(value, false);
          json_t *transformField = transformObj["scale"];
          if (transformField != NULL) {
            if (ZJsonParser::arraySize(transformField) == 3) {
              for (int i = 0; i < 3; ++i) {
                scaleFactor[i] = ZJsonParser::numberValue(transformField, i);
              }
            }
          }

         transformField = transformObj["translate"];
          if (transformField != NULL) {
            if (ZJsonParser::arraySize(transformField) == 3) {
              for (int i = 0; i < 3; ++i) {
                offset[i] = ZJsonParser::numberValue(transformField, i);
              }
            }
          }

          transformField = transformObj["scale_first"];
          if (transformField != NULL) {
            scaleFirst = ZJsonParser::booleanValue(transformField);
          }

          if (scaleFirst) {
            rescale(scaleFactor[0], scaleFactor[1], scaleFactor[2]);
          }
          translate(offset[0], offset[1], offset[2]);
          if (!scaleFirst) {
            rescale(scaleFactor[0], scaleFactor[1], scaleFactor[2]);
          }
        }
      }
    }
    fclose(fp);
  }

  deprecate(ALL_COMPONENT);
}

//void ZSwcTree::display(QImage *image, int n, Display_Style style) const
//{
//  if (!isVisible())
//    return;

//  UNUSED_PARAMETER(style);
//#if defined(_QT_GUI_USED_)
//  updateIterator(2);

//  if (style == NORMAL) {
//    style = SOLID;
//  }

//  QColor rootColor(164, 164, 255);
//  QColor branchPointColor(164, 255, 164);
//  QColor nodeColor(255, 164, 164);

//  QColor rootFocusColor(0, 0, 255);
//  QColor branchPointFocusColor(0, 255, 0);
//  QColor nodeFocusColor(255, 0, 0);


//  QPainter painter(image);
//  painter.setPen(QPen(QColor(0, 0, 255, 255)));
//  painter.setBrush(Qt::NoBrush);

//  double bottom_position[3], top_position[3];
//  voxel_t src, dst;

//  for (const Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
//    if (Swc_Tree_Node_Edge(tn, bottom_position, top_position) == TRUE) {
//      bool labeled = false;
//      if ((SwcTreeNode::label(tn) == 1) &&
//          (SwcTreeNode::label(tn->parent) == 1)) {
//        labeled = true;
//      }

//      for (int i = 0; i < 3; i++) {
//        src[i] = iround(bottom_position[i]);
//        dst[i] = iround(top_position[i]);
//      }

//      Object_3d *obj = Line_To_Object_3d(src, dst);

//      for (size_t i = 0; i < obj->size; i++) {
//        if (((obj->voxels[i][2] == n) || (n == -1)) &&
//            IS_IN_CLOSE_RANGE(obj->voxels[i][0], 0, image->width() - 1) &&
//            IS_IN_CLOSE_RANGE(obj->voxels[i][1], 0, image->height() - 1)) {
//          uchar *pixel =
//              image->scanLine(obj->voxels[i][1]) + 4 * obj->voxels[i][0];
//          if ((i == 0) || (i == obj->size - 1)) {
//            pixel[RED] = 255;
//            pixel[GREEN] = 128;
//            pixel[BLUE] = 0;
//            pixel[ALPHA] = 255;
//          } else {
//            pixel[RED] = 255;
//            pixel[GREEN] = 0;
//            pixel[BLUE] = 0;
//            pixel[ALPHA] = 255;
//          }

//          /*
//          if (labeled) {
//            pixel[GREEN] = 255;
//          }
//          */
//        }
//      }

//      Kill_Object_3d(obj);
//    }
//  }

//  for (const Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
//    switch (style) {
//    case BOUNDARY:
//#ifdef _ADVANCED_
//      if (Swc_Tree_Node_Is_Regular(tn)) {
//        if (SwcTreeNode::label(tn) > 0) {
//          if (tn->parent != NULL) {
//            if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
//              Swc_Tree_Node_Pos(tn, top_position);
//              for (int i = 0; i < 3; i++) {
//                dst[i] = iround(top_position[i]);
//              }
//              painter.setPen(QPen(QColor(0, 255, 255, 255)));
//              /*
//              painter.drawText(dst[0], dst[1], QString("%1").
//                               arg(Swc_Tree_Node_Data(tn)->id));
//                               */
//            }
//          }
//        }
//      }
//#endif
//      if ((Swc_Tree_Node_Is_Root(tn) && (Swc_Tree_Node_Is_Regular(tn)))) {
//        if ((iround(Swc_Tree_Node_Const_Data(tn)->z) == n) || (n == -1)) {
//          int half_size = iround(Swc_Tree_Node_Radius(tn) - 0.5);
//          int cx = iround(Swc_Tree_Node_Const_Data(tn)->x);
//          int cy = iround(Swc_Tree_Node_Const_Data(tn)->y);
//          painter.setPen(QPen(rootColor));
//          painter.drawRect(cx - half_size, cy - half_size,
//                           half_size * 2 + 1, half_size * 2 + 1);
//        }

//      } else if (Swc_Tree_Node_Is_Branch_Point(tn)) {
//        Swc_Tree_Node_Pos(tn, top_position);
//        painter.setPen(QPen(branchPointColor));
//        //painter.setPen(QPen(QColor(0, 0, 255, 255)));

//        ZCircle circle(Swc_Tree_Node_Const_Data(tn)->x,
//                       Swc_Tree_Node_Const_Data(tn)->y,
//                       Swc_Tree_Node_Const_Data(tn)->z,
//                       Swc_Tree_Node_Radius(tn));
//        circle.display(&painter, n, style);
//      }
//      break;
//    case SOLID:
//      if (Swc_Tree_Node_Is_Regular(tn)) {
//        double r = Swc_Tree_Node_Radius(tn);
//        bool visible = false;
//        bool focused = false;

//        if ((iround(Swc_Tree_Node_Const_Data(tn)->z) == n) || (n == -1)) {
//          visible = true;
//          focused = true;
//        } else if (fabs(Swc_Tree_Node_Const_Data(tn)->z - n) < r) {
//          r = sqrt(r * r - (Swc_Tree_Node_Const_Data(tn)->z - n) *
//                   (Swc_Tree_Node_Const_Data(tn)->z - n));
//          visible = true;
//          if (fabs(Swc_Tree_Node_Const_Data(tn)->z - n) <= 0.5) {
//            focused = true;
//          }
//        }


//        if (visible) {
//          QColor color = nodeColor;
//          QColor focusColor = nodeFocusColor;
//          if (Swc_Tree_Node_Is_Regular_Root(tn)) {
//            color = rootColor;
//            focusColor = rootFocusColor;
//          } else if (Swc_Tree_Node_Is_Branch_Point(tn)) {
//            color = branchPointColor;
//            focusColor = branchPointFocusColor;
//            //painter.setPen(QPen(branchPointColor));
//          }
//          //color.setAlpha(255);

//          if (focused) {
//            color.setAlpha(16);
//            //painter.setBrush(QBrush(color));
//            painter.setPen(QPen(focusColor));
//          } else {
//            painter.setPen(QPen(color));
//            //painter.setBrush(Qt::NoBrush);
//          }
//          /*
//          QPainterPath circlePath;
//          circlePath.addEllipse(QPointF(SwcTreeNode::x(tn), SwcTreeNode::y(tn)),
//                                r, r);
//          painter.drawPath(circlePath);
//          */
//          /*
//          painter.drawArc(SwcTreeNode::x(tn) - r, SwcTreeNode::y(tn) - r,
//                          r * 2, r * 2, 0, 16*360);
//                          */

//          //painter.setRenderHint(QPainter::Antialiasing);
//          painter.drawEllipse(QPointF(Swc_Tree_Node_Const_Data(tn)->x,
//                                      Swc_Tree_Node_Const_Data(tn)->y),
//                              r, r);
//        }
//      }
//      break;
//    case SKELETON:
//      if (Swc_Tree_Node_Is_Branch_Point(tn)) {
//        Swc_Tree_Node_Pos(tn, top_position);
//        for (int i = 0; i < 3; i++) {
//          dst[i] = iround(top_position[i]);
//        }
//        if ((dst[2] == n) || (n == -1)) {
//          painter.setPen(QPen(QColor(0, 0, 255, 255)));
//          painter.drawPoint(dst[0], dst[1]);
//        }
//      }
//      break;
//    default:
//      break;
//    }
//  }
//#endif
//}

void ZSwcTree::display(QPainter &painter, int n, ZStackDrawable::Display_Style style) const
{
  if (!isVisible())
    return;

#if defined(_QT_GUI_USED_)
  const double strokeWidth = m_defaultPenWidth;

  updateIterator(2);

  if (style == NORMAL) {
    style = SOLID;
  }

  QColor rootColor(164, 164, 255);
  QColor branchPointColor(164, 255, 164);
  QColor nodeColor(255, 164, 164);

  QColor rootFocusColor(0, 0, 255);
  QColor branchPointFocusColor(0, 255, 0);
  QColor nodeFocusColor(255, 0, 0);

  painter.setPen(QPen(QColor(0, 0, 255, 255), strokeWidth));
  painter.setBrush(Qt::NoBrush);

  double bottom_position[3], top_position[3];
  voxel_t dst;

  for (const Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Edge(tn, bottom_position, top_position) == TRUE) {
      bool labeled = false;
      if ((SwcTreeNode::label(tn) == 1) &&
          (SwcTreeNode::label(tn->parent) == 1)) {
        labeled = true;
      }

      if (Swc_Tree_Node_Is_Regular(tn)) {
        double r = Swc_Tree_Node_Radius(tn);
        bool visible = false;

        if ((iround(Swc_Tree_Node_Const_Data(tn)->z) == n) || (n == -1)) {
          visible = true;
        } else if (fabs(Swc_Tree_Node_Const_Data(tn)->z - n) < r) {
          r = sqrt(r * r - (Swc_Tree_Node_Const_Data(tn)->z - n) *
                   (Swc_Tree_Node_Const_Data(tn)->z - n));
          visible = true;
        }


        if (visible) {
          QColor lineTerminalColor(255, 255, 0);
          QColor lineColor(255, 0, 0);
          if (Swc_Tree_Node_Is_Leaf(tn) || Swc_Tree_Node_Is_Regular_Root(tn->parent))
            painter.setPen(QPen(lineTerminalColor, strokeWidth));
          else
            painter.setPen(QPen(lineColor, strokeWidth));


          painter.drawLine(QPointF(Swc_Tree_Node_Const_Data(tn)->x,
                                   Swc_Tree_Node_Const_Data(tn)->y),
                           QPointF(Swc_Tree_Node_Const_Data(tn->parent)->x,
                                   Swc_Tree_Node_Const_Data(tn->parent)->y));
        }
      }
    }
  }

  for (const Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    switch (style) {
    case BOUNDARY:
#ifdef _ADVANCED_
      if (Swc_Tree_Node_Is_Regular(tn)) {
        if (SwcTreeNode::label(tn) > 0) {
          if (tn->parent != NULL) {
            if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
              Swc_Tree_Node_Pos(tn, top_position);
              for (int i = 0; i < 3; i++) {
                dst[i] = iround(top_position[i]);
              }
              painter.setPen(QPen(QColor(0, 255, 255, 255), strokeWidth));
              /*
              painter.drawText(dst[0], dst[1], QString("%1").
                               arg(Swc_Tree_Node_Data(tn)->id));
                               */
            }
          }
        }
      }
#endif
      if ((Swc_Tree_Node_Is_Root(tn) && (Swc_Tree_Node_Is_Regular(tn)))) {
        if ((iround(Swc_Tree_Node_Const_Data(tn)->z) == n) || (n == -1)) {
          int half_size = iround(Swc_Tree_Node_Radius(tn) - 0.5);
          int cx = iround(Swc_Tree_Node_Const_Data(tn)->x);
          int cy = iround(Swc_Tree_Node_Const_Data(tn)->y);
          painter.setPen(QPen(rootColor, strokeWidth));
          QRect rootRect(cx - half_size, cy - half_size,
                         half_size * 2 + 1, half_size * 2 + 1);
          painter.drawRect(rootRect);
        }

      } else if (Swc_Tree_Node_Is_Branch_Point(tn)) {
        Swc_Tree_Node_Pos(tn, top_position);
        painter.setPen(QPen(branchPointColor, strokeWidth));
        //painter.setPen(QPen(QColor(0, 0, 255, 255)));

        ZCircle circle(Swc_Tree_Node_Const_Data(tn)->x,
                       Swc_Tree_Node_Const_Data(tn)->y,
                       Swc_Tree_Node_Const_Data(tn)->z,
                       Swc_Tree_Node_Radius(tn));
        circle.display(&painter, n, style);
      }
      break;
    case SOLID:
      if (Swc_Tree_Node_Is_Regular(tn)) {
        double r = Swc_Tree_Node_Radius(tn);
        bool visible = false;
        bool focused = false;

        if ((iround(Swc_Tree_Node_Const_Data(tn)->z) == n) || (n == -1)) {
          visible = true;
          focused = true;
        } else if (fabs(Swc_Tree_Node_Const_Data(tn)->z - n) < r) {
          r = sqrt(r * r - (Swc_Tree_Node_Const_Data(tn)->z - n) *
                   (Swc_Tree_Node_Const_Data(tn)->z - n));
          visible = true;
          if (fabs(Swc_Tree_Node_Const_Data(tn)->z - n) <= 0.5) {
            focused = true;
          }
        }


        if (visible) {
          QColor color = nodeColor;
          QColor focusColor = nodeFocusColor;
          if (Swc_Tree_Node_Is_Regular_Root(tn)) {
            color = rootColor;
            focusColor = rootFocusColor;
          } else if (Swc_Tree_Node_Is_Branch_Point(tn)) {
            color = branchPointColor;
            focusColor = branchPointFocusColor;
            //painter.setPen(QPen(branchPointColor));
          }
          //color.setAlpha(255);

          if (focused) {
            color.setAlpha(16);
            //painter.setBrush(QBrush(color));
            painter.setPen(QPen(focusColor, strokeWidth));
          } else {
            painter.setPen(QPen(color, strokeWidth));
            //painter.setBrush(Qt::NoBrush);
          }
          /*
          QPainterPath circlePath;
          circlePath.addEllipse(QPointF(SwcTreeNode::x(tn), SwcTreeNode::y(tn)),
                                r, r);
          painter.drawPath(circlePath);
          */
          /*
          painter.drawArc(SwcTreeNode::x(tn) - r, SwcTreeNode::y(tn) - r,
                          r * 2, r * 2, 0, 16*360);
                          */

          //painter.setRenderHint(QPainter::Antialiasing);
          double adjustedRadius = r + m_defaultPenWidth * 0.5;
          painter.drawEllipse(QPointF(Swc_Tree_Node_Const_Data(tn)->x,
                                      Swc_Tree_Node_Const_Data(tn)->y),
                              adjustedRadius, adjustedRadius);
        }
      }
      break;
    case SKELETON:
      if (Swc_Tree_Node_Is_Branch_Point(tn)) {
        Swc_Tree_Node_Pos(tn, top_position);
        for (int i = 0; i < 3; i++) {
          dst[i] = iround(top_position[i]);
        }
        if ((dst[2] == n) || (n == -1)) {
          painter.setPen(QPen(QColor(0, 0, 255, 255), strokeWidth));
          painter.drawPoint(dst[0], dst[1]);
        }
      }
      break;
    default:
      break;
    }
  }
#endif
}

int ZSwcTree::size()
{
  int n = 0;

  if (m_tree->root != NULL) {
    n = Swc_Tree_Node_Fsize(m_tree->root);
  }

  if (Swc_Tree_Node_Is_Virtual(m_tree->root)) {
    n--;
  }

  return n;
}

int ZSwcTree::size(Swc_Tree_Node *start)
{
  int count = updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, start, FALSE);

  return count;
}

int ZSwcTree::updateIterator(int option, BOOL indexing) const
{
  if (isEmpty()) {
    if (m_tree->root != NULL) {
      m_tree->root->next = NULL;
    }
    return 0;
  }

  if (m_iteratorReady) {
    if (Swc_Tree_Iterator_Is_Active(option)) {
      option = SWC_TREE_ITERATOR_NO_UPDATE;
    }
    m_iteratorReady = false;
  }

  return Swc_Tree_Iterator_Start(const_cast<Swc_Tree*>(m_tree), option, indexing);
}

int ZSwcTree::updateIterator(int option, const set<Swc_Tree_Node*> &blocker,
                   BOOL indexing) const
{
  return updateIterator(option, NULL, blocker, indexing);
}

int ZSwcTree::updateIterator(int option, Swc_Tree_Node *start,
                             BOOL indexing) const
{
  set<Swc_Tree_Node*> blocker;
  return updateIterator(option, start, blocker, indexing);
}

int ZSwcTree::updateIterator(int option, Swc_Tree_Node *start,
                             const set<Swc_Tree_Node*> &blocker,
                             BOOL indexing) const
{
  if (isEmpty()) {
    if (m_tree->root != NULL) {
      m_tree->root->next = NULL;
    }
    return 0;
  }

  if (m_iteratorReady) {
    if (Swc_Tree_Iterator_Is_Active(option)) {
      option = SWC_TREE_ITERATOR_NO_UPDATE;
    }
    m_iteratorReady = false;
  }

  if (option == SWC_TREE_ITERATOR_VOID ||
      option == SWC_TREE_ITERATOR_NO_UPDATE ||
      option== SWC_TREE_ITERATOR_REVERSE) {
    return updateIterator(option, indexing);
  }

  if (m_tree->root == NULL) {
    m_tree->iterator = NULL;
    m_tree->begin = NULL;
    return 0;
  }

  Swc_Tree_Node *tn = NULL;
  int count = 1;

  if (start == NULL) {
    start = m_tree->root;
  }

  switch(option) {
    case 1:
      {
        m_tree->begin = start;
        tn = m_tree->begin;
        if (indexing) {
          tn->index = 0;
        }
        while (tn != NULL) {
          tn->next = NULL;
          Swc_Tree_Node *child = tn->first_child;
          //Link an unblocked child
          while (child != NULL) {
            if (blocker.count(child) == 0) {
              tn->next = child;
              break;
            }

            child = child->next_sibling;
          }

          if ((tn->next == NULL) && (tn != m_tree->begin)) { //If no child linked
            Swc_Tree_Node *sibling = tn->next_sibling;
            //Link an unblocked sibling
            while (sibling != NULL) {
              if (blocker.count(sibling) == 0) {
                tn->next = sibling;
                break;
              }

              sibling = sibling->next_sibling;
            }

            if (tn->next == NULL) { //If no sibling linked
              Swc_Tree_Node *parent_tn = tn->parent;

              //Linke an ancenster's sibling that is not blocked
              while ((parent_tn != m_tree->begin) && (parent_tn != NULL)) {
                Swc_Tree_Node *sibling = parent_tn->next_sibling;
                while (sibling != NULL) {
                  if (blocker.count(sibling) == 0) {
                    tn->next = sibling;
                    break;
                  }
                  sibling = sibling->next_sibling;
                }

                if (tn->next == NULL) {
                  parent_tn = parent_tn->parent;
                } else {
                  break;
                }
              }
            }
          }

          if (tn->next != NULL) {
            if (indexing) {
              tn->next->index = tn->index + 1;
            }
            count++;
          }
          tn = tn->next;
        }
      }
      break;
    case 2:
      {
        m_tree->begin = start;
        tn = m_tree->begin;
        if (indexing) {
          tn->index = 0;
        }
        Swc_Tree_Node *pointer = tn;
        pointer->next = NULL;
        while (pointer != NULL) {
          Swc_Tree_Node *child = pointer->first_child;
          while (child != NULL) {
            if (blocker.count(child) == 0) {
              tn->next = child;
              if (indexing) {
                tn->next->index = tn->index + 1;
              }
              count++;
              tn = tn->next;
              if (tn != NULL) {
                tn->next = NULL;
              }
            }
            child = child->next_sibling;
          }
          pointer = pointer->next;
        }
      }
      break;
    case SWC_TREE_ITERATOR_LEAF:
      {
        updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, start, blocker, FALSE);
        Swc_Tree_Node *current_node = NULL;
        Swc_Tree_Node *tn = NULL;
        Swc_Tree_Node *firstLeaf = NULL;
        count = 0;
        for (tn = const_cast<Swc_Tree_Node*>(begin());
             tn != NULL; tn = const_cast<Swc_Tree_Node*>(next())) {
          if (Swc_Tree_Node_Is_Leaf(tn)) {
            count++;
            if (current_node == NULL) {
              current_node = tn;
              firstLeaf = tn;
              if (indexing == TRUE) {
                current_node->index = 0;
              }
            } else {
              current_node->next = tn;
              if (indexing == TRUE) {
                tn->index = current_node->index + 1;
              }
              current_node = tn;
              current_node->next = NULL;
            }
          }
        }

        m_tree->begin = firstLeaf;
      }
      break;

  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  return count;
}

int ZSwcTree::swcFprint(FILE *fp, int start_id, int parent_id, double z_scale)
{
  resortId();

  updateIterator(2);

  int current_id = start_id;

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      current_id = Swc_Tree_Node_Id(tn);
      int current_pid = Swc_Tree_Node_Parent_Id(tn);

      if (parent_id >= 0) {
        if (current_pid == -1) {
          current_pid = parent_id;
        } else {
          current_pid += parent_id;
        }
        current_id += parent_id;
      }

      double z = Swc_Tree_Node_Data(tn)->z;
      if (z_scale != 1.0) {
        z /= z_scale;
      }
      fprintf(fp, "%d %d %g %g %g %g %d\n",
              current_id, Swc_Tree_Node_Data(tn)->type,
              Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
              z, Swc_Tree_Node_Data(tn)->d, current_pid);
    }
  }

  return current_id + 1;
}

void ZSwcTree::swcExport(const char *filePath)
{
  save(filePath);
}

void ZSwcTree::print(int iterOption)
{
  if (m_tree == NULL) {
    cout << "Empty tree." << endl;
  }

  if (m_tree->root == NULL) {
    cout << "Empty tree." << endl;
  }

  updateIterator(iterOption);
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    cout << SwcTreeNode::toString(tn) << endl;
  }
}

int ZSwcTree::saveAsLocsegChains(const char *prefix, int startNum)
{
  TZ_ASSERT(m_iteratorReady == false, "Iterator must be on");

  int idx = startNum;

  ZSwcTree *tree = clone();

  Swc_Tree_Node *tn;
  //Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  tree->updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  for (Swc_Tree_Node *tn = tree->begin(); tn != tree->end(); tn = tree->next()) {
    tn->node.label = 0;
  }

  tree->updateIterator(SWC_TREE_ITERATOR_LEAF);
  //Swc_Tree_Iterator_Leaf(tree);
  tn = tree->begin();
  while (tn != NULL) {
    Swc_Tree_Node *tmptn = tn;
    Locseg_Chain* nschain = New_Locseg_Chain();
    while (!Swc_Tree_Node_Is_Virtual(tmptn->parent) && tmptn->node.label == 0) {
      Local_Neuroseg* ns = Swc_Tree_Node_To_Locseg(tmptn, NULL);
      Trace_Record* tr = New_Trace_Record();
      Locseg_Chain_Add(nschain, ns, tr, DL_HEAD);
      tmptn->node.label = 1;
      tmptn = tmptn->parent;
    }
    if (nschain->list != NULL) {
      ostringstream filePathStream;
      filePathStream << prefix << idx << ".tb";
      idx++;
      Write_Locseg_Chain(filePathStream.str().c_str(), nschain);
    }
    Kill_Locseg_Chain(nschain);
    tn = tn->next;
  }

  delete tree;

  return idx;
}

bool ZSwcTree::labelBranch(double x, double y, double z, double thre)
{
  double pos[3];
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;

  if (z > -0.5) {
    Swc_Tree_Node *tn = Swc_Tree_Closest_Node(m_tree, pos);

    if (Geo3d_Dist(Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
                   Swc_Tree_Node_Data(tn)->z, pos[0], pos[1], pos[2]) <= thre) {
      if (Swc_Tree_Node_Data(tn)->label == 0) {
        Swc_Tree_Node_Label_Branch(tn, 1);
      } else {
        Swc_Tree_Node_Label_Branch(tn, 0);
      }
      return true;
    }
  } else {
    updateIterator(2);
    Swc_Tree_Node *tn = NULL;
    Swc_Tree_Node *close_tn = NULL;
    double mindist = -1.0;
    for (tn = begin(); tn != end(); tn = next()) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        double tn_pos[3];
        Swc_Tree_Node_Pos(tn, tn_pos);
        if (mindist < 0) {
          close_tn = tn;
          mindist = sqrt(dsqr(pos[0] - tn_pos[0]) + dsqr(pos[1] - tn_pos[1]));
        } else {
          double d = sqrt(dsqr(pos[0] - tn_pos[0]) + dsqr(pos[1] - tn_pos[1]));
          if (d < mindist) {
            mindist = d;
            close_tn = tn;
          }
        }
      }
    }

    if ((close_tn != NULL) && (mindist <= thre)) {
      if (Swc_Tree_Node_Data(close_tn)->label == 0) {
        Swc_Tree_Node_Label_Branch(close_tn, 1);
      } else {
        Swc_Tree_Node_Label_Branch(close_tn, 0);
      }
      return true;
    }
  }

  return false;
}

void ZSwcTree::labelBranchLevel(int rootType)
{
  Swc_Tree_Node *tn = firstRegularRoot();
  while (tn != NULL) {
    if (rootType < 0 || SwcTreeNode::type(tn) != rootType) {
      SwcTreeNode::setLabel(tn, 1);
      Swc_Tree_Node *child = tn->first_child;
      while (child != NULL) {
        SwcTreeNode::setLabel(child, 1);
        if (!SwcTreeNode::isContinuation(child)) {
          break;
        }
        child = child->first_child;
      }
    } else {
      break;
    }
    tn = tn->next_sibling;
  }

  updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);
  for (tn = begin(); tn != NULL; tn = next()) {
    if (rootType >= 0 && SwcTreeNode::type(tn) == rootType) {
      SwcTreeNode::setLabel(tn, 1);
    } else {
      if (SwcTreeNode::isRegular(tn->parent)) {
        if (SwcTreeNode::isBranchPoint(tn->parent)) {
          SwcTreeNode::setLabel(tn, SwcTreeNode::label(tn->parent) + 1);
        } else {
          SwcTreeNode::setLabel(tn, SwcTreeNode::label(tn->parent));
        }
      }
    }
  }
}

void ZSwcTree::labelBranchLevelFromLeaf()
{
  setLabel(0);

  updateIterator(SWC_TREE_ITERATOR_LEAF);

  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    int currentLabel = 1;
    Swc_Tree_Node *parent = tn;
    while (parent != NULL) {
      if (SwcTreeNode::isBranchPoint(parent)) {
        if (SwcTreeNode::label(parent) <= currentLabel) {
          SwcTreeNode::setLabel(parent, ++currentLabel);
        } else {
          break;
        }
      } else {
        SwcTreeNode::setLabel(parent, currentLabel);
      }
      parent = SwcTreeNode::parent(parent);
    }
  }
}

void ZSwcTree::boundBox(double *corner) const
{
  Swc_Tree_Bound_Box(m_tree, corner);
}

ZCuboid ZSwcTree::boundBox() const
{
  ZCuboid box;

  /*
  vector<ZPoint> box(2);
    */

  double corner[6];

  boundBox(corner);
  box.set(corner);

  //box[0].set(corner[0], corner[1], corner[2]);
  //box[1].set(corner[3], corner[4], corner[5]);

  return box;
}

ZSwcTree* ZSwcTree::createCuboidSwc(const ZCuboid &box)
{
  ZSwcTree *tree = new ZSwcTree;
  tree->forceVirtualRoot();

  const static double radius = 1.0;

  Swc_Tree_Node *parent = NULL;
  int indexArray[] = {0, 1, 2, 4, 6, 2, 4, 7, 3, 1, 2, 7, 5, 1, 4, 7};
  int index = 0;

  for (int i = 0; i < 4; ++i) {
    Swc_Tree_Node *tn =
        SwcTreeNode::makePointer(box.corner(indexArray[index++]), radius);
    SwcTreeNode::setParent(tn, tree->root());
    parent = tn;

    for (int j = 0; j < 3; ++j) {
      tn = SwcTreeNode::makePointer(box.corner(indexArray[index++]), radius);
      SwcTreeNode::setParent(tn, parent);
    }
  }

  tree->resortId();

  return tree;
}

ZSwcTree* ZSwcTree::createBoundBoxSwc(double margin)
{
  double corner[6];
  boundBox(corner);

  ZCuboid boundingBox;
  boundingBox.set(corner);

  boundingBox.expand(margin);

  ZSwcTree *tree = createCuboidSwc(boundingBox);

  return tree;
}

Swc_Tree_Node* ZSwcTree::hitTest(double x, double y, double z)
{
  if (data() != NULL) {
    return Swc_Tree_Hit_Node(data(), x, y, z);
  }

  return NULL;
}

Swc_Tree_Node* ZSwcTree::hitTest(double x, double y)
{
  if (data() != NULL) {
    return Swc_Tree_Hit_Node_P(data(), x, y);
  }

  return NULL;
}

void ZSwcTree::toSvgFile(const char *filePath)
{
  Swc_Tree_To_Svg_File(m_tree, 800, 600, filePath);
}

void ZSwcTree::translateRootTo(double x, double y, double z)
{
  double offset[3];
  Swc_Tree_Node *tn = m_tree->root;
  if (Swc_Tree_Node_Is_Virtual(tn)) {
    Swc_Tree_Node *child = tn->first_child;
    offset[0] = x - child->node.x;
    offset[1] = y - child->node.y;
    offset[2] = z - child->node.z;
  } else {
    offset[0] = x - tn->node.x;
    offset[1] = y - tn->node.y;
    offset[2] = z - tn->node.z;
  }
  updateIterator(1, FALSE);
  for (tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Data(tn)->x += offset[0];
      Swc_Tree_Node_Data(tn)->y += offset[1];
      Swc_Tree_Node_Data(tn)->z += offset[2];
    }
  }
}

void ZSwcTree::rescale(double scaleX, double scaleY, double scaleZ)
{
  if (scaleX != 1.0 || scaleY != 1.0 || scaleZ != 1.0) {
    updateIterator(1, FALSE);
    for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        Swc_Tree_Node_Data(tn)->x *= scaleX;
        Swc_Tree_Node_Data(tn)->y *= scaleY;
        Swc_Tree_Node_Data(tn)->z *= scaleZ;
        Swc_Tree_Node_Data(tn)->d *= sqrt(scaleX * scaleY);
      }
    }
  }
}

void ZSwcTree::rescale(double srcPixelPerUmXY, double srcPixelPerUmZ,
                       double dstPixelPerUmXY, double dstPixelPerUmZ)
{
  if (srcPixelPerUmXY == 0 || srcPixelPerUmZ == 0 || dstPixelPerUmXY == 0 || dstPixelPerUmZ == 0)
    return;
  double scaleX = dstPixelPerUmXY/srcPixelPerUmXY;
  double scaleY = dstPixelPerUmXY/srcPixelPerUmXY;
  double scaleZ = dstPixelPerUmZ/srcPixelPerUmZ;
  rescale(scaleX, scaleY, scaleZ);
}

void ZSwcTree::rescaleRadius(double scale, int startdepth, int enddepth)
{
  if (enddepth < 0)
    enddepth = INT_MAX;
  if (enddepth < startdepth)
    return;
  if (enddepth == startdepth)
    enddepth++;

  updateIterator(2, FALSE);
  //Swc_Tree_Iterator_Start(m_tree, 2, FALSE);
  //Swc_Tree_Node *tn = m_tree->root;
  //while ((tn = Swc_Tree_Next(m_tree)) != NULL && swcNodeDepth(tn) < enddepth) {
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (swcNodeDepth(tn) >= startdepth) {
      Swc_Tree_Node_Data(tn)->d *= scale;
    } else {
      break;
    }
  }
}

void ZSwcTree::changeRadius(double dr, double scale)
{
  updateIterator(2, FALSE);

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    SwcTreeNode::changeRadius(tn, dr, scale);
  }
}

int ZSwcTree::swcNodeDepth(Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Virtual(tn))
    return -1;
  else if (Swc_Tree_Node_Is_Root(tn))
    return 0;
  else {
    int depth = 0;
    Swc_Tree_Node *tmptn = tn;
    while (tmptn != NULL && !Swc_Tree_Node_Is_Root(tmptn)) {
      depth++;
      tmptn = tmptn->parent;
    }
    return depth;
  }
}

void ZSwcTree::reduceNodeNumber(double lengthThre)
{
  //Swc_Tree_Iterator_Start(m_tree, 1, FALSE);
  //Swc_Tree_Node *tn = m_tree->root;
  //while ((tn = Swc_Tree_Next(m_tree)) != NULL) {
  updateIterator(1, FALSE);
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Continuation(tn) &&
        Swc_Tree_Node_Length(tn) <= lengthThre) {
      Swc_Tree_Node_Merge_To_Parent(tn);
    }
  }

  deprecate(ALL_COMPONENT);
}

ZSwcTree& ZSwcTree::operator=(const ZSwcTree &other)
{
  //swap(*this, other);
  setData(other.cloneData());

  return *this;
}

ZSwcForest* ZSwcTree::toSwcTreeArray()
{
  ZSwcForest *forest = new ZSwcForest;

  if (Swc_Tree_Node_Is_Virtual(m_tree->root)) {
    Swc_Tree_Node *child = m_tree->root->first_child;
    while (child != NULL) {
      Swc_Tree_Node *sibling = child->next_sibling;
      Swc_Tree_Node_Detach_Parent(child);
      ZSwcTree *tree = new ZSwcTree;
      tree->setDataFromNode(child);
      forest->push_back(tree);
      child = sibling;
    }
  }

  return forest;
}

double ZSwcTree::distanceTo(ZSwcTree *tree, Swc_Tree_Node **source,
                            Swc_Tree_Node **target)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, false);
  double mindist = Infinity;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Leaf(tn) || Swc_Tree_Node_Is_Root(tn)) {
        Swc_Tree_Node *tmpTarget = NULL;
        double dist = Swc_Tree_Point_Dist_N(tree->m_tree, Swc_Tree_Node_X(tn),
                                            Swc_Tree_Node_Y(tn),
                                            Swc_Tree_Node_Z(tn), &tmpTarget);
        if (dist < mindist) {
          mindist = dist;
          if (source != NULL) {
            *source = tn;
            *target = tmpTarget;
          }
        }
      }
    }
  }

  return mindist;
}

double ZSwcTree::distanceTo(double x, double y, double z, double zScale, Swc_Tree_Node **node) const
{
  return Point_Tree_Distance(x, y, z, m_tree, zScale, node);
}

void ZSwcTree::resortId()
{
  Swc_Tree_Resort_Id(m_tree);
}

void ZSwcTree::flipY(double height)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    tn->node.y = height - tn->node.y;
  }
}

void ZSwcTree::removeBranch(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    bool continuing = true;
    while (continuing) {
      Swc_Tree_Node *tmpTn = tn->parent;
      continuing = Swc_Tree_Node_Is_Continuation(tmpTn);
      Swc_Tree_Node_Detach_Parent(tn);
      Kill_Swc_Tree_Node(tn);
      tn = tmpTn;
    }
  }

  deprecate(ALL_COMPONENT);
}

double ZSwcTree::computeRedundancy(Swc_Tree_Node *leaf)
{
  double redundacy = 0.0;

  if (leaf != NULL) {
    if (Swc_Tree_Node_Is_Leaf(leaf)) {
      Swc_Tree_Node *tn = leaf;
      Swc_Tree_Node *parent = leaf->parent;
      int nodeNumber = 1;
      while (Swc_Tree_Node_Is_Continuation(parent)) {
        tn = parent;
        parent = tn->parent;
        nodeNumber++;
      }

      Swc_Tree_Node_Detach_Parent(tn);
      int hitCount = 0;
      Swc_Tree_Node *tmptn = leaf;
      Swc_Tree_Iterator_Start(m_tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
      while (tmptn != NULL) {
        if (Swc_Tree_Hit_Test(m_tree, SWC_TREE_ITERATOR_NO_UPDATE,
                              Swc_Tree_Node_X(tmptn), Swc_Tree_Node_Y(tmptn),
                              Swc_Tree_Node_Z(tmptn)) > 0) {
          hitCount++;
        }
        tmptn = tmptn->parent;
      }
      Swc_Tree_Node_Set_Parent(tn, parent);
      redundacy = static_cast<double>(hitCount) / nodeNumber;
    }


  }

  return redundacy;
}

void ZSwcTree::removeRedundantBranch(double redundacyThreshold)
{
  double maxRedundancy = 1.0;
  while (maxRedundancy >= redundacyThreshold) {
    if (Swc_Tree_Iterator_Start(m_tree, SWC_TREE_ITERATOR_LEAF, TRUE) <= 1) {
      break;
    }

    vector<Swc_Tree_Node*> leafArray;
    Swc_Tree_Node *tn = NULL;
    while ((tn = Swc_Tree_Next(m_tree)) != NULL) {
      leafArray.push_back(tn);
    }

    Swc_Tree_Node *redundantBranch = NULL;
    maxRedundancy = 0.0;
    for (unsigned int i = 0; i < leafArray.size(); i++){
      tn = leafArray[i];
      double redundancy = computeRedundancy(tn);

      if (redundancy > maxRedundancy) {
        maxRedundancy = redundancy;
        redundantBranch = tn;
      }
    }

    leafArray.clear();

    if (maxRedundancy >= redundacyThreshold) {
      removeBranch(redundantBranch);
    }
  }

  deprecate(ALL_COMPONENT);
}

struct SwcNodePair {
    Swc_Tree_Node *firstNode;
    Swc_Tree_Node *secondNode;
    double first;
    double second;
};

struct SwcNodePairLessThan {
    bool operator() (const SwcNodePair &p1, const SwcNodePair &p2)
    {
        if (p1.first < p2.first) {
            return true;
        }

        return false;
    }
};

vector<int> ZSwcTree::shollAnalysis(double rStart, double rEnd, double rStep,
                                    ZPoint center)
{
  vector<SwcNodePair> distanceArray;

  vector<int> crossingNumberArray;

  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, false);
  double maxLength = 0.0;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn) && !Swc_Tree_Node_Is_Root(tn)) {
      //Compute the central distances of the current node and its parent
      SwcNodePair distancePair;
      ZPoint v1(Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),
                Swc_Tree_Node_Z(tn));
      ZPoint v2(Swc_Tree_Node_X(tn->parent),
                Swc_Tree_Node_Y(tn->parent),
                Swc_Tree_Node_Z(tn->parent));
      double d1 = v1.distanceTo(center);
      double d2 = v2.distanceTo(center);

      //Make sure that distancePair.first < distancePair.second
      if (d1 > d2) {
        distancePair.first = d2;
        distancePair.second = d1;
        distancePair.firstNode = tn->parent;
        distancePair.secondNode = tn;
      } else {
        distancePair.first = d1;
        distancePair.second = d2;
        distancePair.firstNode = tn;
        distancePair.secondNode = tn->parent;
      }

      //Calculate the distance between v1 and v2
      double length = v1.distanceTo(v2);
      if (length > maxLength) {
        maxLength = length;
      }
      distanceArray.push_back(distancePair);
    }
  }

  sort(distanceArray.begin(), distanceArray.end(),
       SwcNodePairLessThan());

  int startIndex = 0;
  int endIndex = 0;
  int lastIndex = int(distanceArray.size()) - 1;

  for (double r = rStart; r <= rEnd; r += rStep) {
    if (startIndex <= lastIndex) {
      //Update start index and end index
      while (distanceArray[startIndex].first < r - maxLength) {
        startIndex++;
        if (startIndex > lastIndex) {
          break;
        }
      }

      if (endIndex <= lastIndex) {
        while (distanceArray[endIndex].first < r) {
          endIndex++;
          if (endIndex > lastIndex) {
            break;
          }
        }
      }


      //Crossing test
      int crossingNumber = 0;
      if (startIndex <= lastIndex) {
        for (int i = startIndex; i < endIndex; ++i) {
          //If a crossing point is detected
          if (distanceArray[i].second >= r) {
            crossingNumber++;
          }
        }
      }

      crossingNumberArray.push_back(crossingNumber);
    } else {
      crossingNumberArray.push_back(0);
    }
  }

  /*
    qDebug() << "Crossing number: " << crossingNumberArray.size() << ' ' <<
                crossingNumberArray[0];
*/
  return crossingNumberArray;
}

Swc_Tree_Node* ZSwcTree::firstRegularRoot() const
{
  if (isEmpty()) {
    return NULL;
  }

  Swc_Tree_Node *root = m_tree->root;
  if (Swc_Tree_Node_Is_Virtual(m_tree->root)) {
    root = m_tree->root->first_child;
  }

  return root;
}

bool ZSwcTree::isEmpty() const
{
  if (m_tree == NULL) {
    return true;
  }

  if (m_tree->root == NULL) {
    return true;
  }

  if (SwcTreeNode::isVirtual(m_tree->root)) {
    if (SwcTreeNode::hasChild(m_tree->root)) {
      return false;
    }
  } else {
    return false;
  }

  return true;
}

Swc_Tree_Node* ZSwcTree::firstLeaf()
{
  if (m_tree == NULL) {
    return NULL;
  }

  if (m_tree->root == NULL) {
    return NULL;
  }

  Swc_Tree_Node *leaf = m_tree->root;
  while (leaf->first_child != NULL) {
    leaf = leaf->first_child;
  }

  return leaf;
}

ZSwcBranch* ZSwcTree::extractBranch(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  Swc_Tree_Node_Detach_Parent(firstRegularRoot());

  Swc_Tree_Node *begin = tn1;
  Swc_Tree_Node *end = tn2;

  Swc_Tree_Node_Set_Root(begin);

  if (Swc_Tree_Node_Is_Regular(data()->root)) {
    data()->root = begin;
  } else {
#ifdef _DEBUG_2
    cout << beginId << endl;
    cout << endId << endl;
    cout << SwcTreeNode::toString(begin) << endl;
    cout << SwcTreeNode::toString(data()->root) << endl;
#endif

    Swc_Tree_Node_Set_Parent(begin, data()->root);
  }

#ifdef _DEBUG_2
      if (!isValid()) {
        cout << "degug here" << endl;
      }
#endif

  ZSwcBranch *branch = new ZSwcBranch(begin, end);

  return branch;
}

ZSwcBranch* ZSwcTree::extractBranch(int beginId, int endId)
{
  Swc_Tree_Node *begin = Swc_Tree_Query_Node(data(), beginId,
                                             SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Node *end = Swc_Tree_Query_Node(data(), endId,
                                           SWC_TREE_ITERATOR_NO_UPDATE);

  return extractBranch(begin, end);
}

ZSwcBranch* ZSwcTree::extractBranch(int label)
{
  TZ_ASSERT(m_iteratorReady == FALSE, "Iterator must be activated");

  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  Swc_Tree_Iterator_Start(m_tree, SWC_TREE_ITERATOR_REVERSE, FALSE);

  Swc_Tree_Node *lowerEnd = NULL;
  Swc_Tree_Node *upperEnd = NULL;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Label(tn) == label) {
      if (lowerEnd == NULL) {
        lowerEnd = tn;
      }
      upperEnd = tn;
    } else if (lowerEnd != NULL) {
      if (Swc_Tree_Node_Is_Regular(upperEnd->parent)) {
        upperEnd = upperEnd->parent;
      }
      break;
    }
  }

  if ((lowerEnd == NULL) || (upperEnd == NULL)) {
    return NULL;
  }

  return (new ZSwcBranch(upperEnd, lowerEnd));
}

vector<Swc_Tree_Node*> ZSwcTree::terminalArray()
{
  updateIterator(SWC_TREE_ITERATOR_LEAF, FALSE);

  vector<Swc_Tree_Node*> leafArray;

  if (SwcTreeNode::childNumber(firstRegularRoot()) < 2) {
    leafArray.push_back(firstRegularRoot());
  }

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    leafArray.push_back(tn);
  }

  return leafArray;
}

ZSwcBranch* ZSwcTree::extractLongestBranch()
{
  //TZ_ASSERT(FALSE, "Under development");

  TZ_ASSERT(regularRootNumber() == 1, "multiple trees not supported yet");

  updateIterator(SWC_TREE_ITERATOR_LEAF, FALSE);

  vector<Swc_Tree_Node*> leafArray;
  leafArray.push_back(firstRegularRoot());

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    leafArray.push_back(tn);
  }

  Swc_Tree_Node *leaf1 = NULL;
  Swc_Tree_Node *leaf2 = NULL;
  double maxLength = -1.0;

  //Calculate the distance of all the node to the regular root
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, true);

  double *distanceArray = Swc_Tree_Accm_Length(data(), NULL);

  //Calculate distances for each pair of leaves
  for (size_t i = 0; i < leafArray.size(); i++) {
    for (size_t j = 0; j < leafArray.size(); j++) {
      if (leafArray[i] != leafArray[j]) {
        //Find the common ancestor of the leaves
        Swc_Tree_Node *ancestor = SwcTreeNode::commonAncestor(leafArray[i],
                                                              leafArray[j]);
        double length = distanceArray[SwcTreeNode::index(leafArray[i])] +
            distanceArray[SwcTreeNode::index(leafArray[j])] -
            2.0 * distanceArray[SwcTreeNode::index(ancestor)];
        if (length > maxLength) {
          maxLength = length;
          leaf1 = leafArray[i];
          leaf2 = leafArray[j];
        }
      }
    }
  }

  free(distanceArray);

  ZSwcBranch *branch = extractBranch(leaf1, leaf2);

  return branch;
}

ZSwcBranch* ZSwcTree::extractFurthestBranch()
{
  updateIterator(SWC_TREE_ITERATOR_LEAF, FALSE);

  vector<Swc_Tree_Node*> leafArray;

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    leafArray.push_back(tn);
  }

  if (Swc_Tree_Node_Child_Number(Swc_Tree_Regular_Root(m_tree)) == 1) {
    leafArray.push_back(Swc_Tree_Regular_Root(m_tree));
  }

  double maxDist = -1.0;
  Swc_Tree_Node *start = NULL;
  Swc_Tree_Node *end = NULL;

  for (size_t i = 0; i < leafArray.size(); i++) {
    for (size_t j = i + 1; j < leafArray.size(); j++) {
      double dist = Swc_Tree_Node_Dist(leafArray[i], leafArray[j]);
      if (dist > maxDist) {
        maxDist = dist;
        start = leafArray[i];
        end = leafArray[j];
      }
    }
  }

  if (start == NULL || end == NULL) {
    return NULL;
  }

  if (Swc_Tree_Node_Const_Data(start)->z >
      Swc_Tree_Node_Const_Data(end)->z) {
    Swc_Tree_Node *tmp = start;
    start = end;
    end = tmp;
  }

  ZSwcBranch *branch = extractBranch(start, end);

  return branch;
}

Swc_Tree_Node* ZSwcTree::queryNode(int id, int iterOption)
{
  return Swc_Tree_Query_Node(m_tree, id, iterOption);
}

Swc_Tree_Node* ZSwcTree::queryNode(const ZPoint &pt)
{
  double pos[3];
  pt.toArray(pos);

  return Swc_Tree_Closest_Node(data(), pos);
}

ZPoint ZSwcTree::somaCenter()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, false);
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  double w = 0.0;
  int count = 0;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Type(tn) == 1) {
      w += tn->node.d * tn->node.d;
      x += tn->node.d * tn->node.d * tn->node.x;
      y += tn->node.d * tn->node.d * tn->node.y;
      z += tn->node.d * tn->node.d * tn->node.z;
      count++;
    }
  }

  if (w == 0.0) {
    if (count > 0) {
      x /= count;
      y /= count;
      z /= count;
    }
  } else {
    x /= w;
    y /= w;
    z /= w;
  }

  return ZPoint(x, y, z);
}

std::vector<ZSwcTree*> ZSwcTree::loadTreeArray(string dirPath)
{
  ZFileList fileList;
  fileList.load(dirPath, "swc", ZFileList::SORT_ALPHABETICALLY);
  std::vector<ZSwcTree*> treeArray(fileList.size());

  for (int i = 0; i < fileList.size(); i++) {
    treeArray[i] = new ZSwcTree();
    treeArray[i]->load(fileList.getFilePath(i));
  }

  return treeArray;
}

Swc_Tree_Node* ZSwcTree::makeArrow(const ZPoint &startPos, double startSize,
                              int startType, const ZPoint &endPos,
                              double endSize, int endType, bool addBreak)
{
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  tn->node.type = startType;
  tn->node.x = startPos.x();
  tn->node.y = startPos.y();
  tn->node.z = startPos.z();
  tn->node.d = startSize;

  Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
  tn2->node.type = endType;
  tn2->node.x = endPos.x();
  tn2->node.y = endPos.y();
  tn2->node.z = endPos.z();
  tn2->node.d = endSize;

  Swc_Tree_Node_Set_Parent(tn2, tn);

  if (addBreak) {
    Swc_Tree_Node *tn3 = Swc_Tree_Node_Add_Break(tn2, 0.7);
    tn3->node.d = startSize;
    tn3->node.type = startType;
  }

  return tn;
}

vector<double> ZSwcTree::computeAllBranchingAngle()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  vector<double> angleArray;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Branch_Point(tn) && !Swc_Tree_Node_Is_Root(tn)) {
      //Needs to consider multiple children
      Swc_Tree_Node *child = tn->first_child;
      Swc_Tree_Node *anotherChild = child->next_sibling;

      double pt0[3], pt1[3], pt2[3];
      Swc_Tree_Node_Pos(tn, pt0);
      Swc_Tree_Node_Pos(child, pt1);
      Swc_Tree_Node_Pos(anotherChild, pt2);

      Coordinate_3d_Sub(pt1, pt0, pt1);
      Coordinate_3d_Sub(pt2, pt0, pt2);
      double angle = Coordinate_3d_Angle2(pt1, pt2);

      angleArray.push_back(angle);
    }
  }

  return angleArray;
}

vector<double> ZSwcTree::computeAllContinuationAngle(bool rotating)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  vector<double> angleArray;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Continuation(tn)) { //Needs to consider multiple children
      Swc_Tree_Node *p = tn->parent;
      Swc_Tree_Node *child = tn->first_child;

      double pt0[3], pt1[3], pt2[3];
      Swc_Tree_Node_Pos(p, pt0);
      Swc_Tree_Node_Pos(tn, pt1);
      Swc_Tree_Node_Pos(child, pt2);

      double vec1[3], vec2[3];
      Coordinate_3d_Sub(pt1, pt0, vec1);
      Coordinate_3d_Sub(pt2, pt1, vec2);

      if (!rotating) {
        double angle = Coordinate_3d_Angle2(vec1, vec2);
        angleArray.push_back(angle);
      } else {
        double theta, psi;
        Geo3d_Coord_Orientation(vec1[0], vec1[1], vec1[2], &theta, &psi);
        Geo3d_Rotate_Coordinate(vec2, vec2 + 1, vec2 + 2, theta, psi, 1);
        Geo3d_Coord_Orientation(vec2[0], vec2[1], vec2[2], &theta, &psi);

        /*
        if (fabs(psi - TZ_PI_2) < GEOANGLE_COMPARE_EPS) {
          qDebug() << vec2[0] << " " << vec2[1] << " " << vec2[2];
        }
        */

        angleArray.push_back(theta);
        angleArray.push_back(psi);
      }
    }
  }

  return angleArray;
}

static void GenerateRandomPos(const double *targetVec, double theta,
                              const double *orgPos, const double *prevPos,
                              const double *currPos, double *pos)
{
  double bestScore = 0.0;

  double currOrt[3];
  Coordinate_3d_Sub(currPos, prevPos, currOrt);

  coordinate_3d_t tmp_pos;

  for (int i = 0; i < 4; i++) {
    double psi = Unifrnd() * TZ_2PI;
    tmp_pos[0] = 0.0;
    tmp_pos[1] = 0.0;
    tmp_pos[2] = 5.0;

    Geo3d_Rotate_Coordinate(tmp_pos, tmp_pos + 1, tmp_pos + 2, theta, psi, 0);

    double newTheta;
    Geo3d_Coord_Orientation(currOrt[0], currOrt[1], currOrt[2], &newTheta, &psi);
    Geo3d_Rotate_Coordinate(tmp_pos, tmp_pos + 1, tmp_pos + 2, newTheta, psi, 0);
    Coordinate_3d_Add(tmp_pos, currPos, tmp_pos);

    coordinate_3d_t currVec;
    Coordinate_3d_Sub(tmp_pos, orgPos, currVec);
    double score = Coordinate_3d_Dot(currVec, targetVec);

    if (i == 0) {
      Coordinate_3d_Copy(pos, tmp_pos);
    } else {
      if (score > bestScore) {
        bestScore = score;
        Coordinate_3d_Copy(pos, tmp_pos);
      }
    }
  }
}

bool ZSwcTree::isValid()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  for (Swc_Tree_Node *tn = begin(); tn!= end(); tn = next()) {
    if (tn == tn->parent) {
      return false;
    }
  }

  return true;
}

Swc_Tree_Node* ZSwcTree::removeRandomBranch()
{
  int n = updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, 1);
  //Generate an inter
  ZRandomGenerator rnd;
  int rn = rnd.rndint(n);

  Swc_Tree_Node *detachedNode = NULL;
  for (Swc_Tree_Node *tn = begin(); tn!= end(); tn = next()) {
    if (tn->index == rn) {
      Swc_Tree_Node_Detach_Parent(tn);

      detachedNode = tn;
    }
  }

  deprecate(ALL_COMPONENT);

  return detachedNode;
}

ZSwcTree* ZSwcTree::generateRandomSwcTree(int n, double branchingProb,
                                     double contAngleMu, double contAngleSigma,
                                     double branchAngleMu,
                                     double branchAngleSigma)
{
  std::vector<Swc_Tree_Node*> terminalArray;
  std::vector<ZPoint> targetVectorArray;
  std::vector<ZPoint> branchOrgArray;

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  tn->node.x = 0;
  tn->node.y = 0;
  tn->node.z = 0;
  tn->node.d = 3.0;

  Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
  tn2->node.x = 0;
  tn2->node.y = 0;
  tn2->node.z = 5.0;
  tn2->node.d = 3.0;

  Swc_Tree_Node_Set_Parent(tn2, tn);

  ZSwcTree *tree = new ZSwcTree();
  tree->setDataFromNode(tn);

  double generatedNumber = 2;
  Swc_Tree_Node *currTn = tn2;
  terminalArray.push_back(tn2);
  targetVectorArray.push_back(ZPoint(0.0, 0.0, 5.0));
  branchOrgArray.push_back(ZPoint(0.0, 0.0, 0.0));

  while (generatedNumber < n) {
    double x = Unifrnd();
    bool branching = false;
    if (x < branchingProb) {
      branching = true;
    }

    int index = Unifrnd_Int(terminalArray.size() - 1);
    currTn = terminalArray[index];

    if (branching) {
      double theta = Normrnd(branchAngleMu, branchAngleSigma);
      theta *= 1 - Bernrnd(0.5) * 2;
      double psi = Unifrnd() * TZ_2PI;
      double pos[3];
      pos[0] = 0.0;
      pos[1] = 0.0;
      pos[2] = 5.0;
      Geo3d_Rotate_Coordinate(pos, pos + 1, pos + 2, theta, psi, 0);

      double currOrt[3];
      double currPos[3];
      Swc_Tree_Node_Pos(currTn, currPos);

      double prevPos[3];
      Swc_Tree_Node_Pos(currTn->parent, prevPos);
      Coordinate_3d_Sub(currPos, prevPos, currOrt);

      Geo3d_Coord_Orientation(currOrt[0], currOrt[1], currOrt[2], &theta, &psi);
      Geo3d_Rotate_Coordinate(pos, pos + 1, pos + 2, theta, psi, 0);

      Swc_Tree_Node_Pos(currTn->parent, currPos);

      targetVectorArray.push_back(ZPoint(pos[0], pos[1], pos[2]));
      Coordinate_3d_Add(pos, currPos, pos);
      tn = New_Swc_Tree_Node();
      Swc_Tree_Node_Set_Pos(tn, pos);
      Swc_Tree_Node_Set_Parent(tn, currTn->parent);
      terminalArray.push_back(tn);
      branchOrgArray.push_back(ZPoint(prevPos[0], prevPos[1], prevPos[2]));
    } else {
      double theta = Normrnd(contAngleMu, contAngleSigma);
      theta *= 1 - Bernrnd(0.5) * 2;
      /*
      double psi = Unifrnd() * 0.3;
      psi *= 1 - Bernrnd(0.5) * 2;

      double pos[3];
      pos[0] = 0.0;
      pos[1] = 0.0;
      pos[2] = 5.0;

      Geo3d_Rotate_Coordinate(pos, pos + 1, pos + 2, theta, psi, 0);

      double currOrt[3];
      double currPos[3];
      Swc_Tree_Node_Pos(currTn, currPos);

      double prevPos[3];
      Swc_Tree_Node_Pos(currTn->parent, prevPos);
      Coordinate_3d_Sub(currPos, prevPos, currOrt);


      Geo3d_Coord_Orientation(currOrt[0], currOrt[1], currOrt[2], &theta, &psi);
      Geo3d_Rotate_Coordinate(pos, pos + 1, pos + 2, theta, psi, 0);
      Coordinate_3d_Add(pos, currPos, pos);
      */
      double targetVec[3];
      double pos[3];
      double currPos[3];
      Swc_Tree_Node_Pos(currTn, currPos);
      double prevPos[3];
      Swc_Tree_Node_Pos(currTn->parent, prevPos);
      double orgPos[3];
      branchOrgArray[index].toArray(orgPos);
      targetVectorArray[index].toArray(targetVec);

      //qDebug() << "target: " << targetVec[0] << ", " << targetVec[1] << ", "
       //        << targetVec[2];

      ::GenerateRandomPos(targetVec, theta, orgPos, prevPos, currPos, pos);

      tn = New_Swc_Tree_Node();
      Swc_Tree_Node_Set_Pos(tn, pos);
      Swc_Tree_Node_Set_Parent(tn, currTn);
      terminalArray[index] = tn;
    }

    generatedNumber++;
  }

  return tree;
}

void ZSwcTree::moveToSurface(double *x, double *y, double *z)
{
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Point_Dist_N(m_tree, *x, *y, *z, &tn);

  ZPoint pt(*x, *y, *z);

  pt -= ZPoint(tn->node.x, tn->node.y, tn->node.z);
  pt.normalize();
  if (pt.isApproxOrigin()) {
    pt.set(1.0, 0.0, 0.0);
  }
  pt *= tn->node.d;
  pt += ZPoint(tn->node.x, tn->node.y, tn->node.z);

  *x = pt.x();
  *y = pt.y();
  *z = pt.z();
}

void ZSwcTree::moveToSurface(ZPoint *pt)
{
  moveToSurface(pt->xRef(), pt->yRef(), pt->zRef());
}

Swc_Tree* ZSwcTree::cloneData() const
{
  return Copy_Swc_Tree(data());
}

ZSwcTree* ZSwcTree::clone() const
{
  ZSwcTree *tree = new ZSwcTree;
  tree->setData(cloneData(), CLEAN_ALL);
  tree->m_iteratorReady = m_iteratorReady;
  tree->m_source = m_source;
  tree->setColor(getColor());

  return tree;
}

void ZSwcTree::merge(Swc_Tree *tree, bool freeInput)
{
  if (tree != NULL) {
    if (data() == NULL) {
      m_tree = New_Swc_Tree();
    }

    Swc_Tree_Merge(data(), tree);

    if (freeInput) {
      free(tree);
    }

    deprecate(ALL_COMPONENT);
  }
}

void ZSwcTree::merge(ZSwcTree *tree, bool freeInput)
{
  if (tree != NULL) {
    Swc_Tree *data = tree->data();
    tree->setData(NULL, ZSwcTree::LEAVE_ALONE);
    merge(data, true);
    if (freeInput) {
      delete tree;
    }
  }
}

void ZSwcTree::setLabel(int v)
{
  Swc_Tree_Set_Label(data(), v);
}

void ZSwcTree::setType(int type)
{
  Swc_Tree_Set_Type(data(), type);
}

void ZSwcTree::setTypeByLabel()
{
  Swc_Tree_Set_Type_As_Label(data());
}

vector<Swc_Tree_Node*> ZSwcTree::extractLeaf(Swc_Tree_Node *start)
{
  vector<Swc_Tree_Node*> leafArray;

  if (Swc_Tree_Node_Is_Leaf(start)) {
    leafArray.push_back(start);
  } else {
    Swc_Tree_Node *tn = start->next;
    while (tn != NULL) {
      if (Swc_Tree_Node_Is_Leaf(tn)) {
        if (Swc_Tree_Node_Is_Upstream(start, tn) == TRUE) {
          leafArray.push_back(tn);
        } else {
          break;
        }
      }
      tn = tn->next;
    }
  }

  return leafArray;
}

void ZSwcTree::translate(double x, double y, double z)
{
  Swc_Tree_Translate(data(), x, y, z);
}

void ZSwcTree::translate(const ZPoint &offset)
{
  Swc_Tree_Translate(data(), offset.x(), offset.y(), offset.z());
}

void ZSwcTree::scale(double x, double y, double z)
{
  Swc_Tree_Resize(data(), x, y, z, FALSE);
}

void ZSwcTree::rotate(double theta, double psi, const ZPoint &center)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    SwcTreeNode::rotate(tn, theta, psi, center);
  }
}

void ZSwcTree::resample(double step)
{
  /* Label the branches */
  int n = Swc_Tree_Label_Branch_All(data());

  for (int i = 1; i <= n; i++) {
    ZSwcBranch *branch = extractBranch(i);
    branch->resample(step);
  }

  deprecate(ALL_COMPONENT);
}

void ZSwcTree::labelTrunkLevel(ZSwcTrunkAnalyzer *trunkAnalyzer)
{
  setLabel(0);
  ZSwcPath branch = mainTrunk(trunkAnalyzer);

#ifdef _DEBUG_2
  cout << "Main turnk: " << endl;
  branch.print();
#endif

  branch.label(1);

  SwcTreeNode::setAsRoot(branch[0]);
  setDataFromNodeRoot(branch[0], LEAVE_ALONE);


  queue<Swc_Tree_Node*> seedArray;

  set<Swc_Tree_Node*> blocker;

  //Push all branch points in the branch into the seed set
  for (ZSwcPath::iterator iter = branch.begin(); iter != branch.end(); ++iter)
  {
    if (SwcTreeNode::isBranchPoint(*iter)) {
      seedArray.push(*iter);
    }

     blocker.insert(*iter);
  }

  //While the seed set is not empty
  while (!seedArray.empty()) {
#ifdef _DEBUG_2
    cout << "Seed number: " << seedArray.size() << endl;
#endif

    //Pop a seed from the set
    Swc_Tree_Node *tn = seedArray.front();
    seedArray.pop();

#ifdef _DEBUG_2
    cout << "Seed: " << SwcTreeNode::toString(tn) << endl;
#endif

    //Extract the trunk starting from the seed
    branch = subTrunk(tn, blocker, trunkAnalyzer);

#ifdef _DEBUG_2
    cout << "Trunk: " << endl;
    branch.print();
#endif

    //Label a trunk starting from the seed with its level plus 1
    branch.label(SwcTreeNode::label(tn) + 1);
    tn->node.label--;

    //Push all uncompleted nodes in the trunk into the seed set
    for (ZSwcPath::iterator iter = branch.begin(); iter != branch.end(); ++iter)
    {
      if (*iter != tn) {
        if (SwcTreeNode::isBranchPoint(*iter)) {
          seedArray.push(*iter);
        }

        blocker.insert(*iter);
      }
    }

#if _DEBUG_2
  return;
#endif
  }
}

bool ZSwcTree::contains(Swc_Tree_Node *tn) const
{
  return (data()->root == SwcTreeNode::root(tn));
}

int ZSwcTree::regularRootNumber()
{
  int n = 0;

  Swc_Tree_Node *root = firstRegularRoot();
  while (root != NULL) {
    n++;
    root = root->next_sibling;
  }

  return n;
}

int ZSwcTree::regularDepth()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  int maxLabel = 0;

  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (!SwcTreeNode::isRegular(tn)) {
      SwcTreeNode::setLabel(tn, 0);
    } else {
      SwcTreeNode::setLabel(tn, SwcTreeNode::label(tn->parent) + 1);
    }

    if (maxLabel < SwcTreeNode::label(tn)) {
      maxLabel = SwcTreeNode::label(tn);
    }
  }

  return maxLabel;
}

ZSwcPath ZSwcTree::mainTrunk(ZSwcTrunkAnalyzer *trunkAnalyzer)
{
  //TZ_ASSERT(regularRootNumber() == 1, "multiple trees not supported yet");

  ZSwcPath path;
  if (regularRootNumber() == 1) {
    path = trunkAnalyzer->extractMainTrunk(this);
  } else {
    ZSwcTree *tree = clone();
    Swc_Tree_Node *root = firstRegularRoot();
    int maxSize = SwcTreeNode::downstreamSize(root);
    Swc_Tree_Node *tn = SwcTreeNode::nextSibling(root);
    while (tn != NULL) {
      int s = SwcTreeNode::downstreamSize(tn);
      Swc_Tree_Node *next = SwcTreeNode::nextSibling(tn);
      if (s > maxSize) {
        maxSize = s;
        SwcTreeNode::killSubtree(root);
        root = tn;
      } else {
        SwcTreeNode::killSubtree(tn);
      }
      tn = next;
    }
    path = trunkAnalyzer->extractMainTrunk(tree);
  }

  return path;
}


ZSwcPath ZSwcTree::subTrunk(Swc_Tree_Node *start, int label)
{
  Swc_Tree_Node *leaf = Swc_Tree_Node_Furthest_Leaf_L(start, label);

  return ZSwcPath(start, leaf);
}

ZSwcPath ZSwcTree::subTrunk(Swc_Tree_Node *start,
                            const std::set<Swc_Tree_Node*> &blocker,
                            ZSwcTrunkAnalyzer *trunkAnalyzer)
{
  trunkAnalyzer->setBlocker(blocker);
  ZSwcPath path = trunkAnalyzer->extractTrunk(this, start);

  return path;
}

vector<ZSwcPath> ZSwcTree::getBranchArray()
{
  vector<ZSwcPath> branchArray;
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  ZSwcPath path;
  int state = 0; //0 - start; 1 - end; 2 - cont;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (SwcTreeNode::isRegular(tn)) {
      switch(state) {
      case 2:
        if (!SwcTreeNode::isContinuation(tn)) {
          state = 1;
        }
        break;
      case 0:
        if (!SwcTreeNode::isRegularRoot(tn)) {
          path.push_back(tn->parent);
        }
        break;
      default:
        break;
      }

      path.push_back(tn);

      switch(state) {
      case 0:
        state = 2;
        break;
      case 1:
        branchArray.push_back(path);
        path.clear();
        state = 0;
        break;
      default:
        break;
      }
    }  
  }

  if (!path.empty()) {
    branchArray.push_back(path);
  }

  return branchArray;
}

Swc_Tree_Node* ZSwcTree::maxLabelNode()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  Swc_Tree_Node *target = begin();
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (SwcTreeNode::label(tn) > SwcTreeNode::label(target)) {
      target = tn;
    }
  }

  return target;
}

void ZSwcTree::addRegularRoot(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (m_tree == NULL) {
      m_tree = New_Swc_Tree();
    }

    if (m_tree->root == NULL) {
      m_tree->root = SwcTreeNode::makeVirtualNode();
    } else if (SwcTreeNode::isRegular(m_tree->root)) {
      Swc_Tree_Node *root = SwcTreeNode::makeVirtualNode();
      SwcTreeNode::setParent(m_tree->root, root);
      m_tree->root = root;
    }

    SwcTreeNode::setParent(tn, m_tree->root);

    deprecate(ALL_COMPONENT);
  }
}

void ZSwcTree::forceVirtualRoot()
{
  if (m_tree == NULL) {
    m_tree = New_Swc_Tree();
  }

  if (m_tree->root == NULL) {
    m_tree->root = SwcTreeNode::makeVirtualNode();
  }

  if (SwcTreeNode::isRegular(m_tree->root)) {
    Swc_Tree_Node *tn = SwcTreeNode::makeVirtualNode();
    SwcTreeNode::setParent(m_tree->root, tn);
    m_tree->root = tn;
  }

  deprecate(ALL_COMPONENT);
}

void ZSwcTree::setBranchSizeWeight()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);

  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    if (SwcTreeNode::isBranchPoint(tn) || SwcTreeNode::isLeaf(tn)) {
      double weight = SwcTreeNode::radius(tn);
      int count = 1;
      Swc_Tree_Node *branchNode = tn->parent;
      while (SwcTreeNode::isContinuation(branchNode)) {
        weight += SwcTreeNode::radius(tn);
        count++;
        branchNode = SwcTreeNode::parent(branchNode);
      }

      weight /= count;
      SwcTreeNode::setWeight(tn, weight);
      branchNode = tn->parent;
      while (SwcTreeNode::isContinuation(branchNode)) {
        SwcTreeNode::setWeight(branchNode, weight);
        branchNode = SwcTreeNode::parent(branchNode);
      }
    }
  }
}

double ZSwcTree::length()
{
  return Swc_Tree_Overall_Length(data());
}

double ZSwcTree::length(int type)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  double length = 0.0;
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (SwcTreeNode::type(tn) == type) {
      length += Swc_Tree_Node_Length(tn);
    }
  }

  return length;
}

set<int> ZSwcTree::typeSet()
{
  set<int> allTypes;

  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      allTypes.insert(SwcTreeNode::type(tn));
    }
  }

  return allTypes;
}

const std::string& ZSwcTree::className() const {
  static const std::string name = "ZSwcTree";

  return name;
}

vector<Swc_Tree_Node*> ZSwcTree::toSwcTreeNodeArray(bool includingVirtual)
{
  vector<Swc_Tree_Node*> nodeArray;

  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
    if (Swc_Tree_Node_Is_Regular(tn) || includingVirtual) {
      nodeArray.push_back(tn);
    }
  }

  return nodeArray;
}

bool ZSwcTree::isDeprecated(EComponent component) const
{
  switch (component) {
  case DEPTH_FIRST_ARRAY:
    return m_depthFirstArray.empty();
  case BREADTH_FIRST_ARRAY:
    return m_breadthFirstArray.empty();
  case LEAF_ARRAY:
    return m_leafArray.empty();
  case TERMINAL_ARRAY:
    return m_terminalArray.empty();
  case BRANCH_POINT_ARRAY:
    return m_branchPointArray.empty();
  case Z_SORTED_ARRAY:
    return m_zSortedArray.empty();
  default:
    break;
  }

  return true;
}

void ZSwcTree::deprecateDependent(EComponent component)
{
  switch (component) {
  case DEPTH_FIRST_ARRAY:
    deprecate(LEAF_ARRAY);
    deprecate(BRANCH_POINT_ARRAY);
    deprecate(TERMINAL_ARRAY);
    deprecate(Z_SORTED_ARRAY);
    break;
  case BREADTH_FIRST_ARRAY:
    break;
  default:
    break;
  }
}

void ZSwcTree::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case DEPTH_FIRST_ARRAY:
    m_depthFirstArray.clear();
    break;
  case BREADTH_FIRST_ARRAY:
    m_breadthFirstArray.clear();
    break;
  case LEAF_ARRAY:
    m_leafArray.clear();
    break;
  case TERMINAL_ARRAY:
    m_terminalArray.clear();
    break;
  case BRANCH_POINT_ARRAY:
    m_branchPointArray.clear();
    break;
  case Z_SORTED_ARRAY:
    m_zSortedArray.clear();
    break;
  case ALL_COMPONENT:
    deprecate(DEPTH_FIRST_ARRAY);
    deprecate(BREADTH_FIRST_ARRAY);
    break;
  default:
    break;
  }
}

const std::vector<Swc_Tree_Node *> &ZSwcTree::getSwcTreeNodeArray(
    EIteratorOption iteratorOption) const
{
  switch (iteratorOption) {
  case DEPTH_FIRST_ITERATOR:
    if (isDeprecated(DEPTH_FIRST_ARRAY)) {
      updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
      for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
        m_depthFirstArray.push_back(tn);
      }
    }
    return m_depthFirstArray;
  case BREADTH_FIRST_ITERATOR:
    if (isDeprecated(BREADTH_FIRST_ARRAY)) {
      updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);
      for (Swc_Tree_Node *tn = begin(); tn != end(); tn = next()) {
        m_breadthFirstArray.push_back(tn);
      }
    }
    return m_breadthFirstArray;
  case LEAF_ITERATOR:
    if (isDeprecated(LEAF_ARRAY)) {
      const vector<Swc_Tree_Node*> &depthFirstArray =
          getSwcTreeNodeArray(DEPTH_FIRST_ITERATOR);
      for (vector<Swc_Tree_Node*>::const_iterator iter = depthFirstArray.begin();
           iter != depthFirstArray.end(); ++iter) {
        if (SwcTreeNode::isLeaf(*iter)) {
          m_leafArray.push_back(*iter);
        }
      }
    }
    return m_leafArray;
  case TERMINAL_ITERATOR:
    if (isDeprecated(TERMINAL_ARRAY) && !isEmpty()) {
      const vector<Swc_Tree_Node*> &depthFirstArray =
          getSwcTreeNodeArray(DEPTH_FIRST_ITERATOR);

      Swc_Tree_Node *root = firstRegularRoot();
      while (root != NULL) {
        if (SwcTreeNode::childNumber(root) < 2) {
          m_terminalArray.push_back(root);
        }
        root = SwcTreeNode::nextSibling(root);
      }

      for (vector<Swc_Tree_Node*>::const_iterator iter = depthFirstArray.begin();
           iter != depthFirstArray.end(); ++iter) {
        if (SwcTreeNode::isLeaf(*iter)) {
          m_terminalArray.push_back(*iter);
        }
      }
    }
    return m_terminalArray;
  case BRANCH_POINT_ITERATOR:
    if (isDeprecated(BRANCH_POINT_ARRAY)) {
      const vector<Swc_Tree_Node*> &depthFirstArray =
          getSwcTreeNodeArray(DEPTH_FIRST_ITERATOR);
      for (vector<Swc_Tree_Node*>::const_iterator iter = depthFirstArray.begin();
           iter != depthFirstArray.end(); ++iter) {
        if (SwcTreeNode::isBranchPoint(*iter)) {
          m_branchPointArray.push_back(*iter);
        }
      }
    }
    return m_branchPointArray;
  case Z_SORT_ITERATOR:
    if (isDeprecated(Z_SORTED_ARRAY)) {
      const vector<Swc_Tree_Node*> &depthFirstArray =
          getSwcTreeNodeArray(DEPTH_FIRST_ITERATOR);
      m_zSortedArray = depthFirstArray;
      vector<Swc_Tree_Node*>::iterator firstIter = m_zSortedArray.begin();

      if (SwcTreeNode::isVirtual(*firstIter)) {
        ++firstIter;
      }

      //sort by z
      sort(firstIter, m_zSortedArray.end(), SwcTreeNode::lessThanZ);
    }

    return m_zSortedArray;
  default:
    return getSwcTreeNodeArray(DEPTH_FIRST_ITERATOR);
  }
}

bool ZSwcTree::hasGoodSourceName()
{
  if (ZFileType::fileType(source()) == ZFileType::SWC_FILE) {
    return true;
  }

  return false;
}

void ZSwcTree::labelStack(Stack *stack)
{
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);
  for (Swc_Tree_Node *iter = begin(); iter != NULL; iter = next()) {
    if (SwcTreeNode::isRegular(iter)) {
      Swc_Tree_Node_Label_Stack(iter, stack, &workspace);
    }
  }
}

double ZSwcTree::computeBackTraceLength()
{
  if (isEmpty()) {
    return 0.0;
  }

  updateIterator(SWC_TREE_ITERATOR_BREADTH_FIRST);
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    SwcTreeNode::setWeight(tn, 0);
  }

  updateIterator(SWC_TREE_ITERATOR_REVERSE);

  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    double length = SwcTreeNode::length(tn);
    SwcTreeNode::addWeight(SwcTreeNode::parent(tn),
                           length + SwcTreeNode::weight(tn));
  }

  return SwcTreeNode::weight(m_tree->root);
}

void ZSwcTree::labelSubtree(Swc_Tree_Node *tn, int label)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, tn, 0);
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    SwcTreeNode::setLabel(tn, label);
  }
}

void ZSwcTree::addLabelSubtree(Swc_Tree_Node *tn, int label)
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, tn, 0);
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    SwcTreeNode::addLabel(tn, label);
  }
}

double ZSwcTree::getMaxSegmentLenth()
{
  updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
  double maxLength = 0.0;
  double length = 0.0;

  bool isStart = true;
  bool isEnd = false;
  for (Swc_Tree_Node *tn = begin(); tn != NULL; tn = next()) {
    if (isStart) {
      length = 0.0;
    }
    length += SwcTreeNode::length(tn);

    if (SwcTreeNode::isLeaf(tn) || SwcTreeNode::isBranchPoint(tn)) {
      isEnd = true;
    }

    if (isEnd) {
      if (maxLength < length) {
        maxLength = length;
      }
      isStart = true;
      isEnd = false;
    }
  }

  return maxLength;
}
