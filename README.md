

# GPSense

GPSense is a C library for parsing NMEA messages related to GPS data. The library provides a simple and straightforward interface for decoding NMEA sentences and extracting relevant GPS data such as latitude, longitude, speed, and course.

## Features

- Parses NMEA sentences related to GPS data
- Extracts relevant GPS data such as latitude, longitude, speed, and course
- Provides a simple and straightforward interface for accessing GPS data

## Usage and example

To use GPSense in your C project, simply include the `GPSense.h` header file in your source code and link against the `GPSense` library.

GPSense provides a simple and straightforward interface for accessing GPS data. Here's an example of how to use GPSense to parse a sample NMEA sentence and extract the latitude and longitude:
```c
#include "GPSense.h"
#include <stdio.h>
#include <string.h>

const char* nmea_test_data[] = {
  "123$abc\r\n$def\r\n$hjk\r",
  "$ab$cde\r\n",
  "$GNGGA,165726.000,1049.65224,N,10641.48942,E,1,16,0.7,8.8,M,-4.1,M,,*51\r\n",
  "$GPGSV,4,2,14,22,,,20,23,57,035,30,24,20,034,17,25,39,130,29,0*53\r\n",
  "$BDGSV,2,1,05,22,37,044,23,26,43,198,39,35,16,176,35,40,16,161,31,0*7E\r\n",
  "$GPGSA,A,3,17,16,10,21,20,31,23,27,,,,,1.93,1.03,1.65*0C\r\n",
  "$GPGLL,3723.4845,N,12158.4435,W,161229.487,A,A*41\r\n",
  "$GPDTM,999,,0.0,S,0.0,W,,,N*45\r\n",
  "$GPBWC,081837,,,,,,T,,M,,N,*13\r\n",
  "$GPZDA,172809.456,21,07,2002,00,00*4D\r\n",
  "$GPXTE,A,A,0.67,L,N*6D\r\n",
  "$GPGGA,153049.99,3723.44788,N,12158.",
  "12345,W,1,10,0.8,18.5,M,-23.4,M,,0000*4F\r\n"
};

int main() {
  GPSense_init();
  GPSense_feed(nmea_test_data[4], strlen(nmea_test_data[4]));
  return 0;
}

```

## License

GPSense is licensed under the MIT License.

## Contributing

// TODO