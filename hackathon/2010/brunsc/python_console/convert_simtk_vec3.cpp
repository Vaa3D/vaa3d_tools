#include "convert_simtk_vec3.hpp"
#include "SimTKcommon/SmallMatrix.h"
#include <boost/python.hpp>

namespace bp = boost::python;
namespace stk = SimTK;

struct vec3_to_python_tuple
{
    static PyObject* convert(SimTK::Vec3 const& v)
    {
        std::vector<SimTK::Real> values(&v[0], &v[0] + 3);
        return bp::incref(bp::tuple(values).ptr());
    }
};

struct vec3_from_python_tuple
{
    vec3_from_python_tuple() {
        bp::converter::registry::push_back(
            &convertible,
            &construct,
            bp::type_id<SimTK::Vec3>());
    }
    
    static void* convertible(PyObject* obj_ptr)
    {
        if( !PySequence_Check( obj_ptr ) ) {
            return 0;
        }

        if( !PyObject_HasAttrString( obj_ptr, "__len__" ) ) {
            return 0;
        }

        bp::object py_sequence( bp::handle<>( bp::borrowed( obj_ptr ) ) );
    
        if( 3 != bp::len( py_sequence ) ) {
            return 0;
        }

        // Ensure that the element type is correct
        for (Py_ssize_t i = 0; i < 3; ++i)
        {
            if (! boost::python::extract<double>(py_sequence[i]).check() )
                return 0;
        }
        
        return obj_ptr;
    }
    
    static void construct(
            PyObject* obj_ptr, 
            bp::converter::rvalue_from_python_stage1_data* data) 
    {
        // Fill in values
        double x, y, z;
        if (!PyArg_ParseTuple(obj_ptr, "ddd", &x, &y, &z))
        {
            // Raise exception, error will have been set by PyArg_ParseTuple
            bp::throw_error_already_set();
        }

        // Grab pointer to memory into which to construct the new Vec3
        typedef bp::converter::rvalue_from_python_storage<SimTK::Vec3> vec3_storage;
        void* const storage = reinterpret_cast<vec3_storage*>(data)->storage.bytes;
        
        // in-place construct the new QString using the character data
        // extraced from the python object
        new (storage) SimTK::Vec3(x, y, z);

        // Stash the memory chunk pointer for later use by boost.python
        data->convertible = storage;
    }

};

void register_simtk_vec3_conversion() {
    // std::cout << "registering vec3 conversion" << std::endl;
    // bp::to_python_converter<SimTK::Vec3, vec3_to_python_tuple>();
    vec3_from_python_tuple();
}
