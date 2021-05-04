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

/** @file roadc.h
 *  @brief Function prototypes for roadC.
 *
 *  This contains the prototypes for the roadC
 *  functions, types, and constants needed.
 *
 *  @author Dr. Steffen Goerzig
 *  @bug No known bugs.
 */

#ifndef ROADC_H_   
#define ROADC_H_

#ifdef __cplusplus
#include <climits>
#include <ctime>
#else
#include <limits.h>
#include <time.h>
#endif /* __cplusplus */


/* roadC defines and types */

/*! Maximal input size for roadC. 
    The sum of all input arrays must not be larger than this value.
    Adding arrays which will lead to exceed this value are ignored. 
 */
#define ROADC_MAX_INPUT_SIZE (ULONG_MAX/4)

/*! roadC unsigned integer type with size 32 bit */
typedef unsigned long tRoadcUInt32;
/*! roadC byte type with size 8 bit */
typedef unsigned char tRoadcByte;
/*! roadC byte pointer type */
typedef unsigned char* tRoadcBytePtr;
/*! roadC float type size 64 bit */
typedef double tRoadcFloat64;

/* no doxygen parsing for internal types */
/*! \cond */
struct tRoadcDataEntryStruct{
  tRoadcBytePtr pData;
  tRoadcBytePtr pPaddingByteMask;
  tRoadcUInt32 size;
  tRoadcUInt32 alignment;
  tRoadcUInt32 alignmentOffset;
  struct tRoadcDataEntryStruct *pPrevious;
  struct tRoadcDataEntryStruct *pNext;
};

typedef struct tRoadcDataEntryStruct tRoadcDataEntry;
typedef tRoadcDataEntry* tRoadcDataEntryPtr;
typedef tRoadcDataEntryPtr* tRoadcDataEntryPtrPtr;

struct tRoadcStruct{
  tRoadcDataEntryPtr pRoadcFirstDataEntry;
  tRoadcDataEntryPtr pRoadcLastDataEntry;
  tRoadcDataEntryPtr pRoadcFirstLeftOverlapCheckDataEntry;
  tRoadcDataEntryPtr pRoadcFirstRightOverlapCheckDataEntry;
  tRoadcDataEntryPtr pRoadcGreedyCurrentDataEntryLarger;
  tRoadcDataEntryPtr pRoadcGreedyCurrentDataEntrySmaller;
  tRoadcUInt32 roadcGreedyCurrentOverlapSize;
  tRoadcUInt32 roadcNumberDataEntries;
  clock_t roadcStartTime;
  clock_t roadcEndTime;
  tRoadcByte roadcStopedByTimeout;
  tRoadcUInt32 roadcCurrentInputSize;
};

typedef struct tRoadcStruct tRoadc;
/*! \endcond */

/*! pointer to roadC context */
typedef tRoadc* tRoadcPtr;

/* roadC functions */

/*! \brief Initialize roadC context
 
  Initialization of a (static) roadC context. 
  For dynamic allocation use the functions roadcNew() and roadcDelete() instead.
  A roadC context is needed as a parameter for the most other functions.
  \param[in] pRoadc pointer roadC context data
  \sa roadcNew and roadcDelete
*/

void roadcInitialize(tRoadc * const pRoadc);

/*! \brief Generate a new roadC context
 
  Dynamic allocation of a new roadC context. The context is already initialized
  by calling this function, an additional call of roadcInitialize() is not needed.
  A roadC context is needed as a parameter for the most other functions.
  \return roadc context data 
  \sa roadcDelete
*/
tRoadcPtr roadcNew(void);

/*! \brief Delete a roadC context
 
  Call this function to release the dynamically allocated memory for a roadC context.
  \param[in] pRoadc pointer to roadC context data
  \sa roadcNew
*/

void roadcDelete(tRoadcPtr pRoadc);

/*! \brief Add new array data
 
  Add new elements for calculation. Use this function until all elements 
  are added, than start compaction algorithm.

  Use the very same parameter when searching the position of the data in the
  resulting compressed array by calling roadcGetCompactedDataAlignment().

  \pre get roadc context data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \param[in] pData data array
  \param[in] pPaddingByteMask padding byte mask or NULL
  \param[in] size array length of data and paddingByteMask (if not NULL)
  \param[in] alignment memory alignment, for no special alignment use 1
  \sa roadcCalculation, roadcNew, and roadcGetCompactedDataAlignment
*/
void roadcAddElement(tRoadcPtr pRoadc,
		     tRoadcBytePtr pData,
		     tRoadcBytePtr pPaddingByteMask,
		     tRoadcUInt32 size,
		     tRoadcUInt32 alignment);

/*! \brief Calculate roadC algorithm with a given timeout
 
  Compression calculation with or without a given timeout constraint. Please note that depending on the input data 
  calculation can be very slow.

  Please also note that only a part of the calculation (compression alogithm) is restricted by the given 
  timeout value. Afterwards the calculated data is concatenated to a single array. The time needed for this 
  concatenation step is difficult to predict and is therefore not part of the timout constraint. 
  For large input data this step can also consume an notable additional amount of time. So the
  given timeout value is a lower limit of the total time consumption of the calculation.

  \pre get roadc context data and add data elements before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \param[in] compressionLevel 0:    remove multiple arrays (arrays which are completely included in other arrays)
                              else: remove multiple arrays and overlapping arrays
  \param[in] maxTimeInSeconds 0: no timeout; else: timeout in seconds (lower limit) for the compression calculation
  \sa roadcNew and roadcAddElement
*/
void roadcCalculation(tRoadcPtr pRoadc,
		      tRoadcByte compressionLevel,
		      tRoadcUInt32 maxTimeInSeconds);

/*! \brief Get the resulting array after roadC calculation
 
  \pre compress data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \return data single byte array containing the compressed data
  \sa roadcGetCompactedDataSize, roadcGetCompactedDataPaddingByteMask, 
      roadcGetCompactedDataAlignment, and roadcGetPositionInCompactedData
*/
tRoadcBytePtr roadcGetCompactedData(tRoadcPtr pRoadc);

/*! \brief Get the size of the resulting array after roadC calculation

  The size is also the size for the padding byte mask as given by calling
  roadcGetCompactedDataPaddingByteMask()
 
  \pre compress data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \return size size of the resulting compressed byte array
  \sa roadcGetCompactedData, roadcGetCompactedDataPaddingByteMask, 
      roadcGetCompactedDataAlignment, and roadcGetPositionInCompactedData
*/
tRoadcUInt32 roadcGetCompactedDataSize(tRoadcPtr pRoadc);

/*! \brief Get the resulting padding byte mask array after roadC calculation
 
  Call roadcGetCompactedDataSize() to get he size of the padding byte mask array.
 
  \pre compress data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \return mask padding byte mask array of the resulting compressed byte array
  \sa roadcGetCompactedData, roadcGetCompactedSize, 
      roadcGetCompactedDataAlignment, and roadcGetPositionInCompactedData
*/
tRoadcBytePtr roadcGetCompactedDataPaddingByteMask(tRoadcPtr pRoadc);

/*! \brief Get the resulting alignment value of the compressed byte array

  \pre compress data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \return alignment alignment value of the compressed byte array
  \sa roadcGetCompactedDataSize, roadcGetCompactedDataPaddingByteMask, 
      roadcGetCompactedData, and roadcGetPositionInCompactedData
*/
tRoadcUInt32 roadcGetCompactedDataAlignment(tRoadcPtr pRoadc);

/*! \brief Get the resulting array after roadC calculation

  Get the position of the input data in the compressed array.
  The parameter to search for must be the same as in calling the
  function roadcAddElement()
 
  \pre compress data before calling this function
  \param[in] pRoadc pointer to roadC context data as given by roadcNew()
  \param[in] pData data array
  \param[in] pPaddingByteMask padding byte mask or NULL
  \param[in] size array length of data and paddingByteMask (if not NULL)
  \param[in] alignment memory alignment, for no special alignment use 1
  \return position if position==roadcGetCompactedDataSize(): not found; 
                   if position==ROADC_MAX_INPUT_SIZE: error (parameter not valid);
                   else: starting position of pData in compressed array
  \sa roadcGetCompactedData, roadcGetCompactedDataSize, 
      roadcGetCompactedDataAlignment, and roadcGetCompactedDataPaddingByteMask
*/
tRoadcUInt32 roadcGetPositionInCompactedData(tRoadcPtr pRoadc,
					     tRoadcBytePtr pData, 
					     tRoadcBytePtr pPaddingByteMask,
					     tRoadcUInt32 size,
					     tRoadcUInt32 alignment);



#endif // ROADC_H_



