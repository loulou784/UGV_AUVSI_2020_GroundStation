#include "tcpcommunicationmanager.h"

TCPCommunicationManager::TCPCommunicationManager(QObject *parent) : QObject(parent)
{
    m_tcpSocket = nullptr;
}

void TCPCommunicationManager::connectSocket(QString host, quint16 port) {
    if(m_tcpSocket != nullptr) {
        disconnectSocket();
    }

    m_tcpSocket = new QTcpSocket();

    m_tcpSocket->connectToHost(host, port);
    connect(m_tcpSocket, SIGNAL(connected()),this, SLOT(socketConnected()));
    connect(m_tcpSocket, SIGNAL(disconnected()),this, SLOT(socketDisconnected()));
    connect(m_tcpSocket, SIGNAL(readyRead()),this, SLOT(socketReadyRead()));
}

void TCPCommunicationManager::disconnectSocket() {
    if(m_tcpSocket != nullptr) {
        m_tcpSocket->disconnectFromHost();
        delete m_tcpSocket;
        m_tcpSocket = nullptr;
        isSocketConnected = false;
        emit connectionChanged(isSocketConnected);
    }
}

void TCPCommunicationManager::socketConnected() {
    isSocketConnected = true;
    emit connectionChanged(isSocketConnected);
}

void TCPCommunicationManager::socketDisconnected() {
    isSocketConnected = false;
    emit connectionChanged(isSocketConnected);
}

void TCPCommunicationManager::socketReadyRead() {
    QByteArray ba = m_tcpSocket->readAll();
    qDebug() << ba;
    uint8_t buf[ba.count()];
    for(int i = 0; i < ba.count(); i++) {
        buf[i] = (uint8_t)ba.at(i);
    }

    emit bytesAvailable(buf, sizeof(buf));
}

bool TCPCommunicationManager::sendData(uint8_t *data, uint8_t len) {
    if(m_tcpSocket != nullptr) {
        bool a = len == m_tcpSocket->write((const char*)data, len);
        m_tcpSocket->write("\n", 1);
        return a;
    }
    return false;
}
