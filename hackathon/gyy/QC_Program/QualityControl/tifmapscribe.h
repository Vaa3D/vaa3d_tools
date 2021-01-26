#ifndef TIFMAPSCRIBE_H
#define TIFMAPSCRIBE_H
#include <QFileDialog>
#include <iostream>
using namespace std;

class tifMapScribe
{
public:
    tifMapScribe();
    tifMapScribe(int a, int b, int x, int y);
    tifMapScribe(int a, int b, int x);
    tifMapScribe(int x);

    void move(int x, int y);
    void set(int a, int b);
    void set(int a, int b, int x, int y);
    void set(int a, int b, int x);
    int distance(int x, int y);

    int leftDownX;
    int leftDownY;
    int rightUpX;
    int rightUpY;
    int sizeX;
    int sizeY;
};

#endif // TIFMAPSCRIBE_H
