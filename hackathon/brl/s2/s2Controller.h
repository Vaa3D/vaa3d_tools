

#ifndef S2CONTROLLER_H
#define S2CONTROLLER_H

#include <QWidget>
#include <QTcpSocket>
#include <QThread>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

QT_END_NAMESPACE

class Sleeper : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

// useful parameters stored in the class S2Data
class S2Data{
public:
    int resonantMode;
    int zMode;
    float opticalZoom;
    float pixelSizeX;
    float pixelSizeY;
    float pixelSizeZ;
    int pixelsPerLine;
    int rowsPerImage;
    float currentScanX;
    float currentScanY;
    float currentStageX;
    float currentStageY;
    float currentPiezoZ;
    float currentStepperZ;
    float currentPockelsCell;
    QString messageString;

    S2Data(int resonantMode=0,
           int zMode=0,
           float opticalZoom=1.0,
           float pixelSizeX=1.0,
           float pixelSizeY=1.0,
           float pixelSizeZ= 1.0,
           int pixelsPerLine = 256,
           int rowsPerImage = 256,
           float currentScanX = 0.0,
           float currentScanY = 0.0,
           float currentStageX = 0.0,
           float currentStageY = 0.0,
           float currentPiezoZ = 0.0,
           float currentStepperZ = 0.0,
           float currentPockelsCell = 0.0)
        : resonantMode(0),// 0 = nonresonant, 1 = resonant
          opticalZoom(1.0),// 0  = stepper , 1 = piezo
          pixelSizeX(1.0),
          pixelSizeY(1.0),
          pixelSizeZ(1.0),
          pixelsPerLine(256),
          rowsPerImage(256),
          currentScanX(0.0),
          currentScanY(0.0),
          currentStageX(0.0),
          currentStageY(0.0),
          currentPiezoZ(0.0),
          currentStepperZ(0.0),
          currentPockelsCell(0.0){}
};

class S2Controller : public QWidget
{
    Q_OBJECT

public:
    S2Controller(QWidget *parent = 0);
    S2Data myS2Data;
    bool okToSend = true;
    QString stringMessage;
    QString message;
    QString displayMessage;
public slots:
    void sendCommand();
    bool cleanAndSend(QString);
    void sendAndReceive(QString);
    void initializeParameters();
    void initializeS2();//    [set up microscope. Ideal version would include transferring all microscope parameters into internal attributes within Vaa3D.  Minimal version would just load a fixed configuration from an .xml file.
    //             /* some parameters will be stored in a separate class described below
    //                */
    void initROI();//    set up the microscope with appropriate parameters for small 3D ROI.  This could be done with a single .xml file from a saved configuration or through setting parameters from Vaa3D.
    void startROI(); //    set a target file location and trigger the 3D ROI.
    void getROIData();//    FILE VERSION: Wait for PV to signal ROI completion (?), wait for arbitrary delay or poll filesystem for available file
    //                        //SHARED MEMORY VERSION: during ROI initiation, Vaa3D will allocate a new 1d byte array and send the address and length to PV. It might be a bit tricky to know when this data is valid.
    void processROIData(); //Process image data and return 1 or more next locations.  Many alternative approaches could be used here, including: Run APP2 and locate ends of structure on boundary.  Identify foreground blobs in 1-D max or sum projections of ROI faces. Identify total intensity and variance in the entire ROI. Identify total tubularity in the ROI or near the edges, etc etc.  In any case, the resulting image coordinates will be transformed into coordinates that PV understands for (e.g.) "PanXY"  commands.
    void startNextROI();//    Move to the next ROI location and start the scan.  With the new 'PanXY' command, this should be trivial.
    void getPosition(int axis, int subAxis=0);
signals:
    void newS2Data(S2Data myS2Data);
    void messageIsComplete();
    void newMessage(QString message);
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
private:

    void convertCoordinates(); //Convert coordinates between image data (with a known pixel size, ROI galvo location, z stepper location, z piezo location and stage XY location) and sample location.  Reverse conversion will also be needed.
    void connectToS2();
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
    QPushButton *getReplyButton;
    QDialogButtonBox *buttonBox;

    QTcpSocket *tcpSocket;
    QString totalMessage;
    QString currentMessage;
    quint16 blockSize;

    QNetworkSession *networkSession;

};

// TODO:
// implement case/switch in controller for PV or other
// create listener/handler for data from PV this will
// ask about why QDialog::exec  is used for the plugin gui instead of .show()
// is there a .run()?  basically, I want the GUI open until I close it, but
// not blocking cout<< or whatever

// I need to clean up the S2controller code so the high-level stuff doesn't have any
// pv-specific syntax or variables.

// now that I have a growing list, I can parse the responses...
// split up the string between ACK and DONE and pull out the intervening text
// store that text in some kind of buffer?  that has a sequence of data...?
//   TODAY:  get file name back and parse
//           load that in vaa3d ?!
//
// -gts directory SingelImage  returns correct directory.
//  file should contain ch1  .ome.tif





#endif
