#ifndef HID_MSC_H_
#define HID_MSC_H_

#include <Arduino.h>
#include <src/hid_msc/esp_hid_msc_host.h>
#include "USB_Keyboard.h"
/* GPIO Pin number for quit from example logic */

#define P1 46
#define P2 45
#define P4 41
#define P6 39
#define P7 38
#define P8 37
#define P9 36
#define P10 35
#define P11 48
#define P12 15

#define RELAY_GPIO P1
// #define APP_QUIT_PIN GPIO_NUM_0
#define APP_QUIT_PIN P2


extern QueueHandle_t hid_host_event_queue;
extern bool user_shutdown;
extern bool Hid_disconnected;
extern const char *TAG;
extern bool current_switch_state;


struct usb_tasks
{
    enum
    {
        HID,
        MSC
    };
    uint8_t current_task;
    bool status;
};

extern usb_tasks cusb_task;

/**
   @brief HID Host event

   This event is used for delivering the HID Host event from callback to a task.
*/
typedef struct
{
    hid_host_device_handle_t hid_device_handle;
    hid_host_driver_event_t event;
    void *arg;
} hid_host_event_queue_t;

/**
   @brief HID Protocol string names
*/
static const char *hid_proto_name_str[] = {"NONE", "KEYBOARD", "MOUSE"};

/**
   @brief Key event
*/
typedef struct
{
    enum key_state
    {
        KEY_STATE_PRESSED = 0x00,
        KEY_STATE_RELEASED = 0x01
    } state;
    uint8_t modifier;
    uint8_t key_code;
} key_event_t;

/* Main char symbol for ENTER key */
#define KEYBOARD_ENTER_MAIN_CHAR '\r'
/* When set to 1 pressing ENTER will be extending with LineFeed during serial
   debug output */
#define KEYBOARD_ENTER_LF_EXTEND 1

/**
   @brief Scancode to ascii table
*/
const uint8_t keycode2ascii[57][2] = {
    {0, 0},                                               /* HID_KEY_NO_PRESS        */
    {0, 0},                                               /* HID_KEY_ROLLOVER        */
    {0, 0},                                               /* HID_KEY_POST_FAIL       */
    {0, 0},                                               /* HID_KEY_ERROR_UNDEFINED */
    {'a', 'A'},                                           /* HID_KEY_A               */
    {'b', 'B'},                                           /* HID_KEY_B               */
    {'c', 'C'},                                           /* HID_KEY_C               */
    {'d', 'D'},                                           /* HID_KEY_D               */
    {'e', 'E'},                                           /* HID_KEY_E               */
    {'f', 'F'},                                           /* HID_KEY_F               */
    {'g', 'G'},                                           /* HID_KEY_G               */
    {'h', 'H'},                                           /* HID_KEY_H               */
    {'i', 'I'},                                           /* HID_KEY_I               */
    {'j', 'J'},                                           /* HID_KEY_J               */
    {'k', 'K'},                                           /* HID_KEY_K               */
    {'l', 'L'},                                           /* HID_KEY_L               */
    {'m', 'M'},                                           /* HID_KEY_M               */
    {'n', 'N'},                                           /* HID_KEY_N               */
    {'o', 'O'},                                           /* HID_KEY_O               */
    {'p', 'P'},                                           /* HID_KEY_P               */
    {'q', 'Q'},                                           /* HID_KEY_Q               */
    {'r', 'R'},                                           /* HID_KEY_R               */
    {'s', 'S'},                                           /* HID_KEY_S               */
    {'t', 'T'},                                           /* HID_KEY_T               */
    {'u', 'U'},                                           /* HID_KEY_U               */
    {'v', 'V'},                                           /* HID_KEY_V               */
    {'w', 'W'},                                           /* HID_KEY_W               */
    {'x', 'X'},                                           /* HID_KEY_X               */
    {'y', 'Y'},                                           /* HID_KEY_Y               */
    {'z', 'Z'},                                           /* HID_KEY_Z               */
    {'1', '!'},                                           /* HID_KEY_1               */
    {'2', '@'},                                           /* HID_KEY_2               */
    {'3', '#'},                                           /* HID_KEY_3               */
    {'4', '$'},                                           /* HID_KEY_4               */
    {'5', '%'},                                           /* HID_KEY_5               */
    {'6', '^'},                                           /* HID_KEY_6               */
    {'7', '&'},                                           /* HID_KEY_7               */
    {'8', '*'},                                           /* HID_KEY_8               */
    {'9', '('},                                           /* HID_KEY_9               */
    {'0', ')'},                                           /* HID_KEY_0               */
    {KEYBOARD_ENTER_MAIN_CHAR, KEYBOARD_ENTER_MAIN_CHAR}, /* HID_KEY_ENTER */
    {0, 0},                                               /* HID_KEY_ESC             */
    {'\b', 0},                                            /* HID_KEY_DEL             */
    {0, 0},                                               /* HID_KEY_TAB             */
    {' ', ' '},                                           /* HID_KEY_SPACE           */
    {'-', '_'},                                           /* HID_KEY_MINUS           */
    {'=', '+'},                                           /* HID_KEY_EQUAL           */
    {'[', '{'},                                           /* HID_KEY_OPEN_BRACKET    */
    {']', '}'},                                           /* HID_KEY_CLOSE_BRACKET   */
    {'\\', '|'},                                          /* HID_KEY_BACK_SLASH      */
    {'\\', '|'},
    /* HID_KEY_SHARP           */ // HOTFIX: for NonUS Keyboards repeat
    // HID_KEY_BACK_SLASH
    {';', ':'},  /* HID_KEY_COLON           */
    {'\'', '"'}, /* HID_KEY_QUOTE           */
    {'`', '~'},  /* HID_KEY_TILDE           */
    {',', '<'},  /* HID_KEY_LESS            */
    {'.', '>'},  /* HID_KEY_GREATER         */
    {'/', '?'}   /* HID_KEY_SLASH           */
};

void hub_off();
void hub_on();
void msc_setup(void);
void app_main(void);
bool wait_for_app_quit(long int timeout);
esp_err_t sendHIDReport(uint8_t led_state);

#endif