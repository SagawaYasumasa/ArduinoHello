#define MAJOR_VERSION 1
#define MINOR_VERSION 0

#include "private.h"
#define DATA_FILE_NAME  "/datafile.txt"
/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit for more information: https://docs.m5stack.com/en/core/core2
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/core/core2
*
* Describe: Wifi scan.  wifi扫描
* Date: 2021/7/28
*******************************************************************************
*/
#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ssiddata.h"
#include "debugTool.h"

hw_timer_t *timer = NULL;
volatile unsigned long gTimerCounter = 0;

void setup() {
  M5.begin();  //Init M5Stack.
  // Init Timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer,&onTimer,true);
  timerAlarmWrite(timer,1000*1000,true);      // 1000msec interval
  timerAlarmEnable(timer);
  
  WiFi.mode(
      WIFI_STA);  // Set WiFi to station mode and disconnect from an AP if it was previously connected.  将WiFi设置为站模式，如果之前连接过AP，则断开连接
  WiFi.disconnect();  //Turn off all wifi connections.  关闭所有wifi连接
  delay(100);         //100 ms delay.  延迟100ms
  M5.Lcd.print("WIFI SCAN\n");  //Screen print string.  屏幕打印字符串
  M5.Lcd.printf("version %d.%02d\n",MAJOR_VERSION,MINOR_VERSION);
  Serial.printf("Hello, %s.\n",MYNAME);
  delay(1000);
}

void onTimer(){
  gTimerCounter++;
//  Serial.printf("onTimer %d\n",gTimerCounter);
}

void loop() {
  M5.Lcd.setCursor(0, 0);  //Set the cursor at (0,0).  将光标设置在(0,0)处
  M5.Lcd.println("Please press Btn.A to (re)scan");
  M5.update();  //Check the status of the key.  检测按键的状态

  SsidData ssidData = SsidData();

  // Buton A /////////////////////////////////////////////////////////////
  if (M5.BtnA.isPressed()) {  //If button A is pressed.
    M5.Lcd.clear();           //Clear the screen.
    M5.Lcd.println("scan start");
//   int n= WiFi.scanNetworks(false, false, false, 100);  //return the number of networks found. Active Scan
    int n = WiFi.scanNetworks(false, false, true,150);  //return the number of networks found. Passive Scan
    if (n == 0) {  //If no network is found.
      M5.Lcd.println("no networks found");
    } else {  //If have network is found.
      M5.Lcd.printf("networks found:%d\n\n", n);
      for (int i = 0; i < n; ++i) {  // Print SSID and RSSI for each network found.
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
//        M5.Lcd.printf("fileRecord=%s\n",ssidData.getFileRecord());  
        writeRecord(ssidData.getFileRecord());
        delay(10);
      }
    }
    delay(1000);
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
  int   ret = 0;
  File  file;
  int   filePtr = 0;
  char  c = 0x00;    
  const char* fileName = DATA_FILE_NAME;

  Serial.printf("function:dumpRecord\n");
  file = SD.open(fileName, FILE_READ);

  Serial.printf("function:dumpRecord, available=%d\n",file.available());
  Serial.printf("function:dumpRecord, size=%d\n",file.size());
  Serial.printf("function:dumpRecord, position=%d\n",file.position());
  /*
  while(filePtr < file.available()){
    c = file.read();
    if( c == -1) break;
    Serial.write(c);    
    filePtr++;
  }  
  */
  while( 0 < file.available()){
    c = file.read();
    if( c == -1) break;
    Serial.write(c);    
  }  
  
 // file.print(record);
  file.close();  
  Serial.printf("function:dumpRecord, return(%d)\n",ret);


  return ret;
}
bool deleteFile(){
  bool   ret = false;
  const char* fileName = DATA_FILE_NAME;

  Serial.printf("function:DeleteFile\n");
  ret = SD.remove(fileName);
  Serial.printf("function:DeleteFile, return(%d)\n",ret);
  return ret;
}
