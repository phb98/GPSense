cmake_minimum_required(VERSION 3.5)
project(GPSense C CXX)
set(GPSense_source GPSense.c 
                   gps_utility.c
                   gps_data.c
                   gps_parser/gps_parser.c
                   gps_parser/gps_parser_main.c
                   gps_parser/GGA_sub_parser.c
                   gps_parser/RMC_sub_parser.c
                   gps_parser/VTG_sub_parser.c)
add_library(GPSense ${GPSense_source})
target_include_directories(GPSense PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                           "./gps_parser")
