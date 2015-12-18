

#ifndef S2CLIENT_H
#define S2CLIENT_H

#include <QDialog>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

//! [0]
class S2Client : public QDialog
{
    Q_OBJECT

public:
    S2Client(QWidget *parent = 0);

private slots:
    void sendCommand();
    void readPV();
    void displayError(QAbstractSocket::SocketError socketError);
    void enablesendCommandButton();
    void sessionOpened();
	void cleanAndSend(QString);
	void sendX();
	void cleanUp();
	void connectToPV();
    void initConnection(); //[initialize connection to PV over TCP/IP]
    void initScope();//    [set up microscope. Ideal version would include transferring all microscope parameters into internal attributes within Vaa3D.  Minimal version would just load a fixed configuration from an .xml file.
             /* some parameters will be stored in a separate class described below
                */
    void initROI();//    set up the microscope with appropriate parameters for small 3D ROI.  This could be done with a single .xml file from a saved configuration or through setting parameters from Vaa3D.
    void startROI(); //    set a target file location and trigger the 3D ROI.
    void getROIData();//    FILE VERSION: Wait for PV to signal ROI completion (?), wait for arbitrary delay or poll filesystem for available file
                        //SHARED MEMORY VERSION: during ROI initiation, Vaa3D will allocate a new 1d byte array and send the address and length to PV. It might be a bit tricky to know when this data is valid.
    void processROIData(); //Process image data and return 1 or more next locations.  Many alternative approaches could be used here, including: Run APP2 and locate ends of structure on boundary.  Identify foreground blobs in 1-D max or sum projections of ROI faces. Identify total intensity and variance in the entire ROI. Identify total tubularity in the ROI or near the edges, etc etc.  In any case, the resulting image coordinates will be transformed into coordinates that PV understands for (e.g.) "PanXY"  commands.
    void startNextROI();//    Move to the next ROI location and start the scan.  With the new 'PanXY' command, this should be trivial.
    void convertCoordinates(); //Convert coordinates between image data (with a known pixel size, ROI galvo location, z stepper location, z piezo location and stage XY location) and sample location.  Reverse conversion will also be needed.

private:
    QLabel *hostLabel;
    QLabel *portLabel;
	QLabel *cmdLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
	QLineEdit *cmdLineEdit;
    QLabel *statusLabel;
    QPushButton *sendCommandButton;
    QPushButton *quitButton;
	QPushButton *connectButton;
    QDialogButtonBox *buttonBox;

    QTcpSocket *tcpSocket;
    QString currentMessage;
    quint16 blockSize;

    QNetworkSession *networkSession;

    QObject ss2Params;
    /*  this will include:
     * optical zoom
     * pixel size in microns
     * pixels/line
     * rows/image
     * resonant/nonresonant mode
    * Current scan xy location in microns (within field of view)
    * xy stage position (for future use  )
    * piezo z position
    * stepper z position.
    * piezo v. stepper z mode
    * Pockels cell voltage
    *  ???
    * this object may include polling on its own*/

};
//! [0]

#endif
