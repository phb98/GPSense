/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_parser.h"
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
#define SUB_PARSER_FUNC(name) name##_parser
#define SUB_PARSER_DEFINE(name) void name##_parser(sub_parser_param_t data)
#define MSG_ID_MAX_LENGTH (8)
#define NMEA_MESSAGE_TERMINATOR ('*')
#define IS_DATA_FIELD_AVAILABLE(p_data) (!(*((char*)p_data + 1) == ',' || *((char*)p_data + 1) == '*'))
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/

typedef struct 
{
  gps_parser_talker_id_t talker_id;
  const uint8_t * p_msg_id;
  size_t msg_id_len;
  const uint8_t * p_data;
  size_t data_len;
} sub_parser_param_t;
typedef struct
{
  const char * msg_id;
  const gps_parser_talker_id_t talker_id_support;
  void (*parser_func)(sub_parser_param_t data);
} sub_parser_func_t;

typedef enum
{
  GGA_UTC,
  GGA_LAT,
  GGA_LAT_DIR,
  GGA_LON,
  GGA_LON_DIR,
  GGA_QUALITY,
  GGA_NUM_SAT,
  NUM_OF_GGA_SUPPORT,
} gga_data_t; // must be in order of data appear in message
/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/
static bool main_parser_func(const uint8_t * p_data, uint32_t data_len);
static int get_parser_func_idx(const gps_parser_talker_id_t talker_id, const uint8_t * msg_id);

SUB_PARSER_DEFINE(GGA);
SUB_PARSER_DEFINE(GLL);
SUB_PARSER_DEFINE(GSV);

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
void gps_parser_gps_init()
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
  parser_param.p_msg_id = p_data;
  // Discard the NMEA header so that subparser does not have to do
  uint32_t comma_idx = char_find_idx(p_data, data_len, ',');
  if(comma_idx == -1)
  {
    GPS_LOGD("Can not discard NMEA header");
    return false;
  }
  p_data += (comma_idx + 1);
  data_len -= (comma_idx + 1);

  parser_param.msg_id_len = comma_idx;
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
SUB_PARSER_DEFINE(GGA)
{
  // sanity check
  if(!data.p_data)
  {
    GPS_LOGE("Invalid input");
    return;
  }
  #define GPS_LOG_POS(type, pos, dir) GPS_LOGV("GGA %s:%f %c", type, pos, dir)
  GPS_LOGV("%.*s", data.data_len, data.p_data);
  uint8_t temp_buffer[32];
  memset(temp_buffer, 0x0, sizeof(temp_buffer));
  struct 
  {
    gps_clock_time_t utc_time;
    gps_pos_t   longtitude;
    gps_pos_t   latitude;
    uint8_t num_sat;
  } parsed_data =
  {
    .latitude.pos_type =   GPS_POS_LAT,
    .longtitude.pos_type = GPS_POS_LONG
  };
  const uint8_t * p_start = data.p_data;
  const uint8_t * p_end  = p_start + data.data_len - 1;
  gga_data_t currert_parsing_data = GGA_UTC;
  int comma_idx = -1;
  GPS_LOGV("Start:%c - End:%c", *p_start, *p_end);
  while(p_start <= p_end && (currert_parsing_data < NUM_OF_GGA_SUPPORT))
  {
    if(*p_start == NMEA_MESSAGE_TERMINATOR)
    {
      GPS_LOGV("Sub parser done");
      break;
    }
    // parser each data field
    comma_idx = char_find_idx(p_start, p_end - p_start + 1, ',');    
    if(comma_idx == -1) 
    {
      GPS_LOGD("Cannot find next ',', break");
      break;
    }
    else if(comma_idx > 0)
    {
      // this data field has data
      GPS_LOGV("Comma_idx:%d", comma_idx);
      memcpy(temp_buffer, p_start, comma_idx);
      GPS_LOGV("Field %d: %.*s, len:%d", currert_parsing_data, comma_idx, temp_buffer, strlen((const char *)temp_buffer));
      switch(currert_parsing_data)
      {
        case GGA_UTC:
        {
          // make sure we have enough data
          if(comma_idx < 10)
          {
            GPS_LOGD("Field %d not have enough data", currert_parsing_data);
            break;
          }
          // Parsing utc time
          const char * p_temp = (const char *) p_start;
          parsed_data.utc_time.hour = str2int(p_temp, 2); p_temp +=2;
          parsed_data.utc_time.minute = str2int(p_temp, 2);p_temp += 2;
          parsed_data.utc_time.second = str2int(p_temp, 2);p_temp += 3;
          parsed_data.utc_time.centisecond = str2int(p_temp, 3);
          GPS_LOGD("UTC time:%02d:%02d:%02d.%0d",parsed_data.utc_time.hour, parsed_data.utc_time.minute,
                                                  parsed_data.utc_time.second, parsed_data.utc_time.centisecond);
          break;
        }
        case GGA_LAT:
        {
          const char * p_temp = (const char *)p_start;
          parsed_data.latitude.pos = str2int(p_temp, 2); p_temp +=2;
          parsed_data.latitude.pos += (atof(p_temp) / 60.0);
          GPS_LOGD("Latitude:%f", parsed_data.latitude.pos);
          break;
        }
        case GGA_LAT_DIR:
        {
          if(*p_start == 'N') parsed_data.latitude.dir = GPS_DIR_NORTH;
          else parsed_data.latitude.dir = GPS_DIR_SOUTH; // this risky, but I will take it anyway
          GPS_LOGD("Latitude Dir:%c", parsed_data.latitude.dir);
          break;
        }
        case GGA_LON_DIR:
        {
          if(*p_start == 'W') parsed_data.longtitude.dir = GPS_DIR_WEST;
          else parsed_data.longtitude.dir = GPS_DIR_EAST; // this risky, but I will take it anyway
          GPS_LOGD("Longtitude Dir:%c", parsed_data.longtitude.dir);
          break;
        }
        case GGA_LON:
        {
          const char *p_temp = (const char *)p_start;
          parsed_data.longtitude.pos = str2int(p_temp, 3); p_temp +=3;
          parsed_data.longtitude.pos += (atof(p_temp) / 60.0);
          GPS_LOGD("Longtitude:%f", parsed_data.longtitude.pos);
          break;
        }
        case GGA_QUALITY:
        {
          if(*p_start == 0) GPS_LOGD("Not fixed"); else GPS_LOGD("Fixed");
          break;
        }
        case GGA_NUM_SAT:
        {
          parsed_data.num_sat = str2int((const char *)p_start, 2);
          GPS_LOGD("Number Satellites:%d", parsed_data.num_sat);
        }
        default:
        break;
      }
      memset(temp_buffer, 0x0, sizeof(temp_buffer));
      p_start += (comma_idx + 1);
    }
    else
    {
      // This data field has no data, try next field
      GPS_LOGV("Field:%d has no data", currert_parsing_data);
      p_start++;
    }
    currert_parsing_data++;
  }
}
SUB_PARSER_DEFINE(GLL)
{
  GPS_LOGV("%.*s", data.data_len, data.p_data);

}
SUB_PARSER_DEFINE(GSV)
{
  GPS_LOGV("%.*s", data.data_len, data.p_data);

}
