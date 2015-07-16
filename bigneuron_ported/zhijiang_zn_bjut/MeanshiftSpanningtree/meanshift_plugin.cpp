/* example_plugin.cpp
 * This is an example plugin perform binary thresholding on an image. You can use it as a demo
 * 2012-02-10 : by Yinan Wan
 */
 
#include "v3d_message.h"
#include <vector>

#include "meanshift_plugin.h"
#include "meanshift_func.h"


// 1- Export the plugin class to a target, the first item in the bracket should match the TARGET parameter in the .pro file
Q_EXPORT_PLUGIN2(meanshift, MeanShiftPlugin);
 

// 2- Set up the items in plugin domenu
QStringList MeanShiftPlugin::menulist() const
{
        return QStringList() 
                <<tr("meanshift")
                <<tr("about");
}

// 3 - Set up the function list in plugin dofunc
QStringList MeanShiftPlugin::funclist() const
{
        return QStringList()
                <<tr("meanshift")
                <<tr("help");
}

// 4 - Call the functions corresponding to the domenu items. 
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
void MeanShiftPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("meanshift"))
        {
            bool bmenu = true;

            v3dhandle curwin = callback.currentImageWindow();
            if(!curwin)
            {
                v3d_msg("No image is open.");
                return;
            }

            V3DLONG in_sz[4];
            Image4DSimple *p4d = callback.getImage(curwin);
            unsigned char* img1d = p4d->getRawDataAtChannel(1);
            QString image_name = callback.getImageName(curwin);
            in_sz[0] = p4d->getXDim();
            in_sz[1] = p4d->getYDim();
            in_sz[2] = p4d->getZDim();
            meanshift_plugin_vn4(callback,parent,img1d, in_sz,image_name,bmenu);
        }
        else if (menu_name == tr("about"))
        {
                v3d_msg(tr("This is a demo plugin to perform spanning tree algorithm which combined with breadth First search on the current image.\n"
                        "Developed by John Wan, 2012-02-10"));
        }
}

// 5 - Call the functions corresponding to dofunc
//     The functions may not necessarily be in example_func.cpp, but you are strongly recommended to do so
//     to seperate the Interface from the core functions, and it is consistant with the form in plugin_creator
bool MeanShiftPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
        if (func_name == tr("meanshift"))
        {
            bool bmenu = false;
            std::vector<char*> * pinfiles = (input.size() >= 1) ? (std::vector<char*> *) input[0].p : 0;
            std::vector<char*> * pparas = (input.size() >= 2) ? (std::vector<char*> *) input[1].p : 0;
            std::vector<char*> infiles = (pinfiles != 0) ? * pinfiles : std::vector<char*>();
            std::vector<char*> paras = (pparas != 0) ? * pparas : std::vector<char*>();
            QString image_name;
            if(infiles.empty())
            {
                fprintf (stderr, "Need input image. \n");
                return false;
            }
            else
                image_name = infiles[0];

            unsigned char* data1d = 0;
            V3DLONG in_sz[4];
            int datatype = 0;
            if (!simple_loadimage_wrapper(callback,image_name.toStdString().c_str(), data1d, in_sz, datatype))
            {
                v3d_msg("Error happens in reading the subject file [%s]. Exit.",0);
                return false;
            }
            meanshift_plugin_vn4(callback,parent,data1d,in_sz,image_name,bmenu);
        }
        else if (func_name == tr("help"))
        {
                printHelp();
        }
		return true;
}
