#pragma once
/**
  \file minimalOpenGLmatrix.h
  \author Morgan McGuire, http://casual-effects.com

  Minimal headers emulating a basic set of 3D graphics classes.
  This is intended to be replaced with your own implementation
  or that of your favorite library, such as G3D http://casual-effects.com/g3d

  All 3D math from http://graphicscodex.com
*/
#include <iostream>
#include <iomanip>
#include <math.h>

#ifndef _HUGE_ENUF
    #define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))
#define PI (3.1415927f)

class Vector3 {
public:
    float x, y, z;
    /** initializes to zero */
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    explicit Vector3(const class Vector4&);

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(y * other.z - z * other.y,
                       z * other.x - x * other.z,
                       x * other.y - y * other.x);
    }
    
    Vector3& operator+=(const Vector3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
    
    Vector3 operator+(const Vector3& v) const {
        return Vector3(x + v.x, y + v.y, z + v.z);
    }

    Vector3& operator-=(const Vector3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    Vector3 operator-(const Vector3& v) const {
        return Vector3(x - v.x, y - v.y, z - v.z);
    }

    Vector3 operator-() const {
        return Vector3(-x, -y, -z);
    }

    Vector3 operator*(float s) const {
        return Vector3(x * s, y * s, z * s);
    }

    Vector3 operator/(float s) const {
        return Vector3(x / s, y / s, z / s);
    }

    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    Vector3 normalize() const {
        return *this / length();
    }

    float& operator[](int i) {
        return (&x)[i];
    }

    float operator[](int i) const {
        return (&x)[i];
    }
};


class Vector4 {
public:
    float x, y, z, w;
    /** initializes to zero */
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
    float dot(const Vector4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
    float& operator[](int i) {
        return (&x)[i];
    }
    float operator[](int i) const {
        return (&x)[i];
    }
};


Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z) {}


class Matrix4x4 {
public:
    /** row-major */
    float data[16];

    /** row-major */
    Matrix4x4(float a, float b, float c, float d,
              float e, float f, float g, float h,
              float i, float j, float k, float l,
              float m, float n, float o, float p) {
        data[0]  = a; data[1]  = b; data[2]  = c; data[3]  = d;
        data[4]  = e; data[5]  = f; data[6]  = g; data[7]  = h;
        data[8]  = i; data[9]  = j; data[10] = k; data[11] = l;
        data[12] = m; data[13] = n; data[14] = o; data[15] = p;
    }

    /** initializes to the identity matrix */
    Matrix4x4() {
        memset(data, 0, sizeof(float) * 16);
        data[0] = data[5] = data[10] = data[15] = 1.0f;
    }

    Matrix4x4(const Matrix4x4& M) {
        memcpy(data, M.data, sizeof(float) * 16);
    }

    Matrix4x4& operator=(const Matrix4x4& M) {
        memcpy(data, M.data, sizeof(float) * 16);
        return *this;
    }

    static Matrix4x4 zero() {
        return Matrix4x4(0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 0.0f);
    }

    static Matrix4x4 roll(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix4x4(   c,  -s,  0.0f, 0.0f, 
                            s,   c,  0.0f, 0.0f,
                         0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 yaw(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix4x4(   c, 0.0f,    s, 0.0f, 
                         0.0f, 1.0f, 0.0f, 0.0f,
                           -s, 0.0f,    c, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 pitch(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f, 
                         0.0f,    c,   -s, 0.0f,
                         0.0f,    s,    c, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 scale(float x, float y, float z) {
        return Matrix4x4(   x, 0.0f, 0.0f, 0.0f, 
                         0.0f,    y, 0.0f, 0.0f,
                         0.0f, 0.0f,    z, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 translate(float x, float y, float z) {
        return Matrix4x4(1.0f, 0.0f, 0.0f,    x, 
                         0.0f, 1.0f, 0.0f,    y,
                         0.0f, 0.0f, 1.0f,    z,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    static Matrix4x4 translate(const Vector3& v) {
        return translate(v.x, v.y, v.z);
    }

    /** 
        Maps the view frustum to the cube [-1, +1]^3 in the OpenGL style.

        \param verticalRadians Vertical field of view from top to bottom
        \param nearZ Negative number
        \param farZ Negative number less than (higher magnitude than) nearZ. May be negative infinity 
    */
    static Matrix4x4 perspective(float pixelWidth, float pixelHeight, float nearZ, float farZ, float verticalRadians, float subpixelShiftX = 0.0f, float subpixelShiftY = 0.0f) {
        const float k = 1.0f / tan(verticalRadians / 2.0f);

        const float c = (farZ == -INFINITY) ? -1.0f : (nearZ + farZ) / (nearZ - farZ);
        const float d = (farZ == -INFINITY) ?  1.0f : farZ / (nearZ - farZ);

        Matrix4x4 P(k * pixelHeight / pixelWidth, 0.0f, subpixelShiftX * k / (nearZ * pixelWidth), 0.0f,
                    0.0f, k, subpixelShiftY * k / (nearZ * pixelHeight), 0.0f,
                    0.0f, 0.0f, c, -2.0f * nearZ * d,
                    0.0f, 0.0f, -1.0f, 0.0f);

        return P;        
    }

    /** 
        Maps the view frustum to the cube [-1, +1]^3 in the OpenGL
        style by orthographic projection in which (0, 0) will become
        the top-left corner of the screen after the viewport is
        applied and (pixelWidth - 1, pixelHeight - 1) will be the
        lower-right corner.
        
        \param nearZ Negative number
        \param farZ Negative number less than (higher magnitude than) nearZ. Must be finite
    */
    static Matrix4x4 ortho(float pixelWidth, float pixelHeight, float nearZ, float farZ) {
        return Matrix4x4(2.0f / pixelWidth, 0.0f, 0.0f, -1.0f,
                         0.0f, -2.0f / pixelHeight, 0.0f, 1.0f,
                         0.0f, 0.0f, -2.0f / (nearZ - farZ), (farZ + nearZ) / (nearZ - farZ),
                         0.0f, 0.0f, 0.0f, 1.0f);
    }

    Matrix4x4 transpose() const {
        return Matrix4x4(data[ 0], data[ 4], data[ 8], data[12],
                         data[ 1], data[ 5], data[ 9], data[13],
                         data[ 2], data[ 6], data[10], data[14],
                         data[ 3], data[ 7], data[11], data[15]);
    }

    /** Computes the inverse by Cramer's rule (based on MESA implementation) */
    Matrix4x4 inverse() const {
        Matrix4x4 result;
        const float* m = data;
        float* inv = result.data;

        inv[0] = m[5] * m[10] * m[15] -
            m[5] * m[11] * m[14] -
            m[9] * m[6] * m[15] +
            m[9] * m[7] * m[14] +
            m[13] * m[6] * m[11] -
            m[13] * m[7] * m[10];

        inv[4] = -m[4] * m[10] * m[15] +
            m[4] * m[11] * m[14] +
            m[8] * m[6] * m[15] -
            m[8] * m[7] * m[14] -
            m[12] * m[6] * m[11] +
            m[12] * m[7] * m[10];

        inv[8] = m[4] * m[9] * m[15] -
            m[4] * m[11] * m[13] -
            m[8] * m[5] * m[15] +
            m[8] * m[7] * m[13] +
            m[12] * m[5] * m[11] -
            m[12] * m[7] * m[9];

        inv[12] = -m[4] * m[9] * m[14] +
            m[4] * m[10] * m[13] +
            m[8] * m[5] * m[14] -
            m[8] * m[6] * m[13] -
            m[12] * m[5] * m[10] +
            m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15] +
            m[1] * m[11] * m[14] +
            m[9] * m[2] * m[15] -
            m[9] * m[3] * m[14] -
            m[13] * m[2] * m[11] +
            m[13] * m[3] * m[10];

        inv[5] = m[0] * m[10] * m[15] -
            m[0] * m[11] * m[14] -
            m[8] * m[2] * m[15] +
            m[8] * m[3] * m[14] +
            m[12] * m[2] * m[11] -
            m[12] * m[3] * m[10];

        inv[9] = -m[0] * m[9] * m[15] +
            m[0] * m[11] * m[13] +
            m[8] * m[1] * m[15] -
            m[8] * m[3] * m[13] -
            m[12] * m[1] * m[11] +
            m[12] * m[3] * m[9];

        inv[13] = m[0] * m[9] * m[14] -
            m[0] * m[10] * m[13] -
            m[8] * m[1] * m[14] +
            m[8] * m[2] * m[13] +
            m[12] * m[1] * m[10] -
            m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15] -
            m[1] * m[7] * m[14] -
            m[5] * m[2] * m[15] +
            m[5] * m[3] * m[14] +
            m[13] * m[2] * m[7] -
            m[13] * m[3] * m[6];

        inv[6] = -m[0] * m[6] * m[15] +
            m[0] * m[7] * m[14] +
            m[4] * m[2] * m[15] -
            m[4] * m[3] * m[14] -
            m[12] * m[2] * m[7] +
            m[12] * m[3] * m[6];

        inv[10] = m[0] * m[5] * m[15] -
            m[0] * m[7] * m[13] -
            m[4] * m[1] * m[15] +
            m[4] * m[3] * m[13] +
            m[12] * m[1] * m[7] -
            m[12] * m[3] * m[5];

        inv[14] = -m[0] * m[5] * m[14] +
            m[0] * m[6] * m[13] +
            m[4] * m[1] * m[14] -
            m[4] * m[2] * m[13] -
            m[12] * m[1] * m[6] +
            m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11] +
            m[1] * m[7] * m[10] +
            m[5] * m[2] * m[11] -
            m[5] * m[3] * m[10] -
            m[9] * m[2] * m[7] +
            m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11] -
            m[0] * m[7] * m[10] -
            m[4] * m[2] * m[11] +
            m[4] * m[3] * m[10] +
            m[8] * m[2] * m[7] -
            m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11] +
            m[0] * m[7] * m[9] +
            m[4] * m[1] * m[11] -
            m[4] * m[3] * m[9] -
            m[8] * m[1] * m[7] +
            m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10] -
            m[0] * m[6] * m[9] -
            m[4] * m[1] * m[10] +
            m[4] * m[2] * m[9] +
            m[8] * m[1] * m[6] -
            m[8] * m[2] * m[5];

        float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        return result / det;
    }

    float& operator()(int r, int c) {
        return data[r * 4 + c];         
    }

    float operator()(int r, int c) const {
        return data[r * 4 + c];         
    }

    Vector4 row(int r) const {
        const int i = r * 4;
        return Vector4(data[i], data[i + 1], data[i + 2], data[i + 3]);
    }

    Vector4 col(int c) const {
        return Vector4(data[c], data[c + 4], data[c + 8], data[c + 12]);
    }

    Matrix4x4 operator*(const Matrix4x4& B) const {
        Matrix4x4 D;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                D(r, c) = row(r).dot(B.col(c));
            }
        }
        return D;
    }

    Matrix4x4 operator*(const float s) const {
        Matrix4x4 D;
        for (int i = 0; i < 16; ++i) {
            D.data[i] = data[i] * s;
        }
        return D;
    }

    Matrix4x4 operator/(const float s) const {
        Matrix4x4 D;
        for (int i = 0; i < 16; ++i) {
            D.data[i] = data[i] / s;
        }
        return D;
    }

    Vector4 operator*(const Vector4& v) const {
        Vector4 d;
        for (int r = 0; r < 4; ++r) {
            d[r] = row(r).dot(v);
        }
        return d;
    }
};



class Matrix3x3 {
public:
    /** row-major */
    float data[9];

    /** row-major */
    Matrix3x3(float a, float b, float c, 
        float d, float e, float f, 
        float g, float h, float i) {
        data[0] = a; data[1] = b; data[2] = c; 
        data[3] = d; data[4] = e; data[5] = f; 
        data[6] = g; data[7] = h; data[8] = i;
    }

    /** Takes the upper-left 3x3 submatrix */
    Matrix3x3(const Matrix4x4& M) {
        data[0] = M.data[0]; data[1] = M.data[1]; data[2] = M.data[2];
        data[3] = M.data[4]; data[4] = M.data[5]; data[5] = M.data[6];
        data[6] = M.data[8]; data[7] = M.data[9]; data[8] = M.data[10];
    }

    /** initializes to the identity matrix */
    Matrix3x3() {
        memset(data, 0, sizeof(float) * 9);
        data[0] = data[4] = data[8] = 1.0f;
    }

    Matrix3x3(const Matrix3x3& M) {
        memcpy(data, M.data, sizeof(float) * 9);
    }

    Matrix3x3& operator=(const Matrix3x3& M) {
        memcpy(data, M.data, sizeof(float) * 9);
        return *this;
    }

    static Matrix3x3 zero() {
        return Matrix3x3(0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f);
    }

    static Matrix3x3 roll(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix3x3(c, -s, 0.0f,
            s, c, 0.0f,
            0.0f, 0.0f, 1.0f);
    }

    static Matrix3x3 yaw(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix3x3(c, 0.0f, s,
            0.0f, 1.0f, 0.0f,
            -s, 0.0f, c);
    }

    static Matrix3x3 pitch(float radians) {
        const float c = cos(radians), s = sin(radians);
        return Matrix3x3(1.0f, 0.0f, 0.0f,
            0.0f, c, -s,
            0.0f, s, c);
    }

    static Matrix3x3 scale(float x, float y, float z) {
        return Matrix3x3(x, 0.0f, 0.0f,
            0.0f, y, 0.0f,
            0.0f, 0.0f, z);
    }

    Matrix3x3 transpose() const {
        return Matrix3x3(data[0], data[3], data[6],
            data[1], data[4], data[7],
            data[2], data[5], data[8]);
    }

    /** Computes the inverse by Cramer's rule */
    Matrix3x3 inverse() const {
        const Matrix3x3& m(*this);
        const float det = m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
                          m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
                          m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));

        return Matrix3x3(
            (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) / det,
            (m(0, 2) * m(2, 1) - m(0, 1) * m(2, 2)) / det,
            (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1)) / det,

            (m(1, 2) * m(2, 0) - m(1, 0) * m(2, 2)) / det,
            (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0)) / det,
            (m(1, 0) * m(0, 2) - m(0, 0) * m(1, 2)) / det,

            (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1)) / det,
            (m(2, 0) * m(0, 1) - m(0, 0) * m(2, 1)) / det,
            (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1)) / det);
    }

    float& operator()(int r, int c) {
        return data[r * 3 + c];
    }

    float operator()(int r, int c) const {
        return data[r * 3 + c];
    }

    Vector3 row(int r) const {
        const int i = r * 3;
        return Vector3(data[i], data[i + 1], data[i + 2]);
    }

    Vector3 col(int c) const {
        return Vector3(data[c], data[c + 3], data[c + 6]);
    }

    Matrix3x3 operator*(const Matrix3x3& B) const {
        Matrix3x3 D;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                D(r, c) = row(r).dot(B.col(c));
            }
        }
        return D;
    }

    Matrix3x3 operator*(const float s) const {
        Matrix3x3 D;
        for (int i = 0; i < 9; ++i) {
            D.data[i] = data[i] * s;
        }
        return D;
    }

    Matrix3x3 operator/(const float s) const {
        Matrix3x3 D;
        for (int i = 0; i < 9; ++i) {
            D.data[i] = data[i] / s;
        }
        return D;
    }

    Vector3 operator*(const Vector3& v) const {
        Vector3 d;
        for (int r = 0; r < 3; ++r) {
            d[r] = row(r).dot(v);
        }
        return d;
    }
};

std::ostream& operator<<(std::ostream& os, const Vector3& v) {
    return os << "Vector3(" << std::setprecision(2) << v.x << ", " << v.y << ")";
}


std::ostream& operator<<(std::ostream& os, const Vector4& v) {
    return os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ")";
}


std::ostream& operator<<(std::ostream& os, const Matrix4x4& M) {
    os << "\nMatrix4x4(";
    
    for (int r = 0, i = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c, ++i) {
            os << M.data[i];
            if (c < 3) { os << ", "; }
        }
        if (r < 3) { os << ",\n          "; }
    }

    return os << ")\n";
}


std::ostream& operator<<(std::ostream& os, const Matrix3x3& M) {
    os << "\nMatrix3x3(";

    for (int r = 0, i = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c, ++i) {
            os << M.data[i];
            if (c < 2) { os << ", "; }
        }
        if (r < 2) { os << ",\n          "; }
    }

    return os << ")\n";
}
