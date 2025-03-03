#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef enum
{
    COMMON_STATUS_OK =                      0,
    COMMON_STATUS_FAIL =                    -1,
    COMMON_STATUS_INVALID_PARAMETERS =      -3,
    COMMON_STATUS_NO_NEED_TO_UPDATE =       -4
} CommonStatus;

uint16_t    Common_Crc16(uint8_t *pcBlock, uint16_t len);
long        Common_Constrain(long Value, long Min, long Max);
int         Common_Map(int x, int inMin, int inMax, int outMin, int outMax);
double      Common_RoundDoubleToPrecision(double value, double precision);
uint64_t    Common_ConvertFromDoubleToU64(double d);
uint16_t    Common_ConvertFromStrToU16(char *pId, int base);
uint32_t    Common_ConvertFromStrToU32(char *pId, int base);
uint64_t    Common_ConvertFromStrToU64(char *pId, int base);

CommonStatus Common_SeparateData(char *pData, char *pOut, size_t elemSize, size_t elemCount, char *pSep);
void         Common_RemoveCharFromString(char c, char *str);
void         Common_RemoveCharFromStringGuarded(char c, char *str, char guardLeft, char guardRight);

#if defined (USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif

#endif
