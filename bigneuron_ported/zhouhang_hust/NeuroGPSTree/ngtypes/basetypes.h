//time : 2013-11-20
//this file include Eigen and std::vector to provide base data structure
//author:zhouhang,WLNO BMP

#ifndef BASETYPES_H
#define BASETYPES_H
#include <Eigen/Core>
#include <vector>

typedef unsigned char NGCHAR;

typedef Eigen::Vector2i Vec2i;
typedef Eigen::Vector3i Vec3i;
typedef Eigen::Vector4i Vec4i;
typedef Eigen::VectorXi VecXi;

typedef Eigen::Matrix2i Mat2i;
typedef Eigen::Matrix3i Mat3i;
typedef Eigen::Matrix4i Mat4i;

typedef Eigen::Vector2f Vec2f;
typedef Eigen::Vector3f Vec3f;
typedef Eigen::Vector4f Vec4f;
typedef Eigen::Matrix<float, 5, 1> Vec5f;


typedef Eigen::Matrix2f Mat2f;
typedef Eigen::Matrix3f Mat3f;
typedef Eigen::Matrix4f Mat4f;

typedef Eigen::Vector2d Vec2d;
typedef Eigen::Vector3d Vec3d;
typedef Eigen::Vector4d Vec4d;
typedef Eigen::Matrix<double, 5, 1> Vec5d;
typedef Eigen::Matrix<double, 6, 1> Vec6d;
typedef Eigen::Matrix<double, 7, 1> Vec7d;

typedef Eigen::Matrix2d Mat2d;
typedef Eigen::Matrix3d Mat3d;
typedef Eigen::Matrix4d Mat4d;

typedef Eigen::VectorXd VecXd;
typedef Eigen::MatrixXd MatXd;

typedef Eigen::VectorXi VecXi;
typedef Eigen::MatrixXi MatXi;

typedef std::vector<Vec2i, Eigen::aligned_allocator<Vec2i> > VectorVec2i;
typedef std::vector<Vec3i, Eigen::aligned_allocator<Vec3i> > VectorVec3i;
typedef std::vector<Vec4i, Eigen::aligned_allocator<Vec4i> > VectorVec4i;

typedef std::vector<Vec3f, Eigen::aligned_allocator<Vec3f> > VectorVec3f;
typedef std::vector<Vec4f, Eigen::aligned_allocator<Vec4f> > VectorVec4f;

typedef std::vector<Vec2d, Eigen::aligned_allocator<Vec2d> > VectorVec2d;
typedef std::vector<Vec3d, Eigen::aligned_allocator<Vec3d> > VectorVec3d;
typedef std::vector<Vec4d, Eigen::aligned_allocator<Vec4d> > VectorVec4d;
typedef std::vector<Vec5d, Eigen::aligned_allocator<Vec5d> > VectorVec5d;
typedef std::vector<Vec6d, Eigen::aligned_allocator<Vec6d> > VectorVec6d;
typedef std::vector<VecXd, Eigen::aligned_allocator<VecXd> > VectorVecXd;

typedef std::vector<Mat2i, Eigen::aligned_allocator<Mat2i> > VectorMat2i;

typedef std::vector<Mat2d, Eigen::aligned_allocator<Mat2d> > VectorMat2d;
typedef std::vector<Mat3d, Eigen::aligned_allocator<Mat3d> > VectorMat3d;
typedef std::vector<Mat4d, Eigen::aligned_allocator<Mat4d> > VectorMat4d;
typedef std::vector<Vec7d, Eigen::aligned_allocator<Vec7d> > VectorMat7d;
typedef std::vector<MatXd, Eigen::aligned_allocator<MatXd> > VectorMatXd;

#endif // BASETYPES_H
