#ifndef ZCURSORSTORE_H
#define ZCURSORSTORE_H

#include <QCursor>
#include <QBitmap>

class ZCursorStore
{
public:

  static ZCursorStore& getInstance() {
    static ZCursorStore store;

    return store;
  }

  inline const QCursor& getCircleCursor() { return m_circleCursor10; }
  const QCursor& getCircleCursor(int radius);

  static QBitmap circleCursorBitmap(int radius = 10);
  static inline int maxCircleCursorRadius() { return 20; }
  static inline int minCircleCursorRadius() { return 5; }
  static int prevCircleRadius(int radius);
  static int nextCircleRadius(int radius);

  static QBitmap smallCrossCursorBitmap();

  inline const QCursor& getSmallCrossCursor() { return m_smallCrossCursor; }

private:
  ZCursorStore();
  ZCursorStore(const ZCursorStore&);
  void operator=(const ZCursorStore&);

  QCursor m_circleCursor5;
  QCursor m_circleCursor10;
  QCursor m_circleCursor15;
  QCursor m_circleCursor20;
  QCursor m_smallCrossCursor;
};

#endif // ZCURSORSTORE_H
