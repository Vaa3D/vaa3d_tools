#include "texture_features.h"
#include "bilateral_filter.h"

long PyTextureFeature::Excute(char *argv[])
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    long result = -1;

    _putenv_s("PYTHONPATH", ".\\plugins\\imPreProcess\\");

    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("print(sys.path)");

    pName = PyUnicode_FromString(argv[0]);

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if(pModule)
    {
        pFunc = PyObject_GetAttrString(pModule, "cal_features");
        if(pFunc && PyCallable_Check(pFunc))
        {
            PyObject* pyParams1 = PyList_New(0);
            PyObject* pyParams2 = PyList_New(0);
            pArgs = PyTuple_New(2);
            for(V3DLONG i=0; i<m_tolSz; i++)
            {
                PyList_Append(pyParams1, Py_BuildValue("i", m_data1d[i]));
            }

            for(int i=0; i<4; i++)
            {
                PyList_Append(pyParams2, Py_BuildValue("i", m_imsz[i]));
            }

            PyTuple_SetItem(pArgs, 0, pyParams1);
            PyTuple_SetItem(pArgs, 1, pyParams2);

            pValue = PyObject_CallObject(pFunc, pArgs);

            Py_DECREF(pArgs);
            Py_DECREF(pyParams1);
            Py_DECREF(pyParams2);

            result = PyLong_AsLong(pValue);
            printf_s("C: run code = %ld\n", result);
            Py_DECREF(pValue);
        }
        else
        {
             printf("ERROR: function quality control\n");
             return result;
        }
    }
    else
    {
        printf_s("ERROR: Quality control: module not imported\n");
        return result;
    }

    Py_XDECREF(pFunc);
    Py_DECREF(pModule);

    Py_Finalize();
    return result;
}


bool PyTextureFeature::Calculate()
{

    return true;
}
