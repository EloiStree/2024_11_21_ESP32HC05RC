
# ESP32 to XInput

> Allows using the Wi-Fi of an ESP32 to simulate a Bluetooth XInput gamepad.

This tool listens to the [IID](https://github.com/EloiStree/IID) and [S2W](https://github.com/EloiStree/S2W) formats through UDP messages, which are interpreted as Xbox Input.

I was previously using UART.
However, Bluetooth multiplatform plugins are a pain to add to the game engine I use for teaching.

**Previous tutorial:**
[https://youtu.be/-XuE9P1T2_k?t=201](https://youtu.be/-XuE9P1T2_k?t=201)

To simulate XInput, we need the following library. ([Archived ZIP](https://github.com/EloiStree/2024_11_21_ESP32HC05RC/releases/tag/ZIP_SAVE))

Go to:

```
C:\Users\USER_NAME\Documents\Arduino\libraries
```

Unzip the following tools from the release section.

**Required:**

* [https://github.com/tomstewart89/Callback](https://github.com/tomstewart89/Callback)
* [https://github.com/h2zero/NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
* [https://github.com/Mystfit/ESP32-BLE-CompositeHID](https://github.com/Mystfit/ESP32-BLE-CompositeHID)

To use them, you will need to add the board firmware.

As a precaution, I added the reference to the ESP32 GitHub repository:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

Some tools take too long to download or install and can block Arduino on Windows.
See the error and solution here:

```
https://github.com/espressif/arduino-esp32/issues/12161#issuecomment-3680101494
```

You need to go to:

```
C:\Users\USER_NAME\.arduinoIDE\arduino-cli.yaml
```

And add this line to prevent Arduino from timing out during installation:

```
network:
  connection_timeout: 300s
```

---


![image](https://github.com/user-attachments/assets/7e3dacf4-0b70-4392-93af-99d580b50749)


Introduction of ESP32
[<img width="932" height="517" alt="image" src="https://github.com/user-attachments/assets/d499daad-57ad-4906-8271-83ef5d9d1aa1" />](https://www.youtube.com/watch?v=RiYnucfy_rs)  
https://www.youtube.com/watch?v=RiYnucfy_rs    



