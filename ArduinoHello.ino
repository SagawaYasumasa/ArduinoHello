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
  unsigned long startTime;
  unsigned long finishTime;
  M5.Lcd.setCursor(0, 0);  //Set the cursor at (0,0).  将光标设置在(0,0)处
  M5.Lcd.println("Please press Btn.A to (re)scan");
  M5.update();  //Check the status of the key.  检测按键的状态

  SsidData ssidData = SsidData();

  // Buton A /////////////////////////////////////////////////////////////
  if (M5.BtnA.isPressed()) {  //If button A is pressed.  如果按键A按下
    M5.Lcd.clear();           //Clear the screen.  清空屏幕
    M5.Lcd.println("scan start");
    startTime = millis();
    int n =
//        WiFi.scanNetworks(false, false, false, 100);  //return the number of networks found. Active Scan
        WiFi.scanNetworks(false, false, true,150);  //return the number of networks found. Passive Scan
    finishTime =millis();
    if (n == 0) {  //If no network is found.  如果没有找到网络
      M5.Lcd.println("no networks found");
    } else {  //If have network is found.  找到网络
      M5.Lcd.printf("networks found:%d\n\n", n);
      for (
          int i = 0; i < n;
          ++i) {  // Print SSID and RSSI for each network found.  打印每个找到的网络的SSID和信号强度
        M5.Lcd.printf("%d:", i + 1);
        M5.Lcd.print(WiFi.SSID(i));
        M5.Lcd.printf("(%d)", WiFi.RSSI(i));
        M5.Lcd.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
        M5.Lcd.printf("Start=%d, Finish=%d, diff=%d\n",startTime,finishTime,finishTime-startTime);
        delay(10);
      }
    }
    delay(1000);
  }
  // Button C ////////////////////////////////////////////////////////////
  if (M5.BtnC.isPressed()) {  //If button C is pressed.
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
    writeRecord(0,"sems-eap","00:11:22:33:44:55",-60,2400,0.1,0.2);
    ssidData.id=2;
    strcpy(ssidData.essid,"sems-eap");
    strcpy(ssidData.bssid,"11:22:33:44:55:66");
    ssidData.rssi = -55;
    ssidData.frequency = 2400;
    ssidData.latitude = 43.058095;
    ssidData.longitude = 144.843528;
    strcpy(ssidData.datetime,"2023-01-15 10:28:10");

    M5.Lcd.printf("fileRecord=%s\n",ssidData.getFileRecord());  
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
int writeRecord(int id, char *ssid, char*bssid, long rssi, int frequency, double latitude, double longitude){
  int   ret = 0;
  File  file;
  const char* fileName = DATA_FILE_NAME;

  Serial.printf("function:writeRecord, ssid=%s\n",ssid);
  file = SD.open(fileName, FILE_APPEND);
  file.print(ssid);
  file.close();  
  Serial.printf("function:writeRecord, return(%d)\n",ret);
  return ret;
}
