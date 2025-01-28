/*

bamocar-due.h

*/

#pragma once

#include <Arduino.h>
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
#include "due_can.h"
#include "bamocar-registers.h"
#include <functional>

// #define CAN_TIMEOUT 0.01
#define TORQUE_MAX_PERCENT 1.00

class M_data {
    public:
        M_data(uint8_t regID, uint16_t m_data16) {
            data.bytes[0] = regID;
            data.bytes[1] = m_data16 & 0xFF;
            data.bytes[2] = m_data16 >> 8;
            dataLength = 3;
        }

        M_data(uint8_t regID, int16_t m_data16) {
            data.bytes[0] = regID;
            data.bytes[1] = m_data16 & 0xFF;
            data.bytes[2] = m_data16 >> 8;
            dataLength = 3;
        }

        M_data(uint8_t regID, uint32_t m_data32) {
            data.bytes[0] = regID;
            data.bytes[1] = m_data32 & 0xFF;
            data.bytes[2] = (m_data32 >> 8) & 0xFF;
            data.bytes[3] = (m_data32 >> 16) & 0xFF;
            data.bytes[4] = (m_data32 >> 24) & 0xFF;
            dataLength = 5;
        }

        M_data(uint8_t regID, int32_t m_data32) {
            data.bytes[0] = regID;
            data.bytes[1] = m_data32 & 0xFF;
            data.bytes[2] = (m_data32 >> 8) & 0xFF;
            data.bytes[3] = (m_data32 >> 16) & 0xFF;
            data.bytes[4] = (m_data32 >> 24) & 0xFF;
            dataLength = 5;
        }

        M_data(uint8_t regID, uint8_t requestedRegID, uint8_t interval) {
            data.bytes[0] = regID;
            data.bytes[1] = requestedRegID;
            data.bytes[2] = interval;
            dataLength = 3;
        }

        // uint8_t get(uint8_t position) {
        //     if(position >= dataLength) return 0;
        //     return data.bytes[position];
        // }

        BytesUnion getData() {
            return data;
        }

        uint8_t length() {
            return dataLength;
        }
    
    protected:
        BytesUnion data;
        uint8_t dataLength;
};

// -------------------------------------

class Bamocar {
    public:
        Bamocar(uint8_t mailbox, int frequency = STD_BAUD_RATE) {
            instance = this;
            Can0.setCallback(mailbox, _forwardFrame);

            _rxID = STD_RX_ID;
            _txID = STD_TX_ID;
        }

        float getSpeed();
        bool setSpeed(int16_t speed);
        bool requestSpeed(uint8_t interval = INTVL_IMMEDIATE);

        bool setAccel(int16_t accel);
        bool setDecel(int16_t decel);

        float getTorque();
        bool setTorque(float torque);
        bool requestTorque(uint8_t interval = INTVL_IMMEDIATE);

        float getCurrent();
        bool requestCurrent(uint8_t interval = INTVL_IMMEDIATE);

        uint16_t getMotorTemp();
        uint16_t getControllerTemp();
        uint16_t getAirTemp();
        bool requestMotorTemp(uint8_t interval = INTVL_IMMEDIATE);
        bool requestControllerTemp(uint8_t interval = INTVL_IMMEDIATE);
        bool requestAirTemp(uint8_t interval = INTVL_IMMEDIATE);

        uint16_t getStatus();
        bool requestStatus(uint8_t interval = INTVL_IMMEDIATE);

        void setSoftEnable(bool enable);
        bool getHardEnable();
        bool requestHardEnabled(uint8_t interval = INTVL_IMMEDIATE);

        void setRxID(uint16_t rxID);
        void setTxID(uint16_t txID);

    protected:
        static Bamocar *instance;
        uint16_t _rxID;
        uint16_t _txID;

        struct _rcvd {
            int16_t N_ACTUAL = 0, N_MAX = 0, TORQUE = 0;
            uint16_t READY = 0,
                    I_ACTUAL = 0, I_DEVICE = 0, I_200PC = 0,
                    RAMP_ACC = 0, RAMP_DEC = 0, 
                    TEMP_MOTOR = 0, TEMP_IGBT = 0, TEMP_AIR = 0,
                    HARD_ENABLED = 0;
            uint32_t STATUS = 0;
        } _rcvd;
    
        bool _sendCAN(M_data m_data);
        bool _requestData(uint8_t requestedRegID, uint8_t interval = INTVL_IMMEDIATE);
        static void _forwardFrame(CAN_FRAME *msg);
        void _parseMessage(CAN_FRAME &msg);
        int16_t _getReceived16Bit(CAN_FRAME &msg);
        int32_t _getReceived32Bit(CAN_FRAME &msg);
};