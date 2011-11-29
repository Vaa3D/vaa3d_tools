#ifndef V3D_PYTHON_CONVERT_QHASH_H_
#define V3D_PYTHON_CONVERT_QHASH_H_

/*
 * convert_qhash.h
 *
 *  Created on: Jan 04, 2011
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
#include <boost/python/dict.hpp>
#include <QHash>

namespace sandbox { namespace {

template<class HashType>
struct qhash_to_python_dict
{
    typedef typename HashType::key_type key_type;
    typedef typename HashType::const_iterator const_iterator;

    static PyObject* convert(HashType const& hash)
    {
        boost::python::dict pyDict;
        const_iterator i;
        for (i = hash.begin(); i != hash.end(); ++i)
        {
            pyDict[i.key()] = i.value();
        }
        return boost::python::incref( pyDict.ptr() );
    }
};

template<class HashType>
struct qhash_from_python_dict
{
    typedef typename HashType::key_type key_type;
    typedef typename HashType::mapped_type value_type;

    qhash_from_python_dict()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<HashType>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PyDict_Check(obj_ptr))
            return 0;

        boost::python::object py_obj(
                boost::python::handle<>( boost::python::borrowed( obj_ptr ) ));
        boost::python::dict py_dict =
                boost::python::extract<boost::python::dict>(py_obj);

        boost::python::list iterkeys =
                (boost::python::list)py_dict.iterkeys();
        for (int i = 0; i < boost::python::len(iterkeys); i++)
        {
            if (! boost::python::extract<key_type>(iterkeys[i]).check() )
                return 0;
            if (! boost::python::extract<value_type>(py_dict[iterkeys[i]]).check() )
                return 0;
        }

        return obj_ptr;
    }

    static void construct(
            PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        boost::python::object py_obj(
                boost::python::handle<>( boost::python::borrowed( obj_ptr ) ));
        boost::python::dict py_dict =
                boost::python::extract<boost::python::dict>(py_obj);

        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<HashType >*)
                data)->storage.bytes;
        new (storage) HashType();

        HashType *container = static_cast<HashType* >(storage);
        boost::python::list iterkeys =
                (boost::python::list)py_dict.iterkeys();
        for (int i = 0; i < boost::python::len(iterkeys); i++)
        {
            container->insert(
                    boost::python::extract<key_type>(iterkeys[i]),
                    boost::python::extract<value_type>(py_dict[iterkeys[i]]));
        }

        data->convertible = storage;
    }
};

}} // namespace sandbox::<anonymous>

template<class HashType>
void register_qhash_conversion()
{
    boost::python::to_python_converter<
            HashType,
            sandbox::qhash_to_python_dict<HashType> >();

    sandbox::qhash_from_python_dict<HashType>();
}

#endif // V3D_PYTHON_CONVERT_QHASH_H_
