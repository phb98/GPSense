/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "GPSense.h"
#include "gps_data.h"
#include "GPSense_conf.h"
#include "gps_def.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                          MODULE VARIABLE                                                 */
/************************************************************************************************************/
// This struct contained all the data that has been parsed, this struct is updated by all sub parser
typedef struct
{
  gps_clock_time_t utc_time;
  uint16_t num_active_sat;
} gps_current_data_t;
static gps_current_data_t gps_current_data;
/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/
static bool gps_data_update(gps_current_data_t * const p_cur_data, const gps_new_data_t * const p_new_data);
/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void gps_data_init()
{
  GPS_LOGI("Gps data init");
}
bool gps_data_add(const gps_new_data_t * p_new_data)
{
  return gps_data_update(&gps_current_data, p_new_data);
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/

static bool gps_data_update(gps_current_data_t * const p_cur_data, const gps_new_data_t * const p_new_data)
{
  if(!p_new_data || !p_cur_data)
  {
    GPS_LOGE("Can not add new data, NULL ptr");
    return false;
  }
  // skip the '$' if needed
  const uint8_t * p_msg_id = p_new_data->new_data_hdr.p_msg_id;
  uint16_t msg_id_len = p_new_data->new_data_hdr.msg_id_len;
  if(*(p_new_data->new_data_hdr.p_msg_id) == '$') 
  {
    p_msg_id++;
    msg_id_len--;
  }
  switch(p_new_data->new_data_hdr.type)
  {
    case GPS_DATA_UTC_CLOCK_TIME:
    {
      // Check if we allow update utc time from this message ID
      if(memcmp(p_msg_id, CONFIG_GPS_UTC_TIME_MSG_ID, msg_id_len) == 0)
      {
        // Update UTC time
        GPS_LOGD("Update UTC time:%02d:%02d:%02d.%03d", p_new_data->data.utc_clock_time.hour, 
                                                        p_new_data->data.utc_clock_time.minute,
                                                        p_new_data->data.utc_clock_time.second,
                                                        p_new_data->data.utc_clock_time.centisecond);
        memcpy(&(p_cur_data->utc_time), &(p_new_data->data.utc_clock_time), sizeof(gps_clock_time_t));
      }
      break;
    }
    default:
    GPS_LOGE("Can not add new GPS data, unknow type");
    break;
  }
  return true;
}