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

#include "board.h"
#include "fsl_lpuart.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "logging.h"

#include "lpuart_gps.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* logging tool define */
#define gps_bit LOGGING_LPUART_GPS_BIT

#define gps_log_e(...) log_e(gps, ##__VA_ARGS__)
#define gps_log_w(...) log_w(gps, ##__VA_ARGS__)
#define gps_log_i(...) log_i(gps, ##__VA_ARGS__)
#define gps_log_d(...) log_d(gps, ##__VA_ARGS__)

/* lpuart buffer size for gps info transfer */
#define BUFFER_SIZE 128

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LPUART_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*!
     * time: (UTC)06:05:30.00;
     * latitude: 31,16.78005';
     * N: northern hemisphere;
     * longitude: 120,31.84986';
     * E: eastern hemisphere;
     * fix quality: 1(standard gps positioning);
     * satellite in use: 04;
     * hdop(horizontal accuracy): 2.20(0.5~99.9);
     * altitude: 9.6, unit: M;
     * height: 7.7, unit: M;
     */
uint8_t LOG_GNGGA[] = 
    "$GNGGA,060530.00,3116.78005,N,12031.84986,E,1,04,2.20,9.6,M,7.7,M,,*7C\r\n";

/*!
     * time: (UTC)06:05:36.00;
     * fix quality: A ;
     * latitude: 31,16.78116';
     * N: northern hemisphere;
     * longitude: 120,31.84786';
     * E: eastern hemisphere;
     * speed: 1.530*1.85=2.83km/h;
     * course: 322.05;
     * date: 2020/06/11;
     * fix quality: A;
     */
uint8_t LOG_GNRMC[] = 
    "$GNRMC,060536.00,A,3116.78116,N,12031.84786,E,1.530,322.05,110620,,,A,V*01\r\n";

/*!
     * ;
     */
uint8_t LOG_GNGLL[] = 
    "$GNGLL,3116.78185,N,12031.84742,E,060538.00,A,A*75\r\n";

/*!
     * ;
     */
uint8_t LOG_GPGSV[] = 
    "$GPGSV,3,1,11,01,16,050,18,02,,,35,03,44,061,30,06,37,260,32,0*59\r\n"
    "$GPGSV,3,2,11,17,56,343,29,19,39,317,28,22,,,20,24,00,311,,0*54\r\n"
    "$GPGSV,3,3,11,28,76,206,33,30,,,23,50,45,139,32,0*66\r\n"
    "$GBGSV,1,1,01,28,,,31,0*7E\r\n";

/*!
     * ;
     */
uint8_t LOG_GNGSA[] = 
    "$GNGSA,A,3,17,06,28,19,03,,,,,,,,4.00,1.82,3.56,1*0F\r\n"
    "$GNGSA,A,3,,,,,,,,,,,,,4.00,1.82,3.56,4*0B\r\n";

/*!
     * ;
     */       
uint8_t LOG_GNVTG[] = 
    "$GNVTG,,T,,M,0.027,N,0.050,K,A*3D\r\n";

/*!
     * ;
     */
uint8_t LOG_GBGSV[] = 
    "$GBGSV,1,1,01,28,,,31,0*7E\r\n";


gps_output_t gpou;

lpuart_handle_t g_lpuartHandle;

uint8_t g_rxBuffer[BUFFER_SIZE] = {0};

volatile uint16_t rxIndex;

volatile bool rxBufferEmpty            = true;
volatile bool rxOnGoing                = false;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLT(x)              ((gps_float_t)(x))
#define CRC_ADD(_gh, ch)    (_gh)->crc_calc ^= (uint8_t)(ch)
#define TERM_ADD(_gh, ch)   do { \
    if ((_gh)->term_pos < (sizeof((_gh)->term_str) - 1)) {  \
        (_gh)->term_str[(_gh)->term_pos++] = (ch);  \
        (_gh)->term_str[(_gh)->term_pos] = 0;   \
    }} while (0)
#define TERM_NEXT(_gh)  do { \
    (_gh)->term_str[((_gh)->term_pos = 0)] = 0; \
    (_gh)->term_num++; } while (0)

#define CIN(x)   ((x) >= '0' && (x) <= '9')
#define CIHN(x)  (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || \
                 ((x) >= 'A' && (x) <= 'F'))
#define CTN(x)   ((x) - '0')
#define CHTN(x)  (((x) >= '0' && (x) <= '9') ? ((x) - '0') : \
                 (((x) >= 'a' && (x) <= 'z') ? ((x) - 'a' + 10) : \
                 (((x) >= 'A' && (x) <= 'Z') ? ((x) - 'A' + 10) : 0)))

/*******************************************************************************
 * Code
 ******************************************************************************/

gps_float_t gps_transferFloatNumber(gps_output_t* g_out, const char* t) 
{
    gps_float_t res;
    if (t == NULL) 
    {
        t = g_out->term_str;
    }
    for(; t!= NULL && *t==' '; t++) {}

#if GPS_CFG_DOUBLE
    res = strtod(t, NULL);
#else /* GPS_CFG_DOUBLE */
    res = strtof(t, NULL);
#endif /* !GPS_CFG_DOUBLE */

    return FLT(res);
}

int32_t gps_transferInteger(gps_output_t* g_out, const char* t) 
{
    int32_t res = 0;
    uint8_t minus;

    if (t == NULL) 
    {
        t = g_out->term_str;
    }
    for (; t!=NULL && *t==' '; t++) {}

    minus = (*t == '-' ? (t++, 1) : 0);
    for (; t!=NULL && CIN(*t); t++) 
    {
        res = 10 * res + CTN(*t);
    }
    return minus ? -res : res;
}


void gps_init(gps_output_t* g_out)
{
    memset(g_out, 0x00, sizeof(gps_output_t));
}

uint8_t gps_checkCrc(gps_output_t* g_out) 
{
    uint8_t crc;
    crc = (uint8_t)((CHTN(g_out->term_str[0])&0x0F)<<0x04) | (CHTN(g_out->term_str[1])&0x0F);
    return g_out->crc_calc == crc;
}

// Parse latitude/longitude NMEA format to double
gps_float_t gps_parseLatiLongi(gps_output_t* g_out) 
{
    gps_float_t ll, deg, min;
    ll = gps_transferFloatNumber(g_out, NULL);
    deg = FLT((int)((int)ll / 100));
    min = ll - (deg * FLT(100));
    ll = deg + (min / FLT(60.0));
    return ll;
}

uint8_t gps_parse_term(gps_output_t* g_out)
{
    if(g_out->term_num == 0) 
    {
        if(0)
        {
        } 
        else if(!strncmp(g_out->term_str, "$GNGGA", 6)) 
        {
            g_out->stat = STAT_GNGGA;
        } 
        else if(!strncmp(g_out->term_str, "$GNRMC", 6)) 
        {
            g_out->stat = STAT_GNRMC;
        } 
        else 
        {
            g_out->stat = STAT_UNKNOWN;
        }
        return 0;
    }
    
    if (g_out->stat == STAT_UNKNOWN) 
    {

    }
    else if(g_out->stat == STAT_GNGGA)
    {
        switch(g_out->term_num)
        {
            case 1:
                g_out->gps_data.gngga.hours = 10*CTN(g_out->term_str[0]) + CTN(g_out->term_str[1]);
                g_out->gps_data.gngga.minutes = 10*CTN(g_out->term_str[2]) + CTN(g_out->term_str[3]);
                g_out->gps_data.gngga.seconds = 10*CTN(g_out->term_str[4]) + CTN(g_out->term_str[5]);
                g_out->b_time.hours = g_out->gps_data.gngga.hours + 8;  // beijing time
                g_out->b_time.minutes = g_out->gps_data.gngga.minutes;
                g_out->b_time.seconds = g_out->gps_data.gngga.seconds;
                break;
            case 2:
                g_out->gps_data.gngga.latitude = gps_parseLatiLongi(g_out);
                break;
            case 3:
                if(g_out->term_str[0] == 'S' || g_out->term_str[0] == 's') 
                {
                    g_out->gps_data.gngga.North2South = 'S';
                }
                else if(g_out->term_str[0] == 'N' || g_out->term_str[0] == 'n')
                {
                    g_out->gps_data.gngga.North2South = 'N';
                }
                break;
            case 4:
                g_out->gps_data.gngga.longitude = gps_parseLatiLongi(g_out);
                break;
            case 5:
                if(g_out->term_str[0] == 'W' || g_out->term_str[0] == 'w')
                {
                    g_out->gps_data.gngga.West2East = 'W';
                }
                else if(g_out->term_str[0] == 'E' || g_out->term_str[0] == 'e')
                {
                    g_out->gps_data.gngga.West2East = 'E';
                }
                break;
            case 6:
                g_out->gps_data.gngga.fix_quality = (uint8_t)gps_transferInteger(g_out, NULL);
                break;
            case 7:
                g_out->gps_data.gngga.sats_in_use = (uint8_t)gps_transferInteger(g_out, NULL);
                break;
            case 9:
                g_out->gps_data.gngga.altitude = gps_transferFloatNumber(g_out, NULL);
                break;
            case 11:
                g_out->gps_data.gngga.height = gps_transferFloatNumber(g_out, NULL);
                break;
            default:
                break;
        }
    }
    else if(g_out->stat == STAT_GNRMC)
    {
        switch (g_out->term_num)
        {  
            case 2:
              if(g_out->term_str[0] == 'A')
              {
                  g_out->gps_data.gnrmc.fix_quality = 'A';
              }
              else if(g_out->term_str[0] == 'V')
              {
                  g_out->gps_data.gnrmc.fix_quality = 'V';
              }
                break;
            case 7:
                g_out->gps_data.gnrmc.speed = gps_transferFloatNumber(g_out, NULL);
                g_out->gps_data.gnrmc.speed  = g_out->gps_data.gnrmc.speed * 1.85;
                break;
            case 8:
                g_out->gps_data.gnrmc.course = gps_transferFloatNumber(g_out, NULL);
                break;
            case 9:
                g_out->gps_data.gnrmc.date = (uint8_t)(10*CTN(g_out->term_str[0])+CTN(g_out->term_str[1]));
                g_out->gps_data.gnrmc.month = (uint8_t)(10*CTN(g_out->term_str[2])+CTN(g_out->term_str[3]));
                g_out->gps_data.gnrmc.year = (uint8_t)(10*CTN(g_out->term_str[4])+CTN(g_out->term_str[5]));
                g_out->b_time.date = g_out->gps_data.gnrmc.date;
                g_out->b_time.month = g_out->gps_data.gnrmc.month;
                g_out->b_time.year = g_out->gps_data.gnrmc.year;
                break;
            case 10:
                g_out->gps_data.gnrmc.variation = gps_transferFloatNumber(g_out, NULL);
                break;
            case 11:
                if (g_out->term_str[0] == 'W' || g_out->term_str[0] == 'w') 
                {
                    g_out->gps_data.gnrmc.variation = -g_out->gps_data.gnrmc.variation;
                }
                break;
            default:
                break;
        }
    }
    else
    {
        //other type of gps info, skip parsing so far
    }
    return 0;
}

void gps_printGnggaInfo(gps_output_t* g_out)
{
    if(g_out->gps_data.gngga.fix_quality == 1)
    {
        PRINTF("[Valid location] %02d:%02d:%02d %.1f%c %.1f%c (Alt)%.1f(m) (H)%.1f(m) \r\n",
               g_out->b_time.hours, g_out->b_time.minutes, g_out->b_time.seconds, 
               g_out->gps_data.gngga.latitude, g_out->gps_data.gngga.North2South, 
               g_out->gps_data.gngga.longitude, g_out->gps_data.gngga.West2East, 
               g_out->gps_data.gngga.altitude, g_out->gps_data.gngga.height);
    }
    else
    {
        PRINTF("[Invalid loctin] \r\n");
    }
}

void gps_printGnrmcInfo(gps_output_t* g_out)
{
    if(g_out->gps_data.gnrmc.fix_quality == 'A')
    {
        PRINTF("[Valid location] 20%02d/%02d/%02d (Spd)%.2f (km/h) (Course) %.1f \r\n",
               g_out->b_time.year, g_out->b_time.month, g_out->b_time.date, 
               g_out->gps_data.gnrmc.speed, g_out->gps_data.gnrmc.course);
    }
    else
    {
        PRINTF("[Invalid loctin] \r\n");
    }
}

void gps_processInfo(gps_output_t* g_out, const void* data, size_t len)
{
    const uint8_t* d = data;
    for (; len > 0; ++d, --len)
    {
        if (*d=='$')
        {
            memset(g_out, 0x00, sizeof(gps_output_t));
            TERM_ADD(g_out, *d);
        }
        else if(*d==',')
        {
            gps_parse_term(g_out); 
            CRC_ADD(g_out, *d);
            TERM_NEXT(g_out);
        }
        else if(*d=='*')
        {
            gps_parse_term(g_out); 
            g_out->star = 1; 
            TERM_NEXT(g_out);
        }
        else if(*d=='\r')
        {
            if (gps_checkCrc(g_out)) 
            {
                if(g_out->stat == STAT_GNGGA)
                {
                    gps_printGnggaInfo(g_out);
                }
                
                if(g_out->stat == STAT_GNRMC)
                {
                    gps_printGnrmcInfo(g_out);
                }
            }
            else
            {
                g_out->stat = STAT_INVALID;
            }
        }
        else
        {
            if(!g_out->star)
            {
                CRC_ADD(g_out, *d);
            }
            TERM_ADD(g_out, *d);
        }
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
void LPUART_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{    
    if (kStatus_LPUART_RxIdle == status)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
    }
}

void lpuart_gps_task(void *pvParameters)
{
    lpuart_config_t config;   
    lpuart_transfer_t receiveXfer;
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 9600U;
    config.enableTx     = true;
    config.enableRx     = true;
    
    size_t receivedBytes = 0U;
    
    LPUART_Init(LPUART11, &config, BOARD_DebugConsoleSrcFreq());
    LPUART_TransferCreateHandle(LPUART11, &g_lpuartHandle, LPUART_UserCallback, NULL);   
    
    receiveXfer.data     = g_rxBuffer;
    receiveXfer.dataSize = BUFFER_SIZE;
    
    gps_init(&gpou);
    
    while (1)
    {  
        if (!rxBufferEmpty)
        {         
            gps_processInfo(&gpou, g_rxBuffer, BUFFER_SIZE);
            rxBufferEmpty = true;            
        }

        if ((!rxOnGoing) && rxBufferEmpty)
        {
            rxOnGoing = true;
            LPUART_TransferReceiveNonBlocking(LPUART11, &g_lpuartHandle, &receiveXfer, &receivedBytes);
            if (BUFFER_SIZE == receivedBytes)
            {
                rxBufferEmpty = false;
                rxOnGoing     = false;
            }
        }
    }
}

