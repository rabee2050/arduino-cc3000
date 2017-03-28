/*
  Done by TATCO Inc.
  Contacts:
  info@tatco.cc
*/

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER);
                         
#define WLAN_SSID       "Mi rabee"   // cannot be longer than 32 characters!
#define WLAN_PASS       "1231231234"
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define LISTEN_PORT           80
Adafruit_CC3000_Server httpServer(LISTEN_PORT);

char mode_action[54];
int mode_val[54];

void setup(void)
{
  Serial.begin(115200);

  while (!Serial) {
    ;
  }


#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 || i == 50 || i == 51 || i == 52 || i == 53) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
    }
  }

  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 || i == 50 || i == 51 || i == 52 || i == 53 ) {} else {
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
    }
  }
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 ) {} else {
      pinMode(i, OUTPUT);
    }
  }
#endif


  if (!cc3000.begin())
  {
    Serial.println(F("Failed"));
    while (1);
  }

  Serial.print(F("connecting to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }

  while (! displayConnectionDetails()) {
    delay(1000);
  }
  httpServer.begin();
}

void loop(void)
{
  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client.available()) {
    process(client);


  }
  delay(100);
  client.close();

}

void process(Adafruit_CC3000_ClientRef client) {

  String a = client.readStringUntil('/');
  a = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "analog") {
    analogCommand(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }


}


void digitalCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;

  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln("Content-Type: text/plain");
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  client.print(value);
   // client.close();
    
  }
}

void analogCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;

  }

}

void modeCommand(Adafruit_CC3000_ClientRef client) {
  int pin;
  pin = client.parseInt();

  String mode = client.readStringUntil(' ');
  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln("Content-Type: text/plain");
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as INPUT!"));
    //return;
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as OUTPUT!"));
    //return;
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as PWM!"));
    //return;
  }

}

void allonoff(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  value = client.parseInt();
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
    }
  }
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
    }
  }
#endif

}


void allstatus(Adafruit_CC3000_ClientRef client) {

  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln("content-type:application/json");
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  client.fastrprintln("{");

  client.fastrprint(F("\"mode\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 53) {
      client.fastrprint(F("\""));
      client.print(mode_action[i]);
      client.fastrprint(F("\""));
    }
    else {
      client.fastrprint(F("\""));
      client.print(mode_action[i]);
      client.fastrprint(F("\","));
    }
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 9; i++) {
    if (i == 9) {
      client.fastrprint(F("\""));
      client.print(mode_action[i]);
      client.fastrprint(F("\""));
    }
    else {
      client.fastrprint(F("\""));
      client.print(mode_action[i]);
      client.fastrprint(F("\","));
    }
  }
#endif
  client.fastrprintln(F("],"));

  client.fastrprint(F("\"mode_val\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 | i == 1 | i == 3 | i == 5 || i == 10 || i == 11 || i == 12 || i == 13 || i == 50 || i == 51 || i == 52 || i == 53) {
      if (i == 53) {
        client.fastrprint(F("\"x\""));
      }
      else {
        client.fastrprint(F("\"x\","));
      }

    }

    if (mode_action[i] == 'o') {
      if (i == 53) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }

    if ( mode_action[i] == 'i') {
     
      if (i == 53) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }

    if (mode_action[i] == 'p') {
      if (i == 53) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 9; i++) {
    if (i == 0 | i == 1 | i == 3 | i == 5) {
      client.fastrprint(F("\"x\","));
    }

    if (mode_action[i] == 'o') {
      if (i == 9) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }

    if ( mode_action[i] == 'i') {
     
      if (i == 9) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }

    if (mode_action[i] == 'p') {
      if (i == 9) {
        client.print(mode_val[i]);
      }
      else {
        client.print(mode_val[i]);
        client.fastrprint(F(","));
      }
    }
  }
#endif
  client.fastrprintln(F("],"));

  client.fastrprint(F("\"analog\":["));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 15; i++) {
    if (i == 15) {
      client.print(analogRead(i));
    }
    else {
      client.print(analogRead(i));
      client.fastrprint(",");
    }
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 5; i++) {
    if (i == 5) {
      client.print(analogRead(i));
    }
    else {
      client.print(analogRead(i));
      client.fastrprint(",");
    }
  }
#endif
  client.fastrprintln("],");

  client.fastrprintln(F("\"boardname\":\"cc3000\","));
  client.fastrprintln(F("\"boardstatus\":1"));
  client.fastrprintln(F("}"));
}


bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F(" IP Address failed"));
    return false;
  }
  else
  {
    Serial.print(F("IP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    //    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    //    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    //    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    //    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
