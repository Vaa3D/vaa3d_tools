/* ireg.cpp
 * created by Yang Yu, Dec 20, 2011
 */


//
#ifndef __IREG_CPP__
#define __IREG_CPP__

#include <QtGui>

#include <cmath>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#include "ireg.h"
#include "y_imgreg.h"
#include "y_NiftiImageIO.h"

#include "stackutil.h"
#include "mg_utilities.h"
#include "mg_image_lib.h"
#include "basic_landmark.h"
#include "basic_surf_objs.h"

// Open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    QStringList fileList = dir.entryList(imgSuffix, QDir::Files, QDir::Name);

    foreach (fileList.size(), i)
    {
        QString file = fileList[i];

        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    //foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}



//plugin interface
const QString title = "ImageRegister";
Q_EXPORT_PLUGIN2(ireg, IRegPlugin);

// GUI interface
QStringList IRegPlugin::menulist() const
{
    return QStringList() << tr("ImageRegister")
                         << tr("NiftiImageImporter")
                         << tr("NiftiImageExporter")
                         << tr("About");
}

void IRegPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("ImageRegister"))
    {
        //
    }
    else if (menu_name == tr("NiftiImageImporter"))
    {
        // test
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image"),
                                                          QDir::currentPath(),
                                                          QObject::tr("Image (*)"));

        if(m_FileName.isEmpty())
        {
            cout<<"File "<<m_FileName.toStdString().c_str()<<" does not exist"<<endl;
            return;
        }

        //
        NiftiImageIO imageIO;

        if(!imageIO.canReadFile(const_cast<char *>(m_FileName.toStdString().c_str())))
        {
            cout<<"Fail to recognize image as Nifity file format"<<endl;
            return;
        }

        void *pData = 0;
        if(!imageIO.load(pData))
        {
            cout<<"Fail to load image as Nifity file format"<<endl;
            return;
        }

        V3DLONG sx, sy, sz, sc;
        int datatype;

        sx = imageIO.getDimx();
        sy = imageIO.getDimy();
        sz = imageIO.getDimz();
        sc = imageIO.getDimc();

        datatype = imageIO.getDataType();

        //
        Image4DSimple p4DImage;

        if(datatype == UCHAR || datatype == CHAR)
        {
            p4DImage.setData((unsigned char*)pData, sx, sy, sz, sc, V3D_UINT8);
        }
        else if(datatype == USHORT || datatype == SHORT)
        {
            p4DImage.setData((unsigned char*)pData, sx, sy, sz, sc, V3D_UINT16);
        }
        else if(datatype == FLOAT)
        {
            p4DImage.setData((unsigned char*)pData, sx, sy, sz, sc, V3D_FLOAT32);
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return;
        }

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, m_FileName.toStdString().c_str());
        callback.updateImageWindow(newwin);

    }
    else if (menu_name == tr("NiftiImageExporter"))
    {
        //
        QString filename = QFileDialog::getSaveFileName( parent,
                                                         tr("Save Image"),
                                                         QDir::currentPath(),
                                                         QObject::tr("Image (*.nii;*.img;*.hdr)") );

        if( filename.isNull() )
        {
            cout<<"Please specify a name"<<endl;
            return;
        }

        //
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            cout<<"You don't have any image open in the main window."<<endl;
            return;
        }

        //
        Image4DSimple* subject = callback.getImage(curwin);

        V3DLONG sx = subject->getXDim();
        V3DLONG sy = subject->getYDim();
        V3DLONG sz = subject->getZDim();
        V3DLONG sc = subject->getCDim();

        int datatype = subject->getDatatype();

        //
        NiftiImageIO imageIO;

        if(!imageIO.canWriteFile(const_cast<char *>(filename.toStdString().c_str())))
        {
            cout<<"Fail to write an image"<<endl;
            return;
        }
        imageIO.write((void *) (subject->getRawData()), sx, sy, sz, sc, 1, datatype);

    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Image Registration Plugin %1 (Dec. 20, 2011) developed by Yang Yu. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

// command line interface
void errorPrint_ireg()
{
    printf("\nUsage: v3d -x ireg -f ireg -i <input_image> -o <output_image> -p \"#cs <subject_refchannel> #ct <target_refchannel>\"\n");
}

void errorPrint_globalalign()
{
    printf("\nUsage: v3d -x ireg -f globalalign -o <output_name> -p \"#s <subject_image> #cs <subject_refchannel> #t <target_image> #ct <target_refchannel>\"\n");
}

void errorPrint_rigidreg()
{
    printf("\nUsage: v3d -x ireg -f rigidreg -o <output_name> -p \"#s <subject_image> #cs <subject_refchannel> #t <target_image> #ct <target_refchannel>\"\n");
}

void errorPrint_evalAlignQuality()
{
    printf("\nUsage: v3d -x ireg -f evalAlignQuality -o <output_name> -p \"#s <subject_image> #cs <subject_refchannel> #t <target_image> #ct <target_refchannel>\"\n");
}

void errorPrint_localalign()
{
    printf("\nUsage: v3d -x ireg -f localalign -o <output_name> -p \"#s <subject_image> #cs <subject_refchannel> #t <target_image> #ct <target_refchannel>\"\n");
}

void errorPrint_iwarp()
{
    printf("\nUsage: v3d -x ireg -f iwarp -i <input_image> -o <output_image> -p \"#s <subject_image> #t <target_image> #a <affinematrix.txt> #d <displacementfields>\"\n");
}

void errorPrint_NiftiImageConverter()
{
    printf("\nUsage: v3d -x ireg -f NiftiImageConverter -i <input_image> -o <output_image> -p \"#b <0/1> #v <bit_width> #r <rescale>\"\n");
    printf("\nExample: ./v3d -x ireg -f NiftiImageConverter -i XX.tif -o XX.nii");
    printf("\n         ./v3d -x ireg -f NiftiImageConverter -i XX_c1.nii XX_c2.nii XX_c3.nii -o XX.v3draw -p \"#b 1\"\n\n");
}

// plugin func
QStringList IRegPlugin::funclist() const
{
    return QStringList() << tr("ireg")
                         << tr("globalalign")
                         << tr("localalign")
                         << tr("iwarp")
                         << tr("iwarp2")
                         << tr("NiftiImageConverter")
                         << tr("splitColorChannels")
                         << tr("mergeColorChannels")
                         << tr("prepare40xData")
                         << tr("prepare20xData")
                         << tr("resizeImage")
                         << tr("MultiLabelImageConverter")
                         << tr("convertSigned16bit2Unsigned16bit")
                         << tr("icut")
                         << tr("xflip")
                         << tr("yflip")
                         << tr("zflip")
                         << tr("zmip")
                         << tr("isampler")
                         << tr("iContrastEnhancer")
                         << tr("bwthinning")
                         << tr("QiScoreStas")
                         << tr("iSymmetryDet")
                         << tr("iRecenter")
                         << tr("iTiling")
                         << tr("lmreg")
                         << tr("ierode")
                         << tr("inormalize")
                         << tr("createMaskImage")
                         << tr("resamplebyspacing")
                         << tr("gaussianfilter")
                         << tr("cropImage")
                         << tr("convertTC2AM")
                         << tr("zeropadding")
                         << tr("rigidreg")
                         << tr("rotEstimate")
                         << tr("evalAlignQuality")
                         << tr("meandeviation")
                         << tr("extractRotMat")
                         << tr("recoverRef")
                         << tr("rescaleInt")
                         << tr("esimilarity")
                         << tr("convertDatatype")
                         << tr("ppleftopticlobe")
                         << tr("computeRef")
                         << tr("celltracker")
                         << tr("mipSequence")
                         << tr("cellprofiler")
                         << tr("marker2volume")
                         << tr("valueReplace")
                         << tr("absOrient")
                         << tr("extractVOI")
                         << tr("genVOIs");
}

bool IRegPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("ireg"))
    {
        // input  : Source image, Template image
        // output : affine transform, displacement field, warped result

        // global alignment

        // local alignment

        //
        return true;
    }
    else if (func_name == tr("globalalign"))
    {
        // input  : Source image, Template image
        // output : affine transform, displacement field, warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint_globalalign();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_affinematrix, qs_filename_flipmatrix;
        QString qs_filename_warped; // displacement field
        QString qs_filename_deformed;

        bool b_save_affine = true;
        bool b_save_warped = true;
        bool b_save_dpfield = false;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint_globalalign();
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint_globalalign();
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_affinematrix=qs_pathname_output+"/"+qs_basename_output+"Affine.txt";
                qs_filename_flipmatrix=qs_pathname_output+"/"+qs_basename_output+"Flip.txt";
                qs_filename_warped=qs_pathname_output+"/"+qs_basename_output+"Warped.v3draw";
                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+"Deformed.v3draw";
            }
            else
            {
                qs_filename_affinematrix=qs_pathname_input+"/"+qs_basename_input+"Affine.txt";
                qs_filename_flipmatrix=qs_pathname_input+"/"+qs_basename_input+"Flip.txt";
                qs_filename_warped=qs_pathname_input+"/"+qs_basename_input+"Warped.v3draw";
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"Deformed.v3draw";
            }

            //
            printf("\ninputs:\n");
            printf("  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
            printf("  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
            printf("  input ref channel of sub:     %ld\n",channel_ref_sub+1);
            printf("  input ref channel of tar:     %ld\n",channel_ref_tar+1);
            printf("\noutputs:\n");
            printf("  output affine matrix:         %s\n",qPrintable(qs_filename_affinematrix));
            printf("  output flip matrix:           %s\n",qPrintable(qs_filename_flipmatrix));
            printf("  output displacement field:    %s\n",qPrintable(qs_filename_warped));
            printf("  output deformed image:        %s\n",qPrintable(qs_filename_deformed));

        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read the input(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            errorPrint_globalalign();

            return false;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Align. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG offsets_sub = channel_ref_sub*sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_tar = channel_ref_tar*sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        V3DLONG *sz_sub=NULL, *sz_tar=NULL;
        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_tar, 4);

        for(V3DLONG i=0; i<3; i++)
        {
            sz_sub[i] = sz_img_sub_input[i];
            sz_tar[i] = sz_img_tar_input[i];
        }
        sz_sub[3] = 1;
        sz_tar[3] = 1;

        // affine matrix
        V3DLONG *sz_affine=NULL;
        y_new<V3DLONG, V3DLONG>(sz_affine, 4);

        sz_affine[0] = 4; sz_affine[1] = 4; sz_affine[2] = 1; sz_affine[3] = 1;

        Y_IMG_REAL pOut;
        pOut.initImage(sz_affine, 4);

        Y_IMG_REAL pOutFlip;
        pOutFlip.initImage(sz_affine, 4);

        // deformed output
        Y_IMG_REAL pSDeformedD;
        Y_IMG_UINT8 pSDeformedD8bit;

        //
        Scale<REAL, V3DLONG> scale;
        scale.setSampleRatio3D(0.125, 0.125, 0.125);
        scale.setIterations(300); //

        vector< Scale<REAL, V3DLONG> > multiscales;
        multiscales.push_back(scale);

        RegistrationType regtype  = AFFINE;
        OptimizationType opttype  = GradientDescent;
        SimilarityType smltype  = SSD;
        InterpolationType inttype  = TriLinear;
        TransformDirection transdir = FORWARD;

        // global align
        if(datatype_tar_input==1)
        {

            Y_IMG_UINT8 pTar;
            pTar.setImage((unsigned char*)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char*)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt);
                }

            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT16, Y_IMG_UINT8, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    Y_IMG_REAL pSDeformedDTmp;
                    pSDeformedDTmp.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedDTmp.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedDTmp, pSub, pOut, globalalign.opt);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pSDeformedDTmp.pImg[i]; //

                    // flip
                    Y_MAT<REAL, V3DLONG> F(pOutFlip.pImg, pOutFlip.dims[0], pOutFlip.dims[1]); // 3D A4x4

                    if(F.v[0][0]==-1)
                    {
                        flip<REAL, V3DLONG>(pSDeformedD.pImg, pSDeformedDTmp.pImg, pSDeformedDTmp.dims, AX); // x
                        foreach(pTar.totalplxs, i)
                            pSDeformedDTmp.pImg[i] = pSDeformedD.pImg[i]; //
                    }

                    if(F.v[1][1]==-1)
                    {
                        flip<REAL, V3DLONG>(pSDeformedD.pImg, pSDeformedDTmp.pImg, pSDeformedDTmp.dims, AY); // y
                        foreach(pTar.totalplxs, i)
                            pSDeformedDTmp.pImg[i] = pSDeformedD.pImg[i]; //
                    }

                    if(F.v[2][2]==-1)
                    {
                        flip<REAL, V3DLONG>(pSDeformedD.pImg, pSDeformedDTmp.pImg, pSDeformedDTmp.dims, AZ); // z
                    }

                    pSDeformedDTmp.clean();
                }
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_UINT8, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_REAL, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt); // pOut is affine matrix
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==2)
        {
            Y_IMG_UINT16 pTar;
            pTar.setImage((unsigned short *)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char *)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_UINT16, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt);
                }
            }
            else if(datatype_sub_input==2)
            {

                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt);
                }
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_UINT16, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_REAL, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt); // pOut is affine matrix
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==4)
        {

            Y_IMG_REAL pTar;
            pTar.setImage((REAL*)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char *)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt);
                }

            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                // warp
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt);
                }

            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_REAL, Y_IMG_REAL, Y_IMG_REAL> globalalign;

                globalalign.opt.regtype  = regtype;
                globalalign.opt.opttype  = opttype;
                globalalign.opt.smltype  = smltype;
                globalalign.opt.inttype  = inttype;
                globalalign.opt.transdir = transdir;
                globalalign.pyr.setScales(multiscales);

                // global alignment
                globalalign.globalregistration(pOutFlip, pOut, pSub, pTar);

                //
                if(b_save_warped)
                {
                    pSDeformedD.initImage(sz_tar, 4);

                    foreach(pTar.totalplxs, i)
                        pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                    warp<V3DLONG, Y_IMG_REAL, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, globalalign.opt); // pOut is affine matrix
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        Y_MAT<REAL, V3DLONG> A(pOut.pImg, pOut.dims[0], pOut.dims[1]);
        qDebug()<<"affine matrix ...";
        for(V3DLONG i=0; i<A.column; i++)
        {
            for(V3DLONG j=0; j<A.row; j++)
            {
                cout<<" "<<A.v[i][j];
            }
            cout<<endl;
        }
        cout<<endl;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // save the warped image
        if(b_save_warped)
        {
            if(qs_filename_deformed!=NULL)
            {
                // save 8-bit deformed reference
                pSDeformedD8bit.initImage(pSDeformedD.dims, 4);
                normalizing<REAL, V3DLONG>(pSDeformedD.pImg, pSDeformedD.totalplxs, 0, 255);
                foreach(pSDeformedD.totalplxs, i)
                    pSDeformedD8bit.pImg[i] = pSDeformedD.pImg[i];

                if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformedD8bit.pImg),pSDeformedD8bit.dims,1))
                {
                    printf("ERROR: saveImage failed!\n");
                    return false;
                }
            }
        }

        // save affine matrix
        if(b_save_affine)
        {
            // affine
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.setTransform(pOut.pImg, 3);

            if(qs_filename_affinematrix!=NULL)
            {
                if(!afftrans.write(qPrintable(qs_filename_affinematrix)))
                {
                    printf("ERROR: saving affine matrix failed!\n");
                    return false;
                }
            }
            afftrans.clean();

            // flip
            Y_AFFINETRANSFORM<REAL, V3DLONG> fliptrans;
            fliptrans.setTransform(pOutFlip.pImg, 3);

            if(qs_filename_flipmatrix!=NULL)
            {
                if(!fliptrans.write(qPrintable(qs_filename_flipmatrix)))
                {
                    printf("ERROR: saving flip matrix failed!\n");
                    return false;
                }
            }
            fliptrans.clean();
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        y_del<V3DLONG>(sz_affine);
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_tar);

        pSDeformedD.clean();
        pSDeformedD8bit.clean();
        pOut.clean();
        pOutFlip.clean();

        //
        return true;
    }
    else if (func_name == tr("iwarp"))
    {
        // input  : Source image, Template image, affine matrix, displacement field
        // output : warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint_iwarp();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;
        QString qs_filename_affinematrix, qs_filename_flipmatrix;
        QString qs_filename_warped; // displacement field

        // outputs
        QString qs_filename_deformed;

        // interpolation type
        int iptype = 2; // trilinear

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "a"))
                            {
                                qs_filename_affinematrix = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "f"))
                            {
                                qs_filename_flipmatrix = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "d"))
                            {
                                qs_filename_warped = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                iptype = atoi( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint_iwarp();
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
            else
            {
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"_deformed.v3draw";
            }

            //
            printf("\ninputs:\n");
            printf("  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
            printf("  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
            printf("  input affine matrix:          %s\n",qPrintable(qs_filename_affinematrix));
            printf("  input flip matrix:            %s\n",qPrintable(qs_filename_flipmatrix));
            printf("  input displacement field:     %s\n",qPrintable(qs_filename_warped));
            printf("\noutputs:\n");
            printf("  output sub2tar image:         %s\n",qPrintable(qs_filename_deformed));

        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read the input(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            errorPrint_globalalign();

            return false;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Warp. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        V3DLONG *sz_sub=NULL, *sz_tar=NULL;
        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_tar, 4);

        for(V3DLONG i=0; i<3; i++)
        {
            sz_sub[i] = sz_img_sub_input[i];
            sz_tar[i] = sz_img_tar_input[i];
        }
        sz_sub[3] = 1;
        sz_tar[3] = 1;

        // affine matrix
        V3DLONG *sz_affine=NULL;
        y_new<V3DLONG, V3DLONG>(sz_affine, 4);

        sz_affine[0] = 4; sz_affine[1] = 4; sz_affine[2] = 1; sz_affine[3] = 1;

        Y_IMG_REAL pOut;
        pOut.initImage(sz_affine, 4);

        Y_IMG_REAL pOutFlip;
        pOutFlip.initImage(sz_affine, 4);

        // deformed output
        Y_IMG_UINT8 pSDeformed1D;
        Y_IMG_UINT8 pSDeformed;
        Y_IMG_UINT16 pSDeformed1Du16;
        Y_IMG_UINT16 pSDeformedu16;

        //
        RegistrationOption opt;

        // load affine matrix
        if(qs_filename_affinematrix!=NULL && QFile(qs_filename_affinematrix).exists())
        {
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.initTransform(3); // 3D

            if(!afftrans.read(qPrintable(qs_filename_affinematrix)))
            {
                printf("ERROR: loading affine matrix failed!\n");
                return false;
            }
            foreach(afftrans.sz, i) pOut.pImg[i] = afftrans.pTransform[i];

            //
            Y_MAT<REAL, V3DLONG> A(pOut.pImg, pOut.dims[0], pOut.dims[1]);
            qDebug()<<"affine matrix ...";
            for(V3DLONG i=0; i<A.column; i++)
            {
                for(V3DLONG j=0; j<A.row; j++)
                {
                    cout<<" "<<A.v[i][j];
                }
                cout<<endl;
            }
            cout<<endl;

            bool b_flip = false;
            Y_MAT<REAL, V3DLONG> F;
            F.init(pOut.dims[0], pOut.dims[1], 1);
            if(qs_filename_flipmatrix!=NULL && QFile(qs_filename_flipmatrix).exists())
            {
                Y_AFFINETRANSFORM<REAL, V3DLONG> fliptrans;
                fliptrans.initTransform(3); // 3D

                if(!fliptrans.read(qPrintable(qs_filename_flipmatrix)))
                {
                    printf("ERROR: loading flip matrix failed!\n");
                    return false;
                }

                b_flip = true;

                //
                foreach(fliptrans.sz, i)
                    pOutFlip.pImg[i] = fliptrans.pTransform[i];

                //
                Y_MAT<REAL, V3DLONG> F1(pOutFlip.pImg, pOutFlip.dims[0], pOutFlip.dims[1]);
                F.assign(F1);
                F1.clean();
                qDebug()<<"flip matrix ...";
                for(V3DLONG i=0; i<F.column; i++)
                {
                    for(V3DLONG j=0; j<F.row; j++)
                    {
                        cout<<" "<<F.v[i][j];
                    }
                    cout<<endl;
                }
                cout<<endl;
            }

            //
            opt.regtype  = AFFINE;
            opt.opttype  = GradientDescent;
            opt.smltype  = SSD;
            opt.inttype  = InterpolationType(iptype); // NearestNeighbor=1 TriLinear=2
            opt.transdir = FORWARD;

            // warp
            if(datatype_sub_input==1)
            {
                pSDeformed1D.initImage(sz_tar, 4);
                sz_tar[3] = sz_img_sub_input[3];
                pSDeformed.initImage(sz_tar, 4);

                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    V3DLONG offset_sub = c*sz_sub[0]*sz_sub[1]*sz_sub[2];
                    V3DLONG offset_tar = c*sz_tar[0]*sz_tar[1]*sz_tar[2];

                    Y_IMG_UINT8 pSub;
                    pSub.setImage((unsigned char*)p_img_sub_input + offset_sub, sz_sub, 4);

                    // warp
                    warp<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, pOut, opt);

                    unsigned char *p = pSDeformed.pImg + offset_tar;

                    foreach(pSDeformed1D.totalplxs, i)
                        p[i] = pSDeformed1D.pImg[i]; //

                    // flip
                    if(b_flip)
                    {
                        // flip
                        if(F.v[0][0]==-1)
                        {
                            flip<unsigned char, V3DLONG>(p, pSDeformed1D.pImg, pSDeformed1D.dims, AX); // x
                            foreach(pSDeformed1D.totalplxs, i)
                                pSDeformed1D.pImg[i] = p[i]; //
                        }

                        if(F.v[1][1]==-1)
                        {
                            flip<unsigned char, V3DLONG>(p, pSDeformed1D.pImg, pSDeformed1D.dims, AY); // y
                            foreach(pSDeformed1D.totalplxs, i)
                                pSDeformed1D.pImg[i] = p[i]; //
                        }

                        if(F.v[2][2]==-1)
                        {
                            flip<unsigned char, V3DLONG>(p, pSDeformed1D.pImg, pSDeformed1D.dims, AZ); // z
                        }
                    }
                }

            }
            else if(datatype_sub_input==2)
            {
                pSDeformed1Du16.initImage(sz_tar, 4);
                sz_tar[3] = sz_img_sub_input[3];
                pSDeformedu16.initImage(sz_tar, 4);

                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    V3DLONG offset_sub = c*sz_sub[0]*sz_sub[1]*sz_sub[2];
                    V3DLONG offset_tar = c*sz_tar[0]*sz_tar[1]*sz_tar[2];

                    Y_IMG_UINT16 pSub;
                    pSub.setImage((unsigned short*)p_img_sub_input + offset_sub, sz_sub, 4);

                    // warp
                    warp<V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16, Y_IMG_REAL>(pSDeformed1Du16, pSub, pOut, opt);

                    unsigned short *p = pSDeformedu16.pImg + offset_tar;

                    foreach(pSDeformed1Du16.totalplxs, i)
                        p[i] = pSDeformed1Du16.pImg[i]; //

                    // flip
                    if(b_flip)
                    {
                        // flip
                        if(F.v[0][0]==-1)
                        {
                            flip<unsigned short, V3DLONG>(p, pSDeformed1Du16.pImg, pSDeformed1Du16.dims, AX); // x
                            foreach(pSDeformed1Du16.totalplxs, i)
                                pSDeformed1Du16.pImg[i] = p[i]; //
                        }

                        if(F.v[1][1]==-1)
                        {
                            flip<unsigned short, V3DLONG>(p, pSDeformed1Du16.pImg, pSDeformed1Du16.dims, AY); // y
                            foreach(pSDeformed1Du16.totalplxs, i)
                                pSDeformed1Du16.pImg[i] = p[i]; //
                        }

                        if(F.v[2][2]==-1)
                        {
                            flip<unsigned short, V3DLONG>(p, pSDeformed1Du16.pImg, pSDeformed1Du16.dims, AZ); // z
                        }
                    }
                }
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_img_sub_input, sz_sub, 4);

                // warp
                // warp<V3DLONG, Y_IMG_REAL, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, pOut, opt);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        y_del<V3DLONG>(sz_affine);

        pOut.clean();
        pOutFlip.clean();

        // warp image based on the input displacement field
        if(qs_filename_warped!=NULL && QFile(qs_filename_warped).exists())
        {
            // load df
            QFileInfo curfile_info(qs_filename_warped);
            if(curfile_info.isFile())
            {
                if (! ((curfile_info.suffix().toUpper()=="IMG") || (curfile_info.suffix().toUpper()=="HDR") || (curfile_info.suffix().toUpper()=="NII") || (curfile_info.suffix().toUpper()=="GZ")) )
                {
                    cout << "Error: your file are not recognized as Nifti images." << endl;
                    return false;
                }
            }
            else
            {
                cout << "Please input Nifti images"<<endl;
                return false;
            }

            //
            V3DLONG dfsx, dfsy, dfsz, dfsc;
            int dfdatatype;
            void *pDF = 0;

            if(QFile::exists(qs_filename_warped))
            {
                NiftiImageIO imageIO;

                if(!imageIO.canReadFile(const_cast<char *>(qs_filename_warped.toStdString().c_str())))
                {
                    cout<<"Fail to recognize image as Nifity file format"<<endl;
                    return false;
                }

                if(!imageIO.load(pDF))
                {
                    cout<<"Fail to load image as Nifity file format"<<endl;
                    return false;
                }

                dfsx = imageIO.getDimx();
                dfsy = imageIO.getDimy();
                dfsz = imageIO.getDimz();
                dfsc = imageIO.getDimt(); // Vector dimt = 3

                dfdatatype = imageIO.getDataType();
            }
            else
            {
                cout << "The image <"<<qs_filename_warped.toStdString()<<"> does not exist."<<endl;
                return false;
            }

            //
            Y_IMG_REAL df;

            V3DLONG *szdf=NULL;
            y_new<V3DLONG, V3DLONG>(szdf, 4);

            szdf[0] = dfsc; // =3
            szdf[1] = dfsx;
            szdf[2] = dfsy;
            szdf[3] = dfsz;

            df.initField(szdf, 4); // init

            if(dfdatatype == UCHAR || dfdatatype == CHAR)
            {
                unsigned char *pInput = (unsigned char *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else if(dfdatatype == USHORT || dfdatatype == SHORT)
            {
                unsigned short *pInput = (unsigned short *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else if(dfdatatype == FLOAT)
            {
                float *pInput = (float *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }

            // de-alloc
            y_del<void>(pDF);

            // warping
            sz_sub[0] = sz_tar[0];
            sz_sub[1] = sz_tar[1];
            sz_sub[2] = sz_tar[2];
            sz_sub[3] = 1;

            // output
            if(datatype_sub_input==1)
            {
                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    V3DLONG offsets = c*sz_tar[0]*sz_tar[1]*sz_tar[2];

                    unsigned char *p = pSDeformed.pImg + offsets;

                    Y_IMG_UINT8 pSub;
                    pSub.setImage(p, sz_sub, 4);

                    //opt.inttype  = ThinPlateSpline;

                    interpolation<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, df, opt);

                    foreach(pSDeformed1D.totalplxs, i)
                        p[i] = pSDeformed1D.pImg[i]; //
                }
            }
            else if(datatype_sub_input==2)
            {

            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {

            }


        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // save the warped image
        if(qs_filename_deformed!=NULL)
        {
            if(datatype_sub_input==1)
            {
                if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformed.pImg),pSDeformed.dims,pSDeformed.dt))
                {
                    printf("ERROR: saveImage failed!\n");
                    return false;
                }
                pSDeformed.clean();
                pSDeformed1D.clean();
            }
            else if(datatype_sub_input==2)
            {
                if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformedu16.pImg),pSDeformedu16.dims,pSDeformedu16.dt))
                {
                    printf("ERROR: saveImage failed!\n");
                    return false;
                }
                pSDeformedu16.clean();
                pSDeformed1Du16.clean();
            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {

            }

        }

        //de-alloc
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_tar);


        //
        return true;
    }
    else if (func_name == tr("iwarp2"))
    {
        // input  : Source image, Template image, affine matrix, displacement field
        // output : warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint_iwarp();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // DIMENSIONS
        V3DLONG tx=0.0,ty=0.0,tz=0.0;

        // sampling ratio
        REAL srx=1.0,sry=1.0,srz=1.0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;
        QString qs_filename_affinematrix;
        QString qs_filename_warped; // displacement field

        // outputs
        QString qs_filename_deformed;

        // interpolation type
        int iptype = 2; // trilinear

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "a"))
                            {
                                qs_filename_affinematrix = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "d"))
                            {
                                qs_filename_warped = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                iptype = atoi( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sx"))
                            {
                                key++;
                                srx = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sy"))
                            {
                                key++;
                                sry = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "sz"))
                            {
                                key++;
                                srz = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "dx"))
                            {
                                key++;
                                tx = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "dy"))
                            {
                                key++;
                                ty = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "dz"))
                            {
                                key++;
                                tz = atol( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input subject image!\n");
                errorPrint_iwarp();
                return false;
            }

            if(qs_filename_img_tar==NULL)
            {
                if(tx<=1.0 || ty<=1.0 || tz<=1.0)
                {
                    printf("\nERROR: invalid input target image or dimensions!\n");
                    errorPrint_iwarp();
                    return false;
                }
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
            else
            {
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"_deformed.v3draw";
            }

            //
            printf("\ninputs:\n");
            printf("  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
            printf("  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
            printf("  input affine matrix:          %s\n",qPrintable(qs_filename_affinematrix));
            printf("  input displacement field:     %s\n",qPrintable(qs_filename_warped));
            printf("\noutputs:\n");
            printf("  output sub2tar image:         %s\n",qPrintable(qs_filename_deformed));

        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read the input(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;
        bool b_targetfile=false;
        if(QFile::exists(qs_filename_img_tar))
        {
            if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
            {
                printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
                y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
                return false;
            }
            printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
            printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
            printf("\t\tdatatype: %d\n",datatype_tar_input);

            b_targetfile=true;
        }
        else
        {

        }

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);


        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Warp. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        V3DLONG *sz_sub=NULL, *sz_tar=NULL;
        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_tar, 4);

        for(V3DLONG i=0; i<3; i++)
        {
            sz_sub[i] = sz_img_sub_input[i];

            if(b_targetfile)
                sz_tar[i] = sz_img_tar_input[i];
        }
        sz_sub[3] = 1;

        if(!b_targetfile)
        {
            sz_tar[0] = tx;
            sz_tar[1] = ty;
            sz_tar[2] = tz;
        }
        sz_tar[3] = 1;

        // affine matrix
        V3DLONG *sz_affine=NULL;
        y_new<V3DLONG, V3DLONG>(sz_affine, 4);

        sz_affine[0] = 4; sz_affine[1] = 4; sz_affine[2] = 1; sz_affine[3] = 1;

        Y_IMG_REAL pOut;
        pOut.initImage(sz_affine, 4);

        // deformed output
        Y_IMG_UINT8 pSDeformed1D;
        Y_IMG_UINT8 pSDeformed;
        Y_IMG_UINT16 pSDeformed1Du16;
        Y_IMG_UINT16 pSDeformedu16;

        //
        RegistrationOption opt;

        // load affine matrix
        if(qs_filename_affinematrix!=NULL && QFile(qs_filename_affinematrix).exists())
        {
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.initTransform(3); // 3D

            if(!afftrans.read(qPrintable(qs_filename_affinematrix)))
            {
                printf("ERROR: loading affine matrix failed!\n");
                return false;
            }
            foreach(afftrans.sz, i) pOut.pImg[i] = afftrans.pTransform[i];

            //
            Y_MAT<REAL, V3DLONG> A(pOut.pImg, pOut.dims[0], pOut.dims[1]);

            qDebug()<<"affine matrix ...";
            for(V3DLONG i=0; i<A.column; i++)
            {
                for(V3DLONG j=0; j<A.row; j++)
                {
                    cout<<" "<<A.v[i][j];
                }
                cout<<endl;
            }
            cout<<endl;

            //
            opt.regtype  = AFFINE;
            opt.opttype  = GradientDescent;
            opt.smltype  = SSD;
            opt.inttype  = InterpolationType(iptype); // NearestNeighbor=1 TriLinear=2
            opt.transdir = FORWARD;

            // warp
            if(datatype_sub_input==1)
            {
                pSDeformed1D.initImage(sz_tar, 4);
                sz_tar[3] = sz_img_sub_input[3];
                pSDeformed.initImage(sz_tar, 4);

                V3DLONG pagesztar=sz_tar[0]*sz_tar[1]*sz_tar[2];
                V3DLONG pageszsub=sz_sub[0]*sz_sub[1]*sz_sub[2];

                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    unsigned char *p_sub = p_img_sub_input + c*pageszsub;
                    unsigned char *p_tar = pSDeformed.pImg + c*pagesztar;

                    Y_IMG_UINT8 pSub;
                    pSub.setImage(p_sub, sz_sub, 4);

                    // warp
                    warp2<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, pOut, srx, sry, srz, opt);

                    foreach(pSDeformed1D.totalplxs, i)
                        p_tar[i] = pSDeformed1D.pImg[i]; //
                }

            }
            else if(datatype_sub_input==2)
            {
                pSDeformed1Du16.initImage(sz_tar, 4);
                sz_tar[3] = sz_img_sub_input[3];
                pSDeformedu16.initImage(sz_tar, 4);

                V3DLONG pagesztar=sz_tar[0]*sz_tar[1]*sz_tar[2];
                V3DLONG pageszsub=sz_sub[0]*sz_sub[1]*sz_sub[2];

                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    unsigned short *p_sub = (unsigned short*)p_img_sub_input + c*pageszsub;
                    unsigned short *p_tar = pSDeformedu16.pImg + c*pagesztar;

                    Y_IMG_UINT16 pSub;
                    pSub.setImage(p_sub, sz_sub, 4);

                    // warp
                    warp2<V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16, Y_IMG_REAL>(pSDeformed1Du16, pSub, pOut, srx, sry, srz, opt);

                    foreach(pSDeformed1Du16.totalplxs, i)
                        p_tar[i] = pSDeformed1Du16.pImg[i]; //
                }
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                //pSub.setImage((REAL*)p_img_sub_input, sz_sub, 4);

                // warp
                // warp<V3DLONG, Y_IMG_REAL, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, pOut, opt);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }

        //de-alloc
        if(b_targetfile)
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        y_del<V3DLONG>(sz_affine);

        pOut.clean();

        // warp image based on the input displacement field
        if(qs_filename_warped!=NULL && QFile(qs_filename_warped).exists())
        {
            // load df
            QFileInfo curfile_info(qs_filename_warped);
            if(curfile_info.isFile())
            {
                if (! ((curfile_info.suffix().toUpper()=="IMG") || (curfile_info.suffix().toUpper()=="HDR") || (curfile_info.suffix().toUpper()=="NII") || (curfile_info.suffix().toUpper()=="GZ")) )
                {
                    cout << "Error: your file are not recognized as Nifti images." << endl;
                    return false;
                }
            }
            else
            {
                cout << "Please input Nifti images"<<endl;
                return false;
            }

            //
            V3DLONG dfsx, dfsy, dfsz, dfsc;
            int dfdatatype;
            void *pDF = 0;

            if(QFile::exists(qs_filename_warped))
            {
                NiftiImageIO imageIO;

                if(!imageIO.canReadFile(const_cast<char *>(qs_filename_warped.toStdString().c_str())))
                {
                    cout<<"Fail to recognize image as Nifity file format"<<endl;
                    return false;
                }

                if(!imageIO.load(pDF))
                {
                    cout<<"Fail to load image as Nifity file format"<<endl;
                    return false;
                }

                dfsx = imageIO.getDimx();
                dfsy = imageIO.getDimy();
                dfsz = imageIO.getDimz();
                dfsc = imageIO.getDimt(); // Vector dimt = 3

                dfdatatype = imageIO.getDataType();
            }
            else
            {
                cout << "The image <"<<qs_filename_warped.toStdString()<<"> does not exist."<<endl;
                return false;
            }

            //
            Y_IMG_REAL df;

            V3DLONG *szdf=NULL;
            y_new<V3DLONG, V3DLONG>(szdf, 4);

            szdf[0] = dfsc; // =3
            szdf[1] = dfsx;
            szdf[2] = dfsy;
            szdf[3] = dfsz;

            df.initField(szdf, 4); // init

            if(dfdatatype == UCHAR || dfdatatype == CHAR)
            {
                unsigned char *pInput = (unsigned char *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else if(dfdatatype == USHORT || dfdatatype == SHORT)
            {
                unsigned short *pInput = (unsigned short *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else if(dfdatatype == FLOAT)
            {
                float *pInput = (float *)pDF;

                V3DLONG pagesz = dfsx*dfsy*dfsz;

                for(V3DLONG k=0; k<dfsz; k++)
                {
                    V3DLONG offset_k = k*dfsx*dfsy;
                    V3DLONG offset_k_df = k*dfsx*dfsy*dfsc;
                    for(V3DLONG j=0; j<dfsy; j++)
                    {
                        V3DLONG offset_j = offset_k + j*dfsx;
                        V3DLONG offset_j_df = offset_k_df + j*dfsx*dfsc;
                        for(V3DLONG i=0; i<dfsx; i++)
                        {
                            for(V3DLONG c=0; c<dfsc; c++)
                            {
                                df.pImg[ offset_j_df + i + c ] = pInput[offset_j + i + c*pagesz];
                            }
                        }
                    }
                }
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }

            // de-alloc
            y_del<void>(pDF);

            // warping
            sz_sub[0] = sz_tar[0];
            sz_sub[1] = sz_tar[1];
            sz_sub[2] = sz_tar[2];
            sz_sub[3] = 1;

            // output
            if(datatype_sub_input==1)
            {
                for(V3DLONG c=0; c<sz_tar[3]; c++)
                {
                    V3DLONG offsets = c*sz_tar[0]*sz_tar[1]*sz_tar[2];

                    unsigned char *p = pSDeformed.pImg + offsets;

                    Y_IMG_UINT8 pSub;
                    pSub.setImage(p, sz_sub, 4);

                    //opt.inttype  = ThinPlateSpline;

                    interpolation<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL>(pSDeformed1D, pSub, df, opt);

                    foreach(pSDeformed1D.totalplxs, i)
                        p[i] = pSDeformed1D.pImg[i]; //
                }
            }
            else if(datatype_sub_input==2)
            {

            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {

            }


        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // save the warped image
        if(qs_filename_deformed!=NULL)
        {
            if(datatype_sub_input==1)
            {
                if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformed.pImg),pSDeformed.dims,pSDeformed.dt))
                {
                    printf("ERROR: saveImage failed!\n");
                    return false;
                }
                pSDeformed.clean();
                pSDeformed1D.clean();
            }
            else if(datatype_sub_input==2)
            {
                if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformedu16.pImg),pSDeformedu16.dims,pSDeformedu16.dt))
                {
                    printf("ERROR: saveImage failed!\n");
                    return false;
                }
                pSDeformedu16.clean();
                pSDeformed1Du16.clean();
            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {

            }

        }

        //de-alloc
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_tar);


        //
        return true;
    }
    else if (func_name == tr("NiftiImageConverter"))
    {
        if(input.size()<1) // no inputs
        {
            //print Help info
            errorPrint_NiftiImageConverter();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_image_file

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) {outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); } }  // specify output
        if(input.size()>1) {paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        int b_invert = 0; // 1 backward convert 0 forward
        int szvxl = -1; // -1 do nothing; or 1 uint8 2 uint16 4 float

        QString qs_filename_img_input(infile);
        QString qs_filename_img_output;

        int b_rescale = 1;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "b"))
                            {
                                b_invert = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "v"))
                            {
                                szvxl = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "r"))
                            {
                                b_rescale = atoi( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }

                }
            }
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();

        if(outfile)
        {
            QString qs_pathname_output=QFileInfo(QString(outfile)).path();

            if(b_invert==0)
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+".nii";
            }
            else
            {
                QString qs_basename_output=QFileInfo(QString(outfile)).baseName();
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
        }
        else
        {
            QString qs_pathname_output=QFileInfo(qs_filename_img_input).path();

            if(b_invert==0)
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+".nii";
            }
            else
            {
                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_input+".v3draw";
            }
        }

        // error check
        if(qs_filename_img_input==NULL || qs_filename_img_output==NULL)
        {
            printf("\nERROR: invalid input file name!\n");
            errorPrint_NiftiImageConverter();
            return false;
        }

        qDebug()<<"input "<<qs_filename_img_input<<"output "<<qs_filename_img_output;

        //
        if(b_invert==0)
        {
            // convert .tif/.lsm/.v3draw to .img/.nii

            // load image
            unsigned char *inputImg1d=NULL;
            V3DLONG *sz_inputImg=NULL;
            int datatype_inputImg;

            V3DLONG sx, sy, sz, sc;

            if(QFile::exists(qs_filename_img_input))
            {
                if (loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), inputImg1d, sz_inputImg, datatype_inputImg)!=true)
                {
                    cout << "Fail to load the input image."<<endl;
                    return false;
                }

                sx= sz_inputImg[0], sy = sz_inputImg[1], sz = sz_inputImg[2]; sc = sz_inputImg[3];
            }
            else
            {
                cout << "The image <"<<qs_filename_img_input.toStdString()<<"> does not exist."<<endl;
                return false;
            }

            // write image(s)
            QString qs_filename_output = qs_filename_img_output;
            for(V3DLONG c=0; c<sc; c++)
            {
                V3DLONG offset_c = c*sx*sy*sz;

                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_img_output=qs_pathname_output+"/"+qs_basename_output+QString("_c%1.nii").arg(c);

                //
                NiftiImageIO imageIO;

                if(!imageIO.canWriteFile(const_cast<char *>(qs_filename_img_output.toStdString().c_str())))
                {
                    cout<<"Fail to write an image"<<endl;
                    return false;
                }

                if(datatype_inputImg==V3D_UINT8)
                {
                    imageIO.write((void *) (inputImg1d + offset_c), sx, sy, sz, 1, 1, datatype_inputImg);
                }
                else if(datatype_inputImg==V3D_UINT16)
                {
                    unsigned short *p = (unsigned short *)inputImg1d;
                    imageIO.write((void *) (p+offset_c), sx, sy, sz, 1, 1, datatype_inputImg);
                }
                else if(datatype_inputImg==V3D_FLOAT32)
                {
                    float *p = (float *)inputImg1d;
                    imageIO.write((void *) (p+offset_c), sx, sy, sz, 1, 1, datatype_inputImg);
                }
                else
                {
                    cout << "Image data type is not supported!"<<endl;
                    return false;
                }
            }
        }
        else
        {
            // convert .nii to .v3draw
            QFileInfo curfile_info(qs_filename_img_input);
            if(curfile_info.isFile())
            {
                if (! ((curfile_info.suffix().toUpper()=="IMG") || (curfile_info.suffix().toUpper()=="HDR") || (curfile_info.suffix().toUpper()=="NII")) )
                {
                    cout << "Error: your file are not recognized as Nifti images." << endl;
                    return false;
                }
            }
            else
            {
                cout << "Please input Nifti images"<<endl;
                return false;
            }

            //
            V3DLONG sx, sy, sz, sc;
            int datatype;
            void *pData = 0;
            void *pOutput = 0;
            V3DLONG pagesz;

            int szData=1;

            sc = infilelist->size();
            for(int inputcnt=0; inputcnt<sc; inputcnt++)
            {
                infile = infilelist->at(inputcnt);
                qs_filename_img_input = QString(infile);

                V3DLONG ssx, ssy, ssz, ssc;
                int sdatatype;

                if(QFile::exists(qs_filename_img_input))
                {
                    NiftiImageIO imageIO;

                    if(!imageIO.canReadFile(const_cast<char *>(qs_filename_img_input.toStdString().c_str())))
                    {
                        cout<<"Fail to recognize image as Nifity file format"<<endl;
                        return false;
                    }

                    if(!imageIO.load(pData))
                    {
                        cout<<"Fail to load image as Nifity file format"<<endl;
                        return false;
                    }

                    ssx = imageIO.getDimx();
                    ssy = imageIO.getDimy();
                    ssz = imageIO.getDimz();
                    ssc = imageIO.getDimc();

                    sdatatype = imageIO.getDataType();
                }
                else
                {
                    cout << "The image <"<<qs_filename_img_input.toStdString()<<"> does not exist."<<endl;
                    return false;
                }

                if(inputcnt==0)
                {
                    sx=ssx; sy=ssy; sz=ssz; datatype=sdatatype;
                    pagesz = sx*sy*sz;

                    if(szvxl!=-1 && (szvxl==1 || szvxl==2) )
                    {
                        szData = szvxl;
                    }
                    else
                    {
                        if(datatype == UCHAR || datatype == CHAR)
                        {
                            szData = sizeof(unsigned char);
                        }
                        else if(datatype == USHORT || datatype == SHORT)
                        {
                            szData = sizeof(unsigned short);
                        }
                        else if(datatype == FLOAT || datatype == DOUBLE)
                        {
                            szData = sizeof(float);
                        }
                        else
                        {
                            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                            return false;
                        }
                    }

                    try
                    {
                        pOutput = new unsigned char [pagesz*sc*szData];
                    }
                    catch(...)
                    {
                        cout<<"Fail to allocate memory"<<endl;
                        return false;
                    }

                }
                else
                {
                    if(ssx!=sx || ssy!=sy || ssz!=sz )
                    {
                        cout<<"Input images are not consistent"<<endl;
                        return false;
                    }
                }

                //
                if(sdatatype==DOUBLE && (szvxl==1 || szvxl==2) )
                {
                    if(szvxl==1)
                    {
                        double *pNifti = (double*)pData;

                        if(b_rescale==1)
                        {
                            rescaling<double, V3DLONG>(pNifti, pagesz, (double)0.0, (double)255.0);
                        }

                        unsigned char *p = (unsigned char*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned char)(((double*)pData)[i]);

                        datatype = UCHAR;
                    }
                    else if(szvxl==2)
                    {
                        double *pNifti = (double*)pData;
                        if(b_rescale==1)
                        {
                            rescaling<double, V3DLONG>(pNifti, pagesz, (double)0.0, (double)4095.0);
                        }

                        unsigned short *p = (unsigned short*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned short)(((double*)pData)[i]);

                        datatype = USHORT;
                    }
                }
                else if(sdatatype==FLOAT && (szvxl==1 || szvxl==2) )
                {
                    if(szvxl==1)
                    {
                        REAL *pNifti = (REAL*)pData;
                        if(b_rescale==1)
                        {
                            rescaling<REAL, V3DLONG>(pNifti, pagesz, (REAL)0.0, (REAL)255.0);
                        }

                        unsigned char *p = (unsigned char*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned char)(((REAL*)pData)[i]);

                        datatype = UCHAR;
                    }
                    else if(szvxl==2)
                    {
                        REAL *pNifti = (REAL*)pData;
                        if(b_rescale==1)
                        {
                            rescaling<REAL, V3DLONG>(pNifti, pagesz, (REAL)0.0, (REAL)4095.0);
                        }

                        unsigned short *p = (unsigned short*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned short)(((REAL*)pData)[i]);

                        datatype = USHORT;
                    }
                }
                else if((sdatatype == USHORT || sdatatype == SHORT) && szvxl==1 )
                {
                    if(sdatatype == USHORT)
                    {
                        unsigned short *pNifti = (unsigned short*)pData;
                        if(b_rescale==1)
                        {
                            rescaling<unsigned short, V3DLONG>(pNifti, pagesz, (unsigned short)0, (unsigned short)255);
                        }

                        unsigned char *p = (unsigned char*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned char)(((unsigned short*)pData)[i]);
                    }
                    else if(sdatatype == SHORT)
                    {
                        short *pNifti = (short*)pData;
                        if(b_rescale==1)
                        {
                            rescaling<short, V3DLONG>(pNifti, pagesz, (short)0, (short)255);
                        }

                        unsigned char *p = (unsigned char*)pOutput+pagesz*inputcnt;
                        foreach(pagesz, i)
                            p[i] = (unsigned char)(((short*)pData)[i]);
                    }
                    datatype = UCHAR;
                }
                else
                {
                    memcpy((unsigned char*)pOutput+pagesz*inputcnt*szData, (unsigned char*)pData, szData*pagesz);
                }

            }

            // save the output image
            V3DLONG szoutput[4];
            szoutput[0]=sx; szoutput[1]=sy; szoutput[2]=sz; szoutput[3]=sc;

            if(datatype == UCHAR || datatype == CHAR)
            {
                if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pOutput,szoutput,1))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else if(datatype == USHORT || datatype == SHORT)
            {
                if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pOutput,szoutput,2))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else if(datatype == FLOAT)
            {
                if(szvxl!=-1 && (szvxl==1 || szvxl==2) )
                {
                    if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pOutput,szoutput,szvxl))
                    {
                        printf("ERROR: saveImage() return false!\n");
                        return false;
                    }
                }
                else
                {
                    if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pOutput,szoutput,4))
                    {
                        printf("ERROR: saveImage() return false!\n");
                        return false;
                    }
                }

            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }

            // de-alloc
            y_del<void>(pOutput);
            y_del<void>(pData);

        }
        //
        return true;
    }
    else if (func_name == tr("splitColorChannels"))
    {
        // input  : image with multiple color channels
        // output : images with single color channel

        if(input.size()<1) // no inputs
        {
            cout<<"Fails to call function splitColorChannels"<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        // input
        QString qs_filename_input=NULL;

        // outputs
        QString qs_filename_output=NULL;

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);  if(!infilelist->empty()) { infile = infilelist->at(0); qs_filename_input = QString(infile);}}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); qs_filename_output = QString(outfile);}}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras = paralist->at(0); } // parameters

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        // split color channels
        unsigned char *inputImg1d=NULL;
        V3DLONG *sz_inputImg=NULL;
        int datatype_inputImg;

        V3DLONG sx, sy, sz, sc;

        // load the input image
        if(QFile::exists(qs_filename_input))
        {
            if (loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()), inputImg1d, sz_inputImg, datatype_inputImg)!=true)
            {
                cout << "Fail to load the input image."<<endl;
                return false;
            }

            sx= sz_inputImg[0]; sy = sz_inputImg[1]; sz = sz_inputImg[2]; sc = sz_inputImg[3];
        }
        else
        {
            cout << "The image <"<<qs_filename_input.toStdString()<<"> does not exist."<<endl;
            return false;
        }

        // write image(s)
        QString qs_filename_img_output = qs_filename_output;
        sz_inputImg[3] = 1;
        for(V3DLONG c=0; c<sc; c++)
        {
            V3DLONG offset_c = c*sx*sy*sz;

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_img_output=qs_pathname_output+"/"+qs_basename_output+QString("_c%1.v3draw").arg(c);

            //
            if(datatype_inputImg==V3D_UINT8)
            {
                if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)inputImg1d + offset_c,sz_inputImg,1))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else if(datatype_inputImg==V3D_UINT16)
            {
                if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)inputImg1d + offset_c*2,sz_inputImg,2))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else if(datatype_inputImg==V3D_FLOAT32)
            {
                if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)inputImg1d + offset_c*4,sz_inputImg,4))
                {
                    printf("ERROR: saveImage() return false!\n");
                    return false;
                }
            }
            else
            {
                cout << "Image data type is not supported!"<<endl;
                return false;
            }
        }

        // de-alloc
        y_del<unsigned char>(inputImg1d);
        y_del<V3DLONG>(sz_inputImg);

        return true;
    }
    else if (func_name == tr("mergeColorChannels"))
    {
        // input  : images with single color channel
        // output : image with multiple color channels

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<"Usage: v3d -x ireg -f mergeColorChannels -i <inputs> -o <output>"<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        // input
        QString qs_filename_input=NULL;

        // outputs
        QString qs_filename_output=NULL;

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); qs_filename_input = QString(infile);}}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); qs_filename_output = QString(outfile);}}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras = paralist->at(0);} // parameters

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            if(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") != 0 &&
                    QFileInfo(qs_filename_output).suffix().toUpper().compare("RAW") != 0)
            {
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();
                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }

        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        // merge color channels
        unsigned char *inputImg1d=NULL;
        V3DLONG *sz_inputImg=NULL;
        int datatype_inputImg;

        V3DLONG sx, sy, sz, sc;
        sc = infilelist->size();
        int datatype_output = 1; // UINT8 by default
        unsigned char * pOutput = NULL;

        V3DLONG ssx, ssy, ssz;

        // load the input images
        for(int c=0; c<sc; c++)
        {
            infile = infilelist->at(c);
            QString qs_filename_img_input = QString(infile);

            if(QFile::exists(qs_filename_img_input))
            {
                if (loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), inputImg1d, sz_inputImg, datatype_inputImg)!=true)
                {
                    cout << "Fail to load the input image."<<endl;
                    return false;
                }

                ssx= sz_inputImg[0]; ssy = sz_inputImg[1]; ssz = sz_inputImg[2];
            }
            else
            {
                cout << "The image <"<<qs_filename_img_input.toStdString()<<"> does not exist."<<endl;
                return false;
            }

            V3DLONG pagesz;
            if(c==0)
            {
                datatype_output = datatype_inputImg;
                sx = ssx;
                sy = ssy;
                sz = ssz;
                pagesz = sx*sy*sz*datatype_output;

                y_new<unsigned char, V3DLONG>(pOutput, sx*sy*sz*sc*datatype_output);

                foreach(pagesz, i)
                    pOutput[i] = inputImg1d[i];
            }
            else
            {
                if(sx!=ssx || sy!=ssy || sz!=ssz || datatype_output!=datatype_inputImg)
                {
                    cout << "Invalid Input Images!"<<endl;

                    y_del<unsigned char>(pOutput);
                    y_del<unsigned char>(inputImg1d);
                    y_del<V3DLONG>(sz_inputImg);

                    return false;
                }
                else
                {
                    foreach(pagesz, i)
                        pOutput[c*pagesz + i] = inputImg1d[i];
                }
            }

            // de-alloc
            y_del<unsigned char>(inputImg1d);
        }

        qDebug()<<"test ... "<<sc<<qPrintable(qs_filename_output);

        // write image(s)
        sz_inputImg[3] = sc;
        if(datatype_output==V3D_UINT8)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char*)pOutput,sz_inputImg,1))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }
        else if(datatype_output==V3D_UINT16)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char*)pOutput,sz_inputImg,2))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }
        else if(datatype_output==V3D_FLOAT32)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char*)pOutput,sz_inputImg,4))
            {
                printf("ERROR: saveImage() return false!\n");
                return false;
            }
        }
        else
        {
            cout << "Image data type is not supported!"<<endl;
            return false;
        }

        // de-alloc
        y_del<V3DLONG>(sz_inputImg);
        y_del<unsigned char>(pOutput);

        //
        return true;
    }
    else if (func_name == tr("zflip"))
    {
        // input  : 3D image stack (e.g. fruit fly brains)
        // output : flipped along z

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f zflip -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images
        V3DLONG *sz_img = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFileInfo(qs_filename_img_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), p1dImg, sz_img, datatype_img, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }

        //
        V3DLONG pagesz = sz_img[0]*sz_img[1]*sz_img[2];
        V3DLONG totalplxs = pagesz * sz_img[3];

        /// flip along z
        unsigned char *p_flip = NULL;
        y_new<unsigned char, V3DLONG>(p_flip, totalplxs*datatype_img);

        foreach(sz_img[3], c)
        {
            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*pagesz;
                unsigned char *poutput = p_flip + c*pagesz;

                flip<unsigned char, V3DLONG>(poutput, pinput, sz_img, AZ); // z
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*pagesz;
                unsigned short *poutput = (unsigned short *)p_flip + c*pagesz;

                flip<unsigned short, V3DLONG>(poutput, pinput, sz_img, AZ); // z
            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_flip),sz_img,datatype_img))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg);
        y_del<unsigned char>(p_flip);
        y_del<V3DLONG>(sz_img);

        return true;
    }
    else if (func_name == tr("xflip"))
    {
        // input  : 3D image stack (e.g. fruit fly brains)
        // output : flipped along x

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f xflip -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images
        V3DLONG *sz_img = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFileInfo(qs_filename_img_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), p1dImg, sz_img, datatype_img, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }

        //
        V3DLONG pagesz = sz_img[0]*sz_img[1]*sz_img[2];
        V3DLONG totalplxs = pagesz * sz_img[3];

        /// flip along y
        unsigned char *p_flip = NULL;
        y_new<unsigned char, V3DLONG>(p_flip, totalplxs*datatype_img);

        foreach(sz_img[3], c)
        {
            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*pagesz;
                unsigned char *poutput = p_flip + c*pagesz;

                flip<unsigned char, V3DLONG>(poutput, pinput, sz_img, AX); // x
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*pagesz;
                unsigned short *poutput = (unsigned short *)p_flip + c*pagesz;

                flip<unsigned short, V3DLONG>(poutput, pinput, sz_img, AX); // x
            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_flip),sz_img,datatype_img))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg);
        y_del<unsigned char>(p_flip);
        y_del<V3DLONG>(sz_img);

        return true;
    }
    else if (func_name == tr("yflip"))
    {
        // input  : 3D image stack (e.g. fruit fly brains)
        // output : flipped along y

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f yflip -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images
        V3DLONG *sz_img = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFileInfo(qs_filename_img_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), p1dImg, sz_img, datatype_img, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }

        //
        V3DLONG pagesz = sz_img[0]*sz_img[1]*sz_img[2];
        V3DLONG totalplxs = pagesz * sz_img[3];

        /// flip along y
        unsigned char *p_flip = NULL;
        y_new<unsigned char, V3DLONG>(p_flip, totalplxs*datatype_img);

        foreach(sz_img[3], c)
        {
            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*pagesz;
                unsigned char *poutput = p_flip + c*pagesz;

                flip<unsigned char, V3DLONG>(poutput, pinput, sz_img, AY); // y
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*pagesz;
                unsigned short *poutput = (unsigned short *)p_flip + c*pagesz;

                flip<unsigned short, V3DLONG>(poutput, pinput, sz_img, AY); // y
            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_flip),sz_img,datatype_img))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg);
        y_del<unsigned char>(p_flip);
        y_del<V3DLONG>(sz_img);

        return true;
    }
    else if (func_name == tr("zmip"))
    {
        // input  : 3D image stack (e.g. fruit fly brains)
        // output : Max Intensity Projection along z

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f zmip -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

//            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
//            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

//            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images
        V3DLONG *sz_img = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFileInfo(qs_filename_img_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), p1dImg, sz_img, datatype_img, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }

        //
        V3DLONG pagesz = sz_img[0]*sz_img[1]*sz_img[2];
        V3DLONG totalplxs = pagesz * sz_img[3];

        /// mip along z
        unsigned char *p_mip = NULL;
        y_new<unsigned char, V3DLONG>(p_mip, totalplxs/sz_img[2]);

        V3DLONG sz = sz_img[2];
        sz_img[2] = 1;

        foreach(sz_img[3], c)
        {
            V3DLONG offsetz = sz_img[0]*sz_img[1];

            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*pagesz;
                unsigned char *poutput = p_mip + c*offsetz;

                for(V3DLONG j=0; j<sz_img[1]; j++)
                {
                    V3DLONG offset_j = j*sz_img[0];
                    for(V3DLONG i=0; i<sz_img[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        unsigned char val = pinput[idx];

                        for(V3DLONG k=1; k<sz; k++)
                        {
                            unsigned char curval = pinput[ k*offsetz +  idx];

                            if(val < curval) val = curval;
                        }
                        poutput[idx] = val;
                    }
                }

                unsigned char *ptmp = NULL;
                y_new<unsigned char, V3DLONG>(ptmp, offsetz);

                pinput = poutput;
                poutput = ptmp;

                flip<unsigned char, V3DLONG>(poutput, pinput, sz_img, AY); // y

                foreach(offsetz, i) pinput[i] = poutput[i];

                y_del<unsigned char>(ptmp);
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*pagesz;
                unsigned char *poutput = p_mip + c*offsetz;

                double ratio = 255.0/4095.0;

                qDebug()<<ratio;

                for(V3DLONG j=0; j<sz_img[1]; j++)
                {
                    V3DLONG offset_j = j*sz_img[0];
                    for(V3DLONG i=0; i<sz_img[0]; i++)
                    {
                        V3DLONG idx = offset_j + i;
                        double val = (double) (pinput[idx]);

                        for(V3DLONG k=1; k<sz; k++)
                        {
                            unsigned short curval = pinput[ k*offsetz + idx ];

                            if(val < curval) val = curval;
                        }
                        poutput[idx] = (unsigned char)( val*ratio );
                    }
                }

                unsigned char *ptmp = NULL;
                y_new<unsigned char, V3DLONG>(ptmp, offsetz);

                unsigned char *p_input = poutput;
                poutput = ptmp;

                flip<unsigned char, V3DLONG>(poutput, p_input, sz_img, AY); // y

                foreach(offsetz, i) p_input[i] = poutput[i];

                y_del<unsigned char>(ptmp);

            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        qDebug()<<"save as ..."<<qs_filename_output;

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_mip),sz_img,1))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg);
        y_del<unsigned char>(p_mip);
        y_del<V3DLONG>(sz_img);

        return true;
    }
    else if (func_name == tr("prepare40xData"))
    {
        // input  : 40x central brain image (fruit fly)
        // output : converted to 8bit, downsampled twice in x and y, resized, and flipped along z

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f prepare40xData -o <output> -p \"#s <subject> #t <target>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_img_sub).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }

        /// load images
        // target
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_img_tar).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        if(sz_img1[3]<channel_ref_tar+1)
        {
            printf("\nERROR: invalid reference channel!\n");
            return false;
        }

        // subject
        V3DLONG *sz_img2 = 0;
        int datatype_img2 = 0;
        unsigned char* p1dImg2 = 0;

        if(QFileInfo(qs_filename_img_sub).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()),p1dImg2,sz_img2,datatype_img2))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }

        //
        /// convert sub to 8bit
        unsigned char *p_sub=NULL;
        V3DLONG sx = sz_img2[0];
        V3DLONG sy = sz_img2[1];
        V3DLONG sz = sz_img2[2];
        V3DLONG sc = sz_img2[3];
        V3DLONG pagesz = sx*sy*sz;
        V3DLONG totalplxs = pagesz*sc;

        if(datatype_img2==UINT8)
        {
            //
            p_sub = p1dImg2;
        }
        else if(datatype_img2==UINT16)
        {
            // convert to 8 bit
            unsigned short *p = (unsigned short *)p1dImg2;
            rescaling<unsigned short, V3DLONG>(p, totalplxs, 0, 255);

            y_new<unsigned char, V3DLONG>(p_sub, totalplxs);
            foreach(totalplxs, i)
                p_sub[i] = p[i];

        }
        else if(datatype_img2==FLOAT32)
        {
            // convert to 8 bit
            float *p = (float *)p1dImg2;
            rescaling<float, V3DLONG>(p, totalplxs, 0.0f, 255.0f);

            y_new<unsigned char, V3DLONG>(p_sub, totalplxs);
            foreach(totalplxs, i)
                p_sub[i] = p[i];
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        //
        /// resample to 20x
        unsigned char *p_subds = NULL;
        V3DLONG *szds=NULL;

        y_new<V3DLONG, V3DLONG>(szds, 4);
        szds[0] = 0.5*sz_img2[0];
        szds[1] = 0.5*sz_img2[1];
        szds[2] = sz_img2[2];
        szds[3] = sz_img2[3];

        sx = szds[0];
        sy = szds[1];

        V3DLONG pageszds = sx*sy*sz;
        V3DLONG totalplxsds = pagesz*sc;

        y_new<unsigned char, V3DLONG>(p_subds, totalplxsds);

        //
        V3DLONG *sz_sub=NULL, *sz_subds=NULL;

        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_subds, 4);

        foreach(3, i)
        {
            sz_sub[i] = sz_img2[i];
            sz_subds[i] = szds[i];
        }

        sz_sub[3] = 1;
        sz_subds[3] = 1;

        foreach(sc, c)
        {
            //
            unsigned char *pinput = p_sub + c*pagesz;
            unsigned char *poutput = p_subds + c*pageszds;

            //
            Y_IMG_UINT8 pIn;
            pIn.setImage((unsigned char*)pinput, sz_sub, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage((unsigned char*)poutput, sz_subds, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        // de-alloc
        y_del<unsigned char>(p_sub);
        y_del<unsigned char>(p1dImg1);
        y_del<unsigned char>(p1dImg2);
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_img2);

        //
        /// resize
        sz_img1[3] = sc;
        pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        totalplxs = pagesz*sz_img1[3];

        unsigned char *p_subdsrec = NULL;
        y_new<unsigned char, V3DLONG>(p_subdsrec, totalplxs);

        foreach(sc, c)
        {
            //
            unsigned char *pinput = p_subds + c*pageszds;
            unsigned char *poutput = p_subdsrec + c*pagesz;

            //
            Y_IMG_UINT8 pIn;
            pIn.setImage((unsigned char*)pinput, sz_subds, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage((unsigned char*)poutput, sz_img1, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, IMCENTER);
        }

        // de-alloc
        y_del<unsigned char>(p_subds);
        y_del<V3DLONG>(sz_subds);

        //
        /// flip along z
        unsigned char *p_subdsrecflip = NULL;
        y_new<unsigned char, V3DLONG>(p_subdsrecflip, totalplxs);

        foreach(sc, c)
        {
            //
            unsigned char *pinput = p_subdsrec + c*pagesz;
            unsigned char *poutput = p_subdsrecflip + c*pagesz;

            flip<unsigned char, V3DLONG>(poutput, pinput, sz_img1, AZ); // z
        }

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_subdsrecflip),sz_img1,1))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p_subdsrec);
        y_del<unsigned char>(p_subdsrecflip);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("prepare20xData"))
    {
        // input  : 20x fly brain image (VNC segmented)
        // output : resized (center of mass is adjusted to the center of image)

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f prepare20xData -o <output> -p \"#s <subject> #t <target> #k <keepinputdatatype>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output;

        int keepdatatype = 0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "k"))
                            {
                                keepdatatype = atoi( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_img_sub).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") == 0
                     || QFileInfo(qs_filename_output).suffix().toUpper().compare("V3DRAW") == 0))
                {
                    QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                    QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                    qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
                }

            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }

        /// load images

        // target
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_img_tar).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // subject
        V3DLONG *sz_img2 = 0;
        int datatype_img2 = 0;
        unsigned char* p1dImg2 = 0;

        if(QFileInfo(qs_filename_img_sub).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }

            qDebug()<<"pixelnbits ... "<<pixelnbits<<" datatype ... "<<datatype_img2;
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()),p1dImg2,sz_img2,datatype_img2))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }

        //
        /// convert to 8bit data
        unsigned char *p_sub=NULL;
        V3DLONG sc = sz_img2[3];
        V3DLONG totalplxs_sub = sz_img2[0]*sz_img2[1]*sz_img2[2]*sc;

        bool b_uint8 = false;

        if(datatype_img2==UINT8)
        {
            //
            p_sub = p1dImg2;
            b_uint8 = true;
        }
        else if(datatype_img2==UINT16)
        {
            //
            unsigned short *p = (unsigned short *)p1dImg2;

            if(!keepdatatype)
            {
                rescaling<unsigned short, V3DLONG>(p, totalplxs_sub, 0, 255);

                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub);
                foreach(totalplxs_sub, i)   p_sub[i] = p[i];
            }
            else
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*2);

                unsigned short *psub = (unsigned short *)p_sub;
                foreach(totalplxs_sub, i)   psub[i] = p[i];
            }

        }
        else if(datatype_img2==FLOAT32)
        {
            //
            float *p = (float *)p1dImg2;
            rescaling<float, V3DLONG>(p, totalplxs_sub, 0.0f, 255.0f);

            y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub);
            foreach(totalplxs_sub, i)   p_sub[i] = p[i];
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg1);

        //
        /// resize
        sz_img1[3] = sc;
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        V3DLONG totalplxs = pagesz*sz_img1[3];

        unsigned char *p_subrec = NULL;

        if(!keepdatatype || datatype_img2==UINT8) // 8-bit
        {
            //
            y_new<unsigned char, V3DLONG>(p_subrec, totalplxs);

            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_sub, sz_img2, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(p_subrec, sz_img1, 4);

            resizeMC<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, channel_ref_sub);

            // de-alloc
            y_del<unsigned char>(p1dImg2);
            y_del<V3DLONG>(sz_img2);
            if(!b_uint8 && p_sub){delete []p_sub; p_sub=NULL;}

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_subrec),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(datatype_img2==UINT16)
        {
            // 16-bit

            //
            y_new<unsigned char, V3DLONG>(p_subrec, totalplxs*2);

            unsigned short *psub = (unsigned short *)p_sub;
            unsigned short *psubrec = (unsigned short *)p_subrec;

            //
            Y_IMG_UINT16 pIn;
            pIn.setImage(psub, sz_img2, 4);

            Y_IMG_UINT16 pOut;
            pOut.setImage(psubrec, sz_img1, 4);

            resizeMC<V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16>(pOut, pIn, channel_ref_sub);

            // de-alloc
            y_del<unsigned char>(p1dImg2);
            y_del<V3DLONG>(sz_img2);
            if(!b_uint8 && p_sub){delete []p_sub; p_sub=NULL;}

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_subrec),sz_img1,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        // de-alloc
        y_del<unsigned char>(p_subrec);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("resizeImage"))
    {
        // input  : an image
        // output : a resized image to the same dimension of the reference image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f resizeImage -o <output> -p \"#s <subject> #t <target> #k <keepinputdatatype>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }} // specify input
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        int b_interptype = 0; // trilinear interpolation
        bool b_skipsampling = false;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output;

        int keepdatatype = 0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "k"))
                            {
                                keepdatatype = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                b_interptype = atoi( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "y")) // skip sampling
                            {
                                b_skipsampling = atoi( argv[i+1] )!=0?true:false;
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_img_sub).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") == 0
                     || QFileInfo(qs_filename_output).suffix().toUpper().compare("V3DRAW") == 0))
                {
                    QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                    QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                    qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
                }

            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }

        /// load images

        // target
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_img_tar).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // subject
        V3DLONG *sz_img2 = 0;
        int datatype_img2 = 0;
        unsigned char* p1dImg2 = 0;

        if(QFileInfo(qs_filename_img_sub).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }

            qDebug()<<"pixelnbits ... "<<pixelnbits<<" datatype ... "<<datatype_img2;
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()),p1dImg2,sz_img2,datatype_img2))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }

        //
        /// convert to 8bit data by default
        unsigned char *p_sub=NULL;
        V3DLONG sc = sz_img2[3];
        V3DLONG totalplxs_sub = sz_img2[0]*sz_img2[1]*sz_img2[2]*sc;

        bool b_uint8 = false;

        if(datatype_img2==UINT8)
        {
            //
            p_sub = p1dImg2;
            b_uint8 = true;
        }
        else if(datatype_img2==UINT16)
        {
            //
            unsigned short *p = (unsigned short *)p1dImg2;

            if(!keepdatatype)
            {
                rescaling<unsigned short, V3DLONG>(p, totalplxs_sub, 0, 255);

                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub);
                foreach(totalplxs_sub, i)   p_sub[i] = p[i];
            }
            else
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*2);

                unsigned short *psub = (unsigned short *)p_sub;
                foreach(totalplxs_sub, i)   psub[i] = p[i];
            }

        }
        else if(datatype_img2==FLOAT32)
        {
            //
            float *p = (float *)p1dImg2;
            rescaling<float, V3DLONG>(p, totalplxs_sub, 0.0f, 255.0f);

            y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub);
            foreach(totalplxs_sub, i)   p_sub[i] = p[i];
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg1);

        //
        /// resize
        sz_img1[3] = sc;
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        V3DLONG totalplxs = pagesz*sz_img1[3];

        unsigned char *p_subrec = NULL;

        if(!keepdatatype || datatype_img2==UINT8) // 8-bit
        {
            //
            y_new<unsigned char, V3DLONG>(p_subrec, totalplxs);

            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_sub, sz_img2, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(p_subrec, sz_img1, 4);

            resizeImage<unsigned char, V3DLONG, Y_IMG_UINT8>(pOut, pIn, b_interptype, b_skipsampling);

            // de-alloc
            if(!b_uint8) y_del<unsigned char>(p1dImg2);

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_subrec),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }

            // de-alloc
            pIn.clean();
            pOut.clean();
        }
        else if(datatype_img2==UINT16)
        {
            // 16-bit

            //
            y_new<unsigned char, V3DLONG>(p_subrec, totalplxs*2);

            unsigned short *psub = (unsigned short *)p_sub;
            unsigned short *psubrec = (unsigned short *)p_subrec;

            //
            Y_IMG_UINT16 pIn;
            pIn.setImage(psub, sz_img2, 4);

            Y_IMG_UINT16 pOut;
            pOut.setImage(psubrec, sz_img1, 4);

            resizeImage<unsigned short, V3DLONG, Y_IMG_UINT16>(pOut, pIn, b_interptype, b_skipsampling);

            // de-alloc
            if(!b_uint8) y_del<unsigned char>(p1dImg2);

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_subrec),sz_img1,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }

            // de-alloc
            pIn.clean();
            pOut.clean();
        }

        //
        return true;
    }
    else if (func_name == tr("MultiLabelImageConverter"))
    {
        // input  : MultiLabelImage
        // output : MultiLabelImage

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f MultiLabelImageConverter -i <input> -o <output> -p \"#b <1/2/4>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        int outputbyte=1; // 1: 8bit 2: 16bit 4: 32bit

        // inputs
        QString qs_filename_input(infile);

        // outputs
        QString qs_filename_output(outfile);

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "b"))
                            {
                                outputbyte = atoi( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

        }

        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        //
        /// convert
        //

        if(outputbyte<0)
        {
            cout<<"Invalid input"<<endl;
            return false;
        }

        unsigned char *p_sub=NULL;
        V3DLONG totalplxs_sub = sz_img1[0]*sz_img1[1]*sz_img1[2]*sz_img1[3];

        if(datatype_img1==UINT8)
        {
            if(outputbyte==1)
            {
                p_sub = p1dImg1;
            }
            else if(outputbyte==2)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*outputbyte);

                unsigned short *p = (unsigned short *)p_sub;

                foreach(totalplxs_sub, i) p[i] = p1dImg1[i];
            }
            else if(outputbyte==4)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*outputbyte);

                float *p = (float *)p_sub;

                foreach(totalplxs_sub, i) p[i] = p1dImg1[i];
            }
            else
            {
                cout<<"Invalide byte specified"<<endl;
                return false;
            }
        }
        else if(datatype_img1==UINT16)
        {
            unsigned short *pinput = (unsigned short *)p1dImg1;

            if(outputbyte==1)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*outputbyte);

                unsigned char *p = (unsigned char *)p_sub;

                foreach(totalplxs_sub, i) p[i] = pinput[i];
            }
            else if(outputbyte==2)
            {
                p_sub = p1dImg1;
            }
            else if(outputbyte==4)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*outputbyte);

                float *p = (float *)p_sub;

                foreach(totalplxs_sub, i) p[i] = pinput[i];
            }
            else
            {
                cout<<"Invalide byte specified"<<endl;
                return false;
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        /// save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_sub),sz_img1,outputbyte))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p_sub);
        y_del<unsigned char>(p1dImg1);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("isampler"))
    {
        // input  : an image stack
        // output : resampled image stack

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f isampler -o <output> -p \"#x <sample_ratio_x> #y <sample_ratio_y> #z <sample_ratio_z> #i <interp_type>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;

        int b_interptype = 0; // trilinear interpolation

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                b_interptype = atoi( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_input==NULL || srx<0 || sry<0 || srz<0)
            {
                printf("\nERROR: invalid input!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_input).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_sampled.v3draw";
            }

        }

        /// load images

        // subject
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        /// resample the input

        unsigned char *pResample = NULL;
        V3DLONG *szResample=NULL;

        y_new<V3DLONG, V3DLONG>(szResample, 4);
        szResample[0] = srx*sz_img1[0]; // srx>1 upsample srx<1 downsample
        szResample[1] = sry*sz_img1[1];
        szResample[2] = srz*sz_img1[2];
        szResample[3] = sz_img1[3];

        V3DLONG pagesz_input = sz_img1[0]*sz_img1[1]*sz_img1[2];
        V3DLONG pagesz = szResample[0]*szResample[1]*szResample[2];
        V3DLONG totalplxs = pagesz*szResample[3];
        V3DLONG cdims = szResample[3];

        //
        y_new<unsigned char, V3DLONG>(pResample, totalplxs*datatype_img1);

        foreach(cdims, c)
        {
            if(datatype_img1==UINT8)
            {
                //
                unsigned char *pinput = p1dImg1 + c*pagesz_input;
                unsigned char *poutput = pResample + c*pagesz;

                //
                Y_IMG_UINT8 pIn;
                pIn.setImage(pinput, sz_img1, 4);

                Y_IMG_UINT8 pOut;
                pOut.setImage(poutput, szResample, 4);


                if(b_interptype)
                {
                    // NN
                    resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLENN);
                }
                else
                {
                    // Linear
                    resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
                }

            }
            else if(datatype_img1==UINT16)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg1 + c*pagesz_input;
                unsigned short *poutput = (unsigned short *)pResample + c*pagesz;

                //
                Y_IMG_UINT16 pIn;
                pIn.setImage(pinput, sz_img1, 4);

                Y_IMG_UINT16 pOut;
                pOut.setImage(poutput, szResample, 4);

                if(b_interptype)
                {
                    // NN
                    resize<V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16>(pOut, pIn, SAMPLENN);
                }
                else
                {
                    // Linear
                    resize<V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16>(pOut, pIn, SAMPLE);
                }

            }
            else if(datatype_img1==FLOAT32)
            {

                //
                float *pinput = (float *)p1dImg1 + c*pagesz_input;
                float *poutput = (float *)pResample + c*pagesz;

                //
                Y_IMG_REAL pIn;
                pIn.setImage(pinput, sz_img1, 4);

                Y_IMG_REAL pOut;
                pOut.setImage(poutput, szResample, 4);

                if(b_interptype)
                {
                    // NN
                    resize<V3DLONG, Y_IMG_REAL, Y_IMG_REAL>(pOut, pIn, SAMPLENN);
                }
                else
                {
                    // Linear
                    resize<V3DLONG, Y_IMG_REAL, Y_IMG_REAL>(pOut, pIn, SAMPLE);
                }

            }
            else
            {
                cout<<"Invalid input datatype!"<<endl;
                return false;
            }
        }

        // de-alloc
        y_del<unsigned char>(p1dImg1);
        y_del<V3DLONG>(sz_img1);

        /// save
        if(datatype_img1==UINT8)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pResample),szResample,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(datatype_img1==UINT16)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pResample),szResample,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(datatype_img1==FLOAT32)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pResample),szResample,4))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(pResample);
        y_del<V3DLONG>(szResample);

        //
        return true;
    }
    else if (func_name == tr("iContrastEnhancer"))
    {
        // input  : an image stack
        // output : contrast enhanced image stack

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f iContrastEnhancer -i <input> -o <output> -p \"#s <block_size> #m <maximum_slope> #b <histogram_bins>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        REAL maxslope = 3.0;
        unsigned long szblock = 16;
        unsigned long histbins = 256;

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "s"))
                            {
                                szblock = atol( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                maxslope = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "b"))
                            {
                                histbins = atol( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".tif";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_enhanced.tif";
        }


        /// load images

        // subject
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        /// enhance the input

        V3DLONG sx = sz_img1[0];
        V3DLONG sy = sz_img1[1];

        V3DLONG ssx = sx;
        V3DLONG ssy = sy;

        bool b_resize = false;

        if(sx%szblock)
        {
            b_resize = true;

            ssx = (sx/szblock + 1)*szblock;
        }

        if(sy%szblock)
        {
            b_resize = true;

            ssy = (sy/szblock + 1)*szblock;
        }

        V3DLONG i,j,k, offset, offset_r, offsety, offsety_r, pagesz = sx*sy;
        V3DLONG pages = sz_img1[2]*sz_img1[3];
        V3DLONG totalplxs = ssx*ssy*pages;

        //
        if(datatype_img1==UINT8)
        {
            //
            unsigned char *p = NULL;

            if(b_resize)
            {
                y_new<unsigned char, V3DLONG>(p, totalplxs);
                foreach(totalplxs, i) p[i] = 0;

                for(k=0; k<pages; k++)
                {
                    offset = k*pagesz;
                    offset_r = k*ssx*ssy;

                    for(j=0; j<sy; j++)
                    {
                        offsety = offset + j*sx;
                        offsety_r = offset_r + j*ssx;

                        for(i=0; i<sx; i++)
                        {
                            p[i+offsety_r] = p1dImg1[i+offsety];
                        }
                    }
                }

                sz_img1[0] = ssx;
                sz_img1[1] = ssy;

                contrastEnhancing<unsigned char, unsigned long>(p, (unsigned long *)sz_img1, (unsigned long)datatype_img1, szblock, histbins, maxslope);

                sz_img1[0] = sx;
                sz_img1[1] = sy;

                for(k=0; k<pages; k++)
                {
                    offset = k*pagesz;
                    offset_r = k*ssx*ssy;

                    for(j=0; j<sy; j++)
                    {
                        offsety = offset + j*sx;
                        offsety_r = offset_r + j*ssx;

                        for(i=0; i<sx; i++)
                        {
                            p1dImg1[i+offsety] = p[i+offsety_r];
                        }
                    }
                }
                y_del<unsigned char>(p);

            }
            else
            {
                p = p1dImg1;

                contrastEnhancing<unsigned char, unsigned long>(p, (unsigned long *)sz_img1, (unsigned long)datatype_img1, szblock, histbins, maxslope);
            }
        }
        else if(datatype_img1==UINT16)
        {
            //
            unsigned short *p = NULL;
            if(b_resize)
            {
                y_new<unsigned short, V3DLONG>(p, totalplxs);
                foreach(totalplxs, i) p[i] = 0;

                for(k=0; k<pages; k++)
                {
                    offset = k*pagesz;
                    offset_r = k*ssx*ssy;

                    for(j=0; j<sy; j++)
                    {
                        offsety = offset + j*sx;
                        offsety_r = offset_r + j*ssx;

                        for(i=0; i<sx; i++)
                        {
                            p[i+offsety_r] = p1dImg1[i+offsety];
                        }
                    }
                }

                sz_img1[0] = ssx;
                sz_img1[1] = ssy;

                contrastEnhancing<unsigned short, unsigned long>(p, (unsigned long *)sz_img1, (unsigned long)datatype_img1, szblock, histbins, maxslope);

                sz_img1[0] = sx;
                sz_img1[1] = sy;

                for(k=0; k<pages; k++)
                {
                    offset = k*pagesz;
                    offset_r = k*ssx*ssy;

                    for(j=0; j<sy; j++)
                    {
                        offsety = offset + j*sx;
                        offsety_r = offset_r + j*ssx;

                        for(i=0; i<sx; i++)
                        {
                            p1dImg1[i+offsety] = p[i+offsety_r];
                        }
                    }
                }
                y_del<unsigned short>(p);

            }
            else
            {
                p = (unsigned short *) p1dImg1;

                contrastEnhancing<unsigned short, unsigned long>(p, (unsigned long *)sz_img1, (unsigned long)datatype_img1, szblock, histbins, maxslope);
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        /// save
        if(datatype_img1==UINT8)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p1dImg1),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(datatype_img1==UINT16)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p1dImg1),sz_img1,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg1);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("bwthinning"))
    {
        // input  : an image stack with original signals (red) and binary mask (green)
        // output : thinning result

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f bwthinning -i <input> -o <output> -p \"\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        REAL maxslope = 3.0;
        V3DLONG szblock = 16;
        V3DLONG histbins = 256;

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".tif";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_enhanced.tif";
        }


        /// load images

        // subject
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        /// thinning

        if(sz_img1[3]<2)
        {
            cout<<"invalid input. at least two color channels needed"<<endl;
            return false;
        }


        //
        if(datatype_img1==UINT8)
        {
            //
            unsigned char threshold = 10;

            //
            V3DLONG sx, sy, sz;

            sx = sz_img1[0];
            sy = sz_img1[1];
            sz = sz_img1[2];

            V3DLONG offset_y, offset_z;
            offset_y = sx;
            offset_z = offset_y*sy;
            V3DLONG pagesz = offset_z*sz;

            //
            unsigned char *pR = p1dImg1;
            unsigned char *pG = p1dImg1 + pagesz;

            V3DLONG neighbors[6] = {-1, 1, -offset_y, offset_y, -offset_z, offset_z};

            bool b_dothinning = true;
            int iters = 0;

            while(b_dothinning)
            {
                cout<<"itering ... "<<++iters<<endl;

                bool b_surfacepoints = false;
                V3DLONG sum = 0;

                for(V3DLONG k=1; k<sz-1; k++)
                {
                    V3DLONG idxk = k*offset_z;
                    for(V3DLONG j=1; j<sy-1; j++)
                    {
                        V3DLONG idxj = idxk + j*offset_y;
                        for(V3DLONG i=1; i<sx-1; i++)
                        {
                            V3DLONG idx = idxj + i;

                            if(pG[idx]>0)
                            {
                                for(int n=0; n<6; n++)
                                {
                                    if(pG[idx+neighbors[n]]==0)
                                    {
                                        b_surfacepoints = true; //qDebug()<<"find a surface point";
                                        break;
                                    }
                                }
                            }

                            // surface points
                            if(b_surfacepoints)
                            {
                                b_surfacepoints = false;

                                if(pR[idx]<threshold)
                                {
                                    pG[idx] = 0;
                                    //qDebug()<<"delete a surface point";
                                    sum++;
                                }
                            }
                        }
                    }
                }
                qDebug()<<"surface point(s) removed ..."<<sum;
                if(sum<1) b_dothinning = false;
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        /// save
        if(datatype_img1==UINT8)
        {
            //
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p1dImg1),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg1);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("lmreg"))
    {
        // input  : Source image, Template image, Source Landmarkers, Template Landmarkers
        // output : warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<"Usage: vaa3d -x ireg -f lmreg -p \"#s <subject_image> #l <subject_markers> #t <target_image> #L <target_markers>\""<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;
        QString qs_filename_markers_sub, qs_filename_markers_tar;

        // outputs
        QString qs_filename_displacement; // displacement field
        QString qs_filename_deformed;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "L"))
                            {
                                qs_filename_markers_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                qs_filename_markers_sub = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL || qs_filename_markers_tar==NULL || qs_filename_markers_sub==NULL)
            {
                printf("\nERROR: invalid inputs!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+"Deformed.v3draw";
            }
            else
            {
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"Deformed.v3draw";
            }
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read the inputs. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("Error: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("Error: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }

        if(sz_img_tar_input[3]>1 || sz_img_sub_input[3]>1)
        {
            cout<<"Error in specifying the reference channel"<<endl;
            return false;
        }

        QList <ImageMarker> landmarkers_sub, landmarkers_tar;
        landmarkers_sub = readMarker_file(qs_filename_markers_sub);
        landmarkers_tar = readMarker_file(qs_filename_markers_tar);

        Y_LANDMARK lm_sub, lm_tar;

        V3DLONG m = landmarkers_sub.size(); // assume the quantity markers same
        for(V3DLONG i=0; i<m; i++)
        {
            Y_MARKER<double> p;

            p.x = landmarkers_sub.at(i).x;
            p.y = landmarkers_sub.at(i).y;
            p.z = landmarkers_sub.at(i).z;

            lm_sub.push_back(p);

            p.x = landmarkers_tar.at(i).x;
            p.y = landmarkers_tar.at(i).y;
            p.z = landmarkers_tar.at(i).z;

            lm_tar.push_back(p);
        }


        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Align. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // solution matrix for TPS linear system
        V3DLONG *sz_wa=NULL;
        y_new<V3DLONG, V3DLONG>(sz_wa, 4);

        sz_wa[0] = 3; sz_wa[1] = m+4; sz_wa[2] = 1; sz_wa[3] = 1;

        Y_IMG_REAL pOut;
        pOut.initImage(sz_wa, 4);

        y_del<V3DLONG>(sz_wa);

        // deformed output
        Y_IMG_REAL pSDeformedD;
        pSDeformedD.initImage(sz_img_tar_input, 4);

        //
        RegistrationType regtype  = NONRIGID;
        OptimizationType opttype  = GradientDescent;
        SimilarityType smltype  = SSD;
        InterpolationType inttype  = NearestNeighbor;
        TransformDirection transdir = FORWARD;

        // landmark-based registration
        if(datatype_tar_input==1)
        {

            Y_IMG_UINT8 pTar;
            pTar.setImage((unsigned char*)p_img_tar_input, sz_img_tar_input, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char*)p_img_sub_input, sz_img_sub_input, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8, Y_IMG_REAL> lmaligner;

                lmaligner.opt.regtype  = regtype;
                lmaligner.opt.opttype  = opttype;
                lmaligner.opt.smltype  = smltype;
                lmaligner.opt.inttype  = inttype;
                lmaligner.opt.transdir = transdir;

                // alignment
                lmaligner.lmregistration(pOut, pSub, pTar, lm_sub, lm_tar);

                // warp
                //foreach(pTar.totalplxs, i) pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                // source image control points
                pSDeformedD.pImg[0] = m; // m
                for(V3DLONG i=0; i<m; i++)
                {
                    pSDeformedD.pImg[3*i+3] = lm_tar.at(i).x;
                    pSDeformedD.pImg[3*i+4] = lm_tar.at(i).y;
                    pSDeformedD.pImg[3*i+5] = lm_tar.at(i).z;
                }

                //
                warp<V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, lmaligner.opt);

            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_img_sub_input, sz_img_sub_input, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT16, Y_IMG_UINT8, Y_IMG_REAL> lmaligner;

                lmaligner.opt.regtype  = regtype;
                lmaligner.opt.opttype  = opttype;
                lmaligner.opt.smltype  = smltype;
                lmaligner.opt.inttype  = inttype;
                lmaligner.opt.transdir = transdir;

                // alignment
                lmaligner.lmregistration(pOut, pSub, pTar, lm_sub, lm_tar);

                // warp
                //foreach(pTar.totalplxs, i) pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                // source image control points
                pSDeformedD.pImg[0] = m; // m
                for(V3DLONG i=0; i<m; i++)
                {
                    pSDeformedD.pImg[3*i+3] = lm_tar.at(i).x;
                    pSDeformedD.pImg[3*i+4] = lm_tar.at(i).y;
                    pSDeformedD.pImg[3*i+5] = lm_tar.at(i).z;
                }

                //
                warp<V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, lmaligner.opt);
            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==2)
        {

            Y_IMG_UINT16 pTar;
            pTar.setImage((unsigned short *)p_img_tar_input, sz_img_tar_input, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char*)p_img_sub_input, sz_img_sub_input, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT8, Y_IMG_UINT16, Y_IMG_REAL> lmaligner;

                lmaligner.opt.regtype  = regtype;
                lmaligner.opt.opttype  = opttype;
                lmaligner.opt.smltype  = smltype;
                lmaligner.opt.inttype  = inttype;
                lmaligner.opt.transdir = transdir;

                // alignment
                lmaligner.lmregistration(pOut, pSub, pTar, lm_sub, lm_tar);

                // warp
                //foreach(pTar.totalplxs, i) pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                // source image control points
                pSDeformedD.pImg[0] = m; // m
                for(V3DLONG i=0; i<m; i++)
                {
                    pSDeformedD.pImg[3*i+3] = lm_tar.at(i).x;
                    pSDeformedD.pImg[3*i+4] = lm_tar.at(i).y;
                    pSDeformedD.pImg[3*i+5] = lm_tar.at(i).z;
                }

                //
                warp<V3DLONG, Y_IMG_UINT8, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, lmaligner.opt);

            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_img_sub_input, sz_img_sub_input, 4);

                Y_IREG<REAL, V3DLONG, Y_IMG_UINT16, Y_IMG_UINT16, Y_IMG_REAL> lmaligner;

                lmaligner.opt.regtype  = regtype;
                lmaligner.opt.opttype  = opttype;
                lmaligner.opt.smltype  = smltype;
                lmaligner.opt.inttype  = inttype;
                lmaligner.opt.transdir = transdir;

                // alignment
                lmaligner.lmregistration(pOut, pSub, pTar, lm_sub, lm_tar);

                // warp
                //foreach(pTar.totalplxs, i) pSDeformedD.pImg[i] = pTar.pImg[i]; // init pSWarpD = pTar for matching mass of centers (optional)

                // source image control points
                pSDeformedD.pImg[0] = m; // m
                for(V3DLONG i=0; i<m; i++)
                {
                    pSDeformedD.pImg[3*i+3] = lm_tar.at(i).x;
                    pSDeformedD.pImg[3*i+4] = lm_tar.at(i).y;
                    pSDeformedD.pImg[3*i+5] = lm_tar.at(i).z;
                }

                //
                warp<V3DLONG, Y_IMG_UINT16, Y_IMG_REAL, Y_IMG_REAL>(pSDeformedD, pSub, pOut, lmaligner.opt);
            }
            else if(datatype_sub_input==4)
            {

            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==4)
        {
            //
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // save the warped image
        if(qs_filename_deformed!=NULL)
        {
            // save 8-bit deformed reference
            Y_IMG_UINT8 pSDeformedD8bit;
            pSDeformedD8bit.initImage(pSDeformedD.dims, 4);
            normalizing<REAL, V3DLONG>(pSDeformedD.pImg, pSDeformedD.totalplxs, 0, 255);
            foreach(pSDeformedD.totalplxs, i)   pSDeformedD8bit.pImg[i] = pSDeformedD.pImg[i];

            if(!saveImage(qPrintable(qs_filename_deformed),(unsigned char *)(pSDeformedD.pImg),pSDeformedD.dims,4))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
            pSDeformedD8bit.clean();
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        pSDeformedD.clean();
        pOut.clean();

        //
        return true;
    }
    else if (func_name == tr("QiScoreStas"))
    {
        // input  : target landmarks, aligned control points, optic lobes and central brain label image
        // output : Qi Score for left optic lobe, central brain, and right optic lobe

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f QiScoreStas -o <output> -p \"#l <aligned_landmarks> #t <target_landmarks> #m <imagemask>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }


        // inputs
        QString qs_filename_image, qs_filename_marker;
        QString qs_filename_tgt_marker;

        // outputs
        QString qs_filename_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "l"))
                            {
                                qs_filename_marker = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "t"))
                            {
                                qs_filename_tgt_marker = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                qs_filename_image = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_image==NULL || qs_filename_marker==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_image).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_image).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".csv";
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".csv";
            }

        }

        /// load files
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_image).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_image.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_image.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        //
        V3DLONG sx = sz_img1[0];
        V3DLONG sy = sz_img1[1];
        V3DLONG sz = sz_img1[2];

        //
        REAL cbmarkers = 231;
        REAL lomarkers = 144;
        REAL romarkers = 125;

        QList <ImageMarker> landmarkers, landmarkers_tgt;
        landmarkers = readMarker_file(qs_filename_marker);

        Y_LANDMARK lm;

        V3DLONG m = landmarkers.size(); //

        qDebug()<<"we have found "<<m<<" corresponding points! ";

        for(V3DLONG i=0; i<m; i++)
        {
            Y_MARKER<double> p;

            p.x = landmarkers.at(i).x;
            p.y = landmarkers.at(i).y;
            p.z = landmarkers.at(i).z;

            lm.push_back(p);
        }

        if(qs_filename_tgt_marker!=NULL)
        {
            landmarkers_tgt = readMarker_file(qs_filename_tgt_marker);

            V3DLONG m_tgt = landmarkers_tgt.size(); //

            qDebug()<<"we have "<<m_tgt<<" markers of target! ";

            V3DLONG cb=0, lo=0, ro=0;

            for(V3DLONG i=0; i<m_tgt; i++)
            {
                V3DLONG z = landmarkers_tgt.at(i).z;
                V3DLONG y = landmarkers_tgt.at(i).y;
                V3DLONG x = landmarkers_tgt.at(i).x;

                if(z>sz-1){ z=sz-1;}
                if(y>sy-1){ y=sy-1;}
                if(x>sx-1){ x=sx-1;}

                V3DLONG idx = z * sx * sy + y * sx + x;

                if(p1dImg1[idx]==0)
                {
                    cb++;
                }
                else if(p1dImg1[idx]==254)
                {
                    lo++;
                }
                else if(p1dImg1[idx]==255)
                {
                    ro++;
                }
            }

            //
            cbmarkers = cb;
            lomarkers = lo;
            romarkers = ro;

            if(cbmarkers<1 || lomarkers<1 || romarkers<1)
            {
                cout<<"Invalid input target landmarks!"<<endl;
                return -1;
            }
        }

        //
        V3DLONG cntlo=0,cntcb=0,cntro=0;

        if(datatype_img1==UINT8)
        {
            //
            for(V3DLONG i=0; i<m; i++)
            {

                V3DLONG z = lm.at(i).z;
                V3DLONG y = lm.at(i).y;
                V3DLONG x = lm.at(i).x;

                if(z>sz-1){ z=sz-1;}
                if(y>sy-1){ y=sy-1;}
                if(x>sx-1){ x=sx-1;}

                V3DLONG idx = z * sx * sy + y * sx + x;

                if(p1dImg1[idx]==0)
                {
                    cntcb++;
                }
                else if(p1dImg1[idx]==254)
                {
                    cntlo++;
                }
                else if(p1dImg1[idx]==255)
                {
                    cntro++;
                }
            }

            //qDebug()<<"test ... "<<cntcb<<cntlo<<cntro;

        }
        else if(datatype_img1==UINT16)
        {
            //
        }
        else if(datatype_img1==FLOAT32)
        {
            //
        }
        else
        {
            cout<<"Invalid input datatype!"<<endl;
            return false;
        }

        // save
        FILE *pFileOut=0;

        pFileOut = fopen(qPrintable(qs_filename_output),"wt");
        if (pFileOut == NULL) perror ("Error opening writing file!");
        else
        {
            fprintf(pFileOut, "Qi_Score_LeftOL,Qi_Score_CentralBrain,Qi_Score_RightOL\n");
            fprintf(pFileOut, "%f,%f,%f\n", (REAL)cntlo/lomarkers, (REAL)cntcb/cbmarkers, (REAL)cntro/romarkers);
        }
        fclose(pFileOut);

        // de-alloc
        y_del<unsigned char>(p1dImg1);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("iSymmetryDet"))
    {
        // input  : Source image, Template image
        // output : affine transform, displacement field, warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nUsage: ./vaa3d -x ireg -f iSymmetryDet -i <input_image> -o <output_image>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_input_img = QString(infile);

        // outputs
        QString qs_output_img;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                //qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                //channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                //qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                //channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // error check inputs
        if(qs_input_img==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_input_img).baseName();
        QString qs_pathname_input=QFileInfo(qs_basename_input).path();

        if(outfile)
        {
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_output_img=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_output_img=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        V3DLONG *szImg_input=NULL;
        unsigned char *pImg1d_input=NULL;
        int datatype_input=0;

        if(!loadImage((char *)qPrintable(qs_input_img),pImg1d_input,szImg_input,datatype_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_input_img));
            y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);
            return false;
        }

        // output
        Y_IMG_REAL yImg;
        Y_LANDMARK yLM;
        Y_IMG_UINT8 yImg8bit;

        //
        yImg.initImage(szImg_input, 4);
        foreach(yImg.totalplxs, i)
            yImg.pImg[i] = pImg1d_input[i];

        //
        symmetryDet<V3DLONG, REAL, Y_IMG_REAL>(yImg, yLM);

        // save the aligned image
        if(qs_output_img!=NULL)
        {
            // save 8-bit deformed reference
            yImg8bit.initImage(yImg.dims, 4);
            normalizing<REAL, V3DLONG>(yImg.pImg, yImg.totalplxs, 0, 255);
            foreach(yImg.totalplxs, i)
                yImg8bit.pImg[i] = yImg.pImg[i];

            if(!saveImage(qPrintable(qs_output_img),(unsigned char *)(yImg8bit.pImg),yImg8bit.dims,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);

        yImg.clean();
        yImg8bit.clean();

        //
        return true;
    }
    else if (func_name == tr("iRecenter"))
    {
        // input  : Source Image
        // output : Recentered Image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nUsage: ./vaa3d -x ireg -f iRecenter -i <input_image> -o <output_image>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // inputs
        QString qs_input_img = QString(infile);

        // outputs
        QString qs_output_img;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                //qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                //channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                //qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                //channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // error check inputs
        if(qs_input_img==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_input_img).baseName();
        QString qs_pathname_input=QFileInfo(qs_basename_input).path();

        if(outfile)
        {
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_output_img=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_output_img=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        V3DLONG *szImg_input=NULL;
        unsigned char *pImg1d_input=NULL;
        int datatype_input=0;

        if(!loadImage((char *)qPrintable(qs_input_img),pImg1d_input,szImg_input,datatype_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_input_img));
            y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);
            return false;
        }

        // output
        Y_IMG_UINT8 yImgIn;
        Y_IMG_UINT8 yImgOut;

        //
        yImgIn.initImage(szImg_input, 4);
        foreach(yImgIn.totalplxs, i)
            yImgIn.pImg[i] = pImg1d_input[i];

        yImgOut.initImage(szImg_input, 4);

        //
        resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(yImgOut, yImgIn, CENTER);

        // save
        if(qs_output_img!=NULL)
        {
            //
            if(!saveImage(qPrintable(qs_output_img),(unsigned char *)(yImgOut.pImg),yImgOut.dims,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);

        yImgIn.clean();
        yImgOut.clean();

        //
        return true;
    }
    else if (func_name == tr("iTiling"))
    {
        // input  : Source Image
        // output : Tiled Images

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nUsage: ./vaa3d -x ireg -f iTiling -i <input_image> -o <output_image>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_input_img = QString(infile);

        // outputs
        QString qs_output_img = NULL;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                //qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                //channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                //qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                //channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // error check inputs
        if(qs_input_img==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_input_img).baseName();
        QString qs_pathname_input=QFileInfo(qs_basename_input).path();

        if(outfile)
        {
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_output_img=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_output_img=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        V3DLONG *szImg_input=NULL;
        unsigned char *pImg1d_input=NULL;
        int datatype_input=0;

        if(!loadImage((char *)qPrintable(qs_input_img),pImg1d_input,szImg_input,datatype_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_input_img));
            y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);
            return false;
        }

        // Image Tiling
        V3DLONG sx = szImg_input[0];
        V3DLONG sy = szImg_input[1];
        V3DLONG sz = szImg_input[2];
        V3DLONG sc = szImg_input[3];


        // length = 0 ---- 40%
        // 1: [begin, length]; 2: [30%, 30%+length]; 3: [end-length,end]

        V3DLONG blckdimx = sx*0.75;
        V3DLONG blckdimy = sy*0.75;
        V3DLONG blckdimz = sz*0.4;
        V3DLONG blckdimc = sc;

        V3DLONG szblck = blckdimx * blckdimy * blckdimz * blckdimc;

        V3DLONG bx[2]={0, sx-blckdimx};
        V3DLONG by[2]={0, sy-blckdimy};
        V3DLONG bz[3]={0, sz*0.3, sz-blckdimz};

        V3DLONG sz_tile[4]={blckdimx, blckdimy, blckdimz, blckdimc};

        //
        V3DLONG xn=2, yn=2, zn=3;

        QString qs_filename_img_output = qs_output_img;
        if(datatype_input==V3D_UINT8)
        {
            unsigned char *pTile = NULL;
            y_new<unsigned char, V3DLONG>(pTile, szblck);

            foreach(zn,k)
            {
                foreach(yn,j)
                {
                    foreach(xn,i)
                    {
                        V3DLONG cnt = k*yn*xn + j*xn + i;

                        QString qs_pathname_output=QFileInfo(qs_output_img).path();
                        QString qs_basename_output=QFileInfo(qs_output_img).baseName();

                        qs_filename_img_output=qs_pathname_output+"/"+qs_basename_output+QString("_tile%1.v3draw").arg(cnt);

                        V3DLONG offsets = bz[k]*sy*sx + by[j]*sx + bx[i];

                        // copy
                        foreach(blckdimz,z)
                        {
                            foreach(blckdimy,y)
                            {
                                foreach(blckdimx, x)
                                {
                                    pTile[z*blckdimy*blckdimx + y*blckdimx + x] = pImg1d_input[offsets + z*sy*sx + y*sx + x];
                                }
                            }
                        }

                        // save
                        if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pTile,sz_tile,1))
                        {
                            printf("ERROR: saveImage() return false!\n");
                            return false;
                        }
                    }
                }
            }

            //
            y_del<unsigned char>(pTile);
        }
        else if(datatype_input==V3D_UINT16)
        {
            unsigned short *pTile = NULL;
            unsigned short *pInput = (unsigned short *)pImg1d_input;
            y_new<unsigned short, V3DLONG>(pTile, szblck);

            foreach(zn,k)
            {
                foreach(yn,j)
                {
                    foreach(xn,i)
                    {
                        V3DLONG cnt = k*yn*xn + j*xn + i;

                        QString qs_pathname_output=QFileInfo(qs_output_img).path();
                        QString qs_basename_output=QFileInfo(qs_output_img).baseName();

                        qs_filename_img_output=qs_pathname_output+"/"+qs_basename_output+QString("_tile%1.v3draw").arg(cnt);

                        V3DLONG offsets = bz[k]*sy*sx + by[j]*sx + bx[i];

                        // copy
                        foreach(blckdimz,z)
                        {
                            foreach(blckdimy,y)
                            {
                                foreach(blckdimx, x)
                                {
                                    pTile[z*blckdimy*blckdimx + y*blckdimx + x] = pInput[offsets + z*sy*sx + y*sx + x];
                                }
                            }
                        }

                        // save
                        if(!saveImage(qPrintable(qs_filename_img_output),(unsigned char*)pTile,sz_tile,2))
                        {
                            printf("ERROR: saveImage() return false!\n");
                            return false;
                        }
                    }
                }
            }

            //
            y_del<unsigned short>(pTile);
        }
        else if(datatype_input==V3D_FLOAT32)
        {
            cout << "Image data type is not supported!"<<endl;
            return false;
        }
        else
        {
            cout << "Image data type is not supported!"<<endl;
            return false;
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);

        //
        return true;
    }
    else if (func_name == tr("ierode"))
    {
        // input  : Source Image
        // output : Eroded Image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nUsage: ./vaa3d -x ireg -f ierode -i <input_image> -o <output_image>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_input_img = QString(infile);

        // outputs
        QString qs_output_img;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                //qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                //channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                //qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                //channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // error check inputs
        if(qs_input_img==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_input_img).baseName();
        QString qs_pathname_input=QFileInfo(qs_basename_input).path();

        if(outfile)
        {
            QString qs_filename_output=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_output_img=qs_pathname_output+"/"+qs_basename_output+".tif";
        }
        else
        {
            qs_output_img=qs_pathname_input+"/"+qs_basename_input+".tif";
        }

        V3DLONG *szImg_input=NULL;
        unsigned char *pImg1d_input=NULL;
        int datatype_input=0;

        if(!loadImage((char *)qPrintable(qs_input_img),pImg1d_input,szImg_input,datatype_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_input_img));
            y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);
            return false;
        }

        // erode
        V3DLONG sx = szImg_input[0];
        V3DLONG sy = szImg_input[1];
        V3DLONG sz = szImg_input[2];
        V3DLONG sc = szImg_input[3];
        V3DLONG offset_y = sx;
        V3DLONG offset_z = sx*sy;

        unsigned char *pImg1d_output=NULL;

        V3DLONG totalpxls = sx*sy*sz*sc*datatype_input;
        y_new<unsigned char, V3DLONG>(pImg1d_output,totalpxls);

        for(V3DLONG i=0; i<totalpxls; i++)
            pImg1d_output[i] = pImg1d_input[i];

        for(V3DLONG iter=0; iter<1; iter++)
        {
            V3DLONG neighbors[6] = {-1, 1, -offset_y, offset_y, -offset_z, offset_z};

            for(V3DLONG k=1; k<sz-1; k++)
            {
                V3DLONG idxk = k*offset_z;
                for(V3DLONG j=1; j<sy-1; j++)
                {
                    V3DLONG idxj = idxk + j*offset_y;
                    for(V3DLONG i=1; i<sx-1; i++)
                    {
                        V3DLONG idx = idxj + i;

                        unsigned char val = pImg1d_input[idx];

                        for(int n=0; n<6; n++)
                        {
                            if(pImg1d_input[idx+neighbors[n]]!=val)
                            {
                                pImg1d_output[idx] = 0; //qDebug()<<"find a surface point";
                                break;
                            }
                        }
                    }
                }
            }
        }

        // save
        if(qs_output_img!=NULL)
        {
            //
            if(!saveImage(qPrintable(qs_output_img),(unsigned char *)(pImg1d_output),szImg_input,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(pImg1d_input, szImg_input);
        y_del<unsigned char>(pImg1d_output);

        //
        return true;
    }
    else if (func_name == tr("inormalize"))
    {
        // input  : Source image, Template image
        // output : normalized result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f inormalize -o <output> -p \" #s <sub> #t <tar>\" \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_normalized;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }

                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_normalized=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }
            else
            {
                qs_filename_normalized=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int process_step = 0;

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Read the input(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            return false;
        }

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Normalize. \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        V3DLONG *sz_sub=NULL, *sz_tar=NULL;
        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_tar, 4);

        for(V3DLONG i=0; i<3; i++)
        {
            sz_sub[i] = sz_img_sub_input[i];
            sz_tar[i] = sz_img_tar_input[i];
        }
        sz_sub[3] = 1;
        sz_tar[3] = 1;

        // histogram matching for 8-bit data
        V3DLONG pagesz = sz_sub[0]*sz_sub[1]*sz_sub[2];
        unsigned char * pOut = NULL;
        y_new<unsigned char, V3DLONG>(pOut, pagesz);

        //

        //--------------------------------------------------------------------------------------------------
        printf("\n%d. Save the output(s). \n", ++process_step);
        //--------------------------------------------------------------------------------------------------

        // save the normalized image
        if(qs_filename_normalized!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_normalized),(unsigned char *)(pOut),sz_sub,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_tar);

        //
        return true;
    }
    else if (func_name == tr("createMaskImage"))
    {
        // input  : Source image, Template image
        // output : Mask image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f createMaskImage -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // parsing parameters
        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"Mask.v3draw";
        }


        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        /// create a whole image with label 1

        //
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        unsigned char * pOut = NULL;
        y_new<unsigned char, V3DLONG>(pOut, pagesz);

        //
        foreach(pagesz, i) pOut[i] = 1;

        sz_img1[3] = 1;

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(pOut);

        //
        return true;
    }
    else if (func_name == tr("resamplebyspacing"))
    {
        // input  : a Nifti image
        // output : resampled Nifti image

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f resamplebyspacing -i <input> -o <output> -p \"#x <sample_ratio_x> #y <sample_ratio_y> #z <sample_ratio_z>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_input==NULL || srx<0 || sry<0 || srz<0)
            {
                printf("\nERROR: invalid input!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_input).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".nii";
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_sampled.nii";
            }

        }

        /// load images

        QFileInfo curfile_info(qs_filename_input);
        if(curfile_info.isFile())
        {
            if (! ((curfile_info.suffix().toUpper()=="IMG") || (curfile_info.suffix().toUpper()=="HDR") || (curfile_info.suffix().toUpper()=="NII")) )
            {
                cout << "Error: your file are not recognized as Nifti images." << endl;
                return false;
            }
        }
        else
        {
            cout << "Please input Nifti images"<<endl;
            return false;
        }

        //
        V3DLONG sx, sy, sz, sc;
        int datatype, sdatatype;
        void *pData = 0;

        int szData=1;

        sc = 1;

        if(QFile::exists(qs_filename_input))
        {
            NiftiImageIO imageIO;

            if(!imageIO.canReadFile(const_cast<char *>(qs_filename_input.toStdString().c_str())))
            {
                cout<<"Fail to recognize image as Nifity file format"<<endl;
                return false;
            }

            if(!imageIO.load(pData))
            {
                cout<<"Fail to load image as Nifity file format"<<endl;
                return false;
            }

            sx = imageIO.getDimx();
            sy = imageIO.getDimy();
            sz = imageIO.getDimz();
            sc = imageIO.getDimc();

            datatype = imageIO.getDataType();

        }
        else
        {
            cout << "The image <"<<qs_filename_input.toStdString()<<"> does not exist."<<endl;
            return false;
        }

        //        if(datatype == UCHAR || datatype == CHAR) // 8-bit
        //        {
        //            szData = sizeof(unsigned char);
        //        }
        //        else if(datatype == USHORT || datatype == SHORT) // 16-bit
        //        {
        //            szData = sizeof(unsigned short);
        //        }
        //        else if(datatype == FLOAT || datatype == DOUBLE) // 32-bit
        //        {
        //            szData = sizeof(float);
        //        }
        //        else
        //        {
        //            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
        //            return false;
        //        }


        unsigned char *pResample = NULL;
        sdatatype = UCHAR;
        V3DLONG ssx, ssy, ssz;

        ssx = srx*sx; // srx>1 upsample srx<1 downsample
        ssy = sry*sy;
        ssz = srz*sz;

        V3DLONG pagesz_input = sx*sy*sz;
        V3DLONG pagesz = ssx*ssy*ssz;

        y_new<unsigned char, V3DLONG>(pResample, pagesz*sc);


        //
        REAL *pInput = NULL;
        V3DLONG totalplxs = pagesz_input*sc;
        y_new<REAL, V3DLONG>(pInput, totalplxs);

        REAL lv=0.0, rv=255.0;


        if(datatype == UCHAR)
        {
            unsigned char *p = (unsigned char *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];
        }
        else if(datatype == CHAR)
        {
            char *p = (char *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];

            rescaling<REAL, V3DLONG>(pInput, totalplxs, lv, rv);
        }
        else if(datatype == USHORT)
        {
            unsigned short *p = (unsigned short *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];

            rescaling<REAL, V3DLONG>(pInput, totalplxs, lv, rv);
        }
        else if(datatype == SHORT)
        {
            short *p = (short *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];

            rescaling<REAL, V3DLONG>(pInput, totalplxs, lv, rv);
        }
        else if(datatype == FLOAT)
        {
            float *p = (float *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];

            rescaling<REAL, V3DLONG>(pInput, totalplxs, lv, rv);
        }
        else if(datatype == DOUBLE)
        {
            double *p = (double *)pData;
            foreach(totalplxs, i) pInput[i] = p[i];

            rescaling<REAL, V3DLONG>(pInput, totalplxs, lv, rv);
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        // de-alloc
        y_del<void>(pData);


        //
        /// resampling

        V3DLONG szIn[4], szOut[4];
        szIn[0] = sx;
        szIn[1] = sy;
        szIn[2] = sz;
        szIn[3] = 1;

        szOut[0] = ssx;
        szOut[1] = ssy;
        szOut[2] = ssz;
        szOut[3] = 1;

        foreach(sc, c)
        {
            //
            unsigned char *poutput = pResample + c*pagesz;

            Y_IMG_UINT8 pOut;
            pOut.setImage(poutput, szOut, 4);


            //
            float *pinput = pInput + c*pagesz_input;

            Y_IMG_REAL pIn;
            pIn.setImage(pinput, szIn, 4);

            resize<V3DLONG, Y_IMG_REAL, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        // de-alloc
        y_del<REAL>(pInput);

        //
        /// save
        //
        NiftiImageIO imageIO;

        if(!imageIO.canWriteFile(const_cast<char *>(qs_filename_output.toStdString().c_str())))
        {
            cout<<"Fail to write an image"<<endl;
            return false;
        }

        imageIO.writeSampled((void *) (pResample), ssx, ssy, ssz, sc, 1, sdatatype, srx, sry, srz);

        // de-alloc
        y_del<unsigned char>(pResample);

        return true;
    }
    else if (func_name == tr("gaussianfilter"))
    {
        // input  : image
        // output : filtered image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f gaussianfilter -i <input> -o <output> -p \"#x <radius_x> #y <radius_y> #z <radius_z>\"  \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // parsing parameters
        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();
        QString qs_suffix_input=QFileInfo(qs_filename_input).suffix();

        if(outfile)
        {
            qs_filename_output=QString(outfile);
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"GaussianFiltered."+ qs_suffix_input;
        }

        //
        V3DLONG rx = 5, ry = 5, rz = 5;
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "x"))
                            {
                                rx = atol( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                ry = atol( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                rz = atol( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        //
        /// load images
        //

        V3DLONG sx, sy, sz, sc;
        int datatype;
        void *pData = 0;

        int datatype_out = UINT8;

        int szData=1;

        double dx=1.0, dy=1.0, dz=1.0;

        sc = 1; // only one channel image

        if(QFile::exists(qs_filename_input))
        {
            if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
            {
                unsigned char *p1dImg1 = (unsigned char *)pData;
                V3DLONG *sz_img1=NULL;

                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }

                pData=(void *)p1dImg1;

                if(datatype==UINT8)
                {
                    szData = 1;
                }
                else if(datatype==UINT16)
                {
                    szData = 2;
                }
                else
                {
                    szData = 4;
                }

                sx = sz_img1[0];
                sy = sz_img1[1];
                sz = sz_img1[2];
                sc = sz_img1[3];

                y_del<V3DLONG>(sz_img1);

            }
            else if(QFileInfo(qs_filename_input).suffix().toUpper().compare("NII") == 0)
            {
                datatype_out = UCHAR;

                NiftiImageIO imageIO;

                if(!imageIO.canReadFile(const_cast<char *>(qs_filename_input.toStdString().c_str())))
                {
                    cout<<"Fail to recognize image as Nifity file format"<<endl;
                    return false;
                }

                if(!imageIO.load(pData))
                {
                    cout<<"Fail to load image as Nifity file format"<<endl;
                    return false;
                }

                sx = imageIO.getDimx();
                sy = imageIO.getDimy();
                sz = imageIO.getDimz();
                sc = imageIO.getDimc();

                datatype = imageIO.getDataType();

                if(datatype == UCHAR || datatype == CHAR)
                {
                    szData = sizeof(unsigned char);
                }
                else if(datatype == USHORT || datatype == SHORT)
                {
                    szData = sizeof(unsigned short);
                }
                else if(datatype == FLOAT)
                {
                    szData = sizeof(float);
                }
                else
                {
                    printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                    return false;
                }

                dx = imageIO.getDx();
                dy = imageIO.getDy();
                dz = imageIO.getDz();
            }
            else
            {
                unsigned char *p1dImg1 = (unsigned char *)pData;
                V3DLONG *sz_img1=NULL;

                // libtif
                if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }

                pData=(void *)p1dImg1;

                if(datatype==UINT8)
                {
                    szData = 1;
                }
                else if(datatype==UINT16)
                {
                    szData = 2;
                }
                else
                {
                    szData = 4;
                }

                sx = sz_img1[0];
                sy = sz_img1[1];
                sz = sz_img1[2];
                sc = sz_img1[3];

                y_del<V3DLONG>(sz_img1);

            }
        }
        else
        {
            printf("\nERROR: the input does not exist!\n");
            return false;
        }

        /// guassian filtering

        V3DLONG pagesz = sx*sy*sz;
        V3DLONG szOut[4];
        szOut[0] = sx;
        szOut[1] = sy;
        szOut[2] = sz;
        szOut[3] = 1;

        V3DLONG w[3];
        w[0] = rx;
        w[1] = ry;
        w[2] = rz;

        REAL *p = NULL;
        y_new<REAL, V3DLONG>(p, pagesz);

        foreach(sc, c)
        {

            if(szData==1)
            {
                unsigned char *poutput = (unsigned char *)pData + c*pagesz;

                foreach(pagesz, i)
                    p[i] = poutput[i];

                guassianFiltering<REAL, V3DLONG>(p, szOut, w);
                rescaling<REAL, V3DLONG>(p,pagesz,0,255);

                foreach(pagesz, i)
                    poutput[i] = (unsigned char)(p[i]);
            }
            else if(szData==2)
            {
                unsigned short *poutput = (unsigned short *)pData + c*pagesz;

                foreach(pagesz, i)
                    p[i] = poutput[i];

                guassianFiltering<REAL, V3DLONG>(p, szOut, w);
                rescaling<REAL, V3DLONG>(p,pagesz,0,4096);

                foreach(pagesz, i)
                    poutput[i] = (unsigned short)(p[i]);
            }
            else if(szData==4)
            {
                float *poutput = (float *)pData + c*pagesz;

                foreach(pagesz, i)
                    p[i] = poutput[i];

                guassianFiltering<REAL, V3DLONG>(p, szOut, w);

                foreach(pagesz, i)
                    poutput[i] = (float)(p[i]);
            }
            else
            {
                cout<<"Invalid input datatype!"<<endl;
                return false;
            }
        }

        // de-alloc
        y_del<REAL>(p);

        //
        /// save
        //

        if(QFileInfo(qs_filename_output).suffix().toUpper().compare("NII") == 0)
        {
            NiftiImageIO imageIO;

            if(!imageIO.canWriteFile(const_cast<char *>(qs_filename_output.toStdString().c_str())))
            {
                cout<<"ERROR: saveImage failed!"<<endl;
                return false;
            }

            imageIO.writeSampled((void *) (pData), sx, sy, sz, sc, 1, datatype_out, dx, dy, dz);

        }
        else
        {
            szOut[3] = sc;
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pData),szOut,datatype_out))
            {
                cout<<"ERROR: saveImage failed!"<<endl;
                return false;
            }

        }

        // de-alloc
        y_del<void>(pData);

        //
        return true;
    }
    else if (func_name == tr("cropImage"))
    {
        // input  : Source image, Mask image
        // output : Cropped image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f cropImage -i <input> -o <output> -p \"#m <mask> #x <samplingratio_x> #y <samplingratio_y> #z <samplingratio_z>\" \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;
        QString qs_filename_mask = NULL; // maskimage or maskconfiguration

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "m"))
                            {
                                qs_filename_mask = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;
        QString qs_filename_crop_configure=NULL;

        // error check
        if(qs_filename_input==NULL || qs_filename_mask==NULL || srx<0 || sry<0 || srz<0)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            qs_filename_crop_configure = qs_pathname_output+"/"+qs_basename_output+"_cropconfigure.txt";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_cropped.v3draw";
            qs_filename_crop_configure = qs_pathname_input+"/"+qs_basename_input+"_cropconfigure.txt";
        }

        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // original image size
        V3DLONG osx, osy, osz;
        osx = sz_img1[0];
        osy = sz_img1[1];
        osz = sz_img1[2];

        // mask
        V3DLONG adjpixels=4;

        V3DLONG minx, miny, minz;
        V3DLONG maxx, maxy, maxz;

        if(QFileInfo(qs_filename_mask).suffix().toUpper().compare("TXT") == 0)
        {
            ifstream pFileLUT(qs_filename_mask.toStdString().c_str());
            string str;
            char letter;

            if(pFileLUT.is_open())
            {
                //
                pFileLUT >> letter;
                if(letter=='#')
                    getline(pFileLUT, str); // read comments line
                if(strcmp(str.c_str(), " IREG Crop Configuration File v1.0 "))
                {
                    cout << "Invalid file!";
                    pFileLUT.close();
                    return false;
                }
                else
                {
                    // file name
                    pFileLUT >> str;

                    //                    if(strcmp(QFileInfo(qs_filename_input).fileName().toStdString().c_str(),QFileInfo(str.c_str()).fileName().toStdString().c_str()))
                    //                    {
                    //                        cout << "Invalid image file!";
                    //                        pFileLUT.close();
                    //                        return false;
                    //                    }
                }

                if(pFileLUT.eof())
                {
                    cout << "Invalid file!";
                    pFileLUT.close();
                    return false;
                }
                else
                {
                    // configurations
                    pFileLUT >> minx >> maxx >> miny >> maxy >> minz >> maxz;
                }

            }
            else
            {
                cout << "Unable to open the file";
                pFileLUT.close();
                return false;
            }
            pFileLUT.close();
        }
        else
        {
            V3DLONG *sz_mask = 0;
            int datatype_mask = 0;
            unsigned char* p1dMask = 0;

            if(QFileInfo(qs_filename_mask).suffix().toUpper().compare("LSM") == 0)
            {
                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(qs_filename_mask.toStdString().c_str()), p1dMask, sz_mask, datatype_mask, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);
                    return false;
                }
            }
            else
            {
                // libtif
                if(!loadImage(const_cast<char *>(qs_filename_mask.toStdString().c_str()),p1dMask,sz_mask,datatype_mask))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);
                    return false;
                }
            }

            // find min max
            minx=sz_mask[0]; miny=sz_mask[1]; minz=sz_mask[2];
            maxx=0; maxy=0; maxz=0;

            foreach(sz_mask[2],k)
            {
                V3DLONG offset_k = k*sz_mask[0]*sz_mask[1];
                foreach(sz_mask[1],j)
                {
                    V3DLONG offset_j = offset_k + j*sz_mask[0];
                    foreach(sz_mask[0],i)
                    {
                        if(p1dMask[offset_j + i]>0)
                        {
                            if(i<minx)
                                minx = i;
                            if(i>maxx)
                                maxx = i;
                            if(j<miny)
                                miny = j;
                            if(j>maxy)
                                maxy = j;
                            if(k<minz)
                                minz = k;
                            if(k>maxz)
                                maxz = k;
                        }
                    }
                }
            }

            minx -= adjpixels;
            miny -= adjpixels;
            minz -= adjpixels;

            maxx += adjpixels;
            maxy += adjpixels;
            maxz += adjpixels;

            if(minx<0) minx = 0;
            if(miny<0) miny = 0;
            if(minz<0) minz = 0;

            if(maxx>=sz_mask[0]) maxx = sz_mask[0] - 1;
            if(maxy>=sz_mask[1]) maxy = sz_mask[1] - 1;
            if(maxz>=sz_mask[2]) maxz = sz_mask[2] - 1;

            //de-alloc
            y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);

        }

        minx *= srx;
        miny *= sry;
        minz *= srz;

        maxx *= srx;
        maxy *= sry;
        maxz *= srz;

        if(minx<0) minx = 0;
        if(miny<0) miny = 0;
        if(minz<0) minz = 0;

        if(maxx>=sz_img1[0]) maxx = sz_img1[0] - 1;
        if(maxy>=sz_img1[1]) maxy = sz_img1[1] - 1;
        if(maxz>=sz_img1[2]) maxz = sz_img1[2] - 1;

        //
        /// cropping
        //

        V3DLONG sx, sy, sz, sc;
        sx = maxx - minx + 1;
        sy = maxy - miny + 1;
        sz = maxz - minz + 1;
        sc = sz_img1[3];

        if(sx>osx || sy>osy || sz>osz)
        {
            cout<<"Invalid crop configuration!"<<endl;
            return false;
        }

        //
        V3DLONG pagesz = sx*sy*sz;
        V3DLONG totalpxls = pagesz*sc;
        unsigned char * pOut = NULL;
        y_new<unsigned char, V3DLONG>(pOut, totalpxls*datatype_img1);

        //
        foreach(sc, c)
        {
            V3DLONG offset_c = c*pagesz;
            V3DLONG offset_c_input = c*sz_img1[0]*sz_img1[1]*sz_img1[2];
            foreach(sz, k)
            {
                V3DLONG offset_k = offset_c + k*sx*sy;
                V3DLONG offset_k_input = offset_c_input + (k+minz)*sz_img1[0]*sz_img1[1];
                foreach(sy, j)
                {
                    V3DLONG offset_j = offset_k + j*sx;
                    V3DLONG offset_j_input = offset_k_input + (j+miny)*sz_img1[0];
                    foreach(sx, i)
                    {
                        if(datatype_img1==UINT8)
                        {
                            pOut[offset_j + i] = p1dImg1[offset_j_input + i + minx];
                        }
                        else if(datatype_img1==UINT16)
                        {
                            unsigned short *p = (unsigned short *)pOut;

                            p[offset_j + i] = ((unsigned short *)p1dImg1)[offset_j_input + i + minx];
                        }
                        else if(datatype_img1==FLOAT32)
                        {
                            float *p = (float *)pOut;

                            p[offset_j + i] = ((float *)p1dImg1)[offset_j_input + i + minx];
                        }
                        else
                        {
                            cout<<"Unsupported datatype!"<<endl;
                            return false;
                        }

                    }
                }
            }
        }

        // reuse sz_img1
        sz_img1[0] = sx;
        sz_img1[1] = sy;
        sz_img1[2] = sz;

        //
        /// save
        //

        // save crop configuration file
        FILE *pFileLUT=0;
        pFileLUT = fopen(qs_filename_crop_configure.toStdString().c_str(),"wt");

        fprintf(pFileLUT, "# IREG Crop Configuration File v1.0 \n"); //
        fprintf(pFileLUT, "%s \n\n", qs_filename_input.toStdString().c_str());
        fprintf(pFileLUT, "%ld %ld %ld %ld %ld %ld \n", minx, maxx, miny, maxy, minz, maxz);
        fprintf(pFileLUT, "%ld %ld %ld \n\n", osx, osy, osz);
        fclose(pFileLUT);

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img1,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(pOut);

        //
        return true;
    }
    else if (func_name == tr("convertTC2AM"))
    {
        // input  : stitching tile configuration .tc file
        // output : Insight Transform .txt file

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f convertTC2AM -i <.tc> -o <.txt> -p \"#x <ratio_x> #y <ratio_y> #z <ratio-z> \" \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL || srx<0 || sry<0 || srz<0)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".txt";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".txt";
        }

        //
        /// load tc file
        //

        ifstream pFileLUT(qs_filename_input.toStdString().c_str());

        string str, fn_str;
        char letter;
        char buf[2048];

        REAL tx, ty, tz; // translations
        REAL cx, cy, cz; // centers

        if(pFileLUT.is_open())
        {
            //
            pFileLUT >> letter;
            if(letter=='#')
                getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT1))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // thumbnail file name
            }
            do
            {
                pFileLUT >> letter;
            }
            while(letter!='#' && !pFileLUT.eof());

            if(pFileLUT.eof()) return errorRead(&pFileLUT);
            getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT2))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // tiles
            }
            do
            {
                pFileLUT >> letter;
            }
            while(letter!='#' && !pFileLUT.eof());

            if(pFileLUT.eof()) return errorRead(&pFileLUT);
            getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT3))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // dimensions
            }
            do
            {
                pFileLUT >> letter;
            }
            while(letter!='#' && !pFileLUT.eof());

            if(pFileLUT.eof()) return errorRead(&pFileLUT);
            getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT4))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // origins
            }
            do
            {
                pFileLUT >> letter;
            }
            while(letter!='#' && !pFileLUT.eof());

            if(pFileLUT.eof()) return errorRead(&pFileLUT);
            getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT5))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // resolutions
            }
            do
            {
                pFileLUT >> letter;
            }
            while(letter!='#' && !pFileLUT.eof());

            if(pFileLUT.eof()) return errorRead(&pFileLUT);
            getline(pFileLUT, str); // read comments line
            if(strcmp(str.c_str(), TC_COMMENT6))
            {
                return errorRead(&pFileLUT);
            }
            else
            {
                // lut
                // first image end positions
                getline(pFileLUT, str);

                istringstream iss(str);
                iss >> fn_str;
                int tmpval;

                //
                iss >> buf; iss >> tmpval;
                iss >> buf; iss >> tmpval;
                iss >> buf; iss >> tmpval;

                iss >> buf;

                iss >> buf; iss >> cx;
                iss >> buf; iss >> cy;
                iss >> buf; iss >> cz;
            }
            if(!pFileLUT.eof()) // compatible with old .tc file
            {
                do
                {
                    pFileLUT >> letter;
                }
                while(letter!='#' && !pFileLUT.eof());

                if(!pFileLUT.eof())
                {
                    getline(pFileLUT, str); // read comments line

                    if(strcmp(str.c_str(), TC_COMMENT7) == 0)
                    {
                        while( !pFileLUT.eof() && getline(pFileLUT, str) )
                        {
                            istringstream iss(str);

                            if(iss.fail() || iss.eof())
                                continue;

                            int current, predecessor;
                            iss >> current;
                            iss >> predecessor;

                            iss >> tx;
                            iss >> ty;
                            iss >> tz;
                        }
                    }
                }
                else
                {
                    cout<<"Warning: This stitching configuration file is old version without MST info."<<endl;
                }
            }
        }
        else
        {
            cout << "Unable to open the file";
            pFileLUT.close();
            return false;
        }
        pFileLUT.close();

        //
        /// save as insight transform file
        //

        Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
        afftrans.initTransform(3);

        // translations
        afftrans.pTransform[3]  = tx*srx;
        afftrans.pTransform[7]  = ty*sry;
        afftrans.pTransform[11] = tz*srz;

        afftrans.pTransform[12] = cx/2.0*srx;
        afftrans.pTransform[13] = cy/2.0*sry;
        afftrans.pTransform[14] = cz/2.0*srz;

        // save
        if(qs_filename_output!=NULL)
        {
            if(!afftrans.write(qPrintable(qs_filename_output)))
            {
                printf("ERROR: saving affine matrix failed!\n");
                return false;
            }
        }
        afftrans.clean();

        //
        return true;
    }
    else if (func_name == tr("zeropadding"))
    {
        // input  : Source image, Mask image
        // output : Cropped image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f zeropadding -i <input> -o <output> -p \"#c <configure.txt> #x <samplingratio_x> #y <samplingratio_y> #z <samplingratio_z>\" \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;
        QString qs_filename_crop_configure = NULL; // maskimage or maskconfiguration

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "c"))
                            {
                                qs_filename_crop_configure = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL || qs_filename_crop_configure==NULL || srx<0 || sry<0 || srz<0)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_padding.v3draw";
        }

        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // mask
        V3DLONG adjpixels=4;

        V3DLONG minx, miny, minz;
        V3DLONG maxx, maxy, maxz;

        V3DLONG sx, sy, sz;
        V3DLONG sc = sz_img1[3];

        if(QFileInfo(qs_filename_crop_configure).suffix().toUpper().compare("TXT") == 0)
        {
            ifstream pFileLUT(qs_filename_crop_configure.toStdString().c_str());
            string str;
            char letter;

            if(pFileLUT.is_open())
            {
                //
                pFileLUT >> letter;
                if(letter=='#')
                    getline(pFileLUT, str); // read comments line
                if(strcmp(str.c_str(), " IREG Crop Configuration File v1.0 "))
                {
                    cout << "Invalid file!";
                    pFileLUT.close();
                    return false;
                }
                else
                {
                    // file name
                    pFileLUT >> str;

                    //                    if(strcmp(QFileInfo(qs_filename_input).fileName().toStdString().c_str(),QFileInfo(str.c_str()).fileName().toStdString().c_str()))
                    //                    {
                    //                        cout << "Invalid image file!";
                    //                        pFileLUT.close();
                    //                        return false;
                    //                    }
                }

                if(pFileLUT.eof())
                {
                    cout << "Invalid file!";
                    pFileLUT.close();
                    return false;
                }
                else
                {
                    // configurations
                    pFileLUT >> minx >> maxx >> miny >> maxy >> minz >> maxz;
                    pFileLUT >> sx >> sy >> sz;
                }

            }
            else
            {
                cout << "Unable to open the file";
                pFileLUT.close();
                return false;
            }
            pFileLUT.close();
        }
        else
        {
            cout<<"Invalid configuration file!"<<endl;
            return false;
        }

        minx *= srx;
        miny *= sry;
        minz *= srz;

        maxx *= srx;
        maxy *= sry;
        maxz *= srz;

        sx *= srx;
        sy *= sry;
        sz *= srz;

        if(sx<0 || sy <0 || sz <0)
        {
            cout<<"Invalid Inputs"<<endl;
            return false;
        }

        if(minx<0) minx = 0;
        if(miny<0) miny = 0;
        if(minz<0) minz = 0;

        if(maxx>=sz_img1[0]) maxx = sz_img1[0] - 1;
        if(maxy>=sz_img1[1]) maxy = sz_img1[1] - 1;
        if(maxz>=sz_img1[2]) maxz = sz_img1[2] - 1;

        //
        /// padding
        //

        V3DLONG csx, csy, csz;
        csx = sz_img1[0];
        csy = sz_img1[1];
        csz = sz_img1[2];

        if(sx<csx || sy<csy || sz<csz)
        {
            cout<<"Invalid configuration"<<endl;
            return false;
        }

        //
        V3DLONG pagesz = sx*sy*sz;
        V3DLONG totalpxls = pagesz*sc;
        unsigned char * pOut = NULL;

        y_new<unsigned char, V3DLONG>(pOut, totalpxls*datatype_img1);
        memset(pOut, 0, totalpxls*datatype_img1);

        if(datatype_img1==V3D_UINT8)
        {
            //
            foreach(sc, c)
            {
                V3DLONG offset_c = c*pagesz;
                V3DLONG offset_c_input = c*sz_img1[0]*sz_img1[1]*sz_img1[2];
                foreach(csz, k)
                {
                    V3DLONG offset_k = offset_c + (k+minz)*sx*sy;
                    V3DLONG offset_k_input = offset_c_input + k*csx*csy;
                    foreach(csy, j)
                    {
                        V3DLONG offset_j = offset_k + (j+miny)*sx;
                        V3DLONG offset_j_input = offset_k_input + j*csx;
                        foreach(csx, i)
                        {
                            pOut[offset_j + i + minx] = p1dImg1[offset_j_input + i];
                        }
                    }
                }
            }
        }
        else if(datatype_img1==V3D_UINT16)
        {
            unsigned short *poutput = (unsigned short *)pOut;
            unsigned short *pinput = (unsigned short *)p1dImg1;

            //
            foreach(sc, c)
            {
                V3DLONG offset_c = c*pagesz;
                V3DLONG offset_c_input = c*sz_img1[0]*sz_img1[1]*sz_img1[2];
                foreach(csz, k)
                {
                    V3DLONG offset_k = offset_c + (k+minz)*sx*sy;
                    V3DLONG offset_k_input = offset_c_input + k*csx*csy;
                    foreach(csy, j)
                    {
                        V3DLONG offset_j = offset_k + (j+miny)*sx;
                        V3DLONG offset_j_input = offset_k_input + j*csx;
                        foreach(csx, i)
                        {
                            poutput[offset_j + i + minx] = pinput[offset_j_input + i];
                        }
                    }
                }
            }
        }
        else if(datatype_img1==V3D_FLOAT32)
        {
            float *poutput = (float *)pOut;
            float *pinput = (float *)p1dImg1;

            //
            foreach(sc, c)
            {
                V3DLONG offset_c = c*pagesz;
                V3DLONG offset_c_input = c*sz_img1[0]*sz_img1[1]*sz_img1[2];
                foreach(csz, k)
                {
                    V3DLONG offset_k = offset_c + (k+minz)*sx*sy;
                    V3DLONG offset_k_input = offset_c_input + k*csx*csy;
                    foreach(csy, j)
                    {
                        V3DLONG offset_j = offset_k + (j+miny)*sx;
                        V3DLONG offset_j_input = offset_k_input + j*csx;
                        foreach(csx, i)
                        {
                            poutput[offset_j + i + minx] = pinput[offset_j_input + i];
                        }
                    }
                }
            }
        }
        else
        {
            cout<<"Datatype is not supported"<<endl;
            return false;
        }

        // reuse sz_img1
        sz_img1[0] = sx;
        sz_img1[1] = sy;
        sz_img1[2] = sz;

        //
        /// save
        //

        // save the output image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img1,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(pOut);

        //
        return true;
    }
    else if (func_name == tr("rigidreg"))
    {
        // input  : Source image, Template image
        // output : transform matrix, aligned result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint_rigidreg();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_affinematrix;
        QString qs_filename_deformed;

        bool b_save_affine = true;
        bool b_save_warped = true;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint_rigidreg();
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint_rigidreg();
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_affinematrix=qs_pathname_output+"/"+qs_basename_output+"Affine.txt";
                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+"Deformed.v3draw";
            }
            else
            {
                qs_filename_affinematrix=qs_pathname_input+"/"+qs_basename_input+"Affine.txt";
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"Deformed.v3draw";
            }

        }
        else
        {
            printf("\nERROR: invalid inputs!\n");
            errorPrint_rigidreg();
            return false;
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            errorPrint_rigidreg();

            return false;
        }

        if(sz_img_sub_input[0]!=sz_img_tar_input[0] || sz_img_sub_input[1]!=sz_img_tar_input[1] ||sz_img_sub_input[2]!=sz_img_tar_input[2])
        {
            qDebug()<<"Your target and subject must have the same size"<<endl;
            return false;
        }

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG offsets_sub = channel_ref_sub*sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_tar = channel_ref_tar*sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        // downsampling
        unsigned char *pSubDS = NULL, *pTarDS = NULL;
        V3DLONG *szResample=NULL;

        REAL srx = 0.25;
        REAL sry = 0.25;
        REAL srz = 0.25;

        y_new<V3DLONG, V3DLONG>(szResample, 4);
        szResample[0] = srx*sz_img_sub_input[0]; // srx>1 upsample srx<1 downsample
        szResample[1] = sry*sz_img_sub_input[1];
        szResample[2] = srz*sz_img_sub_input[2];
        szResample[3] = 1;

        V3DLONG pagesz = szResample[0]*szResample[1]*szResample[2];

        //
        y_new<unsigned char, V3DLONG>(pSubDS, pagesz);
        y_new<unsigned char, V3DLONG>(pTarDS, pagesz);

        //
        if(datatype_tar_input!=UINT8)
        {
            qDebug()<<"Only support UINT8!";
            return false;
        }
        else
        {
            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_tar, sz_img_tar_input, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(pTarDS, szResample, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        if(datatype_sub_input!=UINT8)
        {
            qDebug()<<"Only support UINT8!";
            return false;
        }
        else
        {
            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_sub, sz_img_sub_input, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(pSubDS, szResample, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        // rigid registration
        Y_MAT<REAL, V3DLONG> M; // 3D A4x4
        M.init(4,4,0);

        REAL theta = 45;
        REAL thetastep = 15;
        REAL translate = 5;
        REAL translatestep = 1;

        // scale 1:

        rigidregbf<unsigned char, V3DLONG, Y_MAT<REAL, V3DLONG> >(M, pSubDS, pTarDS, szResample[0], szResample[1], szResample[2], theta, thetastep, translate, translatestep);


        // save affine matrix
        if(b_save_affine)
        {
            // affine
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.setTransform(M.p, 3);

            if(qs_filename_affinematrix!=NULL)
            {
                if(!afftrans.write(qPrintable(qs_filename_affinematrix)))
                {
                    printf("ERROR: saving affine matrix failed!\n");
                    return false;
                }
            }
            afftrans.clean();
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
        y_del<V3DLONG>(szResample);
        y_del<unsigned char>(pSubDS);
        y_del<unsigned char>(pTarDS);
        M.clean();

        //
        return true;
    }
    else if (func_name == tr("rotEstimate"))
    {
        // input  : Source image, Template image
        // output : transform matrix, aligned result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<"./vaa3d -x ireg -f rotEstimate ..."<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_affinematrix;
        QString qs_filename_deformed;

        bool b_save_affine = true;
        bool b_save_warped = true;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint_rigidreg();
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint_rigidreg();
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_affinematrix=qs_pathname_output+"/"+qs_basename_output+"Affine.txt";
                qs_filename_deformed=qs_pathname_output+"/"+qs_basename_output+"Deformed.v3draw";
            }
            else
            {
                qs_filename_affinematrix=qs_pathname_input+"/"+qs_basename_input+"Affine.txt";
                qs_filename_deformed=qs_pathname_input+"/"+qs_basename_input+"Deformed.v3draw";
            }

        }
        else
        {
            printf("\nERROR: invalid inputs!\n");
            return false;
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            return false;
        }

        if(sz_img_sub_input[0]!=sz_img_tar_input[0] || sz_img_sub_input[1]!=sz_img_tar_input[1] ||sz_img_sub_input[2]!=sz_img_tar_input[2])
        {
            qDebug()<<"Your target and subject must have the same size"<<endl;
            return false;
        }

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG offsets_sub = channel_ref_sub*sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_tar = channel_ref_tar*sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        // downsampling
        unsigned char *pSubDS = NULL, *pTarDS = NULL;
        V3DLONG *szResample=NULL;

        REAL srx = 0.25;
        REAL sry = 0.25;
        REAL srz = 0.25;

        y_new<V3DLONG, V3DLONG>(szResample, 4);
        szResample[0] = srx*sz_img_sub_input[0]; // srx>1 upsample srx<1 downsample
        szResample[1] = sry*sz_img_sub_input[1];
        szResample[2] = srz*sz_img_sub_input[2];
        szResample[3] = 1;

        V3DLONG pagesz = szResample[0]*szResample[1]*szResample[2];

        //
        y_new<unsigned char, V3DLONG>(pSubDS, pagesz);
        y_new<unsigned char, V3DLONG>(pTarDS, pagesz);

        //
        if(datatype_tar_input!=UINT8)
        {
            qDebug()<<"Only support UINT8!";
            return false;
        }
        else
        {
            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_tar, sz_img_tar_input, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(pTarDS, szResample, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        if(datatype_sub_input!=UINT8)
        {
            qDebug()<<"Only support UINT8!";
            return false;
        }
        else
        {
            //
            Y_IMG_UINT8 pIn;
            pIn.setImage(p_sub, sz_img_sub_input, 4);

            Y_IMG_UINT8 pOut;
            pOut.setImage(pSubDS, szResample, 4);

            resize<V3DLONG, Y_IMG_UINT8, Y_IMG_UINT8>(pOut, pIn, SAMPLE);
        }

        // rigid registration
        Y_MAT<REAL, V3DLONG> M; // 3D A4x4
        M.init(4,4,0);

        //
        rotregbf<unsigned char, V3DLONG, Y_MAT<REAL, V3DLONG> >(M, pSubDS, pTarDS, szResample[0], szResample[1], szResample[2], 15, 15, 180, 5, 1, 1);

        // save affine matrix
        if(b_save_affine)
        {
            // affine
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.setTransform(M.p, 3);

            if(qs_filename_affinematrix!=NULL)
            {
                if(!afftrans.write(qPrintable(qs_filename_affinematrix)))
                {
                    printf("ERROR: saving affine matrix failed!\n");
                    return false;
                }
            }
            afftrans.clean();
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
        y_del<V3DLONG>(szResample);
        y_del<unsigned char>(pSubDS);
        y_del<unsigned char>(pTarDS);
        M.clean();

        //
        return true;
    }
    else if (func_name == tr("convertSigned16bit2Unsigned16bit"))
    {
        // input  : signed 16-bit image
        // output : unsigned 16-bit image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f convertSigned16bit2unsigned16bit -i <input> -o <output>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_input(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") == 0
                 || QFileInfo(qs_filename_output).suffix().toUpper().compare("V3DRAW") == 0))
            {
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }

        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }


        //
        /// load images
        //
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1d_img1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1d_img1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1d_img1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }

        //
        /// convert short to unsigned short data
        //
        short *p_input=NULL;
        unsigned short *p_out=NULL;
        V3DLONG sc = sz_img1[3];
        V3DLONG totalplxs_sub = sz_img1[0]*sz_img1[1]*sz_img1[2]*sc;

        int datatype_out = 2;

        p_input = (short *)p1d_img1;
        y_new<unsigned short, V3DLONG>(p_out, totalplxs_sub);

        short max_v, min_v;
        imaxmin<short, V3DLONG>(p_input, totalplxs_sub, max_v, min_v);
        unsigned short irange = max_v;
        irange -= min_v;

        foreach(totalplxs_sub, i)
        {
            p_out[i]=irange*((REAL)(p_input[i]) - min_v)/(REAL)irange;
        }

        //
        /// save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_out),sz_img1,2))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1d_img1);
        y_del<V3DLONG>(sz_img1);
        y_del<unsigned short>(p_out);

        return true;
    }
    else if (func_name == tr("icut"))
    {
        // input  : an image
        // output : a blank boundaries cut off image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f icut -i <input> -o <output> -p \"#t <threshold> #f <front_z> #b <back_z> #u <up_y> #d <down_y> #l <left_x> #r <right_x>\"\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_input(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        REAL thresholdcut = 0;
        V3DLONG left_x=0, right_x=0;
        V3DLONG up_y=0, down_y=0;
        V3DLONG front_z=0, back_z=0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                thresholdcut = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "f"))
                            {
                                front_z = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "b"))
                            {
                                back_z = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "u"))
                            {
                                up_y = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "d"))
                            {
                                down_y = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                left_x = atol( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "r"))
                            {
                                right_x = atol( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") == 0
                 || QFileInfo(qs_filename_output).suffix().toUpper().compare("V3DRAW") == 0))
            {
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
            }

        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        //
        /// load images
        //
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1d_img1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1d_img1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1d_img1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }

        //
        /// resize by cutting off blank planes
        //
        V3DLONG *szOut=NULL;
        y_new<V3DLONG, V3DLONG>(szOut,4);
        unsigned char *pOut = NULL;

        if(datatype_img1==UINT8)
        {
            imgcutting<unsigned char, V3DLONG>(pOut, szOut, p1d_img1, sz_img1, thresholdcut, left_x, right_x, up_y, down_y, front_z, back_z);
        }
        else if(datatype_img1==UINT16)
        {
            unsigned short *p_input = (unsigned short *)p1d_img1;
            unsigned short *p = (unsigned short *)pOut;
            imgcutting<unsigned short, V3DLONG>(p, szOut, p_input, sz_img1, thresholdcut, left_x, right_x, up_y, down_y, front_z, back_z);
            pOut=(unsigned char *)p;
        }
        else
        {
            cout<<"Unsupported data type"<<endl;
            return false;
        }

        //
        /// save
        //
        if(pOut)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),szOut,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        // de-alloc
        y_del<unsigned char>(p1d_img1);
        y_del<V3DLONG>(sz_img1);
        y_del<unsigned char>(pOut);
        y_del<V3DLONG>(szOut);

        return true;
    }
    else if (func_name == tr("meandeviation"))
    {
        // input  : an image
        // output : mean value and stdandard deviation

        if(input.size()<1 ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f meandeviation -i <input> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_input(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input file!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TXT") == 0))
            {
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".txt";
            }

        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".txt";
        }

        //
        /// load images
        //
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1d_img1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1d_img1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1d_img1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1d_img1, sz_img1);
                return false;
            }
        }

        //
        /// compute mean and deviation
        //
        double sum=0;
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];

        if(datatype_img1==1)
        {
            foreach(pagesz, i) sum += p1d_img1[i];
            sum /= pagesz;
        }
        else if(datatype_img1==2)
        {
            unsigned short *p = (unsigned short *)p1d_img1;
            foreach(pagesz, i) sum += p[i];
            sum /= pagesz;
        }
        else
        {

        }

        qDebug()<< "mean value: "<<sum;
        qDebug()<< "file: " <<qs_filename_input;

        //
        /// save
        //
        std::ofstream out(qPrintable(qs_filename_output), std::ios_base::out | std::ios_base::app);

        //
        if(out.is_open())
        {
            out.seekp (0, ios::beg);

            out << "mean: " <<sum<< std::endl;
            out << "file: " <<qs_filename_input.toStdString().c_str()<< std::endl;

            //
            out.close();
        }
        else
        {
            cout<<"Unable to write the output file"<<endl;
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1d_img1);
        y_del<V3DLONG>(sz_img1);

        return true;
    }
    else if (func_name == tr("esimilarity"))
    {
        // input  : Source image, Template image, mask image
        // output : ncc score in text file

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<"vaa3d -x ireg -f esimilarity -o <score.txt> -p \"#s <aligned.tif> #cs <aligned_refchn> #t <template.tif> #ct <template_refchn> #m <mask.tif>\""<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar, qs_filename_mask=NULL;

        // outputs
        QString qs_filename_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                qs_filename_mask = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_QAscore.txt";
            }

        }
        else
        {
            printf("\nERROR: invalid inputs!\n");
            return false;
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }

        if(sz_img_sub_input[0]!=sz_img_tar_input[0] || sz_img_sub_input[1]!=sz_img_tar_input[1] ||sz_img_sub_input[2]!=sz_img_tar_input[2])
        {
            qDebug()<<"Your target and subject must have the same size"<<endl;
            return false;
        }

        if(datatype_sub_input!=datatype_tar_input)
        {
            qDebug()<<"Your target and subject must be 8-bit data"<<endl;
            return false;
        }

        V3DLONG *sz_mask=0;
        unsigned char *p_img_mask=0;
        int datatype_mask=0;

        if(qs_filename_mask!=NULL && QFile(qs_filename_mask).exists())
        {
            if(!loadImage((char *)qPrintable(qs_filename_mask),p_img_mask,sz_mask,datatype_mask))
            {
                printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_mask));
                y_del2<unsigned char, V3DLONG>(p_img_mask, sz_mask);
                return false;
            }
        }

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG pagesz = sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_sub = channel_ref_sub*pagesz;
        V3DLONG offsets_tar = channel_ref_tar*pagesz;

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        // evaluating
        double qa_ncc = -1;

        if(qs_filename_mask!=NULL && QFile(qs_filename_mask).exists())
        {
            qa_ncc = nccwmask<unsigned char, V3DLONG>(p_sub, p_tar, p_img_mask, pagesz);
        }
        else
        {
            qa_ncc = ncc<unsigned char, V3DLONG>(p_sub, p_tar, pagesz);
        }


        // save
        std::ofstream out(qPrintable(qs_filename_output), std::ios_base::out | std::ios_base::app);

        //
        if(out.is_open())
        {
            out.seekp (0, ios::beg);

            out << "# Quality of the Alignment (NCC)" << std::endl;
            out << qa_ncc << std::endl;

            //
            out.close();
        }
        else
        {
            cout<<"Unable to write the output file"<<endl;
            return false;
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        //
        return true;
    }
    else if (func_name == tr("evalAlignQuality"))
    {
        // input  : Source image, Template image
        // output : transform matrix, aligned result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            errorPrint_evalAlignQuality();
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                errorPrint_rigidreg();
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Assume R(1)G(2)B(3) ...!\n");
                errorPrint_rigidreg();
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_QAscore.txt";
            }

        }
        else
        {
            printf("\nERROR: invalid inputs!\n");
            errorPrint_evalAlignQuality();
            return false;
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            errorPrint_evalAlignQuality();

            return false;
        }

        if(sz_img_sub_input[0]!=sz_img_tar_input[0] || sz_img_sub_input[1]!=sz_img_tar_input[1] ||sz_img_sub_input[2]!=sz_img_tar_input[2])
        {
            qDebug()<<"Your target and subject must have the same size"<<endl;
            return false;
        }

        if(datatype_sub_input!=datatype_tar_input)
        {
            qDebug()<<"Your target and subject must be 8-bit data"<<endl;
            return false;
        }

        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG pagesz = sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_sub = channel_ref_sub*pagesz;
        V3DLONG offsets_tar = channel_ref_tar*pagesz;

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        // evaluating
        double qa_nmi = nmi<unsigned char, V3DLONG>(p_sub, p_tar, pagesz);
        double qa_ncc = ncc<unsigned char, V3DLONG>(p_sub, p_tar, pagesz);

        //double qa = 0.25*qa_ncc + 0.75*qa_nmi;
        //qDebug()<<"QA: nmi ... ncc ... qa ... "<<qa_nmi<<qa_ncc<<qa;

        // save
        std::ofstream out(qPrintable(qs_filename_output), std::ios_base::out | std::ios_base::app);

        //
        if(out.is_open())
        {
            out.seekp (0, ios::beg);

            out << "# Quality of the Alignment (NMI, NCC)" << std::endl;
            out << qa_nmi << ", "<< qa_ncc << std::endl;

            //
            out.close();
        }
        else
        {
            cout<<"Unable to write the output file"<<endl;
            return false;
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        //
        return true;
    }
    else if (func_name == tr("extractRotMat"))
    {
        // input  : Rigid Transformation Matrix
        // output : Rotation Transformation Matrix

        if(input.size()<1) // no inputs
        {
            //print Help info
            printf("\nUsage: v3d -x ireg -f extractRotMat -i <input_rigid_transformation> -o <output_rotation_transformation>\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) outfile = outfilelist->at(0);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_affinematrix(infile);

        // outputs
        QString qs_filename_rotationmatrix;

        // error check
        if(qs_filename_affinematrix==NULL)
        {
            printf("\nERROR: invalid input file name!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_affinematrix).baseName();
        QString qs_pathname_input=QFileInfo(qs_basename_input).path();

        if(outfile)
        {
            qs_filename_rotationmatrix=QString(outfile);
            QString qs_pathname_output=QFileInfo(qs_filename_rotationmatrix).path();
            QString qs_basename_output=QFileInfo(qs_filename_rotationmatrix).baseName();

            qs_filename_rotationmatrix=qs_pathname_output+"/"+qs_basename_output+".txt";
        }
        else
        {
            qs_filename_rotationmatrix=qs_pathname_input+"/"+qs_basename_input+".txt";
        }

        // load affine matrix and extract rotations
        if(qs_filename_affinematrix!=NULL && QFile(qs_filename_affinematrix).exists())
        {
            Y_AFFINETRANSFORM<REAL, V3DLONG> afftrans;
            afftrans.initTransform(3); // 3D

            if(!afftrans.read(qPrintable(qs_filename_affinematrix)))
            {
                printf("ERROR: loading affine matrix failed!\n");
                return false;
            }

            // save rotation matrix
            Y_AFFINETRANSFORM<REAL, V3DLONG> rotations;
            rotations.setTransform(afftrans.pTransform, 3);

            rotations.keepRotations();

            Y_MAT<REAL,V3DLONG> mat(rotations.pTransform,4,4);
            mat.transpose();

            rotations.rotationMatrix2EulerAngles();

            // only xy-plane rotation / rotation along z-axis
            rotations.eulerAngles2RotationMatrix(0.0, 0.0, rotations.phi);

            //
            if(qs_filename_rotationmatrix!=NULL)
            {
                if(!rotations.write(qPrintable(qs_filename_rotationmatrix)))
                {
                    printf("ERROR: saving rotation matrix failed!\n");
                    return false;
                }
            }
            else
            {
                printf("ERROR: rotation matrix name is invalid!\n");
                return false;
            }

            //
            afftrans.clean();
            rotations.clean();
        }

        //
        return true;
    }
    else if (func_name == tr("recoverRef"))
    {
        // input  : source image (at least 2 channels)
        // output : recovered image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f recoverRef -i <input> -o <output> -p \"#r <ref_channel>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        V3DLONG refn = 0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                refn = atol( argv[i+1] ) - 1; //red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"recovered.v3draw";
        }


        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        if(sz_img1[3]<2)
        {
            cout<<"At least 2 color channel needed"<<endl;
            return false;
        }

        if(refn<0 || refn>sz_img1[3]-1)
        {
            cout<<"Invalid reference channel specified"<<endl;
            return false;
        }

        /// recover
        //
        // model: x = aX+(1-b)Y; y = (1-a)X+bY;
        // a = x1/(x1+y1);
        // b = y2/(x2+y2);
        //
        V3DLONG pagesz = sz_img1[0]*sz_img1[1]*sz_img1[2];
        V3DLONG totalsz = pagesz*sz_img1[3];

        REAL a,b;
        V3DLONG idx;
        unsigned char *pOut = NULL;

        V3DLONG offs = (sz_img1[3]-1-refn)*pagesz;
        V3DLONG offr = refn*pagesz;

        if(datatype_img1==1)
        {
            unsigned char *pX = p1dImg1 + offr;
            unsigned char *pY = p1dImg1 + offs;

            unsigned char x1, y1, x2, y2;

            getMaximum<unsigned char, V3DLONG>(pX, pagesz, x1, idx);
            y1 = pY[idx];

            getMaximum<unsigned char, V3DLONG>(pY, pagesz, y2, idx);
            x2 = pX[idx];

            a = (REAL)x1/((REAL)x1+(REAL)y1);
            b = (REAL)y2/((REAL)x2+(REAL)y2);

            y_new<unsigned char, V3DLONG>(pOut, totalsz);

            REAL c = 1-b;
            REAL d = a+b-1;

            for(V3DLONG i=0; i<pagesz; i++)
            {
                pOut[i+offs] = pY[i];
                pOut[i+offr] = (unsigned char)((b*(REAL)(pX[i]) - c*(REAL)(pY[i]))/d);
            }

        }
        else if(datatype_img1==2)
        {
            unsigned short *pX = (unsigned short *)p1dImg1 + offr;
            unsigned short *pY = (unsigned short *)p1dImg1 + offs;

            unsigned short x1, y1, x2, y2;

            getMaximum<unsigned short, V3DLONG>(pX, pagesz, x1, idx);
            y1 = pY[idx];

            getMaximum<unsigned short, V3DLONG>(pY, pagesz, y2, idx);
            x2 = pX[idx];

            a = (REAL)x1/((REAL)x1+(REAL)y1);
            b = (REAL)y2/((REAL)x2+(REAL)y2);

            y_new<unsigned char, V3DLONG>(pOut, totalsz*sizeof(unsigned short));

            unsigned short *pOutput = (unsigned short *)pOut;

            REAL c = 1-b;
            REAL d = a+b-1;

            cout<<" "<<x1<<" "<<y1<<" "<<x2<<" "<<y2<<endl;
            cout<<"test ... "<<a<<" "<<b<<" "<<c<<" "<<d<<endl;

            for(V3DLONG i=0; i<pagesz; i++)
            {
                pOutput[i+offs] = pY[i];

                REAL val = (b*(REAL)(pX[i]) - c*(REAL)(pY[i]))/d;

                pOutput[i+offr] = (unsigned short)(val>0?val:0);
            }
        }
        else
        {
            cout<<"Currently only support 8-bit and 16-bit data"<<endl;
            return false;
        }

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img1,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(pOut);

        //
        return true;
    }
    else if (func_name == tr("rescaleInt"))
    {
        // input  : Source image, Aligned image
        // output : Rescaled aligned result

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) infile = infilelist->at(0); }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) outfile = outfilelist->at(0); }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");

                //print Help info
                cout<<"vaa3d -x ireg -f rescaleInt -o <output> -p \"#s <original_image> #t <aligned_image>\""<<endl;
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_tar).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_img_tar).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);
            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }
        else
        {
            printf("\nERROR: invalid inputs!\n");
            //print Help info
            cout<<"vaa3d -x ireg -f rescaleInt -o <output> -p \"#s <original_image> #t <aligned_image>\""<<endl;
            return false;
        }

        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        //
        V3DLONG sc=sz_img_sub_input[3];
        V3DLONG pagesz_sub = sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];

        V3DLONG tc=sz_img_tar_input[3];
        V3DLONG pagesz_tar = sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];
        V3DLONG totalsz = pagesz_tar*tc;

        if(sc!=tc)
        {
            qDebug()<<"Your target and subject must have the same color channels"<<endl;
            return false;
        }

        unsigned char *pOut = NULL;
        V3DLONG i,c;

        y_new<unsigned char, V3DLONG>(pOut, totalsz*datatype_tar_input);

        // rescale
        if(datatype_sub_input==1)
        {
            for(c=0; c<sc; c++)
            {
                REAL ratio=1, maxval=0;
                REAL ro=1, mv=0;

                unsigned char *p = p_img_sub_input + c*pagesz_sub;

                for(i=0; i<pagesz_sub; i++)
                {
                    if(p[i]>maxval) maxval = p[i];
                }

                ratio = maxval/255;

                if(datatype_tar_input==1)
                {
                    unsigned char *ptar = p_img_tar_input + c*pagesz_tar;
                    unsigned char *poutput = pOut + c*pagesz_tar;

                    for(i=0; i<pagesz_tar; i++)
                    {
                        if(ptar[i]>mv) mv = ptar[i];
                    }

                    ro = mv/255;
                    ratio /= (ro+EPS);

                    for(i=0; i<pagesz_tar; i++)
                    {
                        poutput[i] = (unsigned char)( (REAL)(ptar[i])*ratio );
                    }
                }
                else if(datatype_tar_input==2)
                {
                    unsigned short *ptar = (unsigned short *)p_img_tar_input + c*pagesz_tar;
                    unsigned short *poutput = (unsigned short *)pOut + c*pagesz_tar;

                    for(i=0; i<pagesz_tar; i++)
                    {
                        if(ptar[i]>mv) mv = ptar[i];
                    }

                    ro = mv/4095;
                    ratio /= (ro+EPS);

                    for(i=0; i<pagesz_tar; i++)
                    {
                        poutput[i] = (unsigned short)( (REAL)(ptar[i])*ratio );
                    }
                }
                else
                {
                    cout<<"Only support 8bit and 16bit"<<endl;
                    return false;
                }
            }
        }
        else if(datatype_sub_input==2)
        {
            for(c=0; c<sc; c++)
            {
                REAL ratio=1, maxval=0;
                REAL ro=1, mv=0;

                unsigned short *p = (unsigned short *)p_img_sub_input + c*pagesz_sub;

                for(i=0; i<pagesz_sub; i++)
                {
                    if(p[i]>maxval) maxval = p[i];
                }

                ratio = maxval/4095;

                if(datatype_tar_input==1)
                {
                    unsigned char *ptar = p_img_tar_input + c*pagesz_tar;
                    unsigned char *poutput = pOut + c*pagesz_tar;

                    for(i=0; i<pagesz_tar; i++)
                    {
                        if(ptar[i]>mv) mv = ptar[i];
                    }

                    ro = mv/255;
                    ratio /= (ro+EPS);

                    for(i=0; i<pagesz_tar; i++)
                    {
                        poutput[i] = (unsigned char)( (REAL)(ptar[i])*ratio );
                    }
                }
                else if(datatype_tar_input==2)
                {
                    unsigned short *ptar = (unsigned short *)p_img_tar_input + c*pagesz_tar;
                    unsigned short *poutput = (unsigned short *)pOut + c*pagesz_tar;

                    for(i=0; i<pagesz_tar; i++)
                    {
                        if(ptar[i]>mv) mv = ptar[i];
                    }

                    ro = mv/4095;
                    ratio /= (ro+EPS);

                    for(i=0; i<pagesz_tar; i++)
                    {
                        poutput[i] = (unsigned short)( (REAL)(ptar[i])*ratio );
                    }
                }
                else
                {
                    cout<<"Only support 8bit and 16bit"<<endl;
                    return false;
                }
            }
        }
        else
        {
            cout<<"Only support 8bit and 16bit"<<endl;
            return false;
        }


        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img_tar_input,datatype_tar_input))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }


        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
        y_del<unsigned char>(pOut);

        //
        return true;
    }
    else if (func_name == tr("convertDatatype"))
    {
        // input  : 8bit/16bit datatype
        // output : convert to target datatype

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f convertDatatype -i <input> -o <output> -p \"#t <target> #k <keepinputscale>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p);if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_input(infile);

        // outputs
        QString qs_filename_output;

        int keepinputscale = 0;
        int targetdatatype = 1; // 8-bit

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                targetdatatype = atoi( argv[i+1] ); // uint8 1 uint16 2 float 4
                                i++;
                            }
                            else if (!strcmp(key, "k"))
                            {
                                keepinputscale = atoi( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(keepinputscale<0 || targetdatatype<0)
            {
                printf("\nERROR: invalid inputs!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
            QString qs_pathname_input=QFileInfo(qs_filename_input).path();

            if(outfile)
            {
                qs_filename_output=QString(outfile);

                if(!(QFileInfo(qs_filename_output).suffix().toUpper().compare("TIF") == 0
                     || QFileInfo(qs_filename_output).suffix().toUpper().compare("V3DRAW") == 0))
                {
                    QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                    QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                    qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
                }

            }
            else
            {
                qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
            }

        }

        /// load images
        //
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: load %s fails.\n",qPrintable(qs_filename_input));
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        //
        bool b_new = false;
        unsigned char *p_sub=NULL;
        V3DLONG sc = sz_img1[3];
        V3DLONG totalplxs_sub = sz_img1[0]*sz_img1[1]*sz_img1[2]*sc;

        if(datatype_img1==UINT8)
        {
            if(targetdatatype==1)
            {
                p_sub = p1dImg1;
            }
            else if(targetdatatype==2)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub*2);
                b_new = true;
                unsigned short *p = (unsigned short *)p_sub;

                if(keepinputscale)
                {
                    foreach(totalplxs_sub, i)   p[i] = p1dImg1[i];
                }
                else
                {
                    foreach(totalplxs_sub, i)   p[i] = p1dImg1[i];
                    rescaling<unsigned short, V3DLONG>(p, totalplxs_sub, 0, 4095);
                }
            }
            else if(targetdatatype==4)
            {

            }
            else
            {

            }
        }
        else if(datatype_img1==UINT16)
        {
            if(targetdatatype==1)
            {
                y_new<unsigned char, V3DLONG>(p_sub, totalplxs_sub);
                b_new = true;
                unsigned short *p = (unsigned short *)p1dImg1;

                if(keepinputscale)
                {
                    foreach(totalplxs_sub, i)   p_sub[i] = p[i];
                }
                else
                {
                    rescaling<unsigned short, V3DLONG>(p, totalplxs_sub, 0, 4095);
                    foreach(totalplxs_sub, i)   p_sub[i] = p[i];
                }
            }
            else if(targetdatatype==2)
            {
                p_sub = p1dImg1;
            }
            else if(targetdatatype==4)
            {

            }
            else
            {

            }
        }
        else if(datatype_img1==FLOAT32)
        {

        }
        else
        {

        }

        // saving
        if(targetdatatype==1)
        {
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_sub),sz_img1,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(targetdatatype==2)
        {
            /// save
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p_sub),sz_img1,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else if(targetdatatype==4)
        {

        }
        else
        {

        }

        // de-alloc
        y_del<V3DLONG>(sz_img1);
        y_del<unsigned char>(p1dImg1);
        if(b_new && p_sub){delete []p_sub; p_sub=NULL;}

        //
        return true;
    }
    else if (func_name == tr("ppleftopticlobe"))
    {
        // input  : left optic lobe image stack
        // output : preprocess (rotate 90 degree clockwise, then flip alogn x-axis)

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f ppleftopticlobe -i <input> -o <output> \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_img_input=QString(infile);

        // outputs
        QString qs_filename_output;

        //
        QString qs_basename_input=QFileInfo(qs_filename_img_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_img_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images
        V3DLONG *sz_img = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = 0;

        if(QFileInfo(qs_filename_img_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_input.toStdString().c_str()), p1dImg, sz_img, datatype_img, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_input.toStdString().c_str()),p1dImg,sz_img,datatype_img))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg, sz_img);
                return false;
            }
        }

        //
        V3DLONG slicesz = sz_img[0]*sz_img[1];
        V3DLONG channelsz = slicesz*sz_img[2];
        V3DLONG totalplxs = channelsz*sz_img[3];
        V3DLONG i,j,k;

        /// rotate 90 degree clockwise
        unsigned char *pOut = NULL;
        y_new<unsigned char, V3DLONG>(pOut, totalplxs*datatype_img);

        foreach(sz_img[3], c)
        {
            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*channelsz;
                unsigned char *poutput = pOut + c*channelsz;

                for (k=0;k<sz_img[2];k++)
                {
                    V3DLONG offk = k*slicesz;
                    for (j=0;j<sz_img[1];j++)
                    {
                        V3DLONG offj = offk + j*sz_img[0];
                        for (i=0;i<sz_img[0];i++)
                        {
                            poutput[offk + (i)*sz_img[1] + (sz_img[1]-1-j)] = pinput[offj + i];
                        }
                    }
                }
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*channelsz;
                unsigned short *poutput = (unsigned short *)pOut + c*channelsz;

                for (k=0;k<sz_img[2];k++)
                {
                    V3DLONG offk = k*slicesz;
                    for (j=0;j<sz_img[1];j++)
                    {
                        V3DLONG offj = offk + j*sz_img[0];
                        for (i=0;i<sz_img[0];i++)
                        {
                            poutput[offk + (i)*sz_img[1] + (sz_img[1]-1-j)] = pinput[offj + i];
                        }
                    }
                }
            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        //
        memcpy(p1dImg, pOut, totalplxs*datatype_img);


        /// flip along x-axis
        foreach(sz_img[3], c)
        {
            if(datatype_img == 1)
            {
                //
                unsigned char *pinput = p1dImg + c*channelsz;
                unsigned char *poutput = pOut + c*channelsz;

                flip<unsigned char, V3DLONG>(poutput, pinput, sz_img, AX); // x
            }
            else if(datatype_img == 2)
            {
                //
                unsigned short *pinput = (unsigned short *)p1dImg + c*channelsz;
                unsigned short *poutput = (unsigned short *)pOut + c*channelsz;

                flip<unsigned short, V3DLONG>(poutput, pinput, sz_img, AX); // x
            }
            else
            {
                cout<<"your datatype is not supported!"<<endl;
                return false;
            }
        }

        // save
        if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img,datatype_img))
        {
            printf("ERROR: saveImage failed!\n");
            return false;
        }

        // de-alloc
        y_del<unsigned char>(p1dImg);
        y_del<unsigned char>(pOut);
        y_del<V3DLONG>(sz_img);

        return true;
    }
    else if (func_name == tr("computeRef"))
    {
        // input  : sequence image stack
        // output : ref image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f computeRef -i <folder> -o <output> -p \"#r <ref_channel>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        V3DLONG refn = 0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                refn = atol( argv[i+1] ) - 1; //red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"ref.v3draw";
        }


        /// load images
        QStringList imgList = importSeriesFileList_addnumbersort(qs_filename_input);

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        float *p=NULL;
        V3DLONG sx,sy,sz,sc, imgsz;

        V3DLONG N=imgList.size();

        for(int i=0; i<N; i++)
        {
            QString inputFile = imgList[i];

            //qDebug()<<"loading ... "<<inputFile;

            if(QFileInfo(inputFile).suffix().toUpper().compare("LSM") == 0)
            {
                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(inputFile.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }
            else
            {
                // libtif
                if(!loadImage(const_cast<char *>(inputFile.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }

            //
            if(i==0)
            {
                sx = sz_img1[0];
                sy = sz_img1[1];
                sz = sz_img1[2];
                sc = sz_img1[3];

                imgsz = sx*sy*sz*sc;

                y_new<float, V3DLONG>(p, imgsz);
                memset(p,0,sizeof(float)*imgsz);
            }

            if(datatype_img1==1)
            {
                //
                for(int j=0; j<imgsz; j++)
                    p[j] += p1dImg1[j];
            }
            else if(datatype_img1==2)
            {
                //cout<<"UINT16"<<endl;

                unsigned short *pIn = (unsigned short *)p1dImg1;

                //
                for(int j=0; j<imgsz; j++)
                    p[j] += pIn[j];

//                for(int j=0; j<imgsz; j++)
//                {
//                    //p[j] = p[j] > pIn[j] ? p[j] : pIn[j];

//                    p[j] = std::max(float(p[j]), float(pIn[j]));
//                }
            }

            //
            if(i<N-1)
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        }

        for(int j=0; j<imgsz; j++)
            p[j] /= N;


        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p),sz_img1,4))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<float>(p);

        //
        return true;
    }
    else if (func_name == tr("celltracker"))
    {
        // input  : sequence image stack
        // output : segmented mask image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f celltracker -i <folder> -o <output> -p \"#r <ref_channel> #m <marker_file> #t <thresh> #v <var>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        V3DLONG refn = 0;
        char *fileMarker;
        double thresh = 45;
        double var = 160;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                refn = atol( argv[i+1] ) - 1; //red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                fileMarker = argv[i+1];
                                i++;
                            }
                            else if (!strcmp(key, "t"))
                            {
                                thresh = double(atof(argv[i+1]));
                                i++;
                            }
                            else if (!strcmp(key, "v"))
                            {
                                var = double(atof(argv[i+1]));
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"/sta.txt";
        }

        /// load markers
        QList <ImageMarker> seeds = readMarker_file(QString(fileMarker));

        /// load images
        QStringList imgList = importSeriesFileList_addnumbersort(qs_filename_input);

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = NULL;

        unsigned short *p=NULL;
        V3DLONG sx,sy,sz,sc, imgsz;

        V3DLONG N=imgList.size();

        for(int i=0; i<N; i++)
        {
            QString inputFile = imgList[i];

            qDebug()<<"loading ... "<<inputFile;

            if(QFileInfo(inputFile).suffix().toUpper().compare("LSM") == 0)
            {
                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(inputFile.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }
            else
            {
                // libtif
                if(!loadImage(const_cast<char *>(inputFile.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }

            //
            if(i==0)
            {
                sx = sz_img1[0];
                sy = sz_img1[1];
                sz = sz_img1[2];
                sc = sz_img1[3];

                imgsz = sx*sy*sz*sc;

                y_new<unsigned short, V3DLONG>(p, imgsz);
                memset(p,0,sizeof(unsigned short)*imgsz);
            }

            if(datatype_img1==1)
            {
                unsigned char *pIn = (unsigned char *)p1dImg1;

                //
                for(int j=0; j<seeds.size(); j++)
                {
                    regiongrow<unsigned char, unsigned short, V3DLONG>(pIn, p, sx, sy, sz, seeds[j].x, seeds[j].y, seeds[j].z, j+1, thresh, var);
                    cout<<"progress ... "<<double(j)/double(seeds.size())<<endl;
                }
            }
            else if(datatype_img1==2)
            {
                unsigned short *pIn = (unsigned short *)p1dImg1;

                //
                for(int j=0; j<seeds.size(); j++)
                {
                    regiongrow<unsigned short, unsigned short, V3DLONG>(pIn, p, sx, sy, sz, seeds[j].x, seeds[j].y, seeds[j].z, j+1, thresh, var);
                }
            }

            //
            if(i<N-1)
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        }

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p),sz_img1,2))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned short>(p);

        //
        return true;
    }
    else if (func_name == tr("mipSequence"))
    {
        // input  : sequence image stacks
        // output : MIP

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f mipSequence -i <folder> -o <output> -p \"#r <ref_channel>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        V3DLONG refn = 0;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                refn = atol( argv[i+1] ) - 1; //red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"/sta.txt";
        }

        /// load images

        V3DLONG dimx, dimy, dimz;

        // image
        QStringList imgList = importSeriesFileList_addnumbersort(qs_filename_input);

        V3DLONG n=imgList.size(), imgsz;

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char *p1dImg1 = NULL;
        unsigned char *p = NULL;

        for(V3DLONG i=0; i<n; i++)
        {
            QString inputFile = imgList[i];

            qDebug()<<"loading ... "<<inputFile;

            if(QFileInfo(inputFile).suffix().toUpper().compare("LSM") == 0)
            {
                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(inputFile.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }
            else
            {
                // libtif
                if(!loadImage(const_cast<char *>(inputFile.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }

            if(i==0)
            {
                imgsz = sz_img1[0]*sz_img1[1]*sz_img1[2];
                y_new<unsigned char, V3DLONG>(p, imgsz*datatype_img1);
                memset(p,0,imgsz*datatype_img1);
            }

            if(datatype_img1==1)
            {
                // unsigned char *pIn = (unsigned char *)p1dImg1;
            }
            else if(datatype_img1==2)
            {
                unsigned short *pOut = (unsigned short *)p;
                unsigned short *pIn = (unsigned short *)p1dImg1 + refn*imgsz;

                for(V3DLONG j=0; j<imgsz; j++)
                {
                    pOut[j] = pOut[j]>pIn[j]?pOut[j]:pIn[j];
                }

            }

            //
            if(i<n-1)
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        }


        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p),sz_img1,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(p);

        //
        return true;
    }
    else if (func_name == tr("cellprofiler"))
    {
        // input  : sequence image stacks, mask(label) image
        // output : profile

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f cellprofiler -i <folder> -o <output> -p \"#r <ref_channel> #m <marker_file>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        V3DLONG refn = 0;
        char *fileMask = NULL;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                refn = atol( argv[i+1] ) - 1; //red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "m"))
                            {
                                fileMask = argv[i+1];
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"/sta.txt";
        }

        /// load images

        //
        float *p=NULL;
        V3DLONG sx=0, sy=0; // sy = labels, sx = time

        // mask
        V3DLONG *sz_mask = 0;
        int datatype_mask = 0;
        unsigned char* p1dMask = NULL;

        if(!loadImage(const_cast<char *>(fileMask),p1dMask,sz_mask,datatype_mask))
        {
            printf("ERROR: loadImage() fails.\n");
            y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);
            return false;
        }

        V3DLONG dimx, dimy, dimz;
        dimx = sz_mask[0];
        dimy = sz_mask[1];
        dimz = sz_mask[2];

        if(datatype_mask==1)
        {
            foreach(dimx*dimy*dimz, i)
            {
                if(p1dMask[i]>sy)
                    sy = p1dMask[i];
            }
        }
        else if(datatype_mask==2)
        {
            unsigned short *pMask = (unsigned short *)p1dMask;

            foreach(dimx*dimy*dimz, i)
            {
                if(pMask[i]>sy)
                    sy = pMask[i];
            }

            //
//            for(int i=1; i<=sy; i++)
//            {
//                double mx=0,my=0,mz=0, sum=0;

//                for(int z=0; z<dimz; z++)
//                    for(int y=0; y<dimy; y++)
//                        for(int x=0; x<dimx; x++)
//                        {

//                            short val = pMask[z*dimx*dimy + y*dimx + x];

//                            if(val==i)
//                            {
//                                mx+=x*val;
//                                my+=y*val;
//                                mz+=z*val;

//                                sum+=val;
//                            }
//                        }


//                cout<<int(mx/sum)<<","<<int(my/sum)<<","<<int(mz/sum)<<",5,1,,"<<endl;


//            }

//            return true;

        }

        //
        cout<<"masks ... "<<sy<<endl;

        // image
        QStringList imgList = importSeriesFileList_addnumbersort(qs_filename_input);

        sx=imgList.size();

        y_new<float, V3DLONG>(p, sx*sy);
        memset(p, 0, sizeof(float)*sx*sy);

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = NULL;

        for(int i=0; i<sx; i++)
        {
            QString inputFile = imgList[i];

            qDebug()<<"loading ... "<<inputFile;

            if(QFileInfo(inputFile).suffix().toUpper().compare("LSM") == 0)
            {
                // Mylib
                int pixelnbits = 1;
                if(loadTif2StackMylib(const_cast<char *>(inputFile.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }
            else
            {
                // libtif
                if(!loadImage(const_cast<char *>(inputFile.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
                {
                    printf("ERROR: loadImage() fails.\n");
                    y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                    return false;
                }
            }

            double sumval = 0, sumvol = 0;

            if(datatype_img1==1)
            {
                // unsigned char *pIn = (unsigned char *)p1dImg1;
            }
            else if(datatype_img1==2)
            {
                unsigned short *pIn = (unsigned short *)p1dImg1;

                if(datatype_mask==1)
                {

                }
                else if(datatype_mask==2)
                {
                    unsigned short *pMask = (unsigned short *)p1dMask;

                    for(int j=0; j<sy; j++)
                    {
                        for(V3DLONG z=0; z<dimz; z++)
                            for(V3DLONG y=0; y<dimy; y++)
                                for(V3DLONG x=0; x<dimx; x++)
                                {
                                    V3DLONG idx = z*dimx*dimy + y*dimx + x;

                                    if(pMask[idx]==j+1)
                                    {
                                        sumval += pIn[idx];
                                        sumvol ++;
                                    }
                                }

                        //
                        p[j*sx + i] = sumval / (sumvol + EPS);

                        sumval = 0;
                        sumvol = 0;
                    }
                }
            }

            //
            if(i<sx-1)
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        }

        // print result
        cout<<endl;
        for(int j=0; j<sy; j++)
        {
            for(int i=0; i<sx; i++)
            {
                cout<<" "<< p[j*sx+i];
            }
            cout<<endl;
        }


        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p),sz_mask,4))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<float>(p);

        //
        return true;
    }
    else if (func_name == tr("marker2volume"))
    {
        // input  : marker
        // output : image stack

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f marker2volume -i <marker> -o <image> -p \"#r <radius> #i <intensity_value> #x <dimx> #y <dimy> #z <dimz> \"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        float radius=5;
        float val = 128;
        V3DLONG sx, sy, sz;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "r"))
                            {
                                radius = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                val = atof(argv[i+1]);
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                sx = atol(argv[i+1]);
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sy = atol(argv[i+1]);
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                sz = atol(argv[i+1]);
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        if(sx<1 || sy<1 || sz<1 || radius<1)
        {
            cout<<"invalid inputs"<<endl;
            return false;
        }

        /// load markers
        QList <ImageMarker> seeds = readMarker_file(QString(qs_filename_input));

        /// create a volume
        unsigned char *p = NULL;
        double dist;
        double cx, cy, cz, dx, dy, dz;

        y_new<unsigned char, V3DLONG>(p, sx*sy*sz);
        memset(p, 0, sx*sy*sz);

        for(int j=0; j<seeds.size(); j++)
        {
            cx = double(seeds[j].x);
            cy = double(seeds[j].y); // / 0.6250;
            cz = double(seeds[j].z); // * 0.6720 / 0.6250;

            for(long z=0; z<sz; z++)
                for(long y=0; y<sy; y++)
                    for(long x=0; x<sx; x++)
                    {
                        dx = (x - cx)/0.3162;
                        dy = y - cy;
                        dz = (z - cz)/0.8638;

                        dist = sqrt( dx*dx + dy*dy + dz*dz ); // ball

                        if(dist<radius)
                        {
                            p[z*sx*sy + y*sx + x] = j+1; // val;
                        }
                    }
        }

        // save the mask image
        V3DLONG szimg[4];

        szimg[0] = sx;
        szimg[1] = sy;
        szimg[2] = sz;
        szimg[3] = 1;

        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p),szimg,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del<unsigned char>(p);

        //
        return true;
    }
    else if (func_name == tr("valueReplace"))
    {
        // input  : image
        // output : image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f cellprofiler -i <folder> -o <output> -p \"#r <ref_channel> #m <marker_file>\"\n");
            return false;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        //
        double oldval=-1, newval=-1;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "o"))
                            {
                                oldval = atof( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "n"))
                            {
                                newval = atof( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        if(oldval == newval)
        {
            cout<<"No need to process!"<<endl;
            return true;
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;

        // error check
        if(qs_filename_input==NULL)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+".v3draw";
        }

        /// load images

        // mask
        V3DLONG *szimg = 0;
        int datatype_img = 0;
        unsigned char* p1dImg = NULL;

        if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg,szimg,datatype_img))
        {
            printf("ERROR: loadImage() fails.\n");
            y_del2<unsigned char, V3DLONG>(p1dImg, szimg);
            return false;
        }

        V3DLONG dimx, dimy, dimz;
        dimx = szimg[0];
        dimy = szimg[1];
        dimz = szimg[2];

        foreach(dimx*dimy*dimz, i)
        {
            if(p1dImg[i]==oldval)
                p1dImg[i] = newval;
        }

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(p1dImg),szimg,datatype_img))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg, szimg);

        //
        return true;
    }
    else if (func_name == tr("absOrient"))
    {
        // input  : Source image, Template image
        // output : affine transform, displacement field, warped result

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<"Invalid inputs!"<<endl;
            cout<<"./vaa3d -x ireg -f absoluteOrientation -o <rotation_matrix> -p \"#s <sub> #t <tar>\" "<<endl;
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); } }
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); } }  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // init
        V3DLONG channel_ref_tar = 0;
        V3DLONG channel_ref_sub = 0;

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_affinematrix;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "ct"))
                            {
                                key++;
                                channel_ref_tar = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "cs"))
                            {
                                key++;
                                channel_ref_sub = atoi( argv[i+1] ) - 1; // red 1 green 2 blue 3
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }
            if(channel_ref_sub<0 || channel_ref_tar<0)
            {
                printf("\nERROR: invalid reference channel! Input R(1)G(2)B(3) ...!\n");
                return false;
            }

            //
            QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
            QString qs_pathname_input=QFileInfo(qs_basename_input).path();

            if(outfile)
            {
                QString qs_filename_output=QString(outfile);
                QString qs_pathname_output=QFileInfo(qs_filename_output).path();
                QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

                qs_filename_affinematrix=qs_pathname_output+"/"+qs_basename_output+"Affine.txt";
            }
            else
            {
                qs_filename_affinematrix=qs_pathname_input+"/"+qs_basename_input+"Affine.txt";
            }
        }

        // load images
        V3DLONG *sz_img_tar_input=0,*sz_img_sub_input=0;
        unsigned char *p_img_tar_input=0,*p_img_sub_input=0;

        int datatype_tar_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar_input,datatype_tar_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            return false;
        }
        printf("\t>>read target image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar_input[0],sz_img_tar_input[1],sz_img_tar_input[2],sz_img_tar_input[3]);
        printf("\t\tdatatype: %d\n",datatype_tar_input);

        int datatype_sub_input=0;
        if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub_input,datatype_sub_input))
        {
            printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);
            return false;
        }
        printf("\t>>read subject image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
        printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub_input[0],sz_img_sub_input[1],sz_img_sub_input[2],sz_img_sub_input[3]);
        printf("\t\tdatatype: %d\n",datatype_sub_input);

        if(sz_img_tar_input[3]<channel_ref_tar+1 || sz_img_sub_input[3]<channel_ref_sub+1)
        {
            printf("\nERROR: invalid reference channel!\n");

            y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
            y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

            return false;
        }

        // align
        unsigned char *p_sub=NULL;
        unsigned char *p_tar=NULL;

        // reference channels
        V3DLONG offsets_sub = channel_ref_sub*sz_img_sub_input[0]*sz_img_sub_input[1]*sz_img_sub_input[2];
        V3DLONG offsets_tar = channel_ref_tar*sz_img_tar_input[0]*sz_img_tar_input[1]*sz_img_tar_input[2];

        p_sub = p_img_sub_input+offsets_sub*datatype_sub_input;
        p_tar = p_img_tar_input+offsets_tar*datatype_tar_input;

        V3DLONG *sz_sub=NULL, *sz_tar=NULL;
        y_new<V3DLONG, V3DLONG>(sz_sub, 4);
        y_new<V3DLONG, V3DLONG>(sz_tar, 4);

        for(V3DLONG i=0; i<3; i++)
        {
            sz_sub[i] = sz_img_sub_input[i];
            sz_tar[i] = sz_img_tar_input[i];
        }
        sz_sub[3] = 1;
        sz_tar[3] = 1;

        // affine matrix
        V3DLONG *sz_affine=NULL;
        y_new<V3DLONG, V3DLONG>(sz_affine, 4);

        sz_affine[0] = 4; sz_affine[1] = 4; sz_affine[2] = 1; sz_affine[3] = 1;

        Y_MAT_REAL R;
        R.init(4,4,1); // 3D

        //
        if(datatype_tar_input==1)
        {

            Y_IMG_UINT8 pTar;
            pTar.setImage((unsigned char*)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char*)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT8, Y_IMG_UINT8, Y_MAT_REAL>(pSub, pTar, R);
            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT16, Y_IMG_UINT8, Y_MAT_REAL>(pSub, pTar, R);
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_REAL, Y_IMG_UINT8, Y_MAT_REAL>(pSub, pTar, R);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==2)
        {
            Y_IMG_UINT16 pTar;
            pTar.setImage((unsigned short *)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char *)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT8, Y_IMG_UINT16, Y_MAT_REAL>(pSub, pTar, R);
            }
            else if(datatype_sub_input==2)
            {

                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT16, Y_IMG_UINT16, Y_MAT_REAL>(pSub, pTar, R);
            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_REAL, Y_IMG_UINT16, Y_MAT_REAL>(pSub, pTar, R);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else if(datatype_tar_input==4)
        {

            Y_IMG_REAL pTar;
            pTar.setImage((REAL*)p_tar, sz_tar, 4);

            if(datatype_sub_input==1)
            {
                Y_IMG_UINT8 pSub;
                pSub.setImage((unsigned char *)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT8, Y_IMG_REAL, Y_MAT_REAL>(pSub, pTar, R);

            }
            else if(datatype_sub_input==2)
            {
                Y_IMG_UINT16 pSub;
                pSub.setImage((unsigned short *)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_UINT16, Y_IMG_REAL, Y_MAT_REAL>(pSub, pTar, R);

            }
            else if(datatype_sub_input==4)
            {
                Y_IMG_REAL pSub;
                pSub.setImage((REAL*)p_sub, sz_sub, 4);

                absoluteOrientation<Y_IMG_REAL, Y_IMG_REAL, Y_MAT_REAL>(pSub, pTar, R);
            }
            else
            {
                printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
                return false;
            }
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

        //
        ///////
        //
        for(V3DLONG i=0; i<R.column; i++)
        {
            for(V3DLONG j=0; j<R.row; j++)
            {
                cout<<" "<<R.v[i][j];
            }
            cout<<endl;
        }
        cout<<endl;

        // save affine matrix
        Y_AFFINETRANSFORM_REAL afftrans;
        afftrans.setTransform(R.p, 3);

        if(qs_filename_affinematrix!=NULL)
        {
            if(!afftrans.write(qPrintable(qs_filename_affinematrix)))
            {
                printf("ERROR: saving affine matrix failed!\n");
                return false;
            }
        }
        afftrans.clean();

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p_img_tar_input, sz_img_tar_input);
        y_del2<unsigned char, V3DLONG>(p_img_sub_input, sz_img_sub_input);

        y_del<V3DLONG>(sz_affine);
        y_del<V3DLONG>(sz_sub);
        y_del<V3DLONG>(sz_tar);

        R.clean();

        //
        return true;
    }
    else if (func_name == tr("extractVOI"))
    {
        // input  : Source image, Mask image
        // output : extract VOI image

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("/nUsage: vaa3d -x ireg -f extractVOI -i <input> -o <output> -p \"#m <mask> #x <samplingratio_x> #y <samplingratio_y> #z <samplingratio_z>\" \n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) {infilelist = (vector<char*> *)(input.at(0).p);}
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p);}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters
        if(!infilelist->empty()) { infile = infilelist->at(0); }
        if(!outfilelist->empty()) { outfile = outfilelist->at(0); }

        // init
        REAL srx = 1.0, sry = 1.0, srz = 1.0;
        QString qs_filename_mask = NULL; // maskimage or maskconfiguration

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "m"))
                            {
                                qs_filename_mask = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                srx = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "y"))
                            {
                                sry = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                srz = atof( argv[i+1] ); //
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }
        }

        // inputs
        QString qs_filename_input=QString(infile);

        // outputs
        QString qs_filename_output=NULL;
        QString qs_filename_crop_configure=NULL;

        // error check
        if(qs_filename_input==NULL || qs_filename_mask==NULL || srx<0 || sry<0 || srz<0)
        {
            printf("\nERROR: invalid input!\n");
            return false;
        }

        //
        QString qs_basename_input=QFileInfo(qs_filename_input).baseName();
        QString qs_pathname_input=QFileInfo(qs_filename_input).path();

        if(outfile)
        {
            qs_filename_output=QString(outfile);

            QString qs_pathname_output=QFileInfo(qs_filename_output).path();
            QString qs_basename_output=QFileInfo(qs_filename_output).baseName();

            qs_filename_output=qs_pathname_output+"/"+qs_basename_output+".v3draw";
        }
        else
        {
            qs_filename_output=qs_pathname_input+"/"+qs_basename_input+"_cropped.v3draw";
        }

        /// load images

        // input
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_input).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_input.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_input.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // mask
        V3DLONG *sz_mask = 0;
        int datatype_mask = 0;
        unsigned char* p1dMask = 0;

        if(QFileInfo(qs_filename_mask).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_mask.toStdString().c_str()), p1dMask, sz_mask, datatype_mask, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_mask.toStdString().c_str()),p1dMask,sz_mask,datatype_mask))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dMask, sz_mask);
                return false;
            }
        }

        //
        /// extracting
        //

        V3DLONG sx, sy, sz, sc;
        sx = sz_img1[0];
        sy = sz_img1[1];
        sz = sz_img1[2];
        sc = sz_img1[3];

        //
        V3DLONG pagesz = sx*sy*sz;
        V3DLONG totalpxls = pagesz*sc;
        unsigned char * pOut = NULL;
        y_new<unsigned char, V3DLONG>(pOut, totalpxls*datatype_img1);

        //
        foreach(sc, c)
        {
            V3DLONG offset_c = c*pagesz;
            foreach(sz, k)
            {
                V3DLONG offset_k = offset_c + k*sx*sy;
                V3DLONG offk = k*sx*sy;
                foreach(sy, j)
                {
                    V3DLONG offset_j = offset_k + j*sx;
                    V3DLONG offj = offk + j*sx;
                    foreach(sx, i)
                    {
                        if(datatype_img1==UINT8)
                        {
                            pOut[offset_j + i] = p1dImg1[offset_j + i]*p1dMask[offj + i];
                        }
                        else if(datatype_img1==UINT16)
                        {
                            unsigned short *p = (unsigned short *)pOut;

                            p[offset_j + i] = ((unsigned short *)p1dImg1)[offset_j + i]*p1dMask[offj + i];
                        }
                        else
                        {
                            cout<<"Unsupported datatype!"<<endl;
                            return false;
                        }

                    }
                }
            }
        }

        //
        /// save
        //

        // save the mask image
        if(qs_filename_output!=NULL)
        {
            if(!saveImage(qPrintable(qs_filename_output),(unsigned char *)(pOut),sz_img1,datatype_img1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }

        //de-alloc
        y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
        y_del<unsigned char>(pOut);
        y_del<unsigned char>(p1dMask);

        //
        return true;
    }
    else if (func_name == tr("genVOIs"))
    {
        // input  : subject and target
        // output : resize both into the same size

        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            printf("\nError occurred!\n");
            printf("\nUsage: vaa3d -x ireg -f genVOIs -p \"#s <subject> #t <target>\"!\n");
            return true;
        }

        vector<char*> * infilelist;
        vector<char*> * paralist;
        vector<char*> * outfilelist;

        char * infile = NULL; //input_image_file
        char * paras = NULL; // parameters
        char * outfile = NULL; // output_name

        if(input.size()>0) { infilelist = (vector<char*> *)(input.at(0).p); if(!infilelist->empty()) { infile = infilelist->at(0); }} // specify input
        if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); if(!outfilelist->empty()) { outfile = outfilelist->at(0); }}  // specify output
        if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

        // inputs
        QString qs_filename_img_sub, qs_filename_img_tar;

        // outputs
        QString qs_filename_output_sub, qs_filename_output_tar;

        // parsing parameters
        if(paras)
        {
            int argc = 0;
            int len = strlen(paras);
            int posb[1000];

            for(int i = 0; i < len; i++)
            {
                if(i==0 && paras[i] != ' ' && paras[i] != '\t')
                {
                    posb[argc++] = i;
                }
                else if((paras[i-1] == ' ' || paras[i-1] == '\t') && (paras[i] != ' ' && paras[i] != '\t'))
                {
                    posb[argc++] = i;
                }
            }

            char **argv = NULL;
            try
            {
                argv =  new char* [argc];
                for(int i = 0; i < argc; i++)
                {
                    argv[i] = paras + posb[i];
                }
            }
            catch(...)
            {
                printf("\nError: fail to allocate memory!\n");
                return false;
            }

            for(int i = 0; i < len; i++)
            {
                if(paras[i]==' ' || paras[i]=='\t')
                    paras[i]='\0';
            }

            char* key;
            for(int i=0; i<argc; i++)
            {
                if(i+1 != argc) // check that we haven't finished parsing yet
                {
                    key = argv[i];

                    qDebug()<<">>key ..."<<key;

                    if (*key == '#')
                    {
                        while(*++key)
                        {
                            if (!strcmp(key, "t"))
                            {
                                qs_filename_img_tar = QString( argv[i+1] );
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                qs_filename_img_sub = QString( argv[i+1] );
                                i++;
                            }
                            else
                            {
                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                                return false;
                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

            // error check
            if(qs_filename_img_tar==NULL || qs_filename_img_sub==NULL)
            {
                printf("\nERROR: invalid input file name (target or subject)!\n");
                return false;
            }

            //
            if(!outfile)
            {
                QString qs_basename_input=QFileInfo(qs_filename_img_sub).baseName();
                QString qs_pathname_input=QFileInfo(qs_filename_img_sub).path();

                qs_filename_output_sub=qs_pathname_input+"/"+qs_basename_input+"_rs.v3draw";

                qs_basename_input=QFileInfo(qs_filename_img_tar).baseName();
                qs_pathname_input=QFileInfo(qs_filename_img_tar).path();

                qs_filename_output_tar=qs_pathname_input+"/"+qs_basename_input+"_rs.v3draw";
            }

        }

        /// load images

        // target
        V3DLONG *sz_img1 = 0;
        int datatype_img1 = 0;
        unsigned char* p1dImg1 = 0;

        if(QFileInfo(qs_filename_img_tar).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()), p1dImg1, sz_img1, datatype_img1, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_tar.toStdString().c_str()),p1dImg1,sz_img1,datatype_img1))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg1, sz_img1);
                return false;
            }
        }

        // subject
        V3DLONG *sz_img2 = 0;
        int datatype_img2 = 0;
        unsigned char* p1dImg2 = 0;

        if(QFileInfo(qs_filename_img_sub).suffix().toUpper().compare("LSM") == 0)
        {
            // Mylib
            int pixelnbits = 1;
            if(loadTif2StackMylib(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()), p1dImg2, sz_img2, datatype_img2, pixelnbits))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }
        else
        {
            // libtif
            if(!loadImage(const_cast<char *>(qs_filename_img_sub.toStdString().c_str()),p1dImg2,sz_img2,datatype_img2))
            {
                printf("ERROR: loadImage() fails.\n");
                y_del2<unsigned char, V3DLONG>(p1dImg2, sz_img2);
                return false;
            }
        }

        //
        /// resize
        //

        V3DLONG extraVoxels = 64;

        if(datatype_img1==UINT8 && datatype_img2==UINT8) // 8-bit
        {
            // assuming tar color = 1 sub color >= 1

            //
            Y_IMG_UINT8 pTar;
            pTar.setImage(p1dImg1, sz_img1, 4);
            pTar.getBoundingBox();

            //
            Y_IMG_UINT8 pSub;
            pSub.setImage(p1dImg2, sz_img2, 4);
            pSub.getBoundingBox();

            V3DLONG szimg[4];

            szimg[0] = y_max<V3DLONG>(pTar.bex - pTar.bbx, pSub.bex - pSub.bbx) + extraVoxels;
            szimg[1] = y_max<V3DLONG>(pTar.bey - pTar.bby, pSub.bey - pSub.bby) + extraVoxels;
            szimg[2] = y_max<V3DLONG>(pTar.bez - pTar.bbz, pSub.bez - pSub.bbz) + extraVoxels;
            szimg[3] = 1;

            V3DLONG totalplxs = szimg[0]*szimg[1]*szimg[2];


            // tar
            unsigned char *p_tar = NULL;
            y_new<unsigned char, V3DLONG>(p_tar, totalplxs);

            Y_IMG_UINT8 pTarRs;
            pTarRs.setImage(p_tar, szimg, 4);

            resizeImage<unsigned char, V3DLONG, Y_IMG_UINT8>(pTarRs, pTar, 0, true);

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output_tar),(unsigned char *)(pTarRs.pImg),szimg,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }

            // sub
            szimg[3] = y_max<V3DLONG>(sz_img1[3], sz_img2[3]);
            totalplxs *= szimg[3];

            unsigned char *p_sub = NULL;
            y_new<unsigned char, V3DLONG>(p_sub, totalplxs);

            Y_IMG_UINT8 pSubRs;
            pSubRs.setImage(p_sub, szimg, 4);

            resizeImage<unsigned char, V3DLONG, Y_IMG_UINT8>(pSubRs, pSub, 0, true);

            //
            /// save
            if(!saveImage(qPrintable(qs_filename_output_sub),(unsigned char *)(pSubRs.pImg),szimg,1))
            {
                printf("ERROR: saveImage failed!\n");
                return false;
            }
        }
        else
        {
            cout<<"unsupported datatype\n";
            return false;
        }

        // de-alloc


        //
        return true;
    }
    else
    {
        printf("\nWrong function specified.\n");
        return false;
    }
}

#endif // __IREG_CPP__


