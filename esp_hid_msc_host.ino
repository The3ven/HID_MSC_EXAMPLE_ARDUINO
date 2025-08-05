#include <Arduino.h>
#include "hid_msc.h"

void setup()
{
  Serial.begin(115200);

  pinMode(APP_QUIT_PIN, INPUT_PULLUP);
  pinMode(RELAY_GPIO, OUTPUT);
  pinMode(P4, OUTPUT);    // a pin short to app quit pin
  digitalWrite(P4, HIGH); // unshort the app quit pin
  Serial.println(digitalRead(APP_QUIT_PIN));
  hub_on(); // switching power of the hub ON

  app_main();
}

int last_task = -1;
bool last_status = false;


void loop()
{
  if (last_task != cusb_task.current_task || cusb_task.status != last_status)
  {
    Serial.println("====================================================");
    Serial.println("Current Task : " + String(cusb_task.current_task == usb_tasks::HID ? "HID" : "MSC"));
    Serial.println("Status : " + String(cusb_task.status ? "ON" : "OFF"));
    Serial.println("====================================================");

    last_task = cusb_task.current_task;
    last_status = cusb_task.status;

  }


  if (Serial.available() > 0)
  {
    String d = Serial.readString();
    Serial.println(d);

    if (d.indexOf("E") > -1)
    {
    try_again:

      if (cusb_task.current_task != usb_tasks::MSC)
      {

        for (int i = 0; i < 2 && !Hid_disconnected; i++)
        {

          hub_off(); // switching off power of the hub

          if (Hid_disconnected) // wait for devices to disconnect
          {
            Serial.println("HID closed " + String(__LINE__));
            break;
          }

          delay(1000); // wait for devices to disconnect

          if (!Hid_disconnected)
          {
            hub_on(); // switching power of the hub back ON for next iteration
            continue; // continue to next iteration
          }
        }

        if (!Hid_disconnected) // wait for devices to disconnect
        {
          goto try_again;
        }

        Serial.println("Setting APP Quit Pin to Low " + String(__LINE__));

        digitalWrite(P4, LOW); // short the app quit pin

        Serial.println("waiting for Reset pin to go LOW...");

        if (wait_for_app_quit(5000))
        {
          Serial.println("App Quit Detected");
        }
        else
        {
          Serial.println("App Still isn't Quitting after 5 seconds");
        }

        delay(1000);

        Serial.println("Settign APP Quit Pin to HIGH " + String(__LINE__));

        digitalWrite(P4, HIGH); // unshort the app quit pin
      }
      else
      {
        Serial.println("Setting APP Quit Pin to Low " + String(__LINE__));
        digitalWrite(P4, LOW); // short the app quit pin
        delay(100);
        hub_off();
        Serial.println("Trying to close msc task");
        delay(2000);
        Serial.println("Timeout of 2 sec");
        digitalWrite(P4, HIGH); // unshort the app quit pin
      }
    }

    if (d.indexOf("HID") > -1)
    {
      if (cusb_task.current_task != usb_tasks::HID && cusb_task.status != true)
      {
        digitalWrite(P4, HIGH); // unshort the app quit pin
        hub_on();               // switching power of the hub ON
        delay(2000);
        app_main();
      }
      else
      {
        Serial.println("HID Host is already running");
      }
    }

    if (d.indexOf("MSC") > -1)
    {
      if (cusb_task.current_task != usb_tasks::MSC && cusb_task.status != true)
      {
        digitalWrite(P4, HIGH); // unshort the app quit pin
        hub_on();               // switching power of the hub ON
        delay(2000);
        msc_setup();
      }
    }
  }
  delay(100);
}
