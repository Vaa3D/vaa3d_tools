#ifndef SHAPE_H
#define SHAPE_H
#include "ineurondataobject.h"
#include "basetypes.h"
#include <memory>

class Shape : public INeuronDataObject
{
public:
    typedef std::shared_ptr<Shape> ShapePointer;

    Shape(INeuronProcessObject *p = nullptr);
    ~Shape();
    static ShapePointer New();
    bool IsEmpty()const;
    Shape(const Shape&);
    Shape& operator=(const Shape&);
    void swap(Shape&);
    void SetProcessObject(INeuronProcessObject*){}
    void ReleaseProcessObject(){}

private:
    int ID;//soma id
    double x,y,z;
    std::vector<Vec3i> m_Shape;
    std::vector<Vec3i> m_Inner;
    double m_Surface;
    double m_Volume;
};

class ShapeCluster : INeuronDataObject
{
public:
    ShapeCluster(INeuronProcessObject* p = nullptr);
    ~ShapeCluster();
    bool IsEmpty()const{return m_Shapes.empty();}
    void swap(ShapeCluster&);
    void push_back(Shape::ShapePointer&);
//    void SetProcessObject(INeuronProcessObject*);
//    void ReleaseProcessObject();

    typedef std::shared_ptr<ShapeCluster> ShapeClusterPointer;
private:
    std::vector<Shape::ShapePointer> m_Shapes;
    ShapeCluster(const ShapeCluster&) ;
    ShapeCluster& operator=(const ShapeCluster&);
};

#endif // SHAPE_H
