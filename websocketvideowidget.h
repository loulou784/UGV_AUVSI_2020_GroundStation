#ifndef WEBSOCKETVIDEOWIDGET_H
#define WEBSOCKETVIDEOWIDGET_H

#include <QObject>
#include <QWidget>

class WebSocketVideoWidget : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketVideoWidget(QObject *parent = nullptr);

signals:

};

#endif // WEBSOCKETVIDEOWIDGET_H
