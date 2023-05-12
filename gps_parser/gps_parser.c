/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#define DEBUG_LEVEL 1
#include "GPSense.h"
#include "gps_parser.h"
#include "GPSense_conf.h"
#include <string.h>
#include "gps_utility.h"
#include "gps_parser_main.h"
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define NMEA_MSG_MAX_LEN    (256)
#define MAX_PARSER_FUNC     (10)
#define CHECKSUM_START_CHAR ('*')
#define CHECKSUM_STR_LEN    (2)
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                          MODULE VARIABLE                                                 */
/************************************************************************************************************/
static struct 
{
  uint16_t rcv_slot;
  uint16_t processing_slot;
  uint16_t num_processing_msg;
  gps_parser_func_t parser_func[MAX_PARSER_FUNC];
  uint8_t num_parser_func;
} gps_parser;

/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/
static bool gps_parser_validate(const uint8_t * p_data, uint32_t data_len);
static void gps_parser_process(const uint8_t * p_data, uint32_t data_len);
/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void gps_parser_init()
{
  GPS_LOGI("Init Parser");
  gps_parser_main_init();
}
bool gps_parser_push(const uint8_t * p_data, uint32_t data_len)
{
  if(!p_data || data_len == 0 || data_len > NMEA_MSG_MAX_LEN) return false;
  GPS_LOGV("Receive new NMEA msg, len:%d, %.*s", data_len, data_len, p_data);
  // validate if msg is valid
  if(CONFIG_GPS_SKIP_CHECKSUM_CHECK || gps_parser_validate(p_data, data_len))
  {
    // message is valid
    GPS_LOGV("Checksum %s", CONFIG_GPS_SKIP_CHECKSUM_CHECK ? "Skipped" : "Valid");
    // Truncate the checksum path since it's not needed anymore
    data_len -= (CHECKSUM_STR_LEN);
    gps_parser_process(p_data, data_len);
  }
  else
  {
    GPS_LOGE("Checksum invalid, drop msg");
  }
  return true;
}

bool gps_parser_register_parser_func(gps_parser_func_t parser_func)
{
  if(gps_parser.num_parser_func >= MAX_PARSER_FUNC || !parser_func)
  {
    GPS_LOGE("Parser func register failed");
    return false;
  }
  gps_parser.parser_func[gps_parser.num_parser_func] = parser_func;
  gps_parser.num_parser_func++;
  return true;
}

gps_parser_talker_id_t gps_parser_get_talker_id(const uint8_t * p_data, uint32_t data_len)
{
  if(!p_data || data_len < 2) return GPS_TALKER_ID_INVALID;
  if(p_data[0] == '$' && data_len < 3) return GPS_TALKER_ID_INVALID;
  else {
    p_data++; // skip '$'
    data_len--;
  }
  const struct {
    const char * talker_id_str;
    const uint32_t talker_id;
  } talker_id_table[] = 
  {
    {
      .talker_id_str = "GP",
      .talker_id     = GPS_TALKER_ID_GPS,
    },
    {
      .talker_id_str = "GA",
      .talker_id     = GPS_TALKER_ID_GALILEO,
    },
    {
      .talker_id_str = "GL",
      .talker_id     = GPS_TALKER_ID_GLONASS,
    },
    {
      .talker_id_str = "GB",
      .talker_id     = GPS_TALKER_ID_BEIDOU,
    },
    {
      .talker_id_str = "BD",
      .talker_id     = GPS_TALKER_ID_BEIDOU,
    },
    {
      .talker_id_str = "GN",
      .talker_id     = GPS_TALKER_ID_COMBINE,
    },
  };
  for(uint8_t i = 0; i < (sizeof(talker_id_table) / sizeof(talker_id_table[0])); i++)
  {
    if(strlen(talker_id_table[i].talker_id_str) < 2)
    {
      GPS_LOGE("look up table invalid id:%d", i);
      continue;
    }
    if(memcmp(talker_id_table[i].talker_id_str, p_data, strlen(talker_id_table[i].talker_id_str)) == 0)
    {
      GPS_LOGV("Talker id:%d", talker_id_table[i].talker_id);
      return talker_id_table[i].talker_id;
    }
  }
  return GPS_TALKER_ID_UNKNOW;
}

/**
 * Functions to get message id from nmea message
 * Assuming nmea talker id is 2 byte long
*/
bool gps_parser_get_message_id(const uint8_t * p_data, uint32_t data_len, uint8_t * p_ret)
{
  if(!p_data || data_len < 3 || !p_ret) return false;
  if(p_data[0] == '$' && data_len < 4) return false;
  else {
    p_data++; // skip '$'
    data_len--;
  }
  // skip the first 2 byte talker id
  p_data += 2;
  data_len -=2;
  //get the idx of ','
  int comma_idx = char_find_idx(p_data, data_len, ',');
  if( comma_idx == -1) return false; // fail to get idx of ','
  // copy to return buffer
  memcpy(p_ret, p_data, comma_idx);
  GPS_LOGV("Message id: %.*s", comma_idx, p_ret);
  return true;
}
/**
 * This function return a string contain data at num field in nmea message
 * String is null terminated
 * This assume nmea header is discarded
*/
bool gps_parser_get_data_field(const uint8_t * p_data, uint32_t data_len, 
                               const uint8_t num_field, uint8_t * const p_ret, const uint32_t ret_max_len)
{
  // sanity check
  if(!p_data || !p_ret || ret_max_len == 0 || data_len == 0)
  {
    GPS_LOGE("Invalid Input");
    return false;
  }
  memset(p_ret, 0x0, ret_max_len);
  const uint8_t * p_start = p_data;
  const uint8_t * p_end   = p_data + data_len - 1;
  uint8_t temp_buffer[32];
  uint8_t current_num_field = 0;
  while(p_start <= p_end && (current_num_field <= num_field))
  {
    if(*p_start == CHECKSUM_START_CHAR)
    {
      GPS_LOGV("Sub parser done");
      break;
    }
    // parser each data field
    uint32_t comma_idx = char_find_idx(p_start, p_end - p_start + 1, ',');    
    if(comma_idx == -1) 
    {
      GPS_LOGV("Cannot find next ',', break");
      break;
    }
    else if(comma_idx > 0)
    {
      // this data field has data
      GPS_LOGV("Comma_idx:%d", comma_idx);
      memcpy(temp_buffer, p_start, comma_idx);
      GPS_LOGV("Field %d: %.*s, len:%d", current_num_field, comma_idx, temp_buffer, strlen((const char *)temp_buffer));
      // at the field we want
      if(current_num_field == num_field)
      {
        GPS_LOGV("Found field");
        if(comma_idx < ret_max_len){ 
          memcpy(p_ret, p_start, comma_idx);
          return true;
        }
        else return false;
      }
      memset(temp_buffer, 0x0, sizeof(temp_buffer));
      p_start += (comma_idx + 1);
    }
    else
    {
      // This data field has no data, try next field
      GPS_LOGV("Field:%d has no data", current_num_field);
      p_start++;
    }
    current_num_field++;
  }
  GPS_LOGD("Can not find data field:%d", num_field);
  return false; // fail to find
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/
static void gps_parser_process(const uint8_t * p_data, uint32_t data_len)
{
  bool msg_processed = false;
  // call each parser function
  for(uint16_t i = 0; i < gps_parser.num_parser_func; i++)
  {
    if(gps_parser.parser_func[i])
    {
      if(gps_parser.parser_func[i](p_data, data_len)) msg_processed = true;
    }
  }
  if(!msg_processed)
  {
    uint8_t nmea_header[16] = {0};
    get_nmea_header(p_data, data_len, nmea_header);
    GPS_LOGD("No parser for this NMEA: %s", nmea_header);
  } 
}

static bool gps_parser_validate(const uint8_t * p_data, uint32_t data_len)
{
  if(!p_data) return false;
  // ignore the $
  if(*p_data == '$') {
    p_data++; 
    data_len--;
  }
  // check if message has checksum
  if((data_len < (CHECKSUM_STR_LEN + 1)) || (p_data[data_len - 1 - CHECKSUM_STR_LEN] != CHECKSUM_START_CHAR)) return false;
  uint32_t expect_checksum = hex_str_to_int((const char *)&p_data[data_len-CHECKSUM_STR_LEN], CHECKSUM_STR_LEN);
  uint32_t calculated_checksum = 0;
  // calculate checksum by xor together
  for(uint32_t i = 0; i < (data_len - 1 - CHECKSUM_STR_LEN); i++)
  {
    calculated_checksum ^= p_data[i];
  }
  GPS_LOGV("Expect:0x%x, calculate:0x%x", expect_checksum, calculated_checksum);
  if(calculated_checksum == expect_checksum) return true; 
  else return false;
}