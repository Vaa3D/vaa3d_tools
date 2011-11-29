#ifndef V3D_PYTHON_CONVERT_C_ARRAY_STRUCT_H_
#define V3D_PYTHON_CONVERT_C_ARRAY_STRUCT_H_

/*
 * convert_c_array_struct.h
 *
 *  Created on: Jan 08, 2011
 *      Author: Christopher M. Bruns
 *
 *  Adapted from
 *    http://www.boost.org/doc/libs/1_42_0/libs/python/doc/v2/faq.html#custom_string
 *
 *    This header included boost/python, and thus should NOT be included
 *    by anything parsed by gccxml in pyplusplus.
 */

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/to_python_converter.hpp>
#include "c_array_struct.hpp"
#include <iostream>

//////////////////////////////////////////////////////////////
// Boost python indexing helpers for c_array wrapper struct //
//////////////////////////////////////////////////////////////

#include "indexing_suite/container_suite.hpp"
#include "indexing_suite/suite_utils.hpp"
#include "indexing_suite/list.hpp"

namespace bp = boost::python;
using namespace boost::python::indexing;

template<class ArrayType>
struct c_array_struct_container_traits
{
    typedef ArrayType                      container;
    typedef int                            size_type;
    typedef typename ArrayType::value_type value_type;
    typedef typename ArrayType::iterator   iterator;

    typedef value_type&                    reference;
    typedef value_type                     key_type;
    typedef typename ArrayType::index_type index_type; // signed!

    typedef value_type                     value_param;
    typedef key_type                       key_param;
    typedef index_type                     index_param;

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
        //    | method_contains // requires begin and end methods, which Vec3 lacks
        //    | method_count // compile error
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

//////////////////////////////////////////////////////////
// To/from Python converters for c_array wrapper struct //
//////////////////////////////////////////////////////////

namespace sandbox { namespace {

template<class ArrayType>
struct c_array_struct_to_python_list
{
    static PyObject* convert(ArrayType const& array)
    {
        boost::python::list pyList;
        for (int i = 0; i < array.size(); ++i)
            pyList.append( boost::python::object(array[i]) );
        return boost::python::incref( pyList.ptr() );
    }
};

template<class ArrayType>
struct c_array_struct_from_python_list
{
    typedef typename ArrayType::value_type ELT;

    c_array_struct_from_python_list()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<ArrayType>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PySequence_Check(obj_ptr))
            return 0;

        if( !PyObject_HasAttrString( obj_ptr, "__len__" ) ) {
            return 0;
        }

        int n = PySequence_Size(obj_ptr);
        if (ArrayType::size() != n)
            return 0;

        boost::python::object py_sequence(
                bp::handle<>( bp::borrowed( obj_ptr ) ) );
        // Ensure each element is of the correct type (ELT)
        for (Py_ssize_t i = 0; i < n; ++i)
        {
            if (! boost::python::extract<ELT>(py_sequence[i]).check() )
                return 0;
        }

        return obj_ptr;
    }

    static void construct(
            PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        boost::python::object pyList(boost::python::handle<>(boost::python::borrowed(obj_ptr)));

        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<ArrayType >*)
                data)->storage.bytes;
        new (storage) ArrayType();

        ArrayType *container = static_cast<ArrayType* >(storage);
        int n = PySequence_Size(obj_ptr);
        ArrayType& array = *container;
        for (int i = 0; i < n; ++i)
            array[i] = boost::python::extract<ELT>(pyList[i]);

        data->convertible = storage;
    }
};

}} // namespace sandbox::<anonymous>

template<class ArrayType>
void register_c_array_struct_conversion()
{
    // boost::python::to_python_converter<
    //         ArrayType,
    //         sandbox::c_array_struct_to_python_list<ArrayType> >();

    sandbox::c_array_struct_from_python_list<ArrayType>();
}

#endif // V3D_PYTHON_CONVERT_C_ARRAY_STRUCT_H_
