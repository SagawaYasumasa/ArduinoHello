#define MAJOR_VERSION 1
#define MINOR_VERSION 0

#include "private.h"
#define DATA_FILE_NAME  "/json.txt"

#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>

#include "ssiddata.h"
#include "debugTool.h"

static const uint32_t GPSBaud = 9600;
static const int  Font1Height = 8;      // Font=1, Adafruit 8 pixels ascii font
hw_timer_t *timer = NULL;
volatile unsigned long gTimerCounter = 0;

//Creat The TinyGPS++ object.
TinyGPSPlus gps;
// The serial connection to the GPS device.
HardwareSerial ss(2);

SsidData ssidData;


void setup() {
  M5.begin();  //Init M5Stack.
  ss.begin(GPSBaud, SERIAL_8N1, 33,32);  //It requires the use of SoftwareSerial, and assumes that you have a 4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).  

  // Init Timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer,&onTimer,true);
  timerAlarmWrite(timer,1000*1000,true);      // 1000msec interval
  timerAlarmEnable(timer);
  
  WiFi.mode(WIFI_STA);  // Set WiFi to station mode and disconnect from an AP if it was previously connected. 
  WiFi.disconnect();  //Turn off all wifi connections.
  delay(100);         //100 ms delay.
  M5.Lcd.setCursor(0,0 * Font1Height,1);
  M5.Lcd.printf("Wi-Fi Clawer for M5Stack. version %d.%02d \n",MAJOR_VERSION,MINOR_VERSION);
  M5.Lcd.printf("User name:%s\n",MYNAME);
//  ssidData.SsidData();
  delay(1000);
}

void onTimer(){
  gTimerCounter++;
}

void loop() {
  M5.Lcd.setCursor(0, 2 * Font1Height, 1);
  printGpsSatellites(gps.satellites.value(),gps.satellites.isValid());
  printGpsHdop(gps.hdop.value(),gps.hdop.isValid());  
  printGpsDateTime(gps.date,gps.time);  
  printGpsLocation(gps.location.lat(),gps.location.lng(),gps.location.isValid());

  M5.Lcd.println("Please press Btn.A to (re)scan");
  M5.update();  //Check the status of the key.

//  SsidData ssidData = SsidData();

  // Buton A /////////////////////////////////////////////////////////////
  if (M5.BtnA.isPressed()) {  //If button A is pressed.
    M5.Lcd.clear();           //Clear the screen.
    M5.Lcd.println("scan start");
    // Init Wi-Fi scan 
//   int n= WiFi.scanNetworks(false, false, false, 100);  //return the number of networks found. Active Scan
//    int n = WiFi.scanNetworks(false, false, true,150);  //return the number of networks found. Passive Scan
    WiFi.scanNetworks(true, false, true,150);     // async,passive mode

    int numberOfWifi = WIFI_SCAN_RUNNING;
    unsigned long timeout = 0;    
    // Init GPS scan
    int gpsStatus = 0;    // 0=scanning gps
    timeout = millis() + 5000; // 5000 milliseconds
    while( millis() < timeout ){
      // Scanning Wi-Fi
      if(numberOfWifi == WIFI_SCAN_RUNNING){
        numberOfWifi = WiFi.scanComplete();
      } 
      // Scanning GPS
      if(gpsStatus == 0){
        gpsStatus = 1; // temporary    
      }    
      if((numberOfWifi != WIFI_SCAN_RUNNING) && (gpsStatus !=0)){
        // WiFi & GPS scan complete, or scan failed
        break;
      }        
      M5.Lcd.printf(".");
      delay(10);
    }
    Serial.printf("\nScan result numberOfWifi=%d,gpsStatus=%d\n",numberOfWifi,gpsStatus);

    if ((numberOfWifi == WIFI_SCAN_RUNNING) ||(numberOfWifi == WIFI_SCAN_FAILED) ||(numberOfWifi == 0)) {  //If no network is found.
      M5.Lcd.println("\nno networks found");
    } else {  //If have network is found.
      String json="";
      M5.Lcd.printf("\nnetworks found:%d\n\n", numberOfWifi);
      for (int i = 0; i < numberOfWifi; ++i) {  // Print SSID and RSSI for each network found.
        M5.Lcd.printf("%d:", i + 1);
        M5.Lcd.print(WiFi.SSID(i));
        M5.Lcd.printf("(%d)", WiFi.RSSI(i));

        ssidData.id = i ;
        WiFi.SSID(i).getBytes((unsigned char *)ssidData.essid,sizeof(ssidData.essid));
        strcpy(ssidData.bssid,"00:00:00:00:00:00");
        ssidData.rssi = WiFi.RSSI(i);
        ssidData.frequency = 0;
        ssidData.latitude = 43.058095;
        ssidData.longitude = 144.843528;
        strcpy(ssidData.datetime,"2023-01-01 00:00:00");
        delay(10);
        json = json + ssidData.getJson() + ",\n";
      }
      writeRecord((char *)json.c_str());
    }
    delay(500);
  }
  // Buton B /////////////////////////////////////////////////////////////
  if (M5.BtnB.isPressed()) {  //If button B is pressed.
    M5.Lcd.clear();           //Clear the screen.
    M5.Lcd.println("dump SD " DATA_FILE_NAME "\n");
    dumpRecord();
    delay(1000);
  }
  // Button C ////////////////////////////////////////////////////////////
  if (M5.BtnC.isPressed()) {  //If button C is pressed.
    M5.Lcd.clear();           //Clear the screen.
    M5.Lcd.println("Connect to Server\n");
    Serial.printf("function:loop, ButtunC Pressed\n");
    if(connectWiFi()){
      if(connectToServer()){
        M5.Lcd.printf("success.\n");
      } else
      {
        M5.Lcd.printf("failed to connect to server.\n");
      }
    }
    disconnectWiFi();
    deleteFile();
    Serial.printf("function:loop, ButtunC Pressed-exit\n");
  }

  smartDelay(200);
  if (millis() > 5000 && gps.charsProcessed() < 10){
    M5.Lcd.println(F("No GPS data received: check wiring"));  
  }
}
// This custom version of delay() ensures that GPS objects work properly.
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available()){
      gps.encode(ss.read());
    }
  } while (millis() - start < ms);
}
// Print number of acquired satellites 
static void printGpsSatellites(int numberOfSatellites, bool valid){
  if(valid){
    M5.Lcd.printf("Number of acquired satellite(s) : %d     \n",numberOfSatellites);
  } else {
    M5.Lcd.printf("Invalid number of acquired satellite      \n");
  }
}
// Print horizontal dilution of Precision(HDOP)
static void printGpsHdop(int hdop, bool valid){
  if(valid){
    M5.Lcd.printf("Horizontal dilution of precision(HDOP) : %d     \n",hdop);
  } else {
    M5.Lcd.printf("Invalid HDOP                                     \n");
  }
}
// Print Date and Time(UTC+0000)
static void printGpsDateTime(TinyGPSDate &d, TinyGPSTime &t) {
  char tmpDate[10+1]; // "YYYY/MM/DD"
  char tmpTime[8+1];  //  "hh:mm:ss"

  memset(tmpDate,0x00,sizeof(tmpDate));
  memset(tmpTime,0x00,sizeof(tmpTime));
  if(d.isValid()){
    sprintf(tmpDate,"%04d/%02d/%02d",d.year(),d.month(),d.day());    
  } else {
    sprintf(tmpDate,"****/**/**");    
  }
  if(t.isValid()){
    sprintf(tmpTime,"%02d/%02d/%02d",t.hour(),t.minute(),t.second());    
  } else {
    sprintf(tmpTime,"**:**:**");    
  }
  M5.Lcd.printf("%s %s (UTC+0000)\n",tmpDate,tmpTime);
  smartDelay(0);
}
// Print Latitude and Longitude
static void printGpsLocation(double latitude, double longitude, bool valid){
  char  tmpLat[11+1];
  char  tmpLng[11+1];
  memset(tmpLat,0x00,sizeof(tmpLat));
  memset(tmpLng,0x00,sizeof(tmpLng));
  if(valid){
    dtostrf(latitude,11,6,tmpLat);
    dtostrf(longitude,11,6,tmpLng);
  } else {
    sprintf(tmpLat," ***.******");
    sprintf(tmpLng," ***.******");
  }
  M5.Lcd.printf("Latitude :%s\n",tmpLat);
  M5.Lcd.printf("Longitude:%s\n",tmpLng); 
}
bool connectWiFi(){
  bool ret = false;
  int i;
  
  WiFi.begin(HOME_SSID, HOME_SSID_PASS);    
  for(i=0;i<60;i++){
    delay(100);
    ret = (WiFi.status() == WL_CONNECTED);
    if(ret) break;
    M5.Lcd.printf(".");
  }
  Serial.printf("function:connectWiFi, i=%d,return(%d)\n",i,ret);
  return ret;
}
void disconnectWiFi(){
  WiFi.disconnect();
  delay(100);
  Serial.printf("function:disconnectWiFi, exit\n");
}
bool connectToServer(void){
  const char *server = SERVER_HOST "/heatmap/echo.php";
  bool ret = false;
  int responseCode;
  int responseSize;
  String responseString;
  String msg = "[{\"CLIENT\":\"ARDUINO\"}]";

  Serial.printf("function:connectToServer, server=%s\n",server);

  HTTPClient http;
  http.begin(server);
  http.addHeader("Content-Type","application/x-www-form-urlencoded");
  responseCode = http.POST(msg.c_str());
  if(responseCode == 200){
    responseSize = http.getSize();
    responseString = http.getString();
    Serial.println(responseString);
    if( responseString.compareTo(msg) == 0 ){
      //success
      ret = true;
    }   
  }
  http.end();
  Serial.printf("function:connectToServer, return(%d)\n",ret);
  return ret;

}
int writeRecord(char *record){
  int   ret = 0;
  File  file;
  const char* fileName = DATA_FILE_NAME;

  Serial.printf("function:writeRecord, ssid=%s\n",record);
  file = SD.open(fileName, FILE_APPEND);
  file.print(record);
  file.close();  
  Serial.printf("function:writeRecord, return(%d)\n",ret);
  return ret;
}
int dumpRecord(){
  const char* fileName = DATA_FILE_NAME;
  char *buf;  
  int   ptr = 0;
  String jsonString;
  File  file;

  Serial.printf("function:dumpRecord\n");
  file = SD.open(fileName, FILE_READ);
  Serial.printf("function:dumpRecord, available=%d\n",file.available());

  buf = (char *)malloc(file.size()+2);  // top='[' ,tail=0x00
  buf[ptr++] = '[';                     // start json
  while( 0 < file.available()){
    buf[ptr++] = file.read();
  }
  buf[ptr]=0x00;                        // null
  file.close();  

  jsonString = buf;
  ptr = jsonString.lastIndexOf(',');
  jsonString.setCharAt(ptr,']');        // replace last ',' to ']'. finish json
  
  Serial.println(jsonString);
  free(buf);
  Serial.printf("function:dumpRecord, return(%d)\n",jsonString.length());

  return jsonString.length();
}
bool deleteFile(){
  bool   ret = false;
  const char* fileName = DATA_FILE_NAME;

  Serial.printf("function:DeleteFile\n");
  ret = SD.remove(fileName);
  Serial.printf("function:DeleteFile, return(%d)\n",ret);
  return ret;
}
