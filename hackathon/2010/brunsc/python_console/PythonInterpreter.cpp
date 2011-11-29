#include "PythonInterpreter.h"
#include <iostream>
#include <cstdio>
#include <string>

namespace bp = boost::python;
using namespace std;


// On Linux, there can be some problems loading modules, into an embedded python interpreter,
// that is itself in a shared library dynamically loaded by an application.  That is exactly the
// situation we have here with V3D.
#ifdef __linux__
#include <dlfcn.h> // dlopen()
void *v3d_ref_to_python_library = NULL;
void apply_embedded_dynamic_python_hack()
{
	string pyversion_full = std::string(Py_GetVersion()); // e.g. "2.7 (#67, Dec 31 1997, 22:34:28) [GCC 2.7.2.2]"
	// Even if a sub-version of python is shown (e.g. "2.6.3"), we just want the "2.6".
	string pyversion = pyversion_full.substr(0, 3); // e.g. "2.7"
	string pylibname = std::string("libpython") + pyversion + ".so"; // e.g. "libpython2.7.so"
	v3d_ref_to_python_library = dlopen(pylibname.c_str(), RTLD_LAZY | RTLD_GLOBAL);
	if (! v3d_ref_to_python_library) {
		cerr << "Problem with explicit loading of python library (" << pylibname << "): " << dlerror() << endl;
	}
	/*
	if (pylib)
		cerr << "Python library load succeeded!" << endl;
	else
		cerr << "Python library load failed!" << endl;
	 */
}
void clean_up_embedded_dynamic_python_hack() {
	if (v3d_ref_to_python_library)
		dlclose(v3d_ref_to_python_library);
}
#else
void apply_embedded_dynamic_python_hack() {}
void clean_up_embedded_dynamic_python_hack() {}
#endif

PythonOutputRedirector::PythonOutputRedirector(PythonInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

void PythonOutputRedirector::write( std::string const& str )
{
    interpreter->onOutput(QString(str.c_str()));
}

void PythonOutputRedirector::write_wide( std::wstring const& ws )
{
    std::string temp;
    std::copy(ws.begin(), ws.end(), std::back_inserter(temp));
    QString s(temp.c_str());
    interpreter->onOutput(s);
}

PythonInputRedirector::PythonInputRedirector(PythonInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

// TODO - this is a hack that does not actually get user input
std::string PythonInputRedirector::readline()
{
    return interpreter->readline();
    // cerr << "readline" << endl;
    // return "\n"; // TODO this is a hack to avoid hanging during input.
}

std::string PythonInterpreter::readline()
{
    emit startReadline();
    readlineLoop.exec(); // block until readline
    return readlineString.toStdString();
}

PythonInterpreter::PythonInterpreter()
	: QObject(NULL),
	  stdinRedirector(this),
	  stdoutRedirector(this),
	  stderrRedirector(this)
{
	apply_embedded_dynamic_python_hack();
	try {
		Py_Initialize();

		// Using python Tkinter GUI requires that sys.argv be populated
		const char *argv[1] = {"python"};
		PySys_SetArgv(1, const_cast<char**>(argv));

		main_module = bp::object((
		  bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
		main_namespace = main_module.attr("__dict__");

		// Connect python stdout/stderr to output to GUI
		// Adapted from
		//   http://onegazhang.spaces.live.com/blog/cns!D5E642BC862BA286!727.entry
		main_namespace["PythonOutputRedirector"] =
			bp::class_<PythonOutputRedirector>(
					"PythonOutputRedirector", bp::init<>())
				.def("write", &PythonOutputRedirector::write)
                .def("write", &PythonOutputRedirector::write_wide)
				;
		main_namespace["PythonInputRedirector"] =
			bp::class_<PythonInputRedirector>(
					"PythonInputRedirector", bp::init<>())
				.def("readline", &PythonInputRedirector::readline)
				;

	    bp::import("sys").attr("stdin") = stdinRedirector;
	    bp::import("sys").attr("stdout") = stdoutRedirector;
		bp::import("sys").attr("stderr") = stderrRedirector;
	}
	catch( bp::error_already_set ) {
		PyErr_Print();
	}
}

PythonInterpreter::~PythonInterpreter() {
	// TODO - using Py_Finalize() may be unsafe with boost::python
	// http://www.boost.org/libs/python/todo.html#pyfinalize-safety
	// http://lists.boost.org/Archives/boost/2006/07/107149.php
	Py_Finalize();
	clean_up_embedded_dynamic_python_hack();
}

void PythonInterpreter::onOutput(QString msg) {
    emit outputSent(msg);
}

void PythonInterpreter::runScriptFile(QString fileName)
{
    QByteArray fname = fileName.toLocal8Bit();
	FILE *fp = fopen((const char*)fname, "r");
	if (fp) {
	    // Set argv to add file directory to os.path, just like regular python does
	    char *argv[1];
	    argv[0] = const_cast<char*>((const char*)(fname));
	    PySys_SetArgv(1, argv); // python < 2.6.6 does not have PySys_SetArgvEx()

		PyRun_SimpleFileEx(fp, (const char*)fname, 1); // 1 means close it for me

		// Revert path to what it was before PySys_SetArgv
		PyRun_SimpleString("import sys; sys.path.pop(0)\n");
	}
	emit commandComplete();
}

void PythonInterpreter::finishReadline(QString line)
{
    // Don't run command if python stdin is waiting for readline input
    readlineString = line;
    readlineLoop.exit();
    return;
}

void PythonInterpreter::interpretLine(QString line)
{
	std::string command0 = line.toStdString();

	// Skip empty lines
	if (command0.length() == 0) {
		emit commandComplete();
		return; // empty command
	}
	size_t firstNonSpacePos = command0.find_first_not_of(" \t\r\n");
	if (firstNonSpacePos == std::string::npos) {
		emit commandComplete();
		return; // all blanks command
	}
	if (command0[firstNonSpacePos] == '#') {
		emit commandComplete();
		return; // comment line
	}
	// Append newline for best parsing of nascent multiline commands.
	std::string command = command0 + "\n";

	try {
		// First compile the expression without running it.
		bp::object compiledCode(bp::handle<>(Py_CompileString(
				command.c_str(),
				"<stdin>",
				Py_single_input)));
		if (! compiledCode.ptr()) {
			// command failed
			emit commandComplete();
			return;
		}

		bp::object result(bp::handle<>( PyEval_EvalCode(
				(PyCodeObject*) compiledCode.ptr(),
				main_namespace.ptr(),
				main_namespace.ptr())));
	}
	catch( bp::error_already_set )
	{
		// Distinguish incomplete input from invalid input
		char *msg = NULL;
		PyObject *exc, *val, *obj, *trb;
		if (PyErr_ExceptionMatches(PyExc_SyntaxError))
		{
			PyErr_Fetch (&exc, &val, &trb);        /* clears exception! */
			if (PyArg_ParseTuple (val, "sO", &msg, &obj) &&
					!strcmp (msg, "unexpected EOF while parsing")) /* E_EOF */
			{
				Py_XDECREF (exc);
				Py_XDECREF (val);
				Py_XDECREF (trb);
				emit incompleteCommand(line);
				return;
			}
			PyErr_Restore (exc, val, trb);
		}

		PyErr_Print();
	}

	emit commandComplete();
	return;
}
