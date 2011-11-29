#include "SimTKcommon/SmallMatrix.h"
#include "indexing_suite/suite_utils.hpp"
#include "indexing_suite/container_suite.hpp"
#include "indexing_suite/suite_utils.hpp"
#include "indexing_suite/list.hpp"

// Include suite_utils to get index_style_t
using namespace boost::python::indexing;

// Code to help use boost python indexing suite v2 with SimTK::Vec3
namespace boost { namespace python { namespace indexing {

// Compile error, due to Vec3 as value type in map<> in AtomTargetLocations
// 3>src\molmodel\generated_code\AtomTargetLocations.pypp.cpp(7) : fatal error C1083: Cannot open include file: '_Vec_less__3_comma__double_comma__1__greater___value_traits.pypp.hpp': No such file or directory^M
template<>
struct value_traits< SimTK::Vec3 > {
    static bool const equality_comparable = false;
    static bool const less_than_comparable = false;
    template<typename PythonClass, typename Policy>
    static void visit_container_class(PythonClass &, Policy const &){}
};
template<>
struct value_traits< SimTK::Quaternion > {
    static bool const equality_comparable = false;
    static bool const less_than_comparable = false;
    template<typename PythonClass, typename Policy>
    static void visit_container_class(PythonClass &, Policy const &){}
};

template<class VEC>
struct vec_container_traits {
  typedef VEC                        container;
  typedef int                        size_type;
  typedef SimTK::Real                value_type;
  typedef value_type*                iterator;

  typedef value_type&                reference;
  typedef value_type                 key_type;
  typedef int                        index_type; // signed!

  typedef value_type                 value_param;
  typedef key_type                   key_param;
  typedef index_type                 index_param;

  static bool const has_copyable_iter = false;
  static bool const has_mutable_ref   = true;
  static bool const has_find          = true;
  static bool const has_insert        = false;
  static bool const has_erase         = false;
  static bool const has_pop_back      = false;
  static bool const has_push_back     = false;
  static bool const is_reorderable    = false;

  BOOST_STATIC_CONSTANT(
      method_set_type,
      supported_methods = (
            method_len
            | method_getitem
            | method_getitem_slice
      //    | method_index // requires begin and end methods, which Vec3 lacks
            | method_setitem
            | method_setitem_slice
      //    | method_contains // requires begin and end methods, which Vec3 lacks^M
      //    | method_count // compile error^M
      ));


  static boost::python::indexing::index_style_t const index_style
    = boost::python::indexing::index_style_linear;

  struct value_traits_ {
    // Traits information for our value_type
    static bool const equality_comparable = true;
    static bool const lessthan_comparable = true;
  };

  template<typename PythonClass, typename Policy>
  static void visit_container_class (PythonClass &, Policy const &)
  {
    // Empty
  }
};


}}}
