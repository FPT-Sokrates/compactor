/*
The MIT License

Copyright (c) 2021 MBition GmbH

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

/** @file roadc.c
 *  @brief Reference implementation for roadC.
 *
 *  This file contains an reference implementation of the roadC functionality.
 *
 *  @author Dr. Steffen Görzig
 */

#include "roadc.h"

/* use next define for standard print output of some internal roadc information */
/*#define ROADC_PRINTINTERNALS */

#ifdef ROADC_PRINTINTERNALS
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */
#define ROADC_NOTIFICATION_MSG(...) fprintf (stdout, __VA_ARGS__)
#define ROADC_ERROR_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define ROADC_NOTIFICATION_MSG(...)
#define ROADC_ERROR_MSG(...)
#endif /* ROADC_PRINTINTERNALS */

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif /* __cplusplus */

#ifndef __cplusplus
#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */
#endif /* __cplusplus */

/* value to calculate without timeout in roadcCalculation() */ 
#define ROADC_NO_TIMEOUT 0
/* Define for padding byte mask values: is no padding byte */
#define ROADC_NO_PADDING_BITS 0
/* Define for padding byte mask values: all bits are padding bits */
#define ROADC_ALL_PADDING_BITS 255

#define ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(ptr, returnType)  { \
  if(ptr==NULL){                                                 \
    return (returnType)NULL;                                     \
  }                                                              \
} 

#define ROADC_ALLOC_FAILED_WITHOUT_RETURN_VALUE(ptr)  { \
  if(ptr==NULL){                                        \
    return;                                             \
  }                                                     \
} 


void roadcInitDataEntry(tRoadcDataEntryPtr pEntry){
  pEntry->pData=(tRoadcBytePtr)NULL;
  pEntry->pPaddingByteMask=(tRoadcBytePtr)NULL;
  pEntry->size=0;
  pEntry->alignment=1;
  pEntry->alignmentOffset=0;
  pEntry->pPrevious=(struct tRoadcDataEntryStruct *)NULL;
  pEntry->pNext=(struct tRoadcDataEntryStruct *)NULL;
};

void roadcCopyDataEntry(tRoadcDataEntryPtr pFrom, 
                        tRoadcDataEntryPtr pTo){
  tRoadcUInt32 i;
  void *pTmp0=NULL;
  void *pTmp1=NULL;
  void *pTmp2=NULL;
  void *pTmp3=NULL;
  
  /* get memory right */
  if(NULL==pFrom->pData){
    return;
  }
  if(NULL==pTo->pData){
    pTmp0 = malloc(pFrom->size); 
    if(NULL==pTmp0){
      return;
    }
    pTo->pData = (tRoadcBytePtr)pTmp0;
  }
  if((NULL==pFrom->pPaddingByteMask) &&
     (NULL!=pTo->pPaddingByteMask)){
    free(pTo->pPaddingByteMask);
    pTo->pPaddingByteMask=NULL;
  }
  if((NULL!=pFrom->pPaddingByteMask) &&
     (NULL==pTo->pPaddingByteMask)){
    pTmp1 = malloc(pFrom->size); 
    if(NULL==pTmp1){
      if(NULL!=pTmp0){
	free(pTmp0);
	pTo->pData = (tRoadcBytePtr)NULL;
      }
      return;
    }
    pTo->pPaddingByteMask = (tRoadcBytePtr)pTmp1;
  }
  if(pFrom->size!=pTo->size){
    if(pTo->pPaddingByteMask!=NULL){
      free(pTo->pPaddingByteMask);
      pTo->pPaddingByteMask=NULL;
    }
    if(pFrom->pPaddingByteMask!=NULL){
      pTmp2 = malloc(pFrom->size); 
      if(NULL==pTmp2){
	if(NULL!=pTmp0){
	  free(pTmp0);
	  pTo->pData = (tRoadcBytePtr)NULL;
	}
	if(NULL!=pTmp1){
	  free(pTmp1);
	  pTo->pPaddingByteMask = (tRoadcBytePtr)NULL;
	}
	return;
      }
      pTo->pPaddingByteMask = (tRoadcBytePtr)pTmp2;
    }
    if(pTo->pData!=NULL){
      free(pTo->pData);
      pTo->pData=NULL;
    }
    pTmp3 = malloc(pFrom->size); 
    if(NULL==pTmp3){
      if(NULL!=pTmp0){
	free(pTmp0);
	pTo->pData = (tRoadcBytePtr)NULL;
      }
      if(NULL!=pTmp1){
	free(pTmp1);
	pTo->pPaddingByteMask = (tRoadcBytePtr)NULL;
      }
      if(NULL!=pTmp2){
	free(pTmp2);
	pTo->pPaddingByteMask = (tRoadcBytePtr)NULL;
      }
      return;
    }
    pTo->pData = (tRoadcBytePtr)pTmp3;
  }
    
  if(pFrom->pPaddingByteMask==NULL){
    for (i=0;i<pFrom->size;i++){
      pTo->pData[i] = pFrom->pData[i];
    }
  } else {
    for (i=0;i<pFrom->size;i++){
      pTo->pData[i] = pFrom->pData[i];
      pTo->pPaddingByteMask[i] = pFrom->pPaddingByteMask[i];
    }
  }
  pTo->size = pFrom->size;
  pTo->alignment=pFrom->alignment;
  pTo->alignmentOffset=pFrom->alignmentOffset;

}

tRoadcBytePtr roadcMallocUnsignedCharArray(tRoadcUInt32 size){
  void *pTmp;
  pTmp = malloc(size); 
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmp, tRoadcBytePtr);
  return (tRoadcBytePtr)pTmp;
}

tRoadcBytePtr roadcMallocUnsignedCharArrayAndFill(tRoadcUInt32 size, 
                                                  tRoadcByte value){
  tRoadcBytePtr pTmp;
  tRoadcUInt32 i;
  pTmp = roadcMallocUnsignedCharArray(size);
  /* malloc failed? */
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmp, tRoadcBytePtr);
  for(i=0;i<size;i++){
    pTmp[i] = value;
  }
  return pTmp;
}

tRoadcBytePtr roadcMallocUnsignedCharArrayAndCopyData(tRoadcBytePtr pInputArray,
                                                      tRoadcUInt32 inputArraySize){
  tRoadcBytePtr pTmpArray;
  tRoadcUInt32 i;
  pTmpArray = roadcMallocUnsignedCharArray(inputArraySize);
  /* malloc failed? */
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmpArray, tRoadcBytePtr);
  /* copy data */
  for(i=0;i<inputArraySize;i++){
    pTmpArray[i] = pInputArray[i];
  }
  return pTmpArray;
}

tRoadcUInt32 roadcGreatestCommonDivisor(tRoadcUInt32 a, 
                                        tRoadcUInt32 b)
{
  tRoadcUInt32 dividend_u32, divisor_u32, remainder_u32;
  dividend_u32 = a;
  divisor_u32 = b;
  while (0 != divisor_u32){
    remainder_u32 = dividend_u32 % divisor_u32;
    dividend_u32 = divisor_u32;
    divisor_u32 = remainder_u32;
  }
#ifdef ASTREE
  __ASTREE_known_fact((1 <= dividend_u32));
#endif  
  return dividend_u32;  
}

tRoadcUInt32 roadcLeastCommonMultiple(tRoadcUInt32 a, 
                                      tRoadcUInt32 b)
{
  tRoadcUInt32 gcd;
  tRoadcUInt32 result;
  gcd = roadcGreatestCommonDivisor(a,b);
  result = ((a * b) / gcd);
  if(result>=ROADC_MAX_INPUT_SIZE){
    ROADC_ERROR_MSG("roadC ERROR: the alignment values of the input arrays caused an overflow in alignment calculation. The result of roadC is NOT valid anymore!\n");
  }
  return result;
}

tRoadcByte roadcAlignmentPositionInResultCheck(tRoadcUInt32 alignmentResult, 
					       tRoadcUInt32 alignmentInputArray, 
					       tRoadcUInt32 positionInResult){
  tRoadcUInt32 i;
  for (i=0;i<2;i++){
    if((((alignmentResult*i)+positionInResult)%alignmentInputArray)!=0){
      return 0;
    }
  }
  return 1;
}

tRoadcByte roadcAlignmentMergeCheckSolution(tRoadcUInt32 alignment, 
                                            tRoadcUInt32 offset,
                                            tRoadcUInt32 alignmentLeft, 
                                            tRoadcUInt32 offsetLeft,
                                            tRoadcUInt32 alignmentRight, 
                                            tRoadcUInt32 offsetRight,
                                            tRoadcUInt32 overlapPosition){
  tRoadcUInt32 i;
  tRoadcUInt32 positionLeft;
  tRoadcUInt32 positionRight;
  tRoadcUInt32 localAlignmentLeft;
  tRoadcUInt32 localAlignmentRight;
  localAlignmentLeft = alignmentLeft;
  if(localAlignmentLeft==0){
    /* this should not happen, only the improve software analysis */
    localAlignmentLeft=1;
  }
  localAlignmentRight = alignmentRight;
  if(localAlignmentRight==0){
    /* this should not happen, only the improve software analysis */
    localAlignmentRight=1;
  }
  for (i=0;i<2;i++){
    positionLeft = offset - offsetLeft + (i*alignment);
    positionRight = offset + overlapPosition - offsetRight + (i*alignment);
    if((positionLeft%localAlignmentLeft)!=0){
      return 0;
    }
    if((positionRight%localAlignmentRight)!=0){
      return 0;
    }
  }
  return 1;
}

void roadcAlignmentConcatenationCalculation(tRoadcByte copyData,
                                            tRoadcDataEntryPtr pLeft,
                                            tRoadcDataEntryPtr pRight,
                                            tRoadcDataEntryPtr pResult){

  tRoadcUInt32 numberFillBytes;
  tRoadcUInt32 resIndex;
  tRoadcUInt32 lastLeftIndex;
  tRoadcUInt32 completeLenLeft;
  tRoadcUInt32 completeLenRight;
  tRoadcUInt32 appendDataAlignment;
  tRoadcUInt32 i;
  tRoadcByte appendLeftDataFirst;
  pResult->size = 0;
  numberFillBytes = 0;
  completeLenLeft = pLeft->size+pLeft->alignmentOffset;
  completeLenRight = pRight->size+pRight->alignmentOffset;
  if(completeLenLeft==completeLenRight){
    if(pLeft->alignment > pRight->alignment){
      appendLeftDataFirst = 1;
    } else {
      appendLeftDataFirst = 0;
    }
  } else if (completeLenLeft>completeLenRight){
    appendLeftDataFirst = 1;
  } else {
    appendLeftDataFirst = 0;
  }
  if(appendLeftDataFirst==1){
    pResult->size = completeLenLeft;
    appendDataAlignment = pRight->alignment;
  } else {
    pResult->size = completeLenRight;
    appendDataAlignment = pLeft->alignment;
  }
  if(appendDataAlignment==0){
    /* this should not happen, added to increase 
       accuracy of static software analysis tools */
    appendDataAlignment=1;
  }
  if((pResult->size%appendDataAlignment)!=0){
    numberFillBytes = appendDataAlignment - (pResult->size%appendDataAlignment);
    pResult->size = pResult->size + numberFillBytes;
  }
  if(appendLeftDataFirst==1){
    pResult->size = pResult->size + completeLenRight;
  } else {
    pResult->size = pResult->size + completeLenLeft;
  }
  pResult->alignment = roadcLeastCommonMultiple(pLeft->alignment, pRight->alignment);
  pResult->alignmentOffset = 0;

  if(copyData==1){
    resIndex = 0;
    if(appendLeftDataFirst==1){
      resIndex=resIndex+pLeft->alignmentOffset;
      /* pResult must have a padding byte mask */
      /* delete possible old values from old runs */
      for (i=0;i<resIndex;i++){
	pResult->pData[i] = 0;
	pResult->pPaddingByteMask[i] = ROADC_ALL_PADDING_BITS; 
      }
      if(pLeft->pPaddingByteMask==NULL){
        for (i=0;i<pLeft->size;i++){
          pResult->pData[resIndex] = pLeft->pData[i];
	  pResult->pPaddingByteMask[resIndex] = ROADC_NO_PADDING_BITS;
          resIndex++;
        }
      } else {
        for (i=0;i<pLeft->size;i++){
          pResult->pData[resIndex] = pLeft->pData[i];
          pResult->pPaddingByteMask[resIndex] = pLeft->pPaddingByteMask[i];
          resIndex++;
        }
      }
    } else {
      resIndex=resIndex+pRight->alignmentOffset;
      /* pResult must have a padding byte mask */
      /* delete possible old values from old runs */
      for (i=0;i<resIndex;i++){
	pResult->pData[i] = 0;
	pResult->pPaddingByteMask[i] = ROADC_ALL_PADDING_BITS; 
      }
      if(pRight->pPaddingByteMask==NULL){
        for (i=0;i<pRight->size;i++){
          pResult->pData[resIndex] = pRight->pData[i];
	  pResult->pPaddingByteMask[resIndex] = ROADC_NO_PADDING_BITS;
          resIndex++;
        }
      } else {
	/* pResult must have a padding byte mask */
	/* delete possible old values from old runs */
        for (i=0;i<resIndex;i++){
          pResult->pData[i] = 0;
	  pResult->pPaddingByteMask[i] = ROADC_ALL_PADDING_BITS; 
        }
        for (i=0;i<pRight->size;i++){
          pResult->pData[resIndex] = pRight->pData[i];
          pResult->pPaddingByteMask[resIndex] = pRight->pPaddingByteMask[i];
          resIndex++;
        }
      }
    }
    lastLeftIndex = resIndex;
    resIndex=resIndex + numberFillBytes;
    if(appendLeftDataFirst==1){
      resIndex=resIndex+pRight->alignmentOffset;
      /* pResult must have a padding byte mask */
      /* delete possible old values from old runs */
      for (i=lastLeftIndex;i<resIndex;i++){
	pResult->pData[i] = 0;
	pResult->pPaddingByteMask[i] = ROADC_ALL_PADDING_BITS; 
      }
      if(pRight->pPaddingByteMask==NULL){
        for (i=0;i<pRight->size;i++){
          pResult->pData[resIndex] = pRight->pData[i];
	  pResult->pPaddingByteMask[resIndex] = ROADC_NO_PADDING_BITS;
          resIndex++;
        }
      } else {
        for (i=0;i<pRight->size;i++){
          pResult->pData[resIndex] = pRight->pData[i];
          pResult->pPaddingByteMask[resIndex] = pRight->pPaddingByteMask[i];
          resIndex++;
        }
      }
    } else {
      resIndex=resIndex+pLeft->alignmentOffset;
      /* pResult must have a padding byte mask */
      /* delete possible old values from old runs */
      for (i=lastLeftIndex;i<resIndex;i++){
	pResult->pData[i] = 0;
	pResult->pPaddingByteMask[i] = ROADC_ALL_PADDING_BITS; 
      }
      if(pLeft->pPaddingByteMask==NULL){
        for (i=0;i<pLeft->size;i++){
          pResult->pData[resIndex] = pLeft->pData[i];
	  pResult->pPaddingByteMask[resIndex] = ROADC_NO_PADDING_BITS;
          resIndex++;
        }
      } else {
        for (i=0;i<pLeft->size;i++){
          pResult->pData[resIndex] = pLeft->pData[i];
          pResult->pPaddingByteMask[resIndex] = pLeft->pPaddingByteMask[i];
          resIndex++;
        }
      }
    }
  }
}

tRoadcByte roadcAlignmentMergeCalculation(tRoadcDataEntryPtr pLeft,
                                          tRoadcDataEntryPtr pRight,
                                          tRoadcUInt32 overlapBeginIndex,
                                          tRoadcDataEntryPtr pResult){

  tRoadcUInt32 offsetRight;
  tRoadcUInt32 dataMergeLength;
  tRoadcUInt32 minMergeLength;
  tRoadcUInt32 limit;
  tRoadcUInt32 offset;
  tRoadcUInt32 alignment;

  offset = pLeft->alignmentOffset;
  if(pRight->alignmentOffset>overlapBeginIndex){
    offsetRight = (pRight->alignmentOffset-overlapBeginIndex);
    if(offset<offsetRight){
      offset = offsetRight;
    }
  }
  alignment = roadcLeastCommonMultiple(pLeft->alignment, pRight->alignment);
  /* set limit to concatenation size */
  dataMergeLength = pLeft->size + (pRight->size-(pLeft->size-overlapBeginIndex));
  minMergeLength = offset + dataMergeLength;
  roadcAlignmentConcatenationCalculation(0,pLeft, pRight, pResult);
  limit = pResult->size; 
  /* only safety check, concatLength<minMergeLength should not happen */
  if(limit<minMergeLength){
    limit = minMergeLength;
  }
  /* stop when len(concatenated arrays) < len(merged arrays) */
  while(offset<=limit){
    if(roadcAlignmentMergeCheckSolution(alignment, offset,
                                        pLeft->alignment, pLeft->alignmentOffset,
                                        pRight->alignment, pRight->alignmentOffset,
                                        overlapBeginIndex)==1){
      /* solution correct */
      pResult->alignment = alignment;
      pResult->alignmentOffset = offset;
      return 1;
    }
    offset = offset + pLeft->alignment;
  }
  /* no solution or solution larger than concatenation */
  return 0;
}

void roadcTimerStart(tRoadcPtr pRoadc){
  pRoadc->roadcStartTime=clock();
}

tRoadcUInt32 roadcTimerRead(tRoadcPtr pRoadc){
  pRoadc->roadcEndTime=clock();
  return ((tRoadcUInt32)((tRoadcFloat64)(pRoadc->roadcEndTime - pRoadc->roadcStartTime) / (tRoadcFloat64)CLOCKS_PER_SEC));
}
  

tRoadcDataEntryPtr roadcNewRoadcDataEntry(tRoadcBytePtr pInputArray,
                                          tRoadcUInt32 inputArraySize,
                                          tRoadcBytePtr pInputPaddingByteMask,
                                          tRoadcUInt32 inputAlignment){

  tRoadcBytePtr pTmpArray;
  tRoadcBytePtr pTmpArrayPaddingByteMask;
  void *pTmp;
  tRoadcDataEntryPtr pElem;
  pTmp = malloc(sizeof(tRoadcDataEntry)); 
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmp, tRoadcDataEntryPtr);
  pElem = (tRoadcDataEntryPtr)pTmp;

  pTmpArray = roadcMallocUnsignedCharArrayAndCopyData(pInputArray,
                                                     inputArraySize);
  /* malloc failed? */
  if(pTmpArray==NULL){
    free(pTmp);
    return (tRoadcDataEntryPtr)NULL;
  }
  if(pInputPaddingByteMask!=NULL){
    pTmpArrayPaddingByteMask = 
      roadcMallocUnsignedCharArrayAndCopyData(pInputPaddingByteMask,
                                              inputArraySize);
  /* malloc failed? */
    if(pTmpArrayPaddingByteMask==NULL){
      free(pTmp);
      free(pTmpArray);
      return (tRoadcDataEntryPtr)NULL;
    }
  } else {
    pTmpArrayPaddingByteMask = NULL  ;
  }
  pElem->size = inputArraySize;
  pElem->pData = pTmpArray;
  pElem->pPaddingByteMask = pTmpArrayPaddingByteMask;
  pElem->alignment = inputAlignment;
  pElem->alignmentOffset = 0;
  pElem->pPrevious = NULL;
  pElem->pNext = NULL;

  return pElem;
}

tRoadcBytePtr roadcNewAndConcatenateData(tRoadcBytePtr pArr1, 
                                         tRoadcUInt32 sizeArr1,
                                         tRoadcBytePtr pArr2,
                                         tRoadcUInt32 sizeArr2){
  tRoadcBytePtr pTmpArray;
  tRoadcUInt32 i;
  pTmpArray = roadcMallocUnsignedCharArray(sizeArr1+sizeArr2);
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmpArray, tRoadcBytePtr);
  /* copy data */
  for(i=0;i<sizeArr1;i++){
    pTmpArray[i] = pArr1[i];
  }
  for(i=0;i<sizeArr2;i++){
    pTmpArray[i+sizeArr1] = pArr2[i];
  }
  return pTmpArray;
}

tRoadcBytePtr roadcNewAndConcatenatePaddingByteMask(tRoadcBytePtr pMask1, 
                                                    tRoadcUInt32 maskSize1,
                                                    tRoadcBytePtr pMask2,
                                                    tRoadcUInt32 maskSize2){
  tRoadcBytePtr pTmpArray;
  tRoadcUInt32 i;
  pTmpArray = roadcMallocUnsignedCharArray(maskSize1+maskSize2);
  /* malloc failed? */
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmpArray, tRoadcBytePtr);
  if(pMask1==NULL){
    /* generate data */
    for(i=0;i<maskSize1;i++){
      pTmpArray[i] = ROADC_NO_PADDING_BITS;
    }
  } else {
    /* copy data */
    for(i=0;i<maskSize1;i++){
      pTmpArray[i] = pMask1[i];
    }
  }
  if(pMask2==NULL){
    /* generate data */
    for(i=0;i<maskSize2;i++){
      pTmpArray[i+maskSize1] = ROADC_NO_PADDING_BITS;
    }
  } else {
    /* copy data */
    for(i=0;i<maskSize2;i++){
      pTmpArray[i+maskSize1] = pMask2[i];
    }
  }
  return pTmpArray;
}

tRoadcDataEntryPtr roadcNewRoadcDataEntryByConcatenation(tRoadcDataEntryPtr pElem1, 
                                                         tRoadcDataEntryPtr pElem2,
                                                         tRoadcUInt32 newAlignment,
                                                         tRoadcUInt32 newAlignmentOffset,
                                                         tRoadcUInt32 elem2OverlapSize){
  tRoadcBytePtr pTmpArray;
  tRoadcBytePtr pTmpPaddingByteMask;
  void *pTmp;
  tRoadcDataEntryPtr pElem;

  pTmp = malloc(sizeof(tRoadcDataEntry)); 
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmp, tRoadcDataEntryPtr);
  pElem = (tRoadcDataEntryPtr)pTmp;
  pTmpArray = roadcNewAndConcatenateData(pElem1->pData, pElem1->size,
                                        &(pElem2->pData[elem2OverlapSize]), pElem2->size-elem2OverlapSize);

  /* malloc failed? */
  if(pTmpArray==NULL){
    free(pTmp);
    return (tRoadcDataEntryPtr)NULL;
  }

  if((pElem1->pPaddingByteMask==NULL) &&(pElem2->pPaddingByteMask==NULL)){
    pTmpPaddingByteMask = NULL;
  } else {
    pTmpPaddingByteMask = roadcNewAndConcatenatePaddingByteMask(pElem1->pPaddingByteMask, pElem1->size,
                                                               &(pElem2->pPaddingByteMask[elem2OverlapSize]), 
                                                               pElem2->size-elem2OverlapSize);
    /* malloc failed? */
    if(pTmpPaddingByteMask==NULL){
      free(pTmp);
      free(pTmpArray);
      return (tRoadcDataEntryPtr)NULL;
    }
  }

  pElem->pData = pTmpArray;
  pElem->pPaddingByteMask = pTmpPaddingByteMask;
  pElem->size = pElem1->size+pElem2->size-elem2OverlapSize;
  pElem->alignment = newAlignment;
  pElem->alignmentOffset = newAlignmentOffset;
  pElem->pPrevious = NULL;
  pElem->pNext = NULL;

  return pElem;
}

void roadcInsertDataEntry(tRoadcPtr pRoadc,
                          tRoadcDataEntryPtr pNewElem){
  tRoadcDataEntryPtr pCurrent;
  tRoadcDataEntryPtr pNext;

  /* pRoadc->pRoadcFirstDataEntry pointer is NULL if pRoadc->roadcNumberDataEntries==0, check anyway */
 
  if((pRoadc->roadcNumberDataEntries==0) || (pRoadc->pRoadcFirstDataEntry==NULL)){
    pRoadc->roadcNumberDataEntries=0;
    pNewElem->pPrevious = NULL;
    pNewElem->pNext = NULL;
    pRoadc->pRoadcFirstDataEntry=pNewElem;
    pRoadc->pRoadcLastDataEntry = pNewElem;
  } else if ((*pRoadc->pRoadcFirstDataEntry).size<=pNewElem->size) {
    (*pRoadc->pRoadcFirstDataEntry).pPrevious = pNewElem;
    pNewElem->pPrevious = NULL;
    pNewElem->pNext = pRoadc->pRoadcFirstDataEntry;
    pRoadc->pRoadcFirstDataEntry=pNewElem;
  } else {
    pCurrent = pRoadc->pRoadcFirstDataEntry;
    pNext = (*pRoadc->pRoadcFirstDataEntry).pNext;
    while((pNext!=NULL)&&(pNext->size>pNewElem->size)){
      pCurrent = pCurrent->pNext;
      pNext = pNext->pNext;
    }
    pNewElem->pPrevious = pCurrent;
    pNewElem->pNext = pNext;
    pCurrent->pNext = pNewElem;
    if(pNext==NULL){
      pRoadc->pRoadcLastDataEntry = pNewElem;
    } else {
      pNext->pPrevious = pNewElem;
    }
  }
  pRoadc->roadcNumberDataEntries++;

}


void roadcFreeRoadcDataEntry(tRoadcDataEntryPtr pElem){
  if(pElem!=NULL){
    if(pElem->pData!=NULL){
      free(pElem->pData);
    }
    if(pElem->pPaddingByteMask!=NULL){
      free(pElem->pPaddingByteMask);
    }
    free(pElem);
  }
}

void roadcRemoveDataEntry(tRoadcPtr pRoadc,
                          tRoadcDataEntryPtr pElem){
  tRoadcDataEntryPtr pCurrent;
  tRoadcDataEntryPtr pNext;

  if(pRoadc->roadcNumberDataEntries==0){
    /* empty list */
    return;
  }
  if(pRoadc->roadcNumberDataEntries==1){
    pRoadc->pRoadcFirstDataEntry=NULL;
    pRoadc->pRoadcLastDataEntry=NULL;
  } else {
    if (pRoadc->pRoadcFirstDataEntry==pElem){
      /* remove first element */
      pRoadc->pRoadcFirstDataEntry = (*pRoadc->pRoadcFirstDataEntry).pNext;
      (*pRoadc->pRoadcFirstDataEntry).pPrevious = NULL;
    } else {
      pCurrent = pRoadc->pRoadcFirstDataEntry;
      pNext = (*pRoadc->pRoadcFirstDataEntry).pNext;
      while((pNext!=NULL)&&(pNext!=pElem)){
        pCurrent = pCurrent->pNext;
        pNext = pNext->pNext;
      }
      if(pNext==NULL){
        /* not found */
        return;
      }
      pCurrent->pNext = pNext->pNext;
      if(pNext->pNext==NULL){
        pRoadc->pRoadcLastDataEntry=pCurrent;
      } else {
        (*pNext->pNext).pPrevious = pCurrent;
      }
    }
  }
  pRoadc->roadcNumberDataEntries--;
  roadcFreeRoadcDataEntry(pElem);
}

void roadcAddElement(tRoadcPtr pRoadc,
                     tRoadcBytePtr pData,
                     tRoadcBytePtr pPaddingByteMask,
                     tRoadcUInt32 size,
                     tRoadcUInt32 alignment){
  tRoadcDataEntryPtr pTmp;
  
  if(size==0){
    /* empty array */
    return;
  }
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pData==(tRoadcBytePtr)NULL)){
    return;
  }
  if(size>ROADC_MAX_INPUT_SIZE){
    /* data too large, add not possible */
    return;
  }
  if(pRoadc->roadcCurrentInputSize+size>ROADC_MAX_INPUT_SIZE){
    /* sum of all data too large, add not possible */
    return;
  }
  pRoadc->roadcCurrentInputSize=pRoadc->roadcCurrentInputSize+size;

  if(alignment==0){
    /* wrong value for alignment */
    return;
  }
  pTmp =  roadcNewRoadcDataEntry(pData, size, pPaddingByteMask, alignment);
  /* alloc failed? */
  ROADC_ALLOC_FAILED_WITHOUT_RETURN_VALUE(pTmp);
  roadcInsertDataEntry(pRoadc, pTmp);
}

void roadcInitialize(tRoadc * const pRoadc){
  if (NULL == pRoadc) {
    return;
  }

  pRoadc->pRoadcFirstDataEntry=NULL;
  pRoadc->pRoadcLastDataEntry=NULL;
  pRoadc->pRoadcFirstLeftOverlapCheckDataEntry=NULL;
  pRoadc->pRoadcFirstRightOverlapCheckDataEntry=NULL;
  pRoadc->pRoadcGreedyCurrentDataEntryLarger=NULL;
  pRoadc->pRoadcGreedyCurrentDataEntrySmaller=NULL;
  pRoadc->roadcGreedyCurrentOverlapSize=0;
  pRoadc->roadcNumberDataEntries=0;
  pRoadc->roadcStopedByTimeout=0;
  pRoadc->roadcCurrentInputSize=0;

}


tRoadcPtr roadcNew(void){
  tRoadcPtr pRoadc;
  void *pTmp;

  pTmp = malloc(sizeof(tRoadc)); 
  ROADC_ALLOC_FAILED_WITH_RETURN_VALUE(pTmp, tRoadcPtr);
  pRoadc = (tRoadcPtr)pTmp;

  roadcInitialize(pRoadc);
  return pRoadc;
}


void roadcDeleteEntries(tRoadcPtr pRoadc){
  tRoadcDataEntryPtr pCurrent;
  tRoadcDataEntryPtr pTmp;

  pCurrent = pRoadc->pRoadcFirstDataEntry;
  if(pRoadc->roadcNumberDataEntries>0){
    while(pCurrent!=NULL){
      pTmp = pCurrent->pNext;
      roadcFreeRoadcDataEntry(pCurrent);
      pCurrent = pTmp;
    }
  }
  pRoadc->roadcNumberDataEntries=0;
  pRoadc->pRoadcFirstDataEntry=NULL;
  pRoadc->pRoadcLastDataEntry=NULL;
}

void roadcDelete(tRoadcPtr pRoadc){
  if (NULL == pRoadc) {
    return;
  }
  roadcDeleteEntries(pRoadc);
  if(pRoadc!=NULL){
    free(pRoadc);
  }
}

tRoadcByte roadcArraysEqual(tRoadcBytePtr pArr1, 
                            tRoadcBytePtr pArr1PaddingByteMask,
                            tRoadcBytePtr pArr2, 
                            tRoadcBytePtr pArr2PaddingByteMask,
                            tRoadcUInt32 length){
  tRoadcUInt32 i;
  tRoadcByte tmp1;
  tRoadcByte tmp2;
  if((pArr1PaddingByteMask==NULL) && (pArr2PaddingByteMask==NULL)){
    for(i=0;i<(tRoadcUInt32)length;i++){
      if(pArr1[i]!=pArr2[i]){
        return 0;
      }
    }
  } else if((pArr1PaddingByteMask!=NULL) && (pArr2PaddingByteMask==NULL)){
    for(i=0;i<(tRoadcUInt32)length;i++){
      tmp1 = pArr1[i] | pArr1PaddingByteMask[i];
      tmp2 = pArr2[i] | pArr1PaddingByteMask[i];
      if(tmp1!=tmp2){
        return 0;
      }
    }
  } else if(pArr1PaddingByteMask==NULL){
    for(i=0;i<(tRoadcUInt32)length;i++){
      tmp1 = pArr1[i] | pArr2PaddingByteMask[i];
      tmp2 = pArr2[i] | pArr2PaddingByteMask[i];
      if(tmp1!=tmp2){
        return 0;
      }
    }
  } else {
    for(i=0;i<(tRoadcUInt32)length;i++){
      tmp1 = pArr1[i] | pArr1PaddingByteMask[i] | pArr2PaddingByteMask[i];
      tmp2 = pArr2[i] | pArr1PaddingByteMask[i] | pArr2PaddingByteMask[i];
      if(tmp1!=tmp2){
        return 0;
      }
    }
  }
  return 1;
}

tRoadcUInt32 roadcArrayGetPosition(tRoadcDataEntryPtr pThisArray,
                                   tRoadcDataEntryPtr pInThisArray){
  tRoadcUInt32 numberOfCompares;
  tRoadcUInt32 i;
  tRoadcBytePtr pTmpPaddingByteMask;
  tRoadcByte useInThisArrayPaddingByteMask;
  if(pThisArray->size>pInThisArray->size){
    /* pThisArray too long, can not be part of pInThisArray */
    return pInThisArray->size;
  }
  numberOfCompares = (pInThisArray->size-pThisArray->size) + 1;
  pTmpPaddingByteMask = NULL;
  useInThisArrayPaddingByteMask=0;
  if(pInThisArray->pPaddingByteMask!=NULL){
    useInThisArrayPaddingByteMask=1;
  } 

  for(i=0;i<numberOfCompares;i++){
    if(useInThisArrayPaddingByteMask){
      pTmpPaddingByteMask = &pInThisArray->pPaddingByteMask[i];
    }
    if(roadcArraysEqual(pThisArray->pData, pThisArray->pPaddingByteMask,
                        &pInThisArray->pData[i], pTmpPaddingByteMask, 
                        pThisArray->size)){
      if(roadcAlignmentPositionInResultCheck(pInThisArray->alignment,
					     pThisArray->alignment,
					     i)){
	/* found */
	return i;
      }
    }
  }
  /* not found */
  return pInThisArray->size;
}

void roadcAdaptInPlacePaddingByteMaskForCaseIncluded(tRoadcBytePtr pThisArray, 
						     tRoadcBytePtr pThisArrayPaddingByteMask,
						     tRoadcBytePtr pInThisArrayAdaptedInPlace,
						     tRoadcBytePtr pInThisArrayPaddingByteMaskAdaptedInPlace,
						     tRoadcUInt32 size){
  tRoadcUInt32 i;
  tRoadcByte tmpP;
  tRoadcByte tmpD;
  if(pInThisArrayPaddingByteMaskAdaptedInPlace==NULL){
    /* no change in padding bytes needed */
    return;
  }
  if(pThisArrayPaddingByteMask==NULL){
    /* all padding bytes have to be set to false
       all values must be copied */
    for(i=0;i<size;i++){
      pInThisArrayPaddingByteMaskAdaptedInPlace[i]=ROADC_NO_PADDING_BITS;
      pInThisArrayAdaptedInPlace[i] = pThisArray[i];
    }
    return;
  }
  for(i=0;i<size;i++){
    tmpP = pInThisArrayPaddingByteMaskAdaptedInPlace[i] & pThisArrayPaddingByteMask[i];
    tmpD = ((pInThisArrayAdaptedInPlace[i] & (~pInThisArrayPaddingByteMaskAdaptedInPlace[i])) | 
            (pThisArray[i] & (~pThisArrayPaddingByteMask[i])));
    pInThisArrayPaddingByteMaskAdaptedInPlace[i] = tmpP;
    pInThisArrayAdaptedInPlace[i] = tmpD;
  }
}

void roadcAdaptInPlacePaddingByteMaskForCaseOverlap(tRoadcBytePtr pThisArrayAdaptInPlace, 
						    tRoadcBytePtr pThisArrayPaddingByteMaskAdaptInPlace,
						    tRoadcBytePtr pWithThisArrayAdaptInPlace,
						    tRoadcBytePtr pWithThisArrayPaddingByteMaskAdaptInPlace,
						    tRoadcUInt32 size){
  tRoadcUInt32 i;
  tRoadcByte tmpP;
  tRoadcByte tmpD;
  if(pThisArrayPaddingByteMaskAdaptInPlace==NULL){
    if(pWithThisArrayPaddingByteMaskAdaptInPlace==NULL){
      /* no change in padding bytes needed */
      return;
    } else {
      for(i=0;i<size;i++){
        pWithThisArrayPaddingByteMaskAdaptInPlace[i]=ROADC_NO_PADDING_BITS;
        pWithThisArrayAdaptInPlace[i] = pThisArrayAdaptInPlace[i];
      }
      return;
    }
  } else {
    if(pWithThisArrayPaddingByteMaskAdaptInPlace==NULL){
      for(i=0;i<size;i++){
        pThisArrayPaddingByteMaskAdaptInPlace[i]=ROADC_NO_PADDING_BITS;
        pThisArrayAdaptInPlace[i] = pWithThisArrayAdaptInPlace[i];
      }
      return;
    } else {
      for(i=0;i<size;i++){
        tmpP = pWithThisArrayPaddingByteMaskAdaptInPlace[i] & pThisArrayPaddingByteMaskAdaptInPlace[i];
        tmpD = ((pWithThisArrayAdaptInPlace[i] & (~pWithThisArrayPaddingByteMaskAdaptInPlace[i])) | 
                (pThisArrayAdaptInPlace[i] & (~pThisArrayPaddingByteMaskAdaptInPlace[i])));
        pWithThisArrayPaddingByteMaskAdaptInPlace[i] = tmpP;
        pWithThisArrayAdaptInPlace[i] = tmpD;
        pThisArrayPaddingByteMaskAdaptInPlace[i]=tmpP;
        pThisArrayAdaptInPlace[i] = tmpD;
      }
    }
  }
}

tRoadcByte roadcIsOverlap(tRoadcBytePtr pLeftArray,
                          tRoadcBytePtr pLeftArrayPaddingByteMask,
                          tRoadcUInt32 leftArraySize,
                          tRoadcBytePtr pRightArray,
                          tRoadcBytePtr pRightArrayPaddingByteMask,
                          tRoadcUInt32 overlap){
  tRoadcBytePtr pTmpData;
  tRoadcBytePtr pTmpPaddingByteMask;
  tRoadcUInt32 i;
  tRoadcByte tmp1;
  tRoadcByte tmp2;

  pTmpData=&pLeftArray[leftArraySize-overlap];
  pTmpPaddingByteMask=NULL;
  if(pLeftArrayPaddingByteMask!=NULL){
    pTmpPaddingByteMask=&pLeftArrayPaddingByteMask[leftArraySize-overlap];
  }

  if((pLeftArrayPaddingByteMask==NULL) && (pRightArrayPaddingByteMask==NULL)){
    for(i=0;i<overlap;i++){
      if(pTmpData[i]!=pRightArray[i]){
        return 0;
      }
    }
    return 1;
  } else if((pLeftArrayPaddingByteMask!=NULL) && (pRightArrayPaddingByteMask==NULL)){
    for(i=0;i<overlap;i++){
      tmp1 = pTmpData[i] | pTmpPaddingByteMask[i];
      tmp2 = pRightArray[i] | pTmpPaddingByteMask[i];
      if(tmp1!=tmp2){
          return 0;
      }
    }
    return 1;
  } else if(pLeftArrayPaddingByteMask==NULL){
    for(i=0;i<overlap;i++){
      tmp1 = pTmpData[i] | pRightArrayPaddingByteMask[i];
      tmp2 = pRightArray[i] | pRightArrayPaddingByteMask[i];
      if(tmp1!=tmp2){
        return 0;
      }
    }
    return 1;
  } else {
    for(i=0;i<overlap;i++){
      tmp1 = pTmpData[i] | pRightArrayPaddingByteMask[i] | pTmpPaddingByteMask[i];
      tmp2 = pRightArray[i] | pRightArrayPaddingByteMask[i] | pTmpPaddingByteMask[i];
      if(tmp1!=tmp2){
        return 0;
      }
    }
    return 1;
  }
}

void roadcSetGreedyVariables(tRoadcPtr pRoadc,
                             tRoadcDataEntryPtr pGreedyCurrentDataEntryLarger,
                             tRoadcDataEntryPtr pGreedyCurrentDataEntrySmaller,
                             tRoadcUInt32 currentOverlapSize,
                             tRoadcDataEntryPtr pFirstLeftOverlapCheckDataEntry,
                             tRoadcDataEntryPtr pFirstRightOverlapCheckDataEntry){
  pRoadc->pRoadcGreedyCurrentDataEntryLarger=pGreedyCurrentDataEntryLarger;
  pRoadc->pRoadcGreedyCurrentDataEntrySmaller=pGreedyCurrentDataEntrySmaller;
  pRoadc->roadcGreedyCurrentOverlapSize=currentOverlapSize;
  pRoadc->pRoadcFirstLeftOverlapCheckDataEntry=pFirstLeftOverlapCheckDataEntry;
  pRoadc->pRoadcFirstRightOverlapCheckDataEntry=pFirstRightOverlapCheckDataEntry;
}

tRoadcByte roadcGreedyConcatenate(tRoadcPtr pRoadc,
                                  tRoadcDataEntryPtr pElemA,
                                  tRoadcDataEntryPtr pElemB,
                                  tRoadcDataEntryPtrPtr ppNewElem,
                                  tRoadcUInt32 newAlignment,
                                  tRoadcUInt32 newAlignmentOffset,
                                  tRoadcUInt32 elemBOverlapSize){
  *ppNewElem = roadcNewRoadcDataEntryByConcatenation(pElemA, pElemB, 
                                                  newAlignment, newAlignmentOffset,
                                                  elemBOverlapSize);
  if(*ppNewElem==NULL){
    /* malloc error */
    return 1; 
  }
  roadcRemoveDataEntry(pRoadc, pElemA);
  roadcRemoveDataEntry(pRoadc, pElemB);
  roadcInsertDataEntry(pRoadc, *ppNewElem);
  return 0; /* no malloc error */
}

tRoadcByte roadcGreedyStep(tRoadcPtr pRoadc){
  tRoadcDataEntryPtr pTmp;
  tRoadcDataEntryPtr pTmp2;
  tRoadcByte checkOverlapLeft;
  tRoadcByte checkOverlapRight;
  tRoadcUInt32 tmpPos;
  tRoadcBytePtr pTmpPaddingByteMask;
  tRoadcDataEntry calcResult;
  tRoadcDataEntryPtr pNewElem;

  roadcInitDataEntry(&calcResult);
  /* this is also done by the logic in the lines below, 
     but for a better style variables are initialized */
  checkOverlapLeft=1;
  checkOverlapRight=1;

  while(1){
    if((pRoadc->pRoadcFirstLeftOverlapCheckDataEntry==NULL)||
       (pRoadc->pRoadcGreedyCurrentDataEntrySmaller==pRoadc->pRoadcFirstLeftOverlapCheckDataEntry)){
      checkOverlapLeft=1;
    }
    if((pRoadc->pRoadcFirstRightOverlapCheckDataEntry==NULL)||
       (pRoadc->pRoadcGreedyCurrentDataEntrySmaller==pRoadc->pRoadcFirstRightOverlapCheckDataEntry)){
      checkOverlapRight=1;
    }
    if(checkOverlapLeft &&
       (roadcIsOverlap((*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pData,
                       (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pPaddingByteMask,
                       (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).size,
                       (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pData,
                       (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pPaddingByteMask,
                       pRoadc->roadcGreedyCurrentOverlapSize))&&
       (roadcAlignmentMergeCalculation(pRoadc->pRoadcGreedyCurrentDataEntryLarger,
                                       pRoadc->pRoadcGreedyCurrentDataEntrySmaller,
                                       (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).size-
                                       pRoadc->roadcGreedyCurrentOverlapSize,
                                       &calcResult)) 
       ){
      /* overlap left found */
      tmpPos = (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).size-pRoadc->roadcGreedyCurrentOverlapSize;
      pTmpPaddingByteMask=NULL;
      if((*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pPaddingByteMask!=NULL){
        pTmpPaddingByteMask=&(*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pPaddingByteMask[tmpPos];
      }
      roadcAdaptInPlacePaddingByteMaskForCaseOverlap(&(*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pData[tmpPos],
						     pTmpPaddingByteMask,
						     (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pData,
						     (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pPaddingByteMask,
						     pRoadc->roadcGreedyCurrentOverlapSize); 
      /* store roadcGreedyCurrentIndexSmaller and -Larger 
         due to this variables and roadcFirstIndexToCheckForOverlapRight 
         are changed by calling roadcGreedyConcatenate() */
      checkOverlapLeft=0;
      checkOverlapRight=0;
      pTmp = (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pNext;
      pTmp2 = (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext;
      if(pTmp2==pRoadc->pRoadcGreedyCurrentDataEntrySmaller){
        /* element is deleted, take next one */
        pTmp2=pTmp;
      }
      if(roadcGreedyConcatenate(pRoadc,
                                pRoadc->pRoadcGreedyCurrentDataEntryLarger, 
                                pRoadc->pRoadcGreedyCurrentDataEntrySmaller, 
                                &pNewElem,
                                calcResult.alignment,
                                calcResult.alignmentOffset,
                                pRoadc->roadcGreedyCurrentOverlapSize)){
        /* malloc error */
        return 1;
      }
      /* the call of roadcGreedyConcatenate() removes two elements and adds a new element
         The greedy variable are adapted (e.g. pointer pRoadc->pRoadcGreedyCurrentDataEntrySmaller 
         and pRoadc->pRoadcGreedyCurrentDataEntryLarger */
      roadcSetGreedyVariables(pRoadc, pNewElem, pNewElem->pNext, 
                              pRoadc->roadcGreedyCurrentOverlapSize, pTmp2, pTmp);
    } else if(checkOverlapRight&&
              (roadcIsOverlap((*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pData,
                              (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pPaddingByteMask,
                              (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).size,
                              (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pData,
                              (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pPaddingByteMask,
                              pRoadc->roadcGreedyCurrentOverlapSize)) &&
              (roadcAlignmentMergeCalculation(pRoadc->pRoadcGreedyCurrentDataEntrySmaller,
                                              pRoadc->pRoadcGreedyCurrentDataEntryLarger,
                                              (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).size-
                                              pRoadc->roadcGreedyCurrentOverlapSize,
                                              &calcResult)) 
              ){
      /* overlap right found */
      tmpPos = (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).size-pRoadc->roadcGreedyCurrentOverlapSize;
      pTmpPaddingByteMask=NULL;
      if((*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pPaddingByteMask!=NULL){
        pTmpPaddingByteMask=&(*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pPaddingByteMask[tmpPos];
      }
      roadcAdaptInPlacePaddingByteMaskForCaseOverlap(&(*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pData[tmpPos],
						     pTmpPaddingByteMask,
						     (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pData,
						     (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pPaddingByteMask,
						     pRoadc->roadcGreedyCurrentOverlapSize); 
      /* store roadcGreedyCurrentIndexSmaller and -Larger due to this variables 
         and roadcFirstIndexToCheckForOverlapRight
         are changed by calling roadcGreedyConcatenate() */
      checkOverlapLeft=0;
      checkOverlapRight=0;
      pTmp = (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pNext;
      pTmp2 = (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext;
      if(pTmp2==pRoadc->pRoadcGreedyCurrentDataEntrySmaller){
        /* element is deleted, take next one */
        pTmp2=pTmp;
      }
      if(roadcGreedyConcatenate(pRoadc,
                                pRoadc->pRoadcGreedyCurrentDataEntrySmaller, 
                                pRoadc->pRoadcGreedyCurrentDataEntryLarger, 
                                &pNewElem,
                                calcResult.alignment,
                                calcResult.alignmentOffset,
                                pRoadc->roadcGreedyCurrentOverlapSize)){
        /* malloc error */
        return 1;
      }
      /* the call of roadcGreedyConcatenate() removes two elements and adds a new element
        The greedy variable are adapted (e.g. pointer pRoadc->pRoadcGreedyCurrentDataEntrySmaller 
        and pRoadc->pRoadcGreedyCurrentDataEntryLarger */
      roadcSetGreedyVariables(pRoadc, pNewElem, pNewElem->pNext, 
                              pRoadc->roadcGreedyCurrentOverlapSize, pTmp, pTmp2);
    } else {
      /* no overlap take next array for overlap check */
      pRoadc->pRoadcGreedyCurrentDataEntrySmaller = (*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).pNext;
    }
    if(pRoadc->roadcNumberDataEntries==1){
      /* only one array left, done */
      return 0;
    }

    if((pRoadc->pRoadcGreedyCurrentDataEntrySmaller==NULL)||
       ((*pRoadc->pRoadcGreedyCurrentDataEntrySmaller).size<=pRoadc->roadcGreedyCurrentOverlapSize)){
      if((*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext==NULL){
        /* done for the current overlap size */
        return 0;
      } else if((*(*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext).pNext==NULL){
        /* done for the current overlap size */
        return 0;
      } else {
        if((*(*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext).size>pRoadc->roadcGreedyCurrentOverlapSize){ 
          roadcSetGreedyVariables(pRoadc, 
                                  (*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext,
                                  (*(*pRoadc->pRoadcGreedyCurrentDataEntryLarger).pNext).pNext,
                                  pRoadc->roadcGreedyCurrentOverlapSize, NULL,NULL);
        } else {
          /* done for the current overlap size */
          return 0;
        }
      }
    }
  }
}

tRoadcByte roadcCalculationGreedy(tRoadcPtr pRoadc,
                                  tRoadcUInt32 maxTimeInSeconds){
  tRoadcUInt32 elapsedTime;

  /* next element exists (at least two elements) this has to be ensured before calling this function */
  pRoadc->roadcGreedyCurrentOverlapSize = (*(*pRoadc->pRoadcFirstDataEntry).pNext).size;
  if(pRoadc->roadcGreedyCurrentOverlapSize==0){
    /* nothing to do */
    return 0;
  }
  while(1){
    /* set greedy variables
       before call it is assured that data set is >1
       max size at start is second element -1 (otherwise it would have been eliminated during include run) */
    roadcSetGreedyVariables(pRoadc, 
                            pRoadc->pRoadcFirstDataEntry, (*pRoadc->pRoadcFirstDataEntry).pNext, 
                            pRoadc->roadcGreedyCurrentOverlapSize-1, NULL, NULL);
    if(pRoadc->roadcNumberDataEntries==1){
      /* only one entry left */
      return 0;
    } 
    if(pRoadc->roadcGreedyCurrentOverlapSize==0){
      /* no overlap any more */
      return 0;
    } 
    /* calculate for current values */
    if(roadcGreedyStep(pRoadc)){
      /* malloc error */
      return 1;
    }
    if(maxTimeInSeconds!=ROADC_NO_TIMEOUT){
      elapsedTime = roadcTimerRead(pRoadc);
      if(elapsedTime>=maxTimeInSeconds){
        pRoadc->roadcStopedByTimeout=1;
        ROADC_NOTIFICATION_MSG("Greedy stop due to elapsed time: %lu\n", elapsedTime);
        return 0;
      }
    }
  }
}


void roadcCalculationMultiple(tRoadcPtr pRoadc,
                              tRoadcUInt32 maxTimeInSeconds){
  tRoadcUInt32 elapsedTime;
  tRoadcByte found, done;
  tRoadcUInt32 foundPos;
  tRoadcDataEntryPtr pLarger; 
  tRoadcDataEntryPtr pSmaller;
  tRoadcDataEntryPtr pTmp;
  tRoadcBytePtr pTmpPaddingByteMask;
  tRoadcDataEntry calcResult;

  roadcInitDataEntry(&calcResult);
  /* check if any array is already part of an existing array */
  pSmaller = pRoadc->pRoadcLastDataEntry;
  pLarger = pSmaller->pPrevious;
  while(pLarger!=NULL){
    found=0;
    done=0;
    while((!found)&&(!done)){
      foundPos = roadcArrayGetPosition(pSmaller, pLarger);
      if(foundPos!=pLarger->size){
	if(roadcAlignmentMergeCalculation(pLarger, pSmaller, foundPos, &calcResult)){
	  /* remove included array */
	  found=1;
	  /* adjust padding byte masks */
	  pTmpPaddingByteMask=NULL; 
	  if(pLarger->pPaddingByteMask!=NULL){
	    pTmpPaddingByteMask=&pLarger->pPaddingByteMask[foundPos];
	  }
	  roadcAdaptInPlacePaddingByteMaskForCaseIncluded(pSmaller->pData, pSmaller->pPaddingByteMask, 
							  &pLarger->pData[foundPos], pTmpPaddingByteMask,
							  pSmaller->size);
	  /* adjust alignment data */
	  pLarger->alignment = calcResult.alignment;
	  pLarger->alignmentOffset = calcResult.alignmentOffset;
	}
      }
      if(pLarger->pPrevious==NULL){
	done = 1;
      } else {
	pLarger = pLarger->pPrevious;
      }
    }
    pTmp = pSmaller->pPrevious;
    if(found){
      /* element is deleted */
      roadcRemoveDataEntry(pRoadc, pSmaller);
    }
    pSmaller = pTmp;
    pLarger = pSmaller->pPrevious;
    
    if(maxTimeInSeconds!=ROADC_NO_TIMEOUT){
      elapsedTime = roadcTimerRead(pRoadc);
      if(elapsedTime>=maxTimeInSeconds){
	pRoadc->roadcStopedByTimeout=1;
	ROADC_NOTIFICATION_MSG("Multiple stop due to elapsed time: %lu\n", elapsedTime);
	return;
      }
    }
  }
}

void roadcCalculation(tRoadcPtr pRoadc,
                      tRoadcByte compressionLevel,
                      tRoadcUInt32 maxTimeInSeconds){
  tRoadcBytePtr pTmp;
  clock_t startTime, endTime;
  tRoadcFloat64 elapsedTime;
  tRoadcFloat64 totalTime;
  tRoadcDataEntryPtr pCurrent;
  tRoadcDataEntry calcCurrent;
  tRoadcDataEntryPtr calcResult;
  void *pVoid;

  totalTime = 0.0;

  if (NULL == pRoadc) {
    return;
  }

  if(pRoadc->roadcNumberDataEntries==0){
    /* nothing to do */
    return;
  }

  if(maxTimeInSeconds!=ROADC_NO_TIMEOUT){
    roadcTimerStart(pRoadc);
  }
  pRoadc->roadcStopedByTimeout=0;
  ROADC_NOTIFICATION_MSG("number arrays before compaction: %lu\n", pRoadc->roadcNumberDataEntries);
  if(pRoadc->roadcNumberDataEntries>1){
    startTime=clock();
    roadcCalculationMultiple(pRoadc, maxTimeInSeconds);
    endTime=clock();
    elapsedTime = (tRoadcFloat64)((tRoadcFloat64)(endTime - startTime) / (tRoadcFloat64)CLOCKS_PER_SEC);
    totalTime = elapsedTime;
    ROADC_NOTIFICATION_MSG("remove multiple time (s): %f\n", elapsedTime);

  }
  ROADC_NOTIFICATION_MSG("number arrays after multiple: %lu\n", pRoadc->roadcNumberDataEntries);
  if((pRoadc->roadcStopedByTimeout==0)&&compressionLevel&&(pRoadc->roadcNumberDataEntries>1)){
    startTime=clock();
    /* nothing to do for only one entry */
    if(roadcCalculationGreedy(pRoadc, maxTimeInSeconds)){
      /* malloc error */
      return;
    }
    endTime=clock();
    elapsedTime = (tRoadcFloat64)((tRoadcFloat64)(endTime - startTime) / (tRoadcFloat64)CLOCKS_PER_SEC);
    totalTime += elapsedTime;
    ROADC_NOTIFICATION_MSG("greedy time (s): %f\n", elapsedTime);
    ROADC_NOTIFICATION_MSG("number arrays after greedy: %lu\n", pRoadc->roadcNumberDataEntries);
  }
  startTime=clock();
  pCurrent = pRoadc->pRoadcFirstDataEntry;
  calcCurrent.size = 0;
  calcCurrent.alignment = 1;
  calcCurrent.alignmentOffset = 0;
  pVoid = malloc(sizeof(tRoadcDataEntry)); 
  ROADC_ALLOC_FAILED_WITHOUT_RETURN_VALUE(pVoid);
  calcResult = (tRoadcDataEntryPtr)pVoid;
  roadcInitDataEntry(calcResult);

  while(pCurrent!=NULL){
    roadcAlignmentConcatenationCalculation(0, &calcCurrent, pCurrent, calcResult);
    calcCurrent.size = calcResult->size;
    calcCurrent.alignment = calcResult->alignment;
    calcCurrent.alignmentOffset = calcResult->alignmentOffset;
    pCurrent = pCurrent->pNext;
  }

  pTmp=roadcMallocUnsignedCharArrayAndFill(calcCurrent.size, 0);
  if(pTmp==NULL){
    /* alloc failed */
    free(calcResult);
    return;
  }
  calcCurrent.pData = pTmp;
  pTmp=roadcMallocUnsignedCharArrayAndFill(calcCurrent.size, ROADC_NO_PADDING_BITS);
  if(pTmp==NULL){
    /* alloc failed */
    free(calcResult);
    free(calcCurrent.pData);
    return;
  }
  calcCurrent.pPaddingByteMask = pTmp;
  pTmp=roadcMallocUnsignedCharArrayAndFill(calcCurrent.size, 0);
  if(pTmp==NULL){
    /* alloc failed */
    free(calcResult);
    free(calcCurrent.pData);
    free(calcCurrent.pPaddingByteMask);
    return;
  }
  calcResult->pData = pTmp;
  pTmp=roadcMallocUnsignedCharArrayAndFill(calcCurrent.size, ROADC_NO_PADDING_BITS);
  if(pTmp==NULL){
    /* alloc failed */
    free(calcCurrent.pData);
    free(calcCurrent.pPaddingByteMask);
    free(calcResult->pData);
    free(calcResult);
    return;
  }
  calcResult->pPaddingByteMask = pTmp;

  /* concatenate result data */
  calcCurrent.size = 0;
  calcCurrent.alignment = 1;
  calcCurrent.alignmentOffset = 0;
  pCurrent = pRoadc->pRoadcFirstDataEntry;
  while(pCurrent!=NULL){
    roadcAlignmentConcatenationCalculation(1, &calcCurrent, pCurrent, calcResult);
    roadcCopyDataEntry(calcResult, &calcCurrent);
    pCurrent = pCurrent->pNext;
  }

  free(calcCurrent.pData);
  free(calcCurrent.pPaddingByteMask);
  roadcDeleteEntries(pRoadc);
  pRoadc->roadcCurrentInputSize=calcResult->size;
  roadcInsertDataEntry(pRoadc, calcResult);

  endTime=clock();
  elapsedTime = (tRoadcFloat64)((tRoadcFloat64)(endTime - startTime) / (tRoadcFloat64)CLOCKS_PER_SEC);
  totalTime += elapsedTime;
  ROADC_NOTIFICATION_MSG("concatenation time (s): %f\n", elapsedTime);
  ROADC_NOTIFICATION_MSG("Total time (s): %f\n", totalTime);
}  

tRoadcBytePtr roadcGetCompactedData(tRoadcPtr pRoadc){
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pRoadc->pRoadcFirstDataEntry==NULL)){
    return (tRoadcBytePtr)NULL;
  }
  return pRoadc->pRoadcFirstDataEntry->pData;
}

tRoadcBytePtr roadcGetCompactedDataPaddingByteMask(tRoadcPtr pRoadc){
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pRoadc->pRoadcFirstDataEntry==NULL)){
    return (tRoadcBytePtr)NULL;
  }
  return pRoadc->pRoadcFirstDataEntry->pPaddingByteMask;
}

tRoadcUInt32 roadcGetCompactedDataSize(tRoadcPtr pRoadc){
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pRoadc->pRoadcFirstDataEntry==NULL)){
    return 0;
  }
  return pRoadc->pRoadcFirstDataEntry->size;
}

tRoadcUInt32 roadcGetCompactedDataAlignment(tRoadcPtr pRoadc){
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pRoadc->pRoadcFirstDataEntry==NULL)){
    return 0;
  }
  return pRoadc->pRoadcFirstDataEntry->alignment;
}

tRoadcUInt32 roadcGetPositionInCompactedData(tRoadcPtr pRoadc,
                                             tRoadcBytePtr pData, 
                                             tRoadcBytePtr pPaddingByteMask,
                                             tRoadcUInt32 size,
                                             tRoadcUInt32 alignment){
  tRoadcDataEntry thisArray;
  tRoadcDataEntry inThisArray;
  if((pRoadc==(tRoadcPtr)NULL) ||
     (pRoadc->pRoadcFirstDataEntry==(tRoadcDataEntryPtr)NULL) ||
     (pData==(tRoadcBytePtr)NULL)){
    return ROADC_MAX_INPUT_SIZE;
  }
  thisArray.size = size;
  thisArray.pData = pData;
  thisArray.pPaddingByteMask = pPaddingByteMask;
  thisArray.alignment = alignment;
  thisArray.alignmentOffset = 0;
  inThisArray.size = roadcGetCompactedDataSize(pRoadc);
  inThisArray.pData = roadcGetCompactedData(pRoadc);
  /* resulting padding byte mask must not be used since this would produce wrong matches */
  inThisArray.pPaddingByteMask = NULL;
  inThisArray.alignment = roadcGetCompactedDataAlignment(pRoadc);
  inThisArray.alignmentOffset = 0;
  return roadcArrayGetPosition(&thisArray, &inThisArray);
}

