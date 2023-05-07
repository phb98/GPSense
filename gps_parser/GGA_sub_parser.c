/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_parser.h"
#include "gps_parser_gps.h"
#include "gps_def.h"
#include "GPSense_conf.h"
#include "gps_data.h"
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define NMEA_MESSAGE_TERMINATOR ('*')
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/
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
/*                                          MODULE VARIABLE                                                 */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
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
  gps_new_data_t new_data = 
  {
    .new_data_hdr.p_msg_id = data.p_msg_id,
    .new_data_hdr.msg_id_len = data.msg_id_len,
    .new_data_hdr.talker_id = data.talker_id
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
          new_data.data.utc_clock_time.hour   = str2int(p_temp, 2); p_temp +=2;
          new_data.data.utc_clock_time.minute = str2int(p_temp, 2);p_temp += 2;
          new_data.data.utc_clock_time.second = str2int(p_temp, 2);p_temp += 3;
          new_data.data.utc_clock_time.centisecond = str2int(p_temp, 3);
          new_data.new_data_hdr.type = GPS_DATA_UTC_CLOCK_TIME;
          gps_data_add(&new_data);
          GPS_LOGD("UTC time:%02d:%02d:%02d.%03d",new_data.data.utc_clock_time.hour, new_data.data.utc_clock_time.minute,
                                                  new_data.data.utc_clock_time.second, new_data.data.utc_clock_time.centisecond);
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
          if(*p_start == '0') GPS_LOGD("Not fixed"); else GPS_LOGD("Fixed");
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
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/