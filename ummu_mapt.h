/* SPDX-License-Identifier: MIT */
/*
 * Copyright (c) 2025 HiSilicon Technologies Co., Ltd. All rights reserved.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */

#ifndef _UMMU_MAPT_H_
#define _UMMU_MAPT_H_

#include <stdint.h>
#include <pthread.h>
#include "ummu_api.h"

/* =============================================== common data structure ======================================== */

#define MAX_LEVEL_INDEX 3U

struct ummu_ctx_info {
        uint32_t tid_cnt;
        pthread_mutex_t ctx_mutex;
        void *mapt_map;
        int shared_fd;
};

/* =========================================== common global variable ============================================ */

#endif
