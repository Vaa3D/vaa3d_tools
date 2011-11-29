#ifndef WRAPPABLE_V3D_H_
#define WRAPPABLE_V3D_H_

#include "ImageWindow.h"
#include "v3d_qt_environment.h"
#include "SimTKcommon/internal/Rotation.h"
#include "SimTKcommon/internal/Quaternion.h"
#include "ImageWindow.h"

std::string get_argitem_type(const V3DPluginArgItem& item);
void set_argitem_type(V3DPluginArgItem& item, const std::string& s);
std::string get_argitem_pointer(const V3DPluginArgItem& item);
void set_argitem_pointer(V3DPluginArgItem& item, void* ptr);

// Unfortunately, we might have to instantiate each possible subtype of V3DPluginArg
// to expose it to python
template<class T>
class V3DPluginArg : public V3DPluginArgItem {
public:
    V3DPluginArg(T& item, const std::string& typeName) {
        setItem(item);
        setType(typeName);
    }
    const T& getItem() const {
        const T* ptr = static_cast<const T*>(p);
        return *ptr;
    }
    void setItem(T& item) {p = (void*) &item;}
    std::string getType() const {return type.toStdString();}
    void setType(const std::string& s) {type = QString::fromStdString(s);}
};

// typedef c_array< double, 3 > double3;
// typedef c_array< double3, 3 > double3x3;

// "extern" to avoid multiply defined symbol error on Mac
#ifdef _MSC_VER
#define PYV3D_HEADER_TEMPLATE template
unsigned int qHash(const LocationSimple& loc);
unsigned int qHash(const QPolygon&);
unsigned int qHash(const V3DPluginArgItem& lhs);
bool operator==(const LocationSimple&, const LocationSimple&);
// operator== needed for wrapping QList<V3DPluginArgItem>
bool operator==(const V3DPluginArgItem& lhs, const V3DPluginArgItem& rhs);
#else
#define PYV3D_HEADER_TEMPLATE extern template
#endif

PYV3D_HEADER_TEMPLATE class QList<LocationSimple>;
PYV3D_HEADER_TEMPLATE class QVector<QPoint>;
PYV3D_HEADER_TEMPLATE class QList<QPolygon>;
PYV3D_HEADER_TEMPLATE class QHash<int, int>;
PYV3D_HEADER_TEMPLATE class QList<V3DPluginArgItem>;
PYV3D_HEADER_TEMPLATE class c_array< double, 3 >;
PYV3D_HEADER_TEMPLATE class c_array< c_array< double, 3 >, 3 >;
PYV3D_HEADER_TEMPLATE class V3DPluginArg< c_array< c_array< double, 3 >, 3 > >;
PYV3D_HEADER_TEMPLATE class SimTK::Rotation_<double>;
PYV3D_HEADER_TEMPLATE class SimTK::Vec<3, double, 1>;
PYV3D_HEADER_TEMPLATE class SimTK::Quaternion_<double>;

/*! \brief Returns general parameters of the V3D program.
 *
 */
V3D_GlobalSetting getGlobalSetting();

/*! \brief Calls a function in a dynamically loaded V3D plugin module.
 *
 */
bool callPluginFunc(const QString & plugin_name, const QString & func_name,
        const V3DPluginArgList & input, V3DPluginArgList & output);

/*! \brief Sets general parameters of the V3D program.
 *
 */
bool setGlobalSetting(V3D_GlobalSetting& gs);


namespace pyplusplus { namespace alias {
    // Need to wrap QVector<QPoint> to avoid runtime error at startup on Mac
    // This wrapper is hollow.
    typedef QVector<QPoint> QVector_QPoint;
    typedef c_array<unsigned char, 3> c_array_uint_3;
    typedef c_array<int, 3> c_array_int_3;
    typedef c_array<unsigned char, 4> c_array_uint_4;
    typedef c_array<float, 3> c_array_float_3;
    typedef c_array<short, 3> c_array_short_3;

    typedef c_array< double, 3 > double3;
    typedef c_array< double3, 3 > double3x3;
    typedef V3DPluginArg<double3x3> V3DPluginArg_double3x3;
    typedef SimTK::Rotation_<double> Rotation;
    typedef SimTK::Vec<3, double, 1> Vec3;
}}

#endif /* WRAPPABLE_V3D_H_ */
