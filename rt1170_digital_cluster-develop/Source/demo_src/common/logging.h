/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DG_LOGGING_H
#define DG_LOGGING_H
#include "shell_color.h"

#define LOG_LVL_DEBUG  (0U)
#define LOG_LVL_INFO   (1U)
#define LOG_LVL_WARN   (2U)
#define LOG_LVL_ERROR  (3U)
 
#ifdef LOGGING_CONFIG_ENABLE
#define log(mod_name, mod_bit, lvl, fmt, ...) \
    do {\
	    if ((g_logflags & (1U << mod_bit)) != 0 && lvl >= g_loglevel) { \
              PRINTF("[%s]%s%s" NOCOLOR fmt "\n\r", mod_name, g_log_color[lvl], g_log_level[lvl], ##__VA_ARGS__); \
		}\
	} while(0)

#define log_e(mod_name, fmt, ...) log(#mod_name, mod_name##_bit, LOG_LVL_ERROR, fmt, ##__VA_ARGS__)
#define log_w(mod_name, fmt, ...) log(#mod_name, mod_name##_bit, LOG_LVL_WARN, fmt, ##__VA_ARGS__)
#define log_i(mod_name, fmt, ...) log(#mod_name, mod_name##_bit, LOG_LVL_INFO, fmt, ##__VA_ARGS__)
#define log_d(mod_name, fmt, ...) log(#mod_name, mod_name##_bit, LOG_LVL_DEBUG, fmt, ##__VA_ARGS__)
#else /* LOGGING_CONFIG_ENABLE */
#define log(...)
#define log_e(...)
#define log_w(...) 
#define log_i(...)
#define log_d(...)
#endif /* LOGGING_CONFIG_ENABLE */

extern uint32_t g_loglevel;
extern uint32_t g_logflags;
extern const char *g_log_color[];
extern const char *g_log_level[];
 
#if defined(__cplusplus)
extern "C" {
#endif
  
void LogCmdRegister(void);

#if defined(__cplusplus)
}
#endif
#endif /* DG_LOGGING_H */