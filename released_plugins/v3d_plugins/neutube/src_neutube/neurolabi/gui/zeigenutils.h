#ifndef ZEIGENUTILS_H
#define ZEIGENUTILS_H

#include <Eigen/Dense>
#include <fstream>
#include <sstream>
#include "QsLog.h"
#include <map>
#include <set>
#include <limits>
#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include "zglmutils.h"
#ifndef Q_MOC_RUN
#include <boost/math/special_functions.hpp>
#include <boost/type_traits/conditional.hpp>  // will be supported by c++11
#endif

//using namespace Eigen;

// qDebug output of eigen matrix and vector
template<typename Derived>
QDebug& operator << (QDebug s, const Eigen::DenseBase<Derived> & m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

template<typename ExpressionType>
QDebug& operator << (QDebug s, const Eigen::WithFormat<ExpressionType> & m)
{
  std::ostringstream oss;
  oss << m;
  s.nospace() << oss.str().c_str();
  return s.space();
}

// from eigen doc
template<typename Scalar>
struct CwiseClampOp {
  CwiseClampOp(const Scalar& inf, const Scalar& sup) : m_inf(inf), m_sup(sup) {}
  const Scalar operator()(const Scalar& x) const { return x<m_inf ? m_inf : (x>m_sup ? m_sup : x); }
  Scalar m_inf, m_sup;
};

template <typename T, bool IsInteger = Eigen::NumTraits<T>::IsInteger>
struct ZVectorCompare
{
  bool operator()(const Eigen::Matrix<T , Eigen::Dynamic , 1> &v1,
                  const Eigen::Matrix<T , Eigen::Dynamic , 1> &v2) const
  {
    if (v1.size() == v2.size()) {
      for (int i=0; i<v1.size(); i++) {
        if (std::abs(v1(i) - v2(i)) < Eigen::NumTraits<T>::dummy_precision() )
          continue;
        else
          return v1(i) < v2(i);
      }
      return false;   //equal
    } else {
      return v1.size() < v2.size();
    }
  }
};

template <typename T>
struct ZVectorCompare<T, true>
{
  bool operator()(const Eigen::Matrix<T , Eigen::Dynamic , 1> &v1,
                  const Eigen::Matrix<T , Eigen::Dynamic , 1> &v2) const
  {
    if (v1.size() == v2.size()) {
      for (int i=0; i<v1.size(); i++) {
        if (v1(i) == v2(i) )
          continue;
        else
          return v1(i) < v2(i);
      }
      return false;   //equal
    } else {
      return v1.size() < v2.size();
    }
  }
};

// type traits to find float type with sufficient range for general calculation of the two input type
template<typename T1, typename T2>
struct MaxFloatType
{
  typedef typename Eigen::NumTraits<T1>::NonInteger T1NonIntegerType;
  typedef typename Eigen::NumTraits<T2>::NonInteger T2NonIntegerType;
  // get larger type of two float type, use this type for all calculations
  typedef typename boost::conditional<(std::numeric_limits<T1NonIntegerType>::digits10 >
                                       std::numeric_limits<T2NonIntegerType>::digits10),
                T1NonIntegerType, T2NonIntegerType>::type type;
};

class ZEigenUtils
{
public:

  inline static Eigen::MatrixXd readMatrix(const char *filename, int startRow = 0, int startCol = 0,
                             int endRow = -1, int endCol = -1)
  {
    return readMatrix(filename, "", startRow, startCol, endRow, endCol);
  }

  // result matrix **do not** include endRow and endCol, if there are empty lines between [startRow endRow),
  // the result matrix size will be smaller. Number of cols are detected from statrRow
  // By default consecutive delimiters are seen as one, unless you set strictDelimiter to true.
  // Strict delimiter should be given in uSep. It **can not** be empty or space or tab.
  static Eigen::MatrixXd readMatrix(const char *filename, const char *uSep, int startRow = 0, int startCol = 0,
                             int endRow = -1, int endCol = -1, bool strictDelimiter = false,
                             double fillValue = std::numeric_limits<double>::quiet_NaN());

  // Read numbers in the string to a row vector. Number of data in this line will be in actualNumOfData
  // if it is not NULL.
  // nReadData can be used to control the length of the output vector, if actual number of data is less than nReadData,
  // the rest will be filled with fillValue. If nData is -1, vector length will be same as nActualData.
  // If strictDelimiter is used, empty data will be filled with fillValue.
  // Strict delimiter should be given in uSep. It **can not** be empty or space or tab.
  static Eigen::RowVectorXd readRowVector(const std::string &line, const char *uSep, int *nActualData = NULL,
                                   int nReadData = -1, bool strictDelimiter = false,
                                   double fillValue = std::numeric_limits<double>::quiet_NaN());

  // convenient version if you don't use strictdelimiter
  inline static Eigen::RowVectorXd readRowVector(const std::string &line, int nReadData = -1,
                                   double fillValue = std::numeric_limits<double>::quiet_NaN())
  {
    return readRowVector(line, "", NULL, nReadData, false, fillValue);
  }


  template<typename Derived>
  static bool writeMatrix(const Eigen::DenseBase<Derived>& mat, const char *filename, const char *sep = " ",
                          int startRow = 0, int startCol = 0,
                          int endRow = -1, int endCol = -1);

  // convert vector of vec3 to a vector.size x 3 matrix
  template<typename T>
  static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>
  convertToMatrix(const std::vector<glm::detail::tvec3<T> > &vec)
  {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> result(vec.size(), 3);
    for (int r=0; r < result.rows(); r++) {
      result(r, 0) = vec[r].x;
      result(r, 1) = vec[r].y;
      result(r, 2) = vec[r].z;
    }
    return result;
  }

  template<typename T>
  static Eigen::Matrix<T, Eigen::Dynamic, 1> convertToVector(const std::vector<T> &vec)
  {
    Eigen::Matrix<T, Eigen::Dynamic, 1> result(vec.size());
    memcpy(result.data(), &(vec[0]), vec.size()*sizeof(T));
    return result;
  }

  template<typename T>
  static Eigen::Matrix<T, 1, Eigen::Dynamic> convertToRowVector(const std::vector<T> &vec)
  {
    Eigen::Matrix<T, 1, Eigen::Dynamic> result(vec.size());
    memcpy(result.data(), &(vec[0]), vec.size()*sizeof(T));
    return result;
  }

  // similar to matlab corrcoef
  template<typename Real>
  static Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> corrcoef(
      const Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic>& x)
  {
    typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> MatrixXni;
    MatrixXni cov = featureCovariance(x);
    MatrixXni res(cov.rows(), cov.cols());
    for (int r=0; r<cov.rows(); ++r) {
      for (int c=r; c<cov.cols(); ++c) {
        if (r == c) {
          res(r,c) = 1.0;
        } else {
          res(r,c) = cov(r,c) / std::sqrt(cov(r,r)*cov(c,c));
          if (std::abs(res(r,c)) > 1.0)
            res(r,c) = res(r,c) / std::abs(res(r,c));
          res(c,r) = res(r,c);
        }
      }
    }
    return res;
  }

  static Eigen::MatrixXd removeRowsContainNaNOrInF(const Eigen::MatrixXd &srcMat);

  template<typename Derived>
  inline static Derived clampMatrix(const Eigen::MatrixBase<Derived>& srcMat,
                                    typename Derived::Scalar inf,
                                    typename Derived::Scalar sup)
  {
    return srcMat.unaryExpr(CwiseClampOp<typename Derived::Scalar>(inf, sup));
  }

  template<typename Derived>
  inline static bool matrixIsFinite(const Eigen::MatrixBase<Derived>& x)
  {
     return ( (x - x).array() == (x - x).array()).all();
  }


  template<typename Derived>
  inline static bool matrixIsNotNaN(const Eigen::MatrixBase<Derived>& x)
  {
     return ((x.array() == x.array())).all();
  }

  template<typename Real>
  inline static bool matrixIsPositiveDefinite(
      const Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic>& x,
      Real thre = Eigen::NumTraits<Real>::dummy_precision())
  {
     Eigen::SelfAdjointEigenSolver<Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> > es;
     es.compute(x);
     return (es.eigenvalues().array() > thre).all();
  }

  template<typename Derived>
  inline static Derived matrixDigamma(const Eigen::MatrixBase<Derived>& x)
  {
    return x.unaryExpr(std::pointer_to_unary_function<typename Derived::Scalar, typename Derived::Scalar>
                       (boost::math::digamma<typename Derived::Scalar>));
  }

  template<class T>
  inline static typename Eigen::NumTraits<T>::NonInteger digamma(const T x)
  {
    return boost::math::digamma((typename Eigen::NumTraits<T>::NonInteger)x);
  }

  template<typename Derived>
  inline static Derived matrixGammaln(const Eigen::MatrixBase<Derived>& x)
  {
    return x.unaryExpr(std::pointer_to_unary_function<typename Derived::Scalar, typename Derived::Scalar>
                       (boost::math::lgamma<typename Derived::Scalar>));
  }

  template<class T>
  inline static typename Eigen::NumTraits<T>::NonInteger gammaln(const T x)
  {
    return boost::math::lgamma((typename Eigen::NumTraits<T>::NonInteger)x);
  }

  // Compute log(det(x)) where x is positive-definite
  template<class T>
  inline static typename Eigen::NumTraits<T>::NonInteger logdet(
      const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x)
  {
    typedef Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger,
        Eigen::Dynamic, Eigen::Dynamic> MatrixXni;
    MatrixXni floatx = x.template cast<typename Eigen::NumTraits<T>::NonInteger>();
    Eigen::LLT<MatrixXni> chol;
    chol.compute(floatx);
    if (chol.info() == Eigen::Success) {
      MatrixXni c = chol.matrixU();
      return c.diagonal().array().log().sum() * 2;
    } else {
      LERROR() << "logdet: Matrix is not positive definite";
      return 0;
    }
  }

  // returns the log of multivariate gamma(n, alpha) value.
  inline static double mvtGammaln(double n, double alpha)
  {
    Eigen::VectorXd tmp = 0.5*(Eigen::VectorXd::LinSpaced(n, -n, -1).array()+1) + alpha;
    return ((n*(n-1))/4)*std::log(M_PI) + matrixGammaln(tmp).sum();
  }

  inline static float mvtGammaln(float n, float alpha)
  {
    Eigen::VectorXf tmp = alpha+0.5*(Eigen::VectorXf::LinSpaced(n, -n, -1).array()+1);
    return ((n*(n-1))/4)*std::log(M_PI) + matrixGammaln(tmp).sum();
  }

  inline static long double mvtGammaln(long double n, long double alpha)
  {
    Eigen::Matrix<long double, Eigen::Dynamic, 1> tmp = alpha+
        0.5*(Eigen::Matrix<long double, Eigen::Dynamic, 1>::LinSpaced(n, -n, -1).array()+1);
    return ((n*(n-1))/4)*std::log(M_PI) + matrixGammaln(tmp).sum();
  }

  // return log(sum(exp(x))) along each row
  template<class T>
  inline static Eigen::Matrix<T, Eigen::Dynamic, 1> logsumexpRow(
      const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &x)
  {
    Eigen::Matrix<T, Eigen::Dynamic, 1> y = x.rowwise().maxCoeff();
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> xdiff = x - y*Eigen::Matrix<T, 1, Eigen::Dynamic>::Ones(x.cols());
    Eigen::Matrix<T, Eigen::Dynamic, 1> s = y + log(exp(xdiff.array()).rowwise().sum()).matrix();
    for (int i=0; i<s.size(); i++) {
      if (s(i) == std::numeric_limits<T>::infinity()) {
        s(i) = y(i);
      }
    }
    return s;
  }

  // return log(sum(exp(x))) along each col
  template<class T>
  inline static Eigen::Matrix<T, 1, Eigen::Dynamic> logsumexpCol(
      const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &x)
  {
    Eigen::Matrix<T, 1, Eigen::Dynamic> y = x.colwise().maxCoeff();
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> xdiff = x - y*Eigen::Matrix<T, Eigen::Dynamic, 1>::Ones(x.rows());
    Eigen::Matrix<T, 1, Eigen::Dynamic> s = y + log(exp(xdiff.array()).colwise().sum()).matrix();
    for (int i=0; i<s.size(); i++) {
      if (s(i) == std::numeric_limits<T>::infinity()) {
        s(i) = y(i);
      }
    }
    return s;
  }

  template<class T>
  inline static size_t rank(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x, double thre = 1e-8)
  {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> xcopy = x;
    for (int i=0; i<x.cols(); i++)
      for (int j=0; j<x.rows(); j++)
        if (x(j,i) < std::numeric_limits<T>::epsilon())
          xcopy(j,i) = 0;
    Eigen::FullPivLU<Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> > lu_decomp(xcopy);
    lu_decomp.setThreshold(thre);
    return lu_decomp.rank();
  }

  // replicate features based on weight
  template<class T, class WeightT>
  inline static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> replicateFeature(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                                             const Eigen::Matrix<WeightT, Eigen::Dynamic, 1>& weight)
  {
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> mat((size_t)(weight.sum()), x.cols());
    size_t rowIdx = 0;
    for (int i=0; i<x.rows(); i++) {
      mat.block(rowIdx, 0, weight(i), x.cols()) = x.row(i).replicate(weight(i),1);
      rowIdx += weight(i);
    }
    return mat;
  }

  // similar to matlab mean, get mean of each column
  template<class T>
  inline static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, 1, Eigen::Dynamic>
  featureMean(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x)
  {
    return featureMeanImpl<T>::mean(x);
  }

  template<typename T, typename WeightT>
  inline static Eigen::Matrix<typename MaxFloatType<T,WeightT>::type, 1, Eigen::Dynamic> featureMean(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                                                                       const Eigen::Matrix<WeightT, Eigen::Dynamic, 1>& weight)
  {
    typedef typename MaxFloatType<T,WeightT>::type ResultDataType;
    Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> newX;
    Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> res(x.cols());
    newX = x.template cast<ResultDataType>();
    for (int i=0; i<newX.cols(); ++i)
      res(i) = newX.col(i).dot(weight);
    return res / weight.sum();
  }

  // sample covariance
  template<class T>
  static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
  featureCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                    bool bias = false);

  // sample weighted covariance
  template<typename T, typename WeightT>
  static Eigen::Matrix<typename MaxFloatType<T,WeightT>::type, Eigen::Dynamic, Eigen::Dynamic>
  featureCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                    const Eigen::Matrix<WeightT, Eigen::Dynamic, 1>& weight,
                    bool duplicateObservation = true,
                    bool bias = false);

  // The shrunk Ledoit-Wolf covariance, return invertible covariance matrix estimator
  //"A Well-Conditioned Estimator for Large-Dimensional Covariance Matrices", Journal of Multivariate Analysis, Volume 88,
  // Issue 2, February 2004, pages 365-411.
  // if inputShrink is specified, then this const. is used for shrinkage. inputShrink should be range [0 1]
  // shrink from biased sample covariance to I.*(trace(cov)/p)
  template<class T>
  static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
  featureLWShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                            typename Eigen::NumTraits<T>::NonInteger *outputShrink = NULL,
                            typename Eigen::NumTraits<T>::NonInteger inputShrink = -1);

  // shrink from biased sample covariance to diagonal matrix
  template<class T>
  static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
  featureLWDiagShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                typename Eigen::NumTraits<T>::NonInteger *outputShrink = NULL,
                                typename Eigen::NumTraits<T>::NonInteger inputShrink = -1);

  // shrink from unbiased sample covariance to diagonal matrix  , from pmtk3
  template<class T>
  static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
  featureLWDiagShrunkUnbiasCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                      typename Eigen::NumTraits<T>::NonInteger *outputShrink = NULL,
                                      typename Eigen::NumTraits<T>::NonInteger inputShrink = -1);

  // Estimate covariance with the Oracle Approximating Shrinkage algorithm. Under the assumption that the data are
  // Gaussian distributed, This shrinkage
  // coefficient yields a smaller Mean Squared Error than the one given by Ledoit and Wolf's formula.
  // Chen et al., "Shrinkage Algorithms for MMSE Covariance Estimation", IEEE Trans. on Sign. Proc., Volume 58,
  // Issue 10, October 2010.
  // if inputShrink is specified, then this const. is used for shrinkage. inputShrink should be range [0 1]
  // shrink from biased sample covariance to I.*(trace(cov)/p)
  template<class T>
  static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
  featureOAShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                            typename Eigen::NumTraits<T>::NonInteger *outputShrink = NULL,
                            typename Eigen::NumTraits<T>::NonInteger inputShrink = -1);

  template<class T>
  static Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> featureUnique(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x, int *num = NULL);


private:
  template<class T, bool IsInteger = Eigen::NumTraits<T>::IsInteger>
  struct featureMeanImpl
  {
    static Eigen::Matrix<T, 1, Eigen::Dynamic> mean(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x)
    {
      return x.colwise().mean();
    }
  };
  template<class T>
  struct featureMeanImpl<T, true>
  {
    static Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, 1, Eigen::Dynamic> mean(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x)
    {
      return x.template cast<typename Eigen::NumTraits<T>::NonInteger>().colwise().mean();
    }
  };
};


///////////////////////////////////////////////  impl
template<typename Derived>
bool ZEigenUtils::writeMatrix(const Eigen::DenseBase<Derived>& mat, const char *filename, const char *sep,
                        int startRow, int startCol,
                        int endRow, int endCol)
{
  if (endRow == -1)
    endRow = mat.rows();
  if (endCol == -1)
    endCol = mat.cols();
  if (startRow < 0 || startCol < 0 || endRow <= startRow || endCol <= startCol || endRow > mat.rows()
      || endCol > mat.cols()) {
    LERROR() << "writeMatrix: wrong matrix range";
    return false;
  }

  //open file
  std::ofstream outFile(filename);
  if (!outFile.is_open()) {
    std::perror((std::string("writeMatrix: error while opening file ") + filename).c_str());
    return false;
  }

  std::string sepstr(sep);
  if (sepstr == " " || sepstr == "") {
    Eigen::IOFormat fmt(Eigen::FullPrecision, Eigen::DontAlignCols, " ");
    if (startRow == 0 && startCol == 0 && endRow == mat.rows() && endCol == mat.cols()) {
      outFile << mat.format(fmt);
    } else {
      outFile << mat.block(startRow, endRow-startRow, startCol, endCol-startCol).format(fmt);
    }
  } else {
    Eigen::IOFormat fmt(Eigen::FullPrecision, Eigen::DontAlignCols, sepstr);
    if (startRow == 0 && startCol == 0 && endRow == mat.rows() && endCol == mat.cols()) {
      outFile << mat.format(fmt);
    } else {
      outFile << mat.block(startRow, endRow-startRow, startCol, endCol-startCol).format(fmt);
    }
  }

  outFile.close();
  return true;
}

template<class T>
Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x, bool bias)
{
  assert(x.rows() > 0);
  typedef typename Eigen::NumTraits<T>::NonInteger ResultDataType;
  ResultDataType factor = 0;
  if (! bias && x.rows() > 1)
    factor = 1.0/(x.rows()-1.0);
  else
    factor = 1.0/x.rows();
  // convert to result type
  const Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> &rtX = x.template cast<ResultDataType>();
  Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> mean = featureMean(rtX);
  return (rtX.rowwise() - mean).transpose() * (rtX.rowwise() - mean) * factor;
}

template<class T, class WeightT>
Eigen::Matrix<typename MaxFloatType<T,WeightT>::type, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                               const Eigen::Matrix<WeightT, Eigen::Dynamic, 1>& weight,
                               bool duplicateObservation, bool bias)
{
  assert(x.rows() > 0 && weight.rows() >= x.rows());
  typedef typename MaxFloatType<T,WeightT>::type ResultDataType;
  ResultDataType factor = 0;
  // convert to result type
  const Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> &rtX = x.template cast<ResultDataType>();
  const Eigen::Matrix<ResultDataType, Eigen::Dynamic, 1> &rtWeight = weight.template cast<ResultDataType>();
  if (! bias && rtWeight.sum() > 1) {
    if (duplicateObservation) {
      // if we treat weight as duplicated observation, then the factor should be :
      // (this estimation of covariance will be smaller than the normal one)
      factor = 1.0/(rtWeight.sum()-1.0);
    } else {
      // http://en.wikipedia.org/wiki/Sample_covariance_matrix#Weighted_samples
      ResultDataType weightsSum = rtWeight.sum();
      ResultDataType weightsSquareSum = rtWeight.array().square().sum();
      factor = weightsSum / (weightsSum*weightsSum - weightsSquareSum);
    }
  } else {
    factor = 1.0/rtWeight.sum();
  }
  Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> mean = featureMean(rtX, rtWeight);
  return (rtX.rowwise() - mean).transpose() * rtWeight.asDiagonal() * (rtX.rowwise() - mean) * factor;
}

template<class T>
Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureLWShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                       typename Eigen::NumTraits<T>::NonInteger *outputShrink,
                                       typename Eigen::NumTraits<T>::NonInteger inputShrink)
{
  assert(x.rows() > 0);
  assert(x.cols() > 0);
  assert(inputShrink == -1 || (inputShrink >= 0 && inputShrink <= 1));
  typedef typename Eigen::NumTraits<T>::NonInteger ResultDataType;
  typedef Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> ResultMatType;
  typedef Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> ResultRowVecType;
  int nsample = x.rows();
  int ndim = x.cols();
  ResultMatType samplecov = featureCovariance(x, true);
  ResultMatType prior = ResultMatType::Identity(ndim, ndim) * (samplecov.trace() / ndim);

  if (samplecov.isApprox(prior)) {
    if (outputShrink != NULL)
      *outputShrink = 0;
    return samplecov;
  }

  if (inputShrink != -1) {
    if (outputShrink != NULL)
      *outputShrink = inputShrink;
    return inputShrink*prior + (1-inputShrink)*samplecov;
  }

  ResultMatType mat = x.template cast<ResultDataType>();
  ResultRowVecType matmean = featureMean(mat);
  mat.rowwise() -= matmean;
  ResultMatType mat2 = mat.array().square();
  ResultMatType phiMat = mat2.transpose() * mat2 / nsample - samplecov.array().square().matrix();
  ResultDataType phi = phiMat.sum();
  ResultDataType gamma = (samplecov - prior).squaredNorm();
  ResultDataType kappa = phi/gamma;
  ResultDataType shrinkage = std::max(0.0, std::min(1.0, kappa/nsample));
  if (outputShrink != NULL)
    *outputShrink = shrinkage;
  return shrinkage*prior + (1-shrinkage)*samplecov;
}

template<class T>
Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureLWDiagShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                           typename Eigen::NumTraits<T>::NonInteger *outputShrink,
                                           typename Eigen::NumTraits<T>::NonInteger inputShrink)
{
  assert(x.rows() > 0);
  assert(x.cols() > 0);
  assert(inputShrink == -1 || (inputShrink >= 0 && inputShrink <= 1));
  typedef typename Eigen::NumTraits<T>::NonInteger ResultDataType;
  typedef Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> ResultMatType;
  typedef Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> ResultRowVecType;
  int nsample = x.rows();
  ResultMatType samplecov = featureCovariance(x, true);
  ResultMatType prior = samplecov.diagonal().asDiagonal();

  if (samplecov.isApprox(prior)) {
    if (outputShrink != NULL)
      *outputShrink = 0;
    return samplecov;
  }

  if (inputShrink != -1) {
    if (outputShrink != NULL)
      *outputShrink = inputShrink;
    return inputShrink*prior + (1-inputShrink)*samplecov;
  }

  ResultMatType mat = x.template cast<ResultDataType>();
  ResultRowVecType matmean = featureMean(mat);
  mat.rowwise() -= matmean;
  ResultMatType mat2 = mat.array().square();
  ResultMatType phiMat = mat2.transpose() * mat2 / nsample - samplecov.array().square().matrix();
  ResultDataType phi = phiMat.sum();
  ResultDataType rho = phiMat.diagonal().sum();
  ResultDataType gamma = (samplecov - prior).squaredNorm();
  ResultDataType kappa = (phi-rho)/gamma;
  ResultDataType shrinkage = std::max(0.0, std::min(1.0, kappa/nsample));
  if (outputShrink != NULL)
    *outputShrink = shrinkage;
  return shrinkage*prior + (1-shrinkage)*samplecov;
}

template<class T>
Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureLWDiagShrunkUnbiasCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                                 typename Eigen::NumTraits<T>::NonInteger *outputShrink,
                                                 typename Eigen::NumTraits<T>::NonInteger inputShrink)
{
  assert(x.rows() > 0);
  assert(x.cols() > 0);
  assert(inputShrink == -1 || (inputShrink >= 0 && inputShrink <= 1));
  typedef typename Eigen::NumTraits<T>::NonInteger ResultDataType;
  typedef Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> ResultMatType;
  typedef Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> ResultRowVecType;
  int nsample = x.rows();
  int ndim = x.cols();
  ResultMatType samplecov = featureCovariance(x);
  ResultMatType biasedcov = featureCovariance(x, true);
  ResultMatType prior = samplecov.diagonal().asDiagonal();

  if (samplecov.isApprox(prior)) {
    if (outputShrink != NULL)
      *outputShrink = 0;
    return samplecov;
  }

  if (inputShrink != -1) {
    if (outputShrink != NULL)
      *outputShrink = inputShrink;
    return inputShrink*prior + (1-inputShrink)*samplecov;
  }

  ResultMatType mat = x.template cast<ResultDataType>();
  ResultRowVecType matmean = featureMean(mat);
  mat.rowwise() -= matmean;
  ResultMatType varS = ResultMatType::Zero(ndim, ndim);
  for (int i=0; i<nsample; i++) {
    varS += (mat.row(i).transpose() * mat.row(i) - biasedcov).array().square().matrix();
  }
  varS *= nsample/((nsample-1.0)*(nsample-1.0)*(nsample-1.0));
  ResultMatType varSup = varS.template triangularView<Eigen::StrictlyUpper>();
  ResultMatType Sup = samplecov.template triangularView<Eigen::StrictlyUpper>();
  ResultDataType num = varSup.sum();
  ResultDataType den = Sup.array().square().sum();
  ResultDataType shrinkage = std::max(0.0, std::min(1.0, num/den));
  if (outputShrink != NULL)
    *outputShrink = shrinkage;
  return shrinkage*prior + (1-shrinkage)*samplecov;
}

template<class T>
Eigen::Matrix<typename Eigen::NumTraits<T>::NonInteger, Eigen::Dynamic, Eigen::Dynamic>
ZEigenUtils::featureOAShrunkCovariance(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x,
                                       typename Eigen::NumTraits<T>::NonInteger *outputShrink,
                                       typename Eigen::NumTraits<T>::NonInteger inputShrink)
{
  assert(x.rows() > 0);
  assert(x.cols() > 0);
  assert(inputShrink == -1 || (inputShrink >= 0 && inputShrink <= 1));
  typedef typename Eigen::NumTraits<T>::NonInteger ResultDataType;
  typedef Eigen::Matrix<ResultDataType, Eigen::Dynamic, Eigen::Dynamic> ResultMatType;
  typedef Eigen::Matrix<ResultDataType, 1, Eigen::Dynamic> ResultRowVecType;
  int nsample = x.rows();
  int ndim = x.cols();
  ResultMatType samplecov = featureCovariance(x, true);
  ResultMatType prior = ResultMatType::Identity(ndim, ndim) * (samplecov.trace() / ndim);

  if (samplecov.isApprox(prior)) {
    if (outputShrink != NULL)
      *outputShrink = 0;
    return samplecov;
  }

  if (inputShrink != -1) {
    if (outputShrink != NULL)
      *outputShrink = inputShrink;
    return inputShrink*prior + (1-inputShrink)*samplecov;
  }

  ResultMatType mat = x.template cast<ResultDataType>();
  // from Chen et al.'s implementation (not same as paper)
  ResultRowVecType matmean = featureMean(mat);
  mat.rowwise() -= matmean;
  ResultDataType traceS = samplecov.trace();
  ResultDataType traceS2 = traceS * traceS;
  ResultDataType traceSS = (samplecov*samplecov).trace();
  ResultDataType shrinkage = ( traceS2+traceSS ) / ( (nsample+1.0)* (traceSS-traceS2/ndim) );
  //NonInteger mu = samplecov.trace()/ndim;
  //NonInteger alpha = samplecov.array().square().mean();
  //NonInteger num = alpha + mu*mu;
  //NonInteger den = (nsample+1.0)*(alpha-mu*mu/ndim);
  //NonInteger shrinkage = num/den;
  shrinkage = std::min(1.0, shrinkage);
  if (outputShrink != NULL)
    *outputShrink = shrinkage;
  return shrinkage*prior + (1-shrinkage)*samplecov;
}

template<class T>
Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> ZEigenUtils::featureUnique(const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& x, int *num)
{
  typedef Eigen::Matrix<T, 1, Eigen::Dynamic> VectorXt;
  int nUniqueData = 0;
  Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> uniqueMat(x.rows(), x.cols());
  std::set<VectorXt,ZVectorCompare<T> > myset;
  std::pair<typename std::set<VectorXt,ZVectorCompare<T> >::iterator,bool> ret;
  for (int r=0; r < x.rows(); r++) {
    ret = myset.insert(x.row(r));
    if (ret.second != false) {
      uniqueMat.row(nUniqueData++) = x.row(r);
    }
  }
  if (num != NULL)
    *num = nUniqueData;
  uniqueMat.conservativeResize(nUniqueData, Eigen::NoChange);
  return uniqueMat;
}

#endif // ZEIGENUTILS_H
