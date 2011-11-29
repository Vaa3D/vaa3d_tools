#ifndef V3D_PYTHON_CONVERT_QLIST_H_
#define V3D_PYTHON_CONVERT_QLIST_H_

/*
 * convert_qlist.h
 *
 *  Created on: Jan 03, 2011
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
#include <QList>
#include <QVector>

namespace sandbox { namespace {

template<class ListType>
struct qlist_to_python_list
{
    typedef typename ListType::value_type ELT;

    static PyObject* convert(ListType const& list)
    {
        boost::python::list pyList;
        for (int i = 0; i < list.size(); ++i)
        {
            pyList.append( boost::python::object(list[i]) );
        }
        return boost::python::incref( pyList.ptr() );
    }
};

template<class ListType>
struct qlist_from_python_list
{
    typedef typename ListType::value_type ELT;

    qlist_from_python_list()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<ListType>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PySequence_Check(obj_ptr))
            return 0;

        if( !PyObject_HasAttrString( obj_ptr, "__len__" ) ) {
            return 0;
        }

        boost::python::object py_sequence(
                bp::handle<>( bp::borrowed( obj_ptr ) ) );
        // Ensure each element is of the correct type (ELT)
        int n = PySequence_Size(obj_ptr);
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
                (boost::python::converter::rvalue_from_python_storage<ListType >*)
                data)->storage.bytes;
        new (storage) ListType();

        ListType *container = static_cast<ListType* >(storage);
        int n = PySequence_Size(obj_ptr);
        for (int i = 0; i < n; ++i)
            container->append( boost::python::extract<ELT>(pyList[i]) );

        data->convertible = storage;
    }
};

}} // namespace sandbox::<anonymous>

template<class ListType>
void register_qlist_conversion()
{
    boost::python::to_python_converter<
            ListType,
            sandbox::qlist_to_python_list<ListType> >();

    sandbox::qlist_from_python_list<ListType>();
}

#endif // V3D_PYTHON_CONVERT_QLIST_H_
