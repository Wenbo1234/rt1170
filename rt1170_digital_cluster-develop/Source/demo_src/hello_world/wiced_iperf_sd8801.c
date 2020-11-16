/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/   
#include "stdint.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sdmmc_host.h"
#include "fsl_debug_console.h"

#include "logging.h"

/* change the following macro in the wifi_config.h, default SD8977  */
#if defined(SD8801)
#include "sd8801_wlan.h"
#elif defined(SD8977)
#include "sduart8977_wlan_bt.h"
#endif

#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"

#include "lwiperf.h"
#include "lwip/tcpip.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* logging tool define */
#define sdio_wifi_mrvl_bit LOGGING_SDIO_MRVL_BIT

#define sdio_mrvl_log_e(...) log_e(sdio_wifi_mrvl, ##__VA_ARGS__)
#define sdio_mrvl_log_w(...) log_w(sdio_wifi_mrvl, ##__VA_ARGS__)
#define sdio_mrvl_log_i(...) log_i(sdio_wifi_mrvl, ##__VA_ARGS__)
#define sdio_mrvl_log_d(...) log_d(sdio_wifi_mrvl, ##__VA_ARGS__)

/* Common WiFi parameters */
/* for SD8801, I tried the NXPOPEN but cannot connected successfully, so need mobile's wifi */
/* for SD8977, everything works fine, so set the default wifi to sd8977 for a stable performance */
#ifndef WIFI_SSID
#define WIFI_SSID "NXPOPEN"           
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "Use4Internet" 
#endif

#define MAX_RETRY_TICKS 50
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
struct iperf_test_context
{
    bool server_mode;
    bool tcp;
    enum lwiperf_client_type client_type;
    void *iperf_session;
};

struct wlan_network nxp_network;
int retry_cnt     = 3;          //try to join the nwk after 3 times
int connect_retry = 10;
enum wlan_connection_state state;
static int network_added = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void menuConnectToAP(void)
{
    int ret;
    do
    {
        sdio_mrvl_log_i("Connecting to %s .....\r\n", nxp_network.ssid);

        ret = wlan_connect(nxp_network.name);

        if (ret != 0)
        {
            sdio_mrvl_log_e("Failed to connect %d\r\n", ret);
            goto retry;
        }

        connect_retry = 1000;

        do
        {
            /* wait for interface up */
            os_thread_sleep(os_msec_to_ticks(50));

            if (wlan_get_connection_state(&state))
            {
                sdio_mrvl_log_e("Error: unable to get connection state\r\n");
                continue;
            }

            if (state == WLAN_CONNECTED || connect_retry == 0)
                break;
        } while (connect_retry--);

        if (state != WLAN_CONNECTED)
        {
            sdio_mrvl_log_e("Error: Not connected.\r\n");
            goto retry;
        }

        return;
    retry:
        retry_cnt--;
        sdio_mrvl_log_e("Connect to AP FAIL ! Retrying ..... \r\n");

    } while (retry_cnt != 0);

    if (retry_cnt == 0)
    {
        os_thread_sleep(os_msec_to_ticks(500));

        sdio_mrvl_log_e("Connection Failed !");

        ret = wlan_disconnect();
        if (ret != 0)
            sdio_mrvl_log_e("Failed to disconnect \r\n");
    }
}

static int wlan_event_callback(enum wlan_event_reason event, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ip[16];
    wifi_fw_version_ext_t ver;

    /* After successful wlan subsytem initialization */
    if (event == WLAN_REASON_INITIALIZED)
    {
        wifi_set_packet_retry_count(MAX_RETRY_TICKS);

        uint32_t tsf_high, tsf_low;
        int ret;
        uint8_t network_name_len = 0;
        uint8_t ssid_len         = 0;
        uint8_t psk_len          = 0;

        ret = wlan_get_tsf(&tsf_high, &tsf_low);

        if (ret == WM_SUCCESS)
            sdio_mrvl_log_d("TSF: %d.%d\r\n", tsf_high, tsf_low);

        sdio_mrvl_log_i("WLAN is Initialized\r\n");

        /* Print WLAN FW Version */
        wifi_get_device_firmware_version_ext(&ver);
        sdio_mrvl_log_d("WLAN FW Version: %s\r\n", ver.version_str);

        if (!network_added)
        {
          //set the network name, ssid and password, in event WLAN_REASON_INITIALIZED   
          memset(&nxp_network, 0, sizeof(struct wlan_network));

            network_name_len = (strlen("nxp_network") < WLAN_NETWORK_NAME_MAX_LENGTH) ? (strlen("nxp_network") + 1) :
                                                                                        WLAN_NETWORK_NAME_MAX_LENGTH;
            strncpy(nxp_network.name, "nxp_network", network_name_len);

            ssid_len = (strlen(WIFI_SSID) <= MAX_SSID_LEN) ? strlen(WIFI_SSID) : MAX_SSID_LEN;
            memcpy(nxp_network.ssid, (const char *)WIFI_SSID, ssid_len);
            nxp_network.ip.ipv4.addr_type = ADDR_TYPE_DHCP;
            nxp_network.ssid_specific     = 1;

            if (strlen(WIFI_PASSWORD))
            {
                nxp_network.security.type = WLAN_SECURITY_WILDCARD;
                psk_len = (strlen(WIFI_PASSWORD) <= (WLAN_PSK_MAX_LENGTH - 1)) ? strlen(WIFI_PASSWORD) :
                                                                                     (WLAN_PSK_MAX_LENGTH - 1);
                strncpy(nxp_network.security.psk, WIFI_PASSWORD, psk_len);
                nxp_network.security.psk_len = psk_len;
            }
            else
            {
                nxp_network.security.type = WLAN_SECURITY_NONE;
            }
            
            //add this network
            ret = wlan_add_network(&nxp_network);

            if (ret != 0)
            {
                sdio_mrvl_log_e(" Failed to add network %d\r\n", ret);
                return 0;
            }
            network_added = 1;
        }
    }

    if (event == WLAN_REASON_UAP_SUCCESS)
    {
        sdio_mrvl_log_d("Soft AP \"%s\" Started successfully\r\n", WIFI_SSID);
        if (dhcp_server_start(net_get_uap_handle()))
            sdio_mrvl_log_e("Error in starting dhcp server\r\n");

        sdio_mrvl_log_d("DHCP Server started successfully\r\n");
    }

    if (event == WLAN_REASON_UAP_CLIENT_ASSOC)
    {
        sdio_mrvl_log_i("Client => ");
        print_mac((const char *)data);
        sdio_mrvl_log_d("Associated with Soft AP\r\n");
    }

    if (event == WLAN_REASON_UAP_CLIENT_DISSOC)
    {
        sdio_mrvl_log_i("Client => ");
        print_mac((const char *)data);
        sdio_mrvl_log_d("Dis-Associated from Soft AP\r\n");
    }

    if (event == WLAN_REASON_SUCCESS)
    {
        ret = wlan_get_address(&addr);
        if (ret != WM_SUCCESS)
        {
            sdio_mrvl_log_e("failed to get IP address");
            return 0;
        }
        net_inet_ntoa(addr.ipv4.address, ip);

        sdio_mrvl_log_i("Connected to following BSS:");
        sdio_mrvl_log_d("SSID = [%s], IP = [%s]\r\n", nxp_network.ssid, ip);
    }

    if (event == WLAN_REASON_USER_DISCONNECT)
    {
        sdio_mrvl_log_d("Dis-connected from: %s\r\n", nxp_network.ssid);
    }

    return 0;
}

void iperf_sd8801_task(void *arg)
{
    int32_t result = 0;
    (void)result;
    
    sdio_mrvl_log_i("Digital Cluster sd8801 wifi task\r\n");

    /* Initialize WIFI Driver */
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
    assert(WM_SUCCESS == result);

    result = wlan_start(wlan_event_callback);
    assert(WM_SUCCESS == result);

    os_thread_sleep(1000);
    
    menuConnectToAP();
    
    while (1)
    {
        vTaskDelay(500);
    }
}