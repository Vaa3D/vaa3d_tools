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
        string label_file = "/local1/work/caffe/data/ilsvrc12/synset_IVSCC.txt";
        Classifier classifier(model_file, trained_file, mean_file, label_file);


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
        std::vector<std::vector<Prediction> > all_predictions = classifier.Classify(imgs);
        double p_num = 0;
        double n_num = 0;

        /* Print the top N predictions. */
        for (size_t i = 0; i < all_predictions.size(); ++i)
        {
            std::vector<Prediction>& predictions = all_predictions[i];
            Prediction p = predictions[0];
            if(p.first == "0") n_num++;
            if(p.first == "1") p_num++;
        }

        double p_rate = p_num/(p_num+n_num);
        double n_rate = n_num/(p_num+n_num);
        printf("Positive rate is %.2f, and negative rate is %.2f\n",p_rate,n_rate);
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

        QString label_file = paras.empty() ? "" : paras[k]; if(label_file == "NULL") label_file = ""; k++;
        if(label_file.isEmpty())
        {
            cerr<<"Need a label_file"<<endl;
            return false;
        }

        cout<<"inimg_file = "<<m_InputfolderName.toStdString().c_str()<<endl;
        cout<<"model_file = "<<model_file.toStdString().c_str()<<endl;
        cout<<"trained_file = "<<model_file.toStdString().c_str()<<endl;
        cout<<"mean_file = "<<model_file.toStdString().c_str()<<endl;
        cout<<"label_file = "<<model_file.toStdString().c_str()<<endl;

        Classifier classifier(model_file.toStdString(), trained_file.toStdString(), mean_file.toStdString(), label_file.toStdString());

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
        std::vector<std::vector<Prediction> > all_predictions = classifier.Classify(imgs);
        double p_num = 0;
        double n_num = 0;

        /* Print the top N predictions. */
        for (size_t i = 0; i < all_predictions.size(); ++i)
        {
            std::vector<Prediction>& predictions = all_predictions[i];
            Prediction p = predictions[0];
            if(p.first == "0") n_num++;
            if(p.first == "1") p_num++;
        }
        cout<<"\positive rate: "<<p_num/all_predictions.size()<<" and negative rate: "<<n_num/all_predictions.size()<<endl;

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

