#ifndef __GPS_DATA_H__
#define __GPS_DATA_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_def.h"
#include "gps_parser.h"
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/
typedef enum 
{
  GPS_DATA_UTC_CLOCK_TIME,
  GPS_DATA_LONGTITUDE,
  GPS_DATA_LATITUDE,
  GPS_DATA_FIX_STATUS,
} gps_new_data_type_t;

typedef struct
{
  gps_new_data_type_t type;
  const uint8_t * p_msg_id;
  uint16_t msg_id_len;
  gps_parser_talker_id_t talker_id;
} gps_new_data_hdr_t;

typedef struct
{
  gps_new_data_hdr_t new_data_hdr;
  union
  {
    gps_clock_time_t utc_clock_time;
    gps_pos_t longtitude;
    gps_pos_t latitude;
  } data;
} gps_new_data_t;

/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/

void gps_data_init();
bool gps_data_add(const gps_new_data_t * p_new_data);
#endif