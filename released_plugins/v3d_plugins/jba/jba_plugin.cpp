/* jba_plugin.cpp
 * 2015-4-13 : by Yang Yu (yuy@janelia.hhmi.org)
 */

#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "jba_plugin.h"
#include "jba_mainfunc.h"

#include "stackutil.h"
#include "volimg_proc.h"

#include "convert_type2uint8.h"

using namespace std;
Q_EXPORT_PLUGIN2(jba, JBAPlugin);

void printHelp()
{
    char JBA_VERSION[128] = "1.000";
    printf("\n===================================================================================================\n");
    printf("\nUsage: ./vaa3d -x brainaligner -f brainaligner -p \"#t <input_target_image> #s <input_subject_image> #o <aligned/warped_subject_image> #w <registration_type> ... <other_arguments>\"\n");
    printf("\n");
    printf("\t [-t <input_target_image>]          .raw/.tif/.lsm file containing 3D template/target stack to which a subject image will be warped/mapped. The .raw file is in V3D's RAW format.\n");
    printf("\t [-C <target_ref_channel>]          the ID of the reference channel of the target image (default = 0). \n");
    printf("\t [-X <target_xy_pixel_sz>]          target image: X/Y direction pixel size (resolution) in microns (or relative values). Assume the X and Y pixel size are always the same. \n");
    printf("\t [-Z <subject_z_pixel_sz>]          target image: Z direction resolution in microns (or relative values). \n");
    printf("\t [-L <landmark_file_target>]        a plain text marker file (in V3D's .marker format) containing target image landmarks (if not specified, JBA defines these landmarks automatically based on the target image's content).\n");
    printf("\n");
    printf("\t -s <input_subject_image>           .raw/.tif/.lsm file containing 3D subject which will deform/warp to best appriximate the geometry of the template/target. The .raw file is in V3D's RAW format.\n");
    printf("\t [-c <subject_ref_channel>]         the ID of the reference channel of the subject image (default = 0) which is used in computing the registration and displacement field. All other channels will use the same transform/warp.\n");
    printf("\t [-x <subject_xy_pixel_sz]          subject image: X/Y direction pixel size (resolution) in microns (or relative values). Assume the X and Y pixel size are always the same. \n");
    printf("\t [-z <subject_xy_pixel_sz]          subject image: Z direction resolution in microns (or relative values). \n");
    printf("\t [-l <landmark_file_subject>]       a plain text marker file (in V3D's .marker format) containing subject image landmarks. If un-specified, then JBA searches the best corresponding landmarks using the RLM algoriothm.\n");
    printf("\n");
    printf("\t -o <aligned/warped_subject_image>  .raw/.tif file containing the aligned result stack. (Highly recommended to use V3D's RAW format)\n");
    printf("\n");
    printf("\t -w <registration_type>             the type of registration:\n");
    printf("\t                                         0 -- global\n");
    printf("\t                                        10 -- local\n");
    printf("\t [-H hier_level]                    hierarchical matching level. \n");
    printf("\t                                        Default is 1, i.e. no matching at downsampled domain. \n");
    printf("\t                                        If set to >=2, then first search the matching points on downsampled images and then refine the search at the original resolution. \n");
    printf("\t                                        If the image size is 1024x1024xN, recommend to use '-H 2' for faster matching. \n");
    printf("\t [-j <landmark_match_method>]       landmark matching method:\n");
    printf("\t                                        0 -- MI\n");
    printf("\t                                        1 -- RLM (MI+Intensity+CORR) (default)\n");
    printf("\t                                        2 -- Intensity\n");
    printf("\t                                        3 -- CORR\n");
    printf("\t [-i <displacement_field_method>]   displacement generation method:\n");
    printf("\t                                        0 -- TPS\n");
    printf("\t                                        1 -- TPS-linear (default)\n");
    printf("\t                                        2 -- TPS-B_spline\n");
    printf("\t                                        3 -- B_spline\n");
    printf("\t [-n <interpolation_method>]        interpolation method used in warping:\n");
    printf("\t                                        0 -- linear interpolation (default)\n");
    printf("\t                                        1 -- nearest neighbor\n");
    printf("\t [-B <xy_plane_size>]               XY plane size of image mapping (default = 512, can be 1024, 2048, or 256. Note: if use 1024 and above, should run on a machine with a lot of memory). \n");
    printf("\t [-e <m_factor>]                    a multiplication factor for control points in an existing landmark file (e.g. if image size is 1024, and existing landmarks are defined using 512 image, then use -e 2. Default is 1.). \n");
    printf("\n");
    printf("\t [-F <displacement_field_file>]     output the displacement field to file(s). V3D's RAW format files (4-byte float) containing the displacement field (DF) will be produced in separate files with _sx, _sy, and _sz tags. \n");
    printf("\t                                        Note: This file name is just a basename for the separate files for the three dimensions. \n");
    printf("\t [-J <volume_expand_shrink_map>]    output the volume change map (a single-channel float32 image) for each voxel in the reference channel.\n");
    printf("\n");
    printf("\t [-A <target_chan_to_attach>]       attach (i.e. copy the content) a channel of the target image to a channel of the 'aligned' subject. Have no effect unless both -A and -a are set. \n");
    printf("\t [-a <subject_chan_to_attach>]      attach a channel of the target image to REPLACE a channel of the subject in the output aligned image. This option specifies the channel of the subject image to copy to. Have no effect unless both -A and -a are set\n");
    printf("\n");
    printf("\t [-T]                               only convert the file format of the -s file to the -o file (if they have the same suffixes, then just a save as). \n");
    printf("\t                                        Note: if attaching a channel from a target image to the suvject image and save to an output image, then -T must be used. \n");
    printf("\n");
    printf("\t [-Y]                               force data conversion to UINT8 before processing.\n");
    printf("\t [-R]                               do image resampling to 0.58/0.8.\n");
    printf("\n");
    printf("\t [-v]                               verbose printing enabled. \n");
    printf("\t [-h]                               print this message.\n");
    printf("\n");
    printf("\t BrainAligner version:              %s (Copyright: Hanchuan Peng, 2006-2011).\n", JBA_VERSION);
    printf("\n===================================================================================================\n\n");
    return;
}

QStringList JBAPlugin::menulist() const
{
    return QStringList()
            <<tr("about");
}

QStringList JBAPlugin::funclist() const
{
    return QStringList()
            <<tr("brainaligner")
           <<tr("help");
}

void JBAPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
    {
        v3d_msg(tr("This is a plugin of Janelia Brain Aligner (Hanchuan2011).\n"
                   "More details can be found at http://penglab.janelia.org/proj/brainaligner/BrainAligner/Download.html\n"
                   "Developed by Yang Yu (yuy@janelia.hhmi.org), 2015-6-1"));
    }
}

bool JBAPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("brainaligner"))
    {
        if(input.size()<1 || (input.size()==1 && output.size()<1) ) // no inputs
        {
            //print Help info
            cout<<endl<<"Vaa3D JBA-Plugin"<<endl;
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

        BasicWarpParameter my_para;

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
                            if (!strcmp(key, "t"))
                            {
                                dfile_target = argv[i+1];
                                i++;
                            }
                            else if (!strcmp(key, "C"))
                            {
                                channelNo_target = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "X"))
                            {
                                target_xy_rez = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "Z"))
                            {
                                target_z_rez = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "L"))
                            {
                                dfile_landmark_target = argv[i+1]; //
                                my_para.file_landmark_target = dfile_landmark_target;
                                i++;
                            }
                            else if (!strcmp(key, "s"))
                            {
                                dfile_subject = argv[i+1]; //
                                i++;
                            }
                            else if (!strcmp(key, "c"))
                            {
                                channelNo_subject = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "x"))
                            {
                                subject_xy_rez = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "z"))
                            {
                                subject_z_rez = atof( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "l"))
                            {
                                dfile_landmark_subject = argv[i+1]; //
                                my_para.file_landmark_subject = dfile_landmark_subject;
                                i++;
                            }
                            else if (!strcmp(key, "o"))
                            {
                                dfile_warped = argv[i+1];
                                my_para.tag_output_image_file = dfile_warped;
                                i++;
                            }
                            else if (!strcmp(key, "w"))
                            {
                                warpType = atoi( argv[i+1] ); //
                                i++;
                            }
                            else if (!strcmp(key, "H"))
                            {
                                int tmpv = atoi (argv[i+1]);
                                if (tmpv<1 || tmpv>=4)
                                {
                                    tmpv = 1;
                                    printf ("Your choice of for -H option is not between 1 and 4. Force it to be 1.\n");
                                }
                                my_para.hierachical_match_level = tmpv;
                                i++;
                            }
                            else if (!strcmp(key, "i"))
                            {
                                int tmpv = atoi (argv[i+1]);
                                if (tmpv<0 || tmpv>=3)
                                {
                                    tmpv = 1;
                                    printf ("Your choice of for -i option is not between 0 and 3. Force it to be 1.\n");
                                }
                                if (tmpv==0) my_para.method_df_compute = DF_GEN_TPS;
                                else if (tmpv==1) my_para.method_df_compute = DF_GEN_TPS_LINEAR_INTERP;
                                else my_para.method_df_compute = DF_GEN_TPS;
                                i++;
                            }
                            else if (!strcmp(key, "j"))
                            {
                                int tmpv = atoi (argv[i+1]);
                                if (tmpv<0 || tmpv>=3)
                                {
                                    tmpv = 1;
                                    printf ("Your choice of for -j option is not between 0 and 3. Force it to be 1.\n");
                                }
                                if (tmpv==0) my_para.method_match_landmarks = MATCH_MI;
                                else if (tmpv==1) my_para.method_match_landmarks = MATCH_MULTIPLE_MI_INT_CORR;
                                else if (tmpv==2) my_para.method_match_landmarks = MATCH_INTENSITY;
                                else if (tmpv==3) my_para.method_match_landmarks = MATCH_CORRCOEF;
                                else my_para.method_match_landmarks = MATCH_MI;
                                i++;
                            }
                            else if (!strcmp(key, "n"))
                            {
                                int tmpv = atoi (argv[i+1]);
                                if (tmpv<0 || tmpv>1)
                                {
                                    tmpv = 0;
                                    printf ("Your choice of for -n option is not between 0 and 1. Force it to be 0.\n");
                                }
                                if (tmpv==1) my_para.b_nearest_interp = true;
                                else my_para.b_nearest_interp = false;
                                i++;
                            }
                            else if (!strcmp(key, "B"))
                            {
                                XYPlaneMaxSize = atoi (argv[i+1]);
                                if (XYPlaneMaxSize <= 0)
                                {
                                    fprintf (stderr, "Illeagal XY plane image size for warping found! It must be >0.\n");
                                    return 1;
                                }
                                i++;
                            }
                            else if (!strcmp(key, "e"))
                            {
                                factor_landmark_coord_resample = atof (argv[i+1]);
                                if (factor_landmark_coord_resample <= 0)
                                {
                                    fprintf (stderr, "Illeagal landmark coordinate multiplcation factor found! It must be >0.\n");
                                    return 1;
                                }
                                i++;
                            }
                            else if (!strcmp(key, "F"))
                            {
                                dfile_df = argv[i+1]; //
                                i++;
                            }
                            else if (!strcmp(key, "J"))
                            {
                                dfile_volumechangemap = argv[i+1]; //
                                i++;
                            }
                            else if (!strcmp(key, "A"))
                            {
                                chan_attached_target = atoi (argv[i+1]);
                                if (chan_attached_target<0)
                                {
                                    printf ("Your choice of for -A option is <0. Disable this function.\n");
                                }
                                i++;
                            }
                            else if (!strcmp(key, "a"))
                            {
                                chan_attached_subject = atoi (argv[i+1]);
                                if (chan_attached_subject<0)
                                {
                                    printf ("Your choice of for -a option is <0. Disable this function.\n");
                                }
                                i++;
                            }
                            else if (!strcmp(key, "T"))
                            {
                                b_onlyconvertformat = true; //
                                i++;
                            }
                            else if (!strcmp(key, "Y"))
                            {
                                b_convert2uint8 = true; //
                                i++;
                            }
                            else if (!strcmp(key, "R"))
                            {
                                b_do_resampling = true; //
                                i++;
                            }
                            else if (!strcmp(key, "v"))
                            {
                                my_para.b_verbose_print = 1; //
                                i++;
                            }
                            else if (!strcmp(key, "h"))
                            {
                                printHelp();
                                return true;
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

        Vol3DSimple<unsigned short int> *img_target_matchrange = 0;
        Vol3DSimple<unsigned short int> *img_subject_matchrange = 0;

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

        if (!dfile_target)
        {
            printf("You have not specified the target image. Do nothing.\n");
            return 0;
        }
        if (loadImage(dfile_target, img_target, sz_target, datatype_target)!=true)
        {
            fprintf (stderr, "Error happens in reading the target file [%s]. Exit. \n", dfile_target);
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}

            return false;
        }

        if (b_convert2uint8 && datatype_target!=1)
        {
            if (datatype_target==2 || datatype_target==4)
            {
                fprintf (stdout, "Now try to convert target image datatype from [%d]bytes per pixel to 1byte per pixel.\n", datatype_target);
                convert_type2uint8_3dimg_1dpt((void * &)img_target, sz_target, datatype_target);
                datatype_target=1;
            }
            else
            {
                fprintf (stderr, "The datatype of the target image cannot be converted to UINT8. Exit. \n");
                if (img_target) {delete [] img_target; img_target=0;}
                if (sz_target) {delete [] sz_target; sz_target=0;}

                if (img_subject) {delete [] img_subject; img_subject=0;}
                if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                if (img_warped) {delete [] img_warped; img_warped=0;}
                if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                return false;
            }
        }

        dfactor_target = 1;
        if (sz_target[0]>XYPlaneMaxSize || sz_target[1]>XYPlaneMaxSize)
        {
            dfactor_target = (double)(sz_target[0]>sz_target[1]?sz_target[0]:sz_target[1])/XYPlaneMaxSize;
            if (datatype_target==1)
                downsample3dimg_1dpt(img_target, sz_target, dfactor_target);
            else if (datatype_target==2)
                downsample3dimg_1dpt((unsigned short int * &)img_target, sz_target, dfactor_target);
            else if (datatype_target==4)
                downsample3dimg_1dpt((float * &)img_target, sz_target, dfactor_target);
            else
            {
                fprintf (stderr, "The datatype of the target is invalid. Exit. \n");
                if (img_target) {delete [] img_target; img_target=0;}
                if (sz_target) {delete [] sz_target; sz_target=0;}

                if (img_subject) {delete [] img_subject; img_subject=0;}
                if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                if (img_warped) {delete [] img_warped; img_warped=0;}
                if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                return false;
            }
        }

        if (datatype_target==1)
            reslice_Z(img_target, sz_target, target_xy_rez*dfactor_target, target_z_rez, 1);
        else if (datatype_target==2)
            reslice_Z((unsigned short int * &)img_target, sz_target, target_xy_rez*dfactor_target, target_z_rez, 1);
        else if (datatype_target==4)
            reslice_Z((float * &)img_target, sz_target, target_xy_rez*dfactor_target, target_z_rez, 1);
        else
        {
            fprintf (stderr, "The datatype of the target is invalid. Exit. \n");
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}
            return false;
        }

        my_para.targetMultiplyFactor.x = factor_landmark_coord_resample/dfactor_target;
        my_para.targetMultiplyFactor.y = factor_landmark_coord_resample/dfactor_target;
        my_para.targetMultiplyFactor.z = factor_landmark_coord_resample*target_z_rez/(target_xy_rez*dfactor_target);

        if (loadImage(dfile_subject, img_subject, sz_subject, datatype_subject)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", dfile_subject);
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}
            return false;
        }

        if (b_convert2uint8 && datatype_subject!=1)
        {
            if (datatype_subject==2 || datatype_subject==4)
            {
                fprintf (stdout, "Now try to convert subject image datatype from [%d]bytes per pixel to 1byte per pixel.\n", datatype_subject);
                convert_type2uint8_3dimg_1dpt((void * &)img_subject, sz_subject, datatype_subject);
                datatype_subject=1;
            }
            else
            {
                fprintf (stderr, "The datatype of the subject image cannot be converted to UINT8. Exit. \n");
                if (img_target) {delete [] img_target; img_target=0;}
                if (sz_target) {delete [] sz_target; sz_target=0;}

                if (img_subject) {delete [] img_subject; img_subject=0;}
                if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                if (img_warped) {delete [] img_warped; img_warped=0;}
                if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                return false;
            }
        }

        //resample to 0.58/0.8. for some labs' data
        if (b_do_resampling)
        {
            double dfactor_x = 0.58/subject_xy_rez; if (dfactor_x<1) dfactor_x=1;
            double dfactor_y = 0.58/subject_xy_rez; if (dfactor_y<1) dfactor_y=1;
            double dfactor_z = 0.8/subject_z_rez;   if (dfactor_z<1) dfactor_z=1;
            int interp_method=1;
            if (dfactor_x>1 || dfactor_z>1)
            {
                bool res_resampling = false;
                if (datatype_subject==1)
                    res_resampling = resample3dimg_interp(img_subject, sz_subject, dfactor_x, dfactor_y, dfactor_z, interp_method);
                else if (datatype_subject==2)
                    res_resampling = resample3dimg_interp((unsigned short int * &)img_subject, sz_subject, dfactor_x, dfactor_y, dfactor_z, interp_method);
                else if (datatype_subject==4)
                    res_resampling = resample3dimg_interp((float * &)img_subject, sz_subject, dfactor_x, dfactor_y, dfactor_z, interp_method);
                else
                {
                    fprintf (stderr, "The datatype of the subject is invalid. Exit. \n");
                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    if (img_subject) {delete [] img_subject; img_subject=0;}
                    if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                    if (img_warped) {delete [] img_warped; img_warped=0;}
                    if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                    return false;
                }

                if (!res_resampling)
                {
                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    if (img_subject) {delete [] img_subject; img_subject=0;}
                    if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                    if (img_warped) {delete [] img_warped; img_warped=0;}
                    if (sz_warped) {delete [] sz_warped; sz_warped=0;}

                    return false;
                }
                if (dfactor_z>1) subject_z_rez = 0.8;
                if (dfactor_x>1) subject_xy_rez = 0.58;
                fprintf (stdout, "Now have resampled the subject image data to pixel size 0.58/0.8um.\n");
            }
            else
            {
                fprintf (stdout, "After analysis it is found that the resampling is not needed for the subject image data.\n");
                b_do_resampling = false;
            }
        }

        dfactor_subject = 1;
        if (sz_subject[0]>XYPlaneMaxSize || sz_subject[1]>XYPlaneMaxSize )
        {
            fprintf (stdout, "The subject image has a large XY dimension, and will be downsampled now.\n");

            dfactor_subject = (double)(sz_subject[0]>sz_subject[1]?sz_subject[0]:sz_subject[1])/XYPlaneMaxSize;
            if (datatype_subject==1)
                downsample3dimg_1dpt(img_subject, sz_subject, dfactor_subject);
            else if (datatype_subject==2)
                downsample3dimg_1dpt((unsigned short int * &)img_subject, sz_subject, dfactor_subject);
            else if (datatype_subject==4)
                downsample3dimg_1dpt((float * &)img_subject, sz_subject, dfactor_subject);
            else
            {
                fprintf (stderr, "The datatype of the subject is invalid. Exit. \n");
                if (img_target) {delete [] img_target; img_target=0;}
                if (sz_target) {delete [] sz_target; sz_target=0;}

                if (img_subject) {delete [] img_subject; img_subject=0;}
                if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                if (img_warped) {delete [] img_warped; img_warped=0;}
                if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                return false;
            }
        }

        if (datatype_subject==1)
            reslice_Z(img_subject, sz_subject, subject_xy_rez*dfactor_subject, subject_z_rez, 1);
        else if (datatype_subject==2)
            reslice_Z((unsigned short int * &)img_subject, sz_subject, subject_xy_rez*dfactor_subject, subject_z_rez, 1);
        else if (datatype_subject==4)
            reslice_Z((float * &)img_subject, sz_subject, subject_xy_rez*dfactor_subject, subject_z_rez, 1);
        else
        {
            fprintf (stderr, "The datatype of the subject is invalid. Exit. \n");
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}
            return false;
        }

        my_para.subjectMultiplyFactor.x = factor_landmark_coord_resample/dfactor_subject;
        my_para.subjectMultiplyFactor.y = factor_landmark_coord_resample/dfactor_subject;
        my_para.subjectMultiplyFactor.z = factor_landmark_coord_resample*subject_z_rez/(subject_xy_rez*dfactor_subject);

        printf("After preprocessing: \n Size of subject_image = [%d %d %d %d], size of target_image = [%d %d %d %d].\n",
               sz_subject[0], sz_subject[1], sz_subject[2], sz_subject[3],
                sz_target[0], sz_target[1], sz_target[2], sz_target[3]);

        if (channelNo_subject >= sz_subject[3] || channelNo_target >= sz_target[3])
        {
            fprintf (stderr, "The reference channelNo of either subject image or target image is (or both are) invalid (bigger than the number of channels the images have). Exit. \n");
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}
            return false;
        }

        // do the computation
        printf("\n-------------------------------------------------\n");
        printf("Target   file: [%s]\n", dfile_target);
        printf("Target    rez: [%5.4f][%5.4f][%5.4f]\n", target_xy_rez, target_xy_rez, target_z_rez);
        printf("Ref chanl tgt: [%d]\n", channelNo_target);
        printf("Landmark tgt:  [%s]\n", dfile_landmark_target);
        printf("Subject  file: [%s]\n", dfile_subject);
        printf("Subject   rez: [%5.4f][%5.4f][%5.4f]\n", subject_xy_rez, subject_xy_rez, subject_z_rez);
        printf("Warp res file: [%s]\n", dfile_warped);
        printf("Warp img size: [%d]\n", XYPlaneMaxSize);
        printf("Ref chanl sbj: [%d]\n", channelNo_subject);
        printf("Landmark sbj:  [%s]\n", dfile_landmark_subject);
        printf("Align G/L type: [%s]\n", (warpType==0)?"Global":"Local");
        printf("Hierarchical landmark search level: [%d]\n", int(my_para.hierachical_match_level));
        printf("Landmark matching method: [%d]\n", int(my_para.method_match_landmarks));
        printf("Displacement field producing method: [%d]\n", int(my_para.method_df_compute));
        printf("Use nearest neighbor interpolation method: [%d]\n", (my_para.b_nearest_interp)?1:0);
        printf("Displacement field file: [%s]\n", dfile_df);
        printf("Convert to UINT8 for processing: [%s]\n", (b_convert2uint8)?"Yes":"No");
        printf("Resample to pixel size 0.58um for processing: [%s]\n", (b_do_resampling)?"Yes":"No");
        printf("Volume change map file: [%s]\n", dfile_volumechangemap);
        printf("Target copy channel (for attaching mask): [%d]\n", chan_attached_target);
        printf("Subject paste channel (for attaching mask): [%d]\n", chan_attached_subject);
        printf("Verbose print: [%d]\n", my_para.b_verbose_print);
        printf("---------------------------------------------------\n\n");

        datatype_warped = datatype_subject;

        my_vcmap_file = (!dfile_volumechangemap) ? string("") : string(dfile_volumechangemap);

        if (do_jba_core_computation( img_target,
                                     sz_target,
                                     datatype_target,
                                     string(dfile_target),
                                     channelNo_target,
                                     img_subject,
                                     sz_subject,
                                     datatype_subject,
                                     string(dfile_subject),
                                     channelNo_subject,
                                     img_warped,
                                     sz_warped,
                                     datatype_warped,
                                     string(dfile_warped),
                                     my_vcmap_file,
                                     string((!dfile_df)?"":dfile_df),
                                     warpType,
                                     my_para)==false)
        {
            fprintf(stderr, "Some error happened in the computation. Exit.\n");
            if (img_target) {delete [] img_target; img_target=0;}
            if (sz_target) {delete [] sz_target; sz_target=0;}

            if (img_subject) {delete [] img_subject; img_subject=0;}
            if (sz_subject) {delete [] sz_subject; sz_subject=0;}

            if (img_warped) {delete [] img_warped; img_warped=0;}
            if (sz_warped) {delete [] sz_warped; sz_warped=0;}
            return false;
        }


        if (img_warped && sz_warped && b_same_sz_warp_target)
        {
            V3DLONG tmpi;
            bool b_need_resize=false;
            for (tmpi=0;tmpi<3;tmpi++)
            {
                if (sz_warped[tmpi]!=sz_target[tmpi])
                {
                    b_need_resize=true;
                    break;
                }
            }

            if (b_need_resize)
            {
                printf("Now fit the warped image [pointer=%p] with size [%ld, %ld, %ld, %ld] to the target size of [%ld, %ld, %ld, %ld]\n",
                       img_warped, sz_warped[0], sz_warped[1], sz_warped[2], sz_warped[3], sz_target[0], sz_target[1], sz_target[2], sz_target[3]);
                if (datatype_warped==1)
                    fit_to_cube(img_warped, sz_warped, sz_target);
                else if (datatype_warped==2)
                    fit_to_cube((unsigned short int * &)img_warped, sz_warped, sz_target);
                else if (datatype_warped==4)
                    fit_to_cube((float * &)img_warped, sz_warped, sz_target);
                else
                {
                    fprintf (stderr, "The datatype of the output image is invalid. Exit. \n");
                    if (img_target) {delete [] img_target; img_target=0;}
                    if (sz_target) {delete [] sz_target; sz_target=0;}

                    if (img_subject) {delete [] img_subject; img_subject=0;}
                    if (sz_subject) {delete [] sz_subject; sz_subject=0;}

                    if (img_warped) {delete [] img_warped; img_warped=0;}
                    if (sz_warped) {delete [] sz_warped; sz_warped=0;}
                    return false;
                }
            }
        }

        if (img_warped && sz_warped && dfile_warped)
        {
            printf("Now save the image [pointer=%p] datatype=%d sz=[%ld, %ld, %ld, %ld]\n", img_warped, datatype_warped, sz_warped[0], sz_warped[1], sz_warped[2], sz_warped[3]);

            switch (datatype_warped)
            {
            case 1:
                if (saveImage(dfile_warped, (const unsigned char *)img_warped, sz_warped, sizeof(unsigned char))!=true)
                {
                    fprintf(stderr, "Error happens in 8bit file writing. Exit. \n");
                }
                break;

            case 2:
                if (saveImage(dfile_warped, (const unsigned char *)img_warped, sz_warped, 2)!=true)
                {
                    fprintf(stderr, "Error happens in 16bit file writing. Exit. \n");
                }
                break;

            case 4:
                if (saveImage(dfile_warped, (const unsigned char *)img_warped, sz_warped, 4)!=true)
                {
                    fprintf(stderr, "Error happens in 32bit file writing. Exit. \n");
                }
                break;

            default:
                fprintf(stderr, "Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
                return false;
            }
            printf("The image has been saved to the file [%s].\n", dfile_warped);
            return true;
        }
        else
        {
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
        cout<<endl<<"Vaa3D JBA-Plugin"<<endl;
        printHelp();
    }
    else
    {
        cout<<"Invalid function specified."<<endl;
        return false;
    }

    return true;
}

