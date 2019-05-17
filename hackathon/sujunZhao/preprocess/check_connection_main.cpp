# include "check_connection.h"

bool check_connection(QString in_swc, QString connection_file, QString img_file, QString out_dir, double radius, V3DPluginCallback2 & callback){
    printf("welcome to use check_connection");
    QString infileLabel;
    if (in_swc.endsWith(".swc") || in_swc.endsWith(".SWC")){
        infileLabel = in_swc.left(in_swc.length() - 4);
    }
    if (in_swc.endsWith(".eswc") || in_swc.endsWith(".ESWC")){
        infileLabel = in_swc.left(in_swc.length() - 5);
    }
    infileLabel = infileLabel.right(infileLabel.length()-infileLabel.lastIndexOf("/")-1);

    // Center of the cropped region can be sepecified by an swc or apo file
    if(connection_file != NULL){

        QList<NeuronSWC> ntlist_connection;
        // Case 1: input is an swc
        if(connection_file.endsWith("swc") || connection_file.endsWith("SWC")){
            ntlist_connection = readSWC_file(connection_file).listNeuron;
        }
        // Case 2: input is an apo
        if(connection_file.endsWith("apo") || connection_file.endsWith("APO")){
            QList<CellAPO> markers = readAPO_file(connection_file);
            for(int i=0; i<markers.size(); i++){
                ntlist_connection.append(CellAPO_to_NeuronSWC(markers.at(i)));
            }
        }
        NeuronTree nt = readSWC_file(in_swc);
        int soma;
        for(int i=0; i<ntlist_connection.size(); i++){
            NeuronSWC center = ntlist_connection.at(i);
            if(center.pn==-1){
                for(int j=0; j<nt.listNeuron.size();j++){
                    if(isEqual(computeDist2(nt.listNeuron.at(j), center, XSCALE, YSCALE, ZSCALE), 0.0)){
                        // Crop subvolume of a connection point
                        QString out_swc = out_dir+"/"+infileLabel+"."+QString::number(i)+".swc";
                        crop_swc(in_swc, out_swc, radius, j, 0, -1);
                        NeuronTree retype = readSWC_file(out_swc);
                        for(int k=0; k<retype.listNeuron.size();k++){
                            retype.listNeuron[k].type = center.type;
                        }
                        export_listNeuron_2swc(retype.listNeuron, qPrintable(out_swc));
                        // Calculate confidence score for the connection. Terafly image required.
                        if(img_file != NULL){
                            // Acquired from $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_reliability_score/calculate_reliability_score_plugin.cpp
                            int scoreType = 1;
                            float radiusFactor = 2;
                            float img_thres = 255; // Subvolume of raw image will be generated if confidence score > img_thres;
                            NeuronTree nt = readSWC_file(out_swc);
                            V_NeuronSWC_list nt_decomposed = NeuronTree__2__V_NeuronSWC_list(nt);
                            NeuronTree nt_new = V_NeuronSWC_list__2__NeuronTree(nt_decomposed);
                            qDebug()<<"calculateScoreTerafly";
//                            NeuronTree nt_scored = calculateScoreTerafly(callback, img_file, nt_new, scoreType, radiusFactor, img_thres, out_dir+"/"+infileLabel+"."+QString::number(i));
                            NeuronTree nt_scored = calculateScoreTerafly(callback, img_file, nt_new, scoreType, radiusFactor, img_thres);
                            qDebug()<<"calculateScoreTerafly Done";
                            out_swc = out_dir+"/"+infileLabel+"."+QString::number(i)+".confidence.swc";
                            writeESWC_file(out_swc, nt_scored);
                            QList<CellAPO> apolist;
                            apolist.append(get_marker(center, 10, 255,0,0));
                            writeAPO_file(out_dir+"/"+infileLabel+"."+QString::number(i)+".confidence.apo", apolist);
                            my_saveANO(out_dir+"/"+infileLabel+"."+QString::number(i)+".confidence", true, true);
                        }
                    }
                }
            }
        }
        return 1;
    }
    else{
        qDebug()<<"No connection file specified.";
        return 0;
    }
}

bool check_connection_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;

    if(input.size() != 2)
    {
        printf("Please specify parameter set.\n");
        printHelp_check_connection();
        return false;
    }
    paralist = (vector<char*>*)(input.at(1).p);


    if (paralist->size()!=1)
    {
        printf("Please specify all paramters in one text string.\n");
        printHelp_check_connection();
        return false;
    }


    char * paras = paralist->at(0);
    int argc = 1;
    enum {kArgMax = 64};
    char *argv[kArgMax];

    //parsing parameters
    if (paras)
    {
        int len = strlen(paras);
        for (int i=0;i<len;i++)
        {
            if (paras[i]=='#')
                paras[i] = '-';
        }

        char* pch = strtok(paras, " ");
        while (pch && argc<kArgMax)
        {
            argv[argc++] = pch;
            pch = strtok(NULL, " ");
        }
    }
    else
        printHelp_check_connection();


    //0. read arguments

    char *dfile_input = NULL;
    char *connection_file = NULL;
    char *img_file = NULL;
    char *dfile_result = NULL;
    double radius = 30;

    int c;
    static char optstring[]="h:i:c:t:o:r:";
    extern char * optarg;
    extern int optind, opterr;
    optind = 1;
    while ((c = getopt(argc, argv, optstring))!=-1)
    {
        switch (c)
        {
            case 'h':
                printHelp_check_connection();
                return 0;
            case 'i':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                dfile_input = optarg;
                cout << "Input file name:\t" << dfile_input <<endl;
                break;
            case 'c':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                connection_file = optarg;
                cout << "Connection file name:\t" << connection_file <<endl;
                break;
            case 't':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -i.\n");
                    return 1;
                }
                img_file = optarg;
                cout << "Connection file name:\t" << img_file <<endl;
                break;
            case 'o':
                if (strcmp(optarg,"(null)")==0 || optarg[0]=='-')
                {
                    fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
                    return 1;
                }
                dfile_result = optarg;
                break;
            case '?':
                fprintf(stderr,"Unknown option '-%c' or incomplete argument lists.\n",optopt);
                return 1;
                break;
        }
    }
    check_connection(QString(dfile_input), QString(connection_file), QString(img_file), QString(dfile_result), radius, callback);

    return 1;
}

void printHelp_check_connection()
{
    printf("\t#i <input_filename> :   input swc.\n");
    printf("\t#c <connection_filename> :  connection file (swc or apo format).\n");
    printf("\t#o <output_dir> :   output directory.\n");
    printf("\t#r <radius> :   radius used for cropping area near the new connections; default=30\n");
    printf("Usage: vaa3d -x preprocess -f check_connection -p \"#i input.swc #c connection.swc/apo #o result.swc #r 30\"\n");
}
