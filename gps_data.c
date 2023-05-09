/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#define DEBUG_LEVEL 4
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
  gps_pos_t longtitude;
  gps_pos_t latitiude;
  gps_pos_t altitude;
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
  #define IS_MSG_ID_EXPECT(EXPECT_MSGID)   (memcmp(p_msg_id, EXPECT_MSGID, msg_id_len) == 0)
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
      #ifdef CONFIG_GPS_UTC_TIME_MSG_ID
      if(IS_MSG_ID_EXPECT(CONFIG_GPS_UTC_TIME_MSG_ID))
      #endif
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
    case GPS_DATA_POSITION:
    #ifdef CONFIG_GPS_COORDINATE_MSG_ID
    if(IS_MSG_ID_EXPECT(CONFIG_GPS_COORDINATE_MSG_ID))
    #endif
    {
      GPS_LOGD("Update Position(Lat Lon):%f %c, %f %c", p_new_data->data.pos.latitude.pos, p_new_data->data.pos.latitude.dir,
                                                        p_new_data->data.pos.longtitude.pos, p_new_data->data.pos.longtitude.dir);
      p_cur_data->longtitude.dir = p_new_data->data.pos.longtitude.dir;
      p_cur_data->longtitude.pos = p_new_data->data.pos.longtitude.pos;
      p_cur_data->latitiude.dir  = p_new_data->data.pos.latitude.dir;
      p_cur_data->latitiude.pos  = p_new_data->data.pos.latitude.pos;
      p_cur_data->latitiude.pos_type   = GPS_POS_LAT;
      p_cur_data->longtitude.pos_type  = GPS_POS_LONG;
      break;
    }
    case GPS_DATA_NUM_SAT:
    #ifdef CONFIG_GPS_NUM_SAT_MSG_ID
    if(IS_MSG_ID_EXPECT(CONFIG_GPS_NUM_SAT_MSG_ID))
    #endif
    {
      GPS_LOGD("Update num sat:%d", p_new_data->data.num_sat);
      p_cur_data->num_active_sat = p_new_data->data.num_sat;
      break;
    }
    case GPS_DATA_ALTITUDE:
    #ifdef CONFIG_GPS_ALTITUDE_MSG_ID
    if(IS_MSG_ID_EXPECT(CONFIG_GPS_NUM_SAT_MSG_ID))
    #endif
    {
      if(p_new_data->data.altitude.unit != GPS_UNIT_METER)
      {
        GPS_LOGE("Only support altitude meter for now");
        break;
      }
      GPS_LOGD("Update Altitude:%f %c", p_new_data->data.altitude.pos, p_new_data->data.altitude.unit);
      memcpy(&(p_cur_data->altitude), &(p_new_data->data.altitude), sizeof(gps_pos_t));
      p_cur_data->altitude.pos_type = GPS_POS_ALTITUDE;
      break;
    }
    default:
    GPS_LOGE("Can not add new GPS data, unknow type");
    break;
  }
  return true;
}