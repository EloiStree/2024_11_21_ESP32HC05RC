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
    #include <BleMouse.h>
    #include <BleKeyboard.h>

    #define RXD2 16   // GPIO16 (U2RXD)
    #define TXD2 17   // GPIO17 (U2TXD)

    BleKeyboard bleKeyboard;
    BleMouse bleMouse;
    BleGamepad bleGamepad;

    // RETURN THE RECEIVED BYTE TO THE SENDER
    // IT ALLOWS TO SEE IF HC05 BRIDGE IS WORKING PROPERLY
    bool m_return_byte_received = true;
    // DISPLAY BYTE RECEIVED
    bool m_use_print_byte_debug= true;
    // DISPLAY ACTION TRIGGERED
    bool m_use_print_action_debug= true;

    // Allow 
    bool allowDirectGPIOWriting =false;
    int m_allowToWritePinByDeveloper[] = {

        //  19,20,21,22,47,48,45,0,35,36,37,38,39,40,41,42,2,1,
        //  14,13,12,11,10,9,46,3,8,18,17,16,15,7,6,5,4
        2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
    };



    char cLeft = ' ';
    char cRight = ' ';

    bool m_use_print_received_double_char = true;

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

    int m_axis0 = 0;
    int m_axis1 = 0;
    int m_axis2 = 0;
    int m_axis3 = 0;
    int m_axis4 = 0;
    int m_axis5 = 0;
    int m_axis6 = 0;
    int m_axis7 = 0;

    bool m_button_0=false;
    bool m_button_1=false;
    bool m_button_2=false;
    bool m_button_3=false;
    bool m_button_4=false;
    bool m_button_5=false;
    bool m_button_6=false;
    bool m_button_7=false;
    bool m_button_8=false;
    bool m_button_9=false;
    bool m_button_10=false;
    bool m_button_11=false;
    bool m_button_12=false;
    bool m_button_13=false;
    bool m_button_14=false;
    bool m_button_15=false;
    bool m_button_start=false;

    bool isCharDigital(char c) {
        return c >= '0' && c <= '9';
    }

    // Helper function to convert 1-99 to percentage (-1.0 to 1.0)
    float turnFrom1To99AsPercent(int value) {
        if (value == 0) return 0.0;
        return (value - 50) / 50.0; // Maps 1-99 to -0.98 to 0.98
    }

    // Helper function to check if a specific bit is set (right to left)
    bool isIntegerBitRightToLeftTrue(int value, int bit) {
        return (value >> bit) & 1;
    }

    // Helper functions for setting gamepad axes
    void setLeftHorizontal(float percent) {
        m_axis0 = (int)((percent * MAX_VALUE / 2) + MIDDLE_VALUE); // Map -1.0 to 1.0 to 0-32767
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    void setLeftVertical(float percent) {
        m_axis1 = (int)((percent * MAX_VALUE / 2) + MIDDLE_VALUE);
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    void setRightHorizontal(float percent) {
        m_axis2 = (int)((percent * MAX_VALUE / 2) + MIDDLE_VALUE);
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    void setRightVertical(float percent) {
        m_axis3 = (int)((percent * MAX_VALUE / 2) + MIDDLE_VALUE);
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    void setTriggerLeftPercent(float percent) {
        m_axis4 = (int)(percent * MAX_VALUE);
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    void setTriggerRightPercent(float percent) {
        m_axis5 = (int)(percent * MAX_VALUE);
        bleGamepad.setAxes(m_axis0, m_axis1, m_axis2, m_axis3, m_axis4, m_axis5, m_axis6, m_axis7);
        print_axis();
    }

    // D-pad control functions
    void pressArrowN() { bleGamepad.setHat1(DPAD_UP); }
    void pressArrowNE() { bleGamepad.setHat1(DPAD_UP_RIGHT); }
    void pressArrowE() { bleGamepad.setHat1(DPAD_RIGHT); }
    void pressArrowSE() { bleGamepad.setHat1(DPAD_DOWN_RIGHT); }
    void pressArrowS() { bleGamepad.setHat1(DPAD_DOWN); }
    void pressArrowSW() { bleGamepad.setHat1(DPAD_DOWN_LEFT); }
    void pressArrowW() { bleGamepad.setHat1(DPAD_LEFT); }
    void pressArrowNW() { bleGamepad.setHat1(DPAD_UP_LEFT); }
    void releaseDPad() { bleGamepad.setHat1(DPAD_CENTERED); }

    // Button control functions
    void pressA(bool state) { if (state) { bleGamepad.press(BUTTON_1); m_button_0 = true; } else { bleGamepad.release(BUTTON_1); m_button_0 = false; } print_buttons(); }
    void pressX(bool state) { if (state) { bleGamepad.press(BUTTON_2); m_button_1 = true; } else { bleGamepad.release(BUTTON_2); m_button_1 = false; } print_buttons(); }
    void pressB(bool state) { if (state) { bleGamepad.press(BUTTON_3); m_button_2 = true; } else { bleGamepad.release(BUTTON_3); m_button_2 = false; } print_buttons(); }
    void pressY(bool state) { if (state) { bleGamepad.press(BUTTON_4); m_button_3 = true; } else { bleGamepad.release(BUTTON_4); m_button_3 = false; } print_buttons(); }
    void pressLeftSideButton(bool state) { if (state) { bleGamepad.press(BUTTON_5); m_button_4 = true; } else { bleGamepad.release(BUTTON_5); m_button_4 = false; } print_buttons(); }
    void pressRightSideButton(bool state) { if (state) { bleGamepad.press(BUTTON_6); m_button_5 = true; } else { bleGamepad.release(BUTTON_6); m_button_5 = false; } print_buttons(); }
    void pressLeftStick(bool state) { if (state) { bleGamepad.press(BUTTON_7); m_button_6 = true; } else { bleGamepad.release(BUTTON_7); m_button_6 = false; } print_buttons(); }
    void pressRightStick(bool state) { if (state) { bleGamepad.press(BUTTON_8); m_button_7 = true; } else { bleGamepad.release(BUTTON_8); m_button_7 = false; } print_buttons(); }
    void pressMenuLeft(bool state) { if (state) { bleGamepad.press(BUTTON_9); m_button_8 = true; } else { bleGamepad.release(BUTTON_9); m_button_8 = false; } print_buttons(); }
    void pressMenuRight(bool state) { if (state) { bleGamepad.press(BUTTON_10); m_button_9 = true; } else { bleGamepad.release(BUTTON_10); m_button_9 = false; } print_buttons(); }
    void pressHomeXboxButton(bool state) { if (state) { bleGamepad.pressStart(); m_button_start = true; } else { bleGamepad.releaseStart(); m_button_start = false; } print_buttons(); }

    void setup() {
    
        Serial.begin(115200);
        Serial.println("Starting BLE work!");
        Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
        Serial.println("UART1 initialized.");
        bleGamepad.begin();
        bleKeyboard.begin();
        bleMouse.begin();
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
        cLeft = cRight;
        cRight = c;
        if (isCharDigital(cRight)) {
            uartCommand(cLeft, cRight);
        }
        }
        delay(10);
    }


    void print_button(int id, bool isPress){

    String t = ""+String(id);
    print_button(t,isPress);

    }

    void print_button(String text, bool isPress) {
    if(!m_use_print_action_debug)
        return;
    Serial.print("Button: ");
    Serial.print(text);
    Serial.print(" ");
    Serial.println(isPress ? "pressed" : "released");
    }

    void print_axis(){
    print_axis(
    m_axis0,
    m_axis1,
    m_axis2,
    m_axis3,
    m_axis4,
    m_axis5,
    m_axis6,
    m_axis7

    );

    }

    void print_buttons(){
        if(!m_use_print_action_debug)
            return;
        Serial.print("Buttons: ");
        Serial.print(m_button_0);
        Serial.print(" ");
        Serial.print(m_button_1);
        Serial.print(" ");
        Serial.print(m_button_2);
        Serial.print(" ");
        Serial.print(m_button_3);
        Serial.print(" ");
        Serial.print(m_button_4);
        Serial.print(" ");
        Serial.print(m_button_5);
        Serial.print(" ");
        Serial.print(m_button_6);
        Serial.print(" ");
        Serial.print(m_button_7);
        Serial.print(" ");
        Serial.print(m_button_8);
        Serial.print(" ");
        Serial.print(m_button_9);
        Serial.print(" ");
        Serial.print(m_button_10);
        Serial.print(" ");
        Serial.print(m_button_11);
        Serial.print(" ");
        Serial.print(m_button_12);
        Serial.print(" ");
        Serial.print(m_button_13);
        Serial.print(" ");
        Serial.print(m_button_14);
        Serial.print(" ");
        Serial.print(m_button_15);
        Serial.print(" ");
        Serial.println(m_button_start);
    }


    void print_axis(int m_axis0, int m_axis1, int m_axis2, int m_axis3, int m_axis4, int m_axis5, int m_axis6, int m_axis7) {
    if(!m_use_print_action_debug)
        return;
    Serial.print("Axis: ");
    Serial.print(m_axis0);
    Serial.print(" ");
    Serial.print(m_axis1);
    Serial.print(" ");
    Serial.print(m_axis2);
    Serial.print(" ");
    Serial.print(m_axis3);
    Serial.print(" ");
    Serial.print(m_axis4);
    Serial.print(" ");
    Serial.print(m_axis5);
    Serial.print(" ");
    Serial.print(m_axis6);
    Serial.print(" ");
    Serial.println(m_axis7);
    }
    void uartCommand(char cLeft, char cRight) {
        if (m_use_print_received_double_char) {
            Serial.print("Left: ");
            Serial.print(cLeft);
            Serial.print(" Right: ");
            Serial.println(cRight);
        }

        if (!bleGamepad.isConnected())
            return;

        if (cRight == '0') {
            switch (cLeft) {
                case 'A': bleGamepad.press(BUTTON_1);m_button_0=true; break;
                case 'a': bleGamepad.release(BUTTON_1);m_button_0=false; break;
                case 'B': bleGamepad.press(BUTTON_2);m_button_1=true; break;
                case 'b': bleGamepad.release(BUTTON_2);m_button_1=false; break;
                case 'C': bleGamepad.press(BUTTON_3);m_button_2=true; break;
                case 'c': bleGamepad.release(BUTTON_3);m_button_2=false; break;
                case 'D': bleGamepad.press(BUTTON_4);m_button_3=true; break;
                case 'd': bleGamepad.release(BUTTON_4);m_button_3=false; break;
                case 'E': bleGamepad.press(BUTTON_5);m_button_4=true; break;
                case 'e': bleGamepad.release(BUTTON_5);m_button_4=false; break;
                case 'F': bleGamepad.press(BUTTON_6);m_button_5=true; break;
                case 'f': bleGamepad.release(BUTTON_6);m_button_5=false; break;
                case 'G': bleGamepad.press(BUTTON_7);m_button_6=true; break;
                case 'g': bleGamepad.release(BUTTON_7);m_button_6=false; break;
                case 'H': bleGamepad.press(BUTTON_8);m_button_7=true; break;
                case 'h': bleGamepad.release(BUTTON_8);m_button_7=false; break;
                case 'I': bleGamepad.press(BUTTON_9);m_button_8=true; break;
                case 'i': bleGamepad.release(BUTTON_9);m_button_8=false; break;
                case 'J': bleGamepad.press(BUTTON_10);m_button_9=true; break;
                case 'j': bleGamepad.release(BUTTON_10);m_button_9=false; break;
                case 'K': bleGamepad.press(BUTTON_11);m_button_10=true; break;
                case 'k': bleGamepad.release(BUTTON_11);m_button_10=false; break;
                case 'L': bleGamepad.press(BUTTON_12);m_button_11=true; break;
                case 'l': bleGamepad.release(BUTTON_12);m_button_11=false; break;
                case 'M': bleGamepad.press(BUTTON_13);m_button_12=true; break;
                case 'm': bleGamepad.release(BUTTON_13);m_button_12=false; break;
                case 'N': bleGamepad.press(BUTTON_14);m_button_13=true; break;
                case 'n': bleGamepad.release(BUTTON_14);m_button_13=false; break;
                case 'O': bleGamepad.press(BUTTON_15);m_button_14=true; break;
                case 'o': bleGamepad.release(BUTTON_15);m_button_14=false; break;
                case 'P': bleGamepad.press(BUTTON_16);m_button_15=true; break;
                case 'p': bleGamepad.release(BUTTON_16);m_button_15=false; break;
                case 'Q': bleGamepad.pressStart();m_button_start=true; break;
                case 'q': bleGamepad.releaseStart();m_button_start=false; break;
            }
            print_buttons();
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
        print_axis();
    }

    void integerToXbox(int value){
    // COMMAND TO SE TRUE OR FALSE BUTTONS OR BUTTON LIKE 
    if(value>=1000 && value<=2999){
            switch(value){
                // case 1399: randomInputAllGamepadNoMenu(); break;
                // case 2399: releaseAllGamepad(); break;
                // case 1390: m_useHardwareJoystick=true;  Serial.println("Hardware Joystick ON"); break;
                // case 2390: m_useHardwareJoystick=false;  Serial.println("Hardware Joystick OFF"); break;
                case 1300: pressA(true); break;
                case 2300: pressA(false); break;
                case 1301: pressX(true); break;
                case 2301: pressX(false); break;
                case 1302: pressB(true); break;
                case 2302: pressB(false); break;
                case 1303: pressY(true); break;
                case 2303: pressY(false); break;
                case 1304: pressLeftSideButton(true); break;
                case 2304: pressLeftSideButton(false); break;
                case 1305: pressRightSideButton(true); break;
                case 2305: pressRightSideButton(false); break;
                case 1306: pressLeftStick(true); break;
                case 2306: pressLeftStick(false); break;
                case 1307: pressRightStick(true); break;
                case 2307: pressRightStick(false); break;
                case 1308: pressMenuRight(true); break;
                case 2308: pressMenuRight(false); break;
                case 1309: pressMenuLeft(true); break;
                case 2309: pressMenuLeft(false); break;
                case 1310: releaseDPad(); break;
                case 2310: releaseDPad(); break;
                case 1311: pressArrowN(); break;
                case 2311: releaseDPad(); break;
                case 1312: pressArrowNE(); break;
                case 2312: releaseDPad(); break;
                case 1313: pressArrowE(); break;
                case 2313: releaseDPad(); break;
                case 1314: pressArrowSE(); break;
                case 2314: releaseDPad(); break;
                case 1315: pressArrowS(); break;
                case 2315: releaseDPad(); break;
                case 1316: pressArrowSW(); break;
                case 2316: releaseDPad(); break;
                case 1317: pressArrowW(); break;
                case 2317: releaseDPad(); break;
                case 1318: pressArrowNW(); break;
                case 2318: releaseDPad(); break;
                case 1319: pressHomeXboxButton(true); break;
                case 2319: pressHomeXboxButton(false); break;
                // case 1320: randomAxis(); break;
                // case 2320: releaseAxis(); break;
                // case 1321: recordStart(); break;
                // case 2321: recordStop(); break;
                // Turn in clockwise
                // case 1330: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 2330: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1331: setLeftVertical(1); setLeftHorizontal(0);     break;
                // case 2331: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1332: setLeftVertical(1); setLeftHorizontal(1);     break;
                // case 2332: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1333: setLeftVertical(0); setLeftHorizontal(1);     break;
                // case 2333: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1334: setLeftVertical(-1); setLeftHorizontal(1);    break;
                // case 2334: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1335: setLeftVertical(-1); setLeftHorizontal(0);    break;
                // case 2335: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1336: setLeftVertical(-1); setLeftHorizontal(-1);   break;
                // case 2336: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1337: setLeftVertical(0); setLeftHorizontal(-1);    break;
                // case 2337: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1338: setLeftVertical(1); setLeftHorizontal(-1);    break;
                // case 2338: setLeftVertical(0); setLeftHorizontal(0);     break;
                // case 1340: setRightVertical(0); setRightHorizontal(0);    break;
                // case 2340: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1341: setRightVertical(1); setRightHorizontal(0);    break;
                // case 2341: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1342: setRightVertical(1); setRightHorizontal(1);    break;
                // case 2342: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1343: setRightVertical(0); setRightHorizontal(1);    break;
                // case 2343: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1344: setRightVertical(-1); setRightHorizontal(1);   break;
                // case 2344: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1345: setRightVertical(-1); setRightHorizontal(0);   break;
                // case 2345: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1346: setRightVertical(-1); setRightHorizontal(-1);  break;
                // case 2346: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1347: setRightVertical(0); setRightHorizontal(-1);   break;
                // case 2347: setRightVertical(0); setRightHorizontal(0);    break;
                // case 1348: setRightVertical(1); setRightHorizontal(-1);   break;
                // case 2348: setRightVertical(0); setRightHorizontal(0);    break;
                case 1350: setLeftHorizontal(1); break;
                case 2350: setLeftHorizontal(0); break;
                case 1351: setLeftHorizontal(-1); break;
                case 2351: setLeftHorizontal(0); break;
                case 1352: setLeftVertical(1); break;
                case 2352: setLeftVertical(0); break;
                case 1353: setLeftVertical(-1); break;
                case 2353: setLeftVertical(0); break;
                case 1354: setRightHorizontal(1); break;
                case 2354: setRightHorizontal(0); break;
                case 1355: setRightHorizontal(-1); break;
                case 2355: setRightHorizontal(0); break;
                case 1356: setRightVertical(1); break;
                case 2356: setRightVertical(0); break;
                case 1357: setRightVertical(-1); break;
                case 2357: setRightVertical(0); break;
                case 1358: setTriggerLeftPercent(1); break;
                case 2358: setTriggerLeftPercent(0); break;
                case 1359: setTriggerRightPercent(1); break;
                case 2359: setTriggerRightPercent(0); break;   
                case 1360: setLeftHorizontal(0.75); break;
                case 2360: setLeftHorizontal(0); break; 
                case 1361: setLeftHorizontal(-0.75); break;
                case 2361: setLeftHorizontal(0); break;
                case 1362: setLeftVertical(0.75); break;
                case 2362: setLeftVertical(0); break;
                case 1363: setLeftVertical(-0.75); break;
                case 2363: setLeftVertical(0); break;
                case 1364: setRightHorizontal(0.75); break;
                case 2364: setRightHorizontal(0); break;
                case 1365: setRightHorizontal(-0.75); break;
                case 2365: setRightHorizontal(0); break;
                case 1366: setRightVertical(0.75); break;
                case 2366: setRightVertical(0); break;
                case 1367: setRightVertical(-0.75); break;
                case 2367: setRightVertical(0); break;
                case 1368: setTriggerLeftPercent(0.75); break;
                case 2368: setTriggerLeftPercent(0); break;
                case 1369: setTriggerRightPercent(0.75); break;
                case 2369: setTriggerRightPercent(0); break;
                case 1370: setLeftHorizontal(0.5); break;
                case 2370: setLeftHorizontal(0); break;
                case 1371: setLeftHorizontal(-0.5); break;
                case 2371: setLeftHorizontal(0); break;
                case 1372: setLeftVertical(0.5); break;
                case 2372: setLeftVertical(0); break;
                case 1373: setLeftVertical(-0.5); break;
                case 2373: setLeftVertical(0); break;
                case 1374: setRightHorizontal(0.5); break;
                case 2374: setRightHorizontal(0); break;
                case 1375: setRightHorizontal(-0.5); break;
                case 2375: setRightHorizontal(0); break;
                case 1376: setRightVertical(0.5); break;
                case 2376: setRightVertical(0); break;
                case 1377: setRightVertical(-0.5); break;
                case 2377: setRightVertical(0); break;
                case 1378: setTriggerLeftPercent(0.5); break;
                case 2378: setTriggerLeftPercent(0); break;
                case 1379: setTriggerRightPercent(0.5); break;
                case 2379: setTriggerRightPercent(0); break;
                case 1380: setLeftHorizontal(0.25); break;
                case 2380: setLeftHorizontal(0); break;
                case 1381: setLeftHorizontal(-0.25); break;
                case 2381: setLeftHorizontal(0); break;
                case 1382: setLeftVertical(0.25); break;
                case 2382: setLeftVertical(0); break;
                case 1383: setLeftVertical(-0.25); break;
                case 2383: setLeftVertical(0); break;
                case 1384: setRightHorizontal(0.25); break;
                case 2384: setRightHorizontal(0); break;
                case 1385: setRightHorizontal(-0.25); break;
                case 2385: setRightHorizontal(0); break;
                case 1386: setRightVertical(0.25); break;
                case 2386: setRightVertical(0); break;
                case 1387: setRightVertical(-0.25); break;
                case 2387: setRightVertical(0); break;
                case 1388: setTriggerLeftPercent(0.25); break;
                case 2388: setTriggerLeftPercent(0); break;
                case 1389: setTriggerRightPercent(0.25); break;
                case 2389: setTriggerRightPercent(0); break;
                default:break;   
            }
    }
    
    else if(value>=1500000000 && value<=1599999999){
        


            // 1599998888
            // 15 Mouse Move in application focus screen
            // 14 Mouse Move on OS screen
            // 9999: percent width screen 0-9999 Left Right
            // 8888: percent height screen 0-9999 Down Top

            int mode = (value / 100000000) % 10; // 5 for app, 4 for OS
            int x_percent = (value / 10000) % 10000; // 0-9999
            int y_percent = value % 10000; // 0-9999
            float x = x_percent / 9999.0 * 100.0; // Scale to 0-100%
            float y = y_percent / 9999.0 * 100.0; // Scale to 0-100%
            
            // Assuming BleMouse.move() takes relative pixel values; adjust scaling as needed
            int move_x = (int)(x * 10); // Arbitrary scaling for relative movement
            int move_y = (int)(y * 10);
            if (mode == 5) {
                // Application-focused movement
                bleMouse.move(move_x, move_y);
            } else if (mode == 4) {
                // OS screen movement
                bleMouse.move(move_x, move_y); // Adjust if absolute positioning is needed
            }
            if (m_use_print_action_debug) {
                Serial.print("Mouse Move: mode=");
                Serial.print(mode == 5 ? "App" : "OS");
                Serial.print(", x=");
                Serial.print(x);
                Serial.print("%, y=");
                Serial.println(y);
            }

    }
    else if(value>=1800000000 && value<=1899999999){
        
        // //18 50 20 00 10
        // //1850200010
        // //4 bytes because integer
        int leftHorizontalfrom1to99 =   (value/1000000)%100;
        int leftVerticalfrom1to99 =     (value/10000)%100;
        int rightHorizontalfrom1to99 =  (value/100)%100;
        int rightVerticalfrom1to99 =    (value/1)%100;
        float leftHorizontalPercent= turnFrom1To99AsPercent(leftHorizontalfrom1to99);
        float leftVerticalPercent= turnFrom1To99AsPercent(leftVerticalfrom1to99);
        float rightHorizontalPercent= turnFrom1To99AsPercent(rightHorizontalfrom1to99);
        float rightVerticalPercent= turnFrom1To99AsPercent(rightVerticalfrom1to99);
        setLeftHorizontal(leftHorizontalPercent);
        setLeftVertical(leftVerticalPercent);
        setRightHorizontal(rightHorizontalPercent);
        setRightVertical(rightVerticalPercent);
    }
    else if(value>=1700000000 && value<=1799999999)
    {
        //   m_binaryBufferOfInteger[33]; // Buffer to store the binary representation (32 bits + null terminator)
        //   intToBinaryBuffer(value, m_binaryBufferOfInteger, 33);
        //   Serial.println(m_binaryBufferOfInteger);
        //   value=value-1700000000;
        //   intToBinaryBuffer(value,m_binaryBufferOfInteger,33);
        //   Serial.println(m_binaryBufferOfInteger);
        

        //   float triggerLeft=0.0;
        //   float triggerRight=0.0;
        //   float arrowHorizontal=0;
        //   float arrowVertical =0;
        value -= 1700000000; // Normalize for bitfield
        float triggerLeft = 0.0;
        float triggerRight = 0.0;
        float arrowHorizontal = 0;
        float arrowVertical = 0;

        if (isIntegerBitRightToLeftTrue(value, 0)) pressA(true); else pressA(false);
        if (isIntegerBitRightToLeftTrue(value, 1)) pressX(true); else pressX(false);
        if (isIntegerBitRightToLeftTrue(value, 2)) pressB(true); else pressB(false);
        if (isIntegerBitRightToLeftTrue(value, 3)) pressY(true); else pressY(false);
        if (isIntegerBitRightToLeftTrue(value, 4)) pressLeftSideButton(true); else pressLeftSideButton(false);
        if (isIntegerBitRightToLeftTrue(value, 5)) pressRightSideButton(true); else pressRightSideButton(false);
        if (isIntegerBitRightToLeftTrue(value, 6)) pressLeftStick(true); else pressLeftStick(false);
        if (isIntegerBitRightToLeftTrue(value, 7)) pressRightStick(true); else pressRightStick(false);
        if (isIntegerBitRightToLeftTrue(value, 8)) pressMenuLeft(true); else pressMenuLeft(false);
        if (isIntegerBitRightToLeftTrue(value, 9)) pressMenuRight(true); else pressMenuRight(false);
        if (isIntegerBitRightToLeftTrue(value, 10)) pressHomeXboxButton(true); else pressHomeXboxButton(false);
        if (isIntegerBitRightToLeftTrue(value, 11)) arrowVertical += 1; // N
        if (isIntegerBitRightToLeftTrue(value, 12)) arrowHorizontal += 1; // E
        if (isIntegerBitRightToLeftTrue(value, 13)) arrowVertical += -1; // S
        if (isIntegerBitRightToLeftTrue(value, 14)) arrowHorizontal += -1; // W
        
        if(isIntegerBitRightToLeftTrue(value, 18)) triggerLeft+=(0.25);
        if(isIntegerBitRightToLeftTrue(value, 19)) triggerLeft+=(0.25);
        if(isIntegerBitRightToLeftTrue(value, 20)) triggerLeft+=(0.5);
        if(isIntegerBitRightToLeftTrue(value, 21)) triggerRight+=(0.25);
        if(isIntegerBitRightToLeftTrue(value, 22)) triggerRight+=(0.25);
        if(isIntegerBitRightToLeftTrue(value, 23)) triggerRight+=(0.5);
        setTriggerLeftPercent(triggerLeft);
        setTriggerRightPercent(triggerRight);

        if(arrowVertical==1 && arrowHorizontal==0)
            pressArrowN();
        else if(arrowVertical==1 && arrowHorizontal==1)
            pressArrowNE();
        else if(arrowVertical==0 && arrowHorizontal==1)
            pressArrowE();
        else if(arrowVertical==-1 && arrowHorizontal==1)
            pressArrowSE();
        else if(arrowVertical==-1 && arrowHorizontal==0)
            pressArrowS();
        else if(arrowVertical==-1 && arrowHorizontal==-1)
            pressArrowSW();
        else if(arrowVertical==0 && arrowHorizontal==-1)
            pressArrowW();
        else if(arrowVertical==1 && arrowHorizontal==-1)
            pressArrowNW();
        else
            releaseDPad();
        bool useDebugPrint = false;
    if(useDebugPrint){
    Serial.print(" A:");
    Serial.print(isIntegerBitRightToLeftTrue(value, 0));
    Serial.print(" X:");
    Serial.print(isIntegerBitRightToLeftTrue(value, 1));
    Serial.print(" B:");
    Serial.print(isIntegerBitRightToLeftTrue(value, 2));
    Serial.print(" Y:");
    Serial.print(isIntegerBitRightToLeftTrue(value, 3));
        Serial.print(" LB:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 4));
        Serial.print(" RB:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 5));
        Serial.print(" LS:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 6));
        Serial.print(" RS:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 7));
        Serial.print(" MENU:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 8));
        Serial.print(" HOME:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 9));
        Serial.print(" DPad N:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 10));
        Serial.print(" DPad NE:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 11));
        Serial.print(" DPad E:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 12));
        Serial.print(" DPad SE:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 13));
        Serial.print(" DPad S:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 14));
        Serial.print(" DPad SW:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 15));
        Serial.print(" DPad W:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 16));
        Serial.print(" DPad NW:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 17));
        Serial.print(" Left Trigger 0.25 1:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 18));
        Serial.print(" Left Trigger 0.25 2:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 19));
        Serial.print(" Left Trigger 0.5 3 :");
        Serial.print(isIntegerBitRightToLeftTrue(value, 20));
        Serial.print(" Right Trigger 0.25 1:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 21));
        Serial.print(" Right Trigger 0.25 2:");
        Serial.print(isIntegerBitRightToLeftTrue(value, 22));
        Serial.print(" Right Trigger 0.5 3 :");
        Serial.print(isIntegerBitRightToLeftTrue(value, 23));
        Serial.println();
        }
        else{


            // Mouse Left	1260	2260
            // Mouse Middle	1261	2261
            // Mouse Right	1262	2262
            // Mouse Button 4	1263	2263
            // Mouse Button 5	1264	2264
            // Mouse Double Click Left	1265	2265
            // Mouse Triple Click Left	1266	2266
            // Mouse Double Click Right	1267	2267
            // Mouse Triple Click Right	1268	2268

            if (value >= 1260 && value <= 2268) {
                switch (value) {
                    case 1260: bleMouse.press(MOUSE_LEFT); break;
                    case 2260: bleMouse.release(MOUSE_LEFT); break;
                    case 1261: bleMouse.press(MOUSE_MIDDLE); break;
                    case 2261: bleMouse.release(MOUSE_MIDDLE); break;
                    case 1262: bleMouse.press(MOUSE_RIGHT); break;
                    case 2262: bleMouse.release(MOUSE_RIGHT); break;
                    case 1263: bleMouse.press(MOUSE_BACK); break;
                    case 2263: bleMouse.release(MOUSE_BACK); break;
                    case 1264: bleMouse.press(MOUSE_FORWARD); break;
                    case 2264: bleMouse.release(MOUSE_FORWARD); break;
                    case 1265: bleMouse.click(MOUSE_LEFT); bleMouse.click(MOUSE_LEFT); break; // Double click
                    case 2265: break; // No release needed for click
                    case 1266: bleMouse.click(MOUSE_LEFT); bleMouse.click(MOUSE_LEFT); bleMouse.click(MOUSE_LEFT); break; // Triple click
                    case 2266: break;
                    case 1267: bleMouse.click(MOUSE_RIGHT); bleMouse.click(MOUSE_RIGHT); break;
                    case 2267: break;
                    case 1268: bleMouse.click(MOUSE_RIGHT); bleMouse.click(MOUSE_RIGHT); bleMouse.click(MOUSE_RIGHT); break;
                    case 2268: break;
                }
                if (m_use_print_action_debug) {
                    Serial.print("Mouse Action: ");
                    Serial.println(value);
                }
            }

        if (allowDirectGPIOWriting){
            if (value >= 1401 && value <= 1480 || value >= 2401 && value <= 2480) {
                int pin = (value % 100) - 1; // Extract pin number (1-80)
                bool state = value < 2000; // 1401-1480 is HIGH, 2401-2480 is LOW
                bool validPin = false;
                for (int allowedPin : m_allowToWritePinByDeveloper) {
                    if (pin == allowedPin) {
                        validPin = true;
                        break;
                    }
                }
                if (validPin) {
                    pinMode(pin, OUTPUT);
                    digitalWrite(pin, state ? HIGH : LOW);
                    if (m_use_print_action_debug) {
                        Serial.print("GPIO "); Serial.print(pin); Serial.println(state ? " HIGH" : " LOW");
                    }
                }
            }

        }
        // GPIO1	1401	2401
        // GPIO2	1402	2402
        // GPIO3	1403	2403
        // GPIO3	14..	24..
        // GPIO40	1440	2440
        // Allowed by dev 1	1441	2441
        // Allowed by dev 2	1442	2442
        // Allowed by dev 3	1443	2443
        // Allowed by dev 3	144.	24..
        // Allowed by dev 40	1480	2480


        //     Char	Decimal	Short Description
        // 4032	SPACE
        // Under Escape left of number
        // `	4096	GRAVE ACCENT
        // ~	4126	TILDE
        // Numerical top key
        // !	4033	EXCLAMATION MARK
        // @	4064	COMMERCIAL AT
        // #	4035	NUMBER SIGN
        // $	4036	DOLLAR SIGN
        // %	4037	PERCENT SIGN
        // ^	4094	CIRCUMFLEX ACCENT
        // &	4038	AMPERSAND
        // *	4042	ASTERISK
        // (	4040	LEFT PARENTHESIS
        // )	4041	RIGHT PARENTHESIS
        // Left of backspace char
        // _	4095	LOW LINE
        // -	4045	HYPHEN-MINUS
        // +	4043	PLUS SIGN
        // =	4061	EQUALS SIGN
        // Coding char top line
        // [	4091	LEFT SQUARE BRACKET
        // {	4123	LEFT CURLY BRACKET
        // ]	4093	RIGHT SQUARE BRACKET
        // }	4125	RIGHT CURLY BRACKET
        // Ponctuation middle line
        // ;	4059	SEMICOLON
        // :	4058	COLON
        // "	4034	QUOTATION MARK
        // '	4039	APOSTROPHE
        // Ponctuation down line
        // ,	4044	COMMA
        // <	4060	LESS-THAN SIGN
        // .	4046	FULL STOP
        // >	4062	GREATER-THAN SIGN
        // /	4047	SOLIDUS
        // ?	4063	QUESTION MARK
        // Pinky finger left or right button
        // |	4124	VERTICAL LINE
        // \	4092	REVERSE SOLIDUS
        // Special
        // ¯	4175	MACRON
        // ´	4180	ACUTE ACCENT
        // ×	4215	MULTIPLICATION SIGN
        // ÷	4247	DIVISION SIGN
        // 4160	NO-BREAK SPACE
        // Full list of the UTF8 Char as integer.

        // Char	Decimal	Short Description
        // 4032	SPACE
        // !	4033	EXCLAMATION MARK
        // "	4034	QUOTATION MARK
        // #	4035	NUMBER SIGN
        // $	4036	DOLLAR SIGN
        // %	4037	PERCENT SIGN
        // &	4038	AMPERSAND
        // '	4039	APOSTROPHE
        // (	4040	LEFT PARENTHESIS
        // )	4041	RIGHT PARENTHESIS
        // *	4042	ASTERISK
        // +	4043	PLUS SIGN
        // ,	4044	COMMA
        // -	4045	HYPHEN-MINUS
        // .	4046	FULL STOP
        // /	4047	SOLIDUS
        // 0	4048	DIGIT ZERO
        // 1	4049	DIGIT ONE
        // 2	4050	DIGIT TWO
        // 3	4051	DIGIT THREE
        // 4	4052	DIGIT FOUR
        // 5	4053	DIGIT FIVE
        // 6	4054	DIGIT SIX
        // 7	4055	DIGIT SEVEN
        // 8	4056	DIGIT EIGHT
        // 9	4057	DIGIT NINE
        // :	4058	COLON
        // ;	4059	SEMICOLON
        // <	4060	LESS-THAN SIGN
        // =	4061	EQUALS SIGN
        // >	4062	GREATER-THAN SIGN
        // ?	4063	QUESTION MARK
        // @	4064	COMMERCIAL AT
        // A	4065	LATIN CAPITAL LETTER A
        // B	4066	LATIN CAPITAL LETTER B
        // C	4067	LATIN CAPITAL LETTER C
        // D	4068	LATIN CAPITAL LETTER D
        // E	4069	LATIN CAPITAL LETTER E
        // F	4070	LATIN CAPITAL LETTER F
        // G	4071	LATIN CAPITAL LETTER G
        // H	4072	LATIN CAPITAL LETTER H
        // I	4073	LATIN CAPITAL LETTER I
        // J	4074	LATIN CAPITAL LETTER J
        // K	4075	LATIN CAPITAL LETTER K
        // L	4076	LATIN CAPITAL LETTER L
        // M	4077	LATIN CAPITAL LETTER M
        // N	4078	LATIN CAPITAL LETTER N
        // O	4079	LATIN CAPITAL LETTER O
        // P	4080	LATIN CAPITAL LETTER P
        // Q	4081	LATIN CAPITAL LETTER Q
        // R	4082	LATIN CAPITAL LETTER R
        // S	4083	LATIN CAPITAL LETTER S
        // T	4084	LATIN CAPITAL LETTER T
        // U	4085	LATIN CAPITAL LETTER U
        // V	4086	LATIN CAPITAL LETTER V
        // W	4087	LATIN CAPITAL LETTER W
        // X	4088	LATIN CAPITAL LETTER X
        // Y	4089	LATIN CAPITAL LETTER Y
        // Z	4090	LATIN CAPITAL LETTER Z
        // [	4091	LEFT SQUARE BRACKET
        // \	4092	REVERSE SOLIDUS
        // ]	4093	RIGHT SQUARE BRACKET
        // ^	4094	CIRCUMFLEX ACCENT
        // _	4095	LOW LINE
        // `	4096	GRAVE ACCENT
        // a	4097	LATIN SMALL LETTER A
        // b	4098	LATIN SMALL LETTER B
        // c	4099	LATIN SMALL LETTER C
        // d	4100	LATIN SMALL LETTER D
        // e	4101	LATIN SMALL LETTER E
        // f	4102	LATIN SMALL LETTER F
        // g	4103	LATIN SMALL LETTER G
        // h	4104	LATIN SMALL LETTER H
        // i	4105	LATIN SMALL LETTER I
        // j	4106	LATIN SMALL LETTER J
        // k	4107	LATIN SMALL LETTER K
        // l	4108	LATIN SMALL LETTER L
        // m	4109	LATIN SMALL LETTER M
        // n	4110	LATIN SMALL LETTER N
        // o	4111	LATIN SMALL LETTER O
        // p	4112	LATIN SMALL LETTER P
        // q	4113	LATIN SMALL LETTER Q
        // r	4114	LATIN SMALL LETTER R
        // s	4115	LATIN SMALL LETTER S
        // t	4116	LATIN SMALL LETTER T
        // u	4117	LATIN SMALL LETTER U
        // v	4118	LATIN SMALL LETTER V
        // w	4119	LATIN SMALL LETTER W
        // x	4120	LATIN SMALL LETTER X
        // y	4121	LATIN SMALL LETTER Y
        // z	4122	LATIN SMALL LETTER Z
        // {	4123	LEFT CURLY BRACKET
        // |	4124	VERTICAL LINE
        // }	4125	RIGHT CURLY BRACKET
        // ~	4126	TILDE
        // 4160	NO-BREAK SPACE
        // ¡	4161	INVERTED EXCLAMATION MARK
        // ¢	4162	CENT SIGN
        // £	4163	POUND SIGN
        // ¤	4164	CURRENCY SIGN
        // ¥	4165	YEN SIGN
        // ¦	4166	BROKEN BAR
        // §	4167	SECTION SIGN
        // ¨	4168	DIAERESIS
        // ©	4169	COPYRIGHT SIGN
        // ª	4170	FEMININE ORDINAL INDICATOR
        // «	4171	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        // ¬	4172	NOT SIGN
        // 4173	SOFT HYPHEN
        // ®	4174	REGISTERED SIGN
        // ¯	4175	MACRON
        // °	4176	DEGREE SIGN
        // ±	4177	PLUS-MINUS SIGN
        // ²	4178	SUPERSCRIPT TWO
        // ³	4179	SUPERSCRIPT THREE
        // ´	4180	ACUTE ACCENT
        // µ	4181	MICRO SIGN
        // ¶	4182	PILCROW SIGN
        // ·	4183	MIDDLE DOT
        // ¸	4184	CEDILLA
        // ¹	4185	SUPERSCRIPT ONE
        // º	4186	MASCULINE ORDINAL INDICATOR
        // »	4187	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        // ¼	4188	VULGAR FRACTION ONE QUARTER
        // ½	4189	VULGAR FRACTION ONE HALF
        // ¾	4190	VULGAR FRACTION THREE QUARTERS
        // ¿	4191	INVERTED QUESTION MARK
        // À	4192	LATIN CAPITAL LETTER A WITH GRAVE
        // Á	4193	LATIN CAPITAL LETTER A WITH ACUTE
        // Â	4194	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
        // Ã	4195	LATIN CAPITAL LETTER A WITH TILDE
        // Ä	4196	LATIN CAPITAL LETTER A WITH DIAERESIS
        // Å	4197	LATIN CAPITAL LETTER A WITH RING ABOVE
        // Æ	4198	LATIN CAPITAL LETTER AE
        // Ç	4199	LATIN CAPITAL LETTER C WITH CEDILLA
        // È	4200	LATIN CAPITAL LETTER E WITH GRAVE
        // É	4201	LATIN CAPITAL LETTER E WITH ACUTE
        // Ê	4202	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
        // Ë	4203	LATIN CAPITAL LETTER E WITH DIAERESIS
        // Ì	4204	LATIN CAPITAL LETTER I WITH GRAVE
        // Í	4205	LATIN CAPITAL LETTER I WITH ACUTE
        // Î	4206	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
        // Ï	4207	LATIN CAPITAL LETTER I WITH DIAERESIS
        // Ð	4208	LATIN CAPITAL LETTER ETH
        // Ñ	4209	LATIN CAPITAL LETTER N WITH TILDE
        // Ò	4210	LATIN CAPITAL LETTER O WITH GRAVE
        // Ó	4211	LATIN CAPITAL LETTER O WITH ACUTE
        // Ô	4212	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
        // Õ	4213	LATIN CAPITAL LETTER O WITH TILDE
        // Ö	4214	LATIN CAPITAL LETTER O WITH DIAERESIS
        // ×	4215	MULTIPLICATION SIGN
        // Ø	4216	LATIN CAPITAL LETTER O WITH STROKE
        // Ù	4217	LATIN CAPITAL LETTER U WITH GRAVE
        // Ú	4218	LATIN CAPITAL LETTER U WITH ACUTE
        // Û	4219	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
        // Ü	4220	LATIN CAPITAL LETTER U WITH DIAERESIS
        // Ý	4221	LATIN CAPITAL LETTER Y WITH ACUTE
        // Þ	4222	LATIN CAPITAL LETTER THORN
        // ß	4223	LATIN SMALL LETTER SHARP S
        // à	4224	LATIN SMALL LETTER A WITH GRAVE
        // á	4225	LATIN SMALL LETTER A WITH ACUTE
        // â	4226	LATIN SMALL LETTER A WITH CIRCUMFLEX
        // ã	4227	LATIN SMALL LETTER A WITH TILDE
        // ä	4228	LATIN SMALL LETTER A WITH DIAERESIS
        // å	4229	LATIN SMALL LETTER A WITH RING ABOVE
        // æ	4230	LATIN SMALL LETTER AE
        // ç	4231	LATIN SMALL LETTER C WITH CEDILLA
        // è	4232	LATIN SMALL LETTER E WITH GRAVE
        // é	4233	LATIN SMALL LETTER E WITH ACUTE
        // ê	4234	LATIN SMALL LETTER E WITH CIRCUMFLEX
        // ë	4235	LATIN SMALL LETTER E WITH DIAERESIS
        // ì	4236	LATIN SMALL LETTER I WITH GRAVE
        // í	4237	LATIN SMALL LETTER I WITH ACUTE
        // î	4238	LATIN SMALL LETTER I WITH CIRCUMFLEX
        // ï	4239	LATIN SMALL LETTER I WITH DIAERESIS
        // ð	4240	LATIN SMALL LETTER ETH
        // ñ	4241	LATIN SMALL LETTER N WITH TILDE
        // ò	4242	LATIN SMALL LETTER O WITH GRAVE
        // ó	4243	LATIN SMALL LETTER O WITH ACUTE
        // ô	4244	LATIN SMALL LETTER O WITH CIRCUMFLEX
        // õ	4245	LATIN SMALL LETTER O WITH TILDE
        // ö	4246	LATIN SMALL LETTER O WITH DIAERESIS
        // ÷	4247	DIVISION SIGN
        // ø	4248	LATIN SMALL LETTER O WITH STROKE
        // ù	4249	LATIN SMALL LETTER U WITH GRAVE
        // ú	4250	LATIN SMALL LETTER U WITH ACUTE
        // û	4251	LATIN SMALL LETTER U WITH CIRCUMFLEX
        // ü	4252	LATIN SMALL LETTER U WITH DIAERESIS
        // ý	4253	LATIN SMALL LETTER Y WITH ACUTE
        // þ	4254	LATIN SMALL LETTER THORN
        // ÿ	4255	LATIN SMALL LETTER Y WITH DIAERESIS
            if (value >= 4032 && value <= 4255) {
                if (bleKeyboard.isConnected()) {
                    bleKeyboard.write((char)value);
                    if (m_use_print_action_debug) {
                        Serial.print("Keyboard: ");
                        Serial.println((char)value);
                    }
                }
            }


        //1715243245
        //11111111 11111111 11111111 11111111
        /*   
        00 bit 1 0 
        byte 11111111  255
        signe byte 01111111 -127 127
        float integer -1.0 ,1.0
        11111111 11111111 11111111 11111111
        deux bytes (short)
        11111111 11011111 65535
        11111111 * 20 * 100000000000
        2 bytes 2 char
        BD BR BU BL  JD JR BTL BTR  ML MC MR AD AR AU AL  
        0  0  0  0   0  0  0   0    0  0  0  0  0  0  0   
        jlv jlh jrv jrh tl tr
        9   9   9   9   9  9
        */     
        }
    }
}