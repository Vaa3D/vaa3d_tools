#include "v3d_message.h"

#include "r_plugin.h"
#include "r_miscfunc.h"
#include <vector>

using namespace std;

// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(r_plugin, RPlugin);


// 2- Set up the items in plugin domenu
QStringList RPlugin::menulist() const
{
	return QStringList()
		<< tr("Hello")
        << tr("ML demo")
        << tr("Image processing demo")
		<< tr("about");
}

// 3 - Set up the function list in plugin dofunc
QStringList RPlugin::funclist() const
{
    return QStringList();
        //<< tr("r_SVMclassify");
}

// 4 - Call the functions corresponding to the domenu items. 
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
void RPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Hello"))
	{
        v3d_msg(tr(hello().c_str())); //simple hello world protocol to show R works
	}
    else if (menu_name == tr("ML demo"))
    {
        cout<<"This is a basic command to demonstrate R's machine learning capabilities.\n Following along in the code is recommended, as there won't be much interactivity here."<<endl;
        vector<vector <double> > trainX(300, vector<double>(40, 0 ));
        vector<vector <double> > testX(300, vector<double>(40, 0 ));
        vector < double > trainY(300, 0 );
        for (int i=0;i<trainX.size();i++)
        {
            for (int j=0;j<trainX[i].size();j++)
            {
                trainX[i][j]=(rand()%1000)/1000.0;
                testX[i][j]=(rand()%1000)/1000.0;
            }    
        }
        for (int i=0;i<trainY.size();i++) trainY[i] = round((rand()%1000)/1000.0);
        

        cout<<"Three meaningless data structures initialized:\n    trainX, a 2D vector of 300x40 training features \n    trainY, a 1D vector of 300 training classifications \n    testX, a 2D vector of 300 (x40) unknown examples\nPassing to SVM function..."<<endl;
        vector <double> out = r_SVMclassify(trainX,trainY,testX); 
        for (int i=0;i<100;i++) cout<< round(out[i]) <<","<<trainY[i]<<endl;
        cout<<"1D vector of predicted class identities returned successfully."<<endl;
        v3d_msg(tr("Test passed."));

    }
    else if (menu_name == tr("Image processing demo"))
	{
		r_demo_processImage(callback,parent);
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("This is a test plugin to demonstrate the R interface."));
	}
	else
	{
		v3d_msg(tr("This is a test plugin to demonstrate the R interface.\n"
			"Also, unrecognized input."));
	}
}

// 5 - Call the functions corresponding to dofunc
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
bool RPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr(""))
	{
		
	}
	else if (func_name == tr("help"))
	{
		//printHelp();
	}
}
