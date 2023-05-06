/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "GPSense.h"
#include "GPSense_conf.h"
#include "gps_parser.h"
#include "gps_utility.h"
#include <stdint.h>
#include <string.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define GPS_START_MSG_CHAR  ('$')
#define GPS_END_MSG_CHAR    ('\n')

/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                          MODULE VARIABLE                                                 */
/************************************************************************************************************/
static struct
{
  uint8_t rcv_buffer[CONFIG_GPS_RCV_BUFFER_LEN];
  uint32_t rcv_idx;
} gps;
/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/
static void gps_process();
static void gps_send_nmea_msg(uint8_t * p_decoded, uint32_t len);

/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void GPSense_init()
{
  GPS_LOGI("Init GPS Library");
  gps_parser_init();
}

bool GPSense_feed(const char * p_data, const uint32_t data_len)
{
  //sanity check
  if(!p_data || data_len == 0) return false;
  if(data_len + gps.rcv_idx > CONFIG_GPS_RCV_BUFFER_LEN)
  {
    // that nmea message is too long, flush the buffer
    GPS_LOGE("NMEA message too long");
    memset(gps.rcv_buffer, 0x0, sizeof(gps.rcv_buffer));
    gps.rcv_idx = 0;
  }
  GPS_LOGV("GPS feed data:%.*s len:%d", data_len, p_data, data_len);
  //copy data to buffer to process
  memcpy(&gps.rcv_buffer[gps.rcv_idx], (uint8_t*) p_data, data_len);
  gps.rcv_idx += data_len;
  gps_process();
  return true;
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/
static void gps_process()
{
  //find the first '$'
  int start_idx = char_find_idx(gps.rcv_buffer, CONFIG_GPS_RCV_BUFFER_LEN, GPS_START_MSG_CHAR);
  int end_idx = 0;
  if(start_idx == -1)
  {
    // invalid buffer data, all msg must have at least 1 '$'
    GPS_LOGE("Invalid receive buffer, flush buffer");
    memset(gps.rcv_buffer, 0x0, sizeof(gps.rcv_buffer));
    gps.rcv_idx = 0;
    return;
  }
  // Find the idx = '$', expect 0
  if(start_idx > 0)
  {
    // all the data before '$' is consider invalid, let shift buffer so '$' is first index
    GPS_LOGE("Dropping invalid data, %c idx:%d", GPS_START_MSG_CHAR, start_idx);
    mem_shift_left(gps.rcv_buffer, CONFIG_GPS_RCV_BUFFER_LEN, start_idx);
    gps.rcv_idx -= start_idx;
    start_idx = 0; // now we sure the idex 0 is '$'
  }
  while(gps.rcv_buffer[0] == GPS_START_MSG_CHAR)
  {
    int temp;
    end_idx = char_find_idx(gps.rcv_buffer, CONFIG_GPS_RCV_BUFFER_LEN, GPS_END_MSG_CHAR);
    if(end_idx == -1)
    {
      // did not find the end message, maybe the message is not finished
      GPS_LOGV("Process message: not finised");
      break;
    }
    // if there is other '$' between start and end idx, consider that message is invalid
    temp = char_find_idx(gps.rcv_buffer+1, end_idx, GPS_START_MSG_CHAR);
    if(temp >= 0)
    {
      GPS_LOGE("Has '$' in unfinished message, drop it");
      mem_shift_left(gps.rcv_buffer, CONFIG_GPS_RCV_BUFFER_LEN, temp + 1);
      gps.rcv_idx -= (temp + 1);
      continue;
    }
    if(end_idx > 2) 
    {
      // Found valid message, pass to other layer
      gps_send_nmea_msg(&gps.rcv_buffer[0], end_idx - 2 + 1);
    }
    mem_shift_left(&gps.rcv_buffer[0], CONFIG_GPS_RCV_BUFFER_LEN, end_idx + 1);
    gps.rcv_idx -= (end_idx + 1);
  }
  
}

static void gps_send_nmea_msg(uint8_t * p_msg, uint32_t len)
{
  if(!p_msg || len == 0) return;
  GPS_LOGD("%.*s", len, p_msg);
  if(!gps_parser_push(p_msg, len)) GPS_LOGE("push NMEA msg to parser fail");
}
