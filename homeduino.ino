#include <SerialCommand.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdlib.h>

#define ONE_WIRE_BUS 12

void argument_error();

SerialCommand sCmd;

#include "rfcontrol.h"
#ifdef KEYPAD_ENABLED
#include "keypad.h"
#endif
#include "ping.h"
#include "dht.h"


void digital_read_command();
void digital_write_command();
void analog_read_command();
void analog_write_command();
void reset_command();
void pin_mode_command();
void unrecognized(const char *command);
void dst_command();
void printAddress(DeviceAddress deviceAddress);

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void setup() {
  sensors.begin();

  Serial.begin(115200);
  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("DR", digital_read_command);
  sCmd.addCommand("DW", digital_write_command);
  sCmd.addCommand("AR", analog_read_command);
  sCmd.addCommand("AW", analog_write_command);
  sCmd.addCommand("PM", pin_mode_command);
  sCmd.addCommand("RF", rfcontrol_command);    
  sCmd.addCommand("PING", ping_command);
  sCmd.addCommand("DHT", dht_command);
  sCmd.addCommand("DST", dst_command);
  sCmd.addCommand("RESET", reset_command);
#ifdef KEYPAD_ENABLED
  sCmd.addCommand("K", keypad_command);
#endif
  sCmd.setDefaultHandler(unrecognized);
  Serial.print("ready\r\n");
}

void loop() {
  // handle serial command
  sCmd.readSerial();
  // handle rf control receiving
  rfcontrol_loop();
  #ifdef KEYPAD_ENABLED
  // handle keypad keypress
  keypad_loop();
  #endif
}

void digital_read_command() {
  char* arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int pin = atoi(arg);
    int val = digitalRead(pin);
    Serial.print("ACK ");
    Serial.write('0' + val);
    Serial.print("\r\n");
}

void analog_read_command() {
  char* arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int pin = atoi(arg);
    int val = analogRead(pin);
    Serial.print("ACK ");
    Serial.print(val);
    Serial.print("\r\n");
}

void digital_write_command() {
  char* arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int pin = atoi(arg);
    arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int val = atoi(arg);
    digitalWrite(pin, val);
    Serial.print("ACK\r\n");
}

void analog_write_command() {
  char* arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int pin = atoi(arg);
    arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int val = atoi(arg);
    analogWrite(pin, val);
    Serial.print("ACK\r\n");
}

void pin_mode_command() {
  char* arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    int pin = atoi(arg);
    arg = sCmd.next();
    if(arg == NULL) {
      argument_error();
      return;
    }
    // INPUT 0x0
  // OUTPUT 0x1
    int mode = atoi(arg);
    pinMode(pin, mode);
    Serial.print("ACK\r\n");  
}

void reset_command() {
  RFControl::stopReceiving();
  Serial.print("ready\r\n");
}

void argument_error() {
  Serial.print("ERR argument_error\r\n");
}
// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(const char *command) {
  Serial.print("ERR unknown_command\r\n");
}


//DST sensors
//DST [address]
void dst_command(){

    char* arg = sCmd.next();
    if(arg == NULL) {
        argument_error();
        return;
    }

    if(strcmp(arg, "sensors") == 0) {
      DeviceAddress address;
            
      oneWire.reset_search();
      
      Serial.print("ACK sensors ");
      while(oneWire.search(address)){
        printAddress(address);
        Serial.print(" ");
      }
      Serial.print("\r\n");
    } else {      
      byte address[8];

      int i;
        
      uint8_t str_len = strlen(arg);

      Serial.print("LEN: ");
      Serial.print(str_len, 1);
      Serial.print( + "\r\n");

      memset(address, 0x00, sizeof(address));
      for (i = 0; i < ((str_len / 2)); i++) {
          sscanf(arg+2*i, "%02x", &address[i]);
      }
      printAddress(address);
      Serial.print( + "\r\n");

      double temp = sensors.getTempC(address); 

      Serial.print("ACK ");
      Serial.print(temp, 1);
      Serial.print("\r\n");
    }
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}