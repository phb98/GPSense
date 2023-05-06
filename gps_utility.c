/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_utility.h"
#include <stdint.h>
#include <stdio.h>
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

/************************************************************************************************************/
/*                                     PRIVATE FUNCTION PROTOTYPES                                          */
/************************************************************************************************************/

/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void mem_shift_left(uint8_t *p_buffer, uint32_t buffer_len, uint32_t num_shift)
{
  if(!p_buffer || buffer_len == 0 || num_shift > buffer_len) return;
  memmove(p_buffer, &p_buffer[num_shift], buffer_len - num_shift);
  memset(&p_buffer[buffer_len - num_shift], 0x0, num_shift);
}

int char_find_idx(const uint8_t * p_buffer, const uint32_t buffer_len, const char ch)
{
  if(!p_buffer || buffer_len == 0) return -1;
  uint8_t * p_ch = memchr(p_buffer, ch, buffer_len);
  if(p_ch != NULL) return (int)(p_ch - p_buffer);
  else return -1;
}

/**
 * hex_str_to_int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 */
uint32_t hex_str_to_int(const char *hex_str, uint32_t str_len) 
{
  if(!str_len) return 0;
  uint32_t val = 0;
  while (str_len) {
    str_len--;
    // get current character then increment
    uint8_t byte = *hex_str++; 
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (byte >= '0' && byte <= '9') byte = byte - '0';
    else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
    // shift 4 to make space for new digit, and add the 4 bits of the new digit 
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}
void get_nmea_header(const uint8_t * p_data, uint32_t data_len, uint8_t *p_ret)
{
  if(!p_data || !p_ret || !data_len) return;
  int comma_idx = char_find_idx(p_data, data_len, ',');
  if(comma_idx != -1)
  {
    memcpy(p_ret, p_data, comma_idx);
    p_ret[comma_idx] = '\0';
  }
  else p_ret[0] = '\0';
}

int str2int(const char* str, int len)
{
    int i;
    int ret = 0;
    for(i = 0; i < len; ++i)
    {
        if(str[i] > '9' || str[i] < '0') return 0;
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}
/************************************************************************************************************/
/*                                          PRIVATE FUNCTION                                                */
/************************************************************************************************************/