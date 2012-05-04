#ifndef __READ_ESWC_H__
#define __READ_ESWC_H__
//below is adapted from basic_surf_objs.cpp by RZC
NeuronTree read_eswc(vector<V3DLONG> & seg_id, vector<V3DLONG> & seg_layer, vector<double> & feature, QString & filename)
{
	NeuronTree nt;
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

        QStringList qsl = QString(buf).split(" ",QString::SkipEmptyParts);
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
		//add 3 additional columns 12-02-16
		else if (i==7) seg_id.push_back(qsl[i].toInt());
		else if (i==8) seg_layer.push_back(qsl[i].toInt());
		else if (i==9) feature.push_back(qsl[i].toFloat());
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
    nt.file = QFileInfo(filename).absoluteFilePath();
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0,0,0,0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    nt.name = name.remove('\n'); if (nt.name.isEmpty()) nt.name = QFileInfo(filename).baseName();
    nt.comment = comment.remove('\n');

	return nt;
}
#endif
