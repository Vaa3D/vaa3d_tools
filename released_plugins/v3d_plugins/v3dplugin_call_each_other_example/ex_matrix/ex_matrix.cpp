/* ex_matrix.h
 * an example program to test dofunc function in the plugin interface
* 2010-08-23: by Zongcai Ruan
 */


#include "ex_matrix.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(ex_matrix, ExMatrixPlugin);

bool compute(const V3DPluginArgList & input, V3DPluginArgList & output, int method);
bool check_input_output(const V3DPluginArgList & input, V3DPluginArgList & output,
		QStringList & input_types, QStringList & output_types);

//plugin funcs
QStringList ExMatrixPlugin::funclist() const
{
    return QStringList()
		<< ("add_3x3")
		<< ("multiply_3x3")
		<< ("transpose_3x3");
}

bool ExMatrixPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
		 V3DPluginCallback2 & v3d,  QWidget * parent)
{
    if (func_name == ("add_3x3"))
    {
		return	compute(input, output, '+');
    }
	else if (func_name == ("multiply_3x3"))
	{
		return	compute(input, output, '*');
	}
	else if (func_name == ("transpose_3x3"))
	{
		return	compute(input, output, 't');
	}
    return false;
}


bool check_args(const V3DPluginArgList & args, const QStringList & types)
{
	if (args.size() < types.size())
	{
		qDebug("args size < expected size");
		return false;
	}
	for (int i=0; i<types.size(); i++)
	{
		if (args.at(i).type != types.at(i))
		{
			qDebug("args[%i] type is incorrect");
			return false;
		}
		if (args.at(i).p == 0)
		{
			qDebug("args[%i] pointer is null");
			return false;
		}
	}
	return true;
}
bool check_input_output(const V3DPluginArgList & input, V3DPluginArgList & output,
		QStringList & input_types, QStringList & output_types)
{
	qDebug("ex_matrix check input args....");
	if (! check_args(input, input_types))
		return false;

	qDebug("ex_matrix check output args....");
	if (! check_args(input, input_types))
		return false;

	return true;
}

bool compute(const V3DPluginArgList & input, V3DPluginArgList & output, int method_code)
{
	if (method_code=='+')
	{
    	QStringList input_types, output_types;
    	input_types <<"double3x3" <<"double3x3";
    	output_types <<"double3x3";
    	if (! check_input_output(input, output, input_types, output_types) )
    		return false;

    	const double* A = (const double*)(input.at(0).p);
    	const double* B = (const double*)(input.at(1).p);
    	double* C = (double*)(output.at(0).p);

    	for (int i=0; i<3*3; i++)
    	{
    		C[i] = A[i] + B[i];
    	}

		return true;
	}
	else if (method_code=='*')
	{
    	QStringList input_types, output_types;
    	input_types <<"double3x3" <<"double3x3";
    	output_types <<"double3x3";
    	if (! check_input_output(input, output, input_types, output_types) )
    		return false;

    	const double* A = (const double*)(input.at(0).p);
    	const double* B = (const double*)(input.at(1).p);
    	double* C = (double*)(output.at(0).p);
		#define A2(i,j) A[i*3+j]
		#define B2(i,j) B[i*3+j]
		#define C2(i,j) C[i*3+j]

    	for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
    	{
    		C2(i,j) = A2(i,0)*B2(0,j) + A2(i,1)*B2(1,j) + A2(i,2)*B2(2,j);
    	}

    	return true;
	}
	else if (method_code=='t')
	{
    	QStringList input_types, output_types;
    	input_types <<"double3x3";
    	output_types <<"double3x3";
    	if (! check_input_output(input, output, input_types, output_types) )
    		return false;

    	const double* A = (const double*)(input.at(0).p);
    	double* C = (double*)(output.at(0).p);
		#define A2(i,j) A[i*3+j]
		#define C2(i,j) C[i*3+j]

    	for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
    	{
    		C2(i,j) = A2(j,i);
    	}

    	return true;
	}
	return false;
}

