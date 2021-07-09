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

#include "printHelpText.h"
#include "commandLineParameter.h"

void printHelpText(void){
  printf("Usage: compactor [options] file\n");
  printf("Options:\n");
  /*printf("  -c <arg>    | --compiler <arg>  compiler type [ACME]. Default: ACME\n");*/
  printf("  -b <file>   | --binary <file>   Binary output filename for 'bin' format.         Default: 'compactedData.bin'.\n");
  printf("  -f <arg>    | --format <arg>    Output data format [bin|hex|int|oct].            Default: 'int'.\n");
  printf("  -h          | --help            Print this help text and exit.\n");
  printf("  -n <string> | --name <string>   Compressed array name.                           Default: 'compactedData'.\n");
  printf("  -o <file>   | --outfile <file>  Output filename.                                 Default: 'compactedData.txt'.\n");
  printf("  -s <string> | --size <string>   Add array sizes to output with postfix <string>. Default: no sizes.\n");
  printf("  -t <int>    | --timeout <int>   Compaction timeout in seconds.                   Default: no timeout.\n");
  printf("  -v          | --verbose         Provide more details.\n");
  if(clVerbose == 0){
    printf ("Type 'compactor -h -v' for more information.\n");
  } else {
    printf("************************************************************************************************************************\n");
    printf("Input file format:\n");
    printf("  <array name> <opt: alignment>\n");
    printf("  <ascii type> [<value><seperator>]... | <binary type> filename [,[numberBytes] [, [skipBytes]]]\n");
    printf("  <opt padding bit mask: <ascii type> [<value><seperator>]... | <binary type> filename [,[numberBytes] [, [skipBytes]]]>\n");
    printf("  Empty or commented lines are skipped. Comment token: '/', '#', ':', ';'.\n");
    printf("\n");
    printf("Format explanation:\n");
    printf("  <opt: alignment>  Alignment to compacted array. Default: '1'.\n");
    printf("  <ascii type>      Ascii byte type keyword [!byte|!BYTE|!by|!BY|!8|!08].\n");
    printf("  <binary type>     Binary byte type keyword [!binary|!bin].\n");
    printf("  <value>           Encoding for bytes can be:\n");
    printf("                    decimal, e.g. '14' or '-128'.\n");
    printf("                    hexadecimal, e.g. '$3d' or '0x02'.\n");
    printf("                    binary, e.g. '%c0110' or '%c10001000'.\n", '%', '%');
    printf("                    octal, e.g. '&304'.\n");
    printf("                    char, e.g. %cC%c or '?'. Only charset 1 is supported.\n", '"', '"');
    printf("  <seperator>       seperator token ' ', ','.\n");
    printf("************************************************************************************************************************\n");
    printf("Input file example:\n");
    printf("-- 'unCompactedData.txt' begin ---\n");
    printf("# comment 1\n");
    printf("firstArray \n");
    printf("!byte 0, 255, -1, -128\n");
    printf("secondArray  \n");
    printf("!by 0x0, $ff\n");
    printf("thirdArray \n");
    printf("!08 %c0 %c01 %c11111111 \n", '%','%','%');
    printf("fourthArray \n");
    printf("!BYTE &0,&01,&377, \n");
    printf("; comment 2\n");
    printf("fifthArray 4\n");
    printf("!BYTE %cH%c, 'E', 'L', 'L', 'O', '!'\n", '"', '"');
    printf("-- 'unCompactedData.txt' end -----\n");
    printf("************************************************************************************************************************\n");
    printf("Call example:\n");
    printf("$ compactor -s Size unCompactedData.txt\n");
    printf("************************************************************************************************************************\n");
    printf("Output file example:\n");
    printf("-- 'compactedData.txt' begin --------------------------\n");
    printf("compactedData\n");
    printf("!byte 8, 5, 12, 12, 15, 33, 0, 255, 255, 128, 0, 1, 255\n");
    printf("firstArray = compactedData + 6\n");
    printf("firstArraySize = 4\n");
    printf("secondArray = compactedData + 6\n");
    printf("secondArraySize = 2\n");
    printf("thirdArray = compactedData + 10\n");
    printf("thirdArraySize = 3\n");
    printf("fourthArray = compactedData + 10\n");
    printf("fourthArraySize = 3\n");
    printf("fifthArray = compactedData + 0\n");
    printf("fifthArraySize = 6\n");
    printf("-- 'compactedData.txt' end ----------------------------\n");
  }
}
