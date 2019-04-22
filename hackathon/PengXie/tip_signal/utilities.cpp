#include "utilities.h"

bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix){
    if(!ano_dir.endsWith('/')){ano_dir = ano_dir+'/';}
    FILE * fp=0;
    fp = fopen((char *)qPrintable(ano_dir+fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    for(int i=0; i<suffix.size(); i++)
    {
        QString tail = suffix.at(i);
        if(!tail.startsWith('.')){tail = '.'+tail;}
        QString header;
        if(tail.endsWith("SWC") || tail.endsWith("swc")){header = "SWCFILE=";}
        else if(tail.endsWith("APO") || tail.endsWith("apo")){header = "APOFILE=";}
        else{header = "RAWIMG=";}
        fprintf(fp, qPrintable(header+fileNameHeader+tail));
        fprintf(fp, "\n");
    }
    if(fp){fclose(fp);}
    return true;
}
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

double node_dist(NeuronSWC a, NeuronSWC b, bool scale)
{
    if(scale){
        a.x = a.x*RX;
        a.y = a.y*RY;
        a.z = a.z*RZ;
        b.x = b.x*RX;
        b.y = b.y*RY;
        b.z = b.z*RZ;
    }
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}


block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}

int get_soma(NeuronTree nt){
    printf("Checking soma\n");
    const int N=nt.listNeuron.size();
    int soma;
    int soma_ct=0;
    // check whether unique soma
    for(int i=0; i<N; i++){
        // soma check
        if(nt.listNeuron.at(i).type==1){
            soma=i;
            soma_ct++;
            break;
//            if(soma_ct>1){return -1;}
        }
    }
    if(soma_ct==0){
        qDebug() << "Error: No soma found\n";
        return -1;
    }
    return soma;
}

QList<int> get_tips(NeuronTree nt, bool include_root){
    // whether a node is a tip;
    QList<int> tip_list;
    QList<int> plist;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        if(include_root & nt.listNeuron.at(i).pn == -1){
            tip_list.append(i);
        }
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){tip_list.append(i);}
    }
    return(tip_list);
}

inline bool exists_file (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
//inline bool exists_file_qs (QString name) {
//  struct stat buffer;
//  std::string string_name = qPrintable(name);
//  return (stat (string_name.c_str(), &buffer) == 0);
//}
void my_mkdir(QString path)
{
    // Creating a directory
    if (mkdir(qPrintable(path), 0777) == 0)
        printf("Directory created:%s\n", qPrintable(path));
}
void read_directory(QString name, QList<QString> & files)
{
    DIR* dirp = opendir(qPrintable(name));
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        files.append(dp->d_name);
    }
    closedir(dirp);
}

// functions for getting blocks

void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format)
{
    printf("welcome to use crop_img\n");
    if(output_format.size()==0){output_format=QString(".tiff");}

    QString save3DName = outputdir_img + "/" + crop_block.name + output_format;
    QString save2DName = outputdir_img + "/" + crop_block.name + "tif";

    // Qstring to const char*
//    const char* fileName = Qstring_to_char(saveName);
    QByteArray array = save3DName.toLatin1();
    array = array.replace(" ", "");
    const char* file3DName = array.data();

    qDebug()<<"Output image:"<<QString(file3DName);

    V3DLONG *in_zz;
    if(!callback.getDimTeraFly(image.toStdString(), in_zz))
    {
        v3d_msg("Cannot load terafly images.",0);
        return;
    }
    // 1. When cropping, ranges must be integers
    XYZ small=XYZ(crop_block.small);
    XYZ large=XYZ(crop_block.large);
//     // pixels at large values won't be included, so set large.x/y/z as large.x/y/z + 1
//    small.x = floor(small.x);
//    small.y = floor(small.y);
//    small.z = floor(small.z);
//    large.x = ceil(large.x)+1;
//    large.y = ceil(large.y)+1;
//    large.z = ceil(large.z)+1;

    // 2. Crop image. image is stored as 1d array. 2 parameters needed for cropping:
    // 2.1. 'cropped_image' is a pointer to the beginning of the region of interest
    unsigned char * data1d = 0;
    qDebug()<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z;
    data1d = callback.getSubVolumeTeraFly(image.toStdString(),
                                          small.x, large.x,
                                          small.y, large.y,
                                          small.z, large.z);
    // 2.2. 'in_sz' sets the size of the region.
    V3DLONG in_sz[4];
    in_sz[0] = large.x-small.x;
    in_sz[1] = large.y-small.y;
    in_sz[2] = large.z-small.z;
    in_sz[3] = in_zz[3];   // channel information

    // 3. Save image
    if((QString(file3DName).endsWith(".nrrd")) || (QString(file3DName).endsWith(".v3draw")))
    {
        simple_saveimage_wrapper(callback, file3DName, data1d, in_sz, 1);
    }
    else{
        printf("bad output image name:%s\n", file3DName);
//        qDebug()<<"bad name:"<<QString(fileName);
    }
    return;
}

void crop_swc(QString input_swc, QString output_swc, block crop_block)
{
    QString cmd = "vaa3d -x preprocess -f crop_swc_cuboid -i " + input_swc + " -o " +output_swc  + " -p "
             + "\""
             + "#a " + QString::number(crop_block.small.x)
             + " #b " + QString::number(crop_block.small.y)
             + " #c " + QString::number(crop_block.small.z)
             + " #d " + QString::number(crop_block.large.x)
             + " #e " + QString::number(crop_block.large.y)
             + " #f " + QString::number(crop_block.large.z)
             + " #r " + QString::number(crop_block.small.x)
             + " #s " + QString::number(crop_block.small.y)
             + " #t " + QString::number(crop_block.small.z)
             + "\"";
    qDebug()<<cmd;
    system(qPrintable(cmd));
    return;
}

bool generate_2d_img(V3DPluginCallback & cb, QString raw_img, QString swc, V3DLONG in_sz[4], QString output_file)
{
    // 1. Load image
    unsigned char * data1d = 0;
    const char * raw_img_char;
    raw_img_char = Qstring_to_char(raw_img);
    int input_format = 1;
    simple_loadimage_wrapper(cb, raw_img_char, data1d, in_sz, input_format);
    qDebug()<<"in_sz"<<in_sz[0]<<in_sz[1]<<in_sz[2]<<in_sz[3];
    qDebug()<<"input_format"<<input_format;

    // 2. Converte 3D image to 2D
    V3DLONG stacksz =in_sz[0]*in_sz[1];
    unsigned char *image_mip=0;
    image_mip = new unsigned char [stacksz];
    unsigned char* outputImg1DPtr = new unsigned char[stacksz];
    for(V3DLONG iy = 0; iy < in_sz[1]; iy++)
    {
        V3DLONG offsetj = iy*in_sz[0];
        for(V3DLONG ix = 0; ix < in_sz[0]; ix++)
        {
            int max_mip = 0;
            for(V3DLONG iz = 0; iz < in_sz[2]; iz++)
            {
                V3DLONG offsetk = iz*in_sz[1]*in_sz[0];
                if(data1d[offsetk + offsetj + ix] >= max_mip)
                {
                    image_mip[iy*in_sz[0] + ix] = data1d[offsetk + offsetj + ix];
                    max_mip = data1d[offsetk + offsetj + ix];
                }
            }
        }
    }
    in_sz[2] = 1;
    in_sz[3] = 3;

//    // Set color channels
//    unsigned char* data1d_2D = 0;
//    data1d_2D = new unsigned char [3*stacksz];

//    for(V3DLONG i=0; i<stacksz; i++)
//    {
//        data1d_2D[i] = image_mip[i];
//    }
//    for(V3DLONG i=0; i<stacksz; i++)
//    {
//        data1d_2D[i+stacksz] = image_mip[i];
//    }
//    for(V3DLONG i=0; i<stacksz; i++)
//    {
//        data1d_2D[i+2*stacksz] = image_mip[i];
//    }

//    int datatype = 1;
//    simple_saveimage_wrapper(cb, Qstring_to_char(output_file), data1d_2D, in_sz, datatype);
//    return true;

//    // Addaptive thresholding
//    int imgDims[3];
//    imgDims[0] = in_sz[0];
//    imgDims[1] = in_sz[1];
//    imgDims[2] = 1;
//    ImgProcessor::simpleAdaThre(image_mip, image_mip, imgDims, 5, 3);

    // 3. Load swc and generate 2D mask
    NeuronTree nt = readSWC_file(swc);
    unsigned char* data1d_mask = 0;
    data1d_mask = new unsigned char [stacksz];

    memset(data1d_mask, 0, stacksz*sizeof(unsigned char));

    for(V3DLONG i =0; i < nt.listNeuron.size();i++)
    {
        nt.listNeuron[i].z = 0;
        nt.listNeuron[i].r = 0;
    }

    double margin=0;//by PHC 20170531
    ComputemaskImage(nt, data1d_mask, in_sz[0], in_sz[1], 1, margin);

    // 4. Set background / foreground color channels
    unsigned char* data1d_2D = 0;
    data1d_2D = new unsigned char [3*stacksz];

    for(V3DLONG i=0; i<stacksz; i++)
    {
        data1d_2D[i] = (data1d_mask[i]==255) ? 255:image_mip[i];
    }
    for(V3DLONG i=0; i<stacksz; i++)
    {
        data1d_2D[i+stacksz] = image_mip[i];
    }
    for(V3DLONG i=0; i<stacksz; i++)
    {
        data1d_2D[i+2*stacksz] = image_mip[i];
    }


    int datatype = 1;
    simple_saveimage_wrapper(cb, Qstring_to_char(output_file), data1d_2D, in_sz, datatype);
    return true;
}

void run_matlab(QString data_dir, QString block_name)
{
    // 1. write to a matlab script
    QString mscript = data_dir+block_name+"/"+block_name+".m";
    FILE * fp = 0;
    fp = fopen((char *)qPrintable(mscript), "wt");
    if(!fp){
        printf("Cannot open matlab script!\n");
        return;
    }
    my_mkdir(data_dir+block_name+"/Profiles/");
    my_mkdir(data_dir+block_name+"/Results/");
    fprintf(fp, "im_pth='%s';\n", qPrintable(data_dir+block_name+"/Image/"));
    fprintf(fp, "im_id='%s';\n", qPrintable(block_name+".tif"));
    fprintf(fp, "trace_pth='%s';\n", qPrintable(data_dir+block_name+"/Traces/"));
    fprintf(fp, "profile_file='%s';\n", qPrintable(data_dir+block_name+"/Profiles/"));
    fprintf(fp, "results_file='%s';\n", qPrintable(data_dir+block_name+"/Results/"));
    fprintf(fp, "apo_file='%s';\n", qPrintable(data_dir+block_name+"/"+block_name+".apo"));
    fprintf(fp, "ch=%d';\n", 1);
    fprintf(fp, "[AxonMat, Profile] = cmd_BoutonAnalyzer(im_pth, im_id, trace_pth, profile_file, results_file, ch, apo_file);\n");
    fclose(fp);
    QString cmd="cat "+mscript + "|matlab";
    system(qPrintable(cmd));
    return;
}

bool my_saveANO(QString fileNameHeader, bool swc, bool apo, QString swc_name){
    FILE * fp=0;
    fp = fopen((char *)qPrintable(fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    if(fileNameHeader.count("/")>0){
        fileNameHeader = fileNameHeader.right(fileNameHeader.size()-fileNameHeader.lastIndexOf("/")-1);
    }

    if(swc){
        if(swc_name.length()==0){
            fprintf(fp, "SWCFILE=%s\n", qPrintable(fileNameHeader+QString(".swc")));
        }
        else{
            swc_name = swc_name.right(swc_name.size()-swc_name.lastIndexOf("/")-1);
            fprintf(fp, "SWCFILE=%s\n", qPrintable(swc_name));
        }
    }
    if(apo){fprintf(fp, "APOFILE=%s\n", qPrintable(fileNameHeader+QString(".apo")));}
    if(fp){fclose(fp);}
    return true;
}
QList<CellAPO> offset_apo(QString input_apo, XYZ offset)
{
    QList<CellAPO> apo_list;
    if(exists_file(qPrintable(input_apo))){
        apo_list = readAPO_file(input_apo);
        for(int i=0; i<apo_list.size(); i++){
            apo_list[i].x += offset.x;
            apo_list[i].y += offset.y;
            apo_list[i].z += offset.z;
        }
    }
    return apo_list;
}

const char * Qstring_to_char(QString qs)
{
//    QByteArray array = qs.toLatin1();
//    array = array.replace(" ", "");
//    const char* result = array.data();
//    return result;

    const char * str;
    QString path;
    QByteArray ba;
    ba = qs.toLatin1();
    str = ba.data();
//    printf("the string path will be:%s\n", str);

    return str;
}

