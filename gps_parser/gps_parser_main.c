/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_parser.h"
#include "gps_parser_main.h"
#include "GPSense_conf.h"
#include "gps_utility.h"
#include "gps_def.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))
#define MSG_ID_MAX_LENGTH (8)
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/
static bool main_parser_func(const uint8_t * p_data, uint32_t data_len);
static int get_parser_func_idx(const gps_parser_talker_id_t talker_id, const uint8_t * msg_id);


/************************************************************************************************************/
/*                                          MODULE VARIABLE                                                 */
/************************************************************************************************************/
static const sub_parser_func_t sub_parser_table[] = 
{
  {
    .msg_id = "GGA",
    .talker_id_support = GPS_TALKER_ID_GPS | GPS_TALKER_ID_GALILEO | GPS_TALKER_ID_BEIDOU | GPS_TALKER_ID_GLONASS | GPS_TALKER_ID_COMBINE,
    .parser_func = SUB_PARSER_FUNC(GGA)
  },
  {
    .msg_id = "RMC",
    .talker_id_support = GPS_TALKER_ID_GPS | GPS_TALKER_ID_GALILEO | GPS_TALKER_ID_BEIDOU | GPS_TALKER_ID_GLONASS | GPS_TALKER_ID_COMBINE,
    .parser_func = SUB_PARSER_FUNC(RMC)
  },
  {
    .msg_id = "GLL",
    .talker_id_support = GPS_TALKER_ID_GPS | GPS_TALKER_ID_GALILEO | GPS_TALKER_ID_BEIDOU | GPS_TALKER_ID_GLONASS | GPS_TALKER_ID_COMBINE,
    .parser_func = SUB_PARSER_FUNC(GLL),
  },
  {
    .msg_id = "GSV",
    .talker_id_support = GPS_TALKER_ID_GPS | GPS_TALKER_ID_GALILEO | GPS_TALKER_ID_BEIDOU | GPS_TALKER_ID_GLONASS | GPS_TALKER_ID_COMBINE,
    .parser_func = SUB_PARSER_FUNC(GSV),
  }
};
/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void gps_parser_main_init()
{
  GPS_LOGI("GPS type parser init");
  gps_parser_register_parser_func(main_parser_func);
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/
static bool main_parser_func(const uint8_t * p_data, uint32_t data_len)
{
  // Get talker id
  gps_parser_talker_id_t talker_id = gps_parser_get_talker_id(p_data, data_len);
  if(talker_id == GPS_TALKER_ID_UNKNOW || talker_id == GPS_TALKER_ID_INVALID)
  {
    GPS_LOGV("talker id get fail:%d", talker_id);
    return false;
  }
  // Get message id
  uint8_t msg_id[MSG_ID_MAX_LENGTH];
  if(!gps_parser_get_message_id(p_data, data_len, msg_id))
  {
    GPS_LOGV("Fail to get message id");
    return false;
  }
  // get the sub parser function
  int sub_parser_idx = get_parser_func_idx(talker_id, msg_id);
  if(sub_parser_idx == -1 || sub_parser_table[sub_parser_idx].parser_func == NULL)
  {
    uint8_t nmea_header[16] = {};
    get_nmea_header(p_data, data_len, nmea_header);
    GPS_LOGV("Unsupported NMEA header:%s", nmea_header);
    return false;
  }
  // Create param for sub parser
  sub_parser_param_t parser_param = {0};
  parser_param.talker_id = talker_id;
  // Discard the NMEA header so that subparser does not have to do
  uint32_t comma_idx = char_find_idx(p_data, data_len, ',');
  if(comma_idx == -1)
  {
    GPS_LOGD("Can not discard NMEA header");
    return false;
  }
  parser_param.p_msg_id = p_data + 3; // 2 byte talker id, 1 byte for '$'
  parser_param.msg_id_len = comma_idx - 3;
  p_data += (comma_idx + 1);
  data_len -= (comma_idx + 1);

  parser_param.p_data = p_data;
  parser_param.data_len = data_len;
  // Excute the sub parser 
  GPS_LOGV("Found sub parser idx:%d", sub_parser_idx);
  sub_parser_table[sub_parser_idx].parser_func(parser_param);
  return true;
}

static int get_parser_func_idx(const gps_parser_talker_id_t talker_id, const uint8_t * p_msg_id)
{
  if(!p_msg_id) return -1;
  for(uint16_t i = 0; i < ARRAY_LENGTH(sub_parser_table); i++)
  {
    // check if match talker id and msg id
    if((talker_id & sub_parser_table[i].talker_id_support) && 
       (0 == memcmp(p_msg_id, sub_parser_table[i].msg_id, strlen(sub_parser_table[i].msg_id))))
      {
        return i;
      }
  }
  return -1;
}

/************************************************************************************************************/
/*                                          SUB PARSER                                                      */
/************************************************************************************************************/
SUB_PARSER_DEFINE(GLL)
{
  GPS_LOGV("%.*s", data.data_len, data.p_data);

}
SUB_PARSER_DEFINE(GSV)
{
  GPS_LOGV("%.*s", data.data_len, data.p_data);

}
