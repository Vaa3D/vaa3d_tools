/*
 * qlist_py_indexing.h
 *
 *  Created on: Dec 31, 2010
 *      Author: Christopher M. Bruns
 */

#ifndef QLIST_PY_INDEXING_H_
#define QLIST_PY_INDEXING_H_

#include "indexing_suite/container_suite.hpp"
#include "indexing_suite/suite_utils.hpp"
#include "indexing_suite/list.hpp"

using namespace boost::python::indexing;

// Pyplusplus includes a copy of the unreleased boost.python indexing_suite version 2,
// which wraps python-like indexing behavior onto C++ containers.
// This indexing suite requires the construction of a special "container_traits" struct
// for each wrapped container.  The following container_traits template struct
// is intended to help with the creation of this struct.  This struct will be invoked
// in "add_registration_code" calls in generate_whatever_source.py wrapping scripts.

template<class QListType>
struct qlist_container_traits {
    typedef QListType                      container;
    typedef typename QListType::size_type  size_type;
    typedef typename QListType::value_type value_type;
    typedef typename QListType::iterator   iterator;

    typedef typename QListType::reference  reference;
    typedef value_type                     key_type;
    typedef int                            index_type; // signed!

    typedef value_type                     value_param;
    typedef key_type                       key_param;
    typedef int                            index_param;

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
              | method_setitem
              | method_setitem_slice
              | method_index
              | method_contains
        //    | method_count // needs equal_to to work
        ));

    static boost::python::indexing::index_style_t const index_style
        = boost::python::indexing::index_style_linear;

    struct value_traits_ {
        // Traits information for our value_type
        static bool const equality_comparable = true;
        static bool const lessthan_comparable = false;
    };

    typedef value_traits<value_type> value_traits_type; // Dec 2010 CMB

    template<typename PythonClass, typename Policy>
        static void visit_container_class (PythonClass &, Policy const &)
    {
        // Empty
    }
};

#endif /* QLIST_PY_INDEXING_H_ */
