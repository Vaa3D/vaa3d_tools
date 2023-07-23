/* imPreProcess_plugin.cpp
 * This is a plugin for image enhancement.
 * 2020-12-10 : by S.GUO
 */
 
#include <vector>
#include <iostream>
#include <fstream>
#include "v3d_message.h"
#include "imPreProcess_plugin.h"

using namespace std;
// Q_EXPORT_PLUGIN2(imPreProcess, TestPlugin);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("sigma correction...")
        <<tr("subtract minimum")
        <<tr("bilateral filter...")
        <<tr("fft filter")
        <<tr("grey morphology operator...")
        <<tr("image enhancement")
        <<tr("quality check...")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("im_sigma_correction")
        <<tr("subtract_minimum")
        <<tr("im_bilateral_filter")
        <<tr("im_fft_filter")
        <<tr("im_grey_morph")
        <<tr("im_enhancement")
        <<tr("im_quality_check")
        <<tr("im_test_enhancement")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("sigma correction..."))
    {
        sigma_correction_domenu(callback, parent);
    }
    else if (menu_name == tr("subtract minimum"))
    {
        subtract_minimum_domenu(callback, parent);
    }
    else if (menu_name == tr("bilateral filter..."))
	{
        bilateral_filter_domenu(callback, parent);
	}
    else if (menu_name == tr("fft filter"))
    {
        fft_domenu(callback, parent);
    }
    else if (menu_name == tr("grey morphology operator..."))
    {
        grey_morphology_domenu(callback, parent);
    }
    else if (menu_name == tr("image enhancement"))
	{
        enhancement_domenu(callback, parent);
	}
    else if (menu_name == tr("quality check..."))
    {
        quality_test_domenu(callback, parent);
    }
	else
	{
        v3d_msg(tr("This is a plugin for image enhancement. "
			"Developed by S.GUO, 2020-12-10"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("im_sigma_correction"))
    {
        return(sigma_correction_dofunc(input, output, callback));
    }
    else if (func_name == tr("im_subtract_minimum"))
    {
        return(subtract_minimum_dofunc(input, output, callback));
    }
    else if (func_name == tr("im_bilateral_filter"))
	{
        return(bilateral_filter_dofunc(input, output, callback));
	}
    else if (func_name == tr("im_fft_filter"))
    {
        return(fft_dofunc(input, output, callback));
    }
    else if (func_name == tr("im_grey_morph"))
    {
        return(grey_morphology_dofunc(input, output, callback));
    }
    else if (func_name == tr("im_enhancement"))
	{
        return(enhancement_dofunc(input, output, callback));
	}
    else if (func_name == tr("im_quality_check"))
    {
        return(quality_test_dofunc(input, output, callback));
    }
    else if (func_name == tr("im_test_enhancement"))
    {
        return(test_enhancement_dofunc(input, output, callback));
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool subtract_minimum_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    ImagePixelType pixeltype = p4DImage->getDatatype();


    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    void* outimg = 0; //new unsigned char[tolSZ];
    switch (pixeltype) {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                outimg =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in subtract min.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }

            subtract_min(data1d, mysz, (unsigned char* &)outimg);
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                outimg =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in subtract min.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            subtract_min(data1d, mysz, (short int* &)outimg);
        }
        break;

    default: ;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char*) outimg, N, M, P, 1, pixeltype);
    v3dhandle newwin;
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
        newwin = callback.currentImageWindow();
    else
        newwin = callback.newImageWindow();
    QString title = QObject::tr("Subtract Minimum Plugin");
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);

    outimg = 0;
    return 1;
}

bool subtract_minimum_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
         printf("Need an image.\n");
         return false;
    }

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    void* dst = 0;
    switch (pixeltype)
    {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                dst =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in subtract min.");
                if (dst) {delete []dst; dst=0;}
                return false;
            }
            subtract_min(data1d, mysz, (unsigned char* &)dst);
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                dst =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in subtract min.");
                if (dst) {delete []dst; dst=0;}
                return false;
            }
            subtract_min(data1d, mysz, (short int* &)dst);
        }
        break;

    default: ;
    }

    Image4DSimple outimg;
    outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

    callback.saveImage(&outimg, output_image);
outimg.cleanExistData();
    if(p4DImage) {delete p4DImage; p4DImage=0;}

    dst = 0;
    return 1;
}


bool bilateral_filter_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    //add input dialog
    BilateralFilterDialog dialog(callback, parent);
    if (!dialog.image)
        return 0;

    if (dialog.exec()!=QDialog::Accepted)
        return 0;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return 0;
    ROIList pRoiList = dialog.pRoiList;

    int c = dialog.ch;
    int k_sz[3] = {dialog.Wx, dialog.Wy, dialog.Wz};

    double colorSigma = dialog.colorSigma;

    double spaceSigmaXY = k_sz[0]/3.0;
    double spaceSigmaZ = k_sz[2]/3.0;

    cout<<"Wx = "<<k_sz[0]<<endl;
    cout<<"Wy = "<<k_sz[1]<<endl;
    cout<<"Wz = "<<k_sz[2]<<endl;
    cout<<"colorSigma = "<<colorSigma<<endl;
    cout<<"ch = "<<c<<endl;

    // gaussian_filter
    void* outimg = 0; //new unsigned char[tolSZ];
    switch (pixeltype) {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                outimg =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in bilateral.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            bilateralfilter(data1d, (unsigned char *&)outimg, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                outimg =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in bilateral.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            bilateralfilter(data1d, (short int *&)outimg, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 2);
        }
        break;

    default: ;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);

    v3dhandle newwin;
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
        newwin = callback.currentImageWindow();
    else
        newwin = callback.newImageWindow();

    QString title = QObject::tr("Bilateral Filter Plugin");
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);

    outimg = 0;
    return 1;
}

bool bilateral_filter_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
         printf("Need an image.\n");
         return false;
    }

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);

    double spaceSigmaXY, spaceSigmaZ, colorSigma;

    int k_sz[3] = {(inparas.size() >= 1) ? atoi(inparas[0]) : 3, (inparas.size() >= 1) ? atoi(inparas[0]) : 3, (inparas.size() >= 2) ? atoi(inparas[1]) : 1};

    spaceSigmaXY = k_sz[0]/3.0;
    spaceSigmaZ = k_sz[2]/3.0;

    colorSigma = (inparas.size() >= 3) ? atoi(inparas[2]) : 35;

    if (k_sz[0]>7) k_sz[0]=5;
    if (k_sz[1]>7) k_sz[1]=5;
    if (k_sz[2]>5) k_sz[2]=3;

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    void* dst = 0;
    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = 0;
                data1d = p4DImage->getRawData();
                try
                {
                    dst =new unsigned char[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in bilateral.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                bilateralfilter(data1d, (unsigned char* &)dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);

            }
        break;

        case V3D_UINT16:
            {
                short int * data1d_16 = 0;
                data1d_16 = (short int *)p4DImage->getRawData();
                try
                {
                    dst =new short int[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in bilateral.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                bilateralfilter(data1d_16, (short int* &)dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 2);
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }

    Image4DSimple outimg;
    outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

    callback.saveImage(&outimg, output_image);
outimg.cleanExistData();
    if(p4DImage)
    {
        delete p4DImage;
        p4DImage=0;
    }
    dst = 0;
    return 1;
}

bool fft_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    ImagePixelType pixeltype = p4DImage->getDatatype();

    // gaussian_filter
    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    void* outimg = 0; //new unsigned char[tolSZ];
    switch (pixeltype) {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                outimg =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in fft.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            fft_filter(data1d, mysz, (unsigned char* &) outimg, 1);
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                outimg =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in fft.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            fft_filter(data1d, mysz, (short int* &) outimg, 2);
        }
        break;

    default: ;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char*) outimg, N, M, P, 1, pixeltype);
    v3dhandle newwin;
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
        newwin = callback.currentImageWindow();
    else
        newwin = callback.newImageWindow();
    QString title = QObject::tr("FFT Plugin");
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);

    outimg = 0;
    return 1;
}

bool fft_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
        printf("Need an image.\n");
        return false;
    }

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    void* dst = 0;
    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = 0;
                data1d = p4DImage->getRawData();
                try
                {
                    dst =new unsigned char[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in fft.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                fft_filter(data1d, mysz, (unsigned char* &)dst, pixeltype);
            }
            break;

        case V3D_UINT16:
            {
                short int * data1d_16 = 0;
                data1d_16 = (short int *)p4DImage->getRawData();
                try
                {
                    dst =new short int[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in fft.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                fft_filter(data1d_16, mysz, (short int* &)dst, pixeltype);
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }

    Image4DSimple outimg;
    outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

    callback.saveImage(&outimg, output_image);
outimg.cleanExistData();
    if(p4DImage)
    {
        delete p4DImage;
        p4DImage=0;
    }

    dst = 0;
    return 1;
}

bool grey_morphology_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    ImagePixelType pixeltype = p4DImage->getDatatype();

    //add input dialog
    MorphologyDialog dialog(callback, parent);
    if (!dialog.image)
        return 0;

    if (dialog.exec()!=QDialog::Accepted)
        return 0;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return 0;
    ROIList pRoiList = dialog.pRoiList;

    int c = dialog.ch;
    int Wx = dialog.Wx, Wy=dialog.Wy;
    int Opt = dialog.opt;

    cout<<"Wx = "<<Wx<<endl;
    cout<<"Wy = "<<Wy<<endl;
    cout<<"ch = "<<c<<endl;
    cout<<"Opt = "<<Opt<<endl;

    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    void* outimg = 0; //new unsigned char[tolSZ];
    switch (pixeltype) {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                outimg =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in morphology operation.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            switch(Opt)
            {
                case 0:
                    cout<<"do Tophat"<<endl;
                    grey_tophat(data1d, mysz, Wx, Wy, c, (unsigned char* &)outimg);
                    break;
                case 1:
                    cout<<"do Erosion"<<endl;
                    grey_erosion(data1d, mysz, Wx, Wy, c, (unsigned char* &)outimg);
                    break;
                case 2:
                    cout<<"do Dilation"<<endl;
                    grey_dilation(data1d, mysz, Wx, Wy, c, (unsigned char* &)outimg);
                    break;
                default:
                    break;
            }
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                outimg =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in morphology operation.");
                if (outimg) {delete []outimg; outimg=0;}
                return false;
            }
            switch(Opt)
            {
                case 0:
                    cout<<"do Tophat"<<endl;
                    grey_tophat(data1d, mysz, Wx, Wy, c, (short int* &)outimg);
                    break;
                case 1:
                    cout<<"do Erosion"<<endl;
                    grey_erosion(data1d, mysz, Wx, Wy, c, (short int* &)outimg);
                    break;
                case 2:
                    cout<<"do Dilation"<<endl;
                    grey_dilation(data1d, mysz, Wx, Wy, c, (short int* &)outimg);
                    break;
                default:
                    break;
            }
        }
        break;

    default: ;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char*)outimg, N, M, P, 1, pixeltype);
    v3dhandle newwin;
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
        newwin = callback.currentImageWindow();
    else
        newwin = callback.newImageWindow();

    QString title = QObject::tr("Grey Morphology Plugin");
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);

    outimg = 0;
    return 1;
}


bool grey_morphology_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);
    unsigned int Wxy=11, Opt=0;

    if(inparas.size() >= 1) Wxy = atoi(inparas.at(0));
    if(inparas.size() >= 2) Opt = atoi(inparas.at(1));

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    void* dst = 0; //new unsigned char[tolSZ];
    switch (pixeltype) {
        case V3D_UINT8:
        {
            unsigned char* data1d = 0;
            data1d = p4DImage->getRawData();
            try
            {
                dst =new unsigned char[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in morphology operation.");
                if (dst) {delete []dst; dst=0;}
                return false;
            }
            switch(Opt)
            {
                case 0:
                    cout<<"do Tophat"<<endl;
                    grey_tophat(data1d, mysz, Wxy, Wxy, 1, (unsigned char* &)dst);
                    break;
                case 1:
                    cout<<"do Erosion"<<endl;
                    grey_erosion(data1d, mysz, Wxy, Wxy, 1, (unsigned char* &)dst);
                    break;
                case 2:
                    cout<<"do Dilation"<<endl;
                    grey_dilation(data1d, mysz, Wxy, Wxy, 1, (unsigned char* &)dst);
                    break;
                default:
                    break;
            }
        }
        break;

    case V3D_UINT16:
        {
            short int* data1d = 0;
            data1d = (short int *) p4DImage->getRawData();
            try
            {
                dst =new short int[tolSZ];
            }
                catch (...)
            {
                v3d_msg("Fail to allocate memory in morphology operation.");
                if (dst) {delete []dst; dst=0;}
                return false;
            }
            switch(Opt)
            {
                case 0:
                    cout<<"do Tophat"<<endl;
                    grey_tophat(data1d, mysz, Wxy, Wxy, 1, (short int* &)dst);
                    break;
                case 1:
                    cout<<"do Erosion"<<endl;
                    grey_erosion(data1d, mysz, Wxy, Wxy, 1, (short int* &)dst);
                    break;
                case 2:
                    cout<<"do Dilation"<<endl;
                    grey_dilation(data1d, mysz, Wxy, Wxy, 1, (short int* &)dst);
                    break;
                default:
                    break;
            }
        }
        break;

    default: ;
    }


    Image4DSimple outimg;
    outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

    callback.saveImage(&outimg, output_image);
outimg.cleanExistData();
    if(p4DImage) {delete p4DImage; p4DImage=0;}

    dst = 0;
    return 1;
}

bool sigma_correction_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }


    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    ImagePixelType pixeltype = p4DImage->getDatatype();


    //add input dialog

    SigmaCorrectionDialog dialog(callback, parent);
    if (!dialog.image)
        return 0;

    if (dialog.exec()!=QDialog::Accepted)
        return 0;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return 0;
    ROIList pRoiList = dialog.pRoiList;

    int c = dialog.ch;
    double cutoff = dialog.cutoff, gain=dialog.gain;

    cout<<"gain = "<<gain<<endl;
    cout<<"cutoff = "<<cutoff<<endl;
    cout<<"ch = "<<c<<endl;

    // gaussian_filter
    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    void* outimg = 0;
    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = p4DImage->getRawData();
                try
                {
                    outimg =new unsigned char[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in sigma correction.");
                    if (outimg) {delete []outimg; outimg=0;}
                    return false;
                }
                sigma_correction(data1d, mysz, cutoff, gain, (unsigned char* &)outimg, 1);
            }
            break;

        case V3D_UINT16:
            {
                short int * data1d_16 = (short int *)p4DImage->getRawData();
                try
                {
                    outimg =new short int[tolSZ];
                }
                    catch (...)
                {
                    v3d_msg("Fail to allocate memory in sigma correction.");
                    if (outimg) {delete []outimg; outimg=0;}
                    return false;
                }
                sigma_correction(data1d_16, mysz, cutoff, gain, (short int* &) outimg, 2);
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, N, M, P, 1, pixeltype);

    v3dhandle newwin;
    if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
        newwin = callback.currentImageWindow();
    else
        newwin = callback.newImageWindow();

    QString title = QObject::tr("Sigma Correction Plugin");
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);
    outimg = 0;
    return 1;
}

bool sigma_correction_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);
    double gain=5, cutoff=25;

    if(inparas.size() >= 1) gain = atoi(inparas.at(0));
    if(inparas.size() >= 2) cutoff = atoi(inparas.at(1));

    cout<<"cutoff = "<<cutoff<<endl;

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    void* dst = 0;
    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = p4DImage->getRawData();
                try
                {
                    dst = new unsigned char [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in sigma correction.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                sigma_correction(data1d, mysz, cutoff, gain, (unsigned char* &)dst, 1);
            }
            break;

        case V3D_UINT16:
            {
                dst = (void *)(new short int [N*M*P*sc]);
                try
                {
                    dst = new short int [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in sigma correction.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                short int * data1d_16 = (short int *)p4DImage->getRawData();
                sigma_correction(data1d_16, mysz, cutoff, gain, (short int* &) dst, 2);
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }

    Image4DSimple outimg;
    outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

    callback.saveImage(&outimg, output_image);
outimg.cleanExistData();
    if(p4DImage) {delete p4DImage; p4DImage=0;}
    dst = 0;
    return 1;
}

bool enhancement_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return 0;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return 0;
    }

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    ImagePixelType pixeltype = p4DImage->getDatatype();

    //add input dialog
    PreProcessDialog dialog(callback, parent);
    if (!dialog.image)
        return 0;

    if (dialog.exec()!=QDialog::Accepted)
        return 0;

    dialog.update();

    Image4DSimple* subject = dialog.image;
    if (!subject)
        return 0;
    ROIList pRoiList = dialog.pRoiList;

    int c = dialog.ch;
    int Wxy_bf=dialog.Wxy_bf, Wz_bf=dialog.Wz_bf;
    double colorSigma = dialog.colorSigma;
    double cutoff = dialog.cutoff, gain=dialog.gain;
    bool do_fft = dialog.b_do_fft, do_bf = dialog.b_do_bilateral;
    double spaceSigmaXY=Wxy_bf/3.0, spaceSigmaZ=Wz_bf/3.0;

    cout<<"do fft = "<<do_fft<<endl;
    cout<<"do bilateral = "<<do_bf<<endl;

    cout<<"wxy bf = "<<Wxy_bf<<endl;
    cout<<"wz bf = "<<Wz_bf<<endl;
    cout<<"sigmaxy bf = "<<spaceSigmaXY<<endl;
    cout<<"sigmaz bf = "<<spaceSigmaZ<<endl;
    cout<<"sigma color bf = "<<colorSigma<<endl;

    cout<<"sigma correction gain = "<<gain<<endl;
    cout<<"sigma correction cutoff = "<<cutoff<<endl;

    int *k_sz = new int[3];
    k_sz[0] = Wxy_bf; k_sz[1] = Wxy_bf; k_sz[2] = Wz_bf;

    V3DLONG mysz[4];
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = M*N*P*sc;

    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = p4DImage->getRawData();
                unsigned char* dst = 0;
                try
                {
                    dst = new unsigned char [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in image enhancement.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }
                cout<<"do sigma correction "<<endl;
                sigma_correction(data1d, mysz, cutoff, gain, dst, 1);
                cout<<"finish sigma correction "<<endl;
                for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];

                cout<<"do subtract min "<<endl;
                subtract_min(data1d, mysz, dst);
                cout<<"finish subtract min "<<endl;

                if(do_bf)
                {
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"do bilateral filter "<<endl;
                    bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);
                    cout<<"finish bilateral filter "<<endl;
                }

                if(do_fft)
                {
                    cout<<"do fft "<<endl;
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    fft_filter(data1d, mysz, dst, 1);
                    cout<<"finish fft "<<endl;
                }

                for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];

                cout<<"do intensity rescale "<<endl;
                intensity_rescale(data1d, mysz, dst, 1);
                cout<<"finish intensity rescale "<<endl;

                // display
                Image4DSimple * new4DImage = new Image4DSimple();
                new4DImage->setData(dst, N, M, P, 1, pixeltype);
                v3dhandle newwin;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
                    newwin = callback.currentImageWindow();
                else
                    newwin = callback.newImageWindow();
                QString title = QObject::tr("Enhancement Plugin");
                callback.setImage(newwin, new4DImage);
                callback.setImageName(newwin, title);
                callback.updateImageWindow(newwin);
                dst = 0;
                data1d = 0;
            }
        break;
        case V3D_UINT16:
            {
                short int* data1d = (short int *)p4DImage->getRawData();
                short int*  dst = 0;
                try
                {
                    dst = new short int [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in image enhancement.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }

                cout<<"do sigma correction "<<endl;
                sigma_correction(data1d, mysz, cutoff, gain, dst, 2);
                cout<<"finish sigma correction "<<endl;
                for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];

                cout<<"do subtract min"<<endl;
                subtract_min(data1d, mysz, dst);
                cout<<"finish subtract min"<<endl;

                if(do_bf)
                {
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"do bilateral filter "<<endl;
                    bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 2);
                    cout<<"finish bilateral filter "<<endl;
                }

                if(do_fft)
                {
                    cout<<"do fft "<<endl;
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    fft_filter(data1d, mysz, dst, 1);
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"finish fft "<<endl;
                }

                cout<<"do intensity rescale"<<endl;
                intensity_rescale(data1d, mysz, dst, 2);
                cout<<"finish intensity rescale"<<endl;

                // display
                Image4DSimple * new4DImage = new Image4DSimple();
                new4DImage->setData((unsigned char*)dst, N, M, P, 1, pixeltype);
                v3dhandle newwin;
                if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
                    newwin = callback.currentImageWindow();
                else
                    newwin = callback.newImageWindow();
                QString title = QObject::tr("Enhancement Plugin");
                callback.setImage(newwin, new4DImage);
                callback.setImageName(newwin, title);
                callback.updateImageWindow(newwin);
                dst = 0;
                data1d = 0;
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }
    return 1;
}

bool enhancement_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);

    double gain=5, cutoff=25;
    double spaceSigmaXY, spaceSigmaZ, colorSigma=35;
    int do_bf=1, do_fft=1;

    int k_sz[3] = {3, 3, 1};

    if(inparas.size() >= 1)
    {
        k_sz[0] = atoi(inparas.at(0));
        k_sz[1] = atoi(inparas.at(0));
    }

    if(inparas.size() >= 2) k_sz[2] = atoi(inparas.at(1));
    if(inparas.size() >= 3) colorSigma = atoi(inparas[2]);

    if(inparas.size() >= 4) gain = atoi(inparas.at(3));
    if(inparas.size() >= 5) cutoff = atoi(inparas.at(4));

    if(inparas.size() >= 6) do_bf = atoi(inparas.at(5));
    if(inparas.size() >= 7) do_fft = atoi(inparas.at(6));

    spaceSigmaXY = k_sz[0]/3.0;
    spaceSigmaZ = k_sz[2]/3.0;

    cout<<"wxy bf = "<<k_sz[0]<<endl;
    cout<<"wz bf = "<<k_sz[2]<<endl;
    cout<<"sigmaxy bf = "<<spaceSigmaXY<<endl;
    cout<<"sigmaz bf = "<<spaceSigmaZ<<endl;
    cout<<"sigma color bf = "<<colorSigma<<endl;
    cout<<"do fft = "<<do_fft<<endl;
    cout<<"do bilateral = "<<do_bf<<endl;
    cout<<"sigma correction gain = "<<gain<<endl;
    cout<<"sigma correction cutoff = "<<cutoff<<endl;

    Image4DSimple *p4DImage = callback.loadImage(input_image);
    if(!p4DImage || !p4DImage->valid())
    {
         v3d_msg("Fail to load the input image.");
         if (p4DImage) {delete p4DImage; p4DImage=0;}
         return false;
    }
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    mysz[0] = N; mysz[1] = M; mysz[2] = P; mysz[3] = sc;
    V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];

    ImagePixelType pixeltype = p4DImage->getDatatype();
    cout<<"pixeltype = "<<pixeltype<<endl;

    switch (pixeltype)
    {
        case V3D_UINT8:
            {
                unsigned char* data1d = p4DImage->getRawData();
                unsigned char*  dst = 0;
                try
                {
                    dst = new unsigned char [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in image enhancement.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }

                cout<<"do sigma correction "<<endl;
                sigma_correction(data1d, mysz, cutoff, gain, dst, 1);
                cout<<"finish sigma correction "<<endl;
                for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];

                cout<<"do subtract min"<<endl;
                subtract_min(data1d, mysz, dst);
                cout<<"finish subtract min"<<endl;

                if(do_bf)
                {
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"do bilateral filter "<<endl;
                    bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 1);
                    cout<<"finish bilateral filter "<<endl;
                }

                if(do_fft)
                {
                    cout<<"do fft "<<endl;
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    fft_filter(data1d, mysz, dst, 1);
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"finish fft "<<endl;
                }
                cout<<"do intensity rescale"<<endl;
                intensity_rescale(data1d, mysz, dst, 1);
                cout<<"finish intensity rescale"<<endl;

                Image4DSimple outimg;
                outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

                callback.saveImage(&outimg, output_image);
                outimg.cleanExistData();
                dst = 0;
                data1d = 0;
            }
        break;

        case V3D_UINT16:
            {
                short int* data1d = (short int *)p4DImage->getRawData();
                short int* dst = 0;
                try
                {
                    dst = new short int [tolSZ];
                }
                catch (...)
                {
                    v3d_msg("Fail to allocate memory in image enhancement.");
                    if (dst) {delete []dst; dst=0;}
                    return false;
                }

                cout<<"do sigma correction "<<endl;
                sigma_correction(data1d, mysz, cutoff, gain, dst, 2);
                cout<<"finish sigma correction "<<endl;
                for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];

                cout<<"do subtract min"<<endl;
                subtract_min(data1d, mysz, dst);
                cout<<"finish subtract min"<<endl;

                if(do_bf)
                {
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"do bilateral filter "<<endl;
                    bilateralfilter(data1d, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma, 2);
                    cout<<"finish bilateral filter "<<endl;
                }

                if(do_fft)
                {
                    cout<<"do fft "<<endl;
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    fft_filter(data1d, mysz, dst, 1);
                    for(V3DLONG i=0; i<tolSZ; i++) data1d[i]=dst[i];
                    cout<<"finish fft "<<endl;
                }

                cout<<"do intensity rescale"<<endl;
                intensity_rescale(data1d, mysz, dst, 2);
                cout<<"finish intensity rescale"<<endl;

                Image4DSimple outimg;
                outimg.setData((unsigned char *)dst, N, M, P, sc, pixeltype);

                callback.saveImage(&outimg, output_image);
                outimg.cleanExistData();
                dst = 0;
                data1d = 0;
            }
        break;

        default:
            v3d_msg("Invalid data type. Do nothing.");
            return 0;
    }

    p4DImage->cleanExistData();
    if(p4DImage) {delete p4DImage; p4DImage=0;}
    return 1;
}

long quality_test_domenu(V3DPluginCallback2 &callback, QWidget *parent)
{
//    char* argvs[2] = {(char*)("features"), (char*)("cal_features")};
//    PyTextureFeature pytf(callback);

//    long result = pytf.Excute(argvs);

    return 1;
}

long quality_test_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
//    vector<char*> infiles, outfiles;
//    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
//    char * output_file = ((vector<char*> *)(output.at(0).p))->at(0);

//    V3DLONG mysz[4] = {0,0,0,0};
//    unsigned char* src = 0;
//    int datatype = 1;

//    simple_loadimage_wrapper(callback, input_image, src, mysz, datatype);

//    char* argvs[2] = {(char*)("features"), (char*)("cal_features")};
//    PyTextureFeature pytf(src, mysz);

//    long result = pytf.Excute(argvs);

//    string label = "NULL";
//    switch(result){
//    case -100:
//        label = "Bad";
//        break;
//    case 0:
//        label = "Median";
//        break;
//    case 100:
//        label = "Good";
//        break;
//    default:
//        break;
//    }

//    ofstream myFile;
//    myFile.open(output_file, fstream::in | fstream::out | fstream::app);
//    if (myFile)
//    {
//        cout << "csv file found, appending result..."<<endl;
//        myFile << input_image << "," << label << "\n";
//    }
//    else
//    {
//        cout << "Cannot open csv file, file does not exist. Creating a new file..."<<endl;
//        myFile.open(output_file,  fstream::in | fstream::out | fstream::trunc);
//        myFile << input_image << "," << label << "\n";
//    }
//    myFile.close();
    return 1;
}

long test_enhancement_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
//    vector<char*> infiles, inparas, outfiles;

//    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);

//    char * input_image = ((vector<char*> *)(input.at(0).p))->at(0);
//    char * output_image = ((vector<char*> *)(output.at(0).p))->at(0);

//    int Wxy_mp=11;
//    double gain=5, cutoff=25;
//    double spaceSigmaXY, spaceSigmaZ, colorSigma=35;
//    int do_th=1;

//    int k_sz[3] = {3, 3, 1};

//    if(inparas.size() >= 1)
//    {
//        k_sz[0] = atoi(inparas.at(0));
//        k_sz[1] = atoi(inparas.at(0));
//    }

//    if(inparas.size() >= 2) k_sz[2] = atoi(inparas.at(1));
//    if(inparas.size() >= 3) colorSigma = atoi(inparas[2]);

//    if(inparas.size() >= 4) gain = atoi(inparas.at(3));
//    if(inparas.size() >= 5) cutoff = atoi(inparas.at(4));

//    if(inparas.size() >= 6) do_th = atoi(inparas.at(5));
//    if(inparas.size() >= 7) Wxy_mp = atoi(inparas.at(6));

//    spaceSigmaXY = k_sz[0]/3.0;
//    spaceSigmaZ = k_sz[2]/3.0;

//    cout<<"wxy bf = "<<k_sz[0]<<endl;
//    cout<<"wz bf = "<<k_sz[2]<<endl;
//    cout<<"sigmaxy bf = "<<spaceSigmaXY<<endl;
//    cout<<"sigmaz bf = "<<spaceSigmaZ<<endl;
//    cout<<"sigma color bf = "<<colorSigma<<endl;
//    cout<<"do tophat = "<<do_th<<endl;
//    cout<<"wxy tophat = "<<Wxy_mp<<endl;
//    cout<<"sigma correction gain = "<<gain<<endl;
//    cout<<"sigma correction cutoff = "<<cutoff<<endl;

//    V3DLONG mysz[4] = {0,0,0,0};
//    unsigned char* src = 0;
//    unsigned char* temp = 0;
//    unsigned char* dst = 0;
//    int datatype = 1;

//    simple_loadimage_wrapper(callback, input_image, src, mysz, datatype);

//    char* argvs[2] = {(char*)("features"), (char*)("cal_features")};
//    PyTextureFeature pytf(src, mysz);

//    long result = pytf.Excute(argvs);
//    cout<<"do quality check: "<<result<<endl;
//    if(result>=0)
//    {
//        V3DLONG tolSZ = mysz[0]*mysz[1]*mysz[2]*mysz[3];
//        temp = new unsigned char[tolSZ];

//        cout<<"do sigma correction "<<endl;
//        sigma_correction(src, mysz, cutoff, gain, dst);
//        cout<<"finish sigma correction "<<endl;
//        for(V3DLONG i=0; i<tolSZ; i++) temp[i]=dst[i];
//        dst = 0;
//        subtract_min(temp, mysz, dst);

//        for(V3DLONG i=0; i<tolSZ; i++) temp[i]=dst[i];
//        dst = 0;
//        cout<<"do bilateral filter "<<endl;
//        bilateralfilter(temp, dst, mysz, k_sz, spaceSigmaXY, spaceSigmaZ, colorSigma);
//        cout<<"finish bilateral filter "<<endl;

//        cout<<"do fft "<<endl;
//        fft_filter(dst, mysz);
//        for(V3DLONG i=0; i<tolSZ; i++) temp[i]=dst[i];
//        dst = 0;
//        cout<<"finish fft "<<endl;
//        intensity_rescale(temp, mysz, dst);
//        simple_saveimage_wrapper(callback, output_image, dst, mysz, datatype);
//    }

    return 1;
}
