/* updatepxlvalplugin.cpp
 * 2009-08-21: create this program by Yang Yu
 */
// upgrade to plugin interface 2.1 and add dofunc() interface by Jianlong Zhou, 2012-04-18.


#include <QtGui>
#include <iostream>
#include <math.h>
#include <stdlib.h>

#include "updatepxlvalplugin.h"
#include "stackutil.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(updatepxlval, UpdatePxlValPlugin)

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

const QString title = QObject::tr("Change Pixel Value Plugin");

QStringList UpdatePxlValPlugin::menulist() const
{
    return QStringList() << tr("Change Pixel Value")
                         <<tr("about this plugin");
}


void UpdatePxlValPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if(menu_name == tr("Change Pixel Value"))
    {
        processImage(callback,parent);
    }
    else if(menu_name == tr("about this plugin"))
    {
        QMessageBox::information(parent, "Version info",
                                 QString("Change Pixel Value %1 (2009): this plugin is developed by Yang Yu.")
                                 .arg(getPluginVersion()));
        return;
    }

}


QStringList UpdatePxlValPlugin::funclist() const
{
    return QStringList()
            <<tr("change_pixel_value")
           <<tr("help");
}


bool UpdatePxlValPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (func_name == tr("change_pixel_value"))
    {
        return processImage(callback, input, output);
    }
    else if(func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f change_pixel_value -i <inimg_file> -o <outimg_file> -p <nx> <ny> <nz> <chval1> [<chval2> ...] "<<endl;
        cout<<endl;
        cout<<"nx          pixel position to be changed in x direction, default 0"<<endl;
        cout<<"ny          pixel position to be changed in y direction, default 0"<<endl;
        cout<<"nz          pixel position to be changed in z direction, default 0"<<endl;
        cout<<"chval1      the pixel value to be set on channel 1, default 0. "<<endl;
        cout<<"            chval number should be same as the channel number. Otherwise the left channel value will be set to 0."<<endl;
        cout<<endl;
        cout<<"e.g. v3d -x dll_name -f change_pixel_value -i input.raw -o output.raw -p 1 2 3 0"<<endl;
        cout<<endl;
        return true;
    }
}

bool processImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to Single pixel value change plugin"<<endl;
    if (output.size() != 1) return false;

    V3DLONG nx = 0, ny = 0, nz = 0;
    vector<char*> paras;
    if (input.size()>=2)
    {
        paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size() >= 1) nx = atol(paras.at(0));
        if(paras.size() >= 2) ny = atol(paras.at(1));
        if(paras.size() >= 3) nz = atol(paras.at(2));

        // get new value after loading image
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    cout<<"nx = "<<nx<<endl;
    cout<<"ny = "<<ny<<endl;
    cout<<"nz = "<<nz<<endl;

    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];

    int datatype;
    if(simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    // parse new pixel value paras
    int* ncval = new int [sc];

    if(!ncval)
        return false;

    V3DLONG val_num = paras.size()-3;
    if(val_num>=sc)
    {
        for(int i=0; i<sc; i++)
        {
            if(paras.size() >= 4) ncval[i] = atoi(paras.at(i+3));
        }
    }
    else
    {
        // use set value and left be set as defult value 0
        for(int i=0; i<sc; i++)
        {
            if(i<val_num)
                ncval[i]=atoi(paras.at(i+3));
            else
                ncval[i] = 0;
        }
    }


    // ============================================

    //change the pixel value
    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offsetc = c*pagesz;
        V3DLONG indLoop = offsetc + nz*M*N + ny*N + nx;
        data1d[indLoop] = ncval[c];

        printf("ncval[%ld] %d \n", c, ncval[c]);
        printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[indLoop], nx, ny, nz, indLoop);
    }


    // ============================================
    // save image
    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)data1d, in_sz, datatype);

    if(data1d) {delete []data1d; data1d=0;}

    return true;
}





void processImage(V3DPluginCallback2 &callback, QWidget *parent)
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

    unsigned char* data1d = p4DImage->getRawData();
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //define datatype here
    //

    //input
    //update the pixel value

    ChangePxlValDialog d(p4DImage, parent);

    if (d.exec()!=QDialog::Accepted)
        return;
    else
    {
        //get the data
        V3DLONG nx = 0, ny = 0, nz = 0;
        int* ncval = new int [sc];
        if(!ncval)
            return;
        else
        {
            for(V3DLONG c=0; c<sc; c++)
            {
                ncval[c] = d.nval[c]->text().toInt();
                printf("ncval[%ld] %d \n", c, ncval[c]);
            }
        }

        nx = d.coord_x->text().toLong()-1;
        ny = d.coord_y->text().toLong()-1;
        nz = d.coord_z->text().toLong()-1;

        //change the pixel value
        for(V3DLONG c=0; c<sc; c++)
        {
            V3DLONG offsetc = c*pagesz;
            V3DLONG indLoop = offsetc + nz*M*N + ny*N + nx;
            data1d[indLoop] = ncval[c];

            printf("ncval[%ld] %d \n", c, ncval[c]);
            printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[indLoop], nx, ny, nz, indLoop);
        }
    }


    //de-alloc
    //		if (coord_x) {delete []coord_x; coord_x=0;}
    //		if (coord_y) {delete []coord_y; coord_y=0;}
    //		if (coord_z) {delete []coord_z; coord_z=0;}
    //
    //		if (ok) {delete []ok; ok=0;}
    //		if (cancel) {delete []cancel; cancel=0;}


}


void ChangePxlValDialog::update()
{
    //get current data
    if(!data1d) printf("No data passing");

    nx = coord_x->text().toLong()-1;
    ny = coord_y->text().toLong()-1;
    nz = coord_z->text().toLong()-1;

    for(V3DLONG c=0; c<sc; c++)
    {
        V3DLONG offsetc = c*pagesz;
        V3DLONG ind = offsetc + nz*M*N + ny*N + nx;
        nval[c]->setValue(data1d[ind]);

        //printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
    }

}


