/* prediction_caffe_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-1-31 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "prediction_caffe_plugin.h"

#include <classification.h>
#include "../../../../released_plugins/v3d_plugins/istitch/y_imglib.h"


using namespace std;
Q_EXPORT_PLUGIN2(prediction_caffe, prediction_caffe);
 
QStringList prediction_caffe::menulist() const
{
	return QStringList() 
        <<tr("Prediction")
        <<tr("Quality_Assess")
		<<tr("about");
}

QStringList prediction_caffe::funclist() const
{
	return QStringList()
        <<tr("Prediction")
		<<tr("help");
}

void prediction_caffe::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Prediction"))
	{
        string model_file = "/local1/work/caffe/models/bvlc_reference_caffenet/deploy.prototxt";
        string trained_file = "/local1/work/caffe/models/bvlc_reference_caffenet/caffenet_train_iter_2nd_450000.caffemodel";
        string mean_file = "/local1/work/caffe/data/ilsvrc12/imagenet_mean.binaryproto";
        Classifier classifier(model_file, trained_file, mean_file);


        QString m_InputfolderName = 0;
        m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all images "),
                                              QDir::currentPath(),
                                              QFileDialog::ShowDirsOnly);
        if(m_InputfolderName == 0)
            return;

        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);
            count++;
        }

        int NTILES  = vim.tilesList.size();
        std::vector<cv::Mat> imgs;
        for (V3DLONG i = 0; i <NTILES; i++)
        {
            cv::Mat img = cv::imread(vim.tilesList.at(i).fn_image.c_str());
            imgs.push_back(img);
        }
        std::vector<std::vector<float> > outputs = classifier.Predict(imgs);
        double p_num = 0;
        double n_num = 0;
        for (int j = 0; j < outputs.size(); j++)
        {
            std::vector<float> output = outputs[j];
            if(output.at(0) > output.at(1))
                n_num++;
            else
                p_num++;

        }
        double p_rate = p_num/(p_num+n_num);
        double n_rate = n_num/(p_num+n_num);

        printf("Positive rate is %.2f, and negative rate is %.2f\n",p_rate,n_rate);
        imgs.clear();
    }else if (menu_name == tr("Quality_Assess"))
    {
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

        unsigned char* data1d = p4DImage->getRawData();
        QString imagename = callback.getImageName(curwin);

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

        bool ok1, ok2, ok3;
        unsigned int Wx=1, Wy=1, Wz=1;

        Wx = QInputDialog::getInteger(parent, "Window X ",
                                      "Enter radius (window size is 2*radius+1):",
                                      30, 1, N, 1, &ok1);

        if(ok1)
        {
            Wy = QInputDialog::getInteger(parent, "Window Y",
                                          "Enter radius (window size is 2*radius+1):",
                                          30, 1, M, 1, &ok2);
        }
        else
            return;

        if(ok2)
        {
            Wz = QInputDialog::getInteger(parent, "Window Z",
                                          "Enter radius (window size is 2*radius+1):",
                                          25, 1, P, 1, &ok3);
        }
        else
            return;


        QString SWCfileName;
        SWCfileName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(SWCfileName.isEmpty())
            return;

        NeuronTree nt = readSWC_file(SWCfileName);

        string model_file = "/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/deploy.prototxt";
        string trained_file = "/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/caffenet_train_iter_270000.caffemodel";
        string mean_file = "/local4/Data/IVSCC_test/comparison/Caffe_testing_3nd/train_package_4th/imagenet_mean.binaryproto";

        Classifier classifier(model_file, trained_file, mean_file);
        std::vector<cv::Mat> imgs;

        for (V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            V3DLONG tmpx = nt.listNeuron.at(i).x;
            V3DLONG tmpy = nt.listNeuron.at(i).y;
            V3DLONG tmpz = nt.listNeuron.at(i).z;

            V3DLONG xb = tmpx-1-Wx; if(xb<0) xb = 0;if(xb>=N-1) xb = N-1;
            V3DLONG xe = tmpx-1+Wx; if(xe>=N-1) xe = N-1;
            V3DLONG yb = tmpy-1-Wy; if(yb<0) yb = 0;if(yb>=M-1) yb = M-1;
            V3DLONG ye = tmpy-1+Wy; if(ye>=M-1) ye = M-1;
            V3DLONG zb = tmpz-1-Wz; if(zb<0) zb = 0;if(zb>=P-1) zb = P-1;
            V3DLONG ze = tmpz-1+Wz; if(ze>=P-1) ze = P-1;

            V3DLONG im_cropped_sz[4];
            im_cropped_sz[0] = xe - xb + 1;
            im_cropped_sz[1] = ye - yb + 1;
            im_cropped_sz[2] = 1;
            im_cropped_sz[3] = sc;

            unsigned char *im_cropped = 0;

            V3DLONG pagesz = im_cropped_sz[0]* im_cropped_sz[1]* im_cropped_sz[2]*im_cropped_sz[3];
            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for im_cropped."); return;}
            memset(im_cropped, 0, sizeof(unsigned char)*pagesz);

            for(V3DLONG iz = zb; iz <= ze; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                V3DLONG j = 0;
                for(V3DLONG iy = yb; iy <= ye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = xb; ix <= xe; ix++)
                    {
                        if(data1d[offsetk + offsetj + ix] >= im_cropped[j])
                            im_cropped[j] = data1d[offsetk + offsetj + ix];
                        j++;
                    }
                }
            }

            cv::Mat img(im_cropped_sz[1], im_cropped_sz[0], CV_8UC1, im_cropped);
            imgs.push_back(img);
        }

        std::vector<std::vector<float> > outputs = classifier.Predict(imgs);
        double p_num = 0;
        double n_num = 0;
        QList <ImageMarker> marklist;
        QString markerpath =  SWCfileName + QString("_fp.marker");
        for (V3DLONG j=0;j<nt.listNeuron.size();j++)
        {
            std::vector<float> output = outputs[j];
            if(output.at(0) > output.at(1))
            {
                ImageMarker S;
                S.x = nt.listNeuron.at(j).x;
                S.y = nt.listNeuron.at(j).y;
                S.z = nt.listNeuron.at(j).z;
                marklist.append(S);
                n_num++;
            }
            else
                p_num++;

        }

        writeMarker_file(markerpath.toStdString().c_str(),marklist);
        cout<<"\positive rate: "<<p_num/outputs.size()<<" and negative rate: "<<n_num/outputs.size()<<endl;
        imgs.clear();

    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2017-1-31"));
	}
}

bool prediction_caffe::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, paras, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) paras = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("Prediction"))
	{
        cout<<"Welcome to Caffe prediction plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input image folder"<<endl;
            return false;
        }
        QString  m_InputfolderName =  infiles[0];
        int k=0;

        QString model_file = paras.empty() ? "" : paras[k]; if(model_file == "NULL") model_file = ""; k++;
        if(model_file.isEmpty())
        {
            cerr<<"Need a model_file"<<endl;
            return false;
        }

        QString trained_file = paras.empty() ? "" : paras[k]; if(trained_file == "NULL") trained_file = ""; k++;
        if(trained_file.isEmpty())
        {
            cerr<<"Need a trained_file"<<endl;
            return false;
        }

        QString mean_file = paras.empty() ? "" : paras[k]; if(mean_file == "NULL") mean_file = ""; k++;
        if(mean_file.isEmpty())
        {
            cerr<<"Need a mean_file"<<endl;
            return false;
        }

        cout<<"inimg_file = "<<m_InputfolderName.toStdString().c_str()<<endl;
        cout<<"model_file = "<<model_file.toStdString().c_str()<<endl;
        cout<<"trained_file = "<<trained_file.toStdString().c_str()<<endl;
        cout<<"mean_file = "<<mean_file.toStdString().c_str()<<endl;

        Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString());

        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);
            count++;
        }

        int NTILES  = vim.tilesList.size();
        std::vector<cv::Mat> imgs;
        for (V3DLONG i = 0; i <NTILES; i++)
        {
            cv::Mat img = cv::imread(vim.tilesList.at(i).fn_image.c_str(),-1);
            imgs.push_back(img);
        }
        std::vector<std::vector<float> > outputs = classifier.Predict(imgs);

        double p_num = 0;
        double n_num = 0;
        for (int j = 0; j < outputs.size(); j++)
        {
            std::vector<float> output = outputs[j];
            if(output.at(0) > output.at(1))
                n_num++;
            else
                p_num++;

        }

        cout<<"\positive rate: "<<p_num/outputs.size()<<" and negative rate: "<<n_num/outputs.size()<<endl;

        if(!outfiles.empty())
        {
            QString  outputfile =  outfiles[0];
            QFile saveTextFile;
            saveTextFile.setFileName(outputfile);
            if (!saveTextFile.isOpen()){
                if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
                    qDebug()<<"unable to save file!";
                    return false;}     }
            QTextStream outputStream;
            outputStream.setDevice(&saveTextFile);
            outputStream<< m_InputfolderName<<"\t"<< NTILES<<"\t"<<p_num<<"\t"<< n_num<<"\n";
            saveTextFile.close();
        }
        imgs.clear();
        return true;
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

