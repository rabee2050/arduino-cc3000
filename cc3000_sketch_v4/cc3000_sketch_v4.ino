/*
  Done by TATCO Inc.

  Contacts:
  info@tatco.cc

  Release Notes:
  - V1 Created 23 Feb 2016
  - V2 Updated 14 May 2016
  - V3 Updated 11 Nov 2017
  - V4 Updated 22 Oct 2017


  Tested on CC3000 WiFi Shield with:
  1- Uno
  2- Leonardo
  3- Mega
*/

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <Servo.h>
#include "utility/debug.h"
#include "utility/socket.h"

#define WLAN_SSID       "Mi rabee"   //  WIFI SSID
#define WLAN_PASS       "1231231234" //  WIFI Password
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define LISTEN_PORT           80

#define ADAFRUIT_CC3000_IRQ   3 
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER);// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
                         
Adafruit_CC3000_Server httpServer(LISTEN_PORT);

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
Servo servoArray[54];
char pinsMode[54];
int pinsValue[54];
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)//Leonardo or UNO
Servo servoArray[14];
char pinsMode[14];
int pinsValue[14];
#endif

#define lcdSize 3 //this will define number of LCD on the phone app
String lcd[lcdSize];

byte digitalArraySize, analogArraySize;
unsigned long serialTimer = millis();
char* protectionPassword = "";
char* boardType;
String httpOk;

void setup(void)
{
  Serial.begin(115200);
  if (!cc3000.begin())
  {
    Serial.print(F("."));
    while (1);
  }

  Serial.print(F("connecting to: ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }

  while (! displayConnectionDetails()) {
    delay(1000);
  }
  httpServer.begin();
  boardInit();
  httpOk = F("HTTP/1.1 200 OK\r\n Content-Type: text/plain \r\n\r\n");
}

void loop(void)
{
  lcd[0] = "Test 1 LCD";// Send any data to the LCD dashboard in the App.
  lcd[1] = analogRead(A0);// Send analog value of A0 to the LCD dashboard in the App.
  lcd[2] = random(1, 100);// Send any data to the LCD dashboard in the App.

  Adafruit_CC3000_ClientRef client = httpServer.available();
  if (client) {
    if (client.available()) {
      update_input();
      process(client);

    }
    delay(100);
    client.close();
  }
  serialPrintIpAddress();
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

  if (command == F("pwm")) {
    pwmCommand(client);
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

  if (command == F("password")) {
    changePassword(client);
  }

  if (command == F("allstatus")) {
    allstatus(client);
  }
}

void terminalCommand(Adafruit_CC3000_ClientRef client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  Serial.println(data);
//  client.print(httpOk+"Ok from Arduino");

}

void changePassword(Adafruit_CC3000_ClientRef client) {
  String data = client.readStringUntil('/');
//  protectionPassword = char* data;
  data.toCharArray(protectionPassword, sizeof(data));
  Serial.println(data);
  client.print(httpOk);
}

void digitalCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpOk + value);
  }
}

void pwmCommand(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    pinsValue[pin] = value;
    client.print(httpOk + value);
  }

}

void servo(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    servoArray[pin].write(value);
    pinsValue[pin] = value;
    client.print(httpOk + value);
  }
}

void modeCommand(Adafruit_CC3000_ClientRef client) {
  String  pinString = client.readStringUntil('/');
  int pin = pinString.toInt();
  String mode = client.readStringUntil('/');
  if (mode != F("servo")) {
    servoArray[pin].detach();
  };

  if (mode == F("output")) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    pinsMode[pin] = 'o';
    pinsValue[pin] = 0;
    allstatus(client);
  }
  if (mode == F("push")) {
    pinsMode[pin] = 'm';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }
  if (mode == F("schedule")) {
    pinsMode[pin] = 'c';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
    allstatus(client);
  }

  if (mode == F("input")) {
    pinsMode[pin] = 'i';
    pinsValue[pin] = 0;
    pinMode(pin, INPUT);
    allstatus(client);
  }

  if (mode == F("pwm")) {
    pinsMode[pin] = 'p';
    pinsValue[pin] = 0;
    pinMode(pin, OUTPUT);
    analogWrite(pin, 0);
    allstatus(client);
  }

  if (mode == F("servo")) {
    pinsMode[pin] = 's';
    pinsValue[pin] = 0;
    servoArray[pin].attach(pin);
    servoArray[pin].write(0);
    allstatus(client);
  }
}

void allonoff(Adafruit_CC3000_ClientRef client) {
  int pin, value;
  value = client.parseInt();
  client.print(httpOk + value);
  for (byte i = 0; i <= digitalArraySize; i++) {
    if (pinsMode[i] == 'o') {
      digitalWrite(i, value);
    }
  }


}


void allstatus(Adafruit_CC3000_ClientRef client) {


  client.fastrprintln(F("HTTP/1.1 200 OK"));
  client.fastrprintln(F("content-type:application/json"));
  client.fastrprintln(F("Connection: close"));
  client.fastrprintln(F("Server: Adafruit CC3000"));
  client.fastrprintln(F(""));
  client.fastrprint(F("{"));

  client.fastrprint(F("\"m\":["));//m for Pin Mode
  for (byte i = 0; i <= digitalArraySize; i++) {
    client.fastrprint(F("\""));
    client.print(pinsMode[i]);
    client.fastrprint(F("\""));
    if (i != digitalArraySize)client.fastrprint(F(","));
  }
  client.fastrprint(F("],"));

  client.fastrprint(F("\"v\":["));// v for Mode value
  for (byte i = 0; i <= digitalArraySize; i++) {
    client.print(pinsValue[i]);
    if (i != digitalArraySize)client.fastrprint(F(","));
  }
  client.fastrprint(F("],"));

  client.fastrprint(F("\"a\":["));// a For Analog
  for (byte i = 0; i <= analogArraySize; i++) {
    client.print(analogRead(i));
    if (i != analogArraySize)client.fastrprint(F(","));
  }
  client.fastrprint(F("],"));

  client.fastrprint(F("\"l\":["));// // l for LCD
  for (byte i = 0; i <= lcdSize - 1; i++) {
    client.fastrprint(F("\""));
    client.print( lcd[i]);
    client.fastrprint(F("\""));
    if (i != lcdSize - 1)client.fastrprint(F(","));
  }
  client.fastrprint(F("],"));
  client.fastrprint(F("\"p\":\""));//p for protection password .
  client.fastrprint(protectionPassword);//
  client.fastrprint(F("\","));
  client.fastrprint(F("\"t\":\""));//t for board type .
  client.fastrprint(boardType);
  client.fastrprint(F("\""));
  client.fastrprint(F("}"));
}


bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("IP Fail"));
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
      pinsMode[i] = F('x');
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = F('o');
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1 || i == 3 || i == 5 || i == 10 || i == 11 || i == 12 || i == 13 ) {
      pinsMode[i] = 'x';
      pinsValue[i] = 0;
    }
    else {
      pinsMode[i] = 'o';
      pinsValue[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  boardType = "uno";
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__) || defined(__SAM3X8E__)
  boardType = "mega";
#elif defined(__AVR_ATmega32U4__)
  boardType = "leo";
#else
  boardType = "uno";
#endif

  if (boardType == "mega") {
    digitalArraySize = 53;
    analogArraySize = 15;
  } else {
    digitalArraySize = 13;
    analogArraySize = 5;
  }
}

void update_input() {
  for (byte i = 0; i <= digitalArraySize; i++) {
    if (pinsMode[i] == 'i') {
      pinsValue[i] = digitalRead(i);
    }
  }
}

void serialPrintIpAddress() {
  if (Serial.read() > 0) {
    if (millis() - serialTimer > 2000) {
      displayConnectionDetails();
    }
    serialTimer = millis();
  }
}
