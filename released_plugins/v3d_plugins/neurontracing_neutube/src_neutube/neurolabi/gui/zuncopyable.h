#ifndef ZUNCOPYABLE_H
#define ZUNCOPYABLE_H

class ZUncopyable {
protected:
  ZUncopyable() {}
  ~ZUncopyable() {}

private:
  ZUncopyable(const ZUncopyable&);
  ZUncopyable& operator=(const ZUncopyable&);
};

#endif // ZUNCOPYABLE_H
