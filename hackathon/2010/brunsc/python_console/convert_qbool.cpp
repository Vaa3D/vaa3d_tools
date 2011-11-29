/*
 * convert_qbool.cpp
 *
 *  Created on: Jan 3, 2011
 *      Author: Christopher M. Bruns
 *
 *  Adapted from
 *    http://www.boost.org/doc/libs/1_42_0/libs/python/doc/v2/faq.html#custom_string
 */

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/to_python_converter.hpp>
#include <QBool>

namespace bp = boost::python;

namespace sandbox { namespace {
  struct qbool_to_python_bool
  {
    static PyObject* convert(QBool const& qbool)
    {
      return bp::incref(bp::object((bool)qbool).ptr());
    }
  };

  struct qbool_from_python_bool
  {
    qbool_from_python_bool()
    {
      boost::python::converter::registry::push_back(
        &convertible,
        &construct,
        boost::python::type_id<QBool>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
      if (!PyBool_Check(obj_ptr)) return 0;
      return obj_ptr;
    }

    static void construct(
      PyObject* obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      bool value = (obj_ptr == Py_True);
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<QBool>*)
          data)->storage.bytes;
      new (storage) QBool(value);
      data->convertible = storage;
    }
  };


}} // namespace sandbox::<anonymous>

void register_qbool_conversion()
{
    bp::to_python_converter<
            QBool,
            sandbox::qbool_to_python_bool>();

    sandbox::qbool_from_python_bool();
}
