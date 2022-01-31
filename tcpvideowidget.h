#ifndef TCPVIDEOWIDGET_H
#define TCPVIDEOWIDGET_H

#include <QObject>
#include <QWidget>

#if !defined(Q_OS_WASM)

#include <QLabel>
#include <QVBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>


#define STATE_HEADER 0
#define STATE_FOOTER 1

class TCPVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TCPVideoWidget(QWidget *parent = nullptr);

private slots:
    void readyRead();
    void newConnection();

signals:

private:
    QByteArray header = QByteArray::fromHex("FFD8FF");
    QByteArray footer = QByteArray::fromHex("FFD9");
    QTcpServer *m_server;
    QTcpSocket *m_socket;
    uint8_t m_state = STATE_HEADER;
    QPixmap m_pixmap;
    QByteArray m_currentImage;
    QByteArray m_incomingData;
    QLabel *m_label;
    int startIdx = 0;
    int endIdx = 0;
};
#endif

#endif // TCPVIDEOWIDGET_H
