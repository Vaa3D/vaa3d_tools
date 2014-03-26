#ifndef ZACTIONACTIVATOR_H
#define ZACTIONACTIVATOR_H

#include <QAction>
#include <QMenu>
#include <QSet>

class ZStackFrame;
class ZStackDoc;
class ZStackPresenter;
class Z3DWindow;

/*!
 * \brief A class of managing actions
 */
class ZActionActivator
{
public:
  ZActionActivator();
  virtual ~ZActionActivator();

  /*!
   * \brief Test if the current doc context is positive
   */
  virtual bool isPositive(const ZStackFrame *frame) const;
  virtual bool isPositive(const ZStackDoc *doc) const;
  virtual bool isPositive(const ZStackPresenter *presenter) const;
  virtual bool isPositive(const Z3DWindow *window) const;

  void update(bool positive);
  void update(const ZStackFrame *frame);
  void update(const ZStackDoc *doc);
  void update(const ZStackPresenter *presenter);
  void update(const Z3DWindow *window);
  void registerAction(QAction *action, bool positive);

private:
  QSet<QAction*> m_postiveActionList;
  QSet<QAction*> m_negativeActionList;
};


/////////////ZStackActionActivator////////////////////
class ZStackActionActivator : public ZActionActivator
{
public:
  ZStackActionActivator();
  ~ZStackActionActivator();

public:
  bool isPositive(const ZStackDoc *doc) const;
};

//////////ZSingleSwcNodeActionActivator/////////////////
class ZSingleSwcNodeActionActivator : public ZActionActivator
{
public:
  ZSingleSwcNodeActionActivator();
  ~ZSingleSwcNodeActionActivator();

public:
  bool isPositive(const ZStackDoc *doc) const;
};

#endif // ZACTIONACTIVATOR_H
