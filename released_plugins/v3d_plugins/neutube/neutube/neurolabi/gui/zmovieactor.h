#ifndef ZMOVIEACTOR_H
#define ZMOVIEACTOR_H

#include "zpoint.h"

class ZMovieStage;

/**
 * @brief The ZMovieActor class
 *
 * The class has a reset function to be called when the movie enters a new
 * scene. The reset function is to reset some properties of an actor. Currently
 * the properties include color and alpha. A property itself may be passed from
 * the actor wrapper to real data. These are impletemnted in pull<Property>/
 * push<Property> functions. A take<Property> funcion only buffers a property
 * without passing it. A set<Property> function performs both take and push
 * steps. take<Property> and set<Property> are the same if the property only
 * exists in the wrapper.
 */

class ZMovieActor
{
public:
  ZMovieActor();
  virtual ~ZMovieActor();

public:
  enum EType {
    UNKNOWN, SWC, PUNCTA, STACK
  };

  virtual void show() = 0;
  virtual void hide() = 0;
  virtual void move(double t) = 0;

  inline void setId(const std::string &id) { m_id = id; }
  inline std::string getId() const { return m_id; }

  inline void setGroupId(const std::string &id) {m_groupId = id; }
  inline std::string getGroupId() const { return m_groupId; }

  inline void setVisible(bool visible) { m_isVisible = visible; }
  void setMovingOffset(double x, double y, double z);
  void setMovingOffset(const ZPoint &offset);
  bool isMoving();
  inline bool isActive() { return m_isActive; }
  inline void setActive(bool active) { m_isActive = active; }

  virtual void reset() = 0;
  virtual void perform(double t);

  inline void setStage(ZMovieStage *stage) { m_stage = stage; }

  void print();

  void takeColor(double red, double green, double blue);
  void takeAlpha(double alpha);

  void setColor(double red, double green, double blue);
  void setAlpha(double alpha);

  void addColor(double dr, double dg, double db);
  void addAlpha(double da); //<da> can be negative to reduce alpha

  virtual void pushColor() = 0;
  virtual void pushAlpha() = 0;
  virtual void pullColor() = 0;
  virtual void pullAlpha() = 0;

  inline void setFadingFactor(double f) { m_fadingFactor = f; }
  inline void setTransitFactor(double dr, double dg, double db) {
    m_transitFactor[0] = dr;
    m_transitFactor[1] = dg;
    m_transitFactor[2] = db;
  }

  inline ZMovieStage* getStage() {
    return m_stage;
  }

protected:
  bool m_isVisible;
  bool m_isActive;
  ZPoint m_movingOffset;
  double m_fadingFactor;
  double m_transitFactor[3];
  EType m_type;
  std::string m_id;
  std::string m_groupId;
  //Z3DWindow *m_stage;
  ZMovieStage *m_stage;

  //colors ([0, 1])
  double m_red;
  double m_green;
  double m_blue;
  double m_alpha;
};

#endif // ZMOVIEACTOR_H
