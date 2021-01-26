#include "tifMapScribe.h"

tifMapScribe::tifMapScribe()
{
    tifMapScribe::rightUpX = 0;
    tifMapScribe::rightUpY = 0;
    tifMapScribe::leftDownX = 1999;
    tifMapScribe::leftDownY = 1599;
    sizeX = 2000;
    sizeY = 1600;
}

tifMapScribe::tifMapScribe(int a,int b,int x, int y)
{
    set(a, b, x, y);
}

tifMapScribe::tifMapScribe(int a,int b,int x)
{
    set(a, b, x, x);
}

tifMapScribe::tifMapScribe(int x)
{
    set(0, 0, x, x);
}

void tifMapScribe::move(int x,int y)
{
    tifMapScribe::rightUpX += x;
    tifMapScribe::leftDownX += x;
    tifMapScribe::rightUpY += y;
    tifMapScribe::leftDownY += y;
}

void tifMapScribe::set(int a,int b)
{
     set(a, b, sizeX, sizeY);
}

void tifMapScribe::set(int a,int b,int x, int y)
{
    tifMapScribe::leftDownX = a;
    tifMapScribe::leftDownY = b;
    sizeX = x;
    sizeY = y;
    tifMapScribe::rightUpX = leftDownX + sizeX - 1;
    tifMapScribe::rightUpY = leftDownY + sizeY - 1;
}

void tifMapScribe::set(int a,int b,int x)
{
    set(a, b, x, x);
}

int tifMapScribe::distance(int x, int y)
{
    int temp1, temp2, temp3, temp4, temp5=2000;

    temp1 = (x > leftDownX) ? (x - leftDownX) : (leftDownX - x);
    temp2 = (x > rightUpX) ? (x - rightUpX) : (rightUpX - x);
    temp3 = (y > leftDownY) ? (y - leftDownY) : (leftDownY - y);
    temp4 = (y > rightUpY) ? (y - rightUpY) : (rightUpY - y);

    if((x>rightUpX || x<leftDownX) || (y>rightUpY || y<leftDownY))
        temp5 = 0;
    int low = temp1;

    if(temp2 < low)
        low = temp2;
    if(low > temp3)
        low = temp3;
    if(low > temp4)
        low = temp4;
    if(low > temp5)
        low = temp5;

    return low;
}

