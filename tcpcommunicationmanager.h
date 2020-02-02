#ifndef TCPCOMMUNICATIONMANAGER_H
#define TCPCOMMUNICATIONMANAGER_H

#include <QObject>
#include <QTcpSocket>

class TCPCommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit TCPCommunicationManager(QObject *parent = nullptr);
    void connectSocket(QString host, quint16 port);
    void disconnectSocket();

signals:
    void bytesAvailable(uint8_t *data, uint8_t len);
    void connectionChanged(bool isConnected);

private slots:
    void socketConnected();
    void socketDisconnected();
    void socketReadyRead();

public slots:
    bool sendData(uint8_t *data, uint8_t len);

private:
    QTcpSocket *m_tcpSocket;
    bool isSocketConnected;
};

#endif // TCPCOMMUNICATIONMANAGER_H
