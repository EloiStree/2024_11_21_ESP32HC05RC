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

 // LAST UPDATE: 2024 11 26

// This code is desing to allow: learning by codde,  QA testing in game and remapping of input
// ESP32:   https://github.com/EloiStree/HelloInput/issues/288
// Context: https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// Learn:   https://github.com/EloiStree/HelloInput



#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <KeyboardDevice.h>
#include <MouseDevice.h>
#include <XboxGamepadDevice.h>
#include <KeyboardHIDCodes.h>


#define RXD2 16   // GPIO16 (U2RXD) Not Setable on the ESP32
#define TXD2 17   // GPIO17 (U2TXD) Not Setable on the ESP32
int m_lepPin = 2;  // Can bet 15 on some (depend of the model)
int m_vibrationPin=3; // If you want game feedback
int m_activablePinIOLength=16;
// https://chatgpt.com/share/67449be8-09c0-800e-a018-a2246365eedb
// Not verified yet
int m_activablePinIO[] = {4,5,12,13,14,15,18,19,21,22,23,25,26,27,32,33};
int m_activablePinIOState[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};


bool m_useBluetoothElectronicFeedBack=true;

bool m_useHardwareJoystick = false;
int m_pinJosytickLeftVertical = 33;
int m_pinJosytickLeftHorizontal = 32;
int m_pinJosytickRightVertical = 35;
int m_pinJosytickRightHorizontal = 34;

XboxGamepadDevice *gamepad;
GamepadDevice *gamepadGeneric;
KeyboardDevice* keyboard;
MouseDevice* mouse;
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
        digitalWrite(m_lepPin, LOW);
    } else {
        digitalWrite(m_lepPin, HIGH);
    }
    //Serial.println("Vibration event. Weak motor: " + String(data.weakMotorMagnitude) + " Strong motor: " + String(data.strongMotorMagnitude));
}
void pressReleaseMediaKey(uint32_t keyCode, bool press){
  if (press)
    keyboard->mediaKeyPress(keyCode);
  else 
    keyboard->mediaKeyRelease(keyCode);
}

bool m_mode_xinput=true;
void setup()
{
    Serial.begin(115200);
    pinMode(m_lepPin, OUTPUT); // sets the digital pin as output

    pinMode(m_pinJosytickLeftVertical, INPUT);
    pinMode(m_pinJosytickLeftHorizontal, INPUT);
    pinMode(m_pinJosytickRightVertical, INPUT);
    pinMode(m_pinJosytickRightHorizontal, INPUT);
    
    // Uncomment one of the following two config types depending on which controller version you want to use
    // The XBox series X controller only works on linux kernels >= 6.5
    if(m_mode_xinput){

      //XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
      XboxSeriesXControllerDeviceConfiguration* gamepadConfig = new XboxSeriesXControllerDeviceConfiguration();
      // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
      // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver. 
      BLEHostConfiguration hostConfig = gamepadConfig->getIdealHostConfiguration();
      Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
      Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
      Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
      Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
      Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));
      gamepad = new XboxGamepadDevice(gamepadConfig);
      FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
      gamepad->onVibrate.attach(vibrationSlot);
      compositeHID.addDevice(gamepad);
      Serial.println("Starting composite HID device...");
      compositeHID.begin(hostConfig);
      //setLeftVertical(1);
      //setRightVertical(1);
      releaseAllGamepad();
  }
  else if(!m_mode_xinput)
  {
      // Set up gamepad
      GamepadConfiguration gamepadClassicConfig;
      gamepadClassicConfig.setButtonCount(32);
      gamepadClassicConfig.setHatSwitchCount(1);
      Serial.print("iiikik");
      Serial.println(gamepadClassicConfig.getAxisCount());
      //gamepadConfig.setSimulationMode(true);
      gamepadGeneric = new GamepadDevice(gamepadClassicConfig);
    
      KeyboardConfiguration keyboardConfig;
      keyboardConfig.setUseMediaKeys(true);
      keyboard = new KeyboardDevice(keyboardConfig);
      MouseConfiguration mouseConfig;
      mouse = new MouseDevice(mouseConfig);
      compositeHID.addDevice(keyboard);
      compositeHID.addDevice(mouse);
      compositeHID.addDevice(gamepadGeneric);
      compositeHID.begin();
      delay(3000);
  }
    

      
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

void BE_SendSoundPing(char tag ='S',float volume=1){
  if(!m_useBluetoothElectronicFeedBack) 
    return;
  Serial2.print("*");
  Serial2.print(tag);
  Serial2.print("V");
  Serial2.println(int(volume*100.0));
}

void BE_SetGaugeChar(char tag='G', float percent=1.0){

  if(!m_useBluetoothElectronicFeedBack) 
    return;
  Serial2.print("*");
  Serial2.print(tag);
  Serial2.print(int(percent*100.0));
  Serial2.println("*");


}

void BE_SendColorInfo(char tag, float r, float g, float b){

  if(!m_useBluetoothElectronicFeedBack) 
    return;
  Serial2.print("*");
  Serial2.print(tag);
  Serial2.print("R");
  Serial2.print(int(r*255.0));
  Serial2.print("G");
  Serial2.print(int(g*255.0));
  Serial2.print("B");
  Serial2.println(int(b*255.0));
  Serial2.println("*");
}
void BE_SendColorInfoString(String tag, float r=1.0, float g=1.0, float b=1.0){
    // Bluetooth Electronic don't support other then char as tag.
  if(!m_useBluetoothElectronicFeedBack) 
    return;
  Serial2.print("*");
  Serial2.print(tag);
  Serial2.print("R");
  Serial2.print(int(r*255.0));
  Serial2.print("G");
  Serial2.print(int(g*255.0));
  Serial2.print("B");
  Serial2.print(int(b*255.0));
  Serial2.println("*");
}


void SwitchAllPins(){

    for(int i=0; i<m_activablePinIOLength; i++){
        SwitchStateOfPin(i);
    }
}
void SetAllPinsTo(bool value){

    for(int i=0; i<m_activablePinIOLength; i++){
        SetPinOnOff(i, value);
    }
  
}


void traditionalTextCommand(String text){
  
  
    text.trim();
    Serial.print("CMD:");
    Serial.println(text);
    BE_SendSoundPing();
    

  // 32 (Space)
  // 33-47 (Special Characters): ! " # $ % & ' ( ) * + , - . /
  // 48-57 (Numbers): 0 1 2 3 4 5 6 7 8 9
  // 58-64 (Special Characters): : ; < = > ? @
  // 65-90 (Uppercase Letters): A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
  // 91-96 (Special Characters): `[ \ ] ^ _ ``
  // 97-122 (Lowercase Letters): a b c d e f g h i j k l m n o p q r s t u v w x y z
  // 123-126 (Special Characters): { | } ~

    // Using Bluetooth Electronic Default version
    // Buttons 
    if(text=="G") pressA(true);
    else if(text=="BEDEBUG") m_useBluetoothElectronicFeedBack=true;
    else if(text=="bedebug") m_useBluetoothElectronicFeedBack=false;
    else if(text=="PINSWITCH") SwitchAllPins();
    else if(text=="g") pressA(false);
    else if(text=="Y") pressY(true);
    else if(text=="y") pressY(false);
    else if(text=="B") pressX(true);
    else if(text=="b") pressX(false);
    else if(text=="R") pressB(true);
    else if(text=="r") pressB(false);
    else if(text=="S" || text=="START") pressMenuRight(true);
    else if(text=="s" || text=="start") pressMenuRight(false);
    else if(text=="BACK") pressMenuLeft(true);
    else if(text=="back") pressMenuLeft(false);
    else if(text=="H") pressHomeXboxButton(true);
    else if(text=="h") pressHomeXboxButton(false);
    else if(text=="M") pressMenuLeft(true);
    else if(text=="m") pressMenuLeft(false);
    else if(text=="N") pressMenuRight(true);
    else if(text=="n") pressMenuRight(false);
    else if(text=="BA" || text=="BD") pressA(true);
    else if(text=="ba"|| text=="bd") pressA(false);
    else if(text=="BY"|| text=="BU") pressY(true);
    else if(text=="by"|| text=="bu") pressY(false);
    else if(text=="BX"|| text=="BL") pressX(true);
    else if(text=="bx"|| text=="bl") pressX(false);
    else if(text=="BB"|| text=="BR") pressB(true);
    else if(text=="bb"|| text=="br") pressB(false);
    else if(text=="AU") pressArrowN();
    else if(text=="AC") releaseDPad();
    else if(text=="AR") pressArrowE();
    else if(text=="AD") pressArrowS();
    else if(text=="AL") pressArrowW();
    else if(text=="AN") pressArrowN();
    else if(text=="AE") pressArrowE();
    else if(text=="AS") pressArrowS();
    else if(text=="AW") pressArrowW();
    else if(text=="ANW") pressArrowNW();
    else if(text=="ANE") pressArrowNE();
    else if(text=="ASE") pressArrowSE();
    else if(text=="ASW") pressArrowSW();
    else if(text=="ANW") pressArrowNW();
    else if(text=="ANE") pressArrowNE();
    else if(text=="ASE") pressArrowSE();
    else if(text=="ASW") pressArrowSW();
    else if(text=="RECORD") recordStart();
    else if(text=="record") recordStop();
    else if(text=="SBL") pressLeftSideButton(true);
    else if(text=="sbl") pressLeftSideButton(false);
    else if(text=="SBR") pressRightSideButton(true);
    else if(text=="sbr") pressRightSideButton(false);
    else if(text=="JL") pressLeftStick(true);
    else if(text=="jl") pressLeftStick(false);
    else if(text=="TL") setTriggerLeftPercent(1);
    else if(text=="tl") setTriggerLeftPercent(0);
    else if(text=="TR") setTriggerRightPercent(1);
    else if(text=="tr") setTriggerRightPercent(0);
    else if(text=="JR") pressRightStick(true);
    else if(text=="jr") pressRightStick(false);
    else if(text=="MR") pressMenuRight(true);
    else if(text=="mr") pressMenuRight(false);
    else if(text=="ML") pressMenuLeft(true);
    else if(text=="ml") pressMenuLeft(false);
    else if(text=="MC") pressHomeXboxButton(true);
    else if(text=="mc") pressHomeXboxButton(false);
    // Big Grey Circle
    else if(text=="M"){}
    // Big Rectancle 
    else if(text=="W"){}
    // Switch vertical on off
    else if(text=="C") m_useHardwareJoystick=true;
    else if(text=="c") m_useHardwareJoystick=false;
    // Power Button Switch on off
    else if(text=="D") digitalWrite(m_lepPin, HIGH);
    else if(text=="d") digitalWrite(m_lepPin, LOW);
    else if(text=="V") setVibrationOn(true);
    else if(text=="v") setVibrationOn(false);
    else{
        ParseIfContainsJoystickFromBlueElec(text);
        ParseIfContainsPinOnOff(text);
    }
}

void setVibrationOn(bool value){
    if(value){
        digitalWrite(m_vibrationPin, HIGH);
    }
    else{
        digitalWrite(m_vibrationPin, LOW);
    }
}



void BE_RemindMeOfPinNumber(int pinIndex){

    if(!m_useBluetoothElectronicFeedBack) 
        return;
    if(pinIndex<0 || pinIndex>= m_activablePinIOLength){
       Serial2.print("No pin for index:");
       Serial2.println(pinIndex);
    }
    else{
        Serial2.print("Pin index ");
        Serial2.print(pinIndex);
        Serial2.print(" is ");
        Serial2.println(m_activablePinIO[pinIndex]);
    }

}
void SwitchStateOfPin(int pinIndex){

  if(pinIndex<0 || pinIndex>= m_activablePinIOLength)
        return;

      if(m_activablePinIOState[pinIndex]==LOW){
          SetPinOnOff(pinIndex, true);
          }
      else {
          SetPinOnOff(pinIndex, false);
      }
}

void BE_SendPinState(int index){
if(m_useBluetoothElectronicFeedBack){
    if(index<0 || index>= m_activablePinIOLength)
        return;
       char p = '0';
       switch(index){
        case 0:p='0'; break;
        case 1:p='1'; break;
        case 2:p='2'; break;
        case 3:p='3'; break;
        case 4:p='4'; break;
        case 5:p='5'; break;
        case 6:p='6'; break;
        case 7:p='7'; break;
        case 8:p='8'; break;
        case 9:p='9'; break;
       }
      if(m_activablePinIOState[index]==HIGH)
        BE_SendColorInfo(p, 1.0, 1.0, 1.0);
      else
        BE_SendColorInfo(p, 0.1, 0.1, 0.1); 
    }
}

void SetPinOnOff (int pinIndex, bool isOn){
    if(pinIndex<0 || pinIndex>= m_activablePinIOLength)
      return;

    if(isOn){
        digitalWrite(m_activablePinIO[pinIndex], HIGH);
        m_activablePinIOState[pinIndex]=HIGH;
    }
    else{
        digitalWrite(m_activablePinIO[pinIndex], LOW);
        m_activablePinIOState[pinIndex]=LOW;
    }
    BE_SendPinState(pinIndex);
}

void ParseIfContainsPinOnOff(String text){

    int length = text.length();
    if(length>4) 
        return;
    if(!(text.charAt(0)=='P'||text.charAt(0)=='p'))
        return;
    bool isOn = text.charAt(0)=='P';
    int pin = text.substring(1).toInt();
    if(pin<0 || pin>=m_activablePinIOLength)
        return;
        
    SetPinOnOff(pin, isOn);
}

void ParseIfContainsJoystickFromBlueElec(String text){
    // 🤖 Genereted by GPT not tested yet.
    // Joystick left  X50Y50 from 0 to 100
    // Joystick left  LX50Y50 from 0 to 100
    // Joystick right RX50Y50 from 0 to 100
    bool isFound = false;
    float x = 0;
    float y = 0;
    // Check if the string starts with LX or RX or just X
    if (text.startsWith("LX")) {
        isFound = true;
        int xIndex = text.indexOf('X') + 1; // After LX
        int yIndex = text.indexOf('Y', xIndex) + 1;
        if (xIndex > 0 && yIndex > 0) {
            x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
            y = text.substring(yIndex).toFloat();
            x = ((x-50.0)/50.0);
            y = ((y-50.0)/50.0);
            setLeftHorizontal(x);
            setLeftVertical(-y);
        }
    } 
    else if (text.startsWith("RX")) {
        isFound = true;
        int xIndex = text.indexOf('X') + 1; // After RX
        int yIndex = text.indexOf('Y', xIndex) + 1;

        if (xIndex > 0 && yIndex > 0) {
            x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
            y = text.substring(yIndex).toFloat();
            x = ((x-50.0)/50.0);
            y = ((y-50.0)/50.0);
            setRightHorizontal(x);
            setRightVertical(-y);
        }

    } 
    else if (text.startsWith("X")) {
        isFound = true;
        int xIndex = text.indexOf('X') + 1;
        int yIndex = text.indexOf('Y', xIndex) + 1;

        if (xIndex > 0 && yIndex > 0) {
            x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
            y = text.substring(yIndex).toFloat();
            x = ((x-50.0)/50.0);
            y = ((y-50.0)/50.0);
            setLeftHorizontal(x);
            setLeftVertical(-y);
        }

    }
    else if (text.startsWith("TL")) {
        isFound = true;
        int tlIndex = text.indexOf('L') + 1; // After TL

        if (tlIndex > 0) {
            float triggerValue = text.substring(tlIndex).toInt();
            setTriggerLeftPercent(triggerValue/100.0);
        }
    } 
    else if (text.startsWith("TR")) {
        isFound = true;
        int trIndex = text.indexOf('R') + 1; // After TR

        if (trIndex > 0) {
            float triggerValue = text.substring(trIndex).toInt();
            setTriggerRightPercent(triggerValue/100.0);
        }
    } // Check if the string starts with 'A'
    else if (text.startsWith("A")) {
        isFound = true;
        float sensibilityInDegree=15.0;
        // Find positions of the key characters
        int startIndex = text.indexOf('A') + 1; // Start after 'A'
        int commaIndex = text.indexOf(',');
        int starIndex = text.indexOf('*');

        // Ensure all necessary characters are present
        if (startIndex > 0 && commaIndex > startIndex && starIndex > commaIndex) {
            // Extract and convert X and Y values
            String pitch = text.substring(startIndex, commaIndex);
            String roll = text.substring(commaIndex + 1, starIndex);
            
            x = ((pitch.toFloat()) /sensibilityInDegree); // From 'A' to ','
            y = ((roll.toFloat()) / sensibilityInDegree); // From ',' to '*'
            setLeftHorizontal(x);
            setLeftVertical(-y);
        }
    }
}


void integerCommandReceived(int32_t value){
    Serial.print("Int:");
    Serial.println(value); // Echo the input back to the serial monitor
    
    integerToXbox(value);
    integerToKeyboard(value);
    //uartCommand(m_clr0, m_clr1, m_clr2, m_clr3);
}

int m_gamepadReportModulo=200;
int m_nextReportCount =0;
int m_nextReadAnalogicCount=0;
int m_nextReadAnalogicModulo=50;

float ANALOG_MIN=0; 
float ANALOG_MAX=65535;
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

  if (m_useHardwareJoystick) {
    m_nextReadAnalogicCount--;
    if(m_nextReadAnalogicCount<0)
    {
       
        m_nextReadAnalogicCount=m_nextReadAnalogicModulo;
        int value = analogRead(m_pinJosytickLeftVertical);
        float percentLV = ((value/4095.0)-0.5)*2.0;
        value = analogRead(m_pinJosytickLeftHorizontal);
        float percentLH = ((value/4095.0)-0.5)*2.0;
        value = analogRead(m_pinJosytickRightVertical);
        float percentRV = ((value/4095.0)-0.5)*2.0;
        value = analogRead(m_pinJosytickRightHorizontal);
        float percentRH = ((value/4095.0)-0.5)*2.0;
      


        if(m_useHardwareJoystick){
            setLeftVertical(percentLV);
            setLeftHorizontal(percentLH);
            setRightVertical(percentRV);
            setRightHorizontal(percentRH);
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



void pressA(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_A);}else{releaseButtonId(C_XBOX_BUTTON_A);} BE_SendColorInfo('L',0,1,0); }
void pressB(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_B);}else{releaseButtonId(C_XBOX_BUTTON_B);}BE_SendColorInfo('L',1,0,0); }
void pressX(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_X);}else{releaseButtonId(C_XBOX_BUTTON_X);}BE_SendColorInfo('L',0,0,1); }
void pressY(bool isPress){if(isPress){pressButtonId(C_XBOX_BUTTON_Y);}else{releaseButtonId(C_XBOX_BUTTON_Y);}BE_SendColorInfo('L',1,1,0); }
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

void setTriggerLeftPercent(float percent){ gamepad->setLeftTrigger(percent*XBOX_TRIGGER_MAX); gamepad->sendGamepadReport(); BE_SetGaugeChar('L',percent);}
void setTriggerRightPercent(float percent){ gamepad->setRightTrigger(percent*XBOX_TRIGGER_MAX); gamepad->sendGamepadReport();BE_SetGaugeChar('R',percent);}

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
#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_LALT   0x04
#define KEY_MOD_LMETA  0x08
#define KEY_MOD_RCTRL  0x10
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_RALT   0x40
#define KEY_MOD_RMETA  0x80
#define KEY_A 0x04 // Keyboard a and A
#define KEY_B 0x05 // Keyboard b and B
#define KEY_C 0x06 // Keyboard c and C
#define KEY_D 0x07 // Keyboard d and D
#define KEY_E 0x08 // Keyboard e and E
#define KEY_F 0x09 // Keyboard f and F
#define KEY_G 0x0a // Keyboard g and G
#define KEY_H 0x0b // Keyboard h and H
#define KEY_I 0x0c // Keyboard i and I
#define KEY_J 0x0d // Keyboard j and J
#define KEY_K 0x0e // Keyboard k and K
#define KEY_L 0x0f // Keyboard l and L
#define KEY_M 0x10 // Keyboard m and M
#define KEY_N 0x11 // Keyboard n and N
#define KEY_O 0x12 // Keyboard o and O
#define KEY_P 0x13 // Keyboard p and P
#define KEY_Q 0x14 // Keyboard q and Q
#define KEY_R 0x15 // Keyboard r and R
#define KEY_S 0x16 // Keyboard s and S
#define KEY_T 0x17 // Keyboard t and T
#define KEY_U 0x18 // Keyboard u and U
#define KEY_V 0x19 // Keyboard v and V
#define KEY_W 0x1a // Keyboard w and W
#define KEY_X 0x1b // Keyboard x and X
#define KEY_Y 0x1c // Keyboard y and Y
#define KEY_Z 0x1d // Keyboard z and Z

#define KEY_1 0x1e // Keyboard 1 and !
#define KEY_2 0x1f // Keyboard 2 and @
#define KEY_3 0x20 // Keyboard 3 and #
#define KEY_4 0x21 // Keyboard 4 and $
#define KEY_5 0x22 // Keyboard 5 and %
#define KEY_6 0x23 // Keyboard 6 and ^
#define KEY_7 0x24 // Keyboard 7 and &
#define KEY_8 0x25 // Keyboard 8 and *
#define KEY_9 0x26 // Keyboard 9 and (
#define KEY_0 0x27 // Keyboard 0 and )

#define KEY_ENTER 0x28 // Keyboard Return (ENTER)
#define KEY_ESC 0x29 // Keyboard ESCAPE
#define KEY_BACKSPACE 0x2a // Keyboard DELETE (Backspace)
#define KEY_TAB 0x2b // Keyboard Tab
#define KEY_SPACE 0x2c // Keyboard Spacebar
#define KEY_MINUS 0x2d // Keyboard - and _
#define KEY_EQUAL 0x2e // Keyboard = and +
#define KEY_LEFTBRACE 0x2f // Keyboard [ and {
#define KEY_RIGHTBRACE 0x30 // Keyboard ] and }
#define KEY_BACKSLASH 0x31 // Keyboard \ and |
#define KEY_HASHTILDE 0x32 // Keyboard Non-US # and ~
#define KEY_SEMICOLON 0x33 // Keyboard ; and :
#define KEY_APOSTROPHE 0x34 // Keyboard ' and "
#define KEY_GRAVE 0x35 // Keyboard ` and ~
#define KEY_COMMA 0x36 // Keyboard , and <
#define KEY_DOT 0x37 // Keyboard . and >
#define KEY_SLASH 0x38 // Keyboard / and ?
#define KEY_CAPSLOCK 0x39 // Keyboard Caps Lock

#define KEY_F1 0x3a // Keyboard F1
#define KEY_F2 0x3b // Keyboard F2
#define KEY_F3 0x3c // Keyboard F3
#define KEY_F4 0x3d // Keyboard F4
#define KEY_F5 0x3e // Keyboard F5
#define KEY_F6 0x3f // Keyboard F6
#define KEY_F7 0x40 // Keyboard F7
#define KEY_F8 0x41 // Keyboard F8
#define KEY_F9 0x42 // Keyboard F9
#define KEY_F10 0x43 // Keyboard F10
#define KEY_F11 0x44 // Keyboard F11
#define KEY_F12 0x45 // Keyboard F12

#define KEY_SYSRQ 0x46 // Keyboard Print Screen
#define KEY_SCROLLLOCK 0x47 // Keyboard Scroll Lock
#define KEY_PAUSE 0x48 // Keyboard Pause
#define KEY_INSERT 0x49 // Keyboard Insert
#define KEY_HOME 0x4a // Keyboard Home
#define KEY_PAGEUP 0x4b // Keyboard Page Up
#define KEY_DELETE 0x4c // Keyboard Delete Forward
#define KEY_END 0x4d // Keyboard End
#define KEY_PAGEDOWN 0x4e // Keyboard Page Down
#define KEY_RIGHT 0x4f // Keyboard Right Arrow
#define KEY_LEFT 0x50 // Keyboard Left Arrow
#define KEY_DOWN 0x51 // Keyboard Down Arrow
#define KEY_UP 0x52 // Keyboard Up Arrow

#define KEY_NUMLOCK 0x53 // Keyboard Num Lock and Clear
#define KEY_KPSLASH 0x54 // Keypad /
#define KEY_KPASTERISK 0x55 // Keypad *
#define KEY_KPMINUS 0x56 // Keypad -
#define KEY_KPPLUS 0x57 // Keypad +
#define KEY_KPENTER 0x58 // Keypad ENTER
#define KEY_KP1 0x59 // Keypad 1 and End
#define KEY_KP2 0x5a // Keypad 2 and Down Arrow
#define KEY_KP3 0x5b // Keypad 3 and PageDn
#define KEY_KP4 0x5c // Keypad 4 and Left Arrow
#define KEY_KP5 0x5d // Keypad 5
#define KEY_KP6 0x5e // Keypad 6 and Right Arrow
#define KEY_KP7 0x5f // Keypad 7 and Home
#define KEY_KP8 0x60 // Keypad 8 and Up Arrow
#define KEY_KP9 0x61 // Keypad 9 and Page Up
#define KEY_KP0 0x62 // Keypad 0 and Insert
#define KEY_KPDOT 0x63 // Keypad . and Delete

#define KEY_102ND 0x64 // Keyboard Non-US \ and |
#define KEY_COMPOSE 0x65 // Keyboard Application
#define KEY_POWER 0x66 // Keyboard Power
#define KEY_KPEQUAL 0x67 // Keypad =

#define KEY_F13 0x68 // Keyboard F13
#define KEY_F14 0x69 // Keyboard F14
#define KEY_F15 0x6a // Keyboard F15
#define KEY_F16 0x6b // Keyboard F16
#define KEY_F17 0x6c // Keyboard F17
#define KEY_F18 0x6d // Keyboard F18
#define KEY_F19 0x6e // Keyboard F19
#define KEY_F20 0x6f // Keyboard F20
#define KEY_F21 0x70 // Keyboard F21
#define KEY_F22 0x71 // Keyboard F22
#define KEY_F23 0x72 // Keyboard F23
#define KEY_F24 0x73 // Keyboard F24

#define KEY_OPEN 0x74 // Keyboard Execute
#define KEY_HELP 0x75 // Keyboard Help
#define KEY_PROPS 0x76 // Keyboard Menu
#define KEY_FRONT 0x77 // Keyboard Select
#define KEY_STOP 0x78 // Keyboard Stop
#define KEY_AGAIN 0x79 // Keyboard Again
#define KEY_UNDO 0x7a // Keyboard Undo
#define KEY_CUT 0x7b // Keyboard Cut
#define KEY_COPY 0x7c // Keyboard Copy
#define KEY_PASTE 0x7d // Keyboard Paste
#define KEY_FIND 0x7e // Keyboard Find
#define KEY_MUTE 0x7f // Keyboard Mute
#define KEY_VOLUMEUP 0x80 // Keyboard Volume Up
#define KEY_VOLUMEDOWN 0x81 // Keyboard Volume Down
// 0x82  Keyboard Locking Caps Lock
// 0x83  Keyboard Locking Num Lock
// 0x84  Keyboard Locking Scroll Lock
#define KEY_KPCOMMA 0x85 // Keypad Comma

#define KEY_LEFTCTRL 0xe0 // Keyboard Left Control
#define KEY_LEFTSHIFT 0xe1 // Keyboard Left Shift
#define KEY_LEFTALT 0xe2 // Keyboard Left Alt
#define KEY_LEFTMETA 0xe3 // Keyboard Left GUI
#define KEY_RIGHTCTRL 0xe4 // Keyboard Right Control
#define KEY_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define KEY_RIGHTALT 0xe6 // Keyboard Right Alt
#define KEY_RIGHTMETA 0xe7 // Keyboard Right GUI

// Media key bitflags
#define KEY_MEDIA_PLAY 0x1
#define KEY_MEDIA_PAUSE 0x2
#define KEY_MEDIA_RECORD 0x4
#define KEY_MEDIA_FASTFORWARD 0x8
#define KEY_MEDIA_REWIND 0x10
#define KEY_MEDIA_NEXTTRACK 0x20
#define KEY_MEDIA_PREVIOUSTRACK 0x40
#define KEY_MEDIA_STOP 0x80
#define KEY_MEDIA_EJECT 0x100
#define KEY_MEDIA_RANDOMPLAY 0x200
#define KEY_MEDIA_REPEAT 0x400
#define KEY_MEDIA_PLAYPAUSE 0x800
#define KEY_MEDIA_MUTE 0x1000
#define KEY_MEDIA_VOLUMEUP 0x2000
#define KEY_MEDIA_VOLUMEDOWN 0x4000
#define KEY_MEDIA_WWWHOME 0x8000
#define KEY_MEDIA_MYCOMPUTER 0x10000
#define KEY_MEDIA_CALCULATOR 0x20000
#define KEY_MEDIA_WWWFAVORITES 0x40000
#define KEY_MEDIA_WWWSEARCH 0x80000
#define KEY_MEDIA_WWWSTOP 0x100000
#define KEY_MEDIA_WWWBACK 0x200000
#define KEY_MEDIA_MEDIASELECT 0x400000
#define KEY_MEDIA_MAIL 0x800000

// LED bitflags
#define KEY_LED_NUMLOCK 0x1
#define KEY_LED_CAPSLOCK 0x2
#define KEY_LED_SCROLLLOCK 0x4
#define KEY_LED_COMPOSE 0x8
#define KEY_LED_KANA 0x10

void integerToKeyboard(int value){

    if(value>=1000 && value <=2999){
        // https://github.com/Mystfit/ESP32-BLE-CompositeHID/blob/master/KeyboardHIDCodes.h
        // https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
        //keyRelease
        switch(value){
            case 1008: keyboard->keyPress(KEY_BACKSPACE); break;
            case 1009: keyboard->keyPress(KEY_TAB); break;
            case 1013: keyboard->keyPress(KEY_ENTER); break;
            case 1016: keyboard->keyPress(KEY_LEFTSHIFT); break;
            case 1017: keyboard->keyPress(KEY_LEFTCTRL); break;
            case 1018: keyboard->keyPress(KEY_LEFTALT); break;
            case 1019: keyboard->keyPress(KEY_PAUSE); break;
            case 1020: keyboard->keyPress(KEY_CAPSLOCK); break;
            case 1027: keyboard->keyPress(KEY_ESC); break;
            case 1032: keyboard->keyPress(KEY_SPACE); break;
            case 1033: keyboard->keyPress(KEY_PAGEUP); break;
            case 1034: keyboard->keyPress(KEY_PAGEDOWN); break;
            case 1035: keyboard->keyPress(KEY_END); break;
            case 1036: keyboard->keyPress(KEY_HOME); break;
            case 1037: keyboard->keyPress(KEY_LEFT); break;
            case 1038: keyboard->keyPress(KEY_UP); break;
            case 1039: keyboard->keyPress(KEY_RIGHT); break;
            case 1040: keyboard->keyPress(KEY_DOWN); break;
            case 1041: keyboard->keyPress(KEY_FRONT ); break;
            case 1042: keyboard->keyPress(KEY_SYSRQ ); break;
            case 1043: keyboard->keyPress(KEY_OPEN ); break;
            case 1044: keyboard->keyPress(KEY_SYSRQ ); break;
            case 1045: keyboard->keyPress(KEY_INSERT); break;
            case 1046: keyboard->keyPress(KEY_DELETE); break;
            case 1048: keyboard->keyPress(KEY_0); break;
            case 1049: keyboard->keyPress(KEY_1); break;
            case 1050: keyboard->keyPress(KEY_2); break;
            case 1051: keyboard->keyPress(KEY_3); break;
            case 1052: keyboard->keyPress(KEY_4); break;
            case 1053: keyboard->keyPress(KEY_5); break;
            case 1054: keyboard->keyPress(KEY_6); break;
            case 1055: keyboard->keyPress(KEY_7); break;
            case 1056: keyboard->keyPress(KEY_8); break;
            case 1057: keyboard->keyPress(KEY_9); break;
            case 1065: keyboard->keyPress(KEY_A); break;
            case 1066: keyboard->keyPress(KEY_B); break;
            case 1067: keyboard->keyPress(KEY_C); break;
            case 1068: keyboard->keyPress(KEY_D); break;
            case 1069: keyboard->keyPress(KEY_E); break;
            case 1070: keyboard->keyPress(KEY_F); break;
            case 1071: keyboard->keyPress(KEY_G); break;
            case 1072: keyboard->keyPress(KEY_H); break;
            case 1073: keyboard->keyPress(KEY_I); break;
            case 1074: keyboard->keyPress(KEY_J); break;
            case 1075: keyboard->keyPress(KEY_K); break;
            case 1076: keyboard->keyPress(KEY_L); break;
            case 1077: keyboard->keyPress(KEY_M); break;
            case 1078: keyboard->keyPress(KEY_N); break;
            case 1079: keyboard->keyPress(KEY_O); break;
            case 1080: keyboard->keyPress(KEY_P); break;
            case 1081: keyboard->keyPress(KEY_Q); break;
            case 1082: keyboard->keyPress(KEY_R); break;
            case 1083: keyboard->keyPress(KEY_S); break;
            case 1084: keyboard->keyPress(KEY_T); break;
            case 1085: keyboard->keyPress(KEY_U); break;
            case 1086: keyboard->keyPress(KEY_V); break;
            case 1087: keyboard->keyPress(KEY_W); break;
            case 1088: keyboard->keyPress(KEY_X); break;
            case 1089: keyboard->keyPress(KEY_Y); break;
            case 1090: keyboard->keyPress(KEY_Z); break;
            case 1091: keyboard->keyPress(KEY_LEFTMETA); break;
            case 1092: keyboard->keyPress(KEY_RIGHTMETA); break;
            case 1093: keyboard->keyPress(KEY_PROPS); break;
            case 1096: keyboard->keyPress(KEY_KP0); break;
            case 1097: keyboard->keyPress(KEY_KP1); break;
            case 1098: keyboard->keyPress(KEY_KP2); break;
            case 1099: keyboard->keyPress(KEY_KP3); break;
            case 1100: keyboard->keyPress(KEY_KP4); break;
            case 1101: keyboard->keyPress(KEY_KP5); break;
            case 1102: keyboard->keyPress(KEY_KP6); break;
            case 1103: keyboard->keyPress(KEY_KP7); break;
            case 1104: keyboard->keyPress(KEY_KP8); break;
            case 1105: keyboard->keyPress(KEY_KP9); break;
            case 1106: keyboard->keyPress(KEY_KPASTERISK); break;
            case 1107: keyboard->keyPress(KEY_KPPLUS); break;
            case 1108: keyboard->keyPress(KEY_KPCOMMA); break;
            case 1109: keyboard->keyPress(KEY_KPMINUS); break;
            case 1110: keyboard->keyPress(KEY_KPDOT); break;
            case 1111: keyboard->keyPress(KEY_KPSLASH); break;
            case 1112: keyboard->keyPress(KEY_F1); break;
            case 1113: keyboard->keyPress(KEY_F2); break;
            case 1114: keyboard->keyPress(KEY_F3); break;
            case 1115: keyboard->keyPress(KEY_F4); break;
            case 1116: keyboard->keyPress(KEY_F5); break;
            case 1117: keyboard->keyPress(KEY_F6); break;
            case 1118: keyboard->keyPress(KEY_F7); break;
            case 1119: keyboard->keyPress(KEY_F8); break;
            case 1120: keyboard->keyPress(KEY_F9); break;
            case 1121: keyboard->keyPress(KEY_F10); break;
            case 1122: keyboard->keyPress(KEY_F11); break;
            case 1123: keyboard->keyPress(KEY_F12); break;
            case 1124: keyboard->keyPress(KEY_F13); break;
            case 1125: keyboard->keyPress(KEY_F14); break;
            case 1126: keyboard->keyPress(KEY_F15); break;
            case 1127: keyboard->keyPress(KEY_F16); break;
            case 1128: keyboard->keyPress(KEY_F17); break;
            case 1129: keyboard->keyPress(KEY_F18); break;
            case 1130: keyboard->keyPress(KEY_F19); break;
            case 1131: keyboard->keyPress(KEY_F20); break;
            case 1132: keyboard->keyPress(KEY_F21); break;
            case 1133: keyboard->keyPress(KEY_F22); break;
            case 1134: keyboard->keyPress(KEY_F23); break;
            case 1135: keyboard->keyPress(KEY_F24); break;
            case 1144: keyboard->keyPress(KEY_NUMLOCK); break;
            case 1145: keyboard->keyPress(KEY_SCROLLLOCK); break;
            case 1160: keyboard->keyPress(KEY_LEFTSHIFT); break;
            case 1161: keyboard->keyPress(KEY_RIGHTSHIFT); break;
            case 1162: keyboard->keyPress(KEY_LEFTCTRL); break;
            case 1163: keyboard->keyPress(KEY_RIGHTCTRL); break;
            case 1164: keyboard->keyPress(KEY_LEFTALT); break;
            case 1165: keyboard->keyPress(KEY_RIGHTALT); break;
            case 1166: keyboard->keyPress(KEY_MEDIA_WWWBACK); break;
            case 1169: keyboard->keyPress(KEY_MEDIA_WWWSTOP); break;
            case 1170: keyboard->keyPress(KEY_MEDIA_WWWSEARCH); break;
            case 1171: keyboard->keyPress(KEY_MEDIA_WWWFAVORITES); break;
            case 1172: keyboard->keyPress(KEY_MEDIA_WWWHOME); break;
            case 1173: keyboard->keyPress( KEY_MUTE); break;
            case 1174: keyboard->keyPress(KEY_MEDIA_VOLUMEDOWN); break;
            case 1175: keyboard->keyPress(KEY_MEDIA_VOLUMEUP); break;
            case 1176: keyboard->keyPress( KEY_MEDIA_NEXTTRACK); break;
            case 1177: keyboard->keyPress( KEY_MEDIA_PREVIOUSTRACK); break;
            case 1178: keyboard->keyPress(KEY_MEDIA_STOP); break;
            case 1179: keyboard->keyPress( KEY_MEDIA_PLAYPAUSE); break;
            case 1180: keyboard->keyPress( KEY_MEDIA_MAIL); break;
            case 1181: keyboard->keyPress( KEY_MEDIA_MEDIASELECT); break;
            case 1250: keyboard->keyPress( KEY_MEDIA_PLAY); break;
            case 1260: pressReleaseMediaKey(KEY_MEDIA_PLAY,true); break;        
            case 1261: pressReleaseMediaKey(KEY_MEDIA_PAUSE,true); break;
            case 1262: pressReleaseMediaKey(KEY_MEDIA_RECORD,true); break;
            case 1263: pressReleaseMediaKey(KEY_MEDIA_FASTFORWARD,true); break;
            case 1264: pressReleaseMediaKey(KEY_MEDIA_REWIND,true); break;
            case 1265: pressReleaseMediaKey(KEY_MEDIA_NEXTTRACK,true); break;
            case 1266: pressReleaseMediaKey(KEY_MEDIA_PREVIOUSTRACK,true); break;
            case 1267: pressReleaseMediaKey(KEY_MEDIA_STOP,true); break;
            case 1268: pressReleaseMediaKey(KEY_MEDIA_EJECT,true); break;
            case 1269: pressReleaseMediaKey(KEY_MEDIA_RANDOMPLAY,true); break;
            case 1270: pressReleaseMediaKey(KEY_MEDIA_REPEAT,true); break;
            case 1271: pressReleaseMediaKey(KEY_MEDIA_PLAYPAUSE,true); break;
            case 1272: pressReleaseMediaKey(KEY_MEDIA_MUTE,true); break;
            case 1273: pressReleaseMediaKey(KEY_MEDIA_VOLUMEUP,true); break;
            case 1274: pressReleaseMediaKey(KEY_MEDIA_VOLUMEDOWN,true); break;
            case 2008: keyboard->keyRelease(KEY_BACKSPACE); break;
            case 2009: keyboard->keyRelease(KEY_TAB); break;
            case 2013: keyboard->keyRelease(KEY_ENTER); break;
            case 2016: keyboard->keyRelease(KEY_LEFTSHIFT); break;
            case 2017: keyboard->keyRelease(KEY_LEFTCTRL); break;
            case 2018: keyboard->keyRelease(KEY_LEFTALT); break;
            case 2019: keyboard->keyRelease(KEY_PAUSE); break;
            case 2020: keyboard->keyRelease(KEY_CAPSLOCK); break;
            case 2027: keyboard->keyRelease(KEY_ESC); break;
            case 2032: keyboard->keyRelease(KEY_SPACE); break;
            case 2033: keyboard->keyRelease(KEY_PAGEUP); break;
            case 2034: keyboard->keyRelease(KEY_PAGEDOWN); break;
            case 2035: keyboard->keyRelease(KEY_END); break;
            case 2036: keyboard->keyRelease(KEY_HOME); break;
            case 2037: keyboard->keyRelease(KEY_LEFT); break;
            case 2038: keyboard->keyRelease(KEY_UP); break;
            case 2039: keyboard->keyRelease(KEY_RIGHT); break;
            case 2040: keyboard->keyRelease(KEY_DOWN); break;
            case 2041: keyboard->keyRelease(KEY_FRONT ); break;
            case 2042: keyboard->keyRelease(KEY_SYSRQ ); break;
            case 2043: keyboard->keyRelease(KEY_OPEN ); break;
            case 2044: keyboard->keyRelease(KEY_SYSRQ ); break;
            case 2045: keyboard->keyRelease(KEY_INSERT); break;
            case 2046: keyboard->keyRelease(KEY_DELETE); break;
            case 2048: keyboard->keyRelease(KEY_0); break;
            case 2049: keyboard->keyRelease(KEY_1); break;
            case 2050: keyboard->keyRelease(KEY_2); break;
            case 2051: keyboard->keyRelease(KEY_3); break;
            case 2052: keyboard->keyRelease(KEY_4); break;
            case 2053: keyboard->keyRelease(KEY_5); break;
            case 2054: keyboard->keyRelease(KEY_6); break;
            case 2055: keyboard->keyRelease(KEY_7); break;
            case 2056: keyboard->keyRelease(KEY_8); break;
            case 2057: keyboard->keyRelease(KEY_9); break;
            case 2065: keyboard->keyRelease(KEY_A); break;
            case 2066: keyboard->keyRelease(KEY_B); break;
            case 2067: keyboard->keyRelease(KEY_C); break;
            case 2068: keyboard->keyRelease(KEY_D); break;
            case 2069: keyboard->keyRelease(KEY_E); break;
            case 2070: keyboard->keyRelease(KEY_F); break;
            case 2071: keyboard->keyRelease(KEY_G); break;
            case 2072: keyboard->keyRelease(KEY_H); break;
            case 2073: keyboard->keyRelease(KEY_I); break;
            case 2074: keyboard->keyRelease(KEY_J); break;
            case 2075: keyboard->keyRelease(KEY_K); break;
            case 2076: keyboard->keyRelease(KEY_L); break;
            case 2077: keyboard->keyRelease(KEY_M); break;
            case 2078: keyboard->keyRelease(KEY_N); break;
            case 2079: keyboard->keyRelease(KEY_O); break;
            case 2080: keyboard->keyRelease(KEY_P); break;
            case 2081: keyboard->keyRelease(KEY_Q); break;
            case 2082: keyboard->keyRelease(KEY_R); break;
            case 2083: keyboard->keyRelease(KEY_S); break;
            case 2084: keyboard->keyRelease(KEY_T); break;
            case 2085: keyboard->keyRelease(KEY_U); break;
            case 2086: keyboard->keyRelease(KEY_V); break;
            case 2087: keyboard->keyRelease(KEY_W); break;
            case 2088: keyboard->keyRelease(KEY_X); break;
            case 2089: keyboard->keyRelease(KEY_Y); break;
            case 2090: keyboard->keyRelease(KEY_Z); break;
            case 2091: keyboard->keyRelease(KEY_LEFTMETA); break;
            case 2092: keyboard->keyRelease(KEY_RIGHTMETA); break;
            case 2093: keyboard->keyRelease(KEY_PROPS); break;
            case 2096: keyboard->keyRelease(KEY_KP0); break;
            case 2097: keyboard->keyRelease(KEY_KP1); break;
            case 2098: keyboard->keyRelease(KEY_KP2); break;
            case 2099: keyboard->keyRelease(KEY_KP3); break;
            case 2100: keyboard->keyRelease(KEY_KP4); break;
            case 2101: keyboard->keyRelease(KEY_KP5); break;
            case 2102: keyboard->keyRelease(KEY_KP6); break;
            case 2103: keyboard->keyRelease(KEY_KP7); break;
            case 2104: keyboard->keyRelease(KEY_KP8); break;
            case 2105: keyboard->keyRelease(KEY_KP9); break;
            case 2106: keyboard->keyRelease(KEY_KPASTERISK); break;
            case 2107: keyboard->keyRelease(KEY_KPPLUS); break;
            case 2108: keyboard->keyRelease(KEY_KPCOMMA); break;
            case 2109: keyboard->keyRelease(KEY_KPMINUS); break;
            case 2110: keyboard->keyRelease(KEY_KPDOT); break;
            case 2111: keyboard->keyRelease(KEY_KPSLASH); break;
            case 2112: keyboard->keyRelease(KEY_F1); break;
            case 2113: keyboard->keyRelease(KEY_F2); break;
            case 2114: keyboard->keyRelease(KEY_F3); break;
            case 2115: keyboard->keyRelease(KEY_F4); break;
            case 2116: keyboard->keyRelease(KEY_F5); break;
            case 2117: keyboard->keyRelease(KEY_F6); break;
            case 2118: keyboard->keyRelease(KEY_F7); break;
            case 2119: keyboard->keyRelease(KEY_F8); break;
            case 2120: keyboard->keyRelease(KEY_F9); break;
            case 2121: keyboard->keyRelease(KEY_F10); break;
            case 2122: keyboard->keyRelease(KEY_F11); break;
            case 2123: keyboard->keyRelease(KEY_F12); break;
            case 2124: keyboard->keyRelease(KEY_F13); break;
            case 2125: keyboard->keyRelease(KEY_F14); break;
            case 2126: keyboard->keyRelease(KEY_F15); break;
            case 2127: keyboard->keyRelease(KEY_F16); break;
            case 2128: keyboard->keyRelease(KEY_F17); break;
            case 2129: keyboard->keyRelease(KEY_F18); break;
            case 2130: keyboard->keyRelease(KEY_F19); break;
            case 2131: keyboard->keyRelease(KEY_F20); break;
            case 2132: keyboard->keyRelease(KEY_F21); break;
            case 2133: keyboard->keyRelease(KEY_F22); break;
            case 2134: keyboard->keyRelease(KEY_F23); break;
            case 2135: keyboard->keyRelease(KEY_F24); break;
            case 2144: keyboard->keyRelease(KEY_NUMLOCK); break;
            case 2145: keyboard->keyRelease(KEY_SCROLLLOCK); break;
            case 2160: keyboard->keyRelease(KEY_LEFTSHIFT); break;
            case 2161: keyboard->keyRelease(KEY_RIGHTSHIFT); break;
            case 2162: keyboard->keyRelease(KEY_LEFTCTRL); break;
            case 2163: keyboard->keyRelease(KEY_RIGHTCTRL); break;
            case 2164: keyboard->keyRelease(KEY_LEFTALT); break;
            case 2165: keyboard->keyRelease(KEY_RIGHTALT); break;
            case 2166: keyboard->keyRelease(KEY_MEDIA_WWWBACK); break;
            case 2169: keyboard->keyRelease(KEY_MEDIA_WWWSTOP); break;
            case 2170: keyboard->keyRelease(KEY_MEDIA_WWWSEARCH); break;
            case 2171: keyboard->keyRelease(KEY_MEDIA_WWWFAVORITES); break;
            case 2172: keyboard->keyRelease(KEY_MEDIA_WWWHOME); break;
            case 2173: keyboard->keyRelease( KEY_MUTE); break;
            case 2174: keyboard->keyRelease(KEY_MEDIA_VOLUMEDOWN); break;
            case 2175: keyboard->keyRelease(KEY_MEDIA_VOLUMEUP); break;
            case 2176: keyboard->keyRelease( KEY_MEDIA_NEXTTRACK); break;
            case 2177: keyboard->keyRelease( KEY_MEDIA_PREVIOUSTRACK); break;
            case 2178: keyboard->keyRelease(KEY_MEDIA_STOP); break;
            case 2179: keyboard->keyRelease( KEY_MEDIA_PLAYPAUSE); break;
            case 2180: keyboard->keyRelease( KEY_MEDIA_MAIL); break;
            case 2181: keyboard->keyRelease( KEY_MEDIA_MEDIASELECT); break;
            case 2250: keyboard->keyRelease( KEY_MEDIA_PLAY); break;
            case 2260: pressReleaseMediaKey(KEY_MEDIA_PLAY,false); break;        
            case 2261: pressReleaseMediaKey(KEY_MEDIA_PAUSE,false); break;
            case 2262: pressReleaseMediaKey(KEY_MEDIA_RECORD,false); break;
            case 2263: pressReleaseMediaKey(KEY_MEDIA_FASTFORWARD,false); break;
            case 2264: pressReleaseMediaKey(KEY_MEDIA_REWIND,false); break;
            case 2265: pressReleaseMediaKey(KEY_MEDIA_NEXTTRACK,false); break;
            case 2266: pressReleaseMediaKey(KEY_MEDIA_PREVIOUSTRACK,false); break;
            case 2267: pressReleaseMediaKey(KEY_MEDIA_STOP,false); break;
            case 2268: pressReleaseMediaKey(KEY_MEDIA_EJECT,false); break;
            case 2269: pressReleaseMediaKey(KEY_MEDIA_RANDOMPLAY,false); break;
            case 2270: pressReleaseMediaKey(KEY_MEDIA_REPEAT,false); break;
            case 2271: pressReleaseMediaKey(KEY_MEDIA_PLAYPAUSE,false); break;
            case 2272: pressReleaseMediaKey(KEY_MEDIA_MUTE,false); break;
            case 2273: pressReleaseMediaKey(KEY_MEDIA_VOLUMEUP,false); break;
            case 2274: pressReleaseMediaKey(KEY_MEDIA_VOLUMEDOWN,false); break;
             
        }   
    }
}

void integerToXbox(int value){
  // COMMAND TO SE TRUE OR FALSE BUTTONS OR BUTTON LIKE 
  if(value>=1000 && value<=2999){
        switch(value){
            case 1399: randomInputAllGamepadNoMenu(); break;
            case 2399: releaseAllGamepad(); break;
            case 1390: m_useHardwareJoystick=true;  Serial.println("Hardware Joystick ON"); break;
            case 2390: m_useHardwareJoystick=false;  Serial.println("Hardware Joystick OFF"); break;
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
   else if(value>=1700000000 && value<=1799999999){
      m_binaryBufferOfInteger[33]; // Buffer to store the binary representation (32 bits + null terminator)
      intToBinaryBuffer(value, m_binaryBufferOfInteger, 33);
      Serial.println(m_binaryBufferOfInteger);
      value=value-1700000000;
      intToBinaryBuffer(value,m_binaryBufferOfInteger,33);
      Serial.println(m_binaryBufferOfInteger);
      

      float triggerLeft=0.0;
      float triggerRight=0.0;
      float arrowHorizontal=0;
      float arrowVertical =0;
      if(isIntegerBitRightToLeftTrue(value, 0)) pressA(true);
      else pressA(false);
      if(isIntegerBitRightToLeftTrue(value, 1)) pressX(true);
      else pressX(false);
      if(isIntegerBitRightToLeftTrue(value, 2)) pressB(true);
      else pressB(false);
      if(isIntegerBitRightToLeftTrue(value, 3)) pressY(true);
      else pressY(false);
      if(isIntegerBitRightToLeftTrue(value, 4)) pressLeftSideButton(true);
      else pressLeftSideButton(false);
      if(isIntegerBitRightToLeftTrue(value, 5)) pressRightSideButton(true);
      else pressRightSideButton(false);
      if(isIntegerBitRightToLeftTrue(value, 6)) pressLeftStick(true);
      else pressLeftStick(false);
      if(isIntegerBitRightToLeftTrue(value, 7)) pressRightStick(true);
      else pressRightStick(false);
      if(isIntegerBitRightToLeftTrue(value, 8)) pressMenuLeft(true);
      else pressMenuLeft(false);
      if(isIntegerBitRightToLeftTrue(value, 9)) pressMenuRight(true);
      else pressMenuRight(false);
      if(isIntegerBitRightToLeftTrue(value, 10)) pressHomeXboxButton(true);
      else pressHomeXboxButton(false);
      if(isIntegerBitRightToLeftTrue(value, 11)) arrowVertical+=1; // CLOCK WISE N
      if(isIntegerBitRightToLeftTrue(value, 12)) arrowHorizontal+=1; // CLOCK WISE E
      if(isIntegerBitRightToLeftTrue(value, 13)) arrowVertical+=-1; // CLOCK WISE S
      if(isIntegerBitRightToLeftTrue(value, 14)) arrowHorizontal+=-1; //// CLOCK WISE W
      
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



float turnFrom1To99AsPercent(int value){

    if(value == 0) return 0.0;
    // Turn 1 to 99 to -1.0 to 1.0
    return float((double(value) - 50.0) / 49.0);
}

int binaryTag= 1700000000 ;// 01100101010100111111000100000000

// bool isIntegerBitRightToLeftTrueUsingBinaryTag(int value, int index){
//   //01100101010100111111000100000000
//   bool inBinaryTag= (binaryTag & (1 << index)) ? true: false;
//   bool inValue = (value & (1 << index)) ? true: false;
  
//   if(inBinaryTag) return !inValue;
//   return inValue;
// }
bool isIntegerBitRightToLeftTrue(int value, int index){
  //Don't forget to remove the tag (like 1700000000)
  return (value & (1 << index)) ? true: false;  
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
void intToBinaryBufferLess(int value, int lessValue, char* buffer, size_t size) {
    if (size < 32) {
        return; // Ensure buffer is large enough for 32 bits
    }
    for (int i = 0; i < 32; i++) {
        bool inverse = (binaryTag & (1 << i));
        buffer[31 - i] = (value & (1 << i)) ? (inverse ? '0' : '1') : (inverse ? '1' : '0');
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



/**

       
*/