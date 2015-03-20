/* image_transform_and_combine_by_affine_mat_plugin.cpp
 * This plugin will transform and combine image by given affine matrix. Some functions are based on littleQuickWarper.
 * 2015-3-18 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_transform_and_combine_by_affine_mat_plugin.h"
#include "image_transform_and_combine_by_affine_mat_func.cpp"

using namespace std;
Q_EXPORT_PLUGIN2(image_transform_and_combine_by_affine_mat, imageTransfromAndCombine);
 
QStringList imageTransfromAndCombine::menulist() const
{
	return QStringList() 
		<<tr("transform_and_combine_images_by_affine_mat")
		<<tr("about");
}

QStringList imageTransfromAndCombine::funclist() const
{
	return QStringList()
        <<tr("affineImage")
		<<tr("help");
}

void imageTransfromAndCombine::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("transform_and_combine_images_by_affine_mat"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin will transform and combine image by given affine matrix. Some functions are based on littleQuickWarper.. "
			"Developed by Hanbo Chen, 2015-3-18"));
	}
}

bool imageTransfromAndCombine::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void imageTransfromAndCombine::doAffineImage(V3DPluginCallback2 & callback, QString fname_tar,
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

void imageTransfromAndCombine::printHelp()
{
    qDebug()<<"Usage : v3d -x dllname -f affineImage -i <target_img_file> <subject_img_file> <affine_matrix.txt> -p <(0)interpmethod> <(1)negative shifting> <(0)seperate channel> -o <out_img_file>";
    qDebug()<<"interpmethod: 0(default): linera interpolate; 1: nearest neighbor interpolate.";
    qDebug()<<"negative shifting: if set 0, the image component with negative index after affine transform will not be saved. Otherwise(default), the whole image will be shifted to make space for that part.";
    qDebug()<<"seperate channel: if set 1, target and subject image will be saved in different channels (for visualization), otherwise(default), they will be saved in the same channel.";
    qDebug()<<" ";
}
