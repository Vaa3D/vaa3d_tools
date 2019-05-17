#include "some_function.h"




NeuronTree readESWC_file(const QString& filename)
{
    NeuronTree nt;
    nt.file = QFileInfo(filename).absoluteFilePath();
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
        return nt;
    }

    int count = 0;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    QString name = "";
    QString comment = "";

    qDebug("-------------------------------------------------------");
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            if (buf[1]=='n'&&buf[2]=='a'&&buf[3]=='m'&&buf[4]=='e'&&buf[5]==' ')
                name = buf+6;
            if (buf[1]=='c'&&buf[2]=='o'&&buf[3]=='m'&&buf[4]=='m'&&buf[5]=='e'&&buf[6]=='n'&&buf[7]=='t'&&buf[8]==' ')
                comment = buf+9;

            continue;
        }

        count++;
        NeuronSWC S;

        QStringList qsl = QString(buf).trimmed().split(" ",QString::SkipEmptyParts);
        if (qsl.size()==0)   continue;

        for (int i=0; i<qsl.size(); i++)
        {
            qsl[i].truncate(99);
            if (i==0) S.n = qsl[i].toInt();
            else if (i==1) S.type = qsl[i].toInt();
            else if (i==2) S.x = qsl[i].toFloat();
            else if (i==3) S.y = qsl[i].toFloat();
            else if (i==4) S.z = qsl[i].toFloat();
            else if (i==5) S.r = qsl[i].toFloat();
            else if (i==6) S.pn = qsl[i].toInt();
            //the ESWC extension, by PHC, 20120217
            else if (i==7) S.seg_id = qsl[i].toInt();
            else if (i==8) S.level = qsl[i].toInt();
            else if (i==9) S.creatmode = qsl[i].toInt();
            else if (i==10) S.timestamp = qsl[i].toInt();
             else if (i==11) S.tfresindex = qsl[i].toInt();
    //change ESWC format to adapt to flexible feature number, by WYN, 20150602
            else
        S.fea_val.append(qsl[i].toFloat());
       }

        //if (! listNeuron.contains(S)) // 081024
        {
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    qDebug("---------------------read %d lines, %d remained lines", count, listNeuron.size());

    if (listNeuron.size()<1)
        return nt;


    //now update other NeuronTree members

    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

    return nt;
}

bool writeESWC_file1(const QString& filename, const NeuronTree& nt)
{
    QString curFile = filename;
    if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
    {
        curFile = QFileDialog::getSaveFileName(0,
                                               "Select a ESWC (enhanced SWC) file to save the neuronal or relational data... ",
                                               ".eswc",
                                               QObject::tr("Enhanced Neuron structure file (*.eswc);;(*.*)"
                                                           ));
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("save file: %1").arg(curFile), false);
#endif

        if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
            return false;
    }

    FILE * fp = fopen(curFile.toLatin1(), "wt");
    if (!fp)
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg("Could not open the file to save the neuron.");
#endif
        return false;
    }

    fprintf(fp, "#name %s\n", qPrintable(nt.name.trimmed()));
    fprintf(fp, "#comment %s\n", qPrintable(nt.comment.trimmed()));

    fprintf(fp, "##n,type,x,y,z,radius,parent,seg_id,level,mode,timestamp,feature_value\n");
    NeuronSWC * p_pt=0;
    for (int i=0;i<nt.listNeuron.size(); i++)
    {
        p_pt = (NeuronSWC *)(&(nt.listNeuron.at(i)));
        fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld %ld %ld %d %.0f",
                p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn, p_pt->seg_id, p_pt->level, p_pt->creatmode, p_pt->timestamp);
        for (int j=0;j<p_pt->fea_val.size();j++)
            fprintf(fp, " %.5f", p_pt->fea_val.at(j));
        fprintf(fp, "\n");
    }
    fclose(fp);
#ifndef DISABLE_V3D_MSG
    v3d_msg(QString("done with saving file: ")+filename, false);
#endif
    return true;
}
