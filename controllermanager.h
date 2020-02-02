#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#define CONTROLLER_UPDATE_FREQUENCY 10

#include <QObject>
#include <QGamepad>
#include <QDebug>
#include <QTimer>
#include <stdint.h>
#include <math.h>


struct controllerState {
  bool connected = false;
  bool buttonA = false;
  bool buttonB = false;
  bool buttonX = false;
  bool buttonY = false;
  bool dpadUp = false;
  bool dpadDown = false;
  bool dpadLeft = false;
  bool dpadRight = false;
  bool rbButton = false;
  bool lbButton = false;
  double leftXAxis = 0;
  double leftYAxis = 0;
  double rightXAxis = 0;
  double rightYAxis = 0;
  double rtTrigger = 0;
  double ltTrigger = 0;
};

extern "C" {
/*
    struct minimalControllerState {
        uint8_t leftXAxis;
        uint8_t leftYAxis;
        uint8_t rightXAxis;
        uint8_t rightYAxis;
        uint8_t ltTrigger;
        uint8_t rtTrigger;

        uint16_t buttons;
    };
*/
    enum buttons {
        a = 0,
        b,
        x,
        y,
        dUp,
        dDown,
        dLeft,
        dRight,
        rb,
        lb
    };
}

Q_DECLARE_METATYPE(controllerState)


class ControllerManager : public QObject
{
    Q_OBJECT
public:
    explicit ControllerManager(QObject *parent = nullptr);
    void convertToArray(uint8_t *data, uint8_t *len);
    ~ControllerManager();
signals:
    void controllerStateChanged(QVariant data);

public slots:
    void tryConnect();
    void controllerStateUpdate();

private:
    QGamepad *m_gamepad;
    QTimer *m_connectTimer;
    QTimer *m_controllerUpdateTimer;
    controllerState cState;
};

#endif // CONTROLLERMANAGER_H
