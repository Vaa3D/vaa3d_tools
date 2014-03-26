#ifndef ZMOVIESTAGE_H
#define ZMOVIESTAGE_H

#include <string>

class Z3DWindow;

class ZMovieStage
{
public:
  explicit ZMovieStage(Z3DWindow *window = 0x0);

  inline Z3DWindow *getWindow() {
    return m_window;
  }

  inline void setWindow(Z3DWindow *window) {
    m_window = window;
  }

  inline bool isSwcChanged() {
    return m_isSwcChanged;
  }

  inline bool isVolumeChanged() {
    return m_isVolumeChanged;
  }

  inline bool isPunctaChanged() {
    return m_isPunctaChanged;
  }

  inline void setSwcChanged(bool changed) {
    m_isSwcChanged = changed;
  }

  inline void setVolumeChanged(bool changed) {
    m_isVolumeChanged = changed;
  }

  inline void setPunctaChanged(bool changed) {
    m_isPunctaChanged = changed;
  }

  void updateWindow();

  void hideVolume();
  void showVolume();

  void saveScreenShot(const std::string &filePath, int width, int height);

  bool hasAnyChange();

private:
  Z3DWindow *m_window;
  bool m_isSwcChanged;
  bool m_isVolumeChanged;
  bool m_isPunctaChanged;
};

#endif // ZMOVIESTAGE_H
