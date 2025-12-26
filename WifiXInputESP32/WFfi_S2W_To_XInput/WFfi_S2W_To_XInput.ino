#include <WiFi.h>
#include <WiFiUdp.h>



#include <BleConnectionStatus.h>
#include <BleCompositeHID.h>
#include <XboxGamepadDevice.h>

// Optional: if you use debug prints
// #include "DebugPrintStatic.h"  // Uncomment if you have this

const String WIFI_NAME = "EloiStreeWifi2G";
const String WIFI_PASSWORD = "11234566";
//const char* WIFI_NAME = "Stree_5G";
//const char* WIFI_PASSWORD = "11234566";
const char* ESP32_NAME = "ESP32 XInput";
const char* ESP32_COMPANY = "APIntIO";



class IntAndBinaryUtility{

  public:
    static constexpr  int binary_tag= 1700000000 ;// 01100101010100111111000100000000
    
    static float turn_from_1_to_99_as_percent(int value){

      if(value == 0) return 0.0;
      // Turn 1 to 99 to -1.0 to 1.0
      return float((double(value) - 50.0) / 49.0);
    }

        

    static bool is_integer_bit_right_to_left_true_using_binary_tag(int value, int index){
      //01100101010100111111000100000000
      bool in_binary_tag= (binary_tag & (1 << index)) ? true: false;
      bool in_value = (value & (1 << index)) ? true: false;

      if(in_binary_tag) return !in_value;
      return in_value;
    }

    static bool is_integer_bit_right_to_left_true(int value, int index){
      //Don't forget to remove the tag (like 1700000000)
      return (value & (1 << index)) ? true: false;
    }

    static void int_to_binary_buffer(int value, char* buffer, size_t buffer_size) {
        if (buffer_size < 32) {
            return; // Ensure buffer is large enough for 32 bits
        }
        for (int i = 0; i < 32; i++) {
            buffer[31 - i] = (value & (1 << i)) ? '1' : '0';
        }
        buffer[32] = '\0'; // Null-terminate the string
    }

    static void int_to_binary_buffer_less(int value, int less_value, char* buffer, size_t buffer_size) {
        if (buffer_size < 32) {
            return; // Ensure buffer is large enough for 32 bits
        }
        for (int i = 0; i < 32; i++) {
            bool inverse = (binary_tag & (1 << i));
            buffer[31 - i] = (value & (1 << i)) ? (inverse ? '0' : '1') : (inverse ? '1' : '0');
        }
        buffer[32] = '\0'; // Null-terminate the string
    }
};

class GamepadXboxBLE {
private:
    //XboxOneSControllerDeviceConfiguration* gamepadConfig  = nullptr;
    XboxSeriesXControllerDeviceConfiguration* gamepadConfig = nullptr;
    XboxGamepadDevice* gamepad = nullptr;
    BleCompositeHID compositeHID;

    // Axis and trigger limits (constexpr for compile-time constants)
    static constexpr int16_t AXIS_MIN = -32767;
    static constexpr int16_t AXIS_MAX = 32767;
    static constexpr int16_t AXIS_MAX_PLUS_ONE = 32768;
    static constexpr int16_t AXIS_NEUTRAL = 0;

    static constexpr int16_t TRIGGER_MIN = 0;
    static constexpr int16_t TRIGGER_MAX = 65535;


    int16_t m_left_horizontal = 0.0f;
    int16_t m_left_vertical = 0.0f;
    int16_t m_right_horizontal = 0.0f;
    int16_t m_right_vertical = 0.0f;
    uint8_t m_trigger_left = 0.0f;
    uint8_t m_trigger_right = 0.0f;

    

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

public:
    // Constructor now takes name/device info for flexibility
    GamepadXboxBLE(
        const char* deviceName = ESP32_NAME,
        const char* manufacturer = ESP32_COMPANY,
        uint8_t batteryLevel = 100
    ) : compositeHID(deviceName, manufacturer, batteryLevel) {}

    ~GamepadXboxBLE() {
        if (gamepadConfig) delete gamepadConfig;
        if (gamepad) delete gamepad;
    }

    bool begin() {
        
        Serial.println("[XboxBLE] Initializing BLE Xbox Gamepad...");

        // Create configuration
        
        //gamepadConfig = new XboxOneSControllerDeviceConfiguration();
        gamepadConfig = new XboxSeriesXControllerDeviceConfiguration();
        if (!gamepadConfig) {
            Serial.println("[ERROR] Failed to allocate gamepad config!");
            return false;
        }

        // Get ideal host config (includes VID/PID, etc.)
        BLEHostConfiguration hostConfig = gamepadConfig->getIdealHostConfiguration();

        Serial.printf("[INFO] VID: 0x%04X  PID: 0x%04X  GUID: 0x%02X  Serial: %s\n",
                hostConfig.getVid(),
                hostConfig.getPid(),
                hostConfig.getGuidVersion(),
                hostConfig.getSerialNumber());

        // Create the actual gamepad device
        gamepad = new XboxGamepadDevice(gamepadConfig);
        if (!gamepad) {
            Serial.println("[ERROR] Failed to create XboxGamepadDevice!");
            return false;
        }

        // Add to composite HID
        compositeHID.addDevice(gamepad);

        // Start BLE
        compositeHID.begin(hostConfig);

        // Ensure clean state
        release_all();

        Serial.println("[XboxBLE] Ready! Waiting for connection...");
        return true;
    }

  



      void press_button_id(uint16_t value){
              gamepad->press(value);
              gamepad->sendGamepadReport();
      }

      void release_button_id(uint16_t value){
              gamepad->release(value);
              gamepad->sendGamepadReport();
      }

      void press_a(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_A);}else{release_button_id(C_XBOX_BUTTON_A);} }
      void press_b(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_B);}else{release_button_id(C_XBOX_BUTTON_B);}}
      void press_x(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_X);}else{release_button_id(C_XBOX_BUTTON_X);} }
      void press_y(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_Y);}else{release_button_id(C_XBOX_BUTTON_Y);} }
      void press_left_side_button(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_LB);}else{release_button_id(C_XBOX_BUTTON_LB);}}
      void press_right_side_button(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_RB);}else{release_button_id(C_XBOX_BUTTON_RB);}}
      void press_left_stick(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_LS);}else{release_button_id(C_XBOX_BUTTON_LS);}}
      void press_right_stick(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_RS);}else{release_button_id(C_XBOX_BUTTON_RS);}}
      void press_menu_right(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_START);}else{release_button_id(C_XBOX_BUTTON_START);}}
      void press_menu_left(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_SELECT);}else{release_button_id(C_XBOX_BUTTON_SELECT);}}
      void press_home_xbox_button(bool isPress){if(isPress){press_button_id(C_XBOX_BUTTON_HOME);}else{release_button_id(C_XBOX_BUTTON_HOME);}}



      void record_start(){
          gamepad->pressShare();
          gamepad->sendGamepadReport();
      }
      void record_stop(){
          gamepad->releaseShare();
          gamepad->sendGamepadReport();
      }
      void release_dpad(){
              gamepad->releaseDPad();
              gamepad->sendGamepadReport();
      }
      


    void set_trigger_left_percent(float percent){ 
      gamepad->setLeftTrigger(percent*TRIGGER_MAX); 
      gamepad->sendGamepadReport(); 
    }
    void set_trigger_right_percent(float percent){
       gamepad->setRightTrigger(percent*TRIGGER_MAX);
        gamepad->sendGamepadReport();
      }

    void press_dpad(XboxDpadFlags direction , bool isPress){
        if(isPress){
              gamepad->pressDPadDirectionFlag(direction);
              gamepad->sendGamepadReport();
        }
        else{

              gamepad->releaseDPad();
              gamepad->sendGamepadReport();
        }
    }
    void press_arrow_n(){press_dpad(XboxDpadFlags::NORTH,true);}
    void press_arrow_e(){press_dpad(XboxDpadFlags::EAST,true);}
    void press_arrow_s(){press_dpad(XboxDpadFlags::SOUTH,true);}
    void press_arrow_w(){press_dpad(XboxDpadFlags::WEST,true);}
    void press_arrow_ne(){press_dpad(XboxDpadFlags((uint8_t)XboxDpadFlags::NORTH | (uint8_t)XboxDpadFlags::EAST),true);}
    void press_arrow_nw(){press_dpad(XboxDpadFlags((uint8_t)XboxDpadFlags::WEST | (uint8_t)XboxDpadFlags::NORTH),true);}
    void press_arrow_se(){press_dpad(XboxDpadFlags((uint8_t)XboxDpadFlags::EAST | (uint8_t)XboxDpadFlags::SOUTH),true);}
    void press_arrow_sw(){press_dpad(XboxDpadFlags((uint8_t)XboxDpadFlags::SOUTH | (uint8_t)XboxDpadFlags::WEST),true);}

    void set_left_vertical_percent(float percent) {
      if (!gamepad) return;
      percent*=-1.0;
      int16_t value = (int16_t)(percent * AXIS_MAX);
      // Clamp to valid range
      if (value < AXIS_MIN) value = AXIS_MIN;
      if (value > AXIS_MAX) value = AXIS_MAX;

      m_left_vertical = value;
      gamepad->setLeftThumb(m_left_horizontal, m_left_vertical);
      send_report();
    }

    void set_left_horizontal_percent(float percent) {
      if (!gamepad) return;
      int16_t value = (int16_t)(percent * AXIS_MAX);
      // Clamp to valid range
      if (value < AXIS_MIN) value = AXIS_MIN;
      if (value > AXIS_MAX) value = AXIS_MAX;
      
      m_left_horizontal = value;
      gamepad->setLeftThumb(m_left_horizontal, m_left_vertical);
      send_report();
    }

    void set_right_vertical_percent(float percent) {
      if (!gamepad) return;
      percent*=-1.0;
      int16_t value = (int16_t)(percent * AXIS_MAX);
      // Clamp to valid range
      if (value < AXIS_MIN) value = AXIS_MIN;
      if (value > AXIS_MAX) value = AXIS_MAX;
      
      m_right_vertical = value;
      gamepad->setRightThumb(m_right_horizontal, m_right_vertical);
      send_report();
    }

    void set_right_horizontal_percent(float percent) {
      if (!gamepad) return;
      int16_t value = (int16_t)(percent * AXIS_MAX);
      // Clamp to valid range
      if (value < AXIS_MIN) value = AXIS_MIN;
      if (value > AXIS_MAX) value = AXIS_MAX;
      
      m_right_horizontal = value;
      gamepad->setRightThumb(m_right_horizontal, m_right_vertical);
      send_report();
    }

    // Randomize all axes and triggers
    void set_axis_randomly() {
        int16_t lx = random(AXIS_MIN, AXIS_MAX_PLUS_ONE);
        int16_t ly = random(AXIS_MIN, AXIS_MAX_PLUS_ONE);
        int16_t rx = random(AXIS_MIN, AXIS_MAX_PLUS_ONE);
        int16_t ry = random(AXIS_MIN, AXIS_MAX_PLUS_ONE);

        uint8_t lt = random(TRIGGER_MIN, TRIGGER_MAX);
        uint8_t rt = random(TRIGGER_MIN, TRIGGER_MAX);

        gamepad->setLeftThumb(lx, ly);
        gamepad->setRightThumb(rx, ry);
        gamepad->setLeftTrigger(lt);
        gamepad->setRightTrigger(rt);

        send_report();
    }
 
    // Center all sticks, zero triggers
    void set_axis_to_neutral() {
        gamepad->setLeftThumb(AXIS_NEUTRAL, AXIS_NEUTRAL);
        gamepad->setRightThumb(AXIS_NEUTRAL, AXIS_NEUTRAL);
        gamepad->setLeftTrigger(TRIGGER_MIN);
        gamepad->setRightTrigger(TRIGGER_MIN);

        send_report();
    }

    // Release everything (buttons + axes)
    void release_all() {
        if (!gamepad) return;
    
        // gamepad->releaseAll();
        // setNeutralAxes();  
        send_report();
    }

    // Check connection status
    // bool isConnected() const {
    //     return compositeHID.isConnected();
    // }


    


    void random_input_all_gamepad_no_menu(){

            press_a(random(0,2));
            press_b(random(0,2));
            press_x(random(0,2));
            press_y(random(0,2));
            press_left_side_button(random(0,2));
            press_right_side_button(random(0,2));
            press_left_stick(random(0,2));
            press_right_stick(random(0,2));
            byte rArrow = random(0,10);
            switch(rArrow){
                case 0: press_arrow_n(); break;
                case 1: press_arrow_ne(); break;
                case 2: press_arrow_e(); break;
                case 3: press_arrow_se(); break;
                case 4: press_arrow_s(); break;
                case 5: press_arrow_sw(); break;
                case 6: press_arrow_w(); break;
                case 7: press_arrow_nw(); break;
                default: release_dpad(); break;
            }
            set_left_horizontal_percent(random(-100,101)/100.0);
            set_left_vertical_percent(random(-100,101)/100.0);
            set_right_horizontal_percent(random(-100,101)/100.0);
            set_right_vertical_percent(random(-100,101)/100.0);
            set_trigger_left_percent(random(0,101)/100.0);
            set_trigger_right_percent(random(0,101)/100.0);

            gamepad->sendGamepadReport();
    }
    void release_all_gamepad(){

            set_left_horizontal_percent(0);
            set_left_vertical_percent(0);
            set_right_horizontal_percent(0);
            set_right_vertical_percent(0);
            set_trigger_left_percent(0);
            set_trigger_right_percent(0);
            press_a(false);
            press_b(false);
            press_x(false);
            press_y(false);
            press_left_side_button(false);
            press_right_side_button(false);
            press_left_stick(false);
            press_right_stick(false);
            press_menu_right(false);
            press_menu_left(false);
            press_home_xbox_button(false);
            release_dpad();
            gamepad->sendGamepadReport();
    }

    void release_axis(){
            set_left_horizontal_percent(0);
            set_left_vertical_percent(0);
            set_right_horizontal_percent(0);
            set_right_vertical_percent(0);
            set_trigger_left_percent(0);
            set_trigger_right_percent(0);
            gamepad->sendGamepadReport();
    }
    void random_axis(){
            set_left_horizontal_percent(random(-100,101)/100.0);
            set_left_vertical_percent(random(-100,101)/100.0);
            set_right_horizontal_percent(random(-100,101)/100.0);
            set_right_vertical_percent(random(-100,101)/100.0);
            set_trigger_left_percent(random(0,101)/100.0);
            set_trigger_right_percent(random(0,101)/100.0);
            gamepad->sendGamepadReport();
    }


private:
    void send_report() {
        if (gamepad) {
            gamepad->sendGamepadReport();
        }
    }
};



class IIDUtility {
public:
  static int32_t byte_to_little_endian_int(byte b0, byte b1, byte b2, byte b3) {
    return ((int32_t)b0) | ((int32_t)b1 << 8) | ((int32_t)b2 << 16) | ((int32_t)b3 << 24);
  }

  static uint64_t bytes_to_little_endian_uint64(byte b0, byte b1, byte b2, byte b3, byte b4, byte b5, byte b6, byte b7) {
    return ((uint64_t)b0) | ((uint64_t)b1 << 8) | ((uint64_t)b2 << 16) | ((uint64_t)b3 << 24) | ((uint64_t)b4 << 32) | ((uint64_t)b5 << 40) | ((uint64_t)b6 << 48) | ((uint64_t)b7 << 56);
  }

  static void little_endian_int_to_bytes(int32_t value, uint8_t bytes[4]) {
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
  }

  static void little_endian_uint64_to_bytes(uint64_t value, uint8_t bytes[8]) {
    bytes[0] = value & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
    bytes[4] = (value >> 32) & 0xFF;
    bytes[5] = (value >> 40) & 0xFF;
    bytes[6] = (value >> 48) & 0xFF;
    bytes[7] = (value >> 56) & 0xFF;
  }
};

/// @brief Allows to switch off print when code is ready.
class DebugPrintStatic {
public:
  static void debug_print(String text) {
    if (m_use_print_debug)
      Serial.print(text);
  }
  static void debug_println(String text) {
    if (m_use_print_debug)
      Serial.println(text);
  }

  static bool is_using_debug_print() {
    return m_use_print_debug;
  }
  static bool m_use_print_debug;
};

bool DebugPrintStatic::m_use_print_debug = true;




class GamepadXboxIntegerWrapper {
public:
  GamepadXboxBLE* gamepad() {
    return m_gamepad;
  }
  GamepadXboxBLE* m_gamepad = nullptr;
  void set_gamepad(GamepadXboxBLE* gp) {
    m_gamepad = gp;
  }

  static constexpr int32_t COMMAND_PRESS_A = 1300;
  static constexpr int32_t COMMAND_RELEASE_A = 2300;
  static constexpr int32_t COMMAND_PRESS_X = 1301;
  static constexpr int32_t COMMAND_RELEASE_X = 2301;
  static constexpr int32_t COMMAND_PRESS_B = 1302;
  static constexpr int32_t COMMAND_RELEASE_B = 2302;
  static constexpr int32_t COMMAND_PRESS_Y = 1303;
  static constexpr int32_t COMMAND_RELEASE_Y = 2303;

  static constexpr int32_t COMMAND_PRESS_LEFT_SIDE_BUTTON = 1304;
  static constexpr int32_t COMMAND_RELEASE_LEFT_SIDE_BUTTON = 2304;
  static constexpr int32_t COMMAND_PRESS_RIGHT_SIDE_BUTTON = 1305;
  static constexpr int32_t COMMAND_RELEASE_RIGHT_SIDE_BUTTON = 2305;

  static constexpr int32_t COMMAND_PRESS_LEFT_STICK = 1306;
  static constexpr int32_t COMMAND_RELEASE_LEFT_STICK = 2306;
  static constexpr int32_t COMMAND_PRESS_RIGHT_STICK = 1307;
  static constexpr int32_t COMMAND_RELEASE_RIGHT_STICK = 2307;

  static constexpr int32_t COMMAND_PRESS_MENU_RIGHT = 1308;
  static constexpr int32_t COMMAND_RELEASE_MENU_RIGHT = 2308;
  static constexpr int32_t COMMAND_PRESS_MENU_LEFT = 1309;
  static constexpr int32_t COMMAND_RELEASE_MENU_LEFT = 2309;
  static constexpr int32_t COMMAND_RELEASE_DPAD_A = 1310;
  static constexpr int32_t COMMAND_RELEASE_DPAD_B = 2310;

  static constexpr int32_t COMMAND_PRESS_DPAD_NORTH = 1311;
  static constexpr int32_t COMMAND_RELEASE_DPAD_NORTH = 2311;
  static constexpr int32_t COMMAND_PRESS_DPAD_NORTHEAST = 1312;
  static constexpr int32_t COMMAND_RELEASE_DPAD_NORTHEAST = 2312;
  static constexpr int32_t COMMAND_PRESS_DPAD_EAST = 1313;
  static constexpr int32_t COMMAND_RELEASE_DPAD_EAST = 2313;
  static constexpr int32_t COMMAND_PRESS_DPAD_SOUTHEAST = 1314;
  static constexpr int32_t COMMAND_RELEASE_DPAD_SOUTHEAST = 2314;
  static constexpr int32_t COMMAND_PRESS_DPAD_SOUTH = 1315;
  static constexpr int32_t COMMAND_RELEASE_DPAD_SOUTH = 2315;
  static constexpr int32_t COMMAND_PRESS_DPAD_SOUTHWEST = 1316;
  static constexpr int32_t COMMAND_RELEASE_DPAD_SOUTHWEST = 2316;
  static constexpr int32_t COMMAND_PRESS_DPAD_WEST = 1317;
  static constexpr int32_t COMMAND_RELEASE_DPAD_WEST = 2317;
  static constexpr int32_t COMMAND_PRESS_DPAD_NORTHWEST = 1318;
  static constexpr int32_t COMMAND_RELEASE_DPAD_NORTHWEST = 2318;

  static constexpr int32_t COMMAND_PRESS_HOME_XBOX_BUTTON = 1319;
  static constexpr int32_t COMMAND_RELEASE_HOME_XBOX_BUTTON = 2319;

  static constexpr int32_t COMMAND_PRESS_RANDOM_AXIS = 1320;
  static constexpr int32_t COMMAND_RELEASE_RANDOM_AXIS = 2320;

  static constexpr int32_t COMMAND_START_RECORDING = 1321;
  static constexpr int32_t COMMAND_STOP_RECORDING = 2321;

  char m_binaryBufferOfInteger[33];
  void parse_integer(int32_t value) {

    if(value>=1000 && value<=2999){
      switch (value) {
        case COMMAND_PRESS_A: gamepad()->press_a(true); break;
        case COMMAND_RELEASE_A: gamepad()->press_a(false); break;
        case COMMAND_PRESS_X: gamepad()->press_x(true); break;
        case COMMAND_RELEASE_X: gamepad()->press_x(false); break;
        case COMMAND_PRESS_B: gamepad()->press_b(true); break;
        case COMMAND_RELEASE_B: gamepad()->press_b(false); break;
        case COMMAND_PRESS_Y: gamepad()->press_y(true); break;
        case COMMAND_RELEASE_Y: gamepad()->press_y(false); break;
        
        case COMMAND_PRESS_LEFT_SIDE_BUTTON: gamepad()->press_left_side_button(true); break;
        case COMMAND_RELEASE_LEFT_SIDE_BUTTON: gamepad()->press_left_side_button(false); break;
        case COMMAND_PRESS_RIGHT_SIDE_BUTTON: gamepad()->press_right_side_button(true); break;
        case COMMAND_RELEASE_RIGHT_SIDE_BUTTON: gamepad()->press_right_side_button(false); break;
        
        case COMMAND_PRESS_LEFT_STICK: gamepad()->press_left_stick(true); break;
        case COMMAND_RELEASE_LEFT_STICK: gamepad()->press_left_stick(false); break;
        case COMMAND_PRESS_RIGHT_STICK: gamepad()->press_right_stick(true); break;
        case COMMAND_RELEASE_RIGHT_STICK: gamepad()->press_right_stick(false); break;
        
        case COMMAND_PRESS_MENU_RIGHT: gamepad()->press_menu_right(true); break;
        case COMMAND_RELEASE_MENU_RIGHT: gamepad()->press_menu_right(false); break;
        case COMMAND_PRESS_MENU_LEFT: gamepad()->press_menu_left(true); break;
        case COMMAND_RELEASE_MENU_LEFT: gamepad()->press_menu_left(false); break;


        case COMMAND_PRESS_RANDOM_AXIS: gamepad()->set_axis_randomly(); break;
        case COMMAND_RELEASE_RANDOM_AXIS: gamepad()->set_axis_to_neutral(); break;
        
        case 1330: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 2330: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1331: gamepad()->set_left_vertical_percent(1);  gamepad()->set_left_horizontal_percent(0);     break;
        case 2331: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1332: gamepad()->set_left_vertical_percent(1);  gamepad()->set_left_horizontal_percent(1);     break;
        case 2332: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1333: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(1);     break;
        case 2333: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1334: gamepad()->set_left_vertical_percent(-1); gamepad()->set_left_horizontal_percent(1);    break;
        case 2334: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1335: gamepad()->set_left_vertical_percent(-1); gamepad()->set_left_horizontal_percent(0);    break;
        case 2335: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1336: gamepad()->set_left_vertical_percent(-1); gamepad()->set_left_horizontal_percent(-1);   break;
        case 2336: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1337: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(-1);    break;
        case 2337: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1338: gamepad()->set_left_vertical_percent(1);   gamepad()->set_left_horizontal_percent(-1);    break;
        case 2338: gamepad()->set_left_vertical_percent(0);  gamepad()->set_left_horizontal_percent(0);     break;
        case 1340: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 2340: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1341: gamepad()->set_right_vertical_percent(1); gamepad()->set_right_horizontal_percent(0);    break;
        case 2341: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1342: gamepad()->set_right_vertical_percent(1); gamepad()->set_right_horizontal_percent(1);    break;
        case 2342: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1343: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(1);    break;
        case 2343: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1344: gamepad()->set_right_vertical_percent(-1); gamepad()->set_right_horizontal_percent(1);   break;
        case 2344: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1345: gamepad()->set_right_vertical_percent(-1); gamepad()->set_right_horizontal_percent(0);   break;
        case 2345: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1346: gamepad()->set_right_vertical_percent(-1); gamepad()->set_right_horizontal_percent(-1);  break;
        case 2346: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1347: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(-1);   break;
        case 2347: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;
        case 1348: gamepad()->set_right_vertical_percent(1); gamepad()->set_right_horizontal_percent(-1);   break;
        case 2348: gamepad()->set_right_vertical_percent(0); gamepad()->set_right_horizontal_percent(0);    break;




        case 1310: gamepad()->release_dpad(); break;
        case 2310: gamepad()->release_dpad(); break;
        case 1311: gamepad()->press_arrow_n(); break;
        case 2311: gamepad()->release_dpad(); break;
        case 1312: gamepad()->press_arrow_ne(); break;
        case 2312: gamepad()->release_dpad(); break;
        case 1313: gamepad()->press_arrow_e(); break;
        case 2313: gamepad()->release_dpad(); break;
        case 1314: gamepad()->press_arrow_se(); break;
        case 2314: gamepad()->release_dpad(); break;
        case 1315: gamepad()->press_arrow_s(); break;
        case 2315: gamepad()->release_dpad(); break;
        case 1316: gamepad()->press_arrow_sw(); break;
        case 2316: gamepad()->release_dpad(); break;
        case 1317: gamepad()->press_arrow_w(); break;
        case 2317: gamepad()->release_dpad(); break;
        case 1318: gamepad()->press_arrow_nw(); break;
        case 2318: gamepad()->release_dpad(); break;
        case 1319: gamepad()->press_home_xbox_button(true); break;
        case 2319: gamepad()->press_home_xbox_button(false); break;
  
        case 1321: gamepad()->record_start(); break;
        case 2321: gamepad()->record_stop(); break;

              case 1350: gamepad()->set_left_horizontal_percent(1); break;
              case 2350: gamepad()->set_left_horizontal_percent(0); break;
              case 1351: gamepad()->set_left_horizontal_percent(-1); break;
              case 2351: gamepad()->set_left_horizontal_percent(0); break;
              case 1352: gamepad()->set_left_vertical_percent(1); break;
              case 2352: gamepad()->set_left_vertical_percent(0); break;
              case 1353: gamepad()->set_left_vertical_percent(-1); break;
              case 2353: gamepad()->set_left_vertical_percent(0); break;
              case 1354: gamepad()->set_right_horizontal_percent(1); break;
              case 2354: gamepad()->set_right_horizontal_percent(0); break;
              case 1355: gamepad()->set_right_horizontal_percent(-1); break;
              case 2355: gamepad()->set_right_horizontal_percent(0); break;
              case 1356: gamepad()->set_right_vertical_percent(1); break;
              case 2356: gamepad()->set_right_vertical_percent(0); break;
              case 1357: gamepad()->set_right_vertical_percent(-1); break;
              case 2357: gamepad()->set_right_vertical_percent(0); break;
              case 1358: gamepad()->set_trigger_left_percent(1); break;
              case 2358: gamepad()->set_trigger_left_percent(0); break;
              case 1359: gamepad()->set_trigger_right_percent(1); break;
              case 2359: gamepad()->set_trigger_right_percent(0); break;
              case 1360: gamepad()->set_left_horizontal_percent(0.75); break;
              case 2360: gamepad()->set_left_horizontal_percent(0); break;
              case 1361: gamepad()->set_left_horizontal_percent(-0.75); break;
              case 2361: gamepad()->set_left_horizontal_percent(0); break;
              case 1362: gamepad()->set_left_vertical_percent(0.75); break;
              case 2362: gamepad()->set_left_vertical_percent(0); break;
              case 1363: gamepad()->set_left_vertical_percent(-0.75); break;
              case 2363: gamepad()->set_left_vertical_percent(0); break;
              case 1364: gamepad()->set_right_horizontal_percent(0.75); break;
              case 2364: gamepad()->set_right_horizontal_percent(0); break;
              case 1365: gamepad()->set_right_horizontal_percent(-0.75); break;
              case 2365: gamepad()->set_right_horizontal_percent(0); break;
              case 1366: gamepad()->set_right_vertical_percent(0.75); break;
              case 2366: gamepad()->set_right_vertical_percent(0); break;
              case 1367: gamepad()->set_right_vertical_percent(-0.75); break;
              case 2367: gamepad()->set_right_vertical_percent(0); break;
              case 1368: gamepad()->set_trigger_left_percent(0.75); break;
              case 2368: gamepad()->set_trigger_left_percent(0); break;
              case 1369: gamepad()->set_trigger_right_percent(0.75); break;
              case 2369: gamepad()->set_trigger_right_percent(0); break;
              case 1370: gamepad()->set_left_horizontal_percent(0.5); break;
              case 2370: gamepad()->set_left_horizontal_percent(0); break;
              case 1371: gamepad()->set_left_horizontal_percent(-0.5); break;
              case 2371: gamepad()->set_left_horizontal_percent(0); break;
              case 1372: gamepad()->set_left_vertical_percent(0.5); break;
              case 2372: gamepad()->set_left_vertical_percent(0); break;
              case 1373: gamepad()->set_left_vertical_percent(-0.5); break;
              case 2373: gamepad()->set_left_vertical_percent(0); break;
              case 1374: gamepad()->set_right_horizontal_percent(0.5); break;
              case 2374: gamepad()->set_right_horizontal_percent(0); break;
              case 1375: gamepad()->set_right_horizontal_percent(-0.5); break;
              case 2375: gamepad()->set_right_horizontal_percent(0); break;
              case 1376: gamepad()->set_right_vertical_percent(0.5); break;
              case 2376: gamepad()->set_right_vertical_percent(0); break;
              case 1377: gamepad()->set_right_vertical_percent(-0.5); break;
              case 2377: gamepad()->set_right_vertical_percent(0); break;
              case 1378: gamepad()->set_trigger_left_percent(0.5); break;
              case 2378: gamepad()->set_trigger_left_percent(0); break;
              case 1379: gamepad()->set_trigger_right_percent(0.5); break;
              case 2379: gamepad()->set_trigger_right_percent(0); break;
              case 1380: gamepad()->set_left_horizontal_percent(0.25); break;
              case 2380: gamepad()->set_left_horizontal_percent(0); break;
              case 1381: gamepad()->set_left_horizontal_percent(-0.25); break;
              case 2381: gamepad()->set_left_horizontal_percent(0); break;
              case 1382: gamepad()->set_left_vertical_percent(0.25); break;
              case 2382: gamepad()->set_left_vertical_percent(0); break;
              case 1383: gamepad()->set_left_vertical_percent(-0.25); break;
              case 2383: gamepad()->set_left_vertical_percent(0); break;
              case 1384: gamepad()->set_right_horizontal_percent(0.25); break;
              case 2384: gamepad()->set_right_horizontal_percent(0); break;
              case 1385: gamepad()->set_right_horizontal_percent(-0.25); break;
              case 2385: gamepad()->set_right_horizontal_percent(0); break;
              case 1386: gamepad()->set_right_vertical_percent(0.25); break;
              case 2386: gamepad()->set_right_vertical_percent(0); break;
              case 1387: gamepad()->set_right_vertical_percent(-0.25); break;
              case 2387: gamepad()->set_right_vertical_percent(0); break;
              case 1388: gamepad()->set_trigger_left_percent(0.25); break;
              case 2388: gamepad()->set_trigger_left_percent(0); break;
              case 1389: gamepad()->set_trigger_right_percent(0.25); break;
              case 2389: gamepad()->set_trigger_right_percent(0); break;
              
//             case 1399: randomInputAllGamepadNoMenu(); break;
//             case 2399: releaseAllGamepad(); break;

        default:
          break;
      }
    }
    else if(value>=1800000000 && value<=1899999999){

      //18 50 20 00 10
      //1850200010
      //4 bytes because integer
      int left_horizontal_from_1_to_99 =   (value/1000000)%100;
      int left_vertical_from_1_to_99 =     (value/10000)%100;
      int right_horizontal_from_1_to_99 =  (value/100)%100;
      int right_vertical_from_1_to_99 =    (value/1)%100;
      float left_horizontal_percent= IntAndBinaryUtility::turn_from_1_to_99_as_percent(left_horizontal_from_1_to_99);
      float left_vertical_percent= IntAndBinaryUtility::turn_from_1_to_99_as_percent(left_vertical_from_1_to_99);
      float right_horizontal_percent= IntAndBinaryUtility::turn_from_1_to_99_as_percent(right_horizontal_from_1_to_99);
      float right_vertical_percent= IntAndBinaryUtility::turn_from_1_to_99_as_percent(right_vertical_from_1_to_99);
      gamepad()->set_left_horizontal_percent(left_horizontal_percent);
      gamepad()->set_left_vertical_percent(left_vertical_percent);
      gamepad()->set_right_horizontal_percent(right_horizontal_percent);
      gamepad()->set_right_vertical_percent(right_vertical_percent );
    }
    else if(value>=1700000000 && value<=1799999999){
            m_binaryBufferOfInteger[33]; // Buffer to store the binary representation (32 bits + null terminator)
            //IntAndBinaryUtility::int_to_binary_buffer(value, m_binaryBufferOfInteger, 33);
            //Serial.println(m_binaryBufferOfInteger);
            value=value-1700000000;
            IntAndBinaryUtility::int_to_binary_buffer(value,m_binaryBufferOfInteger,33);
            //Serial.println(m_binaryBufferOfInteger);


            float triggerLeft=0.0;
            float triggerRight=0.0;
            float arrowHorizontal=0;
            float arrowVertical =0;
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 0)) gamepad()->press_y(true); else gamepad()->press_y(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 1)) gamepad()->press_b(true); else gamepad()->press_b(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 2)) gamepad()->press_a(true); else gamepad()->press_a(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 3)) gamepad()->press_x(true); else gamepad()->press_x(false);

            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 4)) arrowVertical+=1; // CLOCK WISE N
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 5)) arrowHorizontal+=1; // CLOCK WISE E
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 6)) arrowVertical+=-1; // CLOCK WISE S
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 7)) arrowHorizontal+=-1; //// CLOCK WISE W

            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 8)) gamepad()->press_left_side_button(true);            else gamepad()->press_left_side_button(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 9)) gamepad()->press_right_side_button(true);            else gamepad()->press_right_side_button(false);

            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 10)) gamepad()->press_left_stick(true);  else gamepad()->press_left_stick(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 11)) gamepad()->press_right_stick(true);  else gamepad()->press_right_stick(false);


            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 12)) gamepad()->press_menu_left(true);        else gamepad()->press_menu_left(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 13)) gamepad()->press_home_xbox_button(true);    else gamepad()->press_home_xbox_button(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 14)) gamepad()->press_menu_right(true);        else gamepad()->press_menu_right(false);
            if(IntAndBinaryUtility::is_integer_bit_right_to_left_true (value, 15)) {/*KILL SWITCH ON*/}        else /*KILL SWITCH OFF*/;

          //Decode trigger values from bits
          //Left trigger: bits 16-19
          //Right trigger: bits 20-23
          //Pattern: 0.10 (16/20), 0.15 (17/21), 0.25 (18/22), 0.50 (19/23)          
          //This is the decoding logic (inverse of encoding above)
          // For reference when implementing the decode function:
           float trigger_left = 0.0;
           float trigger_right = 0.0;
           if is_integer_bit_right_to_left_true(value, 16): trigger_left += 0.10
           if is_integer_bit_right_to_left_true(value, 17): trigger_left += 0.15
           if is_integer_bit_right_to_left_true(value, 18): trigger_left += 0.25
           if is_integer_bit_right_to_left_true(value, 19): trigger_left += 0.50
           if is_integer_bit_right_to_left_true(value, 20): trigger_right += 0.10
           if is_integer_bit_right_to_left_true(value, 21): trigger_right += 0.15
           if is_integer_bit_right_to_left_true(value, 22): trigger_right += 0.25
           if is_integer_bit_right_to_left_true(value, 23): trigger_right += 0.50

            gamepad()->set_trigger_left_percent(triggerLeft);
            gamepad()->set_trigger_right_percent(triggerRight);

            if(arrowVertical==1 && arrowHorizontal==0)
                 gamepad()->press_arrow_n();
            else if(arrowVertical==1 && arrowHorizontal==1)
                gamepad()->press_arrow_ne();
            else if(arrowVertical==0 && arrowHorizontal==1)
                gamepad()->press_arrow_e();
            else if(arrowVertical==-1 && arrowHorizontal==1)
                gamepad()->press_arrow_se();
            else if(arrowVertical==-1 && arrowHorizontal==0)
                gamepad()->press_arrow_s();
            else if(arrowVertical==-1 && arrowHorizontal==-1)
                gamepad()->press_arrow_sw();
            else if(arrowVertical==0 && arrowHorizontal==-1)
                gamepad()->press_arrow_w();
            else if(arrowVertical==1 && arrowHorizontal==-1)
                gamepad()->press_arrow_nw();
            else
                gamepad()->release_dpad();
          }
      }
};









class AbstractIndexIntegerDateUdpRelay {
public:
  virtual void push_in_integer(int32_t index, int32_t value, uint64_t date) = 0;
  virtual ~AbstractIndexIntegerDateUdpRelay() = default;
};


class DefaultIntegerUdpRelay : public AbstractIndexIntegerDateUdpRelay {
public:
  GamepadXboxIntegerWrapper int_to_gamepad_xbox;
  GamepadXboxBLE* gamepad = nullptr;

  void set_gamepad(GamepadXboxBLE* gp) {
    gamepad = gp;
    int_to_gamepad_xbox.set_gamepad(gp);
  }
  
  void push_in_integer(int32_t index, int32_t value, uint64_t date) override {
    DebugPrintStatic::debug_println("Index: " + String(index) + " Value: " + String(value) + " Date: " + String(date));
    int_to_gamepad_xbox.parse_integer(value);
    
  }
};


class IntegerListenToUDP {

public:
  const char* ssid = WIFI_NAME;
  const char* password = WIFI_PASSWORD;

  WiFiUDP wifi_udp_listener;
  int UDP_PORT = 3615;
  String only_accept_from_ip = "0.0.0.0";
  AbstractIndexIntegerDateUdpRelay* udp_relay = nullptr;

  void set_udp_relay(AbstractIndexIntegerDateUdpRelay* relay) {
    udp_relay = relay;
  }

  void setup(String wifi_name, String wifi_password, int32_t udp_port_to_listen, String accept_from_ip = "0.0.0.0") {
    ssid = wifi_name.c_str();
    password = wifi_password.c_str();
    UDP_PORT = udp_port_to_listen;
    only_accept_from_ip = accept_from_ip;
    WiFi.begin(ssid, password);
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(300);
      Serial.print(".");
    }
    Serial.println("\nConnected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    wifi_udp_listener.begin(udp_port_to_listen);
    Serial.printf("UDP server listening on 0.0.0.0:%d\n", udp_port_to_listen);
    Serial.println("Accepting from IP: " + only_accept_from_ip);
  }

  void loop() {
    int packetSize = wifi_udp_listener.parsePacket();
    if (packetSize > 0) {
      uint8_t buffer[512];
      int len = wifi_udp_listener.read(buffer, sizeof(buffer) - 1);
      String fromIp = wifi_udp_listener.remoteIP().toString();

      if (only_accept_from_ip != "0.0.0.0" && fromIp != only_accept_from_ip) {
        DebugPrintStatic::debug_println("Ignored packet from " + fromIp);
        return;
      }


      if (len > 0) {
        if (DebugPrintStatic::is_using_debug_print()) {
          Serial.printf("Received %d bytes from %s:%d, forwarded to UART\n",
                        len,
                        wifi_udp_listener.remoteIP().toString().c_str(),
                        wifi_udp_listener.remotePort());
        }
        int value_received=0;
        if (len == 4) {
          int32_t value = IIDUtility::byte_to_little_endian_int(buffer[0], buffer[1], buffer[2], buffer[3]);
          if (udp_relay != nullptr) {
            udp_relay->push_in_integer(0, value, 0);
            value_received = value;
          }
        } else if (len == 16) {

          int32_t index = IIDUtility::byte_to_little_endian_int(buffer[0], buffer[1], buffer[2], buffer[3]);
          int32_t value = IIDUtility::byte_to_little_endian_int(buffer[4], buffer[5], buffer[6], buffer[7]);
          uint64_t date = IIDUtility::bytes_to_little_endian_uint64(buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
          if (udp_relay != nullptr) {
            udp_relay->push_in_integer(index, value, date);
            value_received = value;
          }
        } else if (len == 8) {
          int32_t index = IIDUtility::byte_to_little_endian_int(buffer[0], buffer[1], buffer[2], buffer[3]);
          int32_t value = IIDUtility::byte_to_little_endian_int(buffer[4], buffer[5], buffer[6], buffer[7]);
          if (udp_relay != nullptr) {
            udp_relay->push_in_integer(index, value, 0);
            value_received = value;
          }
        } else if (len == 12) {
          int32_t value = IIDUtility::byte_to_little_endian_int(buffer[0], buffer[1], buffer[2], buffer[3]);
          uint64_t date = IIDUtility::bytes_to_little_endian_uint64(buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11]);
          if (udp_relay != nullptr) {
            udp_relay->push_in_integer(0, value, date);
            value_received = value;
          }
        }

        if (value_received == 3123) {
            String ip = WiFi.localIP().toString();
            String device_name = ESP32_NAME;
            String unique_id = WiFi.macAddress();
            String response = ip + "|" + device_name + "|" + unique_id;
            int response_length = response.length();
            uint8_t response_buffer[response_length];
            response.getBytes(response_buffer, response_length + 1);
            wifi_udp_listener.beginPacket(wifi_udp_listener.remoteIP(), 3123);
            wifi_udp_listener.write(response_buffer, response_length);
            wifi_udp_listener.endPacket();
        }
      }
    }
    delay(1);
  }
};


class MainManager {
public:
  IntegerListenToUDP udp_listener;
  DefaultIntegerUdpRelay udp_iid_relay;
  
  void setup() {
    GamepadXboxBLE* gamepad = new GamepadXboxBLE();
    gamepad->begin();
    udp_iid_relay = DefaultIntegerUdpRelay();
    udp_listener = IntegerListenToUDP();
    udp_listener.set_udp_relay(&udp_iid_relay);
    udp_iid_relay.set_gamepad(gamepad);
    udp_listener.setup("EloiStreeWifi2G", "11234566", 3615, "192.168.178.38");
  }

  void loop() {
    udp_listener.loop();
    delay(1);
  }
};

MainManager main_manager;

void setup() {
  Serial.begin(115200);
  main_manager.setup();
}

void loop() {
  main_manager.loop();
}






// /*
//  * ----------------------------------------------------------------------------
//  * "PIZZA LICENSE":
//  * https://github.com/EloiStree wrote this file.
//  * As long as you retain this notice you
//  * can do whatever you want with this stuff.
//  * If you think my code made you win a day of work,
//  * send me a good 🍺 or a 🍕 at
//  *  - https://www.patreon.com/eloiteaching
//  *
//  * You can also support my work by building your own DIY input using Amazon links:
//  * - https://github.com/EloiStree/HelloInput
//  *
//  * May the code be with you.
//  *
//  * Updated version: https://github.com/EloiStree/License
//  * ----------------------------------------------------------------------------
//  */

//  // LAST UPDATE: 2024 11 26

// // This code is desing to allow: learning by codde,  QA testing in game and remapping of input
// // ESP32:   https://github.com/EloiStree/HelloInput/issues/288
// // Context: https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// // Learn:   https://github.com/EloiStree/HelloInput



// #include <BleConnectionStatus.h>
// #include <BleCompositeHID.h>
// #include <KeyboardDevice.h>
// #include <MouseDevice.h>
// #include <XboxGamepadDevice.h>
// #include <KeyboardHIDCodes.h>


// #define RXD2 16   // GPIO16 (U2RXD) Not Setable on the ESP32
// #define TXD2 17   // GPIO17 (U2TXD) Not Setable on the ESP32
// int m_lepPin = 2;  // Can bet 15 on some (depend of the model)
// int m_vibrationPin=3; // If you want game feedback
// int m_activablePinIOLength=16;
// // https://chatgpt.com/share/67449be8-09c0-800e-a018-a2246365eedb
// // Not verified yet
// int m_activablePinIO[] = {4,5,12,13,14,15,18,19,21,22,23,25,26,27,32,33};
// int m_activablePinIOState[] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};


// bool m_useBluetoothElectronicFeedBack=true;

// bool m_useHardwareJoystick = false;
// int m_pinJosytickLeftVertical = 33;
// int m_pinJosytickLeftHorizontal = 32;
// int m_pinJosytickRightVertical = 35;
// int m_pinJosytickRightHorizontal = 34;

// XboxGamepadDevice *gamepad;
// GamepadDevice *gamepadGeneric;
// KeyboardDevice* keyboard;
// MouseDevice* mouse;
// BleCompositeHID compositeHID("eLabRC XInput ESP32", "eLabRC", 100);


// // RETURN THE RECEIVED BYTE TO THE SENDER
// // IT ALLOWS TO SEE IF HC05 BRIDGE IS WORKING PROPERLY
// bool m_return_byte_received = true;
// // DISPLAY BYTE RECEIVED
// bool m_use_print_byte_debug= true;
// // DISPLAY ACTION TRIGGERED
// bool m_use_print_action_debug= true;



// //############################### START INTEGER FOUR CHAR INPUT ####################################

// // TEXT is more easy for Human but my library out of hardware use integer little endian
// // Turn in text mode if you use text as input
// // If you need to come back to integer mode, restart the device or send the max int value
// // SEND FFFF as text in your app to trigger the UTF8 MODE
// // SEND 0000 as text or integer to reset the index to 0 for the following input
// // In case there are some byte lost in the way.
// // 0:c(IIII) THE DEVICE EXPECT INTEGER AS FOUR BYTES
// // 1:c(FFFF) THE DEVICE EXPECT UTF8 AS FOUR CHARACTERS TURN THEM INTO INTEGER
// // 2:c(ffff) THE DEVICE EXPECT \n AS END OF COMMAND THEN TURN TEXT IN INTEGER
// // Those byte are reserved for mode change
// //M:1 b(70 70 70 70) = UTF8 MODE C4:FFFF | Int:1179010630
// //M:2 b(214 214 214 214) = TEXT INTEGER MODE C4:ffff |  Int:1717986918
// //M:0 b(73 73 73 73) = INTEGER MODE C4:IIII |  Int:1229539657
// //CMD b(48 48 48 48) = RESET INDEX TO ZERO C4:0000 | Int:808464432
// //CMD b(0 0 0 0) = RESET INDEX TO ZERO from bytes Int:0
// int m_readingMode=2;
// int INTEGER_MODE=0;
// int INTEGER_MODE_INT = 1229539657;
// int UTF8_MODE=1;
// int UTF8_MODE_INT = 1179010630;
// int TEXT_INTEGER_MODE=2;
// int TEXT_INTEGER_MODE_INT = 1717986918;



// byte m_blr0='0';
// byte m_blr1='0';
// byte m_blr2='0';
// byte m_blr3='0';
// int m_intCmd; // from 4 bytes as little endian
// String m_textCmd="" ; // Corresponding UTF8 as text
// String m_lineReturn="" ; // Corresponding UTF8 as text
// int m_fourCharIndex=0;

// //USE AS BIT BUFFER TO PARSE 1 0 TO ACTIONS
// char m_binaryBufferOfInteger[33];

// bool IsInUTF8FourCharMode(){return m_readingMode==1;}
// bool IsInUTF8LineReturnMode(){return m_readingMode==2;}
// bool IsInByteLittleEndianMode(){return m_readingMode==0;}


// void CheckFromSwitchModeFromLineReturn(){
//     if(m_readingMode ==TEXT_INTEGER_MODE){
//         if(m_textCmd == "IIII"){
//             m_readingMode=0;
//             Serial.println("############## Switch to BYTE INTEGER mode ############## ");
//         }
//         else if(m_textCmd == "FFFF"){
//             m_readingMode=1;
//             Serial.println("############## Switch to UTF8 mode ############## ");
//         }
//     }
// }




// // ##########  RECEIVED INTEGER AS BYTES
// // https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// /// CONVERT FOUR BYTE IN LITTLE ENDIAN FORMAT TO SIGNED INTEGER 32 BITS
// // USE: byte bytes[4] = {0xC0, 0x1D, 0xFE, 0xFF};  // Little-endian encoding of -123456
// // USE: int32_t result = parseLittleEndian(bytes[0], bytes[1], bytes[2], bytes[3]);
// int32_t parseLittleEndian(byte b0, byte b1, byte b2, byte b3) {
//   // Combine bytes in little-endian order
//   return ((int32_t)b0) | ((int32_t)b1 << 8) | ((int32_t)b2 << 16) | ((int32_t)b3 << 24);
// }

// // ##########  SEND INTEGER TO BYTES
// // https://github.com/EloiStree/2024_08_29_ScratchToWarcraft
// // CONVERT SIGNE INTEGER INTO FOUR BYTES IN LITTLE ENDIAN
// // USE: int32_t value = -123456;
// // USE: byte bytes[4];
// // USE: intToBytes(value, bytes);
// void intToBytes(int32_t value, uint8_t bytes[4]) {
//   bytes[0] = value & 0xFF;         // Extract the lowest 8 bits
//   bytes[1] = (value >> 8) & 0xFF;  // Shift right by 8 bits and extract the next 8 bits
//   bytes[2] = (value >> 16) & 0xFF; // Shift right by 16 bits and extract the next 8 bits
//   bytes[3] = (value >> 24) & 0xFF; // Shift right by 24 bits and extract the highest 8 bits
// }




// void displayFourByteAndChars(){
//     Serial.print("B:");
//     Serial.print(m_blr0);
//     Serial.print(" ");
//     Serial.print(m_blr1);
//     Serial.print(" ");
//     Serial.print(m_blr2);
//     Serial.print(" ");
//     Serial.print(m_blr3);
//     Serial.print("|Int:");
//     Serial.print(m_intCmd);
// }

// void notifyModeChanged(){
//     if(m_readingMode==0){
//         Serial.println("############## BYTE INTEGER MODE ############## ");
//     }
//     if(m_readingMode==1){
//         Serial.println("############## UTF8 MODE ############## ");
//     }
//     if(m_readingMode==2){
//         Serial.println("############## TEXT INTEGER MODE ############## ");
//     }
// }

// void stackByte(byte b){
//     m_blr0 = m_blr1;
//     m_blr1 = m_blr2;
//     m_blr2 = m_blr3;
//     m_blr3 = b;
//     m_intCmd = parseLittleEndian(m_blr0, m_blr1, m_blr2, m_blr3);

//     m_fourCharIndex++;
//     displayFourByteAndChars();
//     if(m_fourCharIndex>=4){
//         m_fourCharIndex=0;
//         Serial.println("-->> Four byte Received");
//         integerCommandReceived(m_intCmd);
//     }
// }
// //############################### STOP INTEGER FOUR CHAR INPUT ####################################


// bool m_use_print_received_double_char = true;

// int MIN_VALUE = 0;
// int MAX_VALUE = 32767;
// int MIN_VALUE_D2 = (int)(MAX_VALUE * 1.0 / 4.0);
// int MAX_VALUE_D2 = (int)(MAX_VALUE * 3.0 / 4.0);
// int MIN_VALUE_D3 = (int)(MAX_VALUE * 2.0 / 6.0);
// int MAX_VALUE_D3 = (int)(MAX_VALUE * 4.0 / 6.0);
// int MIN_VALUE_D4 = (int)(MAX_VALUE * 3.0 / 8.0);
// int MAX_VALUE_D4 = (int)(MAX_VALUE * 5.0 / 8.0);
// int MIN_VALUE_D8 = (int)(MAX_VALUE * 7.0 / 16.0);
// int MAX_VALUE_D8 = (int)(MAX_VALUE * 9.0 / 16.0);

// int MIDDLE_VALUE = 32767 /2;

// bool isCharDigital(char c) {
//     return c >= '0' && c <= '9';
// }

// void OnVibrateEvent(XboxGamepadOutputReportData data)
// {
//     if(data.weakMotorMagnitude > 0 || data.strongMotorMagnitude > 0){
//         digitalWrite(m_lepPin, LOW);
//     } else {
//         digitalWrite(m_lepPin, HIGH);
//     }
// }

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(m_lepPin, OUTPUT);
//   XboxSeriesXControllerDeviceConfiguration* gamepadConfig = new XboxSeriesXControllerDeviceConfiguration();
//   // The composite HID device pretends to be a valid Xbox controller via vendor and product IDs (VID/PID).
//   // Platforms like windows/linux need this in order to pick an XInput driver over the generic BLE GATT HID driver.
//   BLEHostConfiguration hostConfig = gamepadConfig->getIdealHostConfiguration();
//   Serial.println("Using VID source: " + String(hostConfig.getVidSource(), HEX));
//   Serial.println("Using VID: " + String(hostConfig.getVid(), HEX));
//   Serial.println("Using PID: " + String(hostConfig.getPid(), HEX));
//   Serial.println("Using GUID version: " + String(hostConfig.getGuidVersion(), HEX));
//   Serial.println("Using serial number: " + String(hostConfig.getSerialNumber()));
//   gamepad = new XboxGamepadDevice(gamepadConfig);
//   FunctionSlot<XboxGamepadOutputReportData> vibrationSlot(OnVibrateEvent);
//   gamepad->onVibrate.attach(vibrationSlot);
//   compositeHID.addDevice(gamepad);
//   Serial.println("Starting composite HID device...");
//   compositeHID.begin(hostConfig);
//   //setLeftVertical(1);
//   //setRightVertical(1);
//   releaseAllGamepad();

//   Serial.println("Starting BLE work!");
//   Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
//   Serial.println("UART1 initialized.");

// }



// void integerToCharAndBytesFour(int value){
//     uint8_t bytes[4];
//     intToBytes(value, bytes);
//     for(int i=0; i<4; i++){
//         stackByte(bytes[i]);
//     }
// }

// void reset_four_bytes_buffer_index_to_zero(){m_fourCharIndex = 0;}
// void reset_four_bytes_buffer_value_to_zero(){ m_blr0 =0;   m_blr1 = 0; m_blr2 = 0; m_blr3 = 0; }
// void reset_four_bytes_buffer_index_value_to_zero(){
//   reset_four_bytes_buffer_index_to_zero();
//   reset_four_bytes_buffer_value_to_zero();
// }

// bool use_debug_print=True;
// void debug_print(String text){
//   if (use_debug_print)
//         Serial.print(text);
// }
// void debug_println(String text){
//   if (use_debug_print)
//         Serial.println(text);
// }

// void serialReceivedString(String input){
//     if(isNumericString(input)){
//         Serial.print("Int:");
//         Serial.println(input); // Echo the input back to the serial monitor
//         int number = input.toInt(); // Convert the string to an integer
//         Serial.print("You entered the number: ");
//         Serial.println(number);
//         reset_four_bytes_buffer_index_value_to_zero();
//         integerCommandReceived(number);
//     }
//     else{
//         Serial.print("CMD:");
//         Serial.println(input);
//         if(input=="Hello"){
//           Serial.println("Hello there");
//         }
//     }
// }
// void serialReceivedByte(byte incomingByte){
//     stackByte(incomingByte);
// }

// // Function to check if a string contains only numeric characters
// bool isNumericString(String str) {
//   for (int i = 0; i < str.length(); i++) {
//     char c = str.charAt(i);
//     if(c=='0' || c=='1' || c=='2' || c=='3' || c=='4'
//     || c=='5' || c=='6' || c=='7' || c=='8' || c=='9'
//     || c=='\n' || c=='\r' || c==' '){
//         continue;
//     }
//     return false;
//   }
//   return true;
// }






// void SwitchAllPins(){

//     for(int i=0; i<m_activablePinIOLength; i++){
//         SwitchStateOfPin(i);
//     }
// }
// void SetAllPinsTo(bool value){

//     for(int i=0; i<m_activablePinIOLength; i++){
//         SetPinOnOff(i, value);
//     }

// }


// void traditionalTextCommand(String text){


//     text.trim();
//     Serial.print("CMD:");
//     Serial.println(text);
//     BE_SendSoundPing();


//   // 32 (Space)
//   // 33-47 (Special Characters): ! " # $ % & ' ( ) * + , - . /
//   // 48-57 (Numbers): 0 1 2 3 4 5 6 7 8 9
//   // 58-64 (Special Characters): : ; < = > ? @
//   // 65-90 (Uppercase Letters): A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
//   // 91-96 (Special Characters): `[ \ ] ^ _ ``
//   // 97-122 (Lowercase Letters): a b c d e f g h i j k l m n o p q r s t u v w x y z
//   // 123-126 (Special Characters): { | } ~

//     // Using Bluetooth Electronic Default version
//     // Buttons
//     if(text=="G") pressA(true);
//     else if(text=="BEDEBUG") m_useBluetoothElectronicFeedBack=true;
//     else if(text=="bedebug") m_useBluetoothElectronicFeedBack=false;
//     else if(text=="PINSWITCH") SwitchAllPins();
//     else if(text=="g") pressA(false);
//     else if(text=="Y") pressY(true);
//     else if(text=="y") pressY(false);
//     else if(text=="B") pressX(true);
//     else if(text=="b") pressX(false);
//     else if(text=="R") pressB(true);
//     else if(text=="r") pressB(false);
//     else if(text=="S" || text=="START") pressMenuRight(true);
//     else if(text=="s" || text=="start") pressMenuRight(false);
//     else if(text=="BACK") pressMenuLeft(true);
//     else if(text=="back") pressMenuLeft(false);
//     else if(text=="H") pressHomeXboxButton(true);
//     else if(text=="h") pressHomeXboxButton(false);
//     else if(text=="M") pressMenuLeft(true);
//     else if(text=="m") pressMenuLeft(false);
//     else if(text=="N") pressMenuRight(true);
//     else if(text=="n") pressMenuRight(false);
//     else if(text=="BA" || text=="BD") pressA(true);
//     else if(text=="ba"|| text=="bd") pressA(false);
//     else if(text=="BY"|| text=="BU") pressY(true);
//     else if(text=="by"|| text=="bu") pressY(false);
//     else if(text=="BX"|| text=="BL") pressX(true);
//     else if(text=="bx"|| text=="bl") pressX(false);
//     else if(text=="BB"|| text=="BR") pressB(true);
//     else if(text=="bb"|| text=="br") pressB(false);
//     else if(text=="AU") pressArrowN();
//     else if(text=="AC") releaseDPad();
//     else if(text=="AR") pressArrowE();
//     else if(text=="AD") pressArrowS();
//     else if(text=="AL") pressArrowW();
//     else if(text=="AN") pressArrowN();
//     else if(text=="AE") pressArrowE();
//     else if(text=="AS") pressArrowS();
//     else if(text=="AW") pressArrowW();
//     else if(text=="ANW") pressArrowNW();
//     else if(text=="ANE") pressArrowNE();
//     else if(text=="ASE") pressArrowSE();
//     else if(text=="ASW") pressArrowSW();
//     else if(text=="ANW") pressArrowNW();
//     else if(text=="ANE") pressArrowNE();
//     else if(text=="ASE") pressArrowSE();
//     else if(text=="ASW") pressArrowSW();
//     else if(text=="RECORD") recordStart();
//     else if(text=="record") recordStop();
//     else if(text=="SBL") pressLeftSideButton(true);
//     else if(text=="sbl") pressLeftSideButton(false);
//     else if(text=="SBR") pressRightSideButton(true);
//     else if(text=="sbr") pressRightSideButton(false);
//     else if(text=="JL") pressLeftStick(true);
//     else if(text=="jl") pressLeftStick(false);
//     else if(text=="TL") setTriggerLeftPercent(1);
//     else if(text=="tl") setTriggerLeftPercent(0);
//     else if(text=="TR") setTriggerRightPercent(1);
//     else if(text=="tr") setTriggerRightPercent(0);
//     else if(text=="JR") pressRightStick(true);
//     else if(text=="jr") pressRightStick(false);
//     else if(text=="MR") pressMenuRight(true);
//     else if(text=="mr") pressMenuRight(false);
//     else if(text=="ML") pressMenuLeft(true);
//     else if(text=="ml") pressMenuLeft(false);
//     else if(text=="MC") pressHomeXboxButton(true);
//     else if(text=="mc") pressHomeXboxButton(false);
//     // Big Grey Circle
//     else if(text=="M"){}
//     // Big Rectancle
//     else if(text=="W"){}
//     // Switch vertical on off
//     else if(text=="C") m_useHardwareJoystick=true;
//     else if(text=="c") m_useHardwareJoystick=false;
//     // Power Button Switch on off
//     else if(text=="D") digitalWrite(m_lepPin, HIGH);
//     else if(text=="d") digitalWrite(m_lepPin, LOW);
//     else if(text=="V") setVibrationOn(true);
//     else if(text=="v") setVibrationOn(false);
//     else{
//         ParseIfContainsJoystickFromBlueElec(text);
//         ParseIfContainsPinOnOff(text);
//     }
// }

// void setVibrationOn(bool value){
//     if(value){
//         digitalWrite(m_vibrationPin, HIGH);
//     }
//     else{
//         digitalWrite(m_vibrationPin, LOW);
//     }
// }



// void BE_RemindMeOfPinNumber(int pinIndex){

//     if(!m_useBluetoothElectronicFeedBack)
//         return;
//     if(pinIndex<0 || pinIndex>= m_activablePinIOLength){
//        Serial2.print("No pin for index:");
//        Serial2.println(pinIndex);
//     }
//     else{
//         Serial2.print("Pin index ");
//         Serial2.print(pinIndex);
//         Serial2.print(" is ");
//         Serial2.println(m_activablePinIO[pinIndex]);
//     }

// }
// void SwitchStateOfPin(int pinIndex){

//   if(pinIndex<0 || pinIndex>= m_activablePinIOLength)
//         return;

//       if(m_activablePinIOState[pinIndex]==LOW){
//           SetPinOnOff(pinIndex, true);
//           }
//       else {
//           SetPinOnOff(pinIndex, false);
//       }
// }

// void BE_SendPinState(int index){
// if(m_useBluetoothElectronicFeedBack){
//     if(index<0 || index>= m_activablePinIOLength)
//         return;
//        char p = '0';
//        switch(index){
//         case 0:p='0'; break;
//         case 1:p='1'; break;
//         case 2:p='2'; break;
//         case 3:p='3'; break;
//         case 4:p='4'; break;
//         case 5:p='5'; break;
//         case 6:p='6'; break;
//         case 7:p='7'; break;
//         case 8:p='8'; break;
//         case 9:p='9'; break;
//        }
//       if(m_activablePinIOState[index]==HIGH)
//         BE_SendColorInfo(p, 1.0, 1.0, 1.0);
//       else
//         BE_SendColorInfo(p, 0.1, 0.1, 0.1);
//     }
// }

// void SetPinOnOff (int pinIndex, bool isOn){
//     if(pinIndex<0 || pinIndex>= m_activablePinIOLength)
//       return;

//     if(isOn){
//         digitalWrite(m_activablePinIO[pinIndex], HIGH);
//         m_activablePinIOState[pinIndex]=HIGH;
//     }
//     else{
//         digitalWrite(m_activablePinIO[pinIndex], LOW);
//         m_activablePinIOState[pinIndex]=LOW;
//     }
//     BE_SendPinState(pinIndex);
// }

// void ParseIfContainsPinOnOff(String text){

//     int length = text.length();
//     if(length>4)
//         return;
//     if(!(text.charAt(0)=='P'||text.charAt(0)=='p'))
//         return;
//     bool isOn = text.charAt(0)=='P';
//     int pin = text.substring(1).toInt();
//     if(pin<0 || pin>=m_activablePinIOLength)
//         return;

//     SetPinOnOff(pin, isOn);
// }

// void ParseIfContainsJoystickFromBlueElec(String text){
//     // 🤖 Genereted by GPT not tested yet.
//     // Joystick left  X50Y50 from 0 to 100
//     // Joystick left  LX50Y50 from 0 to 100
//     // Joystick right RX50Y50 from 0 to 100
//     bool isFound = false;
//     float x = 0;
//     float y = 0;
//     // Check if the string starts with LX or RX or just X
//     if (text.startsWith("LX")) {
//         isFound = true;
//         int xIndex = text.indexOf('X') + 1; // After LX
//         int yIndex = text.indexOf('Y', xIndex) + 1;
//         if (xIndex > 0 && yIndex > 0) {
//             x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
//             y = text.substring(yIndex).toFloat();
//             x = ((x-50.0)/50.0);
//             y = ((y-50.0)/50.0);
//             setLeftHorizontal(x);
//             setLeftVertical(-y);
//         }
//     }
//     else if (text.startsWith("RX")) {
//         isFound = true;
//         int xIndex = text.indexOf('X') + 1; // After RX
//         int yIndex = text.indexOf('Y', xIndex) + 1;

//         if (xIndex > 0 && yIndex > 0) {
//             x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
//             y = text.substring(yIndex).toFloat();
//             x = ((x-50.0)/50.0);
//             y = ((y-50.0)/50.0);
//             setRightHorizontal(x);
//             setRightVertical(-y);
//         }

//     }
//     else if (text.startsWith("X")) {
//         isFound = true;
//         int xIndex = text.indexOf('X') + 1;
//         int yIndex = text.indexOf('Y', xIndex) + 1;

//         if (xIndex > 0 && yIndex > 0) {
//             x = text.substring(xIndex, text.indexOf('Y', xIndex)).toFloat();
//             y = text.substring(yIndex).toFloat();
//             x = ((x-50.0)/50.0);
//             y = ((y-50.0)/50.0);
//             setLeftHorizontal(x);
//             setLeftVertical(-y);
//         }

//     }
//     else if (text.startsWith("TL")) {
//         isFound = true;
//         int tlIndex = text.indexOf('L') + 1; // After TL

//         if (tlIndex > 0) {
//             float triggerValue = text.substring(tlIndex).toInt();
//             setTriggerLeftPercent(triggerValue/100.0);
//         }
//     }
//     else if (text.startsWith("TR")) {
//         isFound = true;
//         int trIndex = text.indexOf('R') + 1; // After TR

//         if (trIndex > 0) {
//             float triggerValue = text.substring(trIndex).toInt();
//             setTriggerRightPercent(triggerValue/100.0);
//         }
//     } // Check if the string starts with 'A'
//     else if (text.startsWith("A")) {
//         isFound = true;
//         float sensibilityInDegree=15.0;
//         // Find positions of the key characters
//         int startIndex = text.indexOf('A') + 1; // Start after 'A'
//         int commaIndex = text.indexOf(',');
//         int starIndex = text.indexOf('*');

//         // Ensure all necessary characters are present
//         if (startIndex > 0 && commaIndex > startIndex && starIndex > commaIndex) {
//             // Extract and convert X and Y values
//             String pitch = text.substring(startIndex, commaIndex);
//             String roll = text.substring(commaIndex + 1, starIndex);

//             x = ((pitch.toFloat()) /sensibilityInDegree); // From 'A' to ','
//             y = ((roll.toFloat()) / sensibilityInDegree); // From ',' to '*'
//             setLeftHorizontal(x);
//             setLeftVertical(-y);
//         }
//     }
// }




// void update_sticks(){

//         int16_t lx = m_left_horizontal * XBOX_STICK_MAX;
//         int16_t ly = m_left_vertical * XBOX_STICK_MAX;
//         int16_t rx = m_right_horizontal * XBOX_STICK_MAX;
//         int16_t ry = m_right_vertical * XBOX_STICK_MAX;
//         gamepad->setLeftThumb(lx, ly);
//         gamepad->setRightThumb(rx, ry);
//         gamepad->sendGamepadReport();


//         Serial.print("LX: ");
//         Serial.print(lx);
//         Serial.print(" LY: ");
//         Serial.print(ly);
//         Serial.print(" RX: ");
//         Serial.print(rx);
//         Serial.print(" RY: ");
//         Serial.println(ry);

// }







// /**


// */
