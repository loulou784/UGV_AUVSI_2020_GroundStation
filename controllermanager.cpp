#include "controllermanager.h"

ControllerManager::ControllerManager(QObject *parent) : QObject(parent), m_gamepad(nullptr), m_connectTimer(nullptr)
{
    // Check for new controller every seconds
    m_connectTimer = new QTimer();
    connect(m_connectTimer, SIGNAL(timeout()), this, SLOT(tryConnect()));
    m_connectTimer->setInterval(1000);
    m_connectTimer->start();

    // Emits the controller state every CONTROLLER_UPDATE_FREQUENCY in Hz
    m_controllerUpdateTimer = new QTimer();
    connect(m_controllerUpdateTimer, SIGNAL(timeout()), this, SLOT(controllerStateUpdate()));
    m_controllerUpdateTimer->setInterval((1000/CONTROLLER_UPDATE_FREQUENCY));
    m_controllerUpdateTimer->start();
}

void ControllerManager::tryConnect() {
    if(m_gamepad != nullptr) return;

    auto gamepads = QGamepadManager::instance()->connectedGamepads();

    if (gamepads.isEmpty()) {
        return;
    }

    m_gamepad = new QGamepad(*gamepads.begin(), this);

    connect(m_gamepad, &QGamepad::axisLeftXChanged, this, [this](double value){ this->cState.leftXAxis = value; });
    connect(m_gamepad, &QGamepad::axisLeftYChanged, this, [this](double value){ this->cState.leftYAxis = value; });
    connect(m_gamepad, &QGamepad::axisRightXChanged, this, [this](double value){ this->cState.rightXAxis = value; });
    connect(m_gamepad, &QGamepad::axisRightYChanged, this, [this](double value){ this->cState.rightYAxis = value; });
    connect(m_gamepad, &QGamepad::buttonAChanged, this, [this](bool value){ this->cState.buttonA = value; });
    connect(m_gamepad, &QGamepad::buttonBChanged, this, [this](bool value){ this->cState.buttonB = value; });
    connect(m_gamepad, &QGamepad::buttonXChanged, this, [this](bool value){ this->cState.buttonX = value; });
    connect(m_gamepad, &QGamepad::buttonYChanged, this, [this](bool value){ this->cState.buttonY = value; });
    connect(m_gamepad, &QGamepad::connectedChanged, this, [this](bool value){ this->cState.connected = value; });
    connect(m_gamepad, &QGamepad::buttonUpChanged, this, [this](bool value){ this->cState.dpadUp = value; });
    connect(m_gamepad, &QGamepad::buttonDownChanged, this, [this](bool value){ this->cState.dpadDown = value; });
    connect(m_gamepad, &QGamepad::buttonLeftChanged, this, [this](bool value){ this->cState.dpadLeft = value; });
    connect(m_gamepad, &QGamepad::buttonRightChanged, this, [this](bool value){ this->cState.dpadRight = value; });
    connect(m_gamepad, &QGamepad::buttonL1Changed, this, [this](bool value){ this->cState.lbButton = value; });
    connect(m_gamepad, &QGamepad::buttonR1Changed, this, [this](bool value){ this->cState.rbButton = value; });
    connect(m_gamepad, &QGamepad::buttonR2Changed, this, [this](double value){ this->cState.rtTrigger = value; });
    connect(m_gamepad, &QGamepad::buttonL2Changed, this, [this](double value){ this->cState.ltTrigger = value; });
}

uint8_t mapValue(double x) {
    uint8_t val = ceil((x - (-1.0f)) * (255.0f - 0.0f) / ((1.0f) - (-1.0f)) + 0.0f);
    return val;
}

void ControllerManager::convertToArray(uint8_t *data, uint8_t *len) {
    data[0] = mapValue(this->cState.leftXAxis);     //lx
    data[1] = mapValue(this->cState.leftYAxis);     //ly
    data[2] = mapValue(this->cState.rightXAxis);    //rx
    data[3] = mapValue(this->cState.rightYAxis);    //ry
    data[4] = ceil(this->cState.ltTrigger*255);     //lt
    data[5] = ceil(this->cState.rtTrigger*255);     //rt
    uint16_t btn = 0;

    btn |= this->cState.buttonA   << buttons::a;
    btn |= this->cState.buttonB   << buttons::b;
    btn |= this->cState.buttonX   << buttons::x;
    btn |= this->cState.buttonY   << buttons::y;
    btn |= this->cState.dpadUp    << buttons::dUp;
    btn |= this->cState.dpadDown  << buttons::dDown;
    btn |= this->cState.dpadLeft  << buttons::dLeft;
    btn |= this->cState.dpadRight << buttons::dRight;
    btn |= this->cState.rbButton  << buttons::rb;
    btn |= this->cState.lbButton  << buttons::lb;

    btn = (btn << 8) + (btn >> 8);
    memcpy(&data[5], &btn, 2);
    *len = 8;
}

void ControllerManager::controllerStateUpdate() {
    QVariant data;
    data.setValue(this->cState);
    emit controllerStateChanged(data);
    // TODO: Send button struct when needed
}

ControllerManager::~ControllerManager()
{
    delete m_gamepad;
}
