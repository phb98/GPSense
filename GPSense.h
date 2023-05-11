#ifndef __GPSENSE_H__
#define __GPSENSE_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "gps_def.h"
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/
typedef enum
{
  LAT_DIR_NORTH,
  LAT_DIR_SOUTH,
} GPSense_lat_dir_t;

typedef enum
{
  LONG_DIR_WEST,
  LONG_DIR_EAST,
} GPSense_long_dir_t;

typedef enum
{
  NO_FIX,
  FIXED_2D,
  FIXED_3D,
} GPSense_fix_t;
typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
} GPSense_time_t;
typedef gps_current_data_t GPSense_data_t;

/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void GPSense_init();
bool GPSense_feed(const char * p_data, const uint32_t data_len);
void GPSense_get_data(GPSense_data_t * const p_data);
#endif