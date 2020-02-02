#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <stdint.h>
#include <math.h>
#include "tcpcommunicationmanager.h"

#ifndef Q_OS_IOS
#include "serialcommunicationmanager.h"
#endif

#define TARGET_ID   0x42            // ID to talk to
#define MY_ID       0x00            // My own ID
#define START_BYTE  0xAB

extern "C" {
    enum CMD {
        CONTROLLER = 0,             // Contains controller data
        READPARAM,                  // Reads the UGV Parameter
        WRITEPARAM,                 // Sends the UGV Parameter
        MODE,                       // Sets the UGV Mode
        BOOTLOADER,                 // Reboots to bootloader
        CHECKSUM,                   // Verify code checksum
        WRITE,                      // Write x amount of byte
        ERASE,                      // Erase x amount of byte
        READ,                       // Reads x number of byte
        JUMP,                       // Jump to user address
        STATUS,                     // Reply of a command
        PING,                       // PING Request
        PONG                        // PONG Reply
    };
}

enum commSM {
    START = 0,
    TARGET,
    SOURCE,
    COMMAND,
    LENGTH,
    PAYLOAD,
    CRC16
};

// START_BYTE           --  1 byte
// TARGET_ID            --  1 byte
// SOURCE_ID            --  1 byte
// COMMAND              --  1 byte
// PAYLOAD_LENGTH       --  1 byte
// PAYLOAD              --  [0 to 248] bytes
// CRC16                --  2 bytes
//               Max Total: 256 bytes

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(QObject *parent = nullptr);
    void reset();
    uint16_t calculateCRC16(uint8_t *arr, uint8_t len);

private slots:
    void processChar(uint8_t ch);
    void processMultipleBytes(uint8_t *data, uint8_t len);
    void processMessage(uint8_t *arr, uint8_t len);
    void serialStateChanged(bool isConnected);
    void TCPStateChanged(bool isConnected);
    void sendBytes(uint8_t* data, uint8_t len);

signals:
    void newMessageToProcess(uint8_t *arr, uint8_t len);
    void connectionChanged(bool isSerial, bool isConnected);

public slots:
    void startTCPComm(QString host, quint16 port);
    void stopTCPComm();
    void sendControllerData(uint8_t *data, uint8_t len);


#ifndef Q_OS_IOS
    void startSerialComm(QString port, int baud);
    void stopSerialComm();
    QList<QSerialPortInfo> listSerialPort();
#endif

private:
    commSM currentState = commSM::START;            // Our current state
    uint8_t dataBuffer[256];                        // Our incoming data buffer
    uint8_t crcCounter    = 2;
    uint8_t payloadLength = 0;
    uint8_t currentLength = 0;
    bool isSerialConnected = false;
    bool isTCPConnected = false;

    TCPCommunicationManager *m_tcpCommunicationManager;
#ifndef Q_OS_IOS
    SerialCommunicationManager *m_serialCommunicationManager;
#endif
};

#endif // COMMUNICATIONMANAGER_H
