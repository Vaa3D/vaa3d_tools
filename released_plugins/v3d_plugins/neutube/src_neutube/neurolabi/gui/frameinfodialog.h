/**@file frameinfodialog.h
 * @brief Frame information dialog
 * @author Ting Zhao
 */
#ifndef FRAMEINFODIALOG_H
#define FRAMEINFODIALOG_H

#include <QDialog>

#include "ui_frameinfodialog.h"
#include "plotsettings.h"

class Plotter;
class ZCurve;

class FrameInfoDialog : public QDialog
               , public Ui_FrameInfoDialog
{
  Q_OBJECT

public:
  FrameInfoDialog(QWidget *parent = 0);
  virtual ~FrameInfoDialog();

  void setText(const QString &info);
  void setCurve(const ZCurve &curve);

  inline PlotSettings *plotSettings() { return &m_plotSettings; }

  void updatePlotSettings();

  int curveOption();
signals:
  void newCurveSelected(int index);

private:
  Plotter *m_curveWidget;
  PlotSettings m_plotSettings;
};

#endif
