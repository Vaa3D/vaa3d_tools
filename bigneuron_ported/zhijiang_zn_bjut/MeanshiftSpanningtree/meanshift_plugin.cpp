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

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};
 

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

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, QString image_name,bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();

        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
    meanshift_plugin_vn4(callback,parent,data1d, in_sz,image_name,bmenu);

    //Output

      QString swc_name = PARA.inimg_file + "_fastmarching_spanningtree.swc";
//    NeuronTree nt;
//    nt.name = "tracing method";
//    writeSWC_file(swc_name.toStdString().c_str(),nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
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
            input_PARA PARA;
            in_sz[0] = p4d->getXDim();
            in_sz[1] = p4d->getYDim();
            in_sz[2] = p4d->getZDim();
           // meanshift_plugin_vn4(callback,parent,img1d, in_sz,image_name,bmenu);
            reconstruction_func(callback,parent,PARA,image_name,bmenu);
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
             input_PARA PARA;
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
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;

           // meanshift_plugin_vn4(callback,parent,data1d,in_sz,image_name,bmenu);
             reconstruction_func(callback,parent,PARA,image_name,bmenu);
        }
        else if (func_name == tr("help"))
        {
                printHelp();
        }
		return true;
}


