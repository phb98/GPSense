/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#define DEBUG_LEVEL 4
#include "GPSense_conf.h"
#include "gps_parser.h"
#include "gps_parser_main.h"
#include "gps_def.h"
#include "gps_data.h"
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define NMEA_MESSAGE_TERMINATOR ('*')
#define DEFAULT_ALTITUDE_SEALEVEL (0xDEADBEEF)
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
  GGA_HDOP,
  GGA_ALTITUDE_SEALEVEL,
  GGA_ALTITUDE_UNIT,
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
    gps_pos_t   longtitude;
    gps_pos_t   latitude;
    gps_pos_t   alt;
  } parsed_data =
  {
    .latitude.pos_type    = GPS_POS_LAT,
    .latitude.pos         = 0,
    .longtitude.pos_type  = GPS_POS_LONG,
    .longtitude.pos       = 0,
    .alt.pos_type         = GPS_POS_ALTITUDE,
    .alt.unit             = '\0',
    .alt.pos              = DEFAULT_ALTITUDE_SEALEVEL, // random number
  };
  gps_new_data_t new_data = 
  {
    .new_data_hdr.p_msg_id = data.p_msg_id,
    .new_data_hdr.msg_id_len = data.msg_id_len,
    .new_data_hdr.talker_id = data.talker_id
  };

  for(gga_data_t currert_parsing_data = GGA_UTC; currert_parsing_data < NUM_OF_GGA_SUPPORT; currert_parsing_data++)
  {
    if(gps_parser_get_data_field(data.p_data, data.data_len, currert_parsing_data, temp_buffer, sizeof(temp_buffer)))
    {
      switch(currert_parsing_data)
      {
        case GGA_UTC:
        {
          // Parsing utc time
          const char * p_temp = (const char *) temp_buffer;
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
          const char * p_temp = (const char *)temp_buffer;
          parsed_data.latitude.pos = str2int(p_temp, 2); p_temp +=2;
          parsed_data.latitude.pos += (atof(p_temp) / 60.0);
          GPS_LOGD("Latitude:%f", parsed_data.latitude.pos);
          break;
        }
        case GGA_LAT_DIR:
        {
          if(*temp_buffer == 'N') parsed_data.latitude.dir = GPS_DIR_NORTH;
          else parsed_data.latitude.dir = GPS_DIR_SOUTH; // this risky, but I will take it anyway
          GPS_LOGD("Latitude Dir:%c", parsed_data.latitude.dir);
          break;
        }
        case GGA_LON_DIR:
        {
          if(*temp_buffer == 'W') parsed_data.longtitude.dir = GPS_DIR_WEST;
          else parsed_data.longtitude.dir = GPS_DIR_EAST; // this risky, but I will take it anyway
          GPS_LOGD("Longtitude Dir:%c", parsed_data.longtitude.dir);
          break;
        }
        case GGA_LON:
        {
          const char *p_temp = (const char *)temp_buffer;
          parsed_data.longtitude.pos = str2int(p_temp, 3); p_temp +=3;
          parsed_data.longtitude.pos += (atof(p_temp) / 60.0);
          GPS_LOGD("Longtitude:%f", parsed_data.longtitude.pos);
          break;
        }
        case GGA_QUALITY:
        {
          if(*temp_buffer == '0') GPS_LOGD("Not fixed"); else GPS_LOGD("Fixed");
          break;
        }
        case GGA_NUM_SAT:
        {
          new_data.new_data_hdr.type = GPS_DATA_NUM_SAT;
          new_data.data.num_sat = str2int((const char *)temp_buffer, 2);
          gps_data_add(&new_data);
          GPS_LOGD("Number Satellites:%d", new_data.data.num_sat);
          break;
        }
        case GGA_ALTITUDE_SEALEVEL:
        {
          parsed_data.alt.pos = atof(temp_buffer);
          break;
        }
        case GGA_ALTITUDE_UNIT:
        {
          if(*temp_buffer == 'M') parsed_data.alt.unit = GPS_UNIT_METER;
          else parsed_data.alt.unit = GPS_UNIT_FEET;
          break;
        }
        default:
        break;
      }
    }
    else GPS_LOGD("Fail to get data field:%d", currert_parsing_data);
  }
  if(parsed_data.latitude.pos != 0 && parsed_data.longtitude.pos != 0)
  {
    // Update new position
    new_data.new_data_hdr.type = GPS_DATA_POSITION;
    memcpy(&new_data.data.pos.latitude, &parsed_data.latitude, sizeof(gps_pos_t));
    memcpy(&new_data.data.pos.longtitude, &parsed_data.longtitude, sizeof(gps_pos_t));
    gps_data_add(&new_data);
  }
  if(parsed_data.alt.pos != DEFAULT_ALTITUDE_SEALEVEL && parsed_data.alt.unit != '\0')
  {
    new_data.new_data_hdr.type = GPS_DATA_ALTITUDE;
    memcpy(&new_data.data.altitude, &parsed_data.alt, sizeof(gps_pos_t));
    gps_data_add(&new_data);
    GPS_LOGD("Altitude:%f%c", parsed_data.alt.pos, parsed_data.alt.unit);
  }
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/