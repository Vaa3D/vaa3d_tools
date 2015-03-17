#include "shape.h"

Shape::Shape(INeuronProcessObject *p)
{
    m_ProcessObject =  p;
    identifyName =  std::string("Shape");
}

Shape::~Shape()
{

}

Shape::ShapePointer Shape::New()//static
{
    return ShapePointer(new Shape);
}

void Shape::swap(Shape &rhs)
{
    std::swap(this->m_Shape, rhs.m_Shape);
    std::swap(this->m_Surface, rhs.m_Surface);
    std::swap(this->m_Volume, rhs.m_Volume);
}

bool Shape::IsEmpty() const
{
    return m_Shape.empty();
}


ShapeCluster::ShapeCluster(INeuronProcessObject *p)
{
    m_ProcessObject =  p;
    identifyName =  std::string("ShapeCluster");
}

void ShapeCluster::swap(ShapeCluster &rhs)
{
    std::swap(this->m_Shapes, rhs.m_Shapes);
}

void ShapeCluster::push_back(Shape::ShapePointer &rhs)
{
    m_Shapes.push_back(rhs);
}


ShapeCluster::~ShapeCluster()
{

}
