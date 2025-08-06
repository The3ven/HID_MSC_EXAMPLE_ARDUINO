#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#include "definations.h"

#define DEBUG_USB_KEYBOARD 1

/* --------------------------------------------------------------------------------------------------- */

// #include "ORIGHT_Global_Vars.h"

/* --------------------------------------------------------------------------------------------------- */
// #if defined(ESP32_S3) && defined(ENABLE_USB_KEYBOARD)
  // #define BACK_KEY char(8)    // BACKSPACE
  // #define TAB char(9)        // TAB Key
  // #define ENTER char(13)     // ENTER Key
  // #define MAIN_MENU char(27) // Esc
  // #define DEL char(127)      // DELETE Key
  // #define RIGHT_ARROW char(187)
  // #define LEFT_ARROW char(171)
  // #define DOWN_ARROW char(208)
  // #define UP_ARROW char(181)
  // #define MISC ENTER
  // #define SPACEBAR char(32)

  /* --------------------------------------------------------------------------------------------------- */

  // class MyEspUsbHost : public EspUsbHost
  // {
  //     void 
  // };
  // extern MyEspUsbHost usbHost;

// #endif
  extern uint16_t myusbkey;
  
  // bool setKeyboardLED();
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier);
  void Keyboard_input_checker(uint16_t keyCode);
#endif

/* --------------------------------------------------------------------------------------------------- */