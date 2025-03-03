#include "common.h"

uint16_t Common_Crc16(uint8_t *pcBlock, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (len--)
    {
        crc ^= *pcBlock++ << 8;

        for (i = 0; i < 8; i++)
        {
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
        }
    }
    return crc;
}

int Common_Map(int x, int inMin, int inMax, int outMin, int outMax)
{
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

long Common_Constrain(long Value, long Min, long Max)
{
  return (Value < Min)? Min : (Value > Max)? Max : Value;
}

double Common_RoundDoubleToPrecision(double value, double precision)
{
    return round(value/precision) * precision;
}

uint64_t Common_ConvertFromDoubleToU64 (double d)
{
    if (d > (double)UINT64_MAX)
    {
        return 0;
    }

    return (uint64_t)d;
}

uint16_t Common_ConvertFromStrToU16(char *pId, int base)
{
    char * pEnd;
    uint16_t u16 = (uint16_t)strtoul(pId, &pEnd, base);
    return u16;
}

uint32_t Common_ConvertFromStrToU32(char *pId, int base)
{
    char * pEnd;
    uint32_t u32 = (uint32_t)strtoul(pId, &pEnd, base);
    return u32;
}

uint64_t Common_ConvertFromStrToU64(char *pId, int base)
{
    char * pEnd;
    uint64_t u64 = (uint64_t)strtoull(pId, &pEnd, base);
    return u64;
}

CommonStatus Common_SeparateData(char *pData, char *pOut, size_t elemSize, size_t elemCount, char *pSep)
{
    if (pData == NULL || pOut == NULL || pSep == NULL)
    {
        return COMMON_STATUS_INVALID_PARAMETERS;
    }

    char *pToken;
    pToken = strtok (pData, pSep);

    // Parse 1st Data
    sscanf (pToken, "%[^\n]", pOut);
    // Parse Other Data
    for (int i = 1; i < elemCount; i++)
    {
        pToken = strtok (NULL, pSep);
        if (pToken != NULL)
        {
            sscanf (pToken, "%[^\n]", pOut + i*elemSize);
        }
        else
        {
            break;
        }
    }
#if (CONFIG_IDF_TARGET_ESP32S3) || (CONFIG_IDF_TARGET_ESP32)
    for (int i = 0; i < elemCount; i++)
    {
        ESP_LOGD(COMMON_TAG, "data%d (%d): %s\n\r", i, strlen(pOut + i*elemSize), pOut + i*elemSize);
    }
#endif

    return COMMON_STATUS_OK;
}

static void Common_PushZerosToEnd(uint8_t arr[], int n)
{
    int count = {0};  // Count of non-zero elements
 
    // Traverse the array. If element encountered is non-
    // zero, then replace the element at index 'count'
    // with this element
    for (int i = 0; i < n; i++)
        if (arr[i] != 0)
            arr[count++] = arr[i]; // here count is
                                   // incremented
 
    // Now all non-zero elements have been shifted to
    // front and  'count' is set as index of first 0.
    // Make all elements 0 from count to end.
    while (count < n)
        arr[count++] = 0;
}

void Common_RemoveCharFromString(char c, char *str)
{
    int i = 0;
    int len = strlen(str) + 1;

    for(i= 0; i < len; i++)
    {
        if(str[i] == c)
        {
            // Move all the char following the char "c" by one to the left.
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }
}

void Common_RemoveCharFromStringGuarded(char c, char *str, char guardLeft, char guardRight)
{
    int i = 0;
    int len = strlen(str) + 1;

    bool lDetect = false;
    bool rDetect = false;

    for(i= 0; i < len; i++)
    {
        if(str[i] == c)
        {
            // Try to find one of borders in all string
            for (int j = 0; j < len; j++)
            {
                // Reach str borders, abort
                if ((i - j < 0) || (i + j > len))
                {
                    //printf("hit array border: i:%d j:%d\n\r", i,j);
                    break;
                }
                // Check left
                if (str[i-j] == guardLeft)
                {
                    lDetect = true;
                    //printf("find l1: %c <-[%c]\n\r", str[i-j], str[i]);
                    break;
                }
                // Check right
                if (str[i+j] == guardRight)
                {
                    rDetect = true;
                    //printf("find r1: [%c]-> %c\n\r", str[i], str[i+j]);
                    break;
                }
                //printf("not borders: %c <-[%c]-> %c\n\r", str[i-j], str[i], str[i+j]);
            }

            // Try to find rBorder to the end of string, but if you find lBoarder -> abort
            if (lDetect && !rDetect)
            {
                for (int j = i; j < len; j++)
                {
                    if (str[j] == guardLeft)
                    {
                        break;
                    }
                    if (str[j] == guardRight)
                    {
                        //printf("find r2: [%c]-> %c\n\r", str[i], str[j]);
                        rDetect = true;
                        break;
                    }
                }
            }

            // Or to find lBorder to the end of string, but if you find rBoarder -> abort
            if (!lDetect && rDetect)
            {
                for (int j = i; j > 0; j--)
                {
                    if (str[j] == guardRight)
                    {
                        break;
                    }

                    if (str[j] == guardLeft)
                    {
                        //printf("find l2: %c <-[%c]\n\r", str[j], str[i]);
                        lDetect = true;
                        break;
                    }
                }
            }

            // Move all the char following the char "c" by one to the left if they not included in guard
            if (!lDetect || !rDetect)
            {
                //printf("remove [%d] | lDetect %d | rDetect %d\n\r", i, lDetect, rDetect);
                strncpy(&str[i], &str[i + 1], len - i);
                //printf("%s\n\r", str);
            }
            else
            {
                //printf("keep [%d] | lDetect %d | rDetect %d\n\r", i, lDetect, rDetect);
                //printf("%s\n\r", str);
            }

            lDetect = false;
            rDetect = false;
        }
    }
}
