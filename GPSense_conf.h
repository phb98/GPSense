#ifndef __GPSENSE_CONFIG_H__
#define __GPSENSE_CONFIG_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include <stdio.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define CONFIG_GPS_RCV_BUFFER_LEN      (256)


// PRINTF MACRO HERE
#define GPS_PRINTF printf

#define GPS_LOGI(fmt, ...) GPS_PRINTF("GPS\t[I]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#define GPS_LOGE(fmt, ...) GPS_PRINTF("GPS\t[E]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#define GPS_LOGD(fmt, ...) GPS_PRINTF("GPS\t[D]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)
#define GPS_LOGV(fmt, ...) GPS_PRINTF("GPS\t[V]\t%s(): " fmt " \r\n",__func__, ##__VA_ARGS__)


/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/


/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/

#endif