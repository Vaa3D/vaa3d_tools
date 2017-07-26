

#ifndef S2CONTROLLER_H
#define S2CONTROLLER_H

#include <QWidget>
#include <QTcpSocket>
#include <QThread>
#include <QMap>
#include <QGroupBox>
#include <v3d_interface.h>


QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;
QT_END_NAMESPACE


/*! \brief simple class to contain microscope information
 *
 *  each S2Parameter has a string to send to PV, a current response string from PV and a current value from PV
 *  along with the expected type of the value, e.g. float for a parameter like the stage position
 *
 * this class can more-or-less be used for any hardware implementation, depending on exactly where the microscope controls are wrapped.
 */
class S2Parameter
{
public:
	S2Parameter();//defined with initialization list

	S2Parameter(QString parameterN,
				QString sendS,
				float currentV=0.0,
				QString currentS="",
				QString expectedT="float");

	QString getSendString();
	void setCurrentString(QString inputString);
	QString getCurrentString();
	void setCurrentValue(float value);
	float getCurrentValue();
	QString getParameterName();
	QString getExpectedType();
private:
	QString parameterName;
	QString sendString;
	QString expectedType;
	float currentValue;
	QString currentString;
};





/*! \brief main class for controlling the microscope
 *
 *  the current implementation with PrairieView uses PV's TCP/IP scripting commands
 *  to communicate with PV.  This class wraps many of those commands and can be extended.
 * S2Controller operates in 2 modes, switched by S2Controller.inPosMonMode .  S2Controller::startPosMon()
 * initiates the posMon ("position monitor") mode which cycles through all the parameters in S2UI.s2ParameterMap,
 * querying PV for each value, updating it and then querying the next value through
 * S2UI::posMon()
 * S2UI.tcpSocket::readyRead()
 * S2UI::checkForMessage()
 * S2UI::processMessage()
 * S2UI::newMessage()
 * S2UI::posMonListener()
 */

enum opMode {offline, online};

class S2Controller : public QWidget
{
	Q_OBJECT

public:
	S2Controller(QWidget *parent = 0);
	bool okToSend;
	QString stringMessage;
	QString message;
	QString displayMessage;
	QMap<int, S2Parameter> s2ParameterMap;
	QLineEdit *hostLineEdit;
	bool inPosMonMode;
	bool cancelPosMon;
	opMode mode;

public slots:
	void sendCommand();
	bool cleanAndSend(QString);
	bool sendAndReceive(QString);
	void initializeParameters();
	void initializeS2();//    [set up microscope. Ideal version would include transferring all microscope parameters into internal attributes within Vaa3D.  Minimal version would just load a fixed configuration from an .xml file.
	//             /* some parameters will be stored in a separate class described below
	//                */
	void initROI(LocationSimple);//    set up the microscope with appropriate parameters for small 3D ROI.  This could be done with a single .xml file from a saved configuration or through setting parameters from Vaa3D.
	void initROIwithStage(LocationSimple, float xStage, float yStage);
	void startPosMon();
	void stopPosMon();
	bool getPosMon();
	void startScan();
	void closeEvent(QCloseEvent *event);
	void centerGalvos();
	void startZStack();
	void overviewSetup();
	void stackSetup();
	void stackSetup(float zsize, float zoom, int pixelsPerLine, int linesPerFrame);
	void cancelQueueSlot();
	void addToQueue(QString cString);
signals:
	void messageIsComplete();
	void newMessage(QString message);
	void newBroadcast(QString message);
	void newPosMonIndex();
	void pmStatus(bool inPosMonMode);
	void newS2Parameter( QMap<int, S2Parameter> parameterMap);
	void statusSig(QString statusMessage);
	
	// Signals to simulated scope, MK, July 2017
	void shootFakeScope(LocationSimple, float, float);

private slots:
	void checkForMessage();
	void processMessage();
	void messageHandler(QString messageH);
	void displayError(QAbstractSocket::SocketError socketError);
	void enablesendCommandButton();
	void sessionOpened();
	void sendX();
	void cleanUp();
	void initConnection(); //[initialize connection to PV over TCP/IP]
	void posMon();
	void posMonListener(QString messageL);
	void overviewHandler();
	void commandQueueMonitor();
	void tryToSend();
private:
	void convertCoordinates(); //Convert coordinates between image data (with a known pixel size, ROI galvo location, z stepper location, z piezo location and stage XY location) and sample location.  Reverse conversion will also be needed.
	void connectToS2();
	QLabel *hostLabel;
	QLabel *portLabel;
	QLabel *cmdLabel;

	QLineEdit *portLineEdit;
	QLineEdit *cmdLineEdit;
	QLabel *statusLabel;
	QPushButton *sendCommandButton;
	QPushButton *quitButton;
	QPushButton *connectButton;
	QPushButton *getReplyButton;
	QGroupBox *buttonBox;

	QTcpSocket *tcpSocket;
	QString totalMessage;
	QString currentMessage;
	QStringList commandQueue;
	QString commandToSend;
	quint16 blockSize;
	int ii;
	int maxParams;//temp!
	QNetworkSession *networkSession;
	void status(QString statusM);
	bool cancelQueue;
	bool newCommandToSend;
};







#endif
