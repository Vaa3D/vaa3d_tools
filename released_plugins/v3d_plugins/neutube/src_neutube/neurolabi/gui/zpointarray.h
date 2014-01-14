#ifndef ZPOINTARRAY_H
#define ZPOINTARRAY_H

#include <vector>
#include <zpoint.h>
#include <string>

class ZPointArray : public std::vector<ZPoint>
{
public:
  ZPointArray();

public:
  void translate(const ZPoint &pt);
  void scale(double sx, double sy, double sz);
  void exportSwcFile(const std::string &filePath, double radius) const;
  void exportTxtFile(const std::string &filePath) const;
  void print() const;

};

#endif // ZPOINTARRAY_H
