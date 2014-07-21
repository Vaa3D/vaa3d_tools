/* typeset_plugin.cpp
 * Typeset child brances based on node marker
 * 2014-06-26 : by Surobhi Ganguly
 */
 

#include "v3d_message.h"
#include "typeset_plugin.h"
#include "typeset_func.h"
#include "sort_func.h"
//#include "sort_func.cpp"

Q_EXPORT_PLUGIN2(typeset_swc, TypesetPlugin)

QStringList TypesetPlugin::menulist() const
{
    return QStringList()
        <<tr("typeset")
        <<tr("directions")
        <<tr("about");
}

QStringList TypesetPlugin::funclist() const
{
    return QStringList()
        <<tr("typeset_swc")
        <<tr("help");
}

void TypesetPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("typeset"))
    {
        typeset_swc(callback,parent);
    }
    else if (menu_name == tr("directions"))
    {
        v3d_msg(tr("1. Trace neuron. \n"
                   "2. Create marker files. Each marker file will set each branch type.\n"
                   "3. Run plugin and follow directions in opening swc and marker files. "));
    }
    else
    {
        v3d_msg(tr("This is a plugin to set branch type of traced neuron \n"
            "Developed by Surobhi Ganguly, 2014-07-14"));
    }
}

bool TypesetPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("typeset_swc"))
    {
        return typeset_swc(input, output);
    }
    else if (func_name == tr("TOOLBOXtypeset"))
    {
        typeset_swc_toolbox(input);
        return true;
    }
    if (func_name == tr("help"))
    {
        printf("\n\n(version 1.0) typeset points in a swc file indicate beginning of branch of type to be set. Developed by Surobhi Ganguly 2014-07-14\n");
        printf("usage:\n");
        printf("\t-f <function_name>:     typeset_swc\n");
        printf("\t-i <input_file_name>:   input .swc\n");
        printf("\t-o <output_file_name>:  (not required) typesetd swc file. DEFAULT: 'inputName_typesetd.swc'\n");
        printf("\t-p <step_length>:       step length for resampling.\n");
        printf("Demo: v3d -x typeset_swc -f typeset_swc -i test.swc -o test_typesetd.swc -p 1\n\n");
    }
    return false;
}


