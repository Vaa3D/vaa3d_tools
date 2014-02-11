/* index2binary_plugin.cpp
 * Convert a volume index image to a binary image based on user selection.
 * 2014-02-10 : by Brian Long
 */

#include "v3d_message.h"
#include <vector>
#include <iostream>

#include "index2binary_plugin.h"

using namespace std;
Q_EXPORT_PLUGIN2(index2binary, Index2Binary);


void index2binaryFunc(V3DPluginCallback2 &callback, QWidget *parent);



QStringList Index2Binary::menulist() const
{
    return QStringList()
            <<tr("index2binary")
           <<tr("about");
}

QStringList Index2Binary::funclist() const
{
    return QStringList()
            <<tr("index2binary")
           <<tr("help");
}

void Index2Binary::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("index2binary"))
    {
        // handle everything in my internal function
        index2binaryFunc(callback, parent);
    }
    else
    {
        v3d_msg(tr("Convert a volume index image to a binary image based on user selection.. "
                   "Developed by Brian Long, 2014-02-10"));
    }
}

bool Index2Binary::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("index2binary"))
    {
        v3d_msg("To be implemented.");
    }
    else if (func_name == tr("help"))
    {
        v3d_msg("To be implemented.");
    }
    else return false;

    return true;
}

// internal function processes the current image and returns a binary image
// where the current image is equal to a user-supplied
void index2binaryFunc(V3DPluginCallback2 &callback, QWidget *parent)
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



    unsigned char* data1d = p4DImage->getRawData(); //initialize a new pointer to the rawdata in the current window

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG C = p4DImage->getCDim();
    V3DLONG size = p4DImage->getTotalBytes();
    ImagePixelType pixeltype = p4DImage->getDatatype();
    unsigned char * data1d2 = 0; // initialize a new pointer to be used for a copy of data1d.
    data1d2 = new unsigned char [size]; // and now allocate enough memory for a copy of data1d.
    //use memcpy to directly copy 'size' bytes from the location pointed to by data1d to the location pointed to by data1d2
    // critical to get 'size' right here.
    memcpy(data1d2,data1d,size);
    V3DLONG npixels = N*M*P*C;


cout<<N<<endl;
cout<<M<<endl;
cout<<P<<endl;
cout<<C<<endl;
cout<<npixels<<endl;
cout<<size<<endl;
V3DLONG maxIntensity = 0;
V3DLONG pixval =0;
bool ok1;
V3DLONG indexchoice =0;
    if (pixeltype == V3D_UINT8)
    {
        //  calculate max index:
        for(V3DLONG ii = 0; (ii<npixels); ii++)
        {
            if (data1d2[ii]>maxIntensity){maxIntensity=data1d2[ii];}
        }

        indexchoice = QInputDialog::getInteger(parent, "Index for Binary Image",
                                                          "Choose Index Value for Binary Image:",
                                                          1, 1, maxIntensity, 1, &ok1);
   }
    else if (pixeltype == V3D_UINT16)
    {
        v3d_msg("v3d_uint16");

        //  calculate max index:
        for(V3DLONG ii = 0; (ii<npixels); ii++)
        {
            pixval=data1d2[2*ii]+255*data1d2[2*ii+1];
            if (pixval>maxIntensity){maxIntensity=pixval;}
        }
cout<<pixval<<endl;
        indexchoice = QInputDialog::getInteger(parent, "Index for Binary Image",
                                                          "Choose Index Value for Binary Image:",
                                                          1, 1, maxIntensity, 1, &ok1);
    }
    else
    {
        v3d_msg("invalid image type");
        return;
    }

    if (!ok1){v3d_msg("Invalid choice of index");return;}
cout<<maxIntensity<<endl;
    cout<<indexchoice<<endl;

    // now process the data...
    for(V3DLONG ii = 0; (ii<npixels); ii++)
    {
        pixval=data1d2[2*ii]+255*data1d2[2*ii+1];

        if (pixval==indexchoice){
            data1d2[2*ii] = 1;
            data1d2[2*ii+1]=0;//data1d2[ii]/data1d2[ii];
        }
        else
        {
            data1d2[2*ii] = 0;
            data1d2[2*ii+1]=0;;//data1d2[ii]-data1d2[ii];
        }

    }





// display new data:  this is from ZZs median filter

Image4DSimple * new4DImage = new Image4DSimple(); //new Image4DSimple
new4DImage->setData((unsigned char *)data1d2, N, M, P, C, pixeltype);  // set data from NEW COPY here.
v3dhandle newwin = callback.newImageWindow();  //create new window
callback.setImage(newwin, new4DImage);        // set the image to be the new data
callback.setImageName(newwin, "Single Channel Image"); // name the window
callback.updateImageWindow(newwin);         //  update (?)




return;


}
