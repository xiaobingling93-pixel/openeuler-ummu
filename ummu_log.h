/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#ifndef _UMMU_LOG_H_
#define _UMMU_LOG_H_
#include <stdbool.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

typedef enum ummu_vlog_level {
	UMMU_VLOG_LEVEL_EMERG = 0,
	UMMU_VLOG_LEVEL_ALERT = 1,
	UMMU_VLOG_LEVEL_CRIT = 2,
	UMMU_VLOG_LEVEL_ERR = 3,
	UMMU_VLOG_LEVEL_WARNING = 4,
	UMMU_VLOG_LEVEL_NOTICE = 5,
	UMMU_VLOG_LEVEL_INFO = 6,
	UMMU_VLOG_LEVEL_DEBUG = 7,
	UMMU_VLOG_LEVEL_MAX = 8,
} ummu_vlog_level_t;

#define MAX_LOG_LEN 256
#define UMMU_MAPT_LOG_TAG "LogTag_UMMU_MAPT"

extern ummu_vlog_level_t g_loglevel;

static inline bool ummu_log_drop(ummu_vlog_level_t level)
{
	return ((level > g_loglevel) ? true : false);
}

static void ummu_log(const char *function, int line, ummu_vlog_level_t level, const char *format, ...)
	__attribute__ ((format (gnu_printf, 4, 5)));
static void ummu_log(const char *function, int line, ummu_vlog_level_t level, const char *format, ...)
{
	va_list va;
	int ret;

	va_start(va, format);
	char newformat[MAX_LOG_LEN + 1] = {0};
	char logmsg[MAX_LOG_LEN + 1] = {0};

	/* add log head info, "UMMU_MAPT_LOG_TAG|function|[line]|format" */
	ret = snprintf(newformat, MAX_LOG_LEN, "[%d]%s|%s[%d]|%s", getpid(), UMMU_MAPT_LOG_TAG, function, line, format);
	if (ret <= 0 || ret >= (int)sizeof(newformat)) {
		va_end(va);
		return;
	}

	ret = vsnprintf(logmsg, MAX_LOG_LEN, newformat, va);
	if (ret == -1) {
		(void)printf("logmsg size exceeds MAX_LOG_LEN size : %d\n", MAX_LOG_LEN);
		va_end(va);
		return;
	}

	syslog((int)level, "%s", logmsg);
	va_end(va);
}

#define UMMU_LOG(l, ...)						    \
	if (!ummu_log_drop(UMMU_VLOG_LEVEL_##l)) {			      \
		ummu_log(__func__, __LINE__, UMMU_VLOG_LEVEL_##l, __VA_ARGS__);     \
	}

#define UMMU_MAPT_INFO_LOG(...) UMMU_LOG(INFO, __VA_ARGS__)

#define UMMU_MAPT_ERROR_LOG(...) UMMU_LOG(ERR, __VA_ARGS__)

#define UMMU_MAPT_WARN_LOG(...) UMMU_LOG(WARNING, __VA_ARGS__)

#define UMMU_MAPT_DEBUG_LOG(...) UMMU_LOG(DEBUG, __VA_ARGS__)

#endif

