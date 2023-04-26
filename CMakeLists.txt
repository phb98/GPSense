cmake_minimum_required(VERSION 3.5)
project(GPSense VERSION 1.0.0)
message("Hello World")
set(GPSense_source GPSense.c 
                   gps_utility.c
                   gps_parser/gps_parser.c
                   gps_parser/gps_parser_gps.c)
add_library(GPSense ${GPSense_source})
target_include_directories(GPSense PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                           "./gps_parser")
