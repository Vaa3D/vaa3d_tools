/* klb_plugin.cpp
 * 2015-6-2 : by Yang Yu (yuy@janelia.hhmi.org)
 */

#include <vector>
#include <iostream>

#include "stackutil.h"
#include "volimg_proc.h"

#include "klb_plugin.h"

using namespace std;
Q_EXPORT_PLUGIN2(klb_file_io, KLBPlugin);

void printHelp()
{
    return;
}

QStringList KLBPlugin::menulist() const
{
    return QStringList()
            <<tr("about");
}

QStringList KLBPlugin::funclist() const
{
    return QStringList()
            <<tr("klbReadWrite")
           <<tr("help");
}

void KLBPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
    {
        QMessageBox::information(parent, "klb file io", tr("This is a plugin of KLB IO.\n Developed by Yang Yu (yuy@janelia.hhmi.org), 2015-6-2"));
    }
}

bool KLBPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("klbReadWrite"))
    {
        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<endl<<"Vaa3D KLB-Plugin"<<endl;
            printHelp();
            return false;
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

        // parameters
        char *dfile_target = NULL;
        char *dfile_subject = NULL;
        char *dfile_warped = NULL;
        char *dfile_df = NULL;
        char *dfile_landmark_target = NULL;
        char *dfile_landmark_subject = NULL;
        char *dfile_volumechangemap = NULL;
        string my_vcmap_file;
        int channelNo_subject = 0, channelNo_target = 0;
        int warpType = 0;
        int chan_attached_target=-1, chan_attached_subject=-1;
        bool b_attach_channel_to_subject = false;
        bool b_same_sz_warp_target = true; //need to put as a command line parameter later

        bool b_onlyconvertformat=false;
        bool b_do_resampling=false;
        bool b_convert2uint8 = false;

        double target_xy_rez=1, target_z_rez=1;
        double subject_xy_rez=1, subject_z_rez=1;
        double dfactor_subject=1, dfactor_target=1;

        V3DLONG XYPlaneMaxSize=512; //the -B option
        double factor_landmark_coord_resample=1; //-e option

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
//                            if (!strcmp(key, "t"))
//                            {
//                                dfile_target = argv[i+1];
//                                i++;
//                            }
//                            else if (!strcmp(key, "C"))
//                            {
//                                channelNo_target = atoi( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "X"))
//                            {
//                                target_xy_rez = atof( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "Z"))
//                            {
//                                target_z_rez = atof( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "L"))
//                            {
//                                dfile_landmark_target = argv[i+1]; //
//                                my_para.file_landmark_target = dfile_landmark_target;
//                                i++;
//                            }
//                            else if (!strcmp(key, "s"))
//                            {
//                                dfile_subject = argv[i+1]; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "c"))
//                            {
//                                channelNo_subject = atoi( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "x"))
//                            {
//                                subject_xy_rez = atof( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "z"))
//                            {
//                                subject_z_rez = atof( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "l"))
//                            {
//                                dfile_landmark_subject = argv[i+1]; //
//                                my_para.file_landmark_subject = dfile_landmark_subject;
//                                i++;
//                            }
//                            else if (!strcmp(key, "o"))
//                            {
//                                dfile_warped = argv[i+1];
//                                my_para.tag_output_image_file = dfile_warped;
//                                i++;
//                            }
//                            else if (!strcmp(key, "w"))
//                            {
//                                warpType = atoi( argv[i+1] ); //
//                                i++;
//                            }
//                            else if (!strcmp(key, "H"))
//                            {
//                                int tmpv = atoi (argv[i+1]);
//                                if (tmpv<1 || tmpv>=4)
//                                {
//                                    tmpv = 1;
//                                    printf ("Your choice of for -H option is not between 1 and 4. Force it to be 1.\n");
//                                }
//                                my_para.hierachical_match_level = tmpv;
//                                i++;
//                            }
//                            else if (!strcmp(key, "i"))
//                            {
//                                int tmpv = atoi (argv[i+1]);
//                                if (tmpv<0 || tmpv>=3)
//                                {
//                                    tmpv = 1;
//                                    printf ("Your choice of for -i option is not between 0 and 3. Force it to be 1.\n");
//                                }
//                                if (tmpv==0) my_para.method_df_compute = DF_GEN_TPS;
//                                else if (tmpv==1) my_para.method_df_compute = DF_GEN_TPS_LINEAR_INTERP;
//                                else my_para.method_df_compute = DF_GEN_TPS;
//                                i++;
//                            }
//                            else if (!strcmp(key, "j"))
//                            {
//                                int tmpv = atoi (argv[i+1]);
//                                if (tmpv<0 || tmpv>=3)
//                                {
//                                    tmpv = 1;
//                                    printf ("Your choice of for -j option is not between 0 and 3. Force it to be 1.\n");
//                                }
//                                if (tmpv==0) my_para.method_match_landmarks = MATCH_MI;
//                                else if (tmpv==1) my_para.method_match_landmarks = MATCH_MULTIPLE_MI_INT_CORR;
//                                else if (tmpv==2) my_para.method_match_landmarks = MATCH_INTENSITY;
//                                else if (tmpv==3) my_para.method_match_landmarks = MATCH_CORRCOEF;
//                                else my_para.method_match_landmarks = MATCH_MI;
//                                i++;
//                            }
//                            else if (!strcmp(key, "n"))
//                            {
//                                int tmpv = atoi (argv[i+1]);
//                                if (tmpv<0 || tmpv>1)
//                                {
//                                    tmpv = 0;
//                                    printf ("Your choice of for -n option is not between 0 and 1. Force it to be 0.\n");
//                                }
//                                if (tmpv==1) my_para.b_nearest_interp = true;
//                                else my_para.b_nearest_interp = false;
//                                i++;
//                            }
//                            else if (!strcmp(key, "B"))
//                            {
//                                XYPlaneMaxSize = atoi (argv[i+1]);
//                                if (XYPlaneMaxSize <= 0)
//                                {
//                                    fprintf (stderr, "Illeagal XY plane image size for warping found! It must be >0.\n");
//                                    return 1;
//                                }
//                                i++;
//                            }
//                            else if (!strcmp(key, "e"))
//                            {
//                                factor_landmark_coord_resample = atof (argv[i+1]);
//                                if (factor_landmark_coord_resample <= 0)
//                                {
//                                    fprintf (stderr, "Illeagal landmark coordinate multiplcation factor found! It must be >0.\n");
//                                    return 1;
//                                }
//                                i++;
//                            }
//                            else if (!strcmp(key, "F"))
//                            {
//                                dfile_df = argv[i+1]; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "J"))
//                            {
//                                dfile_volumechangemap = argv[i+1]; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "A"))
//                            {
//                                chan_attached_target = atoi (argv[i+1]);
//                                if (chan_attached_target<0)
//                                {
//                                    printf ("Your choice of for -A option is <0. Disable this function.\n");
//                                }
//                                i++;
//                            }
//                            else if (!strcmp(key, "a"))
//                            {
//                                chan_attached_subject = atoi (argv[i+1]);
//                                if (chan_attached_subject<0)
//                                {
//                                    printf ("Your choice of for -a option is <0. Disable this function.\n");
//                                }
//                                i++;
//                            }
//                            else if (!strcmp(key, "T"))
//                            {
//                                b_onlyconvertformat = true; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "Y"))
//                            {
//                                b_convert2uint8 = true; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "R"))
//                            {
//                                b_do_resampling = true; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "v"))
//                            {
//                                my_para.b_verbose_print = 1; //
//                                i++;
//                            }
//                            else if (!strcmp(key, "h"))
//                            {
//                                printHelp();
//                                return true;
//                            }
//                            else
//                            {
//                                cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
//                                return false;
//                            }
                        }
                    }
                    else
                    {
                        cout<<"parsing ..."<<key<<i<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                        return false;
                    }
                }
            }

        } // parameters


        //
        if (!dfile_subject || !dfile_warped)
        {
            printf("You have not specified the subject image or the output image. Do nothing.\n");
            return 0;
        }

        b_attach_channel_to_subject = (chan_attached_subject>=0 && chan_attached_target>=0) ? true : false;

        // some necessary variables.

        unsigned char * img_target = 0; /* note that this variable must be initialized as NULL. */
        V3DLONG * sz_target = 0; /* note that this variable must be initialized as NULL. */
        int datatype_target = 0;

        unsigned char * img_subject = 0; /* note that this variable must be initialized as NULL. */
        V3DLONG * sz_subject = 0; /* note that this variable must be initialized as NULL. */
        int datatype_subject = 0;

        unsigned char * img_warped = 0;
        V3DLONG * sz_warped = 0;
        int datatype_warped = 0;

        //if only convert file format. then just do it

        if (b_onlyconvertformat==true)
        {
            if (loadImage(dfile_subject, img_subject, sz_subject, datatype_subject)!=true)
            {
                fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", dfile_subject);

                if (img_subject) {delete [] img_subject; img_subject=0;}
                if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                return false;
            }

            if (b_attach_channel_to_subject)
            {
                if (loadImage(dfile_target, img_target, sz_target, datatype_target)!=true)
                {
                    fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", dfile_target);

                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    return false;
                }

                if (!img_subject || !img_target || sz_subject[0]!=sz_target[0] || sz_subject[1]!=sz_target[1] || sz_subject[2]!=sz_target[2]
                        || datatype_subject!=datatype_target
                        || chan_attached_subject>sz_subject[3] || chan_attached_target>=sz_target[3])
                {
                    fprintf(stderr, "******************** Error *************** \n Try to run convert format & attaching channel function for invalid input images \n (image sizes may not match, datatype may not be the same, attached channel exceed the valid range, etc). Do nothing.\n");
                    fprintf(stderr, "sz_subject = %ld %ld %ld %ld  datatype_subject=%d  chan_attached_subject=%d\n", sz_subject[0], sz_subject[1], sz_subject[2], sz_subject[3], datatype_subject, chan_attached_subject);
                    fprintf(stderr, "sz_target = %ld %ld %ld %ld  datatype_target=%d  chan_attached_target=%d \n", sz_target[0], sz_target[1], sz_target[2], sz_target[3], datatype_target, chan_attached_target);

                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    if (img_subject) {delete [] img_subject; img_subject=0;}
                    if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                    return false;
                }
                else
                {
                    V3DLONG chansz = sz_subject[0]*sz_subject[1]*sz_subject[2];
                    V3DLONG subject_sz_old[4]; for (int ii=0; ii<4; ii++) subject_sz_old[ii] = sz_subject[ii];
                    if (sz_subject[3]<=chan_attached_subject) sz_subject[3]=chan_attached_subject+1;

                    unsigned char * output_chan_head = 0;
                    if (chan_attached_subject < subject_sz_old[3])
                        output_chan_head = img_subject + chan_attached_subject*chansz*datatype_subject;
                    else {
                        unsigned char *output_img = new unsigned char [sz_subject[3]*chansz*datatype_subject];
                        memcpy(output_img, img_subject, subject_sz_old[3]*chansz*datatype_subject);

                        if (img_subject) {delete []img_subject; img_subject=0;}
                        img_subject = output_img;

                        output_chan_head = img_subject + chan_attached_subject*chansz*datatype_subject;
                    }

                    unsigned char * target_chan_head = img_target + chan_attached_target*chansz*datatype_target;
                    memcpy(output_chan_head, target_chan_head, chansz*datatype_target);
                }
            }


            if (img_subject && sz_subject)
            {
                switch (datatype_subject)
                {
                case 1:
                    if (saveImage(dfile_warped, (const unsigned char *)img_subject, sz_subject, sizeof(unsigned char))!=true)
                    {
                        fprintf(stderr, "Error happens in file writing. Exit. \n");
                    }
                    break;

                case 2:
                    if (saveImage(dfile_warped, (const unsigned char *)img_subject, sz_subject, 2)!=true)
                    {
                        fprintf(stderr, "Error happens in file writing. Exit. \n");
                    }
                    break;

                case 4:
                    if (saveImage(dfile_warped, (const unsigned char *)img_subject, sz_subject, 4)!=true)
                    {
                        fprintf(stderr, "Error happens in file writing. Exit. \n");
                    }
                    break;

                default:
                    fprintf(stderr, "Something wrong with the program, -- should NOT display this message at all. Check your program. \n");

                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    if (img_subject) {delete [] img_subject; img_subject=0;}
                    if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                    if (img_warped) {delete [] img_warped; img_warped=0;}
                    if (sz_warped) {delete [] sz_warped; sz_warped=0;}

                    return false;
                }
                printf("The output image has been saved to the file [%s].\n", dfile_warped);
            }

            printf("BrainAligner has converted the format of subject file [%s] and save to the file [%s].\n", dfile_subject, dfile_warped);

            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}

            return false;
        }



        //
        if (img_target) {delete [] img_target; img_target=0;}
        if (sz_target) {delete [] sz_target; sz_target=0;}

        if (img_subject) {delete [] img_subject; img_subject=0;}
        if (sz_subject) {delete [] sz_subject; sz_subject=0;}

        if (img_warped) {delete [] img_warped; img_warped=0;}
        if (sz_warped) {delete [] sz_warped; sz_warped=0;}

    }
    else if(func_name == tr("help"))
    {
        cout<<endl<<"Vaa3D KLB-Plugin"<<endl;
        printHelp();
    }
    else
    {
        cout<<"Invalid function specified."<<endl;
        return false;
    }

    return true;
}

