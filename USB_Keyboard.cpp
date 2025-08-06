#include "USB_Keyboard.h"
#include <map>
#include <array>

/* --------------------------------------------------------------------------------------------------- */

// MyEspUsbHost usbHost;
uint16_t myusbkey = 0;
bool CAPS_LOCK = false;
bool NUM_LOCK = false;
bool SCROLL_LOCK = false;

/* Lookup tables for key mappings */
struct KeyMapping
{
  uint8_t keycode;
  uint16_t value;
};

// Special keys (ascii == 0)
const std::map<uint8_t, uint16_t> specialKeyMap =
{
  {41, ESCAPE_KEY},   // Escape Key
  {43, TAB},          // Tab
  {57, 0},            // CAPS LOCK (handled separately)
  {58, F1},
  {59, F2},           // Modified by CTRL/SHIFT in logic
  {60, F3},
  {61, F4},
  {62, F5},
  {63, F6},
  {64, F7},
  {65, F8},
  {66, F9},
  {67, F10},
  {68, F11},
  {69, F12},
  {71, 0},            // SCROLL LOCK (handled separately)
  {73, INSERT_KEY},
  {74, HOME_KEY},
  {75, PAGE_UP},
  {76, DEL},
  {77, END_KEY},
  {78, PAGE_DOWN},
  {79, RIGHT_ARROW},
  {80, LEFT_ARROW},
  {81, DOWN_ARROW},
  {82, UP_ARROW},
  {83, 0},            // NUM LOCK (handled separately)
  
  
  
  // NUMPAD
  {84, '/'},
  {85, '*'},
  {86, '-'},
  {87, '+'},
  {88, ENTER},  // ENTER
  {89, '1'},    // numpad (deactivated when NUM_LOCK)
  {90, '2'},    // numpad (Down Arrow when NUM_LOCK)
  {91, '3'},    // numpad (deactivated when NUM_LOCK)
  {92, '4'},    // numpad (Left Arrow when NUM_LOCK)
  {93, '5'},    // numpad (deactivated when NUM_LOCK)
  {94, '6'},    // numpad (Right Arrow when NUM_LOCK)
  {95, '7'},    // numpad (deactivated when NUM_LOCK)
  {96, '8'},    // numpad (Up Arrow when NUM_LOCK)
  {97, '9'},    // numpad (deactivated when NUM_LOCK)
  {98, '0'},    // numpad (deactivated when NUM_LOCK)
  {99, '.'}     // numpad (deactivated when NUM_LOCK)

};

const std::map<std::pair<uint8_t, uint8_t>, uint16_t> asciiKeyOverrides = {
  {{13, 40}, ENTER}
};

const std::map<uint16_t, uint16_t> numpadOverrideMap =
{
  {'2', NUMPAD_DOWN},
  {'4', NUMPAD_LEFT},
  {'6', NUMPAD_RIGHT},
  {'8', NUMPAD_UP}
};


/**
 * Handles special keys (e.g., Caps Lock, Function keys).
 */
void handleSpecialKey(uint8_t keycode, uint8_t modifier)
{
  switch (keycode)
  {
    case 57: CAPS_LOCK = !CAPS_LOCK; return;
    case 71: SCROLL_LOCK = !SCROLL_LOCK; return;
    case 83: NUM_LOCK = !NUM_LOCK; return;
  }

  if (keycode == 59)
  {
    myusbkey = (modifier == 2) ? SHIFT_F2 :
               (modifier == 1) ? CTRL_F2 : F2;
    return;
  }

  auto it = specialKeyMap.find(keycode);
  if (it != specialKeyMap.end())
  {
    myusbkey = it->second;
    Serial.println("myusbkey before : " + String(myusbkey));

    // Numpad override when NUM_LOCK is off
    if (!NUM_LOCK && keycode >= 89 && keycode <= 99)
    {
      auto np = numpadOverrideMap.find(myusbkey);
      myusbkey = (np != numpadOverrideMap.end()) ? np->second : 0;
    }

    Serial.println("myusbkey after : " + String(myusbkey));
  }
  else
  {
    myusbkey = 0; // Unknown keycode
  }
}



/**
 * Handles special keys (e.g., Caps Lock, Function keys).
 */
void handleAsciiSpecialKey(uint8_t ascii, uint8_t keycode)
{
  auto it = asciiKeyOverrides.find({ascii, keycode});
  myusbkey = (it != asciiKeyOverrides.end()) ? it->second : ascii;
}

/**
 * Main keyboard event handler.
 */
// void MyEspUsbHost::onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier)
void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier)
{
#ifdef DEBUG_USB_KEYBOARD
  Serial.print("[A]");
  Serial.print(ascii);
  Serial.print(" => ");
  Serial.print(char(ascii));
  Serial.print("\tkeycode => ");
  Serial.print(keycode);
  Serial.print("\tmodifier => ");
  Serial.print(modifier);
  Serial.print("\tcaps => ");
  Serial.print(CAPS_LOCK);
  Serial.println();
#endif

  myusbkey = 0; // Reset key
  if (ascii >= 32 && ascii <= 126)
  {
    if (CAPS_LOCK && ascii >= 97 && ascii <= 122)
    {
      myusbkey = (ascii - 32 );
    }
    else
    {
      myusbkey = ascii;
    }
  }
  else if (ascii < 32 && ascii > 0)
  {
    handleAsciiSpecialKey(ascii, keycode);
  }
  else if (ascii == 0)
  {
    handleSpecialKey(keycode, modifier);
  }

  #ifdef DEBUG_USB_KEYBOARD
    Keyboard_input_checker(myusbkey);
  #endif
}

void Keyboard_input_checker(uint16_t key)
{
  Serial.println("CAPS : " + String(CAPS_LOCK));
  Serial.println("SCROLL_LOCK : " + String(SCROLL_LOCK));
  Serial.println("NUM_LOCK : " + String(NUM_LOCK));

  if (key >= 32 && key <= 126)
  {
    Serial.println("User Pressed : " + String(char(key)));
  }
  else
  {
    switch(key)
    {
      case TAB:
        Serial.println("User pressed TAB");
        break;
      case ESCAPE_KEY:
        Serial.println("User pressed ESCAPE_KEY");
        break;
      case F1:
        Serial.println("User pressed F1");
        break;
      case F2:
        Serial.println("User pressed F2");
        break;
      case F3:
        Serial.println("User pressed F3");
        break;
      case F4:
        Serial.println("User pressed F4");
        break;
      case F5:
        Serial.println("User pressed F5");
        break;
      case F6:
        Serial.println("User pressed F6");
        break;
      case F7:
        Serial.println("User pressed F7");
        break;
      case F8:
        Serial.println("User pressed F8");
        break;
      case F9:
        Serial.println("User pressed F9");
        break;
      case F10:
        Serial.println("User pressed F10");
        break;
      case F11:
        Serial.println("User pressed F11");
        break;
      case F12:
        Serial.println("User pressed F12");
        break;
      case CTRL_F2:
        Serial.println("User pressed CTRL_F2");
        break;
      case SHIFT_F2:
        Serial.println("User pressed SHIFT_F2");
        break;
      case INSERT_KEY:
        Serial.println("User pressed INSERT_KEY");
        break;
      case HOME_KEY:
        Serial.println("User pressed HOME_KEY");
        break;
      case PAGE_UP:
        Serial.println("User pressed PAGE_UP");
        break;
      case DEL:
        Serial.println("User pressed DEL");
        break;
      case END_KEY:
        Serial.println("User pressed END_KEY");
        break;
      case PAGE_DOWN:
        Serial.println("User pressed PAGE_DOWN");
        break;
      case RIGHT_ARROW:
        Serial.println("User pressed RIGHT_ARROW");
        break;
      case LEFT_ARROW:
        Serial.println("User pressed LEFT_ARROW");
        break;
      case DOWN_ARROW:
        Serial.println("User pressed DOWN_ARROW");
        break;
      case UP_ARROW:
        Serial.println("User pressed UP_ARROW");
        break;
      case ENTER:
        Serial.println("User pressed ENTER");
        break;
    }
  }
}

bool setKeyboardLED()
{

  uint8_t ledMask;

  // Combination 1: All OFF
  if (!CAPS_LOCK && !NUM_LOCK && !SCROLL_LOCK) {
    ledMask = 0x00;
  }

  // Combination 2: Only SCROLL_LOCK ON
  if (!CAPS_LOCK && !NUM_LOCK && SCROLL_LOCK) {
    ledMask = LED_SCROLL_LOCK; // Both CAPS LOCK and NUM LOCK are on
  }

  // Combination 3: Only NUM_LOCK ON
  if (!CAPS_LOCK && NUM_LOCK && !SCROLL_LOCK) {
    ledMask = LED_NUM_LOCK; // Both CAPS LOCK and NUM LOCK are on

  }

  // Combination 4: NUM_LOCK + SCROLL_LOCK ON
  if (!CAPS_LOCK && NUM_LOCK && SCROLL_LOCK) {
    ledMask = LED_NUM_LOCK | LED_SCROLL_LOCK; // Both CAPS LOCK and NUM LOCK are on

  }

  // Combination 5: Only CAPS_LOCK ON
  if (CAPS_LOCK && !NUM_LOCK && !SCROLL_LOCK) {
    ledMask = LED_CAPS_LOCK; // Both CAPS LOCK and NUM LOCK are on

  }

  // Combination 6: CAPS_LOCK + SCROLL_LOCK ON
  if (CAPS_LOCK && !NUM_LOCK && SCROLL_LOCK)
  {
    ledMask = LED_CAPS_LOCK | LED_SCROLL_LOCK; // Both CAPS LOCK and NUM LOCK are on
  }

  // Combination 7: CAPS_LOCK + NUM_LOCK ON
  if (CAPS_LOCK && NUM_LOCK && !SCROLL_LOCK)
  {
    ledMask = LED_CAPS_LOCK | LED_NUM_LOCK; // Both CAPS LOCK and NUM LOCK are on
  }

  // Combination 8: All ON
  if (CAPS_LOCK && NUM_LOCK && SCROLL_LOCK)
  {
    ledMask = LED_CAPS_LOCK | LED_NUM_LOCK | LED_SCROLL_LOCK; // Both CAPS LOCK and NUM LOCK are on
  }





  uint8_t bmRequestType = 0x21;                // Host-to-device, Class, Interface
  uint8_t bRequest = 0x09;                     // SET_REPORT
  uint16_t wValue = (0x02 << 8) | 0x00;        // Report Type (Output) and Report ID
  uint16_t wIndex = usbHost._bInterfaceNumber; // Use the correct interface number
  uint8_t report[1] = {ledMask};       // Report data containing the LED mask
  esp_err_t err;
  bool resp = false;

  int retries = 3;
  while (retries-- > 0)
  {
    err = sendHIDReport(bmRequestType, bRequest, wValue, wIndex, report, sizeof(report));
    if (err == ESP_OK)
    {
      Serial.printf("Set LED mask: 0x%02X\n", report);
      resp = true; // Successfully set the LED state
      break;      // Exit the loop on success
    }
    else
    {
      Serial.printf("Retrying... Remaining attempts: %d\n", retries);
      delay(100); // Short delay before retrying
      resp = false; // Failed to set the LED state
    }
  }

  if (err == ESP_ERR_INVALID_STATE)
  {
    Serial.println("USB pipe is not in an active state. Resetting pipe...");
    usbHost.task(); // Perform a task to reset the pipe
    resp = false;
  }

  return resp;
}