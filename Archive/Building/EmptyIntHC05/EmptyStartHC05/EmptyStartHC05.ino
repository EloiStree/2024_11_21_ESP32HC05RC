#include <Arduino.h>
#include <BleGamepad.h>

#define RXD2 9  //17 for serial2
#define TXD2 10 //16 for serial2
#define LED_PIN 2

BleGamepad bleGamepad("eLabRC Gamepad", "eLabRC", 100);

bool is_ble_gamepad_connected = false;


int max_axis_value =32767;
int joystick_axe_x =0;
int joystick_axe_y =0;
int joystick_axe_z =0;
int joystick_axe_rx =0;
int joystick_axe_ry =0;
int joystick_axe_rz =0;
int joystick_axe_slider1 =0;
int joystick_axe_slider2 =0;



void push_report_axes(){
//bleGamepad.setAxes(32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767);       //(X, Y, Z, RX, RY, RZ)
 bleGamepad.setAxes(joystick_axe_x, joystick_axe_y, joystick_axe_z, joystick_axe_rx, joystick_axe_ry, joystick_axe_rz, joystick_axe_slider1, joystick_axe_slider2);
}

int get_random_axe_32767(){
  // Returns a random value between -32767 and 32767
  return random(-32767, 32768);
}


void try_button_16(int button_value){

      bleGamepad.press(button_value);
      delay(100); 
      bleGamepad.release(button_value);
      push_report_axes();
      delay(2000); 

}


void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial1.println("Starting BLE work!");
  bleGamepad.begin();
  Serial1.println("BLE Gamepad started");
}

int parse_bytes_to_integer_little_endian(byte b4, byte b3, byte b2, byte b1) {
  return ((int)b1 << 24) | ((int)b2 << 16) | ((int)b3 << 8) | (int)b4;
}

void parse_integer_to_action(int value_action) {
  switch (value_action) {
    case 1650614882:
    case 1633771873:
      bleGamepad.press(BUTTON_1);
      delay(50); 
      bleGamepad.release(BUTTON_1);
      break;
      
    // BUTTON_1 through to BUTTON_16
    // DPAD_CENTERED, DPAD_UP, DPAD_UP_RIGHT, DPAD_RIGHT, DPAD_DOWN_RIGHT, DPAD_DOWN, DPAD_DOWN_LEFT, DPAD_LEFT, DPAD_UP_LEFT
    // x axis, y axis, z axis, rz axis, ry axis, rz axis, slider 1, slider 2)  <- order HID report is actually given in
    // rudder, throttle, accelerator, brake, steering), but they are not enabled by default.
    // start, select, menu, home, back, volume increase, volume decrease, volume mute)

    case 1630548016:  //000a
      bleGamepad.press(BUTTON_1);
      delay(500); 
      bleGamepad.release(BUTTON_1);
      break;
    case 1630548017:  //000b
      bleGamepad.press(BUTTON_2);
      delay(500); 
      bleGamepad.release(BUTTON_2);
      break;

    case 1633759280:  //a000
        joystick_axe_x =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_y =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_z =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_rx =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_ry =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_rz =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_slider1 =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
        joystick_axe_slider2 =get_random_axe_32767();
        push_report_axes();
        delay(2000); 
      break;

    case 1650536496:  //00ab
      try_button_16(BUTTON_1);
      try_button_16(BUTTON_2);
      try_button_16(BUTTON_3);
      try_button_16(BUTTON_4);
      try_button_16(BUTTON_5);
      try_button_16(BUTTON_6);
      try_button_16(BUTTON_7);
      try_button_16(BUTTON_8);
      try_button_16(BUTTON_9);
      try_button_16(BUTTON_10);
      try_button_16(BUTTON_11);
      try_button_16(BUTTON_12);
      try_button_16(BUTTON_13);
      try_button_16(BUTTON_14);
      try_button_16(BUTTON_15);
      try_button_16(BUTTON_16);
      break;

       case 1667313712:  //00ac
      
       bleGamepad.setHat1(DPAD_CENTERED );  
        delay(2000);       
       bleGamepad.setHat1( DPAD_UP );
        delay(2000); 
       bleGamepad.setHat1(  DPAD_UP_RIGHT );
        delay(2000); 
       bleGamepad.setHat1(   DPAD_RIGHT );
        delay(2000); 
       bleGamepad.setHat1(    DPAD_DOWN_RIGHT );
        delay(2000); 
       bleGamepad.setHat1(     DPAD_DOWN );
        delay(2000); 
       bleGamepad.setHat1(      DPAD_DOWN_LEFT );
        delay(2000); 
       bleGamepad.setHat1(       DPAD_LEFT ); 
        delay(2000); 
       bleGamepad.setHat1(        DPAD_UP_LEFT );
        delay(2000); 
       bleGamepad.setHat1(DPAD_CENTERED );  
        delay(2000);       
      break;


     case 1684090928:  // 0x00AD

        bleGamepad.pressStart();
        delay(2000);
        bleGamepad.releaseStart();
        delay(2000);

        bleGamepad.pressSelect();
        delay(2000);
        bleGamepad.releaseSelect();
        delay(2000);

        bleGamepad.pressMenu();
        delay(2000);
        bleGamepad.releaseMenu();
        delay(2000);

        bleGamepad.pressHome();
        delay(2000);
        bleGamepad.releaseHome();
        delay(2000);

        bleGamepad.pressBack();
        delay(2000);
        bleGamepad.releaseBack();
        delay(2000);

        bleGamepad.pressVolumeDec();
        delay(2000);
        bleGamepad.releaseVolumeDec();
        delay(2000);

        bleGamepad.pressVolumeInc();
        delay(2000);
        bleGamepad.releaseVolumeInc();
        delay(2000);

        bleGamepad.pressVolumeMute();
        delay(2000);
        bleGamepad.releaseVolumeMute();
        delay(2000);

    break;



    default:
      Serial1.print("Unrecognized: ");
      Serial1.println(value_action);
      break;
  }
}

void loop() {
  if (bleGamepad.isConnected()) {
    if (!is_ble_gamepad_connected) {
      is_ble_gamepad_connected = true;
      Serial1.println("BLE Gamepad connected");
    }
  }

  static byte buffer[4];
  static int buf_index = 0;

  while (Serial1.available()) {
    byte incomingByte = Serial1.read();
    buffer[buf_index++] = incomingByte;
    if (buf_index == 4) {
      int little_endian_value = parse_bytes_to_integer_little_endian(buffer[0], buffer[1], buffer[2], buffer[3]);
      parse_integer_to_action(little_endian_value);
      Serial1.println(little_endian_value);
      buf_index = 0;
    }
    delay(1);
  }
  delay(1);
}
