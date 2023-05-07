#ifndef __GPSENSE_CONFIG_H__
#define __GPSENSE_CONFIG_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include <stdio.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define CONFIG_GPS_SKIP_CHECKSUM_CHECK  (0)
#define CONFIG_GPS_RCV_BUFFER_LEN       (256)


// PRINTF MACRO HERE
#define GPS_DEBUG_LEVEL_INFO    1
#define GPS_DEBUG_LEVEL_ERROR   2
#define GPS_DEBUG_LEVEL_DEBUG   3
#define GPS_DEBUG_LEVEL_VERBOSE 4
#define GPS_PRINTF(...)         printf(__VA_ARGS__)

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL GPS_DEBUG_LEVEL_INFO
#endif

#if DEBUG_LEVEL >= GPS_DEBUG_LEVEL_INFO
  #define GPS_LOGI(fmt, ...) GPS_PRINTF("GPS\t[I]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#else
  #define GPS_LOGI(format, ...)
#endif

#if DEBUG_LEVEL >= GPS_DEBUG_LEVEL_ERROR
  #define GPS_LOGE(fmt, ...) GPS_PRINTF("GPS\t[E]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#else
  #define GPS_LOGE(format, ...)
#endif

#if DEBUG_LEVEL >= GPS_DEBUG_LEVEL_DEBUG
  #define GPS_LOGD(fmt, ...) GPS_PRINTF("GPS\t[D]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#else
  #define GPS_LOGD(format, ...)
#endif

#if DEBUG_LEVEL >= GPS_DEBUG_LEVEL_VERBOSE
  #define GPS_LOGV(fmt, ...) GPS_PRINTF("GPS\t[V]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#else
  #define GPS_LOGV(format, ...)
#endif



/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/


/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/

#endif