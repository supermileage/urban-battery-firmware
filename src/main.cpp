#include <Arduino.h>
#include <Adafruit_SH110X.h>
#include "mcp2515_can.h"
#include "can_common.h"

#define SERIAL_SPEED 115200 // one of most common serial port speeds
#define CAN_CS_PIN  10 // PIN D10
#define CAN_SPEED CAN_500KBPS // speed of CAN network (fixed for car)
#define CAN_CONTROLLER_SPEED MCP_8MHz // speed of crystal
#define DEBUG_MSG_RECEIVE 0 // change to 0 if using battery
#define CAN_BATT_SOC_SCALING_FACTOR 2.0
#define CAN_BATT_VOLTAGE_SCALING_FACTOR 10.0
#define BUTTON_PIN 5 // PIN D5

mcp2515_can can(CAN_CS_PIN); // creating CAN object

// Global varible declaration
float g_voltage = -1.0;
float g_soc = -1.0;
int currentButtonState;
int lastButtonState;
bool screenState; 

// This struct contains all the components of a CAN message. dataLength must be <= 8, 
// and the first [dataLength] positions of data[] must contain valid data
typedef uint8_t CanBuffer[8];
	struct CanMessage {
			uint32_t id;
			uint8_t dataLength;
			CanBuffer data;
	};

// pass in width and length of screen
Adafruit_SH1107 oled(64, 128); 

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

    // Button set up
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    currentButtonState = digitalRead(BUTTON_PIN);

    // Screen initialization
    oled.begin(0x3C, false);
    oled.setRotation(1);

    oled.clearDisplay();
    oled.display();
}

void displayToScreen(){
// Using global variables

    oled.clearDisplay();
    oled.setTextColor(SH110X_WHITE);

    oled.setCursor(5, 5);
    oled.setTextSize(1);
    oled.write("volt ");
    oled.setTextSize(3);
    oled.write(String(g_voltage).c_str());

    oled.setCursor(5, 35);
    oled.setTextSize(1);
    oled.write("soc ");
    oled.setTextSize(3);
    oled.write(String(g_soc).c_str());

    oled.display();
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
    if (message.id == CAN_ORIONBMS_PACK) {
            uint8_t socData = message.data[4];
            g_soc = socData / CAN_BATT_SOC_SCALING_FACTOR;
            Serial.print("state of charge = ");
            Serial.println(g_soc);
            uint16_t voltageData = message.data[1] | message.data[0] << 8;
            g_voltage = voltageData / CAN_BATT_VOLTAGE_SCALING_FACTOR;
            Serial.print("voltage = ");
            Serial.println(g_voltage);
    }
}
void loop() {
    // save the last state
    lastButtonState = currentButtonState; 
    // read new state
    currentButtonState = digitalRead(BUTTON_PIN); 
    if (lastButtonState == HIGH && currentButtonState == LOW){
        if (screenState == true){
            screenState = false;
            oled.clearDisplay();
            oled.display();
        }
        else{
            screenState = true;
        }
    }
    if (screenState == true){
        displayToScreen();
    }
    
     // Listen for CAN messages
    if (can.checkReceive() == CAN_MSGAVAIL) {
        receive();

    }
}

