/* adaEnhancement_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-8-14 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "adaEnhancement_plugin.h"

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

#include "fastmarching_dt.h"
#include<map>

using namespace std;
Q_EXPORT_PLUGIN2(adaEnhancement, adaEnhancementPlugin);

template<class T>
bool strctureLineEnhencement(T * data1d, V3DLONG* sz, T * &outimg, double ratio);

template<class T>
int getXYRadius(T * data1d, V3DLONG* sz, V3DLONG x0, V3DLONG y0, V3DLONG z0);

template<class T>
int getZRadius(T * data1d, V3DLONG* sz, V3DLONG x0, V3DLONG y0, V3DLONG z0);

template<class T>
bool computeCubePcaEigVec(T* data1d, V3DLONG* sz,
                          V3DLONG x0, V3DLONG y0, V3DLONG z0,
                          V3DLONG wx, V3DLONG wy, V3DLONG wz,
                          double &pc1, double &pc2, double &pc3,
                          double *vec1, double *vec2, double *vec3);

template<class T>
bool computeCylinderPcaEigVec(T* data1d, V3DLONG* sz,
                              V3DLONG x1, V3DLONG y1, V3DLONG z1,
                              V3DLONG x2, V3DLONG y2, V3DLONG z2,
                              double r,
                              double &pc1, double &pc2, double &pc3,
                              double *vec1, double *vec2, double *vec3);

void computeNeuronTreeLineScore(unsigned char* data1d, V3DLONG* sz, NeuronTree& nt);

QStringList adaEnhancementPlugin::menulist() const
{
	return QStringList() 
        <<tr("getXYRadius")
		<<tr("menu2")
		<<tr("about");
}

QStringList adaEnhancementPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void adaEnhancementPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("getXYRadius"))
	{
        v3dhandle cur = callback.currentImageWindow();
        LandmarkList m = callback.getLandmark(cur);
        Image4DSimple* image = callback.getImage(cur);
        unsigned char* data = image->getRawData();
        V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};
        V3DLONG x,y,z;
        for(int i=0; i<m.size(); i++){
            x = m[i].x - 0.5;
            y = m[i].y - 0.5;
            z = m[i].z - 0.5;
            getXYRadius(data,sz,x,y,z);
        }
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ZX, 2020-8-14"));
	}
}

bool adaEnhancementPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
        QString inImgPath = infiles[0];
        QString outImgPath = inImgPath + "_out.v3draw";

        double ratio = inparas.size()>=1 ? atof(inparas[0]) : 1.0;
        unsigned char* data1d = 0;
        V3DLONG sz[4] = {0,0,0,0};

        int dataType = 1;
        simple_loadimage_wrapper(callback,inImgPath.toStdString().c_str(),data1d,sz,dataType);

        V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

        unsigned char* outImg = 0;
        strctureLineEnhencement(data1d,sz,outImg,ratio);

        unsigned char* lineSomaU = new unsigned char[tolSZ*3];
        for(int i=0 ;i<tolSZ; i++){
            if(outImg[i] == 2){
                lineSomaU[i] = data1d[i];
            }else {
                lineSomaU[i] = 0;
            }
            if(outImg[i] == 4){
                lineSomaU[i+tolSZ] = data1d[i];
            }else {
                lineSomaU[i+tolSZ] = 0;
            }
            if(outImg[i] == 6){
                lineSomaU[i+tolSZ*2] = data1d[i];
            }else {
                lineSomaU[i+tolSZ*2] = 0;
            }
        }

        sz[4] = 3;
        simple_saveimage_wrapper(callback,outImgPath.toStdString().c_str(),lineSomaU,sz,dataType);

        if(lineSomaU){
            delete[] lineSomaU;
            lineSomaU = 0;
        }
        if(data1d){
            delete[] data1d;
            data1d = 0;
        }
        if(outImg){
            delete[] outImg;
            outImg = 0;
        }
	}
	else if (func_name == tr("func2"))
	{
        QString imgPath = infiles[0];
        QString swcPath = infiles[1];

        V3DLONG sz[4] = {0,0,0,0};
        int dataType = 1;
        unsigned char* data1d = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,dataType);

        NeuronTree nt = readSWC_file(swcPath);
        computeNeuronTreeLineScore(data1d,sz,nt);

        if(data1d){
            delete[] data1d;
            data1d = 0;
        }

        QString swcOutPath = swcPath + "_result.swc";
        writeSWC_file(swcOutPath,nt);

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

template<class T>
bool strctureLineEnhencement(T * data1d, V3DLONG* sz, T * &outimg, double ratio){
    enum{ZX_UNDEFINEd = 0 ,ZX_DOT = 1, ZX_Line = 2, ZX_PlANE = 3, ZX_SOMA = 4, ZX_TMP = 5, ZX_UNDEFINED2 = 6, ZX_OTHER = -1};



    double imgMean, imgStd;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    mean_and_std(data1d,tolSZ,imgMean,imgStd);

    outimg = new T[tolSZ];

    float* gsdtData1d = 0;
    int cnnType = 3;
    fastmarching_dt(data1d,gsdtData1d,sz[0],sz[1],sz[2],cnnType,imgMean+0.5*imgStd);

    int* state = new int[tolSZ];
    T* dataCompute = new T[tolSZ];
//    memcpy(dataCompute,data1d,tolSZ*sizeof(T));

    BasicHeap<HeapElem> heap;
    map<V3DLONG,HeapElem*> elems;
    for(int i=0; i<tolSZ; i++){
        state[i] = ZX_UNDEFINEd;
        dataCompute[i] = data1d[i];
        if(data1d[i] > imgMean){
            HeapElem* elem = new HeapElem(i,1.0/gsdtData1d[i]);
            heap.insert(elem);
            elems[i] = elem;
        }
    }
    double LOG2 = log(2.0);

    qDebug()<<"elems size:"<<elems.size();
    int count = 0;

    double pc1,pc2,pc3;
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];

    int eSize = elems.size();
    int h = 0;

    double process1 = 0;
    while (!heap.empty()) {

        double process2 = (h++)*100000.0/eSize;
        if(process2 - process1 >= 10) {
            std::cout<<"\r"<<((V3DLONG)process2)/1000.0<<"%";std::cout.flush(); process1 = process2;
        }

        HeapElem* minElem = heap.delete_min();
        elems.erase(minElem->img_ind);


        V3DLONG minIndex = minElem->img_ind;
        delete minElem;

        if(state[minIndex] != ZX_UNDEFINEd)
            continue;

        V3DLONG x0 = minIndex%sz[0];
        V3DLONG y0 = (minIndex/sz[0])%sz[1];
        V3DLONG z0 = minIndex/(sz[0]*sz[1]);

        int wx,wy,wz;
//        wx = (int)round((ratio*log((double)gsdtData1d[minIndex]+1.0)/LOG2));
//        if(ratio == 0.1){
//            if(wx == 0)
//                wx = 1;
//            wy = wx;
//            wz = wx;
//        }else {
//            wy = wx;
//            wz = wx*2;
//        }
        wx = wy = getXYRadius(data1d,sz,x0,y0,z0);
        wz = getZRadius(data1d,sz,x0,y0,z0);

//        if(count%1000 == 0){
//            qDebug()<<"wx:"<<wx<<" wy:"<<wy<<" wz:"<<wz;
//        }

        if(wx>0){
            V3DLONG xb = x0 - wx; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
            V3DLONG xe = x0 + wx; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
            V3DLONG yb = y0 - wy; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
            V3DLONG ye = y0 + wy; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
            V3DLONG zb = z0 - wz; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
            V3DLONG ze = z0 + wz; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

            V3DLONG i,j,k;
            V3DLONG index;

            double w;

            //first get the center of mass
            double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
            for (k = zb; k <= ze; k++)
            {
                for (j = yb; j <= ye; j++)
                {
                    for (i = xb; i <= xe; i++)
                    {
                        index = k*sz[0]*sz[1] + j*sz[0] + i;
                        w = (double) dataCompute[index];
                        xm += w*i;
                        ym += w*j;
                        zm += w*k;
                        s += w;
                    }
                }
            }

            if (s>0)
            {
                xm /= s; ym /= s; zm /= s;
                mv = s / ((double)((ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1)));
            }
            else
            {
                state[minIndex] = ZX_OTHER;
                printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
                continue;
            }

            double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
            double dfx, dfy, dfz;
            for (k = zb; k <= ze; k++)
            {
                dfz = double(k) - zm;
                for (j = yb; j <= ye; j++)
                {
                    dfy = double(j) - ym;
                    for (i = xb; i <= xe; i++)
                    {
                        dfx = double(i) - xm;

                        //                w = img3d[k][j][i]; //140128
                        index = k*sz[0]*sz[1] + j*sz[0] + i;
                        w = dataCompute[index] - mv;  if (w<0) w = 0; //140128 try the new formula

                        cc11 += w*dfx*dfx;
                        cc12 += w*dfx*dfy;
                        cc13 += w*dfx*dfz;
                        cc22 += w*dfy*dfy;
                        cc23 += w*dfy*dfz;
                        cc33 += w*dfz*dfz;
                    }
                }
            }

            cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
//            if(count%1000 == 0){
//                qDebug()<<" cc11:"<<cc11<<" cc12:"<<cc12<<" cc13:"<<cc13<<" cc22:"<<cc22<<" cc23:"<<cc23<<" cc33:"<<cc33;
//            }



            try
            {
                //then find the eigen vector
                SymmetricMatrix Cov_Matrix(3);
                Cov_Matrix.Row(1) << cc11;
                Cov_Matrix.Row(2) << cc12 << cc22;
                Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

                DiagonalMatrix DD;
                Matrix VV;
                EigenValues(Cov_Matrix, DD, VV);

                //output the result
                pc1 = DD(3);
                pc2 = DD(2);
                pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

                //output the vector
                for (int i = 1; i <= 3; i++){
                    vec1[i-1] = VV(i, 3);
                    vec2[i-1] = VV(i, 2);
                    vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
                }
            }
            catch (...)
            {
                pc1 = VAL_INVALID;
                pc2 = VAL_INVALID;
                pc3 = VAL_INVALID;

            }

            if(pc1 == VAL_INVALID){
                qDebug()<<"pc1 is VAL_INVALID";
                state[minIndex] = ZX_OTHER;
                continue;
            }

//            if(count%1000 == 0){
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;
//            }
//            count++;


//            V3DLONG x,y,z;
            if(pc1/pc2 > 2.0){
//                qDebug()<<"--------line---------";
                XYZ dire = normalize(XYZ(vec1[0],vec1[1],vec1[2]));
                XYZ direOrthogonal = normalize(XYZ(1.0,1.0,-(dire.x + dire.y)/dire.z));
                XYZ dxyz = XYZ(dire.x*wz,dire.y*wz,dire.z*wz)
                        + XYZ(direOrthogonal.x*wx*sqrt(2.0),direOrthogonal.y*wx*sqrt(2.0),direOrthogonal.z*wx*sqrt(2.0));
                int dx = ceil(abs(dxyz.x));
                int dy = ceil(abs(dxyz.y));
                int dz = ceil(abs(dxyz.z));

                xb = x0 - dx*2; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
                xe = x0 + dx*2; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
                yb = y0 - dy*2; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
                ye = y0 + dy*2; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
                zb = z0 - dz*2; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
                ze = z0 + dz*2; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

                int nlineCount = 0;
                int lineCount = 0;
                for (k = zb; k <= ze; k++)
                {
                    for (j = yb; j <= ye; j++)
                    {
                        for (i = xb; i <= xe; i++)
                        {
                            index = k*sz[0]*sz[1] + j*sz[0] + i;
                            XYZ tmp = XYZ(i-x0,j-y0,k-z0);
                            double l = abs(dot(tmp,dire));
                            double r = abs(dot(tmp,direOrthogonal));
                            if(l<=wz && r<=wx){
                                if(state[index] == ZX_UNDEFINEd || state[index] == ZX_UNDEFINED2){
                                    state[index] = ZX_Line;
                                }
//                                if(state[index] == ZX_UNDEFINEd){
//                                    state[index] = ZX_TMP;
//                                    dataCompute[index] = 0;
//                                }else {
//                                    nlineCount++;
//                                }
//                                lineCount++;
                            }else if (l<=wz*2 && r<=wx*2) {
                                if(state[index] == ZX_UNDEFINEd){
                                    state[index] = ZX_UNDEFINED2;
                                }
                            }
                        }
                    }
                }

//                if(nlineCount/(double)lineCount > 0.3){
//                    for(i=0; i<tolSZ; i++){
//                        if(state[i] == ZX_TMP){
//                            state[i] = ZX_UNDEFINEd;
//                        }
//                    }
//                }else {
//                    for(i=0; i<tolSZ; i++){
//                        if(state[i] == ZX_TMP){
//                            state[i] = ZX_Line;
//                        }
//                    }
//                }

            }else if (pc2/pc3 > 2.0) {
//                qDebug()<<"--------plane---------";
                XYZ dire = normalize(cross(XYZ(vec1[0],vec1[1],vec1[2]),XYZ(vec2[0],vec2[1],vec2[2])));
                XYZ direOrthogonal = normalize(XYZ(1.0,1.0,-(dire.x + dire.y)/dire.z));

                int dx,dy,dz;

                vector<XYZ> queue = vector<XYZ>();
                queue.clear();
                queue.push_back(XYZ(x0,y0,z0));
                int qCount = 0;
                while(!queue.empty()){
//                    std::cout<<"\r"<<queue.size();std::cout.flush();
//                    qDebug()<<qCount<<" size: "<<queue.size();
                    qCount++;
                    XYZ cur = queue.front();
                    queue.erase(queue.begin());
                    index = cur.z*sz[0]*sz[1] + cur.y*sz[0] + cur.x;
//                    qDebug()<<"state: "<<state[index];
                    if(state[index] != ZX_UNDEFINEd)
                        continue;
                    state[index] = ZX_TMP;

                    for (dz = -1; dz <= 1; dz++)
                    {
                        for (dy = -1; dy <= 1; dy++)
                        {
                            for (dx = -1; dx <= 1; dx++)
                            {
                                if(abs(dx) + abs(dy) + abs(dz) != 1)
                                    continue;
//                                qDebug()<<"ijk: "<<i<<j<<k;
                                i = cur.x + dx;
                                j = cur.y + dy;
                                k = cur.z + dz;
                                if(i<0 || i>=sz[0])
                                    continue;
                                if(j<0 || j>=sz[1])
                                    continue;
                                if(k<0 || k>=sz[2])
                                    continue;
                                V3DLONG tmpIndex = k*sz[0]*sz[1] + j*sz[0] + i;
                                XYZ tmp = XYZ(i-x0,j-y0,k-z0);
                                double l = abs(dot(tmp,dire));
                                double r = abs(dot(tmp,direOrthogonal));
                                if(l<=wz && r<=wx && (state[tmpIndex] == ZX_UNDEFINEd || state[tmpIndex] == ZX_UNDEFINED2) && abs(dataCompute[index] - dataCompute[tmpIndex])<5){
                                    queue.push_back(XYZ(i,j,k));
                                }else if (l<=wz && r<=wx*2) {
                                    if(state[index] == ZX_UNDEFINEd){
                                        state[index] = ZX_UNDEFINED2;
                                    }
                                }
//                                qDebug()<<"lr: "<<l<<r;
                            }
                        }
                    }
                }

                int tmpCount = 0;
                for(i=0 ; i<tolSZ; i++){
                    if(state[i] == ZX_TMP)
                        tmpCount++;
                }

                if(tmpCount>200){
                    for(i=0 ; i<tolSZ; i++){
                        if(state[i] == ZX_TMP){
                            state[i] = ZX_SOMA;
//                            dataCompute[i] = 0;
                        }
                    }
                }else {
                    for(i=0 ; i<tolSZ; i++){
                        if(state[i] == ZX_TMP){
                            state[i] = ZX_PlANE;
//                            dataCompute[i] = 0;
                        }
                    }
                }

            }else{
//                qDebug()<<"--------dot---------";
                int dx,dy,dz;
                vector<XYZ> queue = vector<XYZ>();
                queue.clear();
                queue.push_back(XYZ(x0,y0,z0));
                while(!queue.empty()){
//                    std::cout<<"\r"<<queue.size();std::cout.flush();
                    XYZ cur = queue.front();
                    queue.erase(queue.begin());
                    index = cur.z*sz[0]*sz[1] + cur.y*sz[0] + cur.x;
                    if(state[index] != ZX_UNDEFINEd)
                        continue;
                    state[index] = ZX_TMP;

                    for (dz = -1; dz <= 1; dz++)
                    {
                        for (dy = -1; dy <= 1; dy++)
                        {
                            for (dx = -1; dx <= 1; dx++)
                            {
                                if(abs(dx) + abs(dy) + abs(dz) != 1)
                                    continue;
                                i = cur.x + dx;
                                j = cur.y + dy;
                                k = cur.z + dz;
                                if(i<0 || i>=sz[0])
                                    continue;
                                if(j<0 || j>=sz[1])
                                    continue;
                                if(k<0 || k>=sz[2])
                                    continue;
                                V3DLONG tmpIndex = k*sz[0]*sz[1] + j*sz[0] + i;
                                double r = norm(XYZ(i-x0,j-y0,k-z0));
                                if(r<=wx && (state[tmpIndex] == ZX_UNDEFINEd || state[tmpIndex] == ZX_UNDEFINED2) && abs(dataCompute[index] - dataCompute[tmpIndex])<5){
                                    queue.push_back(XYZ(i,j,k));
                                }
                            }
                        }
                    }
                }

                int tmpCount = 0;
                for(i=0 ; i<tolSZ; i++){
                    if(state[i] == ZX_TMP)
                        tmpCount++;
                }

                if(tmpCount>200){
                    for(i=0 ; i<tolSZ; i++){
                        if(state[i] == ZX_TMP){
                            state[i] = ZX_SOMA;
//                            dataCompute[i] = 0;
                        }
                    }
                }else {
                    for(i=0 ; i<tolSZ; i++){
                        if(state[i] == ZX_TMP){
                            state[i] = ZX_DOT;
//                            dataCompute[i] = 0;
                        }
                    }
                }
            }

        }

    }

    for(int i=0; i<tolSZ; i++){
        outimg[i] = state[i];
    }

    if(vec1){
        delete[] vec1;
        vec1 = 0;
    }
    if(vec2){
        delete[] vec2;
        vec2 = 0;
    }
    if(vec3){
        delete[] vec3;
        vec3 = 0;
    }

    if(state){
        delete[] state;
        state = 0;
    }
    if(dataCompute){
        delete[] dataCompute;
        dataCompute = 0;
    }

    return true;

}

template<class T>
int getXYRadius(T * data1d, V3DLONG* sz, V3DLONG x0, V3DLONG y0, V3DLONG z0){
    int r = 1;
    V3DLONG index = z0*sz[0]*sz[1] + y0*sz[0] + x0;
    T coreValue = data1d[index];
    T outerValue;
    qDebug()<<(r-1)<<" : "<<coreValue;
    int count;
    V3DLONG xb,xe,yb,ye;
    V3DLONG i,j;
    while(true){
        xb = x0 - r; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
        xe = x0 + r; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
        yb = y0 - r; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
        ye = y0 + r; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;

        outerValue = 0;
        count = 0;

        for(j=yb; j<=ye; j++){
            for(i=xb; i<=xe; i++){
                double cur_r = sqrt((i-x0)*(i-x0)+(j-y0)*(j-y0));
                if(cur_r>r || cur_r<=r-1 )
                    continue;
                index = z0*sz[0]*sz[1] + j*sz[0] + i;
                count++;
                outerValue += data1d[index];
            }
        }
        if(count>0){
            outerValue /= (double)count;
        }
        qDebug()<<r<<":"<<outerValue;
//        if(coreValue-outerValue>5){
//            break;
//        }else {
//            coreValue = outerValue;
//        }
        r++;
        if(r>10)
            break;
    }
    return r;
}

template<class T>
int getZRadius(T * data1d, V3DLONG* sz, V3DLONG x0, V3DLONG y0, V3DLONG z0){
    int r = 1;
    V3DLONG index = z0*sz[0]*sz[1] + y0*sz[0] + x0;
    T coreValue = data1d[index];
    T outerValue;
    V3DLONG index1,index2;
    while (true) {
        V3DLONG z1 = z0 + r; if(z1<0) z1=0; if(z1>=sz[2]) z1=sz[2]-1;
        V3DLONG z2 = z0 - r; if(z2<0) z2=0; if(z2>=sz[2]) z2=sz[2]-1;
        index1 = z1*sz[0]*sz[1] +y0*sz[0] + x0;
        index2 = z2*sz[0]*sz[1] +y0*sz[0] + x0;
        outerValue = (data1d[index1] + data1d[index2])/2.0;

        if(coreValue-outerValue>5){
            break;
        }else {
            coreValue = outerValue;
        }

        r++;
    }
    return r;
}

template <class T>
bool computeCubePcaEigVec(T* data1d, V3DLONG* sz,
                          V3DLONG x0, V3DLONG y0, V3DLONG z0,
                          V3DLONG wx, V3DLONG wy, V3DLONG wz,
                          double &pc1, double &pc2, double &pc3,
                          double *vec1, double *vec2, double *vec3){
    V3DLONG xb = x0 - wx; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
    V3DLONG xe = x0 + wx; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
    V3DLONG yb = y0 - wy; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
    V3DLONG ye = y0 + wy; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
    V3DLONG zb = z0 - wz; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
    V3DLONG ze = z0 + wz; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

    V3DLONG i,j,k;
    V3DLONG index;

    double w;

    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    for (k = zb; k <= ze; k++)
    {
        for (j = yb; j <= ye; j++)
        {
            for (i = xb; i <= xe; i++)
            {
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = (double) data1d[index];
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
            }
        }
    }

    if (s>0)
    {
        xm /= s; ym /= s; zm /= s;
        mv = s / ((double)((ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1)));
    }
    else
    {
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;
    for (k = zb; k <= ze; k++)
    {
        dfz = double(k) - zm;
        for (j = yb; j <= ye; j++)
        {
            dfy = double(j) - ym;
            for (i = xb; i <= xe; i++)
            {
                dfx = double(i) - xm;

                //                w = img3d[k][j][i]; //140128
                index = k*sz[0]*sz[1] + j*sz[0] + i;
                w = data1d[index] - mv;  if (w<0) w = 0; //140128 try the new formula

                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
            }
        }
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;
//            if(count%1000 == 0){
//                qDebug()<<" cc11:"<<cc11<<" cc12:"<<cc12<<" cc13:"<<cc13<<" cc22:"<<cc22<<" cc23:"<<cc23<<" cc33:"<<cc33;
//            }



    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }

    return true;
}

template<class T>
bool computeCylinderPcaEigVec(T* data1d, V3DLONG* sz,
                              V3DLONG x1, V3DLONG y1, V3DLONG z1,
                              V3DLONG x2, V3DLONG y2, V3DLONG z2,
                              double r,
                              double &pc1, double &pc2, double &pc3,
                              double *vec1, double *vec2, double *vec3){
    XYZ dire = normalize(XYZ(x2-x1,y2-y1,z2-z1));
    XYZ direOrthogonal = normalize(XYZ(1.0,1.0,-(dire.x + dire.y)/dire.z));
    double len = dist_L2(XYZ(x1,y1,z1),XYZ(x2,y2,z2))/2.0;

    XYZ dxyz = XYZ(dire.x*len,dire.y*len,dire.z*len)
            + XYZ(direOrthogonal.x*r*sqrt(2.0),direOrthogonal.y*r*sqrt(2.0),direOrthogonal.z*r*sqrt(2.0));

    int dx = ceil(abs(dxyz.x));
    int dy = ceil(abs(dxyz.y));
    int dz = ceil(abs(dxyz.z));

    V3DLONG x0 = (x2+x1)/2.0 + 0.5;
    V3DLONG y0 = (y2+y1)/2.0 + 0.5;
    V3DLONG z0 = (z2+z1)/2.0 + 0.5;

    V3DLONG xb = x0 - dx*2; if(xb<0) xb = 0; if(xb >= sz[0]) xb = sz[0] - 1;
    V3DLONG xe = x0 + dx*2; if(xe<0) xe = 0; if(xe >= sz[0]) xe = sz[0] - 1;
    V3DLONG yb = y0 - dy*2; if(yb<0) yb = 0; if(yb >= sz[1]) yb = sz[1] - 1;
    V3DLONG ye = y0 + dy*2; if(ye<0) ye = 0; if(ye >= sz[1]) ye = sz[1] - 1;
    V3DLONG zb = z0 - dz*2; if(zb<0) zb = 0; if(zb >= sz[2]) zb = sz[2] - 1;
    V3DLONG ze = z0 + dz*2; if(ze<0) ze = 0; if(ze >= sz[2]) ze = sz[2] - 1;

    V3DLONG i,j,k;
    V3DLONG index;

    double w;
    int count = 0;
    //first get the center of mass
    double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
    for (k = zb; k <= ze; k++)
    {
        for (j = yb; j <= ye; j++)
        {
            for (i = xb; i <= xe; i++)
            {
                index = k*sz[0]*sz[1] + j*sz[0] + i;

                XYZ tmp = XYZ(i-x0,j-y0,k-z0);
                double curl = abs(dot(tmp,dire));
                double curr = abs(dot(tmp,direOrthogonal));
                if(curl<=len && curr<=r){
                    w = (double) data1d[index];
                    xm += w*i;
                    ym += w*j;
                    zm += w*k;
                    s += w;
                    count++;
                }
            }
        }
    }
    qDebug()<<"count: "<<count;

    if(s>0){
        xm /= s; ym /= s; zm /= s;
        mv = s / count;
    }else {
        printf("Sum of window pixels equals or is smaller than 0. The window is not valid or some other problems in the data. Do nothing.\n");
        return false;
    }

    double cc11 = 0, cc12 = 0, cc13 = 0, cc22 = 0, cc23 = 0, cc33 = 0;
    double dfx, dfy, dfz;

    for (k = zb; k <= ze; k++)
    {
        dfz = double(k) - zm;
        for (j = yb; j <= ye; j++)
        {
            dfy = double(j) - ym;
            for (i = xb; i <= xe; i++)
            {
                dfx = double(i) - xm;

                index = k*sz[0]*sz[1] + j*sz[0] + i;

                XYZ tmp = XYZ(i-x0,j-y0,k-z0);
                double curl = abs(dot(tmp,dire));
                double curr = abs(dot(tmp,direOrthogonal));
                if(curl<=len && curr<=r){
                    w = data1d[index] - mv;  if (w<0) w = 0;

                    cc11 += w*dfx*dfx;
                    cc12 += w*dfx*dfy;
                    cc13 += w*dfx*dfz;
                    cc22 += w*dfy*dfy;
                    cc23 += w*dfy*dfz;
                    cc33 += w*dfz*dfz;
                }
            }
        }
    }

    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix, DD, VV);

        //output the result
        pc1 = DD(3);
        pc2 = DD(2);
        pc3 = DD(1);
//                qDebug()<<"pc1:"<<pc1<<"pc2:"<<pc2<<"pc3:"<<pc3;

        //output the vector
        for (int i = 1; i <= 3; i++){
            vec1[i-1] = VV(i, 3);
            vec2[i-1] = VV(i, 2);
            vec3[i-1] = VV(i, 1);
//                    qDebug()<<"vec1i:"<<vec1[i]<<"vec2i:"<<vec2[i]<<"vec3i:"<<vec3[i];
        }
    }
    catch (...)
    {
        pc1 = VAL_INVALID;
        pc2 = VAL_INVALID;
        pc3 = VAL_INVALID;
        return false;
    }


    return true;
}

void computeNeuronTreeLineScore(unsigned char *data1d, long long *sz, NeuronTree &nt){
    V3DLONG x0,y0,z0;
    V3DLONG wx,wy,wz;
    double pc1,pc2,pc3;
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];

    int pointSize = nt.listNeuron.size();

    vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
    QStack<int> stack = QStack<int>();
    double* pathTotal = new double[pointSize];
    int* isRemain = new int[pointSize];

    for(int i=0 ; i<pointSize; i++){
        pathTotal[i] = 0;
        isRemain[i] = 0;
        if(nt.listNeuron[i].parent<0){
            stack.push(i);
            continue;
        }
        V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[i].parent);
        children[prtIndex].push_back(i);
        XYZ p1(nt.listNeuron[i].x,nt.listNeuron[i].y,nt.listNeuron[i].z);
        XYZ p2(nt.listNeuron[prtIndex].x,nt.listNeuron[prtIndex].y,nt.listNeuron[prtIndex].z);

        x0 = (p1.x + p2.x)/2.0 + 0.5;
        y0 = (p1.y + p2.y)/2.0 + 0.5;
        z0 = (p1.z + p2.z)/2.0 + 0.5;
        wx = wy = nt.listNeuron[i].r + 4;
        wz = nt.listNeuron[i].r + 2;
        computeCubePcaEigVec(data1d,sz,x0,y0,z0,wx,wy,wz,pc1,pc2,pc3,vec1,vec2,vec3);
//        computeCylinderPcaEigVec(data1d,sz,p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,wz,pc1,pc2,pc3,vec1,vec2,vec3);
        nt.listNeuron[i].radius = pc1/pc2;

        XYZ dire = normalize(p2 - p1);
        XYZ pcaVec = normalize(XYZ(vec1[0],vec1[1],vec1[2]));
        double d = abs(dot(dire,pcaVec));
        if(d>0.9){
            nt.listNeuron[i].type = 7;
        }else {
            nt.listNeuron[i].type = 2;
        }
    }

    V3DLONG ori;
    for(int i=0; i<pointSize; i++){

        if(nt.listNeuron[i].parent<0){
            ori = i;
            stack.push(ori);
            continue;
        }
        V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[i].parent);

        double r1 = nt.listNeuron[i].r;
        double r2 = nt.listNeuron[prtIndex].r;
        if(r2/r1>2 || r1<2.0){
            nt.listNeuron[i].type = 2;
        }else {
            nt.listNeuron[i].type = 7;
        }
    }
//    vector<V3DLONG> pointIndexs = vector<V3DLONG>();

//    V3DLONG t,tmp;
//    double pathlength;
//    while (!stack.empty()) {
//        t = stack.pop();
//        pointIndexs.push_back(t);
//        vector<V3DLONG> child = children[t];
//        for(int i=0; i<child.size(); i++){
//            tmp = child[i];

//            while(children[tmp].size() == 1){
//                pointIndexs.push_back(tmp);
//                V3DLONG ch = children[tmp][0];
//                tmp = ch;
//            }

//            int chsz = children[tmp].size();
//            if(chsz>1){
//                stack.push(tmp);
//            }else {
//                pointIndexs.push_back(tmp);
//            }

//           pathlength = zx_dist(nt.listNeuron[tmp],nt.listNeuron[t]);


//        }
//    }

//    for(int i=0; i<pointIndexs.size(); i++){
//        t = pointIndexs[i];
//        if(zx_dist(nt.listNeuron[t],nt.listNeuron[ori]) < nt.listNeuron[ori].r*2
//                || nt.listNeuron[t].type != 5 || isRemain[t] == 1){
//            isRemain[t] = 1;
//        }else {
//            V3DLONG prtIndex = nt.hashNeuron.value(nt.listNeuron[t].parent);
//            if(children[t].size()>1 || children[prtIndex].size()>1){
//                V3DLONG cur;
//                if(children[t].size() > 1){
//                    cur = t;
//                }else {
//                    cur = prtIndex;
//                }
//                vector<V3DLONG> segment1 = vector<V3DLONG>();
//                V3DLONG curP = nt.hashNeuron.value(nt.listNeuron[cur].parent);
//                while(children[curP].size() == 1){
//                    segment1.push_back(curP);
//                    curP = nt.hashNeuron.value(nt.listNeuron[curP].parent);
//                }


//            }
//        }



//    }



    if(vec1){
        delete[] vec1;
        vec1 = 0;
    }
    if(vec2){
        delete[] vec2;
        vec2 = 0;
    }
    if(vec3){
        delete[] vec3;
        vec3 = 0;
    }
    if(pathTotal){
        delete[] pathTotal;
        pathTotal = 0;
    }
    if(isRemain){
        delete[] isRemain;
        isRemain = 0;
    }
}

