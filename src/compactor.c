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

/* printf() */
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */

#include "compactor.h"
#include "commandLineParameter.h"
#include "readInput.h"
#include "writeOutputFile.h"

tRoadcPtr pRoadc;
tRoadcBytePtr compactedData;
tRoadcUInt32 compactedDataSize;

int main(int argc, char **argv){
  tRoadcUInt32 i;
  /*int j;*/

  commandLineParameter (argc, argv);
    
  if(clVerbose){
    printf("compactor version 2.0\n");
    printf("Read input start...\n");
  }
  readInput();
  if(clVerbose){
    printf("...read input done.\n");
  }

  if(clVerbose){
    printf("Calculation start....\n");
  }
  pRoadc = roadcNew();

  for(i=0; i<getInputNumArrays(); i++){
    roadcAddElement(pRoadc,
		    getInputArray(i),
		    getInputPaddingByteMaskArray(i),
		    getInputArraySize(i),
		    getInputArrayAlignment(i));
    //printf("add array no:%d size:%d alignment:%d\n", (int)i, (int)getInputArraySize(i), (int)getInputArrayAlignment(i));
  }

  roadcCalculation(pRoadc, 1, clTimeout);
  if(clVerbose){
    printf("...calculation done.\n");
  }

  compactedDataSize = roadcGetCompactedDataSize(pRoadc);
  compactedData = roadcGetCompactedData(pRoadc);

  if(clVerbose){
    printf("Write start...\n");
  }
  writeOutputFile();
  if(clVerbose){
    printf("...write done.\n");
  }

  if(clVerbose){
    printf("Original data size:  %lu\n", getInputNumBytes());
    printf("Compacted data size: %lu\n", compactedDataSize);
    printf("Compaction rate:     %f\n", (float)compactedDataSize/(float)getInputNumBytes());
  }

  roadcDelete(pRoadc);
  readInputFreeArrays();

  return 0; 
}
