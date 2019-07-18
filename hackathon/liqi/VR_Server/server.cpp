#include "server.h"
#include <QString>
#include <QRegExp>
#include<vector>

std::vector<clientproperty> clientsproperty;
std::vector<QString> messagelist;
QString Creator;
Server::Server(QObject* parent) : QObject(parent) {
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()),
            this,   SLOT(onNewConnection()));

    if (!server->listen(QHostAddress::Any, PORT)) {
        qDebug() << "Server is not started.";
    } else {
        qDebug() << "Server is started.";
    }
	clientNum=0;
	sketchNum=0;
}


void Server::sendUserList() {
    QString line = "/users:" + ((QStringList)clients.values()).join(",") + "\n";
    sendToAll(line);
}


void Server::sendToAll(const QString& msg) {
    foreach (QTcpSocket* socket, clients.keys()) {
        socket->write(msg.toUtf8());
    }
}
void Server::sendToUser(const QString& msg,const QString& name) {
    foreach (QTcpSocket* socket, clients.keys()) {
        QString username = clients.value(socket);
        if(username == name)
        socket->write(msg.toUtf8());
    }
}
void Server::sendColorMsg() {
	foreach (QTcpSocket* socket, clients.keys()) {
		QString username = clients.value(socket);
		for(int i=0;i<clientsproperty.size();i++)
		{
			if(clientsproperty.at(i).name!=username) continue;
			QString msg= QString::number(clientsproperty.at(i).colortype, 10);
			sendToAll(QString("/color:"+username+" "+msg+" \n"));
		}
	}

}
void Server::sendCreatorMsg()
{
    foreach (QTcpSocket* socket, clients.keys()) {
            QString username = clients.value(socket);
            for(int i=0;i<clientsproperty.size();i++)
            {
                    if(clientsproperty.at(i).name!=username) continue;
                    if(clientsproperty.at(i).Creator)
                    sendToAll(QString("/creator:"+username+"\n"));
            }
    }

}
void Server::onNewConnection() {
    QTcpSocket* socket = server->nextPendingConnection();
    qDebug() << "Client connected: " << socket->peerAddress().toString();

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));

    clients.insert(socket, "");


}

void Server::onDisconnect() {
    QTcpSocket* socket = (QTcpSocket*)sender();
    qDebug() << "Client disconnected: " << socket->peerAddress().toString();

    QString username = clients.value(socket);
    sendToAll(QString("/system:" + username + " left .\n"));
    clients.remove(socket);
	for(int i=0;i<clientsproperty.size();i++)
	{
		if(clientsproperty.at(i).name!=username) continue;
                //clientsproperty.erase(clientsproperty.begin()+i);
                clientsproperty[i].online = false;
                //i--;
	}
    sendUserList();
}


void Server::onReadyRead() {
    QRegExp loginRex("^/login:(.*)$");
    QRegExp messageRex("^/say:(.*)$");
	QRegExp hmdposRex("^/hmdpos:(.*)$");
	QRegExp deleteRex("^/del_curve:(.*)$");
	QRegExp markerRex("^/marker:(.*)$");
	QRegExp delmarkerRex("^/del_marker:(.*)$");
	QRegExp dragnodeRex("^/drag_node:(.*)$");
        QRegExp askmessageRex("^/ask:(.*)$");
    QTcpSocket* socket = (QTcpSocket*)sender();
    while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();
        if (loginRex.indexIn(line) != -1) {
            QString user = loginRex.cap(1);
            clients[socket] = user;
            qDebug() << user << "logged in.";
			clientproperty client00={
									clientNum,
									user,
                                                                        21,
                                                                        false,
                                                                        true,
                                                                        0

			}; 
			
             if(!containsClient(clientsproperty,user))// see if user is login before,if false ,push new user
			{	
				clientNum=(clientNum+1)%11;
				qDebug()<<"the num of this client is " <<clientNum;
				client00.colortype=clientNum+2;//21+(clientNum*20)%(275-21);
                                //historyclients.insert(user,client00.colortype);
				qDebug()<<"this client's colortype is  " <<client00.colortype;
                                clientsproperty.push_back(client00);
			}
			else
			{
                                //client00.colortype = historyclients[user];
                clientsproperty[getuser(clientsproperty,user)].online = true;
				qDebug()<<"the num of this client is " <<clientNum;
				qDebug()<<"this client's colortype is  " <<client00.colortype;
			}
                        int onlineusernum = 0;
                        for(int i= 0;i<clientsproperty.size();i++)// search online user num
                        {
                            if(clientsproperty[i].online)
                                onlineusernum++;
                        }
                        if(onlineusernum == 1||user == Creator)//update user name
                        {
                            clientsproperty[getuser(clientsproperty,user)].Creator = true;
                            qDebug()<<"current creator is "<<user;
                            Creator = user;
                        }
                        sendToAll(QString("/system:" + user + " joined.\n"));
                        sendUserList();

			sendColorMsg();
                        sendCreatorMsg();
        }
        else if (messageRex.indexIn(line) != -1) {
            QString user = clients.value(socket);
            QString msg = messageRex.cap(1);
           // sendToAll(QString(user + ":" + msg + "\n"));  now client ask for message ,instead of  send to all
            messagelist.push_back(QString(user + ":" + msg + "\n"));
            qDebug() << "User:" << user;
            qDebug() << "Message:" << msg;
            int ctype=21;
            for(int i=0;i<clientsproperty.size();i++)
            {
                if(user==clientsproperty[i].name)
                {
                    ctype = clientsproperty[i].colortype;
                    break;
                }
            }
            AddToNTList(msg,ctype);
        }
        else if (deleteRex.indexIn(line) != -1) {
            QString user = clients.value(socket);
            QString delID = deleteRex.cap(1);
           // sendToAll(QString("/del_curve:" +user+" "+delID + "\n"));
             messagelist.push_back(QString("/del_curve:" +user+" "+delID + "\n"));
            qDebug() << "User:" << user<<" deleted Segment #"<< delID;
            bool delerror = DeleteSegment(delID);
            if(delerror==true)
                qDebug()<<"Segment Deleted.";
            else
                qDebug()<<"Cannot Find the Segment ";
        }
        else if(markerRex.indexIn(line) != -1) {
             QString user = clients.value(socket);
             QString markermsg = markerRex.cap(1);
            // sendToAll(QString("/marker:" +user+" "+markermsg + " \n"));
             messagelist.push_back(QString("/marker:" +user+" "+markermsg + " \n"));
             qDebug() << "User:" << user;
             qDebug() << "marker Position:" << markermsg;
        }
        else if(delmarkerRex.indexIn(line) != -1) {
             QString user = clients.value(socket);
             QString delmarkerpos = delmarkerRex.cap(1);
             //sendToAll(QString("/del_marker:" +user+" "+delmarkerpos + " \n"));
             messagelist.push_back(QString("/del_marker:" +user+" "+delmarkerpos + " \n"));
             qDebug() << "User:" << user;
             qDebug() << "delete marker Position:" << delmarkerpos;
        }
        else if(dragnodeRex.indexIn(line) != -1) {
             QString user = clients.value(socket);
             QString dragnodepos = dragnodeRex.cap(1);
             //sendToAll(QString("/drag_node:" +user+" "+dragnodepos + " \n"));
             messagelist.push_back(QString("/drag_node:" +user+" "+dragnodepos + " \n"));
             QStringList dragnodePOS = dragnodeRex.cap(1).split(" ");
             if(dragnodePOS.size()<5)
             {
                 qDebug()<<"error! size < 5";
                 return;
             }
             int ntnum = dragnodePOS.at(0).toInt();
             int swcnum = dragnodePOS.at(1).toInt();
             float mx = dragnodePOS.at(2).toFloat();
             float my = dragnodePOS.at(3).toFloat();
             float mz = dragnodePOS.at(4).toFloat();
             qDebug() << "User:" << user;
             qDebug() << "drag node' seiral number and new Position:" << dragnodepos;
             //update node position in server' NTList
             if((ntnum>=NeuronList.size())||(ntnum<0)) return;
             if((swcnum<0)||(swcnum>=NeuronList.at(ntnum).listNeuron.size())) return;
             NeuronSWC ss = NeuronList.at(ntnum).listNeuron.at(swcnum);
             ss.x = mx;
             ss.y = my;
             ss.z = mz;
             NeuronList[ntnum].listNeuron[swcnum]=ss;
        }
        else if (hmdposRex.indexIn(line) != -1) {
			//qDebug()<<"run here";
            QString user = clients.value(socket);
            QString hmd = hmdposRex.cap(1);
            sendToAll(QString("/hmdpos:" +user+" "+hmd + " \n"));
            //qDebug() << "User:" << user;
            //qDebug() << "HMD Position:" << hmd;
        }
        else if (askmessageRex.indexIn(line) != -1)
        {
            QString user = clients.value(socket);
            //QString msg = messageRex.cap(1);
            //qDebug()<<"update user message with "<<user;
            UpdateUsermessage(user);
        }    
        else {
            qDebug() << "Bad message from " << socket->peerAddress().toString();
        }
    }
}


void Server::AddToNTList(QString &msg, int type)
{
	QStringList qsl = QString(msg).trimmed().split(" ",QString::SkipEmptyParts);
	int str_size = qsl.size()-(qsl.size()%7);//to make sure that the string list size always be 7*N;
	//qDebug()<<"qsl.size()"<<qsl.size()<<"str_size"<<str_size;
	NeuronSWC S_temp;
	NeuronTree tempNT;
	tempNT.listNeuron.clear();
	tempNT.hashNeuron.clear();

	tempNT.name  = "sketch_"+ QString("%1").arg(sketchNum++);
	for(int i=0;i<str_size;i++)
	{
		qsl[i].truncate(99);
		//qDebug()<<qsl[i];
		int iy = i%7;
		if (iy==0)
		{
			S_temp.n = qsl[i].toInt();
		}
		else if (iy==1)
		{
			S_temp.type = type;
		}
		else if (iy==2)
		{
			S_temp.x = qsl[i].toFloat();

		}
		else if (iy==3)
		{
			S_temp.y = qsl[i].toFloat();

		}
		else if (iy==4)
		{
			S_temp.z = qsl[i].toFloat();

		}
		else if (iy==5)
		{
			S_temp.r = qsl[i].toFloat();

		}
		else if (iy==6)
		{
			S_temp.pn = qsl[i].toInt();

			tempNT.listNeuron.append(S_temp);
			tempNT.hashNeuron.insert(S_temp.n, tempNT.listNeuron.size()-1);
		}
	}//*/
	NeuronList.push_back(tempNT);
	//updateremoteNT
}
bool Server::DeleteSegment(QString segName)
{
	if(segName=="") return false;
	for(int i=0;i<NeuronList.size();i++)
	{
		QString NTname="";
		NTname = NeuronList.at(i).name;
		if(segName==NTname)
		{
			NeuronList.removeAt(i);
			return true;
		}
	}
	return false;
}
bool Server::containsClient(std::vector<clientproperty> Clients,QString name)
{
    for(int i=0;i<Clients.size();i++)
    {
        if(Clients[i].name == name )
        {
            return true;
        }
    }
    return false;
}
int Server::getuser(std::vector<clientproperty> clients,QString name)
{
    for(int i=0;i<clients.size();i++)
    {
        if(clients[i].name == name )
            return i;
    }
    return -1;
}
void Server::UpdateUsermessage(QString user)
{
        int messageindex = clientsproperty[getuser(clientsproperty,user)].messageindex;
   //send to clientsproperty[getuser(user)]
        //qDebug()<<"messageindex = "<<messageindex;
        //qDebug()<<"messagelist.size = "<<messagelist.size();
    if(messageindex < messagelist.size())
    {

        if(clientsproperty[getuser(clientsproperty,user)].online)
        {
            sendToUser(messagelist[messageindex],user);
            clientsproperty[getuser(clientsproperty,user)].messageindex++;
        qDebug()<<"send user message success";}
        else
            qDebug()<<"user is outline can't update messagelist";
    }

}











