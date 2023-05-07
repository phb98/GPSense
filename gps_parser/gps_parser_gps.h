#ifndef __GPS_PARSER_GPS_H__
#define __GPS_PARSER_GPS_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include "gps_parser.h"
#include "gps_def.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/
#define SUB_PARSER_FUNC(name) name##_parser
#define SUB_PARSER_DEFINE(name) void name##_parser(sub_parser_param_t data)

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
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/


/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
// sub parser function
void gps_parser_gps_init();

SUB_PARSER_DEFINE(GGA);
SUB_PARSER_DEFINE(GLL);
SUB_PARSER_DEFINE(GSV);
#endif