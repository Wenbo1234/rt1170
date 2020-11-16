#define SD8977  // SD8801

//# if !(defined (_WINSOCK_H) || defined (_WINSOCKAPI_) || defined (__USE_W32_SOCKETS))
/*  The standard <select.h> uses Windows Socket to exclude the time header file. 
    For Embedded system, LWIP Socket is used. Need to define the WinSock macro here
    to avoid to include the time defintion for Socket. */
#define __USE_W32_SOCKETS

#define CONFIG_MLAN_WMSDK 1
#define CONFIG_WMM 1
#define CONFIG_11N 1
#define CONFIG_MAX_AP_ENTRIES 10
#define CONFIG_5GHz_SUPPORT 1
#define CONFIG_SDIO_MULTI_PORT_RX_AGGR 1
#define CONFIG_STA_AMPDU_RX 1
#define CONFIG_STA_AMPDU_TX 1
#define CONFIG_ENABLE_AMSDU_RX 1

/* Wi-Fi Internal */
#define INCLUDE_FROM_MLAN 1
#define WLAN_LOW_POWER_ENABLE 1
#define STA_SUPPORT 1
#define UAP_SUPPORT 1
#define WPA 1
#define KEY_MATERIAL_WEP 1
#define KEY_PARAM_SET_V2 1
#define ENABLE_802_11W 1

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS 1
#define CONFIG_ENABLE_WARNING_LOGS 1
#define CONFIG_DEBUG_BUILD 1
#define CONFIG_DEBUG_OUTPUT 1

/* WLCMGR debug */
#undef CONFIG_WLCMGR_DEBUG

/*
 * Wifi extra debug options
 */
#undef CONFIG_WIFI_EXTRA_DEBUG
#undef CONFIG_WIFI_EVENTS_DEBUG
#undef CONFIG_WIFI_CMD_RESP_DEBUG
#undef CONFIG_WIFI_SCAN_DEBUG
#undef CONFIG_WIFI_IO_INFO_DUMP
#undef CONFIG_WIFI_IO_DEBUG
#undef CONFIG_WIFI_IO_DUMP
#undef CONFIG_WIFI_MEM_DEBUG
#undef CONFIG_WIFI_AMPDU_DEBUG
#undef CONFIG_WIFI_TIMER_DEBUG
#undef CONFIG_WIFI_SDIO_DEBUG
