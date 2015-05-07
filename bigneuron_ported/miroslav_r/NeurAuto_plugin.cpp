/* NeurAuto_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-5-5 : by YourName
 */
 
#include <math.h>
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "toolbox.h"

#include "NeurAuto_plugin.h"
Q_EXPORT_PLUGIN2(NeurAuto, TestPlugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    int     diam;

};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("neurauto_menu")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("neurauto_func")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neurauto_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;

        //input throught menu
        bool ok1, ok2;
        long channel = 1;
        int  diam      = 8;

        PARA.channel = QInputDialog::getInteger(parent, "Channel ", "Enter channel:", 1, 1, channel, 1, &ok1);

        if(ok1)
        {
            PARA.diam = QInputDialog::getInteger(parent, "Neuron diameter ", "Neurite diameter in pix:", diam, 4, diam, 1, &ok2);
        }
        else
            return;

//        if(ok2)
//        {
//            Wz = QInputDialog::getInteger(parent, "Window Z", "Enter radius (window size is 2*radius+1):", 1, 1, P, 1, &ok3);
//        }
//        else
//            return;

        printf("-------------------------------\n");
        printf("channel = %d\n", PARA.channel);
        printf("diam = %d\n", PARA.diam);
        printf("-------------------------------\n");

        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2015-5-5"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("neurauto_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];

        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        PARA.diam = (paras.size() >= k+1) ? atoi(paras[k]) : 4;  k++;

        printf("-------------------------------\n");
        printf("channel = %d\n", PARA.channel);
        printf("diam = %d\n", PARA.diam);
        printf("-------------------------------\n");

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN
		printf("**** Usage of NeurAuto tracing **** \n");
		printf("vaa3d -x NeurAuto -f neurauto_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* inimg = 0; // image is loaded into 1d array
    V3DLONG W,H,L,sc,ch;
    V3DLONG in_sz[4];
    int datatype = 0;

    if(bmenu){ // through menu
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

        inimg = p4DImage->getRawData();
        W = p4DImage->getXDim();
        H = p4DImage->getYDim();
        L = p4DImage->getZDim();
        sc = p4DImage->getCDim();
        ch = PARA.channel;
        PARA.inimg_file = p4DImage->getFileName();
        in_sz[0] = W;
        in_sz[1] = H;
        in_sz[2] = L;
        in_sz[3] = sc;
    }
    else
    { // command

        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), inimg, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel != 1)
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        W = in_sz[0];
        H = in_sz[1];
        L = in_sz[2];
        sc = in_sz[3];
        ch = PARA.channel;
    }

    //main neuron reconstruction code
    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    printf("\n\t...reconstruction func...\n");

    int diam = PARA.diam;  //
    unsigned char bth = 0; // don't consider zeros

    printf("input image: %d x %d x %d \t channel: %d \n", W, H, L, ch);
    long size = W*H*L;

    unsigned char * scr; // score
    try {scr = new unsigned char [size];}
    catch(...)  {v3d_msg("cannot allocate memory for scr."); return;}

    unsigned char * nbhood;
    try{nbhood = new unsigned char [(2*diam+1)*(2*diam+1)];}
    catch(...) {v3d_msg("cannot allocate memory for nbhood."); return;}

    for(long i = 0; i < size; i++)
    {

        int x  = i % W;
        int z  = i / (W*H);
        int y  = i /  W - z * H;
//        int ii = z*(W*H)+y*W+x;

        if (x>=diam && y<W-diam && inimg[i]>bth) {
//            printf("%d\t---> %d, %d, %d \t--> %d, \t %d \n", i, x, y, z, ii, inimg[i]);
            int cnt = 0;
            for (int xx = x-diam; xx <= x+diam; ++xx)
            {
                for (int yy = y-diam; yy <= y+diam; ++yy)
                {
                    if(cnt >= (2*diam+1)*(2*diam+1))
//                        printf("=bug! exceed nbhood alloc %d \tdiam = %d \n", cnt, (2*diam+1)*(2*diam+1));
//                    if(z*(W*H)+yy*W+xx >= size)
//                        printf("=bug! exceed inimg alloc\n");
                    nbhood[cnt] = inimg[z*(W*H)+yy*W+xx];
                    cnt++;
                }
            }

            unsigned char m05 = 0;
            unsigned char m95 = 0;
            m05 = quantile(nbhood, (2*diam+1)*(2*diam+1), 1,  20);
            m95 = quantile(nbhood, (2*diam+1)*(2*diam+1), 19, 20);
            scr[i] = m95-m05;

        }
        else
            scr[i] = 0;

    }

    unsigned char min_th = -1; // since we are working with 8 bit

    // count the number of those that have discrepancy criteria above min_th
    int count = 0;
    for (int i = 0; i < size; i++) {
        if (scr[i]>min_th)
            count++;
    }

    unsigned char * scr_sel;
    scr_sel = new unsigned char [count];
    if(!scr_sel) {printf("error: bad alloc");return;}

    // place them in a reduced score
    count = 0;
    for (int i = 0; i < size; i++) {
        if (scr[i]>min_th) {
            scr_sel[count] = scr[i];
            count++;
        }
    }

    // exclude those that were zero criteria (reduced) when calculating percentile threshold
    unsigned char th = quantile(scr_sel, count, 13, 20);

    unsigned char * fg;
    fg = new unsigned char [size];
    if(!fg) {printf("error: bad alloc");return;}

    int cnt_fg = 0;

    for (int i = 0; i < size; ++i) {
        if (inimg[i]>th) {fg[i] = 255; cnt_fg++;} // perhaps dilatate??
        else fg[i] = 0;
    }
    printf("--->%3.6f \t locs kept\n", ((float)cnt_fg/(float)size)*100.0);

    // delete unnecessary ones
    delete scr;
    delete scr_sel;
    delete nbhood;

    // store mapping i2xyz and xyz2i
//      example
//      int** ary = new int*[sizeY];
//      for(int i = 0; i < sizeY; ++i)
//        ary[i] = new int[sizeX];

    printf("\n\nwill be %d \n\n", cnt_fg);

    int ** i2xyz = new int*[cnt_fg];
    for(int i = 0; i < cnt_fg; ++i)
        i2xyz[i] = new int[3];

    int * xyz2i = new int[size];
    for(int i = 0; i < size; ++i)
        xyz2i[i] = -1;

    cnt_fg = 0;
    for (int i = 0; i < size; ++i) {
        if (fg[i]==255) {

            int xf  = i % W;
            int zf  = i / (W*H);
            int yf  = i /  W - zf * H;

            i2xyz[cnt_fg][0] = xf;
            i2xyz[cnt_fg][1] = yf;
            i2xyz[cnt_fg][2] = zf;

            xyz2i[i] = cnt_fg;

            cnt_fg++;
        }
    }

    /// save fg
    QString fg_path1 = PARA.inimg_file + "_fg.tif";

//    Image4DSimple * im_fg = 0;
//    im_fg = new Image4DSimple;
//    if(!im_fg->createImage(W,H,L,1, V3D_UINT8)) return;
//    memcpy(im_fg->getRawData(), fg, size);
//    im_fg->saveImage(fg_path2);

    printf("\nsaving:\t %s", fg_path1.toStdString().c_str());
    if (!simple_saveimage_wrapper(callback,fg_path1.toStdString().c_str(), fg, in_sz, datatype))
    {
        fprintf (stderr, "Error happens in saving the subject file [%s]. Exit. \n",fg_path1.toStdString().c_str());
        return;
    }
    printf("\tdone saving.\n");

    // zncc outputs
    float * i2zncc     = new float[cnt_fg];
    float * i2sigma    = new float[cnt_fg];
    float * i2vx       = new float[cnt_fg];
    float * i2vy       = new float[cnt_fg];
//    float * i2vz       = new float[cnt_fg];

    // kernels
    // double round(double d){return floor(d + 0.5);}

    int L2 = (int) floor(ceil(diam/2.0) + .5);  // radius/samplingStep
    int LL = 2 * L2 + 1;

    printf("LL=%d,\t diam=%d,\tL2=%d \n", LL, diam, L2);

    // sigmas define
    float sigma_min = 1.0;
    float sigma_step = .5;
    float arcRes = 1.0;
    int nr_sigmas = 0;
    for (float sg = sigma_min; sg <= .4*L2; sg+=sigma_step) nr_sigmas++;
    printf("%d sigmas...\n", nr_sigmas);
    float * sigmas = new float[nr_sigmas];
    nr_sigmas = 0;
    for (float sg = sigma_min; sg <= .4*L2; sg+=sigma_step) sigmas[nr_sigmas++] = sg;

    // form Ndir theta (Ndir directions)
    int Ndir 	= (int) floor(ceil(((3.14*(diam/2.0))/arcRes)) +.5);
    // perhaps better manually define Ndir (covers half of the circle)
    Ndir = 10;
    printf("%d directions...\n", Ndir);

    float ** kernels        = new float*[Ndir*nr_sigmas];
    for(int i = 0; i < Ndir*nr_sigmas; ++i)
        kernels[i] = new float[LL*LL];

    float *  kernels_avg    = new float[Ndir*nr_sigmas];

    float ** kernels_hat    = new float*[Ndir*nr_sigmas];
    for(int i = 0; i < Ndir*nr_sigmas; ++i)
        kernels_hat[i] = new float[LL*LL];

    float * kernels_hat_sum_2   = new float[Ndir*nr_sigmas];

    ////////////////////////////////
    // kernel formation
    ////////////////////////////////
    printf("\nkernel formation");
    for (int i = 0; i < Ndir*nr_sigmas; i++) {

        printf(".");

        int direc_idx = i % Ndir;
        int scale_idx = i / Ndir;

        float sigx = sigmas[scale_idx];
        float sigy = L2;                      // broader than sigmax
        float ang = direc_idx * (3.14 / Ndir);

        float vx =  ((float) cos(ang));
        float vy =  ((float) sin(ang));

        kernels_avg[i] = 0; // average

        for (int j = 0; j < LL*LL; j++) {

            int xx = j % LL;
            int yy = j / LL;

            float currx = (xx - L2) *   vx  + (yy - L2) * vy;
            float curry = (xx - L2) * (-vy) + (yy - L2) * vx;

            kernels[i][j] = (float) exp( -(  ((pow(currx,2)/(2*pow(sigx,2))) + (pow(curry,2)/(2*pow(sigy,2)))   ) )   );
            kernels_avg[i] += kernels[i][j];

        }

        kernels_avg[i] /= (float)(LL*LL);

        kernels_hat_sum_2[i] = 0;

        for (int j = 0; j < LL*LL; j++) {

            kernels_hat[i][j] = kernels[i][j] - kernels_avg[i];
            kernels_hat_sum_2[i] += pow(kernels_hat[i][j], 2);

        }
    }

    // save kernels (todo: check how to save float[] images)
    in_sz[0]=LL; in_sz[1]=LL; in_sz[2]=1; in_sz[2]=1;
    datatype = 4;
    unsigned char * k_temp = new unsigned char[LL*LL];
    for (int ki = 0; ki < Ndir*nr_sigmas; ++ki) {
        QString suffix = QString ("_kernel%1.tif").arg(ki);
        fg_path1 = PARA.inimg_file + suffix;
        printf("\nsaving kernels:\t %s", fg_path1.toStdString().c_str());

        // convert float * to unsigned char *
        for (int kii = 0; kii < LL*LL; ++kii) {
            k_temp[kii] = kernels[ki][kii] * 255;
        }

        if (!simple_saveimage_wrapper(callback,fg_path1.toStdString().c_str(), k_temp, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in saving the kernel [%s]. Exit. \n",fg_path1.toStdString().c_str());
            return;
        }
        printf("\tdone saving kernels.\n");
    }
    delete k_temp;

    printf("done.\n");

    ////////////////////////////////
    // filtering at selected locs
    ////////////////////////////////
    float * vals = new float[LL*LL];
    for (int loci = 0; loci < cnt_fg; ++loci) {

        int at_x = i2xyz[loci][0];
        int at_y = i2xyz[loci][1];
        int at_z = i2xyz[loci][2];

        i2zncc[loci]   = -1; // lower boundary (because we look for the highest)
        i2sigma[loci]  = -1;
        i2vx[loci]     = -1;
        i2vy[loci]     = -1;

        float curr_zncc;
        float vals_avg;

        for (int kidx = 0; kidx < Ndir*nr_sigmas; kidx++) {

            for (int pidx = 0; pidx < LL*LL; pidx++) {

                int dx = (pidx % LL) - L2;
                int dy = (pidx / LL) - L2;

                int xx = at_x+dx;
                int yy = at_y+dy;

                vals[pidx] = 0;
                if (xx>=0 && xx<W && yy>=0 && yy<H) {
                    vals[pidx] = inimg[at_z*(W*H)+yy*W+xx];//_inimg_xy[xx][yy];
                }

            }

            vals_avg = 0;
            for (int pidx = 0; pidx < LL*LL; pidx++) {
//                vals[cnt] = (vals[cnt]-vals_min)/(vals_max-vals_min);
                vals_avg += vals[pidx];
            }
            vals_avg /= (float)(LL*LL);

            // calculate zncc
            curr_zncc = zncc(vals, LL*LL, vals_avg, kernels_hat[kidx], kernels_hat_sum_2[kidx]);

            if (curr_zncc>i2zncc[loci]) {

                i2zncc[loci] = curr_zncc;

                int direc_idx = kidx % Ndir;
                float ang = direc_idx * (3.14 / (float)Ndir);

                float vx = -((float) sin(ang));
                float vy =  ((float) cos(ang));

                i2vx[loci] = vx;
                i2vy[loci] = vy;

                int scale_idx = kidx / Ndir;
                i2sigma[loci] = sigmas[scale_idx];

            }

        }

//        printf("%d", loci);

    } // go through fg. locs

    // save as an image (convert to unsigned char first)
    unsigned char * img_zncc = new unsigned char[size];
    for (int loci = 0; loci < cnt_fg; ++loci) {

        //
        int xx = i2xyz[loci][0];
        int yy = i2xyz[loci][1];
        int zz = i2xyz[loci][2];
        //
        img_zncc[zz*(W*H)+yy*W+xx] = (i2zncc[loci]>=0)? i2zncc[loci]*255 : 0;

    }
//    QString suffix = QString ("_zncc.tif").arg(ki);
    fg_path1 = PARA.inimg_file + "_zncc.tif";
    datatype = 1; // uint8
    in_sz[0] = W; in_sz[1] = H; in_sz[2] = L; in_sz[3] = sc;
    printf("\nsaving:\t %s", fg_path1.toStdString().c_str());
    if (!simple_saveimage_wrapper(callback,fg_path1.toStdString().c_str(), img_zncc, in_sz, datatype))
    {
        fprintf (stderr, "Error happens in saving the subject file [%s]. Exit. \n",fg_path1.toStdString().c_str());
        return;
    }
    printf("\tdone saving %s.\n", fg_path1.toStdString().c_str());

    /////////////////////////////////////////////
    printf("\n---experimental---\n");
    float * aa = new float[4]; aa[0] = 2; aa[1] = 6; aa[2] = 4; aa[3] = 8;
    int * ai = new int[4];

    printf("\nVAL (before) = \t");
    for (int i = 0; i < 4; ++i) {printf("%.1f\t", aa[i]);}

    printf("\nIDX (before) = \t");
    for (int i = 0; i < 4; ++i) {printf("%d\t", ai[i]);} // ai[i] = i;

    descending(aa, 4, ai);

    printf("\nVAL (after) = \t");
    for (int i = 0; i < 4; ++i) {printf("%.1f\t", aa[i]);}

    printf("\nIDX (after) = \t");
    for (int i = 0; i < 4; ++i) {printf("%d\t", ai[i]);}

    printf("\n---experimental---\n");
    /////////////////////////////////////////////

    float zncc_th = 0.8;

    // get locs with high zncc
    int * idesc = new int[cnt_fg];
    descending(i2zncc, cnt_fg, idesc);

    // map
    bool * map = new bool[size];
    for (int ii = 0; ii < size; ++ii) {
        map[ii] = false;
    }

    int nhd = floor(ceil(diam/4.0)+0.5); // roughly radius
    nhd = (nhd<1)? 1 : nhd;
    printf("\nnhd = %d\n", nhd);

    ///// example of usage of std::vector
//    float arr[4];
//    arr[0] = 6.28;
//    arr[1] = 2.50;
//    arr[2] = 9.73;
//    arr[3] = 4.364;
//    std::vector<float*> vec = std::vector<float*>();
//    vec.push_back(arr);
//    float* ptr = vec.front();
//    for (int i = 0; i < 3; i++)
//        printf("%g\n", ptr[i]);

    std::vector<float*> locs_xyz = std::vector<float*>();

    for (int pidx = 0; pidx < cnt_fg; ++pidx) { //

        int get_idx = idesc[pidx];
//        v3d_msg(QString("%1").arg(i2xyz[get_idx][0]));
        int xloc = i2xyz[get_idx][0];
//        v3d_msg(QString("%1").arg(xloc));
        int yloc = i2xyz[get_idx][1];
        int zloc = i2xyz[get_idx][2];

//        v3d_msg("doneaaa reading it!", 0);

        if (i2zncc[pidx]<zncc_th) break;
        else {

            // it is higher then add the location (if the map allows) and fill the map nbhood
//            v3d_msg("test",0);
//            v3d_msg(QString("%1").arg(map[zloc*(W*H)+yloc*W+xloc]), 0);

            if (map[zloc*(W*H)+yloc*W+xloc]==false) {

                float * loc_xyz = new float[3];
                loc_xyz[0] = xloc;
                loc_xyz[1] = yloc;
                loc_xyz[2] = zloc;

                locs_xyz.push_back(loc_xyz);
//                locs_xyz.insert(loc_xyz);
//                printf("%d\t%d\t%d\t%d\n", pidx+1, xloc, yloc, zloc);
//                float* ptr = locs_xyz.back();
//                printf("\t\t\t\t%d \t %f \t %f \t %f \n", locs_xyz.size(), ptr[0], ptr[1], ptr[2]);
//                printf("list has %d \n", locs_xyz.size());

                for (int xx = xloc-nhd; xx <= xloc+nhd; ++xx) {
                    for (int yy = yloc-nhd; yy <= yloc+nhd; ++yy) {
                        for (int zz = zloc-nhd; zz <= zloc+nhd; ++zz) {
                            if (xx>=0 && xx<W && yy>=0 && yy<H && zz>=0 && zz<L) {
                                map[zz*(W*H)+yy*W+xx] = true;
                            }
                        }
                    }
                }
            }
        }
    } // go through locs

    printf("list has %d x,y,z locations\n", locs_xyz.size());

    //////////// export them as swc file independent points
    NeuronTree nt_locs;
    for (int ni = 0; ni < locs_xyz.size(); ++ni) {
        NeuronSWC nn;
        nn.r = 1;
        nn.nodeinseg_id = ni+1;
        nn.type = 6;
        nn.n = ni+1;
        nn.x = locs_xyz.at(ni)[0];
        nn.y = locs_xyz.at(ni)[1];

        // this tweak for visualization
//        float temp = nn.x;
//        nn.x = nn.y;
//        nn.y = temp;

        nn.z = locs_xyz.at(ni)[2];
        nn.parent = -1;
        nt_locs.listNeuron.append(nn);

        printf("%d\t%.1f\t%.1f\t%.1f\n", ni+1, nn.x, nn.y, nn.z);
    }
    QString swc_name = PARA.inimg_file + "_locs.swc";
    nt_locs.name = "Landmarks only";
    writeSWC_file(swc_name.toStdString().c_str(),nt_locs);

    delete img_zncc, idesc;
    delete fg;
    delete i2xyz, xyz2i, i2zncc, i2sigma, i2vx, i2vy;

    /////// output 'reconstruction'
//    NeuronTree nt;
//    swc_name = PARA.inimg_file + "_NeurAuto.swc";
//    nt.name = "NeurAuto";
//    writeSWC_file(swc_name.toStdString().c_str(),nt);

    printf("\nDONE.\n");

    if(!bmenu)
    {
        if(inimg) {delete []inimg; inimg = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
