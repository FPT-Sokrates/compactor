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

static char tmpString[100];
/*
  return: 1: is a byte array
          0: no byte array
*/
unsigned int checkAndConsumeByteArray(unsigned int consumeFlag){
  if(extractString(tmpString, consumeFlag)==0){
    fprintf (stderr, "Line: %d: Column: %d no type keyword given:%s b:%s\n", getCurrentLine(), getCurrentColumn(), getString(), tmpString);
    exit(0);
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

int convertPositiveDecimal(char *str){
  char *endPtr = NULL;
  int value;
  
  value = (int)strtol(str, &endPtr, 10);
  if((value==0) &&
     ((errno==EINVAL) || (errno==ERANGE))){
    return -1;
  }
  checkRange(value);
  return value;
}

int convertNegativeDecimal(char *str){
  char *endPtr = NULL;
  int value;
  
  value = (int)strtol(str, &endPtr, 10);
  if((value==0) &&
     ((errno==EINVAL) || (errno==ERANGE))){
    return -1;
  }
  if(value>128){
    fprintf (stderr, "Line: %d Column: %d: negative value out of range [-1..-128]: -%d.\n", getCurrentLine(), getCurrentColumn(), value);
    exit(0);
  }
  /* calculate two's-complement */
  value = 256 - value;
  checkRange(value);
  return value;
}



int convertHexadecimal(char *str){
  char *endPtr = NULL;
  int value;
  
  value = (int)strtol(str, &endPtr, 16);
  if((value==0) &&
     ((errno==EINVAL) || (errno==ERANGE))){
    return -1;
  }
  checkRange(value);
  return value;
}

int convertBinary(char *str){
  char *endPtr = NULL;
  int value;
  
  value = (int)strtol(str, &endPtr, 2);
  if((value==0) &&
     ((errno==EINVAL) || (errno==ERANGE))){
    return -1;
  }
  checkRange(value);
  return value;
}

int convertOctal(char *str){
  char *endPtr = NULL;
  int value;
  
  value = (int)strtol(str, &endPtr, 8);
  if((value==0) &&
     ((errno==EINVAL) || (errno==ERANGE))){
    return -1;
  }
  checkRange(value);
  return value;
}

/* handels only charset 1 */
int convertChar(char *str){
  int value=-1;
  char c;

  c = str[0];

  /* small a-z are not part of the charset 1  */
  if((c>='a') && (c<='z')){
      fprintf (stderr, "Line: %d Column: %d: only charset 1 supported. Can not convert small char:%c.\n", getCurrentLine(), getCurrentColumn(), c);
      exit(0);
  }
  if((c>='A') && (c<='Z')){
    return (1 + ((int)c - (int)'A'));
  }
  if((c>='0') && (c<='9')){
    return (48 + ((int)c - (int)'0'));
  }
  switch (c){
    case '@':
      value=0;
      break;
    case '[':
      value=27;
      break;
    case '£':
      value=28;
      break;
    case ']':
      value=29;
      break;
    case ' ':
      value=32;
      break;
    case '!':
      value=33;
      break;
    case '"':
      value=34;
      break;
    case '#':
      value=35;
      break;
    case '$':
      value=36;
      break;
    case '%':
      value=37;
      break;
    case '&':
      value=38;
      break;
    case '´':
      value=39;
      break;
    case '(':
      value=40;
      break;
    case ')':
      value=41;
      break;
    case '*':
      value=42;
      break;
    case '+':
      value=43;
      break;
    case ',':
      value=44;
      break;
    case '-':
      value=45;
      break;
    case '.':
      value=46;
      break;
    case '/':
      value=47;
      break;
    case ':':
      value=58;
      break;
    case ';':
      value=59;
      break;
    case '<':
      value=60;
      break;
    case '=':
      value=61;
      break;
    case '>':
      value=62;
      break;
    case '?':
      value=63;
      break;

    default:
      fprintf (stderr, "Line: %d Column: %d: can not convert char '%c'.\n", getCurrentLine(), getCurrentColumn(), c);
      exit(0);
      break;
  }
  checkRange(value);
  return value;
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
     return convertChar(&tmpString[1]);
   }
  if(tmpString[0]=='$'){
    return convertHexadecimal(&tmpString[1]);
  }
  if(strlen(tmpString)>1){
    if((tmpString[1]=='x') ||
       (tmpString[1]=='X')){
      return convertHexadecimal(&tmpString[2]);
    }
  }
  if(tmpString[0]=='%'){
    return convertBinary(&tmpString[1]);
  }
  if(tmpString[0]=='&'){
    return convertOctal(&tmpString[1]);
  }
  if(tmpString[0]=='-'){
    return convertNegativeDecimal(&tmpString[1]);
  }
  if((tmpString[0]>='0') && (tmpString[0]<='9')){
    return convertPositiveDecimal(&tmpString[0]);
  } 

  return -1;
}

