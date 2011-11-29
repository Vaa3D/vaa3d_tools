/*
 * convert_qstring.cpp
 *
 *  Created on: Dec 23, 2010
 *      Author: cmbruns
 *
 *  Adapted from
 *    http://www.boost.org/doc/libs/1_42_0/libs/python/doc/v2/faq.html#custom_string
 */

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/to_python_converter.hpp>
#include <QString>

namespace bp = boost::python;

namespace sandbox { namespace {
  struct qstring_to_python_str
  {
    static PyObject* convert(QString const& s)
    {
      return bp::incref(bp::object(s.toStdString()).ptr());
    }
  };

  struct qstring_from_python_str
  {
    qstring_from_python_str()
    {
      boost::python::converter::registry::push_back(
        &convertible,
        &construct,
        boost::python::type_id<QString>());
    }

    static void* convertible(PyObject* obj_ptr)
    {
      if (!PyString_Check(obj_ptr)) return 0;
      return obj_ptr;
    }

    static void construct(
      PyObject* obj_ptr,
      boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      const char* value = PyString_AsString(obj_ptr);
      if (value == 0) boost::python::throw_error_already_set();
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<QString>*)
          data)->storage.bytes;
      new (storage) QString(value);
      data->convertible = storage;
    }
  };


}} // namespace sandbox::<anonymous>

void register_qstring_conversion()
{
    // If there is a QString member variable, conversion is not enough.
    bp::to_python_converter<
           QString,
           sandbox::qstring_to_python_str>();

    sandbox::qstring_from_python_str();
}

// BOOST_PYTHON_MODULE(qstring)
// {
//   sandbox::register_qstring_conversion();
// }
