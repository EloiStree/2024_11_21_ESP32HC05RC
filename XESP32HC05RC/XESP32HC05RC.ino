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

//USE AS BIT BUFFER TO PARSE 1 0 TO ACTIONS
char m_binaryBufferOfInteger[33];

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
        integerCommandReceived(m_intCmd);
    }
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
    //setLeftVertical(1);
    //setRightVertical(1);
   // pressArrowN();
    
    releaseAllGamepad();
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
    || c=='\n' || c=='\r' || c==' '){
        continue;
    }
    return false;
  }
  return true;
}
void traditionalTextCommand(String text){
    Serial.print("CMD:");
    Serial.println(text); 

    if(text=="A") pressA(true);
    if(text=="a") pressA(false);
    if(text=="RECORD") recordStart();
    if(text=="record") recordStop();
}
void integerCommandReceived(int32_t value){
    Serial.print("Int:");
    Serial.println(value); // Echo the input back to the serial monitor
    
    integerToXbox(value);
    //uartCommand(m_clr0, m_clr1, m_clr2, m_clr3);
}

int m_gamepadReportModulo=200;
int m_nextReportCount =0;
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
  m_nextReportCount--;
  if(m_nextReportCount<0)
  {
      m_nextReportCount=m_gamepadReportModulo;
      gamepad->sendGamepadReport();
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
void pressRightStick(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_RS);}else{releaseButtonId(C_XBOX_BUTTON_RS);}}
void pressMenuRight(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_START);}else{releaseButtonId(C_XBOX_BUTTON_START);}}
void pressMenuLeft(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_SELECT);}else{releaseButtonId(C_XBOX_BUTTON_SELECT);}}
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
void releaseDPad(){
        gamepad->releaseDPad();
        gamepad->sendGamepadReport();
}
void pressDpad(XboxDpadFlags direction , bool isPress){
  if(isPress){
        gamepad->pressDPadDirectionFlag(direction);
        gamepad->sendGamepadReport();
  }
  else{
        
        gamepad->releaseDPad();
        gamepad->sendGamepadReport();
  }
}





float m_left_horizontal=0;
float m_left_vertical=0;
float m_right_horizontal=0;
float m_right_vertical=0;
//XBOX_TRIGGER_MIN  XBOX_TRIGGER_MAX XBOX_STICK_MAX

void setLeftHorizontal(float percent){ m_left_horizontal = percent; update_sticks();}
void setLeftVertical(float percent){ m_left_vertical = -percent; update_sticks();}
void setRightHorizontal(float percent){ m_right_horizontal = percent; update_sticks();}
void setRightVertical(float percent){ m_right_vertical = -percent; update_sticks();}

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



void integerToXbox(int value){
  // COMMAND TO SE TRUE OR FALSE BUTTONS OR BUTTON LIKE 
  if(value>=1000 && value<=2999){
        switch(value){
            case 1399: randomInputAllGamepadNoMenu(); break;
            case 2399: releaseAllGamepad(); break;
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
            case 1320: randomAxis(); break;
            case 2320: releaseAxis(); break;
            case 1321: recordStart(); break;
            case 2321: recordStop(); break;
            // Turn in clockwise
            case 1330: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 2330: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1331: setLeftVertical(1); setLeftHorizontal(0);     break;
            case 2331: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1332: setLeftVertical(1); setLeftHorizontal(1);     break;
            case 2332: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1333: setLeftVertical(0); setLeftHorizontal(1);     break;
            case 2333: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1334: setLeftVertical(-1); setLeftHorizontal(1);    break;
            case 2334: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1335: setLeftVertical(-1); setLeftHorizontal(0);    break;
            case 2335: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1336: setLeftVertical(-1); setLeftHorizontal(-1);   break;
            case 2336: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1337: setLeftVertical(0); setLeftHorizontal(-1);    break;
            case 2337: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1338: setLeftVertical(1); setLeftHorizontal(-1);    break;
            case 2338: setLeftVertical(0); setLeftHorizontal(0);     break;
            case 1340: setRightVertical(0); setRightHorizontal(0);    break;
            case 2340: setRightVertical(0); setRightHorizontal(0);    break;
            case 1341: setRightVertical(1); setRightHorizontal(0);    break;
            case 2341: setRightVertical(0); setRightHorizontal(0);    break;
            case 1342: setRightVertical(1); setRightHorizontal(1);    break;
            case 2342: setRightVertical(0); setRightHorizontal(0);    break;
            case 1343: setRightVertical(0); setRightHorizontal(1);    break;
            case 2343: setRightVertical(0); setRightHorizontal(0);    break;
            case 1344: setRightVertical(-1); setRightHorizontal(1);   break;
            case 2344: setRightVertical(0); setRightHorizontal(0);    break;
            case 1345: setRightVertical(-1); setRightHorizontal(0);   break;
            case 2345: setRightVertical(0); setRightHorizontal(0);    break;
            case 1346: setRightVertical(-1); setRightHorizontal(-1);  break;
            case 2346: setRightVertical(0); setRightHorizontal(0);    break;
            case 1347: setRightVertical(0); setRightHorizontal(-1);   break;
            case 2347: setRightVertical(0); setRightHorizontal(0);    break;
            case 1348: setRightVertical(1); setRightHorizontal(-1);   break;
            case 2348: setRightVertical(0); setRightHorizontal(0);    break;
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
        }
   }
   else if(value>=1800000000 && value<=1899999999){
    
    //18 50 20 00 10
    //1850200010
    //4 bytes because integer
    int rightHorizontalfrom1to99 =  (value/1)%100;
    int rightVerticalfrom1to99 =    (value/100)%100;
    int leftHorizontalfrom1to99 =   (value/10000)%100;
    int leftVerticalfrom1to99 =     (value/1000000)%100;
    setLeftHorizontal(turnFrom1To99AsPercent(leftHorizontalfrom1to99));
    setLeftVertical(turnFrom1To99AsPercent(leftVerticalfrom1to99));
    setRightHorizontal(turnFrom1To99AsPercent(rightHorizontalfrom1to99));
    setRightVertical(turnFrom1To99AsPercent(rightVerticalfrom1to99));
   }
   else if(value>=1700000000 && value<=1799999999){
      m_binaryBufferOfInteger[33]; // Buffer to store the binary representation (32 bits + null terminator)
      intToBinaryBuffer(value, m_binaryBufferOfInteger, 33);
      Serial.println(m_binaryBufferOfInteger);

      //1715243245
      //11111111 11111111 11111111 11111111
      
    }
}



float turnFrom1To99AsPercent(int value){
    if(value == 0) return 0;
    return float((double(value) - 1.0) / 98.0);
}
void intToBinaryBuffer(int value, char* buffer, size_t size) {
    if (size < 32) {
        return; // Ensure buffer is large enough for 32 bits
    }
    for (int i = 0; i < 32; i++) {
        buffer[31 - i] = (value & (1 << i)) ? '1' : '0';
    }
    buffer[32] = '\0'; // Null-terminate the string
}


void randomInputAllGamepadNoMenu(){

        pressA(random(0,2));
        pressB(random(0,2));
        pressX(random(0,2));
        pressY(random(0,2));
        pressLeftSideButton(random(0,2));
        pressRightSideButton(random(0,2));
        pressLeftStick(random(0,2));
        pressRightStick(random(0,2));
        byte rArrow = random(0,10);
        switch(rArrow){
            case 0: pressArrowN(); break;
            case 1: pressArrowNE(); break;
            case 2: pressArrowE(); break;
            case 3: pressArrowSE(); break;
            case 4: pressArrowS(); break;
            case 5: pressArrowSW(); break;
            case 6: pressArrowW(); break;
            case 7: pressArrowNW(); break;
            default: releaseDPad(); break;
        }
        setLeftHorizontal(random(-100,101)/100.0);
        setLeftVertical(random(-100,101)/100.0);
        setRightHorizontal(random(-100,101)/100.0);
        setRightVertical(random(-100,101)/100.0);
        setTriggerLeftPercent(random(0,101)/100.0);
        setTriggerRightPercent(random(0,101)/100.0);
        
        gamepad->sendGamepadReport();
}
void releaseAllGamepad(){

        setLeftHorizontal(0);
        setLeftVertical(0);
        setRightHorizontal(0);
        setRightVertical(0);
        setTriggerLeftPercent(0);
        setTriggerRightPercent(0);
        pressA(false);
        pressB(false);
        pressX(false);
        pressY(false);
        pressLeftSideButton(false);
        pressRightSideButton(false);
        pressLeftStick(false);
        pressRightStick(false);
        pressMenuRight(false);
        pressMenuLeft(false);
        pressHomeXboxButton(false);
        releaseDPad();
        gamepad->sendGamepadReport();
}

void releaseAxis(){
        setLeftHorizontal(0);
        setLeftVertical(0);
        setRightHorizontal(0);
        setRightVertical(0);
        setTriggerLeftPercent(0);
        setTriggerRightPercent(0);
        gamepad->sendGamepadReport();
}   
void randomAxis(){
        setLeftHorizontal(random(-100,101)/100.0);
        setLeftVertical(random(-100,101)/100.0);
        setRightHorizontal(random(-100,101)/100.0);
        setRightVertical(random(-100,101)/100.0);
        setTriggerLeftPercent(random(0,101)/100.0);
        setTriggerRightPercent(random(0,101)/100.0);
        gamepad->sendGamepadReport();
}



