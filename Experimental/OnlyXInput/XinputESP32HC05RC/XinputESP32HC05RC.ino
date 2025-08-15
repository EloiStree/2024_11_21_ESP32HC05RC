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

// This code is designed to allow: learning by code, QA testing in game, and remapping of input
// ESP32:   https://github.com/EloiStree/HelloInput/issues/288
// Context: https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// Learn:   https://github.com/EloiStree/HelloInput

#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>
int ledPin = 2; // LED connected to digital pin 13

#define RXD2 16   // GPIO16 (U2RXD)
#define TXD2 17   // GPIO17 (U2TXD)
int m_lepPin = 2;  // LED pin for vibration feedback
int m_vibrationPin = 3; // Vibration feedback pin
bool m_useHardwareJoystick = false;
int m_pinJosytickLeftVertical = 33;
int m_pinJosytickLeftHorizontal = 32;
int m_pinJosytickRightVertical = 35;
int m_pinJosytickRightHorizontal = 34;

XboxGamepadDevice *gamepad;
BleCompositeHID compositeHID("eLabRC XInput ESP32", "eLabRC", 100);

bool m_return_byte_received = true;
bool m_use_print_byte_debug = true;
bool m_use_print_action_debug = true;

// Input mode handling
int m_readingMode = 2;
int INTEGER_MODE = 0;
int INTEGER_MODE_INT = 1229539657;
int UTF8_MODE = 1;
int UTF8_MODE_INT = 1179010630;
int TEXT_INTEGER_MODE = 2;
int TEXT_INTEGER_MODE_INT = 1717986918;

byte m_blr0 = '0', m_blr1 = '0', m_blr2 = '0', m_blr3 = '0';
char m_clr0 = '0', m_clr1 = '0', m_clr2 = '0', m_clr3 = '0';
int m_intCmd;
String m_textCmd = "";
int m_fourCharIndex = 0;

bool IsInUTF8FourCharMode() { return m_readingMode == 1; }
bool IsInUTF8LineReturnMode() { return m_readingMode == 2; }
bool IsInByteLittleEndianMode() { return m_readingMode == 0; }

void CheckForSwitchModeFromCurrent() {
  if (m_readingMode != TEXT_INTEGER_MODE) {
    if (m_intCmd == 808464432) {
      m_readingMode = 0;
      Serial.println("############## Switch to BYTE INTEGER mode ############## ");
    } else if (m_intCmd == 1179010630) {
      m_readingMode = 1;
      Serial.println("############## Switch to UTF8 mode ############## ");
    } else if (m_intCmd == 1717986918) {
      m_readingMode = 2;
      Serial.println("############## TEXT INTEGER MODE ############## ");
    }
  }
}

void CheckFromSwitchModeFromLineReturn() {
  if (m_readingMode == TEXT_INTEGER_MODE) {
    if (m_textCmd == "IIII") {
      m_readingMode = 0;
      Serial.println("############## Switch to BYTE INTEGER mode ############## ");
    } else if (m_textCmd == "FFFF") {
      m_readingMode = 1;
      Serial.println("############## Switch to UTF8 mode ############## ");
    }
  }
}

int32_t parseLittleEndian(byte b0, byte b1, byte b2, byte b3) {
  return ((int32_t)b0) | ((int32_t)b1 << 8) | ((int32_t)b2 << 16) | ((int32_t)b3 << 24);
}

void intToBytes(int32_t value, uint8_t bytes[4]) {
  bytes[0] = value & 0xFF;
  bytes[1] = (value >> 8) & 0xFF;
  bytes[2] = (value >> 16) & 0xFF;
  bytes[3] = (value >> 24) & 0xFF;
}

void displayFourByteAndChars() {
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

void notifyModeChanged() {
  if (m_readingMode == 0) {
    Serial.println("############## BYTE INTEGER MODE ############## ");
  } else if (m_readingMode == 1) {
    Serial.println("############## UTF8 MODE ############## ");
  } else if (m_readingMode == 2) {
    Serial.println("############## TEXT INTEGER MODE ############## ");
  }
}

void stackByte(byte b) {
  m_blr0 = m_blr1;
  m_blr1 = m_blr2;
  m_blr2 = m_blr3;
  m_blr3 = b;
  m_clr0 = (char)m_blr0;
  m_clr1 = (char)m_blr1;
  m_clr2 = (char)m_blr2;
  m_clr3 = (char)m_blr3;
  m_intCmd = parseLittleEndian(m_blr0, m_blr1, m_blr2, m_blr3);
  m_textCmd = String(m_clr0) + m_clr1 + m_clr2 + m_clr3;

  if (m_blr0 == 70 && m_blr1 == 70 && m_blr2 == 70 && m_blr3 == 70) {
    m_readingMode = UTF8_MODE;
    notifyModeChanged();
  }
  if (m_blr0 == 214 && m_blr1 == 214 && m_blr2 == 214 && m_blr3 == 214) {
    m_readingMode = TEXT_INTEGER_MODE;
    notifyModeChanged();
  }

  m_fourCharIndex++;
  displayFourByteAndChars();
  if (m_fourCharIndex >= 4) {
    m_fourCharIndex = 0;
    CheckForSwitchModeFromCurrent();
    Serial.println("-->> Four byte Received");
    integerCommandReceived(m_intCmd);
  }
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

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(m_lepPin, OUTPUT);
  pinMode(m_vibrationPin, OUTPUT);
  pinMode(m_pinJosytickLeftVertical, INPUT);
  pinMode(m_pinJosytickLeftHorizontal, INPUT);
  pinMode(m_pinJosytickRightVertical, INPUT);
  pinMode(m_pinJosytickRightHorizontal, INPUT);


    XboxOneSControllerDeviceConfiguration* config = new XboxOneSControllerDeviceConfiguration();
    //XboxSeriesXControllerDeviceConfiguration* config = new XboxSeriesXControllerDeviceConfiguration();

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
    FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
    gamepad->onVibrate.attach(vibrationSlot);
    compositeHID.addDevice(gamepad);
    Serial.println("Starting composite HID device...");
    compositeHID.begin(hostConfig);
    releaseAllGamepad();
    Serial.println("Starting BLE work!");
    Serial.println("UART1 initialized.");
}

void integerToCharAndBytesFour(int value) {
  uint8_t bytes[4];
  intToBytes(value, bytes);
  for (int i = 0; i < 4; i++) {
    stackByte(bytes[i]);
  }
}

void serialReceivedString(String input) {
  input.trim();
  if (isNumericString(input)) {
    Serial.print("Int:");
    Serial.println(input);
    int number = input.toInt();
    Serial.print("You entered the number: ");
    Serial.println(number);
    m_intCmd = number;
    integerToCharAndBytesFour(number);
    m_fourCharIndex = 0;
    integerCommandReceived(number);
  } else {
    Serial.print("CMD:");
    Serial.println(input);
    traditionalTextCommand(input);
  }
}

void serialReceivedByte(byte incomingByte) {
  stackByte(incomingByte);
  if (m_return_byte_received) {
    Serial2.write(incomingByte);
  }
}

bool isNumericString(String str) {
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c >= '0' && c <= '9' || c == '\n' || c == '\r' || c == ' ') {
      continue;
    }
    return false;
  }
  return true;
}

void traditionalTextCommand(String text) {
  text.trim();
  Serial.print("CMD:");
  Serial.println(text);

  if (text == "G") pressA(true);
  else if (text == "g") pressA(false);
  else if (text == "Y") pressY(true);
  else if (text == "y") pressY(false);
  else if (text == "B") pressX(true);
  else if (text == "b") pressX(false);
  else if (text == "R") pressB(true);
  else if (text == "r") pressB(false);
  else if (text == "S" || text == "START") pressMenuRight(true);
  else if (text == "s" || text == "start") pressMenuRight(false);
  else if (text == "BACK") pressMenuLeft(true);
  else if (text == "back") pressMenuLeft(false);
  else if (text == "H") pressHomeXboxButton(true);
  else if (text == "h") pressHomeXboxButton(false);
  else if (text == "M") pressMenuLeft(true);
  else if (text == "m") pressMenuLeft(false);
  else if (text == "N") pressMenuRight(true);
  else if (text == "n") pressMenuRight(false);
  else if (text == "BA" || text == "BD") pressA(true);
  else if (text == "ba" || text == "bd") pressA(false);
  else if (text == "BY" || text == "BU") pressY(true);
  else if (text == "by" || text == "bu") pressY(false);
  else if (text == "BX" || text == "BL") pressX(true);
  else if (text == "bx" || text == "bl") pressX(false);
  else if (text == "BB" || text == "BR") pressB(true);
  else if (text == "bb" || text == "br") pressB(false);
  else if (text == "AU") pressArrowN();
  else if (text == "AC") releaseDPad();
  else if (text == "AR") pressArrowE();
  else if (text == "AD") pressArrowS();
  else if (text == "AL") pressArrowW();
  else if (text == "AN") pressArrowN();
  else if (text == "AE") pressArrowE();
  else if (text == "AS") pressArrowS();
  else if (text == "AW") pressArrowW();
  else if (text == "ANW") pressArrowNW();
  else if (text == "ANE") pressArrowNE();
  else if (text == "ASE") pressArrowSE();
  else if (text == "ASW") pressArrowSW();
  else if (text == "RECORD") recordStart();
  else if (text == "record") recordStop();
  else if (text == "SBL") pressLeftSideButton(true);
  else if (text == "sbl") pressLeftSideButton(false);
  else if (text == "SBR") pressRightSideButton(true);
  else if (text == "sbr") pressRightSideButton(false);
  else if (text == "JL") pressLeftStick(true);
  else if (text == "jl") pressLeftStick(false);
  else if (text == "TL") setTriggerLeftPercent(1);
  else if (text == "tl") setTriggerLeftPercent(0);
  else if (text == "TR") setTriggerRightPercent(1);
  else if (text == "tr") setTriggerRightPercent(0);
  else if (text == "JR") pressRightStick(true);
  else if (text == "jr") pressRightStick(false);
  else if (text == "MR") pressMenuRight(true);
  else if (text == "mr") pressMenuRight(false);
  else if (text == "ML") pressMenuLeft(true);
  else if (text == "ml") pressMenuLeft(false);
  else if (text == "MC") pressHomeXboxButton(true);
  else if (text == "mc") pressHomeXboxButton(false);
  else if (text == "C") m_useHardwareJoystick = true;
  else if (text == "c") m_useHardwareJoystick = false;
  else if (text == "V") setVibrationOn(true);
  else if (text == "v") setVibrationOn(false);
  else ParseIfContainsJoystickFromBlueElec(text);
}

void setVibrationOn(bool value) {
  digitalWrite(m_vibrationPin, value ? HIGH : LOW);
}

void ParseIfContainsJoystickFromBlueElec(String text) {
  bool isFound = false;
  float x = 0, y = 0;
  if (text.startsWith("LX")) {
    isFound = true;
    int xIndex = text.indexOf('X') + 1;
    int yIndex = text.indexOf('Y', xIndex) + 1;
    if (xIndex > 0 && yIndex > 0) {
      x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
      y = text.substring(yIndex).toFloat();
      x = ((x - 50.0) / 50.0);
      y = ((y - 50.0) / 50.0);
      setLeftHorizontal(x);
      setLeftVertical(-y);
    }
  } else if (text.startsWith("RX")) {
    isFound = true;
    int xIndex = text.indexOf('X') + 1;
    int yIndex = text.indexOf('Y', xIndex) + 1;
    if (xIndex > 0 && yIndex > 0) {
      x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
      y = text.substring(yIndex).toFloat();
      x = ((x - 50.0) / 50.0);
      y = ((y - 50.0) / 50.0);
      setRightHorizontal(x);
      setRightVertical(-y);
    }
  } else if (text.startsWith("X")) {
    isFound = true;
    int xIndex = text.indexOf('X') + 1;
    int yIndex = text.indexOf('Y', xIndex) + 1;
    if (xIndex > 0 && yIndex > 0) {
      x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
      y = text.substring(yIndex).toFloat();
      x = ((x - 50.0) / 50.0);
      y = ((y - 50.0) / 50.0);
      setLeftHorizontal(x);
      setLeftVertical(-y);
    }
  } else if (text.startsWith("TL")) {
    isFound = true;
    int tlIndex = text.indexOf('L') + 1;
    if (tlIndex > 0) {
      float triggerValue = text.substring(tlIndex).toInt();
      setTriggerLeftPercent(triggerValue / 100.0);
    }
  } else if (text.startsWith("TR")) {
    isFound = true;
    int trIndex = text.indexOf('R') + 1;
    if (trIndex > 0) {
      float triggerValue = text.substring(trIndex).toInt();
      setTriggerRightPercent(triggerValue / 100.0);
    }
  } else if (text.startsWith("A")) {
    isFound = true;
    float sensibilityInDegree = 15.0;
    int startIndex = text.indexOf('A') + 1;
    int commaIndex = text.indexOf(',');
    int starIndex = text.indexOf('*');
    if (startIndex > 0 && commaIndex > startIndex && starIndex > commaIndex) {
      String pitch = text.substring(startIndex, commaIndex);
      String roll = text.substring(commaIndex + 1, starIndex);
      x = (pitch.toFloat() / sensibilityInDegree);
      y = (roll.toFloat() / sensibilityInDegree);
      setLeftHorizontal(x);
      setLeftVertical(-y);
    }
  }
}

void integerCommandReceived(int32_t value) {
  Serial.print("Int:");
  Serial.println(value);
  integerToXbox(value);
}

int m_gamepadReportModulo = 100;
int m_nextReportCount = 0;
int m_nextReadAnalogicCount = 0;
int m_nextReadAnalogicModulo = 50;

float ANALOG_MIN = 0;
float ANALOG_MAX = 65535;

void loop() {
  if (Serial.available() > 0) {
    if (m_readingMode == TEXT_INTEGER_MODE) {
      String input = Serial.readStringUntil('\n');
      serialReceivedString(input);
    } else {
      byte incomingByte = Serial.read();
      serialReceivedByte(incomingByte);
    }
  }
  if (Serial2.available() > 0) {
    if (m_readingMode == TEXT_INTEGER_MODE) {
      String input = Serial2.readStringUntil('\n');
      serialReceivedString(input);
      if (m_return_byte_received) {
        Serial2.println('\n');
      }
    } else {
      byte incomingByte = Serial2.read();
      serialReceivedByte(incomingByte);
    }
  }
  m_nextReportCount--;
  if (m_nextReportCount < 0) {
    m_nextReportCount = m_gamepadReportModulo;
    gamepad->sendGamepadReport();
  }

  if (m_useHardwareJoystick) {
    m_nextReadAnalogicCount--;
    if (m_nextReadAnalogicCount < 0) {
      m_nextReadAnalogicCount = m_nextReadAnalogicModulo;
      int value = analogRead(m_pinJosytickLeftVertical);
      float percentLV = ((value / 4095.0) - 0.5) * 2.0;
      value = analogRead(m_pinJosytickLeftHorizontal);
      float percentLH = ((value / 4095.0) - 0.5) * 2.0;
      value = analogRead(m_pinJosytickRightVertical);
      float percentRV = ((value / 4095.0) - 0.5) * 2.0;
      value = analogRead(m_pinJosytickRightHorizontal);
      float percentRH = ((value / 4095.0) - 0.5) * 2.0;

      if (m_useHardwareJoystick) {
        setLeftVertical(percentLV);
        setLeftHorizontal(percentLH);
        setRightVertical(percentRV);
        setRightHorizontal(percentRH);
      }
    }
  }
  delay(1);
}

// Gamepad Control
uint16_t C_XBOX_BUTTON_A = XBOX_BUTTON_A;
uint16_t C_XBOX_BUTTON_B = XBOX_BUTTON_B;
uint16_t C_XBOX_BUTTON_X = XBOX_BUTTON_X;
uint16_t C_XBOX_BUTTON_Y = XBOX_BUTTON_Y;
uint16_t C_XBOX_BUTTON_LB = XBOX_BUTTON_LB;
uint16_t C_XBOX_BUTTON_RB = XBOX_BUTTON_RB;
uint16_t C_XBOX_BUTTON_START = XBOX_BUTTON_START;
uint16_t C_XBOX_BUTTON_SELECT = XBOX_BUTTON_SELECT;
uint16_t C_XBOX_BUTTON_HOME = XBOX_BUTTON_HOME;
uint16_t C_XBOX_BUTTON_LS = XBOX_BUTTON_LS;
uint16_t C_XBOX_BUTTON_RS = XBOX_BUTTON_RS;

void pressA(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_A);
  else releaseButtonId(C_XBOX_BUTTON_A);
}

void pressB(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_B);
  else releaseButtonId(C_XBOX_BUTTON_B);
}

void pressX(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_X);
  else releaseButtonId(C_XBOX_BUTTON_X);
}

void pressY(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_Y);
  else releaseButtonId(C_XBOX_BUTTON_Y);
}

void pressLeftSideButton(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_LB);
  else releaseButtonId(C_XBOX_BUTTON_LB);
}

void pressRightSideButton(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_RB);
  else releaseButtonId(C_XBOX_BUTTON_RB);
}

void pressLeftStick(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_LS);
  else releaseButtonId(C_XBOX_BUTTON_LS);
}

void pressRightStick(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_RS);
  else releaseButtonId(C_XBOX_BUTTON_RS);
}

void pressMenuRight(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_START);
  else releaseButtonId(C_XBOX_BUTTON_START);
}

void pressMenuLeft(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_SELECT);
  else releaseButtonId(C_XBOX_BUTTON_SELECT);
}

void pressHomeXboxButton(bool isPress) {
  if (isPress) pressButtonId(C_XBOX_BUTTON_HOME);
  else releaseButtonId(C_XBOX_BUTTON_HOME);
}

void pressButtonId(uint16_t value) {
  gamepad->press(value);
  gamepad->sendGamepadReport();
}

void releaseButtonId(uint16_t value) {
  gamepad->release(value);
  gamepad->sendGamepadReport();
}

void recordStart() {
  gamepad->pressShare();
  gamepad->sendGamepadReport();
}

void recordStop() {
  gamepad->releaseShare();
  gamepad->sendGamepadReport();
}

void releaseDPad() {
  gamepad->releaseDPad();
  gamepad->sendGamepadReport();
}

void pressDpad(XboxDpadFlags direction, bool isPress) {
  if (isPress) {
    gamepad->pressDPadDirectionFlag(direction);
    gamepad->sendGamepadReport();
  } else {
    gamepad->releaseDPad();
    gamepad->sendGamepadReport();
  }
}

float m_left_horizontal = 0;
float m_left_vertical = 0;
float m_right_horizontal = 0;
float m_right_vertical = 0;

void setLeftHorizontal(float percent) {
  m_left_horizontal = percent;
  update_sticks();
}

void setLeftVertical(float percent) {
  m_left_vertical = -percent;
  update_sticks();
}

void setRightHorizontal(float percent) {
  m_right_horizontal = percent;
  update_sticks();
}

void setRightVertical(float percent) {
  m_right_vertical = -percent;
  update_sticks();
}

void setTriggerLeftPercent(float percent) {
  gamepad->setLeftTrigger(percent * XBOX_TRIGGER_MAX);
  gamepad->sendGamepadReport();
}

void setTriggerRightPercent(float percent) {
  gamepad->setRightTrigger(percent * XBOX_TRIGGER_MAX);
  gamepad->sendGamepadReport();
}

void pressArrowN() { pressDpad(XboxDpadFlags::NORTH, true); }
void pressArrowE() { pressDpad(XboxDpadFlags::EAST, true); }
void pressArrowS() { pressDpad(XboxDpadFlags::SOUTH, true); }
void pressArrowW() { pressDpad(XboxDpadFlags::WEST, true); }
void pressArrowNE() { pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::NORTH | (uint8_t)XboxDpadFlags::EAST), true); }
void pressArrowNW() { pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::WEST | (uint8_t)XboxDpadFlags::NORTH), true); }
void pressArrowSE() { pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::EAST | (uint8_t)XboxDpadFlags::SOUTH), true); }
void pressArrowSW() { pressDpad(XboxDpadFlags((uint8_t)XboxDpadFlags::SOUTH | (uint8_t)XboxDpadFlags::WEST), true); }

void update_sticks() {
  int16_t lx = m_left_horizontal * XBOX_STICK_MAX;
  int16_t ly = m_left_vertical * XBOX_STICK_MAX;
  int16_t rx = m_right_horizontal * XBOX_STICK_MAX;
  int16_t ry = m_right_vertical * XBOX_STICK_MAX;
  gamepad->setLeftThumb(lx, ly);
  gamepad->setRightThumb(rx, ry);
  gamepad->sendGamepadReport();
}

void integerToXbox(int value) {
  if (value >= 1000 && value <= 2999) {
    switch (value) {
      case 1399: randomInputAllGamepadNoMenu(); break;
      case 2399: releaseAllGamepad(); break;
      case 1390: m_useHardwareJoystick = true; Serial.println("Hardware Joystick ON"); break;
      case 2390: m_useHardwareJoystick = false; Serial.println("Hardware Joystick OFF"); break;
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
      case 1330: setLeftVertical(0); setLeftHorizontal(0); break;
      case 2330: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1331: setLeftVertical(1); setLeftHorizontal(0); break;
      case 2331: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1332: setLeftVertical(1); setLeftHorizontal(1); break;
      case 2332: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1333: setLeftVertical(0); setLeftHorizontal(1); break;
      case 2333: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1334: setLeftVertical(-1); setLeftHorizontal(1); break;
      case 2334: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1335: setLeftVertical(-1); setLeftHorizontal(0); break;
      case 2335: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1336: setLeftVertical(-1); setLeftHorizontal(-1); break;
      case 2336: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1337: setLeftVertical(0); setLeftHorizontal(-1); break;
      case 2337: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1338: setLeftVertical(1); setLeftHorizontal(-1); break;
      case 2338: setLeftVertical(0); setLeftHorizontal(0); break;
      case 1340: setRightVertical(0); setRightHorizontal(0); break;
      case 2340: setRightVertical(0); setRightHorizontal(0); break;
      case 1341: setRightVertical(1); setRightHorizontal(0); break;
      case 2341: setRightVertical(0); setRightHorizontal(0); break;
      case 1342: setRightVertical(1); setRightHorizontal(1); break;
      case 2342: setRightVertical(0); setRightHorizontal(0); break;
      case 1343: setRightVertical(0); setRightHorizontal(1); break;
      case 2343: setRightVertical(0); setRightHorizontal(0); break;
      case 1344: setRightVertical(-1); setRightHorizontal(1); break;
      case 2344: setRightVertical(0); setRightHorizontal(0); break;
      case 1345: setRightVertical(-1); setRightHorizontal(0); break;
      case 2345: setRightVertical(0); setRightHorizontal(0); break;
      case 1346: setRightVertical(-1); setRightHorizontal(-1); break;
      case 2346: setRightVertical(0); setRightHorizontal(0); break;
      case 1347: setRightVertical(0); setRightHorizontal(-1); break;
      case 2347: setRightVertical(0); setRightHorizontal(0); break;
      case 1348: setRightVertical(1); setRightHorizontal(-1); break;
      case 2348: setRightVertical(0); setRightHorizontal(0); break;
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
  } else if (value >= 1800000000 && value <= 1899999999) {
    int leftHorizontalfrom1to99 = (value / 1000000) % 100;
    int leftVerticalfrom1to99 = (value / 10000) % 100;
    int rightHorizontalfrom1to99 = (value / 100) % 100;
    int rightVerticalfrom1to99 = (value / 1) % 100;
    float leftHorizontalPercent = turnFrom1To99AsPercent(leftHorizontalfrom1to99);
    float leftVerticalPercent = turnFrom1To99AsPercent(leftVerticalfrom1to99);
    float rightHorizontalPercent = turnFrom1To99AsPercent(rightHorizontalfrom1to99);
    float rightVerticalPercent = turnFrom1To99AsPercent(rightVerticalfrom1to99);
    setLeftHorizontal(leftHorizontalPercent);
    setLeftVertical(leftVerticalPercent);
    setRightHorizontal(rightHorizontalPercent);
    setRightVertical(rightVerticalPercent);
  } else if (value >= 1700000000 && value <= 1799999999) {
    char m_binaryBufferOfInteger[33];
    intToBinaryBuffer(value, m_binaryBufferOfInteger, 33);
    Serial.println(m_binaryBufferOfInteger);
    value = value - 1700000000;
    intToBinaryBuffer(value, m_binaryBufferOfInteger, 33);
    Serial.println(m_binaryBufferOfInteger);

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
    if (isIntegerBitRightToLeftTrue(value, 11)) arrowVertical += 1;
    if (isIntegerBitRightToLeftTrue(value, 12)) arrowHorizontal += 1;
    if (isIntegerBitRightToLeftTrue(value, 13)) arrowVertical += -1;
    if (isIntegerBitRightToLeftTrue(value, 14)) arrowHorizontal += -1;
    if (isIntegerBitRightToLeftTrue(value, 18)) triggerLeft += 0.25;
    if (isIntegerBitRightToLeftTrue(value, 19)) triggerLeft += 0.25;
    if (isIntegerBitRightToLeftTrue(value, 20)) triggerLeft += 0.5;
    if (isIntegerBitRightToLeftTrue(value, 21)) triggerRight += 0.25;
    if (isIntegerBitRightToLeftTrue(value, 22)) triggerRight += 0.25;
    if (isIntegerBitRightToLeftTrue(value, 23)) triggerRight += 0.5;
    setTriggerLeftPercent(triggerLeft);
    setTriggerRightPercent(triggerRight);

    if (arrowVertical == 1 && arrowHorizontal == 0) pressArrowN();
    else if (arrowVertical == 1 && arrowHorizontal == 1) pressArrowNE();
    else if (arrowVertical == 0 && arrowHorizontal == 1) pressArrowE();
    else if (arrowVertical == -1 && arrowHorizontal == 1) pressArrowSE();
    else if (arrowVertical == -1 && arrowHorizontal == 0) pressArrowS();
    else if (arrowVertical == -1 && arrowHorizontal == -1) pressArrowSW();
    else if (arrowVertical == 0 && arrowHorizontal == -1) pressArrowW();
    else if (arrowVertical == 1 && arrowHorizontal == -1) pressArrowNW();
    else releaseDPad();
  }
}

float turnFrom1To99AsPercent(int value) {
  if (value == 0) return 0.0;
  return float((double(value) - 50.0) / 49.0);
}

int binaryTag = 1700000000;

bool isIntegerBitRightToLeftTrue(int value, int index) {
  return (value & (1 << index)) ? true : false;
}

void intToBinaryBuffer(int value, char* buffer, size_t size) {
  if (size < 32) return;
  for (int i = 0; i < 32; i++) {
    buffer[31 - i] = (value & (1 << i)) ? '1' : '0';
  }
  buffer[32] = '\0';
}

void randomInputAllGamepadNoMenu() {
  pressA(random(0, 2));
  pressB(random(0, 2));
  pressX(random(0, 2));
  pressY(random(0, 2));
  pressLeftSideButton(random(0, 2));
  pressRightSideButton(random(0, 2));
  pressLeftStick(random(0, 2));
  pressRightStick(random(0, 2));
  byte rArrow = random(0, 10);
  switch (rArrow) {
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
  setLeftHorizontal(random(-100, 101) / 100.0);
  setLeftVertical(random(-100, 101) / 100.0);
  setRightHorizontal(random(-100, 101) / 100.0);
  setRightVertical(random(-100, 101) / 100.0);
  setTriggerLeftPercent(random(0, 101) / 100.0);
  setTriggerRightPercent(random(0, 101) / 100.0);
  gamepad->sendGamepadReport();
}

void releaseAllGamepad() {
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

void releaseAxis() {
  setLeftHorizontal(0);
  setLeftVertical(0);
  setRightHorizontal(0);
  setRightVertical(0);
  setTriggerLeftPercent(0);
  setTriggerRightPercent(0);
  gamepad->sendGamepadReport();
}

void randomAxis() {
  setLeftHorizontal(random(-100, 101) / 100.0);
  setLeftVertical(random(-100, 101) / 100.0);
  setRightHorizontal(random(-100, 101) / 100.0);
  setRightVertical(random(-100, 101) / 100.0);
  setTriggerLeftPercent(random(0, 101) / 100.0);
  setTriggerRightPercent(random(0, 101) / 100.0);
  gamepad->sendGamepadReport();
}