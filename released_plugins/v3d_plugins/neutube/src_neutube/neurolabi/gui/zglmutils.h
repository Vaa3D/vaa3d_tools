#ifndef ZGLMUTILS_H
#define ZGLMUTILS_H

// This file includes some commonly used headers from glm and defines some useful functions
// for glm

#define GLM_FORCE_SSE2
#define GLM_FORCE_SIZE_T_LENGTH
#define GLM_MESSAGES
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <sstream>
#include <QDebug>
#include <algorithm>

namespace glm {
typedef detail::tvec3<unsigned char, highp> col3;
typedef detail::tvec4<unsigned char, highp> col4;

// apply transform matrix
template<typename T, precision P>
detail::tvec3<T,P> applyMatrix(const detail::tmat4x4<T,P> &mat, const detail::tvec3<T,P> &vec)
{
  detail::tvec4<T,P> res = mat * detail::tvec4<T,P>(vec, T(1));
  return detail::tvec3<T,P>(res / res.w);
}

// given vec, get normalized vector e1 and e2 to make (e1,e2,vec) orthogonal to each other
// **crash** if vec is zero
template<typename T, precision P>
void getOrthogonalVectors(const detail::tvec3<T,P> &vec, detail::tvec3<T,P> &e1, detail::tvec3<T,P> &e2)
{
  GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'getOrthogonalVectors' only accept floating-point inputs");
  T eps = std::numeric_limits<T>::epsilon() * 1e2;

  e1 = cross(vec, detail::tvec3<T,P>(T(1), T(0), T(0)));
  if (dot(e1, e1) < eps)
    e1 = cross(vec, detail::tvec3<T,P>(T(0), T(1), T(0)));
  e1 = normalize(e1);
  e2 = normalize(cross(e1, vec));
}

}

template<typename T, glm::precision P>
class Vec2Compare
{
  bool less;
public:
  Vec2Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec2<T,P>& lhs, const glm::detail::tvec2<T,P>& rhs) const
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

template<typename T, glm::precision P>
class Vec3Compare
{
  bool less;
public:
  Vec3Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec3<T,P>& lhs, const glm::detail::tvec3<T,P>& rhs) const
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

template<typename T, glm::precision P>
class Vec4Compare
{
  bool less;
public:
  Vec4Compare(bool less = true) : less(less) {}
  bool operator() (const glm::detail::tvec4<T,P>& lhs, const glm::detail::tvec4<T,P>& rhs) const
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

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec2<T,P>& v)
{
  return (s << "(" << v[0] << " " << v[1] << ")");
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec3<T,P>& v)
{
  return (s << "(" << v[0] << " " << v[1] << " " << v[2] << ")");
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec4<T,P>& v)
{
  return (s << "(" << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << ")");
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::detail::tvec2<unsigned char,glm::highp>& v)
{
  return (s << glm::detail::tvec2<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::detail::tvec3<unsigned char,glm::highp>& v)
{
  return (s << glm::detail::tvec3<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::detail::tvec4<unsigned char,glm::highp>& v)
{
  return (s << glm::detail::tvec4<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::detail::tvec2<char,glm::highp>& v)
{
  return (s << glm::detail::tvec2<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::detail::tvec3<char,glm::highp>& v)
{
  return (s << glm::detail::tvec3<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::detail::tvec4<char,glm::highp>& v)
{
  return (s << glm::detail::tvec4<int,glm::highp>(v));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat2x2<T,P>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " |" << std::endl);
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat3x3<T,P>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " " << m[2][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " " << m[2][1] << " |" << std::endl
          << "| " << m[0][2] << " " << m[1][2] << " " << m[2][2] << " |" << std::endl);
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat4x4<T,P>& m)
{
  return (s
          << "| " << m[0][0] << " " << m[1][0] << " " << m[2][0] << " " << m[3][0] << " |" << std::endl
          << "| " << m[0][1] << " " << m[1][1] << " " << m[2][1] << " " << m[3][1] << " |" << std::endl
          << "| " << m[0][2] << " " << m[1][2] << " " << m[2][2] << " " << m[3][2] << " |" << std::endl
          << "| " << m[0][3] << " " << m[1][3] << " " << m[2][3] << " " << m[3][3] << " |" << std::endl);
}

//-------------------------------------------------------------------------------------------------------------------------
// qDebug print

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::detail::tvec2<T,P>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::detail::tvec3<T,P>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::detail::tvec4<T,P>& v)
{
  std::ostringstream oss;
  oss << v;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::detail::tmat2x2<T,P>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug& s, const glm::detail::tmat3x3<T,P>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::detail::tmat4x4<T,P>& m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

//-------------------------------------------------------------------------------------------------------------------------

#endif // ZGLMUTILS_H
