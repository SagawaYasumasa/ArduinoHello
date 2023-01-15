#include "debugTool.h"
#include <string.h>

void dumpByte(char *memory, unsigned int size){
  int ptr = 0;
  unsigned int i = 0;
  while(i != size){
    Serial.printf("%02x",(unsigned int)memory[ptr]);

    ptr++;
    i++;
    if((i % 16) == 0){
      Serial.printf("\n");
    } else {
      Serial.printf(" ");
    }
  }
  Serial.printf("\n");
}
