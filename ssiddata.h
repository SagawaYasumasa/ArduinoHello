#ifndef SsidData_h
#define SsidData_h

#include <M5Core2.h>
#include <string.h>

#define ESSID_SIZE  32
#define BSSID_SIZE  18
#define DATETIME_SIZE 19

/* filerecord structure
  000-000  1  flag(0=dead, 1=alive)
  001-001  1  delimiter(,)
  002-012 11  ID
  013-013  1  delimiter
  014-045 32  ESSID
  046-046  1  delimiter
  047-063 17  BSSID
  064-064  1  delimiter
  065-084 20  RSSI
  085-085  1  delimiter
  086-096 11  FREQUENCY
  097-097  1  delimiter
  098-114 17  LATITUDE
  115-115  1  delimiter
  116-132 17  LONGITUDE
  133-133  1  delimiter
  134-152 19  DATETIME(YYYY-MM-DD hh:mm:ss)
  153-154  2  CrLf  
*/

class SsidData {
  public:
    SsidData();
    ~SsidData();
    char *getFileRecord();
    bool  setFileRecord(char *);
    String  getJson();

    int     id;
    char  essid[ESSID_SIZE+1];
    char  bssid[BSSID_SIZE+1];
    long    rssi;
    int     frequency;
    double  latitude;
    double  longitude;
    char datetime[DATETIME_SIZE+1];
  private:
    char  _fileRecord[155+1];
    String  _json;
};
#endif