#ifndef ZSTACKDOCMENUFACTORY_H
#define ZSTACKDOCMENUFACTORY_H

class QMenu;
class QAction;
class ZStackDoc;
class ZStackPresenter;

#include "neutube.h"

/*!
 * \brief Class of creating menus for ZStackDoc
 */
class ZStackDocMenuFactory
{
public:
  ZStackDocMenuFactory();

public:
  static QMenu* makeSwcNodeContextMenu(const ZStackDoc *doc, QMenu *menu = NULL);
  static QMenu* makeSwcNodeContextMenu(
      const ZStackPresenter *presenter, QMenu *menu = NULL);
  static QMenu* makeSrokePaintContextMenu(
      const ZStackPresenter *presenter, QMenu *menu = NULL);
};

#endif // ZSTACKDOCMENUFACTORY_H
