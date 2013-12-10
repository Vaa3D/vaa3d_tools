#ifndef ZPROGRESSREPORTER_H
#define ZPROGRESSREPORTER_H

/* Interface for reporting progress

Usage:

  ZProgressReporter reporter new ZProgressReporter
  reporter->start();
  reporter->advanceTo(0.1);
  reporter->advance(0.1);
  reporter->start(0.5);
  reporter->advance(0.5);
  reporter->advance(0.5);
  reporter->end(0.5); //Values for the matched start and end must be equal.
  reporter->end();
*/
class ZProgressReporter
{
public:
  ZProgressReporter();
  virtual ~ZProgressReporter() {}

  /*!
   * \brief Start progress
   */
  void start();
  void end();
  void start(double scale);
  void end(double scale);
  void update(double progress);
  void startSubprogress(double scale);
  void endSubprogress(double scale);
  void advance(double dp);
  inline double getProgress() { return m_progress; }
  //void advanceTo(double progress); //progress from 0 to 1

  virtual void open();
  virtual void close();
  virtual void push();
  virtual void pull();

protected:
  double m_progress;
  double m_scale;
  int m_sectionCount;
};

#endif // ZPROGRESSREPORTER_H
