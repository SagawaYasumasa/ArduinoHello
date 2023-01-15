#include "ssiddata.h"
#include "debugTool.h"
#include <string.h>

// Constructor //////////////////////////////////////////////////////////////
SsidData::SsidData(){
  id = 0;
  memset(essid,0x00,sizeof(essid));
  memset(bssid,0x00,sizeof(bssid));
  rssi = 0;
  frequency = 0;
  latitude = 0.0;
  longitude = 0.0;
  memset(datetime,0x00,sizeof(datetime));
  memset(_fileRecord,0x00,sizeof(_fileRecord));
  _json = "";
}
// Destructor ///////////////////////////////////////////////////////////////
SsidData::~SsidData(){

}

String SsidData::getJson(){
  char tmp[200];
  char tmpLatitude[11+1]="";
  char tmpLongitude[11+1]="";

  dtostrf(latitude,11,6,tmpLatitude);
  dtostrf(longitude,11,6,tmpLongitude);
  sprintf(tmp,"{\"ID\":\"%d\",\"ESSID\":\"%s\",\"BSSID\":\"%s\",\"RSSI\":\"%d\",\"FREQUENCY\":\"%d\",\"LATITUDE\":\"%s\",\"LONGITUDE\":\"%s\",\"DATETIME\":\"%s\"}"
          ,id,essid,bssid,rssi,frequency,tmpLatitude,tmpLongitude,datetime);
  _json = tmp;
  return _json;
}

char *SsidData::getFileRecord(){
  //Build fileRecord
  char tmpId[11+1]="";
  char tmpRssi[20+1]="";
  char tmpFrequency[11+1]="";
  char tmpLatitude[17+1]="";
  char tmpLongitude[17+1]="";
  memset(_fileRecord,0x20,sizeof(_fileRecord)-1);
  _fileRecord[sizeof(_fileRecord)-1] = '\n';

  sprintf(tmpId,"%+011d",id);
  sprintf(tmpRssi,"%+020ld",rssi);
  sprintf(tmpFrequency,"%+011d",frequency);
  dtostrf(latitude,17,12,tmpLatitude);
  dtostrf(longitude,17,12,tmpLongitude);
  
  _fileRecord[0]='1';                // alive record
  _fileRecord[1]=',';
  memcpy(&_fileRecord[2],tmpId,strlen(tmpId));
  _fileRecord[13]=',';
  memcpy(&_fileRecord[14],essid,strlen(essid));
  _fileRecord[46]=',';
  memcpy(&_fileRecord[47],bssid,strlen(bssid));
  _fileRecord[64]=',';
  memcpy(&_fileRecord[65],tmpRssi,strlen(tmpRssi));
  _fileRecord[85]=',';
  memcpy(&_fileRecord[86],tmpFrequency,strlen(tmpFrequency));
  _fileRecord[97]=',';
  memcpy(&_fileRecord[98],tmpLatitude,strlen(tmpLatitude));
  _fileRecord[115]=',';
  memcpy(&_fileRecord[116],tmpLongitude,strlen(tmpLongitude));
  _fileRecord[133]=',';
  memcpy(&_fileRecord[134],datetime,strlen(datetime));
  _fileRecord[153]=0x0d;
  _fileRecord[154]=0x0a;
  _fileRecord[155]=0x00;
//  dumpByte(_fileRecord,sizeof(_fileRecord));
  Serial.printf("function:SsidData::getFileRecord, _fileRecord=%s\n",_fileRecord);

  return &_fileRecord[0];
}

bool  SsidData::setFileRecord(char *record){
  //parse fileRecord
  id = 0;
  return true;
}