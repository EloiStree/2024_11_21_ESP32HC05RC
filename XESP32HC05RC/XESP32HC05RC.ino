#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>

#define RXD2 16   // GPIO16 (U2RXD)
#define TXD2 17   // GPIO17 (U2TXD)

int ledPin = 15;

XboxGamepadDevice *gamepad;
BleCompositeHID compositeHID("eLabRC XInput ESP32", "eLabRC", 100);



// RETURN THE RECEIVED BYTE TO THE SENDER
// IT ALLOWS TO SEE IF HC05 BRIDGE IS WORKING PROPERLY
bool m_return_byte_received = false;
// DISPLAY BYTE RECEIVED
bool m_use_print_byte_debug= true;
// DISPLAY ACTION TRIGGERED
bool m_use_print_action_debug= true;

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
void uartCommand(char cLeft, char cRight) {
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

void loop()
{
//test();


  if(Serial.available()){
      byte incomingByte = Serial.read();

      //SEND BACK BYTE TAKE TIME DISABLE IF YOU ARE NOT USING IT
      if(m_return_byte_received){
          Serial.write(incomingByte);
      }
      char c = (char)incomingByte;
      Serial.print("Received: ");
      Serial.println(c);
        cLeft = cRight;
        cRight = c;
        if (isCharDigital(cRight)) {
            uartCommand(cLeft, cRight);
        }

  }

  if (Serial2.available()) {
      // Read the incoming data and send it to the Serial monitor
      byte incomingByte = Serial2.read();

      //SEND BACK BYTE TAKE TIME DISABLE IF YOU ARE NOT USING IT
      if(m_return_byte_received){
          Serial2.write(incomingByte);
      }

      char c = (char)incomingByte;
      Serial.print("Received: ");
      Serial.println(c);
        cLeft = cRight;
        cRight = c;
        if (isCharDigital(cRight)) {
            uartCommand(cLeft, cRight);
        }
    }
    delay(8);
}


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
