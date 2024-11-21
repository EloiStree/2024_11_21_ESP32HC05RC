#include <Arduino.h>
#include <BleGamepad.h>
#define RXD2 16   // GPIO16 (U2RXD)
#define TXD2 17   // GPIO17 (U2TXD)

BleGamepad bleGamepad;

char cLeft = ' ';
char cRight = ' ';

bool m_printDebug = true;

int MIN_VALUE = 0;
int MAX_VALUE = 32767;
int MIN_VALUE_D2 = (int)(MAX_VALUE * 1.0 / 4.0);
int MAX_VALUE_D2 = (int)(MAX_VALUE * 3.0 / 4.0);
int MIN_VALUE_D3 = (int)(MAX_VALUE * 2.0 / 6.0);
int MAX_VALUE_D3 = (int)(MAX_VALUE * 4.0 / 6.0);
int MIN_VALUE_D4 = (int)(MAX_VALUE * 3.0 / 8.0);
int MAX_VALUE_D4 = (int)(MAX_VALUE * 5.0 / 8.0);
int MIN_VALUE_D8 = (int)(MAX_VALUE * 7.0 / 16.0);
int MAX_VALUE_D8 = (int)(MAX_VALUE * 9.0 / 16.0);

int MIDDLE_VALUE = 32767 /2;

float m_axis0 = 0;
float m_axis1 = 0;
float m_axis2 = 0;
float m_axis3 = 0;
float m_axis4 = 0;
float m_axis5 = 0;
float m_axis6 = 0;
float m_axis7 = 0;

bool isCharDigital(char c) {
    return c >= '0' && c <= '9';
}

void setup() {
 
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
    Serial.println("UART1 initialized.");
    bleGamepad.begin();
}
void loop() {
  if (Serial2.available()) {
      // Read the incoming data and send it to the Serial monitor
      byte incomingByte = Serial2.read();
      Serial2.write(incomingByte);
      char c = (char)incomingByte;
      Serial.print("Received: ");
      Serial.println(c);
         cLeft = cRight;
        cRight = c;
        if (isCharDigital(cRight)) {
            uartCommand(cLeft, cRight);
        }
    }
    delay(10);
}




void uartCommand(char cLeft, char cRight) {
    if (m_printDebug) {
        Serial.print("Left: ");
        Serial.print(cLeft);
        Serial.print(" Right: ");
        Serial.println(cRight);
    }

    if (!bleGamepad.isConnected())
        return;

    if (cRight == '0') {
        switch (cLeft) {
            case 'A': bleGamepad.press(BUTTON_1); break;
            case 'a': bleGamepad.release(BUTTON_1); break;
            case 'B': bleGamepad.press(BUTTON_2); break;
            case 'b': bleGamepad.release(BUTTON_2); break;
            case 'C': bleGamepad.press(BUTTON_3); break;
            case 'c': bleGamepad.release(BUTTON_3); break;
            case 'D': bleGamepad.press(BUTTON_4); break;
            case 'd': bleGamepad.release(BUTTON_4); break;
            case 'E': bleGamepad.press(BUTTON_5); break;
            case 'e': bleGamepad.release(BUTTON_5); break;
            case 'F': bleGamepad.press(BUTTON_6); break;
            case 'f': bleGamepad.release(BUTTON_6); break;
            case 'G': bleGamepad.press(BUTTON_7); break;
            case 'g': bleGamepad.release(BUTTON_7); break;
            case 'H': bleGamepad.press(BUTTON_8); break;
            case 'h': bleGamepad.release(BUTTON_8); break;
            case 'I': bleGamepad.press(BUTTON_9); break;
            case 'i': bleGamepad.release(BUTTON_9); break;
            case 'J': bleGamepad.press(BUTTON_10); break;
            case 'j': bleGamepad.release(BUTTON_10); break;
            case 'K': bleGamepad.press(BUTTON_11); break;
            case 'k': bleGamepad.release(BUTTON_11); break;
            case 'L': bleGamepad.press(BUTTON_12); break;
            case 'l': bleGamepad.release(BUTTON_12); break;
            case 'M': bleGamepad.press(BUTTON_13); break;
            case 'm': bleGamepad.release(BUTTON_13); break;
            case 'N': bleGamepad.press(BUTTON_14); break;
            case 'n': bleGamepad.release(BUTTON_14); break;
            case 'O': bleGamepad.press(BUTTON_15); break;
            case 'o': bleGamepad.release(BUTTON_15); break;
            case 'P': bleGamepad.pressStart(); break;
            case 'p': bleGamepad.releaseStart(); break;
        }
        return;
    }


    if (cRight == '1') {
        switch (cLeft) {
            case 'A': m_axis0 = MAX_VALUE; break;
            case 'a': m_axis0 = MIN_VALUE; break;
            case 'B': m_axis1 = MAX_VALUE; break;
            case 'b': m_axis1 = MIN_VALUE; break;
            case 'C': m_axis2 = MAX_VALUE; break;
            case 'c': m_axis2 = MIN_VALUE; break;
            case 'D': m_axis3 = MAX_VALUE; break;
            case 'd': m_axis3 = MIN_VALUE; break;
            case 'E': m_axis4 = MAX_VALUE; break;
            case 'e': m_axis4 = MIN_VALUE; break;
            case 'F': m_axis5 = MAX_VALUE; break;
            case 'f': m_axis5 = MIN_VALUE; break;
            case 'G': m_axis6 = MAX_VALUE; break;
            case 'g': m_axis6 = MIN_VALUE; break;
            case 'H': m_axis7 = MAX_VALUE; break;
            case 'h': m_axis7 = MIN_VALUE; break;
        }
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
    }

    if (cRight == '2') {
        switch (cLeft) {
            case 'A': m_axis0 = MAX_VALUE_D2; break;
            case 'a': m_axis0 = MIN_VALUE_D2; break;
            case 'B': m_axis1 = MAX_VALUE_D2; break;
            case 'b': m_axis1 = MIN_VALUE_D2; break;
            case 'C': m_axis2 = MAX_VALUE_D2; break;
            case 'c': m_axis2 = MIN_VALUE_D2; break;
            case 'D': m_axis3 = MAX_VALUE_D2; break;
            case 'd': m_axis3 = MIN_VALUE_D2; break;
            case 'E': m_axis4 = MAX_VALUE_D2; break;
            case 'e': m_axis4 = MIN_VALUE_D2; break;
            case 'F': m_axis5 = MAX_VALUE_D2; break;
            case 'f': m_axis5 = MIN_VALUE_D2; break;
            case 'G': m_axis6 = MAX_VALUE_D2; break;
            case 'g': m_axis6 = MIN_VALUE_D2; break;
            case 'H': m_axis7 = MAX_VALUE_D2; break;
            case 'h': m_axis7 = MIN_VALUE_D2; break;
        }
    }
 if (cRight == '5') {
        switch (cLeft) {
            case 'A': m_axis0 = MIDDLE_VALUE; break;
            case 'a': m_axis0 = MIDDLE_VALUE; break;
            case 'B': m_axis1 = MIDDLE_VALUE; break;
            case 'b': m_axis1 = MIDDLE_VALUE; break;
            case 'C': m_axis2 = MIDDLE_VALUE; break;
            case 'c': m_axis2 = MIDDLE_VALUE; break;
            case 'D': m_axis3 = MIDDLE_VALUE; break;
            case 'd': m_axis3 = MIDDLE_VALUE; break;
            case 'E': m_axis4 = MIDDLE_VALUE; break;
            case 'e': m_axis4 = MIDDLE_VALUE; break;
            case 'F': m_axis5 = MIDDLE_VALUE; break;
            case 'f': m_axis5 = MIDDLE_VALUE; break;
            case 'G': m_axis6 = MIDDLE_VALUE; break;
            case 'g': m_axis6 = MIDDLE_VALUE; break;
            case 'H': m_axis7 = MIDDLE_VALUE; break;
            case 'h': m_axis7 = MIDDLE_VALUE; break;
        }
    }

    if (cRight == '4') {
        switch (cLeft) {
            case 'A': m_axis0 = MAX_VALUE_D4; break;
            case 'a': m_axis0 = MIN_VALUE_D4; break;
            case 'B': m_axis1 = MAX_VALUE_D4; break;
            case 'b': m_axis1 = MIN_VALUE_D4; break;
            case 'C': m_axis2 = MAX_VALUE_D4; break;
            case 'c': m_axis2 = MIN_VALUE_D4; break;
            case 'D': m_axis3 = MAX_VALUE_D4; break;
            case 'd': m_axis3 = MIN_VALUE_D4; break;
            case 'E': m_axis4 = MAX_VALUE_D4; break;
            case 'e': m_axis4 = MIN_VALUE_D4; break;
            case 'F': m_axis5 = MAX_VALUE_D4; break;
            case 'f': m_axis5 = MIN_VALUE_D4; break;
            case 'G': m_axis6 = MAX_VALUE_D4; break;
            case 'g': m_axis6 = MIN_VALUE_D4; break;
            case 'H': m_axis7 = MAX_VALUE_D4; break;
            case 'h': m_axis7 = MIN_VALUE_D4; break;
        }
    }

    if (cRight == '8') {
        switch (cLeft) {
            case 'A': m_axis0 = MAX_VALUE_D8; break;
            case 'a': m_axis0 = MIN_VALUE_D8; break;
            case 'B': m_axis1 = MAX_VALUE_D8; break;
            case 'b': m_axis1 = MIN_VALUE_D8; break;
            case 'C': m_axis2 = MAX_VALUE_D8; break;
            case 'c': m_axis2 = MIN_VALUE_D8; break;
            case 'D': m_axis3 = MAX_VALUE_D8; break;
            case 'd': m_axis3 = MIN_VALUE_D8; break;
            case 'E': m_axis4 = MAX_VALUE_D8; break;
            case 'e': m_axis4 = MIN_VALUE_D8; break;
            case 'F': m_axis5 = MAX_VALUE_D8; break;
            case 'f': m_axis5 = MIN_VALUE_D8; break;
            case 'G': m_axis6 = MAX_VALUE_D8; break;
            case 'g': m_axis6 = MIN_VALUE_D8; break;
            case 'H': m_axis7 = MAX_VALUE_D8; break;
            case 'h': m_axis7 = MIN_VALUE_D8; break;
        }
    }
    bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
}

