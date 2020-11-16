/*
 * Copyright (c) 2020, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * GPS NMEA PARSER
 * MIT License
 *
 * Copyright (c) 2020 Tilen MAJERLE
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#ifndef _LPUART_GPS_H_
#define _LPUART_GPS_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if GPS_CFG_DOUBLE
typedef double gps_float_t;
#else
typedef float gps_float_t;
#endif

typedef enum {
    STAT_INVALID    = -1,       /* crc check failed */
    STAT_UNKNOWN    = 0,
    STAT_GNGGA      = 1,        /* GGA: Global Positioning System Fix Data */
    STAT_GNRMC      = 2,        /* RMC: Recommended minimum specific GPS/Transit data */
    STAT_GNGLL      = 3,        /* GLL: Geographic Position, Latitude/Longitude and time */        
    STAT_GPGSV      = 4,        /* GSV: GPS Satellites in view */
    STAT_GNGSA      = 5,        /* GSA: GPS DOP and active satellites */
    STAT_GNVTG      = 6,        /* VTG: Track Made Good and Ground Speed */
    STAT_GBGSV      = 7,        
} gps_statement_t;

typedef struct{
    uint8_t hours;              /* UTC of hour */
    uint8_t minutes;            /* UTC of minute */
    uint8_t seconds;            /* UTC of second */
    gps_float_t latitude;       /* latitude of position */
    char North2South;           /* northern or southern hemisphere */
    gps_float_t longitude;      /* longitude of position */
    char West2East;             /* western or eastern hemisphere */
    uint8_t fix_quality;
    uint8_t sats_in_use;
    gps_float_t hdop;           /* Relative accuracy of horizontal position */
    gps_float_t altitude;       /* altitude of position */
    uint8_t altitude_unit;
    gps_float_t height;         /* height of position */
    uint8_t height_unit;
} gngga_info_t;

typedef struct{
    uint8_t hours;              /* UTC of hour */
    uint8_t minutes;            /* UTC of minute */
    uint8_t seconds;            /* UTC of second */
    char fix_quality;           /* A for valid position, V for invalid position */
    gps_float_t latitude;       /* latitude of position */
    char North2South;           /* northern or southern hemisphere */
    gps_float_t longitude;      /* longitude of position */
    char West2East;             /* western or eastern hemisphere */
    gps_float_t speed;          /* speed over ground */
    gps_float_t course;         /* course */
    gps_float_t variation;      /* magnetic variation degrees */
    uint8_t date;               /* UT date */
    uint8_t month;              /* UT month */
    uint8_t year;               /* UT year */
} gnrmc_info_t;

typedef struct {
    uint8_t hours;              /* BTC of hour, UTC+8 */
    uint8_t minutes;            /* BTC of minute */
    uint8_t seconds;            /* BTC of second */
    uint8_t date;               /* BT date */
    uint8_t month;              /* BT month */
    uint8_t year;               /* BT year */
} btc_time_t;
        
typedef struct{
    gps_statement_t stat;
    char term_str[13];
    uint8_t term_pos;
    uint8_t term_num;
    uint8_t star;
    uint8_t crc_calc;
    btc_time_t b_time;
    union {
        uint8_t dummy_data;
        gngga_info_t gngga;
        gnrmc_info_t gnrmc;
    } gps_data;
} gps_output_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* _LPUART_GPS_H_*/