/*
The MIT License

Copyright (c) 2021 Steffen Görzig

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* SPDX-License-Identifier: MIT */

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "readInputByte.h"
#include "readInputFile.h"
#include "stringHandling.h"

static char tmpString[100];
/*
  return: 1: is a byte array
          0: no byte array
*/
unsigned int checkAndConsumeByteArray(unsigned int consumeFlag){
  if(extractString(tmpString, consumeFlag)==0){
    return 0;
  }

  /* check for byte keywords: "!byte", "!BYTE", "!by", "!BY", "!8", "!08"  */
  if(strncmp(tmpString, "!byte", 5)==0){
    return 1;
  }
  if(strncmp(tmpString, "!BYTE", 5)==0){
    return 1;
  }
  if(strncmp(tmpString, "!by", 3)==0){
    return 1;
  }
  if(strncmp(tmpString, "!BY", 3)==0){
    return 1;
  }
  if(strncmp(tmpString, "!8", 2)==0){
    return 1;
  }
  if(strncmp(tmpString, "!08", 3)==0){
    return 1;
  }
  return 0;
}

/*
  return: 1: is a byte array
          0: no byte array
*/
unsigned int isByteArray(void){
  return checkAndConsumeByteArray(0);
}


void consumeKeywordByte(void){
  checkAndConsumeByteArray(1);
}

void checkRange(int value){
  if ((value<0) || (value>255)) {
    fprintf (stderr, "Line: %d Column: %d: Value out of range [0..255]: %d.\n", getCurrentLine(), getCurrentColumn(), value);
    exit(0);
  }
}

/* 
   encoding for bytes can be:
   decimal     (e.g. 14 or -1)
   hexadecimal (e.g. $3d or 0x02)
   binary      (e.g. %0110 or %10001000)
   octal       (e.g. &304)
   char        (e.g. "c" or ';')
 
   return:   -1: no next value
           else: byte value
*/
int getNextValueByte(void){
  int value;
  
  if (isEndOfLine()==1){
    return -1;
  }

  if(extractNextNumberAsString(tmpString, 1)==1){
    return -1;
  }

  if(((int)tmpString[0]==39) || /* sign ' */
     (tmpString[0]=='"')){
     if(strlen(tmpString)!=3){
       fprintf (stderr, "Line: %d Column: %d: Wrong character value: %s.\n", getCurrentLine(), getCurrentColumn(), tmpString);
       exit(0);
     }
     value = convertChar(&tmpString[1]);
     checkRange(value);
     return value;
   }
  if(tmpString[0]=='$'){
    value = convertHexadecimal(&tmpString[1]);
    checkRange(value);
    return value;
  }
  if(strlen(tmpString)>1){
    if((tmpString[1]=='x') ||
       (tmpString[1]=='X')){
      value = convertHexadecimal(&tmpString[2]);
      checkRange(value);
      return value;
    }
  }
  if(tmpString[0]=='%'){
    value = convertBinary(&tmpString[1]);
    checkRange(value);
    return value;
  }
  if(tmpString[0]=='&'){
    value = convertOctal(&tmpString[1]);
    checkRange(value);
    return value;
  }
  if(tmpString[0]=='-'){
    value = convertNegativeDecimal(&tmpString[1]);
    checkRange(value);
    return value;
  }
  if((tmpString[0]>='0') && (tmpString[0]<='9')){
    value = convertPositiveDecimal(&tmpString[0]);
    checkRange(value);
    return value;
  } 

  return -1;
}

