#if !defined(Q_OS_WASM)

#include "tcpvideowidget.h"

TCPVideoWidget::TCPVideoWidget(QWidget *parent) : QWidget(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    m_server->listen(QHostAddress::Any, 2222);
    m_label = new QLabel();
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_label);
    this->setLayout(vLayout);
}

void TCPVideoWidget::readyRead() {
    // START: FF D8 FF
    // END: FF D9
    m_incomingData.append(m_socket->readAll());
    switch (m_state) {
        case STATE_HEADER:
            startIdx = m_incomingData.indexOf(header);
            if(startIdx != -1) {
                m_state = STATE_FOOTER;
            }
        break;

        case STATE_FOOTER:
            endIdx = m_incomingData.indexOf(footer);
            if(endIdx != -1) {
                // We have a picture
                m_currentImage.replace(startIdx, endIdx + footer.size(), m_incomingData);
                m_pixmap.loadFromData(m_currentImage,"JPG");
                m_label->setPixmap(m_pixmap.scaled(m_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_incomingData.remove(startIdx, endIdx + footer.size());
            }

            if(endIdx != -1) {
                m_state = STATE_HEADER;
            }
        break;
    }
    m_socket->flush();
}

void TCPVideoWidget::newConnection() {
    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(readyRead()),this, SLOT(readyRead()));
    qDebug() << "New Connection!";
}

#endif
