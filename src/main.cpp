#include <Arduino.h>
#include "mcp2515_can.h"

#define SERIAL_SPEED 115200 // one of most common serial port speeds
#define CAN_CS_PIN  10 // PIN D10
#define CAN_SPEED CAN_500KBPS // speed of CAN network (fixed for car)
#define CAN_CONTROLLER_SPEED MCP_8MHz // speed of crystal
#define DEBUG_MSG_RECEIVE 1 // change to 0 if using battery 
mcp2515_can can(CAN_CS_PIN); // creating CAN object

// This struct contains all the components of a CAN message. dataLength must be <= 8, 
// and the first [dataLength] positions of data[] must contain valid data
typedef uint8_t CanBuffer[8];
	struct CanMessage {
			uint32_t id;
			uint8_t dataLength;
			CanBuffer data;
	};

String getErrorDescription(int errorCode){
    switch(errorCode){
        case CAN_OK: 
            return "CAN OK";
            break;
        case CAN_FAILINIT:
            return "CAN FAIL INIT";
            break;
        case CAN_FAILTX:
            return "CAN FAIL TX";
            break;
        case CAN_MSGAVAIL:
            return "CAN MSG AVAIL";
            break;
        case CAN_NOMSG:
            return "CAN NO MSG";
            break;
        case CAN_CTRLERROR:
            return "CAN CTRL ERROR";
            break;
        case CAN_GETTXBFTIMEOUT:
            return "CAN TX BF TIMEOUT";
            break;
        case CAN_SENDMSGTIMEOUT:    
            return "CAN SEND MSG TIMEOUT";
            break;
        default:
            return "CAN FAIL";
            break;
    }
}

void setup() {
	Serial.begin(SERIAL_SPEED); // starts serial port to send stuff over port
	Serial.println("test");
	SPI.begin();

	// CAN begin
	int error = can.begin(CAN_SPEED, CAN_CONTROLLER_SPEED); // checks if it can communicate with mcp
	Serial.println("CAN Init Status: " + getErrorDescription(error));
}

void receive() {
    CanMessage message;
    message.dataLength = 0;
    can.readMsgBuf(&message.dataLength, message.data); 
    message.id = can.getCanId();
    if(DEBUG_MSG_RECEIVE) {

        Serial.println("-----------------------------");
        Serial.print("CAN MESSAGE RECEIVED - ID: 0x");
        Serial.println(message.id, HEX);

        for (int i = 0; i < message.dataLength; i++) { // print the data
            Serial.print("0x");
            Serial.print(message.data[i], HEX);
            Serial.print("\t");
        }
        Serial.println();
    }

}
void loop() {
     // Listen for CAN messages
    if (can.checkReceive() == CAN_MSGAVAIL) {
        receive();
    }
}

