#ifndef __GPS_PARSER_H__
#define __GPS_PARSER_H__
/************************************************************************************************************/
/*                                              INCLUDE                                                     */
/************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
/************************************************************************************************************/
/*                                     MACRO AND CONSTANT DEFINE                                            */
/************************************************************************************************************/


#define GPS_TALKER_ID_INVALID           (0)
#define GPS_TALKER_ID_GPS               (1UL << 0)
#define GPS_TALKER_ID_GALILEO           (1UL << 2)
#define GPS_TALKER_ID_BEIDOU            (1UL << 3)
#define GPS_TALKER_ID_GLONASS           (1UL << 4)
#define GPS_TALKER_ID_COMBINE           (1UL << 5)
#define GPS_TALKER_ID_UNKNOW            (1Ul << 31)
typedef uint32_t gps_parser_talker_id_t;

typedef bool (*gps_parser_func_t)(const uint8_t * p_data, uint32_t data_len);
/************************************************************************************************************/
/*                                            MODULE TYPE                                                   */
/************************************************************************************************************/


/************************************************************************************************************/
/*                                           PUBLIC FUNCTION                                                */
/************************************************************************************************************/
void gps_parser_init();
bool gps_parser_push(const uint8_t * p_data, uint32_t data_len);
bool gps_parser_register_parser_func(gps_parser_func_t parser_func);
gps_parser_talker_id_t gps_parser_get_talker_id(const uint8_t * p_data, uint32_t data_len);
bool gps_parser_get_message_id(const uint8_t * p_data, uint32_t data_len, uint8_t * p_ret);
// sub parser function
void gps_parser_common_init();
void gps_parser_gps_init();
void gps_parser_galileo_init();
#endif