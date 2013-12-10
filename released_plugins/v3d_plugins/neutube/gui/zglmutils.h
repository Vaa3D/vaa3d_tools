#ifndef ZGLMUTILS_H
#define ZGLMUTILS_H

// This file includes some commonly used headers from glm and defines some useful functions
// for glm

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <QDebug>

namespace glm {
typedef detail::tvec2<size_t> svec2;
typedef detail::tvec3<size_t> svec3;
typedef detail::tvec4<size_t> svec4;
typedef detail::tvec3<unsigned char> col3;
typedef detail::tvec4<unsigned char> col4;

// apply transform matrix
template<typename T>
detail::tvec3<T> applyMatrix(const detail::tmat4x4<T> &mat, const detail::tvec3<T> &vec)
{
  detail::tvec4<T> res = mat * detail::tvec4<T>(vec, T(1));
  return detail::tvec3<T>(res / res.w);
}

// given vec, get normalized vector e1 and e2 to make (e1,e2,vec) orthogonal to each other
// returned e1 and e2 is undefined if vec is zero
template<typename T>
void getOrthogonalVectors(const detail::tvec3<T> &vec, detail::tvec3<T> &e1, detail::tvec3<T> &e2)
{
  GLM_STATIC_ASSERT(detail::type<T>::is_float, "'normalize' only accept floating-point inputs");
  T eps = std::numeric_limits<T>::epsilon() * 1e2;

  e1 = cross(vec, detail::tvec3<T>(T(1), T(0), T(0)));
  if (dot(e1, e1) < eps)
    e1 = cross(vec, detail::tvec3<T>(T(0), T(1), T(0)));
  e1 = normalize(e1);
  e2 = normalize(cross(e1, vec));
}

}

template<typename T>
class Vec2Compare
{
  bool less;
public:
  Vec2Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec2<T>& lhs, const glm::detail::tvec2<T>& rhs) const
  {
    if (less) {
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

template<typename T>
class Vec3Compare
{
  bool less;
public:
  Vec3Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec3<T>& lhs, const glm::detail::tvec3<T>& rhs) const
  {
    if (less) {
      if (lhs.z != rhs.z)
        return lhs.z < rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.z != rhs.z)
        return lhs.z > rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

template<typename T>
class Vec4Compare
{
  bool less;
public:
  Vec4Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec4<T>& lhs, const glm::detail::tvec4<T>& rhs) const
  {
    if (less) {
      if (lhs.w != rhs.w)
        return lhs.w < rhs.w;
      if (lhs.z != rhs.z)
        return lhs.z < rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.w != rhs.w)
        return lhs.w > rhs.w;
      if (lhs.z != rhs.z)
        return lhs.z > rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

//-------------------------------------------------------------------------------------------------------------------------
// std iostream print

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec2<T>& v)
{
  return (s << "(" << v[0] << " " << v[1] << ")");
}

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec3<T>& v)
{
  return (s << "(" << v[0] << " " << v[1] << " " << v[2] << ")");
}

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec4<T>& v)
{
  return (s << "(" << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << ")");
}

template<>
inline std::ostream& operator << <unsigned char>(std::ostream& s, const glm::detail::tvec2<unsigned char>& v)
{
  return (s << glm::detail::tvec2<int>(v));
}

template<>
inline std::ostream& operator << <unsigned char>(std::ostream& s, const glm::detail::tvec3<unsigned char>& v)
{
  return (s << glm::detail::tvec3<int>(v));
}

template<>
inline std::ostream& operator << <unsigned char>(std::ostream& s, const glm::detail::tvec4<unsigned char>& v)
{
  return (s << glm::detail::tvec4<int>(v));
}

template<>
inline std::ostream& operator << <char>(std::ostream& s, const glm::detail::tvec2<char>& v)
{
  return (s << glm::detail::tvec2<int>(v));
}

template<>
inline std::ostream& operator << <char>(std::ostream& s, const glm::detail::tvec3<char>& v)
{
  return (s << glm::detail::tvec3<int>(v));
}

template<>
inline std::ostream& operator << <char>(std::ostream& s, const glm::detail::tvec4<char>& v)
{
  return (s << glm::detail::tvec4<int>(v));
}

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat2x2<T>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " |" << std::endl);
}

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat3x3<T>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " " << m[2][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " " << m[2][1] << " |" << std::endl
          << "| " << m[0][2] << " " << m[1][2] << " " << m[2][2] << " |" << std::endl);
}

template<typename T>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat4x4<T>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " " << m[2][0] << " " << m[3][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " " << m[2][1] << " " << m[3][1] << " |" << std::endl
          << "| " << m[0][2] << " " << m[1][2] << " " << m[2][2] << " " << m[3][2] << " |" << std::endl
          << "| " << m[0][3] << " " << m[1][3] << " " << m[2][3] << " " << m[3][3] << " |" << std::endl);
}

//-------------------------------------------------------------------------------------------------------------------------
// qDebug print

template<typename T>
QDebug& operator << (QDebug s, const glm::detail::tvec2<T>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T>
QDebug& operator << (QDebug s, const glm::detail::tvec3<T>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T>
QDebug& operator << (QDebug s, const glm::detail::tvec4<T>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T>
QDebug& operator << (QDebug s, const glm::detail::tmat2x2<T>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T>
QDebug& operator << (QDebug& s, const glm::detail::tmat3x3<T>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T>
QDebug& operator << (QDebug s, const glm::detail::tmat4x4<T>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

//-------------------------------------------------------------------------------------------------------------------------

#endif // ZGLMUTILS_H
