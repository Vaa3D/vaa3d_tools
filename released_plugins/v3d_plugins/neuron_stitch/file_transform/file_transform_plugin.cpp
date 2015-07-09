/* file_transform_plugin.cpp
 * This plugin will transform and combine image by given affine matrix. Some functions are based on littleQuickWarper.
 * 2015-3-18 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_stitch_func.h"
#include "file_transform_plugin.h"
#include "image_transform_and_combine_by_affine_mat_func.cpp"
#include "neuron_xforms.h"

using namespace std;
Q_EXPORT_PLUGIN2(file_transform, file_transform)
 
QStringList file_transform::menulist() const
{
	return QStringList() 
		<<tr("transform_and_combine_images_by_affine_mat")
        <<tr("transform_neuron_SWC_by_affine_matrix")
        <<tr("transform_markers_by_affine_matrix")
		<<tr("about");
}

QStringList file_transform::funclist() const
{
	return QStringList()
        <<tr("affineImage")
		<<tr("help");
}

void file_transform::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("transform_and_combine_images_by_affine_mat"))
	{
        v3d_msg("Please check do_func, GUI to be implemented.");
	}
    else if (menu_name == tr("transform_neuron_SWC_by_affine_matrix"))
    {
        dotransform_swc(callback, parent);
    }
    else if (menu_name == tr("transform_markers_by_affine_matrix"))
    {
        dotransform_marker(callback, parent);
    }
	else
	{
		v3d_msg(tr("This plugin will transform and combine image by given affine matrix. Some functions are based on littleQuickWarper.. "
			"Developed by Hanbo Chen, 2015-3-18"));
	}
}

bool file_transform::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("affineImage"))
	{
        qDebug()<<"==== affine and combine images ====";
        if(infiles.size()!=3 || outfiles.size()!=1){
            qDebug()<<"ERROR: please set input and output!";
            printHelp();
            return false;
        }
        QString fname_img_tar=((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_img_sub=((vector<char*> *)(input.at(0).p))->at(1);
        QString fname_amat=((vector<char*> *)(input.at(0).p))->at(2);
        QString fname_output=((vector<char*> *)(output.at(0).p))->at(0);

        //init parameters
        bool b_channelSeperate = false;
        bool b_negativeShift = true;
        int interpMethod = 0;
        if(input.size() >= 2){
            vector<char*> paras = (*(vector<char*> *)(input.at(1).p));

            if(paras.size()>0){
                int tmp=atoi(paras.at(0));
                if(tmp==1)
                    interpMethod = 1;
            }
            if(paras.size()>1){
                int tmp=atoi(paras.at(1));
                if(tmp==0)
                    b_negativeShift = false;
            }
            if(paras.size()>2){
                int tmp=atoi(paras.at(2));
                if(tmp==1)
                    b_channelSeperate = true;
            }
        }
        doAffineImage(callback, fname_img_tar, fname_img_sub, fname_amat, fname_output, interpMethod, b_negativeShift, b_channelSeperate);
	}
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

void file_transform::doAffineImage(V3DPluginCallback2 & callback, QString fname_tar,
                                             QString fname_sub, QString fname_amat, QString fname_output,
                                             int interpMethod, bool b_negativeShift, bool b_channelSeperate)
{
    //load amat and check eligibility
    double amat[16];
    if(!readAmat(fname_amat.toStdString().c_str(), amat) ){
        qDebug()<<"error: cannot read affine matrix: "<<fname_amat;
        return;
    }
    if(fabs(amat[12])>1e-6 ||
            fabs(amat[13])>1e-6 ||
            fabs(amat[14])>1e-6 ||
            fabs(amat[15]-1)>1e-6 )
    {
        qDebug()<<"invalide last row of amat";
        return;
    }

    //load images
    unsigned char * p_img_sub = 0;
    unsigned char * p_img_tar = 0;
    unsigned char * p_img_out = 0;
    V3DLONG sz_sub[4], sz_tar[4], sz_out[4]={0}, globalShift[3]={0};
    int type_sub, type_tar, type_out;

    if(!simple_loadimage_wrapper(callback, fname_tar.toStdString().c_str(), p_img_tar, sz_tar, type_tar))
    {
        qDebug()<<"load image "<<fname_tar<<" error!";
        return;
    }
    if(!simple_loadimage_wrapper(callback, fname_sub.toStdString().c_str(), p_img_sub, sz_sub, type_sub))
    {
        qDebug()<<"load image "<<fname_sub<<" error!";
        return;
    }
    if(type_tar != type_sub){
        qDebug()<<"error: the data type of target image and subject image is not the same";
        return;
    }else{
        type_out = type_tar;
    }

    bool success = false;
    if(type_sub==1){
        success=affineCombineImage(p_img_tar, sz_tar, p_img_sub, sz_sub, p_img_out, sz_out,
                                   globalShift, amat, interpMethod, b_channelSeperate, true);
    }else if(type_sub==2){
        unsigned short * p_tmp = 0;
        success=affineCombineImage((unsigned short *)p_img_tar, sz_tar, (unsigned short *)p_img_sub,
                                   sz_sub, p_tmp, sz_out, globalShift,
                                   amat, interpMethod, b_channelSeperate, b_negativeShift);
        p_img_out = (unsigned char *) p_tmp;
    }else if(type_sub==4){
        float * p_tmp = 0;
        success=affineCombineImage((float *)p_img_tar, sz_tar, (float *)p_img_sub,
                                   sz_sub, p_tmp, sz_out, globalShift,
                                   amat, interpMethod, b_channelSeperate, b_negativeShift);
        p_img_out = (unsigned char *) p_tmp;
    }else{
        qDebug()<<"error: do not support the input type. Currently only support UINT8, UINT16, FLOAT.";
        return;
    }

    qDebug()<<"Saving output: "<<fname_output;
    QString fname_gmat=fname_output+"_offset.txt";
    double gmat[16]={0};
    gmat[0]=gmat[5]=gmat[10]=gmat[15]=1;
    gmat[3]=globalShift[0];
    gmat[7]=globalShift[1];
    gmat[11]=globalShift[2];
    writeAmat(fname_gmat.toStdString().c_str(),gmat);
    if(!simple_saveimage_wrapper(callback, fname_output.toStdString().c_str(), p_img_out, sz_out, type_out) )
    {
        qDebug()<<"save image "<<fname_output<<" error!";
        return;
    }
}

int file_transform::dotransform_swc(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file name
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(fileOpenName.isEmpty())
        return 0;
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
        nt = readSWC_file(fileOpenName);
    }

    QString fileMatName = QFileDialog::getOpenFileName(0, QObject::tr("Open Affine Matrix File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));
    if(fileMatName.isEmpty()) return 0;

    double amat[16]={0};
    if (!readAmat(fileMatName.toStdString().c_str(),amat))
    {
            v3d_msg("error read affine transform matrix.");
            return 0;
    }

    proc_neuron_affine(&nt, amat);

    QString fileDefaultName = fileOpenName+QString("_affine.swc");
    //write new SWC to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.swc)"
                ";;Neuron structure	(*.swc)"
                ));
    if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
    {
        v3d_msg("fail to write the output swc file.");
        return 0;
    }

    return 1;
}

int file_transform::dotransform_marker(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file name
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            "",
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName.isEmpty())
        return 0;
    QList <ImageMarker> inmarker;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker = readMarker_file(fileOpenName);
    }

    QString fileMatName = QFileDialog::getOpenFileName(0, QObject::tr("Open Affine Matrix File"),
            "",
            QObject::tr("Supported file (*.txt)"
                ";;Affine Matrix    (*.txt)"
                ));
    if(fileMatName.isEmpty()) return 0;

    double afmatrix[16]={0};
    if (!readAmat(fileMatName.toStdString().c_str(),afmatrix))
    {
        v3d_msg("error read affine transform matrix.");
        return 0;
    }

    //marker affine
    double x,y,z;
    for(V3DLONG i=0; i<inmarker.size() ; i++)
    {
        ImageMarker* tp = &(inmarker[i]);
        x = afmatrix[0] * tp->x + afmatrix[1] * tp->y + afmatrix[2] * tp->z + afmatrix[3];
        y = afmatrix[4] * tp->x + afmatrix[5] * tp->y + afmatrix[6] * tp->z + afmatrix[7];
        z = afmatrix[8] * tp->x + afmatrix[9] * tp->y + afmatrix[10] * tp->z + afmatrix[11];

        //now update
        tp->x = x;	tp->y = y; tp->z = z;
    }

    QString fileDefaultName = fileOpenName+QString("_affine.marker");
    //write new marker to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker	(*.marker)"
                ));
    if(fileSaveName.isEmpty())
        return 0;
    if (!writeMarker_file(fileSaveName, inmarker))
    {
        v3d_msg("fail to write the output marker file.");
        return 0;
    }

    return 1;
}

void file_transform::printHelp()
{
    qDebug()<<"Usage : v3d -x dllname -f affineImage -i <target_img_file> <subject_img_file> <affine_matrix.txt> -p <(0)interpmethod> <(1)negative shifting> <(0)seperate channel> -o <out_img_file>";
    qDebug()<<"interpmethod: 0(default): linera interpolate; 1: nearest neighbor interpolate.";
    qDebug()<<"negative shifting: if set 0, the image component with negative index after affine transform will not be saved. Otherwise(default), the whole image will be shifted to make space for that part.";
    qDebug()<<"seperate channel: if set 1, target and subject image will be saved in different channels (for visualization), otherwise(default), they will be saved in the same channel.";
    qDebug()<<" ";
}
