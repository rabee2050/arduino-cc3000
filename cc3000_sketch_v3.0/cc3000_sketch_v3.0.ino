/*
  Done by TATCO Inc.

  Contacts:
  info@tatco.cc

  Release Notes:
  - V1 Created 23 Feb 2016
  - V2 Updated 14 May 2016
  - V3 Updated 11 Nov 2017


  tested on:
  1- Uno
  2- Leonardo
  3- Mega
*/

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Servo.h>
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

#define WLAN_SSID       "Mi rabee"   //  WIFI SSID
#define WLAN_PASS       "1231231234" //  WIFI Password
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define LISTEN_PORT           80
Adafruit_CC3000_Server httpServer(LISTEN_PORT);


#define lcd_size 3 //this will define number of LCD on the phone app
int refresh_time = 15; //the data will be updated on the app every 15 seconds.

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
Servo myServo[54];
char mode_action[54];
int mode_val[54];
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leonardo or UNO
Servo myServo[14];
char mode_action[14];
int mode_val[14];
#endif

String lcd[lcd_size];

unsigned long last_ip = millis();
String httpOk;

void setup(void)
{
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  boardInit();

  if (!cc3000.begin())
  {
    Serial.println(F("Failed"));
    while (1);
  }

  Serial.print(F("connecting to: ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

  Serial.println(F("Request DHCP..."));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }

  while (! displayConnectionDetails()) {
    delay(1000);
  }
  httpServer.begin();
  httpOk = F("HTTP/1.1 200 OK\r\n Content-Type: text/plain \r\n\r\n");
}

void loop(void)
{
  lcd[0] = F("Test 1 LCD");// you can send any data to your mobile phone.
  lcd[1] = F("Test 2 LCD");// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1

  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client.available()) {
    update_input();
    process(client);

  }
  delay(100);
  client.close();

}

void process(Adafruit_CC3000_ClientRef client) {

  String getString = client.readStringUntil('/');
  String arduinoString = client.readStringUntil('/');
  String command = client.readStringUntil('/');

  if (command == F("terminal")) {
    terminalCommand(client);
  }

  if (command == F("digital")) {
    digitalCommand(client);
  }

  if (command == F("analog")) {
    analogCommand(client);
  }

  if (command == F("servo")) {
    servo(client);
  }

  if (command == F("mode")) {
    modeCommand(client);
  }

  if (command == F("allonoff")) {
    allonoff(client);
  }

  if (command == F("refresh")) {
    refresh(client);
  }

  if (command == F("allstatus")) {
    allstatus(client);
  }


}

void terminalCommand(Adafruit_CC3000_ClientRef client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  Serial.println(data);
  client.print(httpOk);

}

void digitalCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.print(httpOk);
    client.print(value);
  }
}

void analogCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    client.print(httpOk);
    client.print(value);
  }

}

void servo(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.print(httpOk);
    client.print(value);
  }
}

void modeCommand(Adafruit_CC3000_ClientRef client) {
  int pin = client.parseInt();
  String mode = client.readStringUntil(' ');
  client.print(httpOk);
  client.fastrprintln(F(""));
  myServo[pin].detach();
  if (mode == F("/input")) {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    mode_val[pin] = 0;
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as INPUT!"));
    digitalWrite(pin, LOW);
  }

  if (mode == F("/output")) {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    mode_val[pin] = 0;
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as OUTPUT!"));
    digitalWrite(pin, LOW);
  }

  if (mode == F("/pwm")) {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    mode_val[pin] = 0;
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as PWM!"));
    digitalWrite(pin, LOW);
  }

  if (mode == F("/servo")) {
    myServo[pin].attach(pin);
    mode_action[pin] = 's';
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as SERVO!"));
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
  client.print(httpOk);
  client.print(value);
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
    }
  }
  client.print(httpOk);
  client.print(value);
#endif

}

void refresh(Adafruit_CC3000_ClientRef client) {
  int value;
  value = client.parseInt();
  refresh_time = value;
  client.print(httpOk);
  client.print(value);

}

void allstatus(Adafruit_CC3000_ClientRef client) {
  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln("content-type:application/json");
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  client.print(F("{"));
  client.print(F("\"m\":["));//m for Pin Mode
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 13)client.print(F(","));
  }
#endif
  client.print(F("],"));

  client.print(F("\"v\":["));// v for Mode value
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(mode_val[i]);
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(mode_val[i]);
    if (i != 13)client.print(F(","));
  }
#endif
  client.print(F("],"));

  client.print(F("\"a\":["));// a For Analog
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 15; i++) {
    client.print(analogRead(i));
    if (i != 15)client.print(F(","));

  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 5; i++) {
    client.print(analogRead(i));
    if (i != 5)client.print(F(","));
  }
#endif
  client.print("],");

  client.print(F("\"l\":["));// // l for LCD
  for (byte i = 0; i <= lcd_size - 1; i++) {
    client.print(F("\""));
    client.print(lcd[i]);
    client.print(F("\""));
    if (i != lcd_size - 1)client.print(F(","));
  }
  client.print(F("],"));
  client.print(F("\"t\":\""));//t for refresh Time .
  client.print(refresh_time);
  client.print(F("\""));
  client.print(F("}"));


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
    Serial.println();
    return true;
  }
}

void boardInit() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5  || i == 10  || i == 50 || i == 51 || i == 52 || i == 53) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
    }
  }

  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5  || i == 10  || i == 50 || i == 51 || i == 52 || i == 53 ) {} else {
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 ) {
      mode_action[i] = 'x';
      mode_val[i] = 0;
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

}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}
