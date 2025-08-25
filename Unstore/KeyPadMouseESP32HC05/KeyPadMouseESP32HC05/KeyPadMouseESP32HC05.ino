// YOU NEED TO REMOVE GAMEPAD HID COMPOSED TO USE BLEGAMEPAD.
//
//REMOVE THE HID GAMEPAD OF C:\Users\USERNAME\Documents\Arduino\libraries
//https://github.com/Mystfit/ESP32-BLE-CompositeHID/releases/tag/v0.3.1

//ADD THE HID GAMEPAD OF C:\Users\USERNAME\Documents\Arduino\libraries
//https://github.com/lemmingDev/ESP32-BLE-Gamepad/releases/tag/v0.7.3
//https://github.com/T-vK/ESP32-BLE-Mouse/releases/tag/0.3.1
//https://github.com/T-vK/ESP32-BLE-Keyboard/releases/tag/0.3.0

#include <Arduino.h>
#include <BleGamepad.h>
//#include <BleMouse.h>
#include <BleKeyboard.h>

#define RXD2 16   // GPIO16 (U2RXD)
#define TXD2 17   // GPIO17 (U2TXD)

BleKeyboard bleKeyboard;
//BleMouse bleMouse;
BleGamepad bleGamepad;

int m_allowToWritePinByDeveloper[] = {

    // Line One: 19,20,21,22,47,48,45, 0,35,36,37,38,39,40,41,42,2,1,
    // Line Two: 14,13,12,11,10, 9,46, 3, 8,18,17,16,15,7, 6, 5, 4
    2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

bool m_return_byte_received = true;
bool m_use_print_byte_debug = true;

void setup() {
    
        Serial.begin(115200);
        Serial.println("Starting BLE work!");
        Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
        Serial.println("UART1 initialized.");
        bleGamepad.begin();
      //bleKeyboard.begin();
      // bleMouse.begin();
    }

void loop() {
  if (Serial2.available()) {
    // Read the incoming data and send it to the Serial monitor
    byte incomingByte = Serial2.read();

    //SEND BACK BYTE TAKE TIME DISABLE IF YOU ARE NOT USING IT
    if(m_return_byte_received){
        Serial2.write(incomingByte);
    }

    char c = (char)incomingByte;
    if (m_use_print_byte_debug) {
        Serial.print("Received: ");
        Serial.println(c);
    }
  }
  delay(1);
}
