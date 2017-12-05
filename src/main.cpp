#include <Arduino.h>
#include <SPI.h>
#include <WiFi101.h>
#include <SD.h>
#include <KeyboardController.h>
#include <U8g2lib.h>

// Initialize USB Controller
USBHost usb;
// Attach Keyboard controller to USB
KeyboardController keyboard(usb);

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ 1, /* data=*/ 0, /* cs=*/ 2, /* dc=*/ 11, /* reset=*/ 5);

char* wifiConnect;

void printWiFiStatus();
void keyPressed();
void displayID();
void sendToApiary();
void writeToSD();

File config;
File writeData;

//for wifi
char ssid[30];        // your network SSID (name)
char pass[30];    // your network password (use for WPA, or use as key for WEP)
char server[30];
char url[30];
char apiToken[33];
WiFiClient client;

//for reading sd card
String* buffer = new String[30];
int readLine = 0;

//for reading buzzcards
int key;
char str[10];
char gtid[10];
int counter = 0;

void setup() {
  //Initialize LCD
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_6x12_tr);
  gtid[9] = '\0';

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  WiFi.setPins(8, 3, 4, 9);
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  config = SD.open("config.txt", FILE_READ);

  if (config) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (config.available()) {
      buffer[readLine] = config.readStringUntil('\n');
      readLine++;
      // Serial.println(line);
      // Serial.write(config.read());
    }
    if (readLine < 5) {
      Serial.println("too few arguments in configuration file");
    } else {
      buffer[0].toCharArray(ssid, 30);
      buffer[1].toCharArray(pass, 30);
      buffer[2].toCharArray(server, 30);
      buffer[3].toCharArray(url, 30);
      buffer[4].toCharArray(apiToken, 33);
    }
    // close the file:
    config.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // attempt to connect to WiFi network:
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(100);
  }
  wifiConnect = "Connected to WiFi!";
  printWiFiStatus();

//  u8g2.clearBuffer();          // clear the internal memory
//  u8g2.drawStr(0,10,wifiConnect); // write something to the internal memory
//  u8g2.drawStr(0,30,"Hello world!");
//  u8g2.sendBuffer();         // transfer internal memory to the display
  delay(1000);

  u8g2.clearBuffer();
  u8g2.drawStr(0,10,"Swipe Card");
  u8g2.sendBuffer();

}

void loop() {

  usb.Task();

}

void printWiFiStatus() {

  u8g2.clearBuffer();
  u8g2.drawStr(0,10,WiFi.SSID());

//  IPAddress ip = WiFi.localIP();
//  u8g2.drawStr(0,10,ip);

  long rssi = WiFi.RSSI();
  char rssiStr[30];
  itoa(rssi, rssiStr, 30);
  u8g2.drawStr(0,30,rssiStr);

  u8g2.sendBuffer();

//  // print the SSID of the network you're attached to:
//  Serial.print("SSID: ");
//  Serial.println(WiFi.SSID());
//
//  // print your WiFi shield's IP address:
//  IPAddress ip = WiFi.localIP();
//  Serial.print("IP Address: ");
//  Serial.println(ip);
//
//  // print the received signal strength:
//  long rssi = WiFi.RSSI();
//  Serial.print("signal strength (RSSI):");
//  Serial.print(rssi);
//  Serial.println(" dBm");
}

void keyPressed(){
  if (counter < 9) {
    key = keyboard.getKey() - 48;
    itoa(key, str, 10);
    gtid[counter] = *str;
    counter++;
  } else {
    displayID();
    writeToSD();
    delay(500);
//    sendToApiary();

    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"done");
    u8g2.drawStr(0,20,"ready for new card");
    u8g2.sendBuffer();
//    delay(2000);

    counter = 0;
  }
}

void displayID(){
  u8g2.clearBuffer();         // clear the internal memory
//  u8g2.print(str);  // write something to the internal memory
  u8g2.drawStr(0,10,gtid);
  u8g2.sendBuffer();

}

void sendToApiary(){
//    u8g2.clearBuffer();
//    u8g2.drawStr(0,10,"connecting to");
//    u8g2.drawStr(0,20,server);
//    u8g2.sendBuffer();
//    delay(2000);

//    int result = client.connect(server, 80);
//    char out[5];
//    itoa(result, out, 5);

//    u8g2.clearBuffer();
//    u8g2.drawStr(0,20,out);
//    u8g2.sendBuffer();
//    delay(2000);

  if (client.connect(server, 80)) { //TODO: change this back to client.connect(server,80)
    Serial.println("connected to server");
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"connected to server");
    u8g2.sendBuffer();
    // Make a HTTP request:
    client.print("POST ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Content-Type: application/json");
    client.print("Authorization: Bearer ");
    client.println(apiToken);
    client.println("Cache-Control: no-cache");
    client.print("{\"attendable_type\":\"App\\Event\",\"attendable_id\":\"1\",\"gtid\":");
    client.print(gtid);
    client.println(",\"source\":\"swipe\"}");
    client.println();

    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"request sent");
    u8g2.sendBuffer();
    delay(1000);
  } else {
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"request failed");
    u8g2.sendBuffer();
    delay(2000);
  }

  int buffcounter = 0;
  char response[30];
  while (client.available()) {
    char c = client.read();
    response[buffcounter] = c;
    buffcounter++;
  }
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,response);
    u8g2.sendBuffer();
//    Serial.write(c);

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
}

void writeToSD(){
  writeData = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (writeData) {
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"Writing to SD...");
    u8g2.sendBuffer();
    writeData.write(gtid);
    writeData.write('\n');
    // close the file:
    writeData.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    u8g2.clearBuffer();
    u8g2.drawStr(0,10,"error opening test.txt");
    u8g2.sendBuffer();
  }
}
