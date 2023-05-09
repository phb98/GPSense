#ifndef __GPS_DEF_H__
#define __GPS_DEF_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/
typedef enum
{
  GPS_DIR_WEST = 'W',
  GPS_DIR_EAST = 'E',
  GPS_DIR_NORTH = 'N',
  GPS_DIR_SOUTH = 'S',
} gps_dir_t;
typedef enum
{
  GPS_UNIT_METER = 'M',
  GPS_UNIT_FEET  = 'F'
} gps_unit_t;
typedef enum
{
  GPS_POS_LONG,
  GPS_POS_LAT,
  GPS_POS_ALTITUDE,
  NUM_OF_GPS_POS_T,
} gps_pos_type_t;
typedef float gps_long_lat_t;
typedef struct
{
  gps_long_lat_t      pos;
  gps_pos_type_t      pos_type;
  gps_dir_t           dir;
  gps_unit_t          unit;
} gps_pos_t;

typedef struct
{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t centisecond;
} gps_clock_time_t;

typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
} gps_date_time_t;
typedef enum
{
  GPS_NO_FIX,
  GPS_FIXED_2D,
  GPS_FIXED_3D,
  NUM_OF_GPS_FIX_T,
} gps_fix_t;
/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/

#endif