/*
* bamocar-due.cpp
* Based on timoxd7/Bamocar-CAN
* Ported for the Arduino Due by Hari Mohan
*/

#include "bamocar-due.h"

// necessary to define the static member declared in bamocar-due.h
Bamocar* Bamocar::instance = nullptr; 

void Bamocar::setRxID(uint16_t rxID) {
    _rxID = rxID;
}

void Bamocar::setTxID(uint16_t txID) {
    _txID = txID;
}

bool Bamocar::_sendCAN(M_data m_data) {
    CAN_FRAME msg;

    msg.data = m_data.getData();
    msg.id = _rxID;
    msg.length = m_data.length();

    return Can0.sendFrame(msg);
}

bool Bamocar::_requestData(uint8_t requestedRegID, uint8_t interval) {
    return _sendCAN(M_data(REG_REQUEST, requestedRegID, interval));
}

void Bamocar::_forwardFrame(CAN_FRAME *msg) {
    // if (instance) {
    //     instance->_parseMessage(*msg);
    // }

    instance->_parseMessage(*msg);
}

void Bamocar::_parseMessage(CAN_FRAME &msg) {
    int64_t receivedData;

    if (msg.length == 4) {
        receivedData = _getReceived16Bit(msg);
    } else {
        receivedData = _getReceived32Bit(msg);
    }

    switch (msg.data.bytes[0]) {
        case REG_STATUS:
            _rcvd.STATUS = receivedData;
            break;

        case REG_READY:
            _rcvd.READY = receivedData;
            break;

        case REG_N_ACTUAL:
            _rcvd.N_ACTUAL = receivedData;
            break;

        case REG_N_MAX:
            _rcvd.N_MAX = receivedData;
            break;

        case REG_I_ACTUAL:
            _rcvd.I_ACTUAL = receivedData;
            break;

        case REG_I_DEVICE:
            _rcvd.I_DEVICE = receivedData;
            break;

        case REG_I_200PC:
            _rcvd.I_200PC = receivedData;
            break;

        case REG_TORQUE:
            _rcvd.TORQUE = receivedData;
            break;

        case REG_RAMP_ACC:
            _rcvd.RAMP_ACC = receivedData;
            break;

        case REG_RAMP_DEC:
            _rcvd.RAMP_DEC = receivedData;
            break;

        case REG_TEMP_MOTOR:
            _rcvd.TEMP_MOTOR = receivedData;
            break;

        case REG_TEMP_IGBT:
            _rcvd.TEMP_IGBT = receivedData;
            break;

        case REG_TEMP_AIR:
            _rcvd.TEMP_AIR = receivedData;
            break;

        case REG_HARD_ENABLED:
            _rcvd.HARD_ENABLED = receivedData;
            break;

        default:
            break;
    }
}

int16_t Bamocar::_getReceived16Bit(CAN_FRAME &msg) {
    int16_t val;

    val = msg.data.bytes[1];
    val |= (msg.data.bytes[2] << 8);

    return val;
}

int32_t Bamocar::_getReceived32Bit(CAN_FRAME &msg) {
    int32_t val;

    val = msg.data.bytes[1];
    val |= (msg.data.bytes[2] << 8);
    val |= (msg.data.bytes[3] << 16);
    val |= (msg.data.bytes[4] << 24);

    return val;
}

// ----------------------------------------------------------------------------

float Bamocar::getSpeed() {
    return _rcvd.N_MAX * (_rcvd.N_ACTUAL / 32767);
}

bool Bamocar::setSpeed(int16_t speed) {
    return _sendCAN(M_data(REG_N_CMD, speed));
}

bool Bamocar::requestSpeed(uint8_t interval) {
    bool success = true;
    if (!_requestData(REG_N_ACTUAL, interval))
        success = false;

    if (!_requestData(REG_N_MAX, interval))
        success = false;

    return success;
}

bool Bamocar::setAccel(int16_t accel) {
    return _sendCAN(M_data(REG_RAMP_ACC, accel));
}

bool Bamocar::setDecel(int16_t decel) {
    return _sendCAN(M_data(REG_RAMP_DEC, decel));
}

float Bamocar::getTorque() {
    return _rcvd.TORQUE / 32760;
}

bool Bamocar::setTorque(float torque) {
    if (torque > TORQUE_MAX_PERCENT)
        torque = TORQUE_MAX_PERCENT;

    int16_t torque16 = torque * 32760;
    return _sendCAN(M_data(REG_TORQUE, torque16));
}

bool Bamocar::requestTorque(uint8_t interval) {
    return _requestData(REG_TORQUE, interval);
}

// not really sure how the current stuff works, will need to test
// directly to make sure
float Bamocar::getCurrent() {
    return ((2/10) * _rcvd.I_DEVICE * (_rcvd.I_ACTUAL / _rcvd.I_200PC));
}

bool Bamocar::requestCurrent(uint8_t interval) {
    bool success = true;
    if (!_requestData(REG_I_ACTUAL, interval))
        success = false;

    if (!_requestData(REG_I_DEVICE, interval))
        success = false;

    if (!_requestData(REG_I_200PC, interval))
        success = false;

    return success;
}

uint16_t Bamocar::getMotorTemp() {
    return _rcvd.TEMP_MOTOR;
}

bool Bamocar::requestMotorTemp(uint8_t interval) {
    return _requestData(REG_TEMP_MOTOR, interval);
}

uint16_t Bamocar::getControllerTemp() {
    return _rcvd.TEMP_IGBT;
}

bool Bamocar::requestControllerTemp(uint8_t interval) {
    return _requestData(REG_TEMP_IGBT, interval);
}

uint16_t Bamocar::getAirTemp() {
    return _rcvd.TEMP_AIR;
}

bool Bamocar::requestAirTemp(uint8_t interval) {
    return _requestData(REG_TEMP_AIR, interval);
}

// Status
uint16_t Bamocar::getStatus() {
    return _rcvd.STATUS;
}

bool Bamocar::requestStatus(uint8_t interval) {
    return _requestData(REG_STATUS, interval);
}

// Enable
void Bamocar::setSoftEnable(bool enable) {
    uint8_t m_data2 = 0;

    if (enable) {
        m_data2 = 0x00;
    } else {
        m_data2 = 0x04;
    }

    _sendCAN(M_data(REG_ENABLE, m_data2, 0x00));
}

bool Bamocar::getHardEnable() {
    return _rcvd.HARD_ENABLED;
}

bool Bamocar::requestHardEnabled(uint8_t interval) {
    return _requestData(REG_HARD_ENABLED, interval);
}