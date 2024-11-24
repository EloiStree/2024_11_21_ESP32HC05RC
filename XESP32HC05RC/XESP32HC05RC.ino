/*
 * ----------------------------------------------------------------------------
 * "PIZZA LICENSE":
 * https://github.com/EloiStree wrote this file.
 * As long as you retain this notice you
 * can do whatever you want with this stuff.
 * If you think my code made you win a day of work,
 * send me a good 🍺 or a 🍕 at
 *  - https://www.patreon.com/eloiteaching
 * 
 * You can also support my work by building your own DIY input using Amazon links:
 * - https://github.com/EloiStree/HelloInput
 *
 * May the code be with you.
 *
 * Updated version: https://github.com/EloiStree/License
 * ----------------------------------------------------------------------------
 */

// This code is desing to allow: learning by codde,  QA testing in game and remapping of input
// ESP32:   https://github.com/EloiStree/HelloInput/issues/288
// Context: https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// Learn:   https://github.com/EloiStree/HelloInput


#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>


#define RXD2 16   // GPIO16 (U2RXD) Not Setable on the ESP32
#define TXD2 17   // GPIO17 (U2TXD) Not Setable on the ESP32
int ledPin = 15;  // NOT VERIFIED

XboxGamepadDevice *gamepad;
BleCompositeHID compositeHID("eLabRC XInput ESP32", "eLabRC", 100);


// RETURN THE RECEIVED BYTE TO THE SENDER
// IT ALLOWS TO SEE IF HC05 BRIDGE IS WORKING PROPERLY
bool m_return_byte_received = true;
// DISPLAY BYTE RECEIVED
bool m_use_print_byte_debug= true;
// DISPLAY ACTION TRIGGERED
bool m_use_print_action_debug= true;



//############################### START INTEGER FOUR CHAR INPUT ####################################

// TEXT is more easy for Human but my library out of hardware use integer little endian
// Turn in text mode if you use text as input
// If you need to come back to integer mode, restart the device or send the max int value
// SEND FFFF as text in your app to trigger the UTF8 MODE
// SEND 0000 as text or integer to reset the index to 0 for the following input
// In case there are some byte lost in the way.
// 0:c(IIII) THE DEVICE EXPECT INTEGER AS FOUR BYTES
// 1:c(FFFF) THE DEVICE EXPECT UTF8 AS FOUR CHARACTERS TURN THEM INTO INTEGER
// 2:c(ffff) THE DEVICE EXPECT \n AS END OF COMMAND THEN TURN TEXT IN INTEGER
// Those byte are reserved for mode change
//M:1 b(70 70 70 70) = UTF8 MODE C4:FFFF | Int:1179010630 
//M:2 b(214 214 214 214) = TEXT INTEGER MODE C4:ffff |  Int:1717986918 
//M:0 b(73 73 73 73) = INTEGER MODE C4:IIII |  Int:1229539657  
//CMD b(48 48 48 48) = RESET INDEX TO ZERO C4:0000 | Int:808464432
//CMD b(0 0 0 0) = RESET INDEX TO ZERO from bytes Int:0
int m_readingMode=2;
int INTEGER_MODE=0;
int INTEGER_MODE_INT = 1229539657;
int UTF8_MODE=1;
int UTF8_MODE_INT = 1179010630;
int TEXT_INTEGER_MODE=2;
int TEXT_INTEGER_MODE_INT = 1717986918;



byte m_blr0='0';
byte m_blr1='0';
byte m_blr2='0';
byte m_blr3='0';
char m_clr0='0';
char m_clr1='0';
char m_clr2='0';
char m_clr3='0';
int m_intCmd; // from 4 bytes as little endian
String m_textCmd="" ; // Corresponding UTF8 as text
String m_lineReturn="" ; // Corresponding UTF8 as text
int m_fourCharIndex=0;


bool IsInUTF8FourCharMode(){return m_readingMode==1;}
bool IsInUTF8LineReturnMode(){return m_readingMode==2;}
bool IsInByteLittleEndianMode(){return m_readingMode==0;}

void CheckForSwitchModeFromCurrent(){
    if(m_readingMode != TEXT_INTEGER_MODE){
        if(m_intCmd == 808464432){
            m_readingMode=0;
            Serial.println("############## Switch to BYTE INTEGER mode ############## ");
        }
        else if(m_intCmd == 1179010630){
            m_readingMode=1;
            Serial.println("############## Switch to UTF8 mode ############## ");
        }
        else if(m_intCmd == 1717986918){
            m_readingMode=2;
            Serial.println("############## TEXT INTEGER MODE ############## ");
        }
    }
}
void CheckFromSwitchModeFromLineReturn(){
    if(m_readingMode ==TEXT_INTEGER_MODE){
        if(m_textCmd == "IIII"){
            m_readingMode=0;
            Serial.println("############## Switch to BYTE INTEGER mode ############## ");
        }
        else if(m_textCmd == "FFFF"){
            m_readingMode=1;
            Serial.println("############## Switch to UTF8 mode ############## ");
        }
    }
}




// ##########  RECEIVED INTEGER AS BYTES
// https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
/// CONVERT FOUR BYTE IN LITTLE ENDIAN FORMAT TO SIGNED INTEGER 32 BITS
// USE: byte bytes[4] = {0xC0, 0x1D, 0xFE, 0xFF};  // Little-endian encoding of -123456
// USE: int32_t result = parseLittleEndian(bytes[0], bytes[1], bytes[2], bytes[3]);
int32_t parseLittleEndian(byte b0, byte b1, byte b2, byte b3) {
  // Combine bytes in little-endian order
  return ((int32_t)b0) | ((int32_t)b1 << 8) | ((int32_t)b2 << 16) | ((int32_t)b3 << 24);
}

// ##########  SEND INTEGER TO BYTES
// https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// CONVERT SIGNE INTEGER INTO FOUR BYTES IN LITTLE ENDIAN
// USE: int32_t value = -123456;
// USE: byte bytes[4];
// USE: intToBytes(value, bytes);
void intToBytes(int32_t value, uint8_t bytes[4]) {
  bytes[0] = value & 0xFF;         // Extract the lowest 8 bits
  bytes[1] = (value >> 8) & 0xFF;  // Shift right by 8 bits and extract the next 8 bits
  bytes[2] = (value >> 16) & 0xFF; // Shift right by 16 bits and extract the next 8 bits
  bytes[3] = (value >> 24) & 0xFF; // Shift right by 24 bits and extract the highest 8 bits
}




void displayFourByteAndChars(){
    Serial.print("B:");
    Serial.print(m_blr0);
    Serial.print(" ");
    Serial.print(m_blr1);
    Serial.print(" ");
    Serial.print(m_blr2);
    Serial.print(" ");
    Serial.print(m_blr3);
    Serial.print("|C:");
    Serial.print(m_clr0);
    Serial.print(" ");
    Serial.print(m_clr1);
    Serial.print(" ");
    Serial.print(m_clr2);
    Serial.print(" ");
    Serial.print(m_clr3);
    Serial.print("|Int:");
    Serial.print(m_intCmd);
    Serial.print("|Txt:");
    Serial.println(m_textCmd);
}

void notifyModeChanged(){
    if(m_readingMode==0){
        Serial.println("############## BYTE INTEGER MODE ############## ");
    }
    if(m_readingMode==1){
        Serial.println("############## UTF8 MODE ############## ");
    }
    if(m_readingMode==2){
        Serial.println("############## TEXT INTEGER MODE ############## ");
    }
}

void stackByte(byte b){
    m_blr0 = m_blr1;
    m_blr1 = m_blr2;
    m_blr2 = m_blr3;
    m_blr3 = b;
    m_clr0 =(char) m_blr0;
    m_clr1 =(char) m_blr1;
    m_clr2 =(char) m_blr2;
    m_clr3 =(char) m_blr3;
    m_intCmd = parseLittleEndian(m_blr0, m_blr1, m_blr2, m_blr3);
    m_textCmd=m_clr0+m_clr1+m_clr2+m_clr3;

    if(m_blr0 == 70 && m_blr1 == 70 && m_blr2 == 70 && m_blr3 == 70){
        m_readingMode=UTF8_MODE;
        notifyModeChanged();
    }
    if(m_blr0 == 214 && m_blr1 == 214 && m_blr2 == 214 && m_blr3 == 214){
        m_readingMode=TEXT_INTEGER_MODE;
        notifyModeChanged();
    }

    m_fourCharIndex++;
    displayFourByteAndChars();
    if(m_fourCharIndex>=4){
        m_fourCharIndex=0;
        CheckForSwitchModeFromCurrent();
        Serial.println("-->> Four byte Received");
    }
    integerCommandReceived(m_intCmd);
}
//############################### STOP INTEGER FOUR CHAR INPUT ####################################


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

bool isCharDigital(char c) {
    return c >= '0' && c <= '9';
}


void OnVibrateEvent(XboxGamepadOutputReportData data)
{
    if(data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0){
        digitalWrite(ledPin, LOW);
    } else {
        digitalWrite(ledPin, HIGH);
    }
    Serial.println("Vibration event. Weak motor: " + String(data.weakMotorMagnitude) + " Strong motor: " + String(data.strongMotorMagnitude));
}

void setup()
{
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT); // sets the digital pin as output
    
    // Uncomment one of the following two config types depending on which controller version you want to use
    // The XBox series X controller only works on linux kernels >= 6.5
    
    //XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
    XboxSeriesXControllerDeviceConfiguration* config = new XboxSeriesXControllerDeviceConfiguration();

    // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
    // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver. 
    BLEHostConfiguration hostConfig = config->getIdealHostConfiguration();
    Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
    Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
    Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
    Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
    Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));
    
    // Set up gamepad
    gamepad = new XboxGamepadDevice(config);

    // Set up vibration event handler
    FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
    gamepad->onVibrate.attach(vibrationSlot);

    // Add all child devices to the top-level composite HID device to manage them
    compositeHID.addDevice(gamepad);

    // Start the composite HID device to broadcast HID reports
    Serial.println("Starting composite HID device...");
    compositeHID.begin(hostConfig);

    Serial.println("Starting BLE work!");
    Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  
    Serial.println("UART1 initialized.");

}

void integerToCharAndBytesFour(int value){
    uint8_t bytes[4];
    intToBytes(value, bytes);
    for(int i=0; i<4; i++){
        stackByte(bytes[i]);
    }
}

void serialReceivedString(String input){
    if(isNumericString(input)){
        Serial.print("Int:");
        Serial.println(input); // Echo the input back to the serial monitor
        int number = input.toInt(); // Convert the string to an integer
        Serial.print("You entered the number: ");
        Serial.println(number);
        m_intCmd = number;
        integerToCharAndBytesFour(number);
        m_fourCharIndex = 0;
        integerCommandReceived(number);
    }
    else{
        Serial.print("CMD:"); 
        Serial.println(input); 
        if(input=="FFFF"){
            m_readingMode=UTF8_MODE;
            notifyModeChanged();
        }
        else if(input=="IIII"){
            m_readingMode=INTEGER_MODE;
            notifyModeChanged();
        }
        else if(input=="ffff"){
            m_readingMode=TEXT_INTEGER_MODE;
            notifyModeChanged();
        }
        traditionalTextCommand(input);
    }
}
void serialReceivedByte(byte incomingByte){
    stackByte(incomingByte);
}

// Function to check if a string contains only numeric characters
bool isNumericString(String str) {
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if(c=='0' || c=='1' || c=='2' || c=='3' || c=='4' 
    || c=='5' || c=='6' || c=='7' || c=='8' || c=='9'
    || c=='\n' || c=='\r'){
        continue;
    }
    return false;
  }
  return true;
}
void traditionalTextCommand(String text){
    Serial.print("CMD:");
    Serial.println(text); 

}
void integerCommandReceived(int32_t value){
    Serial.print("Int:");
    Serial.println(value); // Echo the input back to the serial monitor
    
    uartCommand(m_clr0, m_clr1, m_clr2, m_clr3);
}

void loop()
{

  if (Serial.available() > 0) {
    if(m_readingMode== TEXT_INTEGER_MODE){
        String input = Serial.readStringUntil('\n'); // Read the input until a newline character
        serialReceivedString(input);
    }
    else{
        byte incomingByte = Serial.read();
        serialReceivedByte(incomingByte);
    }
  }
  if (Serial2.available() > 0) {
    if(m_readingMode== TEXT_INTEGER_MODE){
        String input = Serial2.readStringUntil('\n'); // Read the input until a newline character
        serialReceivedString(input);
        if(m_return_byte_received){
            Serial2.write('\n');
        }
    }
    else{
        byte incomingByte = Serial2.read();
        serialReceivedByte(incomingByte);
        if(m_return_byte_received){
            Serial2.write(incomingByte);
        }
    }
  }
  delay(1);
}

// ################################ GAMEPAD CONTROL ####################################
uint16_t C_XBOX_BUTTON_A        = XBOX_BUTTON_A       ;
uint16_t C_XBOX_BUTTON_B        = XBOX_BUTTON_B ;
uint16_t C_XBOX_BUTTON_X        = XBOX_BUTTON_X ;
uint16_t C_XBOX_BUTTON_Y        = XBOX_BUTTON_Y;
uint16_t C_XBOX_BUTTON_LB       = XBOX_BUTTON_LB;
uint16_t C_XBOX_BUTTON_RB       = XBOX_BUTTON_RB;
uint16_t C_XBOX_BUTTON_START    = XBOX_BUTTON_START;
uint16_t C_XBOX_BUTTON_SELECT   = XBOX_BUTTON_SELECT;
uint16_t C_XBOX_BUTTON_HOME     = XBOX_BUTTON_HOME;
uint16_t C_XBOX_BUTTON_LS       = XBOX_BUTTON_LS;
uint16_t C_XBOX_BUTTON_RS       = XBOX_BUTTON_RS;



void pressA(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_A);}else{releaseButtonId(C_XBOX_BUTTON_A);}}
void pressB(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_B);}else{releaseButtonId(C_XBOX_BUTTON_B);}}
void pressX(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_X);}else{releaseButtonId(C_XBOX_BUTTON_X);}}
void pressY(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_Y);}else{releaseButtonId(C_XBOX_BUTTON_Y);}}
void pressLeftSideButton(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_LB);}else{releaseButtonId(C_XBOX_BUTTON_LB);}}
void pressRightSideButton(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_RB);}else{releaseButtonId(C_XBOX_BUTTON_RB);}}
void pressLeftStick(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_LS);}else{releaseButtonId(C_XBOX_BUTTON_LS);}}
void pressRigthStick(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_RS);}else{releaseButtonId(C_XBOX_BUTTON_RS);}}
void pressStart(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_START);}else{releaseButtonId(C_XBOX_BUTTON_START);}}
void pressSelectBack(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_SELECT);}else{releaseButtonId(C_XBOX_BUTTON_SELECT);}}
void pressHomeXboxButton(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_HOME);}else{releaseButtonId(C_XBOX_BUTTON_HOME);}}


void pressButtonId(uint16_t value){
        gamepad->press(value);
        gamepad->sendGamepadReport();
}

void releaseButtonId(uint16_t value){
        gamepad->release(value);
        gamepad->sendGamepadReport();
}

void recordStart(){
    gamepad->pressShare();
    gamepad->sendGamepadReport();
}
void recordStop(){
    gamepad->releaseShare();
    gamepad->sendGamepadReport();
}
void releaseDpad(){
        gamepad->releaseDPad();
        gamepad->sendGamepadReport();
}
void pressDpad(XboxDpadFlags direction , bool isPress){
  if(isPress){
        gamepad->pressDPadDirectionFlag(direction);
        gamepad->sendGamepadReport();
  }
}





float m_left_horizontal=0;
float m_left_vertical=0;
float m_right_horizontal=0;
float m_right_vertical=0;
//XBOX_TRIGGER_MIN  XBOX_TRIGGER_MAX XBOX_STICK_MAX

void setLeftHorizontal(float percent){ m_left_horizontal = percent; update_sticks();}
void setLeftVertical(float percent){ m_left_vertical = percent; update_sticks();}
void setRightHorizontal(float percent){ m_right_horizontal = percent; update_sticks();}
void setRightVertical(float percent){ m_right_vertical = percent; update_sticks();}

void setTriggerLeftPercent(float percent){ gamepad->setLeftTrigger(percent*XBOX_TRIGGER_MAX); gamepad->sendGamepadReport();}
void setTriggerRightPercent(float percent){ gamepad->setRightTrigger(percent*XBOX_TRIGGER_MAX); gamepad->sendGamepadReport();}

void pressArrowN(){pressDpad(XboxDpadFlags::NORTH,true);  }
void pressArrowE(){pressDpad(XboxDpadFlags::EAST,true); }
void pressArrowS(){pressDpad(XboxDpadFlags::SOUTH,true); }
void pressArrowW(){pressDpad(XboxDpadFlags::WEST,true); }
void pressArrowNE(){pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::NORTH | (uint8_t)XboxDpadFlags::EAST),true);}
void pressArrowNW(){pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::WEST | (uint8_t)XboxDpadFlags::NORTH),true);}
void pressArrowSE(){pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::EAST | (uint8_t)XboxDpadFlags::SOUTH),true);}
void pressArrowSW(){pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::SOUTH | (uint8_t)XboxDpadFlags::WEST),true);}

void update_sticks(){

        int16_t lx = m_left_horizontal * XBOX_STICK_MAX;
        int16_t ly = m_left_vertical * XBOX_STICK_MAX;
        int16_t rx = m_right_horizontal * XBOX_STICK_MAX;
        int16_t ry = m_right_vertical * XBOX_STICK_MAX;
        gamepad->setLeftThumb(lx, ly);
        gamepad->setRightThumb(rx, ry);
        gamepad->sendGamepadReport();
        
        
        Serial.print("LX: ");
        Serial.print(lx);
        Serial.print(" LY: ");
        Serial.print(ly);
        Serial.print(" RX: ");
        Serial.print(rx);
        Serial.print(" RY: ");
        Serial.println(ry);
        
}









/*_____________________________________________*/

void uartCommand(char clr0, char clr1, char clr2, char clr3) {
  if(clr0==0 && clr1==0){
    char cLeft = clr2;
    char cRight= clr3;
    if (m_use_print_received_double_char) {
        Serial.print("Left: ");
        Serial.print(cLeft);
        Serial.print(" Right: ");
        Serial.println(cRight);
    }

    if(!compositeHID.isConnected())
        return;
         if (cRight == '0') {
          switch (cLeft) {
                case 'A':  pressA(true); break;
                case 'a':  pressA(false); break;
                case 'B': pressB(true); break;
                case 'b': pressB(false); break;
                case 'C': pressX(true); break;
                case 'c': pressX(false); break;
                case 'D': pressY(true); break;
                case 'd': pressY(false); break;
                case 'E': pressLeftSideButton(true); break;
                case 'e': pressLeftSideButton(false); break;
                case 'F': pressRightSideButton(true); break;
                case 'f': pressRightSideButton(false); break;
                case 'G': pressStart(true); break;
                case 'g': pressStart(false); break;
                case 'H': pressSelectBack(true); break;
                case 'h': pressSelectBack(false); break;
                case 'I': pressHomeXboxButton(true); break;
                case 'i': pressHomeXboxButton(false); break;
                case 'J': pressLeftStick(true); break;
                case 'j': pressLeftStick(false); break;
                case 'K': pressRigthStick(true); break;
                case 'k': pressRigthStick(false); break;
                case 'L': recordStart(); break;
                case 'l': recordStop(); break;
                case 'M': releaseDpad(); break;
                case 'm': releaseDpad(); break;
                case 'N': pressArrowN(); break;
                case 'n': releaseDpad(); break;
                case 'O': pressArrowE(); break;
                case 'o': releaseDpad(); break;
                case 'P': pressArrowS(); break;
                case 'p': releaseDpad(); break;
                case 'Q': pressArrowW(); break;
                case 'q': releaseDpad(); break;
                case 'R': pressArrowNE(); break;
                case 'r': releaseDpad(); break;
                case 'S': pressArrowSE(); break;
                case 's': releaseDpad(); break;
                case 'T': pressArrowSW(); break;
                case 't': releaseDpad(); break;
                case 'U': pressArrowNW(); break;
                case 'u': releaseDpad(); break;
                case 'V': setTriggerLeftPercent(1); break;
                case 'v': setTriggerLeftPercent(0); break;
                case 'W': setTriggerRightPercent(1); break;
                case 'w': setTriggerRightPercent(0); break;
          }

        } else if (cRight == '1') {
            switch (cLeft) {
                case 'A': setLeftHorizontal(1); break;
                case 'a': setLeftHorizontal(-1); break;
                case 'B': setLeftVertical(1); break;
                case 'b': setLeftVertical(-1); break;
                case 'C': setRightHorizontal(1); break;
                case 'c': setRightHorizontal(-1); break;
                case 'D': setRightVertical(1); break;
                case 'd': setRightVertical(-1); break;
                case 'E': setTriggerLeftPercent(1); break;
                case 'e': setTriggerLeftPercent(0); break;
                case 'F': setTriggerRightPercent(1); break;
                case 'f': setTriggerRightPercent(0); break;
        }}
         else if (cRight == '5') {
            switch (cLeft) {
                case 'A': setLeftHorizontal(0); break;
                case 'a': setLeftHorizontal(0); break;
                case 'B': setLeftVertical(0); break;
                case 'b': setLeftVertical(0); break;
                case 'C': setRightHorizontal(0); break;
                case 'c': setRightHorizontal(0); break;
                case 'D': setRightVertical(0); break;
                case 'd': setRightVertical(0); break;
                case 'E': setTriggerLeftPercent(0); break;
                case 'e': setTriggerLeftPercent(0); break;
                case 'F': setTriggerRightPercent(0); break;
                case 'f': setTriggerRightPercent(0); break;
        }}
         else if (cRight == '2') {
            float value =0.5;
            switch (cLeft) {
                case 'A': setLeftHorizontal(value); break;
                case 'a': setLeftHorizontal(-value); break;
                case 'B': setLeftVertical(value); break;
                case 'b': setLeftVertical(-value); break;
                case 'C': setRightHorizontal(value); break;
                case 'c': setRightHorizontal(-value); break;
                case 'D': setRightVertical(value); break;
                case 'd': setRightVertical(-value); break;
                case 'E': setTriggerLeftPercent(value); break;
                case 'e': setTriggerLeftPercent(-value); break;
                case 'F': setTriggerRightPercent(value); break;
                case 'f': setTriggerRightPercent(-value); break;
        }}
        else if (cRight == '4') {
            float value =0.25;
            switch (cLeft) {
                case 'A': setLeftHorizontal(value); break;
                case 'a': setLeftHorizontal(-value); break;
                case 'B': setLeftVertical(value); break;
                case 'b': setLeftVertical(-value); break;
                case 'C': setRightHorizontal(value); break;
                case 'c': setRightHorizontal(-value); break;
                case 'D': setRightVertical(value); break;
                case 'd': setRightVertical(-value); break;
                case 'E': setTriggerLeftPercent(value); break;
                case 'e': setTriggerLeftPercent(-value); break;
                case 'F': setTriggerRightPercent(value); break;
                case 'f': setTriggerRightPercent(-value); break;
        }}
        else if (cRight == '8') {
            float value =0.125;
            switch (cLeft) {
                case 'A': setLeftHorizontal(value); break;
                case 'a': setLeftHorizontal(-value); break;
                case 'B': setLeftVertical(value); break;
                case 'b': setLeftVertical(-value); break;
                case 'C': setRightHorizontal(value); break;
                case 'c': setRightHorizontal(-value); break;
                case 'D': setRightVertical(value); break;
                case 'd': setRightVertical(-value); break;
                case 'E': setTriggerLeftPercent(value); break;
                case 'e': setTriggerLeftPercent(-value); break;
                case 'F': setTriggerRightPercent(value); break;
                case 'f': setTriggerRightPercent(-value); break;
        }
        }
        else if (cRight == '7') {
            float value =0.75;
            switch (cLeft) {
                case 'A': setLeftHorizontal(value); break;
                case 'a': setLeftHorizontal(-value); break;
                case 'B': setLeftVertical(value); break;
                case 'b': setLeftVertical(-value); break;
                case 'C': setRightHorizontal(value); break;
                case 'c': setRightHorizontal(-value); break;
                case 'D': setRightVertical(value); break;
                case 'd': setRightVertical(-value); break;
                case 'E': setTriggerLeftPercent(value); break;
                case 'e': setTriggerLeftPercent(-value); break;
                case 'F': setTriggerRightPercent(value); break;
                case 'f': setTriggerRightPercent(-value); break;
        }
        }
        else if (cRight == '9') {
            float value =0.90;
            switch (cLeft) {
                case 'A': setLeftHorizontal(value); break;
                case 'a': setLeftHorizontal(-value); break;
                case 'B': setLeftVertical(value); break;
                case 'b': setLeftVertical(-value); break;
                case 'C': setRightHorizontal(value); break;
                case 'c': setRightHorizontal(-value); break;
                case 'D': setRightVertical(value); break;
                case 'd': setRightVertical(-value); break;
                case 'E': setTriggerLeftPercent(value); break;
                case 'e': setTriggerLeftPercent(-value); break;
                case 'F': setTriggerRightPercent(value); break;
                case 'f': setTriggerRightPercent(-value); break;
        }
        }
        
  }
}

void test(){
    int d =300;
    releaseDpad();
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    // delay(1000);
    // pressB(true);
    // delay(1000);
    // pressB(false);
    
    delay(d);
    pressX(true);
    delay(d);
    pressX(false);
    delay(d);
    pressY(true);
    delay(d);
    pressY(false);
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    delay(d);
    pressLeftSideButton(true);
    delay(d);
    pressLeftSideButton(false);
    delay(d);
    pressRightSideButton(true);
    delay(d);
    pressRightSideButton(false);
    delay(d);
    pressStart(true);
    delay(d);
    pressStart(false);
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    delay(d);
    pressSelectBack(true);
    delay(d);
    pressSelectBack(false);
    delay(d);
    pressHomeXboxButton(true);
    delay(d);
    pressHomeXboxButton(false);
    delay(d);
    pressLeftStick(true);
    delay(d);
    pressLeftStick(false);
    delay(d);
    pressRigthStick(true);
    delay(d);
    pressRigthStick(false);
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    delay(d);
    recordStart();
    delay(d);
    recordStop();
    delay(d);
    releaseDpad();
    delay(d);
    pressArrowN();
    delay(d);
    pressArrowE();
    delay(d);
    pressArrowS();
    delay(d);
    pressArrowW();
    delay(d);
    pressArrowNE();
    delay(d);
    pressArrowNW();
    delay(d);
    pressArrowSE();
    delay(d);
    pressArrowSW();
    delay(d);
    releaseDpad();
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    delay(d);
    setLeftHorizontal(1);
    delay(d);
    setLeftHorizontal(-1);
    delay(d);
    setLeftHorizontal(0);
    delay(d);
    setLeftVertical(1);
    delay(d);
    setLeftVertical(-1);
    delay(d);
    setLeftVertical(0);
    delay(d);
    setRightHorizontal(1);
    delay(d);
    setRightHorizontal(-1);
    delay(d);
    setRightHorizontal(0);
    delay(d);
    setRightVertical(1);
    delay(d);
    setRightVertical(-1);
    delay(d);
    setRightVertical(0);
    delay(d);
    pressA(true);
    delay(d);
    pressA(false);
    delay(d);
    setTriggerLeftPercent(1);
    delay(d);
    setTriggerLeftPercent(0);
    delay(d);
    setTriggerRightPercent(1);
    delay(d);
    setTriggerRightPercent(0);
    delay(d);
    

}







