#ifndef SHAPESTATISTICS_H
#define SHAPESTATISTICS_H

#ifdef QT_VERSION_STR
    #include <QString>
#endif
#include <itkShapeLabelObject.h>

// shape statistics class, link to ITK
template<typename T = itk::ShapeLabelObject<unsigned int, 3> >    // T is the class of the shape obj
class ShapeStatistics
{
private:
    typename T::Pointer shapeInfo;
    unsigned int mLabelIdx;  //label idx (in CC volume)

    unsigned int mAnnotationLabel;  // this can be used by an annotation tool to set the class (e.g. pos/neg) of a certain connected region

public:
    ShapeStatistics() { }
    ShapeStatistics( const typename T::Pointer shPtr, unsigned int lblIdx )
    {
        shapeInfo = shPtr;
        mLabelIdx = lblIdx;
        mAnnotationLabel = 0;
    }

    inline unsigned int labelIdx() const {
        return mLabelIdx;
    }

    inline unsigned int numVoxels() const {
        return shapeInfo->GetSize();
    }

    inline unsigned int annotationLabel() const {
        return mAnnotationLabel;
    }

    inline void setAnnotationLabel(unsigned int k) {
        mAnnotationLabel = k;
    }

    // comparison operator
    static inline bool lessThan( const ShapeStatistics<T> & a, const ShapeStatistics<T> & b )
    {
        return a.numVoxels() < b.numVoxels();
    }

    static inline bool greaterThan( const ShapeStatistics<T> & a, const ShapeStatistics<T> & b )
    {
        return a.numVoxels() > b.numVoxels();
    }
#ifdef QT_VERSION_STR
    QString toString()
    {
        QString s = "<table border=\"1\">";

#define ADD_ROW(x,y)    \
    s += QString("<tr> <td>") + x + QString("</td><td>") + y + QString("</td>")

        ADD_ROW( "Phys size", QString("%1").arg( shapeInfo->GetPhysicalSize() ) );


        ADD_ROW( "Feret Diameter", QString("%1").arg( shapeInfo->GetFeretDiameter() ) );
        ADD_ROW( "Perimeter", QString("%1").arg( shapeInfo->GetPerimeter() ) );
        ADD_ROW( "Roundness", QString("%1").arg( shapeInfo->GetRoundness() ) );
        ADD_ROW( "Flatness", QString("%1").arg( shapeInfo->GetBinaryFlatness() ) );

        typename T::VectorType moments = shapeInfo->GetBinaryPrincipalMoments();
        typename T::MatrixType pAxes = shapeInfo->GetBinaryPrincipalAxes();

        ADD_ROW( "Principal Moments", QString("(%1, %2, %3)").arg(moments[0]).arg(moments[1]).arg(moments[2]) );


        ADD_ROW( "Principal Vector 1", QString("(%1, %2, %3)").arg(pAxes[0][0]).arg(pAxes[0][1]).arg(pAxes[0][2]) );
        ADD_ROW( "Principal Vector 2", QString("(%1, %2, %3)").arg(pAxes[1][0]).arg(pAxes[1][1]).arg(pAxes[1][2]) );
        ADD_ROW( "Principal Vector 3", QString("(%1, %2, %3)").arg(pAxes[2][0]).arg(pAxes[2][1]).arg(pAxes[2][2]) );


#undef ADD_ROW

        s += "</table>";

        return s;
    }
#endif
};

#endif // SHAPESTATISTICS_H
