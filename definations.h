#ifndef DEFINATION_H_
#define DEFINATION_H_

// #if defined (ESP32_S3) && defined (ENABLE_USB_KEYBOARD)

#include <Arduino.h>

// Modifiers and Special Keys
#define BACK_KEY 8               // BACKSPACE
#define TAB 9                    // TAB Key
#define ENTER 13                 // ENTER Key
#define MAIN_MENU_ESC 27         // Esc
#define DEL 127                  // DELETE Key
#define MISC ENTER               // MISC Key
#define SPACEBAR 32              // SPACEBAR Key
#define ESCAPE_KEY MAIN_MENU_ESC // Escape Key

#define NUMPAD_UP 256
#define NUMPAD_DOWN 250
#define NUMPAD_LEFT 254
#define NUMPAD_RIGHT 252

#define LED_NUM_LOCK 0x01
#define LED_CAPS_LOCK 0x02
#define LED_SCROLL_LOCK 0x04

// Navigation Keys
#define HOME_KEY 202    // Home Key
#define UP_ARROW 203    // Arrow Up Key
#define PAGE_UP 204     // Page Up Key
#define LEFT_ARROW 205  // Arrow Left Key
#define RIGHT_ARROW 206 // Arrow Right Key
#define END_KEY 207     // End Key
#define DOWN_ARROW 208  // Arrow Down Key
#define PAGE_DOWN 209   // Arrow Down Key
#define INSERT_KEY 210  // Insert Key

// Modifier Keys
#define LEFT_CONTROL 229  // Left Control Key
#define LEFT_ALT 256      // Left Alt Key
#define RIGHT_CONTROL 297 // Right Control Key
#define RIGHT_ALT 298     // Right Alt Key
#define LEFT_SHIFT 242    // Left Shift Key
#define RIGHT_SHIFT 254   // Right Shift Key

// Function Keys (Standard USB HID Usage IDs)
#define F1 128       // F1 Key
#define F2 129       // F2 Key
#define F3 130       // F3 Key
#define F4 131       // F4 Key
#define F5 132       // F5 Key
#define F6 133       // F6 Key
#define F7 134       // F7 Key
#define F8 135       // F8 Key
#define F9 136       // F9 Key
#define F10 137      // F10 Key
#define F11 138      // F11 Key
#define F12 139      // F12 Key
#define CTRL_F2 140  // F2 with CTRL (handled via modifier byte)
#define SHIFT_F2 141 // F2 with SHIFT (handled via modifier byte)

#endif